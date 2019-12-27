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
//! \file     codechal_encode_csc_ds_g12.cpp
//! \brief    This file implements the Csc+Ds feature for all codecs on Gen12 platform
//!

#include "codechal_encoder_base.h"
#include "codechal_encode_csc_ds_g12.h"
#include "codechal_encode_sfc_g12.h"
#include "codechal_kernel_header_g12.h"
#include "codeckrnheader.h"
#if defined(ENABLE_KERNELS) && !defined(_FULL_OPEN_SOURCE)
#include "igcodeckrn_g12.h"
#endif
#if USE_CODECHAL_DEBUG_TOOL
#include "codechal_debug_encode_par_g12.h"
#endif

uint8_t CodechalEncodeCscDsG12::GetBTCount() const
{
    return (uint8_t)cscNumSurfaces;
}

MOS_STATUS CodechalEncodeCscDsG12::AllocateSurfaceCsc()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodechalEncodeCscDs::AllocateSurfaceCsc());

    // allocate the MbStats surface
    if (Mos_ResourceIsNull(&m_resMbStatsBuffer))
    {
        MOS_ALLOC_GFXRES_PARAMS    allocParamsForBufferLinear;
        MOS_ZeroMemory(&allocParamsForBufferLinear, sizeof(MOS_ALLOC_GFXRES_PARAMS));
        allocParamsForBufferLinear.Type = MOS_GFXRES_BUFFER;
        allocParamsForBufferLinear.TileType = MOS_TILE_LINEAR;
        allocParamsForBufferLinear.Format = Format_Buffer;
        uint32_t alignedWidth = MOS_ALIGN_CEIL(CODECHAL_GET_WIDTH_IN_MACROBLOCKS(m_encoder->m_oriFrameWidth), 64);
        uint32_t alignedHeight = MOS_ALIGN_CEIL(CODECHAL_GET_WIDTH_IN_MACROBLOCKS(m_encoder->m_oriFrameHeight), 64);
        allocParamsForBufferLinear.dwBytes = m_hwInterface->m_avcMbStatBufferSize =
            MOS_ALIGN_CEIL((alignedWidth * alignedHeight << 6) , 1024);
        allocParamsForBufferLinear.pBufName = "MB Statistics Buffer";

        CODECHAL_ENCODE_CHK_STATUS_MESSAGE_RETURN(m_osInterface->pfnAllocateResource(
            m_osInterface,
            &allocParamsForBufferLinear,
            &m_resMbStatsBuffer), "Failed to allocate  MB Statistics Buffer.");
    }

    return eStatus;
}

MOS_STATUS CodechalEncodeCscDsG12::CheckRawColorFormat(MOS_FORMAT format)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    // check input color format, and set target traverse thread space size
    switch (format)
    {
    case Format_NV12:
        m_colorRawSurface = cscColorNv12Linear;
        m_cscRequireColor = 1;
        break;
    case Format_YUY2:
    case Format_YUYV:
        m_colorRawSurface = cscColorYUY2;
        m_cscRequireColor = (uint8_t)HCP_CHROMA_FORMAT_YUV420 == m_outputChromaFormat;
        m_cscRequireConvTo8bPlanar = (uint8_t)HCP_CHROMA_FORMAT_YUV422 == m_outputChromaFormat;
        break;
    case Format_A8R8G8B8:
        m_colorRawSurface = cscColorARGB;
        m_cscUsingSfc = IsSfcEnabled() ? 1 : 0;
        m_cscRequireColor = 1;
        //Use EU for better performance in big resolution cases
        if (m_cscRawSurfWidth * m_cscRawSurfHeight > 1920 * 1088)
        {
            m_cscUsingSfc = 0;
        }
        break;
    case Format_A8B8G8R8:
        m_colorRawSurface = cscColorABGR;
        m_cscRequireColor = 1;
        break;
    case Format_P010:
    case Format_P016:
        m_colorRawSurface = cscColorP010;
        m_cscRequireConvTo8bPlanar = 1;
        break;
    case Format_Y210:
        if (m_encoder->m_vdencEnabled)
        {
            CODECHAL_ENCODE_ASSERTMESSAGE("Input color format Y210 Linear or Tile X not yet supported!");
            eStatus = MOS_STATUS_PLATFORM_NOT_SUPPORTED;
        }
        else
        {
            m_colorRawSurface = cscColorY210;
            m_cscRequireConvTo8bPlanar = 1;
        }
        break;
    case Format_Y216:
        m_colorRawSurface = cscColorY210;
        m_cscRequireConvTo8bPlanar = 1;
        break;
    case Format_P210:
        // not supported yet so fall-thru to default
        m_colorRawSurface = cscColorP210;
        m_cscRequireConvTo8bPlanar = 1;
    default:
        CODECHAL_ENCODE_ASSERTMESSAGE("Input color format = %d not yet supported!", format);
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        break;
    }

    return eStatus;
}

MOS_STATUS CodechalEncodeCscDsG12::InitKernelStateCsc()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_KERNEL_HEADER currKrnHeader;
    auto kernelSize = m_combinedKernelSize;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(GetCommonKernelHeaderAndSizeG12(
        m_kernelBase,
        ENC_SCALING_CONVERSION,
        0,
        &currKrnHeader,
        &kernelSize));

    m_cscKernelState->KernelParams.iBTCount = cscNumSurfaces;
    m_cscKernelState->KernelParams.iThreadCount = m_hwInterface->GetRenderInterface()->GetHwCaps()->dwMaxThreads;
    m_cscKernelState->KernelParams.iCurbeLength = m_cscCurbeLength;
    m_cscKernelState->KernelParams.iBlockWidth = CODECHAL_MACROBLOCK_WIDTH;
    m_cscKernelState->KernelParams.iBlockHeight = CODECHAL_MACROBLOCK_HEIGHT;
    m_cscKernelState->KernelParams.iIdCount = 1;
    m_cscKernelState->KernelParams.iInlineDataLength = m_cscCurbeLength;
    m_cscKernelState->dwCurbeOffset = m_stateHeapInterface->GetSizeofCmdInterfaceDescriptorData();
    m_cscKernelState->KernelParams.pBinary =
        m_kernelBase + (currKrnHeader.KernelStartPointer << MHW_KERNEL_OFFSET_SHIFT);
    m_cscKernelState->KernelParams.iSize = kernelSize;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->CalculateSshAndBtSizesRequested(
        m_cscKernelState->KernelParams.iBTCount,
        &m_cscKernelState->dwSshSize,
        &m_cscKernelState->dwBindingTableSize));

    CODECHAL_ENCODE_CHK_NULL_RETURN(m_renderInterface->m_stateHeapInterface);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->MhwInitISH(m_renderInterface->m_stateHeapInterface, m_cscKernelState));

    m_maxBtCount += MOS_ALIGN_CEIL(cscNumSurfaces,m_renderInterface->m_stateHeapInterface->pStateHeapInterface->GetBtIdxAlignment());

    return eStatus;
}

MOS_STATUS CodechalEncodeCscDsG12::SetKernelParamsCsc(KernelParams* params)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_NULL_RETURN(params);

    m_lastTaskInPhase = params->bLastTaskInPhaseCSC;

    auto inputFrameWidth = m_encoder->m_frameWidth;
    auto inputFrameHeight = m_encoder->m_frameHeight;
    auto inputSurface = m_rawSurfaceToEnc;
    auto output4xDsSurface = m_encoder->m_trackedBuf->Get4xDsSurface(CODEC_CURR_TRACKED_BUFFER);
    auto output2xDsSurface = m_encoder->m_trackedBuf->Get2xDsSurface(CODEC_CURR_TRACKED_BUFFER);
    auto mbStatsSurface = &m_resMbStatsBuffer;

    m_curbeParams.bHevcEncHistorySum = false;
    m_surfaceParamsCsc.hevcExtParams = nullptr;

    if (dsDisabled == params->stageDsConversion)
    {
        m_curbeParams.bConvertFlag = m_cscFlag != 0;

        if (m_2xScalingEnabled && m_scalingEnabled)
        {
            m_curbeParams.downscaleStage = dsStage2x4x;
            m_currRefList->b4xScalingUsed =
            m_currRefList->b2xScalingUsed = true;
            m_surfaceParamsCsc.bScalingInUses16UnormSurfFmt = false;
            m_surfaceParamsCsc.bScalingInUses32UnormSurfFmt = false;
        }
        else if (m_2xScalingEnabled)
        {
            m_curbeParams.downscaleStage = dsStage2x;
            m_currRefList->b2xScalingUsed = true;
            output4xDsSurface = nullptr;
            mbStatsSurface = nullptr;
            m_surfaceParamsCsc.bScalingInUses16UnormSurfFmt = true;
            m_surfaceParamsCsc.bScalingInUses32UnormSurfFmt = false;
        }
        else if (m_scalingEnabled)
        {
            m_curbeParams.downscaleStage = dsStage4x;
            m_currRefList->b4xScalingUsed = true;
            output2xDsSurface = nullptr;
            m_surfaceParamsCsc.bScalingInUses16UnormSurfFmt = false;
            m_surfaceParamsCsc.bScalingInUses32UnormSurfFmt = true;
        }
        else
        {
            // do CSC only
            m_curbeParams.downscaleStage = dsDisabled;
            output4xDsSurface = nullptr;
            output2xDsSurface = nullptr;
            mbStatsSurface = nullptr;
            m_surfaceParamsCsc.bScalingInUses16UnormSurfFmt = false;
            m_surfaceParamsCsc.bScalingInUses32UnormSurfFmt = false;
        }

        // history sum to be enabled only for the 4x stage
        if (params->hevcExtParams)
        {
            auto hevcExtParam = (HevcExtKernelParams*)params->hevcExtParams;
            m_curbeParams.bUseLCU32 = hevcExtParam->bUseLCU32;
            m_curbeParams.bHevcEncHistorySum = hevcExtParam->bHevcEncHistorySum;
            m_surfaceParamsCsc.hevcExtParams = params->hevcExtParams;
        }
    }
    else
    {
        // do 16x/32x downscaling      
        m_curbeParams.bConvertFlag = false;
        mbStatsSurface = nullptr;

        if (dsStage16x == params->stageDsConversion)
        {
            m_currRefList->b16xScalingUsed = true;
            m_lastTaskInPhase = params->bLastTaskInPhase16xDS;
            m_curbeParams.downscaleStage = dsStage16x;
            inputFrameWidth = m_encoder->m_downscaledWidth4x << 2;
            inputFrameHeight = m_encoder->m_downscaledHeight4x << 2;
            
            inputSurface = m_encoder->m_trackedBuf->Get4xDsSurface(CODEC_CURR_TRACKED_BUFFER);
            output4xDsSurface = m_encoder->m_trackedBuf->Get16xDsSurface(CODEC_CURR_TRACKED_BUFFER);
            output2xDsSurface = nullptr;
            m_surfaceParamsCsc.bScalingInUses16UnormSurfFmt = false;
            m_surfaceParamsCsc.bScalingInUses32UnormSurfFmt = true;
        }
        else if (dsStage32x == params->stageDsConversion)
        {
            m_currRefList->b32xScalingUsed = true;
            m_lastTaskInPhase = params->bLastTaskInPhase32xDS;
            m_curbeParams.downscaleStage = dsStage2x;
            inputFrameWidth = m_encoder->m_downscaledWidth16x;
            inputFrameHeight = m_encoder->m_downscaledHeight16x;

            inputSurface = m_encoder->m_trackedBuf->Get16xDsSurface(CODEC_CURR_TRACKED_BUFFER);
            output4xDsSurface = nullptr;
            output2xDsSurface = m_encoder->m_trackedBuf->Get32xDsSurface(CODEC_CURR_TRACKED_BUFFER);
            m_surfaceParamsCsc.bScalingInUses16UnormSurfFmt = true;
            m_surfaceParamsCsc.bScalingInUses32UnormSurfFmt = false;
        }
    }

    // setup Curbe
    m_curbeParams.dwInputPictureWidth = inputFrameWidth;
    m_curbeParams.dwInputPictureHeight = inputFrameHeight;  

    // setup surface states
    m_surfaceParamsCsc.psInputSurface = inputSurface;
    m_surfaceParamsCsc.psOutputCopiedSurface = m_curbeParams.bConvertFlag ? m_encoder->m_trackedBuf->GetCscSurface(CODEC_CURR_TRACKED_BUFFER) : nullptr;
    m_surfaceParamsCsc.psOutput4xDsSurface = output4xDsSurface;
    m_surfaceParamsCsc.psOutput2xDsSurface = output2xDsSurface;
    m_surfaceParamsCsc.presMBVProcStatsBuffer = mbStatsSurface;
    m_surfaceParamsCsc.hevcExtParams = params->hevcExtParams;

    if (dsStage16x == params->stageDsConversion)
    {
        // here to calculate the walker resolution, we need to use the input surface resolution.
        // it is inputFrameWidth/height / 4 in 16xStage, becasue kernel internally will do this.
        inputFrameWidth = inputFrameWidth >> 2;
        inputFrameHeight = inputFrameHeight >> 2;
    }

    // setup walker param
    m_walkerResolutionX = CODECHAL_GET_4xDS_SIZE_32ALIGNED(inputFrameWidth) >> 3;
    m_walkerResolutionY = CODECHAL_GET_4xDS_SIZE_32ALIGNED(inputFrameHeight) >> 3;

    return eStatus;
}

MOS_STATUS CodechalEncodeCscDsG12::SetCurbeCsc()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CscKernelCurbeData curbe;

    curbe.DW0_OutputBitDepthForChroma = m_curbeParams.ucEncBitDepthChroma;
    curbe.DW0_OutputBitDepthForLuma = m_curbeParams.ucEncBitDepthLuma;
    curbe.DW0_RoundingEnable = 1;

    curbe.DW1_PictureFormat = (uint8_t)((m_colorRawSurface == cscColorABGR) ? cscColorARGB : m_colorRawSurface); // Use cscColorARGB for ABGR CSC, just switch B and R coefficients
    curbe.DW1_ConvertFlag = m_curbeParams.bConvertFlag;
    curbe.DW1_DownscaleStage = (uint8_t)m_curbeParams.downscaleStage;
    curbe.DW1_MbStatisticsDumpFlag = (m_curbeParams.downscaleStage == dsStage4x || m_curbeParams.downscaleStage == dsStage2x4x);
    curbe.DW1_YUY2ConversionFlag = (m_colorRawSurface == cscColorYUY2) && m_cscRequireColor;
    curbe.DW1_HevcEncHistorySum = m_curbeParams.bHevcEncHistorySum;
    curbe.DW1_LCUSize = m_curbeParams.bUseLCU32;

    curbe.DW2_OriginalPicWidthInSamples = m_curbeParams.dwInputPictureWidth;
    curbe.DW2_OriginalPicHeightInSamples = m_curbeParams.dwInputPictureHeight;

    // when the input surface is NV12 tiled format and not aligned with 4 bytes,
    // need kernel to do the padding copy with force to linear format, it's
    // transparent to kernel and hw can handle it
    if (m_colorRawSurface == cscColorNv12TileY && m_cscFlag == 1)
        curbe.DW1_PictureFormat = cscColorNv12Linear;


    // RGB->YUV CSC coefficients
    if (m_curbeParams.inputColorSpace == ECOLORSPACE_P709)
    {
        curbe.DW4_CSC_Coefficient_C0 = 0xFFCD;
        curbe.DW5_CSC_Coefficient_C3 = 0x0080;
        curbe.DW6_CSC_Coefficient_C4 = 0x004F;
        curbe.DW7_CSC_Coefficient_C7 = 0x0010;
        curbe.DW8_CSC_Coefficient_C8 = 0xFFD5;
        curbe.DW9_CSC_Coefficient_C11 = 0x0080;
        if (cscColorARGB == m_colorRawSurface)
        {
            curbe.DW4_CSC_Coefficient_C1 = 0xFFFB;
            curbe.DW5_CSC_Coefficient_C2 = 0x0038;
            curbe.DW6_CSC_Coefficient_C5 = 0x0008;
            curbe.DW7_CSC_Coefficient_C6 = 0x0017;
            curbe.DW8_CSC_Coefficient_C9 = 0x0038;
            curbe.DW9_CSC_Coefficient_C10 = 0xFFF3;
        }
        else // cscColorABGR == m_colorRawSurface
        {
            curbe.DW4_CSC_Coefficient_C1 = 0x0038;
            curbe.DW5_CSC_Coefficient_C2 = 0xFFFB;
            curbe.DW6_CSC_Coefficient_C5 = 0x0017;
            curbe.DW7_CSC_Coefficient_C6 = 0x0008;
            curbe.DW8_CSC_Coefficient_C9 = 0xFFF3;
            curbe.DW9_CSC_Coefficient_C10 = 0x0038;
        }
    }
    else if (m_curbeParams.inputColorSpace == ECOLORSPACE_P601)
    {
        curbe.DW4_CSC_Coefficient_C0 = 0xFFD1;
        curbe.DW5_CSC_Coefficient_C3 = 0x0080;
        curbe.DW6_CSC_Coefficient_C4 = 0x0041;
        curbe.DW7_CSC_Coefficient_C7 = 0x0010;
        curbe.DW8_CSC_Coefficient_C8 = 0xFFDB;
        curbe.DW9_CSC_Coefficient_C11 = 0x0080;
        if (cscColorARGB == m_colorRawSurface)
        {
            curbe.DW4_CSC_Coefficient_C1 = 0xFFF7;
            curbe.DW5_CSC_Coefficient_C2 = 0x0038;
            curbe.DW6_CSC_Coefficient_C5 = 0x000D;
            curbe.DW7_CSC_Coefficient_C6 = 0x0021;
            curbe.DW8_CSC_Coefficient_C9 = 0x0038;
            curbe.DW9_CSC_Coefficient_C10 = 0xFFED;
        }
        else // cscColorABGR == m_colorRawSurface
        {
            curbe.DW4_CSC_Coefficient_C1 = 0x0038;
            curbe.DW5_CSC_Coefficient_C2 = 0xFFF7;
            curbe.DW6_CSC_Coefficient_C5 = 0x0021;
            curbe.DW7_CSC_Coefficient_C6 = 0x000D;
            curbe.DW8_CSC_Coefficient_C9 = 0xFFED;
            curbe.DW9_CSC_Coefficient_C10 = 0x0038;
        }
    }
    else
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Unsupported ARGB input color space = %d!", m_curbeParams.inputColorSpace);
        return MOS_STATUS_INVALID_PARAMETER;
    }

    curbe.DW10_BTI_InputSurface = cscSrcYPlane;
    curbe.DW11_BTI_Enc8BitSurface = cscDstConvYPlane;
    curbe.DW12_BTI_4xDsSurface = cscDst4xDs;
    curbe.DW13_BTI_MbStatsSurface = cscDstMbStats;
    curbe.DW14_BTI_2xDsSurface = cscDst2xDs;
    curbe.DW15_BTI_HistoryBuffer = cscDstHistBuffer;
    curbe.DW16_BTI_HistorySumBuffer = cscDstHistSum;
    curbe.DW17_BTI_MultiTaskBuffer = cscDstMultiTask;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_cscKernelState->m_dshRegion.AddData(
        &curbe,
        m_cscKernelState->dwCurbeOffset,
        sizeof(curbe)));

    return eStatus;
}

MOS_STATUS CodechalEncodeCscDsG12::SendSurfaceCsc(PMOS_COMMAND_BUFFER cmdBuffer)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    // PAK input surface (could be 10-bit)
    CODECHAL_SURFACE_CODEC_PARAMS surfaceParams;
    MOS_ZeroMemory(&surfaceParams, sizeof(surfaceParams));
    surfaceParams.bIs2DSurface = true;
    surfaceParams.bUseUVPlane = (cscColorNv12TileY == m_colorRawSurface ||
        cscColorP010 == m_colorRawSurface ||
        cscColorP210 == m_colorRawSurface ||
        cscColorNv12Linear == m_colorRawSurface);
    surfaceParams.bMediaBlockRW = true;

    // Configure to R16/32 for input surface
    if (m_surfaceParamsCsc.bScalingInUses16UnormSurfFmt)
    {
        // 32x scaling requires R16_UNROM
        surfaceParams.bUse16UnormSurfaceFormat = true;
    }
    else if (m_surfaceParamsCsc.bScalingInUses32UnormSurfFmt)
    {
        surfaceParams.bUse32UnormSurfaceFormat = true;
    }
    else
    {
        /*
        * Unify surface format to avoid mismatches introduced by DS kernel between MMC on and off cases.
        * bUseCommonKernel        | FormatIsNV12 | MmcdOn | SurfaceFormatToUse
        *            1            |       1      |  0/1   |        R8
        *            1            |       0      |  0/1   |        R16
        *            0            |       1      |  0/1   |        R8
        *            0            |       0      |   1    |        R8
        *            0            |       0      |   0    |        R32
        */
        surfaceParams.bUse16UnormSurfaceFormat = !(cscColorNv12TileY == m_colorRawSurface ||
                                                   cscColorNv12Linear == m_colorRawSurface);
    }

    // when input surface is NV12 tiled and not aligned by 4 bytes, need kernel to do the
    // padding copy by forcing to linear format and set the HeightInUse as Linear format
    // kernel will use this info to calucate UV offset
    surfaceParams.psSurface = m_surfaceParamsCsc.psInputSurface;
    if (cscColorNv12Linear == m_colorRawSurface ||
        (cscColorNv12TileY == m_colorRawSurface && m_cscFlag == 1))
    {
        surfaceParams.dwHeightInUse = (surfaceParams.psSurface->dwHeight * 3) / 2;
    }
    surfaceParams.dwCacheabilityControl = m_hwInterface->ComposeSurfaceCacheabilityControl(
        MOS_CODEC_RESOURCE_USAGE_ORIGINAL_UNCOMPRESSED_PICTURE_ENCODE,
        (codechalL3 | codechalLLC));

#ifdef _MMC_SUPPORTED
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_encoder->m_mmcState);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_encoder->m_mmcState->SetSurfaceParams(&surfaceParams));
#endif

    surfaceParams.dwBindingTableOffset = cscSrcYPlane;
    surfaceParams.dwUVBindingTableOffset = cscSrcUVPlane;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceParams,
        m_cscKernelState));

    // Converted NV12 output surface, or ENC 8-bit output surface
    if (m_surfaceParamsCsc.psOutputCopiedSurface)
    {
        MOS_ZeroMemory(&surfaceParams, sizeof(surfaceParams));
        surfaceParams.bIs2DSurface =
            surfaceParams.bUseUVPlane =
            surfaceParams.bMediaBlockRW =
            surfaceParams.bIsWritable = true;
        surfaceParams.psSurface = m_surfaceParamsCsc.psOutputCopiedSurface;
        surfaceParams.dwCacheabilityControl = m_hwInterface->ComposeSurfaceCacheabilityControl(
            MOS_CODEC_RESOURCE_USAGE_SURFACE_HME_DOWNSAMPLED_ENCODE,
            codechalLLC);

        surfaceParams.dwBindingTableOffset = cscDstConvYPlane;
        surfaceParams.dwUVBindingTableOffset = cscDstConvUVlane;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceParams,
            m_cscKernelState));
    }

    // 4x downscaled surface
    if (m_surfaceParamsCsc.psOutput4xDsSurface)
    {
        MOS_ZeroMemory(&surfaceParams, sizeof(surfaceParams));
        surfaceParams.bIs2DSurface =
            surfaceParams.bMediaBlockRW =
            surfaceParams.bIsWritable = true;
        surfaceParams.psSurface = m_surfaceParamsCsc.psOutput4xDsSurface;
        surfaceParams.dwCacheabilityControl = m_hwInterface->ComposeSurfaceCacheabilityControl(
            MOS_CODEC_RESOURCE_USAGE_SURFACE_HME_DOWNSAMPLED_ENCODE,
            codechalLLC);
        surfaceParams.dwBindingTableOffset = cscDst4xDs;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceParams,
            m_cscKernelState));
    }

    // MB Stats surface
    if (m_surfaceParamsCsc.presMBVProcStatsBuffer)
    {
        MOS_ZeroMemory(&surfaceParams, sizeof(surfaceParams));
        surfaceParams.dwSize = m_hwInterface->m_avcMbStatBufferSize;
        surfaceParams.bIsWritable = true;
        surfaceParams.presBuffer = m_surfaceParamsCsc.presMBVProcStatsBuffer;
        surfaceParams.dwCacheabilityControl = m_hwInterface->ComposeSurfaceCacheabilityControl(
            MOS_CODEC_RESOURCE_USAGE_MB_STATS_ENCODE,
            codechalLLC);
        surfaceParams.dwBindingTableOffset = cscDstMbStats;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceParams,
            m_cscKernelState));
    }

    // 2x downscaled surface
    if (m_surfaceParamsCsc.psOutput2xDsSurface)
    {
        MOS_ZeroMemory(&surfaceParams, sizeof(surfaceParams));
        surfaceParams.bIs2DSurface =
            surfaceParams.bMediaBlockRW =
            surfaceParams.bIsWritable = true;
        surfaceParams.psSurface = m_surfaceParamsCsc.psOutput2xDsSurface;
        surfaceParams.dwCacheabilityControl = m_hwInterface->ComposeSurfaceCacheabilityControl(
            MOS_CODEC_RESOURCE_USAGE_SURFACE_HME_DOWNSAMPLED_ENCODE,
            codechalLLC);
        surfaceParams.dwBindingTableOffset = cscDst2xDs;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceParams,
            m_cscKernelState));
    }

    if (m_surfaceParamsCsc.hevcExtParams)
    {
        auto hevcExtParams = (HevcExtKernelParams*)m_surfaceParamsCsc.hevcExtParams;

        // History buffer
        if (hevcExtParams->presHistoryBuffer)
        {
            MOS_ZeroMemory(&surfaceParams, sizeof(surfaceParams));
            surfaceParams.dwSize = hevcExtParams->dwSizeHistoryBuffer;
            surfaceParams.dwOffset = hevcExtParams->dwOffsetHistoryBuffer;
            surfaceParams.bIsWritable = true;
            surfaceParams.presBuffer = hevcExtParams->presHistoryBuffer;
            surfaceParams.dwCacheabilityControl = m_hwInterface->ComposeSurfaceCacheabilityControl(
                MOS_CODEC_RESOURCE_USAGE_SURFACE_HME_DOWNSAMPLED_ENCODE,
                codechalLLC);
            surfaceParams.dwBindingTableOffset = cscDstHistBuffer;
            CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
                m_hwInterface,
                cmdBuffer,
                &surfaceParams,
                m_cscKernelState));
        }

        // History sum output buffer
        if (hevcExtParams->presHistorySumBuffer)
        {
            MOS_ZeroMemory(&surfaceParams, sizeof(surfaceParams));
            surfaceParams.dwSize = hevcExtParams->dwSizeHistorySumBuffer;
            surfaceParams.dwOffset = hevcExtParams->dwOffsetHistorySumBuffer;
            surfaceParams.bIsWritable = true;
            surfaceParams.presBuffer = hevcExtParams->presHistorySumBuffer;
            surfaceParams.dwCacheabilityControl = m_hwInterface->ComposeSurfaceCacheabilityControl(
                MOS_CODEC_RESOURCE_USAGE_SURFACE_HME_DOWNSAMPLED_ENCODE,
                codechalLLC);
            surfaceParams.dwBindingTableOffset = cscDstHistSum;
            CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
                m_hwInterface,
                cmdBuffer,
                &surfaceParams,
                m_cscKernelState));
        }

        // multi-thread task buffer
        if (hevcExtParams->presMultiThreadTaskBuffer)
        {
            MOS_ZeroMemory(&surfaceParams, sizeof(surfaceParams));
            surfaceParams.dwSize = hevcExtParams->dwSizeMultiThreadTaskBuffer;
            surfaceParams.dwOffset = hevcExtParams->dwOffsetMultiThreadTaskBuffer;
            surfaceParams.bIsWritable = true;
            surfaceParams.presBuffer = hevcExtParams->presMultiThreadTaskBuffer;
            surfaceParams.dwCacheabilityControl = m_hwInterface->ComposeSurfaceCacheabilityControl(
                MOS_CODEC_RESOURCE_USAGE_SURFACE_HME_DOWNSAMPLED_ENCODE,
                codechalLLC);
            surfaceParams.dwBindingTableOffset = cscDstMultiTask;
            CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
                m_hwInterface,
                cmdBuffer,
                &surfaceParams,
                m_cscKernelState));
        }
    }

    return eStatus;
}

MOS_STATUS CodechalEncodeCscDsG12::InitKernelStateDS()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    m_dsBTCount[0] = ds4xNumSurfaces;
    m_dsCurbeLength[0] =
    m_dsInlineDataLength = sizeof(Ds4xKernelCurbeData);
    m_dsBTISrcY = ds4xSrcYPlane;
    m_dsBTIDstY = ds4xDstYPlane;
    m_dsBTISrcYTopField = ds4xSrcYPlaneTopField;
    m_dsBTIDstYTopField = ds4xDstYPlaneTopField;
    m_dsBTISrcYBtmField = ds4xSrcYPlaneBtmField;
    m_dsBTIDstYBtmField = ds4xDstYPlaneBtmField;
    m_dsBTIDstMbVProc = ds4xDstMbVProc;
    m_dsBTIDstMbVProcTopField = ds4xDstMbVProcTopField;
    m_dsBTIDstMbVProcBtmField = ds4xDstMbVProcBtmField;

    uint32_t kernelSize, numKernelsToLoad = m_encoder->m_interlacedFieldDisabled ? 1 : CODEC_NUM_FIELDS_PER_FRAME;
    m_dsKernelBase = m_kernelBase;
    CODECHAL_KERNEL_HEADER currKrnHeader;
    for (uint32_t krnStateIdx = 0; krnStateIdx < numKernelsToLoad; krnStateIdx++)
    {
        kernelSize = m_combinedKernelSize;
        m_dsKernelState = &m_encoder->m_scaling4xKernelStates[krnStateIdx];

        CODECHAL_ENCODE_CHK_STATUS_RETURN(GetCommonKernelHeaderAndSizeG12(
            m_dsKernelBase,
            ENC_SCALING4X,
            krnStateIdx,
            &currKrnHeader,
            &kernelSize))

        m_dsKernelState->KernelParams.iBTCount = m_dsBTCount[0];
        m_dsKernelState->KernelParams.iThreadCount = m_renderInterface->GetHwCaps()->dwMaxThreads;
        m_dsKernelState->KernelParams.iCurbeLength = m_dsCurbeLength[0];
        m_dsKernelState->KernelParams.iBlockWidth = CODECHAL_MACROBLOCK_WIDTH;
        m_dsKernelState->KernelParams.iBlockHeight = CODECHAL_MACROBLOCK_HEIGHT;
        m_dsKernelState->KernelParams.iIdCount = 1;
        m_dsKernelState->KernelParams.iInlineDataLength = m_dsInlineDataLength;

        m_dsKernelState->dwCurbeOffset = m_stateHeapInterface->GetSizeofCmdInterfaceDescriptorData();
        m_dsKernelState->KernelParams.pBinary = m_dsKernelBase + (currKrnHeader.KernelStartPointer << MHW_KERNEL_OFFSET_SHIFT);
        m_dsKernelState->KernelParams.iSize = kernelSize;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->CalculateSshAndBtSizesRequested(
            m_dsKernelState->KernelParams.iBTCount,
            &m_dsKernelState->dwSshSize,
            &m_dsKernelState->dwBindingTableSize));

        CODECHAL_ENCODE_CHK_NULL_RETURN(m_renderInterface->m_stateHeapInterface);
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->MhwInitISH(m_renderInterface->m_stateHeapInterface, m_dsKernelState));

        if (m_32xMeSupported)
        {
            m_dsKernelState = &m_encoder->m_scaling2xKernelStates[krnStateIdx];

            CODECHAL_ENCODE_CHK_STATUS_RETURN(GetCommonKernelHeaderAndSizeG12(
                m_dsKernelBase,
                ENC_SCALING2X,
                krnStateIdx,
                &currKrnHeader,
                &kernelSize))

            m_dsKernelState->KernelParams.iBTCount = m_dsBTCount[1];
            m_dsKernelState->KernelParams.iThreadCount = m_renderInterface->GetHwCaps()->dwMaxThreads;
            m_dsKernelState->KernelParams.iCurbeLength = m_dsCurbeLength[1];
            m_dsKernelState->KernelParams.iBlockWidth = CODECHAL_MACROBLOCK_WIDTH;
            m_dsKernelState->KernelParams.iBlockHeight = CODECHAL_MACROBLOCK_HEIGHT;

            m_dsKernelState->dwCurbeOffset = m_stateHeapInterface->GetSizeofCmdInterfaceDescriptorData();
            m_dsKernelState->KernelParams.pBinary = m_dsKernelBase + (currKrnHeader.KernelStartPointer << MHW_KERNEL_OFFSET_SHIFT);
            m_dsKernelState->KernelParams.iSize = kernelSize;
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->CalculateSshAndBtSizesRequested(
                m_dsKernelState->KernelParams.iBTCount,
                &m_dsKernelState->dwSshSize,
                &m_dsKernelState->dwBindingTableSize));

            CODECHAL_ENCODE_CHK_NULL_RETURN(m_renderInterface->m_stateHeapInterface);
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->MhwInitISH(m_renderInterface->m_stateHeapInterface, m_dsKernelState));
        }

        if (m_encoder->m_interlacedFieldDisabled)
        {
            m_encoder->m_scaling4xKernelStates[1] = m_encoder->m_scaling4xKernelStates[0];

            if (m_32xMeSupported)
            {
                m_encoder->m_scaling2xKernelStates[1] = m_encoder->m_scaling2xKernelStates[0];
            }
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalEncodeCscDsG12::SetCurbeDS4x()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    if (CODECHAL_AVC != m_standard)
    {
        return CodechalEncodeCscDs::SetCurbeDS4x();
    }

    Ds4xKernelCurbeData curbe;

    curbe.DW0_InputPictureWidth = m_curbeParams.dwInputPictureWidth;
    curbe.DW0_InputPictureHeight = m_curbeParams.dwInputPictureHeight;

    curbe.DW1_InputYBTIFrame = ds4xSrcYPlane;
    curbe.DW2_OutputYBTIFrame = ds4xDstYPlane;

    if (m_curbeParams.bFieldPicture)
    {
        curbe.DW3_InputYBTIBottomField = ds4xSrcYPlaneBtmField;
        curbe.DW4_OutputYBTIBottomField = ds4xDstYPlaneBtmField;
    }

    if ((curbe.DW6_EnableMBFlatnessCheck = m_curbeParams.bFlatnessCheckEnabled))
    {
        curbe.DW5_FlatnessThreshold = 128;
    }

    // For gen10 DS kernel, If Flatness Check enabled, need enable MBVariance as well. Otherwise will not output MbIsFlat.
    curbe.DW6_EnableMBVarianceOutput = curbe.DW6_EnableMBFlatnessCheck || m_curbeParams.bMBVarianceOutputEnabled;
    curbe.DW6_EnableMBPixelAverageOutput = m_curbeParams.bMBPixelAverageOutputEnabled;
    curbe.DW6_EnableBlock8x8StatisticsOutput = m_curbeParams.bBlock8x8StatisticsEnabled;

    if (curbe.DW6_EnableMBVarianceOutput || curbe.DW6_EnableMBPixelAverageOutput)
    {
        curbe.DW8_MBVProcStatsBTIFrame = ds4xDstMbVProc;

        if (m_curbeParams.bFieldPicture)
        {
            curbe.DW9_MBVProcStatsBTIBottomField = ds4xDstMbVProcBtmField;
        }
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_dsKernelState->m_dshRegion.AddData(
        &curbe,
        m_dsKernelState->dwCurbeOffset,
        sizeof(curbe)));

    CODECHAL_DEBUG_TOOL(
        if (m_encoder->m_encodeParState)
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_encoder->m_encodeParState->PopulateDsParam(&curbe));
        }
    )

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalEncodeCscDsG12::InitSfcState()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    if (!m_sfcState)
    {
        m_sfcState = (CodecHalEncodeSfc*)MOS_New(CodecHalEncodeSfcG12);
        CODECHAL_ENCODE_CHK_NULL_RETURN(m_sfcState);

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_sfcState->Initialize(m_hwInterface, m_osInterface));

        m_sfcState->SetInputColorSpace(MHW_CSpace_sRGB);
    }
    return MOS_STATUS_SUCCESS;
}

CodechalEncodeCscDsG12::CodechalEncodeCscDsG12(CodechalEncoderState* encoder)
    : CodechalEncodeCscDs(encoder)
{
    m_cscKernelUID = IDR_CODEC_HME_DS_SCOREBOARD_KERNEL;
    m_cscCurbeLength = sizeof(CscKernelCurbeData);
#if defined(ENABLE_KERNELS) && !defined(_FULL_OPEN_SOURCE)
    m_kernelBase = (uint8_t*)IGCODECKRN_G12;
#endif
}

CodechalEncodeCscDsG12::~CodechalEncodeCscDsG12()
{
    // free the MbStats surface
    m_osInterface->pfnFreeResource(m_osInterface, &m_resMbStatsBuffer);
}
