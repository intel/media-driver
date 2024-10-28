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
//! \file     encode_avc_reference_frames.cpp
//! \brief    Defines reference list related logic for encode avc
//!

#include "encode_avc_reference_frames.h"
#include "encode_avc_basic_feature.h"

namespace encode
{

MOS_STATUS AvcReferenceFrames::Init(AvcBasicFeature *basicFeature, EncodeAllocator *allocator)
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_NULL_RETURN(basicFeature);

    m_basicFeature = basicFeature;
    m_allocator    = allocator;
    ENCODE_CHK_NULL_RETURN(m_allocator);
    ENCODE_CHK_STATUS_RETURN(EncodeAllocateDataList(
        m_refList,
        CODEC_AVC_NUM_UNCOMPRESSED_SURFACE));

    return MOS_STATUS_SUCCESS;
}

AvcReferenceFrames::~AvcReferenceFrames()
{
    ENCODE_FUNC_CALL();

    EncodeFreeDataList(m_refList, CODEC_AVC_NUM_UNCOMPRESSED_SURFACE);
}

MOS_STATUS AvcReferenceFrames::UpdatePicture()
{
    ENCODE_FUNC_CALL();

    auto picParams = m_basicFeature->m_picParam;
    auto avcRefList = &m_refList[0];
    auto avcPicIdx  = &m_picIdx[0];

    auto prevPic    = m_basicFeature->m_currOriginalPic;
    auto prevIdx    = prevPic.FrameIdx;
    auto currPic    = picParams->CurrOriginalPic;
    auto currIdx    = currPic.FrameIdx;

    uint8_t prevRefIdx = m_basicFeature->m_currReconstructedPic.FrameIdx;
    uint8_t currRefIdx = picParams->CurrReconstructedPic.FrameIdx;

    avcRefList[currRefIdx]->sRefReconBuffer = m_basicFeature->m_reconSurface;
    avcRefList[currRefIdx]->sRefRawBuffer   = m_basicFeature->m_rawSurface;
    avcRefList[currRefIdx]->sFrameNumber    = picParams->frame_num;
    avcRefList[currRefIdx]->RefPic          = picParams->CurrOriginalPic;

    // P/B frames with empty ref lists are internally encoded as I frames,
    // while picture header packing remains the original value
    m_pictureCodingType = picParams->CodingType;

    bool emptyRefFrmList = true;
    for (uint8_t i = 0; i < CODEC_AVC_MAX_NUM_REF_FRAME; i++)
    {
        if (picParams->RefFrameList[i].PicFlags != PICTURE_INVALID)
        {
            emptyRefFrmList = false;
            break;
        }
    }

    if (emptyRefFrmList && m_pictureCodingType != I_TYPE)
    {
        m_pictureCodingType = I_TYPE;
    }

    avcRefList[currRefIdx]->bUsedAsRef         = picParams->RefPicFlag;
    avcRefList[currRefIdx]->resBitstreamBuffer = m_basicFeature->m_resBitstreamBuffer;

    for (uint8_t i = 0; i < 16; i++)
    {
        m_frameStoreID[i].inUse = false;
    }

    for (uint8_t i = 0; i < 16; i++)
    {
        avcPicIdx[i].bValid = false;
        if (picParams->RefFrameList[i].PicFlags != PICTURE_INVALID)
        {
            auto    index         = picParams->RefFrameList[i].FrameIdx;
            uint8_t duplicatedIdx = 0;
            for (uint8_t ii = 0; ii < i; ii++)
            {
                if (avcPicIdx[ii].bValid && index == picParams->RefFrameList[ii].FrameIdx)
                {
                    duplicatedIdx = 1;
                    break;
                }
            }
            if (duplicatedIdx)
            {
                continue;
            }

            avcRefList[index]->RefPic.PicFlags =
                CodecHal_CombinePictureFlags(avcRefList[index]->RefPic, picParams->RefFrameList[i]);
            avcRefList[index]->iFieldOrderCnt[0] = picParams->FieldOrderCntList[i][0];
            avcRefList[index]->iFieldOrderCnt[1] = picParams->FieldOrderCntList[i][1];
            avcPicIdx[i].bValid                  = true;
            avcPicIdx[i].ucPicIdx                = index;
            if (prevPic.PicFlags != PICTURE_INVALID)
            {
                uint8_t ii;
                for (ii = 0; ii < avcRefList[prevRefIdx]->ucNumRef; ii++)
                {
                    if (index == avcRefList[prevRefIdx]->RefList[ii].FrameIdx)
                    {
                        if (avcRefList[index]->ucFrameId == 0x1f)
                        {
                            // Should never happen, something must be wrong
                            ENCODE_ASSERT(false);
                            avcRefList[index]->ucFrameId = 0;
                        }
                        m_frameStoreID[avcRefList[index]->ucFrameId].inUse = true;
                        break;
                    }
                }
                if (ii == avcRefList[prevRefIdx]->ucNumRef)
                {
                    avcRefList[index]->ucFrameId = 0x1f;
                }
            }
        }
    }

    // Save the current RefList
    uint8_t ii = 0;
    for (uint8_t i = 0; i < 16; i++)
    {
        if (avcPicIdx[i].bValid)
        {
            avcRefList[currRefIdx]->RefList[ii] = picParams->RefFrameList[i];
            ii++;
        }
    }
    avcRefList[currRefIdx]->ucNumRef = ii;
    m_currRefList                    = avcRefList[currRefIdx];

    ENCODE_CHK_STATUS_RETURN(SetFrameStoreIds(currRefIdx));

    avcRefList[currRefIdx]->iFieldOrderCnt[0]     = picParams->CurrFieldOrderCnt[0];
    avcRefList[currRefIdx]->iFieldOrderCnt[1]     = picParams->CurrFieldOrderCnt[1];

    m_refList[currRefIdx]->ucAvcPictureCodingType =
        CodecHal_PictureIsFrame(picParams->CurrOriginalPic) ?
        CODEC_AVC_PIC_CODING_TYPE_FRAME :
        ((picParams->CurrFieldOrderCnt[0] < picParams->CurrFieldOrderCnt[1]) ?
            CODEC_AVC_PIC_CODING_TYPE_TFF_FIELD :
            CODEC_AVC_PIC_CODING_TYPE_BFF_FIELD);

    if (m_pictureCodingType == B_TYPE)
    {
        GetDistScaleFactor();
        m_biWeight = GetBiWeight(
            m_distScaleFactorList0[0],
            picParams->weighted_bipred_idc);
    }

    avcRefList[currRefIdx]->pRefPicSelectListEntry = nullptr;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS AvcReferenceFrames::UpdateSlice()
{
    ENCODE_FUNC_CALL();

    auto slcParams = m_basicFeature->m_sliceParams;
    auto seqParams = m_basicFeature->m_seqParam;
    auto picParams = m_basicFeature->m_picParam;

    if (m_pictureCodingType != I_TYPE)
    {
        CODECHAL_ENCODE_AVC_VALIDATE_NUM_REFS_PARAMS validateNumRefsParams;
        validateNumRefsParams.pSeqParams            = seqParams;
        validateNumRefsParams.pPicParams            = picParams;
        validateNumRefsParams.pAvcSliceParams       = slcParams;
        validateNumRefsParams.wPictureCodingType    = m_pictureCodingType;
        validateNumRefsParams.wPicHeightInMB        = m_basicFeature->m_picHeightInMb;
        validateNumRefsParams.wFrameFieldHeightInMB = m_basicFeature->m_frameFieldHeightInMb;
        validateNumRefsParams.bVDEncEnabled         = true;

        ENCODE_CHK_STATUS_RETURN(ValidateNumReferences(&validateNumRefsParams));
    }
    else
    {
        slcParams->num_ref_idx_l0_active_minus1 = 0;
        slcParams->num_ref_idx_l1_active_minus1 = 0;
    }

    // Save the QP value
    if (CodecHal_PictureIsBottomField(picParams->CurrOriginalPic))
    {
        m_refList[m_basicFeature->m_currReconstructedPic.FrameIdx]->ucQPValue[1] =
            picParams->pic_init_qp_minus26 + 26 + slcParams->slice_qp_delta;
    }
    else
    {
        m_refList[m_basicFeature->m_currReconstructedPic.FrameIdx]->ucQPValue[0] =
            picParams->pic_init_qp_minus26 + 26 + slcParams->slice_qp_delta;
    }

    if (slcParams->num_ref_idx_l0_active_minus1 >= CODEC_MAX_NUM_REF_FIELD || slcParams->num_ref_idx_l1_active_minus1 >= CODEC_MAX_NUM_REF_FIELD)
    {
        ENCODE_ASSERTMESSAGE("Invalid slice parameters.");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    for (uint32_t sliceCount = 0; sliceCount < m_basicFeature->m_numSlices; sliceCount++)
    {
        if (m_pictureCodingType != I_TYPE)
        {
            for (uint8_t i = 0; i < (slcParams->num_ref_idx_l0_active_minus1 + 1); i++)
            {
                slcParams->PicOrder[0][i].Picture.FrameIdx =
                    m_picIdx[slcParams->RefPicList[0][i].FrameIdx].ucPicIdx;
                slcParams->PicOrder[0][i].Picture.PicFlags =
                    slcParams->RefPicList[0][i].PicFlags;
            }
        }
        if (m_pictureCodingType == B_TYPE)
        {
            for (uint8_t i = 0; i < (slcParams->num_ref_idx_l1_active_minus1 + 1); i++)
            {
                slcParams->PicOrder[1][i].Picture.FrameIdx =
                    m_picIdx[slcParams->RefPicList[1][i].FrameIdx].ucPicIdx;
                slcParams->PicOrder[1][i].Picture.PicFlags =
                    slcParams->RefPicList[1][i].PicFlags;
            }
        }
        slcParams++;
    }

    if (seqParams->NumRefFrames == CODEC_AVC_MAX_NUM_REF_FRAME)
    {
        const uint8_t hwInvalidFrameId              = CODEC_AVC_MAX_NUM_REF_FRAME - 1;
        slcParams                                   = m_basicFeature->m_sliceParams;
        bool          isActiveRef[hwInvalidFrameId] = {};
        uint8_t       swapIndex                     = CODEC_AVC_NUM_UNCOMPRESSED_SURFACE;
        for (uint32_t sliceCount = 0; sliceCount < m_basicFeature->m_numSlices; sliceCount++)
        {
            if (m_pictureCodingType != I_TYPE)
            {
                for (uint8_t i = 0; i < (slcParams->num_ref_idx_l0_active_minus1 + 1); i++)
                {
                    if (slcParams->RefPicList[0][i].FrameIdx >= CODEC_AVC_MAX_NUM_REF_FRAME)
                    {
                        ENCODE_ASSERTMESSAGE("Invalid slice parameters.");
                        return MOS_STATUS_INVALID_PARAMETER;
                    }
                    auto index = m_picIdx[slcParams->RefPicList[0][i].FrameIdx].ucPicIdx;
                    if (m_refList[index]->ucFrameId < hwInvalidFrameId)
                    {
                        isActiveRef[m_refList[index]->ucFrameId] = true;
                    }
                    else if (m_refList[index]->ucFrameId == hwInvalidFrameId && swapIndex == CODEC_AVC_NUM_UNCOMPRESSED_SURFACE)
                    {
                        swapIndex = index;
                    }
                    else
                    {
                        // should never happen, something must be wrong
                        CODECHAL_PUBLIC_ASSERT(false);
                    }
                }
            }
            if (m_pictureCodingType == B_TYPE)
            {
                for (uint8_t i = 0; i < (slcParams->num_ref_idx_l1_active_minus1 + 1); i++)
                {                
                    if (slcParams->RefPicList[1][i].FrameIdx >= CODEC_AVC_MAX_NUM_REF_FRAME)
                    {
                        ENCODE_ASSERTMESSAGE("Invalid slice parameters.");
                        return MOS_STATUS_INVALID_PARAMETER;
                    }
                    auto index = m_picIdx[slcParams->RefPicList[1][i].FrameIdx].ucPicIdx;
                    if (m_refList[index]->ucFrameId < hwInvalidFrameId)
                    {
                        isActiveRef[m_refList[index]->ucFrameId] = true;
                    }
                    else if (m_refList[index]->ucFrameId == hwInvalidFrameId && swapIndex == CODEC_AVC_NUM_UNCOMPRESSED_SURFACE)
                    {
                        swapIndex = index;
                    }
                    else
                    {
                        // should never happen, something must be wrong
                        CODECHAL_PUBLIC_ASSERT(false);
                    }
                }
            }
            slcParams++;
        }

        if (swapIndex < CODEC_AVC_NUM_UNCOMPRESSED_SURFACE)
        {
            uint8_t i = 0;
            for (i = 0; i < hwInvalidFrameId; i++)
            {
                if (isActiveRef[i])
                {
                    continue;
                }
                uint8_t j = 0;
                for (j = 0; j < CODEC_AVC_MAX_NUM_REF_FRAME; j++)
                {
                    if (m_picIdx[j].bValid && m_refList[m_picIdx[j].ucPicIdx]->ucFrameId == i)
                    {
                        std::swap(m_refList[m_picIdx[j].ucPicIdx]->ucFrameId, m_refList[swapIndex]->ucFrameId);
                        break;
                    }
                }
                if (j != CODEC_AVC_MAX_NUM_REF_FRAME)
                {
                    break;
                }
            }
            if (i == hwInvalidFrameId)
            {
                // should never happen, something must be wrong
                CODECHAL_PUBLIC_ASSERT(false);
            }
        }
    }

    return MOS_STATUS_SUCCESS;
}

void AvcReferenceFrames::GetDistScaleFactor()
{
    auto picParams = m_basicFeature->m_picParam;
    auto slcParams = m_basicFeature->m_sliceParams;
    auto refList   = &(m_refList[0]);
    auto picIdx    = &(m_picIdx[0]);

    bool bottom  = CodecHal_PictureIsBottomField(picParams->CurrOriginalPic);
    int  pocCurr = picParams->CurrFieldOrderCnt[bottom];

    MOS_ZeroMemory(m_distScaleFactorList0, sizeof(uint32_t) * CODEC_AVC_MAX_NUM_REF_FRAME * 2);
    for (unsigned int index = 0; index <= slcParams->num_ref_idx_l0_active_minus1; index++)
    {
        auto picture = slcParams->RefPicList[LIST_0][index];
        if (!CodecHal_PictureIsInvalid(picture))
        {
            auto pictureIdx = picIdx[picture.FrameIdx].ucPicIdx;
            int  pocPic0    = CodecHal_PictureIsBottomField(picture) ? refList[pictureIdx]->iFieldOrderCnt[1] : refList[pictureIdx]->iFieldOrderCnt[0];
            picture         = slcParams->RefPicList[LIST_1][0];
            pictureIdx      = picIdx[picture.FrameIdx].ucPicIdx;
            int pocPic1     = CodecHal_PictureIsBottomField(picture) ? refList[pictureIdx]->iFieldOrderCnt[1] : refList[pictureIdx]->iFieldOrderCnt[0];
            int tb          = CodecHal_Clip3(-128, 127, (pocCurr - pocPic0));
            int td          = CodecHal_Clip3(-128, 127, (pocPic1 - pocPic0));
            if (td == 0)
            {
                td = 1;
            }
            int tx = (16384 + ABS(td / 2)) / td;

            m_distScaleFactorList0[index] = CodecHal_Clip3(-1024, 1023, (tb * tx + 32) >> 6);
        }
    }
}

int32_t AvcReferenceFrames::GetBiWeight(
    uint32_t distScaleFactorRefID0List0,
    uint16_t weightedBiPredIdc)
{
    int32_t biWeight = 32;  // default value
    if (weightedBiPredIdc != IMPLICIT_WEIGHTED_INTER_PRED_MODE)
    {
        biWeight = 32;
    }
    else
    {
        biWeight = (distScaleFactorRefID0List0 + 2) >> 2;

        if (biWeight != 16 && biWeight != 21 &&
            biWeight != 32 && biWeight != 43 && biWeight != 48)
        {
            biWeight = 32;  // If # of B-pics between two refs is more than 3. VME does not support it.
        }
    }

    return biWeight;
}

MOS_STATUS AvcReferenceFrames::SetFrameStoreIds(uint8_t frameIdx)
{
    uint8_t invalidFrame = (m_basicFeature->m_mode == CODECHAL_DECODE_MODE_AVCVLD) ? 0x7f : 0x1f;

    for (uint8_t i = 0; i < m_refList[frameIdx]->ucNumRef; i++)
    {
        uint8_t index;
        index = m_refList[frameIdx]->RefList[i].FrameIdx;
        if (m_refList[index]->ucFrameId == invalidFrame)
        {
            uint8_t j;
            for (j = 0; j < CODEC_AVC_MAX_NUM_REF_FRAME; j++)
            {
                if (!m_frameStoreID[j].inUse)
                {
                    m_refList[index]->ucFrameId = j;
                    m_frameStoreID[j].inUse  = true;
                    break;
                }
            }
            if (j == CODEC_AVC_MAX_NUM_REF_FRAME)
            {
                // should never happen, something must be wrong
                CODECHAL_PUBLIC_ASSERT(false);
                m_refList[index]->ucFrameId = 0;
                m_frameStoreID[0].inUse  = true;
            }
        }
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS AvcReferenceFrames::ValidateNumReferences(PCODECHAL_ENCODE_AVC_VALIDATE_NUM_REFS_PARAMS params)
{
    MOS_STATUS        eStatus             = MOS_STATUS_SUCCESS;
    constexpr uint8_t MaxNumRefPMinusOne  = 2;  // caps.MaxNum_Reference0-1 for all TUs
    constexpr uint8_t MaxNumRefB0MinusOne = 0;  // no corresponding caps
    constexpr uint8_t MaxNumRefB1MinusOne = 0;  // caps.MaxNum_Reference1-1 for all TUs

    ENCODE_FUNC_CALL();
    ENCODE_CHK_NULL_RETURN(params);
    ENCODE_CHK_NULL_RETURN(params->pAvcSliceParams);

    uint8_t numRefIdx0MinusOne = params->pAvcSliceParams->num_ref_idx_l0_active_minus1;
    uint8_t numRefIdx1MinusOne = params->pAvcSliceParams->num_ref_idx_l1_active_minus1;

    if (params->wPictureCodingType == P_TYPE)
    {
        if (numRefIdx0MinusOne > MaxNumRefPMinusOne)
        {
            ENCODE_NORMALMESSAGE("Invalid active reference list size (P).");
            numRefIdx0MinusOne = MaxNumRefPMinusOne;
        }

        numRefIdx1MinusOne = 0;
    }
    else if (params->wPictureCodingType == B_TYPE)
    {
        if (numRefIdx0MinusOne > MaxNumRefB0MinusOne)
        {
            ENCODE_NORMALMESSAGE("Invalid active reference list size (B0).");
            numRefIdx0MinusOne = MaxNumRefB0MinusOne;
        }

        if (numRefIdx1MinusOne > MaxNumRefB1MinusOne)
        {
            ENCODE_NORMALMESSAGE("Invalid active reference list1 size (B1).");
            numRefIdx1MinusOne = MaxNumRefB1MinusOne;
        }
    }

    params->pAvcSliceParams->num_ref_idx_l0_active_minus1 = numRefIdx0MinusOne;
    params->pAvcSliceParams->num_ref_idx_l1_active_minus1 = numRefIdx1MinusOne;

    return eStatus;
}

MHW_SETPAR_DECL_SRC(VDENC_PIPE_BUF_ADDR_STATE, AvcReferenceFrames)
{
    ENCODE_FUNC_CALL();

    ENCODE_CHK_NULL_RETURN(m_basicFeature);

    auto trackedBuf = m_basicFeature->m_trackedBuf;
    ENCODE_CHK_NULL_RETURN(trackedBuf);

    auto slcParams = m_basicFeature->m_sliceParams;
    ENCODE_CHK_NULL_RETURN(slcParams);

    if (m_pictureCodingType != I_TYPE)
    {
        // populate the RefPic and DS surface so pfnAddVdencPipeBufAddrCmd() can directly use them
        auto l0RefFrameList = slcParams->RefPicList[LIST_0];
        for (uint8_t refIdx = 0; refIdx <= slcParams->num_ref_idx_l0_active_minus1; refIdx++)
        {
            auto refPic = l0RefFrameList[refIdx];

            if (!CodecHal_PictureIsInvalid(refPic) && m_picIdx[refPic.FrameIdx].bValid)
            {
                // L0 references
                auto refPicIdx                    = m_picIdx[refPic.FrameIdx].ucPicIdx;
                params.refs[refIdx] = &m_refList[refPicIdx]->sRefReconBuffer.OsResource;
                m_allocator->UpdateResourceUsageType(&m_refList[refPicIdx]->sRefReconBuffer.OsResource, MOS_HW_RESOURCE_USAGE_ENCODE_INPUT_RECON);

                // 4x DS surface for VDEnc
                uint8_t scaledIdx = m_refList[refPicIdx]->ucScalingIdx;
                auto vdenc4xDsSurface = trackedBuf->GetSurface(BufferType::ds4xSurface, scaledIdx);
                ENCODE_CHK_NULL_RETURN(vdenc4xDsSurface);
                params.refsDsStage1[refIdx] = &vdenc4xDsSurface->OsResource;
            }
        }
    }

    if (m_pictureCodingType == B_TYPE)
    {
        auto l1RefFrameList = slcParams->RefPicList[LIST_1];
        auto l0RefNum = slcParams->num_ref_idx_l0_active_minus1 + 1;
        for (uint8_t refIdx = 0; refIdx <= slcParams->num_ref_idx_l1_active_minus1; refIdx++)
        {
            auto refPic = l1RefFrameList[refIdx];

            if (!CodecHal_PictureIsInvalid(refPic) && m_picIdx[refPic.FrameIdx].bValid)
            {
                // L1 references
                auto refPicIdx = m_picIdx[refPic.FrameIdx].ucPicIdx;
                params.refs[l0RefNum + refIdx] = &m_refList[refPicIdx]->sRefReconBuffer.OsResource;
                m_allocator->UpdateResourceUsageType(&m_refList[refPicIdx]->sRefReconBuffer.OsResource, MOS_HW_RESOURCE_USAGE_ENCODE_INPUT_RECON);

                // 4x DS surface for VDEnc
                uint8_t scaledIdx = m_refList[refPicIdx]->ucScalingIdx;
                auto vdenc4xDsSurface = trackedBuf->GetSurface(BufferType::ds4xSurface, scaledIdx);
                ENCODE_CHK_NULL_RETURN(vdenc4xDsSurface);
                params.refsDsStage1[l0RefNum + refIdx] = &vdenc4xDsSurface->OsResource;
            }
        }
    }

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(VDENC_AVC_IMG_STATE, AvcReferenceFrames)
{
    auto picParams = m_basicFeature->m_picParam;
    auto slcParams = m_basicFeature->m_sliceParams;
    uint8_t refPoc[2][3] = {};

    if (picParams->CodingType != I_TYPE)
    {
        params.numberOfL0ReferencesMinusOne = slcParams->num_ref_idx_l0_active_minus1;
        params.numberOfL1ReferencesMinusOne = slcParams->num_ref_idx_l1_active_minus1;

        MHW_CHK_STATUS_RETURN(slcParams->num_ref_idx_l0_active_minus1 > 2);
        uint8_t fwdRefIdx[3] = {0xf, 0xf, 0xf};
        for (auto i = 0; i < slcParams->num_ref_idx_l0_active_minus1 + 1; i++)
        {
            auto id = slcParams->RefPicList[LIST_0][i].FrameIdx;
            id = m_picIdx[id].ucPicIdx;
            fwdRefIdx[i] = m_refList[id]->ucFrameId;
            refPoc[0][i] = (uint8_t)m_refList[id]->iFieldOrderCnt[0];
        }
        params.fwdRefIdx0ReferencePicture = fwdRefIdx[0];
        params.fwdRefIdx1ReferencePicture = fwdRefIdx[1];
        params.fwdRefIdx2ReferencePicture = fwdRefIdx[2];

        if (picParams->CodingType == B_TYPE)
        {
            auto id = slcParams->RefPicList[LIST_1][0].FrameIdx;
            params.longtermReferenceFrameBwdRef0Indicator = (id < CODEC_AVC_MAX_NUM_REF_FRAME) && CodecHal_PictureIsLongTermRef(picParams->RefFrameList[id]);
            if (id >= CODEC_AVC_MAX_NUM_REF_FRAME)
            {
                return MOS_STATUS_INVALID_PARAMETER;
            }             
            id = m_picIdx[id].ucPicIdx;
            params.bwdRefIdx0ReferencePicture = m_refList[id]->ucFrameId;
            refPoc[1][0] = (uint8_t)m_refList[id]->iFieldOrderCnt[0];
        }
    }

    params.pocNumberForCurrentPicture = (uint8_t)m_refList[picParams->CurrReconstructedPic.FrameIdx]->iFieldOrderCnt[0];
    params.pocNumberForFwdRef0 = refPoc[0][0];
    params.pocNumberForFwdRef1 = refPoc[0][1];
    params.pocNumberForFwdRef2 = refPoc[0][2];
    params.pocNumberForBwdRef0 = refPoc[1][0];

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(MFX_PIPE_BUF_ADDR_STATE, AvcReferenceFrames)
{
    ENCODE_FUNC_CALL();

    ENCODE_CHK_NULL_RETURN(m_basicFeature);

    auto trackedBuf = m_basicFeature->m_trackedBuf;
    ENCODE_CHK_NULL_RETURN(trackedBuf);

    auto firstValidFrame = &m_basicFeature->m_reconSurface.OsResource;
    auto slcParams = m_basicFeature->m_sliceParams;
    ENCODE_CHK_NULL_RETURN(slcParams);

    // Setting invalid entries to nullptr
    for (uint8_t i = 0; i < CODEC_AVC_MAX_NUM_REF_FRAME; i++)
        params.presReferences[i] = nullptr;

    uint8_t firstValidFrameId = CODEC_AVC_MAX_NUM_REF_FRAME;

    for (uint8_t i = 0; i < CODEC_AVC_MAX_NUM_REF_FRAME; i++)
    {
        if (m_picIdx[i].bValid)
        {
            auto picIdx       = m_picIdx[i].ucPicIdx;
            auto frameStoreId = m_refList[picIdx]->ucFrameId;

            params.presReferences[frameStoreId] = &(m_refList[picIdx]->sRefReconBuffer.OsResource);
            m_allocator->UpdateResourceUsageType(&m_refList[picIdx]->sRefReconBuffer.OsResource, MOS_HW_RESOURCE_USAGE_ENCODE_INPUT_RECON);

            if (picIdx < firstValidFrameId)
            {
                firstValidFrameId = picIdx;
                firstValidFrame   = params.presReferences[picIdx];
            }
        }
    }

    for (uint8_t i = 0; i < CODEC_AVC_MAX_NUM_REF_FRAME; i++)
    {
        // error concealment for the unset reference addresses
        if (!params.presReferences[i])
        {
            params.presReferences[i] = firstValidFrame;
        }
    }

    if (m_pictureCodingType != I_TYPE)
    {
        // populate the RefPic and DS surface so pfnAddVdencPipeBufAddrCmd() can directly use them
        auto l0RefFrameList = slcParams->RefPicList[LIST_0];
        for (uint8_t refIdx = 0; refIdx <= slcParams->num_ref_idx_l0_active_minus1; refIdx++)
        {
            auto refPic = l0RefFrameList[refIdx];

            if (!CodecHal_PictureIsInvalid(refPic) && m_picIdx[refPic.FrameIdx].bValid)
            {
                // L0 references
                auto refPicIdx                    = m_picIdx[refPic.FrameIdx].ucPicIdx;
                params.presVdencReferences[refIdx] = &m_refList[refPicIdx]->sRefReconBuffer.OsResource;
                m_allocator->UpdateResourceUsageType(&m_refList[refPicIdx]->sRefReconBuffer.OsResource, MOS_HW_RESOURCE_USAGE_ENCODE_INPUT_RECON);
            }
        }
    }

    if (m_pictureCodingType == B_TYPE)
    {
        auto l1RefFrameList = slcParams->RefPicList[LIST_1];
        auto l0RefNum = slcParams->num_ref_idx_l0_active_minus1 + 1;
        for (uint8_t refIdx = 0; refIdx <= slcParams->num_ref_idx_l1_active_minus1; refIdx++)
        {
            auto refPic = l1RefFrameList[refIdx];

            if (!CodecHal_PictureIsInvalid(refPic) && m_picIdx[refPic.FrameIdx].bValid)
            {
                // L1 references
                auto refPicIdx = m_picIdx[refPic.FrameIdx].ucPicIdx;
                params.presVdencReferences[l0RefNum + refIdx] = &m_refList[refPicIdx]->sRefReconBuffer.OsResource;
                m_allocator->UpdateResourceUsageType(&m_refList[refPicIdx]->sRefReconBuffer.OsResource, MOS_HW_RESOURCE_USAGE_ENCODE_INPUT_RECON);
            }
        }
    }

    return MOS_STATUS_SUCCESS;
}

}  // namespace encode
