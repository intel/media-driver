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
#ifndef CMRTLIB_LINUX_HARDWARE_CM_DEVICE_H_
#define CMRTLIB_LINUX_HARDWARE_CM_DEVICE_H_

#include "cm_device_base.h"
#include "cm_def_hw.h"
#include "cm_kernel_debugger.h"
#include <vector>

class CmQueue_RT;
class CmSurfaceManager;

class CmDevice_RT : public CmDevice
{
public:
    static int32_t GetSupportedRenderer(uint32_t &count);
    static int32_t Create( CmDevice_RT* &device, uint32_t createOption );
    static int32_t Destroy( CmDevice_RT* &device );
    static int32_t Create(VADisplay &vaDisplay, CmDevice_RT* &device,uint32_t createOption );

    CM_RT_API int32_t CreateBuffer(uint32_t size, CmBuffer* &buffer);

    CM_RT_API int32_t CreateSurface2D(uint32_t width, uint32_t height, CM_SURFACE_FORMAT format, CmSurface2D* &surface );
    CM_RT_API int32_t CreateSurface3D(uint32_t width, uint32_t height, uint32_t depth, CM_SURFACE_FORMAT format, CmSurface3D* &surface );

    CM_RT_API int32_t CreateSurface2D( VASurfaceID vaSurface, CmSurface2D* &surface );
    CM_RT_API int32_t CreateSurface2D( VASurfaceID* vaSurfaceArray, const uint32_t surfaceCount, CmSurface2D **surfaceArray);

    CM_RT_API int32_t DestroySurface( CmBuffer* &buffer);
    CM_RT_API int32_t DestroySurface( CmSurface2D* &surface2d);
    CM_RT_API int32_t DestroySurface( CmSurface3D* &surface3d);

    CM_RT_API int32_t CreateQueue( CmQueue* & queue);

    CM_RT_API int32_t LoadProgram( void* commonISACode, const uint32_t size, CmProgram*& program,  const char* options = nullptr );

    CM_RT_API int32_t CreateKernel( CmProgram* program, const char* kernelName, CmKernel* & kernel, const char* options = nullptr);
    CM_RT_API int32_t CreateKernel( CmProgram* program, const char* kernelName, const void * fncPnt, CmKernel* & kernel, const char* options = nullptr) {return CM_FAILURE;}
    CM_RT_API int32_t CreateSampler( const CM_SAMPLER_STATE & sampleState, CmSampler* &sampler );

    CM_RT_API int32_t CreateTask(CmTask *& task);
    CM_RT_API int32_t GetCaps(CM_DEVICE_CAP_NAME capName, size_t& capValueSize, void* capValue );
    CM_RT_API int32_t CreateThreadSpace( uint32_t width, uint32_t height, CmThreadSpace* &threadSpace);

    CM_RT_API int32_t DestroyKernel( CmKernel*& kernel);
    CM_RT_API int32_t DestroySampler( CmSampler*& sampler );
    CM_RT_API int32_t DestroyProgram( CmProgram*& program );
    CM_RT_API int32_t DestroySampler8x8( CmSampler8x8 *& sampler );
    CM_RT_API int32_t DestroyThreadSpace( CmThreadSpace* &threadSpace);
    CM_RT_API int32_t DestroyTask(CmTask *& task);

    CM_RT_API int32_t CreateVmeSurfaceG7_5 (CmSurface2D* currentSurface, CmSurface2D** forwardSurfaceArray, CmSurface2D** backwardSurfaceArray, const uint32_t surfaceCountForward, const uint32_t surfaceCountBackward, SurfaceIndex* & vmeSurfaceIndex );
    CM_RT_API int32_t DestroyVmeSurfaceG7_5(SurfaceIndex* & vmeSurfaceIndex);
    CM_RT_API int32_t SetVmeSurfaceStateParam(SurfaceIndex* vmeSurfaceIndex, CM_VME_SURFACE_STATE_PARAM *newState);

    CM_RT_API int32_t CreateBufferUP(uint32_t size, void *sysMem, CmBufferUP* &buffer);
    CM_RT_API int32_t DestroyBufferUP( CmBufferUP* &buffer);

    CM_RT_API int32_t GetSurface2DInfo(uint32_t width, uint32_t height, CM_SURFACE_FORMAT format, uint32_t & pitch, uint32_t & physicalSize );
    CM_RT_API int32_t CreateSurface2DUP( uint32_t width, uint32_t height, CM_SURFACE_FORMAT format, void* sysMem, CmSurface2DUP* &surface );
    CM_RT_API int32_t DestroySurface2DUP( CmSurface2DUP* &surface) ;

    CM_RT_API int32_t CreateSampler8x8(const CM_SAMPLER_8X8_DESCR  &samplerDescriptor, CmSampler8x8* &sampler);
    CM_RT_API int32_t CreateSampler8x8Surface(CmSurface2D* surface2d, SurfaceIndex* &sampler8x8SurfaceIndex, CM_SAMPLER8x8_SURFACE surfaceType, CM_SURFACE_ADDRESS_CONTROL_MODE addressControl = CM_SURFACE_CLAMP);
    CM_RT_API int32_t DestroySampler8x8Surface(SurfaceIndex* &sampler8x8SurfaceIndex);

    CM_RT_API int32_t CreateThreadGroupSpace( uint32_t threadSpaceWidth, uint32_t threadSpaceHeight, uint32_t groupSpaceWidth, uint32_t groupSpaceHeight, CmThreadGroupSpace* &threadGroupSpace);
    CM_RT_API int32_t DestroyThreadGroupSpace(CmThreadGroupSpace* &threadGroupSpace);

    CM_RT_API int32_t SetL3Config(const L3ConfigRegisterValues *registerValues);
    CM_RT_API int32_t SetSuggestedL3Config( L3_SUGGEST_CONFIG configIndex);

    CM_RT_API int32_t SetCaps(CM_DEVICE_CAP_NAME capName, size_t capValueSize, void* capValue );

    CM_RT_API int32_t CreateSamplerSurface2D(CmSurface2D* surface2d, SurfaceIndex* & samplerSurface2dIndex);
    CM_RT_API int32_t CreateSamplerSurface3D(CmSurface3D* surface3d, SurfaceIndex* & samplerSurface3dIndex);
    CM_RT_API int32_t DestroySamplerSurface(SurfaceIndex* & samplerSurfaceIndex);

    CM_RT_API int32_t InitPrintBuffer(size_t size = CM_DEFAULT_PRINT_BUFFER_SIZE);

    CM_RT_API int32_t FlushPrintBuffer();

    CM_RT_API int32_t CreateVebox( CmVebox* & vebox );
    CM_RT_API int32_t DestroyVebox(CmVebox* & vebox);

    CM_RT_API int32_t CreateBufferSVM(uint32_t size, void* & sysMem, uint32_t accessFlag, CmBufferSVM* &buffer );
    CM_RT_API int32_t DestroyBufferSVM( CmBufferSVM* & buffer);

    CM_RT_API int32_t CloneKernel( CmKernel * &destKernel, CmKernel *srcKernel );

    CM_RT_API int32_t GetVaDpy(VADisplay* &vaDisplay);
    CM_RT_API int32_t CreateVaSurface2D( uint32_t width, uint32_t height, CM_SURFACE_FORMAT format, VASurfaceID & vaSurface, CmSurface2D* & surface);

    CM_RT_API int32_t CreateSamplerSurface2DUP(CmSurface2DUP* surface2dUP, SurfaceIndex* & samplerSurface2dUPIndex);

    CM_RT_API int32_t CreateSurface2DAlias(CmSurface2D* originalSurface, SurfaceIndex* &aliasIndex);

    CM_RT_API int32_t CreateHevcVmeSurfaceG10(CmSurface2D* currentSurface, CmSurface2D** forwardSurfaceArray, CmSurface2D** backwardSurfaceArray, const uint32_t surfaceCountForward, const uint32_t surfaceCountBackward, SurfaceIndex* & vmeSurfaceIndex);
    CM_RT_API int32_t DestroyHevcVmeSurfaceG10(SurfaceIndex* & vmeSurfaceIndex);
    CM_RT_API int32_t CreateSamplerEx( const CM_SAMPLER_STATE_EX & samplerState, CmSampler* & sampler ) ;

    CM_RT_API int32_t FlushPrintBufferIntoFile(const char *filename);
    CM_RT_API int32_t CreateThreadGroupSpaceEx(uint32_t threadSpaceWidth, uint32_t threadSpaceHeight, uint32_t threadSpaceDepth, uint32_t groupSpaceWidth, uint32_t groupSpaceHeight, uint32_t groupSpaceDepth, CmThreadGroupSpace* &threadGroupSpace);
    CM_RT_API int32_t CreateSampler8x8SurfaceEx(CmSurface2D* surface2d, SurfaceIndex* &sampler8x8SurfaceIndex, CM_SAMPLER8x8_SURFACE surfaceType, CM_SURFACE_ADDRESS_CONTROL_MODE addressControl = CM_SURFACE_CLAMP, CM_FLAG* flag = nullptr);
    CM_RT_API int32_t CreateSamplerSurface2DEx(CmSurface2D* surface2d, SurfaceIndex* & samplerSurface2dIndex, CM_FLAG* flag);
    CM_RT_API int32_t CreateBufferAlias(CmBuffer *originalBuffer, SurfaceIndex* &aliasIndex);

    CM_RT_API int32_t GetVISAVersion(uint32_t& majorVersion, uint32_t& minorVersion);
    CM_RT_API int32_t CreateQueueEx(CmQueue *&queue, CM_QUEUE_CREATE_OPTION queueCreateOption = CM_DEFAULT_QUEUE_CREATE_OPTION);

    CM_RT_API int32_t CreateBufferStateless(size_t size,
                                            uint32_t option,
                                            void *sysMem,
                                            CmBufferStateless *&pBufferStateless);
    CM_RT_API int32_t DestroyBufferStateless(CmBufferStateless *&buffer);

    CM_RT_API int32_t DispatchTask() { return CM_SUCCESS; }

    CM_RT_API int32_t CreateSurface2DStateless(uint32_t width,
                                               uint32_t height,
                                               uint32_t &pitch,
                                               CmSurface2DStateless *&pSurface);

    CM_RT_API int32_t DestroySurface2DStateless(CmSurface2DStateless *&pSurface);

    int32_t CheckDdiVersionSupported(const uint32_t ddiVersion);

    int32_t OSALExtensionExecute(uint32_t functionId,
                                 void *inputData,
                                 uint32_t inputDataLength,
                                 void **resourceList = nullptr,
                                 uint32_t resourceCount = 0);

protected:
    CmDevice_RT(
        VADisplay vaDisplay,
        uint32_t createOption
    );

    ~CmDevice_RT( void );

    int32_t SetCapsInternal(CM_DEVICE_CAP_NAME capName, size_t capValueSize, void* capValue);

    int32_t CreateProgram(void* commonISACode,
                          const uint32_t size,
                          CmProgram*& program,
                          const char* options );

    int32_t CreateDeviceInUmd();

    int32_t FlushPrintBufferInternal(const char *filename);

    CmSurfaceManager *         m_surfaceManager;

    uint32_t m_cmVersion;
    void    *m_deviceInUmd;    //CmDevice pointer in UMD
    bool    m_cmCreated;

    int32_t Initialize(bool isCmCreated, uint32_t Index = 0);
    int32_t FreeResources();
#if USE_EXTENSION_CODE
    int32_t EnableGtpin( void );
    int32_t RegisterGtpinMarkerFunctions();
#endif
    int32_t CreateVmeSurface( CmSurface2D* currentSurface, CmSurface2D** forwardSurfaceArray, CmSurface2D** backwardSurfaceArray, const uint32_t surfaceCountForward, const uint32_t surfaceCountBackward, SurfaceIndex* & vmeSurfaceIndex, CM_FUNCTION_ID functionName);
    int32_t DestroyVmeSurface( SurfaceIndex* & vmeSurfaceIndex );

#ifndef ANDROID
    int32_t FreeLibvaDrm();
    int32_t GetLibvaDisplayDrm(VADisplay & vaDisplay);
#endif

    int32_t InitializeLibvaDisplay(uint32_t Index = 0);
    VADisplay m_vaDisplay;
    uint32_t m_drmIndex;
    pvaCmExtSendReqMsg    m_fvaCmExtSendReqMsg;

#if !defined(ANDROID)
    static uint32_t m_vaReferenceCount;
    static CSync m_vaReferenceCountCriticalSection;
    int32_t m_driFileDescriptor;
    static void  *m_vaDrm;
    static pfVAGetDisplayDRM m_vaGetDisplayDrm;
#else
    Display*  m_display;
#endif

    L3ConfigRegisterValues m_l3Config;

    // Variables used in GTPIN
    bool            m_gtpinEnabled;
    CmBufferUP*     m_gtpinBufferUP0;
    CmBufferUP*     m_gtpinBufferUP1;
    CmBufferUP*     m_gtpinBufferUP2;

    uint32_t       m_createOption;
    uint32_t       m_driverStoreEnabled;

    // Kernel debugger
    CmKernelDebugger *m_kernelDebugger;

    // Queue list and Queue critical section
    CSync          m_criticalSectionQueue;
    std::vector<CmQueue_RT *> m_queue;

private:
    CmDevice_RT(const CmDevice_RT &other);
    CmDevice_RT &operator=(const CmDevice_RT &other);
};

#endif  // #ifndef CMRTLIB_LINUX_HARDWARE_CM_DEVICE_H_
