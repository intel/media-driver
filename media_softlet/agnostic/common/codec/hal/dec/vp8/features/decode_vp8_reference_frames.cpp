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
//! \file     decode_vp8_reference_frames.cpp
//! \brief    Defines reference list related logic for vp8 decode
//!

#include "decode_vp8_basic_feature.h"
#include "decode_utils.h"
#include "decode_vp8_reference_frames.h"
#include "codec_def_decode_vp8.h"

namespace decode
{
    Vp8ReferenceFrames::Vp8ReferenceFrames()
    {
        memset(m_vp8RefList, 0, sizeof(m_vp8RefList));
    }

    MOS_STATUS Vp8ReferenceFrames::Init(Vp8BasicFeature *basicFeature, DecodeAllocator& allocator)
    {
        DECODE_FUNC_CALL();

        DECODE_CHK_NULL(basicFeature);
        m_basicFeature = basicFeature;
        m_allocator = &allocator;
        DECODE_CHK_NULL(m_allocator);
        DECODE_CHK_STATUS(AllocateDataList(m_vp8RefList, CODECHAL_NUM_UNCOMPRESSED_SURFACE_VP8));   // CODECHAL_NUM_UNCOMPRESSED_SURFACE_VP8 128

        return MOS_STATUS_SUCCESS;
    }

    Vp8ReferenceFrames::~Vp8ReferenceFrames()
    {
        DECODE_FUNC_CALL();

        FreeDataList(m_vp8RefList, CODECHAL_NUM_UNCOMPRESSED_SURFACE_VP8);
        m_activeReferenceList.clear();
    }

    MOS_STATUS Vp8ReferenceFrames::UpdatePicture(CODEC_VP8_PIC_PARAMS &picParams)
    {
        DECODE_FUNC_CALL();

        DECODE_CHK_STATUS(UpdateCurFrame(picParams));

        // Override reference list with ref surface passed from DDI if needed
        uint8_t surfCount = 0;
        uint8_t surfIndex = 0;
        while (surfCount < m_basicFeature->m_refSurfaceNum && surfIndex < CODECHAL_NUM_UNCOMPRESSED_SURFACE_VP8)
        {
            if (!m_allocator->ResourceIsNull(&m_basicFeature->m_refFrameSurface[surfIndex].OsResource))
            {
                m_vp8RefList[surfIndex]->resRefPic = m_basicFeature->m_refFrameSurface[surfIndex].OsResource;
                surfCount++;
            }
            surfIndex++;
        }

        if (picParams.key_frame)  // reference surface should be nullptr when key_frame == true
        {
            m_basicFeature->m_LastRefSurface   = nullptr;
            m_basicFeature->m_GoldenRefSurface = nullptr;
            m_basicFeature->m_AltRefSurface    = nullptr;
        }
        else
        {
            if((Mos_ResourceIsNull(&m_vp8RefList[picParams.ucLastRefPicIndex]->resRefPic)) && (m_basicFeature->m_LastRefSurface))
            {
                m_vp8RefList[picParams.ucLastRefPicIndex]->resRefPic = *(m_basicFeature->m_LastRefSurface);
            }
            else
            {
                m_basicFeature->m_LastRefSurface = &(m_vp8RefList[picParams.ucLastRefPicIndex]->resRefPic);
            }
            if((Mos_ResourceIsNull(&m_vp8RefList[picParams.ucGoldenRefPicIndex]->resRefPic)) && (m_basicFeature->m_GoldenRefSurface))
            {
                m_vp8RefList[picParams.ucGoldenRefPicIndex]->resRefPic = *(m_basicFeature->m_GoldenRefSurface);
            }
            else
            {
                m_basicFeature->m_GoldenRefSurface = &(m_vp8RefList[picParams.ucGoldenRefPicIndex]->resRefPic);
            }
            if((Mos_ResourceIsNull(&m_vp8RefList[picParams.ucAltRefPicIndex]->resRefPic)) && (m_basicFeature->m_AltRefSurface))
            {
                m_vp8RefList[picParams.ucAltRefPicIndex]->resRefPic = *(m_basicFeature->m_AltRefSurface);
            }
            else
            {
                m_basicFeature->m_AltRefSurface = &(m_vp8RefList[picParams.ucAltRefPicIndex]->resRefPic);
            }
        }
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Vp8ReferenceFrames::UpdateCurFrame(const CODEC_VP8_PIC_PARAMS &picParams)
    {
        DECODE_FUNC_CALL();

        DECODE_CHK_COND(picParams.CurrPic.FrameIdx >= CODECHAL_NUM_UNCOMPRESSED_SURFACE_VP8,
                        "Invalid frame index of current frame");

        m_currRefList = m_vp8RefList[picParams.CurrPic.FrameIdx];

        m_currRefList->RefPic       = picParams.CurrPic;
        m_currRefList->resRefPic    = m_basicFeature->m_destSurface.OsResource;

        return MOS_STATUS_SUCCESS;
    }

}  // namespace decode
