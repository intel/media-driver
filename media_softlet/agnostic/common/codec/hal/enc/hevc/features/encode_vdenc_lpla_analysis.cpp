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
//! \file     encode_vdenc_lpla_analysis.cpp
//! \brief    Implementation for encode lowpower lookahead(Lookahead Analysis Pass) feature
//!

#include "encode_vdenc_lpla_analysis.h"
#include "encode_hevc_vdenc_feature_manager.h"

namespace encode
{
    VdencLplaAnalysis::VdencLplaAnalysis(
        MediaFeatureManager *featureManager,
        EncodeAllocator     *allocator,
        CodechalHwInterfaceNext *hwInterface,
        void                *constSettings) :
        MediaFeature(constSettings, hwInterface ? hwInterface->GetOsInterface() : nullptr),
        m_hwInterface(hwInterface),
        m_allocator(allocator)
    {
        auto encFeatureManager = dynamic_cast<EncodeHevcVdencFeatureManager *>(featureManager);
        ENCODE_CHK_NULL_NO_STATUS_RETURN(encFeatureManager);

        m_basicFeature = dynamic_cast<EncodeBasicFeature *>(encFeatureManager->GetFeature(FeatureIDs::basicFeature));
        ENCODE_CHK_NULL_NO_STATUS_RETURN(m_basicFeature);
        ENCODE_CHK_NULL_NO_STATUS_RETURN(hwInterface);
        m_osInterface = hwInterface->GetOsInterface();

#if (_DEBUG || _RELEASE_INTERNAL)
        MediaUserSetting::Value outValue;
        auto statusKey = ReadUserSettingForDebug(m_userSettingPtr,
            outValue,
            "lpla ds data address",
            MediaUserSetting::Group::Sequence);
        const char *path_buffer = outValue.ConstString().c_str();

        if (statusKey == MOS_STATUS_SUCCESS && strcmp(path_buffer, "") != 0)
            m_useDSData = true;

        if (m_useDSData)
        {
            std::ifstream fp(path_buffer);
            if (!fp.is_open())
            {
                m_useDSData = false;
                ENCODE_ASSERTMESSAGE("lpla ds data load failed!");
                return;
            }
                
            std::string   line = "";
            getline(fp, line);
            for (int i = 0; i < 600; i++)
            {
                getline(fp, line);
                std::string        number = "";
                std::istringstream readstr(line);
                getline(readstr, number, ',');
                getline(readstr, number, ',');
                getline(readstr, number, ',');
                m_statsBuffer[i][2] = atoi(number.c_str());
                m_statsBuffer[i][2] *= 4;
                getline(readstr, number, ',');
                m_statsBuffer[i][0] = atoi(number.c_str());
                getline(readstr, number, ',');
                m_statsBuffer[i][1] = atoi(number.c_str());
                m_statsBuffer[i][1] *= 8;
            }
        }
#endif
    }

    VdencLplaAnalysis::~VdencLplaAnalysis()
    {
        if (m_lplaHelper)
        {
            MOS_Delete(m_lplaHelper);
            m_lplaHelper = nullptr;
        }
    }

    MOS_STATUS VdencLplaAnalysis::Init(void *setting)
    {
        ENCODE_FUNC_CALL();
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS; 

        m_hcpItf = m_hwInterface->GetHcpInterfaceNext();
        ENCODE_CHK_NULL_RETURN(m_hcpItf);

        m_miItf = m_hwInterface->GetMiInterfaceNext();
        ENCODE_CHK_NULL_RETURN(m_miItf);

        m_lplaHelper = MOS_New(EncodeLPLA);
        ENCODE_CHK_NULL_RETURN(m_lplaHelper);

        ENCODE_CHK_STATUS_RETURN(AllocateResources());

        return eStatus;
    }

    MOS_STATUS VdencLplaAnalysis::SetSequenceStructs()
    {
        ENCODE_FUNC_CALL();
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        if (m_hevcSeqParams->MaxAdaptiveGopPicSize < m_hevcSeqParams->MinAdaptiveGopPicSize)
        {
            m_hevcSeqParams->MaxAdaptiveGopPicSize = m_hevcSeqParams->MinAdaptiveGopPicSize;
        }
        else if ((m_hevcSeqParams->MaxAdaptiveGopPicSize > 0) && (m_hevcSeqParams->MinAdaptiveGopPicSize == 0))
        {
            m_hevcSeqParams->MinAdaptiveGopPicSize = (m_hevcSeqParams->MaxAdaptiveGopPicSize + 1) >> 1;
        }

        m_lookaheadAdaptiveI = (m_hevcSeqParams->MaxAdaptiveGopPicSize != m_hevcSeqParams->MinAdaptiveGopPicSize);
        if (!m_lookaheadAdaptiveI && (m_hevcSeqParams->MaxAdaptiveGopPicSize == 0))
        {
            if (m_hevcSeqParams->GopPicSize > 0)
            {
                m_hevcSeqParams->MaxAdaptiveGopPicSize = m_hevcSeqParams->GopPicSize;
                m_hevcSeqParams->MinAdaptiveGopPicSize = m_hevcSeqParams->GopPicSize;
            }
            else
            {
                ENCODE_ASSERTMESSAGE("Invalid GopPicSize in LPLA!");
                return MOS_STATUS_INVALID_PARAMETER;
            }
        }

        ENCODE_CHK_STATUS_RETURN(m_lplaHelper->CheckFrameRate(m_hevcSeqParams->FrameRate.Numerator,
            m_hevcSeqParams->FrameRate.Denominator,
            m_hevcSeqParams->TargetBitRate,
            m_averageFrameSize));

        ENCODE_CHK_STATUS_RETURN(m_lplaHelper->CheckVBVBuffer(m_hevcSeqParams->VBVBufferSizeInBit,
            m_hevcSeqParams->InitVBVBufferFullnessInBit));

        if (m_targetBufferFulness == 0 && m_prevTargetFrameSize == 0)
        {
            m_targetBufferFulness            = m_hevcSeqParams->VBVBufferSizeInBit - m_hevcSeqParams->InitVBVBufferFullnessInBit;
            uint32_t initVbvFullnessInFrames = MOS_MIN(m_hevcSeqParams->InitVBVBufferFullnessInBit, m_hevcSeqParams->VBVBufferSizeInBit) / m_averageFrameSize;
            uint32_t vbvBufferSizeInFrames   = m_hevcSeqParams->VBVBufferSizeInBit / m_averageFrameSize;
            uint32_t encBufferFullness       = (vbvBufferSizeInFrames - initVbvFullnessInFrames) * m_averageFrameSize;
            m_bufferFulnessError             = (int32_t)((int64_t)m_targetBufferFulness - (int64_t)encBufferFullness);
        }

        return eStatus;
    }

    MOS_STATUS VdencLplaAnalysis::EnableStreamIn(bool is1stPass, bool isLastPass, bool &streaminEnabled)
    {
        ENCODE_FUNC_CALL();
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        if (!m_enabled)
        {
            return eStatus;
        }

        if (m_hevcSeqParams->MaxAdaptiveGopPicSize > 0)
        {
            bool forceIntra = m_intraInterval >= m_hevcSeqParams->MaxAdaptiveGopPicSize;
            if (((!is1stPass && isLastPass) || forceIntra) && (m_hevcPicParams->CodingType != I_TYPE))
            {
                streaminEnabled = true;
            }

            if (!m_lookaheadAdaptiveI && isLastPass)
            {
                m_intraInterval = forceIntra ? 1 : m_intraInterval + 1;
            }
        }
        m_streamInEnabled = m_enabled && streaminEnabled;

        return eStatus;
    }

    MOS_STATUS VdencLplaAnalysis::SetConditionalPass(bool blastPass, bool &condPass)
    {
        ENCODE_FUNC_CALL();
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        if (!m_enabled)
        {
            return eStatus;
        }

        if (m_hevcPicParams->CodingType != I_TYPE && m_lookaheadAdaptiveI)
        {
            if (!blastPass)
            {
                condPass = false;
            }
        }

        return eStatus;
    }

    MOS_STATUS VdencLplaAnalysis::SetupForceIntraStreamIn()
    {
        ENCODE_FUNC_CALL();
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        if (!m_enabled || m_forceIntraSteamInSetupDone)
        {
            return eStatus;
        }

        MOS_LOCK_PARAMS lockFlags;
        MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
        lockFlags.WriteOnly = true;

        uint8_t *data = (uint8_t *)m_osInterface->pfnLockResource(m_osInterface, m_forceIntraStreamInBuf, &lockFlags);
        ENCODE_CHK_NULL_RETURN(data);

        mhw::vdbox::vdenc::VDENC_STREAMIN_STATE_PAR streaminDataParams;
        uint32_t streamInWidth  = (MOS_ALIGN_CEIL(m_basicFeature->m_frameWidth, 64) / 32);
        uint32_t streamInHeight = (MOS_ALIGN_CEIL(m_basicFeature->m_frameHeight, 64) / 32);

        // lookahead pass should lower QP by 2 to encode force intra frame.
        MOS_ZeroMemory(&streaminDataParams, sizeof(streaminDataParams));
        streaminDataParams.setQpRoiCtrl = true;
        streaminDataParams.forceQp[0] = m_hevcPicParams->QpY - 2;
        streaminDataParams.forceQp[1] = m_hevcPicParams->QpY - 2;
        streaminDataParams.forceQp[2] = m_hevcPicParams->QpY - 2;
        streaminDataParams.forceQp[3] = m_hevcPicParams->QpY - 2;
        SetStreaminDataPerRegion(streamInWidth, 0, streamInHeight, 0, streamInWidth, &streaminDataParams, data);

        MOS_ZeroMemory(&streaminDataParams, sizeof(streaminDataParams));
        streaminDataParams.puTypeCtrl = 1;  //force intra
        streaminDataParams.maxTuSize = 3;
        streaminDataParams.maxCuSize = 1;
        streaminDataParams.numMergeCandidateCu64x64 = 2;
        streaminDataParams.numMergeCandidateCu32x32 = 2;
        streaminDataParams.numMergeCandidateCu16x16 = 2;
        streaminDataParams.numMergeCandidateCu8x8 = 0;
        streaminDataParams.numImePredictors = 4;

        uint32_t streamInNumCUs = streamInWidth * streamInHeight;
        for (uint32_t i = 0; i < streamInNumCUs; i++)
        {
            SetStreaminDataPerLcu(&streaminDataParams, data + (i * 64));
        }

        m_osInterface->pfnUnlockResource(m_osInterface, m_forceIntraStreamInBuf);

        m_forceIntraSteamInSetupDone = true;

        return eStatus;
    }

    MOS_STATUS VdencLplaAnalysis::SetStreaminDataPerRegion(
        uint32_t streamInWidth, uint32_t top, uint32_t bottom, uint32_t left, uint32_t right,
        mhw::vdbox::vdenc::VDENC_STREAMIN_STATE_PAR *streaminParams, void *streaminData)
    {
        ENCODE_FUNC_CALL();

        uint8_t *data = (uint8_t *)streaminData;

        for (auto y = top; y < bottom; y++)
        {
            for (auto x = left; x < right; x++)
            {
                //Calculate X Y for the zig zag scan
                uint32_t offset = 0, xyOffset = 0;
                StreaminZigZagToLinearMap(streamInWidth, x, y, &offset, &xyOffset);

                SetStreaminDataPerLcu(streaminParams, data + (offset + xyOffset) * 64);
            }
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS VdencLplaAnalysis::StreaminZigZagToLinearMap(
        uint32_t  streamInWidth, uint32_t  x, uint32_t  y,
        uint32_t *offset, uint32_t *xyOffset)
    {
        ENCODE_FUNC_CALL();

        *offset = streamInWidth * y;
        uint32_t yOffset = 0;
        uint32_t xOffset = 2 * x;

        //Calculate X Y Offset for the zig zag scan with in each 64x64 LCU
        //dwOffset gives the 64 LCU row
        if (y % 2)
        {
            *offset = streamInWidth * (y - 1);
            yOffset = 2;
        }

        if (x % 2)
        {
            xOffset = (2 * x) - 1;
        }

        *xyOffset = xOffset + yOffset;

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS VdencLplaAnalysis::SetStreaminDataPerLcu(
        mhw::vdbox::vdenc::VDENC_STREAMIN_STATE_PAR *streaminParams,
        void * streaminData)
    {
        ENCODE_FUNC_CALL();

        VdencStreamInState *data = (VdencStreamInState *)streaminData;
        if (streaminParams->setQpRoiCtrl)
        {
            if (0)  //(m_vdencNativeROIEnabled || m_brcAdaptiveRegionBoostEnable)
            {
                data->DW0.RoiCtrl = streaminParams->roiCtrl;
            }
            else
            {
                data->DW7.QpEnable = 0xf;
                data->DW14.ForceQp_0 = streaminParams->forceQp[0];
                data->DW14.ForceQp_1 = streaminParams->forceQp[1];
                data->DW14.ForceQp_2 = streaminParams->forceQp[2];
                data->DW14.ForceQp_3 = streaminParams->forceQp[3];
            }
        }
        else
        {
            data->DW0.MaxTuSize = streaminParams->maxTuSize;
            data->DW0.MaxCuSize = streaminParams->maxCuSize;
            data->DW0.NumImePredictors = streaminParams->numImePredictors;
            data->DW0.PuTypeCtrl = streaminParams->puTypeCtrl;
            data->DW6.NumMergeCandidateCu64x64 = streaminParams->numMergeCandidateCu64x64;
            data->DW6.NumMergeCandidateCu32x32 = streaminParams->numMergeCandidateCu32x32;
            data->DW6.NumMergeCandidateCu16x16 = streaminParams->numMergeCandidateCu16x16;
            data->DW6.NumMergeCandidateCu8x8 = streaminParams->numMergeCandidateCu8x8;
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS VdencLplaAnalysis::AllocateResources()
    {
        ENCODE_FUNC_CALL();
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        MOS_ALLOC_GFXRES_PARAMS allocParamsForBufferLinear;
        MOS_ZeroMemory(&allocParamsForBufferLinear, sizeof(MOS_ALLOC_GFXRES_PARAMS));
        allocParamsForBufferLinear.Type     = MOS_GFXRES_BUFFER;
        allocParamsForBufferLinear.TileType = MOS_TILE_LINEAR;
        allocParamsForBufferLinear.Format   = Format_Buffer;
        allocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_NOCACHE;

        // Buffer to store VDEnc frame statistics for lookahead BRC
        m_brcLooaheadStatsBufferSize        = m_numLaDataEntry * sizeof(VdencHevcLaStats);
        allocParamsForBufferLinear.dwBytes  = MOS_ALIGN_CEIL(m_brcLooaheadStatsBufferSize, CODECHAL_PAGE_SIZE);
        allocParamsForBufferLinear.pBufName = "VDENC Lookahead Statistics Buffer";
        m_vdencLaStatsBuffer                = m_allocator->AllocateResource(allocParamsForBufferLinear, true);

        VdencHevcLaStats *lookaheadInfo = (VdencHevcLaStats *)m_allocator->LockResourceForWrite(m_vdencLaStatsBuffer);
        ENCODE_CHK_NULL_RETURN(lookaheadInfo);
        MOS_ZeroMemory(lookaheadInfo, allocParamsForBufferLinear.dwBytes);
        m_allocator->UnLock(m_vdencLaStatsBuffer);

        // Buffer to store lookahead output
        m_brcLooaheadDataBufferSize            = m_numLaDataEntry * sizeof(VdencHevcLaData);
        allocParamsForBufferLinear.dwBytes     = MOS_ALIGN_CEIL(m_brcLooaheadDataBufferSize, CODECHAL_PAGE_SIZE);
        allocParamsForBufferLinear.pBufName    = "VDENC Lookahead Data Buffer";
        m_vdencLaDataBuffer                    = m_allocator->AllocateResource(allocParamsForBufferLinear, true);
        VdencHevcLaData *lookaheadData         = (VdencHevcLaData *)m_allocator->LockResourceForWrite(m_vdencLaDataBuffer);
        ENCODE_CHK_NULL_RETURN(lookaheadData);
        MOS_ZeroMemory(lookaheadData, allocParamsForBufferLinear.dwBytes);
        m_allocator->UnLock(m_vdencLaDataBuffer);

        // Lookahead Init DMEM
        m_vdencLaInitDmemBufferSize         = sizeof(VdencHevcHucLaDmem);
        allocParamsForBufferLinear.dwBytes  = MOS_ALIGN_CEIL(m_vdencLaInitDmemBufferSize, CODECHAL_CACHELINE_SIZE);
        allocParamsForBufferLinear.pBufName = "VDENC Lookahead Init DmemBuffer";
        m_vdencLaInitDmemBuffer             = m_allocator->AllocateResource(allocParamsForBufferLinear, true);
        ENCODE_CHK_NULL_RETURN(m_vdencLaInitDmemBuffer);

        // Lookahead history buffer
        allocParamsForBufferLinear.dwBytes  = MOS_ALIGN_CEIL(m_LaHistoryBufSize, CODECHAL_PAGE_SIZE);
        allocParamsForBufferLinear.pBufName = "VDENC Lookahead History Buffer";
        m_vdencLaHistoryBuffer              = m_allocator->AllocateResource(allocParamsForBufferLinear, true);
        ENCODE_CHK_NULL_RETURN(m_vdencLaHistoryBuffer);

        // Lookahead Update DMEM
        m_vdencLaUpdateDmemBufferSize = sizeof(VdencHevcHucLaDmem);
        MOS_RESOURCE *allocatedbuffer;
        for (auto k = 0; k < CODECHAL_ENCODE_RECYCLED_BUFFER_NUM; k++)
        {
            for (auto i = 0; i < CODECHAL_LPLA_NUM_OF_PASSES; i++)
            {
                allocParamsForBufferLinear.dwBytes  = MOS_ALIGN_CEIL(m_vdencLaUpdateDmemBufferSize, CODECHAL_CACHELINE_SIZE);
                allocParamsForBufferLinear.pBufName = "VDENC Lookahead update Dmem Buffer";
                m_vdencLaUpdateDmemBuffer[k][i] = m_allocator->AllocateResource(allocParamsForBufferLinear, true);
                ENCODE_CHK_NULL_RETURN(m_vdencLaUpdateDmemBuffer[k][i]);
            }
        }

        // streamin Buffer for forced intra encode pass
        allocParamsForBufferLinear.dwBytes  = (MOS_ALIGN_CEIL(m_basicFeature->m_frameWidth, 64) / 32) * (MOS_ALIGN_CEIL(m_basicFeature->m_frameHeight, 64) / 32) * CODECHAL_CACHELINE_SIZE;
        allocParamsForBufferLinear.pBufName = "ForceIntra Streamin Buffer";
        m_forceIntraStreamInBuf = m_allocator->AllocateResource(allocParamsForBufferLinear, true);
        ENCODE_CHK_NULL_RETURN(m_forceIntraStreamInBuf);

        return eStatus; 
    }

    MOS_STATUS VdencLplaAnalysis::Update(void *params)
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;
        ENCODE_FUNC_CALL();

        ENCODE_CHK_NULL_RETURN(params);
        EncoderParams *encodeParams = (EncoderParams *)params;
        m_hevcSeqParams = static_cast<PCODEC_HEVC_ENCODE_SEQUENCE_PARAMS>(encodeParams->pSeqParams);
        ENCODE_CHK_NULL_RETURN(m_hevcSeqParams);
        m_lookaheadDepth = m_hevcSeqParams->LookaheadDepth;
        m_enabled = (m_lookaheadDepth > 0) && m_hevcSeqParams->bLookAheadPhase;
        if (!m_enabled)
        {
            return eStatus;
        }

        m_hevcPicParams = static_cast<PCODEC_HEVC_ENCODE_PICTURE_PARAMS>(encodeParams->pPicParams);
        ENCODE_CHK_NULL_RETURN(m_hevcPicParams);
        m_nalUnitParams = encodeParams->ppNALUnitParams;
        ENCODE_CHK_NULL_RETURN(m_nalUnitParams);
        m_slcData = (PCODEC_ENCODER_SLCDATA)(encodeParams->pSlcHeaderData);
        ENCODE_CHK_NULL_RETURN(m_slcData);
        m_numSlices = encodeParams->dwNumSlices;
        ENCODE_CHK_STATUS_RETURN(SetSequenceStructs());
        m_lastPicInStream = m_hevcPicParams->bLastPicInStream;

        m_hevcSliceParams = static_cast<PCODEC_HEVC_ENCODE_SLICE_PARAMS>(encodeParams->pSliceParams);
        ENCODE_CHK_STATUS_RETURN(SetupForceIntraStreamIn());

        if (!m_lastPicInStream)
        {
            m_numValidLaRecords++;
        }

        if (m_lastPicInStream && m_bLastPicFlagFirstIn)
        {
            m_currLaDataIdx -= 1;
            m_bLastPicFlagFirstIn = false;
        }

        return eStatus;
    }

    MOS_STATUS VdencLplaAnalysis::GetLplaStatusReport(
        EncodeStatusMfx *encodeStatusMfx,
        EncodeStatusReportData *statusReportData)
    {
        ENCODE_FUNC_CALL();
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        if (!m_enabled)
        {
            return eStatus;
        }

        if (m_lookaheadReport && (encodeStatusMfx->lookaheadStatus.targetFrameSize > 0))
        {
            statusReportData->pLookaheadStatus = &encodeStatusMfx->lookaheadStatus;
            encodeStatusMfx->lookaheadStatus.isValid = 1;
            uint64_t targetFrameSize = (uint64_t)encodeStatusMfx->lookaheadStatus.targetFrameSize * m_averageFrameSize;
            encodeStatusMfx->lookaheadStatus.targetFrameSize = (uint32_t)((targetFrameSize + (32 * 8)) / (64 * 8));  // Convert bits to bytes. 64 is normalized average frame size used in lookahead analysis kernel
            uint64_t targetBufferFulness = (uint64_t)encodeStatusMfx->lookaheadStatus.targetBufferFulness * m_averageFrameSize;
            encodeStatusMfx->lookaheadStatus.targetBufferFulness = (uint32_t)((targetBufferFulness + 32) / 64);  // 64 is normalized average frame size used in lookahead analysis kernel

            if (encodeStatusMfx->lookaheadStatus.miniGopSize == 2)
            {
                encodeStatusMfx->lookaheadStatus.miniGopSize = 2;
            }
            else if (encodeStatusMfx->lookaheadStatus.pyramidDeltaQP == 0)
            {
                encodeStatusMfx->lookaheadStatus.miniGopSize = 1;
            }
            else
            {
                if (m_hevcSeqParams->GopRefDist == 1) // LPLA only supports P pyramid for this condition
                {
                    encodeStatusMfx->lookaheadStatus.miniGopSize = 4;
                }
                else
                {
                    encodeStatusMfx->lookaheadStatus.miniGopSize = m_hevcSeqParams->GopRefDist;
                }
            }
        }

        return eStatus;
    }

#if USE_CODECHAL_DEBUG_TOOL
    MOS_STATUS VdencLplaAnalysis::DumpLaResource(EncodePipeline *pipeline, bool isInput)
    {
        ENCODE_FUNC_CALL();
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        if (!m_enabled)
        {
            return eStatus;
        }

        CodechalDebugInterface *debugInterface = pipeline->GetDebugInterface();
        ENCODE_CHK_NULL_RETURN(debugInterface);
        int32_t currentPass = pipeline->GetCurrentPass();

        ENCODE_CHK_STATUS_RETURN(debugInterface->DumpHucDmem(
            m_vdencLaUpdateDmemBuffer[pipeline->m_currRecycledBufIdx][currentPass],
            sizeof(VdencHevcHucLaDmem),
            currentPass,
            hucRegionDumpLAUpdate));

        ENCODE_CHK_STATUS_RETURN(debugInterface->DumpHucRegion(
            m_vdencLaHistoryBuffer,
            0,
            m_LaHistoryBufSize,
            0,
            "_History",
            isInput,
            currentPass,
            hucRegionDumpLAUpdate));

        ENCODE_CHK_STATUS_RETURN(debugInterface->DumpHucRegion(
            m_vdencLaStatsBuffer,
            0,
            m_brcLooaheadStatsBufferSize,
            1,
            "_Stats",
            isInput,
            currentPass,
            hucRegionDumpLAUpdate));

        ENCODE_CHK_STATUS_RETURN(debugInterface->DumpHucRegion(
            m_vdencLaDataBuffer,
            0,
            m_brcLooaheadDataBufferSize,
            2,
            "_Data",
            isInput,
            currentPass,
            hucRegionDumpLAUpdate));

        return eStatus;
    }
#endif

    MOS_STATUS VdencLplaAnalysis::GetLaStatsStoreIdx(uint8_t &index)
    {
        ENCODE_FUNC_CALL();
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        index = m_currLaDataIdx;

        return eStatus;
    }

    MOS_STATUS VdencLplaAnalysis::StoreLookaheadStatistics(MOS_COMMAND_BUFFER &cmdBuffer, MHW_VDBOX_NODE_IND vdboxIndex)
    {
        ENCODE_FUNC_CALL();
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        if (!m_enabled)
        {
            return eStatus;
        }

        uint8_t index = 0;
        ENCODE_CHK_STATUS_RETURN(GetLaStatsStoreIdx(index));
        uint32_t offset = sizeof(VdencHevcLaStats) * index;        
        if (m_useDSData)
        {
            auto &storeFrameByteCount            = m_miItf->MHW_GETPAR_F(MI_STORE_DATA_IMM)();
            storeFrameByteCount                  = {};
            storeFrameByteCount.pOsResource      = m_vdencLaStatsBuffer;
            storeFrameByteCount.dwResourceOffset = offset + CODECHAL_OFFSETOF(VdencHevcLaStats, frameByteCount);
            storeFrameByteCount.dwValue          = m_statsBuffer[index][0];
            ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_STORE_DATA_IMM)(&cmdBuffer));

            auto &storeHeaderBitCount            = m_miItf->MHW_GETPAR_F(MI_STORE_DATA_IMM)();
            storeHeaderBitCount                  = {};
            storeHeaderBitCount.pOsResource      = m_vdencLaStatsBuffer;
            storeHeaderBitCount.dwResourceOffset = offset + CODECHAL_OFFSETOF(VdencHevcLaStats, headerBitCount);
            storeHeaderBitCount.dwValue          = m_statsBuffer[index][1];
            ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_STORE_DATA_IMM)(&cmdBuffer));

            auto &storeIntraCuCount            = m_miItf->MHW_GETPAR_F(MI_STORE_DATA_IMM)();
            storeIntraCuCount                  = {};
            storeIntraCuCount.pOsResource      = m_vdencLaStatsBuffer;
            storeIntraCuCount.dwResourceOffset = offset + CODECHAL_OFFSETOF(VdencHevcLaStats, intraCuCount);
            storeIntraCuCount.dwValue          = m_statsBuffer[index][2];
            ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_STORE_DATA_IMM)(&cmdBuffer));

            auto &flushDwParams = m_miItf->MHW_GETPAR_F(MI_FLUSH_DW)();
            flushDwParams       = {};
            // Make Flush DW call to make sure all previous work is done
            ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_FLUSH_DW)(&cmdBuffer));
        }
        else
        {
            auto &miStoreRegMemParams = m_miItf->MHW_GETPAR_F(MI_STORE_REGISTER_MEM)();
            auto &miLoadRegMemParams  = m_miItf->MHW_GETPAR_F(MI_LOAD_REGISTER_MEM)();
            auto &flushDwParams       = m_miItf->MHW_GETPAR_F(MI_FLUSH_DW)();

            miStoreRegMemParams = {};
            miLoadRegMemParams  = {};
            flushDwParams       = {};

            auto mmioRegistersHcp          = m_hcpItf->GetMmioRegisters(vdboxIndex);
            miStoreRegMemParams.dwRegister = mmioRegistersHcp->hcpEncBitstreamBytecountFrameRegOffset;
            // Store BitstreamBytecount to m_vdencLaStatsBuffer
            miStoreRegMemParams.presStoreBuffer = m_vdencLaStatsBuffer;
            miStoreRegMemParams.dwOffset        = offset + CODECHAL_OFFSETOF(VdencHevcLaStats, frameByteCount);
            ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_STORE_REGISTER_MEM)(&cmdBuffer));

            // Calculate header size including LCU header
            uint32_t headerBitSize = 0;
            for (uint32_t i = 0; i < HEVC_MAX_NAL_UNIT_TYPE; i++)
            {
                headerBitSize += m_nalUnitParams[i]->uiSize * 8;
            }
            for (uint32_t i = 0; i < m_numSlices; i++)
            {
                headerBitSize += m_slcData[i].BitSize;
            }

            // Store headerBitCount to m_vdencLaStatsBuffer
            auto &storeDataParams            = m_miItf->MHW_GETPAR_F(MI_STORE_DATA_IMM)();
            storeDataParams                  = {};
            storeDataParams.pOsResource      = m_vdencLaStatsBuffer;
            storeDataParams.dwResourceOffset = offset + CODECHAL_OFFSETOF(VdencHevcLaStats, headerBitCount);
            storeDataParams.dwValue          = headerBitSize;
            ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_STORE_DATA_IMM)(&cmdBuffer));

            auto mmioRegs = m_miItf->GetMmioRegisters();
            ENCODE_CHK_NULL_RETURN(mmioRegs);
            // VCS_GPR0_Lo = LCUHdrBits
            miLoadRegMemParams.presStoreBuffer = m_basicFeature->m_recycleBuf->GetBuffer(FrameStatStreamOutBuffer, 0);  // LCUHdrBits is in m_resFrameStatStreamOutBuffer DW4
            miLoadRegMemParams.dwOffset        = 4 * sizeof(uint32_t);
            miLoadRegMemParams.dwRegister      = mmioRegs->generalPurposeRegister0LoOffset;  // VCS_GPR0_Lo
            ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_LOAD_REGISTER_MEM)(&cmdBuffer));
            ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_FLUSH_DW)(&cmdBuffer));

            // load headerBitCount(in m_vdencLaStatsBuffer) to VCS_GPR4_Lo
            miLoadRegMemParams.presStoreBuffer = m_vdencLaStatsBuffer;
            miLoadRegMemParams.dwOffset        = offset + CODECHAL_OFFSETOF(VdencHevcLaStats, headerBitCount);
            miLoadRegMemParams.dwRegister      = mmioRegs->generalPurposeRegister4LoOffset;
            ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_LOAD_REGISTER_MEM)(&cmdBuffer));

            mhw::mi::MHW_MI_ALU_PARAMS aluParams[4] = {};
            int32_t                    aluCount     = 0;

            //load1 srca, reg1
            aluParams[aluCount].AluOpcode = MHW_MI_ALU_LOAD;
            aluParams[aluCount].Operand1  = MHW_MI_ALU_SRCA;
            aluParams[aluCount].Operand2  = MHW_MI_ALU_GPREG0;
            ++aluCount;

            //load2 srcb, reg2
            aluParams[aluCount].AluOpcode = MHW_MI_ALU_LOAD;
            aluParams[aluCount].Operand1  = MHW_MI_ALU_SRCB;
            aluParams[aluCount].Operand2  = MHW_MI_ALU_GPREG4;
            ++aluCount;

            //add srca + srcb
            aluParams[aluCount].AluOpcode = MHW_MI_ALU_ADD;
            ++aluCount;

            //store reg1, accu
            aluParams[aluCount].AluOpcode = MHW_MI_ALU_STORE;
            aluParams[aluCount].Operand1  = MHW_MI_ALU_GPREG0;
            aluParams[aluCount].Operand2  = MHW_MI_ALU_ACCU;
            ++aluCount;

            auto &miMathParams          = m_miItf->MHW_GETPAR_F(MI_MATH)();
            miMathParams                = {};
            miMathParams.dwNumAluParams = aluCount;
            miMathParams.pAluPayload    = aluParams;
            ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_MATH)(&cmdBuffer));

            //store VCS_GPR0_Lo to m_vdencLaStatsBuffer
            miStoreRegMemParams.presStoreBuffer = m_vdencLaStatsBuffer;
            miStoreRegMemParams.dwOffset        = offset + CODECHAL_OFFSETOF(VdencHevcLaStats, headerBitCount);
            miStoreRegMemParams.dwRegister      = mmioRegs->generalPurposeRegister0LoOffset;
            ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_STORE_REGISTER_MEM)(&cmdBuffer));

            // Make Flush DW call to make sure all previous work is done
            ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_FLUSH_DW)(&cmdBuffer));

            ENCODE_CHK_STATUS_RETURN(StoreVdencStatistics(cmdBuffer, index));
        }
        return eStatus;
    }

    MOS_STATUS VdencLplaAnalysis::StoreVdencStatistics(MOS_COMMAND_BUFFER &cmdBuffer, uint8_t index)
    {
        ENCODE_FUNC_CALL();
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        uint32_t offset = sizeof(VdencHevcLaStats) * index;
        auto &miCpyMemMemParams       = m_miItf->MHW_GETPAR_F(MI_COPY_MEM_MEM)();
        miCpyMemMemParams             = {};
        miCpyMemMemParams.presSrc     = m_basicFeature->m_recycleBuf->GetBuffer(VdencStatsBuffer, 0);  // 8X8 Normalized intra CU count is in m_resVdencStatsBuffer DW1
        miCpyMemMemParams.dwSrcOffset = 4;
        miCpyMemMemParams.presDst     = m_vdencLaStatsBuffer;
        miCpyMemMemParams.dwDstOffset = offset + CODECHAL_OFFSETOF(VdencHevcLaStats, intraCuCount);
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_COPY_MEM_MEM)(&cmdBuffer));

        return eStatus;
    }

    MOS_STATUS VdencLplaAnalysis::ReadLPLAData(PMOS_COMMAND_BUFFER cmdBuffer, PMOS_RESOURCE resource, uint32_t baseOffset)
    {
        ENCODE_FUNC_CALL();

        // Write lookahead status to encode status buffer
        auto &miCpyMemMemParams = m_miItf->MHW_GETPAR_F(MI_COPY_MEM_MEM)();
        auto &flushDwParams     = m_miItf->MHW_GETPAR_F(MI_FLUSH_DW)();

        miCpyMemMemParams             = {};
        miCpyMemMemParams.presSrc     = m_vdencLaDataBuffer;
        miCpyMemMemParams.dwSrcOffset = m_offset * sizeof(CodechalVdencHevcLaData) + CODECHAL_OFFSETOF(CodechalVdencHevcLaData, encodeHints);
        miCpyMemMemParams.presDst     = resource;
        miCpyMemMemParams.dwDstOffset = baseOffset + CODECHAL_OFFSETOF(LookaheadReport, encodeHints);
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_COPY_MEM_MEM)(cmdBuffer));
        miCpyMemMemParams.dwSrcOffset = m_offset * sizeof(CodechalVdencHevcLaData) + CODECHAL_OFFSETOF(CodechalVdencHevcLaData, targetFrameSize);
        miCpyMemMemParams.dwDstOffset = baseOffset + CODECHAL_OFFSETOF(LookaheadReport, targetFrameSize);
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_COPY_MEM_MEM)(cmdBuffer));
        miCpyMemMemParams.dwSrcOffset = m_offset * sizeof(CodechalVdencHevcLaData) + CODECHAL_OFFSETOF(CodechalVdencHevcLaData, targetBufferFulness);
        miCpyMemMemParams.dwDstOffset = baseOffset + CODECHAL_OFFSETOF(LookaheadReport, targetBufferFulness);
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_COPY_MEM_MEM)(cmdBuffer));
        miCpyMemMemParams.dwSrcOffset = m_offset * sizeof(CodechalVdencHevcLaData) + CODECHAL_OFFSETOF(CodechalVdencHevcLaData, pyramidDeltaQP);
        miCpyMemMemParams.dwDstOffset = baseOffset + CODECHAL_OFFSETOF(LookaheadReport, pyramidDeltaQP);
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_COPY_MEM_MEM)(cmdBuffer));
        //MI_COPY_MEM_MEM reads a DWord from memory and stores it to memory. This copy will include adaptive_rounding and minigop
        miCpyMemMemParams.dwSrcOffset = m_offset * sizeof(CodechalVdencHevcLaData) + CODECHAL_OFFSETOF(CodechalVdencHevcLaData, adaptive_rounding);
        miCpyMemMemParams.dwDstOffset = baseOffset + CODECHAL_OFFSETOF(LookaheadReport, adaptive_rounding);
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_COPY_MEM_MEM)(cmdBuffer));

        flushDwParams = {};
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_FLUSH_DW)(cmdBuffer));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS VdencLplaAnalysis::SetLaUpdateDmemParameters(HUC_DMEM_STATE_PAR_ALIAS &params, uint8_t currRecycledBufIdx, uint16_t curPass, uint16_t numPasses)
    {
        ENCODE_FUNC_CALL();
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        if (!m_enabled)
        {
            return eStatus;
        }

        ENCODE_CHK_STATUS_RETURN(SetLaUpdateDmemBuffer(currRecycledBufIdx, m_currLaDataIdx, m_numValidLaRecords, curPass, numPasses));

        params.hucDataSource = m_vdencLaUpdateDmemBuffer[currRecycledBufIdx][curPass];
        params.dataLength = MOS_ALIGN_CEIL(m_vdencLaUpdateDmemBufferSize, CODECHAL_CACHELINE_SIZE);
        params.dmemOffset = HUC_DMEM_OFFSET_RTOS_GEMS;

        return eStatus;
    }

    MOS_STATUS VdencLplaAnalysis::UpdateLaDataIdx()
    {
        ENCODE_FUNC_CALL();
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        if (!m_lastPicInStream)
        {
            m_currLaDataIdx = (m_currLaDataIdx + 1) % m_numLaDataEntry;
        }

        return eStatus;
    }

    MOS_STATUS VdencLplaAnalysis::SetLaInitDmemBuffer() const
    {
        ENCODE_FUNC_CALL();
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        // Setup LAInit DMEM
        auto hucVdencLaInitDmem = (VdencHevcHucLaDmem *)m_allocator->LockResourceForWrite(m_vdencLaInitDmemBuffer);
        ENCODE_CHK_NULL_RETURN(hucVdencLaInitDmem);
        MOS_ZeroMemory(hucVdencLaInitDmem, sizeof(VdencHevcHucLaDmem));

        uint32_t initVbvFullness = MOS_MIN(m_hevcSeqParams->InitVBVBufferFullnessInBit, m_hevcSeqParams->VBVBufferSizeInBit);
        uint8_t downscaleRatioIndicator = 2;  // 4x downscaling
        if (m_hevcPicParams->DownScaleRatio.fields.X16Minus1_X == 15 && m_hevcPicParams->DownScaleRatio.fields.X16Minus1_Y == 15)
        {
            downscaleRatioIndicator = 0;  // no downscaling
        }

        hucVdencLaInitDmem->lookAheadFunc = 0;
        hucVdencLaInitDmem->lengthAhead = m_hevcSeqParams->LookaheadDepth;
        hucVdencLaInitDmem->vbvBufferSize = m_hevcSeqParams->VBVBufferSizeInBit / m_averageFrameSize;
        hucVdencLaInitDmem->vbvInitialFullness = initVbvFullness / m_averageFrameSize;
        hucVdencLaInitDmem->statsRecords = m_numLaDataEntry;
        hucVdencLaInitDmem->averageFrameSize = m_averageFrameSize >> 3;
        hucVdencLaInitDmem->downscaleRatio = downscaleRatioIndicator;
        ENCODE_CHK_NULL_RETURN(m_basicFeature);
        hucVdencLaInitDmem->enc_frame_width = m_basicFeature->m_frameWidth;
        hucVdencLaInitDmem->enc_frame_height = m_basicFeature->m_frameHeight;
        hucVdencLaInitDmem->codec_type = m_hevcSeqParams->FullPassCodecType;
        hucVdencLaInitDmem->mbr_ratio = (m_hevcSeqParams->TargetBitRate > 0 && m_hevcSeqParams->MaxBitRate >= m_hevcSeqParams->TargetBitRate) ?
                                        m_hevcSeqParams->MaxBitRate * 100 / m_hevcSeqParams->TargetBitRate : 100;

        if (m_hevcSeqParams->GopRefDist == 1)
        {
            hucVdencLaInitDmem->PGop = 4;
        }
        else
        {
            hucVdencLaInitDmem->BGop   = m_hevcSeqParams->GopRefDist;
            hucVdencLaInitDmem->maxGop = m_hevcSeqParams->GopPicSize;
        }

        hucVdencLaInitDmem->GopOpt = m_hevcSeqParams->GopFlags.fields.StrictGop ? 2 : m_hevcSeqParams->GopFlags.fields.ClosedGop;
        hucVdencLaInitDmem->AGop   = m_hevcSeqParams->GopFlags.fields.AdaptiveGop;
        if (m_hevcSeqParams->GopFlags.fields.AdaptiveGop)
        {
            hucVdencLaInitDmem->AGop_Threshold = 16;
        }
        hucVdencLaInitDmem->maxGop = m_hevcSeqParams->MaxAdaptiveGopPicSize;
        hucVdencLaInitDmem->minGop = m_hevcSeqParams->MinAdaptiveGopPicSize;
        hucVdencLaInitDmem->adaptiveIDR = (uint8_t)m_lookaheadAdaptiveI;
        hucVdencLaInitDmem->la_dump_type = 4;

        m_allocator->UnLock(m_vdencLaInitDmemBuffer);

        return eStatus;
    }

    MOS_STATUS VdencLplaAnalysis::SetLaUpdateDmemBuffer(uint8_t currRecycledBufIdx, uint8_t currLaDataIdx, uint32_t numValidLaRecords, uint16_t curPass, uint16_t numPasses)
    {
        ENCODE_FUNC_CALL();
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        // Setup LAUpdate DMEM
        auto hucVdencLaUpdateDmem = (VdencHevcHucLaDmem *)m_allocator->LockResourceForWrite(m_vdencLaUpdateDmemBuffer[currRecycledBufIdx][curPass]);
        ENCODE_CHK_NULL_RETURN(hucVdencLaUpdateDmem);
        MOS_ZeroMemory(hucVdencLaUpdateDmem, sizeof(VdencHevcHucLaDmem));

        hucVdencLaUpdateDmem->lookAheadFunc = 1;
        hucVdencLaUpdateDmem->validStatsRecords = numValidLaRecords;
        hucVdencLaUpdateDmem->offset = m_offset = (m_numLaDataEntry + currLaDataIdx + 1 - numValidLaRecords) % m_numLaDataEntry;
        hucVdencLaUpdateDmem->cqmQpThreshold = m_cqmQpThreshold;
        hucVdencLaUpdateDmem->currentPass = (uint8_t)curPass;

        m_allocator->UnLock(m_vdencLaUpdateDmemBuffer[currRecycledBufIdx][curPass]);

        return eStatus;
    }

    MOS_STATUS VdencLplaAnalysis::SetVdencPipeBufAddrParams(bool enableStreamIn, MHW_VDBOX_PIPE_BUF_ADDR_PARAMS &pipeBufAddrParams)
    {
        ENCODE_FUNC_CALL();
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        if (!m_enabled)
        {
            return eStatus;
        }

        if (enableStreamIn)
        {
            pipeBufAddrParams.presVdencStreamInBuffer = m_forceIntraStreamInBuf;
        }

        return eStatus;
    }

    bool VdencLplaAnalysis::IsLaAnalysisRequired()
    {
        return m_enabled;
    }

    bool VdencLplaAnalysis::IsLaRecordsEmpty()
    {
        if (!m_numValidLaRecords)
        {
            return true; 
        }
        m_numValidLaRecords--;
        return false;
    }

    MOS_STATUS VdencLplaAnalysis::CalculateLaRecords(bool blastPass)
    {
        ENCODE_FUNC_CALL();

        if ((blastPass && m_numValidLaRecords >= m_lookaheadDepth) ||
            (m_lastPicInStream && m_numValidLaRecords))
        {
            m_numValidLaRecords--;
            m_lookaheadReport = true;
        }
        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(VDENC_PIPE_MODE_SELECT, VdencLplaAnalysis)
    {
        ENCODE_FUNC_CALL();

        params.frameStatisticsStreamOut = m_enabled ? true : params.frameStatisticsStreamOut;

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(VDENC_PIPE_BUF_ADDR_STATE, VdencLplaAnalysis)
    {
        ENCODE_FUNC_CALL();

        if (m_streamInEnabled)
        {
            params.streamInBuffer = m_forceIntraStreamInBuf;
        }

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(VDENC_CMD2, VdencLplaAnalysis)
    {
        ENCODE_FUNC_CALL();

        if (m_streamInEnabled)
        {
            params.vdencStreamIn = true;
        }

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(HUC_DMEM_STATE, VdencLplaAnalysis)
    {
        ENCODE_FUNC_CALL();

        if (params.function == LA_INIT)
        {
            ENCODE_CHK_STATUS_RETURN(SetLaInitDmemBuffer());

            params.hucDataSource = m_vdencLaInitDmemBuffer;
            params.dataLength    = MOS_ALIGN_CEIL(m_vdencLaInitDmemBufferSize, CODECHAL_CACHELINE_SIZE);
            params.dmemOffset    = HUC_DMEM_OFFSET_RTOS_GEMS;
        }

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(HUC_VIRTUAL_ADDR_STATE, VdencLplaAnalysis)
    {
        ENCODE_FUNC_CALL();

        switch (params.function)
        {
            case LA_INIT: 
            {
                params.regionParams[0].presRegion = m_vdencLaHistoryBuffer;
                params.regionParams[0].isWritable = true;
                m_lookaheadInit                   = false;

                break;
            }
            case LA_UPDATE: 
            {
                params.regionParams[0].presRegion = m_vdencLaHistoryBuffer;
                params.regionParams[0].isWritable = true;
                params.regionParams[1].presRegion = m_vdencLaStatsBuffer;
                params.regionParams[2].presRegion = m_vdencLaDataBuffer;
                params.regionParams[2].isWritable = true;

                break;
            }
            default:
                break;
        }

        return MOS_STATUS_SUCCESS;
    }

} //encode
