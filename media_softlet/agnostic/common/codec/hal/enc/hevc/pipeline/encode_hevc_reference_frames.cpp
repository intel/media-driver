/*
* Copyright (c) 2018, Intel Corporation
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
//! \file     encode_hevc_reference_frames.cpp
//! \brief    Defines reference list related logic for encode hevc
//!

#include "encode_hevc_basic_feature.h"
#include "encode_utils.h"
#include "encode_hevc_reference_frames.h"
#include "codec_def_encode_hevc.h"
#include "encode_hevc_dfs.h"

namespace encode
{
MOS_STATUS HevcReferenceFrames::Init(HevcBasicFeature *basicFeature, EncodeAllocator *allocator)
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_NULL_RETURN(basicFeature);

    m_basicFeature = basicFeature;
    m_allocator = allocator;
    ENCODE_CHK_NULL_RETURN(m_allocator);
    ENCODE_CHK_STATUS_RETURN(EncodeAllocateDataList(
        m_refList,
        CODECHAL_NUM_UNCOMPRESSED_SURFACE_HEVC));

    return MOS_STATUS_SUCCESS;

}
HevcReferenceFrames::~HevcReferenceFrames()
{
    ENCODE_FUNC_CALL();

    EncodeFreeDataList(m_refList, CODECHAL_NUM_UNCOMPRESSED_SURFACE_HEVC);

}

MOS_STATUS HevcReferenceFrames::UpdatePicture()
{
    ENCODE_FUNC_CALL();

    auto picParams = m_basicFeature->m_hevcPicParams;
    ENCODE_CHK_NULL_RETURN(picParams);

    PCODEC_REF_LIST *refListFull = &m_refList[0];
    m_currRefIdx  = picParams->CurrReconstructedPic.FrameIdx;
    m_currRefList = refListFull[m_currRefIdx];

    // P/B frames with empty ref lists are internally encoded as I frames,
    // while picture header packing remains the original value
    m_pictureCodingType = picParams->CodingType;

    bool emptyRefFrmList = true;
    for (auto i = 0; i < CODEC_MAX_NUM_REF_FRAME_HEVC; i++)
    {
        if (picParams->RefFrameList[i].PicFlags != PICTURE_INVALID)
        {
            emptyRefFrmList = false;
            break;
        }
    }

    if (emptyRefFrmList)
    {
        // If there is no reference frame in the list, just mark the current picture as the I type
        m_pictureCodingType = I_TYPE;
    }

    for (auto i = 0; i < CODEC_MAX_NUM_REF_FRAME_HEVC; i++)
    {
        m_picIdx[i].bValid = false;
        if (picParams->RefFrameList[i].PicFlags != PICTURE_INVALID)
        {
            uint8_t index         = picParams->RefFrameList[i].FrameIdx;
            bool duplicatedIdx = false;
            for (auto ii = 0; ii < i; ii++)
            {
                if (m_picIdx[ii].bValid && index == picParams->RefFrameList[ii].FrameIdx)
                {
                    // We find the same FrameIdx in the ref_frame_list. Multiple reference frames are the same.
                    // In other words, RefFrameList[i] and RefFrameList[ii] have the same surface Id
                    duplicatedIdx = true;
                    break;
                }
            }

            if (duplicatedIdx)
            {
                continue;
            }

            // this reference frame in unique. Save it into the full reference list with 127 items
            refListFull[index]->RefPic.PicFlags =
                CodecHal_CombinePictureFlags(refListFull[index]->RefPic, picParams->RefFrameList[i]);
            refListFull[index]->iFieldOrderCnt[0] = picParams->RefFramePOCList[i];
            refListFull[index]->iFieldOrderCnt[1] = picParams->RefFramePOCList[i];
            refListFull[index]->sRefBuffer        = picParams->bUseRawPicForRef ? refListFull[index]->sRefRawBuffer : refListFull[index]->sRefReconBuffer;

            m_picIdx[i].bValid   = true;
            m_picIdx[i].ucPicIdx = index;
        }
    }

    // Save the current RefList
    uint8_t ii = 0;
    for (auto i = 0; i < CODEC_MAX_NUM_REF_FRAME_HEVC; i++)
    {
        if (m_picIdx[i].bValid)
        {
            refListFull[m_currRefIdx]->RefList[ii] = picParams->RefFrameList[i];
            ii++;
        }
    }

    m_currRefList->RefPic = picParams->CurrOriginalPic;
    m_currRefList->bUsedAsRef = picParams->bUsedAsRef;
    m_currRefList->bFormatConversionDone = false;
    m_currRefList->ucNumRef = ii;
    m_currRefList->iFieldOrderCnt[0] = picParams->CurrPicOrderCnt;
    m_currRefList->iFieldOrderCnt[1] = picParams->CurrPicOrderCnt;
    m_currRefList->sRefReconBuffer = m_basicFeature->m_reconSurface;
    m_currRefList->sRefRawBuffer = m_basicFeature->m_rawSurface;
    m_currRefList->resBitstreamBuffer = m_basicFeature->m_resBitstreamBuffer;

    return MOS_STATUS_SUCCESS;

}

MOS_STATUS HevcReferenceFrames::UpdateSlice()
{
    ENCODE_FUNC_CALL();
    auto picParams = m_basicFeature->m_hevcPicParams;
    ENCODE_CHK_NULL_RETURN(picParams);
    auto sliceParams = m_basicFeature->m_hevcSliceParams;
    ENCODE_CHK_NULL_RETURN(sliceParams);

    m_lowDelay               = true;
    m_sameRefList            = true;

    for (auto i = 0; i < CODEC_MAX_NUM_REF_FRAME_HEVC; i++)
    {
        m_refIdxMapping[i]  = -1;
        m_currUsedRefPic[i] = false;
    }

    // To obtain current "used" reference frames. The number of current used reference frames cannot be greater than 8
    auto slcParams = sliceParams;
    for (uint32_t s = 0; s < m_basicFeature->m_numSlices; s++, slcParams++)
    {
        for (uint8_t ll = 0; ll < 2; ll++)
        {
            uint32_t numRef = (ll == 0) ? slcParams->num_ref_idx_l0_active_minus1 :
                slcParams->num_ref_idx_l1_active_minus1;

            for (uint32_t i = 0; i <= numRef; i++)
            {
                CODEC_PICTURE refPic = slcParams->RefPicList[ll][i];
                if (!CodecHal_PictureIsInvalid(refPic) &&
                    !CodecHal_PictureIsInvalid(picParams->RefFrameList[refPic.FrameIdx]))
                {
                    m_currUsedRefPic[refPic.FrameIdx] = true;
                }
            }
        }
    }

    for (uint8_t i = 0, refIdx = 0; i < CODEC_MAX_NUM_REF_FRAME_HEVC; i++)
    {
        if (!m_currUsedRefPic[i])
        {
            continue;
        }

        uint8_t index         = picParams->RefFrameList[i].FrameIdx;
        bool duplicatedIdx = false;
        for (uint8_t ii = 0; ii < i; ii++)
        {
            if (m_currUsedRefPic[i] && index == picParams->RefFrameList[ii].FrameIdx)
            {
                // We find the same FrameIdx in the ref_frame_list. Multiple reference frames are the same.
                // In other words, RefFrameList[i] and RefFrameList[ii] have the same surface Id
                duplicatedIdx = true;
                m_refIdxMapping[i] = m_refIdxMapping[ii];
                break;
            }
        }

        if (duplicatedIdx)
        {
            continue;
        }

        if (refIdx >= CODECHAL_MAX_CUR_NUM_REF_FRAME_HEVC)
        {
            // Total number of distingushing reference frames cannot be geater than 8.
            ENCODE_ASSERT(false);
            return MOS_STATUS_INVALID_PARAMETER;
        }

        // Map reference frame index [0-15] into a set of unique IDs within [0-7]
        m_refIdxMapping[i] = refIdx;
        refIdx++;
    }

    if (m_pictureCodingType != I_TYPE && picParams->CollocatedRefPicIndex != 0xFF && picParams->CollocatedRefPicIndex < CODEC_MAX_NUM_REF_FRAME_HEVC)
    {
        uint8_t frameStoreId = (uint8_t)m_refIdxMapping[picParams->CollocatedRefPicIndex];

        if (frameStoreId >= CODECHAL_MAX_CUR_NUM_REF_FRAME_HEVC || !m_currUsedRefPic[picParams->CollocatedRefPicIndex])
        {
            // CollocatedRefPicIndex is wrong in this case for the reference frame is not used be used
            return MOS_STATUS_INVALID_PARAMETER;
        }
    }

    m_currRefList->ucQPValue[0] = picParams->QpY + sliceParams->slice_qp_delta;

    m_idxForTempMVP = 0xFF;

    if (picParams->CollocatedRefPicIndex != 0xFF && picParams->CollocatedRefPicIndex < CODEC_MAX_NUM_REF_FRAME_HEVC)
    {
        uint8_t frameIdx = picParams->RefFrameList[picParams->CollocatedRefPicIndex].FrameIdx;
        m_idxForTempMVP = m_refList[frameIdx]->ucScalingIdx;
    }

    if (m_pictureCodingType == I_TYPE)
    {
        m_currGopIFramePOC = picParams->CurrPicOrderCnt;
    }

    auto seqParams = m_basicFeature->m_hevcSeqParams;
    ENCODE_CHK_NULL_RETURN(seqParams);

    slcParams = sliceParams;
    for (uint32_t s = 0; s < m_basicFeature->m_numSlices; s++, slcParams++)
    {
        ENCODE_CHK_STATUS_RETURN(ValidateLowDelayBFrame(slcParams));
        ENCODE_CHK_STATUS_RETURN(ValidateSameRefInL0L1(slcParams));

        if (m_idxForTempMVP == 0xFF && slcParams->slice_temporal_mvp_enable_flag)
        {
            // Temporal reference MV index is invalid and so disable the temporal MVP
            slcParams->slice_temporal_mvp_enable_flag = false;
        }

        if (seqParams->sps_temporal_mvp_enable_flag == 0 && slcParams->slice_temporal_mvp_enable_flag == 1)
        {
            ENCODE_NORMALMESSAGE("Attention: temporal MVP flag is inconsistent between seq and slice.");
            slcParams->slice_temporal_mvp_enable_flag = 0;
        }

        if (!picParams->pps_curr_pic_ref_enabled_flag && m_lowDelay && slcParams->num_ref_idx_l0_active_minus1 == 0
            && m_currGopIFramePOC != -1 && slcParams->slice_temporal_mvp_enable_flag != 0)
        {
            ENCODE_CHK_STATUS_RETURN(ValidateTmvp(slcParams));
        }

        if (m_lowDelay && !m_sameRefList)
        {
            ENCODE_NORMALMESSAGE("Attention: LDB frame but with different L0/L1 list !");
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcReferenceFrames::ValidateTmvp(PCODEC_HEVC_ENCODE_SLICE_PARAMS slcParams)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    ENCODE_FUNC_CALL();

    ENCODE_CHK_NULL_RETURN(slcParams);

    auto idx = m_picIdx[slcParams->RefPicList[0][0].FrameIdx].ucPicIdx;

    if (m_refList[idx]->iFieldOrderCnt[0] == m_currGopIFramePOC)
    {
        slcParams->slice_temporal_mvp_enable_flag = 0;
    }

    return eStatus;
}

MOS_STATUS HevcReferenceFrames::ValidateSameRefInL0L1(PCODEC_HEVC_ENCODE_SLICE_PARAMS slcParams)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    ENCODE_FUNC_CALL();

    ENCODE_CHK_NULL_RETURN(slcParams);

    if (m_sameRefList && slcParams->num_ref_idx_l0_active_minus1 >= slcParams->num_ref_idx_l1_active_minus1)
    {
        for (int refIdx = 0; refIdx < slcParams->num_ref_idx_l1_active_minus1 + 1; refIdx++)
        {
            CODEC_PICTURE refPicL0 = slcParams->RefPicList[0][refIdx];
            CODEC_PICTURE refPicL1 = slcParams->RefPicList[1][refIdx];

            if (!CodecHal_PictureIsInvalid(refPicL0) && !CodecHal_PictureIsInvalid(refPicL1) && refPicL0.FrameIdx != refPicL1.FrameIdx)
            {
                m_sameRefList = false;
                break;
            }
        }
    }
    
    return eStatus;
}

MOS_STATUS HevcReferenceFrames::ValidateLowDelayBFrame(
    PCODEC_HEVC_ENCODE_SLICE_PARAMS slcParams)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    ENCODE_FUNC_CALL();

    auto picParams = m_basicFeature->m_hevcPicParams;
    ENCODE_CHK_NULL_RETURN(picParams);
    ENCODE_CHK_NULL_RETURN(slcParams);

    if (slcParams->slice_type == encodeHevcPSlice)
    {
        m_lowDelay = true;
        // P slice only has forward l0
        for (int refIdx = 0; (refIdx < slcParams->num_ref_idx_l0_active_minus1 + 1) && m_lowDelay; refIdx++)
        {
            if (refIdx >= CODEC_MAX_NUM_REF_FRAME_HEVC)
            {
                break;
            }

            CODEC_PICTURE refPic = slcParams->RefPicList[0][refIdx];
            if (!CodecHal_PictureIsInvalid(refPic) && picParams->RefFramePOCList[refPic.FrameIdx] > picParams->CurrPicOrderCnt)
            {
                m_lowDelay = false;
            }
        }
    }

    // Examine if now it is B type
    if (slcParams->slice_type == encodeHevcBSlice)
    {
        // forward
        for (int refIdx = 0; (refIdx < slcParams->num_ref_idx_l0_active_minus1 + 1) && m_lowDelay; refIdx++)
        {
            if (refIdx >= CODEC_MAX_NUM_REF_FRAME_HEVC)
            {
                break;
            }

            CODEC_PICTURE  refPic = slcParams->RefPicList[0][refIdx];
            if (!CodecHal_PictureIsInvalid(refPic) && picParams->RefFramePOCList[refPic.FrameIdx] > picParams->CurrPicOrderCnt)
            {
                m_lowDelay = false;
            }
        }

        // backward
        for (int refIdx = 0; (refIdx < slcParams->num_ref_idx_l1_active_minus1 + 1) && m_lowDelay; refIdx++)
        {
            if (refIdx >= CODEC_MAX_NUM_REF_FRAME_HEVC)
            {
                break;
            }

            CODEC_PICTURE refPic = slcParams->RefPicList[1][refIdx];
            if (!CodecHal_PictureIsInvalid(refPic) && picParams->RefFramePOCList[refPic.FrameIdx] > picParams->CurrPicOrderCnt)
            {
                m_lowDelay = false;
            }
        }
    }

    return eStatus;
}

bool HevcReferenceFrames::IsCurrentUsedAsRef(uint8_t idx)
{
    ENCODE_FUNC_CALL();

    if (idx < CODEC_MAX_NUM_REF_FRAME_HEVC && 
        m_picIdx[idx].bValid && m_currUsedRefPic[idx])
    {
        return true;
    }
    else
    {
        return false;
    }
}

MOS_STATUS HevcReferenceFrames::UpdateRollingIReferenceLocation()
{
    ENCODE_FUNC_CALL();
    auto picParams = m_basicFeature->m_hevcPicParams;
    ENCODE_CHK_NULL_RETURN(picParams);
    auto sliceParams = m_basicFeature->m_hevcSliceParams;
    ENCODE_CHK_NULL_RETURN(sliceParams);
    uint32_t rollingILimit = (picParams->bEnableRollingIntraRefresh == ROLLING_I_ROW) ? MOS_ROUNDUP_DIVIDE(m_basicFeature->m_frameHeight, 32) : 
                             (picParams->bEnableRollingIntraRefresh == ROLLING_I_COLUMN) ? MOS_ROUNDUP_DIVIDE(m_basicFeature->m_frameWidth, 32) : 0;

    m_refList[m_currRefIdx]->rollingIntraRefreshedPosition =
        CodecHal_Clip3(0, rollingILimit, picParams->IntraInsertionLocation + picParams->IntraInsertionSize);

    // Update pic params rolling intra reference location here before cmd buffer is prepared.
    PCODEC_PICTURE l0RefFrameList = sliceParams->RefPicList[LIST_0];
    for (uint8_t refIdx = 0; refIdx <= sliceParams->num_ref_idx_l0_active_minus1; refIdx++)
    {
        CODEC_PICTURE refPic = l0RefFrameList[refIdx];

        if (!CodecHal_PictureIsInvalid(refPic) && m_picIdx[refPic.FrameIdx].bValid)
        {
            uint8_t refPicIdx = m_picIdx[refPic.FrameIdx].ucPicIdx;
            picParams->RollingIntraReferenceLocation[refIdx] = m_refList[refPicIdx]->rollingIntraRefreshedPosition;
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcReferenceFrames::SetSlotForRecNotFiltered(unsigned char &slotForRecNotFiltered)
{
    ENCODE_FUNC_CALL();
    auto picParams = m_basicFeature->m_hevcPicParams;
    ENCODE_CHK_NULL_RETURN(picParams);

    MHW_VDBOX_PIPE_BUF_ADDR_PARAMS pipeBufAddrParams;

    if (m_pictureCodingType != I_TYPE)
    {
        for (uint8_t i = 0; i < CODEC_MAX_NUM_REF_FRAME_HEVC; i++)
        {
            if (IsCurrentUsedAsRef(i))
            {
                uint8_t idx = m_picIdx[i].ucPicIdx;

                uint8_t frameStoreId                            = m_refIdxMapping[i];
                pipeBufAddrParams.presReferences[frameStoreId] = &(m_refList[idx]->sRefReconBuffer.OsResource);
            }
        }
    }

    if (picParams->pps_curr_pic_ref_enabled_flag)
    {
        // I frame is much simpler
        if (m_pictureCodingType == I_TYPE)
        {
            slotForRecNotFiltered = 0;
        }
        // B frame
        else
        {
            unsigned int i;

            for (i = 0; i < CODECHAL_MAX_CUR_NUM_REF_FRAME_HEVC; i++)
            {
                if (pipeBufAddrParams.presReferences[i] == nullptr)
                {
                    break;
                }
            }

            if (i >= CODECHAL_MAX_CUR_NUM_REF_FRAME_HEVC)
            {
                slotForRecNotFiltered = CODECHAL_MAX_CUR_NUM_REF_FRAME_HEVC;
                ENCODE_ASSERTMESSAGE("Find no available slot.");
                return MOS_STATUS_INVALID_PARAMETER;
            }

            slotForRecNotFiltered = (unsigned char)i;
        }
    }
    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(VDENC_PIPE_BUF_ADDR_STATE, HevcReferenceFrames)
{
    ENCODE_FUNC_CALL();

    ENCODE_CHK_NULL_RETURN(m_basicFeature);
    auto trackedBuf = m_basicFeature->m_trackedBuf;
    ENCODE_CHK_NULL_RETURN(trackedBuf);

    auto picParams = m_basicFeature->m_hevcPicParams;
    ENCODE_CHK_NULL_RETURN(picParams);

    auto sliceParams = m_basicFeature->m_hevcSliceParams;
    ENCODE_CHK_NULL_RETURN(sliceParams);

    auto l0RefFrameList = sliceParams->RefPicList[LIST_0];
    for (uint8_t refIdx = 0; refIdx <= sliceParams->num_ref_idx_l0_active_minus1; refIdx++)
    {
        CODEC_PICTURE refPic = l0RefFrameList[refIdx];

        if (!CodecHal_PictureIsInvalid(refPic) && m_picIdx[refPic.FrameIdx].bValid)
        {
            // L0 references
            uint8_t refPicIdx    = m_picIdx[refPic.FrameIdx].ucPicIdx;

            params.refs[refIdx] = (picParams->bUseRawPicForRef) ? 
                &m_refList[refPicIdx]->sRefBuffer.OsResource : &m_refList[refPicIdx]->sRefReconBuffer.OsResource;

            // 4x/8x DS surface for VDEnc
            uint8_t scaledIdx        = m_refList[refPicIdx]->ucScalingIdx;
            auto    vdenc4xDsSurface = trackedBuf->GetSurface(BufferType::ds4xSurface, scaledIdx);
            ENCODE_CHK_NULL_RETURN(vdenc4xDsSurface);
            auto vdenc8xDsSurface = trackedBuf->GetSurface(BufferType::ds8xSurface, scaledIdx);
            ENCODE_CHK_NULL_RETURN(vdenc8xDsSurface);
            params.refsDsStage2[refIdx] = &vdenc4xDsSurface->OsResource;
            params.refsDsStage1[refIdx] = &vdenc8xDsSurface->OsResource;

            if (sliceParams->slice_type == encodeHevcPSlice)
            {
                params.refs[refIdx + sliceParams->num_ref_idx_l0_active_minus1 + 1] = &m_refList[refPicIdx]->sRefReconBuffer.OsResource;
                params.refsDsStage2[refIdx + sliceParams->num_ref_idx_l0_active_minus1 + 1] =
                    &vdenc4xDsSurface->OsResource;
                params.refsDsStage1[refIdx + sliceParams->num_ref_idx_l0_active_minus1 + 1] =
                    &vdenc8xDsSurface->OsResource;
            }
        }
    }

    const CODEC_PICTURE *l1RefFrameList = sliceParams->RefPicList[LIST_1];
    for (uint8_t refIdx = 0; refIdx <= sliceParams->num_ref_idx_l1_active_minus1; refIdx++)
    {
        CODEC_PICTURE refPic = l1RefFrameList[refIdx];

        if (!CodecHal_PictureIsInvalid(refPic) && m_picIdx[refPic.FrameIdx].bValid && encodeHevcPSlice != sliceParams->slice_type)
        {
            // L1 references
            uint8_t refPicIdx = m_picIdx[refPic.FrameIdx].ucPicIdx;

            params.refs[refIdx + sliceParams->num_ref_idx_l0_active_minus1 + 1] = (picParams->bUseRawPicForRef) ? 
                &m_refList[refPicIdx]->sRefBuffer.OsResource : &m_refList[refPicIdx]->sRefReconBuffer.OsResource;

            // 4x/8x DS surface for VDEnc
            uint8_t scaledIdx        = m_refList[refPicIdx]->ucScalingIdx;
            auto    vdenc4xDsSurface = trackedBuf->GetSurface(BufferType::ds4xSurface, scaledIdx);
            ENCODE_CHK_NULL_RETURN(vdenc4xDsSurface);
            auto vdenc8xDsSurface = trackedBuf->GetSurface(BufferType::ds8xSurface, scaledIdx);
            ENCODE_CHK_NULL_RETURN(vdenc8xDsSurface);
            params.refsDsStage2[refIdx + sliceParams->num_ref_idx_l0_active_minus1 + 1] =
                &vdenc4xDsSurface->OsResource;
            params.refsDsStage1[refIdx + sliceParams->num_ref_idx_l0_active_minus1 + 1] =
                &vdenc8xDsSurface->OsResource;
        }
    }

    auto mvTmpBuffer = trackedBuf->GetBuffer(BufferType::mvTemporalBuffer, m_idxForTempMVP);
    params.colMvTempBuffer[0] = mvTmpBuffer;

    // When partial frame update is enabled, reuse the 4x/8x downscaled surfaces
    if (false)  //m_enablePartialFrameUpdate
    {
        CODEC_PICTURE refPic = sliceParams->RefPicList[LIST_0][0];

        if (!CodecHal_PictureIsInvalid(refPic) && m_picIdx[refPic.FrameIdx].bValid)
        {
            // L0 references
            uint8_t refPicIdx = m_picIdx[refPic.FrameIdx].ucPicIdx;
            // 4x/8x DS surface for VDEnc
            uint8_t scaledIdx        = m_refList[refPicIdx]->ucScalingIdx;
            auto    vdenc4xDsSurface = trackedBuf->GetSurface(BufferType::ds4xSurface, scaledIdx);
            ENCODE_CHK_NULL_RETURN(vdenc4xDsSurface);
            auto vdenc8xDsSurface = trackedBuf->GetSurface(BufferType::ds8xSurface, scaledIdx);
            ENCODE_CHK_NULL_RETURN(vdenc8xDsSurface);
            params.surfaceDsStage2 = vdenc4xDsSurface;
            params.surfaceDsStage1 = vdenc8xDsSurface;
        }
    }

    params.lowDelayB = m_lowDelay;

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(HCP_PIPE_BUF_ADDR_STATE, HevcReferenceFrames)
{
    ENCODE_FUNC_CALL();

    ENCODE_CHK_NULL_RETURN(m_basicFeature);
    auto trackedBuf = m_basicFeature->m_trackedBuf;
    ENCODE_CHK_NULL_RETURN(trackedBuf);
    auto picParams = m_basicFeature->m_hevcPicParams;
    ENCODE_CHK_NULL_RETURN(picParams);

    //add for B frame support
    if (m_pictureCodingType != I_TYPE)
    {
        for (uint8_t i = 0; i < CODEC_MAX_NUM_REF_FRAME_HEVC; i++)
        {
            if (i < CODEC_MAX_NUM_REF_FRAME_HEVC &&
                m_picIdx[i].bValid && m_currUsedRefPic[i])
            {
                uint8_t idx = m_picIdx[i].ucPicIdx;
                //CodecHalGetResourceInfo(m_osInterface, &(m_refList[idx]->sRefReconBuffer));

                uint8_t frameStoreId                            = m_refIdxMapping[i];
                params.presReferences[frameStoreId] = (picParams->bUseRawPicForRef) ? 
                    &(m_refList[idx]->sRefBuffer.OsResource) : &(m_refList[idx]->sRefReconBuffer.OsResource);

                uint8_t refMbCodeIdx = m_refList[idx]->ucScalingIdx;
                auto    mvTmpBuffer  = trackedBuf->GetBuffer(BufferType::mvTemporalBuffer, refMbCodeIdx);
                ENCODE_CHK_NULL_RETURN(mvTmpBuffer);
                params.presColMvTempBuffer[frameStoreId] = mvTmpBuffer;
            }
        }
    }

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(HCP_SURFACE_STATE, HevcReferenceFrames)
{
    ENCODE_FUNC_CALL();

#ifdef _MMC_SUPPORTED
    ENCODE_CHK_NULL_RETURN(m_mmcState);
    if (m_mmcState->IsMmcEnabled())
    {
        ENCODE_CHK_NULL_RETURN(m_basicFeature);
        auto trackedBuf = m_basicFeature->m_trackedBuf;
        ENCODE_CHK_NULL_RETURN(trackedBuf);

        params.refsMmcEnable       = 0;
        params.refsMmcType         = 0;
        params.dwCompressionFormat = 0;

        //add for B frame support
        if (m_pictureCodingType != I_TYPE)
        {
            for (uint8_t i = 0; i < CODEC_MAX_NUM_REF_FRAME_HEVC; i++)
            {
                if (i < CODEC_MAX_NUM_REF_FRAME_HEVC &&
                    m_picIdx[i].bValid && m_currUsedRefPic[i])
                {
                    uint8_t idx          = m_picIdx[i].ucPicIdx;
                    uint8_t frameStoreId = m_refIdxMapping[i];

                    MOS_MEMCOMP_STATE mmcState  = MOS_MEMCOMP_DISABLED;
                    ENCODE_CHK_STATUS_RETURN(m_mmcState->GetSurfaceMmcState(const_cast<PMOS_SURFACE>(&m_refList[idx]->sRefReconBuffer), &mmcState));
                    params.refsMmcEnable |= (mmcState == MOS_MEMCOMP_RC || mmcState == MOS_MEMCOMP_MC) ? (1 << frameStoreId) : 0;
                    params.refsMmcType |= (mmcState == MOS_MEMCOMP_RC) ? (1 << frameStoreId) : 0;
                    if (mmcState == MOS_MEMCOMP_RC || mmcState == MOS_MEMCOMP_MC)
                    {
                        ENCODE_CHK_STATUS_RETURN(m_mmcState->GetSurfaceMmcFormat(const_cast<PMOS_SURFACE>(&m_refList[idx]->sRefReconBuffer), &params.dwCompressionFormat));
                    }
                }
            }
        }
    }
#endif

    return MOS_STATUS_SUCCESS;
}

}  // namespace encode
