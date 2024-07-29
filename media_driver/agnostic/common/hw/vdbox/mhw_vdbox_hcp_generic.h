/*
* Copyright (c) 2014-2018, Intel Corporation
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

//! \file     mhw_vdbox_hcp_generic.h
//! \brief    MHW interface for constructing HCP commands for the Vdbox engine
//! \details  Impelements shared Vdbox HCP command construction functions across all platforms as templates
//!

#ifndef _MHW_VDBOX_HCP_GENERIC_H_
#define _MHW_VDBOX_HCP_GENERIC_H_

#include "mhw_vdbox_hcp_interface.h"
#include "mhw_cp_interface.h"

//!  MHW Vdbox Hcp generic interface
/*!
This class defines the shared Hcp command construction functions across all platforms as templates
*/
template <class THcpCmds>
class MhwVdboxHcpInterfaceGeneric : public MhwVdboxHcpInterface
{
protected:
    static const uint32_t      m_vp9ScalingFactor = (1 << 14);
    static const uint32_t      m_rawUVPlaneAlignment = 4; //! starting Gen9 the alignment is relaxed to 4x instead of 16x
    static const uint32_t      m_reconUVPlaneAlignment = 8;
    static const uint32_t      m_uvPlaneAlignmentLegacy = 8; //! starting Gen9 the alignment is relaxed to 4x instead of 16x

    //!
    //! \brief    Constructor
    //!
    MhwVdboxHcpInterfaceGeneric(
        PMOS_INTERFACE osInterface,
        MhwMiInterface *miInterface,
        MhwCpInterface *cpInterface,
        bool decodeInUse) :
        MhwVdboxHcpInterface(osInterface, miInterface, cpInterface, decodeInUse)
    {
        MHW_FUNCTION_ENTER;
    }

    //!
    //! \brief   Destructor
    //!
    virtual ~MhwVdboxHcpInterfaceGeneric() {}

    MOS_STATUS AddHcpDecodeSurfaceStateCmd(
        PMOS_COMMAND_BUFFER              cmdBuffer,
        PMHW_VDBOX_SURFACE_PARAMS        params)
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        MHW_FUNCTION_ENTER;

        MHW_MI_CHK_NULL(m_osInterface);
        MHW_MI_CHK_NULL(params);
        MHW_ASSERT(params->Mode != CODECHAL_UNSUPPORTED_MODE);

        MHW_MI_CHK_NULL(params->psSurface);

        typename THcpCmds::HCP_SURFACE_STATE_CMD cmd;
        uint32_t uvPlaneAlignment = m_uvPlaneAlignmentLegacy;

        cmd.DW1.SurfaceId = params->ucSurfaceStateId;
        cmd.DW1.SurfacePitchMinus1 = params->psSurface->dwPitch - 1;

        if (params->ucSurfaceStateId == CODECHAL_HCP_SRC_SURFACE_ID)
        {
            uvPlaneAlignment = params->dwUVPlaneAlignment ? params->dwUVPlaneAlignment : m_rawUVPlaneAlignment;
        }
        else
        {
            uvPlaneAlignment = params->dwUVPlaneAlignment ? params->dwUVPlaneAlignment : m_reconUVPlaneAlignment;
        }

        cmd.DW2.YOffsetForUCbInPixel =
            MOS_ALIGN_CEIL(params->psSurface->UPlaneOffset.iYOffset, uvPlaneAlignment);

        MHW_MI_CHK_STATUS(m_osInterface->pfnAddCommand(cmdBuffer, &cmd, cmd.byteSize));

        return eStatus;
    }

    MOS_STATUS AddHcpEncodeSurfaceStateCmd(
        PMOS_COMMAND_BUFFER              cmdBuffer,
        PMHW_VDBOX_SURFACE_PARAMS        params)
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        MHW_FUNCTION_ENTER;

        MHW_MI_CHK_NULL(m_osInterface);
        MHW_MI_CHK_NULL(params);
        MHW_ASSERT(params->Mode != CODECHAL_UNSUPPORTED_MODE);

        MHW_MI_CHK_NULL(params->psSurface);

        typename THcpCmds::HCP_SURFACE_STATE_CMD cmd;

        cmd.DW1.SurfaceId = params->ucSurfaceStateId;
        cmd.DW1.SurfacePitchMinus1 = params->psSurface->dwPitch - 1;

        /* Handling of reconstructed surface is different for Y410 & AYUV formats */
        if ((params->ucSurfaceStateId != CODECHAL_HCP_SRC_SURFACE_ID) &&
            (params->psSurface->Format == Format_Y410))
            cmd.DW1.SurfacePitchMinus1 = params->psSurface->dwPitch / 2 - 1;

        if ((params->ucSurfaceStateId != CODECHAL_HCP_SRC_SURFACE_ID) &&
            (params->psSurface->Format == Format_AYUV))
            cmd.DW1.SurfacePitchMinus1 = params->psSurface->dwPitch / 4 - 1;

        cmd.DW2.YOffsetForUCbInPixel = params->psSurface->UPlaneOffset.iYOffset;

        MHW_MI_CHK_STATUS(m_osInterface->pfnAddCommand(cmdBuffer, &cmd, cmd.byteSize));

        return eStatus;
    }

    MOS_STATUS AddHcpIndObjBaseAddrCmd(
        PMOS_COMMAND_BUFFER                  cmdBuffer,
        PMHW_VDBOX_IND_OBJ_BASE_ADDR_PARAMS  params)
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        MHW_FUNCTION_ENTER;

        MHW_MI_CHK_NULL(m_osInterface);
        MHW_MI_CHK_NULL(params);

        MHW_RESOURCE_PARAMS resourceParams;
        typename THcpCmds::HCP_IND_OBJ_BASE_ADDR_STATE_CMD cmd;

        MOS_ZeroMemory(&resourceParams, sizeof(resourceParams));
        resourceParams.dwLsbNum = MHW_VDBOX_HCP_UPPER_BOUND_STATE_SHIFT;
        resourceParams.HwCommandType = MOS_MFX_INDIRECT_OBJ_BASE_ADDR;

        // mode specific settings
        if (CodecHalIsDecodeModeVLD(params->Mode))
        {
            MHW_MI_CHK_NULL(params->presDataBuffer);

            cmd.HcpIndirectBitstreamObjectMemoryAddressAttributes.DW0.Value |=
                m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_MFX_INDIRECT_BITSTREAM_OBJECT_DECODE].Value;

            resourceParams.presResource = params->presDataBuffer;
            resourceParams.dwOffset = params->dwDataOffset;
            resourceParams.pdwCmd = cmd.HcpIndirectBitstreamObjectBaseAddress.DW0_1.Value;
            resourceParams.dwLocationInCmd = 1;
            resourceParams.dwSize = params->dwDataSize;
            resourceParams.bIsWritable = false;

            // upper bound of the allocated resource will be set at 3 DW apart from address location
            resourceParams.dwUpperBoundLocationOffsetFromCmd = 3;

            MHW_MI_CHK_STATUS(pfnAddResourceToCmd(
                m_osInterface,
                cmdBuffer,
                &resourceParams));
        }

        MHW_MI_CHK_STATUS(m_osInterface->pfnAddCommand(cmdBuffer, &cmd, cmd.byteSize));

        return eStatus;
    }

    MOS_STATUS AddHcpQmStateCmd(
        PMOS_COMMAND_BUFFER              cmdBuffer,
        PMHW_VDBOX_QM_PARAMS             params)
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        MHW_FUNCTION_ENTER;

        MHW_MI_CHK_NULL(m_osInterface);
        MHW_MI_CHK_NULL(cmdBuffer);
        MHW_MI_CHK_NULL(params);

        if (params->Standard == CODECHAL_HEVC)
        {
            typename THcpCmds::HCP_QM_STATE_CMD cmd;
            uint8_t* qMatrix = nullptr;

            MHW_MI_CHK_NULL(params->pHevcIqMatrix);

            qMatrix = (uint8_t*)cmd.Quantizermatrix;

            for (uint8_t sizeId = 0; sizeId < 4; sizeId++)            // 4x4, 8x8, 16x16, 32x32
            {
                for (uint8_t predType = 0; predType < 2; predType++)  // Intra, Inter
                {
                    for (uint8_t color = 0; color < 3; color++)       // Y, Cb, Cr
                    {
                        if ((sizeId == 3) && (color != 0))
                            break;

                        cmd.DW1.Sizeid = sizeId;
                        cmd.DW1.PredictionType = predType;
                        cmd.DW1.ColorComponent = color;
                        switch (sizeId)
                        {
                        case cmd.SIZEID_4X4:
                        case cmd.SIZEID_8X8:
                        default:
                            cmd.DW1.DcCoefficient = 0;
                            break;
                        case cmd.SIZEID_16X16:
                            cmd.DW1.DcCoefficient = params->pHevcIqMatrix->ListDC16x16[3 * predType + color];
                            break;
                        case cmd.SIZEID_32X32:
                            cmd.DW1.DcCoefficient = params->pHevcIqMatrix->ListDC32x32[predType];
                            break;
                        }

                        if (sizeId == cmd.SIZEID_4X4)
                        {
                            for (uint8_t i = 0; i < 4; i++)
                            {
                                for (uint8_t ii = 0; ii < 4; ii++)
                                {
                                    qMatrix[4 * i + ii] = params->pHevcIqMatrix->List4x4[3 * predType + color][4 * i + ii];
                                }
                            }
                        }
                        else if (sizeId == cmd.SIZEID_8X8)
                        {
                            for (uint8_t i = 0; i < 8; i++)
                            {
                                for (uint8_t ii = 0; ii < 8; ii++)
                                {
                                    qMatrix[8 * i + ii] = params->pHevcIqMatrix->List8x8[3 * predType + color][8 * i + ii];
                                }
                            }
                        }
                        else if (sizeId == cmd.SIZEID_16X16)
                        {
                            for (uint8_t i = 0; i < 8; i++)
                            {
                                for (uint8_t ii = 0; ii < 8; ii++)
                                {
                                    qMatrix[8 * i + ii] = params->pHevcIqMatrix->List16x16[3 * predType + color][8 * i + ii];
                                }
                            }
                        }
                        else // 32x32
                        {
                            for (uint8_t i = 0; i < 8; i++)
                            {
                                for (uint8_t ii = 0; ii < 8; ii++)
                                {
                                    qMatrix[8 * i + ii] = params->pHevcIqMatrix->List32x32[predType][8 * i + ii];
                                }
                            }
                        }

                        MHW_MI_CHK_STATUS(m_osInterface->pfnAddCommand(cmdBuffer, &cmd, cmd.byteSize));
                    }
                }
            }
        }
        else
        {
            eStatus = MOS_STATUS_INVALID_PARAMETER;
        }

        return eStatus;
    }

    MOS_STATUS AddHcpDecodePicStateCmd(
        PMOS_COMMAND_BUFFER              cmdBuffer,
        PMHW_VDBOX_HEVC_PIC_STATE        params)
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        MHW_FUNCTION_ENTER;

        MHW_MI_CHK_NULL(m_osInterface);
        MHW_MI_CHK_NULL(params);
        MHW_MI_CHK_NULL(params->pHevcPicParams);

        typename THcpCmds::HCP_PIC_STATE_CMD cmd;

        auto hevcPicParams = params->pHevcPicParams;

        cmd.DW1.Framewidthinmincbminus1  = hevcPicParams->PicWidthInMinCbsY - 1;
        cmd.DW1.Frameheightinmincbminus1 = hevcPicParams->PicHeightInMinCbsY - 1;

        cmd.DW2.Mincusize = (hevcPicParams->log2_min_luma_coding_block_size_minus3) & 0x3;
        cmd.DW2.CtbsizeLcusize = (hevcPicParams->log2_diff_max_min_luma_coding_block_size
            + hevcPicParams->log2_min_luma_coding_block_size_minus3) & 0x3;
        cmd.DW2.Maxtusize = (hevcPicParams->log2_diff_max_min_transform_block_size
            + hevcPicParams->log2_min_transform_block_size_minus2) & 0x3;
        cmd.DW2.Mintusize = (hevcPicParams->log2_min_transform_block_size_minus2) & 0x3;
        cmd.DW2.Minpcmsize = (hevcPicParams->log2_min_pcm_luma_coding_block_size_minus3) & 0x3;
        cmd.DW2.Maxpcmsize = (hevcPicParams->log2_diff_max_min_pcm_luma_coding_block_size
            + hevcPicParams->log2_min_pcm_luma_coding_block_size_minus3) & 0x3;

        // As per HW requirement, CurPicIsI and ColPicIsI should be set to either both correct or both zero
        // Since driver doesn't know Collocated_Ref_Idx for SF, and cannot get accurate CurPicIsI for both LF/SF
        // Have to make ColPicIsI = CurPicIsI = 0 for both LF/SF
        cmd.DW3.Colpicisi = 0;
        cmd.DW3.Curpicisi = 0;

        cmd.DW4.SampleAdaptiveOffsetEnabledFlag         = hevcPicParams->sample_adaptive_offset_enabled_flag;
        cmd.DW4.PcmEnabledFlag                          = hevcPicParams->pcm_enabled_flag;
        cmd.DW4.CuQpDeltaEnabledFlag                    = hevcPicParams->cu_qp_delta_enabled_flag;
        cmd.DW4.DiffCuQpDeltaDepthOrNamedAsMaxDqpDepth  = hevcPicParams->diff_cu_qp_delta_depth;
        cmd.DW4.PcmLoopFilterDisableFlag                = hevcPicParams->pcm_loop_filter_disabled_flag;
        cmd.DW4.ConstrainedIntraPredFlag                = hevcPicParams->constrained_intra_pred_flag;
        cmd.DW4.Log2ParallelMergeLevelMinus2            = hevcPicParams->log2_parallel_merge_level_minus2;
        cmd.DW4.SignDataHidingFlag                      = hevcPicParams->sign_data_hiding_enabled_flag;
        cmd.DW4.LoopFilterAcrossTilesEnabledFlag        = hevcPicParams->loop_filter_across_tiles_enabled_flag;
        cmd.DW4.EntropyCodingSyncEnabledFlag            = hevcPicParams->entropy_coding_sync_enabled_flag;
        cmd.DW4.TilesEnabledFlag                        = hevcPicParams->tiles_enabled_flag;
        cmd.DW4.WeightedPredFlag                        = hevcPicParams->weighted_pred_flag;
        cmd.DW4.WeightedBipredFlag                      = hevcPicParams->weighted_bipred_flag;
        cmd.DW4.Fieldpic                                = (hevcPicParams->RefFieldPicFlag >> 15) & 0x01;
        cmd.DW4.Bottomfield                             = ((hevcPicParams->RefBottomFieldFlag >> 15) & 0x01) ? 0 : 1;
        cmd.DW4.TransformSkipEnabledFlag                = hevcPicParams->transform_skip_enabled_flag;
        cmd.DW4.AmpEnabledFlag                          = hevcPicParams->amp_enabled_flag;
        cmd.DW4.TransquantBypassEnableFlag              = hevcPicParams->transquant_bypass_enabled_flag;
        cmd.DW4.StrongIntraSmoothingEnableFlag          = hevcPicParams->strong_intra_smoothing_enabled_flag;

        cmd.DW5.PicCbQpOffset = hevcPicParams->pps_cb_qp_offset & 0x1f;
        cmd.DW5.PicCrQpOffset = hevcPicParams->pps_cr_qp_offset & 0x1f;
        cmd.DW5.MaxTransformHierarchyDepthIntraOrNamedAsTuMaxDepthIntra = hevcPicParams->max_transform_hierarchy_depth_intra & 0x7;
        cmd.DW5.MaxTransformHierarchyDepthInterOrNamedAsTuMaxDepthInter = hevcPicParams->max_transform_hierarchy_depth_inter & 0x7;
        cmd.DW5.PcmSampleBitDepthChromaMinus1 = hevcPicParams->pcm_sample_bit_depth_chroma_minus1;
        cmd.DW5.PcmSampleBitDepthLumaMinus1   = hevcPicParams->pcm_sample_bit_depth_luma_minus1;

        MHW_MI_CHK_STATUS(m_osInterface->pfnAddCommand(cmdBuffer, &cmd, cmd.byteSize));

        return eStatus;
    }

    MOS_STATUS AddHcpBsdObjectCmd(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMHW_VDBOX_HCP_BSD_PARAMS params)
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        MHW_FUNCTION_ENTER;

        MHW_MI_CHK_NULL(m_osInterface);
        typename THcpCmds::HCP_BSD_OBJECT_CMD   cmd;

        cmd.DW1.IndirectBsdDataLength = params->dwBsdDataLength;
        cmd.DW2.IndirectDataStartAddress = params->dwBsdDataStartOffset;

        MHW_MI_CHK_STATUS(m_osInterface->pfnAddCommand(cmdBuffer, &cmd, sizeof(cmd)));

        return eStatus;
    }

    MOS_STATUS AddHcpTileStateCmd(
        PMOS_COMMAND_BUFFER              cmdBuffer,
        PMHW_VDBOX_HEVC_TILE_STATE       params)
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        MHW_FUNCTION_ENTER;

        typename THcpCmds::HCP_TILE_STATE_CMD cmd;

        MHW_MI_CHK_NULL(m_osInterface);
        MHW_MI_CHK_NULL(params);
        MHW_MI_CHK_NULL(params->pTileColWidth);
        MHW_MI_CHK_NULL(params->pTileRowHeight);

        auto hevcPicParams = params->pHevcPicParams;
        uint32_t colCumulativeValue = 0;
        uint32_t rowCumulativeValue = 0;

        MHW_ASSERT(hevcPicParams->num_tile_rows_minus1 < HEVC_NUM_MAX_TILE_ROW);
        MHW_ASSERT(hevcPicParams->num_tile_columns_minus1 < HEVC_NUM_MAX_TILE_COLUMN);

        cmd.DW1.Numtilecolumnsminus1 = hevcPicParams->num_tile_columns_minus1;
        cmd.DW1.Numtilerowsminus1 = hevcPicParams->num_tile_rows_minus1;

        for (uint8_t i = 0; i < 5; i++)
        {
            cmd.CtbColumnPositionOfTileColumn[i].DW0.Ctbpos0I = colCumulativeValue;
            if ((4 * i) == hevcPicParams->num_tile_columns_minus1)
            {
                break;
            }

            colCumulativeValue += params->pTileColWidth[4 * i];
            cmd.CtbColumnPositionOfTileColumn[i].DW0.Ctbpos1I = colCumulativeValue;
            if ((4 * i + 1) == hevcPicParams->num_tile_columns_minus1)
            {
                break;
            }

            colCumulativeValue += params->pTileColWidth[4 * i + 1];
            cmd.CtbColumnPositionOfTileColumn[i].DW0.Ctbpos2I = colCumulativeValue;
            if ((4 * i + 2) == hevcPicParams->num_tile_columns_minus1)
            {
                break;
            }

            colCumulativeValue += params->pTileColWidth[4 * i + 2];
            cmd.CtbColumnPositionOfTileColumn[i].DW0.Ctbpos3I = colCumulativeValue;
            if ((4 * i + 3) == hevcPicParams->num_tile_columns_minus1)
            {
                break;
            }

            colCumulativeValue += params->pTileColWidth[4 * i + 3];
        }

        for (uint8_t i = 0; i < 5; i++)
        {
            cmd.CtbRowPositionOfTileRow[i].DW0.Ctbpos0I = rowCumulativeValue;
            if ((4 * i) == hevcPicParams->num_tile_rows_minus1)
            {
                break;
            }

            rowCumulativeValue += params->pTileRowHeight[4 * i];
            cmd.CtbRowPositionOfTileRow[i].DW0.Ctbpos1I = rowCumulativeValue;
            if ((4 * i + 1) == hevcPicParams->num_tile_rows_minus1)
            {
                break;
            }

            rowCumulativeValue += params->pTileRowHeight[4 * i + 1];
            cmd.CtbRowPositionOfTileRow[i].DW0.Ctbpos2I = rowCumulativeValue;
            if ((4 * i + 2) == hevcPicParams->num_tile_rows_minus1)
            {
                break;
            }

            rowCumulativeValue += params->pTileRowHeight[4 * i + 2];
            cmd.CtbRowPositionOfTileRow[i].DW0.Ctbpos3I = rowCumulativeValue;
            if ((4 * i + 3) == hevcPicParams->num_tile_rows_minus1)
            {
                break;
            }

            rowCumulativeValue += params->pTileRowHeight[4 * i + 3];
        }

        if (hevcPicParams->num_tile_rows_minus1 == 20)
        {
            cmd.CtbRowPositionOfTileRow[5].DW0.Ctbpos0I = rowCumulativeValue;
        }

        if (hevcPicParams->num_tile_rows_minus1 == 21)
        {
            cmd.CtbRowPositionOfTileRow[5].DW0.Ctbpos0I = rowCumulativeValue;
            rowCumulativeValue += params->pTileRowHeight[20];
            cmd.CtbRowPositionOfTileRow[5].DW0.Ctbpos1I = rowCumulativeValue;
        }

        MHW_MI_CHK_STATUS(m_osInterface->pfnAddCommand(cmdBuffer, &cmd, cmd.byteSize));

        return eStatus;
    }

    MOS_STATUS AddHcpRefIdxStateCmd(
        PMOS_COMMAND_BUFFER             cmdBuffer,
        PMHW_BATCH_BUFFER               batchBuffer,
        PMHW_VDBOX_HEVC_REF_IDX_PARAMS  params)
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        MHW_FUNCTION_ENTER;

        MHW_MI_CHK_NULL(params);

        typename THcpCmds::HCP_REF_IDX_STATE_CMD cmd;

        // Need to add an empty HCP_REF_IDX_STATE_CMD for dummy reference on I-Frame
        // ucNumRefForList could be 0 for encode
        if (!params->bDummyReference)
        {
            MHW_ASSERT(params->CurrPic.FrameIdx != 0x7F);

            cmd.DW1.Refpiclistnum = params->ucList;
            cmd.DW1.NumRefIdxLRefpiclistnumActiveMinus1 = params->ucNumRefForList - 1;

            for (uint8_t i = 0; i < params->ucNumRefForList; i++)
            {
                uint8_t refFrameIDx = params->RefPicList[params->ucList][i].FrameIdx;
                if (refFrameIDx < CODEC_MAX_NUM_REF_FRAME_HEVC)
                {
                    MHW_ASSERT(*(params->pRefIdxMapping + refFrameIDx) >= 0);

                    cmd.Entries[i].DW0.ListEntryLxReferencePictureFrameIdRefaddr07 = *(params->pRefIdxMapping + refFrameIDx);
                    int32_t pocDiff = params->poc_curr_pic - params->poc_list[refFrameIDx];
                    cmd.Entries[i].DW0.ReferencePictureTbValue = CodecHal_Clip3(-128, 127, pocDiff);
                    CODEC_REF_LIST** refList = (CODEC_REF_LIST**)params->hevcRefList;
                    cmd.Entries[i].DW0.Longtermreference = CodecHal_PictureIsLongTermRef(refList[params->CurrPic.FrameIdx]->RefList[refFrameIDx]);
                    cmd.Entries[i].DW0.FieldPicFlag = (params->RefFieldPicFlag >> refFrameIDx) & 0x01;
                    cmd.Entries[i].DW0.BottomFieldFlag = ((params->RefBottomFieldFlag >> refFrameIDx) & 0x01) ? 0 : 1;
                }
                else
                {
                    cmd.Entries[i].DW0.ListEntryLxReferencePictureFrameIdRefaddr07 = 0;
                    cmd.Entries[i].DW0.ReferencePictureTbValue = 0;
                    cmd.Entries[i].DW0.Longtermreference = false;
                    cmd.Entries[i].DW0.FieldPicFlag = 0;
                    cmd.Entries[i].DW0.BottomFieldFlag = 0;
                }
            }

            for (uint8_t i = (uint8_t)params->ucNumRefForList; i < 16; i++)
            {
                cmd.Entries[i].DW0.Value = 0x00;
            }
        }

        if (cmdBuffer == nullptr && batchBuffer == nullptr)
        {
            MHW_ASSERTMESSAGE("There was no valid buffer to add the HW command to.");
            return MOS_STATUS_INVALID_PARAMETER;
        }

        MHW_MI_CHK_STATUS(Mhw_AddCommandCmdOrBB(m_osInterface, cmdBuffer, batchBuffer, &cmd, cmd.byteSize));

        return eStatus;
    }

    MOS_STATUS AddHcpWeightOffsetStateCmd(
        PMOS_COMMAND_BUFFER                  cmdBuffer,
        PMHW_BATCH_BUFFER                    batchBuffer,
        PMHW_VDBOX_HEVC_WEIGHTOFFSET_PARAMS  params)
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        MHW_FUNCTION_ENTER;

        MHW_MI_CHK_NULL(params);

        typename THcpCmds::HCP_WEIGHTOFFSET_STATE_CMD       cmd;
        uint8_t i = 0;

        cmd.DW1.Refpiclistnum = i = params->ucList;

        // Luma
        for (uint8_t refIdx = 0; refIdx < CODEC_MAX_NUM_REF_FRAME_HEVC; refIdx++)
        {
            cmd.Lumaoffsets[refIdx].DW0.DeltaLumaWeightLxI = params->LumaWeights[i][refIdx];
            cmd.Lumaoffsets[refIdx].DW0.LumaOffsetLxI = params->LumaOffsets[i][refIdx];
        }

        // Chroma
        for (uint8_t refIdx = 0; refIdx < CODEC_MAX_NUM_REF_FRAME_HEVC; refIdx++)
        {
            cmd.Chromaoffsets[refIdx].DW0.DeltaChromaWeightLxI0 = params->ChromaWeights[i][refIdx][0];
            cmd.Chromaoffsets[refIdx].DW0.ChromaoffsetlxI0 = params->ChromaOffsets[i][refIdx][0];
            cmd.Chromaoffsets[refIdx].DW0.DeltaChromaWeightLxI1 = params->ChromaWeights[i][refIdx][1];
            cmd.Chromaoffsets[refIdx].DW0.ChromaoffsetlxI1 = params->ChromaOffsets[i][refIdx][1];
        }

        //cmd.DW2[15] and cmd.DW18[15] not be used

        if (cmdBuffer == nullptr && batchBuffer == nullptr)
        {
            MHW_ASSERTMESSAGE("There was no valid buffer to add the HW command to.");
            return MOS_STATUS_INVALID_PARAMETER;
        }

        MHW_MI_CHK_STATUS(Mhw_AddCommandCmdOrBB(m_osInterface, cmdBuffer, batchBuffer, &cmd, cmd.byteSize));

        return eStatus;
    }

    MOS_STATUS AddHcpDecodeSliceStateCmd(
        PMOS_COMMAND_BUFFER              cmdBuffer,
        PMHW_VDBOX_HEVC_SLICE_STATE      hevcSliceState)
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        MHW_FUNCTION_ENTER;

        MHW_MI_CHK_NULL(m_osInterface);
        MHW_MI_CHK_NULL(hevcSliceState);

        typename THcpCmds::HCP_SLICE_STATE_CMD cmd;

        auto hevcSliceParams = hevcSliceState->pHevcSliceParams;
        auto hevcPicParams = hevcSliceState->pHevcPicParams;

        uint32_t ctbSize = 1 << (hevcPicParams->log2_diff_max_min_luma_coding_block_size +
            hevcPicParams->log2_min_luma_coding_block_size_minus3 + 3);
        uint32_t widthInPix = (1 << (hevcPicParams->log2_min_luma_coding_block_size_minus3 + 3)) *
            (hevcPicParams->PicWidthInMinCbsY);
        uint32_t widthInCtb = MOS_ROUNDUP_DIVIDE(widthInPix, ctbSize);

        // It is a hardware requirement that the first HCP_SLICE_STATE of the workload starts at LCU X,Y = 0,0.
        // If first slice doesn't starts from (0,0), that means this is error bitstream.
        if (hevcSliceState->dwSliceIndex == 0)
        {
            cmd.DW1.SlicestartctbxOrSliceStartLcuXEncoder = 0;
            cmd.DW1.SlicestartctbyOrSliceStartLcuYEncoder = 0;
        }
        else
        {
            cmd.DW1.SlicestartctbxOrSliceStartLcuXEncoder = hevcSliceParams->slice_segment_address % widthInCtb;
            cmd.DW1.SlicestartctbyOrSliceStartLcuYEncoder = hevcSliceParams->slice_segment_address / widthInCtb;
        }

        if (hevcSliceState->bLastSlice)
        {
            cmd.DW2.NextslicestartctbxOrNextSliceStartLcuXEncoder = 0;
            cmd.DW2.NextslicestartctbyOrNextSliceStartLcuYEncoder = 0;
        }
        else
        {
            cmd.DW2.NextslicestartctbxOrNextSliceStartLcuXEncoder = (hevcSliceParams + 1)->slice_segment_address % widthInCtb;
            cmd.DW2.NextslicestartctbyOrNextSliceStartLcuYEncoder = (hevcSliceParams + 1)->slice_segment_address / widthInCtb;
        }

        cmd.DW3.SliceType = hevcSliceParams->LongSliceFlags.fields.slice_type;
        cmd.DW3.Lastsliceofpic = hevcSliceState->bLastSlice;
        cmd.DW3.DependentSliceFlag = hevcSliceParams->LongSliceFlags.fields.dependent_slice_segment_flag;
        cmd.DW3.SliceTemporalMvpEnableFlag = hevcSliceParams->LongSliceFlags.fields.slice_temporal_mvp_enabled_flag;
        cmd.DW3.SliceCbQpOffset = hevcSliceParams->slice_cb_qp_offset;
        cmd.DW3.SliceCrQpOffset = hevcSliceParams->slice_cr_qp_offset;

        cmd.DW4.SliceHeaderDisableDeblockingFilterFlag = hevcSliceParams->LongSliceFlags.fields.slice_deblocking_filter_disabled_flag;
        cmd.DW4.SliceTcOffsetDiv2OrFinalTcOffsetDiv2Encoder = hevcSliceParams->slice_tc_offset_div2;
        cmd.DW4.SliceBetaOffsetDiv2OrFinalBetaOffsetDiv2Encoder = hevcSliceParams->slice_beta_offset_div2;
        cmd.DW4.SliceLoopFilterAcrossSlicesEnabledFlag = hevcSliceParams->LongSliceFlags.fields.slice_loop_filter_across_slices_enabled_flag;
        cmd.DW4.SliceSaoChromaFlag = hevcSliceParams->LongSliceFlags.fields.slice_sao_chroma_flag;
        cmd.DW4.SliceSaoLumaFlag = hevcSliceParams->LongSliceFlags.fields.slice_sao_luma_flag;
        cmd.DW4.MvdL1ZeroFlag = hevcSliceParams->LongSliceFlags.fields.mvd_l1_zero_flag;

        uint8_t isLowDelay = 1;

        if (hevcSliceParams->LongSliceFlags.fields.slice_type == cmd.SLICE_TYPE_I_SLICE)
        {
            isLowDelay = 0;
        }
        else
        {
            for (uint8_t i = 0; i < hevcSliceParams->num_ref_idx_l0_active_minus1 + 1; i++)
            {
                uint8_t  refFrameID = hevcSliceParams->RefPicList[0][i].FrameIdx;
                if (hevcPicParams->PicOrderCntValList[refFrameID] > hevcPicParams->CurrPicOrderCntVal)
                {
                    isLowDelay = 0;
                    break;
                }
            }

            if (hevcSliceParams->LongSliceFlags.fields.slice_type == cmd.SLICE_TYPE_B_SLICE)
            {
                for (uint8_t i = 0; i < hevcSliceParams->num_ref_idx_l1_active_minus1 + 1; i++)
                {
                    uint8_t  refFrameID = hevcSliceParams->RefPicList[1][i].FrameIdx;
                    if (hevcPicParams->PicOrderCntValList[refFrameID] > hevcPicParams->CurrPicOrderCntVal)
                    {
                        isLowDelay = 0;
                        break;
                    }
                }
            }
        }

        cmd.DW4.Islowdelay = isLowDelay & 0x1;

        cmd.DW4.CollocatedFromL0Flag = hevcSliceParams->LongSliceFlags.fields.collocated_from_l0_flag;
        cmd.DW4.Chromalog2Weightdenom = hevcSliceParams->luma_log2_weight_denom + hevcSliceParams->delta_chroma_log2_weight_denom;
        cmd.DW4.LumaLog2WeightDenom = hevcSliceParams->luma_log2_weight_denom;
        cmd.DW4.CabacInitFlag = hevcSliceParams->LongSliceFlags.fields.cabac_init_flag;
        cmd.DW4.Maxmergeidx = 5 - hevcSliceParams->five_minus_max_num_merge_cand - 1;

        uint8_t collocatedRefIndex, collocatedFrameIdx, collocatedFromL0Flag;

        if (hevcSliceParams->LongSliceFlags.fields.slice_temporal_mvp_enabled_flag == 1)
        {
            //Get Collocated Picture POC
            collocatedRefIndex = hevcSliceParams->collocated_ref_idx;
            collocatedFrameIdx = 0;
            collocatedFromL0Flag = hevcSliceParams->LongSliceFlags.fields.collocated_from_l0_flag;
            if (hevcSliceParams->LongSliceFlags.fields.slice_type == cmd.SLICE_TYPE_P_SLICE)
            {
                collocatedFrameIdx = hevcSliceParams->RefPicList[0][collocatedRefIndex].FrameIdx;
            }
            else if (hevcSliceParams->LongSliceFlags.fields.slice_type == cmd.SLICE_TYPE_B_SLICE)
            {
                collocatedFrameIdx = hevcSliceParams->RefPicList[!collocatedFromL0Flag][collocatedRefIndex].FrameIdx;
            }

            if (hevcSliceParams->LongSliceFlags.fields.slice_type == cmd.SLICE_TYPE_I_SLICE)
            {
                cmd.DW4.Collocatedrefidx = 0;
            }
            else
            {
                MHW_CHK_COND(*(hevcSliceState->pRefIdxMapping + collocatedFrameIdx) < 0, "Invalid parameter");
                cmd.DW4.Collocatedrefidx = *(hevcSliceState->pRefIdxMapping + collocatedFrameIdx);
            }
        }
        else
        {
            cmd.DW4.Collocatedrefidx = 0;
        }

        static uint8_t  ucFirstInterSliceCollocatedFrameIdx;
        static uint8_t  ucFirstInterSliceCollocatedFromL0Flag;
        static bool   bFinishFirstInterSlice;

        //Need to save the first interSlice collocatedRefIdx value to use on subsequent intra slices
        //this is a HW requrement as collcoated ref fetching from memory may not be complete yet
        if (hevcSliceState->dwSliceIndex == 0)
        {
            ucFirstInterSliceCollocatedFrameIdx = 0;
            ucFirstInterSliceCollocatedFromL0Flag = 0;
            bFinishFirstInterSlice = false;
        }

        if ((!bFinishFirstInterSlice) &&
            (hevcSliceParams->LongSliceFlags.fields.slice_type != cmd.SLICE_TYPE_I_SLICE) &&
            (hevcSliceParams->LongSliceFlags.fields.slice_temporal_mvp_enabled_flag == 1))
        {
            ucFirstInterSliceCollocatedFrameIdx = cmd.DW4.Collocatedrefidx;
            ucFirstInterSliceCollocatedFromL0Flag = cmd.DW4.CollocatedFromL0Flag;
            bFinishFirstInterSlice = true;
        }

        if (bFinishFirstInterSlice &&
            ((hevcSliceParams->LongSliceFlags.fields.slice_type == cmd.SLICE_TYPE_I_SLICE) ||
                (hevcSliceParams->LongSliceFlags.fields.slice_temporal_mvp_enabled_flag == 0)))
        {
            cmd.DW4.Collocatedrefidx = ucFirstInterSliceCollocatedFrameIdx;
            cmd.DW4.CollocatedFromL0Flag = ucFirstInterSliceCollocatedFromL0Flag;
        }

        cmd.DW5.Sliceheaderlength = hevcSliceParams->ByteOffsetToSliceData;

        MHW_MI_CHK_STATUS(m_osInterface->pfnAddCommand(cmdBuffer, &cmd, cmd.byteSize));

        return eStatus;
    }

    MOS_STATUS AddHcpEncodeSliceStateCmd(
        PMOS_COMMAND_BUFFER              cmdBuffer,
        PMHW_VDBOX_HEVC_SLICE_STATE      hevcSliceState)
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        MHW_FUNCTION_ENTER;

        MHW_MI_CHK_NULL(cmdBuffer);
        MHW_MI_CHK_NULL(m_osInterface);
        MHW_MI_CHK_NULL(hevcSliceState);

        typename THcpCmds::HCP_SLICE_STATE_CMD      cmd;

        auto hevcSliceParams = hevcSliceState->pHevcSliceParams;
        auto hevcPicParams = hevcSliceState->pHevcPicParams;

        uint32_t ctbSize = 1 << (hevcPicParams->log2_diff_max_min_luma_coding_block_size +
            hevcPicParams->log2_min_luma_coding_block_size_minus3 + 3);
        uint32_t widthInPix = (1 << (hevcPicParams->log2_min_luma_coding_block_size_minus3 + 3)) *
            (hevcPicParams->PicWidthInMinCbsY);
        uint32_t widthInCtb = MOS_ROUNDUP_DIVIDE(widthInPix, ctbSize);

        // It is a hardware requirement that the first HCP_SLICE_STATE of the workload starts at LCU X,Y = 0,0.
        // If first slice doesn't starts from (0,0), that means this is error bitstream.
        if (hevcSliceState->dwSliceIndex == 0)
        {
            cmd.DW1.SlicestartctbxOrSliceStartLcuXEncoder = 0;
            cmd.DW1.SlicestartctbyOrSliceStartLcuYEncoder = 0;
        }
        else
        {
            cmd.DW1.SlicestartctbxOrSliceStartLcuXEncoder = hevcSliceParams->slice_segment_address % widthInCtb;
            cmd.DW1.SlicestartctbyOrSliceStartLcuYEncoder = hevcSliceParams->slice_segment_address / widthInCtb;
        }

        if (hevcSliceState->bLastSlice)
        {
            cmd.DW2.NextslicestartctbxOrNextSliceStartLcuXEncoder = 0;
            cmd.DW2.NextslicestartctbyOrNextSliceStartLcuYEncoder = 0;
        }
        else
        {
            cmd.DW2.NextslicestartctbxOrNextSliceStartLcuXEncoder = (hevcSliceParams + 1)->slice_segment_address % widthInCtb;
            cmd.DW2.NextslicestartctbyOrNextSliceStartLcuYEncoder = (hevcSliceParams + 1)->slice_segment_address / widthInCtb;
        }

        cmd.DW3.SliceType = hevcSliceParams->LongSliceFlags.fields.slice_type;
        cmd.DW3.Lastsliceofpic = hevcSliceState->bLastSlice;
        cmd.DW3.DependentSliceFlag = hevcSliceParams->LongSliceFlags.fields.dependent_slice_segment_flag;
        cmd.DW3.SliceTemporalMvpEnableFlag = hevcSliceParams->LongSliceFlags.fields.slice_temporal_mvp_enabled_flag;
        cmd.DW3.Sliceqp = hevcSliceParams->slice_qp_delta + hevcPicParams->init_qp_minus26 + 26;
        cmd.DW3.SliceCbQpOffset = hevcSliceParams->slice_cb_qp_offset;
        cmd.DW3.SliceCrQpOffset = hevcSliceParams->slice_cr_qp_offset;

        cmd.DW4.SliceHeaderDisableDeblockingFilterFlag = hevcSliceParams->LongSliceFlags.fields.slice_deblocking_filter_disabled_flag;
        cmd.DW4.SliceTcOffsetDiv2OrFinalTcOffsetDiv2Encoder = hevcSliceParams->slice_tc_offset_div2;
        cmd.DW4.SliceBetaOffsetDiv2OrFinalBetaOffsetDiv2Encoder = hevcSliceParams->slice_beta_offset_div2;
        cmd.DW4.SliceLoopFilterAcrossSlicesEnabledFlag = hevcSliceParams->LongSliceFlags.fields.slice_loop_filter_across_slices_enabled_flag;
        cmd.DW4.SliceSaoChromaFlag = hevcSliceParams->LongSliceFlags.fields.slice_sao_chroma_flag;
        cmd.DW4.SliceSaoLumaFlag = hevcSliceParams->LongSliceFlags.fields.slice_sao_luma_flag;
        cmd.DW4.MvdL1ZeroFlag = hevcSliceParams->LongSliceFlags.fields.mvd_l1_zero_flag;

        uint32_t  numNegativePic = 0;
        uint32_t  numPositivePic = 0;

        if (hevcSliceParams->LongSliceFlags.fields.slice_type != cmd.SLICE_TYPE_I_SLICE)
        {
            for (uint8_t i = 0; i < hevcSliceParams->num_ref_idx_l0_active_minus1 + 1; i++)
            {
                uint8_t  refFrameID = hevcSliceParams->RefPicList[0][i].FrameIdx;
                int32_t  pocDiff = hevcPicParams->CurrPicOrderCntVal - hevcPicParams->PicOrderCntValList[refFrameID];
                if (pocDiff > 0)
                {
                    numNegativePic++;
                }
            }
        }
        else
        {
            numNegativePic = 0;
        }

        if (hevcSliceParams->LongSliceFlags.fields.slice_type == cmd.SLICE_TYPE_B_SLICE)
        {
            for (uint8_t i = 0; i < hevcSliceParams->num_ref_idx_l1_active_minus1 + 1; i++)
            {
                uint8_t  refFrameID = hevcSliceParams->RefPicList[1][i].FrameIdx;
                int32_t  pocDiff = hevcPicParams->CurrPicOrderCntVal - hevcPicParams->PicOrderCntValList[refFrameID];
                if (pocDiff < 0)
                {
                    numPositivePic++;
                }
            }
        }
        else
        {
            numPositivePic = 0;
        }

        if ((numNegativePic == (hevcSliceParams->num_ref_idx_l0_active_minus1 + 1)) &&
            (numPositivePic == 0))
        {
            cmd.DW4.Islowdelay = 1;
        }
        else
        {
            cmd.DW4.Islowdelay = 0;
        }

        cmd.DW4.CollocatedFromL0Flag = hevcSliceParams->LongSliceFlags.fields.collocated_from_l0_flag;
        cmd.DW4.Chromalog2Weightdenom = (hevcPicParams->weighted_pred_flag || hevcPicParams->weighted_bipred_flag) ?
            (hevcSliceParams->luma_log2_weight_denom + hevcSliceParams->delta_chroma_log2_weight_denom) : 0;
        cmd.DW4.LumaLog2WeightDenom = (hevcPicParams->weighted_pred_flag || hevcPicParams->weighted_bipred_flag) ?
            hevcSliceParams->luma_log2_weight_denom : 0;
        cmd.DW4.CabacInitFlag = hevcSliceParams->LongSliceFlags.fields.cabac_init_flag;
        cmd.DW4.Maxmergeidx = 5 - hevcSliceParams->five_minus_max_num_merge_cand - 1;

        uint8_t   collocatedRefIndex, collocatedFrameIdx, collocatedFromL0Flag;

        if (hevcSliceParams->LongSliceFlags.fields.slice_temporal_mvp_enabled_flag == 1)
        {
            //Get Collocated Picture POC
            collocatedRefIndex = hevcSliceParams->collocated_ref_idx;
            collocatedFrameIdx = 0;
            collocatedFromL0Flag = hevcSliceParams->LongSliceFlags.fields.collocated_from_l0_flag;
            if (hevcSliceParams->LongSliceFlags.fields.slice_type == cmd.SLICE_TYPE_P_SLICE)
            {
                collocatedFrameIdx = hevcSliceParams->RefPicList[0][collocatedRefIndex].FrameIdx;
            }
            else if (hevcSliceParams->LongSliceFlags.fields.slice_type == cmd.SLICE_TYPE_B_SLICE)
            {
                collocatedFrameIdx = hevcSliceParams->RefPicList[!collocatedFromL0Flag][collocatedRefIndex].FrameIdx;
            }

            if (hevcSliceParams->LongSliceFlags.fields.slice_type == cmd.SLICE_TYPE_I_SLICE)
            {
                cmd.DW4.Collocatedrefidx = 0;
            }
            else
            {
                MHW_CHK_COND(*(hevcSliceState->pRefIdxMapping + collocatedFrameIdx) < 0, "Invalid parameter");
                cmd.DW4.Collocatedrefidx = *(hevcSliceState->pRefIdxMapping + collocatedFrameIdx);
            }
        }
        else
        {
            cmd.DW4.Collocatedrefidx = 0;
        }

        static uint8_t   ucFirstInterSliceCollocatedFrameIdx;
        static uint8_t   ucFirstInterSliceCollocatedFromL0Flag;
        static bool      bFinishFirstInterSlice;

        //Need to save the first interSlice collocatedRefIdx value to use on subsequent intra slices
        //this is a HW requrement as collcoated ref fetching from memory may not be complete yet
        if (hevcSliceState->dwSliceIndex == 0)
        {
            ucFirstInterSliceCollocatedFrameIdx = 0;
            ucFirstInterSliceCollocatedFromL0Flag = 0;
            bFinishFirstInterSlice = false;
        }

        if ((!bFinishFirstInterSlice) &&
            (hevcSliceParams->LongSliceFlags.fields.slice_type != cmd.SLICE_TYPE_I_SLICE) &&
            (hevcSliceParams->LongSliceFlags.fields.slice_temporal_mvp_enabled_flag == 1))
        {
            ucFirstInterSliceCollocatedFrameIdx = cmd.DW4.Collocatedrefidx;
            ucFirstInterSliceCollocatedFromL0Flag = cmd.DW4.CollocatedFromL0Flag;
            bFinishFirstInterSlice = true;
        }

        if (bFinishFirstInterSlice &&
            ((hevcSliceParams->LongSliceFlags.fields.slice_type == cmd.SLICE_TYPE_I_SLICE) ||
                (hevcSliceParams->LongSliceFlags.fields.slice_temporal_mvp_enabled_flag == 0)))
        {
            cmd.DW4.Collocatedrefidx = ucFirstInterSliceCollocatedFrameIdx;
            cmd.DW4.CollocatedFromL0Flag = ucFirstInterSliceCollocatedFromL0Flag;
        }

        cmd.DW5.Sliceheaderlength = hevcSliceParams->ByteOffsetToSliceData;

        MHW_MI_CHK_STATUS(m_osInterface->pfnAddCommand(cmdBuffer, &cmd, cmd.byteSize));

        return eStatus;
    }

    MOS_STATUS AddHcpDecodeProtectStateCmd(
        PMOS_COMMAND_BUFFER              cmdBuffer,
        PMHW_VDBOX_HEVC_SLICE_STATE      hevcSliceState)
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        MHW_FUNCTION_ENTER;

        MHW_MI_CHK_NULL(hevcSliceState);

        MHW_CP_SLICE_INFO_PARAMS sliceInfoParam;
        sliceInfoParam.presDataBuffer = hevcSliceState->presDataBuffer;
        sliceInfoParam.dwSliceIndex = hevcSliceState->dwSliceIndex;
        sliceInfoParam.dwTotalBytesConsumed = 0;
        sliceInfoParam.bLastPass = hevcSliceState->bLastSlice;
        sliceInfoParam.dwDataStartOffset[0] = hevcSliceState->pHevcSliceParams->slice_data_offset + hevcSliceState->dwOffset;
        sliceInfoParam.dwDataStartOffset[1] = hevcSliceState->pHevcSliceParams->slice_data_offset + hevcSliceState->dwOffset;
        sliceInfoParam.dwDataLength[0] = hevcSliceState->pHevcSliceParams->slice_data_size;
        sliceInfoParam.dwDataLength[1] = hevcSliceState->pHevcSliceParams->slice_data_size;

        MHW_MI_CHK_STATUS(m_cpInterface->SetHcpProtectionState(
            true,
            cmdBuffer,
            nullptr,
            &sliceInfoParam));

        return eStatus;
    }
};

#endif
