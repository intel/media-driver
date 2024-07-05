/*
* Copyright (c) 2021-2023, Intel Corporation
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
//! \file     decode_vvc_reference_frames.cpp
//! \brief    Defines reference list related logic for vvc decode
//!

#include "decode_vvc_basic_feature.h"
#include "decode_utils.h"
#include "codec_utilities_next.h"
#include "decode_vvc_reference_frames.h"
#include "codec_def_decode_vvc.h"

namespace decode
{
    VvcReferenceFrames::VvcReferenceFrames()
    {
        memset(m_refList, 0, sizeof(m_refList));
    }

    MOS_STATUS VvcReferenceFrames::Init(VvcBasicFeature *basicFeature, DecodeAllocator& allocator)
    {
        DECODE_FUNC_CALL();

        DECODE_CHK_NULL(basicFeature);
        m_basicFeature = basicFeature;
        m_allocator = &allocator;
        DECODE_CHK_NULL(m_allocator);
        DECODE_CHK_STATUS(
            CodecUtilities::CodecHalAllocateDataList((CODEC_REF_LIST_VVC **)m_refList, CODEC_MAX_DPB_NUM_VVC));

        return MOS_STATUS_SUCCESS;
    }

    VvcReferenceFrames::~VvcReferenceFrames()
    {
        DECODE_FUNC_CALL();

        CodecUtilities::CodecHalFreeDataList(m_refList, CODEC_MAX_DPB_NUM_VVC);
        //m_activeReferenceList.clear();
    }

    MOS_STATUS VvcReferenceFrames::UpdatePicture(CodecVvcPicParams & picParams)
    {
        DECODE_FUNC_CALL();

        DECODE_CHK_STATUS(UpdateCurFrame(picParams));
        DECODE_CHK_STATUS(UpdateUnavailableRefFrames(picParams));

        return MOS_STATUS_SUCCESS;
    }

    const std::vector<uint8_t> & VvcReferenceFrames::GetActiveReferenceList(CodecVvcPicParams & picParams)
    {
        DECODE_FUNC_CALL();

        m_activeReferenceList.clear();
        for (auto i = 0; i < vvcMaxNumRefFrame; i++)
        {
            uint8_t frameIdx = picParams.m_refFrameList[i].FrameIdx;
            if (frameIdx >= CODEC_MAX_DPB_NUM_VVC)
            {
                continue;
            }
            m_activeReferenceList.push_back(frameIdx);
        }

        return m_activeReferenceList;
    }

    PMOS_RESOURCE VvcReferenceFrames::GetReferenceByFrameIndex(uint8_t frameIndex)
    {
        DECODE_FUNC_CALL();

        if (frameIndex >= CODEC_MAX_DPB_NUM_VVC)
        {
            DECODE_ASSERTMESSAGE("Invalid reference frame index");
            return nullptr;
        }

        PCODEC_REF_LIST_VVC ref = m_refList[frameIndex];

        if (ref == nullptr || m_allocator->ResourceIsNull(&(ref->resRefPic)))
        {
            return nullptr;
        }

        return &(ref->resRefPic);
    }

    MOS_STATUS VvcReferenceFrames::GetRefAttrByFrameIndex(uint8_t frameIndex, VvcRefFrameAttributes* attributes)
    {
        DECODE_FUNC_CALL();

        DECODE_CHK_NULL(attributes);

        if (frameIndex >= CODEC_MAX_DPB_NUM_VVC)
        {
            DECODE_ASSERTMESSAGE("Invalid reference frame index");
            return MOS_STATUS_INVALID_PARAMETER;
        }

        PCODEC_REF_LIST_VVC ref = m_refList[frameIndex];

        if (ref == nullptr || m_allocator->ResourceIsNull(&(ref->resRefPic)))
        {
            return MOS_STATUS_INVALID_PARAMETER;
        }

        attributes->m_refscalingwinleftoffset   = ref->m_ppsScalingWinLeftOffset;
        attributes->m_refscalingwinrightoffset  = ref->m_ppsScalingWinRightOffset;
        attributes->m_refscalingwintopoffset    = ref->m_ppsScalingWinTopOffset;
        attributes->m_refscalingwinbottomoffset = ref->m_ppsScalingWinBottomOffset;
        attributes->m_refpicwidth               = ref->m_ppsPicWidthInLumaSamples;
        attributes->m_refpicheight              = ref->m_ppsPicHeightInLumaSamples;
        attributes->m_currPicScalWinWidthL      = ref->m_currPicScalWinWidthL;
        attributes->m_currPicScalWinHeightL     = ref->m_currPicScalWinHeightL;

        return MOS_STATUS_SUCCESS;
    }

    PMOS_RESOURCE VvcReferenceFrames::GetValidReference()
    {
        DECODE_FUNC_CALL();

        if (m_basicFeature->m_vvcPicParams == nullptr)
        {
            m_validRefFrameIdx = CODEC_MAX_DPB_NUM_VVC;
            return nullptr;
        }
        auto m_picParams = m_basicFeature->m_vvcPicParams;

        for(auto i = 0; i < vvcMaxNumRefFrame; i++)
        {
            uint8_t frameIdx = m_picParams->m_refFrameList[i].FrameIdx;
            if (frameIdx >= CODEC_MAX_DPB_NUM_VVC)
            {
                continue;
            }
            PMOS_RESOURCE buffer = GetReferenceByFrameIndex(frameIdx);
            if (buffer != nullptr)
            {
                m_validRefFrameIdx = frameIdx;
                return buffer;
            }
        }
        m_validRefFrameIdx = m_picParams->m_currPic.FrameIdx;

        return &(m_basicFeature->m_destSurface.OsResource);
    }

    MOS_STATUS VvcReferenceFrames::UpdateCurResource(const PCODEC_REF_LIST_VVC pCurRefList)
    {
        DECODE_FUNC_CALL();
        DECODE_CHK_NULL(pCurRefList);
        DECODE_CHK_NULL(m_basicFeature->m_vvcPicParams);
        auto m_picParams = m_basicFeature->m_vvcPicParams;

        // Overwrite the actual surface height with the coded height and width of the frame
        // since DPB buffer size may be larger than the YUV data size to put in it
        m_basicFeature->m_destSurface.dwWidth   = m_picParams->m_ppsPicWidthInLumaSamples;
        m_basicFeature->m_destSurface.dwHeight  = m_picParams->m_ppsPicHeightInLumaSamples;

        pCurRefList->resRefPic = m_basicFeature->m_destSurface.OsResource;

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS VvcReferenceFrames::UpdateCurFrame(const CodecVvcPicParams & picParams)
    {
        DECODE_FUNC_CALL();

        DECODE_CHK_COND(picParams.m_currPic.FrameIdx >= CODEC_MAX_DPB_NUM_VVC, "Invalid frame index of current frame");
        m_currRefList = m_refList[picParams.m_currPic.FrameIdx];
        MOS_ZeroMemory(m_currRefList, sizeof(CODEC_REF_LIST_VVC));

        DECODE_CHK_STATUS(UpdateCurResource(m_currRefList));

        m_currRefList->m_ppsPicWidthInLumaSamples   = picParams.m_ppsPicWidthInLumaSamples;
        m_currRefList->m_ppsPicHeightInLumaSamples  = picParams.m_ppsPicHeightInLumaSamples;
        m_currRefList->m_ppsScalingWinLeftOffset    = picParams.m_ppsScalingWinLeftOffset;
        m_currRefList->m_ppsScalingWinRightOffset   = picParams.m_ppsScalingWinRightOffset;
        m_currRefList->m_ppsScalingWinTopOffset     = picParams.m_ppsScalingWinTopOffset;
        m_currRefList->m_ppsScalingWinBottomOffset  = picParams.m_ppsScalingWinBottomOffset;
        m_currRefList->m_spsNumSubpicsMinus1        = picParams.m_spsNumSubpicsMinus1;
        m_currRefList->m_spsHorCollocatedChromaFlag = picParams.m_spsFlags1.m_fields.m_spsChromaHorizontalCollocatedFlag;
        m_currRefList->m_spsVerCollocatedChromaFlag = picParams.m_spsFlags1.m_fields.m_spsChromaVerticalCollocatedFlag;
        m_curIsIntra                                = picParams.m_picMiscFlags.m_fields.m_intraPicFlag;

        //Calc CurrPicScalWinWidthL/CurrPicScalWinHeightL
        int8_t subWidthC, subHeightC;
        if (picParams.m_spsChromaFormatIdc == 0 || picParams.m_spsChromaFormatIdc == 3)
        {
            subWidthC  = 1;
            subHeightC = 1;
        }
        else
        {
            subWidthC  = 2;
            subHeightC = (picParams.m_spsChromaFormatIdc == 1) ? 2 : 1;
        }
        m_currRefList->m_currPicScalWinWidthL = picParams.m_ppsPicWidthInLumaSamples - subWidthC * (picParams.m_ppsScalingWinRightOffset + picParams.m_ppsScalingWinLeftOffset);
        m_currRefList->m_currPicScalWinHeightL = picParams.m_ppsPicHeightInLumaSamples - subHeightC * (picParams.m_ppsScalingWinBottomOffset + picParams.m_ppsScalingWinTopOffset);

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS VvcReferenceFrames::UpdateUnavailableRefFrames(const CodecVvcPicParams & picParams)
    {
        DECODE_FUNC_CALL();

        for (auto i = 0; i < vvcMaxNumRefFrame; i++)
        {
            if (picParams.m_refFrameList[i].PicFlags == PICTURE_UNAVAILABLE_FRAME)
            {

                if (picParams.m_refFrameList[i].FrameIdx >=  CODEC_MAX_DPB_NUM_VVC)
                {
                    continue;
                }

                PCODEC_REF_LIST_VVC refList = m_refList[picParams.m_refFrameList[i].FrameIdx];

                refList->resRefPic                      = m_basicFeature->m_destSurface.OsResource;
                refList->m_ppsPicWidthInLumaSamples     = picParams.m_ppsPicWidthInLumaSamples;
                refList->m_ppsPicHeightInLumaSamples    = picParams.m_ppsPicHeightInLumaSamples;
                refList->m_ppsScalingWinLeftOffset      = picParams.m_ppsScalingWinLeftOffset;
                refList->m_ppsScalingWinRightOffset     = picParams.m_ppsScalingWinRightOffset;
                refList->m_ppsScalingWinTopOffset       = picParams.m_ppsScalingWinTopOffset;
                refList->m_ppsScalingWinBottomOffset    = picParams.m_ppsScalingWinBottomOffset;
                refList->m_spsNumSubpicsMinus1          = picParams.m_spsNumSubpicsMinus1;
                refList->m_currPicScalWinWidthL         = picParams.m_ppsPicWidthInLumaSamples;
                refList->m_currPicScalWinHeightL        = picParams.m_ppsPicHeightInLumaSamples;
            }
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS VvcReferenceFrames::CalcRprConstraintsActiveFlag(uint8_t refFrameIdx, bool &flag)
    {
        DECODE_FUNC_CALL();

        if (refFrameIdx >= CODEC_MAX_DPB_NUM_VVC)
        {
            return MOS_STATUS_INVALID_PARAMETER;
        }

        PCODEC_REF_LIST_VVC ref = m_refList[refFrameIdx];

        if (ref == nullptr || m_allocator->ResourceIsNull(&(ref->resRefPic)))
        {
            return MOS_STATUS_INVALID_PARAMETER;
        }

        flag = (ref->m_ppsPicHeightInLumaSamples != m_currRefList->m_ppsPicHeightInLumaSamples) ||
               (ref->m_ppsPicWidthInLumaSamples != m_currRefList->m_ppsPicWidthInLumaSamples)   ||
               (ref->m_ppsScalingWinTopOffset != m_currRefList->m_ppsScalingWinTopOffset)       ||
               (ref->m_ppsScalingWinBottomOffset != m_currRefList->m_ppsScalingWinBottomOffset) ||
               (ref->m_ppsScalingWinLeftOffset != m_currRefList->m_ppsScalingWinLeftOffset)     ||
               (ref->m_ppsScalingWinRightOffset != m_currRefList->m_ppsScalingWinRightOffset)   ||
               (ref->m_spsNumSubpicsMinus1 != m_currRefList->m_spsNumSubpicsMinus1);

        if (flag &&
            (ref->m_spsHorCollocatedChromaFlag != m_currRefList->m_spsHorCollocatedChromaFlag
             || ref->m_spsVerCollocatedChromaFlag != m_currRefList->m_spsVerCollocatedChromaFlag))
        {
            return MOS_STATUS_INVALID_PARAMETER;
        }

        return MOS_STATUS_SUCCESS;
    }


}  // namespace decode
