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
//! \file     encode_avc_vdenc_fullenc.cpp
//! \brief    Defines the common interface for avc vdenc fullenc encode features
//!

#include "encode_avc_vdenc_fullenc.h"
#include <algorithm>

namespace encode
{

AvcVdencFullEnc::AvcVdencFullEnc(
    MediaFeatureManager *featureManager,
    EncodeAllocator     *allocator,
    CodechalHwInterfaceNext *hwInterface,
    void                *constSettings) :
    MediaFeature(constSettings, hwInterface ? hwInterface->GetOsInterface() : nullptr),
    m_allocator(allocator)
{
    ENCODE_FUNC_CALL();

    m_featureManager = featureManager;
    ENCODE_CHK_NULL_NO_STATUS_RETURN(hwInterface);
    m_osInterface = hwInterface->GetOsInterface();
    ENCODE_ASSERT(m_osInterface);

    m_basicFeature = dynamic_cast<AvcBasicFeature *>(m_featureManager->GetFeature(FeatureIDs::basicFeature));
    ENCODE_CHK_NULL_NO_STATUS_RETURN(m_basicFeature);
    m_preEncFeature = dynamic_cast<AvcVdencPreEnc *>(m_featureManager->GetFeature(FeatureIDs::preEncFeature));
    ENCODE_CHK_NULL_NO_STATUS_RETURN(m_preEncFeature);
}

AvcVdencFullEnc::~AvcVdencFullEnc()
{
    ENCODE_FUNC_CALL();
    if (m_pfile0 != nullptr)
    {
        fclose(m_pfile0);
        m_pfile0 = nullptr;
    }
    if (m_pfile1 != nullptr)
    {
        fclose(m_pfile1);
        m_pfile1 = nullptr;
    }
}

MOS_STATUS AvcVdencFullEnc::Init(void *settings)
{
    ENCODE_FUNC_CALL();

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MediaUserSetting::Value outValue;
    ReadUserSetting(
        m_userSettingPtr,
        outValue,
        "Set Media Encode Mode",
        MediaUserSetting::Group::Sequence,
        m_osInterface->pOsContext);
    m_encodeMode = outValue.Get<uint32_t>();

    if (((m_encodeMode & FULL_ENC_PASS) == FULL_ENC_PASS))
    {
        m_enabled = true;
    }

    if (m_encodeMode == MediaEncodeMode::MANUAL_FULL_ENC)
    {
        outValue = std::string();
        eStatus = ReadUserSetting(
            m_userSettingPtr,
            outValue,
            "Fullenc file0 path",
            MediaUserSetting::Group::Sequence);

        int size = outValue.Size();
        if (eStatus != MOS_STATUS_SUCCESS)
            size = 0;

        if (size == MOS_MAX_PATH_LENGTH + 1)
        {
            size = 0;
        }

        ENCODE_CHK_COND_RETURN(size == 0, "PATH LENGTH OF FILE IS TOO LONG");

        std::string path_file = outValue.Get<std::string>();
        if (path_file[size - 2] != MOS_DIRECTORY_DELIMITER)
        {
            path_file.push_back('\0');
        }
        ecodeAvcFullencMember9 = path_file;
        ENCODE_CHK_STATUS_RETURN(MosUtilities::MosSecureFileOpen(&m_pfile0, ecodeAvcFullencMember9.c_str(), "rb"));

        outValue = std::string();
        eStatus = ReadUserSetting(
            m_userSettingPtr,
            outValue,
            "Fullenc file1 path",
            MediaUserSetting::Group::Sequence);

        size = outValue.Size();

        if (eStatus != MOS_STATUS_SUCCESS)
            size = 0;

        if (size == MOS_MAX_PATH_LENGTH + 1)
        {
            size = 0;
        }

        ENCODE_CHK_COND_RETURN(size == 0, "PATH LENGTH OF FILE IS TOO LONG");

        path_file = outValue.Get<std::string>();
        if (path_file[size - 2] != MOS_DIRECTORY_DELIMITER)
        {
            path_file.push_back('\0');
        }
        ecodeAvcFullencMember10 = path_file;
        ENCODE_CHK_STATUS_RETURN(MosUtilities::MosSecureFileOpen(&m_pfile1, const_cast<const char *>(ecodeAvcFullencMember10.c_str()), "rb"));
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS AvcVdencFullEnc::UpdatePreEncSize()
{
    ENCODE_FUNC_CALL();

    PreEncInfo m_preEncInfo = {};

    // if GetPreEncInfo succeeds, use this value, otherwise recalculate
    ENCODE_CHK_NULL_RETURN(m_preEncFeature);
    MOS_STATUS status = m_preEncFeature->GetPreEncInfo(m_preEncInfo);
    if (status != MOS_STATUS_SUCCESS)
    {
        ENCODE_CHK_NULL_RETURN(m_basicFeature);
        uint32_t m_frameWidth  = m_basicFeature->m_frameWidth;
        uint32_t m_frameHeight = m_basicFeature->m_frameHeight;
        ENCODE_CHK_STATUS_RETURN(m_preEncFeature->CalculatePreEncInfo(m_frameWidth, m_frameHeight, m_preEncInfo));
    }

    ecodeAvcFullencMember3   = m_preEncInfo.EncodePreEncInfo2;
    ecodeAvcFullencMember11  = m_preEncInfo.EncodePreEncInfo3;
    ecodeAvcFullencMember4   = m_preEncInfo.EncodePreEncInfo0;
    ecodeAvcFullencMember5   = m_preEncInfo.EncodePreEncInfo1;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS AvcVdencFullEnc::UpdateTrackedBufferParameters()
{
    MOS_ALLOC_GFXRES_PARAMS allocParams;
    MOS_ZeroMemory(&allocParams, sizeof(MOS_ALLOC_GFXRES_PARAMS));
    allocParams.Type     = MOS_GFXRES_BUFFER;
    allocParams.TileType = MOS_TILE_LINEAR;
    allocParams.Format   = Format_Buffer;

    if (ecodeAvcFullencMember5 > 0)
    {
        allocParams.dwBytes  = ecodeAvcFullencMember5 * sizeof(EncodePreencDef1);
        allocParams.pBufName = "preencRef0";
        allocParams.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_CACHE;
        ENCODE_CHK_STATUS_RETURN(m_basicFeature->m_trackedBuf->RegisterParam(encode::BufferType::preencRef0, allocParams));
        allocParams.pBufName = "preencRef1";
        allocParams.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_CACHE;
        ENCODE_CHK_STATUS_RETURN(m_basicFeature->m_trackedBuf->RegisterParam(encode::BufferType::preencRef1, allocParams));
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS AvcVdencFullEnc::Update(void *params)
{
    ENCODE_FUNC_CALL();

    ENCODE_CHK_NULL_RETURN(m_basicFeature);

    if (!m_enabled)
        return MOS_STATUS_SUCCESS;

    ENCODE_CHK_STATUS_RETURN(UpdatePreEncSize());

    if (m_encodeMode == MediaEncodeMode::SINGLE_PRE_FULL_ENC)
    {
        m_preEncFeature->EncodePreencBasicFuntion0(ecodeAvcFullencMember0, ecodeAvcFullencMember1);
    }
    else
    {
        if (m_basicFeature->m_resolutionChanged)
        {
            ENCODE_CHK_STATUS_RETURN(UpdateTrackedBufferParameters());
        }
        ecodeAvcFullencMember0 = m_basicFeature->m_trackedBuf->GetBuffer(BufferType::preencRef0, m_basicFeature->m_trackedBuf->GetCurrIndex());
        ecodeAvcFullencMember1 = m_basicFeature->m_trackedBuf->GetBuffer(BufferType::preencRef1, m_basicFeature->m_trackedBuf->GetCurrIndex());
#if USE_CODECHAL_DEBUG_TOOL
        ENCODE_CHK_STATUS_RETURN(ecodeAvcFullencFuntion0());
#endif
    }

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(VDENC_PIPE_MODE_SELECT, AvcVdencFullEnc)
{
    if (m_enabled)
    {
        params.VdencPipeModeSelectPar2 = 2;
        params.VdencPipeModeSelectPar6 = ecodeAvcFullencMember4;
        if (m_basicFeature->m_pictureCodingType != B_TYPE)
        {
            params.VdencPipeModeSelectPar3 = 1;
        }
        else
        {
            params.VdencPipeModeSelectPar3 = 3;
        }
        params.VdencPipeModeSelectPar4 = 0;
        params.VdencPipeModeSelectPar5 = ecodeAvcFullencMember11;
        params.VdencPipeModeSelectPar7 = ecodeAvcFullencMember3;

        if (m_basicFeature->m_pictureCodingType == I_TYPE)
        {
            params.VdencPipeModeSelectPar2 = 0;
            params.VdencPipeModeSelectPar4 = 0;
            params.VdencPipeModeSelectPar3 = 0;
            params.VdencPipeModeSelectPar5 = 0;
            params.VdencPipeModeSelectPar7 = 0;
            params.VdencPipeModeSelectPar6 = 0;
        }
    }

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(VDENC_PIPE_BUF_ADDR_STATE, AvcVdencFullEnc)
{
    if (m_enabled)
    {
        params.vdencPipeBufAddrStatePar0 = ecodeAvcFullencMember0;
        params.vdencPipeBufAddrStatePar1 = ecodeAvcFullencMember1;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS AvcVdencFullEnc::ecodeAvcFullencFuntion0()
{
    ENCODE_FUNC_CALL();

    if (m_encodeMode == MediaEncodeMode::MANUAL_FULL_ENC)
    {
        void *data = m_allocator->LockResourceForWrite(ecodeAvcFullencMember0);
        ENCODE_CHK_NULL_RETURN(data);

        if (m_basicFeature->m_pictureCodingType != I_TYPE)
        {
            if (fread(data, sizeof(EncodePreencDef1), ecodeAvcFullencMember5, m_pfile0) != ecodeAvcFullencMember5)
            {
                ENCODE_ASSERTMESSAGE("Error of reading file 0");
                ENCODE_CHK_STATUS_RETURN(m_allocator->UnLock(ecodeAvcFullencMember0));
                return MOS_STATUS_FILE_READ_FAILED;
            }
            ENCODE_CHK_STATUS_RETURN(m_allocator->UnLock(ecodeAvcFullencMember0));
        }

        if (m_basicFeature->m_pictureCodingType == B_TYPE)
        {
            data = m_allocator->LockResourceForWrite(ecodeAvcFullencMember1);
            ENCODE_CHK_NULL_RETURN(data);
            if (fread(data, sizeof(EncodePreencDef1), ecodeAvcFullencMember5, m_pfile1) != ecodeAvcFullencMember5)
            {
                ENCODE_ASSERTMESSAGE("Error of reading file 1");
                ENCODE_CHK_STATUS_RETURN(m_allocator->UnLock(ecodeAvcFullencMember1));
                return MOS_STATUS_FILE_READ_FAILED;
            }
            ENCODE_CHK_STATUS_RETURN(m_allocator->UnLock(ecodeAvcFullencMember1));
        }
    }
    return MOS_STATUS_SUCCESS;
}
}  // namespace encode
