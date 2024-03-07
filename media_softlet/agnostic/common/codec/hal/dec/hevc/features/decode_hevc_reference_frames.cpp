/*
* Copyright (c) 2019, Intel Corporation
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
//! \file     decode_hevc_reference_frames.cpp
//! \brief    Defines reference list related logic for hevc decode
//!

#include "decode_hevc_basic_feature.h"
#include "decode_utils.h"
#include "codec_utilities_next.h"
#include "decode_hevc_reference_frames.h"
#include "codec_def_decode_hevc.h"

namespace decode
{

HevcReferenceFrames::HevcReferenceFrames()
{
    memset(m_refList, 0, sizeof(m_refList));
}

MOS_STATUS HevcReferenceFrames::Init(HevcBasicFeature *basicFeature, DecodeAllocator& allocator)
{
    DECODE_FUNC_CALL();
    DECODE_CHK_NULL(basicFeature);

    m_basicFeature = basicFeature;
    m_allocator = &allocator;
    DECODE_CHK_STATUS(CodecUtilities::CodecHalAllocateDataList(m_refList, CODECHAL_NUM_UNCOMPRESSED_SURFACE_HEVC));

    m_osInterface = basicFeature->GetOsInterface();

    return MOS_STATUS_SUCCESS;
}

HevcReferenceFrames::~HevcReferenceFrames()
{
    DECODE_FUNC_CALL();
    CodecUtilities::CodecHalFreeDataList(m_refList, CODECHAL_NUM_UNCOMPRESSED_SURFACE_HEVC);
    m_activeReferenceList.clear();
}

MOS_STATUS HevcReferenceFrames::UpdatePicture(CODEC_HEVC_PIC_PARAMS & picParams, bool isSCCIBCMode)
{
    DECODE_FUNC_CALL();

    DECODE_CHK_STATUS(DetectPocDuplication(picParams.PicOrderCntValList, picParams.RefFrameList));
    DECODE_CHK_STATUS(UpdateCurFrame(picParams, isSCCIBCMode));
    DECODE_CHK_STATUS(UpdateCurRefList(picParams, isSCCIBCMode));
    DECODE_CHK_STATUS(UpdateRefIdxMapping(picParams, isSCCIBCMode));
    DECODE_CHK_STATUS(UpdateRefCachePolicy(picParams));

    return MOS_STATUS_SUCCESS;
}

const std::vector<uint8_t> & HevcReferenceFrames::GetActiveReferenceList(const CODEC_HEVC_PIC_PARAMS & picParams)
{
    DECODE_FUNC_CALL();

    m_activeReferenceList.clear();

    for (auto i = 0; i < CODEC_MAX_NUM_REF_FRAME_HEVC; i++)
    {
        if (m_frameUsedAsCurRef[i])
        {
            m_activeReferenceList.push_back(picParams.RefFrameList[i].FrameIdx);
        }
    }

    return m_activeReferenceList;
}

PMOS_RESOURCE HevcReferenceFrames::GetReferenceByFrameIndex(uint8_t frameIndex)
{
    DECODE_FUNC_CALL();

    if (frameIndex >= CODECHAL_NUM_UNCOMPRESSED_SURFACE_HEVC)
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

PMOS_RESOURCE HevcReferenceFrames::GetValidReference()
{
    DECODE_FUNC_CALL();

    if (m_basicFeature->m_hevcPicParams == nullptr)
    {
        return nullptr;
    }
    auto picParams = m_basicFeature->m_hevcPicParams;

    for(uint32_t i = 0; i < CODEC_MAX_NUM_REF_FRAME_HEVC; i++)
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

MOS_STATUS HevcReferenceFrames::UpdateCurResource(const CODEC_HEVC_PIC_PARAMS &picParams, bool isSCCIBCMode)
{
    DECODE_FUNC_CALL();

    PCODEC_REF_LIST destEntry = m_refList[picParams.CurrPic.FrameIdx];

    if (isSCCIBCMode)
    {
        bool twoVersionsOfCurrDecPicFlag = (!picParams.pps_deblocking_filter_disabled_flag) ||
                                           picParams.sample_adaptive_offset_enabled_flag ||
                                           picParams.deblocking_filter_override_enabled_flag;
#ifdef _MMC_SUPPORTED
        // Due to limitation, IBC reference has to be uncompressed, while recon surface is still compressed.
        // Always need internal surface for IBC reference while MMC is enabled.
        if (m_basicFeature->IsMmcEnabled())
        {
            twoVersionsOfCurrDecPicFlag = true;
        }
#endif
        if (twoVersionsOfCurrDecPicFlag)
        {
            // In downsampling case, m_referenceBeforeLoopFilter will be allocated by downsampling feature later.
            if(m_basicFeature->m_referenceBeforeLoopFilter != nullptr)
            {
                destEntry->resRefPic = m_basicFeature->m_referenceBeforeLoopFilter->OsResource;
            }
        }
        else
        {
            destEntry->resRefPic = m_basicFeature->m_destSurface.OsResource;
        }
    }
    else
    {
        destEntry->resRefPic = m_basicFeature->m_destSurface.OsResource;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcReferenceFrames::UpdateCurFrame(const CODEC_HEVC_PIC_PARAMS & picParams, bool isSCCIBCMode)
{
    DECODE_FUNC_CALL();

    DECODE_CHK_COND(picParams.CurrPic.FrameIdx >= CODECHAL_NUM_UNCOMPRESSED_SURFACE_HEVC,
                    "Invalid frame index of current frame");
    PCODEC_REF_LIST destEntry = m_refList[picParams.CurrPic.FrameIdx];
    MOS_ZeroMemory(destEntry, sizeof(CODEC_REF_LIST));

    DECODE_CHK_STATUS(UpdateCurResource(picParams, isSCCIBCMode));

    if (isSCCIBCMode)
    {
        m_curIsIntra = false;
    }
    else
    {
        m_curIsIntra = !IsCurFrameUseReference(picParams);
    }

    destEntry->RefPic            = picParams.CurrPic;
    destEntry->sFrameNumber      = int16_t(picParams.CurrPicOrderCntVal);
    destEntry->iFieldOrderCnt[0] = picParams.CurrPicOrderCntVal;
    destEntry->bIsIntra          = m_curIsIntra;

    for(auto i = 0; i < CODEC_MAX_NUM_REF_FRAME_HEVC; i++)
    {
        destEntry->RefList[i] = picParams.RefFrameList[i];
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcReferenceFrames::UpdateCurRefList(const CODEC_HEVC_PIC_PARAMS & picParams, bool isSCCIBCMode)
{
    DECODE_FUNC_CALL();

    // Override reference list with ref surface passed from DDI if needed
    uint8_t surfCount = 0;
    uint8_t surfIndex = 0;
    if (m_osInterface->pfnIsMismatchOrderProgrammingSupported())
    {
        while (surfIndex < CODECHAL_NUM_UNCOMPRESSED_SURFACE_HEVC)
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
        while (surfCount < m_basicFeature->m_refSurfaceNum && surfIndex < CODECHAL_NUM_UNCOMPRESSED_SURFACE_HEVC)
        {
            if (!m_allocator->ResourceIsNull(&m_basicFeature->m_refFrameSurface[surfIndex].OsResource))
            {
                m_refList[surfIndex]->resRefPic = m_basicFeature->m_refFrameSurface[surfIndex].OsResource;
                surfCount++;
            }
            surfIndex++;
        }
    }

    memset(m_frameUsedAsCurRef, 0, sizeof(m_frameUsedAsCurRef));

    for (auto i = 0; i < CODECHAL_MAX_CUR_NUM_REF_FRAME_HEVC; i++)
    {
        uint8_t index = picParams.RefPicSetStCurrBefore[i];
        if (index < CODEC_MAX_NUM_REF_FRAME_HEVC)
        {
            m_frameUsedAsCurRef[index] = true;
        }

        index = picParams.RefPicSetStCurrAfter[i];
        if (index < CODEC_MAX_NUM_REF_FRAME_HEVC)
        {
            m_frameUsedAsCurRef[index] = true;
        }

        index = picParams.RefPicSetLtCurr[i];
        if (index < CODEC_MAX_NUM_REF_FRAME_HEVC)
        {
            m_frameUsedAsCurRef[index] = true;
        }
    }

    if (isSCCIBCMode)
    {
        for (auto i = 0; i < CODEC_MAX_NUM_REF_FRAME_HEVC; i++)
        {
            if (picParams.PicOrderCntValList[i] == picParams.CurrPicOrderCntVal)
            {
                m_frameUsedAsCurRef[i] = !CodecHal_PictureIsInvalid(picParams.RefFrameList[i]);
                break;
            }
        }
    }
    else
    {
        uint8_t refCurrIndex   = -1;
        uint8_t refBeforeIndex = -1;
        uint8_t refAfterIndex  = -1;

        for (auto i = 0; i < CODECHAL_MAX_CUR_NUM_REF_FRAME_HEVC; i++)
        {
            uint8_t indexCurr   = picParams.RefPicSetLtCurr[i];
            uint8_t indexBefore = picParams.RefPicSetStCurrBefore[i];
            uint8_t indexAfter  = picParams.RefPicSetStCurrAfter[i];

            if (indexCurr < CODEC_MAX_NUM_REF_FRAME_HEVC)
            {
                refCurrIndex = picParams.RefFrameList[indexCurr].FrameIdx;
            }
            if (indexBefore < CODEC_MAX_NUM_REF_FRAME_HEVC)
            {
                refBeforeIndex = picParams.RefFrameList[indexBefore].FrameIdx;
            }
            if (indexAfter < CODEC_MAX_NUM_REF_FRAME_HEVC)
            {
                refAfterIndex = picParams.RefFrameList[indexAfter].FrameIdx;
            }

            if ((refCurrIndex == picParams.CurrPic.FrameIdx) || (refBeforeIndex == picParams.CurrPic.FrameIdx) || (refAfterIndex == picParams.CurrPic.FrameIdx))
            {
                return MOS_STATUS_INVALID_PARAMETER;
            }
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcReferenceFrames::UpdateRefIdxMapping(const CODEC_HEVC_PIC_PARAMS & picParams, bool isSCCIBCMode)
{
    DECODE_FUNC_CALL();

    memset(m_refIdxMapping, -1, sizeof(m_refIdxMapping));
    m_IBCRefIdx = 0;

    uint8_t curRefIdx = 0;
    for (auto i = 0; i < CODEC_MAX_NUM_REF_FRAME_HEVC; i++)
    {
        if (m_frameUsedAsCurRef[i])
        {
            if (isSCCIBCMode &&
                picParams.PicOrderCntValList[i] == picParams.CurrPicOrderCntVal)
            {
                // pre-dbk reference id for IBC mode
                m_IBCRefIdx = curRefIdx;
            }
            m_refIdxMapping[i] = curRefIdx++;
        }
    }

    DECODE_CHK_COND(curRefIdx > 8, "Invalid reference number for current frame");

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcReferenceFrames::UpdateRefCachePolicy(const CODEC_HEVC_PIC_PARAMS &picParams)
{
    DECODE_FUNC_CALL();
    MOS_STATUS sts = MOS_STATUS_SUCCESS;

    HevcReferenceFrames        &refFrames     = m_basicFeature->m_refFrames;
    const std::vector<uint8_t> &activeRefList = refFrames.GetActiveReferenceList(picParams);
    if (!refFrames.m_curIsIntra)
    {
        for (uint8_t i = 0; i < activeRefList.size(); i++)
        {
            uint8_t frameIdx = activeRefList[i];
            if (frameIdx >= CODECHAL_NUM_UNCOMPRESSED_SURFACE_HEVC)
            {
                continue;
            }
            sts = m_allocator->UpdateResoreceUsageType(&m_refList[frameIdx]->resRefPic, resourceInputReference);
            if (sts != MOS_STATUS_SUCCESS)
            {
                DECODE_NORMALMESSAGE("GetReferenceByFrameIndex invalid\n");
            }
        }
    }

    return MOS_STATUS_SUCCESS;
}

bool HevcReferenceFrames::IsCurFrameUseReference(const CODEC_HEVC_PIC_PARAMS & picParams)
{
    DECODE_FUNC_CALL();

    bool useRef = false;
    if (picParams.IntraPicFlag == 0)
    {
        for (uint32_t i = 0; i < CODECHAL_MAX_CUR_NUM_REF_FRAME_HEVC; i++)
        {
            uint8_t index = picParams.RefPicSetStCurrBefore[i];
            if (index < CODEC_MAX_NUM_REF_FRAME_HEVC)
            {
                useRef = true;
                break;
            }

            index = picParams.RefPicSetStCurrAfter[i];
            if (index < CODEC_MAX_NUM_REF_FRAME_HEVC)
            {
                useRef = true;
                break;
            }

            index = picParams.RefPicSetLtCurr[i];
            if (index < CODEC_MAX_NUM_REF_FRAME_HEVC)
            {
                useRef = true;
                break;
            }
        }
    }

    return useRef;
}

MOS_STATUS HevcReferenceFrames::DetectPocDuplication(const int32_t (&picOrderCntValList)[CODEC_MAX_NUM_REF_FRAME_HEVC],
                                                     CODEC_PICTURE (&refFrameList)[CODEC_MAX_NUM_REF_FRAME_HEVC])
{
    DECODE_FUNC_CALL();

    for (auto i = 0; i < CODEC_MAX_NUM_REF_FRAME_HEVC; i++)
    {
        m_duplicationPocMap[i].clear();
    }

    bool pocListFilter[CODEC_MAX_NUM_REF_FRAME_HEVC]; // Marker to indicate if poc already be filtered
    memset(pocListFilter, 0, sizeof(pocListFilter));

    for (int8_t m = 0; m < CODEC_MAX_NUM_REF_FRAME_HEVC; m++)
    {
        if (pocListFilter[m] || (picOrderCntValList[m] == m_invalidPocValue))
        {
            continue;
        }
        pocListFilter[m] = true;

        for (int8_t n = m + 1; n < CODEC_MAX_NUM_REF_FRAME_HEVC; n++)
        {
            if (picOrderCntValList[m] == picOrderCntValList[n])
            {
                pocListFilter[n] = true;
                m_duplicationPocMap[m].push_back(n);

                refFrameList[n].PicFlags = PICTURE_INVALID;
            }
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcReferenceFrames::FixSliceRefList(const CODEC_HEVC_PIC_PARAMS & picParams,
                                                CODEC_HEVC_SLICE_PARAMS & slc)
{
    DECODE_FUNC_CALL();

    for (auto i = 0; i < CODEC_MAX_NUM_REF_FRAME_HEVC; i++)
    {
        if (m_duplicationPocMap[i].size() == 0)
        {
            continue;
        }

        for (auto dupIdx : m_duplicationPocMap[i])
        {
            for (auto k = 0; k < 2; k++)
            {
                for (auto j = 0; j < CODEC_MAX_NUM_REF_FRAME_HEVC; j++)
                {
                    if (slc.RefPicList[k][j].FrameIdx == picParams.RefFrameList[dupIdx].FrameIdx)
                    {
                        slc.RefPicList[k][j].FrameIdx = picParams.RefFrameList[i].FrameIdx;
                        slc.RefPicList[k][j].PicEntry = picParams.RefFrameList[i].PicEntry;
                        slc.RefPicList[k][j].PicFlags = picParams.RefFrameList[i].PicFlags;
                    }
                }
            }
        }
    }

    return MOS_STATUS_SUCCESS;
}

}  // namespace decode
