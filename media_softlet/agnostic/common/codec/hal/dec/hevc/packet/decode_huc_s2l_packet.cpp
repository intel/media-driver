/*
* Copyright (c) 2021-2022, Intel Corporation
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
//! \file     decode_huc_s2l_packet.cpp
//! \brief    Defines the interface for huc S2L packet
//!
#include "decode_huc_s2l_packet.h"
#include "mhw_vdbox.h"

namespace decode {

    MOS_STATUS HucS2lPkt::Init()
    {
        DECODE_FUNC_CALL();
        DECODE_CHK_NULL(m_hevcPipeline);

        DECODE_CHK_STATUS(DecodeHucBasic::Init());

        m_hevcBasicFeature = dynamic_cast<HevcBasicFeature*>(m_basicFeature);
        DECODE_CHK_NULL(m_hevcBasicFeature);

        DECODE_CHK_STATUS(m_statusReport->RegistObserver(this));

        MHW_VDBOX_STATE_CMDSIZE_PARAMS stateCmdSizeParams;
        stateCmdSizeParams.bShortFormat = true;
        DECODE_CHK_STATUS(m_hwInterface->GetHucStateCommandSize(m_hevcBasicFeature->m_mode, &m_pictureStatesSize,
                                                                 &m_picturePatchListSize, &stateCmdSizeParams));
        uint32_t cpCmdsize        = 0;
        uint32_t cpPatchListSize  = 0;
        m_hwInterface->GetCpInterface()->GetCpSliceLevelCmdSize(cpCmdsize, cpPatchListSize);
        m_sliceStatesSize += cpCmdsize;
        m_slicePatchListSize += cpPatchListSize;

        if (m_s2lControlTempMVRegionBuffer == nullptr)
        {
            m_s2lControlTempMVRegionBuffer = m_allocator->AllocateBuffer(
                sizeof(uint32_t), "S2lControlTempMVRegionBuffer", resourceInternalReadWriteCache, notLockableVideoMem);
            DECODE_CHK_NULL(m_s2lControlTempMVRegionBuffer);
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HucS2lPkt::Prepare()
    {
        DECODE_FUNC_CALL();

        m_hevcPicParams = m_hevcBasicFeature->m_hevcPicParams;
        DECODE_CHK_NULL(m_hevcPicParams);
        m_hevcSliceParams = m_hevcBasicFeature->m_hevcSliceParams;
        DECODE_CHK_NULL(m_hevcSliceParams);

        m_hevcRextPicParams = m_hevcBasicFeature->m_hevcRextPicParams;
        m_hevcSccPicParams  = m_hevcBasicFeature->m_hevcSccPicParams;

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HucS2lPkt::Destroy() 
    {
        return FreeResource();
    }

    MOS_STATUS HucS2lPkt::FreeResource()
    {
        DECODE_FUNC_CALL();

        if (m_allocator != nullptr)
        {
            DECODE_CHK_STATUS(m_allocator->Destroy(m_s2lControlTempMVRegionBuffer));
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HucS2lPkt::CalculateCommandSize(uint32_t &commandBufferSize, uint32_t &requestedPatchListSize)
    {
        DECODE_FUNC_CALL();

        commandBufferSize      = CalculateCommandBufferSize();
        requestedPatchListSize = CalculatePatchListSize();

        return MOS_STATUS_SUCCESS;
    }

    uint32_t HucS2lPkt::CalculateCommandBufferSize()
    {
        DECODE_FUNC_CALL();

        uint32_t commandBufferSize = m_pictureStatesSize +
                                     m_sliceStatesSize * (m_hevcBasicFeature->m_numSlices+1);

        return (commandBufferSize + COMMAND_BUFFER_RESERVED_SPACE);
    }

    uint32_t HucS2lPkt::CalculatePatchListSize()
    {
        DECODE_FUNC_CALL();

        if (!m_osInterface->bUsesPatchList)
        {
            return 0;
        }

        uint32_t requestedPatchListSize = m_picturePatchListSize +
                                          m_slicePatchListSize * (m_hevcBasicFeature->m_numSlices+1);

        return requestedPatchListSize;
    }

    MOS_STATUS HucS2lPkt::SetHucDmemPictureBss(HucHevcS2lPicBss &hucHevcS2LPicBss)
    {
        DECODE_FUNC_CALL();

        hucHevcS2LPicBss.pic_width_in_min_cbs_y                       = m_hevcPicParams->PicWidthInMinCbsY;
        hucHevcS2LPicBss.pic_height_in_min_cbs_y                      = m_hevcPicParams->PicHeightInMinCbsY;
        hucHevcS2LPicBss.log2_min_luma_coding_block_size_minus3       = m_hevcPicParams->log2_min_luma_coding_block_size_minus3;
        hucHevcS2LPicBss.log2_diff_max_min_luma_coding_block_size     = m_hevcPicParams->log2_diff_max_min_luma_coding_block_size;
        hucHevcS2LPicBss.chroma_format_idc                            = m_hevcPicParams->chroma_format_idc;
        hucHevcS2LPicBss.separate_colour_plane_flag                   = m_hevcPicParams->separate_colour_plane_flag;
        hucHevcS2LPicBss.bit_depth_luma_minus8                        = m_hevcPicParams->bit_depth_luma_minus8;
        hucHevcS2LPicBss.bit_depth_chroma_minus8                      = m_hevcPicParams->bit_depth_chroma_minus8;
        hucHevcS2LPicBss.log2_max_pic_order_cnt_lsb_minus4            = m_hevcPicParams->log2_max_pic_order_cnt_lsb_minus4;
        hucHevcS2LPicBss.sample_adaptive_offset_enabled_flag          = m_hevcPicParams->sample_adaptive_offset_enabled_flag;
        hucHevcS2LPicBss.num_short_term_ref_pic_sets                  = m_hevcPicParams->num_short_term_ref_pic_sets;
        hucHevcS2LPicBss.long_term_ref_pics_present_flag              = m_hevcPicParams->long_term_ref_pics_present_flag;
        hucHevcS2LPicBss.num_long_term_ref_pics_sps                   = m_hevcPicParams->num_long_term_ref_pic_sps;
        hucHevcS2LPicBss.sps_temporal_mvp_enable_flag                 = m_hevcPicParams->sps_temporal_mvp_enabled_flag;
        hucHevcS2LPicBss.num_ref_idx_l0_default_active_minus1         = m_hevcPicParams->num_ref_idx_l0_default_active_minus1;
        hucHevcS2LPicBss.num_ref_idx_l1_default_active_minus1         = m_hevcPicParams->num_ref_idx_l1_default_active_minus1;
        hucHevcS2LPicBss.pic_init_qp_minus26                          = m_hevcPicParams->init_qp_minus26;
        hucHevcS2LPicBss.dependent_slice_segments_enabled_flag        = m_hevcPicParams->dependent_slice_segments_enabled_flag;
        hucHevcS2LPicBss.cabac_init_present_flag                      = m_hevcPicParams->cabac_init_present_flag;
        hucHevcS2LPicBss.pps_slice_chroma_qp_offsets_present_flag     = m_hevcPicParams->pps_slice_chroma_qp_offsets_present_flag;
        hucHevcS2LPicBss.weighted_pred_flag                           = m_hevcPicParams->weighted_pred_flag;
        hucHevcS2LPicBss.weighted_bipred_flag                         = m_hevcPicParams->weighted_bipred_flag;
        hucHevcS2LPicBss.output_flag_present_flag                     = m_hevcPicParams->output_flag_present_flag;
        hucHevcS2LPicBss.tiles_enabled_flag                           = m_hevcPicParams->tiles_enabled_flag;
        hucHevcS2LPicBss.entropy_coding_sync_enabled_flag             = m_hevcPicParams->entropy_coding_sync_enabled_flag;
        hucHevcS2LPicBss.loop_filter_across_slices_enabled_flag       = m_hevcPicParams->pps_loop_filter_across_slices_enabled_flag;
        hucHevcS2LPicBss.deblocking_filter_override_enabled_flag      = m_hevcPicParams->deblocking_filter_override_enabled_flag;
        hucHevcS2LPicBss.pic_disable_deblocking_filter_flag           = m_hevcPicParams->pps_deblocking_filter_disabled_flag;
        hucHevcS2LPicBss.lists_modification_present_flag              = m_hevcPicParams->lists_modification_present_flag;
        hucHevcS2LPicBss.slice_segment_header_extension_present_flag  = m_hevcPicParams->slice_segment_header_extension_present_flag;
        hucHevcS2LPicBss.high_precision_offsets_enabled_flag          = 0;
        hucHevcS2LPicBss.chroma_qp_offset_list_enabled_flag           = 0;

        hucHevcS2LPicBss.CurrPicOrderCntVal = m_hevcPicParams->CurrPicOrderCntVal;
        for (uint32_t i = 0; i < CODEC_MAX_NUM_REF_FRAME_HEVC; i++)
        {
            hucHevcS2LPicBss.PicOrderCntValList[i] = m_hevcPicParams->PicOrderCntValList[i];
        }

        for (uint32_t i = 0; i < 8; i++)
        {
            hucHevcS2LPicBss.RefPicSetStCurrBefore[i] = m_hevcPicParams->RefPicSetStCurrBefore[i];
            hucHevcS2LPicBss.RefPicSetStCurrAfter[i]  = m_hevcPicParams->RefPicSetStCurrAfter[i];
            hucHevcS2LPicBss.RefPicSetLtCurr[i]       = m_hevcPicParams->RefPicSetLtCurr[i];
        }

        hucHevcS2LPicBss.RefFieldPicFlag      = m_hevcPicParams->RefFieldPicFlag;
        hucHevcS2LPicBss.RefBottomFieldFlag   = (uint8_t)m_hevcPicParams->RefBottomFieldFlag;
        hucHevcS2LPicBss.pps_beta_offset_div2 = m_hevcPicParams->pps_beta_offset_div2;
        hucHevcS2LPicBss.pps_tc_offset_div2   = m_hevcPicParams->pps_tc_offset_div2;
        hucHevcS2LPicBss.StRPSBits            = m_hevcPicParams->wNumBitsForShortTermRPSInSlice;

        if (m_hevcPicParams->tiles_enabled_flag)
        {
            hucHevcS2LPicBss.num_tile_columns_minus1 = m_hevcPicParams->num_tile_columns_minus1;
            hucHevcS2LPicBss.num_tile_rows_minus1    = m_hevcPicParams->num_tile_rows_minus1;

            const uint16_t *tileColWidth = m_hevcBasicFeature->m_tileCoding.GetTileColWidth();
            for (auto i = 0; i < HEVC_NUM_MAX_TILE_COLUMN; i++)
            {
                hucHevcS2LPicBss.column_width[i] = tileColWidth[i];
            }

            const uint16_t *tileRowHeight = m_hevcBasicFeature->m_tileCoding.GetTileRowHeight();
            for (auto i = 0; i < HEVC_NUM_MAX_TILE_ROW; i++)
            {
                hucHevcS2LPicBss.row_height[i] = tileRowHeight[i];
            }
        }

        hucHevcS2LPicBss.NumSlices                   = (uint16_t)m_hevcBasicFeature->m_numSlices;
        hucHevcS2LPicBss.num_extra_slice_header_bits = m_hevcPicParams->num_extra_slice_header_bits;

        int8_t *refIdxMapping = m_hevcBasicFeature->m_refFrames.m_refIdxMapping;
        for (auto i = 0; i < CODEC_MAX_NUM_REF_FRAME_HEVC; i++)
        {
            hucHevcS2LPicBss.RefIdxMapping[i] = refIdxMapping[i];
        }
        if(m_decodecp)
        {
            DECODE_CHK_STATUS(m_decodecp->SetHucDmemS2LPicBss(&hucHevcS2LPicBss.reserve, &(m_hevcBasicFeature->m_resDataBuffer.OsResource)));
        }
        else
        {
            hucHevcS2LPicBss.reserve.reserve_0 = 0;
            hucHevcS2LPicBss.reserve.reserve_1 = 0;
            hucHevcS2LPicBss.reserve.reserve_2 = 0;
            hucHevcS2LPicBss.reserve.reserve_3 = 0;
        }
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HucS2lPkt::SetHucDmemSliceBss(
        HucHevcS2lSliceBss (&hucHevcS2LSliceBss)[CODECHAL_HEVC_MAX_NUM_SLICES_LVL_6])
    {
        DECODE_FUNC_CALL();

        for (uint32_t i = 0; i < m_hevcBasicFeature->m_numSlices && i < CODECHAL_HEVC_MAX_NUM_SLICES_LVL_6; i++)
        {
            hucHevcS2LSliceBss[i].BSNALunitDataLocation = m_hevcSliceParams[i].slice_data_offset;
            hucHevcS2LSliceBss[i].SliceBytesInBuffer    = m_hevcSliceParams[i].slice_data_size;
            if(m_decodecp)
            {
                DECODE_CHK_STATUS(m_decodecp->SetHucDmemS2LSliceBss(&hucHevcS2LSliceBss[i].reserve, i, m_hevcSliceParams[i].slice_data_size, m_hevcSliceParams[i].slice_data_offset));
            }
        }
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HucS2lPkt::AddHucCpState(MOS_COMMAND_BUFFER &cmdBuffer, uint32_t index, CODEC_HEVC_SLICE_PARAMS &sliceParams)
    {
        if(m_decodecp)
        {
            DECODE_CHK_STATUS(m_decodecp->AddHucState(&cmdBuffer,
                &(m_hevcBasicFeature->m_resDataBuffer.OsResource),
                sliceParams.slice_data_size,
                sliceParams.slice_data_offset,
                index));
        }
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HucS2lPkt::AddCmd_HUC_IMEM_STATE(MOS_COMMAND_BUFFER &cmdBuffer)
    {
          DECODE_FUNC_CALL();  
          auto &par = m_hucItf->MHW_GETPAR_F(HUC_IMEM_STATE)();
          par                  = {};
          par.kernelDescriptor = m_vdboxHucHevcS2lKernelDescriptor;
          DECODE_CHK_STATUS(m_hucItf->MHW_ADDCMD_F(HUC_IMEM_STATE)(&cmdBuffer));

          auto &mfxWaitParams               = m_miItf->MHW_GETPAR_F(MFX_WAIT)();
          mfxWaitParams                     = {};
          mfxWaitParams.iStallVdboxPipeline = true;
          DECODE_CHK_STATUS((m_miItf->MHW_ADDCMD_F(MFX_WAIT)(&cmdBuffer)));

          return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HucS2lPkt::AddCmd_HUC_PIPE_MODE_SELECT(MOS_COMMAND_BUFFER &cmdBuffer)
    {
          DECODE_FUNC_CALL();
          //for gen 11+, we need to add MFX wait for both KIN and VRT before and after HUC Pipemode select...
          auto &mfxWaitParams               = m_miItf->MHW_GETPAR_F(MFX_WAIT)();
          mfxWaitParams                     = {};
          mfxWaitParams.iStallVdboxPipeline = true;
          DECODE_CHK_STATUS((m_miItf->MHW_ADDCMD_F(MFX_WAIT)(&cmdBuffer)));

          auto &par = m_hucItf->MHW_GETPAR_F(HUC_PIPE_MODE_SELECT)();
          par                  = {};
          par.streamOutEnabled = false;
          DECODE_CHK_STATUS(m_hucItf->MHW_ADDCMD_F(HUC_PIPE_MODE_SELECT)(&cmdBuffer));

          mfxWaitParams                     = {};
          mfxWaitParams.iStallVdboxPipeline = true;
          DECODE_CHK_STATUS((m_miItf->MHW_ADDCMD_F(MFX_WAIT)(&cmdBuffer)));
          return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(HUC_IND_OBJ_BASE_ADDR_STATE, HucS2lPkt)
    {
        DECODE_FUNC_CALL();

        params.DataSize   = m_hevcBasicFeature->m_dataSize;
        params.DataOffset = m_hevcBasicFeature->m_dataOffset;
        params.DataBuffer = &(m_hevcBasicFeature->m_resDataBuffer.OsResource);
        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(HUC_VIRTUAL_ADDR_STATE, HucS2lPkt)
    {
        DECODE_FUNC_CALL();
        
        PMHW_BATCH_BUFFER batchBuffer = m_hevcPipeline->GetSliceLvlCmdBuffer();
        DECODE_CHK_NULL(batchBuffer);
        params.regionParams[0].presRegion = &batchBuffer->OsResource;
        params.regionParams[0].isWritable = true;
  
        PMOS_BUFFER regionBuffer = m_s2lControlTempMVRegionBuffer;
        DECODE_CHK_NULL(regionBuffer);
        params.regionParams[1].presRegion = &regionBuffer->OsResource;
        params.regionParams[1].isWritable = true;

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(HUC_DMEM_STATE, HucS2lPkt)
    {
        DECODE_FUNC_CALL();
      
        params.hucDataSource = &m_s2lDmemBuffer->OsResource;
        params.dataLength    = MOS_ALIGN_CEIL(m_dmemTransferSize, CODECHAL_CACHELINE_SIZE);
        params.dmemOffset    = HUC_DMEM_OFFSET_RTOS_GEMS;
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HucS2lPkt::AddCmd_HUC_STREAM_OBJECT(MOS_COMMAND_BUFFER &cmdBuffer, CODEC_HEVC_SLICE_PARAMS sliceParams)
    {
        DECODE_FUNC_CALL();
       
        auto &par                        = m_hucItf->MHW_GETPAR_F(HUC_STREAM_OBJECT)();
        par                              = {};
        par.IndirectStreamInDataLength   = sliceParams.slice_data_size;
        par.StreamOut                    = 0;
        par.IndirectStreamInStartAddress = sliceParams.slice_data_offset;
        par.HucProcessing                = true;

        par.HucBitstreamEnable             = 1;
        par.EmulationPreventionByteRemoval = 1;
        par.StartCodeSearchEngine          = 1;
        par.StartCodeByte0                 = 0;
        par.StartCodeByte1                 = 0;
        par.StartCodeByte2                 = 1;

        DECODE_CHK_STATUS(m_hucItf->MHW_ADDCMD_F(HUC_STREAM_OBJECT)(&cmdBuffer));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HucS2lPkt::AddCmd_HUC_START(MOS_COMMAND_BUFFER &cmdBuffer, bool laststreamobject)
    {
        DECODE_FUNC_CALL();
        auto &par = m_hucItf->MHW_GETPAR_F(HUC_START)();
        par                  = {};
        par.lastStreamObject = laststreamobject;
        DECODE_CHK_STATUS(m_hucItf->MHW_ADDCMD_F(HUC_START)(&cmdBuffer));

        return MOS_STATUS_SUCCESS;
    }


#if USE_CODECHAL_DEBUG_TOOL
    MOS_STATUS HucS2lPkt::DumpHucS2l()
    {
        DECODE_FUNC_CALL();

        int32_t currentPass = m_pipeline->GetCurrentPass();

        CodechalDebugInterface *debugInterface = m_pipeline->GetDebugInterface();
        DECODE_CHK_NULL(debugInterface);

        DECODE_CHK_STATUS(debugInterface->DumpHucDmem(
            &m_s2lDmemBuffer->OsResource,
            m_dmemTransferSize,
            currentPass,
            hucRegionDumpDefault));

        return MOS_STATUS_SUCCESS;
    }
#endif
}
