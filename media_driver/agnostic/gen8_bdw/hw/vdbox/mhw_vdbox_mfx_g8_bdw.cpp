/*
* Copyright (c) 2017, Intel Corporation
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
//! \file     mhw_vdbox_mfx_g8_bdw.cpp
//! \brief    Defines functions for constructing Vdbox MFX commands on BDW
//!

#include "mhw_vdbox_mfx_g8_bdw.h"
#include "mos_os_cp_interface_specific.h"

MOS_STATUS MhwVdboxMfxInterfaceG8Bdw::AddMfxPipeBufAddrCmd(
    PMOS_COMMAND_BUFFER cmdBuffer,
    PMHW_VDBOX_PIPE_BUF_ADDR_PARAMS params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(m_osInterface);
    MHW_MI_CHK_NULL(cmdBuffer);
    MHW_MI_CHK_NULL(params);

    MHW_RESOURCE_PARAMS resourceParams;
    MOS_ZeroMemory(&resourceParams, sizeof(resourceParams));
    resourceParams.dwLsbNum = MHW_VDBOX_MFX_GENERAL_STATE_SHIFT;
    resourceParams.HwCommandType = MOS_MFX_PIPE_BUF_ADDR;

    mhw_vdbox_mfx_g8_bdw::MFX_PIPE_BUF_ADDR_STATE_CMD cmd;

    // Encoding uses both surfaces regardless of deblocking status
    if (params->psPreDeblockSurface != nullptr)
    {
        cmd.DW3.MemoryObjectControlState =
            m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_PRE_DEBLOCKING_CODEC].Value;

        resourceParams.presResource = &(params->psPreDeblockSurface->OsResource);
        resourceParams.dwOffset = params->psPreDeblockSurface->dwOffset;
        resourceParams.pdwCmd = &(cmd.DW1.Value);
        resourceParams.dwLocationInCmd = 1;
        resourceParams.bIsWritable = true;

        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
    }

    if (params->psPostDeblockSurface != nullptr)
    {
        cmd.DW6.MemoryObjectControlState =
            m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_POST_DEBLOCKING_CODEC].Value;

        resourceParams.presResource = &(params->psPostDeblockSurface->OsResource);
        resourceParams.dwOffset = params->psPostDeblockSurface->dwOffset;
        resourceParams.pdwCmd = &(cmd.DW4.Value);
        resourceParams.dwLocationInCmd = 4;
        resourceParams.bIsWritable = true;

        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
    }

    if (params->psRawSurface != nullptr)
    {
        if (!m_decodeInUse)
        {
            cmd.DW9.MemoryObjectControlState =
                m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_ORIGINAL_UNCOMPRESSED_PICTURE_ENCODE].Value;
        }
        else
        {
            cmd.DW9.MemoryObjectControlState =
                m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_ORIGINAL_UNCOMPRESSED_PICTURE_DECODE].Value;
        }

        resourceParams.presResource = &params->psRawSurface->OsResource;
        resourceParams.dwOffset = params->psRawSurface->dwOffset;
        resourceParams.pdwCmd = &(cmd.DW7.Value);
        resourceParams.dwLocationInCmd = 7;
        resourceParams.bIsWritable = false;

        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
    }

    if (params->presStreamOutBuffer != nullptr)
    {
        cmd.DW12.MemoryObjectControlState =
            m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_STREAMOUT_DATA_CODEC].Value;

        resourceParams.presResource = params->presStreamOutBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = &(cmd.DW10.Value);
        resourceParams.dwLocationInCmd = 10;
        resourceParams.bIsWritable = true;

        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));

        if (!m_decodeInUse)
        {
            cmd.DW54.MemoryObjectControlState =
                m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_STREAMOUT_DATA_CODEC].Value;

            resourceParams.presResource = params->presStreamOutBuffer;
            resourceParams.dwOffset = 0;
            resourceParams.pdwCmd = &(cmd.DW52.Value);
            resourceParams.dwLocationInCmd = 52;
            resourceParams.bIsWritable = true;

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                m_osInterface,
                cmdBuffer,
                &resourceParams));
        }
    }

    if (params->presMfdIntraRowStoreScratchBuffer != nullptr)
    {
        cmd.DW15.MemoryObjectControlState =
            m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_INTRA_ROWSTORE_SCRATCH_BUFFER_CODEC].Value;

        resourceParams.presResource = params->presMfdIntraRowStoreScratchBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = &(cmd.DW13.Value);
        resourceParams.dwLocationInCmd = 13;
        resourceParams.bIsWritable = true;

        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
    }

    if (params->presMfdDeblockingFilterRowStoreScratchBuffer != nullptr)
    {
        cmd.DW18.MemoryObjectControlState =
            m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_DEBLOCKINGFILTER_ROWSTORE_SCRATCH_BUFFER_CODEC].Value;

        resourceParams.presResource = params->presMfdDeblockingFilterRowStoreScratchBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = &(cmd.DW16.Value);
        resourceParams.dwLocationInCmd = 16;
        resourceParams.bIsWritable = true;

        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
    }

    for (auto i = 0; i < CODEC_MAX_NUM_REF_FRAME; i++)
    {
        if (params->presReferences[i] != nullptr)
        {
            MOS_SURFACE details;
            MOS_ZeroMemory(&details, sizeof(details));
            details.Format = Format_Invalid;
            MHW_MI_CHK_STATUS(m_osInterface->pfnGetResourceInfo(m_osInterface, params->presReferences[i], &details));

            resourceParams.presResource = params->presReferences[i];
            resourceParams.dwOffset = details.RenderOffset.YUV.Y.BaseOffset;
            resourceParams.pdwCmd = &(cmd.Refpicbaseaddr[i].DW0_1.Value[0]);
            resourceParams.dwLocationInCmd = (i * 2) + 19; // * 2 to account for QW rather than DW
            resourceParams.bIsWritable = false;

            resourceParams.dwSharedMocsOffset = 51 - resourceParams.dwLocationInCmd; // Common Prodected Data bit is in DW51

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                m_osInterface,
                cmdBuffer,
                &resourceParams));
        }
    }

    // there is only one control DW51 for all references
    cmd.DW51.MemoryObjectControlState =
        m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_REFERENCE_PICTURE_CODEC].Value;

    // Reset dwSharedMocsOffset
    resourceParams.dwSharedMocsOffset = 0;

    if (params->presMacroblockIldbStreamOutBuffer1 != nullptr)
    {
        resourceParams.presResource = params->presMacroblockIldbStreamOutBuffer1;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = &(cmd.DW55.Value);
        resourceParams.dwLocationInCmd = 55;
        resourceParams.bIsWritable = true;

        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
    }

    if (params->presMacroblockIldbStreamOutBuffer2 != nullptr)
    {
        resourceParams.presResource = params->presMacroblockIldbStreamOutBuffer2;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = &(cmd.DW58.Value);
        resourceParams.dwLocationInCmd = 58;
        resourceParams.bIsWritable = true;

        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
    }

    MHW_MI_CHK_STATUS(m_osInterface->pfnAddCommand(cmdBuffer, &cmd, sizeof(cmd)));

    return eStatus;
}

MOS_STATUS MhwVdboxMfxInterfaceG8Bdw::AddMfxBspBufBaseAddrCmd(
    PMOS_COMMAND_BUFFER cmdBuffer,
    PMHW_VDBOX_BSP_BUF_BASE_ADDR_PARAMS params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(m_osInterface);
    MHW_MI_CHK_NULL(cmdBuffer);
    MHW_MI_CHK_NULL(params);

    MHW_RESOURCE_PARAMS resourceParams;
    MOS_ZeroMemory(&resourceParams, sizeof(resourceParams));
    resourceParams.dwLsbNum = MHW_VDBOX_MFX_GENERAL_STATE_SHIFT;
    resourceParams.HwCommandType = MOS_MFX_BSP_BUF_BASE_ADDR;

    mhw_vdbox_mfx_g8_bdw::MFX_BSP_BUF_BASE_ADDR_STATE_CMD cmd;

    if (params->presBsdMpcRowStoreScratchBuffer)
    {
        cmd.DW3.MemoryObjectControlState =
            m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_BSDMPC_ROWSTORE_SCRATCH_BUFFER_CODEC].Value;

        resourceParams.presResource = params->presBsdMpcRowStoreScratchBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = &(cmd.DW1.Value);
        resourceParams.dwLocationInCmd = 1;
        resourceParams.bIsWritable = true;

        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
    }

    if (params->presMprRowStoreScratchBuffer)
    {
        cmd.DW6.MemoryObjectControlState =
            m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_MPR_ROWSTORE_SCRATCH_BUFFER_CODEC].Value;

        resourceParams.presResource = params->presMprRowStoreScratchBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = &(cmd.DW4.Value);
        resourceParams.dwLocationInCmd = 4;
        resourceParams.bIsWritable = true;

        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
    }

    if (params->presBitplaneBuffer)
    {
        cmd.DW9.MemoryObjectControlState =
            m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_BITPLANE_READ_CODEC].Value;

        resourceParams.presResource = params->presBitplaneBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = &(cmd.DW7.Value);
        resourceParams.dwLocationInCmd = 7;
        resourceParams.bIsWritable = false;

        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
    }

    MHW_MI_CHK_STATUS(m_osInterface->pfnAddCommand(cmdBuffer, &cmd, sizeof(cmd)));

    return eStatus;
}

MOS_STATUS MhwVdboxMfxInterfaceG8Bdw::AddMfxJpegPicCmd(
    PMOS_COMMAND_BUFFER cmdBuffer,
    PMHW_VDBOX_JPEG_PIC_STATE params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(m_osInterface);
    MHW_MI_CHK_NULL(cmdBuffer);
    MHW_MI_CHK_NULL(params);
    MHW_MI_CHK_NULL(params->pJpegPicParams);

    mhw_vdbox_mfx_g8_bdw::MFX_JPEG_PIC_STATE_CMD cmd;
    auto picParams = params->pJpegPicParams;

    if (picParams->m_chromaType == jpegRGB || picParams->m_chromaType == jpegBGR)
    {
        cmd.DW1.InputFormatYuv = jpegYUV444;
    }
    else
    {
        cmd.DW1.InputFormatYuv = picParams->m_chromaType;
    }

    cmd.DW1.Rotation = picParams->m_rotation;
    cmd.DW1.OutputFormatYuv = GetJpegDecodeFormat((MOS_FORMAT)params->dwOutputFormat);

    if (params->dwOutputFormat == Format_NV12)
    {
        if (picParams->m_chromaType == jpegYUV422H2Y ||
            picParams->m_chromaType == jpegYUV422H4Y)
        {
            cmd.DW1.VerticalDownSamplingEnable = 1;
        }
        else if (picParams->m_chromaType == jpegYUV422V2Y ||
                    picParams->m_chromaType == jpegYUV422V4Y)
        {
            cmd.DW1.HorizontalDownSamplingEnable = 1;
        }
    }
    else if (params->dwOutputFormat == Format_UYVY ||
                params->dwOutputFormat == Format_YUY2)
    {
        if (picParams->m_chromaType == jpegYUV420)
        {
            cmd.DW1.VerticalUpSamplingEnable = 1;
        }
    }

    cmd.DW2.FrameWidthInBlocksMinus1 = params->dwWidthInBlocks;
    cmd.DW2.FrameHeightInBlocksMinus1 = params->dwHeightInBlocks;

    MHW_MI_CHK_STATUS(m_osInterface->pfnAddCommand(cmdBuffer, &cmd, sizeof(cmd)));

    return eStatus;
}

MOS_STATUS MhwVdboxMfxInterfaceG8Bdw::AddMfxDecodeVp8PicCmd(
    PMOS_COMMAND_BUFFER cmdBuffer,
    PMHW_VDBOX_VP8_PIC_STATE params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(m_osInterface);
    MHW_MI_CHK_NULL(cmdBuffer);
    MHW_MI_CHK_NULL(params);

    mhw_vdbox_mfx_g8_bdw::MFX_VP8_PIC_STATE_CMD cmd;
    auto vp8PicParams = params->pVp8PicParams;
    auto vp8IqMatrixParams = params->pVp8IqMatrixParams;

    cmd.DW1.FrameWidthMinus1 = vp8PicParams->wFrameWidthInMbsMinus1;
    cmd.DW1.FrameHeightMinus1 = vp8PicParams->wFrameHeightInMbsMinus1;
    cmd.DW2.McFilterSelect = (vp8PicParams->version != 0);
    cmd.DW2.ChromaFullPixelMcFilterMode = (vp8PicParams->version == 3);
    cmd.DW2.Dblkfiltertype = vp8PicParams->filter_type;
    cmd.DW2.Skeyframeflag = vp8PicParams->key_frame;
    cmd.DW2.SegmentationIdStreamoutEnable =
        (vp8PicParams->segmentation_enabled) && (vp8PicParams->update_mb_segmentation_map);
    cmd.DW2.SegmentationIdStreaminEnable =
        (vp8PicParams->segmentation_enabled) && !(vp8PicParams->update_mb_segmentation_map);
    cmd.DW2.SegmentEnableFlag = vp8PicParams->segmentation_enabled;
    cmd.DW2.UpdateMbsegmentMapFlag =
        (vp8PicParams->segmentation_enabled) ? vp8PicParams->update_mb_segmentation_map : 0;
    cmd.DW2.MbNocoeffSkipflag = vp8PicParams->mb_no_coeff_skip;
    cmd.DW2.ModeReferenceLoopFilterDeltaEnabled = vp8PicParams->loop_filter_adj_enable;
    cmd.DW2.GoldenRefPictureMvSignbiasFlag = vp8PicParams->sign_bias_golden;
    cmd.DW2.AlternateRefPicMvSignbiasFlag = vp8PicParams->sign_bias_alternate;
    cmd.DW2.DeblockSharpnessLevel = vp8PicParams->ucSharpnessLevel;
    cmd.DW3.DblkfilterlevelForSegment3 = vp8PicParams->ucLoopFilterLevel[3];
    cmd.DW3.DblkfilterlevelForSegment2 = vp8PicParams->ucLoopFilterLevel[2];
    cmd.DW3.DblkfilterlevelForSegment1 = vp8PicParams->ucLoopFilterLevel[1];
    cmd.DW3.DblkfilterlevelForSegment0 = vp8PicParams->ucLoopFilterLevel[0];

    uint32_t i = 0;
    uint32_t j = 0;
    cmd.DW4.QuantizerValue0Blocktype0Y1Dc = vp8IqMatrixParams->quantization_values[i][j];
    cmd.DW4.QuantizerValue0Blocktype1Y1Ac = vp8IqMatrixParams->quantization_values[i][j + 1];

    i = 0;
    j = 2;
    cmd.DW5.QuantizerValue0Blocktype2Uvdc = vp8IqMatrixParams->quantization_values[i][j];
    cmd.DW5.QuantizerValue0Blocktype3Uvac = vp8IqMatrixParams->quantization_values[i][j + 1];

    i = 0;
    j = 4;
    cmd.DW6.QuantizerValue0Blocktype4Y2Dc = vp8IqMatrixParams->quantization_values[i][j];
    cmd.DW6.QuantizerValue0Blocktype5Y2Ac = vp8IqMatrixParams->quantization_values[i][j + 1];

    i = 1;
    j = 0;
    cmd.DW7.QuantizerValue1Blocktype0Y1Dc = vp8IqMatrixParams->quantization_values[i][j];
    cmd.DW7.QuantizerValue1Blocktype1Y1Ac = vp8IqMatrixParams->quantization_values[i][j + 1];

    i = 1;
    j = 2;
    cmd.DW8.QuantizerValue1Blocktype2Uvdc = vp8IqMatrixParams->quantization_values[i][j];
    cmd.DW8.QuantizerValue1Blocktype3Uvac = vp8IqMatrixParams->quantization_values[i][j + 1];

    i = 1;
    j = 4;
    cmd.DW9.QuantizerValue1Blocktype4Y2Dc = vp8IqMatrixParams->quantization_values[i][j];
    cmd.DW9.QuantizerValue1Blocktype5Y2Ac = vp8IqMatrixParams->quantization_values[i][j + 1];

    i = 2;
    j = 0;
    cmd.DW10.QuantizerValue2Blocktype0Y1Dc = vp8IqMatrixParams->quantization_values[i][j];
    cmd.DW10.QuantizerValue2Blocktype1Y1Ac = vp8IqMatrixParams->quantization_values[i][j + 1];

    i = 2;
    j = 2;
    cmd.DW11.QuantizerValue2Blocktype2Uvdc = vp8IqMatrixParams->quantization_values[i][j];
    cmd.DW11.QuantizerValue2Blocktype3Uvac = vp8IqMatrixParams->quantization_values[i][j + 1];

    i = 2;
    j = 4;
    cmd.DW12.QuantizerValue2Blocktype4Y2Dc = vp8IqMatrixParams->quantization_values[i][j];
    cmd.DW12.QuantizerValue2Blocktype5Y2Ac = vp8IqMatrixParams->quantization_values[i][j + 1];

    i = 3;
    j = 0;
    cmd.DW13.QuantizerValue3Blocktype0Y1Dc = vp8IqMatrixParams->quantization_values[i][j];
    cmd.DW13.QuantizerValue3Blocktype1Y1Ac = vp8IqMatrixParams->quantization_values[i][j + 1];

    i = 3;
    j = 2;
    cmd.DW14.QuantizerValue3Blocktype2Uvdc = vp8IqMatrixParams->quantization_values[i][j];
    cmd.DW14.QuantizerValue3Blocktype3Uvac = vp8IqMatrixParams->quantization_values[i][j + 1];

    i = 3;
    j = 4;
    cmd.DW15.QuantizerValue3Blocktype4Y2Dc = vp8IqMatrixParams->quantization_values[i][j];
    cmd.DW15.QuantizerValue3Blocktype5Y2Ac = vp8IqMatrixParams->quantization_values[i][j + 1];

    cmd.DW19.Mbsegmentidtreeprobs2 = vp8PicParams->cMbSegmentTreeProbs[2];
    cmd.DW19.Mbsegmentidtreeprobs1 = vp8PicParams->cMbSegmentTreeProbs[1];
    cmd.DW19.Mbsegmentidtreeprobs0 = vp8PicParams->cMbSegmentTreeProbs[0];
    cmd.DW20.Mbnocoeffskipfalseprob = vp8PicParams->ucProbSkipFalse;
    cmd.DW20.Intrambprob = vp8PicParams->ucProbIntra;
    cmd.DW20.Interpredfromlastrefprob = vp8PicParams->ucProbLast;
    cmd.DW20.Interpredfromgrefrefprob = vp8PicParams->ucProbGolden;
    cmd.DW21.Ymodeprob3 = vp8PicParams->ucYModeProbs[3];
    cmd.DW21.Ymodeprob2 = vp8PicParams->ucYModeProbs[2];
    cmd.DW21.Ymodeprob1 = vp8PicParams->ucYModeProbs[1];
    cmd.DW21.Ymodeprob0 = vp8PicParams->ucYModeProbs[0];
    cmd.DW22.Uvmodeprob2 = vp8PicParams->ucUvModeProbs[2];
    cmd.DW22.Uvmodeprob1 = vp8PicParams->ucUvModeProbs[1];
    cmd.DW22.Uvmodeprob0 = vp8PicParams->ucUvModeProbs[0];

    i = 0;
    j = 0;
    cmd.DW23.Mvupdateprobs00 = vp8PicParams->ucMvUpdateProb[i][j];
    cmd.DW23.Mvupdateprobs01 = vp8PicParams->ucMvUpdateProb[i][j + 1];
    cmd.DW23.Mvupdateprobs02 = vp8PicParams->ucMvUpdateProb[i][j + 2];
    cmd.DW23.Mvupdateprobs03 = vp8PicParams->ucMvUpdateProb[i][j + 3];

    i = 0;
    j = 4;
    cmd.DW24.Mvupdateprobs04 = vp8PicParams->ucMvUpdateProb[i][j];
    cmd.DW24.Mvupdateprobs05 = vp8PicParams->ucMvUpdateProb[i][j + 1];
    cmd.DW24.Mvupdateprobs06 = vp8PicParams->ucMvUpdateProb[i][j + 2];
    cmd.DW24.Mvupdateprobs07 = vp8PicParams->ucMvUpdateProb[i][j + 3];

    i = 0;
    j = 8;
    cmd.DW25.Mvupdateprobs08 = vp8PicParams->ucMvUpdateProb[i][j];
    cmd.DW25.Mvupdateprobs09 = vp8PicParams->ucMvUpdateProb[i][j + 1];
    cmd.DW25.Mvupdateprobs010 = vp8PicParams->ucMvUpdateProb[i][j + 2];
    cmd.DW25.Mvupdateprobs011 = vp8PicParams->ucMvUpdateProb[i][j + 3];

    i = 0;
    j = 12;
    cmd.DW26.Mvupdateprobs012 = vp8PicParams->ucMvUpdateProb[i][j];
    cmd.DW26.Mvupdateprobs013 = vp8PicParams->ucMvUpdateProb[i][j + 1];
    cmd.DW26.Mvupdateprobs014 = vp8PicParams->ucMvUpdateProb[i][j + 2];
    cmd.DW26.Mvupdateprobs015 = vp8PicParams->ucMvUpdateProb[i][j + 3];

    i = 0;
    j = 16;
    cmd.DW27.Mvupdateprobs016 = vp8PicParams->ucMvUpdateProb[i][j];
    cmd.DW27.Mvupdateprobs017 = vp8PicParams->ucMvUpdateProb[i][j + 1];
    cmd.DW27.Mvupdateprobs018 = vp8PicParams->ucMvUpdateProb[i][j + 2];

    i = 1;
    j = 0;
    cmd.DW28.Mvupdateprobs10 = vp8PicParams->ucMvUpdateProb[i][j];
    cmd.DW28.Mvupdateprobs11 = vp8PicParams->ucMvUpdateProb[i][j + 1];
    cmd.DW28.Mvupdateprobs12 = vp8PicParams->ucMvUpdateProb[i][j + 2];
    cmd.DW28.Mvupdateprobs13 = vp8PicParams->ucMvUpdateProb[i][j + 3];

    i = 1;
    j = 4;
    cmd.DW29.Mvupdateprobs14 = vp8PicParams->ucMvUpdateProb[i][j];
    cmd.DW29.Mvupdateprobs15 = vp8PicParams->ucMvUpdateProb[i][j + 1];
    cmd.DW29.Mvupdateprobs16 = vp8PicParams->ucMvUpdateProb[i][j + 2];
    cmd.DW29.Mvupdateprobs17 = vp8PicParams->ucMvUpdateProb[i][j + 3];

    i = 1;
    j = 8;
    cmd.DW30.Mvupdateprobs18 = vp8PicParams->ucMvUpdateProb[i][j];
    cmd.DW30.Mvupdateprobs19 = vp8PicParams->ucMvUpdateProb[i][j + 1];
    cmd.DW30.Mvupdateprobs110 = vp8PicParams->ucMvUpdateProb[i][j + 2];
    cmd.DW30.Mvupdateprobs111 = vp8PicParams->ucMvUpdateProb[i][j + 3];

    i = 1;
    j = 12;
    cmd.DW31.Mvupdateprobs112 = vp8PicParams->ucMvUpdateProb[i][j];
    cmd.DW31.Mvupdateprobs113 = vp8PicParams->ucMvUpdateProb[i][j + 1];
    cmd.DW31.Mvupdateprobs114 = vp8PicParams->ucMvUpdateProb[i][j + 2];
    cmd.DW31.Mvupdateprobs115 = vp8PicParams->ucMvUpdateProb[i][j + 3];

    i = 1;
    j = 16;
    cmd.DW32.Mvupdateprobs116 = vp8PicParams->ucMvUpdateProb[i][j];
    cmd.DW32.Mvupdateprobs117 = vp8PicParams->ucMvUpdateProb[i][j + 1];
    cmd.DW32.Mvupdateprobs118 = vp8PicParams->ucMvUpdateProb[i][j + 2];

    cmd.DW33.Reflfdelta0ForIntraFrame = vp8PicParams->cRefLfDelta[0];
    cmd.DW33.Reflfdelta1ForLastFrame = vp8PicParams->cRefLfDelta[1];
    cmd.DW33.Reflfdelta2ForGoldenFrame = vp8PicParams->cRefLfDelta[2];
    cmd.DW33.Reflfdelta3ForAltrefFrame = vp8PicParams->cRefLfDelta[3];
    cmd.DW34.Modelfdelta0ForBPredMode = vp8PicParams->cModeLfDelta[0];
    cmd.DW34.Modelfdelta1ForZeromvMode = vp8PicParams->cModeLfDelta[1];
    cmd.DW34.Modelfdelta2ForNearestNearAndNewMode = vp8PicParams->cModeLfDelta[2];
    cmd.DW34.Modelfdelta3ForSplitmvMode = vp8PicParams->cModeLfDelta[3];

    MHW_RESOURCE_PARAMS resourceParams;
    MOS_ZeroMemory(&resourceParams, sizeof(resourceParams));
    resourceParams.dwLsbNum = MHW_VDBOX_MFX_GENERAL_STATE_SHIFT;
    resourceParams.HwCommandType = MOS_MFX_VP8_PIC;

    resourceParams.presResource = params->presCoefProbBuffer;
    resourceParams.dwOffset = params->dwCoefProbTableOffset;
    resourceParams.pdwCmd = &(cmd.DW16.Value);
    resourceParams.dwLocationInCmd = 16;
    resourceParams.bIsWritable = false;

    MHW_MI_CHK_STATUS(AddResourceToCmd(
        m_osInterface,
        cmdBuffer,
        &resourceParams));

    if (vp8PicParams->segmentation_enabled)
    {
        resourceParams.presResource = params->presSegmentationIdStreamBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = &(cmd.DW35.Value);
        resourceParams.dwLocationInCmd = 35;
        resourceParams.bIsWritable = true;

        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
    }

    MHW_MI_CHK_STATUS(m_osInterface->pfnAddCommand(cmdBuffer, &cmd, sizeof(cmd)));

    return eStatus;
}
