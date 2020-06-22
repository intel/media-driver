/*
* Copyright (c) 2009-2020, Intel Corporation
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
//! \file     media_libva_caps_cp_interface.cpp
//! \brief    The class implementation of MediaLibvaCapsCpInterface
//!

#include "media_libva_caps_cp_interface.h"
#include "cp_interfaces.h"
#include <typeinfo>

static void CapsStubMessage()
{
    MOS_NORMALMESSAGE(
        MOS_COMPONENT_CP,
        MOS_CP_SUBCOMP_CAPS,
        "This function is stubbed as CP is not enabled.");
}

MediaLibvaCapsCpInterface::MediaLibvaCapsCpInterface(DDI_MEDIA_CONTEXT *mediaCtx, MediaLibvaCaps *mediaCaps)
{
    m_mediaCtx = mediaCtx;
    m_mediaCaps = mediaCaps;
}

MediaLibvaCapsCpInterface* Create_MediaLibvaCapsCpInterface(DDI_MEDIA_CONTEXT *mediaCtx, MediaLibvaCaps *mediaCaps)
{
    if (nullptr == mediaCtx || nullptr == mediaCaps)
    {
        MOS_NORMALMESSAGE(MOS_COMPONENT_CP, MOS_CP_SUBCOMP_CAPS, "NULL pointer parameters");
        return nullptr;
    }

    CpInterfaces *cp_interface = CpInterfacesFactory::Create(CP_INTERFACE);
    if (nullptr == cp_interface)
    {
        MOS_NORMALMESSAGE(MOS_COMPONENT_CP, MOS_CP_SUBCOMP_CAPS, "NULL pointer prot");
        return nullptr;
    }

    MediaLibvaCapsCpInterface* pMediaLibvaCapsCpInterface = nullptr;
    pMediaLibvaCapsCpInterface = cp_interface->Create_MediaLibvaCapsCpInterface(mediaCtx, mediaCaps);
    MOS_Delete(cp_interface);

    if (nullptr == pMediaLibvaCapsCpInterface)
    {
        CapsStubMessage();
    }

    return nullptr == pMediaLibvaCapsCpInterface ? MOS_New(MediaLibvaCapsCpInterface, mediaCtx, mediaCaps) : pMediaLibvaCapsCpInterface;
}

void Delete_MediaLibvaCapsCpInterface(MediaLibvaCapsCpInterface* pMediaLibvaCapsCpInterface)
{
    CpInterfaces *cp_interface = CpInterfacesFactory::Create(CP_INTERFACE);
    if (pMediaLibvaCapsCpInterface != nullptr && cp_interface != nullptr)
    {
        cp_interface->Delete_MediaLibvaCapsCpInterface(pMediaLibvaCapsCpInterface);
    }
    MOS_Delete(cp_interface);
}

bool MediaLibvaCapsCpInterface::IsDecEncryptionSupported(DDI_MEDIA_CONTEXT* mediaCtx)
{
    CapsStubMessage();
    return false;
}

int32_t MediaLibvaCapsCpInterface::GetEncryptionTypes(
    VAProfile profile,
    uint32_t *encryptionType,
    uint32_t arraySize)
{
    CapsStubMessage();
    return -1;
}
