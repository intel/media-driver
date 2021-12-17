/*
* Copyright (c) 2019-2021, Intel Corporation
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
//! \file     decode_hevc_slice_packet_xe_m_base.cpp
//! \brief    Defines the interface for hevc decode slice packet
//!
#include "codechal_utilities.h"
#include "decode_hevc_slice_packet_xe_m_base.h"

namespace decode
{

HevcDecodeSlcPktXe_M_Base::~HevcDecodeSlcPktXe_M_Base()
{
}

MOS_STATUS HevcDecodeSlcPktXe_M_Base::Init()
{
    DECODE_FUNC_CALL();
    DECODE_CHK_NULL(m_featureManager);
    DECODE_CHK_NULL(m_hwInterface);
    DECODE_CHK_NULL(m_osInterface);
    DECODE_CHK_NULL(m_miInterface);
    DECODE_CHK_NULL(m_hevcPipeline);
    DECODE_CHK_NULL(m_hcpInterface);

    m_hevcBasicFeature = dynamic_cast<HevcBasicFeature*>(m_featureManager->GetFeature(FeatureIDs::basicFeature));
    DECODE_CHK_NULL(m_hevcBasicFeature);

    m_allocator = m_pipeline ->GetDecodeAllocator();
    DECODE_CHK_NULL(m_allocator);

    m_decodecp = m_pipeline->GetDecodeCp();

    DECODE_CHK_STATUS(CalculateSliceStateCommandSize());

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcDecodeSlcPktXe_M_Base::Prepare()
{
    DECODE_FUNC_CALL();

    DECODE_CHK_NULL(m_hevcBasicFeature->m_hevcPicParams);
    DECODE_CHK_NULL(m_hevcBasicFeature->m_hevcSliceParams);

    m_hevcPicParams       = m_hevcBasicFeature->m_hevcPicParams;
    m_hevcSliceParams     = m_hevcBasicFeature->m_hevcSliceParams;
    m_hevcRextPicParams   = m_hevcBasicFeature->m_hevcRextPicParams;
    m_hevcRextSliceParams = m_hevcBasicFeature->m_hevcRextSliceParams;
    m_hevcSccPicParams    = m_hevcBasicFeature->m_hevcSccPicParams;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcDecodeSlcPktXe_M_Base::ValidateSubTileIdx(
    const HevcTileCoding::SliceTileInfo &sliceTileInfo,
    uint32_t                             subTileIdx)
{
    if (sliceTileInfo.numTiles > 0)
    {
        DECODE_CHK_COND(subTileIdx >= sliceTileInfo.numTiles, "sub tile index exceeds number of tiles!");
    }
    else
    {
        DECODE_CHK_COND(subTileIdx > 0, "sub tile index exceeds number of tiles!");
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcDecodeSlcPktXe_M_Base::SetHcpSliceStateParams(
    MHW_VDBOX_HEVC_SLICE_STATE &sliceStateParams,
    uint32_t                    sliceIdx,
    uint32_t                    subTileIdx)
{
    DECODE_FUNC_CALL();

    const HevcTileCoding::SliceTileInfo *sliceTileInfo = m_hevcBasicFeature->m_tileCoding.GetSliceTileInfo(sliceIdx);
    DECODE_CHK_NULL(sliceTileInfo);

    CODEC_HEVC_SLICE_PARAMS *sliceParams = m_hevcSliceParams + sliceIdx;

    sliceStateParams.presDataBuffer = &(m_hevcBasicFeature->m_resDataBuffer.OsResource);
    sliceStateParams.pRefIdxMapping = m_hevcBasicFeature->m_refFrames.m_refIdxMapping;
    sliceStateParams.pHevcPicParams = m_hevcPicParams;
    sliceStateParams.pHevcSliceParams = sliceParams;

    sliceStateParams.bLastSliceInTile = sliceTileInfo->lastSliceOfTile;
    sliceStateParams.bLastSliceInTileColumn = sliceTileInfo->lastSliceOfTile &&
                                              (sliceTileInfo->sliceTileY == m_hevcPicParams->num_tile_rows_minus1);

    sliceStateParams.dwLength = sliceParams->slice_data_size;
    sliceStateParams.dwSliceIndex = sliceIdx;
    sliceStateParams.bLastSlice = m_hevcBasicFeature->IsLastSlice(sliceIdx);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcDecodeSlcPktXe_M_Base::SetRefIdxParams(
    MHW_VDBOX_HEVC_REF_IDX_PARAMS &refIdxParams,
    uint32_t                       sliceIdx)
{
    CODEC_HEVC_SLICE_PARAMS *sliceParams = m_hevcSliceParams + sliceIdx;

    if (!m_hcpInterface->IsHevcISlice(sliceParams->LongSliceFlags.fields.slice_type))
    {
        HevcReferenceFrames &refFrames = m_hevcBasicFeature->m_refFrames;
        DECODE_CHK_STATUS(refFrames.FixSliceRefList(*m_hevcPicParams, *sliceParams));

        refIdxParams.CurrPic         = m_hevcPicParams->CurrPic;
        refIdxParams.ucNumRefForList = sliceParams->num_ref_idx_l0_active_minus1 + 1;

        DECODE_CHK_STATUS(MOS_SecureMemcpy(&refIdxParams.RefPicList, sizeof(refIdxParams.RefPicList),
                                           &sliceParams->RefPicList, sizeof(sliceParams->RefPicList)));

        refIdxParams.hevcRefList  = (void **)refFrames.m_refList;
        refIdxParams.poc_curr_pic = m_hevcPicParams->CurrPicOrderCntVal;
        for (uint8_t i = 0; i < CODEC_MAX_NUM_REF_FRAME_HEVC; i++)
        {
            refIdxParams.poc_list[i] = m_hevcPicParams->PicOrderCntValList[i];
        }

        refIdxParams.pRefIdxMapping     = refFrames.m_refIdxMapping;
        refIdxParams.RefFieldPicFlag    = m_hevcPicParams->RefFieldPicFlag;
        refIdxParams.RefBottomFieldFlag = m_hevcPicParams->RefBottomFieldFlag;
    }
    else if (m_hevcBasicFeature->m_useDummyReference && !m_osInterface->bSimIsActive)
    {
        refIdxParams.bDummyReference = true;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcDecodeSlcPktXe_M_Base::SetWeightOffsetParams(
    MHW_VDBOX_HEVC_WEIGHTOFFSET_PARAMS &weightOffsetParams,
    uint32_t                            sliceIdx)
{
    CODEC_HEVC_SLICE_PARAMS *sliceParams = m_hevcSliceParams + sliceIdx;

    bool weightedPred   = m_hevcPicParams->weighted_pred_flag &&
                          m_hcpInterface->IsHevcPSlice(sliceParams->LongSliceFlags.fields.slice_type);
    bool weightedBiPred = m_hevcPicParams->weighted_bipred_flag &&
                          m_hcpInterface->IsHevcBSlice(sliceParams->LongSliceFlags.fields.slice_type);
    if (weightedPred || weightedBiPred)
    {
        weightOffsetParams.ucList = 0;

        DECODE_CHK_STATUS(MOS_SecureMemcpy(
            &weightOffsetParams.LumaWeights[0], sizeof(weightOffsetParams.LumaWeights[0]),
            &sliceParams->delta_luma_weight_l0, sizeof(sliceParams->delta_luma_weight_l0)));

        DECODE_CHK_STATUS(MOS_SecureMemcpy(
            &weightOffsetParams.LumaWeights[1], sizeof(weightOffsetParams.LumaWeights[1]),
            &sliceParams->delta_luma_weight_l1, sizeof(sliceParams->delta_luma_weight_l1)));

        PCODEC_HEVC_EXT_SLICE_PARAMS slcRextParams = (m_hevcRextSliceParams == nullptr) ?
                                                     nullptr : (m_hevcRextSliceParams + sliceIdx);
        if (slcRextParams != nullptr)
        {
            DECODE_CHK_STATUS(MOS_SecureMemcpy(
                &weightOffsetParams.LumaOffsets[0], sizeof(weightOffsetParams.LumaOffsets[0]),
                &slcRextParams->luma_offset_l0, sizeof(slcRextParams->luma_offset_l0)));

            DECODE_CHK_STATUS(MOS_SecureMemcpy(
                &weightOffsetParams.LumaOffsets[1], sizeof(weightOffsetParams.LumaOffsets[1]),
                &slcRextParams->luma_offset_l1, sizeof(slcRextParams->luma_offset_l1)));

            DECODE_CHK_STATUS(MOS_SecureMemcpy(
                &weightOffsetParams.ChromaOffsets[0], sizeof(weightOffsetParams.ChromaOffsets[0]),
                &slcRextParams->ChromaOffsetL0, sizeof(slcRextParams->ChromaOffsetL0)));

            DECODE_CHK_STATUS(MOS_SecureMemcpy(
                &weightOffsetParams.ChromaOffsets[1], sizeof(weightOffsetParams.ChromaOffsets[1]),
                &slcRextParams->ChromaOffsetL1, sizeof(slcRextParams->ChromaOffsetL1)));
        }
        else
        {
            for (uint32_t i = 0; i < 15; i++)
            {
                weightOffsetParams.LumaOffsets[0][i] = (int16_t)sliceParams->luma_offset_l0[i];
                weightOffsetParams.LumaOffsets[1][i] = (int16_t)sliceParams->luma_offset_l1[i];

                for (uint32_t j = 0; j < 2; j++)
                {
                    weightOffsetParams.ChromaOffsets[0][i][j] = (int16_t)sliceParams->ChromaOffsetL0[i][j];
                    weightOffsetParams.ChromaOffsets[1][i][j] = (int16_t)sliceParams->ChromaOffsetL1[i][j];
                }
            }
        }

        DECODE_CHK_STATUS(MOS_SecureMemcpy(
            &weightOffsetParams.ChromaWeights[0], sizeof(weightOffsetParams.ChromaWeights[0]),
            &sliceParams->delta_chroma_weight_l0, sizeof(sliceParams->delta_chroma_weight_l0)));

        DECODE_CHK_STATUS(MOS_SecureMemcpy(
            &weightOffsetParams.ChromaWeights[1], sizeof(weightOffsetParams.ChromaWeights[1]),
            &sliceParams->delta_chroma_weight_l1, sizeof(sliceParams->delta_chroma_weight_l1)));
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcDecodeSlcPktXe_M_Base::AddWeightOffset(
    MOS_COMMAND_BUFFER &cmdBuffer,
    uint32_t            sliceIdx)
{
    CODEC_HEVC_SLICE_PARAMS *sliceParams = m_hevcSliceParams + sliceIdx;
    bool weightedPred   = m_hevcPicParams->weighted_pred_flag &&
                          m_hcpInterface->IsHevcPSlice(sliceParams->LongSliceFlags.fields.slice_type);
    bool weightedBiPred = m_hevcPicParams->weighted_bipred_flag &&
                          m_hcpInterface->IsHevcBSlice(sliceParams->LongSliceFlags.fields.slice_type);

    if (weightedPred || weightedBiPred)
    {
        MHW_VDBOX_HEVC_WEIGHTOFFSET_PARAMS weightOffsetParams;
        MOS_ZeroMemory(&weightOffsetParams, sizeof(weightOffsetParams));

        DECODE_CHK_STATUS(SetWeightOffsetParams(weightOffsetParams, sliceIdx));
        DECODE_CHK_STATUS(m_hcpInterface->AddHcpWeightOffsetStateCmd(&cmdBuffer, nullptr, &weightOffsetParams));

        if (weightedBiPred)
        {
            weightOffsetParams.ucList = 1;
            DECODE_CHK_STATUS(m_hcpInterface->AddHcpWeightOffsetStateCmd(&cmdBuffer, nullptr, &weightOffsetParams));
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcDecodeSlcPktXe_M_Base::SetBsdObjParams(
    MHW_VDBOX_HCP_BSD_PARAMS &bsdObjParams,
    uint32_t                  sliceIdx,
    uint32_t                  subTileIdx)
{
    const HevcTileCoding::SliceTileInfo *sliceTileInfo = m_hevcBasicFeature->m_tileCoding.GetSliceTileInfo(sliceIdx);
    DECODE_CHK_NULL(sliceTileInfo);
    DECODE_CHK_STATUS(ValidateSubTileIdx(*sliceTileInfo, subTileIdx));

    CODEC_HEVC_SLICE_PARAMS *sliceParams = m_hevcSliceParams + sliceIdx;

    if (sliceTileInfo->numTiles > 1)
    {
        bsdObjParams.dwBsdDataLength = sliceTileInfo->tileArrayBuf[subTileIdx].bsdLength;
        bsdObjParams.dwBsdDataStartOffset = sliceParams->slice_data_offset +
                                            sliceTileInfo->tileArrayBuf[subTileIdx].bsdOffset;
    }
    else
    {
        bsdObjParams.dwBsdDataLength = sliceParams->slice_data_size;
        bsdObjParams.dwBsdDataStartOffset = sliceParams->slice_data_offset;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcDecodeSlcPktXe_M_Base::AddBsdObj(
    MOS_COMMAND_BUFFER &cmdBuffer,
    uint32_t            sliceIdx,
    uint32_t            subTileIdx)
{
    MHW_VDBOX_HCP_BSD_PARAMS bsdObjParams;
    MOS_ZeroMemory(&bsdObjParams, sizeof(MHW_VDBOX_HCP_BSD_PARAMS));

    DECODE_CHK_STATUS(SetBsdObjParams(bsdObjParams, sliceIdx, subTileIdx));
    DECODE_CHK_STATUS(m_hcpInterface->AddHcpBsdObjectCmd(&cmdBuffer, &bsdObjParams));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcDecodeSlcPktXe_M_Base::AddHcpCpState(MOS_COMMAND_BUFFER &cmdBuffer, uint32_t sliceIdx, uint32_t subTileIdx)
{
    PMOS_RESOURCE buffer = &(m_hevcBasicFeature->m_resDataBuffer.OsResource);
    uint32_t startoffset  = m_hevcSliceParams->slice_data_offset;

    const HevcTileCoding::SliceTileInfo *sliceTileInfo = m_hevcBasicFeature->m_tileCoding.GetSliceTileInfo(sliceIdx);
    DECODE_CHK_NULL(sliceTileInfo);
    DECODE_CHK_STATUS(ValidateSubTileIdx(*sliceTileInfo, subTileIdx));

    CODEC_HEVC_SLICE_PARAMS *sliceParams = m_hevcSliceParams + sliceIdx;

    if (sliceTileInfo->numTiles > 1)
    {
        startoffset = sliceParams->slice_data_offset +
        sliceTileInfo->tileArrayBuf[subTileIdx].bsdOffset;
    }
    else
    {
        startoffset = sliceParams->slice_data_offset;
    }
    if(m_decodecp)
    {
        DECODE_CHK_STATUS(m_decodecp->AddHcpState(&cmdBuffer, buffer, m_hevcSliceParams->slice_data_size, startoffset, sliceIdx));
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcDecodeSlcPktXe_M_Base::CalculateCommandSize(uint32_t &commandBufferSize,
                                                  uint32_t &requestedPatchListSize)
{
    commandBufferSize      = m_sliceStatesSize;
    requestedPatchListSize = m_slicePatchListSize;
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcDecodeSlcPktXe_M_Base::CalculateSliceStateCommandSize()
{
    DECODE_FUNC_CALL();

    // Slice Level Commands
    DECODE_CHK_STATUS(m_hwInterface->GetHcpPrimitiveCommandSize(m_hevcBasicFeature->m_mode,
                                                                 &m_sliceStatesSize,
                                                                 &m_slicePatchListSize,
                                                                 false));

    return MOS_STATUS_SUCCESS;
}

}
