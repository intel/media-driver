/*
* Copyright (c) 2018-2021, Intel Corporation
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
//! \file     decode_avc_pipeline.cpp
//! \brief    Defines the interface for avc decode pipeline
//!
#include "decode_avc_pipeline.h"
#include "decode_utils.h"
#include "media_user_settings_mgr_g12.h"
#include "codechal_setting.h"
#include "decode_avc_feature_manager.h"
#include "decode_huc_packet_creator_base.h" 

namespace decode
{
    AvcPipeline::AvcPipeline(
        CodechalHwInterface *   hwInterface,
        CodechalDebugInterface *debugInterface)
        : DecodePipeline(hwInterface, debugInterface)
    {

    }

    MOS_STATUS AvcPipeline::Initialize(void *settings)
    {
        DECODE_FUNC_CALL();
        DECODE_CHK_STATUS(DecodePipeline::Initialize(settings));

        m_basicFeature = dynamic_cast<AvcBasicFeature*>(m_featureManager->GetFeature(FeatureIDs::basicFeature));
        DECODE_CHK_NULL(m_basicFeature);

        // Create basic GPU context
        DecodeScalabilityPars scalPars;
        MOS_ZeroMemory(&scalPars, sizeof(scalPars));
        DECODE_CHK_STATUS(m_mediaContext->SwitchContext(VdboxDecodeFunc, &scalPars, &m_scalability));
        m_decodeContext = m_osInterface->pfnGetGpuContext(m_osInterface);

        auto *codecSettings = (CodechalSetting*)settings;
        DECODE_CHK_NULL(codecSettings);
        m_intelEntrypointInUse = (codecSettings->intelEntrypointInUse) ? true : false;
        m_shortFormatInUse     = (codecSettings->shortFormatInUse) ? true : false;

        HucPacketCreatorBase *hucPktCreator = dynamic_cast<HucPacketCreatorBase *>(this);
        DECODE_CHK_NULL(hucPktCreator);
        m_formatMonoPicPkt  = hucPktCreator->CreateHucCopyPkt(this, m_task, m_hwInterface);
        DECODE_CHK_NULL(m_formatMonoPicPkt);
        MediaPacket *packet = dynamic_cast<MediaPacket *>(m_formatMonoPicPkt);
        DECODE_CHK_NULL(packet);
        DECODE_CHK_STATUS(RegisterPacket(DecodePacketId(this, avcFormatMonoPicPktId), packet));
        DECODE_CHK_STATUS(packet->Init());

        return MOS_STATUS_SUCCESS;
    }

    static uint32_t LinearToYTiledAddress(
                uint32_t x,
                uint32_t y,
                uint32_t pitch)
    {
        uint32_t tileW = 128;
        uint32_t tileH = 32;

        uint32_t tileSize = tileW * tileH;

        uint32_t rowSize = (pitch / tileW) * tileSize;

        uint32_t xOffWithinTile = x % tileW;
        uint32_t yOffWithinTile = y % tileH;

        uint32_t tileNumberInX = x / tileW;
        uint32_t tileNumberInY = y / tileH;

        uint32_t tileOffset =
                    rowSize * tileNumberInY +
                    tileSize * tileNumberInX +
                    tileH * 16 * (xOffWithinTile / 16) +
                    yOffWithinTile * 16 +
                    (xOffWithinTile % 16);

        return tileOffset;
    }

    MOS_STATUS AvcPipeline::Prepare(void *params)
    {
        DECODE_FUNC_CALL();
        DECODE_CHK_NULL(params);

        DECODE_CHK_STATUS(DecodePipeline::Prepare(params));

        if (m_basicFeature->m_avcPicParams->seq_fields.chroma_format_idc == avcChromaFormatMono)
        {
            uint32_t height = m_basicFeature->m_destSurface.dwHeight;
            uint32_t pitch = m_basicFeature->m_destSurface.dwPitch;
            uint32_t chromaHeight = height >> 1;
            uint32_t frameHeight = MOS_ALIGN_CEIL(height, 16);
            uint32_t alignedFrameHeight = MOS_ALIGN_CEIL(frameHeight, MOS_YTILE_H_ALIGNMENT);
            uint32_t frameSize = pitch * MOS_ALIGN_CEIL((frameHeight + chromaHeight), MOS_YTILE_H_ALIGNMENT);

            uint32_t uvblockHeight = CODECHAL_MACROBLOCK_HEIGHT;
            uint32_t uvrowSize = pitch * uvblockHeight * 2;
            uint32_t dstOffset = 0, x = 0, uvsize = 0;

            //update decode output surface's cpTag before decode submitbuffer, pfnMediaCopyResource2D can decide clear/secure workload by output surface's cptag.
            if (m_osInterface->osCpInterface && m_osInterface->osCpInterface->IsHMEnabled())
            {
                DECODE_CHK_STATUS(m_osInterface->osCpInterface->SetResourceEncryption(&m_basicFeature->m_destSurface.OsResource, true));
            }

            HucCopyPktItf::HucCopyParams copyParams = {};

            if (frameHeight % MOS_YTILE_H_ALIGNMENT)
            {
                dstOffset = LinearToYTiledAddress(x, frameHeight, pitch);

                if (!m_basicFeature->m_usingVeRing)
                {
                    copyParams.srcBuffer  = &(m_basicFeature->m_resMonoPicChromaBuffer->OsResource);
                    copyParams.srcOffset  = 0;
                    copyParams.destBuffer = &(m_basicFeature->m_destSurface.OsResource);
                    copyParams.destOffset = dstOffset;
                    copyParams.copyLength = uvrowSize;
                    m_formatMonoPicPkt->PushCopyParams(copyParams);
                }
                else
                {
                    m_osInterface->pfnMediaCopyResource2D(
                        m_osInterface,
                        &m_basicFeature->m_resMonoPicChromaBuffer->OsResource,
                        &m_basicFeature->m_destSurface.OsResource,
                        pitch,
                        uvblockHeight * 2,
                        0,
                        dstOffset,
                        16,
                        false);
                }
            }

            dstOffset = m_basicFeature->m_destSurface.UPlaneOffset.iSurfaceOffset;
            uvsize    = frameSize - pitch * alignedFrameHeight;

            if (!m_basicFeature->m_usingVeRing)
            {
                copyParams.srcBuffer  = &(m_basicFeature->m_resMonoPicChromaBuffer->OsResource);
                copyParams.srcOffset  = 0;
                copyParams.destBuffer = &(m_basicFeature->m_destSurface.OsResource);
                copyParams.destOffset = dstOffset;
                copyParams.copyLength = uvsize;
                m_formatMonoPicPkt->PushCopyParams(copyParams);
            }
            else
            {
                m_osInterface->pfnMediaCopyResource2D(
                    m_osInterface,
                    &m_basicFeature->m_resMonoPicChromaBuffer->OsResource,
                    &m_basicFeature->m_destSurface.OsResource,
                    pitch,
                    uvsize / pitch,
                    0,
                    dstOffset,
                    16,
                    false);
            }
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AvcPipeline::UserFeatureReport()
    {
        DECODE_FUNC_CALL();
        DECODE_CHK_STATUS(DecodePipeline::UserFeatureReport());
    #if (_DEBUG || _RELEASE_INTERNAL)
        WriteUserFeature(__MEDIA_USER_FEATURE_VALUE_APOGEIOS_AVCD_ENABLE_ID, 1, m_osInterface->pOsContext);
    #endif
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AvcPipeline::Uninitialize()
    {
        DECODE_FUNC_CALL();

        return DecodePipeline::Uninitialize();
    }

    MOS_STATUS AvcPipeline::ActivateDecodePackets()
    {
        DECODE_FUNC_CALL();

        bool immediateSubmit = false;

        if (m_basicFeature->m_avcPicParams->seq_fields.chroma_format_idc == avcChromaFormatMono && !m_basicFeature->m_usingVeRing)
        {
            DECODE_CHK_STATUS(ActivatePacket(DecodePacketId(this, avcFormatMonoPicPktId), immediateSubmit, 0, 0));
        }

        for (uint8_t curPass = 0; curPass < GetPassNum(); curPass++)
        {
            DECODE_CHK_STATUS(ActivatePacket(DecodePacketId(this, avcDecodePacketId), immediateSubmit, curPass, 0));
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AvcPipeline::CreateFeatureManager()
    {
        DECODE_FUNC_CALL();
        m_featureManager = MOS_New(DecodeAvcFeatureManager, m_allocator, m_hwInterface);
        DECODE_CHK_NULL(m_featureManager);
        return MOS_STATUS_SUCCESS;
    }

     MOS_STATUS AvcPipeline::CreateSubPackets(DecodeSubPacketManager& subPacketManager, CodechalSetting &codecSettings)
    {
        DECODE_FUNC_CALL();

        DECODE_CHK_STATUS(DecodePipeline::CreateSubPackets(subPacketManager, codecSettings));

        return MOS_STATUS_SUCCESS;
    }

    AvcPipeline::AvcDecodeMode AvcPipeline::GetDecodeMode()
    {
        return m_decodeMode;
    }

    bool AvcPipeline::IsShortFormat()
    {
        return m_shortFormatInUse;
    }

#if USE_CODECHAL_DEBUG_TOOL
    MOS_STATUS AvcPipeline::DumpPicParams(
        PCODEC_AVC_PIC_PARAMS picParams)
    {
        DECODE_FUNC_CALL();

        if (!m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrPicParams))
        {
            return MOS_STATUS_SUCCESS;
        }

        CODECHAL_DEBUG_CHK_NULL(picParams);

        std::ostringstream oss;
        oss.setf(std::ios::showbase | std::ios::uppercase);

        oss << "CurrPic FrameIdx: " << std::dec << +picParams->CurrPic.FrameIdx << std::endl;
        oss << "CurrPic PicFlags: " << std::hex << +picParams->CurrPic.PicFlags << std::endl;

        //Dump RefFrameList[15]
        for (uint8_t i = 0; i < 15; ++i)
        {
            oss << "RefFrameList[" << std::dec << +i << "] FrameIdx:" << +picParams->RefFrameList[i].FrameIdx << std::endl;
            oss << "RefFrameList[" << +i << "] PicFlags:" << std::hex << +picParams->RefFrameList[i].PicFlags << std::endl;
        }

        oss << "pic_width_in_mbs_minus1: " << std::dec << +picParams->pic_width_in_mbs_minus1 << std::endl;
        oss << "pic_height_in_mbs_minus1: " << +picParams->pic_height_in_mbs_minus1 << std::endl;
        oss << "bit_depth_luma_minus8: " << +picParams->bit_depth_luma_minus8 << std::endl;
        oss << "bit_depth_chroma_minus8: " << +picParams->bit_depth_chroma_minus8 << std::endl;
        oss << "num_ref_frames: " << +picParams->num_ref_frames << std::endl;
        oss << "CurrFieldOrderCnt: " << +picParams->CurrFieldOrderCnt[0] << std::endl;
        oss << "CurrFieldOrderCnt: " << +picParams->CurrFieldOrderCnt[1] << std::endl;

        //Dump FieldOrderCntList (16x2)
        for (uint8_t i = 0; i < 2; ++i)
        {
            oss << "FieldOrderCntList[" << +i << "]:";
            for (uint8_t j = 0; j < 16; j++)
                oss << +picParams->FieldOrderCntList[j][i] << " ";
            oss << std::endl;
        }

        //Dump seq_fields
        oss << "seq_fields value: " << +picParams->seq_fields.value << std::endl;
        oss << "chroma_format_idc: " << +picParams->seq_fields.chroma_format_idc << std::endl;
        oss << "residual_colour_transform_flag: " << std::hex << +picParams->seq_fields.residual_colour_transform_flag << std::endl;
        oss << "frame_mbs_only_flag: " << std::hex << +picParams->seq_fields.frame_mbs_only_flag << std::endl;
        oss << "mb_adaptive_frame_field_flag: " << std::hex << +picParams->seq_fields.mb_adaptive_frame_field_flag << std::endl;
        oss << "direct_8x8_inference_flag: " << std::hex << +picParams->seq_fields.direct_8x8_inference_flag << std::endl;
        oss << "log2_max_frame_num_minus4: " << std::dec << +picParams->seq_fields.log2_max_frame_num_minus4 << std::endl;
        oss << "pic_order_cnt_type: " << +picParams->seq_fields.pic_order_cnt_type << std::endl;
        oss << "log2_max_pic_order_cnt_lsb_minus4: " << +picParams->seq_fields.log2_max_pic_order_cnt_lsb_minus4 << std::endl;
        oss << "delta_pic_order_always_zero_flag: " << std::hex << +picParams->seq_fields.delta_pic_order_always_zero_flag << std::endl;
        oss << "num_slice_groups_minus1:" << std::dec << +picParams->num_slice_groups_minus1 << std::endl;
        oss << "slice_group_map_type:" << std::dec << +picParams->slice_group_map_type << std::endl;
        oss << "slice_group_change_rate_minus1:" << std::dec << +picParams->slice_group_change_rate_minus1 << std::endl;
        oss << "pic_init_qp_minus26:" << std::dec << +picParams->pic_init_qp_minus26 << std::endl;
        oss << "chroma_qp_index_offset:" << std::dec << +picParams->chroma_qp_index_offset << std::endl;
        oss << "second_chroma_qp_index_offset:" << std::dec << +picParams->second_chroma_qp_index_offset << std::endl;

        //Dump pic_fields
        oss << "pic_fields value: " << std::dec << +picParams->pic_fields.value << std::endl;
        oss << "entropy_coding_mode_flag: " << std::hex << +picParams->pic_fields.entropy_coding_mode_flag << std::endl;
        oss << "weighted_pred_flag: " << std::hex << +picParams->pic_fields.weighted_pred_flag << std::endl;
        oss << "weighted_bipred_idc: " << std::dec << +picParams->pic_fields.weighted_bipred_idc << std::endl;
        oss << "transform_8x8_mode_flag: " << std::hex << +picParams->pic_fields.transform_8x8_mode_flag << std::endl;
        oss << "field_pic_flag: " << std::hex << +picParams->pic_fields.field_pic_flag << std::endl;
        oss << "constrained_intra_pred_flag: " << std::hex << +picParams->pic_fields.constrained_intra_pred_flag << std::endl;
        oss << "pic_order_present_flag: " << std::hex << +picParams->pic_fields.pic_order_present_flag << std::endl;
        oss << "deblocking_filter_control_present_flag: " << std::hex << +picParams->pic_fields.deblocking_filter_control_present_flag << std::endl;
        oss << "redundant_pic_cnt_present_flag: " << std::hex << +picParams->pic_fields.redundant_pic_cnt_present_flag << std::endl;
        oss << "reference_pic_flag: " << std::hex << +picParams->pic_fields.reference_pic_flag << std::endl;
        oss << "IntraPicFlag: " << std::hex << +picParams->pic_fields.IntraPicFlag << std::endl;

        //Dump Short format specific
        oss << "num_ref_idx_l0_active_minus1: " << std::dec << +picParams->num_ref_idx_l0_active_minus1 << std::endl;
        oss << "num_ref_idx_l1_active_minus1: " << std::dec << +picParams->num_ref_idx_l1_active_minus1 << std::endl;
        oss << "NonExistingFrameFlags: " << std::hex << +picParams->NonExistingFrameFlags << std::endl;
        oss << "UsedForReferenceFlags: " << std::hex << +picParams->UsedForReferenceFlags << std::endl;
        oss << "frame_num: " << std::dec << +picParams->frame_num << std::endl;
        oss << "StatusReportFeedbackNumber: " << std::dec << +picParams->StatusReportFeedbackNumber << std::endl;

        //Dump FrameNumList[16]
        oss << "scaling_list_present_flag_buffer:";
        for (uint8_t i = 0; i < 16; i++)
            oss << std::hex << picParams->FrameNumList[i];
        oss << std::endl;

        const char *fileName = m_debugInterface->CreateFileName(
            "_DEC",
            CodechalDbgBufferType::bufPicParams,
            CodechalDbgExtType::txt);

        std::ofstream ofs(fileName, std::ios::out);
        ofs << oss.str();
        ofs.close();
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AvcPipeline::DumpSliceParams(
        PCODEC_AVC_SLICE_PARAMS sliceParams,
        uint32_t                numSlices)
    {
        DECODE_FUNC_CALL();

        if (!m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrSlcParams))
        {
            return MOS_STATUS_SUCCESS;
        }

        CODECHAL_DEBUG_CHK_NULL(sliceParams);

        PCODEC_AVC_SLICE_PARAMS sliceControl = nullptr;

        std::ostringstream oss;
        oss.setf(std::ios::showbase | std::ios::uppercase);

        for (uint16_t j = 0; j < numSlices; j++)
        {
            sliceControl = &sliceParams[j];

            oss << "Data for Slice number = " << std::dec << +j << std::endl;
            oss << "slice_data_size: " << std::dec << +sliceControl->slice_data_size << std::endl;
            oss << "slice_data_offset: " << std::dec << +sliceControl->slice_data_offset << std::endl;
            //Dump Long format specific
            oss << "slice_data_bit_offset: " << std::dec << +sliceControl->slice_data_bit_offset << std::endl;
            oss << "first_mb_in_slice: " << std::dec << +sliceControl->first_mb_in_slice << std::endl;
            oss << "NumMbsForSlice: " << std::dec << +sliceControl->NumMbsForSlice << std::endl;
            oss << "slice_type: " << std::dec << +sliceControl->slice_type << std::endl;
            oss << "direct_spatial_mv_pred_flag: " << std::hex << +sliceControl->direct_spatial_mv_pred_flag << std::endl;
            oss << "num_ref_idx_l0_active_minus1: " << std::dec << +sliceControl->num_ref_idx_l0_active_minus1 << std::endl;
            oss << "num_ref_idx_l1_active_minus1: " << std::dec << +sliceControl->num_ref_idx_l1_active_minus1 << std::endl;
            oss << "cabac_init_idc: " << std::dec << +sliceControl->cabac_init_idc << std::endl;
            oss << "slice_qp_delta: " << std::dec << +sliceControl->slice_qp_delta << std::endl;
            oss << "disable_deblocking_filter_idc: " << std::dec << +sliceControl->disable_deblocking_filter_idc << std::endl;
            oss << "slice_alpha_c0_offset_div2: " << std::dec << +sliceControl->slice_alpha_c0_offset_div2 << std::endl;
            oss << "slice_beta_offset_div2: " << std::dec << +sliceControl->slice_beta_offset_div2 << std::endl;

            //Dump RefPicList[2][32]
            for (uint8_t i = 0; i < 32; ++i)
            {
                oss << "RefPicList[0][" << std::dec << +i << "] FrameIdx: " << std::dec << +sliceControl->RefPicList[0][i].FrameIdx << std::endl;
                oss << "RefPicList[0][" << std::dec << +i << "] PicFlags: " << std::hex << +sliceControl->RefPicList[0][i].PicFlags << std::endl;
                oss << "RefPicList[1][" << std::dec << +i << "] FrameIdx: " << std::dec << +sliceControl->RefPicList[1][i].FrameIdx << std::endl;
                oss << "RefPicList[1][" << std::dec << +i << "] PicFlags: " << std::hex << +sliceControl->RefPicList[1][i].PicFlags << std::endl;
            }

            oss << "luma_log2_weight_denom: " << std::dec << +sliceControl->luma_log2_weight_denom << std::endl;
            oss << "chroma_log2_weight_denom: " << std::dec << +sliceControl->chroma_log2_weight_denom << std::endl;
            oss << "slice_id: " << std::dec << +sliceControl->slice_id << std::endl;

            //Dump Weights[2][32][3][2]
            for (uint8_t i = 0; i < 32; ++i)
            {
                oss << "Weights[0][" << std::dec << +i << "][0][0]: " << std::hex << +sliceControl->Weights[0][i][0][0] << std::endl;
                oss << "Weights[0][" << std::dec << +i << "][0][1]: " << std::hex << +sliceControl->Weights[0][i][0][1] << std::endl;
                oss << "Weights[0][" << std::dec << +i << "][1][0]: " << std::hex << +sliceControl->Weights[0][i][1][0] << std::endl;
                oss << "Weights[0][" << std::dec << +i << "][1][1]: " << std::hex << +sliceControl->Weights[0][i][1][1] << std::endl;
                oss << "Weights[1][" << std::dec << +i << "][0][0]: " << std::hex << +sliceControl->Weights[1][i][0][0] << std::endl;
                oss << "Weights[1][" << std::dec << +i << "][0][1]: " << std::hex << +sliceControl->Weights[1][i][0][1] << std::endl;
                oss << "Weights[1][" << std::dec << +i << "][1][0]: " << std::hex << +sliceControl->Weights[1][i][1][0] << std::endl;
                oss << "Weights[1][" << std::dec << +i << "][1][1]: " << std::hex << +sliceControl->Weights[1][i][1][1] << std::endl;
                oss << "Weights[0][" << std::dec << +i << "][2][0]: " << std::hex << +sliceControl->Weights[0][i][2][0] << std::endl;
                oss << "Weights[0][" << std::dec << +i << "][2][1]: " << std::hex << +sliceControl->Weights[0][i][2][1] << std::endl;
                oss << "Weights[1][" << std::dec << +i << "][2][0]: " << std::hex << +sliceControl->Weights[1][i][2][0] << std::endl;
                oss << "Weights[1][" << std::dec << +i << "][2][1]: " << std::hex << +sliceControl->Weights[1][i][2][1] << std::endl;
            }

            const char *fileName = m_debugInterface->CreateFileName(
                "_DEC",
                CodechalDbgBufferType::bufSlcParams,
                CodechalDbgExtType::txt);

            std::ofstream ofs;
            if (j == 0)
            {
                ofs.open(fileName, std::ios::out);
            }
            else
            {
                ofs.open(fileName, std::ios::app);
            }
            ofs << oss.str();
            ofs.close();
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AvcPipeline::DumpIQParams(
        PCODEC_AVC_IQ_MATRIX_PARAMS matrixData)
    {
        DECODE_FUNC_CALL();

        if (!m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrIqParams))
        {
            return MOS_STATUS_SUCCESS;
        }

        CODECHAL_DEBUG_CHK_NULL(matrixData);

        std::ostringstream oss;
        oss.setf(std::ios::showbase | std::ios::uppercase);

        uint32_t idx, idx2;
        // 4x4 block
        for (idx2 = 0; idx2 < 6; idx2++)
        {
            oss << "Qmatrix_H264_ScalingLists4x4[" << std::dec << +idx2 << "]:" << std::endl;
            for (idx = 0; idx < 12; idx += 4)
            {
                oss << "ScalingList4x4[" << std::dec << +idx / 4 << "]:";
                oss << std::hex << +matrixData->ScalingList4x4[idx2][idx] << " ";
                oss << std::hex << +matrixData->ScalingList4x4[idx2][idx + 1] << " ";
                oss << std::hex << +matrixData->ScalingList4x4[idx2][idx + 2] << " ";
                oss << std::hex << +matrixData->ScalingList4x4[idx2][idx + 3] << " ";
                oss << std::endl;
            }
            oss << std::endl;
        }
        // 8x8 block
        for (idx2 = 0; idx2 < 2; idx2++)
        {
            oss << "Qmatrix_H264_ScalingLists8x8[" << std::dec << +idx2 << "]:" << std::endl;
            for (idx = 0; idx < 56; idx += 8)
            {
                oss << "ScalingList8x8[" << std::dec << +idx / 8 << "]:";
                oss << std::hex << +matrixData->ScalingList8x8[idx2][idx] << " " ;
                oss << std::hex << +matrixData->ScalingList8x8[idx2][idx + 1] << " " ;
                oss << std::hex << +matrixData->ScalingList8x8[idx2][idx + 2] << " " ;
                oss << std::hex << +matrixData->ScalingList8x8[idx2][idx + 3] << " " ;
                oss << std::hex << +matrixData->ScalingList8x8[idx2][idx + 4] << " " ;
                oss << std::hex << +matrixData->ScalingList8x8[idx2][idx + 5] << " " ;
                oss << std::hex << +matrixData->ScalingList8x8[idx2][idx + 6] << " " ;
                oss << std::hex << +matrixData->ScalingList8x8[idx2][idx + 7] << " " ;
                oss << std::endl;
            }
            oss << std::endl;
        }

        const char *fileName = m_debugInterface->CreateFileName(
            "_DEC",
            CodechalDbgBufferType::bufIqParams,
            CodechalDbgExtType::txt);

        std::ofstream ofs(fileName, std::ios::out);
        ofs << oss.str();
        ofs.close();

        return MOS_STATUS_SUCCESS;
    }

#endif
}
