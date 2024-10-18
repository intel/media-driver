/*
* Copyright (c) 2019-2024, Intel Corporation
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
//! \file     decode_av1_reference_frames.cpp
//! \brief    Defines reference list related logic for av1 decode
//!

#include "decode_av1_basic_feature.h"
#include "decode_utils.h"
#include "codec_utilities_next.h"
#include "decode_av1_reference_frames.h"
#include "codec_def_decode_av1.h"

namespace decode
{
    Av1ReferenceFrames::Av1ReferenceFrames()
    {
        memset(m_refList, 0, sizeof(m_refList));
    }

    MOS_STATUS Av1ReferenceFrames::Init(Av1BasicFeature *basicFeature, DecodeAllocator& allocator)
    {
        DECODE_FUNC_CALL();

        DECODE_CHK_NULL(basicFeature);
        m_basicFeature = basicFeature;
        m_allocator = &allocator;
        DECODE_CHK_NULL(m_allocator);
        DECODE_CHK_STATUS(
            CodecUtilities::CodecHalAllocateDataList((CODEC_REF_LIST_AV1 **)m_refList, CODECHAL_MAX_DPB_NUM_LST_AV1));

        return MOS_STATUS_SUCCESS;
    }

    Av1ReferenceFrames::~Av1ReferenceFrames()
    {
        DECODE_FUNC_CALL();

        CodecUtilities::CodecHalFreeDataList(m_refList, CODECHAL_MAX_DPB_NUM_LST_AV1);
        m_activeReferenceList.clear();
    }

    MOS_STATUS Av1ReferenceFrames::UpdatePicture(CodecAv1PicParams & picParams)
    {
        DECODE_FUNC_CALL();

        DECODE_CHK_STATUS(UpdateCurFrame(picParams));

        uint8_t refPicIndex = picParams.m_refFrameIdx[picParams.m_primaryRefFrame];
        auto refPic = picParams.m_refFrameMap[refPicIndex];
        if (!CodecHal_PictureIsInvalid(refPic))
        {
            m_prevFrameIdx = refPic.FrameIdx;
        }

        if (picParams.m_picInfoFlags.m_fields.m_largeScaleTile && picParams.m_anchorFrameList != nullptr)
        {
            DECODE_CHK_STATUS(UpdateCurRefList(picParams));
        }

        DECODE_CHK_STATUS(UpdateRefCachePolicy(picParams));

        return MOS_STATUS_SUCCESS;
    }

    const std::vector<uint8_t> & Av1ReferenceFrames::GetActiveReferenceList(CodecAv1PicParams & picParams, CodecAv1TileParams & tileParams)
    {
        DECODE_FUNC_CALL();

        m_activeReferenceList.clear();
        for (auto i = 0; i < av1NumInterRefFrames; i++)
        {
            if (picParams.m_picInfoFlags.m_fields.m_largeScaleTile)
            {
                DECODE_ASSERT(m_basicFeature->m_tileCoding.m_curTile < (int32_t)m_basicFeature->m_tileCoding.m_numTiles);
                uint8_t frameIdx = tileParams.m_anchorFrameIdx.FrameIdx;
                if (frameIdx >= CODECHAL_MAX_DPB_NUM_LST_AV1)
                {
                    continue;
                }
                m_activeReferenceList.push_back(frameIdx);
            }
            else
            {
                PCODEC_PICTURE refFrameList = &(picParams.m_refFrameMap[0]);
                uint8_t refPicIndex = picParams.m_refFrameIdx[i];
                uint8_t frameIdx = 0xFF;
                if (refPicIndex < av1TotalRefsPerFrame &&
                    refFrameList[refPicIndex].FrameIdx < CODECHAL_MAX_DPB_NUM_AV1)
                {
                    frameIdx = refFrameList[refPicIndex].FrameIdx;
                }
                else
                {
                    MOS_STATUS hr = GetValidReferenceIndex(&frameIdx);
                }
                m_activeReferenceList.push_back(frameIdx);
            }
        }

        return m_activeReferenceList;
    }

    PMOS_RESOURCE Av1ReferenceFrames::GetReferenceByFrameIndex(uint8_t frameIndex)
    {
        DECODE_FUNC_CALL();

        if (frameIndex >= CODECHAL_MAX_DPB_NUM_AV1)
        {
            DECODE_ASSERTMESSAGE("Invalid reference frame index");
            return nullptr;
        }

        PCODEC_REF_LIST_AV1 ref = m_refList[frameIndex];

        if (ref == nullptr || m_allocator->ResourceIsNull(&(ref->resRefPic)))
        {
            return nullptr;
        }

        return &(ref->resRefPic);
    }

    PMOS_RESOURCE Av1ReferenceFrames::GetValidReference()
    {
        DECODE_FUNC_CALL();

        if (m_basicFeature->m_av1PicParams == nullptr)
        {
            return nullptr;
        }
        auto m_picParams = m_basicFeature->m_av1PicParams;

        for(auto i = 0; i < av1NumInterRefFrames; i++)
        {
            auto index = m_picParams->m_refFrameIdx[i];
            if (index >= av1TotalRefsPerFrame)
            {
                continue;
            }
            uint8_t frameIdx = m_picParams->m_refFrameMap[index].FrameIdx;
            if (frameIdx >= m_basicFeature->m_maxFrameIndex)
            {
                continue;
            }
            PMOS_RESOURCE buffer = GetReferenceByFrameIndex(frameIdx);
            if (buffer != nullptr)
            {
                return buffer;
            }
        }

        return &(m_basicFeature->m_destSurface.OsResource);
    }

    MOS_STATUS Av1ReferenceFrames::GetValidReferenceIndex(uint8_t *validRefIndex)
    {
        DECODE_FUNC_CALL();

        if (m_basicFeature->m_av1PicParams == nullptr)
        {
            return MOS_STATUS_INVALID_PARAMETER;
        }
        auto m_picParams = m_basicFeature->m_av1PicParams;

        for (auto i = 0; i < av1NumInterRefFrames; i++)
        {
            auto    index    = m_picParams->m_refFrameIdx[i];
            if (index >= av1TotalRefsPerFrame)
            {
                continue;
            }
            uint8_t frameIdx = m_picParams->m_refFrameMap[index].FrameIdx;
            if (frameIdx >= m_basicFeature->m_maxFrameIndex)
            {
                continue;
            }
            PMOS_RESOURCE buffer = GetReferenceByFrameIndex(frameIdx);
            if (buffer != nullptr)
            {
                *validRefIndex = frameIdx;
                return MOS_STATUS_SUCCESS;
            }
        }

        *validRefIndex = m_basicFeature->m_av1PicParams->m_currPic.FrameIdx;
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1ReferenceFrames::InsertAnchorFrame(CodecAv1PicParams & picParams)
    {
        DECODE_FUNC_CALL();

        DECODE_CHK_COND(((!picParams.m_picInfoFlags.m_fields.m_largeScaleTile && (picParams.m_currPic.FrameIdx >= CODECHAL_MAX_DPB_NUM_AV1)) ||
                            picParams.m_picInfoFlags.m_fields.m_largeScaleTile && (picParams.m_currPic.FrameIdx >= CODECHAL_MAX_DPB_NUM_LST_AV1)),
            "Invalid frame index of current frame");

        m_currRefList = m_refList[picParams.m_currPic.FrameIdx];

        DECODE_CHK_STATUS(m_allocator->RegisterResource(&m_basicFeature->m_destSurface.OsResource));

        m_currRefList->resRefPic        = m_basicFeature->m_destSurface.OsResource;
        m_currRefList->m_frameWidth     = picParams.m_superResUpscaledWidthMinus1 + 1;  //DPB buffer are always stored in full frame resolution (Super-Res up-scaled resolution)
        m_currRefList->m_frameHeight    = picParams.m_superResUpscaledHeightMinus1 + 1;
        m_currRefList->m_miCols         = MOS_ALIGN_CEIL(picParams.m_frameWidthMinus1 + 1, 8) >> av1MiSizeLog2;
        m_currRefList->m_miRows         = MOS_ALIGN_CEIL(picParams.m_frameHeightMinus1 + 1, 8) >> av1MiSizeLog2;

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1ReferenceFrames::UpdateCurRefList(const CodecAv1PicParams & picParams)
    {
        DECODE_FUNC_CALL();

        // override internal reference list with anchor_frame_list passed from APP
        if (picParams.m_anchorFrameNum > CODECHAL_MAX_DPB_NUM_LST_AV1)
        {
            return MOS_STATUS_INVALID_PARAMETER;
        }
        for (auto i = 0; i < picParams.m_anchorFrameNum; i++)
        {
            DECODE_CHK_STATUS(m_allocator->GetSurfaceInfo(&picParams.m_anchorFrameList[i]));
            DECODE_CHK_STATUS(m_allocator->RegisterResource(&(picParams.m_anchorFrameList[i].OsResource)));

            m_refList[i]->resRefPic        = picParams.m_anchorFrameList[i].OsResource;
            m_refList[i]->m_frameWidth     = picParams.m_superResUpscaledWidthMinus1 + 1;  //DPB buffer are always stored in full frame resolution (Super-Res up-scaled resolution)
            m_refList[i]->m_frameHeight    = picParams.m_superResUpscaledHeightMinus1 + 1;
            m_refList[i]->m_miCols         = MOS_ALIGN_CEIL(picParams.m_frameWidthMinus1 + 1, 8) >> av1MiSizeLog2;
            m_refList[i]->m_miRows         = MOS_ALIGN_CEIL(picParams.m_frameHeightMinus1 + 1, 8) >> av1MiSizeLog2;
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1ReferenceFrames::UpdateCurResource(const PCODEC_REF_LIST_AV1 pCurRefList)
    {
        DECODE_FUNC_CALL();
        DECODE_CHK_NULL(pCurRefList);
        DECODE_CHK_NULL(m_basicFeature->m_av1PicParams);
        auto m_picParams = m_basicFeature->m_av1PicParams;

        // Overwrite the actual surface height with the coded height and width of the frame
        // for AV1 since it's possible for a AV1 frame to change size during playback
        if (!m_picParams->m_picInfoFlags.m_fields.m_largeScaleTile)
        {
            m_basicFeature->m_destSurface.dwWidth  = m_picParams->m_superResUpscaledWidthMinus1 + 1;  //DPB buffer size may be larger than the YUV data size to put in it, so override it
            m_basicFeature->m_destSurface.dwHeight = m_picParams->m_superResUpscaledHeightMinus1 + 1;
        }

        pCurRefList->resRefPic = m_basicFeature->m_destSurface.OsResource;
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1ReferenceFrames::UpdateCurFrame(const CodecAv1PicParams & picParams)
    {
        DECODE_FUNC_CALL();

        DECODE_CHK_COND(((!picParams.m_picInfoFlags.m_fields.m_largeScaleTile && (picParams.m_currPic.FrameIdx >= CODECHAL_MAX_DPB_NUM_AV1)) ||
                            picParams.m_picInfoFlags.m_fields.m_largeScaleTile && (picParams.m_currPic.FrameIdx >= CODECHAL_MAX_DPB_NUM_LST_AV1)),
                        "Invalid frame index of current frame");
        m_currRefList = m_refList[picParams.m_currPic.FrameIdx];
        MOS_ZeroMemory(m_currRefList, sizeof(CODEC_REF_LIST_AV1));

        DECODE_CHK_STATUS(UpdateCurResource(m_currRefList));
        m_currRefList->m_frameWidth     = picParams.m_superResUpscaledWidthMinus1 + 1;  //DPB buffer are always stored in full frame resolution (Super-Res up-scaled resolution)
        m_currRefList->m_frameHeight    = picParams.m_superResUpscaledHeightMinus1 + 1;
        m_currRefList->m_miCols         = MOS_ALIGN_CEIL(picParams.m_frameWidthMinus1 + 1, 8) >> av1MiSizeLog2;
        m_currRefList->m_miRows         = MOS_ALIGN_CEIL(picParams.m_frameHeightMinus1 + 1, 8) >> av1MiSizeLog2;
        m_currRefList->RefPic           = picParams.m_currPic;
        m_currRefList->m_orderHint      = picParams.m_orderHint;
        m_currRefList->m_segmentEnable  = picParams.m_av1SegData.m_enabled;
        m_currRefList->m_frameType      = picParams.m_picInfoFlags.m_fields.m_frameType;

        if (!AV1_KEY_OR_INRA_FRAME(picParams.m_picInfoFlags.m_fields.m_frameType) &&
            picParams.m_seqInfoFlags.m_fields.m_enableOrderHint)
        {
            for (auto i = 0; i < av1NumInterRefFrames; i++)
            {
                auto index = picParams.m_refFrameIdx[i];
                if (!CodecHal_PictureIsInvalid(picParams.m_refFrameMap[index]))
                {
                    uint8_t frameIdx = picParams.m_refFrameMap[index].FrameIdx;
                    m_currRefList->m_refOrderHint[i] = m_refList[frameIdx]->m_orderHint;
                }
            }
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1ReferenceFrames::UpdateRefCachePolicy(CodecAv1PicParams &picParams)
    {
        DECODE_FUNC_CALL();
        MOS_STATUS sts = MOS_STATUS_SUCCESS;

        Av1ReferenceFrames &refFrames = m_basicFeature->m_refFrames;
        if (picParams.m_picInfoFlags.m_fields.m_frameType != keyFrame)
        {
            const std::vector<uint8_t> &activeRefList = refFrames.GetActiveReferenceList(
                picParams, m_basicFeature->m_av1TileParams[m_basicFeature->m_tileCoding.m_curTile]);

            for (uint8_t i = 0; i < activeRefList.size(); i++)
            {
                uint8_t frameIdx = activeRefList[i];
                sts              = m_allocator->UpdateResoreceUsageType(refFrames.GetReferenceByFrameIndex(frameIdx), resourceInputReference);
                if (sts != MOS_STATUS_SUCCESS)
                {
                    DECODE_NORMALMESSAGE("GetReferenceByFrameIndex invalid\n");
                }
            }
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1ReferenceFrames::Identify1stNearRef(const CodecAv1PicParams & picParams,
        int32_t curFrameOffset, int32_t* refFrameOffset, int32_t* refIdx)
    {
        DECODE_FUNC_CALL();

        for (auto i = 0; i < av1NumInterRefFrames; ++i)
        {
            int32_t refOffset = -1;

            uint8_t refPicIndex = picParams.m_refFrameIdx[i];//i=0 corresponds to LAST_FRAME.
            const CODEC_PICTURE*  refFrameList = &(picParams.m_refFrameMap[0]);

            if (!CodecHal_PictureIsInvalid(refFrameList[refPicIndex]))
            {
                uint8_t refFrameIdx = refFrameList[refPicIndex].FrameIdx;
                refOffset = m_refList[refFrameIdx]->m_orderHint;
            }

            if (GetRelativeDist(picParams, refOffset, curFrameOffset) < 0)
            {
                // Forward reference
                if (refFrameOffset[0] == -1 || GetRelativeDist(picParams, refOffset, refFrameOffset[0]) > 0)
                {
                    refFrameOffset[0] = refOffset;
                    refIdx[0] = i;
                }
            }
            else if (GetRelativeDist(picParams,refOffset, curFrameOffset) > 0)
            {
                // Backward reference
                if (refFrameOffset[1] == INT_MAX ||
                    GetRelativeDist(picParams, refOffset, refFrameOffset[1]) < 0)
                {
                    refFrameOffset[1] = refOffset;
                    refIdx[1] = i;
                }
            }
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1ReferenceFrames::Identify2ndNearRef(const CodecAv1PicParams & picParams,
        int32_t curFrameOffset,int32_t* refFrameOffset,int32_t* refIdx)
    {
        DECODE_FUNC_CALL();

        // == Forward prediction only ==
        // Identify the second nearest forward reference.
        refFrameOffset[1] = -1;
        for (int i = 0; i < av1NumInterRefFrames; ++i)
        {
            int32_t refOffset = -1;

            uint8_t refPicIndex = picParams.m_refFrameIdx[i]; //i=0 corresponds to LAST_FRAME.
            const CODEC_PICTURE* refFrameList = &(picParams.m_refFrameMap[0]);

            if (!CodecHal_PictureIsInvalid(refFrameList[refPicIndex]))
            {
                uint8_t refFrameIdx = refFrameList[refPicIndex].FrameIdx;
                refOffset = m_refList[refFrameIdx]->m_orderHint;
            }

            if ((refFrameOffset[0] != -1 &&
                GetRelativeDist(picParams, refOffset, refFrameOffset[0]) < 0) &&
                (refFrameOffset[1] == -1 ||
                    GetRelativeDist(picParams, refOffset, refFrameOffset[1]) > 0))
            {
                // Second closest forward reference
                refFrameOffset[1] = refOffset;
                refIdx[1] = i;
            }
        }

        return MOS_STATUS_SUCCESS;
    }

    int32_t Av1ReferenceFrames::GetRelativeDist(const CodecAv1PicParams & picParams, int32_t a, int32_t b)
    {
        DECODE_FUNC_CALL();

        if (!picParams.m_seqInfoFlags.m_fields.m_enableOrderHint)
        {
            return 0;
        }

        int32_t bits = picParams.m_orderHintBitsMinus1 + 1;
        DECODE_ASSERT(bits >= 1);
        DECODE_ASSERT(a >= 0 && a < (1 << bits));
        DECODE_ASSERT(b >= 0 && b < (1 << bits));

        int32_t diff = a - b;
        int32_t m = 1 << (bits - 1);
        diff = (diff & (m - 1)) - (diff & m);

        return diff;
    }

    MOS_STATUS Av1ReferenceFrames::SetupMotionFieldProjection(CodecAv1PicParams & picParams)
    {
        DECODE_FUNC_CALL();

        int32_t refBufIdx[av1NumInterRefFrames] = {-1, -1, -1, -1, -1, -1, -1};
        int32_t refOrderHint[av1NumInterRefFrames] = {0, 0, 0, 0, 0, 0, 0};
        uint8_t curOrderHint = m_currRefList->m_orderHint;
        int32_t refStamp = av1MfmvStackSize - 1;

        for (auto ref = 0; ref < av1NumInterRefFrames; ref++)
        {
            uint8_t refPicIndex = picParams.m_refFrameIdx[ref];
            int8_t refFrameIdx = picParams.m_refFrameMap[refPicIndex].FrameIdx;
            refBufIdx[ref] = refFrameIdx;
            if (refFrameIdx >= 0)
            {
                refOrderHint[ref] = m_refList[refFrameIdx]->m_orderHint;
            }
        }

        memset(picParams.m_activeRefBitMaskMfmv, 0, 7 * sizeof(bool));

        if(refBufIdx[lastFrame - lastFrame] >= 0)
        {
            uint8_t refPicIndex = picParams.m_refFrameIdx[0];//0 corresponds to LAST_FRAME
            if (!CodecHal_PictureIsInvalid(picParams.m_refFrameMap[refPicIndex]))
            {
                uint8_t refFrameIdx = picParams.m_refFrameMap[refPicIndex].FrameIdx;
                if (!(m_refList[refFrameIdx]->m_refOrderHint[altRefFrame - lastFrame] == refOrderHint[goldenFrame - lastFrame]))
                {
                    MotionFieldProjection(picParams, lastFrame, 2);
                }
                --refStamp;
            }
        }

        if (GetRelativeDist(picParams, refOrderHint[bwdRefFrame - lastFrame], curOrderHint) > 0)
        {
            if (MotionFieldProjection(picParams, bwdRefFrame, 0))
            {
                --refStamp;
            }
        }

        if (GetRelativeDist(picParams, refOrderHint[altRef2Frame - lastFrame], curOrderHint) > 0)
        {
            if (MotionFieldProjection(picParams, altRef2Frame, 0))
            {
                --refStamp;
            }
        }

        if (GetRelativeDist(picParams, refOrderHint[altRefFrame - lastFrame], curOrderHint) > 0 && refStamp >= 0)
        {
            if (MotionFieldProjection(picParams, altRefFrame, 0))
            {
                --refStamp;
            }
        }

        if (refStamp >= 0 && refBufIdx[last2Frame - lastFrame] >= 0)
        {
            if (MotionFieldProjection(picParams, last2Frame, 2))
            {
                --refStamp;
            }
        }

        return MOS_STATUS_SUCCESS;
    }

    bool Av1ReferenceFrames::MotionFieldProjection(CodecAv1PicParams & picParams, int32_t ref, int32_t dir)
    {
        DECODE_FUNC_CALL();

        //Calculate active reference bit mask for motion field
        uint8_t refPicIndex = picParams.m_refFrameIdx[ref - lastFrame];//0 corresponds to LAST_FRAME
        uint8_t refFrameIdx = m_basicFeature->m_invalidFrameIndex;
        uint32_t miCols = 0, miRows = 0, refFrameType = 0;

        if (!CodecHal_PictureIsInvalid(picParams.m_refFrameMap[refPicIndex]))
        {
            refFrameIdx     = picParams.m_refFrameMap[refPicIndex].FrameIdx;
            miCols          = m_refList[refFrameIdx]->m_miCols;
            miRows          = m_refList[refFrameIdx]->m_miRows;
            refFrameType    = m_refList[refFrameIdx]->m_frameType;
        }

        if ((refFrameIdx == m_basicFeature->m_invalidFrameIndex) || (refFrameType == intraOnlyFrame || refFrameType == keyFrame) ||
            (m_basicFeature->m_tileCoding.m_miCols != miCols) || (m_basicFeature->m_tileCoding.m_miRows != miRows))
        {
            picParams.m_activeRefBitMaskMfmv[ref - lastFrame] = 0;
        }
        else
        {
            picParams.m_activeRefBitMaskMfmv[ref - lastFrame] = 1;
        }

        return picParams.m_activeRefBitMaskMfmv[ref - lastFrame];
    }

    bool Av1ReferenceFrames::CheckSegForPrimFrame(CodecAv1PicParams &picParams)
    {
        DECODE_FUNC_CALL();

        bool isMatched = false;
        if (m_currRefList->m_miCols == m_refList[m_prevFrameIdx]->m_miCols &&
            m_currRefList->m_miRows == m_refList[m_prevFrameIdx]->m_miRows &&
            m_refList[m_prevFrameIdx]->m_segmentEnable)
        {
            auto tempBuffers = &(m_basicFeature->m_tempBuffers);
            auto tempBuf = tempBuffers->GetBufferByFrameIndex(m_prevFrameIdx);
            if ((tempBuf != nullptr) && (tempBuf->segIdBuf != nullptr))
            {
                isMatched = true;
            }
        }

        return isMatched;
    }

    MOS_STATUS Av1ReferenceFrames::ErrorConcealment(CodecAv1PicParams &picParams)
    {
        DECODE_FUNC_CALL();
        MOS_STATUS             hr           = MOS_STATUS_SUCCESS;
        Av1ReferenceFrames     &refFrames    = m_basicFeature->m_refFrames;
        PCODEC_PICTURE         refFrameList = &(picParams.m_refFrameMap[0]);

        uint8_t validfPicIndex   = 0;
        bool    hasValidRefIndex = false;

        for (auto i = 0; i < av1NumInterRefFrames; i++)
        {
            uint8_t refPicIndex = picParams.m_refFrameIdx[i];
            if (refPicIndex >= av1TotalRefsPerFrame)
            {
                continue;
            }
            uint8_t frameIdx = refFrameList[refPicIndex].FrameIdx;
            auto    curframe = refFrames.GetReferenceByFrameIndex(frameIdx);
            if (curframe == nullptr)
            {
                if (hasValidRefIndex == false)
                {
                    uint8_t validfPicIndex = 0;
                    //Get valid reference frame index
                    hr               = GetValidReferenceIndex(&validfPicIndex);
                    hasValidRefIndex = true;
                }
                //Use validfPicIndex to replace invalid Reference index for non-key frame
                refFrameList[refPicIndex].FrameIdx = validfPicIndex;
                DECODE_ASSERTMESSAGE("Hit invalid refFrameList[%d].FrameIdx=%d\n", refPicIndex, refFrameList[refPicIndex].FrameIdx);
            }
        }
        return hr;
    }
    }  // namespace decode
