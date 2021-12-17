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
//! \file     decode_hevc_basic_feature.cpp
//! \brief    Defines the common interface for decode hevc parameter
//!

#include "decode_hevc_basic_feature.h"
#include "decode_utils.h"
#include "codechal_utilities.h"
#include "decode_allocator.h"

namespace decode
{

MOS_STATUS HevcBasicFeature::Init(void *setting)
{
    DECODE_FUNC_CALL();
    DECODE_CHK_NULL(setting);
    DECODE_CHK_NULL(m_hwInterface);

    DECODE_CHK_STATUS(DecodeBasicFeature::Init(setting));

    DECODE_CHK_STATUS(m_refFrames.Init(this, *m_allocator));
    DECODE_CHK_STATUS(m_mvBuffers.Init(*m_hwInterface, *m_allocator, *this,
                                       CODEC_NUM_HEVC_INITIAL_MV_BUFFERS));
    DECODE_CHK_STATUS(m_tileCoding.Init(this, (CodechalSetting*)setting));

    return MOS_STATUS_SUCCESS;
}

HevcBasicFeature::~HevcBasicFeature()
{
    m_allocator->Destroy(m_referenceBeforeLoopFilter);
}

MOS_STATUS HevcBasicFeature::Update(void *params)
{
    DECODE_FUNC_CALL();

    PERF_UTILITY_AUTO(__FUNCTION__, PERF_DECODE, PERF_LEVEL_HAL);

    DECODE_CHK_NULL(params);

    DECODE_CHK_STATUS(DecodeBasicFeature::Update(params));

    CodechalDecodeParams* decodeParams = (CodechalDecodeParams*)params;
    DECODE_CHK_NULL(decodeParams->m_picParams);
    DECODE_CHK_NULL(decodeParams->m_sliceParams);
    DECODE_CHK_NULL(decodeParams->m_iqMatrixBuffer);

    m_hevcPicParams      = static_cast<PCODEC_HEVC_PIC_PARAMS>(decodeParams->m_picParams);
    m_hevcSliceParams    = static_cast<PCODEC_HEVC_SLICE_PARAMS>(decodeParams->m_sliceParams);
    m_hevcIqMatrixParams = static_cast<PCODECHAL_HEVC_IQ_MATRIX_PARAMS>(decodeParams->m_iqMatrixBuffer);

    m_hevcRextPicParams  = static_cast<PCODEC_HEVC_EXT_PIC_PARAMS>(decodeParams->m_extPicParams);
    m_hevcRextSliceParams= static_cast<PCODEC_HEVC_EXT_SLICE_PARAMS>(decodeParams->m_extSliceParams);
    m_hevcSccPicParams   = static_cast<PCODEC_HEVC_SCC_PIC_PARAMS>(decodeParams->m_advPicParams);
    m_hevcSubsetParams   = static_cast<PCODEC_HEVC_SUBSET_PARAMS>(decodeParams->m_subsetParams);

    DECODE_CHK_STATUS(SetPictureStructs());
    DECODE_CHK_STATUS(SetSliceStructs());

    return MOS_STATUS_SUCCESS;
}

bool HevcBasicFeature::IsLastSlice(uint32_t sliceIdx)
{
    return (sliceIdx == (m_numSlices-1)) ||
           m_hevcSliceParams[sliceIdx].LongSliceFlags.fields.LastSliceOfPic;
}

bool HevcBasicFeature::IsIndependentSlice(uint32_t sliceIdx)
{
    return (sliceIdx == 0) ||
           (! m_hevcSliceParams[sliceIdx].LongSliceFlags.fields.dependent_slice_segment_flag);
}

MOS_STATUS HevcBasicFeature::SetRequiredBitstreamSize(uint32_t requiredSize)
{
    DECODE_FUNC_CALL();
    if (requiredSize > m_dataSize)
    {
        m_dataOffset = 0;
        m_dataSize   = MOS_ALIGN_CEIL(requiredSize, MHW_CACHELINE_SIZE);
    }
    DECODE_NORMALMESSAGE("Estimate bitstream size in this Frame: %u", requiredSize);
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcBasicFeature::SetPictureStructs()
{
    DECODE_FUNC_CALL();

    m_curRenderPic = m_hevcPicParams->CurrPic;
    DECODE_CHK_COND(m_curRenderPic.FrameIdx >= CODECHAL_NUM_UNCOMPRESSED_SURFACE_HEVC,
                    "currPic.FrameIdx is out of range!");
    m_secondField = CodecHal_PictureIsBottomField(m_curRenderPic);

    m_isWPPMode = m_hevcPicParams->entropy_coding_sync_enabled_flag;

    if (m_hevcSccPicParams == nullptr)
    {
        m_isSCCIBCMode = false;
        m_isSCCPLTMode = false;
        m_isSCCACTMode = false;
    }
    else
    {
        m_isSCCIBCMode = m_hevcSccPicParams->PicSCCExtensionFlags.fields.pps_curr_pic_ref_enabled_flag;
        m_isSCCPLTMode = m_hevcSccPicParams->PicSCCExtensionFlags.fields.palette_mode_enabled_flag;
        m_isSCCACTMode = m_hevcSccPicParams->PicSCCExtensionFlags.fields.residual_adaptive_colour_transform_enabled_flag;
    }

    if (m_isSCCIBCMode)
    {
        DECODE_CHK_STATUS(CreateReferenceBeforeLoopFilter());
    }

    m_refFrameIndexList.clear();
    for(uint32_t i = 0; i < CODEC_MAX_NUM_REF_FRAME_HEVC; i++)
    {
        if (m_hevcPicParams->RefFrameList[i].FrameIdx < m_maxFrameIndex)
        {
            m_refFrameIndexList.push_back(m_hevcPicParams->RefFrameList[i].FrameIdx);
        }
    }

    m_minCtbSize = 1 << (m_hevcPicParams->log2_min_luma_coding_block_size_minus3 + 3);
    m_width      = m_hevcPicParams->PicWidthInMinCbsY * m_minCtbSize;
    m_height     = m_hevcPicParams->PicHeightInMinCbsY * m_minCtbSize;

    m_ctbSize    = 1 << (m_hevcPicParams->log2_diff_max_min_luma_coding_block_size +
                         m_hevcPicParams->log2_min_luma_coding_block_size_minus3 + 3);
    m_widthInCtb  = MOS_ROUNDUP_DIVIDE(m_width, m_ctbSize);
    m_heightInCtb = MOS_ROUNDUP_DIVIDE(m_height, m_ctbSize);

    // Check LCU size
    if (m_width > CODECHAL_HEVC_MAX_DIM_FOR_MIN_LCU ||
        m_height > CODECHAL_HEVC_MAX_DIM_FOR_MIN_LCU)
    {
        DECODE_CHK_COND(m_ctbSize == CODECHAL_HEVC_MIN_LCU, "Invalid LCU size.");
    }

    m_reportFrameCrc = m_hevcPicParams->RequestCRC;

    DECODE_CHK_STATUS(m_refFrames.UpdatePicture(*m_hevcPicParams, m_isSCCIBCMode));
    DECODE_CHK_STATUS(m_mvBuffers.UpdatePicture(m_curRenderPic.FrameIdx, m_refFrameIndexList));
    DECODE_CHK_STATUS(m_tileCoding.UpdatePicture(*m_hevcPicParams));

    // Not possible to determine whether P or B is used for short format.
    // For long format iterating through all of the slices to determine P vs
    // B, so in order to avoid this, declare all other pictures MIXED_TYPE.
    m_pictureCodingType = m_refFrames.m_curIsIntra ? I_TYPE : MIXED_TYPE;

    // Clean dummy reference slot, will be upated by Hevc picture packet accrodingly
    MOS_ZeroMemory(m_dummyReferenceSlot, sizeof(m_dummyReferenceSlot));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcBasicFeature::SetSliceStructs()
{
    DECODE_FUNC_CALL();

    DECODE_CHK_STATUS(m_tileCoding.UpdateSlice(*m_hevcPicParams, m_hevcSliceParams));

    if (m_numSlices > 0)
    {
        PCODEC_HEVC_SLICE_PARAMS lastSlice = m_hevcSliceParams + (m_numSlices - 1);
        DECODE_CHK_STATUS(SetRequiredBitstreamSize(lastSlice->slice_data_offset + lastSlice->slice_data_size));
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcBasicFeature::CreateReferenceBeforeLoopFilter()
{
    if (m_destSurface.dwPitch  == 0 ||
        m_destSurface.dwHeight == 0)
    {
        return MOS_STATUS_SUCCESS;
    }

    if (m_referenceBeforeLoopFilter == nullptr)
    {
        m_referenceBeforeLoopFilter = m_allocator->AllocateSurface(
            m_destSurface.dwPitch, m_destSurface.dwHeight, "Reference before loop filter",
            m_destSurface.Format, m_destSurface.bCompressible, resourceOutputPicture, notLockableVideoMem);
        DECODE_CHK_NULL(m_referenceBeforeLoopFilter);
    }
    else
    {
        m_allocator->Resize(m_referenceBeforeLoopFilter, m_destSurface.dwPitch, m_destSurface.dwHeight,
            notLockableVideoMem, false, "Reference before loop filter");
    }
    return MOS_STATUS_SUCCESS;
}

}  // namespace encode
