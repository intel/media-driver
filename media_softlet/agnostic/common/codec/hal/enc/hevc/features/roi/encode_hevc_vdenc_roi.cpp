/*
* Copyright (c) 2018-2020, Intel Corporation
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
//! \file     encode_hevc_vdenc_roi.cpp
//! \brief    implementation of ROI feature of HEVC VDENC

#include "mos_defs.h"
#include "encode_hevc_vdenc_roi.h"
#include "encode_hevc_vdenc_feature_manager.h"

namespace encode
{

HevcVdencRoi::HevcVdencRoi(
    MediaFeatureManager *featureManager,
    EncodeAllocator *allocator,
    CodechalHwInterfaceNext *hwInterface,
    void *constSettings) :
    MediaFeature(constSettings, hwInterface ? hwInterface->GetOsInterface() : nullptr),
    m_allocator(allocator),
    m_hwInterface(hwInterface)
{
    ENCODE_CHK_NULL_NO_STATUS_RETURN(m_hwInterface);
    m_osInterface = m_hwInterface->GetOsInterface();
    ENCODE_CHK_NULL_NO_STATUS_RETURN(m_osInterface);

    m_featureManager = featureManager;
    ENCODE_CHK_NULL_NO_STATUS_RETURN(m_featureManager);

    m_basicFeature = dynamic_cast<EncodeBasicFeature *>(m_featureManager->GetFeature(FeatureIDs::basicFeature));
    ENCODE_CHK_NULL_NO_STATUS_RETURN(m_basicFeature);
}
MOS_STATUS HevcVdencRoi::ClearStreaminBuffer(uint32_t lucNumber)
{
    // Clear streamin
    ENCODE_CHK_NULL_RETURN(m_streamInTemp);

    uint8_t *data = (uint8_t *)m_allocator->LockResourceForWrite(m_streamIn);
    ENCODE_CHK_NULL_RETURN(data);

    MOS_ZeroMemory(m_streamInTemp, lucNumber * 64);
    MOS_SecureMemcpy(data, m_streamInSize, m_streamInTemp, m_streamInSize);

    ENCODE_CHK_STATUS_RETURN(m_allocator->UnLock(m_streamIn));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcVdencRoi::Init(void *setting)
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_NULL_RETURN(setting);
    ENCODE_CHK_NULL_RETURN(m_basicFeature);
    ENCODE_CHK_NULL_RETURN(m_basicFeature->m_recycleBuf);

#if (_DEBUG || _RELEASE_INTERNAL)
    MediaUserSetting::Value outValue;
    ReadUserSetting(
        m_userSettingPtr,
        outValue,
        "Disable TCBRC ARB for HEVC VDEnc",
        MediaUserSetting::Group::Sequence);
    m_isArbRoiSupported = !outValue.Get<bool>();
#endif

    MOS_ALLOC_GFXRES_PARAMS allocParams;
    MOS_ZeroMemory(&allocParams, sizeof(MOS_ALLOC_GFXRES_PARAMS));
    allocParams.Type     = MOS_GFXRES_BUFFER;
    allocParams.TileType = MOS_TILE_LINEAR;
    allocParams.Format   = Format_Buffer;

    //reuse 1 streamIn buffer to save latency so enlarge 8-rows buffer w/ different offset for each frame in arb cycle
    allocParams.dwBytes = (MOS_ALIGN_CEIL(m_basicFeature->m_frameWidth, 64) / 32) *
                          ((MOS_ALIGN_CEIL(m_basicFeature->m_frameHeight, 64) / 32) + 8) * CODECHAL_CACHELINE_SIZE;
    m_streamInSize      = allocParams.dwBytes;

    allocParams.pBufName = "VDEnc StreamIn Data Buffer";
    allocParams.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_NOCACHE;
    m_basicFeature->m_recycleBuf->RegisterResource(RecycleResId::StreamInBuffer, allocParams);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcVdencRoi::Update(void *params)
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_NULL_RETURN(params);
    ENCODE_CHK_NULL_RETURN(m_basicFeature->m_recycleBuf);

    PCODEC_HEVC_ENCODE_SEQUENCE_PARAMS hevcSeqParams = nullptr;
    PCODEC_HEVC_ENCODE_PICTURE_PARAMS  hevcPicParams = nullptr;
    PCODEC_HEVC_ENCODE_SLICE_PARAMS    hevcSlcParams = nullptr;

    EncoderParams *encodeParams = (EncoderParams *)params;

    hevcSeqParams = static_cast<PCODEC_HEVC_ENCODE_SEQUENCE_PARAMS>(encodeParams->pSeqParams);
    hevcPicParams = static_cast<PCODEC_HEVC_ENCODE_PICTURE_PARAMS>(encodeParams->pPicParams);
    hevcSlcParams = static_cast<PCODEC_HEVC_ENCODE_SLICE_PARAMS>(encodeParams->pSliceParams);
    ENCODE_CHK_NULL_RETURN(hevcSeqParams);
    ENCODE_CHK_NULL_RETURN(hevcPicParams);
    ENCODE_CHK_NULL_RETURN(hevcSlcParams);

    bool pririotyDirtyROI = true;

    m_dirtyRoiEnabled = hevcPicParams->NumDirtyRects && (B_TYPE == hevcPicParams->CodingType);
    m_mbQpDataEnabled = m_basicFeature->m_mbQpDataEnabled;
    // Adaptive region boost is enabled for TCBRC only
    m_isArbRoi        = hevcPicParams->TargetFrameSize != 0 && (hevcSeqParams->LookaheadDepth == 0) && m_isArbRoiSupported;
    m_roiEnabled      = hevcPicParams->NumROI > 0 || m_mbQpDataEnabled || m_isArbRoi;

    m_enabled = (m_roiEnabled || m_dirtyRoiEnabled) ? true : false;

    if (!m_enabled)
    {
        return MOS_STATUS_SUCCESS;
    }

    if (!m_isArbRoi)
    {
        m_streamIn = m_basicFeature->m_recycleBuf->GetBuffer(RecycleResId::StreamInBuffer, m_basicFeature->m_frameNum);
    }
    else
    {
        uint32_t streamInBufferIdx = hevcPicParams->CodingType == I_TYPE ? 0 : 1;
        m_streamIn                 = m_basicFeature->m_recycleBuf->GetBuffer(RecycleResId::StreamInBuffer, streamInBufferIdx);

        uint16_t ArbBoostRow[8] = {0, 3, 5, 2, 7, 4, 1, 6};
        uint16_t factor = 8 - ArbBoostRow[m_basicFeature->m_frameNum % 8];

        if (factor % 2 == 0)
        {
            m_streamIn->dwResourceOffset = factor * (MOS_ALIGN_CEIL(m_basicFeature->m_frameWidth, 64) / 32) * CODECHAL_CACHELINE_SIZE;
        }
        else
        {
            m_streamIn->dwResourceOffset = ((factor + 1) * (MOS_ALIGN_CEIL(m_basicFeature->m_frameWidth, 64) / 32) - 2) * CODECHAL_CACHELINE_SIZE;
        }
    }
    ENCODE_CHK_NULL_RETURN(m_streamIn);

    if (!m_isArbRoi || (hevcPicParams->CodingType == I_TYPE && !IFrameIsSet) || ((hevcPicParams->CodingType == P_TYPE || hevcPicParams->CodingType == B_TYPE) && !PBFrameIsSet))
    {
        m_streamInTemp = (uint8_t *)MOS_AllocMemory(m_streamInSize);
        ENCODE_CHK_NULL_RETURN(m_streamInTemp);

        uint32_t lcuNumber = GetLCUNumber();

        ENCODE_CHK_STATUS_RETURN(ClearStreaminBuffer(lcuNumber));

        m_roiOverlap.Update(lcuNumber);

        ENCODE_CHK_STATUS_RETURN(ExecuteDirtyRoi(hevcSeqParams, hevcPicParams, hevcSlcParams));

        MEDIA_WA_TABLE *waTable = m_basicFeature->GetWaTable();
        ENCODE_CHK_NULL_RETURN(waTable);

        if (MEDIA_IS_WA(waTable, WaHEVCVDEncForceDeltaQpRoiNotSupported) || m_isArbRoi || m_mbQpDataEnabled)
        {
            m_roiMode = false;
            ENCODE_CHK_STATUS_RETURN(ExecuteRoi(hevcSeqParams, hevcPicParams, hevcSlcParams));
        }
        else
        {
            m_roiMode = true;
            ENCODE_CHK_STATUS_RETURN(ExecuteRoiExt(hevcSeqParams, hevcPicParams, hevcSlcParams));
        }

        ENCODE_CHK_STATUS_RETURN(WriteStreaminData());

        MOS_SafeFreeMemory(m_streamInTemp);

#if (_DEBUG || _RELEASE_INTERNAL)
        ENCODE_CHK_NULL_RETURN(m_hwInterface);
        ENCODE_CHK_NULL_RETURN(m_hwInterface->GetOsInterface());
        ReportUserSettingForDebug(
            m_userSettingPtr ,
            "HEVC VDEnc Force Delta QP Enable",
            m_roiMode,
            MediaUserSetting::Group::Sequence);
#endif

        if (hevcPicParams->CodingType == I_TYPE)
        {
            IFrameIsSet = true;
        }
        else
        {
            PBFrameIsSet = true;
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcVdencRoi::WriteStreaminData()
{
    ENCODE_CHK_NULL_RETURN(m_streamIn);
    ENCODE_CHK_NULL_RETURN(m_streamInTemp);

    uint8_t *streaminBuffer = (uint8_t *)m_allocator->LockResourceForWrite(m_streamIn);
    ENCODE_CHK_NULL_RETURN(streaminBuffer);

    m_roiOverlap.WriteStreaminData(
        m_strategyFactory.GetRoi(), 
        m_strategyFactory.GetDirtyRoi(),
        m_streamInTemp);

    MOS_SecureMemcpy(streaminBuffer, m_streamInSize, m_streamInTemp, m_streamInSize);

    m_allocator->UnLock(m_streamIn);
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcVdencRoi::ExecuteRoi(
    SeqParams *hevcSeqParams,
    PicParams *hevcPicParams,
    SlcParams *hevcSlcParams)
{
    if (!m_roiEnabled)
    {
        return MOS_STATUS_SUCCESS;
    }

    ENCODE_CHK_NULL_RETURN(m_featureManager);

    if (!m_mbQpDataEnabled)
    {
        uint8_t numDistinctDeltaQp = sizeof(hevcPicParams->ROIDistinctDeltaQp) / sizeof(int8_t);
        m_isNativeRoi = ProcessRoiDeltaQp(
            hevcPicParams->NumROI,
            hevcPicParams->ROI,
            numDistinctDeltaQp,
            hevcPicParams->ROIDistinctDeltaQp);

        if (m_isArbRoi)
        {
            m_isNativeRoi = false;
        }
    }

    RoiStrategy *strategy = m_strategyFactory.CreateStrategy(
        m_allocator, m_featureManager, m_osInterface, m_isArbRoi, false, m_isNativeRoi, m_mbQpDataEnabled);
    ENCODE_CHK_NULL_RETURN(strategy);
    strategy->SetFeatureSetting(static_cast<HevcVdencFeatureSettings *>(m_constSettings));
    ENCODE_CHK_STATUS_RETURN(
        strategy->PrepareParams(hevcSeqParams, hevcPicParams, hevcSlcParams));

    ENCODE_CHK_STATUS_RETURN(strategy->SetupRoi(m_roiOverlap));
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcVdencRoi::ExecuteRoiExt(
    SeqParams *hevcSeqParams,
    PicParams *hevcPicParams,
    SlcParams *hevcSlcParams)
{
    if (!m_roiEnabled)
    {
        return MOS_STATUS_SUCCESS;
    }

    ENCODE_CHK_NULL_RETURN(m_featureManager);


    RoiStrategy* strategy = m_strategyFactory.CreateStrategyForceDeltaQP(
                                     m_allocator, m_featureManager, m_osInterface);

    ENCODE_CHK_NULL_RETURN(strategy);
    strategy->SetFeatureSetting(static_cast<HevcVdencFeatureSettings *>(m_constSettings));
    ENCODE_CHK_STATUS_RETURN(
        strategy->PrepareParams(hevcSeqParams, hevcPicParams, hevcSlcParams));

    ENCODE_CHK_STATUS_RETURN(strategy->SetupRoi(m_roiOverlap));
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcVdencRoi::ExecuteDirtyRoi(
    SeqParams *hevcSeqParams,
    PicParams *hevcPicParams,
    SlcParams *hevcSlcParams)
{
    if (!m_dirtyRoiEnabled)
    {
        return MOS_STATUS_SUCCESS;
    }

    ENCODE_CHK_NULL_RETURN(m_featureManager);

    RoiStrategy *strategy = m_strategyFactory.CreateStrategy(
                                    m_allocator, m_featureManager, m_osInterface, false, true, false);
    ENCODE_CHK_NULL_RETURN(strategy);
    strategy->SetFeatureSetting(static_cast<HevcVdencFeatureSettings *>(m_constSettings));
    ENCODE_CHK_STATUS_RETURN(
        strategy->PrepareParams(hevcSeqParams, hevcPicParams, hevcSlcParams));

    ENCODE_CHK_STATUS_RETURN(strategy->SetupRoi(m_roiOverlap));

    return MOS_STATUS_SUCCESS;
}

bool HevcVdencRoi::ProcessRoiDeltaQp(
    uint8_t    numROI,
    CODEC_ROI  *roiRegions,
    uint8_t    numDistinctDeltaQp,
    int8_t     *roiDistinctDeltaQp)
{
    ENCODE_FUNC_CALL();

    // Intialize ROIDistinctDeltaQp to be min expected delta qp, setting to -128
    // Check if forceQp is needed or not
    // forceQp is enabled if there are greater than 3 distinct delta qps or if the deltaqp is beyond range (-8, 7)
    for (auto k = 0; k < numDistinctDeltaQp; k++)
    {
        roiDistinctDeltaQp[k] = -128;
    }

    int32_t numQp = 0;
    for (int32_t i = 0; i < numROI; i++)
    {
        bool dqpNew = true;

        //Get distinct delta Qps among all ROI regions, index 0 having the lowest delta qp
        int32_t k = numQp - 1;
        for (; k >= 0; k--)
        {
            if (roiRegions[i].PriorityLevelOrDQp == roiDistinctDeltaQp[k] || 
                roiRegions[i].PriorityLevelOrDQp == 0)
            {
                dqpNew = false;
                break;
            }
            else if (roiRegions[i].PriorityLevelOrDQp < roiDistinctDeltaQp[k])
            {
                continue;
            }
            else
            {
                break;
            }
        }

        if (dqpNew)
        {
            for (int32_t j = numQp - 1; j >= k + 1; j--)
            {
                roiDistinctDeltaQp[j + 1] = roiDistinctDeltaQp[j];
            }
            roiDistinctDeltaQp[k + 1] = roiRegions[i].PriorityLevelOrDQp;
            numQp++;
        }
    }

    //Set the ROI DeltaQp to zero for remaining array elements
    for (auto k = numQp; k < m_maxNumRoi; k++)
    {
        roiDistinctDeltaQp[k] = 0;
    }

    // return whether is native ROI or not
    return (!(numQp > m_maxNumNativeRoi || roiDistinctDeltaQp[0] < -8 || roiDistinctDeltaQp[numQp - 1] > 7));
}

MOS_STATUS HevcVdencRoi::SetVdencPipeBufAddrParams(
    MHW_VDBOX_PIPE_BUF_ADDR_PARAMS &pipeBufAddrParams)
{
    if (!m_enabled)
    {
        return MOS_STATUS_SUCCESS;
    }

    RoiStrategy *strategy = GetStrategyForParamsSetting();
    ENCODE_CHK_NULL_RETURN(strategy);

    strategy->SetVdencPipeBufAddrParams(m_streamIn, pipeBufAddrParams);
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcVdencRoi::SetDmemHuCBrcInitReset(
    VdencHevcHucBrcInitDmem *hucVdencBrcInitDmem)
{
    if (!m_enabled)
    {
        return MOS_STATUS_SUCCESS;
    }

    RoiStrategy *strategy = GetStrategyForParamsSetting();
    ENCODE_CHK_NULL_RETURN(strategy);

    return strategy->SetDmemHuCBrcInitReset(hucVdencBrcInitDmem);
}

MHW_SETPAR_DECL_SRC(VDENC_PIPE_BUF_ADDR_STATE, HevcVdencRoi)
{
    if (!m_enabled)
    {
        return MOS_STATUS_SUCCESS;
    }

    RoiStrategy *strategy = GetStrategyForParamsSetting();
    ENCODE_CHK_NULL_RETURN(strategy);

    auto buf = strategy->GetStreamInBuf();
    params.streamInBuffer = (buf == nullptr) ? m_streamIn : buf;

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(VDENC_CMD2, HevcVdencRoi)
{
    if (!m_enabled)
    {
        return MOS_STATUS_SUCCESS;
    }

    auto hevcFeature = dynamic_cast<HevcBasicFeature *>(m_basicFeature);
    ENCODE_CHK_NULL_RETURN(hevcFeature);

    params.vdencStreamIn = m_enabled;
    params.roiStreamIn   = m_isNativeRoi || m_isArbRoi;

    if (m_isNativeRoi)
    {
        int8_t roiTable[ENCODE_VDENC_HEVC_MAX_STREAMINROI_G10] = {0};

        for (uint8_t i = 0; i < ENCODE_VDENC_HEVC_MAX_STREAMINROI_G10; i++)
        {
            roiTable[i] = (int8_t)CodecHal_Clip3(
                ENCODE_VDENC_HEVC_MIN_ROI_DELTA_QP_G10, ENCODE_VDENC_HEVC_MAX_ROI_DELTA_QP_G10, hevcFeature->m_hevcPicParams->ROIDistinctDeltaQp[i]);
        }

#if !(_MEDIA_RESERVED)
        params.extSettings.emplace_back(
            [&roiTable](uint32_t *data) {
                data[13] |= (roiTable[0] << 4);
                data[13] |= (roiTable[1] << 8);
                data[13] |= (roiTable[2] << 12);
                return MOS_STATUS_SUCCESS;
            });
#else
        params.vdencCmd2Par43[1] = roiTable[0];
        params.vdencCmd2Par43[2] = roiTable[1];
        params.vdencCmd2Par43[3] = roiTable[2];
#endif  // !(_MEDIA_RESERVED)
    }

    bool flag0 = false;
    bool flag1 = false;
    if (m_roiMode)
    {
        flag0 = 1;
        flag1 = 1;
    }
    else if (!m_roiMode && !m_isNativeRoi && !m_dirtyRoiEnabled && !m_isArbRoi)
    {
        flag0 = 0;
        flag1 = 1;
    }

#if !(_MEDIA_RESERVED)
    params.extSettings.emplace_back(
        [flag0, flag1](uint32_t *data) {
            data[20] |= (flag0 << 17);
            data[20] |= (flag1 << 19);
            return MOS_STATUS_SUCCESS;
        });
#else
    params.vdencCmd2Par60 = flag0;
    params.vdencCmd2Par61 = flag1;
#endif  // !(_MEDIA_RESERVED)

    return MOS_STATUS_SUCCESS;
}
}
