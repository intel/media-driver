/*
* Copyright (c) 2017-2019, Intel Corporation
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
//! \file     media_interfaces.cpp
//! \brief    Helps with gen-specific factory creation.
//!

#include "media_interfaces_mhw.h"
#include "media_interfaces_codechal.h"
#include "media_interfaces_mmd.h"
#include "media_interfaces_cmhal.h"
#include "media_interfaces_mosutil.h"
#include "media_interfaces_vphal.h"
#include "media_interfaces_renderhal.h"
#include "media_interfaces_nv12top010.h"
#include "media_interfaces_decode_histogram.h"

#include "mhw_cp_interface.h"
#include "mhw_mi.h"
#include "mhw_render.h"
#include "mhw_sfc.h"
#include "mhw_state_heap.h"
#include "mhw_vebox.h"
#include "mhw_vdbox_mfx_interface.h"
#include "mhw_vdbox_hcp_interface.h"
#include "mhw_vdbox_huc_interface.h"
#include "mhw_vdbox_vdenc_interface.h"

template class MediaInterfacesFactory<MhwInterfaces>;
template class MediaInterfacesFactory<MmdDevice>;
template class MediaInterfacesFactory<MosUtilDevice>;
template class MediaInterfacesFactory<CodechalDevice>;
template class MediaInterfacesFactory<CMHalDevice>;
template class MediaInterfacesFactory<VphalDevice>;
template class MediaInterfacesFactory<RenderHalDevice>;
template class MediaInterfacesFactory<Nv12ToP010Device>;
template class MediaInterfacesFactory<DecodeHistogramDevice>;

typedef MediaInterfacesFactory<MhwInterfaces> MhwFactory;
typedef MediaInterfacesFactory<MmdDevice> MmdFactory;
typedef MediaInterfacesFactory<MosUtilDevice> MosUtilFactory;
typedef MediaInterfacesFactory<CodechalDevice> CodechalFactory;
typedef MediaInterfacesFactory<CMHalDevice> CMHalFactory;
typedef MediaInterfacesFactory<VphalDevice> VphalFactory;
typedef MediaInterfacesFactory<RenderHalDevice> RenderHalFactory;
typedef MediaInterfacesFactory<Nv12ToP010Device> Nv12ToP010Factory;
typedef MediaInterfacesFactory<DecodeHistogramDevice> DecodeHistogramFactory;

VphalState* VphalDevice::CreateFactory(
    PMOS_INTERFACE  osInterface,
    PMOS_CONTEXT    osDriverContext,
    MOS_STATUS      *eStatus)
{
    VphalState  *vphalState  = nullptr;
    VphalDevice *vphalDevice = nullptr;
    PLATFORM    platform;

    if (eStatus == nullptr)
    {
        VPHAL_DEBUG_ASSERTMESSAGE("Invalid null pointer.");
        return nullptr;
    }

    // pOsInterface not provided - use default for the current OS
    if (osInterface == nullptr)
    {
        osInterface = (PMOS_INTERFACE)MOS_AllocAndZeroMemory(sizeof(MOS_INTERFACE));

        if (osInterface == nullptr)
        {
            VPHAL_DEBUG_ASSERTMESSAGE("Allocate OS interface failed");
            *eStatus = MOS_STATUS_NO_SPACE;
            return nullptr;
        }

        if (MOS_STATUS_SUCCESS != Mos_InitInterface(osInterface, osDriverContext, COMPONENT_VPCommon))
        {
            VPHAL_DEBUG_ASSERTMESSAGE("Initailze OS interface failed");
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

    vphalDevice = VphalFactory::CreateHal(platform.eProductFamily);

    if (vphalDevice == nullptr)
    {
        VPHAL_DEBUG_ASSERTMESSAGE("Failed to create MediaInterface on the given platform!");
        if (osInterface->bDeallocateOnExit)
        {
            MOS_FreeMemAndSetNull(osInterface);
        }
        *eStatus = MOS_STATUS_NO_SPACE;
        return nullptr;
    }

    if (vphalDevice->Initialize(osInterface, osDriverContext, eStatus) != MOS_STATUS_SUCCESS)
    {
        VPHAL_DEBUG_ASSERTMESSAGE("VPHal interfaces were not successfully allocated!");

         // If m_vphalState has been created, osInterface should be released in VphalState::~VphalState.
        if (osInterface->bDeallocateOnExit && nullptr == vphalDevice->m_vphalState)
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

    vphalState = vphalDevice->m_vphalState;
    MOS_Delete(vphalDevice);

    return vphalState;
}

MhwInterfaces* MhwInterfaces::CreateFactory(
    CreateParams params,
    PMOS_INTERFACE osInterface)
{
    if (osInterface == nullptr)
    {
        return nullptr;
    }
    PLATFORM platform = {};
    osInterface->pfnGetPlatform(osInterface, &platform);
    MhwInterfaces *mhw =nullptr;

    mhw = MhwFactory::CreateHal(platform.eProductFamily + MEDIA_EXT_FLAG);
    if(mhw == nullptr)
    {
        mhw = MhwFactory::CreateHal(platform.eProductFamily);
    }

    if (mhw == nullptr)
    {
        MHW_ASSERTMESSAGE("Failed to create MediaInterface on the given platform!");
        return nullptr;
    }

    if (mhw->Initialize(params, osInterface) != MOS_STATUS_SUCCESS)
    {
        MHW_ASSERTMESSAGE("MHW interfaces were not successfully allocated!");
        return nullptr;
    }

    return mhw;
}

void MhwInterfaces::Destroy()
{
    Delete_MhwCpInterface(m_cpInterface);
    m_cpInterface = nullptr;
    MOS_Delete(m_miInterface);
    MOS_Delete(m_renderInterface);
    MOS_Delete(m_sfcInterface);
    MOS_Delete(m_stateHeapInterface);
    MOS_Delete(m_veboxInterface);
    MOS_Delete(m_mfxInterface);
    MOS_Delete(m_hcpInterface);
    MOS_Delete(m_hucInterface);
    MOS_Delete(m_vdencInterface);
    MOS_Delete(m_bltInterface);
}

Codechal* CodechalDevice::CreateFactory(
    PMOS_INTERFACE  osInterface,
    PMOS_CONTEXT    osDriverContext,
    void            *standardInfo,
    void            *settings)
{
#define FAIL_CHK_STATUS(stmt)                   \
{                                               \
    if ((stmt) != MOS_STATUS_SUCCESS)           \
    {                                           \
        CODECHAL_PUBLIC_ASSERTMESSAGE("Status check failed, CodecHal creation failed!"); \
        if (osInterface != nullptr && osInterface->bDeallocateOnExit) \
        {                                       \
            osInterface->pfnDestroy(osInterface, false); \
            MOS_FreeMemory(osInterface);        \
        }                                       \
        MOS_Delete(mhwInterfaces);              \
        MOS_Delete(device);                     \
        return nullptr;                         \
    }                                           \
}

#define FAIL_CHK_NULL(ptr)                      \
{                                               \
    if ((ptr) == nullptr)                       \
    {                                           \
        CODECHAL_PUBLIC_ASSERTMESSAGE("nullptr check failed, CodecHal creation failed!"); \
        if (osInterface != nullptr && osInterface->bDeallocateOnExit) \
        {                                       \
            osInterface->pfnDestroy(osInterface, false); \
            MOS_FreeMemory(osInterface);        \
        }                                       \
        MOS_Delete(mhwInterfaces);              \
        MOS_Delete(device);                     \
        return nullptr;                         \
    }                                           \
}

    if (osDriverContext == nullptr ||
        standardInfo == nullptr)
    {
        return nullptr;
    }

    MhwInterfaces *mhwInterfaces = nullptr;
    CodechalDevice *device = nullptr;

    CODECHAL_FUNCTION CodecFunction = ((PCODECHAL_STANDARD_INFO)standardInfo)->CodecFunction;

    // pOsInterface not provided - use default for the current OS
    if (osInterface == nullptr)
    {
        osInterface =
            (PMOS_INTERFACE)MOS_AllocAndZeroMemory(sizeof(MOS_INTERFACE));
        FAIL_CHK_NULL(osInterface);

        MOS_COMPONENT component = CodecHalIsDecode(CodecFunction) ? COMPONENT_Decode : COMPONENT_Encode;
        FAIL_CHK_STATUS(Mos_InitInterface(osInterface, osDriverContext, component));

        osInterface->bDeallocateOnExit = true;
    }
    // pOsInterface provided - use OS interface functions provided by DDI (OS emulation)
    else
    {
        osInterface->bDeallocateOnExit = false;
    }

    MhwInterfaces::CreateParams params;
    MOS_ZeroMemory(&params, sizeof(params));
    params.Flags.m_render = true;
    params.Flags.m_sfc = true;
    params.Flags.m_vdboxAll = true;
    params.Flags.m_vebox = true;
    params.m_heapMode = (uint8_t)2;
    params.m_isDecode = CodecHalIsDecode(CodecFunction);
    mhwInterfaces = MhwInterfaces::CreateFactory(params, osInterface);
    FAIL_CHK_NULL(mhwInterfaces);

    PLATFORM platform = {};
    osInterface->pfnGetPlatform(osInterface, &platform);
    device = CodechalFactory::CreateHal(platform.eProductFamily + MEDIA_EXT_FLAG);
    if(device == nullptr)
    {
        device = CodechalFactory::CreateHal(platform.eProductFamily);
    }
    FAIL_CHK_NULL(device);
    device->Initialize(standardInfo, settings, mhwInterfaces, osInterface);
    FAIL_CHK_NULL(device->m_codechalDevice);

    Codechal *codechalDevice = device->m_codechalDevice;

    MOS_Delete(mhwInterfaces);
    MOS_Delete(device);

    return codechalDevice;
}
#ifdef _MMC_SUPPORTED
void* MmdDevice::CreateFactory(
    PMOS_CONTEXT    osDriverContext)
{
#define MMD_FAILURE()                                       \
{                                                           \
    if (osInterface != nullptr)                             \
    {                                                       \
        osInterface->pfnDestroy(osInterface, false);        \
        MOS_FreeMemory(osInterface);                        \
    }                                                       \
    if (mhwInterfaces != nullptr)                           \
    {                                                       \
        mhwInterfaces->Destroy();                           \
    }                                                       \
    MOS_Delete(mhwInterfaces);                              \
    MOS_Delete(device);                                     \
    return nullptr;                                         \
}
    MHW_FUNCTION_ENTER;

    if (osDriverContext == nullptr)
    {
        MHW_ASSERTMESSAGE("Invalid(null) pOsDriverContext!");
        return nullptr;
    }

    PMOS_INTERFACE osInterface   = nullptr;
    MhwInterfaces *mhwInterfaces = nullptr;
    MmdDevice     *device        = nullptr;

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
    device = MmdFactory::CreateHal(platform.eProductFamily);
    if (device == nullptr)
    {
        MMD_FAILURE();
    }

    mhwInterfaces = device->CreateMhwInterface(osInterface);
    if (mhwInterfaces == nullptr)
    {
        MMD_FAILURE();
    }
    device->Initialize(osInterface, mhwInterfaces);
    if (device->m_mmdDevice == nullptr)
    {
        MMD_FAILURE();
    }

    void *mmdDevice = device->m_mmdDevice;
    MOS_Delete(mhwInterfaces);
    MOS_Delete(device);

    return mmdDevice;
}

MhwInterfaces* MmdDevice::CreateMhwInterface(PMOS_INTERFACE osInterface)
{
    MhwInterfaces::CreateParams params;
    MOS_ZeroMemory(&params, sizeof(params));
    params.Flags.m_render    = true;
    params.m_heapMode        = (uint8_t)2;
    MhwInterfaces *mhw = MhwInterfaces::CreateFactory(params, osInterface);

    return mhw;
}
#endif
CodechalDecodeNV12ToP010* Nv12ToP010Device::CreateFactory(
    PMOS_INTERFACE osInterface)
{
    if (osInterface == nullptr)
    {
        MHW_ASSERTMESSAGE("Invalid(nullptr) osInterface!");
        return nullptr;
    }

    PLATFORM platform = {};
    osInterface->pfnGetPlatform(osInterface, &platform);
    Nv12ToP010Device *device = nullptr;
    CodechalDecodeNV12ToP010 *nv12ToP01Device = nullptr;
    device = Nv12ToP010Factory::CreateHal(platform.eProductFamily);
    if (device != nullptr)
    {
        device->Initialize(osInterface);
        nv12ToP01Device = device->m_nv12ToP010device;
    }

    MOS_Delete(device);

    return nv12ToP01Device;
}

CM_HAL_GENERIC* CMHalDevice::CreateFactory(
        CM_HAL_STATE *pCmState)
{
    CMHalDevice *device = nullptr;
    device = CMHalFactory::CreateHal(pCmState->platform.eProductFamily);
    if (device == nullptr)
    {
        return nullptr;
    }
    device->Initialize(pCmState);
    if (device->m_cmhalDevice == nullptr)
    {
        MHW_ASSERTMESSAGE("CMHal device creation failed!");
        MOS_Delete(device);
        return nullptr;
    }

    CM_HAL_GENERIC *pRet = device->m_cmhalDevice;
    MOS_Delete(device);
    return pRet;
}

void* MosUtilDevice::CreateFactory(
    PRODUCT_FAMILY productFamily)
{
    MosUtilDevice *device = nullptr;

    device = MosUtilFactory::CreateHal(productFamily + MEDIA_EXT_FLAG);
    if (device == nullptr)
    {
        device = MosUtilFactory::CreateHal(productFamily);
    }

    if (device == nullptr)
    {
        return nullptr;
    }

    device->Initialize();
    if (device->m_mosUtilDevice == nullptr)
    {
        return nullptr;
    }

    void* mosUtiInterface = device->m_mosUtilDevice;

    MOS_Delete(device); // avoid mem leak
    return mosUtiInterface;
}

XRenderHal_Platform_Interface* RenderHalDevice::CreateFactory(
        PMOS_INTERFACE osInterface)
{
    RenderHalDevice *device = nullptr;
    PLATFORM platform = {};
    osInterface->pfnGetPlatform(osInterface, &platform);
    device = RenderHalFactory::CreateHal(platform.eProductFamily);
    if (device == nullptr)
    {
        return nullptr;
    }
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

CodechalDecodeHistogram* DecodeHistogramDevice::CreateFactory(
    CodechalHwInterface *hwInterface,
    PMOS_INTERFACE osInterface)
{
    if (hwInterface == nullptr || osInterface == nullptr)
    {
        MHW_ASSERTMESSAGE("Invalid(nullptr) hwInterface or osInterface!");
        return nullptr;
    }

    PLATFORM platform = {};
    osInterface->pfnGetPlatform(osInterface, &platform);
    DecodeHistogramDevice *device = nullptr;
    CodechalDecodeHistogram *decodeHistogramDevice = nullptr;
    device = DecodeHistogramFactory::CreateHal(platform.eProductFamily);
    if (device != nullptr)
    {
        device->Initialize(hwInterface, osInterface);
        decodeHistogramDevice = device->m_decodeHistogramDevice;
    }

    MOS_Delete(device);

    return decodeHistogramDevice;
}
