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
//! \file     decode_vp9_reference_frames.cpp
//! \brief    Defines reference list related logic for vp9 decode
//!

#include "decode_vp9_basic_feature.h"
#include "decode_utils.h"
#include "codec_utilities_next.h"
#include "decode_vp9_reference_frames.h"
#include "codec_def_decode_vp9.h"

namespace decode
{
    Vp9ReferenceFrames::Vp9ReferenceFrames()
    {
        memset(m_vp9RefList, 0, sizeof(m_vp9RefList));
    }

    MOS_STATUS Vp9ReferenceFrames::Init(Vp9BasicFeature *basicFeature, DecodeAllocator& allocator)
    {
        DECODE_FUNC_CALL();

        DECODE_CHK_NULL(basicFeature);
        m_basicFeature = basicFeature;
        m_allocator = &allocator;
        DECODE_CHK_NULL(m_allocator);
        DECODE_CHK_STATUS(CodecUtilities::CodecHalAllocateDataList(m_vp9RefList, CODECHAL_NUM_UNCOMPRESSED_SURFACE_VP9));

        return MOS_STATUS_SUCCESS;
    }

    Vp9ReferenceFrames::~Vp9ReferenceFrames()
    {
        DECODE_FUNC_CALL();

        CodecUtilities::CodecHalFreeDataList(m_vp9RefList, CODECHAL_NUM_UNCOMPRESSED_SURFACE_VP9);  // CODECHAL_MAX_DPB_NUM_AV1 VP9 WHAT?
        m_activeReferenceList.clear();
    }

    MOS_STATUS Vp9ReferenceFrames::UpdatePicture(CODEC_VP9_PIC_PARAMS &picParams)
    {
        DECODE_FUNC_CALL();

        DECODE_CHK_STATUS(UpdateCurFrame(picParams));

        // Override reference list with ref surface passed from DDI if needed
        uint8_t surfCount = 0;
        uint8_t surfIndex = 0;
        while (surfCount < m_basicFeature->m_refSurfaceNum && surfIndex < CODECHAL_NUM_UNCOMPRESSED_SURFACE_VP9)
        {
            if (!m_allocator->ResourceIsNull(&m_basicFeature->m_refFrameSurface[surfIndex].OsResource))
            {
                m_vp9RefList[surfIndex]->resRefPic = m_basicFeature->m_refFrameSurface[surfIndex].OsResource;
                surfCount++;
            }
            surfIndex++;
        }

        PCODEC_PICTURE refFrameList      = &(m_basicFeature->m_vp9PicParams->RefFrameList[0]);
        uint8_t        lastRefPicIndex   = m_basicFeature->m_vp9PicParams->PicFlags.fields.LastRefIdx;
        uint8_t        goldenRefPicIndex = m_basicFeature->m_vp9PicParams->PicFlags.fields.GoldenRefIdx;
        uint8_t        altRefPicIndex    = m_basicFeature->m_vp9PicParams->PicFlags.fields.AltRefIdx;

        if (m_basicFeature->m_vp9PicParams->PicFlags.fields.frame_type == CODEC_VP9_KEY_FRAME ||
            m_basicFeature->m_vp9PicParams->PicFlags.fields.intra_only)
        {
            // reference surface should be nullptr when key_frame == true or intra only frame
            m_basicFeature->m_presLastRefSurface   = nullptr;
            m_basicFeature->m_presGoldenRefSurface = nullptr;
            m_basicFeature->m_presAltRefSurface    = nullptr;
        }
        else
        {
            if (lastRefPicIndex > 7 || goldenRefPicIndex > 7 || altRefPicIndex > 7)
            {
                DECODE_ASSERTMESSAGE("invalid ref index (should be in [0,7]) in pic parameter!");
                return MOS_STATUS_INVALID_PARAMETER;
            }

            if (refFrameList[lastRefPicIndex].FrameIdx >= CODECHAL_NUM_UNCOMPRESSED_SURFACE_VP9)
            {
                refFrameList[lastRefPicIndex].FrameIdx = CODECHAL_NUM_UNCOMPRESSED_SURFACE_VP9 - 1;
            }
            if (refFrameList[goldenRefPicIndex].FrameIdx >= CODECHAL_NUM_UNCOMPRESSED_SURFACE_VP9)
            {
                refFrameList[goldenRefPicIndex].FrameIdx = CODECHAL_NUM_UNCOMPRESSED_SURFACE_VP9 - 1;
            }
            if (refFrameList[altRefPicIndex].FrameIdx >= CODECHAL_NUM_UNCOMPRESSED_SURFACE_VP9)
            {
                refFrameList[altRefPicIndex].FrameIdx = CODECHAL_NUM_UNCOMPRESSED_SURFACE_VP9 - 1;
            }
            PCODEC_REF_LIST *vp9RefList            = &(m_vp9RefList[0]);
            m_basicFeature->m_presLastRefSurface   = &(vp9RefList[refFrameList[lastRefPicIndex].FrameIdx]->resRefPic);
            m_basicFeature->m_presGoldenRefSurface = &(vp9RefList[refFrameList[goldenRefPicIndex].FrameIdx]->resRefPic);
            m_basicFeature->m_presAltRefSurface    = &(vp9RefList[refFrameList[altRefPicIndex].FrameIdx]->resRefPic);
        }

        PMOS_RESOURCE dummyRef = &(m_basicFeature->m_dummyReference.OsResource);
        PMOS_RESOURCE usedDummyReference = nullptr;
        if(m_basicFeature->m_dummyReferenceStatus &&
                !m_allocator->ResourceIsNull(dummyRef))
        {
            usedDummyReference = &(m_basicFeature->m_dummyReference.OsResource);
        }
        else
        {
            usedDummyReference = &(m_basicFeature->m_destSurface.OsResource);
        }

        // Populate surface param for reference pictures
        if (m_basicFeature->m_vp9PicParams->PicFlags.fields.frame_type == CODEC_VP9_INTER_FRAME &&
            !m_basicFeature->m_vp9PicParams->PicFlags.fields.intra_only &&
            m_basicFeature->m_presLastRefSurface != nullptr &&
            m_basicFeature->m_presGoldenRefSurface != nullptr &&
            m_basicFeature->m_presAltRefSurface != nullptr)
        {
            if (Mos_ResourceIsNull(m_basicFeature->m_presLastRefSurface))
            {
                m_basicFeature->m_presLastRefSurface = usedDummyReference;
            }
            if (Mos_ResourceIsNull(m_basicFeature->m_presGoldenRefSurface))
            {
                m_basicFeature->m_presGoldenRefSurface = usedDummyReference;
            }
            if (Mos_ResourceIsNull(m_basicFeature->m_presAltRefSurface))
            {
                m_basicFeature->m_presAltRefSurface = usedDummyReference;
            }

            //MOS_SURFACE lastRefSurface;
            DECODE_CHK_STATUS(MOS_SecureMemcpy(
                &(m_basicFeature->m_lastRefSurface.OsResource),
                sizeof(MOS_RESOURCE),
                m_basicFeature->m_presLastRefSurface,
                sizeof(MOS_RESOURCE)));
            DECODE_CHK_STATUS(CodecUtilities::CodecHalGetResourceInfo(
                m_basicFeature->m_osInterface,
                &m_basicFeature->m_lastRefSurface));

            //MOS_SURFACE goldenRefSurface;
            DECODE_CHK_STATUS(MOS_SecureMemcpy(
                &m_basicFeature->m_goldenRefSurface.OsResource,
                sizeof(MOS_RESOURCE),
                m_basicFeature->m_presGoldenRefSurface,
                sizeof(MOS_RESOURCE)));
            DECODE_CHK_STATUS(CodecUtilities::CodecHalGetResourceInfo(
                m_basicFeature->m_osInterface,
                &m_basicFeature->m_goldenRefSurface));

            //MOS_SURFACE altRefSurface;
            DECODE_CHK_STATUS(MOS_SecureMemcpy(
                &m_basicFeature->m_altRefSurface.OsResource,
                sizeof(MOS_RESOURCE),
                m_basicFeature->m_presAltRefSurface,
                sizeof(MOS_RESOURCE)));
            DECODE_CHK_STATUS(CodecUtilities::CodecHalGetResourceInfo(
                m_basicFeature->m_osInterface,
                &m_basicFeature->m_altRefSurface));

            return MOS_STATUS_SUCCESS;
        }
        return MOS_STATUS_SUCCESS;
    }

    const std::vector<uint8_t> &Vp9ReferenceFrames::GetActiveReferenceList(CODEC_VP9_PIC_PARAMS &picParams)
    {
        DECODE_FUNC_CALL();

        m_activeReferenceList.clear();
        for (auto i = 0; i < CODECHAL_MAX_CUR_NUM_REF_FRAME_VP9; i++)
        {
            m_activeReferenceList.push_back(picParams.RefFrameList[i].FrameIdx);
        }

        return m_activeReferenceList;
    }

    PMOS_RESOURCE Vp9ReferenceFrames::GetReferenceByFrameIndex(uint8_t frameIndex)
    {
        DECODE_FUNC_CALL();

        if (frameIndex >= CODECHAL_NUM_UNCOMPRESSED_SURFACE_VP9)
        {
            DECODE_ASSERTMESSAGE("Invalid reference frame index");
            return nullptr;
        }

        PCODEC_REF_LIST ref = m_vp9RefList[frameIndex];

        if (ref == nullptr || m_allocator->ResourceIsNull(&(ref->resRefPic)))
        {
            return nullptr;
        }

        return &(ref->resRefPic);
    }


    PMOS_RESOURCE Vp9ReferenceFrames::GetValidReference()
    {
        DECODE_FUNC_CALL();

        if (m_basicFeature->m_vp9PicParams == nullptr)
        {
            return nullptr;
        }
        auto m_picParams = m_basicFeature->m_vp9PicParams;

        for (auto i = 0; i < CODECHAL_DECODE_VP9_MAX_NUM_REF_FRAME; i++)
        {  
            uint8_t frameIdx = m_picParams->RefFrameList[i].FrameIdx;
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

    MOS_STATUS Vp9ReferenceFrames::UpdateCurResource(const CODEC_VP9_PIC_PARAMS& picParams)
    {
        DECODE_FUNC_CALL();

        PCODEC_REF_LIST destEntry    = m_vp9RefList[picParams.CurrPic.FrameIdx];
        destEntry->resRefPic         = m_basicFeature->m_destSurface.OsResource;
        destEntry->dwFrameWidth      = picParams.FrameWidthMinus1 + 1;
        destEntry->dwFrameHeight     = picParams.FrameHeightMinus1 + 1;

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Vp9ReferenceFrames::UpdateCurFrame(const CODEC_VP9_PIC_PARAMS &picParams)
    {
        DECODE_FUNC_CALL();

        DECODE_CHK_COND(picParams.CurrPic.FrameIdx >= CODECHAL_NUM_UNCOMPRESSED_SURFACE_VP9,
                        "Invalid frame index of current frame");
        m_currRefList = m_vp9RefList[picParams.CurrPic.FrameIdx];
        MOS_ZeroMemory(m_currRefList, sizeof(CODEC_REF_LIST));

        // Overwrite the actual surface height with the coded height and width of the frame
        // for VP9 since it's possible for a VP9 frame to change size during playback
        DECODE_CHK_STATUS(UpdateCurResource(picParams));
        m_currRefList->RefPic = picParams.CurrPic;

        for (auto i = 0; i < CODECHAL_DECODE_VP9_MAX_NUM_REF_FRAME; i++)
        {
            m_currRefList->RefList[i] = picParams.RefFrameList[i];
        }
         
        return MOS_STATUS_SUCCESS;
    }

}  // namespace decode
