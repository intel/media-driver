/*
* Copyright (c) 2021-2024, Intel Corporation
*
* Permission is hereby granted, free of charge, to any person obtaining a
* copy of this software and associated documentation files (the "Software"),
* to deal in the Software without restriction, including without limitation
* the rights to use, copy, modify, merge, publish, distribute, sublicense,Av1EncodeTile
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
//! \file     encode_avc_aqm.cpp
//! \brief    Defines the common interface for avc aqm
//!

#include "encode_avc_aqm.h"
#include "encode_avc_vdenc_feature_manager.h"
#include "encode_avc_basic_feature.h"

namespace encode
{
AvcEncodeAqm::AvcEncodeAqm(MediaFeatureManager *featureManager,
    EncodeAllocator *                           allocator,
    CodechalHwInterfaceNext *                       hwInterface,
    void *                                      constSettings) : EncodeAqmFeature(featureManager, allocator, hwInterface, constSettings)
{
    auto encFeatureManager = dynamic_cast<EncodeAvcVdencFeatureManager *>(featureManager);
    ENCODE_CHK_NULL_NO_STATUS_RETURN(encFeatureManager);

    m_basicFeature = dynamic_cast<EncodeBasicFeature *>(encFeatureManager->GetFeature(FeatureIDs::basicFeature));
    ENCODE_CHK_NULL_NO_STATUS_RETURN(m_basicFeature);

    m_numTiles = 1;
};

MOS_STATUS AvcEncodeAqm::Update(void* params)
{
    auto basicFeature = dynamic_cast<AvcBasicFeature*>(m_basicFeature);
    ENCODE_CHK_NULL_RETURN(basicFeature);
    if (basicFeature->m_picParam->QualityInfoSupportFlags.fields.enable_frame)
    {
        m_enabled = true;
        basicFeature->m_suppressReconPicSupported = false;
    }
    m_numTiles = 1;
    m_tile_width[0]     = (uint16_t)m_basicFeature->m_oriFrameWidth;
    m_tile_height[0]    = (uint16_t)m_basicFeature->m_oriFrameHeight;
#if USE_CODECHAL_DEBUG_TOOL
    UpdateFrameDisplayOrder(basicFeature->m_pictureCodingType, basicFeature->m_picParam->CurrFieldOrderCnt[0] / 2, basicFeature->m_seqParam->GopPicSize);
#endif
    ENCODE_CHK_STATUS_RETURN(EncodeAqmFeature::Update(params));
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS AvcEncodeAqm::Init(void *setting)
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_NULL_RETURN(setting);

    ENCODE_CHK_STATUS_RETURN(EncodeAqmFeature::Init(setting));

    auto basicFeature = dynamic_cast<AvcBasicFeature *>(m_basicFeature);
    ENCODE_CHK_NULL_RETURN(basicFeature);

    if (m_enabled)
    {
        // Assuming AvcBasicFeature is already initialized, if VDAQM is in use, enable deblocking/recon generation for all frames
        basicFeature->m_suppressReconPicSupported = false;
    }

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(AQM_PIC_STATE, AvcEncodeAqm)
{
    ENCODE_CHK_STATUS_RETURN(EncodeAqmFeature::MHW_SETPAR_F(AQM_PIC_STATE)(params));
    if (m_enabled)
    {
        params.frameWidthInPixelMinus1  = MOS_ALIGN_CEIL(m_basicFeature->m_oriFrameWidth, 16) - 1;
        params.FrameHeightInPixelMinus1 = MOS_ALIGN_CEIL(m_basicFeature->m_oriFrameHeight, 16) - 1;
        params.lcuSize                  = LCU_SIZE_16X16;
        params.codectype                = CODECTYPE_AVC;
    }

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(AQM_SLICE_STATE, AvcEncodeAqm)
{
    auto basicFeature = dynamic_cast<AvcBasicFeature *>(m_basicFeature);
    ENCODE_CHK_NULL_RETURN(basicFeature);

    auto sliceParams = &basicFeature->m_sliceParams[basicFeature->m_curNumSlices];
    auto frameHeight = static_cast<uint32_t>(CODECHAL_GET_HEIGHT_IN_MACROBLOCKS(basicFeature->m_seqParam->FrameHeight));
    auto frameWidth  = static_cast<uint32_t>(CODECHAL_GET_WIDTH_IN_MACROBLOCKS(basicFeature->m_seqParam->FrameWidth));
    auto nextsliceMbStartYPosition = (sliceParams->first_mb_in_slice + sliceParams->NumMbsForSlice) / frameWidth;

    params.tileSliceStartLcuMbX     = 0;
    params.tileSliceStartLcuMbY     = sliceParams->first_mb_in_slice / frameWidth;
    params.nextTileSliceStartLcuMbX = 0;
    params.nextTileSliceStartLcuMbY = nextsliceMbStartYPosition > frameHeight ? frameHeight : nextsliceMbStartYPosition;

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(MFX_AVC_IMG_STATE, AvcEncodeAqm)
{
    params.vdaqmEnable = m_enabled;

    return MOS_STATUS_SUCCESS;
}

#if USE_CODECHAL_DEBUG_TOOL
MOS_STATUS AvcEncodeAqm::UpdateFrameDisplayOrder(const uint16_t pictureCodingType, const uint32_t framePOC, const uint32_t gopPicSize)
{
    auto basicFeature = dynamic_cast<AvcBasicFeature *>(m_basicFeature);
    ENCODE_CHK_NULL_RETURN(basicFeature);
    if (basicFeature->m_picParam->bIdrPic == 1)
    {
        m_frameNumPrevious += m_gopSizePrevious;
    }
    uint32_t    displayOrderInGOP   = framePOC;
    uint32_t    displayOrderInSeq   = displayOrderInGOP + m_frameNumPrevious;
    m_gopSizePrevious               = gopPicSize;
    m_frameIdxQueue.push(displayOrderInSeq);
    return MOS_STATUS_SUCCESS;
}
#endif

}  // namespace encode