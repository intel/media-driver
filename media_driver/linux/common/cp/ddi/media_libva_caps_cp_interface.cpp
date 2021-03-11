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
#include "media_libva_util.h"
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

    MediaLibvaCapsCpInterface* pInterface = nullptr;
    CpInterfaces *cp_interface = CpInterfacesFactory::Create(CP_INTERFACE);
    if (cp_interface)
    {
        pInterface = cp_interface->Create_MediaLibvaCapsCpInterface(mediaCtx, mediaCaps);
        MOS_Delete(cp_interface);
    }

    if (nullptr == pInterface)
    {
        CapsStubMessage();
    }

    return nullptr == pInterface ? MOS_New(MediaLibvaCapsCpInterface, mediaCtx, mediaCaps) : pInterface;
}

void Delete_MediaLibvaCapsCpInterface(MediaLibvaCapsCpInterface* pInterface)
{
    CpInterfaces *cp_interface = CpInterfacesFactory::Create(CP_INTERFACE);
    if (pInterface != nullptr && cp_interface != nullptr)
    {
        cp_interface->Delete_MediaLibvaCapsCpInterface(pInterface);
        pInterface = nullptr;
    }
    MOS_Delete(pInterface);
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

VAStatus MediaLibvaCapsCpInterface::LoadCpProfileEntrypoints()
{
    CapsStubMessage();
    return VA_STATUS_SUCCESS;
}

bool MediaLibvaCapsCpInterface::IsCpConfigId(VAConfigID configId)
{
    CapsStubMessage();
    return false;
}

VAStatus MediaLibvaCapsCpInterface::CreateCpConfig(
        int32_t profileTableIdx,
        VAEntrypoint entrypoint,
        VAConfigAttrib *attribList,
        int32_t numAttribs,
        VAConfigID *configId)
{
    CapsStubMessage();
    return VA_STATUS_SUCCESS;
}

MediaLibvaCaps::ProfileEntrypoint* MediaLibvaCapsCpInterface::GetProfileEntrypoint(int32_t profileTableIdx)
{
    DDI_CHK_NULL(m_mediaCaps,   "nullptr m_mediaCaps",  nullptr);

    if (profileTableIdx >= MediaLibvaCaps::m_maxProfileEntries)
    {
        DDI_ASSERTMESSAGE("profileTableIdx %d out of bounds", profileTableIdx);
        return nullptr;
    }

    return &(m_mediaCaps->m_profileEntryTbl[profileTableIdx]);
}

uint16_t MediaLibvaCapsCpInterface::GetProfileEntryCount()
{
    DDI_CHK_NULL(m_mediaCaps,   "nullptr m_mediaCaps",  0);

    return m_mediaCaps->m_profileEntryCount;
}

VAStatus MediaLibvaCapsCpInterface::GetProfileEntrypointFromConfigId(
    VAConfigID configId,
    VAProfile *profile,
    VAEntrypoint *entrypoint,
    int32_t *profileTableIdx)
{
    VAStatus status = VA_STATUS_SUCCESS;

    DDI_CHK_NULL(m_mediaCaps,   "nullptr m_mediaCaps",  VA_STATUS_ERROR_OPERATION_FAILED);

    status = m_mediaCaps->GetProfileEntrypointFromConfigId(configId, profile, entrypoint, profileTableIdx);

    return status;
}

VAStatus MediaLibvaCapsCpInterface::AddProfileEntry(
    VAProfile profile,
    VAEntrypoint entrypoint,
    AttribMap *attributeList,
    int32_t configIdxStart,
    int32_t configNum)
{
    VAStatus status = VA_STATUS_SUCCESS;

    DDI_CHK_NULL(m_mediaCaps,       "nullptr m_mediaCaps",      VA_STATUS_ERROR_OPERATION_FAILED);

    status = m_mediaCaps->AddProfileEntry(profile, entrypoint, attributeList, configIdxStart, configNum);

    return status;
}
