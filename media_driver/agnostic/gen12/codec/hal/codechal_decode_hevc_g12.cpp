/*
* Copyright (c) 2017-2019, Intel Corporation
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
//! \file     codechal_decode_hevc_g12.cpp
//! \brief    Implements the decode interface extension for HEVC.
//! \details  Implements all functions required by CodecHal for HEVC decoding.
//!

#include "codechal_decoder.h"
#include "codechal_secure_decode_interface.h"
#include "codechal_decode_hevc_g12.h"
#include "codechal_decode_hevc_long_g12.h"
#include "codechal_decode_sfc_hevc_g12.h"
#include "mhw_vdbox_hcp_g12_X.h"
#include "mhw_vdbox_mfx_g12_X.h"
#include "mhw_vdbox_g12_X.h"
#include "mhw_mi_g12_X.h"
#include "codechal_mmc_decode_hevc_g12.h"
#include "codechal_hw_g12_X.h"
#include "mos_util_user_interface_g12.h"
#include "codechal_decode_histogram.h"
#include "codechal_debug.h"
#include "hal_oca_interface.h"

//==<Functions>=======================================================
MOS_STATUS CodechalDecodeHevcG12::AllocateResourcesVariableSizes ()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    uint8_t maxBitDepth     = (m_is12BitHevc) ? 12: ((m_is10BitHevc) ? 10 : 8);
    uint8_t chromaFormatPic = m_hevcPicParams->chroma_format_idc;
    uint8_t chromaFormat    = m_chromaFormatinProfile;
    CODECHAL_DECODE_ASSERT(chromaFormat >= chromaFormatPic);

    uint32_t widthMax     = MOS_MAX(m_width, m_widthLastMaxAlloced);
    uint32_t heightMax    = MOS_MAX(m_height, m_heightLastMaxAlloced);
    uint32_t frameSizeMax = MOS_MAX((m_copyDataBufferInUse ? m_copyDataBufferSize : m_dataSize), m_frameSizeMaxAlloced);

    uint32_t ctbLog2SizeYPic = m_hevcPicParams->log2_diff_max_min_luma_coding_block_size +
                               m_hevcPicParams->log2_min_luma_coding_block_size_minus3 + 3;
    uint32_t ctbLog2SizeY = MOS_MAX(ctbLog2SizeYPic, m_ctbLog2SizeYMax);

    //for Scalability
    if (CodecHalDecodeScalabilityIsScalableMode(m_scalabilityState))
    {
        MHW_VDBOX_HCP_BUFFER_REALLOC_PARAMS reallocParam;
        MOS_ZeroMemory(&reallocParam, sizeof(reallocParam));
        reallocParam.ucMaxBitDepth     = maxBitDepth;
        reallocParam.ucChromaFormat    = chromaFormat;
        reallocParam.dwCtbLog2SizeY    = ctbLog2SizeY;
        reallocParam.dwCtbLog2SizeYMax  = m_ctbLog2SizeYMax;
        reallocParam.dwPicWidth         = widthMax;
        reallocParam.dwPicWidthAlloced  = m_widthLastMaxAlloced;
        reallocParam.dwPicHeight        = heightMax;
        reallocParam.dwPicHeightAlloced = m_heightLastMaxAlloced;
        reallocParam.dwFrameSize        = frameSizeMax;
        reallocParam.dwFrameSizeAlloced = m_frameSizeMaxAlloced;

        MHW_VDBOX_HCP_BUFFER_SIZE_PARAMS hcpBufSizeParam;
        MOS_ZeroMemory(&hcpBufSizeParam, sizeof(hcpBufSizeParam));
        hcpBufSizeParam.ucMaxBitDepth  = maxBitDepth;
        hcpBufSizeParam.ucChromaFormat = chromaFormat;
        hcpBufSizeParam.dwCtbLog2SizeY = ctbLog2SizeY;
        hcpBufSizeParam.dwPicWidth      = widthMax;
        hcpBufSizeParam.dwPicHeight     = heightMax;
        hcpBufSizeParam.dwMaxFrameSize  = frameSizeMax;

        CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHalDecodeScalability_AllocateResources_VariableSizes_G12(
            m_scalabilityState,
            &hcpBufSizeParam,
            &reallocParam));

        m_frameSizeMaxAlloced = frameSizeMax;
    }

    if (CodecHalDecodeIsSCCIBCMode(m_hevcSccPicParams))
    {
        bool isNeedBiggerSize = (widthMax > m_widthLastMaxAlloced) || (heightMax > m_heightLastMaxAlloced);
        bool isResourceNull = Mos_ResourceIsNull(&m_resRefBeforeLoopFilter);
        if (isNeedBiggerSize || isResourceNull)
        {
            if (!isResourceNull)
            {
                m_osInterface->pfnFreeResource(
                    m_osInterface,
                    &m_resRefBeforeLoopFilter);
            }

            // allocate an internal temporary buffer as reference which holds pixels before in-loop filter
            CODECHAL_DECODE_CHK_STATUS_RETURN(AllocateResourceRefBefLoopFilter());
        }
    }

    // Reallocate second level batch buffer if it is needed
    if (!m_cencBuf)
    {
        uint32_t  count, size;

        if (m_isRealTile)
        {
            count = m_hevcPicParams->num_tile_columns_minus1 + 1;
            size = m_standardDecodeSizeNeeded * (m_decodeParams.m_numSlices + 1 + m_hevcPicParams->num_tile_rows_minus1);
        }
        else if (m_isSeparateTileDecoding)
        {
            count = 1;
            size = m_standardDecodeSizeNeeded * (m_decodeParams.m_numSlices 
                + (m_hevcPicParams->num_tile_rows_minus1 + 1) * (m_hevcPicParams->num_tile_columns_minus1 + 1));
        }
        else
        {
            count = 1;
            size = m_standardDecodeSizeNeeded * m_decodeParams.m_numSlices;
        }

        if ((!Mos_ResourceIsNull(&m_secondLevelBatchBuffer[m_secondLevelBatchBufferIndex].OsResource) &&
                (size > (uint32_t)m_secondLevelBatchBuffer[m_secondLevelBatchBufferIndex].iSize)) ||
            (count > m_secondLevelBatchBuffer[m_secondLevelBatchBufferIndex].count))
        {
            Mhw_FreeBb(m_osInterface, &m_secondLevelBatchBuffer[m_secondLevelBatchBufferIndex], nullptr);
        }
        if (Mos_ResourceIsNull(&m_secondLevelBatchBuffer[m_secondLevelBatchBufferIndex].OsResource))
        {
            MOS_ZeroMemory(&m_secondLevelBatchBuffer[m_secondLevelBatchBufferIndex], sizeof(MHW_BATCH_BUFFER));
            CODECHAL_DECODE_CHK_STATUS_RETURN(Mhw_AllocateBb(
                m_osInterface,
                &m_secondLevelBatchBuffer[m_secondLevelBatchBufferIndex],
                nullptr,
                size,
                count));
            m_secondLevelBatchBuffer[m_secondLevelBatchBufferIndex].bSecondLevel = true;
        }
    }

    CODECHAL_DECODE_CHK_STATUS_RETURN(CodechalDecodeHevc::AllocateResourcesVariableSizes());

#ifdef _MMC_SUPPORTED
    if (m_mmc && m_mmc->IsMmcEnabled() && MEDIA_IS_WA(m_waTable, WaClearCcsVe) && 
        !Mos_ResourceIsNull(&m_destSurface.OsResource) && 
        m_destSurface.OsResource.bConvertedFromDDIResource)
    {
        CODECHAL_DECODE_VERBOSEMESSAGE("Clear CCS by VE resolve before frame %d submission", m_frameNum);
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnDecompResource(m_osInterface, &m_destSurface.OsResource));
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnSetGpuContext(m_osInterface, m_videoContext));
    }
#endif

    return eStatus;
}

MOS_STATUS CodechalDecodeHevcG12::AllocateResourceRefBefLoopFilter()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    if (!Mos_ResourceIsNull(&m_resRefBeforeLoopFilter))
    {
        return MOS_STATUS_SUCCESS;
    }

    MOS_SURFACE surface;
    CODECHAL_DECODE_CHK_STATUS_MESSAGE_RETURN(AllocateSurface(
                                                  &surface,
                                                  m_destSurface.dwPitch,
                                                  m_destSurface.dwHeight,
                                                  "Reference before loop filter",
                                                  m_destSurface.Format),
        "Failed to allocate reference before loop filter for IBC.");

    m_resRefBeforeLoopFilter = surface.OsResource;

    return eStatus;
}

MOS_STATUS CodechalDecodeHevcG12::AllocateHistogramSurface()
{
    MOS_ALLOC_GFXRES_PARAMS allocParams;

    if (m_histogramSurface == nullptr)
    {
        m_histogramSurface = (MOS_SURFACE*)MOS_AllocAndZeroMemory(sizeof(MOS_SURFACE));
        CODECHAL_DECODE_CHK_NULL_RETURN(m_histogramSurface);

        MOS_ZeroMemory(&allocParams, sizeof(MOS_ALLOC_GFXRES_PARAMS));
        allocParams.Type = MOS_GFXRES_BUFFER;
        allocParams.TileType = MOS_TILE_LINEAR;
        allocParams.Format = Format_Buffer;
        allocParams.dwBytes = 256 * 4;
        allocParams.pBufName = "HistogramStreamOut";

        CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnAllocateResource(
            m_osInterface,
            &allocParams,
            &m_histogramSurface->OsResource));

        CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHalGetResourceInfo(
            m_osInterface,
            m_histogramSurface));
    }

    if(m_decodeHistogram)
        m_decodeHistogram->SetSrcHistogramSurface(m_histogramSurface);

    return MOS_STATUS_SUCCESS;
}

CodechalDecodeHevcG12::~CodechalDecodeHevcG12 ()
{
    CODECHAL_DECODE_FUNCTION_ENTER;

    if (m_sinlgePipeVeState)
    {
        MOS_FreeMemAndSetNull(m_sinlgePipeVeState);
    }
    if (m_scalabilityState)
    {
        CodecHalDecodeScalability_Destroy(m_scalabilityState);
        MOS_FreeMemAndSetNull(m_scalabilityState);
    }

    if (!Mos_ResourceIsNull(&m_resRefBeforeLoopFilter))
    {
        m_osInterface->pfnFreeResource(m_osInterface, &m_resRefBeforeLoopFilter);
    }
    for (uint32_t i = 0; i < CODEC_HEVC_NUM_SECOND_BB; i++)
    {
        if (!Mos_ResourceIsNull(&m_secondLevelBatchBuffer[i].OsResource))
    {
            Mhw_FreeBb(m_osInterface, &m_secondLevelBatchBuffer[i], nullptr);
        }
    }
    //Note: virtual engine interface destroy is done in MOS layer
#if (_DEBUG || _RELEASE_INTERNAL)
    // Report real tile frame count and virtual tile frame count
    MOS_USER_FEATURE_VALUE_WRITE_DATA   userFeatureWriteData = __NULL_USER_FEATURE_VALUE_WRITE_DATA__;

    userFeatureWriteData.Value.i32Data = m_rtFrameCount;
    userFeatureWriteData.ValueID = __MEDIA_USER_FEATURE_VALUE_ENABLE_HEVC_DECODE_RT_FRAME_COUNT_ID_G12;
    MOS_UserFeature_WriteValues_ID(nullptr, &userFeatureWriteData, 1);

    userFeatureWriteData.Value.i32Data = m_vtFrameCount;
    userFeatureWriteData.ValueID = __MEDIA_USER_FEATURE_VALUE_ENABLE_HEVC_DECODE_VT_FRAME_COUNT_ID_G12;
    MOS_UserFeature_WriteValues_ID(nullptr, &userFeatureWriteData, 1);

    userFeatureWriteData.Value.i32Data = m_spFrameCount;
    userFeatureWriteData.ValueID = __MEDIA_USER_FEATURE_VALUE_ENABLE_HEVC_DECODE_SP_FRAME_COUNT_ID_G12;
    MOS_UserFeature_WriteValues_ID(nullptr, &userFeatureWriteData, 1);

#endif

    if (m_histogramSurface)
    {
        if (!Mos_ResourceIsNull(&m_histogramSurface->OsResource))
        {
            m_osInterface->pfnFreeResource(
                m_osInterface,
                &m_histogramSurface->OsResource);
        }
        MOS_FreeMemory(m_histogramSurface);
        m_histogramSurface = nullptr;
    }

    return;
}

MOS_STATUS CodechalDecodeHevcG12::CheckLCUSize()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    uint16_t LCUSize = 1 << (m_hevcPicParams->log2_diff_max_min_luma_coding_block_size + 
        m_hevcPicParams->log2_min_luma_coding_block_size_minus3 + 3);

    if (m_width > CODECHAL_HEVC_MAX_DIM_FOR_MIN_LCU || m_height > CODECHAL_HEVC_MAX_DIM_FOR_MIN_LCU)
    {
        if (LCUSize == CODECHAL_HEVC_MIN_LCU)
        {
            CODECHAL_DECODE_ASSERTMESSAGE("Invalid LCU size.");
            return MOS_STATUS_INVALID_PARAMETER;
        }
    }

    return eStatus;
}

MOS_STATUS CodechalDecodeHevcG12::SetGpuCtxCreatOption(
    CodechalSetting *codecHalSetting)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    if (!MOS_VE_CTXBASEDSCHEDULING_SUPPORTED(m_osInterface))
    {
        CodechalDecode::SetGpuCtxCreatOption(codecHalSetting);
    }
    else
    {
        m_gpuCtxCreatOpt = MOS_New(MOS_GPUCTX_CREATOPTIONS_ENHANCED);
        CODECHAL_DECODE_CHK_NULL_RETURN(m_gpuCtxCreatOpt);

        if (static_cast<MhwVdboxMfxInterfaceG12 *>(m_mfxInterface)->IsScalabilitySupported())
        {
            CODECHAL_DECODE_CHK_STATUS_RETURN(CodechalDecodeScalability_ConstructParmsForGpuCtxCreation(
                m_scalabilityState,
                (PMOS_GPUCTX_CREATOPTIONS_ENHANCED)m_gpuCtxCreatOpt,
                codecHalSetting));

            if (((PMOS_GPUCTX_CREATOPTIONS_ENHANCED)m_gpuCtxCreatOpt)->LRCACount == 2)
            {
                m_videoContext = MOS_VE_MULTINODESCALING_SUPPORTED(m_osInterface) ? MOS_GPU_CONTEXT_VIDEO5 : MOS_GPU_CONTEXT_VDBOX2_VIDEO;;

                CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnCreateGpuContext(
                    m_osInterface,
                    m_videoContext,
                    MOS_GPU_NODE_VIDEO,
                    m_gpuCtxCreatOpt));

                MOS_GPUCTX_CREATOPTIONS createOption;
                CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnCreateGpuContext(
                    m_osInterface,
                    MOS_GPU_CONTEXT_VIDEO,
                    m_videoGpuNode,
                    &createOption));
            }
            else if (((PMOS_GPUCTX_CREATOPTIONS_ENHANCED)m_gpuCtxCreatOpt)->LRCACount == 3)
            {
                m_videoContext = MOS_VE_MULTINODESCALING_SUPPORTED(m_osInterface) ? MOS_GPU_CONTEXT_VIDEO7 : MOS_GPU_CONTEXT_VDBOX2_VIDEO2;

                CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnCreateGpuContext(
                    m_osInterface,
                    m_videoContext,
                    MOS_GPU_NODE_VIDEO,
                    m_gpuCtxCreatOpt));

                MOS_GPUCTX_CREATOPTIONS createOption;
                CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnCreateGpuContext(
                    m_osInterface,
                    MOS_GPU_CONTEXT_VIDEO,
                    m_videoGpuNode,
                    &createOption));
            }
            else
            {
                m_videoContext = MOS_GPU_CONTEXT_VIDEO;
            }
        }
        else
        {
            bool sfcInUse = (codecHalSetting->sfcInUseHinted && codecHalSetting->downsamplingHinted 
                             && (MEDIA_IS_SKU(m_skuTable, FtrSFCPipe) && !MEDIA_IS_SKU(m_skuTable, FtrDisableVDBox2SFC)));
            CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHalDecodeSinglePipeVE_ConstructParmsForGpuCtxCreation(
                m_sinlgePipeVeState,
                (PMOS_GPUCTX_CREATOPTIONS_ENHANCED)m_gpuCtxCreatOpt,
                sfcInUse));

            m_videoContext = MOS_GPU_CONTEXT_VIDEO;
        }
    }

    return eStatus;
}

uint32_t CodechalDecodeHevcG12::GetDmemBufferSize()
{
    return MOS_ALIGN_CEIL(sizeof(HUC_HEVC_S2L_BSS_G12), CODECHAL_CACHELINE_SIZE);
}

MOS_STATUS CodechalDecodeHevcG12::SetHucDmemS2LPictureBss(
    PHUC_HEVC_S2L_PIC_BSS       hucHevcS2LPicBss)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    CODECHAL_DECODE_CHK_NULL_RETURN(hucHevcS2LPicBss);

    CODECHAL_DECODE_CHK_STATUS_RETURN(CodechalDecodeHevc::SetHucDmemS2LPictureBss(hucHevcS2LPicBss));

    if (m_hevcExtPicParams)
    {
        hucHevcS2LPicBss->high_precision_offsets_enabled_flag =
            m_hevcExtPicParams->PicRangeExtensionFlags.fields.high_precision_offsets_enabled_flag;
        hucHevcS2LPicBss->chroma_qp_offset_list_enabled_flag =
            m_hevcExtPicParams->PicRangeExtensionFlags.fields.chroma_qp_offset_list_enabled_flag;
    }
    else
    {
        hucHevcS2LPicBss->high_precision_offsets_enabled_flag = 0;
        hucHevcS2LPicBss->chroma_qp_offset_list_enabled_flag = 0;
    }

    PHUC_HEVC_S2L_PIC_BSS_G12 hucHevcS2LPicBssG12 = static_cast<PHUC_HEVC_S2L_PIC_BSS_G12>(hucHevcS2LPicBss);

    hucHevcS2LPicBssG12->IsRealTileEnable = 0;
    if (m_isRealTile)
    {
        hucHevcS2LPicBssG12->IsRealTileEnable  = 1;
        hucHevcS2LPicBssG12->BatchBufferSize  = m_secondLevelBatchBuffer[m_secondLevelBatchBufferIndex].iSize;
        hucHevcS2LPicBssG12->NumScalablePipes  = m_scalabilityState->ucScalablePipeNum;
    }
    else if (CodecHalDecodeNeedsTileDecoding(m_hevcPicParams, m_hevcSccPicParams))
    {
        hucHevcS2LPicBssG12->NumScalablePipes = 1;
    }

    hucHevcS2LPicBssG12->IsSCCIBCMode = CodecHalDecodeIsSCCIBCMode(m_hevcSccPicParams);
    hucHevcS2LPicBssG12->IsSCCPLTMode = CodecHalDecodeIsSCCPLTMode(m_hevcSccPicParams);
    if (hucHevcS2LPicBssG12->IsSCCIBCMode)
    {
        uint8_t i = 0, k = 0;
        for (i = 0; i < CODEC_MAX_NUM_REF_FRAME_HEVC; i++)
        {
            if (hucHevcS2LPicBssG12->PicOrderCntValList[i] == hucHevcS2LPicBssG12->CurrPicOrderCntVal)
            {
                break;
            }
        }
        for (k = 0; k < CODECHAL_MAX_CUR_NUM_REF_FRAME_HEVC; k++)
        {
            if (hucHevcS2LPicBssG12->RefPicSetLtCurr[k] == 0xFF)
            {
                hucHevcS2LPicBssG12->RefPicSetLtCurr[k] = i;
                break;
            }
        }
    }
    
    if (hucHevcS2LPicBssG12->IsSCCPLTMode)
    {
        hucHevcS2LPicBssG12->PredictorPaletteSize = m_hevcSccPicParams->PredictorPaletteSize;
        MOS_SecureMemcpy(hucHevcS2LPicBssG12->PredictorPaletteEntries,
            sizeof(hucHevcS2LPicBssG12->PredictorPaletteEntries),
            m_hevcSccPicParams->PredictorPaletteEntries,
            sizeof(m_hevcSccPicParams->PredictorPaletteEntries));
    }
    else
    {
        hucHevcS2LPicBssG12->PredictorPaletteSize = 0;
        MOS_ZeroMemory(hucHevcS2LPicBssG12->PredictorPaletteEntries, sizeof(hucHevcS2LPicBssG12->PredictorPaletteEntries));
    }

    if (m_hevcSccPicParams)
    {
        hucHevcS2LPicBssG12->UseSliceACTOffset = m_hevcSccPicParams->PicSCCExtensionFlags.fields.pps_slice_act_qp_offsets_present_flag;
        hucHevcS2LPicBssG12->pps_act_y_qp_offset = m_hevcSccPicParams->pps_act_y_qp_offset_plus5 - 5;
        hucHevcS2LPicBssG12->pps_act_cb_qp_offset = m_hevcSccPicParams->pps_act_cb_qp_offset_plus5 - 5;
        hucHevcS2LPicBssG12->pps_act_cr_qp_offset = m_hevcSccPicParams->pps_act_cr_qp_offset_plus3 - 3;
        hucHevcS2LPicBssG12->MVRControlIdc = m_hevcSccPicParams->PicSCCExtensionFlags.fields.motion_vector_resolution_control_idc;
    }
    else
    {
        hucHevcS2LPicBssG12->UseSliceACTOffset = 0;
        hucHevcS2LPicBssG12->pps_act_y_qp_offset = 0;
        hucHevcS2LPicBssG12->pps_act_cb_qp_offset = 0;
        hucHevcS2LPicBssG12->pps_act_cr_qp_offset = 0;
        hucHevcS2LPicBssG12->MVRControlIdc = 0;
    }

    return eStatus;
}

MOS_STATUS CodechalDecodeHevcG12::CalcDownsamplingParams(
    void                        *picParams,
    uint32_t                    *refSurfWidth,
    uint32_t                    *refSurfHeight,
    MOS_FORMAT                  *format,
    uint8_t                     *frameIdx)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_CHK_NULL_RETURN(picParams);
    CODECHAL_DECODE_CHK_NULL_RETURN(refSurfWidth);
    CODECHAL_DECODE_CHK_NULL_RETURN(refSurfHeight);
    CODECHAL_DECODE_CHK_NULL_RETURN(format);
    CODECHAL_DECODE_CHK_NULL_RETURN(frameIdx);

    PCODEC_HEVC_PIC_PARAMS hevcPicParams = (PCODEC_HEVC_PIC_PARAMS)picParams;

    *refSurfWidth = 0;
    *refSurfHeight = 0;
    *format = Format_NV12;
    *frameIdx = hevcPicParams->CurrPic.FrameIdx;

    uint32_t                         widthInPix, heightInPix;

    widthInPix = (1 << (hevcPicParams->log2_min_luma_coding_block_size_minus3 + 3)) * (hevcPicParams->PicWidthInMinCbsY);
    heightInPix = (1 << (hevcPicParams->log2_min_luma_coding_block_size_minus3 + 3)) * (hevcPicParams->PicHeightInMinCbsY);

    *refSurfWidth = MOS_ALIGN_CEIL(widthInPix, 64);
    *refSurfHeight = MOS_ALIGN_CEIL(heightInPix, 64);

    if (hevcPicParams->chroma_format_idc == HCP_CHROMA_FORMAT_YUV444)
    {
        if (hevcPicParams->bit_depth_luma_minus8 > 2 || hevcPicParams->bit_depth_chroma_minus8 > 2)
        {
            *format = Format_Y416;
        }
        else if (hevcPicParams->bit_depth_luma_minus8 > 0 || hevcPicParams->bit_depth_chroma_minus8 > 0)
        {
            *format = Format_Y410;
        }
        else
        {
            *format = Format_AYUV;
        }
    }
    else if (hevcPicParams->chroma_format_idc == HCP_CHROMA_FORMAT_YUV422)
    {
        if (hevcPicParams->bit_depth_luma_minus8 > 2 || hevcPicParams->bit_depth_chroma_minus8 > 2)
        {
            *format = Format_Y216;
        }
        else if (hevcPicParams->bit_depth_luma_minus8 > 0 || hevcPicParams->bit_depth_chroma_minus8 > 0)
        {
            *format = Format_Y210;
        }
        else
        {
            *format = Format_YUY2;
        }
    }
    else
    {
        if (hevcPicParams->bit_depth_luma_minus8 > 2 || hevcPicParams->bit_depth_chroma_minus8 > 2)
        {
            *format = Format_P016;
        }
        else if (hevcPicParams->bit_depth_luma_minus8 > 0 || hevcPicParams->bit_depth_chroma_minus8 > 0)
        {
            *format = Format_P010;
        }
        else
        {
            *format = Format_NV12;
        }
    }

    return eStatus;
}

MOS_STATUS CodechalDecodeHevcG12::SetHucDmemParams (
    PMOS_RESOURCE               dmemBuffer)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    CODECHAL_DECODE_CHK_NULL_RETURN(dmemBuffer);

    CodechalResLock DmemLock(m_osInterface, dmemBuffer);
    auto hucHevcS2LBss = (PHUC_HEVC_S2L_BSS_G12)DmemLock.Lock(CodechalResLock::writeOnly);

    CODECHAL_DECODE_CHK_NULL_RETURN(hucHevcS2LBss);
    hucHevcS2LBss->ProductFamily = m_hucInterface->GetHucProductFamily();
    hucHevcS2LBss->RevId = m_hwInterface->GetPlatform().usRevId;
    hucHevcS2LBss->DummyRefIdxState = 
        MEDIA_IS_WA(m_waTable, WaDummyReference) && !m_osInterface->bSimIsActive;

    CODECHAL_DECODE_CHK_STATUS_RETURN(SetHucDmemS2LPictureBss(&hucHevcS2LBss->PictureBss));
    CODECHAL_DECODE_CHK_STATUS_RETURN(SetHucDmemS2LSliceBss(&hucHevcS2LBss->SliceBss[0]));

    if (m_secureDecoder)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_secureDecoder->SetHevcHucDmemS2LBss(this, &hucHevcS2LBss->PictureBss, &hucHevcS2LBss->SliceBss[0]));
    }

    if (m_numSlices < CODECHAL_HEVC_MAX_NUM_SLICES_LVL_6)
    {
        m_dmemTransferSize = (uint32_t)((uint8_t *)&(hucHevcS2LBss->SliceBss[m_numSlices]) - (uint8_t *)hucHevcS2LBss);
        m_dmemTransferSize = MOS_ALIGN_CEIL(m_dmemTransferSize, CODECHAL_CACHELINE_SIZE);
    }
    else
    {
        m_dmemTransferSize = m_dmemBufferSize;
    }

    return eStatus;
}

MOS_STATUS CodechalDecodeHevcG12 ::InitializeDecodeMode()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

        if (MOS_VE_SUPPORTED(m_osInterface) && static_cast<MhwVdboxMfxInterfaceG12 *>(m_mfxInterface)->IsScalabilitySupported())
    {
        CODECHAL_DECODE_SCALABILITY_INIT_PARAMS_G12 initParams;

        MOS_ZeroMemory(&initParams, sizeof(initParams));
        initParams.u32PicWidthInPixel  = m_width;
        initParams.u32PicHeightInPixel = m_height;
        initParams.bIsTileEnabled      = m_hevcPicParams->tiles_enabled_flag;
        initParams.format              = m_decodeParams.m_destSurface->Format;
        initParams.usingSecureDecode   = m_secureDecoder ? m_secureDecoder->IsSecureDecodeEnabled() : false;
        // Only support SCC real tile mode. SCC virtual tile scalability mode is disabled here
        initParams.bIsSccDecoding   = m_hevcSccPicParams != nullptr;
        initParams.u8NumTileColumns = m_hevcPicParams->num_tile_columns_minus1 + 1;
        initParams.u8NumTileRows    = m_hevcPicParams->num_tile_rows_minus1 + 1;
        initParams.gpuCtxInUse      = GetVideoContext();

        CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHalDecodeScalability_InitScalableParams_G12(
            m_scalabilityState,
            &initParams,
            &m_decodePassNum));

        if (MOS_VE_CTXBASEDSCHEDULING_SUPPORTED(m_osInterface))
        {
            CODECHAL_DECODE_CHK_STATUS_RETURN(CodechalDecodeScalability_ChkGpuCtxReCreation(
                m_scalabilityState,
                (PMOS_GPUCTX_CREATOPTIONS_ENHANCED)m_gpuCtxCreatOpt));
            SetVideoContext(m_scalabilityState->VideoContext);
        }

        m_isRealTile = CodecHalDecodeScalabilityIsRealTileMode(m_scalabilityState);
        if (m_isRealTile)
        {
            m_isSeparateTileDecoding = false;
        }

#if (_DEBUG || _RELEASE_INTERNAL)
        if (m_isRealTile)
        {
            m_rtFrameCount++;
        }
        else if (CodecHalDecodeScalabilityIsScalableMode(m_scalabilityState))
        {
            m_vtFrameCount++;
        }
        else
        {
            m_spFrameCount++;
        }
#endif
    }

    return eStatus;
}

MOS_STATUS CodechalDecodeHevcG12::SetFrameStates ()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    PERF_UTILITY_AUTO(__FUNCTION__, PERF_DECODE, PERF_LEVEL_HAL);
    CODECHAL_DECODE_FUNCTION_ENTER;

    CODECHAL_DECODE_CHK_NULL_RETURN(m_decodeParams.m_destSurface);
    CODECHAL_DECODE_CHK_NULL_RETURN(m_decodeParams.m_dataBuffer);

    m_frameIdx++;

    // Check HuC_status2 Imem loaded bit, if 0,return error
    // As driver doesn't know when can get reg value afer storing HuC_Status2 register,
    // Check the reg value here at the beginning of next frame
    // Check twice, first entry and second entry
    if (m_shortFormatInUse && m_frameIdx < 3 && m_statusQueryReportingEnabled &&
        (((m_decodeStatusBuf.m_decodeStatus->m_hucErrorStatus2 >> 32) & m_hucInterface->GetHucStatus2ImemLoadedMask()) == 0))
    {
        CODECHAL_DECODE_ASSERTMESSAGE("HuC IMEM Loaded fails");
        return MOS_STATUS_UNKNOWN;
    }

    m_cencBuf = m_decodeParams.m_cencBuf;

    if (IsFirstExecuteCall())    // For DRC Multiple Execution Call, no need to update every value in pHevcState except first execute
    {
        m_dataSize   = m_decodeParams.m_dataSize;
        m_dataOffset = m_decodeParams.m_dataOffset;
        m_numSlices  = m_decodeParams.m_numSlices;

        if (m_numSlices > CODECHAL_HEVC_MAX_NUM_SLICES_LVL_6)
        {
            CODECHAL_DECODE_ASSERTMESSAGE("Slice number doesn't support!");
            return MOS_STATUS_INVALID_PARAMETER;
        }

        m_hevcPicParams    = (PCODEC_HEVC_PIC_PARAMS)m_decodeParams.m_picParams;
        m_hevcExtPicParams = (PCODEC_HEVC_EXT_PIC_PARAMS)m_decodeParams.m_extPicParams;
        m_hevcSccPicParams = (PCODEC_HEVC_SCC_PIC_PARAMS)m_decodeParams.m_advPicParams;
        CODECHAL_DECODE_CHK_NULL_RETURN(m_decodeParams.m_sliceParams);
        m_hevcSliceParams    = (PCODEC_HEVC_SLICE_PARAMS)m_decodeParams.m_sliceParams;
        m_hevcExtSliceParams = (PCODEC_HEVC_EXT_SLICE_PARAMS)m_decodeParams.m_extSliceParams;
        m_hevcSubsetParams   = (PCODEC_HEVC_SUBSET_PARAMS)m_decodeParams.m_subsetParams;
        m_hevcIqMatrixParams = (PCODECHAL_HEVC_IQ_MATRIX_PARAMS)m_decodeParams.m_iqMatrixBuffer;
        m_destSurface        = *(m_decodeParams.m_destSurface);
        m_resDataBuffer      = *(m_decodeParams.m_dataBuffer);
        CODECHAL_DECODE_CHK_STATUS_RETURN(InitializeBitstreamCat());
    }
    else
    {
        m_dataSize      = m_decodeParams.m_dataSize;
        m_dataOffset    = 0;
        m_resDataBuffer = *(m_decodeParams.m_dataBuffer);
    }

    if (m_hevcPicParams->RequestCRC)
    {
        m_reportFrameCrc = true;
    }

    CODECHAL_DECODE_CHK_STATUS_RETURN(CheckAndCopyBitstream());

    PCODEC_REF_LIST destEntry = m_hevcRefList[m_hevcPicParams->CurrPic.FrameIdx];
    MOS_ZeroMemory(destEntry, sizeof(CODEC_REF_LIST));
    
    if (m_incompletePicture)
    {
        return MOS_STATUS_SUCCESS;
    }

    CODECHAL_DECODE_CHK_NULL_RETURN(m_hevcPicParams);
    CODECHAL_DECODE_CHK_NULL_RETURN(m_hevcIqMatrixParams);

    // sanity check to make sure no same reference exists
    uint32_t m = 0, n = 0;
    for (m = 0; m < CODEC_MAX_NUM_REF_FRAME_HEVC; m++)
    {

        int32_t poc = m_hevcPicParams->PicOrderCntValList[m];
        for (n = m + 1; n < CODEC_MAX_NUM_REF_FRAME_HEVC; n++)
        {
            if (poc == m_hevcPicParams->PicOrderCntValList[n])
            {
                m_hevcPicParams->RefFrameList[n].PicFlags = PICTURE_INVALID;
            }
        }
    }

    // Calculate bCurPicIntra
    m_curPicIntra = true;
    if (m_hevcPicParams->IntraPicFlag == 0)
    {
        for (uint32_t i = 0; i < CODECHAL_MAX_CUR_NUM_REF_FRAME_HEVC; i++)
        {
            uint8_t frameIdx = m_hevcPicParams->RefPicSetStCurrBefore[i];
            if (frameIdx < 15)
            {
                m_curPicIntra = false;
                break;
            }

            frameIdx = m_hevcPicParams->RefPicSetStCurrAfter[i];
            if (frameIdx < 15)
            {
                m_curPicIntra = false;
                break;
            }

            frameIdx = m_hevcPicParams->RefPicSetLtCurr[i];
            if (frameIdx < 15)
            {
                m_curPicIntra = false;
                break;
            }
        }
    }

    m_twoVersionsOfCurrDecPicFlag = 0;
    if (CodecHalDecodeIsSCCIBCMode(m_hevcSccPicParams))
    {
        m_curPicIntra = false;
        m_twoVersionsOfCurrDecPicFlag = !m_hevcPicParams->pps_deblocking_filter_disabled_flag
            || m_hevcPicParams->sample_adaptive_offset_enabled_flag
            || m_hevcPicParams->deblocking_filter_override_enabled_flag;
#ifdef _MMC_SUPPORTED
        if (m_mmc->IsMmcEnabled())
        {
            // Due to limitation, IBC reference has to be uncompressed, while recon surface is still compressed.
            // Always need internal surface for IBC reference for MMC.
            m_twoVersionsOfCurrDecPicFlag = true;
        }
#endif
    }

    CODECHAL_DECODE_CHK_STATUS_RETURN(SetPictureStructs());

    uint32_t i;
    for (i = 0; i < CODEC_MAX_NUM_REF_FRAME_HEVC; i++)
    {
        m_frameUsedAsCurRef[i] = false;
        m_refIdxMapping[i]     = -1;
    }

    // Calculate RefIdxMapping
    for (i = 0; i < CODECHAL_MAX_CUR_NUM_REF_FRAME_HEVC; i++)
    {
        uint8_t frameIdx = m_hevcPicParams->RefPicSetStCurrBefore[i];
        if (frameIdx < CODEC_MAX_NUM_REF_FRAME_HEVC)
        {
            m_frameUsedAsCurRef[frameIdx] = true;
        }

        frameIdx = m_hevcPicParams->RefPicSetStCurrAfter[i];
        if (frameIdx < CODEC_MAX_NUM_REF_FRAME_HEVC)
        {
            m_frameUsedAsCurRef[frameIdx] = true;
        }

        frameIdx = m_hevcPicParams->RefPicSetLtCurr[i];
        if (frameIdx < CODEC_MAX_NUM_REF_FRAME_HEVC)
        {
            m_frameUsedAsCurRef[frameIdx] = true;
        }
    }

    if (CodecHalDecodeIsSCCIBCMode(m_hevcSccPicParams))
    {
        for (i = 0; i < CODEC_MAX_NUM_REF_FRAME_HEVC; i++)
        {
            if (!CodecHal_PictureIsInvalid(m_hevcPicParams->RefFrameList[i])
                && m_hevcPicParams->PicOrderCntValList[i] == m_hevcPicParams->CurrPicOrderCntVal)
            {
                m_frameUsedAsCurRef[i] = true;
                break;
            }
        }
    }

    uint8_t curRefIdx = 0;
    for (i = 0; i < CODEC_MAX_NUM_REF_FRAME_HEVC; i++)
    {
        if (m_frameUsedAsCurRef[i])
        {
            if (CodecHalDecodeIsSCCIBCMode(m_hevcSccPicParams))
            {
                if (m_hevcPicParams->PicOrderCntValList[i] == m_hevcPicParams->CurrPicOrderCntVal)
                {
                    // pre-dbk reference id for IBC mode
                    m_IBCRefIdx = curRefIdx;
                }
            }
            m_refIdxMapping[i] = curRefIdx++;
        }
    }

    CODECHAL_DECODE_ASSERT(curRefIdx <= 8);

    m_minCtbSize = 1 << (m_hevcPicParams->log2_min_luma_coding_block_size_minus3 + 3);
    m_width      = m_hevcPicParams->PicWidthInMinCbsY * m_minCtbSize;
    m_height     = m_hevcPicParams->PicHeightInMinCbsY * m_minCtbSize;

    m_ctbSize    = 1 << (m_hevcPicParams->log2_diff_max_min_luma_coding_block_size +
                      m_hevcPicParams->log2_min_luma_coding_block_size_minus3 + 3);

    CODECHAL_DECODE_CHK_STATUS_RETURN(CheckLCUSize());

    if (m_hcpInterface->IsRowStoreCachingSupported())
    {
        MHW_VDBOX_ROWSTORE_PARAMS rowstoreParams;
        rowstoreParams.Mode             = CODECHAL_DECODE_MODE_HEVCVLD;
        rowstoreParams.dwPicWidth       = m_width;
        rowstoreParams.bMbaff           = false;
        rowstoreParams.ucBitDepthMinus8 = (uint8_t)MOS_MAX(m_hevcPicParams->bit_depth_luma_minus8, m_hevcPicParams->bit_depth_chroma_minus8);
        rowstoreParams.ucChromaFormat   = m_hevcPicParams->chroma_format_idc;
        rowstoreParams.ucLCUSize        = (uint8_t)m_ctbSize;
        m_hwInterface->SetRowstoreCachingOffsets(&rowstoreParams);
    }

    // Calculate Tile info
    if (m_hevcPicParams->tiles_enabled_flag)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(GetAllTileInfo());
    }

    if (m_curPicIntra)
    {
        m_perfType = I_TYPE;
    }
    else
    {
        // Not possible to determine whether P or B is used for short format.
        // For long format iterating through all of the slices to determine P vs
        // B, so in order to avoid this, declare all other pictures MIXED_TYPE.
        m_perfType = MIXED_TYPE;
    }

    m_crrPic = m_hevcPicParams->CurrPic;
    m_secondField =
        CodecHal_PictureIsBottomField(m_hevcPicParams->CurrPic);

    CODECHAL_DEBUG_TOOL(
        m_debugInterface->m_currPic     = m_crrPic;
        m_debugInterface->m_secondField = m_secondField;
        m_debugInterface->m_frameType   = m_perfType;

        CODECHAL_DECODE_CHK_STATUS_RETURN(DumpPicParams(
            m_hevcPicParams,
            m_hevcExtPicParams,
            m_hevcSccPicParams));

        if (m_hevcIqMatrixParams) {
            CODECHAL_DECODE_CHK_STATUS_RETURN(DumpIQParams(m_hevcIqMatrixParams));
        }

        if (m_hevcSliceParams) {
            CODECHAL_DECODE_CHK_STATUS_RETURN(DumpSliceParams(
                m_hevcSliceParams,
                m_hevcExtSliceParams,
                m_numSlices,
                m_shortFormatInUse));
        }

        if(m_hevcSubsetParams) {
            CODECHAL_DECODE_CHK_STATUS_RETURN(DumpSubsetsParams(m_hevcSubsetParams));
        })

    // Clear DMEM buffer program flag
    if (m_shortFormatInUse)
    {
       m_dmemBufferProgrammed = false;
       m_dmemBufferIdx++;
       m_dmemBufferIdx %= CODECHAL_HEVC_NUM_DMEM_BUFFERS;
    }

    m_isSeparateTileDecoding = CodecHalDecodeNeedsTileDecoding(m_hevcPicParams, m_hevcSccPicParams);

    InitializeDecodeMode();
    
#ifdef _DECODE_PROCESSING_SUPPORTED
    if (m_decodeParams.m_procParams)
    {
        CodechalHevcSfcStateG12  *sfcStateG12 = static_cast<CodechalHevcSfcStateG12*>(m_sfcState);

        CODECHAL_DECODE_CHK_NULL_RETURN(sfcStateG12);
        
        CODECHAL_DECODE_CHK_STATUS_RETURN(AllocateHistogramSurface());

        CODECHAL_DECODE_CHK_STATUS_RETURN(sfcStateG12->CheckAndInitialize(
            (CODECHAL_DECODE_PROCESSING_PARAMS *)m_decodeParams.m_procParams,
            m_hevcPicParams,
            m_scalabilityState,
            m_histogramSurface));
    }
#endif

    CODECHAL_DEBUG_TOOL(
        if (!m_incompletePicture && !IsFirstExecuteCall()) {
            CODECHAL_DECODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
                &m_resCopyDataBuffer,
                CodechalDbgAttr::attrBitstream,
                "_DEC",
                m_estiBytesInBitstream,
                0,
                CODECHAL_NUM_MEDIA_STATES));
        })

    m_secondLevelBatchBufferIndex++;
    m_secondLevelBatchBufferIndex %= CODEC_HEVC_NUM_SECOND_BB;
    CODECHAL_DECODE_CHK_STATUS_RETURN(AllocateResourcesVariableSizes());

    if (m_twoVersionsOfCurrDecPicFlag)
    {
        m_hevcRefList[m_hevcPicParams->CurrPic.FrameIdx]->resRefPic = m_resRefBeforeLoopFilter;
    }
    else
    {
        m_hevcRefList[m_hevcPicParams->CurrPic.FrameIdx]->resRefPic = m_destSurface.OsResource;
    }

    m_hcpDecPhase = CodechalHcpDecodePhaseInitialized;

    if (!m_shortFormatInUse&&!m_cencBuf)
    {
        CodechalResLock bbLock(m_osInterface, &m_secondLevelBatchBuffer[m_secondLevelBatchBufferIndex].OsResource);
        uint8_t *bbBase = (uint8_t*)bbLock.Lock(CodechalResLock::writeOnly);

        HevcDecodeSliceLongG12 hevcLong(this, m_hcpInterface, m_miInterface);

        CODECHAL_DECODE_CHK_STATUS_RETURN(hevcLong.ProcessSliceLong(bbBase, m_secondLevelBatchBuffer[m_secondLevelBatchBufferIndex].iSize));
    }

    return eStatus;
}

//!
//! \brief    Determine Decode Phase
//! \details  Determine decode phase in hevc decode
//! \param    N/A
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success, else fail reason
//!
MOS_STATUS CodechalDecodeHevcG12::DetermineDecodePhase()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    if (static_cast<MhwVdboxMfxInterfaceG12*>(m_mfxInterface)->IsScalabilitySupported())
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHalDecodeScalability_DetermineDecodePhase_G12(
            m_scalabilityState,
            &m_hcpDecPhase));
    }
    else
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(CodechalDecodeHevc::DetermineDecodePhase());
    }

    CODECHAL_DECODE_VERBOSEMESSAGE("Current Decode Phase: %d.", m_hcpDecPhase);

    return eStatus;
}

MOS_STATUS CodechalDecodeHevcG12::SetAndPopulateVEHintParams(
    PMOS_COMMAND_BUFFER       primCmdBuf)
{
    MOS_STATUS                      eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    if (static_cast<MhwVdboxMfxInterfaceG12*>(m_mfxInterface)->IsScalabilitySupported())
    {
        CODECHAL_DECODE_SCALABILITY_SETHINT_PARMS scalSetParms;
        if (!MOS_VE_CTXBASEDSCHEDULING_SUPPORTED(m_osInterface))
        {
            scalSetParms.bNeedSyncWithPrevious       = true;
            scalSetParms.bSameEngineAsLastSubmission = false;
            scalSetParms.bSFCInUse                   = false;
        }
        CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHalDecodeScalability_SetHintParams_G12(m_scalabilityState, &scalSetParms));
        CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHalDecodeScalability_PopulateHintParams(m_scalabilityState, primCmdBuf));
    }
    else
    {
        if (!MOS_VE_CTXBASEDSCHEDULING_SUPPORTED(m_osInterface))
        {
            MOS_VIRTUALENGINE_SET_PARAMS  vesetParams;
            MOS_ZeroMemory(&vesetParams, sizeof(vesetParams));
            vesetParams.bNeedSyncWithPrevious       = true;
            vesetParams.bSameEngineAsLastSubmission = false;
            vesetParams.bSFCInUse                   = false;
            CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHalDecodeSinglePipeVE_SetHintParams(m_sinlgePipeVeState, &vesetParams));
        }
        CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHalDecodeSinglePipeVE_PopulateHintParams(m_sinlgePipeVeState, primCmdBuf, true));
    }

    return eStatus;
}

MOS_STATUS CodechalDecodeHevcG12::DetermineSendProlgwithFrmTracking(
    bool                        *sendPrologWithFrameTracking)
{
    MOS_STATUS       eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    CODECHAL_DECODE_CHK_NULL_RETURN(sendPrologWithFrameTracking);

    if (CodecHalDecodeScalabilityIsScalableMode(m_scalabilityState))
    {
        if (CodecHalDecodeScalability1stPhaseofSubmission(m_scalabilityState) ||
            (CodecHalDecodeScalabilityIsFirstRealTilePhase(m_scalabilityState) && (m_scalabilityState->u8RtCurPipe == 0)))
        {
            *sendPrologWithFrameTracking = true;
        }
    }
    else
    {
        if (m_shortFormatInUse)
        {
            *sendPrologWithFrameTracking = m_enableSf2DmaSubmits ? true : false;
        }
        else
        {
            *sendPrologWithFrameTracking = true;
        }
    }

    return eStatus;
}

uint32_t CodechalDecodeHevcG12::RequestedSpaceSize(uint32_t requestedSize)
{
    CODECHAL_DECODE_FUNCTION_ENTER;

    if (m_scalabilityState && m_scalabilityState->bScalableDecodeMode)
    {
        //primary cmd buffer only including cmd buffer header .
        return COMMAND_BUFFER_RESERVED_SPACE * 2;
    }
    else
    {
        return requestedSize;
    }
}

void CodechalDecodeHevcG12::CalcRequestedSpace(
    uint32_t       &requestedSize,
    uint32_t       &additionalSizeNeeded,
    uint32_t       &requestedPatchListSize)
{
    if (m_isRealTile)
    {
        if (m_cencBuf)
        {
            requestedSize = m_commandBufferSizeNeeded;
            requestedPatchListSize = m_commandPatchListSizeNeeded;
            additionalSizeNeeded = 0;
        }
        else
        {
            requestedSize = m_commandBufferSizeNeeded +
                            (m_standardDecodeSizeNeeded * (m_decodeParams.m_numSlices + 1 + m_hevcPicParams->num_tile_rows_minus1));
            requestedPatchListSize = m_commandPatchListSizeNeeded +
                                     (m_standardDecodePatchListSizeNeeded * (m_decodeParams.m_numSlices + 1 + m_hevcPicParams->num_tile_rows_minus1));
            additionalSizeNeeded = COMMAND_BUFFER_RESERVED_SPACE;
        }
        requestedSize *= m_scalabilityState->u8RtPhaseNum;
        requestedPatchListSize *= m_scalabilityState->u8RtPhaseNum;
    }
    else if (CodecHalDecodeNeedsTileDecoding(m_hevcPicParams, m_hevcSccPicParams))
    {
        if (m_cencBuf)
        {
            requestedSize = m_commandBufferSizeNeeded;
            requestedPatchListSize = m_commandPatchListSizeNeeded;
            additionalSizeNeeded = 0;
        }
        else
        {
            requestedSize = m_commandBufferSizeNeeded +
                            m_standardDecodeSizeNeeded * (m_decodeParams.m_numSlices + (1 + m_hevcPicParams->num_tile_rows_minus1) * (1 + m_hevcPicParams->num_tile_columns_minus1));
            requestedPatchListSize = m_commandPatchListSizeNeeded +
                                     m_standardDecodePatchListSizeNeeded * (m_decodeParams.m_numSlices + (1 + m_hevcPicParams->num_tile_rows_minus1) * (1 + m_hevcPicParams->num_tile_columns_minus1));
            additionalSizeNeeded = COMMAND_BUFFER_RESERVED_SPACE;
        }
    }
    else
    {
        if (m_cencBuf)
        {
            requestedSize = m_commandBufferSizeNeeded;
            requestedPatchListSize = m_commandPatchListSizeNeeded;
            additionalSizeNeeded = 0;
        }
        else
        {
            requestedSize = m_commandBufferSizeNeeded +
                (m_standardDecodeSizeNeeded * (m_decodeParams.m_numSlices + 1));
            requestedPatchListSize = m_commandPatchListSizeNeeded +
                (m_standardDecodePatchListSizeNeeded * (m_decodeParams.m_numSlices + 1));
            additionalSizeNeeded = COMMAND_BUFFER_RESERVED_SPACE;
        }
    }
}

MOS_STATUS CodechalDecodeHevcG12::VerifyExtraSpace(
    uint32_t requestedSize,
    uint32_t additionalSizeNeeded)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    if (m_scalabilityState && m_scalabilityState->bScalableDecodeMode)
    {
        eStatus = MOS_STATUS_NO_SPACE;

        // Try a maximum of 3 attempts to request the required sizes from OS
        // OS could reset the sizes if necessary, therefore, requires to re-verify
        for (auto i = 0; (i < 3) && (eStatus != MOS_STATUS_SUCCESS); i++)
        {
            // Verify secondary cmd buffer
            eStatus = (MOS_STATUS)m_osInterface->pfnVerifyCommandBufferSize(
                m_osInterface,
                requestedSize,
                MOS_VE_HAVE_SECONDARY_CMDBUFFER);

            // Resize command buffer if not enough
            if (eStatus != MOS_STATUS_SUCCESS)
            {
                CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnResizeCommandBufferAndPatchList(
                    m_osInterface,
                    requestedSize + additionalSizeNeeded,
                    0,
                    MOS_VE_HAVE_SECONDARY_CMDBUFFER));
                // Set status to NO_SPACE to enter the commaned buffer size verification on next loop.
                eStatus = MOS_STATUS_NO_SPACE;
            }
        }
    }

    return eStatus;
}

MOS_STATUS CodechalDecodeHevcG12::DecodeStateLevel()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    PERF_UTILITY_AUTO(__FUNCTION__, PERF_DECODE, PERF_LEVEL_HAL);
    CODECHAL_DECODE_FUNCTION_ENTER;

    //HCP Decode Phase State Machine
    CODECHAL_DECODE_CHK_STATUS_RETURN(DetermineDecodePhase());

    if (CodecHalDecodeScalabilityIsScalableMode(m_scalabilityState))
    {
        //Switch GPU context when necessary
        CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHalDecodeScalability_SwitchGpuContext(m_scalabilityState));
    }

    // Set HEVC Decode Phase, and execute it.
    if (m_shortFormatInUse && m_hcpDecPhase == CodechalHcpDecodePhaseLegacyS2L)
    {
        if (m_secureDecoder)
        {
            CODECHAL_DECODE_CHK_STATUS_RETURN(m_secureDecoder->Execute(this));
        }

        CODECHAL_DECODE_CHK_STATUS_RETURN(SendPictureS2L());
    }
    else
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(SendPictureLongFormat());
    }

    return eStatus;
}

MOS_STATUS CodechalDecodeHevcG12::SendPictureS2L()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    if (m_enableSf2DmaSubmits)
    {
        m_osInterface->pfnSetPerfTag(
            m_osInterface,
            (uint16_t)(((CODECHAL_DECODE_MODE_HUC << 4) & 0xF0) | (m_perfType & 0xF)));
    }

    MOS_COMMAND_BUFFER primCmdBuffer;
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnGetCommandBuffer(m_osInterface, &primCmdBuffer, 0));

    MHW_MI_FORCE_WAKEUP_PARAMS forceWakeupParams;
    MOS_ZeroMemory(&forceWakeupParams, sizeof(MHW_MI_FORCE_WAKEUP_PARAMS));
    forceWakeupParams.bMFXPowerWellControl = false;
    forceWakeupParams.bMFXPowerWellControlMask = true;
    forceWakeupParams.bHEVCPowerWellControl = true;
    forceWakeupParams.bHEVCPowerWellControlMask = true;

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiForceWakeupCmd(
        &primCmdBuffer,
        &forceWakeupParams));

    CODECHAL_DECODE_CHK_STATUS_RETURN(SendPrologWithFrameTracking(
        &primCmdBuffer, true));

    PMOS_COMMAND_BUFFER cmdBufferInUse = &primCmdBuffer;
    MOS_COMMAND_BUFFER  scdryCmdBuffer;

    if (CodecHalDecodeScalabilityIsScalableMode(m_scalabilityState) && MOS_VE_SUPPORTED(m_osInterface))
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHalDecodeScalability_GetCmdBufferToUse_G12(
            m_scalabilityState,
            &scdryCmdBuffer,
            &cmdBufferInUse));

        if (cmdBufferInUse == &scdryCmdBuffer)
        {
            MHW_MI_FORCE_WAKEUP_PARAMS forceWakeupParams;
            MOS_ZeroMemory(&forceWakeupParams, sizeof(MHW_MI_FORCE_WAKEUP_PARAMS));
            forceWakeupParams.bMFXPowerWellControl = false;
            forceWakeupParams.bMFXPowerWellControlMask = true;
            forceWakeupParams.bHEVCPowerWellControl = true;
            forceWakeupParams.bHEVCPowerWellControlMask = true;
            CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiForceWakeupCmd(
                cmdBufferInUse,
                &forceWakeupParams));

            //send prolog at the start of a secondary cmd buffer
            CODECHAL_DECODE_CHK_STATUS_RETURN(SendPrologWithFrameTracking(cmdBufferInUse, false));
        }
    }

    if (CodecHalDecodeScalabilityIsVirtualTileMode(m_scalabilityState) && CodecHalDecodeScalability1stDecPhase(m_scalabilityState))
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHalDecodeScalability_InitSemaMemResources(m_scalabilityState, cmdBufferInUse));
    }

    CODECHAL_DECODE_CHK_STATUS_RETURN(AddPictureS2LCmds(cmdBufferInUse));

    m_osInterface->pfnReturnCommandBuffer(m_osInterface, &primCmdBuffer, 0);

    if (CodecHalDecodeScalabilityIsScalableMode(m_scalabilityState) && MOS_VE_SUPPORTED(m_osInterface))
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHalDecodeScalability_ReturnSdryCmdBuffer_G12(m_scalabilityState, &scdryCmdBuffer));
    }

    return eStatus;
}

MOS_STATUS CodechalDecodeHevcG12::InitPicLongFormatMhwParams()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    // Reset all pic Mhw Params
    auto pipeModeSelectParams =
        static_cast<PMHW_VDBOX_PIPE_MODE_SELECT_PARAMS_G12>(m_picMhwParams.PipeModeSelectParams);
    *pipeModeSelectParams = {};
    auto pipeBufAddrParams =
        static_cast<PMHW_VDBOX_PIPE_BUF_ADDR_PARAMS_G12>(m_picMhwParams.PipeBufAddrParams);
    *pipeBufAddrParams = {};
    auto hevcPicStateParams =
        static_cast<PMHW_VDBOX_HEVC_PIC_STATE_G12>(m_picMhwParams.HevcPicState);
    *hevcPicStateParams = {};

    CODECHAL_DECODE_CHK_STATUS_RETURN(CodechalDecodeHevc::InitPicLongFormatMhwParams());
   
    pipeModeSelectParams->bHEVCSeparateTileProgramming = m_isSeparateTileDecoding;

    if (CodecHalDecodeScalabilityIsScalableMode(m_scalabilityState))
    {
        CodecHalDecodeScalablity_DecPhaseToHwWorkMode_G12(
            pipeModeSelectParams->MultiEngineMode,
            pipeModeSelectParams->PipeWorkMode);

        if (m_isRealTile)
        {
            CodecHalDecodeScalablity_SetPhaseIndicator(
                pipeModeSelectParams->ucPhaseIndicator);
        }

        pipeBufAddrParams->presSliceStateStreamOutBuffer =
            &m_scalabilityState->resSliceStateStreamOutBuffer;
        pipeBufAddrParams->presMvUpRightColStoreBuffer =
            &m_scalabilityState->resMvUpRightColStoreBuffer;
        pipeBufAddrParams->presIntraPredUpRightColStoreBuffer =
            &m_scalabilityState->resIntraPredUpRightColStoreBuffer;
        pipeBufAddrParams->presIntraPredLeftReconColStoreBuffer =
            &m_scalabilityState->resIntraPredLeftReconColStoreBuffer;
        pipeBufAddrParams->presCABACSyntaxStreamOutBuffer =
            m_scalabilityState->presCABACStreamOutBuffer;
    }

    hevcPicStateParams->pHevcExtPicParams = m_hevcExtPicParams;
    hevcPicStateParams->pHevcSccPicParams = m_hevcSccPicParams;

    hevcPicStateParams->ucRecNotFilteredID = CodecHalDecodeIsSCCIBCMode(m_hevcSccPicParams) ? m_IBCRefIdx : 0;

    return eStatus;
}

MOS_STATUS CodechalDecodeHevcG12::AddPictureLongFormatCmds(
    PMOS_COMMAND_BUFFER             cmdBufferInUse,
    PIC_LONG_FORMAT_MHW_PARAMS      *picMhwParams)
{
    MOS_STATUS                              eStatus = MOS_STATUS_SUCCESS;
    MHW_MI_VD_CONTROL_STATE_PARAMS          vdCtrlParam;

    CODECHAL_DECODE_FUNCTION_ENTER;

    CODECHAL_DECODE_CHK_NULL_RETURN(cmdBufferInUse);
    CODECHAL_DECODE_CHK_NULL_RETURN(picMhwParams);

    // Send VD_CONTROL_STATE Pipe Initialization
    MOS_ZeroMemory(&vdCtrlParam, sizeof(MHW_MI_VD_CONTROL_STATE_PARAMS));
    vdCtrlParam.initialization = true;
    static_cast<MhwMiInterfaceG12*>(m_miInterface)->AddMiVdControlStateCmd(cmdBufferInUse, &vdCtrlParam);

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpPipeModeSelectCmd(
        cmdBufferInUse,
        picMhwParams->PipeModeSelectParams));

    if (CodecHalDecodeScalabilityIsBEPhaseG12(m_scalabilityState) ||
        m_isRealTile)
    {
        // Send VD_CONTROL_STATE HcpPipeLock
        MOS_ZeroMemory(&vdCtrlParam, sizeof(MHW_MI_VD_CONTROL_STATE_PARAMS));
        vdCtrlParam.scalableModePipeLock = true;
        static_cast<MhwMiInterfaceG12*>(m_miInterface)->AddMiVdControlStateCmd(cmdBufferInUse, &vdCtrlParam);
    }

#ifdef _DECODE_PROCESSING_SUPPORTED
    if (!CodecHalDecodeScalabilityIsFEPhase(m_scalabilityState))
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_sfcState->AddSfcCommands(cmdBufferInUse));
    }
#endif

#ifdef _MMC_SUPPORTED
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_mmc->SetSurfaceState(picMhwParams->SurfaceParams));
#endif

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpSurfaceCmd(
        cmdBufferInUse,
        picMhwParams->SurfaceParams));

    // Let ref always use the same state (including MMC) as decode
    picMhwParams->SurfaceParams->ucSurfaceStateId = CODECHAL_HCP_REF_SURFACE_ID;
#ifdef _MMC_SUPPORTED
    if (CodecHalDecodeIsSCCIBCMode(m_hevcSccPicParams))
    {
        uint8_t skipMask = 0;
        for (uint8_t i = 0; i < CODECHAL_MAX_CUR_NUM_REF_FRAME_HEVC; i++)
        {
            if (picMhwParams->PipeBufAddrParams->presReferences[i] == m_presReferences[m_IBCRefIdx])
            {
                skipMask |= (1 << i);
            }
        }
        picMhwParams->SurfaceParams->mmcSkipMask = skipMask;
        CODECHAL_DECODE_NORMALMESSAGE("IBC ref index %d, MMC skip mask %d,", m_IBCRefIdx, skipMask);
    }

    if (MEDIA_IS_WA(m_waTable, WaDummyReference))
    {
        uint8_t skipMask = 0;
        for (uint8_t i = 0; i < CODECHAL_MAX_CUR_NUM_REF_FRAME_HEVC; i++)
        {
            if (m_dummyReferenceSlot[i])
            {
                skipMask |= (1 << i);
            }
        }
        picMhwParams->SurfaceParams->mmcSkipMask |= skipMask;
    }
#endif
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpSurfaceCmd(
        cmdBufferInUse,
        picMhwParams->SurfaceParams));

    if (CodecHalDecodeIsSCCIBCMode(m_hevcSccPicParams))
    {
        uint8_t refIdxMask = 0;
        for (uint8_t i = 0; i < CODECHAL_MAX_CUR_NUM_REF_FRAME_HEVC; i++)
        {
            if (picMhwParams->PipeBufAddrParams->presReferences[i] == m_presReferences[m_IBCRefIdx])
            {
                refIdxMask |= (1 << i);
            }
        }
        picMhwParams->PipeBufAddrParams->IBCRefIdxMask = refIdxMask;
    }

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpPipeBufAddrCmd(
        cmdBufferInUse,
        picMhwParams->PipeBufAddrParams));

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpIndObjBaseAddrCmd(
        cmdBufferInUse,
        picMhwParams->IndObjBaseAddrParams));

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpQmStateCmd(
        cmdBufferInUse,
        picMhwParams->QmParams));
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpPicStateCmd(
        cmdBufferInUse,
        picMhwParams->HevcPicState));

    if (m_hevcPicParams->tiles_enabled_flag)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpTileStateCmd(
            cmdBufferInUse,
            picMhwParams->HevcTileState));
    }

    return eStatus;
}

MOS_STATUS CodechalDecodeHevcG12::SendPictureLongFormat()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    if (m_enableSf2DmaSubmits)
    {
        m_osInterface->pfnSetPerfTag(
            m_osInterface,
            (uint16_t)(((CODECHAL_DECODE_MODE_HEVCVLD << 4) & 0xF0) | (m_perfType & 0xF)));
    }

    MOS_COMMAND_BUFFER primCmdBuffer;
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnGetCommandBuffer(m_osInterface, &primCmdBuffer, 0));

    bool sendPrologWithFrameTracking = false;
    CODECHAL_DECODE_CHK_STATUS_RETURN(DetermineSendProlgwithFrmTracking(&sendPrologWithFrameTracking));

    if (sendPrologWithFrameTracking)
    {
        MHW_MI_FORCE_WAKEUP_PARAMS forceWakeupParams;
        MOS_ZeroMemory(&forceWakeupParams, sizeof(MHW_MI_FORCE_WAKEUP_PARAMS));
        forceWakeupParams.bMFXPowerWellControl = false;
        forceWakeupParams.bMFXPowerWellControlMask = true;
        forceWakeupParams.bHEVCPowerWellControl = true;
        forceWakeupParams.bHEVCPowerWellControlMask = true;
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiForceWakeupCmd(
            &primCmdBuffer,
            &forceWakeupParams));

        //Frame tracking functionality is called at the start of a command buffer.
        CODECHAL_DECODE_CHK_STATUS_RETURN(SendPrologWithFrameTracking(
            &primCmdBuffer, true));
    }

    PMOS_COMMAND_BUFFER cmdBufferInUse = &primCmdBuffer;
    MOS_COMMAND_BUFFER  scdryCmdBuffer;
    auto                mmioRegisters = m_hwInterface->GetMfxInterface()->GetMmioRegisters(m_vdboxIndex);

    if (CodecHalDecodeScalabilityIsScalableMode(m_scalabilityState) && MOS_VE_SUPPORTED(m_osInterface))
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHalDecodeScalability_GetCmdBufferToUse_G12(
            m_scalabilityState,
            &scdryCmdBuffer,
            &cmdBufferInUse));

        if ((!m_shortFormatInUse && !CodecHalDecodeScalabilityIsFESeparateSubmission(m_scalabilityState) &&
                !m_isRealTile) ||
            CodecHalDecodeScalabilityIsBEPhaseG12(m_scalabilityState) ||
            CodecHalDecodeScalabilityIsFirstRealTilePhase(m_scalabilityState))
        {
            MHW_MI_FORCE_WAKEUP_PARAMS forceWakeupParams;
            MOS_ZeroMemory(&forceWakeupParams, sizeof(MHW_MI_FORCE_WAKEUP_PARAMS));
            forceWakeupParams.bMFXPowerWellControl = false;
            forceWakeupParams.bMFXPowerWellControlMask = true;
            forceWakeupParams.bHEVCPowerWellControl = true;
            forceWakeupParams.bHEVCPowerWellControlMask = true;
            CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiForceWakeupCmd(
                cmdBufferInUse,
                &forceWakeupParams));

            //send prolog at the start of a secondary cmd buffer
            CODECHAL_DECODE_CHK_STATUS_RETURN(SendPrologWithFrameTracking(cmdBufferInUse, false));
        }

        HalOcaInterface::On1stLevelBBStart(scdryCmdBuffer, *m_osInterface->pOsContext, m_osInterface->CurrentGpuContextHandle, *m_miInterface, *mmioRegisters);
    }
    else
    {
        HalOcaInterface::On1stLevelBBStart(primCmdBuffer, *m_osInterface->pOsContext, m_osInterface->CurrentGpuContextHandle, *m_miInterface, *mmioRegisters);
    }

    CODECHAL_DECODE_CHK_STATUS_RETURN(InitPicLongFormatMhwParams());

    CODECHAL_DEBUG_TOOL(
        for (int32_t n = 0; n < CODECHAL_MAX_CUR_NUM_REF_FRAME_HEVC; n++)
        {
            if (m_picMhwParams.PipeBufAddrParams->presReferences[n])
            {
                MOS_SURFACE dstSurface;

                MOS_ZeroMemory(&dstSurface, sizeof(MOS_SURFACE));
                dstSurface.OsResource = *(m_picMhwParams.PipeBufAddrParams->presReferences[n]);
                CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHalGetResourceInfo(
                    m_osInterface,
                    &dstSurface));

                m_debugInterface->m_refIndex = (uint16_t)n;
                CODECHAL_DECODE_CHK_STATUS_RETURN(m_debugInterface->DumpYUVSurface(
                    &dstSurface,
                    CodechalDbgAttr::attrReferenceSurfaces,
                    "RefSurf"));
            }

            if (m_picMhwParams.PipeBufAddrParams->presColMvTempBuffer[n])
            {
                m_debugInterface->m_refIndex = (uint16_t)n;
                // dump mvdata
                CODECHAL_DECODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
                    m_picMhwParams.PipeBufAddrParams->presColMvTempBuffer[n],
                    CodechalDbgAttr::attrMvData,
                    "_DEC",
                    m_mvBufferSize));
            }
        }
    );

    if (CodecHalDecodeScalabilityIsVirtualTileMode(m_scalabilityState) && CodecHalDecodeScalability1stDecPhase(m_scalabilityState))
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHalDecodeScalability_InitSemaMemResources(m_scalabilityState, cmdBufferInUse));
    }

    //Send status report Start
    if (m_statusQueryReportingEnabled)
    {
        bool sendStatusReportStart = true;
        if (CodecHalDecodeScalabilityIsScalableMode(m_scalabilityState))
        {
            sendStatusReportStart = CodecHalDecodeScalabilityIsFEPhase(m_scalabilityState) || m_scalabilityState->bIsRtMode;
        }
        if (sendStatusReportStart)
        {
            CODECHAL_DECODE_CHK_STATUS_RETURN(StartStatusReport(cmdBufferInUse));
        }
    }

    if (m_shortFormatInUse &&
        m_statusQueryReportingEnabled &&
        (!CodecHalDecodeScalabilityIsScalableMode(m_scalabilityState) ||
            CodecHalDecodeScalabilityIsFEPhase(m_scalabilityState)))
    {
        uint32_t statusBufferOffset = (m_decodeStatusBuf.m_currIndex * sizeof(CodechalDecodeStatus)) +
                        m_decodeStatusBuf.m_storeDataOffset +
                        sizeof(uint32_t) * 2;

        // Check HuC_STATUS bit15, HW continue if bit15 > 0, otherwise send COND BB END cmd.
        CODECHAL_DECODE_CHK_STATUS_RETURN(static_cast<CodechalHwInterfaceG12*>(m_hwInterface)->SendCondBbEndCmd(
                &m_decodeStatusBuf.m_statusBuffer,
                statusBufferOffset + m_decodeStatusBuf.m_hucErrorStatusMaskOffset,
                0,
                false,
                false,
                mhw_mi_g12_X::MI_CONDITIONAL_BATCH_BUFFER_END_CMD::COMPARE_OPERATION_MADGREATERTHANIDD,
                cmdBufferInUse));
    }


    if (CodecHalDecodeScalabilityIsScalableMode(m_scalabilityState))
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHalDecodeScalability_FEBESync_G12(
            m_scalabilityState,
            cmdBufferInUse,
            m_osInterface->phasedSubmission));
        if (m_perfFEBETimingEnabled && CodecHalDecodeScalabilityIsLastCompletePhase(m_scalabilityState))
        {
            CODECHAL_DECODE_CHK_STATUS_RETURN(m_perfProfiler->AddPerfCollectStartCmd((void *)this, m_osInterface, m_miInterface, &scdryCmdBuffer));
        }
    }

    if (CodecHalDecodeScalabilityIsBEPhaseG12(m_scalabilityState) || CodecHalDecodeScalabilityIsFirstRealTilePhase(m_scalabilityState))
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddWatchdogTimerStartCmd(cmdBufferInUse));
    }

    CODECHAL_DECODE_CHK_STATUS_RETURN(AddPictureLongFormatCmds(cmdBufferInUse, &m_picMhwParams));

    m_osInterface->pfnReturnCommandBuffer(m_osInterface, &primCmdBuffer, 0);
    if (CodecHalDecodeScalabilityIsScalableMode(m_scalabilityState) && MOS_VE_SUPPORTED(m_osInterface))
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHalDecodeScalability_ReturnSdryCmdBuffer_G12(m_scalabilityState, &scdryCmdBuffer));
    }

    return eStatus;
}

MOS_STATUS CodechalDecodeHevcG12::AddPipeEpilog(
    PMOS_COMMAND_BUFFER cmdBufferInUse,
    MOS_COMMAND_BUFFER &scdryCmdBuffer)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_MI_FLUSH_DW_PARAMS flushDwParams;
    MOS_ZeroMemory(&flushDwParams, sizeof(flushDwParams));
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiFlushDwCmd(
        cmdBufferInUse,
        &flushDwParams));

    if (CodecHalDecodeScalabilityIsFEPhase(m_scalabilityState))
    {
        if (m_scalabilityState->bIsEnableEndCurrentBatchBuffLevel)
        {
            CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHalDecodeScalablity_GetFEReportedCabacStreamoutBufferSize(
                m_scalabilityState,
                cmdBufferInUse));
        }
        else
        {
            CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHalDecodeScalablity_SetFECabacStreamoutOverflowStatus(
                m_scalabilityState,
                cmdBufferInUse));
        }

        CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHalDecodeScalability_SignalFE2BESemaphore(
            m_scalabilityState,
            cmdBufferInUse));

        if (m_perfFEBETimingEnabled)
        {
            CODECHAL_DECODE_CHK_STATUS_RETURN(m_perfProfiler->AddPerfCollectEndCmd((void *)this, m_osInterface, m_miInterface, &scdryCmdBuffer));
        }
    }

    //Sync for decode completion in scalable mode
    if (CodecHalDecodeScalabilityIsBEPhaseG12(m_scalabilityState))
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHalDecodeScalability_BEsCompletionSync(
            m_scalabilityState,
            cmdBufferInUse));
    }

    bool syncDestSurface = true;
    //if scalable decode,  BE0 finish means whole frame complete.
    // Check if destination surface needs to be synchronized
    if (CodecHalDecodeScalabilityIsScalableMode(m_scalabilityState))
    {
        syncDestSurface = CodecHalDecodeScalabilityIsLastCompletePhase(m_scalabilityState) ||
                          CodecHalDecodeScalabilityIsLastRealTilePass(m_scalabilityState);
    }

    if (syncDestSurface)
    {
        MOS_SYNC_PARAMS syncParams = g_cInitSyncParams;
        syncParams.GpuContext               = m_videoContext;
        syncParams.presSyncResource         = &m_destSurface.OsResource;
        syncParams.bReadOnly                = false;
        syncParams.bDisableDecodeSyncLock   = m_disableDecodeSyncLock;
        syncParams.bDisableLockForTranscode = m_disableLockForTranscode;

        if (!CodecHal_PictureIsField(m_hevcPicParams->CurrPic))
        {
            CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnPerformOverlaySync(m_osInterface, &syncParams));
            CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnResourceWait(m_osInterface, &syncParams));

            // Update the resource tag (s/w tag) for On-Demand Sync
            m_osInterface->pfnSetResourceSyncTag(m_osInterface, &syncParams);
        }

        // Update the tag in GPU Sync eStatus buffer (H/W Tag) to match the current S/W tag
        if (m_osInterface->bTagResourceSync)
        {
            CODECHAL_DECODE_CHK_STATUS_RETURN(m_hwInterface->WriteSyncTagToResource(
                cmdBufferInUse,
                &syncParams));
        }

        if (m_statusQueryReportingEnabled)
        {
            CodechalDecodeStatusReport decodeStatusReport;
            MOS_ZeroMemory(&decodeStatusReport, sizeof(decodeStatusReport));

            decodeStatusReport.m_statusReportNumber = m_statusReportFeedbackNumber;
            decodeStatusReport.m_currDecodedPic     = m_hevcPicParams->CurrPic;
            decodeStatusReport.m_currDeblockedPic   = m_hevcPicParams->CurrPic;
            decodeStatusReport.m_codecStatus        = CODECHAL_STATUS_UNAVAILABLE;
            decodeStatusReport.m_currDecodedPicRes  = m_hevcRefList[m_hevcPicParams->CurrPic.FrameIdx]->resRefPic;

#ifdef _DECODE_PROCESSING_SUPPORTED
            CODECHAL_DEBUG_TOOL(
                if (m_downsampledSurfaces && m_sfcState && m_sfcState->m_sfcOutputSurface) {
                    m_downsampledSurfaces[m_hevcPicParams->CurrPic.FrameIdx].OsResource = m_sfcState->m_sfcOutputSurface->OsResource;
                    decodeStatusReport.m_currSfcOutputPicRes                            = &m_downsampledSurfaces[m_hevcPicParams->CurrPic.FrameIdx].OsResource;
                })
#endif
            CODECHAL_DEBUG_TOOL(
                decodeStatusReport.m_secondField = CodecHal_PictureIsBottomField(m_hevcPicParams->CurrPic);
                decodeStatusReport.m_frameType   = m_perfType;);

            CODECHAL_DECODE_CHK_STATUS_RETURN(EndStatusReport(
                decodeStatusReport,
                cmdBufferInUse));
        }
    }

    MOS_ZeroMemory(&flushDwParams, sizeof(flushDwParams));
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiFlushDwCmd(
        cmdBufferInUse,
        &flushDwParams));

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferEnd(
        cmdBufferInUse,
        nullptr));

    return eStatus;
}

MOS_STATUS CodechalDecodeHevcG12::SendShortSlices(PMOS_COMMAND_BUFFER cmdBuffer)
{
    CODECHAL_DECODE_CHK_NULL_RETURN(cmdBuffer);

    MHW_VDBOX_HEVC_SLICE_STATE_G12      hevcSliceState;
    hevcSliceState.presDataBuffer = m_copyDataBufferInUse ? &m_resCopyDataBuffer : &m_resDataBuffer;

    auto slc = m_hevcSliceParams;
    for (uint16_t i = 0; i < m_numSlices; i++, slc++)
    {
        hevcSliceState.pHevcSliceParams = slc;
        hevcSliceState.dwLength = slc->slice_data_size;
        hevcSliceState.dwSliceIndex = i;
        hevcSliceState.bLastSlice = (i == (m_numSlices - 1));

        CODECHAL_DECODE_CHK_STATUS_RETURN(SendSliceS2L(cmdBuffer, &hevcSliceState));
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalDecodeHevcG12::SendHucFlush(PMOS_COMMAND_BUFFER cmdBuffer,
    MOS_COMMAND_BUFFER  &primCmdBuffer,
    MOS_COMMAND_BUFFER  &scdryCmdBuffer,
    uint32_t            renderingFlags)
{
    MOS_STATUS          eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_CHK_NULL_RETURN(cmdBuffer);

    CODECHAL_DECODE_CHK_COND_RETURN(
        (m_vdboxIndex > m_mfxInterface->GetMaxVdboxIndex()),
        "ERROR - vdbox index exceed the maximum");
    auto mmioRegisters = m_hucInterface->GetMmioRegisters(m_vdboxIndex);

    uint32_t statusBufferOffset = (m_decodeStatusBuf.m_currIndex * sizeof(CodechalDecodeStatus)) +
        m_decodeStatusBuf.m_storeDataOffset +
        sizeof(uint32_t) * 2;

    // Send VD Pipe Flush command for SKL+
    MHW_VDBOX_VD_PIPE_FLUSH_PARAMS vdpipeFlushParams;
    MOS_ZeroMemory(&vdpipeFlushParams, sizeof(vdpipeFlushParams));
    vdpipeFlushParams.Flags.bWaitDoneHEVC = 1;
    vdpipeFlushParams.Flags.bFlushHEVC = 1;
    vdpipeFlushParams.Flags.bWaitDoneVDCmdMsgParser = 1;
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_vdencInterface->AddVdPipelineFlushCmd(
        cmdBuffer,
        &vdpipeFlushParams));

    MHW_MI_FLUSH_DW_PARAMS flushDwParams;
    MOS_ZeroMemory(&flushDwParams, sizeof(flushDwParams));
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiFlushDwCmd(
        cmdBuffer,
        &flushDwParams));

    if (m_statusQueryReportingEnabled)
    {
        // Check HuC_STATUS2 bit6, if bit6 > 0 HW continue execution following cmd, otherwise it send a COND BB END cmd.
        eStatus = static_cast<CodechalHwInterfaceG12*>(m_hwInterface)->SendCondBbEndCmd(
            &m_decodeStatusBuf.m_statusBuffer,
            statusBufferOffset + m_decodeStatusBuf.m_hucErrorStatus2MaskOffset,
            0,
            false,
            false,
            mhw_mi_g12_X::MI_CONDITIONAL_BATCH_BUFFER_END_CMD::COMPARE_OPERATION_MADGREATERTHANIDD,
            cmdBuffer);
        CODECHAL_DECODE_CHK_STATUS_RETURN(eStatus);

        // Write HUC_STATUS mask
        MHW_MI_STORE_DATA_PARAMS storeDataParams;
        MOS_ZeroMemory(&storeDataParams, sizeof(storeDataParams));
        storeDataParams.pOsResource = &m_decodeStatusBuf.m_statusBuffer;
        storeDataParams.dwResourceOffset = statusBufferOffset + m_decodeStatusBuf.m_hucErrorStatusMaskOffset;
        storeDataParams.dwValue = m_hucInterface->GetHucStatusHevcS2lFailureMask();
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiStoreDataImmCmd(
            cmdBuffer,
            &storeDataParams));

        // store HUC_STATUS register
        MHW_MI_STORE_REGISTER_MEM_PARAMS storeRegParams;
        MOS_ZeroMemory(&storeRegParams, sizeof(storeRegParams));
        storeRegParams.presStoreBuffer = &m_decodeStatusBuf.m_statusBuffer;
        storeRegParams.dwOffset = statusBufferOffset + m_decodeStatusBuf.m_hucErrorStatusRegOffset;
        storeRegParams.dwRegister = mmioRegisters->hucStatusRegOffset;
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiStoreRegisterMemCmd(
            cmdBuffer,
            &storeRegParams));
    }

    if (m_enableSf2DmaSubmits)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferEnd(
            cmdBuffer,
            nullptr));
    }

    CODECHAL_DECODE_CHK_NULL_RETURN(m_osInterface);

    m_osInterface->pfnReturnCommandBuffer(m_osInterface, &primCmdBuffer, 0);
    if (CodecHalDecodeScalabilityIsScalableMode(m_scalabilityState) && MOS_VE_SUPPORTED(m_osInterface))
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHalDecodeScalability_ReturnSdryCmdBuffer_G12(m_scalabilityState, &scdryCmdBuffer));
    }

    if (m_enableSf2DmaSubmits)
    {
        CODECHAL_DEBUG_TOOL(
            CODECHAL_DECODE_CHK_STATUS_RETURN(m_debugInterface->DumpCmdBuffer(
                cmdBuffer,
                CODECHAL_NUM_MEDIA_STATES,
                "_DEC"));

        );

        CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnSubmitCommandBuffer(
            m_osInterface,
            cmdBuffer,
            renderingFlags));
    }

    return eStatus;
}

MOS_STATUS CodechalDecodeHevcG12::DecodePrimitiveLevel()
{
    MOS_STATUS                          eStatus = MOS_STATUS_SUCCESS;
    MHW_MI_VD_CONTROL_STATE_PARAMS      vdCtrlParam;

    CODECHAL_DECODE_FUNCTION_ENTER;

    // Bitstream is incomplete, don't do any decoding work.
    if (m_incompletePicture)
    {
        return MOS_STATUS_SUCCESS;
    }

    uint32_t renderingFlags = m_videoContextUsesNullHw;

    MOS_COMMAND_BUFFER primCmdBuffer;
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnGetCommandBuffer(m_osInterface, &primCmdBuffer, 0));

    PMOS_COMMAND_BUFFER cmdBufferInUse = &primCmdBuffer;
    MOS_COMMAND_BUFFER  scdryCmdBuffer;
    if (CodecHalDecodeScalabilityIsScalableMode(m_scalabilityState) && MOS_VE_SUPPORTED(m_osInterface))
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHalDecodeScalability_GetCmdBufferToUse_G12(
            m_scalabilityState,
            &scdryCmdBuffer,
            &cmdBufferInUse));
    }

    if (m_shortFormatInUse && m_hcpDecPhase == CodechalHcpDecodePhaseLegacyS2L)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(SendShortSlices(cmdBufferInUse));
        return SendHucFlush(cmdBufferInUse, primCmdBuffer, scdryCmdBuffer, renderingFlags);
    }
    else if (CodecHalDecodeScalabilityIsBEPhaseG12(m_scalabilityState))
    {
        MHW_VDBOX_HCP_TILE_CODING_PARAMS_G12 hcpTileCodingParam;
        CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHalDecodeScalability_CalculateHcpTileCodingParams<MHW_VDBOX_HCP_TILE_CODING_PARAMS_G12>(
            m_scalabilityState,
            m_hevcPicParams,
            &hcpTileCodingParam));
        CODECHAL_DECODE_CHK_STATUS_RETURN(static_cast<MhwVdboxHcpInterfaceG12*>(m_hcpInterface)->AddHcpTileCodingCmd(
            cmdBufferInUse,
            &hcpTileCodingParam));
    }
    else if (m_cencBuf)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(SetCencBatchBuffer(cmdBufferInUse));
    }
    else
    {
        if (m_isRealTile)
        {
            uint8_t col = 0;
            CodecHalDecodeScalability_GetCurrentRealTileColumnId(m_scalabilityState, col);
            CODECHAL_DECODE_ASSERT(col < m_secondLevelBatchBuffer[m_secondLevelBatchBufferIndex].count);
            m_secondLevelBatchBuffer[m_secondLevelBatchBufferIndex].dwOffset = col * m_secondLevelBatchBuffer[m_secondLevelBatchBufferIndex].iSize;
        }
        else
        {
            m_secondLevelBatchBuffer[m_secondLevelBatchBufferIndex].dwOffset = 0;
        }

        CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferStartCmd(
            cmdBufferInUse,
            &m_secondLevelBatchBuffer[m_secondLevelBatchBufferIndex]));
    }

    // Send VD_CONTROL_STATE Memory Implict Flush
    MOS_ZeroMemory(&vdCtrlParam, sizeof(MHW_MI_VD_CONTROL_STATE_PARAMS));
    vdCtrlParam.memoryImplicitFlush = true;
    static_cast<MhwMiInterfaceG12*>(m_miInterface)->AddMiVdControlStateCmd(cmdBufferInUse, &vdCtrlParam);

    if (CodecHalDecodeScalabilityIsBEPhaseG12(m_scalabilityState) ||
        m_isRealTile)
    {
        // Send VD_CONTROL_STATE HCP Pipe Unlock
        MOS_ZeroMemory(&vdCtrlParam, sizeof(MHW_MI_VD_CONTROL_STATE_PARAMS));
        vdCtrlParam.scalableModePipeUnlock = true;
        static_cast<MhwMiInterfaceG12*>(m_miInterface)->AddMiVdControlStateCmd(cmdBufferInUse, &vdCtrlParam);
    }

    if (m_isRealTile)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMfxWaitCmd(cmdBufferInUse, nullptr, true));
    }

    // store CS ENGINE ID register
    if (static_cast<MhwVdboxMfxInterfaceG12*>(m_mfxInterface)->IsScalabilitySupported())
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHalDecodeScalability_ReadCSEngineIDReg_G12(
            m_scalabilityState,
            &m_decodeStatusBuf,
            cmdBufferInUse));
    }

    // Send VD Pipe Flush command for SKL+
    MHW_VDBOX_VD_PIPE_FLUSH_PARAMS vdpipeFlushParams;
    MOS_ZeroMemory(&vdpipeFlushParams, sizeof(vdpipeFlushParams));
    vdpipeFlushParams.Flags.bWaitDoneHEVC = 1;
    vdpipeFlushParams.Flags.bFlushHEVC = 1;
    vdpipeFlushParams.Flags.bWaitDoneVDCmdMsgParser = 1;
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_vdencInterface->AddVdPipelineFlushCmd(
        cmdBufferInUse,
        &vdpipeFlushParams));

    // Needs to be re-set for Linux buffer re-use scenarios
    if (CodecHalDecodeScalabilityIsLastRealTilePass(m_scalabilityState) || !CodecHalDecodeScalabilityIsScalableMode(m_scalabilityState))
    {
        m_hevcRefList[m_hevcPicParams->CurrPic.FrameIdx]->resRefPic =
            m_destSurface.OsResource;
    }

    if (!m_isRealTile ||
        CodecHalDecodeScalabilityIsLastRealTilePhase(m_scalabilityState))
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(AddPipeEpilog(cmdBufferInUse, scdryCmdBuffer));
    }

    m_osInterface->pfnReturnCommandBuffer(m_osInterface, &primCmdBuffer, 0);
    if (CodecHalDecodeScalabilityIsScalableMode(m_scalabilityState) && MOS_VE_SUPPORTED(m_osInterface))
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHalDecodeScalability_ReturnSdryCmdBuffer_G12(m_scalabilityState, &scdryCmdBuffer));
    }

    bool syncCompleteFrame = m_copyDataBufferInUse;
    if (MOS_VE_SUPPORTED(m_osInterface) && CodecHalDecodeScalabilityIsScalableMode(m_scalabilityState))
    {
        syncCompleteFrame = syncCompleteFrame && CodecHalDecodeScalabilityIsFEPhase(m_scalabilityState);
    }

    if (syncCompleteFrame)
    {
        //Sync up complete frame
        MOS_SYNC_PARAMS copyDataSyncParams = g_cInitSyncParams;
        copyDataSyncParams.GpuContext = m_videoContextForWa;
        copyDataSyncParams.presSyncResource = &m_resSyncObjectWaContextInUse;

        CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnEngineSignal(m_osInterface, &copyDataSyncParams));

        copyDataSyncParams = g_cInitSyncParams;
        copyDataSyncParams.GpuContext = m_videoContext;
        copyDataSyncParams.presSyncResource = &m_resSyncObjectWaContextInUse;

        CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnEngineWait(m_osInterface, &copyDataSyncParams));
    }

    CODECHAL_DEBUG_TOOL(
        {
            if (CodecHalDecodeScalabilityIsScalableMode(m_scalabilityState))
            {
                CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHalDecodeScalability_DbgDumpCmdBuffer_G12(
                    this,
                    m_scalabilityState,
                    m_debugInterface,
                    &primCmdBuffer));
            }
            else
            {
                CODECHAL_DECODE_CHK_STATUS_RETURN(m_debugInterface->DumpCmdBuffer(
                    &primCmdBuffer,
                    CODECHAL_NUM_MEDIA_STATES,
                    "_DEC"));
            }
        });

    bool submitCommand = true;
    //submit command buffer
    if (MOS_VE_SUPPORTED(m_osInterface) && CodecHalDecodeScalabilityIsScalableMode(m_scalabilityState))
    {
        submitCommand = CodecHalDecodeScalabilityIsToSubmitCmdBuffer_G12(m_scalabilityState);
        HalOcaInterface::On1stLevelBBEnd(scdryCmdBuffer, *m_osInterface->pOsContext);
    }
    else
    {
        HalOcaInterface::On1stLevelBBEnd(primCmdBuffer, *m_osInterface->pOsContext);
    }

    if (submitCommand || m_osInterface->phasedSubmission)
    {
        //command buffer to submit is the primary cmd buffer.
        if ( MOS_VE_SUPPORTED(m_osInterface))
        {
            CODECHAL_DECODE_CHK_STATUS_RETURN(SetAndPopulateVEHintParams(&primCmdBuffer));
        }

        if(m_osInterface->phasedSubmission
           && MOS_VE_SUPPORTED(m_osInterface)
           && CodecHalDecodeScalabilityIsScalableMode(m_scalabilityState))
        {
            CodecHalDecodeScalability_DecPhaseToSubmissionType_G12(m_scalabilityState,cmdBufferInUse);
            CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnSubmitCommandBuffer(m_osInterface, cmdBufferInUse, renderingFlags));
        }
        else
        {
            CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnSubmitCommandBuffer(m_osInterface, &primCmdBuffer, renderingFlags));
        }
    }

    // If S2L and 2nd pass, jump to 2nd level batch buffer
    if ((!Mos_ResourceIsNull(&m_secondLevelBatchBuffer[m_secondLevelBatchBufferIndex].OsResource)) 
          && m_secondLevelBatchBuffer[m_secondLevelBatchBufferIndex].dwOffset == 0)
    {
#if (_DEBUG || _RELEASE_INTERNAL)
        m_secondLevelBatchBuffer[m_secondLevelBatchBufferIndex].iLastCurrent = m_secondLevelBatchBuffer[m_secondLevelBatchBufferIndex].iSize * m_secondLevelBatchBuffer[m_secondLevelBatchBufferIndex].count;
#endif

        CODECHAL_DEBUG_TOOL(
            CODECHAL_DECODE_CHK_STATUS_RETURN(m_debugInterface->Dump2ndLvlBatch(
                &m_secondLevelBatchBuffer[m_secondLevelBatchBufferIndex],
                CODECHAL_NUM_MEDIA_STATES,
                "_DEC"));)
    }

    CODECHAL_DEBUG_TOOL(
        m_mmc->UpdateUserFeatureKey(&m_destSurface);

        if (m_histogramDebug && m_histogramSurface)
        {
            CODECHAL_DECODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
                &m_histogramSurface->OsResource,
                CodechalDbgAttr::attrSfcHistogram,
                "_DEC",
                256 * 4));
        })

    // Reset status report
    if (m_statusQueryReportingEnabled)
    {
        bool resetStatusReport = true;

        //if scalable decode,  reset status report at final BE phase.
        if (CodecHalDecodeScalabilityIsScalableMode(m_scalabilityState))
        {
            resetStatusReport = CodecHalDecodeScalabilityIsFinalBEPhaseG12(m_scalabilityState);
        }

        if (resetStatusReport)
        {
            CODECHAL_DECODE_CHK_STATUS_RETURN(ResetStatusReport(
                m_videoContextUsesNullHw));
        }
    }

    // Send the signal to indicate decode completion, in case On-Demand Sync is not present
    if (!CodecHal_PictureIsField(m_hevcPicParams->CurrPic))
    {
        MOS_SYNC_PARAMS syncParams      = g_cInitSyncParams;
        syncParams.GpuContext           = m_videoContext;
        syncParams.presSyncResource     = &m_destSurface.OsResource;

        CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnResourceSignal(m_osInterface, &syncParams));
    }

    // Scalability real tile, move to next tile column
    if (m_isRealTile)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHalDecodeScalability_AdvanceRealTilePass(m_scalabilityState));
    }

#ifdef LINUX 
 #ifdef _DECODE_PROCESSING_SUPPORTED
    CODECHAL_DEBUG_TOOL(
    if (m_sfcState->m_sfcOutputSurface)
    {
        MOS_SURFACE dstSurface;
        MOS_ZeroMemory(&dstSurface, sizeof(dstSurface));
        dstSurface.Format = Format_NV12;
        dstSurface.OsResource = m_sfcState->m_sfcOutputSurface->OsResource;
        CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHalGetResourceInfo(
                m_osInterface,
                &dstSurface));

        CODECHAL_DECODE_CHK_STATUS_RETURN(m_debugInterface->DumpYUVSurface(
                &dstSurface,
                CodechalDbgAttr::attrSfcOutputSurface,
                "SfcDstSurf"));
    }
)
#endif
#endif
    return eStatus;
}

MOS_STATUS CodechalDecodeHevcG12::InitMmcState()
{
#ifdef _MMC_SUPPORTED
    m_mmc = MOS_New(CodechalMmcDecodeHevcG12, m_hwInterface, this);
    CODECHAL_DECODE_CHK_NULL_RETURN(m_mmc);
#endif
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalDecodeHevcG12::AllocateStandard (
    CodechalSetting *          settings)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    CODECHAL_DECODE_CHK_NULL_RETURN(settings);

    CODECHAL_DECODE_CHK_STATUS_RETURN(InitMmcState());

    m_width                         = settings->width;
    m_height                        = settings->height;
    m_is10BitHevc                   = (settings->lumaChromaDepth & CODECHAL_LUMA_CHROMA_DEPTH_10_BITS) ? true : false;
    m_is12BitHevc                   = (settings->lumaChromaDepth & CODECHAL_LUMA_CHROMA_DEPTH_12_BITS) ? true : false;
    m_chromaFormatinProfile         = settings->chromaFormat;
    m_shortFormatInUse              = settings->shortFormatInUse;

#ifdef _DECODE_PROCESSING_SUPPORTED
    m_sfcState = MOS_New(CodechalHevcSfcStateG12);
    CODECHAL_DECODE_CHK_NULL_RETURN(m_sfcState);
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_sfcState->InitializeSfcState(
        this,
        m_hwInterface,
        m_osInterface));
#endif
    MOS_ZeroMemory(&m_currPic, sizeof(m_currPic));

    m_frameIdx = 0;

    if (m_shortFormatInUse)
    {
        // Legacy SF has 2 passes, 1st pass is S2L, 2nd pass is HEVC Long decode
        // The pass number will be changed again if is scalable decode mode.
        m_decodePassNum = 2;

        MOS_USER_FEATURE_VALUE_DATA userFeatureData;
        MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
        MOS_UserFeature_ReadValue_ID(
            nullptr,
            __MEDIA_USER_FEATURE_VALUE_HEVC_SF_2_DMA_SUBMITS_ENABLE_ID,
            &userFeatureData);
        m_enableSf2DmaSubmits = userFeatureData.u32Data ? true : false;
    }

    MHW_VDBOX_STATE_CMDSIZE_PARAMS_G12 stateCmdSizeParams;
    stateCmdSizeParams.bShortFormat    = m_shortFormatInUse;
    stateCmdSizeParams.bHucDummyStream = (m_secureDecoder ? m_secureDecoder->IsDummyStreamEnabled() : false);
    stateCmdSizeParams.bScalableMode   = static_cast<MhwVdboxMfxInterfaceG12*>(m_mfxInterface)->IsScalabilitySupported();
    stateCmdSizeParams.bSfcInUse       = true;

    // Picture Level Commands
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_hwInterface->GetHxxStateCommandSize(
        m_mode,
        &m_commandBufferSizeNeeded,
        &m_commandPatchListSizeNeeded,
        &stateCmdSizeParams));

    // Primitive Level Commands
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_hwInterface->GetHxxPrimitiveCommandSize(
        m_mode,
        &m_standardDecodeSizeNeeded,
        &m_standardDecodePatchListSizeNeeded,
        m_shortFormatInUse));

    if ( MOS_VE_SUPPORTED(m_osInterface))
    {
        if (static_cast<MhwVdboxMfxInterfaceG12*>(m_mfxInterface)->IsScalabilitySupported())
        {
            m_scalabilityState = (PCODECHAL_DECODE_SCALABILITY_STATE_G12)MOS_AllocAndZeroMemory(sizeof(CODECHAL_DECODE_SCALABILITY_STATE_G12));
            CODECHAL_DECODE_CHK_NULL_RETURN(m_scalabilityState);
            //scalability initialize
            CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHalDecodeScalability_InitializeState_G12(
                this,
                m_scalabilityState,
                m_hwInterface,
                m_shortFormatInUse));
        }
        else
        {
            //single pipe VE initialize
            m_sinlgePipeVeState = (PCODECHAL_DECODE_SINGLEPIPE_VIRTUALENGINE_STATE)MOS_AllocAndZeroMemory(sizeof(CODECHAL_DECODE_SINGLEPIPE_VIRTUALENGINE_STATE));
            CODECHAL_DECODE_CHK_NULL_RETURN(m_sinlgePipeVeState);
            CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHalDecodeSinglePipeVE_InitInterface(m_osInterface, m_sinlgePipeVeState));
        }
    }

    CODECHAL_DECODE_CHK_STATUS_RETURN(CodechalDecodeHevc::AllocateResourcesFixedSizes());

    // Prepare Pic Params
    m_picMhwParams.PipeModeSelectParams = MOS_New(MHW_VDBOX_PIPE_MODE_SELECT_PARAMS_G12);
    m_picMhwParams.SurfaceParams        = MOS_New(MHW_VDBOX_SURFACE_PARAMS);
    m_picMhwParams.PipeBufAddrParams    = MOS_New(MHW_VDBOX_PIPE_BUF_ADDR_PARAMS_G12);
    m_picMhwParams.IndObjBaseAddrParams = MOS_New(MHW_VDBOX_IND_OBJ_BASE_ADDR_PARAMS);
    m_picMhwParams.QmParams             = MOS_New(MHW_VDBOX_QM_PARAMS);
    m_picMhwParams.HevcPicState         = MOS_New(MHW_VDBOX_HEVC_PIC_STATE_G12);
    m_picMhwParams.HevcTileState        = MOS_New(MHW_VDBOX_HEVC_TILE_STATE);

    MOS_ZeroMemory(m_picMhwParams.SurfaceParams, sizeof(MHW_VDBOX_SURFACE_PARAMS));
    MOS_ZeroMemory(m_picMhwParams.IndObjBaseAddrParams, sizeof(MHW_VDBOX_IND_OBJ_BASE_ADDR_PARAMS));
    MOS_ZeroMemory(m_picMhwParams.QmParams, sizeof(MHW_VDBOX_QM_PARAMS));
    MOS_ZeroMemory(m_picMhwParams.HevcTileState, sizeof(MHW_VDBOX_HEVC_TILE_STATE));

    return eStatus;
}

CodechalDecodeHevcG12::CodechalDecodeHevcG12(
    CodechalHwInterface *   hwInterface,
    CodechalDebugInterface *debugInterface,
    PCODECHAL_STANDARD_INFO standardInfo) : CodechalDecodeHevc(hwInterface, debugInterface, standardInfo),
                                            m_hevcExtPicParams(nullptr),
                                            m_hevcExtSliceParams(nullptr),
                                            m_hevcSccPicParams(nullptr),
                                            m_hevcSubsetParams(nullptr),
                                            m_ctbSize(0),
                                            m_frameSizeMaxAlloced(0),
                                            m_twoVersionsOfCurrDecPicFlag(false),
                                            m_IBCRefIdx(0),

#if (_DEBUG || _RELEASE_INTERNAL)
                                            m_rtFrameCount(0),
                                            m_vtFrameCount(0),
                                            m_spFrameCount(0),
#endif
                                            m_sinlgePipeVeState(nullptr),
                                            m_scalabilityState(nullptr)
{
    CODECHAL_DECODE_FUNCTION_ENTER;

    MOS_ZeroMemory(&m_resRefBeforeLoopFilter, sizeof(m_resRefBeforeLoopFilter));

    CODECHAL_DECODE_CHK_NULL_NO_STATUS_RETURN(m_osInterface);

    Mos_CheckVirtualEngineSupported(m_osInterface, true, true);

#if (_DEBUG || _RELEASE_INTERNAL)
    MOS_USER_FEATURE_VALUE_DATA userFeatureData;
    MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
    MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_HEVC_DECODE_HISTOGRAM_DEBUG_ID_G12,
        &userFeatureData);
    m_histogramDebug = userFeatureData.u32Data ? true : false;
#endif
}

#if USE_CODECHAL_DEBUG_TOOL
MOS_STATUS CodechalDecodeHevcG12::DumpPicParams(
    PCODEC_HEVC_PIC_PARAMS     picParams,
    PCODEC_HEVC_EXT_PIC_PARAMS extPicParams,
    PCODEC_HEVC_SCC_PIC_PARAMS sccPicParams)
{
    CODECHAL_DEBUG_FUNCTION_ENTER;

    if (!m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrPicParams))
    {
        return MOS_STATUS_SUCCESS;
    }

    CODECHAL_DEBUG_CHK_NULL(picParams);

    std::ostringstream oss;
    oss.setf(std::ios::showbase | std::ios::uppercase);
    oss.setf(std::ios::hex, std::ios::basefield);

    oss << "PicWidthInMinCbsY: " << +picParams->PicWidthInMinCbsY << std::endl;
    oss << "PicHeightInMinCbsY: " << +picParams->PicHeightInMinCbsY << std::endl;
    //wFormatAndSequenceInfoFlags
    oss << "chroma_format_idc: " << +picParams->chroma_format_idc << std::endl;
    oss << "separate_colour_plane_flag: " << +picParams->separate_colour_plane_flag << std::endl;
    oss << "bit_depth_luma_minus8: " << +picParams->bit_depth_luma_minus8 << std::endl;
    oss << "bit_depth_chroma_minus8: " << +picParams->bit_depth_chroma_minus8 << std::endl;
    oss << "log2_max_pic_order_cnt_lsb_minus4: " << +picParams->log2_max_pic_order_cnt_lsb_minus4 << std::endl;
    oss << "NoPicReorderingFlag: " << +picParams->NoPicReorderingFlag << std::endl;
    oss << "ReservedBits1: " << +picParams->ReservedBits1 << std::endl;
    oss << "wFormatAndSequenceInfoFlags: " << +picParams->wFormatAndSequenceInfoFlags << std::endl;
    oss << "CurrPic FrameIdx: " << +picParams->CurrPic.FrameIdx << std::endl;
    oss << "CurrPic PicFlags: " << +picParams->CurrPic.PicFlags << std::endl;
    oss << "sps_max_dec_pic_buffering_minus1: " << +picParams->sps_max_dec_pic_buffering_minus1 << std::endl;
    oss << "log2_min_luma_coding_block_size_minus3: " << +picParams->log2_min_luma_coding_block_size_minus3 << std::endl;
    oss << "log2_diff_max_min_luma_coding_block_size: " << +picParams->log2_diff_max_min_luma_coding_block_size << std::endl;
    oss << "log2_min_transform_block_size_minus2: " << +picParams->log2_min_transform_block_size_minus2 << std::endl;
    oss << "log2_diff_max_min_transform_block_size: " << +picParams->log2_diff_max_min_transform_block_size << std::endl;
    oss << "max_transform_hierarchy_depth_intra: " << +picParams->max_transform_hierarchy_depth_intra << std::endl;
    oss << "max_transform_hierarchy_depth_inter: " << +picParams->max_transform_hierarchy_depth_inter << std::endl;
    oss << "num_short_term_ref_pic_sets: " << +picParams->num_short_term_ref_pic_sets << std::endl;
    oss << "num_long_term_ref_pic_sps: " << +picParams->num_long_term_ref_pic_sps << std::endl;
    oss << "num_ref_idx_l0_default_active_minus1: " << +picParams->num_ref_idx_l0_default_active_minus1 << std::endl;
    oss << "num_ref_idx_l1_default_active_minus1: " << +picParams->num_ref_idx_l1_default_active_minus1 << std::endl;
    oss << "init_qp_minus26: " << +picParams->init_qp_minus26 << std::endl;
    oss << "ucNumDeltaPocsOfRefRpsIdx: " << +picParams->ucNumDeltaPocsOfRefRpsIdx << std::endl;
    oss << "wNumBitsForShortTermRPSInSlice: " << +picParams->wNumBitsForShortTermRPSInSlice << std::endl;
    oss << "ReservedBits2: " << +picParams->ReservedBits2 << std::endl;
    //dwCodingParamToolFlags
    oss << "scaling_list_enabled_flag: " << +picParams->scaling_list_enabled_flag << std::endl;
    oss << "amp_enabled_flag: " << +picParams->amp_enabled_flag << std::endl;
    oss << "sample_adaptive_offset_enabled_flag: " << +picParams->sample_adaptive_offset_enabled_flag << std::endl;
    oss << "pcm_enabled_flag: " << +picParams->pcm_enabled_flag << std::endl;
    oss << "pcm_sample_bit_depth_luma_minus1: " << +picParams->pcm_sample_bit_depth_luma_minus1 << std::endl;
    oss << "pcm_sample_bit_depth_chroma_minus1: " << +picParams->pcm_sample_bit_depth_chroma_minus1 << std::endl;
    oss << "log2_min_pcm_luma_coding_block_size_minus3: " << +picParams->log2_min_pcm_luma_coding_block_size_minus3 << std::endl;
    oss << "log2_diff_max_min_pcm_luma_coding_block_size: " << +picParams->log2_diff_max_min_pcm_luma_coding_block_size << std::endl;
    oss << "pcm_loop_filter_disabled_flag: " << +picParams->pcm_loop_filter_disabled_flag << std::endl;
    oss << "long_term_ref_pics_present_flag: " << +picParams->long_term_ref_pics_present_flag << std::endl;
    oss << "sps_temporal_mvp_enabled_flag: " << +picParams->sps_temporal_mvp_enabled_flag << std::endl;
    oss << "strong_intra_smoothing_enabled_flag: " << +picParams->strong_intra_smoothing_enabled_flag << std::endl;
    oss << "dependent_slice_segments_enabled_flag: " << +picParams->dependent_slice_segments_enabled_flag << std::endl;
    oss << "output_flag_present_flag: " << +picParams->output_flag_present_flag << std::endl;
    oss << "num_extra_slice_header_bits: " << +picParams->num_extra_slice_header_bits << std::endl;
    oss << "sign_data_hiding_enabled_flag: " << +picParams->sign_data_hiding_enabled_flag << std::endl;
    oss << "cabac_init_present_flag: " << +picParams->cabac_init_present_flag << std::endl;
    oss << "ReservedBits3: " << +picParams->ReservedBits3 << std::endl;
    oss << "dwCodingParamToolFlags: " << +picParams->dwCodingParamToolFlags << std::endl;
    //dwCodingSettingPicturePropertyFlags
    oss << "constrained_intra_pred_flag: " << +picParams->constrained_intra_pred_flag << std::endl;
    oss << "transform_skip_enabled_flag: " << +picParams->transform_skip_enabled_flag << std::endl;
    oss << "cu_qp_delta_enabled_flag: " << +picParams->cu_qp_delta_enabled_flag << std::endl;
    oss << "diff_cu_qp_delta_depth: " << +picParams->diff_cu_qp_delta_depth << std::endl;
    oss << "pps_slice_chroma_qp_offsets_present_flag: " << +picParams->pps_slice_chroma_qp_offsets_present_flag << std::endl;
    oss << "weighted_pred_flag: " << +picParams->weighted_pred_flag << std::endl;
    oss << "weighted_bipred_flag: " << +picParams->weighted_bipred_flag << std::endl;
    oss << "transquant_bypass_enabled_flag: " << +picParams->transquant_bypass_enabled_flag << std::endl;
    oss << "tiles_enabled_flag: " << +picParams->tiles_enabled_flag << std::endl;
    oss << "entropy_coding_sync_enabled_flag: " << +picParams->entropy_coding_sync_enabled_flag << std::endl;
    oss << "uniform_spacing_flag: " << +picParams->uniform_spacing_flag << std::endl;
    oss << "loop_filter_across_tiles_enabled_flag: " << +picParams->loop_filter_across_tiles_enabled_flag << std::endl;
    oss << "pps_loop_filter_across_slices_enabled_flag: " << +picParams->pps_loop_filter_across_slices_enabled_flag << std::endl;
    oss << "deblocking_filter_override_enabled_flag: " << +picParams->deblocking_filter_override_enabled_flag << std::endl;
    oss << "pps_deblocking_filter_disabled_flag: " << +picParams->pps_deblocking_filter_disabled_flag << std::endl;
    oss << "lists_modification_present_flag: " << +picParams->lists_modification_present_flag << std::endl;
    oss << "slice_segment_header_extension_present_flag: " << +picParams->slice_segment_header_extension_present_flag << std::endl;
    oss << "IrapPicFlag: " << +picParams->IrapPicFlag << std::endl;
    oss << "IdrPicFlag: " << +picParams->IdrPicFlag << std::endl;
    oss << "IntraPicFlag: " << +picParams->IntraPicFlag << std::endl;
    oss << "ReservedBits4: " << +picParams->ReservedBits4 << std::endl;
    oss << "dwCodingSettingPicturePropertyFlags: " << +picParams->dwCodingSettingPicturePropertyFlags << std::endl;
    oss << "pps_cb_qp_offset: " << +picParams->pps_cb_qp_offset << std::endl;
    oss << "pps_cr_qp_offset: " << +picParams->pps_cr_qp_offset << std::endl;
    oss << "num_tile_columns_minus1: " << +picParams->num_tile_columns_minus1 << std::endl;
    oss << "num_tile_rows_minus1: " << +picParams->num_tile_rows_minus1 << std::endl;
    //Dump column width
    oss << "column_width_minus1[19]:";
    for (uint8_t i = 0; i < 19; i++)
        oss << picParams->column_width_minus1[i] << " ";
    oss << std::endl;

    //Dump row height
    oss << "row_height_minus1[21]:";
    for (uint8_t i = 0; i < 21; i++)
        oss << picParams->row_height_minus1[i] << " ";
    oss << std::endl;

    oss << "pps_beta_offset_div2: " << +picParams->pps_beta_offset_div2 << std::endl;
    oss << "pps_tc_offset_div2: " << +picParams->pps_tc_offset_div2 << std::endl;
    oss << "log2_parallel_merge_level_minus2: " << +picParams->log2_parallel_merge_level_minus2 << std::endl;
    oss << "CurrPicOrderCntVal: " << +picParams->CurrPicOrderCntVal << std::endl;

    oss.setf(std::ios::dec, std::ios::basefield);
    //Dump RefFrameList[15]
    for (uint8_t i = 0; i < 15; ++i)
    {
        oss << "RefFrameList[" << +i << "] FrameIdx:" << +picParams->RefFrameList[i].FrameIdx << std::endl;
        oss << "RefFrameList[" << +i << "] PicFlags:" << +picParams->RefFrameList[i].PicFlags << std::endl;
    }

    //Dump POC List
    oss << "PicOrderCntValList[15]:";
    for (uint8_t i = 0; i < 15; i++)
        oss << std::hex << picParams->PicOrderCntValList[i] << " ";
    oss << std::endl;

    //Dump Ref RefPicSetStCurrBefore List
    oss << "RefPicSetStCurrBefore[8]:";
    for (uint8_t i = 0; i < 8; i++)
        oss << picParams->RefPicSetStCurrBefore[i] << " ";
    oss << std::endl;

    //Dump Ref RefPicSetStCurrAfter List
    oss << "RefPicSetStCurrAfter[16]:";
    for (uint8_t i = 0; i < 8; i++)
        oss << picParams->RefPicSetStCurrAfter[i] << " ";
    oss << std::endl;

    //Dump Ref PicSetStCurr List
    oss << "RefPicSetLtCurr[16]:";
    for (uint8_t i = 0; i < 8; i++)
        oss << picParams->RefPicSetLtCurr[i] << " ";
    oss << std::endl;

    //Dump Ref RefPicSetStCurrBefore List with POC
    oss << "RefPicSetStCurrBefore[8] (POC): ";
    for (uint8_t i = 0; i < 8; i++)
        oss << picParams->PicOrderCntValList[picParams->RefPicSetStCurrBefore[i]] << " ";
    oss << std::endl;

    //Dump Ref RefPicSetStCurrAfter List with POC
    oss << "RefPicSetStCurrAfter[16] (POC):";
    for (uint8_t i = 0; i < 8; i++)
        oss << picParams->PicOrderCntValList[picParams->RefPicSetStCurrAfter[i]] << " ";
    oss << std::endl;

    //Dump Ref PicSetStCurr List with POC
    oss << "RefPicSetLtCurr[16] (POC): ";
    for (uint8_t i = 0; i < 8; i++)
        oss << picParams->PicOrderCntValList[picParams->RefPicSetLtCurr[i]] << " ";
    oss << std::endl;

    oss << "RefFieldPicFlag: " << +picParams->RefFieldPicFlag << std::endl;
    oss << "RefBottomFieldFlag: " << +picParams->RefBottomFieldFlag << std::endl;
    oss << "StatusReportFeedbackNumber: " << +picParams->StatusReportFeedbackNumber << std::endl;

    if (extPicParams)
    {
        //PicRangeExtensionFlags
        oss << "transform_skip_rotation_enabled_flag: " << +extPicParams->PicRangeExtensionFlags.fields.transform_skip_rotation_enabled_flag << std::endl;
        oss << "transform_skip_context_enabled_flag: " << +extPicParams->PicRangeExtensionFlags.fields.transform_skip_context_enabled_flag << std::endl;
        oss << "implicit_rdpcm_enabled_flag: " << +extPicParams->PicRangeExtensionFlags.fields.implicit_rdpcm_enabled_flag << std::endl;
        oss << "explicit_rdpcm_enabled_flag: " << +extPicParams->PicRangeExtensionFlags.fields.explicit_rdpcm_enabled_flag << std::endl;
        oss << "extended_precision_processing_flag: " << +extPicParams->PicRangeExtensionFlags.fields.extended_precision_processing_flag << std::endl;
        oss << "intra_smoothing_disabled_flag: " << +extPicParams->PicRangeExtensionFlags.fields.intra_smoothing_disabled_flag << std::endl;
        oss << "high_precision_offsets_enabled_flag: " << +extPicParams->PicRangeExtensionFlags.fields.high_precision_offsets_enabled_flag << std::endl;
        oss << "persistent_rice_adaptation_enabled_flag: " << +extPicParams->PicRangeExtensionFlags.fields.persistent_rice_adaptation_enabled_flag << std::endl;
        oss << "cabac_bypass_alignment_enabled_flag: " << +extPicParams->PicRangeExtensionFlags.fields.cabac_bypass_alignment_enabled_flag << std::endl;
        oss << "cross_component_prediction_enabled_flag: " << +extPicParams->PicRangeExtensionFlags.fields.cross_component_prediction_enabled_flag << std::endl;
        oss << "chroma_qp_offset_list_enabled_flag: " << +extPicParams->PicRangeExtensionFlags.fields.chroma_qp_offset_list_enabled_flag << std::endl;
        oss << "BitDepthLuma16: " << +extPicParams->PicRangeExtensionFlags.fields.BitDepthLuma16 << std::endl;
        oss << "BitDepthChroma16: " << +extPicParams->PicRangeExtensionFlags.fields.BitDepthChroma16 << std::endl;
        oss << "diff_cu_chroma_qp_offset_depth: " << +extPicParams->diff_cu_chroma_qp_offset_depth << std::endl;
        oss << "chroma_qp_offset_list_len_minus1: " << +extPicParams->chroma_qp_offset_list_len_minus1 << std::endl;
        oss << "log2_sao_offset_scale_luma: " << +extPicParams->log2_sao_offset_scale_luma << std::endl;
        oss << "log2_sao_offset_scale_chroma: " << +extPicParams->log2_sao_offset_scale_chroma << std::endl;
        oss << "log2_max_transform_skip_block_size_minus2: " << +extPicParams->log2_max_transform_skip_block_size_minus2 << std::endl;

        //Dump cb_qp_offset_list[6]
        oss << "cb_qp_offset_list[6]: ";
        for (uint8_t i = 0; i < 6; i++)
            oss << extPicParams->cb_qp_offset_list[i] << " ";
        oss << std::endl;

        //Dump cr_qp_offset_list[6]
        oss << "cr_qp_offset_list[6]: ";
        for (uint8_t i = 0; i < 6; i++)
            oss << extPicParams->cr_qp_offset_list[i] << " ";
        oss << std::endl;

        //Dump scc pic parameters
        if(sccPicParams)
        {
            oss << "pps_curr_pic_ref_enabled_flag: " << +sccPicParams->PicSCCExtensionFlags.fields.pps_curr_pic_ref_enabled_flag<< std::endl;
            oss << "palette_mode_enabled_flag: " << +sccPicParams->PicSCCExtensionFlags.fields.palette_mode_enabled_flag << std::endl;
            oss << "motion_vector_resolution_control_idc: " << +sccPicParams->PicSCCExtensionFlags.fields.motion_vector_resolution_control_idc << std::endl;
            oss << "intra_boundary_filtering_disabled_flag: " << +sccPicParams->PicSCCExtensionFlags.fields.intra_boundary_filtering_disabled_flag << std::endl;
            oss << "residual_adaptive_colour_transform_enabled_flag: " << +sccPicParams->PicSCCExtensionFlags.fields.residual_adaptive_colour_transform_enabled_flag << std::endl;
            oss << "pps_slice_act_qp_offsets_present_flag: " << +sccPicParams->PicSCCExtensionFlags.fields.pps_slice_act_qp_offsets_present_flag << std::endl;
            oss << "palette_max_size: " << +sccPicParams->palette_max_size << std::endl;
            oss << "delta_palette_max_predictor_size: " << +sccPicParams->delta_palette_max_predictor_size << std::endl;
            oss << "PredictorPaletteSize: " << +sccPicParams->PredictorPaletteSize << std::endl;

            for(uint8_t i = 0; i < 128; i++)
            {
                oss << "PredictorPaletteEntries[0][" << +i << "]: " << +sccPicParams->PredictorPaletteEntries[0][i] << std::endl;
                oss << "PredictorPaletteEntries[1][" << +i << "]: " << +sccPicParams->PredictorPaletteEntries[1][i] << std::endl;
                oss << "PredictorPaletteEntries[2][" << +i << "]: " << +sccPicParams->PredictorPaletteEntries[2][i] << std::endl;
            }
            
            oss << "pps_act_y_qp_offset_plus5: " << +sccPicParams->pps_act_y_qp_offset_plus5 << std::endl;
            oss << "pps_act_cb_qp_offset_plus5: " << +sccPicParams->pps_act_y_qp_offset_plus5 << std::endl;
            oss << "pps_act_cr_qp_offset_plus3: " << +sccPicParams->pps_act_y_qp_offset_plus5 << std::endl;
        }
    }

    const char *fileName = m_debugInterface->CreateFileName(
        "_DEC",
        CodechalDbgBufferType::bufPicParams,
        CodechalDbgExtType::txt);

    std::ofstream ofs(fileName, std::ios::out);
    ofs << oss.str();
    ofs.close();

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalDecodeHevcG12::DumpSliceParams(
    PCODEC_HEVC_SLICE_PARAMS     sliceParams,
    PCODEC_HEVC_EXT_SLICE_PARAMS extSliceParams,
    uint32_t                     numSlices,
    bool                         shortFormatInUse)
{
    CODECHAL_DEBUG_FUNCTION_ENTER;
    if (!m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrSlcParams))
    {
        return MOS_STATUS_SUCCESS;
    }

    CODECHAL_DEBUG_CHK_NULL(sliceParams);

    PCODEC_HEVC_SLICE_PARAMS     hevcSliceControl    = nullptr;
    PCODEC_HEVC_EXT_SLICE_PARAMS hevcExtSliceControl = nullptr;

    std::ostringstream oss;
    oss.setf(std::ios::showbase | std::ios::uppercase);

    for (uint16_t j = 0; j < numSlices; j++)
    {
        hevcSliceControl = &sliceParams[j];
        if (extSliceParams)
        {
            hevcExtSliceControl = &extSliceParams[j];
        }

        oss << "====================================================================================================" << std::endl;
        oss << "Data for Slice number = " << +j << std::endl;
        oss << "slice_data_size: " << +hevcSliceControl->slice_data_size << std::endl;
        oss << "slice_data_offset: " << +hevcSliceControl->slice_data_offset << std::endl;

        if (!shortFormatInUse)
        {
            //Dump Long format specific
            oss << "ByteOffsetToSliceData: " << +hevcSliceControl->ByteOffsetToSliceData << std::endl;
            oss << "slice_segment_address: " << +hevcSliceControl->slice_segment_address << std::endl;

            //Dump RefPicList[2][15]
            for (uint8_t i = 0; i < 15; ++i)
            {
                oss << "RefPicList[0][" << +i << "]";
                oss << "FrameIdx: " << +hevcSliceControl->RefPicList[0][i].FrameIdx;
                oss << ", PicFlags: " << +hevcSliceControl->RefPicList[0][i].PicFlags;
                oss << std::endl;
            }
            for (uint8_t i = 0; i < 15; ++i)
            {
                oss << "RefPicList[1][" << +i << "]";
                oss << "FrameIdx: " << +hevcSliceControl->RefPicList[1][i].FrameIdx;
                oss << ", PicFlags: " << +hevcSliceControl->RefPicList[1][i].PicFlags;
                oss << std::endl;
            }

            oss << "last_slice_of_pic: " << +hevcSliceControl->LongSliceFlags.fields.LastSliceOfPic << std::endl;
            oss << "dependent_slice_segment_flag: " << +hevcSliceControl->LongSliceFlags.fields.dependent_slice_segment_flag << std::endl;
            oss << "slice_type: " << +hevcSliceControl->LongSliceFlags.fields.slice_type << std::endl;
            oss << "color_plane_id: " << +hevcSliceControl->LongSliceFlags.fields.color_plane_id << std::endl;
            oss << "slice_sao_luma_flag: " << +hevcSliceControl->LongSliceFlags.fields.slice_sao_luma_flag << std::endl;
            oss << "slice_sao_chroma_flag: " << +hevcSliceControl->LongSliceFlags.fields.slice_sao_chroma_flag << std::endl;
            oss << "mvd_l1_zero_flag: " << +hevcSliceControl->LongSliceFlags.fields.mvd_l1_zero_flag << std::endl;
            oss << "cabac_init_flag: " << +hevcSliceControl->LongSliceFlags.fields.cabac_init_flag << std::endl;
            oss << "slice_temporal_mvp_enabled_flag: " << +hevcSliceControl->LongSliceFlags.fields.slice_temporal_mvp_enabled_flag << std::endl;
            oss << "slice_deblocking_filter_disabled_flag: " << +hevcSliceControl->LongSliceFlags.fields.slice_deblocking_filter_disabled_flag << std::endl;
            oss << "collocated_from_l0_flag: " << +hevcSliceControl->LongSliceFlags.fields.collocated_from_l0_flag << std::endl;
            oss << "slice_loop_filter_across_slices_enabled_flag: " << +hevcSliceControl->LongSliceFlags.fields.slice_loop_filter_across_slices_enabled_flag << std::endl;
            oss << "reserved: " << +hevcSliceControl->LongSliceFlags.fields.reserved << std::endl;
            oss << "collocated_ref_idx: " << +hevcSliceControl->collocated_ref_idx << std::endl;
            oss << "num_ref_idx_l0_active_minus1: " << +hevcSliceControl->num_ref_idx_l0_active_minus1 << std::endl;
            oss << "num_ref_idx_l1_active_minus1: " << +hevcSliceControl->num_ref_idx_l1_active_minus1 << std::endl;
            oss << "slice_qp_delta: " << +hevcSliceControl->slice_qp_delta << std::endl;
            oss << "slice_cb_qp_offset: " << +hevcSliceControl->slice_cb_qp_offset << std::endl;
            oss << "slice_cr_qp_offset: " << +hevcSliceControl->slice_cr_qp_offset << std::endl;
            oss << "slice_beta_offset_div2: " << +hevcSliceControl->slice_beta_offset_div2 << std::endl;
            oss << "slice_tc_offset_div2: " << +hevcSliceControl->slice_tc_offset_div2 << std::endl;
            oss << "luma_log2_weight_denom: " << +hevcSliceControl->luma_log2_weight_denom << std::endl;
            oss << "delta_chroma_log2_weight_denom: " << +hevcSliceControl->delta_chroma_log2_weight_denom << std::endl;

            //Dump luma_offset[2][15]
            for (uint8_t i = 0; i < 15; i++)
            {
                oss << "luma_offset_l0[" << +i << "]: " << (extSliceParams ? +hevcExtSliceControl->luma_offset_l0[i] : +hevcSliceControl->luma_offset_l0[i]) << std::endl;
                oss << "luma_offset_l1[" << +i << "]: " << (extSliceParams ? +hevcExtSliceControl->luma_offset_l1[i] : +hevcSliceControl->luma_offset_l1[i]) << std::endl;
            }
            //Dump delta_luma_weight[2][15]
            for (uint8_t i = 0; i < 15; i++)
            {
                oss << "delta_luma_weight_l0[" << +i << "]: " << +hevcSliceControl->delta_luma_weight_l0[i] << std::endl;
                oss << "delta_luma_weight_l1[" << +i << "]: " << +hevcSliceControl->delta_luma_weight_l0[i] << std::endl;
            }
            //Dump chroma_offset[2][15][2]
            for (uint8_t i = 0; i < 15; i++)
            {
                oss << "ChromaOffsetL0[" << +i << "][0]: " << (extSliceParams ? +hevcExtSliceControl->ChromaOffsetL0[i][0] : +hevcSliceControl->ChromaOffsetL0[i][0]) << std::endl;

                oss << "ChromaOffsetL0[" << +i << "][1]: " << (extSliceParams ? +hevcExtSliceControl->ChromaOffsetL0[i][1] : +hevcSliceControl->ChromaOffsetL0[i][1]) << std::endl;

                oss << "ChromaOffsetL1[" << +i << "][0]: " << (extSliceParams ? +hevcExtSliceControl->ChromaOffsetL1[i][0] : +hevcSliceControl->ChromaOffsetL1[i][0]) << std::endl;

                oss << "ChromaOffsetL1[" << +i << "][1]: " << (extSliceParams ? +hevcExtSliceControl->ChromaOffsetL1[i][1] : +hevcSliceControl->ChromaOffsetL1[i][1]) << std::endl;
            }
            //Dump delta_chroma_weight[2][15][2]
            for (uint8_t i = 0; i < 15; i++)
            {
                oss << "delta_chroma_weight_l0[" << +i << "][0]: " << +hevcSliceControl->delta_chroma_weight_l0[i][0] << std::endl;
                oss << "delta_chroma_weight_l0[" << +i << "][1]: " << +hevcSliceControl->delta_chroma_weight_l0[i][1] << std::endl;
                oss << "delta_chroma_weight_l1[" << +i << "][0]: " << +hevcSliceControl->delta_chroma_weight_l1[i][0] << std::endl;
                oss << "delta_chroma_weight_l1[" << +i << "][1]: " << +hevcSliceControl->delta_chroma_weight_l1[i][1] << std::endl;
            }

            oss << "five_minus_max_num_merge_cand: " << +hevcSliceControl->five_minus_max_num_merge_cand << std::endl;
            oss << "num_entry_point_offsets: " << +hevcSliceControl->num_entry_point_offsets << std::endl;
            oss << "EntryOffsetToSubsetArray: " << +hevcSliceControl->EntryOffsetToSubsetArray << std::endl;

            if (extSliceParams)
            {
                oss << "cu_chroma_qp_offset_enabled_flag: " << +hevcExtSliceControl->cu_chroma_qp_offset_enabled_flag << std::endl;

                //Dump scc slice parameters
                oss << "slice_act_y_qp_offset: " << +hevcExtSliceControl->slice_act_y_qp_offset<< std::endl;
                oss << "slice_act_cb_qp_offset: " << +hevcExtSliceControl->slice_act_cb_qp_offset << std::endl;
                oss << "slice_act_cr_qp_offset: " << +hevcExtSliceControl->slice_act_cr_qp_offset << std::endl;
                oss << "use_integer_mv_flag: " << +hevcExtSliceControl->use_integer_mv_flag << std::endl;
            }
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

MOS_STATUS CodechalDecodeHevcG12::DumpSubsetsParams(
        PCODEC_HEVC_SUBSET_PARAMS     subsetsParams)
{
    CODECHAL_DEBUG_FUNCTION_ENTER;
    if (!m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrSubsetsParams))
    {
        return MOS_STATUS_SUCCESS;
    }

    if(subsetsParams)
    {
        std::ostringstream oss;
        oss.setf(std::ios::showbase | std::ios::uppercase);

        for(uint16_t i = 0; i < 440; i++)
        {
            oss << "entry_point_offset_minus1[" << +i << "]: " << +subsetsParams->entry_point_offset_minus1[i]<< std::endl;
        }

        //create the file
        const char *fileName = m_debugInterface->CreateFileName(
        "_DEC",
        CodechalDbgBufferType::bufSubsetsParams,
        CodechalDbgExtType::txt);

        std::ofstream ofs(fileName, std::ios::out);
        ofs << oss.str();
        ofs.close();
    }
    return MOS_STATUS_SUCCESS;
}
#endif
