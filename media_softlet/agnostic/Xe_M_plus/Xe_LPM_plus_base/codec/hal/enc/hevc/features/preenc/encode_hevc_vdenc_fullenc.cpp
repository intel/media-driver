/*
* Copyright (c) 2020-2021, Intel Corporation
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
//! \file     encode_hevc_vdenc_fullenc.cpp
//! \brief    Defines the common interface for full-enc features
//!

#include "encode_hevc_basic_feature.h"
#include "encode_hevc_vdenc_fullenc.h"

namespace encode
{
    HevcVdencFullEnc::HevcVdencFullEnc(
        MediaFeatureManager *featureManager,
        EncodeAllocator *    allocator,
        CodechalHwInterfaceNext *hwInterface,
        void *               constSettings) :
        MediaFeature(constSettings, hwInterface ? hwInterface->GetOsInterface() : nullptr)

    {
        ENCODE_FUNC_CALL();
        auto encFeatureManager = (featureManager);
        ENCODE_CHK_NULL_NO_STATUS_RETURN(encFeatureManager);

        ENCODE_CHK_NULL_NO_STATUS_RETURN(hwInterface);
        CODEC_HW_ASSERT(hwInterface->GetOsInterface());
        m_osInterface = hwInterface->GetOsInterface();

        m_allocator = allocator;

        m_basicFeature = dynamic_cast<EncodeBasicFeature *>(encFeatureManager->GetFeature(HevcFeatureIDs::basicFeature));
        ENCODE_CHK_NULL_NO_STATUS_RETURN(m_basicFeature);
        m_preEncFeature = dynamic_cast<HevcVdencPreEnc *>(encFeatureManager->GetFeature(FeatureIDs::preEncFeature));
        ENCODE_CHK_NULL_NO_STATUS_RETURN(m_preEncFeature);
    }

    HevcVdencFullEnc::~HevcVdencFullEnc()
    {
#if USE_CODECHAL_DEBUG_TOOL
        if (pfile0 != nullptr)
        {
            fclose(pfile0);
            pfile0 = nullptr;
        }
        if (pfile1 != nullptr)
        {
            fclose(pfile1);
            pfile1 = nullptr;
        }
#endif
    }

    MOS_STATUS HevcVdencFullEnc::Init(void *settings)
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        ENCODE_FUNC_CALL();

        CodechalSetting* codecSettings = (CodechalSetting *)settings;
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

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HevcVdencFullEnc::UpdatePreEncSize()
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

        EncodeFullencMember2 = m_preEncInfo.EncodePreEncInfo2;
        EncodeFullencMember3 = m_preEncInfo.EncodePreEncInfo3;
        EncodeFullencMember4 = m_preEncInfo.EncodePreEncInfo0;
        EncodeFullencMember5 = m_preEncInfo.EncodePreEncInfo1;

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HevcVdencFullEnc::UpdateTrackedBufferParameters()
    {
        MOS_ALLOC_GFXRES_PARAMS allocParams;
        MOS_ZeroMemory(&allocParams, sizeof(MOS_ALLOC_GFXRES_PARAMS));
        allocParams.Type     = MOS_GFXRES_BUFFER;
        allocParams.TileType = MOS_TILE_LINEAR;
        allocParams.Format   = Format_Buffer;

        if (EncodeFullencMember5 > 0)
        {
            allocParams.dwBytes  = EncodeFullencMember5 * sizeof(EncodeFullencDef1);
            allocParams.pBufName = "pre_Ref0";
            allocParams.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_NOCACHE;
            ENCODE_CHK_STATUS_RETURN(m_basicFeature->m_trackedBuf->RegisterParam(encode::BufferType::preencRef0, allocParams));
            allocParams.pBufName = "pre_Ref1";
            allocParams.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_NOCACHE;
            ENCODE_CHK_STATUS_RETURN(m_basicFeature->m_trackedBuf->RegisterParam(encode::BufferType::preencRef1, allocParams));
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HevcVdencFullEnc::Update(void *params)
    {
        ENCODE_FUNC_CALL();

        ENCODE_CHK_NULL_RETURN(m_basicFeature);

        if (!m_enabled)
            return MOS_STATUS_SUCCESS;

        ENCODE_CHK_STATUS_RETURN(UpdatePreEncSize());

        if (m_encodeMode == MediaEncodeMode::SINGLE_PRE_FULL_ENC)
        {
            m_preEncFeature->EncodePreencBasicFuntion0(EncodeFullencMember0, EncodeFullencMember1);
        }
        else
        {
            if (m_basicFeature->m_resolutionChanged)
            {
                ENCODE_CHK_STATUS_RETURN(UpdateTrackedBufferParameters());
            }
            EncodeFullencMember0 = m_basicFeature->m_trackedBuf->GetBuffer(BufferType::preencRef0, m_basicFeature->m_trackedBuf->GetCurrIndex());
            EncodeFullencMember1 = m_basicFeature->m_trackedBuf->GetBuffer(BufferType::preencRef1, m_basicFeature->m_trackedBuf->GetCurrIndex());
#if USE_CODECHAL_DEBUG_TOOL
            ENCODE_CHK_STATUS_RETURN(EncodeFullencFuntion0());
#endif
        }

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(VDENC_PIPE_MODE_SELECT, HevcVdencFullEnc)
    {
        auto hevcFeature = dynamic_cast<HevcBasicFeature *>(m_basicFeature);
        ENCODE_CHK_NULL_RETURN(hevcFeature);

        if (!m_enabled)
            return MOS_STATUS_SUCCESS;

        params.VdencPipeModeSelectPar2 = 2;
        if (hevcFeature->m_hevcPicParams->CodingType != I_TYPE && !hevcFeature->m_ref.IsLowDelay())
        {
            params.VdencPipeModeSelectPar3 = 3;
        }
        else
        {
            params.VdencPipeModeSelectPar3 = 1;
        }
        params.VdencPipeModeSelectPar5 = EncodeFullencMember3;
        params.VdencPipeModeSelectPar7 = EncodeFullencMember2;
        params.VdencPipeModeSelectPar6 = EncodeFullencMember4;

        if (hevcFeature->m_hevcPicParams->CodingType == I_TYPE)
        {
            params.VdencPipeModeSelectPar2 = 0;
            params.VdencPipeModeSelectPar4 = 0;
            params.VdencPipeModeSelectPar3 = 0;
            params.VdencPipeModeSelectPar5 = 0;
            params.VdencPipeModeSelectPar7 = 0;
            params.VdencPipeModeSelectPar6 = 0;
        }

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(VDENC_PIPE_BUF_ADDR_STATE, HevcVdencFullEnc)
    {
        if (!m_enabled)
            return MOS_STATUS_SUCCESS;

        params.vdencPipeBufAddrStatePar0 = EncodeFullencMember0;
        params.vdencPipeBufAddrStatePar1 = EncodeFullencMember1;

        return MOS_STATUS_SUCCESS;
    }

#if USE_CODECHAL_DEBUG_TOOL
    MOS_STATUS HevcVdencFullEnc::EncodeFullencFuntion0()
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        ENCODE_FUNC_CALL();

        if (!m_enabled)
            return MOS_STATUS_SUCCESS;

        MediaUserSetting::Value outValue;
        outValue = std::string();
        eStatus = ReadUserSetting(
            m_userSettingPtr,
            outValue,
            "Fullenc file0 path",
            MediaUserSetting::Group::Sequence);

        int size = outValue.Size();
        if (eStatus != MOS_STATUS_SUCCESS || size < 2)
        {
            ENCODE_NORMALMESSAGE("read Fullenc file0 path failed.");
            return MOS_STATUS_SUCCESS;
        }

        std::string path_file = outValue.Get<std::string>();
        if (path_file[size - 2] != MOS_DIRECTORY_DELIMITER)
        {
            path_file.push_back('\0');
        }
        std::string filePath0 = path_file;

        outValue = std::string();
        eStatus = ReadUserSetting(
            m_userSettingPtr,
            outValue,
            "Fullenc file1 path",
            MediaUserSetting::Group::Sequence);

        size = outValue.Size();

        if (eStatus != MOS_STATUS_SUCCESS || size < 2)
        {
            ENCODE_NORMALMESSAGE("read Fullenc file1 path failed.");
            return MOS_STATUS_SUCCESS;
        }

        path_file = outValue.Get<std::string>();
        if (path_file[size - 2] != MOS_DIRECTORY_DELIMITER)
        {
            path_file.push_back('\0');
        }
        std::string filePath1 = path_file;

        auto hevcFeature = dynamic_cast<HevcBasicFeature *>(m_basicFeature);
        ENCODE_CHK_NULL_RETURN(hevcFeature);
        if (hevcFeature->m_hevcPicParams->CodingType > I_TYPE)
        {
            if (pfile0 == nullptr)
            {
                ENCODE_CHK_STATUS_RETURN(MosUtilities::MosSecureFileOpen(&pfile0, filePath0.c_str(), "rb"));
            }
            void *data0 = m_allocator->LockResourceForWrite(EncodeFullencMember0);
            if (pfile0 != nullptr)
            {
                if (fread(data0, sizeof(EncodePreencDef1), EncodeFullencMember5, pfile0) != EncodeFullencMember5)
                {
                    ENCODE_ASSERTMESSAGE("Error of reading file 0");
                    ENCODE_CHK_STATUS_RETURN(m_allocator->UnLock(EncodeFullencMember0));
                    return MOS_STATUS_FILE_READ_FAILED;
                }
            }
            ENCODE_CHK_STATUS_RETURN(m_allocator->UnLock(EncodeFullencMember0));
        }
        if (hevcFeature->m_hevcPicParams->CodingType != I_TYPE && !hevcFeature->m_ref.IsLowDelay())
        {
            if (pfile1 == nullptr)
            {
                ENCODE_CHK_STATUS_RETURN(MosUtilities::MosSecureFileOpen(&pfile1, filePath1.c_str(), "rb"));
            }
            void *data1 = m_allocator->LockResourceForWrite(EncodeFullencMember1);
            if (pfile1 != nullptr)
            {
                if (fread(data1, sizeof(EncodePreencDef1), EncodeFullencMember5, pfile1) != EncodeFullencMember5)
                {
                    ENCODE_ASSERTMESSAGE("Error of reading file 1");
                    ENCODE_CHK_STATUS_RETURN(m_allocator->UnLock(EncodeFullencMember0));
                    return MOS_STATUS_FILE_READ_FAILED;
                }
            }
            ENCODE_CHK_STATUS_RETURN(m_allocator->UnLock(EncodeFullencMember1));
        }

        return MOS_STATUS_SUCCESS;
    }
#endif
}  // namespace encode
