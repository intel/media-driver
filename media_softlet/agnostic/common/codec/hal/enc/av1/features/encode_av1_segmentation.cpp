/*
* Copyright (c) 2019-2022, Intel Corporation
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
//! \file     encode_av1_segmentation.cpp
//! \brief    Defines the common interface for encode av1 segmentation feature
//!

#include "encode_av1_segmentation.h"
#include "encode_av1_basic_feature.h"
#include "encode_av1_stream_in.h"

namespace encode
{
    Av1Segmentation::Av1Segmentation(
        MediaFeatureManager *featureManager,
        EncodeAllocator *allocator,
        void *constSettings) :
        MediaFeature(constSettings),
        m_allocator(allocator)
    {
        ENCODE_FUNC_CALL();
        ENCODE_CHK_NULL_NO_STATUS_RETURN(featureManager);

        m_basicFeature = dynamic_cast<Av1BasicFeature *>(featureManager->GetFeature(Av1FeatureIDs::basicFeature));
        m_featureManager = featureManager;
        ENCODE_CHK_NULL_NO_STATUS_RETURN(m_basicFeature);
    }

    Av1Segmentation::~Av1Segmentation()
    {
        ENCODE_FUNC_CALL();
    }

    inline uint8_t GetBlockSize(uint32_t blockSizeId)
    {
        switch (blockSizeId)
        {
        case 0: return 16;
        case 1: return 32;
        case 2: return 64;
        case 3: return 8;
        default:
            ENCODE_ASSERTMESSAGE("Block size for segment map must be set");
            return 16;
        }
    }

    MOS_STATUS Av1Segmentation::Update(void *params)
    {
        ENCODE_FUNC_CALL();
        ENCODE_CHK_NULL_RETURN(params);

        EncoderParamsAV1 *encodeParams = (EncoderParamsAV1 *)params;

        const auto av1PicParams = static_cast<PCODEC_AV1_ENCODE_PICTURE_PARAMS>(encodeParams->pPicParams);
        ENCODE_CHK_NULL_RETURN(av1PicParams);

        const auto av1SeqParams = static_cast<PCODEC_AV1_ENCODE_SEQUENCE_PARAMS>(encodeParams->pSeqParams);
        ENCODE_CHK_NULL_RETURN(av1SeqParams);

        m_targetUsage = av1SeqParams->TargetUsage;

        const auto& ddiSegments = av1PicParams->stAV1Segments;

        MOS_ZeroMemory(&m_segmentParams, sizeof(m_segmentParams));
        for (auto i = 0; i < av1MaxSegments; i++)
        {
            // set QM related to default value.
            m_segmentParams.m_qmLevelU[i] = m_segmentParams.m_qmLevelV[i] = m_segmentParams.m_qmLevelY[i] = 15;

            if (av1PicParams->wQMatrixFlags.fields.using_qmatrix)
            {
                m_segmentParams.m_qmLevelY[i] = av1PicParams->wQMatrixFlags.fields.qm_y;
                m_segmentParams.m_qmLevelU[i] = av1PicParams->wQMatrixFlags.fields.qm_u;
                m_segmentParams.m_qmLevelV[i] = av1PicParams->wQMatrixFlags.fields.qm_v;
            }
        }

        m_segmentParams.m_enabled = ddiSegments.SegmentFlags.fields.segmentation_enabled;
        m_segmentParams.m_updateMap = ddiSegments.SegmentFlags.fields.update_map;
        m_segmentParams.m_temporalUpdate = ddiSegments.SegmentFlags.fields.temporal_update;

        m_segmentNum = ddiSegments.SegmentFlags.fields.SegmentNumber;

        m_segmentMapBlockSize = GetBlockSize(av1PicParams->PicFlags.fields.SegIdBlockSize);

        m_hasZeroSegmentQIndex = false;

        const auto currRefList = m_basicFeature->m_ref.GetCurrRefList();
        ENCODE_CHK_NULL_RETURN(currRefList);
        if (av1PicParams->PicFlags.fields.frame_type == keyFrame)
        {
            memset(m_segmenBufferinUse, 0, sizeof(m_segmenBufferinUse));
            memset(m_ucScalingIdtoSegID, -1, sizeof(m_ucScalingIdtoSegID));
        }
        if (!m_basicFeature->m_av1PicParams->PicFlags.fields.DisableFrameRecon && 
            m_ucScalingIdtoSegID[currRefList->ucScalingIdx] != -1)
        {
            m_segmenBufferinUse[m_ucScalingIdtoSegID[currRefList->ucScalingIdx]]--;
        }

        if (m_segmentParams.m_enabled)
        {
            ENCODE_CHK_STATUS_RETURN(SetSegmentIdParams(av1PicParams, &ddiSegments));

            m_pSegmentMap = nullptr;
            m_segmentMapProvided = false;
            m_segmentMapDataSize = 0;

            if (encodeParams->pSegmentMap)
            {
                m_pSegmentMap = encodeParams->pSegmentMap;
                m_segmentMapProvided = encodeParams->bSegmentMapProvided;
                m_segmentMapDataSize = encodeParams->segmentMapDataSize;
            }

            if (m_segmentParams.m_temporalUpdate != 0)
            {
                if (!m_segmentParams.m_updateMap)
                {
                    ENCODE_ASSERTMESSAGE("\"Temporal_update\" is set when \"update_map\" is zero");
                    return MOS_STATUS_INVALID_PARAMETER;
                }

                const auto primRefList = m_basicFeature->m_ref.GetPrimaryRefList();
                ENCODE_CHK_NULL_RETURN(primRefList);

                const uint8_t ft = av1PicParams->PicFlags.fields.frame_type;
                if (ft == keyFrame || ft == intraOnlyFrame || !primRefList->m_segmentEnable)
                {
                    ENCODE_ASSERTMESSAGE("Temporal update for segmentation map cannot be applied");
                    return MOS_STATUS_INVALID_PARAMETER;
                }
            }

            ENCODE_CHK_STATUS_RETURN(CheckQPAndLossless());

            if (m_segmentMapProvided)
            {
                m_streamIn = m_basicFeature->GetStreamIn();
                ENCODE_CHK_NULL_RETURN(m_streamIn);
                ENCODE_CHK_STATUS_RETURN(m_streamIn->Update());
                ENCODE_CHK_STATUS_RETURN(SetupSegmentationMap());
            }
        }
        else
        {
            if (m_segmentParams.m_updateData || m_segmentParams.m_temporalUpdate)
            {
                ENCODE_ASSERTMESSAGE("\"Update_map\" or \"temporal_update\" is enabled when segmentation is disabled.");
                return MOS_STATUS_INVALID_PARAMETER;
            }

            // even if segmentation is disabled, segment params for segment 0 are still passed to HW
            // lossless flag should be set correctly for segment 0
            m_segmentParams.m_losslessFlag[0] = IsFrameLossless(*av1PicParams);

            m_hasZeroSegmentQIndex = av1PicParams->base_qindex == 0;
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1Segmentation::CheckQPAndLossless()
    {
        ENCODE_FUNC_CALL();

        const auto* picPar = m_basicFeature->m_av1PicParams;

        ENCODE_CHK_NULL_RETURN(picPar);

        if (IsFrameLossless(*picPar))
        {
            ENCODE_ASSERTMESSAGE("Segmentation can't be enabled for lossless frame.");
            return MOS_STATUS_INVALID_PARAMETER;
        }

        for (uint8_t i = 0; i < m_segmentNum; i++)
        {
            const int16_t segQP = picPar->base_qindex + m_segmentParams.m_featureData[i][segLvlAltQ];

            if (segQP < 0)
            {
                ENCODE_ASSERTMESSAGE("segQP < 0 is not supported.");
                return MOS_STATUS_INVALID_PARAMETER;
            }

            const uint8_t clippedSegQP = static_cast<uint8_t>(CodecHal_Clip3(0, 255, segQP));

            if (clippedSegQP == 0)
            {
                if (DeltaQIsZero(*picPar))
                {
                    ENCODE_ASSERTMESSAGE("Lossless segment isn't supported.");
                    return MOS_STATUS_INVALID_PARAMETER;
                }

                m_hasZeroSegmentQIndex = true;
            }

            m_segmentParams.m_featureData[i][segLvlAltQ] = clippedSegQP - picPar->base_qindex;
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1Segmentation::SetSegmentIdParams(
        const PCODEC_AV1_ENCODE_PICTURE_PARAMS  ddiPicParams,
        const CODEC_Intel_Seg_AV1              *ddiSegParams)
    {
        ENCODE_FUNC_CALL();

        ENCODE_CHK_NULL_RETURN(ddiPicParams);
        ENCODE_CHK_NULL_RETURN(ddiSegParams);
        ENCODE_CHK_NULL_RETURN(m_basicFeature);

        for (auto i = 0; i < av1MaxSegments; i++)
        {
            if (ddiPicParams->wQMatrixFlags.fields.using_qmatrix)
            {
                m_segmentParams.m_qmLevelU[i] = ddiPicParams->wQMatrixFlags.fields.qm_u;
                m_segmentParams.m_qmLevelV[i] = ddiPicParams->wQMatrixFlags.fields.qm_v;
                m_segmentParams.m_qmLevelY[i] = ddiPicParams->wQMatrixFlags.fields.qm_y;
            }
        }

        MOS_SecureMemcpy(
            m_segmentParams.m_featureData,
            sizeof(m_segmentParams.m_featureData),
            ddiSegParams->feature_data,
            sizeof(m_segmentParams.m_featureData));

        MOS_SecureMemcpy(
            m_segmentParams.m_featureMask,
            sizeof(m_segmentParams.m_featureMask),
            ddiSegParams->feature_mask,
            sizeof(m_segmentParams.m_featureMask));

        for (uint8_t seg = 0; seg < av1MaxSegments; seg++)
        {
            for (int lvl = 0; lvl < segLvlMax; lvl++)
            {
                if (m_segmentParams.m_featureMask[seg] & (1 << lvl))
                {
                    m_segmentParams.m_preSkipSegmentIdFlag |= (lvl >= segLvlRefFrame);
                    m_segmentParams.m_lastActiveSegmentId = seg;
                }
            }
        }

        // Configure Segment ID read buffer and SegmentMapIsZeroFlag
        m_segmentParams.m_segmentMapIsZeroFlag = false;
        // pass AVP seg map stream-in only in case of segmentation_temporal_update = 1
        m_segmentParams.m_segIdBufStreamInEnable = m_segmentParams.m_temporalUpdate ? true : false;
        // pass AVP seg map stream-out only if segmentation_update_map = 1
        m_segmentParams.m_segIdBufStreamOutEnable = m_segmentParams.m_updateMap ? true : false;

        const bool usePrimaryMap = !m_segmentParams.m_updateMap || m_segmentParams.m_temporalUpdate;

        if (usePrimaryMap)
        {
            if (!m_basicFeature->m_ref.CheckSegmentForPrimeFrame())
            {
                m_segmentParams.m_segmentMapIsZeroFlag   = true;
                m_segmentParams.m_segIdBufStreamInEnable = false;
            }
        }

        if (!m_basicFeature->m_av1PicParams->PicFlags.fields.DisableFrameRecon)
        {
            const auto currRefList = m_basicFeature->m_ref.GetCurrRefList();
            ENCODE_CHK_NULL_RETURN(currRefList);
            if (!m_segmentParams.m_segIdBufStreamOutEnable)
            {
                const auto primRefList = m_basicFeature->m_ref.GetPrimaryRefList();
                ENCODE_CHK_NULL_RETURN(primRefList);
                currRefList->m_segIdBufIdx = primRefList->m_segIdBufIdx;
            }
            else
            {
                //the maximum DPB length of AV1 is 8, free IDs will always be found.
                for (uint8_t i = 0; i < av1TotalRefsPerFrame; i++)
                {
                    if (m_segmenBufferinUse[i] == 0)
                    {
                        currRefList->m_segIdBufIdx = i;
                        break;
                    }
                }
            }
            if (m_segmentMapBuffer[currRefList->m_segIdBufIdx] == nullptr)
            {
                ENCODE_CHK_STATUS_RETURN(AllocateSegmentationMapBuffer(currRefList->m_segIdBufIdx));
            }
            m_segmenBufferinUse[currRefList->m_segIdBufIdx]++;
            m_ucScalingIdtoSegID[currRefList->ucScalingIdx] = currRefList->m_segIdBufIdx;
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1Segmentation::SetupSegmentationMap()
    {
        ENCODE_FUNC_CALL();

        ENCODE_CHK_STATUS_RETURN(CheckSegmentationMap());

        auto streamInData = m_streamIn->GetStreamInBuffer();
        ENCODE_CHK_STATUS_RETURN(FillSegmentationMap((VdencStreamInState *)streamInData));

        ENCODE_CHK_STATUS_RETURN(m_streamIn->ReturnStreamInBuffer());

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1Segmentation::CheckSegmentationMap() const
    {
        ENCODE_CHK_NULL_RETURN(m_basicFeature->m_av1PicParams);

        auto CurFrameWidth  = m_basicFeature->m_av1PicParams->frame_width_minus1 + 1;
        auto CurFrameHeight = m_basicFeature->m_av1PicParams->frame_height_minus1 + 1;

        const uint32_t segMapPitch  = MOS_ALIGN_CEIL(CurFrameWidth, m_segmentMapBlockSize) / m_segmentMapBlockSize;
        const uint32_t segMapHeight = MOS_ALIGN_CEIL(CurFrameHeight, m_segmentMapBlockSize) / m_segmentMapBlockSize;

        const uint64_t minSegmentMapDataSize = (uint64_t)segMapPitch * (uint64_t)segMapHeight;  // 1 byte per segment Id

        if (m_segmentMapDataSize < minSegmentMapDataSize)
        {
            ENCODE_ASSERTMESSAGE("Size of segmentation map data provided by app isn't enough for frame resolution");
            return MOS_STATUS_INVALID_PARAMETER;
        }

        return MOS_STATUS_SUCCESS;
    }

    inline uint32_t ScaleCoord(uint32_t coord, uint32_t oldUnit, uint32_t newUnit)
    {
        ENCODE_ASSERT(newUnit);
        return (coord * oldUnit) / newUnit;
    }

    MOS_STATUS Av1Segmentation::FillSegmentationMap(VdencStreamInState* streamInData) const
    {
        ENCODE_FUNC_CALL();
        ENCODE_CHK_NULL_RETURN(streamInData);
        ENCODE_CHK_NULL_RETURN(m_basicFeature->m_av1PicParams);

        const uint8_t blockSize = Av1StreamIn::m_streamInBlockSize;
        auto CurFrameWidth  = m_basicFeature->m_av1PicParams->frame_width_minus1 + 1;
        auto CurFrameHeight = m_basicFeature->m_av1PicParams->frame_height_minus1 + 1;

        uint16_t FrameWidthInStreamInBlocks  = MOS_ALIGN_CEIL(CurFrameWidth, blockSize) / blockSize;
        uint16_t FrameHeightInStreamInBlocks = MOS_ALIGN_CEIL(CurFrameHeight, blockSize) / blockSize;

        for (int yIdx = 0; yIdx < FrameHeightInStreamInBlocks; yIdx++)
        {
            for (uint32_t xIdx = 0; xIdx < FrameWidthInStreamInBlocks; xIdx++)
            {
                const uint32_t IdxBlockInStreamIn = m_streamIn->GetCuOffset(xIdx, yIdx);
                const uint32_t segMapPitch = MOS_ALIGN_CEIL(CurFrameWidth, m_segmentMapBlockSize) / m_segmentMapBlockSize;
                const uint32_t segMapY = ScaleCoord(yIdx, blockSize, m_segmentMapBlockSize);
                const uint32_t segMapX = ScaleCoord(xIdx, blockSize, m_segmentMapBlockSize);

                ENCODE_ASSERT(m_pSegmentMap);

                const uint8_t segId = m_pSegmentMap[segMapY * segMapPitch + segMapX];

                streamInData[IdxBlockInStreamIn].DW7.SegIDEnable = 1;
                // Minimum size for a SegID is a 32x32 block.
                // All four 16x16 blocks within a 32x32 should share the same Segmentation ID.
                streamInData[IdxBlockInStreamIn].DW7.SegID = segId | (segId << 4) | (segId << 8) | (segId << 12);

            }
        }
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1Segmentation::AllocateSegmentationMapBuffer(uint8_t segmentBufid) 
    {
        if (segmentBufid >= av1TotalRefsPerFrame)
        {
            ENCODE_ASSERTMESSAGE("segment map number exceed.");
            return MOS_STATUS_INVALID_PARAMETER;
        }
        if (m_segmentMapBuffer[segmentBufid] != nullptr)
        {
            return MOS_STATUS_SUCCESS;
        }

        uint32_t       totalSbPerFrame         = (m_basicFeature->m_picWidthInSb) * (m_basicFeature->m_picHeightInSb);
        const uint16_t num4x4BlocksIn64x64Sb   = 256;
        const uint16_t num4x4BlocksIn128x128Sb = 1024;
        const uint32_t sizeOfSegmentIdMap      = ((m_basicFeature->m_isSb128x128) ? num4x4BlocksIn128x128Sb : num4x4BlocksIn64x64Sb) * totalSbPerFrame;

        MOS_ALLOC_GFXRES_PARAMS allocParams;
        MOS_ZeroMemory(&allocParams, sizeof(MOS_ALLOC_GFXRES_PARAMS));
        allocParams.Type               = MOS_GFXRES_BUFFER;
        allocParams.TileType           = MOS_TILE_LINEAR;
        allocParams.Format             = Format_Buffer;
        allocParams.Flags.bNotLockable = false;
        allocParams.dwBytes            = sizeOfSegmentIdMap;
        allocParams.pBufName           = "segmentIdStreamOutBuffer";
        allocParams.ResUsageType       = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_NOCACHE;
        m_segmentMapBuffer[segmentBufid] = m_allocator->AllocateResource(allocParams, false);

        ENCODE_CHK_NULL_RETURN(m_segmentMapBuffer[segmentBufid]);

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(VDENC_PIPE_BUF_ADDR_STATE, Av1Segmentation)
    {
        params.segmentMapStreamInBuffer  = nullptr;
        params.segmentMapStreamOutBuffer = nullptr;

        if (m_segmentParams.m_enabled && !m_segmentParams.m_updateMap)
        {
            // when segmentation_update_map is 0, send prime_ref_frame's AVP stream out buffer as VDEnc stream in
            const auto primRefList = m_basicFeature->m_ref.GetPrimaryRefList();
            ENCODE_CHK_NULL_RETURN(primRefList);

            params.segmentMapStreamInBuffer = m_segmentMapBuffer[primRefList->m_segIdBufIdx];
        }

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(AVP_PIC_STATE, Av1Segmentation)
    {
        MOS_SecureMemcpy(
            &params.segmentParams,
            sizeof(params.segmentParams),
            &m_segmentParams,
            sizeof(params.segmentParams));

        if (m_basicFeature->m_av1PicParams->PicFlags.fields.DisableFrameRecon)
        {
            params.segmentParams.m_segIdBufStreamOutEnable = false;
        }


        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(AVP_SEGMENT_STATE, Av1Segmentation)
    {
        if (m_segmentNum > av1MaxSegments)
        {
            ENCODE_ASSERTMESSAGE("The Segment number exceeds the max value.");
            return MOS_STATUS_USER_CONTROL_MAX_NAME_SIZE;
        }

        params.numSegments = m_segmentNum;

        MOS_SecureMemcpy(
            &params.av1SegmentParams,
            sizeof(params.av1SegmentParams),
            &m_segmentParams,
            sizeof(params.av1SegmentParams));

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(AVP_PIPE_BUF_ADDR_STATE, Av1Segmentation)
    {
        ENCODE_CHK_NULL_RETURN(m_basicFeature);

        if (m_segmentParams.m_segIdBufStreamInEnable)
        {
            const auto primRefList = m_basicFeature->m_ref.GetPrimaryRefList();
            ENCODE_CHK_NULL_RETURN(primRefList);
            ENCODE_CHK_NULL_RETURN(m_segmentMapBuffer[primRefList->m_segIdBufIdx]);
            params.segmentIdReadBuffer = m_segmentMapBuffer[primRefList->m_segIdBufIdx];
        }

        if (!m_basicFeature->m_av1PicParams->PicFlags.fields.DisableFrameRecon && m_segmentParams.m_segIdBufStreamOutEnable)
        {
            const auto currRefList = m_basicFeature->m_ref.GetCurrRefList();
            ENCODE_CHK_NULL_RETURN(currRefList);
            ENCODE_CHK_NULL_RETURN(m_segmentMapBuffer[currRefList->m_segIdBufIdx]);
            params.segmentIdWriteBuffer = m_segmentMapBuffer[currRefList->m_segIdBufIdx];
        }

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(VDENC_CMD2, Av1Segmentation)
    {
        const auto* picPar = m_basicFeature->m_av1PicParams;
        ENCODE_CHK_NULL_RETURN(picPar);

        params.segmentation = m_segmentParams.m_enabled;

        if (params.segmentation)
        {
            // for VP9 VDEnc this is bit used for programming of "segmentation_temporal_update"
            // for AV1 VDEnc this bit indicates negative of "segmentation_update_map"
            params.segmentationTemporal =  m_segmentParams.m_updateMap ? false : true;
#if _MEDIA_RESERVED
            params.vdencCmd2Par113 = true;
#else
            params.extSettings.emplace_back(
                [this](uint32_t *data) {
                    data[54] |= 1 << 15;
                    return MOS_STATUS_SUCCESS;
                });
#endif  // _MEDIA_RESERVED
        }

        for (auto i = 0; i < av1MaxSegments; i++)
        {
            if (i < m_segmentNum &&
                m_segmentParams.m_enabled &&
                (m_segmentParams.m_featureMask[i] & 0x1 /*SEG_LVL_ALT_Q*/))
            {
                uint16_t tempSegQp = picPar->base_qindex + m_segmentParams.m_featureData[i][0];
                params.qpForSegs[i] = static_cast<uint8_t> (CodecHal_Clip3(1, 255, tempSegQp));

#if _MEDIA_RESERVED
                params.vdencCmd2Par99 = 1;
#else
                params.extSettings.emplace_back(
                    [this](uint32_t *data) {
                        data[54] |= 1;
                        return MOS_STATUS_SUCCESS;
                    });
#endif  // _MEDIA_RESERVED
            }
            else
            {
                params.qpForSegs[i] = static_cast<uint8_t>(picPar->base_qindex);
            }
        }

        return MOS_STATUS_SUCCESS;
    }

}  // namespace encode
