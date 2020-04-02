/*
* Copyright (c) 2017, Intel Corporation
*
* Permission is hereby granted, free of charge, to any person obtaining a
* copy of this software and associated documentation files (the "Software"),
* to deal in the Software without restriction, including without limitation
* the rights to use, copy, modify, merge, publish, distribute, sublicense,
* and/or sell copies of the Software, and to permit persons to whom the
* Software is furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included
* in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
* OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
* OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
* ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
* OTHER DEALINGS IN THE SOFTWARE.
*/
#include "cm_device.h"

#include <dlfcn.h>
#include <cstdio>

#include "cm_mem.h"
#include "cm_surface_manager.h"
#include "cm_queue.h"
#include "cm_timer.h"
#include "cm_debug.h"
#include "cm_extension_creator.h"

#if USE_EXTENSION_CODE
#include "cm_gtpin_external_interface.h"
#endif

#include <unistd.h>
#include <fcntl.h>

#ifndef ANDROID
uint32_t CmDevice_RT::m_vaReferenceCount = 0;
CSync CmDevice_RT::m_vaReferenceCountCriticalSection;
void  *CmDevice_RT::m_vaDrm = nullptr;
pfVAGetDisplayDRM CmDevice_RT::m_vaGetDisplayDrm = nullptr;
#endif
// current binary version, query by command "strings",
//       e.g. "strings  -a igfxcmrt64.so | grep current_version "
volatile static char cmrtCurrentVersion[] = "cmrt_current_version: " \
                          "6.0.0.9010\0";

CSync gDeviceCreationCriticalSection;

int32_t CmDevice_RT::Create( CmDevice_RT* &device, uint32_t createOption )
{
    INSERT_PROFILER_RECORD();

    int32_t result = CM_SUCCESS;

    device = new CmDevice_RT(nullptr, createOption);
    if( device )
    {
        result = device->Initialize(true);
        if( result != CM_SUCCESS )
        {
            CmAssert(0);
            Destroy(device);
        }
    }
    else
    {
        CmAssert( 0 );
        result = CM_OUT_OF_HOST_MEMORY;
    }

    return result;
}

int32_t CmDevice_RT::Create(VADisplay &vaDisplay , CmDevice_RT* &device, uint32_t createOption )
{
    INSERT_PROFILER_RECORD();

    int32_t result = CM_FAILURE;

    device = new (std::nothrow) CmDevice_RT(vaDisplay, createOption);
    if( device )
    {
        result = device->Initialize(false);
        if( result != CM_SUCCESS )
        {
            Destroy( device );
        }
    }
    else
    {
        CmAssert( 0 );
        result = CM_OUT_OF_HOST_MEMORY;
    }

    // leave critical section
    return result;
}

int32_t CmDevice_RT::Destroy( CmDevice_RT* &device )
{
    if(device == nullptr)
    {
        return CM_FAILURE;
    }

    // Destroy the cm device object
    device->FreeResources();

    //Destroy the Device at CMRT@UMD
    CM_DESTROYCMDEVICE_PARAM destroyCmDeviceParam;
    CmSafeMemSet(&destroyCmDeviceParam, 0, sizeof(CM_DESTROYCMDEVICE_PARAM));
    destroyCmDeviceParam.cmDeviceHandle = device->m_deviceInUmd;
    uint32_t inputDataLen = sizeof(CM_DESTROYCMDEVICE_PARAM);

    int32_t result = device->OSALExtensionExecute(CM_FN_DESTROYCMDEVICE,
                                                  &destroyCmDeviceParam,
                                                  inputDataLen);

    CmSafeRelease( device );
    CHK_FAILURE_RETURN(result);

    // leave critical section
    return destroyCmDeviceParam.returnValue;
}

CmDevice_RT::CmDevice_RT(
        VADisplay vaDisplay,
        uint32_t createOption
):
    m_cmVersion(0),
    m_deviceInUmd(nullptr),
    m_cmCreated ( true ),
    m_vaDisplay    (vaDisplay),
#ifdef ANDROID
    m_display       (nullptr),
#endif
    m_fvaCmExtSendReqMsg (nullptr),
    m_gtpinEnabled( false ),
    m_gtpinBufferUP0( nullptr ),
    m_gtpinBufferUP1( nullptr ),
    m_gtpinBufferUP2( nullptr ),
    m_createOption(createOption),
    m_driverStoreEnabled(0),
    m_driFileDescriptor(0)
{

    // New Surface Manager
    m_surfaceManager = new CmSurfaceManager(this);

    // New Kernel Debugger
    m_kernelDebugger = CmExtensionCreator<CmKernelDebugger>::CreateClass();
    
    //Initialize L3 cache config
    CmSafeMemSet(&m_l3Config, 0, sizeof(L3ConfigRegisterValues));

}

CmDevice_RT::~CmDevice_RT( void )
{
    if (m_cmCreated)
    {
        vaTerminate(m_vaDisplay);
#ifndef ANDROID
        FreeLibvaDrm();
#else
        free(m_display); //Android
#endif
    }

    if (m_kernelDebugger != nullptr)
    {
        delete m_kernelDebugger;
    }
}

int32_t CmDevice_RT::FreeResources()
{
    //Destroy Queue
    m_criticalSectionQueue.Acquire();
    for (auto iter = m_queue.begin(); iter != m_queue.end();)
    {
        if (*iter != nullptr)
        {
            CmQueue_RT::Destroy(*iter);
        }
        iter = m_queue.erase(iter);
    }
    m_criticalSectionQueue.Release();

    //Destroy GTPin Used BufferUp
    if( m_gtpinBufferUP0 != nullptr)
    {
        DestroyBufferUP(m_gtpinBufferUP0);
    }

    if( m_gtpinBufferUP1 != nullptr)
    {
        DestroyBufferUP(m_gtpinBufferUP1);
    }

    if( m_gtpinBufferUP2 != nullptr)
    {
        DestroyBufferUP(m_gtpinBufferUP2);
    }

    CmSafeRelease(m_surfaceManager);

    return CM_SUCCESS;
}

static int32_t CmrtVaSurfaceRelease(void *vaDisplay, void *vaSurface)
{
    VAStatus   vaStatus       = VA_STATUS_SUCCESS;
    VADisplay  *display       = (VADisplay *)(vaDisplay);

    //Destroy VaSurface
    vaStatus = vaDestroySurfaces(*display, (VASurfaceID *)vaSurface, 1);

    return vaStatus;
}

int32_t CmDevice_RT::Initialize( bool isCmCreated )
{
    int32_t result = CM_SUCCESS;

    m_cmCreated = isCmCreated;

    CLock locker(gDeviceCreationCriticalSection);

    CHK_RET(InitializeLibvaDisplay());

    CHK_RET(CreateDeviceInUmd());

    CHK_RET(CheckDdiVersionSupported(m_cmVersion));

#if USE_EXTENSION_CODE
    if(GTpinVariables.GTPinEnabled)
    {
        CHK_RET(EnableGtpin());
        CHK_RET(RegisterGtpinMarkerFunctions());

    }
#endif
    if (m_kernelDebugger != nullptr)
    {
        m_kernelDebugger->NotifyNewDevice(this, m_deviceInUmd, m_driverStoreEnabled);
    }

finish:
    return result;
}

int32_t CmDevice_RT::CreateDeviceInUmd()
{
    CmDeviceCreationParam createCmDeviceParam;
    CmSafeMemSet(&createCmDeviceParam, 0, sizeof(createCmDeviceParam));
    createCmDeviceParam.returnValue        = CM_FAILURE;
    createCmDeviceParam.createOption       = m_createOption;
    createCmDeviceParam.releaseSurfaceFunc = &CmrtVaSurfaceRelease;
    uint32_t inputDataLen = sizeof(createCmDeviceParam);

    int32_t result = OSALExtensionExecute(CM_FN_CREATECMDEVICE,
                                          &createCmDeviceParam, inputDataLen);

    CHK_FAILURE_RETURN(result);
    CHK_FAILURE_RETURN(createCmDeviceParam.returnValue);

    m_cmVersion = createCmDeviceParam.version;
    m_deviceInUmd = createCmDeviceParam.deviceHandleInUmd;
    m_driverStoreEnabled = createCmDeviceParam.driverStoreEnabled;
    return CM_SUCCESS;
}

//!
//! Create Libva Surface and wrap it as a CmSurface
//! It is CALLER's responsibility to allocation memory for all pointers to CmSurface2D 
//! Input :
//!     1) Surface's width  [in]
//!     2) Surface's height [in]
//!     3) Surface's format [in]
//!     4) Reference to created VASurfaceID [out]
//!     5) Reference to pointer of created Cm Surface [out]
//! Output: 
//!     CM_SUCCESS if all CmSurface2D are successfully created;
//!     CM_VA_SURFACE_NOT_SUPPORTED if libva surface creation fail;
//!     CM_FAILURE otherwise;
CM_RT_API int32_t CmDevice_RT::CreateVaSurface2D( uint32_t width, uint32_t height, CM_SURFACE_FORMAT format, VASurfaceID &vaSurface, CmSurface2D* &surface)
{
    INSERT_PROFILER_RECORD();

    return m_surfaceManager->CreateVaSurface2D(width, height, format, vaSurface, surface);
}

//!
//!
//! Create a CmSurface2D from an existing LIBVA surface 
//! Input :
//!     Reference to the pointer to the CmSurface2D .
//!     VASurfaceID 
//! Output: 
//!     CM_SUCCESS if the CmSurface2D are successfully created;
//!     CM_OUT_OF_HOST_MEMORY if out of system memory;
//!     CM_FAILURE otherwise;
//!
CM_RT_API int32_t CmDevice_RT::CreateSurface2D( VASurfaceID vaSurface, CmSurface2D* &surface )
{
    INSERT_PROFILER_RECORD();

    return m_surfaceManager->CreateSurface2D(vaSurface, surface);
}

//!
//! Create an array of CmSurface2D from an existing array of LIBVA surfaces, which are created by LIBVA's vaCreateSurfaces
//! It is CALLER's responsibility to allocation memory for all pointers to CmSurface2D 
//! Input :
//!     1) Pointer to the array of pointers pointing to LIBVA surface
//!     2) array size
//!     3) Pointer to the array of pointers pointing to CmSurface2D .
//! Output: 
//!     CM_SUCCESS if all CmSurface2D are successfully created;
//!     CM_OUT_OF_HOST_MEMORY if out of system memory;
//!     CM_FAILURE otherwise;
CM_RT_API int32_t CmDevice_RT::CreateSurface2D( VASurfaceID* vaSurfaceArray, const uint32_t surfaceCount, CmSurface2D**  surfaceArray)
{
    INSERT_PROFILER_RECORD();

    return m_surfaceManager->CreateSurface2D(vaSurfaceArray, surfaceCount, surfaceArray);
}

int32_t CmDevice_RT::OSALExtensionExecute(uint32_t functionId,
                                          void *inputData,
                                          uint32_t inputDataLength,
                                          void **resourceList,
                                          uint32_t resourceCount)
{
    CmAssert(inputData);

    //    uint32_t functionId    = functionId;
    //    void* inputData    = pInputData;
    //    uint32_t inputDataLen  = iInputDataLen;

    void* outputData   = m_deviceInUmd; // pass cm device handle to umd
    uint32_t outputDataLen = sizeof(m_deviceInUmd);
    uint32_t vaModuleId    = VAExtModuleCMRT;
    VAStatus hr        = VA_STATUS_SUCCESS;

    if (m_fvaCmExtSendReqMsg != nullptr)
    {
        hr = m_fvaCmExtSendReqMsg(m_vaDisplay, &vaModuleId, &functionId, inputData, &inputDataLength , 0, outputData , &outputDataLen);
    }
    return hr;
}

//Initalize LibVA's VADisplay
int32_t CmDevice_RT::InitializeLibvaDisplay()
{
    if ( m_cmCreated )
    {
        VAStatus vaStatus = VA_STATUS_SUCCESS;
        int vaMajorVersion, vaMinorVersion;

#ifndef ANDROID
    int32_t ret = GetLibvaDisplayDrm(m_vaDisplay);
    if ( ret != CM_SUCCESS)
    {
        CmAssert(0);
        return ret;
    }
#else
    m_display = (Display*)malloc(sizeof(Display));
    if (m_display == nullptr)
    {
        fprintf(stderr, "Can't connect X server!\n");
        return CM_INVALID_LIBVA_INITIALIZE;
    }

    *(m_display) = ANDROID_DISPLAY;
    m_vaDisplay = vaGetDisplay(m_display);
    if (m_vaDisplay == nullptr)
    {
        return CM_INVALID_LIBVA_INITIALIZE;
    }
#endif  //ANDROID

        vaStatus = vaInitialize(m_vaDisplay, &vaMajorVersion, &vaMinorVersion);
        if ( VA_STATUS_SUCCESS != vaStatus ) {
            return CM_INVALID_LIBVA_INITIALIZE;
        }
    }

    m_fvaCmExtSendReqMsg = (pvaCmExtSendReqMsg) vaGetLibFunc (m_vaDisplay, "vaCmExtSendReqMsg");

    if (m_fvaCmExtSendReqMsg == nullptr) {
        fprintf(stderr, "Cannot get function of m_fvaCmExtSendReqMsg!\n");
        return CM_INVALID_LIBVA_INITIALIZE;
    }
    else
    {
        return CM_SUCCESS;
    }
}

CM_RT_API int32_t CmDevice_RT::GetVaDpy(VADisplay* & vaDpy)
{
    INSERT_PROFILER_RECORD();

    vaDpy = &m_vaDisplay;
    return CM_SUCCESS;
}

#ifndef ANDROID
int32_t CmDevice_RT::GetLibvaDisplayDrm(VADisplay & vaDisplay)
{
    pfVAGetDisplayDRM vaGetDisplayDRM = nullptr;
    char *dlSymErr = nullptr;
    void *hLibVaDRM = nullptr;

    CLock locker(m_vaReferenceCountCriticalSection);

    if(m_vaReferenceCount > 0)
    {
        vaGetDisplayDRM    = m_vaGetDisplayDrm;
        m_vaReferenceCount ++;
    }
    else
    {
        //Load libva-drm.so
        dlerror();
        hLibVaDRM = dlopen( "libva-drm.so", RTLD_LAZY );

        if (!hLibVaDRM)
        {
            if ((dlSymErr = dlerror()) != nullptr)
            {
                fprintf(stderr, "%s\n", dlSymErr);
            }
            return CM_INVALID_LIBVA_INITIALIZE;
        }

        //dynamically load function vaGetDisplayDRM from libva-drm.so
        dlerror();
        vaGetDisplayDRM = (pfVAGetDisplayDRM)dlsym(hLibVaDRM, "vaGetDisplayDRM");
        if ((dlSymErr= dlerror()) != nullptr)  {
            fprintf(stderr, "%s\n", dlSymErr);
            return CM_INVALID_LIBVA_INITIALIZE;
        }

        m_vaReferenceCount ++;
        m_vaDrm        = hLibVaDRM;
        m_vaGetDisplayDrm = vaGetDisplayDRM;
    }

    // open the GPU device
    m_driFileDescriptor = open("/dev/dri/renderD128", O_RDWR);
    if (m_driFileDescriptor < 0)
    {
        fprintf(stderr,"Failed to open GPU device file node\n");
        return CM_INVALID_LIBVA_INITIALIZE;
    }

    if (m_vaGetDisplayDrm == nullptr)
    {
        fprintf(stderr, "m_vaGetDisplayDrm should not be nullptr.\n");
        return CM_INVALID_LIBVA_INITIALIZE;
    }

    // get the display handle.
    if (vaGetDisplayDRM == nullptr)
    {
        fprintf(stderr, "vaGetDisplayDRM should not be nullptr.\n");
        return CM_INVALID_LIBVA_INITIALIZE;
    }
    vaDisplay            = vaGetDisplayDRM(m_driFileDescriptor);

    return CM_SUCCESS;
 }

int32_t CmDevice_RT::FreeLibvaDrm()
{
    CLock locker(m_vaReferenceCountCriticalSection);
    if(m_vaReferenceCount > 1)
    {
        m_vaReferenceCount --;
    }
    else
    {
        dlclose(m_vaDrm);
        m_vaDrm  = nullptr;
        m_vaGetDisplayDrm = nullptr;

        m_vaReferenceCount --;
    }

    if ( m_driFileDescriptor != -1)
    {
        close(m_driFileDescriptor);
        m_driFileDescriptor = -1;
    }
    return CM_SUCCESS;
}
#endif

