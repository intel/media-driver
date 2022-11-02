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

#include "media_interfaces_mcpy_next.h"
#include "media_interfaces_mmd_next.h"
#include "media_interfaces_mhw_next.h"
#include "media_interfaces_codechal_next.h"

template class MediaFactory<uint32_t, CodechalDeviceNext>;
template class MediaFactory<uint32_t, MhwInterfacesNext>;
template class MediaFactory<uint32_t, McpyDeviceNext>;
template class MediaFactory<uint32_t, MmdDeviceNext>;

typedef MediaFactory<uint32_t, CodechalDeviceNext> CodechalFactoryNext;
typedef MediaFactory<uint32_t, MhwInterfacesNext> MhwFactoryNext;
typedef MediaFactory<uint32_t, McpyDeviceNext> McpyFactoryNext;
typedef MediaFactory<uint32_t, MmdDeviceNext> MmdFactoryNext;

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

    Delete_MhwCpInterface(m_cpInterface);
    m_cpInterface = nullptr;
    MOS_Delete(m_stateHeapInterface);
}

void* McpyDeviceNext::CreateFactory(
    PMOS_CONTEXT    osDriverContext)
{
#define MCPY_FAILURE()                                       \
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

    PMOS_INTERFACE     osInterface   = nullptr;
    MhwInterfacesNext *mhwInterfaces = nullptr;
    McpyDeviceNext    *device        = nullptr;

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
        MCPY_FAILURE();
    }

    PLATFORM platform = {};
    osInterface->pfnGetPlatform(osInterface, &platform);
    device = McpyFactoryNext::Create(platform.eProductFamily);
    if (device == nullptr)
    {
        MCPY_FAILURE();
    }

    mhwInterfaces = device->CreateMhwInterface(osInterface);
    if (mhwInterfaces == nullptr)
    {
        MCPY_FAILURE();
    }
    MOS_STATUS status = device->Initialize(osInterface, mhwInterfaces);
    if (status == MOS_STATUS_NO_SPACE)
    {
        MCPY_FAILURE();
    }
    else if (status == MOS_STATUS_UNINITIALIZED)
    {
        MOS_Delete(device);
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
    params.Flags.m_render    = true;

    params.m_heapMode        = (uint8_t)2;
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
