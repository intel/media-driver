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
//! \file     codechal_decode_vp9_g12.cpp
//! \brief    Implements the decode interface extension for Gen12 VP9.
//! \details  Implements all functions required by CodecHal for Gen12 VP9 decoding.
//!

#include "codechal_decoder.h"
#include "codechal_secure_decode_interface.h"
#include "codechal_decode_vp9_g12.h"
#include "codechal_decode_sfc_vp9_g12.h"
#include "codechal_mmc_decode_vp9_g12.h"
#include "mhw_vdbox_hcp_g12_X.h"
#include "mhw_vdbox_mfx_g12_X.h"
#include "mhw_vdbox_g12_X.h"
#include "codechal_hw_g12_X.h"
#include "codechal_decode_histogram.h"
#include "mhw_mi_g12_X.h"
#include "hal_oca_interface.h"

CodechalDecodeVp9G12 ::  ~CodechalDecodeVp9G12()
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
     //Note: virtual engine interface destroy is done in MOS layer
  #ifdef _DECODE_PROCESSING_SUPPORTED
     if (m_sfcState)
     {
         MOS_Delete(m_sfcState);
         m_sfcState = nullptr;
     }
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
}

CodechalDecodeVp9G12::CodechalDecodeVp9G12(
    CodechalHwInterface *   hwInterface,
    CodechalDebugInterface *debugInterface,
    PCODECHAL_STANDARD_INFO standardInfo) : CodechalDecodeVp9(hwInterface, debugInterface, standardInfo),
                                            m_frameSizeMaxAlloced(0),
                                            m_sinlgePipeVeState(nullptr),
                                            m_scalabilityState(nullptr)
{
    CODECHAL_DECODE_FUNCTION_ENTER;

    CODECHAL_DECODE_CHK_NULL_NO_STATUS_RETURN(m_osInterface);

    Mos_CheckVirtualEngineSupported(m_osInterface, true, true);
}

MOS_STATUS CodechalDecodeVp9G12::SetGpuCtxCreatOption(
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
                m_videoContext = MOS_VE_MULTINODESCALING_SUPPORTED(m_osInterface) ? MOS_GPU_CONTEXT_VIDEO5 : MOS_GPU_CONTEXT_VDBOX2_VIDEO;

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

MOS_STATUS CodechalDecodeVp9G12 :: AllocateResourcesVariableSizes()
{
    MOS_STATUS              eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    CODECHAL_DECODE_CHK_STATUS_RETURN(CodechalDecodeVp9 :: AllocateResourcesVariableSizes());

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

    if (CodecHalDecodeScalabilityIsScalableMode(m_scalabilityState))
    {
        uint32_t widthInSb  = MOS_ROUNDUP_DIVIDE(m_width, CODEC_VP9_SUPER_BLOCK_WIDTH);
        uint32_t heightInSb = MOS_ROUNDUP_DIVIDE(m_height, CODEC_VP9_SUPER_BLOCK_HEIGHT);
        uint32_t frameSizeMax = MOS_MAX((m_copyDataBufferInUse ? m_copyDataBufferSize : m_dataSize), m_frameSizeMaxAlloced);
        uint8_t  maxBitDepth  = 8 + m_vp9DepthIndicator * 2;
        uint8_t  chromaFormat = m_chromaFormatinProfile;

        MHW_VDBOX_HCP_BUFFER_SIZE_PARAMS    hcpBufSizeParam;
        MOS_ZeroMemory(&hcpBufSizeParam, sizeof(hcpBufSizeParam));
        hcpBufSizeParam.ucMaxBitDepth  = maxBitDepth;
        hcpBufSizeParam.ucChromaFormat = chromaFormat;
        hcpBufSizeParam.dwPicWidth     = widthInSb;
        hcpBufSizeParam.dwPicHeight    = heightInSb;
        hcpBufSizeParam.dwMaxFrameSize = frameSizeMax;

        MHW_VDBOX_HCP_BUFFER_REALLOC_PARAMS reallocParam;
        MOS_ZeroMemory(&reallocParam, sizeof(reallocParam));
        reallocParam.ucMaxBitDepth       = maxBitDepth;
        reallocParam.ucChromaFormat      = chromaFormat;
        reallocParam.dwPicWidth          = widthInSb;
        reallocParam.dwPicWidthAlloced   = m_allocatedWidthInSb;
        reallocParam.dwPicHeight         = heightInSb;
        reallocParam.dwPicHeightAlloced  = m_allocatedHeightInSb;
        reallocParam.dwFrameSize         = frameSizeMax;
        reallocParam.dwFrameSizeAlloced  = m_frameSizeMaxAlloced;
        CODECHAL_DECODE_CHK_STATUS_RETURN(
            CodecHalDecodeScalability_AllocateResources_VariableSizes_G12(
                m_scalabilityState,
                &hcpBufSizeParam,
                &reallocParam));

        m_frameSizeMaxAlloced = frameSizeMax;
    }

    return eStatus;
}

MOS_STATUS CodechalDecodeVp9G12::InitSfcState()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;
#ifdef _DECODE_PROCESSING_SUPPORTED
    // Check if SFC can be supported
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_sfcState->CheckAndInitialize(
        (CODECHAL_DECODE_PROCESSING_PARAMS *)m_decodeParams.m_procParams, 
        m_vp9PicParams, 
        m_scalabilityState));
#endif
    return eStatus;
}

MOS_STATUS CodechalDecodeVp9G12::CalcDownsamplingParams(
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

    PCODEC_VP9_PIC_PARAMS vp9PicParams = (PCODEC_VP9_PIC_PARAMS)picParams;

    *refSurfWidth = 0;
    *refSurfHeight = 0;
    *format = Format_NV12;
    *frameIdx = vp9PicParams->CurrPic.FrameIdx;


    *refSurfWidth = MOS_ALIGN_CEIL(vp9PicParams->FrameWidthMinus1 + 1, CODEC_VP9_SUPER_BLOCK_WIDTH);
    *refSurfHeight = MOS_ALIGN_CEIL(vp9PicParams->FrameHeightMinus1 + 1, CODEC_VP9_SUPER_BLOCK_HEIGHT);

    if (vp9PicParams->subsampling_x == 1 && vp9PicParams->subsampling_y == 1) //HCP_CHROMA_FORMAT_YUV420
    {
        if (vp9PicParams->BitDepthMinus8 > 2)
        {
            *format = Format_P016;
        }
        else if (vp9PicParams->BitDepthMinus8 > 0)
        {
            *format = Format_P010;
        }
        else
        {
            *format = Format_NV12;
        }
    }
    else if (vp9PicParams->subsampling_x == 0 && vp9PicParams->subsampling_y == 0) //HCP_CHROMA_FORMAT_YUV444
    {
        if (vp9PicParams->BitDepthMinus8 > 2)
        {
            *format = Format_Y416;
        }
        else if (vp9PicParams->BitDepthMinus8 > 0)
        {
            *format = Format_Y410;
        }
        else
        {
            *format = Format_AYUV;
        }
    }
    else
    {
        CODECHAL_DECODE_ASSERTMESSAGE("Invalid Chroma sampling format!");
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        return eStatus;
    }
    return eStatus;
}

MOS_STATUS CodechalDecodeVp9G12 :: InitializeDecodeMode ()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    if ( MOS_VE_SUPPORTED(m_osInterface) && static_cast<MhwVdboxMfxInterfaceG12*>(m_mfxInterface)->IsScalabilitySupported())
    {
        CODECHAL_DECODE_SCALABILITY_INIT_PARAMS_G12  initParams;

        MOS_ZeroMemory(&initParams, sizeof(initParams));
        initParams.u32PicWidthInPixel  = m_usFrameWidthAlignedMinBlk;
        initParams.u32PicHeightInPixel = m_usFrameHeightAlignedMinBlk;
        initParams.format              = m_decodeParams.m_destSurface->Format;
        initParams.gpuCtxInUse         = GetVideoContext();
        initParams.usingSecureDecode   = m_secureDecoder ? m_secureDecoder->IsSecureDecodeEnabled() : false;

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

    }

    return eStatus;
}

MOS_STATUS CodechalDecodeVp9G12::DetermineDecodePhase()
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
        CODECHAL_DECODE_CHK_STATUS_RETURN(CodechalDecodeVp9 :: DetermineDecodePhase());
    }

    return eStatus;
}

MOS_STATUS CodechalDecodeVp9G12::SetAndPopulateVEHintParams(
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

MOS_STATUS CodechalDecodeVp9G12::DetermineSendProlgwithFrmTracking(
    bool                        *sendPrologWithFrameTracking)
{
    MOS_STATUS       eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    CODECHAL_DECODE_CHK_NULL_RETURN(sendPrologWithFrameTracking);

    if (CodecHalDecodeScalabilityIsScalableMode(m_scalabilityState))
    {
        if (CodecHalDecodeScalability1stPhaseofSubmission(m_scalabilityState))
        {
            *sendPrologWithFrameTracking = true;
        }
    }
    else
    {
        *sendPrologWithFrameTracking = true;
    }

    return eStatus;
}

uint32_t CodechalDecodeVp9G12::RequestedSpaceSize(uint32_t requestedSize)
{
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

MOS_STATUS CodechalDecodeVp9G12::VerifyExtraSpace(
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

MOS_STATUS CodechalDecodeVp9G12::AllocateHistogramSurface()
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

MOS_STATUS CodechalDecodeVp9G12::AddPicStateMhwCmds(
    PMOS_COMMAND_BUFFER       cmdBuffer)
{
    MOS_STATUS                              eStatus = MOS_STATUS_SUCCESS;
    MHW_MI_VD_CONTROL_STATE_PARAMS          vdCtrlParam;

    CODECHAL_DECODE_FUNCTION_ENTER;

    CODECHAL_DECODE_CHK_NULL_RETURN(cmdBuffer);

    // Send VD_CONTROL_STATE Pipe Initialization
    MOS_ZeroMemory(&vdCtrlParam, sizeof(MHW_MI_VD_CONTROL_STATE_PARAMS));
    vdCtrlParam.initialization = true;
    static_cast<MhwMiInterfaceG12*>(m_miInterface)->AddMiVdControlStateCmd(cmdBuffer, &vdCtrlParam);

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpPipeModeSelectCmd(
        cmdBuffer,
        m_picMhwParams.PipeModeSelectParams));

    if (CodecHalDecodeScalabilityIsScalableMode(m_scalabilityState) &&
        CodecHalDecodeScalabilityIsBEPhaseG12(m_scalabilityState))
    {
        // Send VD_CONTROL_STATE HcpPipeLock
        MOS_ZeroMemory(&vdCtrlParam, sizeof(MHW_MI_VD_CONTROL_STATE_PARAMS));
        vdCtrlParam.scalableModePipeLock = true;
        static_cast<MhwMiInterfaceG12*>(m_miInterface)->AddMiVdControlStateCmd(cmdBuffer, &vdCtrlParam);
    }
#ifdef _DECODE_PROCESSING_SUPPORTED
    if (!CodecHalDecodeScalabilityIsFEPhase(m_scalabilityState))
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_sfcState->AddSfcCommands(cmdBuffer));
    }
#endif
#ifdef _MMC_SUPPORTED
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_mmc->SetSurfaceState(m_picMhwParams.SurfaceParams[0]));
#endif
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpSurfaceCmd(
        cmdBuffer,
        m_picMhwParams.SurfaceParams[0]));

    // For non-key frame, send extra surface commands for reference pictures
    if (m_vp9PicParams->PicFlags.fields.frame_type == CODEC_VP9_INTER_FRAME &&
        !m_vp9PicParams->PicFlags.fields.intra_only)
    {
        for (uint8_t i = 1; i < 4; i++)
        {
#ifdef _MMC_SUPPORTED
            CODECHAL_DECODE_CHK_STATUS_RETURN(m_mmc->SetSurfaceState(m_picMhwParams.SurfaceParams[i]));
#endif
            CODECHAL_DECODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpSurfaceCmd(
                cmdBuffer,
                m_picMhwParams.SurfaceParams[i]));
        }
    }

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpPipeBufAddrCmd(
        cmdBuffer,
        m_picMhwParams.PipeBufAddrParams));

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpIndObjBaseAddrCmd(
        cmdBuffer,
        m_picMhwParams.IndObjBaseAddrParams));

    if (m_cencBuf)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(SetCencBatchBuffer(cmdBuffer));
    }
    else
    {
        for (uint8_t i = 0; i < CODEC_VP9_MAX_SEGMENTS; i++)
        {
            // Error handling for illegal programming on segmentation fields @ KEY/INTRA_ONLY frames
            PCODEC_VP9_SEG_PARAMS vp9SegData = &(m_picMhwParams.Vp9SegmentState->pVp9SegmentParams->SegData[i]);
            if (vp9SegData->SegmentFlags.fields.SegmentReferenceEnabled &&
                (!m_vp9PicParams->PicFlags.fields.frame_type || m_vp9PicParams->PicFlags.fields.intra_only))
            {
                vp9SegData->SegmentFlags.fields.SegmentReference = CODECHAL_DECODE_VP9_INTRA_FRAME;
            }

            m_picMhwParams.Vp9SegmentState->ucCurrentSegmentId = i;
            CODECHAL_DECODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpVp9SegmentStateCmd(
                cmdBuffer,
                nullptr,
                m_picMhwParams.Vp9SegmentState));

            if (m_vp9PicParams->PicFlags.fields.segmentation_enabled == 0)
            {
                break;
            }
        }

        CODECHAL_DECODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpVp9PicStateCmd(
            cmdBuffer,
            nullptr,
            m_picMhwParams.Vp9PicState));

        if (m_secureDecoder)
        {
            // Add secure decode command
            CODECHAL_DECODE_CHK_STATUS_RETURN(m_secureDecoder->AddHcpSecureState(
                cmdBuffer,
                this));
        }
    }
    return eStatus;
}

MOS_STATUS CodechalDecodeVp9G12::SetFrameStates()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;


#ifdef _DECODE_PROCESSING_SUPPORTED
    if (m_decodeParams.m_procParams)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(AllocateHistogramSurface());

        ((CODECHAL_DECODE_PROCESSING_PARAMS*)m_decodeParams.m_procParams)->pHistogramSurface = m_histogramSurface;

        if(m_decodeHistogram)
            m_decodeHistogram->SetSrcHistogramSurface(m_histogramSurface);

    }
#endif

    CodechalDecodeVp9::SetFrameStates();

    return eStatus;
}

MOS_STATUS CodechalDecodeVp9G12 :: DecodeStateLevel()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    PERF_UTILITY_AUTO(__FUNCTION__, PERF_DECODE, PERF_LEVEL_HAL);
    CODECHAL_DECODE_FUNCTION_ENTER;

    CODECHAL_DECODE_CHK_NULL_RETURN(m_hwInterface->GetCpInterface());

    if (m_secureDecoder && m_hcpDecPhase == CodechalHcpDecodePhaseInitialized)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_secureDecoder->Execute(this));
    }

    //HCP Decode Phase State Machine
    DetermineDecodePhase();

    if (CodecHalDecodeScalabilityIsScalableMode(m_scalabilityState))
    {
        //Switch GPU context when necessary
        CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHalDecodeScalability_SwitchGpuContext(m_scalabilityState));
    }

    MOS_COMMAND_BUFFER primCmdBuffer;
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnGetCommandBuffer(m_osInterface, &primCmdBuffer, 0));

    bool sendPrologWithFrameTracking;
    CODECHAL_DECODE_CHK_STATUS_RETURN(DetermineSendProlgwithFrmTracking(&sendPrologWithFrameTracking));
    if (sendPrologWithFrameTracking)
    {
        if (!CodecHalDecodeScalabilityIsScalableMode(m_scalabilityState))
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
        }
        //Frame tracking functionality is called at the start of primary command buffer.
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

        MHW_MI_FORCE_WAKEUP_PARAMS forceWakeupParams;
        MOS_ZeroMemory(&forceWakeupParams, sizeof(MHW_MI_FORCE_WAKEUP_PARAMS));
        forceWakeupParams.bMFXPowerWellControl = false;
        forceWakeupParams.bMFXPowerWellControlMask = true;
        forceWakeupParams.bHEVCPowerWellControl = true;
        forceWakeupParams.bHEVCPowerWellControlMask = true;
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiForceWakeupCmd(
            cmdBufferInUse,
            &forceWakeupParams));

        if (cmdBufferInUse == &scdryCmdBuffer)
        {
            CODECHAL_DECODE_CHK_STATUS_RETURN(SendPrologWithFrameTracking(cmdBufferInUse, false));
        }

        HalOcaInterface::On1stLevelBBStart(scdryCmdBuffer, *m_osInterface->pOsContext, m_osInterface->CurrentGpuContextHandle, *m_miInterface, *mmioRegisters);

    }
    else
    {
        HalOcaInterface::On1stLevelBBStart(primCmdBuffer, *m_osInterface->pOsContext, m_osInterface->CurrentGpuContextHandle, *m_miInterface, *mmioRegisters);
    }

    auto pipeModeSelectParams =
        static_cast<PMHW_VDBOX_PIPE_MODE_SELECT_PARAMS_G12>(m_picMhwParams.PipeModeSelectParams);
    *pipeModeSelectParams = {};
    auto pipeBufAddrParams =
        static_cast<PMHW_VDBOX_PIPE_BUF_ADDR_PARAMS_G12>(m_picMhwParams.PipeBufAddrParams);
    *pipeBufAddrParams = {};
    CODECHAL_DECODE_CHK_STATUS_RETURN(InitPicStateMhwParams());

    bool secureDecodeStartStatusFlag = true;
    if (CodecHalDecodeScalabilityIsScalableMode(m_scalabilityState))
    {
        // Put all SecureDecode/Start Status related cmd into FE VDBOX
        secureDecodeStartStatusFlag = CodecHalDecodeScalabilityIsFEPhase(m_scalabilityState);
       
        CodecHalDecodeScalablity_DecPhaseToHwWorkMode_G12(
            pipeModeSelectParams->MultiEngineMode,
            pipeModeSelectParams->PipeWorkMode);

        pipeBufAddrParams->presCABACSyntaxStreamOutBuffer =
            m_scalabilityState->presCABACStreamOutBuffer;
        pipeBufAddrParams->presIntraPredUpRightColStoreBuffer =
            &m_scalabilityState->resIntraPredUpRightColStoreBuffer;
        pipeBufAddrParams->presIntraPredLeftReconColStoreBuffer =
            &m_scalabilityState->resIntraPredLeftReconColStoreBuffer;
    }

    if (secureDecodeStartStatusFlag)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(UpdatePicStateBuffers(cmdBufferInUse));

        if (m_statusQueryReportingEnabled)
        {
            CODECHAL_DECODE_CHK_STATUS_RETURN(StartStatusReport(
                cmdBufferInUse));
        }
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

    if (CodecHalDecodeScalabilityIsBEPhaseG12(m_scalabilityState))
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddWatchdogTimerStartCmd(cmdBufferInUse));
    }

    CODECHAL_DECODE_CHK_STATUS_RETURN(AddPicStateMhwCmds(
        cmdBufferInUse));

    if (CodecHalDecodeScalabilityIsBEPhaseG12(m_scalabilityState))
    {
        MHW_VDBOX_HCP_TILE_CODING_PARAMS_G12  hcpTileCodingParam;
        CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHalDecodeScalability_CalculateHcpTileCodingParams<MHW_VDBOX_HCP_TILE_CODING_PARAMS_G12>(
            m_scalabilityState,
            m_vp9PicParams,
            &hcpTileCodingParam));
        CODECHAL_DECODE_CHK_STATUS_RETURN(static_cast<MhwVdboxHcpInterfaceG12*>(m_hcpInterface)->AddHcpTileCodingCmd(
            cmdBufferInUse,
            &hcpTileCodingParam));
    }

    m_osInterface->pfnReturnCommandBuffer(m_osInterface, &primCmdBuffer, 0);
    if (CodecHalDecodeScalabilityIsScalableMode(m_scalabilityState) && MOS_VE_SUPPORTED(m_osInterface))
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHalDecodeScalability_ReturnSdryCmdBuffer_G12(m_scalabilityState, &scdryCmdBuffer));
    }

    return eStatus;
}

MOS_STATUS CodechalDecodeVp9G12 :: DecodePrimitiveLevel()
{
    MOS_STATUS                          eStatus = MOS_STATUS_SUCCESS;
    MHW_MI_VD_CONTROL_STATE_PARAMS      vdCtrlParam;

    PERF_UTILITY_AUTO(__FUNCTION__, PERF_DECODE, PERF_LEVEL_HAL);

    CODECHAL_DECODE_FUNCTION_ENTER;

    // Bitstream is incomplete, don't do any decoding work.
    if (m_incompletePicture)
    {
        eStatus = MOS_STATUS_SUCCESS;
        return eStatus;
    }

    CODECHAL_DECODE_CHK_NULL_RETURN(m_osInterface);

    CODECHAL_DECODE_CHK_COND_RETURN(
        (m_vdboxIndex > m_mfxInterface->GetMaxVdboxIndex()),
        "ERROR - vdbox index exceed the maximum");

    m_osInterface->pfnSetPerfTag(
        m_osInterface,
        (uint16_t)(((m_mode << 4) & 0xF0) | (m_perfType & 0xF)));
    m_osInterface->pfnResetPerfBufferID(m_osInterface);

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

    // store CS ENGINE ID register
    if (static_cast<MhwVdboxMfxInterfaceG12*>(m_mfxInterface)->IsScalabilitySupported())
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHalDecodeScalability_ReadCSEngineIDReg_G12(
            m_scalabilityState,
            &m_decodeStatusBuf,
            cmdBufferInUse));
    }

    //no slice level command for Huc based DRM and scalability decode BE phases
    if (m_cencBuf == nullptr && !CodecHalDecodeScalabilityIsBEPhaseG12(m_scalabilityState))
    {
        MHW_VDBOX_HCP_BSD_PARAMS bsdParams;
        MOS_ZeroMemory(&bsdParams, sizeof(bsdParams));
        bsdParams.dwBsdDataLength =
            m_vp9PicParams->BSBytesInBuffer - m_vp9PicParams->UncompressedHeaderLengthInBytes;
        bsdParams.dwBsdDataStartOffset = m_vp9PicParams->UncompressedHeaderLengthInBytes;

        CODECHAL_DECODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpBsdObjectCmd(
            cmdBufferInUse,
            &bsdParams));
    }

    // Send VD_CONTROL_STATE Memory Implict Flush
    MOS_ZeroMemory(&vdCtrlParam, sizeof(MHW_MI_VD_CONTROL_STATE_PARAMS));
    vdCtrlParam.memoryImplicitFlush = true;
    static_cast<MhwMiInterfaceG12*>(m_miInterface)->AddMiVdControlStateCmd(cmdBufferInUse, &vdCtrlParam);

    if (CodecHalDecodeScalabilityIsScalableMode(m_scalabilityState) &&
        CodecHalDecodeScalabilityIsBEPhaseG12(m_scalabilityState))
    {
        // Send VD_CONTROL_STATE HCP Pipe Unlock
        MOS_ZeroMemory(&vdCtrlParam, sizeof(MHW_MI_VD_CONTROL_STATE_PARAMS));
        vdCtrlParam.scalableModePipeUnlock = true;
        static_cast<MhwMiInterfaceG12*>(m_miInterface)->AddMiVdControlStateCmd(cmdBufferInUse, &vdCtrlParam);
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

    //if scalable decode,  BE0 finish means whole frame complete.
    // Check if destination surface needs to be synchronized
    bool syncDestSurface = true;
    if (CodecHalDecodeScalabilityIsScalableMode(m_scalabilityState))
    {
        syncDestSurface = CodecHalDecodeScalabilityIsLastCompletePhase(m_scalabilityState);
    }

    MOS_SYNC_PARAMS syncParams;
    if (syncDestSurface)
    {
        syncParams                          = g_cInitSyncParams;
        syncParams.GpuContext               = m_videoContext;
        syncParams.presSyncResource         = &m_destSurface.OsResource;
        syncParams.bReadOnly                = false;
        syncParams.bDisableDecodeSyncLock   = m_disableDecodeSyncLock;
        syncParams.bDisableLockForTranscode = m_disableLockForTranscode;

        CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnPerformOverlaySync(
            m_osInterface,
            &syncParams));
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnResourceWait(
            m_osInterface,
            &syncParams));

        // Update the resource tag (s/w tag) for On-Demand Sync
        m_osInterface->pfnSetResourceSyncTag(m_osInterface, &syncParams);

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

            decodeStatusReport.m_statusReportNumber = m_statusReportFeedbackNumber;
            decodeStatusReport.m_currDecodedPic     = m_vp9PicParams->CurrPic;
            decodeStatusReport.m_currDeblockedPic   = m_vp9PicParams->CurrPic;
            decodeStatusReport.m_codecStatus        = CODECHAL_STATUS_UNAVAILABLE;
            decodeStatusReport.m_numMbsAffected     = m_usFrameWidthAlignedMinBlk * m_usFrameHeightAlignedMinBlk;
            decodeStatusReport.m_currDecodedPicRes  = m_vp9RefList[m_vp9PicParams->CurrPic.FrameIdx]->resRefPic;

#ifdef _DECODE_PROCESSING_SUPPORTED
            CODECHAL_DEBUG_TOOL(
                if (m_downsampledSurfaces && m_sfcState && m_sfcState->m_sfcOutputSurface) {
                    m_downsampledSurfaces[m_vp9PicParams->CurrPic.FrameIdx].OsResource = m_sfcState->m_sfcOutputSurface->OsResource;
                    decodeStatusReport.m_currSfcOutputPicRes                           = &m_downsampledSurfaces[m_vp9PicParams->CurrPic.FrameIdx].OsResource;
                })
#endif

            // VP9 plug-in/out was NOT fully enabled; this is just to make sure driver would not crash in CodecHal_DecodeEndFrame(),
            // which requires the value of DecodeStatusReport.presCurrDecodedPic
            CODECHAL_DEBUG_TOOL(
                decodeStatusReport.m_frameType = m_perfType;
            )

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

    m_osInterface->pfnReturnCommandBuffer(m_osInterface, &primCmdBuffer, 0);
    if (CodecHalDecodeScalabilityIsScalableMode(m_scalabilityState) && MOS_VE_SUPPORTED(m_osInterface))
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHalDecodeScalability_ReturnSdryCmdBuffer_G12(m_scalabilityState, &scdryCmdBuffer));
    }

    CODECHAL_DEBUG_TOOL(

        if (CodecHalDecodeScalabilityIsScalableMode(m_scalabilityState)) {
            CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHalDecodeScalability_DbgDumpCmdBuffer_G12(
                this,
                m_scalabilityState,
                m_debugInterface,
                &primCmdBuffer));
        } else {
            CODECHAL_DECODE_CHK_STATUS_RETURN(m_debugInterface->DumpCmdBuffer(
                &primCmdBuffer,
                CODECHAL_NUM_MEDIA_STATES,
                "_DEC"));
            //CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHal_DbgReplaceAllCommands(
            //    m_debugInterface,
            //    &primCmdBuffer));
        }

        m_mmc->UpdateUserFeatureKey(&m_destSurface);)

    bool syncCompleteFrame =
            (m_copyDataBufferInUse && !m_osInterface->bSimIsActive);
    if (MOS_VE_SUPPORTED(m_osInterface) && CodecHalDecodeScalabilityIsScalableMode(m_scalabilityState))
    {
        syncCompleteFrame = syncCompleteFrame && CodecHalDecodeScalabilityIsFEPhase(m_scalabilityState);
    }

    MOS_SYNC_PARAMS copyDataSyncParams;
    if (syncCompleteFrame)
    {
        //Sync up complete frame
        copyDataSyncParams = g_cInitSyncParams;
        copyDataSyncParams.GpuContext = m_videoContextForWa;
        copyDataSyncParams.presSyncResource = &m_resSyncObjectWaContextInUse;

        CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnEngineSignal(
            m_osInterface,
            &copyDataSyncParams));

        copyDataSyncParams = g_cInitSyncParams;
        copyDataSyncParams.GpuContext = m_videoContext;
        copyDataSyncParams.presSyncResource = &m_resSyncObjectWaContextInUse;

        CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnEngineWait(
            m_osInterface,
            &copyDataSyncParams));
    }

    bool submitCommand = true;
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
        uint32_t renderingFlags = m_videoContextUsesNullHw;

        //command buffer to submit is the primary cmd buffer.
        if ( MOS_VE_SUPPORTED(m_osInterface))
        {
            CODECHAL_DECODE_CHK_STATUS_RETURN(SetAndPopulateVEHintParams(&primCmdBuffer));
        }

        if (m_osInterface->phasedSubmission
            && MOS_VE_SUPPORTED(m_osInterface)
            && CodecHalDecodeScalabilityIsScalableMode(m_scalabilityState))
        {
            CodecHalDecodeScalability_DecPhaseToSubmissionType_G12(m_scalabilityState,cmdBufferInUse);
            CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnSubmitCommandBuffer(
                m_osInterface,
                cmdBufferInUse,
                renderingFlags));
        }
        else
        {
            CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnSubmitCommandBuffer(
                m_osInterface,
                &primCmdBuffer,
                renderingFlags));
        }
    }

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

#ifdef CODECHAL_HUC_KERNEL_DEBUG
    CODECHAL_DEBUG_TOOL(
    CODECHAL_DECODE_CHK_STATUS(m_debugInterface->DumpHucRegion(
        &resHucSharedBuffer,
        0,
        CODEC_VP9_PROB_MAX_NUM_ELEM,
        15,
        "",
        false,
        1,
        CodechalHucRegionDumpType::hucRegionDumpDefault));
    )
#endif
    CODECHAL_DEBUG_TOOL(
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
            &m_resVp9SegmentIdBuffer,
            CodechalDbgAttr::attrSegId,
            "SegId",
            (m_allocatedWidthInSb * m_allocatedHeightInSb * CODECHAL_CACHELINE_SIZE)));
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
            &(m_resVp9ProbBuffer[m_frameCtxIdx]),
            CodechalDbgAttr::attrCoefProb,
            "PakHwCoeffProbs",
            CODEC_VP9_PROB_MAX_NUM_ELEM));)

    // Needs to be re-set for Linux buffer re-use scenarios
    //pVp9State->pVp9RefList[pVp9PicParams->ucCurrPicIndex]->resRefPic =
    //    pVp9State->sDestSurface.OsResource;

    // Send the signal to indicate decode completion, in case On-Demand Sync is not present
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnResourceSignal(
        m_osInterface,
        &syncParams));

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

MOS_STATUS CodechalDecodeVp9G12::InitMmcState()
{
#ifdef _MMC_SUPPORTED
    m_mmc = MOS_New(CodechalMmcDecodeVp9G12, m_hwInterface, this);
    CODECHAL_DECODE_CHK_NULL_RETURN(m_mmc);
#endif
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalDecodeVp9G12 :: AllocateStandard (
    CodechalSetting *          settings)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    CODECHAL_DECODE_CHK_NULL_RETURN(settings);

    CODECHAL_DECODE_CHK_STATUS_RETURN(InitMmcState());

    m_width                      = settings->width;
    m_height                     = settings->height;
    if (settings->lumaChromaDepth & CODECHAL_LUMA_CHROMA_DEPTH_8_BITS)
        m_vp9DepthIndicator = 0;
    if (settings->lumaChromaDepth & CODECHAL_LUMA_CHROMA_DEPTH_10_BITS)
        m_vp9DepthIndicator = 1;
    if (settings->lumaChromaDepth & CODECHAL_LUMA_CHROMA_DEPTH_12_BITS)
        m_vp9DepthIndicator = 2;
    m_chromaFormatinProfile = settings->chromaFormat;
#ifdef _DECODE_PROCESSING_SUPPORTED
    // Initialize SFC state
    m_sfcState = MOS_New(CodechalVp9SfcStateG12);
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_sfcState->InitializeSfcState(
        this,
        m_hwInterface,
        m_osInterface));
#endif
    MHW_VDBOX_STATE_CMDSIZE_PARAMS_G12   stateCmdSizeParams;
    stateCmdSizeParams.bHucDummyStream = false;
    stateCmdSizeParams.bScalableMode   = static_cast<MhwVdboxMfxInterfaceG12*>(m_mfxInterface)->IsScalabilitySupported();
    stateCmdSizeParams.bSfcInUse       = true;
    //FE has more commands than BE. To finer control of the cmd buffer size

    // Picture Level Commands
    m_hwInterface->GetHxxStateCommandSize(
        m_mode,
        &m_commandBufferSizeNeeded,
        &m_commandPatchListSizeNeeded,
        &stateCmdSizeParams);

    // Primitive Level Commands
    m_hwInterface->GetHxxPrimitiveCommandSize(
        m_mode,
        &m_standardDecodeSizeNeeded,
        &m_standardDecodePatchListSizeNeeded,
        false);

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
                false));
        }
        else
        {
            //single pipe VE initialize
            m_sinlgePipeVeState = (PCODECHAL_DECODE_SINGLEPIPE_VIRTUALENGINE_STATE)MOS_AllocAndZeroMemory(sizeof(CODECHAL_DECODE_SINGLEPIPE_VIRTUALENGINE_STATE));
            CODECHAL_DECODE_CHK_NULL_RETURN(m_sinlgePipeVeState);
            CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHalDecodeSinglePipeVE_InitInterface(m_osInterface, m_sinlgePipeVeState));
        }
    }

    CODECHAL_DECODE_CHK_STATUS_RETURN(CodechalDecodeVp9::AllocateResourcesFixedSizes());

    // Prepare Pic Params
    m_picMhwParams.PipeModeSelectParams = MOS_New(MHW_VDBOX_PIPE_MODE_SELECT_PARAMS_G12);
    m_picMhwParams.PipeBufAddrParams = MOS_New(MHW_VDBOX_PIPE_BUF_ADDR_PARAMS_G12);
    m_picMhwParams.IndObjBaseAddrParams = MOS_New(MHW_VDBOX_IND_OBJ_BASE_ADDR_PARAMS);
    m_picMhwParams.Vp9PicState = MOS_New(MHW_VDBOX_VP9_PIC_STATE);
    m_picMhwParams.Vp9SegmentState = MOS_New(MHW_VDBOX_VP9_SEGMENT_STATE);

    MOS_ZeroMemory(m_picMhwParams.IndObjBaseAddrParams, sizeof(MHW_VDBOX_IND_OBJ_BASE_ADDR_PARAMS));
    MOS_ZeroMemory(m_picMhwParams.Vp9PicState, sizeof(MHW_VDBOX_VP9_PIC_STATE));
    MOS_ZeroMemory(m_picMhwParams.Vp9SegmentState, sizeof(MHW_VDBOX_VP9_SEGMENT_STATE));

    for (uint16_t i = 0; i < 4; i++)
    {
        m_picMhwParams.SurfaceParams[i] = MOS_New(MHW_VDBOX_SURFACE_PARAMS);
        MOS_ZeroMemory(m_picMhwParams.SurfaceParams[i], sizeof(MHW_VDBOX_SURFACE_PARAMS));
    }

    return eStatus;
}

MOS_STATUS CodechalDecodeVp9G12::SetCencBatchBuffer(
    PMOS_COMMAND_BUFFER cmdBuffer)
{
    CODECHAL_DECODE_CHK_NULL_RETURN(cmdBuffer);

    MHW_BATCH_BUFFER        batchBuffer;
    MOS_ZeroMemory(&batchBuffer, sizeof(MHW_BATCH_BUFFER));
    MOS_RESOURCE *resHeap = nullptr;
    CODECHAL_DECODE_CHK_NULL_RETURN(resHeap = m_cencBuf->secondLvlBbBlock->GetResource());

    batchBuffer.OsResource   = *resHeap;
    batchBuffer.dwOffset     = m_cencBuf->secondLvlBbBlock->GetOffset() + VP9_CENC_PRIMITIVE_CMD_OFFSET_IN_DW * 4;
    batchBuffer.iSize        = m_cencBuf->secondLvlBbBlock->GetSize() - VP9_CENC_PRIMITIVE_CMD_OFFSET_IN_DW * 4;
    batchBuffer.bSecondLevel = true;
#if (_DEBUG || _RELEASE_INTERNAL)
    batchBuffer.iLastCurrent = batchBuffer.iSize;
#endif  // (_DEBUG || _RELEASE_INTERNAL)

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferStartCmd(
        cmdBuffer,
        &batchBuffer));

    CODECHAL_DEBUG_TOOL(
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_debugInterface->Dump2ndLvlBatch(
            &batchBuffer,
            CODECHAL_NUM_MEDIA_STATES,
            "_2ndLvlBatch_Pic_Cmd"));)

    // Primitive level cmds should not be in BE phases
    if (!(CodecHalDecodeScalabilityIsBEPhaseG12(m_scalabilityState) && MOS_VE_SUPPORTED(m_osInterface)))
    {
        batchBuffer.dwOffset = m_cencBuf->secondLvlBbBlock->GetOffset();
        batchBuffer.iSize = VP9_CENC_PRIMITIVE_CMD_OFFSET_IN_DW * 4;
        batchBuffer.bSecondLevel = true;
#if (_DEBUG || _RELEASE_INTERNAL)
        batchBuffer.iLastCurrent = batchBuffer.iSize;
#endif  // (_DEBUG || _RELEASE_INTERNAL)

        CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferStartCmd(
            cmdBuffer,
            &batchBuffer));

        CODECHAL_DEBUG_TOOL(
            CODECHAL_DECODE_CHK_STATUS_RETURN(m_debugInterface->Dump2ndLvlBatch(
                &batchBuffer,
                CODECHAL_NUM_MEDIA_STATES,
                "_2ndLvlBatch_Primitive_Cmd"));)
    }

    // Update GlobalCmdBufId
    MHW_MI_STORE_DATA_PARAMS miStoreDataParams;
    MOS_ZeroMemory(&miStoreDataParams, sizeof(miStoreDataParams));
    miStoreDataParams.pOsResource = m_cencBuf->resTracker;
    miStoreDataParams.dwValue     = m_cencBuf->trackerId;
    CODECHAL_DECODE_VERBOSEMESSAGE("dwCmdBufId = %d", miStoreDataParams.dwValue);
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiStoreDataImmCmd(
        cmdBuffer,
        &miStoreDataParams));
    return MOS_STATUS_SUCCESS;
}

