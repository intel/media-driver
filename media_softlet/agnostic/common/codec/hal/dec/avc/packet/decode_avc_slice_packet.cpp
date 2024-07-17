/*
* Copyright (c) 2021-2024, Intel Corporation
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
//! \file     decode_avc_slice_packet.cpp
//! \brief    Defines the interface for avc decode slice packet
//!
#include "decode_avc_slice_packet.h"
#include "codec_def_common.h"

namespace decode
{
MOS_STATUS AvcDecodeSlcPkt::Init()
{
    DECODE_FUNC_CALL();

    DECODE_CHK_NULL(m_featureManager);
    DECODE_CHK_NULL(m_hwInterface);
    DECODE_CHK_NULL(m_osInterface);
    DECODE_CHK_NULL(m_miItf);
    DECODE_CHK_NULL(m_avcPipeline);
    DECODE_CHK_NULL(m_mfxItf);

    m_avcBasicFeature = dynamic_cast<AvcBasicFeature *>(m_featureManager->GetFeature(FeatureIDs::basicFeature));
    DECODE_CHK_NULL(m_avcBasicFeature);

    m_allocator = m_pipeline->GetDecodeAllocator();
    DECODE_CHK_NULL(m_allocator);

    DECODE_CHK_STATUS(CalculateSliceStateCommandSize());

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS AvcDecodeSlcPkt::Prepare()
{
    DECODE_FUNC_CALL();

    DECODE_CHK_NULL(m_avcBasicFeature->m_avcPicParams);
    DECODE_CHK_NULL(m_avcBasicFeature->m_avcSliceParams);

    m_avcPicParams    = m_avcBasicFeature->m_avcPicParams;
    m_avcSliceParams  = m_avcBasicFeature->m_avcSliceParams;
    m_firstValidSlice = true;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS AvcDecodeSlcPkt::AddCmd_AVC_SLICE_STATE(MOS_COMMAND_BUFFER &cmdBuffer, uint32_t slcIdx)
{
    SET_AVC_SLICE_STATE(cmdBuffer, slcIdx);
    SetAndAddAvcSliceState(cmdBuffer, slcIdx);
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS AvcDecodeSlcPkt::SET_AVC_SLICE_STATE(MOS_COMMAND_BUFFER &cmdBuffer, uint32_t slcIdx)
{
    DECODE_FUNC_CALL();
    auto                   &par        = m_mfxItf->MHW_GETPAR_F(MFX_AVC_SLICE_STATE)();
    par                                = {};
    PCODEC_AVC_SLICE_PARAMS slc        = m_avcSliceParams + slcIdx;
    uint32_t                nextLength = 0;
    uint32_t                nextOffset = 0;
    
    if (slcIdx < m_avcBasicFeature->m_lastValidSlice)
    {
        nextLength = (slc + 1)->slice_data_size;
        nextOffset = (slc + 1)->slice_data_offset;
    }
    par.decodeInUse                      = true;
    par.intelEntrypointInUse             = m_avcPipeline->m_intelEntrypointInUse;
    par.picIdRemappingInUse              = m_avcBasicFeature->m_picIdRemappingInUse;
    par.shortFormatInUse                 = m_avcPipeline->IsShortFormat();
    par.presDataBuffer                   = &m_avcBasicFeature->m_resDataBuffer.OsResource;
    par.avcPicParams                     = m_avcPicParams;
    par.mvcExtPicParams                  = m_avcBasicFeature->m_mvcExtPicParams;
    par.avcPicIdx                        = &m_avcBasicFeature->m_refFrames.m_avcPicIdx[0];
    par.disableDeblockingFilterIndicator = slc->disable_deblocking_filter_idc;
    par.sliceBetaOffsetDiv2              = slc->slice_beta_offset_div2;
    par.sliceAlphaC0OffsetDiv2           = slc->slice_alpha_c0_offset_div2;
    par.avcSliceParams                   = slc;
    par.Offset                           = m_avcBasicFeature->m_sliceRecord[slcIdx].offset;
    par.Length                           = m_avcBasicFeature->m_sliceRecord[slcIdx].length;
    par.nextOffset                       = nextOffset;
    par.nextLength                       = nextLength;
    par.sliceIndex                       = slcIdx;
    par.isLastSlice                      = (slcIdx == m_avcBasicFeature->m_lastValidSlice);
    par.fullFrameData                    = m_avcBasicFeature->m_fullFrameData;
    par.sliceType                        = m_avcBasicFeature->AvcBsdSliceType[slc->slice_type];
    par.log2WeightDenomChroma            = slc->chroma_log2_weight_denom;
    par.log2WeightDenomLuma              = slc->luma_log2_weight_denom;
    par.cabacInitIdc10                   = slc->cabac_init_idc;
    par.sliceQuantizationParameter       = 26 + m_avcPicParams->pic_init_qp_minus26 + slc->slice_qp_delta;

    if (slcIdx > 0)
    {
        par.totalBytesConsumed = m_avcBasicFeature->m_sliceRecord[slcIdx - 1].totalBytesConsumed;
    }
    else
    {
        par.totalBytesConsumed = 0;
    }

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(MFD_AVC_BSD_OBJECT, AvcDecodeSlcPkt)
{
    params.IndirectBsdDataLength                     = m_IndirectBsdDataLength;
    params.IndirectBsdDataStartAddress               = m_IndirectBsdDataStartAddress;
    params.LastsliceFlag                             = m_LastsliceFlag;
    params.FirstMacroblockMbBitOffset                = m_FirstMacroblockMbBitOffset;
    params.FirstMbByteOffsetOfSliceDataOrSliceHeader = m_FirstMbByteOffsetOfSliceDataOrSliceHeader;
    params.decodeInUse                               = m_decodeInUse;
    params.pAvcSliceParams                           = m_pAvcSliceParams;
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS AvcDecodeSlcPkt::SetAndAddAvcSliceState(MOS_COMMAND_BUFFER &cmdBuffer, uint32_t slcIdx)
{
    DECODE_FUNC_CALL();
    auto     &par            = m_mfxItf->MHW_GETPAR_F(MFX_AVC_SLICE_STATE)();
    //No need to clear par here due to par set in AddCmd_AVC_PHANTOM_SLICE and SET_AVC_SLICE_STATE and continue set and add to cmdbuffer here.
    //par has already cleared in AddCmd_AVC_PHANTOM_SLICE and SET_AVC_SLICE_STATE.
    auto     picParams       = m_avcPicParams;
    uint32_t mbaffMultiplier = 1;
    if (picParams->seq_fields.mb_adaptive_frame_field_flag &&
        !picParams->pic_fields.field_pic_flag)
    {
        mbaffMultiplier++;
    }
    uint32_t frameFieldHeightInMb = 0;
    CodecHal_GetFrameFieldHeightInMb(
        picParams->CurrPic,
        picParams->pic_height_in_mbs_minus1 + 1,
        frameFieldHeightInMb);
    auto sliceParams = m_avcSliceParams + slcIdx;
    par.sliceType     = m_avcBasicFeature->AvcBsdSliceType[sliceParams->slice_type];
    par.sliceQuantizationParameter = 26 + picParams->pic_init_qp_minus26 + sliceParams->slice_qp_delta;
    par.disableDeblockingFilterIndicator = sliceParams->disable_deblocking_filter_idc;
    par.roundintra = 5;
    par.roundinter = 2;

    uint32_t widthInMb = picParams->pic_width_in_mbs_minus1 + 1;
    par.sliceStartMbNum         = sliceParams->first_mb_in_slice * mbaffMultiplier;
    par.sliceVerticalPosition   = (sliceParams->first_mb_in_slice / widthInMb) * mbaffMultiplier;
    par.sliceHorizontalPosition = sliceParams->first_mb_in_slice % widthInMb;

    if (par.isLastSlice)
    {
        par.nextSliceVerticalPosition   = frameFieldHeightInMb;
        par.nextSliceHorizontalPosition = 0;
    }
    else
    {
        par.nextSliceVerticalPosition   = (sliceParams->first_mb_in_next_slice / widthInMb) * mbaffMultiplier;
        par.nextSliceHorizontalPosition = sliceParams->first_mb_in_next_slice % widthInMb;
    }
    if (m_avcBasicFeature->IsAvcPSlice(sliceParams->slice_type))
    {
        par.numberOfReferencePicturesInInterPredictionList0 = sliceParams->num_ref_idx_l0_active_minus1 + 1;
        par.weightedPredictionIndicator                     = picParams->pic_fields.weighted_pred_flag;
    }
    else if (m_avcBasicFeature->IsAvcBSlice(sliceParams->slice_type))
    {
        par.numberOfReferencePicturesInInterPredictionList1 = sliceParams->num_ref_idx_l1_active_minus1 + 1;
        par.numberOfReferencePicturesInInterPredictionList0 = sliceParams->num_ref_idx_l0_active_minus1 + 1;
        par.weightedPredictionIndicator                     = picParams->pic_fields.weighted_bipred_idc;
        par.directPredictionType                            = sliceParams->direct_spatial_mv_pred_flag;

        // Set MFX_AVC_WEIGHTOFFSET_STATE_CMD_G6
        if (picParams->pic_fields.weighted_bipred_idc != 1)
        {
            // luma/chroma_log2_weight_denoms need to be set to default value in the case of implicit mode
            par.log2WeightDenomChroma = 5;
            par.log2WeightDenomLuma   = 5;
        }
    }

    DECODE_CHK_STATUS(m_mfxItf->MHW_ADDCMD_F(MFX_AVC_SLICE_STATE)(&cmdBuffer));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS AvcDecodeSlcPkt::AddCmd_AVC_BSD_OBJECT(MOS_COMMAND_BUFFER &cmdBuffer, uint32_t slcIdx)
{
    DECODE_FUNC_CALL();
    auto &parSlice    = m_mfxItf->MHW_GETPAR_F(MFX_AVC_SLICE_STATE)();
    auto sliceParams  = m_avcSliceParams + slcIdx;
    m_LastsliceFlag   = parSlice.isLastSlice;
    if (parSlice.shortFormatInUse)
    {
        if (parSlice.fullFrameData)
        {
            m_IndirectBsdDataLength       = parSlice.Length;
            m_IndirectBsdDataStartAddress = sliceParams->slice_data_offset;
        }
        else
        {
            m_IndirectBsdDataLength       = parSlice.Length + 1 - m_osInterface->dwNumNalUnitBytesIncluded;
            m_IndirectBsdDataStartAddress = sliceParams->slice_data_offset - 1 + m_osInterface->dwNumNalUnitBytesIncluded;
        }
    }
    else
    {
        m_IndirectBsdDataLength       = parSlice.Length;
        m_IndirectBsdDataStartAddress = sliceParams->slice_data_offset + parSlice.Offset;
        m_FirstMacroblockMbBitOffset  = sliceParams->slice_data_bit_offset;
        if (!parSlice.intelEntrypointInUse)
        {
            parSlice.Offset -= (m_osInterface->dwNumNalUnitBytesIncluded - 1);
            m_IndirectBsdDataLength += parSlice.Offset;
            m_IndirectBsdDataStartAddress -= parSlice.Offset;
            m_FirstMbByteOffsetOfSliceDataOrSliceHeader = parSlice.Offset;
        }
    }
    m_decodeInUse     = true;
    m_pAvcSliceParams = sliceParams;
    SETPAR_AND_ADDCMD(MFD_AVC_BSD_OBJECT, m_mfxItf, &cmdBuffer);
    return MOS_STATUS_SUCCESS;
}
MOS_STATUS AvcDecodeSlcPkt::AddCmd_AVC_PHANTOM_SLICE(MOS_COMMAND_BUFFER &cmdBuffer, uint32_t slcIdx)
{
    DECODE_FUNC_CALL();
    PCODEC_AVC_SLICE_PARAMS slc = m_avcSliceParams + slcIdx;
    if (!m_avcBasicFeature->IsAvcISlice(slc->slice_type))
    {
        DECODE_CHK_STATUS(AddCmd_AVC_SLICE_REF_IDX(cmdBuffer, slcIdx));
        DECODE_CHK_STATUS(AddCmd_AVC_SLICE_WEIGHT_OFFSET(cmdBuffer, slcIdx));
    }
    auto &par                            = m_mfxItf->MHW_GETPAR_F(MFX_AVC_SLICE_STATE)();
    par                                  = {};
    par.disableDeblockingFilterIndicator = slc->disable_deblocking_filter_idc;
    par.sliceBetaOffsetDiv2              = slc->slice_beta_offset_div2;
    par.sliceAlphaC0OffsetDiv2           = slc->slice_alpha_c0_offset_div2;
    par.intelEntrypointInUse             = m_avcPipeline->m_intelEntrypointInUse;
    par.picIdRemappingInUse              = m_avcBasicFeature->m_picIdRemappingInUse;
    par.shortFormatInUse                 = false;
    par.presDataBuffer                   = &m_avcBasicFeature->m_resDataBuffer.OsResource;
    par.avcPicParams                     = m_avcPicParams;
    par.mvcExtPicParams                  = m_avcBasicFeature->m_mvcExtPicParams;
    par.avcPicIdx                        = &m_avcBasicFeature->m_refFrames.m_avcPicIdx[0];
    par.phantomSlice                     = true;
    par.totalBytesConsumed               = 0;
    par.avcSliceParams                   = slc;

    par.Offset     = 0;
    par.Length     = slc->slice_data_offset;
    par.nextOffset = slc->slice_data_offset;
    par.nextLength = slc->slice_data_size;

    SetAndAddAvcSliceState(cmdBuffer, slcIdx);
    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(MFX_AVC_WEIGHTOFFSET_STATE, AvcDecodeSlcPkt)
{
    params.decodeInUse = true;
    params.uiList      = m_listID;
    auto slc = m_avcSliceParams + m_curSliceNum;
    MOS_SecureMemcpy(
        &params.Weights,
        sizeof(params.Weights),
        &slc->Weights,
        sizeof(slc->Weights));
    //The correct explicit calculation (like in Cantiga)
    for (auto i = 0; i < CODEC_MAX_NUM_REF_FIELD; i++)
    {
        params.weightoffset[3 * i] = params.Weights[params.uiList][i][0][0] & 0xFFFF;               // Y Weight
        params.weightoffset[3 * i] |= (params.Weights[params.uiList][i][0][1] & 0xFFFF) << 16;      //Y Offset
        params.weightoffset[3 * i + 1] = params.Weights[params.uiList][i][1][0] & 0xFFFF;           // Cb weight
        params.weightoffset[3 * i + 1] |= (params.Weights[params.uiList][i][1][1] & 0xFFFF) << 16;  // Cb offset
        params.weightoffset[3 * i + 2] = params.Weights[params.uiList][i][2][0] & 0xFFFF;           // Cr weight
        params.weightoffset[3 * i + 2] |= (params.Weights[params.uiList][i][2][1] & 0xFFFF) << 16;  // Cr offset
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS AvcDecodeSlcPkt::AddCmd_AVC_SLICE_WEIGHT_OFFSET(MOS_COMMAND_BUFFER &cmdBuffer, uint32_t slcIdx)
{
    DECODE_FUNC_CALL();
    PCODEC_AVC_SLICE_PARAMS slc = m_avcSliceParams + slcIdx;
    if (m_avcBasicFeature->IsAvcPSlice(slc->slice_type) &&
        m_avcPicParams->pic_fields.weighted_pred_flag == 1)
    {
        m_listID = 0;
        SETPAR_AND_ADDCMD(MFX_AVC_WEIGHTOFFSET_STATE, m_mfxItf, &cmdBuffer);
    }
    if (m_avcBasicFeature->IsAvcBSlice(slc->slice_type) &&
        m_avcPicParams->pic_fields.weighted_bipred_idc == 1)
    {
        m_listID = 0;
        SETPAR_AND_ADDCMD(MFX_AVC_WEIGHTOFFSET_STATE, m_mfxItf, &cmdBuffer);
        m_listID = 1;
        SETPAR_AND_ADDCMD(MFX_AVC_WEIGHTOFFSET_STATE, m_mfxItf, &cmdBuffer);
    }
    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(MFX_AVC_REF_IDX_STATE, AvcDecodeSlcPkt)
{
    auto slc = m_avcSliceParams + m_curSliceNum;
    params.CurrPic                 = m_avcPicParams->CurrPic;
    params.uiList                  = m_listID;
    if (params.uiList == 0)
    {
        params.numRefForList[params.uiList] = slc->num_ref_idx_l0_active_minus1 + 1;
    }
    if (params.uiList == 1)
    {
        params.numRefForList[params.uiList] = slc->num_ref_idx_l1_active_minus1 + 1;
    }
    MOS_SecureMemcpy(
        &params.refPicList,
        sizeof(params.refPicList),
        &slc->RefPicList,
        sizeof(slc->RefPicList));
    params.pAvcPicIdx            = &m_avcBasicFeature->m_refFrames.m_avcPicIdx[0];
    params.avcRefList            = (void **)m_avcBasicFeature->m_refFrames.m_refList;
    params.intelEntrypointInUse = m_avcPipeline->m_intelEntrypointInUse;
    params.picIdRemappingInUse  = m_avcBasicFeature->m_picIdRemappingInUse;
     // Need to add an empty MFX_AVC_REF_IDX_STATE_CMD for dummy reference on I-Frame
    if (!params.dummyReference)
    {

        CODEC_REF_LIST **avcRefList         = (CODEC_REF_LIST **)params.avcRefList;
        AvcRefListWrite *cmdAvcRefListWrite = (AvcRefListWrite *)&(params.referenceListEntry);

        uint8_t picIDOneOnOneMapping = 0;

        for (uint32_t i = 0; i < params.numRefForList[params.uiList]; i++)
        {
            uint8_t idx = params.refPicList[params.uiList][i].FrameIdx;

            if (!params.intelEntrypointInUse)
            {
                if (idx >= CODEC_MAX_NUM_REF_FRAME)
                {
                    DECODE_ASSERT(false);  // Idx must be within 0 to 15
                    DECODE_ASSERTMESSAGE("Idx must be within 0 to 15!");
                    idx = 0;
                }

                idx = params.pAvcPicIdx[idx].ucPicIdx;
            }

            uint8_t picID = params.picIdRemappingInUse ? params.refPicList[params.uiList][i].FrameIdx : avcRefList[idx]->ucFrameId;

            // When one on one ref idx mapping is enabled, program picID count from 0, 2 ...
            if (params.oneOnOneMapping)
            {
                picID = picIDOneOnOneMapping;
                picIDOneOnOneMapping += 2;
            }
            cmdAvcRefListWrite->Ref[i].frameStoreID = picID;
            cmdAvcRefListWrite->Ref[i].bottomField =
                CodecHal_PictureIsBottomField(params.refPicList[params.uiList][i]);
            cmdAvcRefListWrite->Ref[i].fieldPicFlag =
                CodecHal_PictureIsField(params.refPicList[params.uiList][i]);
            cmdAvcRefListWrite->Ref[i].longTermFlag =
                CodecHal_PictureIsLongTermRef(avcRefList[idx]->RefPic);
            cmdAvcRefListWrite->Ref[i].nonExisting = 0;
        }

        for (auto i = params.numRefForList[params.uiList]; i < 32; i++)
        {
            cmdAvcRefListWrite->Ref[i].value = 0x80;
        }
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS AvcDecodeSlcPkt::AddCmd_AVC_SLICE_REF_IDX(MOS_COMMAND_BUFFER &cmdBuffer, uint32_t slcIdx)
{
    DECODE_FUNC_CALL();
    PCODEC_AVC_SLICE_PARAMS slc = m_avcSliceParams + slcIdx;
    m_listID = 0;
    SETPAR_AND_ADDCMD(MFX_AVC_REF_IDX_STATE, m_mfxItf, &cmdBuffer);
    if (m_avcBasicFeature->IsAvcBSlice(slc->slice_type))
    {
        m_listID = 1;
        SETPAR_AND_ADDCMD(MFX_AVC_REF_IDX_STATE, m_mfxItf, &cmdBuffer);
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS AvcDecodeSlcPkt::AddCmd_AVC_SLICE_Addr(MOS_COMMAND_BUFFER &cmdBuffer, uint32_t slcIdx)
{
    DECODE_FUNC_CALL();
    SET_AVC_SLICE_STATE(cmdBuffer, slcIdx);
    auto &parSlice           = m_mfxItf->MHW_GETPAR_F(MFX_AVC_SLICE_STATE)();
    auto &parSliceAddr       = m_mfxItf->MHW_GETPAR_F(MFD_AVC_SLICEADDR)();
    parSliceAddr.decodeInUse = true;
    if (parSlice.fullFrameData)
    {
        parSliceAddr.IndirectBsdDataLength       = parSlice.nextLength;
        parSliceAddr.IndirectBsdDataStartAddress = parSlice.nextOffset;
    }
    else
    {
        parSliceAddr.IndirectBsdDataLength       = parSlice.nextLength + 1 - m_osInterface->dwNumNalUnitBytesIncluded;
        parSliceAddr.IndirectBsdDataStartAddress = parSlice.nextOffset - 1 + m_osInterface->dwNumNalUnitBytesIncluded;
    }
    parSliceAddr.presDataBuffer       = parSlice.presDataBuffer;
    parSliceAddr.dwSliceIndex         = parSlice.sliceIndex;
    parSliceAddr.dwTotalBytesConsumed = parSlice.totalBytesConsumed;
    parSliceAddr.avcSliceParams       = parSlice.avcSliceParams;
    if (!parSlice.isLastSlice)
    {
        DECODE_CHK_STATUS(m_mfxItf->MHW_ADDCMD_F(MFD_AVC_SLICEADDR)(&cmdBuffer));
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS AvcDecodeSlcPkt::CalculateCommandSize(uint32_t &commandBufferSize,
    uint32_t &                                             requestedPatchListSize)
{
    DECODE_FUNC_CALL();

    commandBufferSize      = m_sliceStatesSize;
    requestedPatchListSize = m_slicePatchListSize;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS AvcDecodeSlcPkt::CalculateSliceStateCommandSize()
{
    DECODE_FUNC_CALL();

    // Slice Level Commands
    DECODE_CHK_STATUS(m_hwInterface->GetMfxPrimitiveCommandsDataSize(CODECHAL_DECODE_MODE_AVCVLD, &m_sliceStatesSize, &m_slicePatchListSize, m_avcBasicFeature->m_shortFormatInUse));

    return MOS_STATUS_SUCCESS;
}

}  // namespace decode
