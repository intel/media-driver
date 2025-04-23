/*
* Copyright (c) 2021, Intel Corporation
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
//!
//! \file     media_interfaces_next.cpp
//! \brief    Helps with gen-specific factory creation.
//!

#include <stdint.h>
#include "igfxfmid.h"
#include "media_factory.h"
#include "mhw_utilities_next.h"
#include "mos_defs.h"
#include "mos_os.h"
#include "mos_os_specific.h"
#include "mos_utilities.h"
#include "mhw_cp_interface.h"

#include "media_interfaces_vphal.h"
#include "media_interfaces_renderhal.h"
#include "media_interfaces_mcpy_next.h"
#include "media_interfaces_mmd_next.h"
#include "media_interfaces_mhw_next.h"
#include "media_interfaces_codechal_next.h"

template class MediaFactory<uint32_t, VphalDevice>;
template class MediaFactory<uint32_t, RenderHalDevice>;
template class MediaFactory<uint32_t, CodechalDeviceNext>;
template class MediaFactory<uint32_t, MhwInterfacesNext>;
template class MediaFactory<uint32_t, McpyDeviceNext>;
template class MediaFactory<uint32_t, MmdDeviceNext>;

typedef MediaFactory<uint32_t, VphalDevice> VphalFactory;
typedef MediaFactory<uint32_t, RenderHalDevice> RenderHalFactory;
typedef MediaFactory<uint32_t, CodechalDeviceNext> CodechalFactoryNext;
typedef MediaFactory<uint32_t, MhwInterfacesNext> MhwFactoryNext;
typedef MediaFactory<uint32_t, McpyDeviceNext> McpyFactoryNext;
typedef MediaFactory<uint32_t, MmdDeviceNext> MmdFactoryNext;

VpBase *VphalDevice::CreateFactoryNext(
    PMOS_INTERFACE     osInterface,
    MOS_CONTEXT_HANDLE osDriverContext,
    MOS_STATUS         *eStatus,
    bool               clearViewMode)
{
    VpBase                *vpBase            = nullptr;
    VphalDevice           *vphalDevice       = nullptr;
    PLATFORM              platform;

    if (eStatus == nullptr)
    {
        VP_DEBUG_ASSERTMESSAGE("Invalid null pointer.");
        return nullptr;
    }

    // pOsInterface not provided - use default for the current OS
    if (osInterface == nullptr)
    {
        osInterface = (PMOS_INTERFACE)MOS_AllocAndZeroMemory(sizeof(MOS_INTERFACE));

        if (osInterface == nullptr)
        {
            VP_DEBUG_ASSERTMESSAGE("Allocate OS interface failed");
            *eStatus = MOS_STATUS_NO_SPACE;
            return nullptr;
        }

        if (MOS_STATUS_SUCCESS != Mos_InitInterface(osInterface, osDriverContext, COMPONENT_VPCommon))
        {
            VP_DEBUG_ASSERTMESSAGE("Initailze OS interface failed");
            MOS_FreeMemAndSetNull(osInterface);
            *eStatus = MOS_STATUS_NO_SPACE;
            return nullptr;
        }

        osInterface->bDeallocateOnExit = true;
    }
    else
    // pOsInterface provided - use OS interface functions provided by DDI (OS emulation)
    {
        // Copy OS interface structure, save context
        osInterface->pOsContext = (PMOS_CONTEXT)osDriverContext;
        osInterface->bDeallocateOnExit = false;
    }

    // Initialize platform
    osInterface->pfnGetPlatform(osInterface, &platform);

    vphalDevice = VphalFactory::Create(platform.eProductFamily);

    if (vphalDevice == nullptr)
    {
        VP_DEBUG_ASSERTMESSAGE("Failed to create MediaInterface on the given platform!");
        if (osInterface->bDeallocateOnExit)
        {
            MOS_FreeMemAndSetNull(osInterface);
        }
        *eStatus = MOS_STATUS_NO_SPACE;
        return nullptr;
    }

    if (vphalDevice->Initialize(osInterface, true, eStatus) != MOS_STATUS_SUCCESS)
    {
        VP_DEBUG_ASSERTMESSAGE("VPHal interfaces were not successfully allocated!");

        // If m_vpBase has been created, osInterface should be released in VphalState::~VphalState.
        if (osInterface->bDeallocateOnExit && nullptr == vphalDevice->m_vpBase)
        {
            // Deallocate OS interface structure (except if externally provided)
            if (osInterface->pfnDestroy)
            {
                osInterface->pfnDestroy(osInterface, true);
            }
            MOS_FreeMemAndSetNull(osInterface);
        }

        vphalDevice->Destroy();
        MOS_Delete(vphalDevice);

        *eStatus = MOS_STATUS_NO_SPACE;
        return nullptr;
    }

    vpBase = vphalDevice->m_vpBase;
   
    MOS_Delete(vphalDevice);
    return vpBase;
}

MOS_STATUS VphalDevice::CreateVPMhwInterfaces(
    bool                              sfcNeeded,
    bool                              veboxNeeded,
    std::shared_ptr<mhw::vebox::Itf>  &veboxItf,
    std::shared_ptr<mhw::sfc::Itf>    &sfcItf,
    std::shared_ptr<mhw::mi::Itf>     &miItf,
    PMOS_INTERFACE                    osInterface)
{
    MhwInterfacesNext               *mhwInterfaces = nullptr;
    MhwInterfacesNext::CreateParams params         = {};
    params.Flags.m_sfc   = sfcNeeded;
    params.Flags.m_vebox = veboxNeeded;

    mhwInterfaces = MhwInterfacesNext::CreateFactory(params, osInterface);
    if (mhwInterfaces)
    {
        veboxItf = mhwInterfaces->m_veboxItf;
        sfcItf   = mhwInterfaces->m_sfcItf;
        miItf    = mhwInterfaces->m_miItf;

        // MhwInterfaces always create CP and MI interfaces, so we have to delete those we don't need.
        Delete_MhwCpInterface(mhwInterfaces->m_cpInterface);
        mhwInterfaces->m_cpInterface = nullptr;
        MOS_Delete(mhwInterfaces);
        return MOS_STATUS_SUCCESS;
    }

    VP_PUBLIC_ASSERTMESSAGE("Allocate MhwInterfaces failed");
    return MOS_STATUS_NO_SPACE;
}

void VphalDevice::Destroy()
{
    MOS_Delete(m_vpBase);
    MOS_Delete(m_vpPipeline);
    MOS_Delete(m_vpPlatformInterface);
}

XRenderHal_Platform_Interface* RenderHalDevice::CreateFactory(
        PMOS_INTERFACE osInterface)
{
    RenderHalDevice *device = nullptr;
    PLATFORM platform = {};
    osInterface->pfnGetPlatform(osInterface, &platform);
    device = RenderHalFactory::Create(platform.eProductFamily);
    if (device == nullptr)
    {
        return nullptr;
    }
    device->m_osInterface = osInterface;
    device->Initialize();
    if (device->m_renderhalDevice == nullptr)
    {
        MHW_ASSERTMESSAGE("RenderHal device creation failed!");
        MOS_Delete(device);
        return nullptr;
    }

    XRenderHal_Platform_Interface *pRet = device->m_renderhalDevice;
    MOS_Delete(device);
    return pRet;
}

Codechal* CodechalDeviceNext::CreateFactory(
    PMOS_INTERFACE  osInterface,
    PMOS_CONTEXT    osDriverContext,
    void            *standardInfo,
    void            *settings)
{
#define FAIL_DESTROY(msg)                   \
{                                           \
    CODECHAL_PUBLIC_ASSERTMESSAGE((msg));   \
    if (mhwInterfaces != nullptr)           \
    {                                       \
        mhwInterfaces->Destroy();           \
    }                                       \
    if (osInterface != nullptr && osInterface->bDeallocateOnExit) \
    {                                       \
        if (osInterface->pfnDestroy != nullptr) \
        {                                   \
             osInterface->pfnDestroy(osInterface, false); \
        }                                   \
        MOS_FreeMemory(osInterface);        \
    }                                       \
    MOS_Delete(mhwInterfaces);              \
    MOS_Delete(device);                     \
    return nullptr;                         \
}

#define FAIL_CHK_STATUS(stmt)               \
{                                           \
    if ((stmt) != MOS_STATUS_SUCCESS)       \
    {                                       \
        FAIL_DESTROY("Status check failed, CodecHal creation failed!"); \
    }                                       \
}

#define FAIL_CHK_NULL(ptr)                  \
{                                           \
    if ((ptr) == nullptr)                   \
    {                                       \
        FAIL_DESTROY("nullptr check failed, CodecHal creation failed!"); \
    }                                       \
}

    if (osDriverContext == nullptr ||
        standardInfo == nullptr)
    {
        return nullptr;
    }

    MhwInterfacesNext *mhwInterfaces = nullptr;
    CodechalDeviceNext *device = nullptr;

    CODECHAL_FUNCTION CodecFunction = ((PCODECHAL_STANDARD_INFO)standardInfo)->CodecFunction;

    // pOsInterface not provided - use default for the current OS
    if (osInterface == nullptr)
    {
        osInterface =
            (PMOS_INTERFACE)MOS_AllocAndZeroMemory(sizeof(MOS_INTERFACE));
        FAIL_CHK_NULL(osInterface);

        MOS_COMPONENT component = CodecHalIsDecode(CodecFunction) ? COMPONENT_Decode : COMPONENT_Encode;

        osInterface->bDeallocateOnExit = true;
        FAIL_CHK_STATUS(Mos_InitInterface(osInterface, osDriverContext, component));
    }
    // pOsInterface provided - use OS interface functions provided by DDI (OS emulation)
    else
    {
        osInterface->bDeallocateOnExit = false;
    }

    MhwInterfacesNext::CreateParams params;
    MOS_ZeroMemory(&params, sizeof(params));
    params.Flags.m_render = true;
    params.Flags.m_sfc = true;
    params.Flags.m_vdboxAll = true;
    params.Flags.m_vebox = true;
    params.m_heapMode = (uint8_t)2;
    params.m_isDecode = CodecHalIsDecode(CodecFunction);
    mhwInterfaces = MhwInterfacesNext::CreateFactory(params, osInterface);
    FAIL_CHK_NULL(mhwInterfaces);

    PLATFORM platform = {};
    osInterface->pfnGetPlatform(osInterface, &platform);
    device = CodechalFactoryNext::Create(platform.eProductFamily + MEDIA_EXT_FLAG);
    if(device == nullptr)
    {
        device = CodechalFactoryNext::Create(platform.eProductFamily);
    }
    FAIL_CHK_NULL(device);
    FAIL_CHK_STATUS(device->Initialize(standardInfo, settings, mhwInterfaces, osInterface));
    FAIL_CHK_NULL(device->m_codechalDevice);

    Codechal *codechalDevice = device->m_codechalDevice;

    MOS_Delete(mhwInterfaces);
    MOS_Delete(device);

    return codechalDevice;
}

MhwInterfacesNext* MhwInterfacesNext::CreateFactory(
    CreateParams params,
    PMOS_INTERFACE osInterface)
{
    if (osInterface == nullptr)
    {
        return nullptr;
    }
    PLATFORM platform = {};
    osInterface->pfnGetPlatform(osInterface, &platform);

    MhwInterfacesNext *mhwNext = nullptr;

    mhwNext = MhwFactoryNext::Create(platform.eProductFamily + MEDIA_EXT_FLAG);
    if(mhwNext == nullptr)
    {
        mhwNext = MhwFactoryNext::Create(platform.eProductFamily);
    }

    if (mhwNext == nullptr)
    {
        MHW_ASSERTMESSAGE("Failed to create MediaInterface on the given platform!");
        return nullptr;
    }

    if (mhwNext->Initialize(params, osInterface) != MOS_STATUS_SUCCESS)
    {
        MHW_ASSERTMESSAGE("MHW interfaces were not successfully allocated!");
        MOS_Delete(mhwNext);
        return nullptr;
    }

    return mhwNext;
}

void MhwInterfacesNext::Destroy()
{
    if(m_isDestroyed)
    {
        return;
    }

    if(m_osInterface)
    {
        m_osInterface->pfnDeleteMhwCpInterface(m_cpInterface);
        m_cpInterface = nullptr;
    }
    else
    {
        MHW_ASSERTMESSAGE("Failed to destroy cpInterface.");
    }
    MOS_Delete(m_stateHeapInterface);
}

void* McpyDeviceNext::CreateFactory(
    MOS_CONTEXT_HANDLE    osDriverContext)
{
    PMOS_INTERFACE     osInterface   = nullptr;
    McpyDeviceNext    *device        = nullptr;

    auto deleterOnFailure = [&](bool deleteOsInterface) {
        if (deleteOsInterface && osInterface != nullptr)
        {
            if (osInterface->pfnDestroy)
            {
                osInterface->pfnDestroy(osInterface, false);
            }
            MOS_FreeMemory(osInterface);
        }
        MOS_Delete(device);
    };

    if (osDriverContext == nullptr)
    {
        MHW_ASSERTMESSAGE("Invalid(null) pOsDriverContext!");
        return nullptr;
    }

    osInterface = (PMOS_INTERFACE)MOS_AllocAndZeroMemory(sizeof(MOS_INTERFACE));
    if (osInterface == nullptr)
    {
        return nullptr;
    }
    if (Mos_InitInterface(
            osInterface,
            osDriverContext,
            COMPONENT_MCPY) != MOS_STATUS_SUCCESS)
    {
        deleterOnFailure(true);
        return nullptr;
    }

    PLATFORM platform = {};
    osInterface->pfnGetPlatform(osInterface, &platform);
    device = McpyFactoryNext::Create(platform.eProductFamily);
    if (device == nullptr)
    {
        deleterOnFailure(true);
        return nullptr;
    }

    // transfer ownership of osInterface. No need to delete it if init fails
    device->Initialize(osInterface);
    if (device->m_mcpyDevice == nullptr)
    {
        deleterOnFailure(false);
        return nullptr;
    }

    void *mcpyDevice = device->m_mcpyDevice;
    MOS_Delete(device);
    return mcpyDevice;
}

MhwInterfacesNext* McpyDeviceNext::CreateMhwInterface(PMOS_INTERFACE osInterface)
{
    MhwInterfacesNext::CreateParams params;
    MOS_ZeroMemory(&params, sizeof(params));
    params.Flags.m_render = true;
    params.Flags.m_vebox  = true;
    params.Flags.m_blt    = true;

    // the destroy of interfaces happens when the mcpy deviced deconstructor funcs
    MhwInterfacesNext *mhw = MhwInterfacesNext::CreateFactory(params, osInterface);

    return mhw;
}

#ifdef _MMC_SUPPORTED
void* MmdDeviceNext::CreateFactory(PMOS_CONTEXT osDriverContext)
{
#define MMD_FAILURE()                                       \
{                                                           \
    if (mhwInterfaces != nullptr)                           \
    {                                                       \
        mhwInterfaces->Destroy();                           \
    }                                                       \
    MOS_Delete(mhwInterfaces);                              \
    if (osInterface != nullptr)                             \
    {                                                       \
        if (osInterface->pfnDestroy)                        \
        {                                                   \
            osInterface->pfnDestroy(osInterface, false);    \
        }                                                   \
        MOS_FreeMemory(osInterface);                        \
    }                                                       \
    MOS_Delete(device);                                     \
    return nullptr;                                         \
}
    MHW_FUNCTION_ENTER;

    if (osDriverContext == nullptr)
    {
        MHW_ASSERTMESSAGE("Invalid(null) pOsDriverContext!");
        return nullptr;
    }

    PMOS_INTERFACE osInterface = nullptr;
    MhwInterfacesNext* mhwInterfaces = nullptr;
    MmdDeviceNext* device = nullptr;

    osInterface = (PMOS_INTERFACE)MOS_AllocAndZeroMemory(sizeof(MOS_INTERFACE));
    if (osInterface == nullptr)
    {
        return nullptr;
    }
    if (Mos_InitInterface(
        osInterface,
        osDriverContext,
        COMPONENT_MEMDECOMP) != MOS_STATUS_SUCCESS)
    {
        MMD_FAILURE();
    }

    PLATFORM platform = {};
    osInterface->pfnGetPlatform(osInterface, &platform);
    device = MmdFactoryNext::Create(platform.eProductFamily);
    if (device == nullptr)
    {
        MMD_FAILURE();
    }

    mhwInterfaces = device->CreateMhwInterface(osInterface);
    if (mhwInterfaces == nullptr)
    {
        MMD_FAILURE();
    }
    MOS_STATUS status = device->Initialize(osInterface, mhwInterfaces);
    if (device->m_mmdDevice == nullptr)
    {
        if (MOS_STATUS_UNINITIALIZED == status)
        {
            MMD_FAILURE();
        }
        else
        {
            MOS_Delete(mhwInterfaces);
            MOS_Delete(device);
            return nullptr;
        }
    }

    void* mmdDevice = device->m_mmdDevice;
    MOS_Delete(mhwInterfaces);
    MOS_Delete(device);

    return mmdDevice;
}

MhwInterfacesNext* MmdDeviceNext::CreateMhwInterface(PMOS_INTERFACE osInterface)
{
    MhwInterfacesNext::CreateParams params;
    MOS_ZeroMemory(&params, sizeof(params));
    params.Flags.m_vebox = true;
    MhwInterfacesNext* mhw = MhwInterfacesNext::CreateFactory(params, osInterface);

    if (mhw)
    {
        MOS_Delete(mhw->m_veboxInterface);
    }


    return mhw;
}
#endif
