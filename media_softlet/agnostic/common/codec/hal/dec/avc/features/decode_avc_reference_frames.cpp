/*
* Copyright (c) 2020, Intel Corporation
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
//! \file     decode_avc_reference_frames.cpp
//! \brief    Defines reference list related logic for avc decode
//!

#include "decode_avc_basic_feature.h"
#include "decode_utils.h"
#include "codec_utilities_next.h"
#include "decode_avc_reference_frames.h"
#include "codec_def_decode_avc.h"

namespace decode
{

    AvcReferenceFrames::AvcReferenceFrames()
    {
        MOS_ZeroMemory(m_refList, sizeof(PCODEC_REF_LIST)* CODEC_AVC_NUM_UNCOMPRESSED_SURFACE);
        MOS_ZeroMemory(&m_prevPic, sizeof(m_prevPic));
        MOS_ZeroMemory(&m_avcPicIdx, (sizeof(CODEC_PIC_ID) * CODEC_AVC_MAX_NUM_REF_FRAME));
        MOS_ZeroMemory(&m_avcFrameStoreId, (sizeof(CODEC_AVC_FRAME_STORE_ID) * CODEC_AVC_MAX_NUM_REF_FRAME));
    }

    MOS_STATUS AvcReferenceFrames::Init(AvcBasicFeature *basicFeature, DecodeAllocator& allocator)
    {
        DECODE_FUNC_CALL();
        DECODE_CHK_NULL(basicFeature);

        m_basicFeature = basicFeature;
        m_allocator = &allocator;
        DECODE_CHK_STATUS(CodecUtilities::CodecHalAllocateDataList(m_refList, CODEC_AVC_NUM_UNCOMPRESSED_SURFACE));
        m_prevPic.PicFlags = PICTURE_INVALID;
        m_prevPic.FrameIdx = CODEC_AVC_NUM_UNCOMPRESSED_SURFACE;
        m_osInterface = basicFeature->GetOsInterface();

        return MOS_STATUS_SUCCESS;
    }

    AvcReferenceFrames::~AvcReferenceFrames()
    {
        DECODE_FUNC_CALL();
        CodecUtilities::CodecHalFreeDataList(m_refList, CODEC_AVC_NUM_UNCOMPRESSED_SURFACE);
        m_activeReferenceList.clear();
    }

    MOS_STATUS AvcReferenceFrames::UpdatePicture(CODEC_AVC_PIC_PARAMS & picParams)
    {
        DECODE_FUNC_CALL();

        DECODE_CHK_STATUS(UpdateCurFrame(picParams));
        DECODE_CHK_STATUS(UpdateCurRefList(picParams));
        DECODE_CHK_STATUS(UpdateRefCachePolicy(picParams));

        return MOS_STATUS_SUCCESS;
    }

    const std::vector<uint8_t> & AvcReferenceFrames::GetActiveReferenceList(const CODEC_AVC_PIC_PARAMS & picParams)
    {
        DECODE_FUNC_CALL();

        m_activeReferenceList.clear();

        for (auto i = 0; i < CODEC_AVC_MAX_NUM_REF_FRAME; i++)
        {
            if (!CodecHal_PictureIsInvalid(picParams.RefFrameList[i]))
            {
                if (picParams.RefFrameList[i].FrameIdx >= m_basicFeature->m_maxFrameIndex)
                {
                    continue;
                }
                m_activeReferenceList.push_back(picParams.RefFrameList[i].FrameIdx);
            }
        }

        return m_activeReferenceList;
    }

    PMOS_RESOURCE AvcReferenceFrames::GetReferenceByFrameIndex(uint8_t frameIndex)
    {
        DECODE_FUNC_CALL();

        if (frameIndex >= CODEC_AVC_NUM_UNCOMPRESSED_SURFACE)
        {
            DECODE_ASSERTMESSAGE("Invalid reference frame index");
            return nullptr;
        }

        PCODEC_REF_LIST ref = m_refList[frameIndex];

        if (ref == nullptr || m_allocator->ResourceIsNull(&(ref->resRefPic)))
        {
            return nullptr;
        }

        return &(ref->resRefPic);
    }

    PMOS_RESOURCE AvcReferenceFrames::GetValidReference()
    {
        DECODE_FUNC_CALL();

        if (m_basicFeature->m_avcPicParams == nullptr)
        {
            return nullptr;
        }
        auto picParams = m_basicFeature->m_avcPicParams;

        for (uint32_t i = 0; i < CODEC_AVC_MAX_NUM_REF_FRAME; i++)
        {
            uint8_t frameIdx = picParams->RefFrameList[i].FrameIdx;
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

     MOS_STATUS AvcReferenceFrames::UpdateCurResource(const CODEC_AVC_PIC_PARAMS &picParams)
    {
        DECODE_FUNC_CALL();

        PCODEC_REF_LIST destEntry = m_refList[picParams.CurrPic.FrameIdx];
        destEntry->resRefPic = m_basicFeature->m_destSurface.OsResource;

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AvcReferenceFrames::UpdateCurFrame(const CODEC_AVC_PIC_PARAMS & picParams)
    {
        DECODE_FUNC_CALL();

        CODEC_PICTURE currPic = picParams.CurrPic;
        DECODE_CHK_COND(currPic.FrameIdx >= CODEC_AVC_NUM_UNCOMPRESSED_SURFACE,
                        "Invalid frame index of current frame");
        PCODEC_REF_LIST destEntry = m_refList[currPic.FrameIdx];

        DECODE_CHK_STATUS(UpdateCurResource(picParams));

        //MVC related inter-view reference
        destEntry->bUsedAsInterViewRef = false;
        if (m_basicFeature->m_mvcExtPicParams)
        {
            if (m_basicFeature->m_mvcExtPicParams->inter_view_flag)
            {
                destEntry->bUsedAsInterViewRef = true;
            }
        }

        if (!m_basicFeature->m_isSecondField)
        {
            destEntry->ucDMVIdx[0]=0;
            destEntry->ucDMVIdx[1]=0;
        }

        for (auto i = 0; i < CODEC_AVC_MAX_NUM_REF_FRAME; i++)
        {
            m_avcFrameStoreId[i].inUse = false;
        }

        destEntry->RefPic = currPic;

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AvcReferenceFrames::UpdateCurRefList(const CODEC_AVC_PIC_PARAMS &picParams)
    {
        DECODE_FUNC_CALL();

        CODEC_PICTURE currPic = picParams.CurrPic;

        if (currPic.PicFlags != m_prevPic.PicFlags && currPic.FrameIdx == m_prevPic.FrameIdx)
        {
            m_basicFeature->m_isSecondField = true;
        }
        else
        {
            m_basicFeature->m_isSecondField = false;
        }

        // Override reference list with ref surface passed from DDI
        uint8_t surfCount = 0;
        uint8_t surfIndex = 0;
        if (m_osInterface->pfnIsMismatchOrderProgrammingSupported())
        {
            while (surfIndex < CODEC_AVC_NUM_UNCOMPRESSED_SURFACE)
            {
                if (!m_allocator->ResourceIsNull(&m_basicFeature->m_refFrameSurface[surfIndex].OsResource))
                {
                    m_refList[surfIndex]->resRefPic = m_basicFeature->m_refFrameSurface[surfIndex].OsResource;
                }
                surfIndex++;
            }
        }
        else
        {
            while (surfCount < m_basicFeature->m_refSurfaceNum && surfIndex < CODEC_AVC_NUM_UNCOMPRESSED_SURFACE)
            {
                if (!m_allocator->ResourceIsNull(&m_basicFeature->m_refFrameSurface[surfIndex].OsResource))
                {
                    m_refList[surfIndex]->resRefPic = m_basicFeature->m_refFrameSurface[surfIndex].OsResource;
                    surfCount++;
                }
                surfIndex++;
            }
        }


        for (uint8_t i = 0; i < CODEC_AVC_MAX_NUM_REF_FRAME; i++)
        {
            if (!CodecHal_PictureIsInvalid(picParams.RefFrameList[i]))
            {
                auto index = picParams.RefFrameList[i].FrameIdx;
                m_refList[index]->sFrameNumber = picParams.FrameNumList[i];
            }
        }

        for (uint8_t i = 0; i < CODEC_AVC_MAX_NUM_REF_FRAME; i++)
        {
            m_avcPicIdx[i].bValid =false;
            auto index = picParams.RefFrameList[i].FrameIdx;

            if (!CodecHal_PictureIsInvalid(picParams.RefFrameList[i]))
            {
                if (m_allocator->ResourceIsNull(&m_refList[index]->resRefPic))
                {
                    if (picParams.pic_fields.IntraPicFlag)
                    {
                        m_refList[index]->RefPic = picParams.RefFrameList[i];
                        m_refList[index]->resRefPic = m_basicFeature->m_dummyReference.OsResource;
                        m_refList[index]->ucDMVIdx[0] = m_refList[currPic.FrameIdx]->ucDMVIdx[0];
                    }
                }

                bool isDuplicateIdx = false;
                for (uint8_t j = 0; j < i; j++)
                {
                    if (m_avcPicIdx[j].bValid && index == picParams.RefFrameList[j].FrameIdx)
                    {
                        isDuplicateIdx = 1;
                        break;
                    }
                }
                if (isDuplicateIdx)
                {
                    continue;
                }

                m_refList[index]->RefPic.PicFlags = CodecHal_CombinePictureFlags(m_refList[index]->RefPic,
                    picParams.RefFrameList[i]);
                m_refList[index]->iFieldOrderCnt[0] = picParams.FieldOrderCntList[i][0];
                m_refList[index]->iFieldOrderCnt[1] = picParams.FieldOrderCntList[i][1];
                m_avcPicIdx[i].bValid = true;
                m_avcPicIdx[i].ucPicIdx = index;

                // Comparing curPic's DPB and prevPic's DPB, if find same reference frame index, will reuse
                //prevPic's ucFrameId, otherwise, need assign a new ucFrameId in SetFrameStoreIds function.
                if (!CodecHal_PictureIsInvalid(m_prevPic) && !m_osInterface->pfnIsMismatchOrderProgrammingSupported())
                {
                    uint8_t ii;
                    for (ii = 0; ii < m_refList[m_prevPic.FrameIdx]->ucNumRef; ii++)
                    {
                        if (index == m_refList[m_prevPic.FrameIdx]->RefList[ii].FrameIdx)
                        {
                            if (m_refList[index]->ucFrameId == 0x7f)
                            {
                                DECODE_ASSERTMESSAGE("Invaid Ref Frame Id Found");
                                m_refList[index]->ucFrameId = 0;
                            }
                            m_avcFrameStoreId[m_refList[index]->ucFrameId].inUse = true;
                            break;
                        }
                    }
                    if (ii == m_refList[m_prevPic.FrameIdx]->ucNumRef)
                    {
                        m_refList[index]->ucFrameId = 0x7f;
                    }
                }
            }
        }

        // Save the current RefList and corresponding reference frame flags
        uint16_t nonExistingFrameFlags = 0;
        uint32_t usedForReferenceFlags = 0;
        uint8_t refIdx = 0;
        for (uint8_t i = 0; i < CODEC_AVC_MAX_NUM_REF_FRAME; i++)
        {
            if (m_avcPicIdx[i].bValid)
            {
                m_refList[currPic.FrameIdx]->RefList[refIdx] = picParams.RefFrameList[i];
                nonExistingFrameFlags |= ((picParams.NonExistingFrameFlags >> i) & 1) << refIdx;
                usedForReferenceFlags |= ((picParams.UsedForReferenceFlags >> (i * 2)) & 3) << (refIdx * 2);
                refIdx++;
            }
        }

        m_refList[currPic.FrameIdx]->ucNumRef = refIdx;
        m_refList[currPic.FrameIdx]->usNonExistingFrameFlags = nonExistingFrameFlags;
        m_refList[currPic.FrameIdx]->uiUsedForReferenceFlags = usedForReferenceFlags;

        if (m_osInterface->pfnIsMismatchOrderProgrammingSupported())
        {
            for (uint32_t ii = 0; ii < CODEC_AVC_NUM_UNCOMPRESSED_SURFACE; ii++)
            {
                m_refList[ii]->ucFrameId = 0x7f;
            }
        }

        DECODE_CHK_STATUS(SetFrameStoreIds(currPic.FrameIdx));

        // Store CurrFieldOrderCnt
        if (CodecHal_PictureIsBottomField(currPic))
        {
            m_refList[currPic.FrameIdx]->iFieldOrderCnt[1] = picParams.CurrFieldOrderCnt[1];
        }
        else
        {
            m_refList[currPic.FrameIdx]->iFieldOrderCnt[0] = picParams.CurrFieldOrderCnt[0];
            if (CodecHal_PictureIsFrame(currPic))
            {
                m_refList[currPic.FrameIdx]->iFieldOrderCnt[1] = picParams.CurrFieldOrderCnt[1];
            }
        }

        m_prevPic = currPic;

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AvcReferenceFrames::UpdateRefCachePolicy(const CODEC_AVC_PIC_PARAMS &picParams)
    {
        DECODE_FUNC_CALL();
        MOS_STATUS sts = MOS_STATUS_SUCCESS;

        AvcReferenceFrames         &refFrames     = m_basicFeature->m_refFrames;
        const std::vector<uint8_t> &activeRefList = refFrames.GetActiveReferenceList(picParams);
        for (uint8_t i = 0; i < activeRefList.size(); i++)
        {
            uint8_t frameIdx               = activeRefList[i];
            if (frameIdx >= CODEC_AVC_NUM_UNCOMPRESSED_SURFACE)
            {
                continue;
            }
            sts = m_allocator->UpdateResoreceUsageType(&m_refList[frameIdx]->resRefPic, resourceInputReference);
            if (sts != MOS_STATUS_SUCCESS)
            {
                DECODE_NORMALMESSAGE("GetReferenceByFrameIndex invalid\n");
            }
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AvcReferenceFrames::SetFrameStoreIds(uint8_t frameIdx)
    {
        const uint8_t invalidFrame = 0x7f;
        for (uint8_t i = 0; i < m_refList[frameIdx]->ucNumRef; i++)
        {
            uint8_t index = m_refList[frameIdx]->RefList[i].FrameIdx;
            if (m_refList[index]->ucFrameId == invalidFrame)
            {
                uint8_t j;
                for (j = 0; j < CODEC_AVC_MAX_NUM_REF_FRAME; j++)
                {
                    if (!m_avcFrameStoreId[j].inUse)
                    {
                        m_refList[index]->ucFrameId = j;
                        m_avcFrameStoreId[j].inUse  = true;
                        break;
                    }
                }
                if (j == CODEC_AVC_MAX_NUM_REF_FRAME)
                {
                    // should never happen, something must be wrong
                    CODECHAL_PUBLIC_ASSERT(false);
                    m_refList[index]->ucFrameId = 0;
                    m_avcFrameStoreId[0].inUse  = true;
                }
            }
        }

        return MOS_STATUS_SUCCESS;
    }

}  // namespace decode
