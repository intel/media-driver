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

#include "mhw_cp_interface.h"

#ifdef IGFX_MHW_INTERFACES_NEXT_SUPPORT
#include "media_interfaces_mhw_next.h"
#include "media_interfaces_hwinfo_device.h"

template class MediaFactory<uint32_t, MhwInterfacesNext>;

typedef MediaFactory<uint32_t, MhwInterfacesNext> MhwFactoryNext;
#endif

#ifdef IGFX_MHW_INTERFACES_NEXT_SUPPORT
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
#endif
