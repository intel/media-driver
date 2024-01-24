/*
* Copyright (c) 2019-2023, Intel Corporation
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
//! \file     encode_av1_reference_frames.cpp
//! \brief    Defines reference list related logic for encode av1
//!

#include <algorithm>
#include "encode_av1_basic_feature.h"
#include "encode_utils.h"
#include "encode_av1_reference_frames.h"
#include "codec_def_encode_av1.h"
#include "codechal_debug.h"
#include "encode_av1_vdenc_pipeline.h"

namespace encode
{
MOS_STATUS Av1ReferenceFrames::Init(Av1BasicFeature *basicFeature)
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_NULL_RETURN(basicFeature);

    m_basicFeature = basicFeature;
    ENCODE_CHK_STATUS_RETURN(EncodeAllocateDataList(
        m_refList,
        CODEC_AV1_NUM_UNCOMPRESSED_SURFACE));

    return MOS_STATUS_SUCCESS;
}

Av1ReferenceFrames::~Av1ReferenceFrames()
{
    ENCODE_FUNC_CALL();

    EncodeFreeDataList(m_refList, CODEC_AV1_NUM_UNCOMPRESSED_SURFACE);
}
#if USE_CODECHAL_DEBUG_TOOL
MOS_STATUS Av1ReferenceFrames::DumpInput(Av1VdencPipeline *pipeline)
{
    ENCODE_FUNC_CALL();

    CodechalDebugInterface *debugInterface = pipeline->GetDebugInterface();
    ENCODE_CHK_NULL_RETURN(debugInterface);

    std::stringstream pipeIdxStrStream("");
    pipeIdxStrStream << "_" << (int)pipeline->GetCurrentPipe();
    std::string surfacePassName = "Pass" + std::to_string((uint32_t)pipeline->GetCurrentPass());
    surfacePassName += pipeIdxStrStream.str() + "_input";

    if (m_refFrameFlags == 0)
    {
        ENCODE_VERBOSEMESSAGE("Ref list is empty!.Only keyframe is expected.");
        return MOS_STATUS_SUCCESS;
    }
    //Dump surface
    {
        std::string reflagName;
        for (auto i = 0; i < av1NumInterRefFrames; i++)
        {
            if (m_refFrameFlags & (AV1_ENCODE_GET_REF_FALG(i)))
            {
                switch (i+1) {
                case lastFrame:
                    reflagName = "_LastRefSurf";
                    break;
                case last2Frame:
                    reflagName = "_Last2RefSurf";
                    break;
                case last3Frame:
                    reflagName = "_Last3RefSurf";
                    break;
                case bwdRefFrame:
                    reflagName = "_BWDRefSurf";
                    break;
                case goldenFrame:
                    reflagName = "_GoldenRefSurf";
                    break;
                case altRef2Frame:
                    reflagName = "_Alt2RefSurf";
                    break;
                case altRefFrame:
                    reflagName = "_AltRefSurf";
                    break;
                default:
                    reflagName = "";
                    break;
                }
                if (reflagName == "")
                {
                    continue;
                }
                ENCODE_CHK_STATUS_RETURN(debugInterface->DumpYUVSurface(
                    m_currRefPic[i],
                    CodechalDbgAttr::attrReferenceSurfaces,
                    (surfacePassName + reflagName).data()));
            }
        }
    }
    return MOS_STATUS_SUCCESS;
}
#endif
static bool MmcEnabled(MOS_MEMCOMP_STATE state)
{
    return state == MOS_MEMCOMP_RC || state == MOS_MEMCOMP_MC;
}

MOS_STATUS Av1ReferenceFrames::Update()
{
    ENCODE_FUNC_CALL();

    // initialize internal structures for current frame before set up
    m_refFrameFlags = 0;
    m_numRefFrames = 0;
    uint32_t compressionFormat = 0;

    MOS_ZeroMemory(m_currRefPic, sizeof(m_currRefPic));
    for (auto i = 0; i < CODEC_AV1_NUM_REF_FRAMES; i++)
    {
        m_picIdx[i].bValid = false;
    }

    auto picParams = m_basicFeature->m_av1PicParams;
    ENCODE_CHK_NULL_RETURN(picParams);

    auto seqParams = m_basicFeature->m_av1SeqParams;
    ENCODE_CHK_NULL_RETURN(seqParams);

    m_enable_order_hint = seqParams->CodingToolFlags.fields.enable_order_hint;
    m_orderHintBitsMinus1 = seqParams->order_hint_bits_minus_1;
    m_refFrameBiasFlagsForPak.value = 0;
    m_refFrameBiasFlagsForRefManagement.value = 0;

    // save current frame infomation into m_refList
    auto currRefIdx = picParams->CurrReconstructedPic.FrameIdx;
    m_currRefList = m_refList[currRefIdx];

    // needs to confirm later if this should come from App
    m_currRefList->bUsedAsRef = true;
    m_currRefList->sRefReconBuffer = m_basicFeature->m_reconSurface;
    m_currRefList->sRefRawBuffer = m_basicFeature->m_rawSurface;
    m_currRefList->RefPic = picParams->CurrOriginalPic;
    m_currRefList->resBitstreamBuffer = m_basicFeature->m_resBitstreamBuffer;
    m_currRefList->m_frameWidth = m_refWidth? m_refWidth : picParams->frame_width_minus1 + 1;
    m_currRefList->m_frameHeight = m_refHeight? m_refHeight : picParams->frame_height_minus1 + 1;
    m_currRefList->m_orderHint = picParams->order_hint;
    m_currRefList->m_miCols = m_basicFeature->m_miCols;
    m_currRefList->m_miRows = m_basicFeature->m_miRows;
    m_currRefList->m_segmentEnable = picParams->stAV1Segments.SegmentFlags.fields.segmentation_enabled;
    // m_currRefList->ucScalingIdx also is one critical index
    // it used to associate temperal mv buffer, down scaling surfaces.
    // it will be set in m_trackedBuf->Acquire

    // set up reference related structures
    if (!AV1_KEY_OR_INRA_FRAME(picParams->PicFlags.fields.frame_type))
    {
        ENCODE_CHK_STATUS_RETURN(SetupRefFlag()); // set up m_refFrameFlags
        ENCODE_CHK_STATUS_RETURN(SetupCurrRefPic()); // set up m_currRefPic
        ENCODE_CHK_STATUS_RETURN(ValidateLowDelayBFrame()); // validate if low delay mode
        ENCODE_CHK_STATUS_RETURN(ValidatePFrame());  // validate if P frame, P frame should be Low delay first
        ENCODE_CHK_STATUS_RETURN(SetupRefIdx()); // set up m_picIdx
    }

    m_primaryRefFrame = picParams->primary_ref_frame;
    if (m_primaryRefFrame > av1PrimaryRefNone)
    {
        ENCODE_ASSERTMESSAGE("AV1 primary reference frame number exceeds PRIMARY_REF_NONE.");
        return MOS_STATUS_INVALID_PARAMETER;
    }
    if (m_primaryRefFrame != av1PrimaryRefNone)
    {
        uint8_t refPicIndex = picParams->ref_frame_idx[m_primaryRefFrame];
        auto    refPic      = picParams->RefFrameList[refPicIndex];
        if (!CodecHal_PictureIsInvalid(refPic))
        {
            uint8_t prevFrameIdx = refPic.FrameIdx;

            m_primaryRefList = m_refList[prevFrameIdx];
        }
    }



    // Save the RefFrameList for current frame
    uint8_t ii = 0;
    for (auto i = 0; i < CODEC_AV1_NUM_REF_FRAMES; i++)
    {
        if (m_picIdx[i].bValid)
        {
            m_currRefList->RefList[ii] = picParams->RefFrameList[i];
            ii++;
        }
    }
    m_currRefList->ucNumRef = ii;

    auto currRawOrRecon = seqParams->SeqFlags.fields.UseRawReconRef ? &m_basicFeature->m_rawSurface : &m_basicFeature->m_reconSurface;
    if (m_firstValidRefPic == nullptr)
    {
        m_firstValidRefPic = currRawOrRecon;
    }
    m_basicFeature->GetSurfaceMmcInfo(m_firstValidRefPic, m_refMmcState[intraFrame], compressionFormat);
    m_refCompressionFormat = MmcEnabled(m_refMmcState[intraFrame])? compressionFormat : m_refCompressionFormat;
    return MOS_STATUS_SUCCESS;

}

MOS_STATUS Av1ReferenceFrames::UpdateRefFrameSize(uint32_t width, uint32_t height)
{
    m_refWidth  = width;
    m_refHeight = height;

    return MOS_STATUS_SUCCESS;
}

uint8_t Av1ReferenceFrames::RefFrameL0L1(CODEC_Ref_Frame_Ctrl_AV1 const &ref_frame_ctrl) const
{
    uint8_t Refs = 0;
    auto    fields = ref_frame_ctrl.RefFrameCtrl.fields;
    uint32_t tempIndex[7] = {fields.search_idx0,
                             fields.search_idx1,
                             fields.search_idx2,
                             fields.search_idx3,
                             fields.search_idx4,
                             fields.search_idx5,
                             fields.search_idx6};
    for (int i = 0; i < 7; i++)
    {
        if (tempIndex[i] >= 1 && tempIndex[i] <= 7)
        {
            Refs |= (1 << (tempIndex[i] - 1));
        }
    }

    return Refs;
}

MOS_STATUS Av1ReferenceFrames::SetupRefFlag()
{
    ENCODE_FUNC_CALL();

    auto picParams = m_basicFeature->m_av1PicParams;
    ENCODE_CHK_NULL_RETURN(picParams);
    auto ref_frame_ctrl_l0 = RefFrameL0L1(picParams->ref_frame_ctrl_l0);
    auto ref_frame_ctrl_l1 = RefFrameL0L1(picParams->ref_frame_ctrl_l1);

    m_refFrameFlags = ref_frame_ctrl_l0 | ref_frame_ctrl_l1;

    for (auto i = 0; i < av1NumInterRefFrames; i++)
    {
        if (CodecHal_PictureIsInvalid(picParams->RefFrameList[picParams->ref_frame_idx[i]]))
        {
            m_refFrameFlags &= ~(AV1_ENCODE_GET_REF_FALG(i));
        }
    }

    if (m_refFrameFlags == 0)
    {
        ENCODE_ASSERTMESSAGE("Ref list is empty!.");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Av1ReferenceFrames::SetupCurrRefPic()
{
    ENCODE_FUNC_CALL();

    auto picParams = m_basicFeature->m_av1PicParams;
    ENCODE_CHK_NULL_RETURN(picParams);
    auto seqParams = m_basicFeature->m_av1SeqParams;
    ENCODE_CHK_NULL_RETURN(seqParams);
    auto firstValid = false;
    uint32_t compressionFormat = 0;
    for (auto i = 0; i < av1NumInterRefFrames; i++)
    {
        if (m_refFrameFlags & (AV1_ENCODE_GET_REF_FALG(i)))
        {
            auto index = picParams->ref_frame_idx[i];
            auto frameIdx = picParams->RefFrameList[index].FrameIdx;
            m_refList[frameIdx]->sRefBuffer = seqParams->SeqFlags.fields.UseRawReconRef ? m_refList[frameIdx]->sRefRawBuffer :m_refList[frameIdx]->sRefReconBuffer;

            m_currRefPic[i] = &m_refList[frameIdx]->sRefBuffer;
            m_currRefPic[i]->dwWidth = m_refList[frameIdx]->m_frameWidth;
            m_currRefPic[i]->dwHeight = m_refList[frameIdx]->m_frameHeight;
            m_numRefFrames++;

            if (firstValid == false)
            {
                m_firstValidRefPic = m_currRefPic[i];
                firstValid = true;
            }

            m_currRefList->m_refOrderHint[i] = m_refList[frameIdx]->m_orderHint;
        }
    }

    // error concealment for the unset reference and mv buffers addresses
    for (auto i = 0; i < av1NumInterRefFrames; i++)
    {
        if (m_currRefPic[i] == nullptr)
        {
            m_currRefPic[i] = m_firstValidRefPic;
        }
        m_basicFeature->GetSurfaceMmcInfo(m_currRefPic[i], m_refMmcState[i + 1], compressionFormat);
        m_refCompressionFormat = MmcEnabled(m_refMmcState[i + 1]) ? compressionFormat : m_refCompressionFormat;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Av1ReferenceFrames::SetupRefIdx()
{
    ENCODE_FUNC_CALL();

    auto picParams = m_basicFeature->m_av1PicParams;
    ENCODE_CHK_NULL_RETURN(picParams);

    for (auto i = 0; i < CODEC_AV1_NUM_REF_FRAMES; i++)
    {
        if (picParams->RefFrameList[i].PicFlags != PICTURE_INVALID)
        {
            auto index = picParams->RefFrameList[i].FrameIdx;
            bool duplicatedIdx = false;
            for (auto ii = 0; ii < i; ii++)
            {
                if (m_picIdx[ii].bValid && index == picParams->RefFrameList[ii].FrameIdx)
                {
                    // we find the same FrameIdx in the ref_frame_list. Multiple reference frames are the same.
                    duplicatedIdx = true;
                    break;
                }
            }
            if (duplicatedIdx)
            {
                continue;
            }

            // this reference frame in unique. Save it into the full reference list with 127 items
            m_refList[index]->RefPic.PicFlags =
                CodecHal_CombinePictureFlags(m_refList[index]->RefPic, picParams->RefFrameList[i]);

            m_picIdx[i].bValid = true;
            m_picIdx[i].ucPicIdx = index;
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Av1ReferenceFrames::ValidateLowDelayBFrame()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    ENCODE_FUNC_CALL();

    auto picParams = m_basicFeature->m_av1PicParams;
    auto seqParams = m_basicFeature->m_av1SeqParams;
    ENCODE_CHK_NULL_RETURN(picParams);
    ENCODE_CHK_NULL_RETURN(seqParams);
    m_lowDelay = true;

    uint8_t ref_frame_ctrl_l0 = RefFrameL0L1(picParams->ref_frame_ctrl_l0);

    // Examine if now it is in the low delay mode base on order hint
    // First step: set up m_refFrameBiasFlags
    int32_t dist = 0;
    for (auto i = 0; i < av1NumInterRefFrames; i++)
    {
        dist = GetRelativeDist(m_currRefList->m_refOrderHint[i], m_currRefList->m_orderHint);
        if (dist > 0 && (m_refFrameFlags & (AV1_ENCODE_GET_REF_FALG(i))))
        {
            m_refFrameBiasFlagsForPak.value |= AV1_ENCODE_GET_REF_FALG(i);
            if (seqParams->GopRefDist > 1 && (ref_frame_ctrl_l0 & AV1_ENCODE_GET_REF_FALG(i)) == 0)
                m_refFrameBiasFlagsForRefManagement.value |= AV1_ENCODE_GET_REF_FALG(i);
        }
    }

    // collorated RefFrameBiasFlags = 0, means it's before current frame.
    // collorated RefFrameBiasFlags = 1, means it's after current frame.
    // if all references' bias flag are false, then it's low delay case.
    // otherwise, it's random access case
    if (m_refFrameFlags & m_refFrameBiasFlagsForRefManagement.value)
    {
        m_lowDelay = false;
    }

    return eStatus;
}

MOS_STATUS Av1ReferenceFrames::ValidatePFrame()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    ENCODE_FUNC_CALL();

    auto picParams = m_basicFeature->m_av1PicParams;
    ENCODE_CHK_NULL_RETURN(picParams);

    //P frame should be Low delay first
    if (m_lowDelay && picParams->ref_frame_ctrl_l1.RefFrameCtrl.value == 0)
    {
        m_PFrame = true;
    }
    else
    {
        m_PFrame = false;
    }

    return eStatus;
}

int32_t Av1ReferenceFrames::GetRelativeDist(int32_t a, int32_t b) const
{
    if (!m_enable_order_hint)
    {
        return 0;
    }

    int32_t bits = m_orderHintBitsMinus1 + 1;

    if ((bits < 1) || (a < 0) || (a >= (1 << bits)) || (b < 0) || (b >= (1 << bits)))
    {
        ENCODE_ASSERTMESSAGE("Invalid input parameters to get relative distance.");
        return 0;
    }

    int32_t diff = a - b;
    int32_t m = 1 << (bits - 1);
    diff = (diff & (m - 1)) - (diff & m);
    return diff;
}
inline void ConsolidateRefFlag(uint8_t &refFlag, const PCODEC_AV1_ENCODE_PICTURE_PARAMS picParams)
{
    ENCODE_CHK_NULL_NO_STATUS_RETURN(picParams);
    //consilidate the reference flag, becasue two reference frame may have the same index
    for (auto i = 0; i < av1NumInterRefFrames; i++)
    {
        auto basedFrameIdx = picParams->RefFrameList[picParams->ref_frame_idx[i]].FrameIdx;
        for (auto ii = i + 1; ii < av1NumInterRefFrames; ii++)
        {
            if ((refFlag & AV1_ENCODE_GET_REF_FALG(i)) &&
                (basedFrameIdx == picParams->RefFrameList[picParams->ref_frame_idx[ii]].FrameIdx))
            {
                // find same frame index for different ref frame type, skip larger ref frame type
                refFlag &= ~(AV1_ENCODE_GET_REF_FALG(ii));
            }
        }
    }
}
MOS_STATUS Av1ReferenceFrames::GetFwdBwdRefNum(uint8_t &fwdRefNum, uint8_t &bwdRefNum) const
{
    ENCODE_FUNC_CALL();
    auto picParams = m_basicFeature->m_av1PicParams;
    ENCODE_CHK_NULL_RETURN(picParams);
    auto    ref_frame_ctrl0 = picParams->ref_frame_ctrl_l0;
    auto    ref_frame_ctrl1 = picParams->ref_frame_ctrl_l1;

    uint8_t ref_frame_ctrl_l0 = RefFrameL0L1(ref_frame_ctrl0);
    uint8_t ref_frame_ctrl_l1 = RefFrameL0L1(ref_frame_ctrl1);
    uint8_t RefFrameBiasFlags = m_refFrameBiasFlagsForRefManagement.value;

    fwdRefNum = 0;
    bwdRefNum = 0;

    ConsolidateRefFlag(ref_frame_ctrl_l0, picParams);

    for (auto i = 0; i < av1NumInterRefFrames; i++)
    {
        uint8_t mask = AV1_ENCODE_GET_REF_FALG(i);
        if ((ref_frame_ctrl_l0 & mask) && !(RefFrameBiasFlags & mask))
        {
            fwdRefNum++;
        }
        if ((ref_frame_ctrl_l1 & mask) && (RefFrameBiasFlags & mask))
        {
            bwdRefNum++;
        }
    }

    if (fwdRefNum > 3 || bwdRefNum > 1 || fwdRefNum + bwdRefNum > 3)
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }

    return MOS_STATUS_SUCCESS;
}



MOS_STATUS Av1ReferenceFrames::SetPostCdefAsEncRef(bool flag)
{
    m_encUsePostCdefAsRef = flag;

    return MOS_STATUS_SUCCESS;
}

std::vector<uint8_t> Av1ReferenceFrames::GetRefScalingIdx() const
{
    std::vector<uint8_t> idxList;

    auto picParams = m_basicFeature->m_av1PicParams;

    for (auto i = 0; i < av1NumInterRefFrames; i++)
    {
        if (m_refFrameFlags & (AV1_ENCODE_GET_REF_FALG(i)))
        {
            idxList.push_back(m_refList[picParams->RefFrameList[picParams->ref_frame_idx[i]].FrameIdx]->ucScalingIdx);
        }
    }

    return idxList;
}

std::vector<PMOS_SURFACE> Av1ReferenceFrames::GetEncRefSurface() const
{
    if (!m_encUsePostCdefAsRef)
    {
        return GetPakRefSurface();
    }

    auto idxList = GetRefScalingIdx();
    std::vector<PMOS_SURFACE> ret;

    for (auto idx : idxList)
    {
        ret.push_back(m_basicFeature->m_trackedBuf->GetSurface(m_encRefBufType, idx));
    }

    return ret;
}

std::vector<PMOS_SURFACE> Av1ReferenceFrames::GetEnc4xRefSurface() const
{
    auto idxList = GetRefScalingIdx();
    std::vector<PMOS_SURFACE> ret;

    for (auto idx : idxList)
    {
        ret.push_back(m_basicFeature->m_trackedBuf->GetSurface(m_enc4xRefBufType, idx));
    }

    return ret;
}

std::vector<PMOS_SURFACE> Av1ReferenceFrames::GetEnc8xRefSurface() const
{
    auto idxList = GetRefScalingIdx();
    std::vector<PMOS_SURFACE> ret;

    for (auto idx : idxList)
    {
        ret.push_back(m_basicFeature->m_trackedBuf->GetSurface(m_enc8xRefBufType, idx));
    }

    return ret;
}

std::vector<PMOS_SURFACE> Av1ReferenceFrames::GetPakRefSurface() const
{
    std::vector<PMOS_SURFACE> refs;

    auto picParams = m_basicFeature->m_av1PicParams;

    for (auto i = 0; i < av1NumInterRefFrames; i++)
    {
        if (m_refFrameFlags & (AV1_ENCODE_GET_REF_FALG(i)))
        {
            auto frameIdx = picParams->RefFrameList[picParams->ref_frame_idx[i]].FrameIdx;
            refs.push_back(&m_refList[frameIdx]->sRefBuffer);
        }
    }

    return refs;
}

void Av1ReferenceFrames::GetFwdBwdRefPicList(CODEC_PICTURE (&refsPicList)[2][15])
{
    ENCODE_FUNC_CALL();
    auto picParams       = m_basicFeature->m_av1PicParams;
    auto ref_frame_ctrl0 = picParams->ref_frame_ctrl_l0;
    auto ref_frame_ctrl1 = picParams->ref_frame_ctrl_l1;

    uint8_t ref_frame_ctrl_l0 = RefFrameL0L1(ref_frame_ctrl0);
    uint8_t ref_frame_ctrl_l1 = RefFrameL0L1(ref_frame_ctrl1);
    uint8_t RefFrameBiasFlags = m_refFrameBiasFlagsForRefManagement.value;

    uint8_t fwdRefNum = 0;
    uint8_t bwdRefNum = 0;

    for (auto i = 0; i < av1NumInterRefFrames; i++)
    {
        uint8_t mask = AV1_ENCODE_GET_REF_FALG(i);
        if ((ref_frame_ctrl_l0 & mask) && !(RefFrameBiasFlags & mask))
        {
            auto index  = picParams->ref_frame_idx[i];
            refsPicList[0][fwdRefNum].FrameIdx = index;
            refsPicList[0][fwdRefNum].PicEntry = picParams->RefFrameList[index].PicEntry;
            refsPicList[0][fwdRefNum].PicFlags = picParams->RefFrameList[index].PicFlags;
            fwdRefNum++;
        }
        if ((ref_frame_ctrl_l1 & mask) && (RefFrameBiasFlags & mask))
        {
            auto index  = picParams->ref_frame_idx[i];
            refsPicList[1][bwdRefNum].FrameIdx = index;
            refsPicList[1][bwdRefNum].PicEntry = picParams->RefFrameList[index].PicEntry;
            refsPicList[1][bwdRefNum].PicFlags = picParams->RefFrameList[index].PicFlags;
            bwdRefNum++;
        }
    }
    uint8_t CodingType = (m_basicFeature->m_pictureCodingType == I_TYPE) ? I_TYPE : (m_basicFeature->m_ref.IsLowDelay() ? (m_basicFeature->m_ref.IsPFrame() ? P_TYPE : B_TYPE) : B_TYPE);
    if (CodingType == B_TYPE && m_lowDelay && bwdRefNum == 0)
    {
        for (int j = 0; j < fwdRefNum; j++)
        {
            refsPicList[1][j] = refsPicList[0][j];
        }
    }
}

void Av1ReferenceFrames::GetRefFramePOC(int32_t (&refsPOCList)[15], int32_t const orderHint)
{
    auto picParams = m_basicFeature->m_av1PicParams;
    for (auto i = 0; i < av1NumInterRefFrames; i++)
    {
        if (picParams->RefFrameList[i].PicFlags != PICTURE_INVALID)
        {
            auto frameIdx = picParams->RefFrameList[i].FrameIdx;
            auto dist = GetRelativeDist(m_refList[frameIdx]->m_orderHint, m_currRefList->m_orderHint);
            refsPOCList[i] = orderHint + dist;
        }
    }
}

int32_t Av1ReferenceFrames::GetFrameDisplayOrder()
{
    const auto picParams = m_basicFeature->m_av1PicParams;

    int32_t displayOrder = 0;
    if (picParams->PicFlags.fields.frame_type == keyFrame)
    {
        displayOrder = m_frameOut;
    }
    else
    {
        auto dist = GetRelativeDist(m_currRefList->m_orderHint, m_prevFrameOffset);
        displayOrder = m_prevFrameDisplayerOrder + dist;
    }
    m_prevFrameOffset = m_currRefList->m_orderHint;
    m_prevFrameDisplayerOrder = displayOrder;
    m_frameOut++;
    return displayOrder;
}

bool Av1ReferenceFrames::CheckSegmentForPrimeFrame()
{
    ENCODE_FUNC_CALL();

    bool isMatched = false;

    if (m_basicFeature == nullptr)
    {
        ENCODE_ASSERTMESSAGE("Invalid (NULL) Pointer!");
        return false;
    }

    auto picParams = m_basicFeature->m_av1PicParams;
    if (picParams == nullptr)
    {
        ENCODE_ASSERTMESSAGE("Invalid (NULL) Pointer!");
        return false;
    }

    uint8_t refPicIndex = picParams->ref_frame_idx[m_primaryRefFrame];
    auto refPic = picParams->RefFrameList[refPicIndex];
    if (!CodecHal_PictureIsInvalid(refPic))
    {
        uint8_t prevFrameIdx = refPic.FrameIdx;

        if (m_currRefList->m_miCols == m_refList[prevFrameIdx]->m_miCols &&
            m_currRefList->m_miRows == m_refList[prevFrameIdx]->m_miRows &&
            m_refList[prevFrameIdx]->m_segmentEnable)
        {
            isMatched = true;
        }
    }

    return isMatched;
}

MHW_SETPAR_DECL_SRC(VDENC_PIPE_BUF_ADDR_STATE, Av1ReferenceFrames)
{
    auto trackedBuf = m_basicFeature->m_trackedBuf;
    ENCODE_CHK_NULL_RETURN(trackedBuf);

    auto picParams = m_basicFeature->m_av1PicParams;
    ENCODE_CHK_NULL_RETURN(picParams);

    uint8_t fwdRefNum = 0, bwdRefNum = 0;
    ENCODE_CHK_STATUS_RETURN(GetFwdBwdRefNum(fwdRefNum, bwdRefNum));
    params.numActiveRefL0 = fwdRefNum;
    params.numActiveRefL1 = bwdRefNum;

    uint8_t refFlag = m_refFrameFlags;
    ConsolidateRefFlag(refFlag, picParams);

    uint8_t j = 0;
    for (auto i = 0; i < av1NumInterRefFrames; i++)
    {
        if (refFlag & (AV1_ENCODE_GET_REF_FALG(i)))
        {
            auto scalingIdx = m_refList[picParams->RefFrameList[picParams->ref_frame_idx[i]].FrameIdx]->ucScalingIdx;

            auto dsRefSurface4x = trackedBuf->GetSurface(m_enc4xRefBufType, scalingIdx);
            ENCODE_CHK_NULL_RETURN(dsRefSurface4x);

            auto dsRefSurface8x = trackedBuf->GetSurface(m_enc8xRefBufType, scalingIdx);
            ENCODE_CHK_NULL_RETURN(dsRefSurface8x);

            if (m_encUsePostCdefAsRef)
            {
                auto refPostCdefReconSurface = trackedBuf->GetSurface(m_encRefBufType, scalingIdx);
                ENCODE_CHK_NULL_RETURN(refPostCdefReconSurface);
                params.refs[j] = &refPostCdefReconSurface->OsResource;
            }
            else
            {
                params.refs[j] = &m_currRefPic[i]->OsResource;
            }

            params.refsDsStage1[j] = &dsRefSurface8x->OsResource;
            params.refsDsStage2[j] = &dsRefSurface4x->OsResource;
            j++;
        }
    }

    if (j != fwdRefNum + bwdRefNum)
    {
        ENCODE_ASSERTMESSAGE("The sum of forward reference frame number and backword reference frame number is incorrect.");

        return MOS_STATUS_INVALID_PARAMETER;
    }

    if (m_basicFeature->m_pictureCodingType != I_TYPE && picParams->primary_ref_frame != av1PrimaryRefNone)
    {
        uint8_t frameIdx = picParams->RefFrameList[picParams->ref_frame_idx[picParams->primary_ref_frame]].FrameIdx;

        uint8_t idxForTempMV = m_refList[frameIdx]->ucScalingIdx;

        params.colMvTempBuffer[0] = trackedBuf->GetBuffer(BufferType::mvTemporalBuffer, idxForTempMV);
    }

    params.lowDelayB = m_lowDelay;

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(VDENC_CMD2, Av1ReferenceFrames)
{
    ENCODE_FUNC_CALL();

    uint8_t fwdRefNum = 0;
    uint8_t bwdRefNum = 0;
    ENCODE_CHK_STATUS_RETURN(GetFwdBwdRefNum(fwdRefNum, bwdRefNum));

    params.numRefL0 = fwdRefNum;
    params.numRefL1 = bwdRefNum;

    if ((fwdRefNum == 3 && bwdRefNum == 0) || (fwdRefNum == 2 && bwdRefNum == 1) || (fwdRefNum == 3 && bwdRefNum == 3 && m_lowDelay))
    {
        m_basicFeature->m_enableNonDefaultMapping = true;
    }

    uint8_t pocForL0L1[av1NumInterRefFrames] = { 1, 2, 3, 0xff, 0xff, 0xff, 0xff };
    uint8_t frameIdxForL0L1[av1NumInterRefFrames] = { 0x7, 0x7, 0x7, 0x7, 0x7, 0x7, 0x7 };

    const auto picParams = m_basicFeature->m_av1PicParams;
    ENCODE_CHK_NULL_RETURN(picParams);

    const auto frame_type = static_cast<Av1FrameType>(picParams->PicFlags.fields.frame_type);
    params.pictureType = (frame_type == keyFrame)
        ? AV1_I_FRAME : (m_lowDelay ? (m_PFrame ? AV1_P_FRAME : AV1_GPB_FRAME) : AV1_B_FRAME);

    if (AV1_KEY_OR_INRA_FRAME(frame_type))
    {
        std::fill_n(&pocForL0L1[0], av1NumInterRefFrames, (uint8_t)0);
        std::fill_n(&frameIdxForL0L1[0], av1NumInterRefFrames, (uint8_t)0);
    }
    else if (m_enable_order_hint)
    {
        auto ref_frame_ctrl0 = picParams->ref_frame_ctrl_l0;
        auto ref_frame_ctrl1 = picParams->ref_frame_ctrl_l1;

        uint8_t ref_frame_ctrl_l0 = RefFrameL0L1(ref_frame_ctrl0);
        uint8_t ref_frame_ctrl_l1 = RefFrameL0L1(ref_frame_ctrl1);

        auto fwdRef = 0;
        for (uint8_t i = 0; i < av1NumInterRefFrames; i++)
        {
            // Function GetFwdBwdRefNum has already ensured that forward reference number is <= 2 and backward reference number is <= 1
            if ((ref_frame_ctrl_l0 & AV1_ENCODE_GET_REF_FALG(i)) &&
                !(m_refFrameBiasFlagsForRefManagement.value & AV1_ENCODE_GET_REF_FALG(i)))
            {
                auto idx = picParams->ref_frame_idx[i];
                auto frameIdx = picParams->RefFrameList[idx].FrameIdx;
                frameIdxForL0L1[fwdRef] = frameIdx;
                pocForL0L1[fwdRef++] = picParams->order_hint - m_refList[frameIdx]->m_orderHint;
            }
            if ((ref_frame_ctrl_l1 & AV1_ENCODE_GET_REF_FALG(i)) &&
                (m_refFrameBiasFlagsForRefManagement.value & AV1_ENCODE_GET_REF_FALG(i)))
            {
                auto idx = picParams->ref_frame_idx[i];
                auto frameIdx = picParams->RefFrameList[idx].FrameIdx;
                frameIdxForL0L1[3] = frameIdx;
                pocForL0L1[3] = picParams->order_hint - m_refList[frameIdx]->m_orderHint;
            }
        }
    }

    params.pocL0Ref0 = pocForL0L1[0];
    params.pocL0Ref1 = pocForL0L1[1];
    params.pocL0Ref2 = pocForL0L1[2];
    params.pocL1Ref0 = pocForL0L1[3];
    params.frameIdxL0Ref0 = frameIdxForL0L1[0];
    params.frameIdxL0Ref1 = frameIdxForL0L1[1];
    params.frameIdxL0Ref2 = frameIdxForL0L1[2];
    params.frameIdxL1Ref0 = frameIdxForL0L1[3];

    if (params.pictureType == AV1_P_FRAME)
    {
        if (params.numRefL0 == 1)
        {
            params.av1RefId[0][0] = lastFrame;
        }
        else if (params.numRefL0 == 2)
        {
            params.av1RefId[0][0] = lastFrame;
            params.av1RefId[0][1] = goldenFrame;
        }
        else
        {
            params.av1RefId[0][0] = lastFrame;
            params.av1RefId[0][1] = goldenFrame;
            params.av1RefId[0][2] = altRefFrame;
        }
    }
    else if (params.pictureType == AV1_GPB_FRAME)
    {
        params.numRefL1 = params.numRefL0;

        if (params.numRefL0 == 1)
        {
            params.av1RefId[0][0] = bwdRefFrame;
            params.av1RefId[1][0] = altRefFrame;
        }
        else if (params.numRefL0 == 2)
        {
            params.av1RefId[0][0] = lastFrame;
            params.av1RefId[0][1] = last2Frame;
            params.av1RefId[1][0] = bwdRefFrame;
            params.av1RefId[1][1] = altRef2Frame;
        }
        else
        {
            params.av1RefId[0][0] = lastFrame;
            params.av1RefId[0][1] = last2Frame;
            params.av1RefId[0][2] = last3Frame;
            params.av1RefId[1][0] = bwdRefFrame;
            params.av1RefId[1][1] = altRef2Frame;
            params.av1RefId[1][2] = altRefFrame;
        }
    }
    else if (params.pictureType == AV1_B_FRAME)
    {
        if (params.numRefL0 == 1)
        {
            params.av1RefId[0][0] = lastFrame;
            params.av1RefId[1][0] = bwdRefFrame;
        }
        else if (params.numRefL0 == 2)
        {
            params.av1RefId[0][0] = lastFrame;
            params.av1RefId[0][1] = last2Frame;
            params.av1RefId[1][0] = bwdRefFrame;
        }
        else
        {
            params.av1RefId[0][0] = lastFrame;
            params.av1RefId[0][1] = last2Frame;
            params.av1RefId[0][2] = last3Frame;
            params.av1RefId[1][0] = bwdRefFrame;
        }
    }

    CODEC_Ref_Frame_Ctrl_AV1 refCtrlL0 = m_basicFeature->m_av1PicParams->ref_frame_ctrl_l0;
    CODEC_Ref_Frame_Ctrl_AV1 refCtrlL1 = m_basicFeature->m_av1PicParams->ref_frame_ctrl_l1;
    if (m_basicFeature->m_enableNonDefaultMapping)
    {
        params.av1RefId[0][0] = refCtrlL0.RefFrameCtrl.fields.search_idx0;
        params.av1RefId[0][1] = refCtrlL0.RefFrameCtrl.fields.search_idx1;
        params.av1RefId[0][2] = refCtrlL0.RefFrameCtrl.fields.search_idx2;
        params.av1RefId[0][3] = refCtrlL0.RefFrameCtrl.fields.search_idx3;
        params.av1RefId[1][0] = refCtrlL1.RefFrameCtrl.fields.search_idx0;
        params.av1RefId[1][1] = refCtrlL1.RefFrameCtrl.fields.search_idx1;
        params.av1RefId[1][2] = refCtrlL1.RefFrameCtrl.fields.search_idx2;
        params.av1RefId[1][3] = refCtrlL1.RefFrameCtrl.fields.search_idx3;
    }

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(AVP_PIC_STATE, Av1ReferenceFrames)
{
    ENCODE_FUNC_CALL();

    params.postCdefReconPixelStreamoutEn = m_encUsePostCdefAsRef ? true : false;

    params.refFrameRes[intraFrame]    = CAT2SHORTS(m_currRefList->m_frameWidth - 1, m_currRefList->m_frameHeight - 1);
    params.refScaleFactor[intraFrame] = CAT2SHORTS(m_av1ScalingFactor, m_av1ScalingFactor);
    params.refOrderHints[intraFrame]  = m_currRefList->m_orderHint;
    params.refFrameSide               = 0;

    uint32_t horizontalScaleFactor, verticalScaleFactor;

    params.refFrameBiasFlag = m_refFrameBiasFlagsForPak.value << 1;

    for (auto i = 0; i < av1NumInterRefFrames; i++)
    {
        if (!AV1_KEY_OR_INRA_FRAME(m_basicFeature->m_av1PicParams->PicFlags.fields.frame_type))
        {
            horizontalScaleFactor = (m_currRefPic[i]->dwWidth * m_av1ScalingFactor + (m_basicFeature->m_oriFrameWidth >> 1)) / m_basicFeature->m_oriFrameWidth;
            verticalScaleFactor   = (m_currRefPic[i]->dwHeight * m_av1ScalingFactor + (m_basicFeature->m_oriFrameHeight >> 1)) / m_basicFeature->m_oriFrameHeight;

            params.refFrameRes[i + lastFrame]    = CAT2SHORTS(m_currRefPic[i]->dwWidth - 1, m_currRefPic[i]->dwHeight - 1);
            params.refScaleFactor[i + lastFrame] = CAT2SHORTS(verticalScaleFactor, horizontalScaleFactor);
            params.refOrderHints[i + lastFrame]  = m_currRefList->m_refOrderHint[i];
        }
        else
        {
            params.refFrameRes[i + lastFrame]    = params.refFrameRes[intraFrame];
            params.refScaleFactor[i + lastFrame] = params.refScaleFactor[intraFrame];
            params.refOrderHints[i + lastFrame]  = params.refOrderHints[intraFrame];
        }

        if (GetRelativeDist(params.refOrderHints[i + lastFrame], m_currRefList->m_orderHint) > 0 ||
            params.refOrderHints[i + lastFrame] == m_currRefList->m_orderHint)
        {
            params.refFrameSide |= 1 << (i + lastFrame);
        }
    }

    const auto picParams = m_basicFeature->m_av1PicParams;
    ENCODE_CHK_NULL_RETURN(picParams);
    int skipModeFrame[2] = {0};

    if ((picParams->PicFlags.fields.frame_type != keyFrame) && !m_lowDelay)
    {
        int ref_frame_offset[2] = {-1, INT_MAX};
        int ref_idx[2]          = {-1, -1};

        for (int i = lastFrame; i <= av1NumInterRefFrames; i++)
        {
            int ref_offset = params.refOrderHints[i];
            if (m_refFrameFlags & (AV1_ENCODE_GET_REF_FALG((i - 1))))
            {
                if (GetRelativeDist(ref_offset, m_currRefList->m_orderHint) < 0)
                {
                    // Forward reference
                    if (ref_frame_offset[0] == -1 || GetRelativeDist(ref_offset, ref_frame_offset[0]) > 0)
                    {
                        ref_frame_offset[0] = ref_offset;
                        ref_idx[0]          = i;
                    }
                }
                else if (GetRelativeDist(ref_offset, m_currRefList->m_orderHint) > 0)
                {
                    // Backward reference
                    if (ref_frame_offset[1] == INT_MAX || GetRelativeDist(ref_offset, ref_frame_offset[1]) < 0)
                    {
                        ref_frame_offset[1] = ref_offset;
                        ref_idx[1]          = i;
                    }
                }
            }
        }

        if (ref_idx[0] != -1 && ref_idx[1] != -1)
        {
            // == Bi-directional prediction ==
            skipModeFrame[0] = (uint8_t)(ref_idx[0] < ref_idx[1]) ? ref_idx[0] : ref_idx[1];
            skipModeFrame[1] = (uint8_t)(ref_idx[0] > ref_idx[1]) ? ref_idx[0] : ref_idx[1];
        }
        else if (ref_idx[0] != -1 && ref_idx[1] == -1)
        {
            // == Forward prediction only ==
            // Identify the second nearest forward reference.
            ref_frame_offset[1] = -1;
            for (int i = lastFrame; i <= av1NumInterRefFrames; i++)
            {
                if (m_refFrameFlags & (AV1_ENCODE_GET_REF_FALG((i - 1))))
                {
                    int ref_offset = params.refOrderHints[i];
                    if ((ref_frame_offset[0] != -1 && (GetRelativeDist(ref_offset, ref_frame_offset[0]) < 0)) &&
                        (ref_frame_offset[1] == -1 || (GetRelativeDist(ref_offset, ref_frame_offset[1]) > 0)))
                    {
                        // Second closest forward reference
                        ref_frame_offset[1] = ref_offset;
                        ref_idx[1]          = i;
                    }
                }
            }

            if (ref_frame_offset[1] >= 0)
            {
                skipModeFrame[0] = (uint8_t)(ref_idx[0] < ref_idx[1]) ? ref_idx[0] : ref_idx[1];
                skipModeFrame[1] = (uint8_t)(ref_idx[0] > ref_idx[1]) ? ref_idx[0] : ref_idx[1];
            }
        }
    }

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(AVP_PIPE_BUF_ADDR_STATE, Av1ReferenceFrames)
{
    ENCODE_CHK_NULL_RETURN(m_basicFeature);
    ENCODE_CHK_NULL_RETURN(m_basicFeature->m_trackedBuf);

    const auto picParams = m_basicFeature->m_av1PicParams;
    ENCODE_CHK_NULL_RETURN(picParams);

    auto seqParams = m_basicFeature->m_av1SeqParams;
    ENCODE_CHK_NULL_RETURN(seqParams);

    auto currRawOrRecon = seqParams->SeqFlags.fields.UseRawReconRef ?
        &m_basicFeature->m_rawSurface :
        &m_basicFeature->m_reconSurface;

    // fullfill reference frames

    if (!AV1_KEY_OR_INRA_FRAME(picParams->PicFlags.fields.frame_type))
    {
        //set for INTRA_FRAME
        params.refs[0]            = &currRawOrRecon->OsResource;
        uint8_t currSlotIndex      = m_basicFeature->m_trackedBuf->GetCurrIndex();
        params.colMvTempBuffer[0] = m_basicFeature->m_trackedBuf->GetBuffer(BufferType::mvTemporalBuffer, currSlotIndex);

        //set for reference frames and collated temoral buffer
        for (uint8_t i = 0; i < av1NumInterRefFrames; i++)
        {
            params.refs[i + lastFrame] = &m_currRefPic[i]->OsResource;

            if (m_refFrameFlags & (AV1_ENCODE_GET_REF_FALG(i)))
            {
                auto idx      = picParams->ref_frame_idx[i];
                auto frameIdx = picParams->RefFrameList[idx].FrameIdx;

                uint8_t mvTempBufIdx      = m_refList[frameIdx]->ucScalingIdx;
                auto    mvTempBufferForRef = m_basicFeature->m_trackedBuf->GetBuffer(BufferType::mvTemporalBuffer, mvTempBufIdx);
                ENCODE_CHK_NULL_RETURN(mvTempBufferForRef);
                params.colMvTempBuffer[i + lastFrame] = mvTempBufferForRef;
            }
        }
    }
    else
    {
        // reference surface should be nullptr when key_frame == true or intra only frame
        for (auto j = 0; j < av1TotalRefsPerFrame; j++)
        {
            params.refs[j] = nullptr;
        }
    }

    // error concealment for the unset reference and mv buffers addresses
    for (uint8_t i = 0; i < av1TotalRefsPerFrame; i++)
    {
        if (params.refs[i] == nullptr)
        {
            params.refs[i] = &m_firstValidRefPic->OsResource;
        }
        if (params.colMvTempBuffer[i] == nullptr)
        {
            // set for collocated MV temporal buffer
            auto currRefIdx  = picParams->CurrReconstructedPic.FrameIdx;
            auto mvTempBuffer = m_basicFeature->m_trackedBuf->GetBuffer(BufferType::mvTemporalBuffer, m_refList[currRefIdx]->ucScalingIdx);
            ENCODE_CHK_NULL_RETURN(mvTempBuffer);
            params.colMvTempBuffer[i] = mvTempBuffer;
        }
    }

    if (picParams->PicFlags.fields.disable_frame_end_update_cdf || m_primaryRefFrame == av1PrimaryRefNone)
    {
        params.cdfTableInitBuffer       = m_basicFeature->m_defaultCdfBufferInUse;
        params.cdfTableInitBufferOffset = m_basicFeature->m_defaultCdfBufferInUseOffset;
    }
    else
    {
        ENCODE_CHK_COND_RETURN(m_primaryRefFrame >= av1PrimaryRefNone, "ERROR - primary ref frame is invalid!");
        uint8_t refPicIndex = picParams->ref_frame_idx[m_primaryRefFrame];
        ENCODE_CHK_COND_RETURN(refPicIndex >= CODEC_AV1_NUM_REF_FRAMES, "ERROR - ref pic idx is invalid!");
        auto    refPic      = picParams->RefFrameList[refPicIndex];
        if (!CodecHal_PictureIsInvalid(refPic))
        {
            uint8_t prevFrameIdx             = refPic.FrameIdx;
            auto    refScalingIdx            = m_refList[prevFrameIdx]->ucScalingIdx;
            params.cdfTableInitBuffer       = m_basicFeature->m_trackedBuf->GetBuffer(BufferType::bwdAdaptCdfBuffer, refScalingIdx);
            params.cdfTableInitBufferOffset = 0;
        }
    }

    params.cdfTableBwdAdaptBuffer = m_basicFeature->m_trackedBuf->GetBuffer(BufferType::bwdAdaptCdfBuffer, m_currRefList->ucScalingIdx);

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(AVP_INTER_PRED_STATE, Av1ReferenceFrames)
{
    const auto picParams = m_basicFeature->m_av1PicParams;

    ENCODE_CHK_NULL_RETURN(picParams);

    for (auto ref = 0; ref < av1NumInterRefFrames; ref++)
    {
        if (m_refFrameFlags & (AV1_ENCODE_GET_REF_FALG(ref)))
        {
            auto index       = picParams->ref_frame_idx[ref];
            auto refFrameIdx = picParams->RefFrameList[index].FrameIdx;
            for (auto i = 0; i < 7; i++)
            {
                params.savedRefOrderHints[ref][i] = m_refList[refFrameIdx]->m_refOrderHint[i];
            }
        }
    }

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(AVP_SURFACE_STATE, Av1ReferenceFrames)
{
    if (params.surfaceStateId == av1IntraFrame)
    {
        params.pitch   = m_firstValidRefPic->dwPitch;
        params.uOffset = m_firstValidRefPic->YoffsetForUplane;
        params.vOffset = m_firstValidRefPic->YoffsetForVplane;

    }
    else
    {
        if (params.surfaceStateId < av1LastRef || params.surfaceStateId > av1AltRef)
        {
            ENCODE_ASSERTMESSAGE("Incorrect reference passed for AVP_SURFACE_STATE");
            return MOS_STATUS_INVALID_PARAMETER;
        }
        params.pitch   = m_currRefPic[params.surfaceStateId - av1LastRef]->dwPitch;
        params.uOffset = m_currRefPic[params.surfaceStateId - av1LastRef]->YoffsetForUplane;
        params.vOffset = m_currRefPic[params.surfaceStateId - av1LastRef]->YoffsetForVplane;

    }
    std::copy(std::begin(m_refMmcState), std::end(m_refMmcState), params.mmcState);
    params.compressionFormat = m_refCompressionFormat;

    return MOS_STATUS_SUCCESS;
}

}  // namespace encode
