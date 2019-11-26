/*
* Copyright (c) 2012-2018, Intel Corporation
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
//! \file     codechal_decode_vp9_g11.cpp
//! \brief    Implements the decode interface extension for Gen11 VP9.
//! \details  Implements all functions required by CodecHal for Gen11 VP9 decoding.
//!

#include "codechal_decoder.h"
#include "codechal_secure_decode_interface.h"
#include "codechal_decode_vp9_g11.h"
#include "codechal_mmc_decode_vp9.h"
#include "mhw_vdbox_hcp_g11_X.h"
#include "mhw_vdbox_mfx_g11_X.h"
#include "mhw_vdbox_g11_X.h"
#include "codechal_hw_g11_X.h"
#include "hal_oca_interface.h"

CodechalDecodeVp9G11 ::  ~CodechalDecodeVp9G11()
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
}

CodechalDecodeVp9G11::CodechalDecodeVp9G11(
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

MOS_STATUS CodechalDecodeVp9G11::SetGpuCtxCreatOption(
    CodechalSetting *          codecHalSetting)
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

        if (static_cast<MhwVdboxMfxInterfaceG11*>(m_mfxInterface)->IsScalabilitySupported())
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
            CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHalDecodeSinglePipeVE_ConstructParmsForGpuCtxCreation(
                m_sinlgePipeVeState,
                (PMOS_GPUCTX_CREATOPTIONS_ENHANCED)m_gpuCtxCreatOpt,
                false));

            m_videoContext = MOS_GPU_CONTEXT_VIDEO;
        }
    }

    return eStatus;
}

MOS_STATUS CodechalDecodeVp9G11 :: AllocateResourcesVariableSizes()
{
    MOS_STATUS              eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    CODECHAL_DECODE_CHK_STATUS_RETURN(CodechalDecodeVp9 :: AllocateResourcesVariableSizes());

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
            CodecHalDecodeScalability_AllocateResources_VariableSizes(
                m_scalabilityState,
                &hcpBufSizeParam,
                &reallocParam));

        m_frameSizeMaxAlloced = frameSizeMax;
    }

    return eStatus;
}

MOS_STATUS CodechalDecodeVp9G11 :: InitializeDecodeMode ()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    if ( MOS_VE_SUPPORTED(m_osInterface) && static_cast<MhwVdboxMfxInterfaceG11*>(m_mfxInterface)->IsScalabilitySupported())
    {
        CODECHAL_DECODE_SCALABILITY_INIT_PARAMS  initParams;

        MOS_ZeroMemory(&initParams, sizeof(initParams));
        initParams.u32PicWidthInPixel  = m_usFrameWidthAlignedMinBlk;
        initParams.u32PicHeightInPixel = m_usFrameHeightAlignedMinBlk;
        initParams.format = m_decodeParams.m_destSurface->Format;
        initParams.usingSFC            = false;
        initParams.gpuCtxInUse         = GetVideoContext();

        CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHalDecodeScalability_InitScalableParams(
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

MOS_STATUS CodechalDecodeVp9G11::DetermineDecodePhase()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    if (static_cast<MhwVdboxMfxInterfaceG11*>(m_mfxInterface)->IsScalabilitySupported() && MOS_VE_SUPPORTED(m_osInterface))
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHalDecodeScalability_DetermineDecodePhase(
            m_scalabilityState,
            &m_hcpDecPhase));
    }
    else
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(CodechalDecodeVp9 :: DetermineDecodePhase());
    }

    return eStatus;
}

MOS_STATUS CodechalDecodeVp9G11::EndStatusReport(
    CodechalDecodeStatusReport &decodeStatusReport,
    PMOS_COMMAND_BUFFER cmdBuffer)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    CODECHAL_DECODE_CHK_COND_RETURN((m_vdboxIndex > m_mfxInterface->GetMaxVdboxIndex()),
        "ERROR - vdbox index exceed the maximum");

    auto mmioRegistersMfx = m_mfxInterface->GetMmioRegisters(m_vdboxIndex);
    uint32_t currIndex = m_decodeStatusBuf.m_currIndex;

    MHW_MI_STORE_REGISTER_MEM_PARAMS regParams;

    //Frame CRC
    if (m_reportFrameCrc)
    {
        uint32_t frameCrcOffset =
            currIndex * sizeof(CodechalDecodeStatus) +
            m_decodeStatusBuf.m_decFrameCrcOffset +
            sizeof(uint32_t) * 2;

        regParams.presStoreBuffer = &m_decodeStatusBuf.m_statusBuffer;
        regParams.dwOffset = frameCrcOffset;
        regParams.dwRegister = mmioRegistersMfx->mfxFrameCrcRegOffset;

        CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiStoreRegisterMemCmd(
            cmdBuffer,
            &regParams));
    }

    // First copy all the SW data in to the eStatus buffer
    m_decodeStatusBuf.m_decodeStatus[currIndex].m_swStoredData = m_decodeStatusBuf.m_swStoreData;
    m_decodeStatusBuf.m_decodeStatus[currIndex].m_decodeStatusReport = decodeStatusReport;

    uint32_t storeDataOffset =
        currIndex * sizeof(CodechalDecodeStatus) +
        m_decodeStatusBuf.m_storeDataOffset +
        sizeof(uint32_t) * 2;

    MHW_MI_STORE_DATA_PARAMS dataParams;
    dataParams.pOsResource = &m_decodeStatusBuf.m_statusBuffer;
    dataParams.dwResourceOffset = storeDataOffset;
    dataParams.dwValue = CODECHAL_STATUS_QUERY_END_FLAG;

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiStoreDataImmCmd(
        cmdBuffer,
        &dataParams));

    m_decodeStatusBuf.m_currIndex = (m_decodeStatusBuf.m_currIndex + 1) % CODECHAL_DECODE_STATUS_NUM;

    CodechalDecodeStatus *decodeStatus = &m_decodeStatusBuf.m_decodeStatus[m_decodeStatusBuf.m_currIndex];
    MOS_ZeroMemory(decodeStatus, sizeof(CodechalDecodeStatus));

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_perfProfiler->AddPerfCollectEndCmd((void *)this, m_osInterface, m_miInterface, cmdBuffer));

    if (!m_osInterface->bEnableKmdMediaFrameTracking && m_osInterface->bInlineCodecStatusUpdate)
    {
        MHW_MI_FLUSH_DW_PARAMS flushDwParams;
        // Send MI_FLUSH with protection bit off, which will FORCE exit protected mode for MFX
        MOS_ZeroMemory(&flushDwParams, sizeof(flushDwParams));
        flushDwParams.bVideoPipelineCacheInvalidate = true;
        flushDwParams.pOsResource = &m_decodeStatusBuf.m_statusBuffer;
        flushDwParams.dwDataDW1 = m_decodeStatusBuf.m_swStoreData;
        MHW_CHK_STATUS_RETURN(m_miInterface->AddMiFlushDwCmd(
            cmdBuffer,
            &flushDwParams));
    }

    return eStatus;
}

MOS_STATUS CodechalDecodeVp9G11::EndStatusReportForFE(
    PMOS_COMMAND_BUFFER cmdBuffer)
{
    MOS_STATUS                      eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    CODECHAL_DECODE_CHK_COND_RETURN((m_vdboxIndex > m_mfxInterface->GetMaxVdboxIndex()),
        "ERROR - vdbox index exceed the maximum");
    auto mmioRegistersMfx = m_mfxInterface->GetMmioRegisters(m_vdboxIndex);
    auto mmioRegistersHcp = m_hcpInterface ? m_hcpInterface->GetMmioRegisters(m_vdboxIndex) : nullptr;

    uint32_t currIndex = m_decodeStatusBuf.m_currIndex;
    //Error Status report
    uint32_t errStatusOffset =
        currIndex * sizeof(CodechalDecodeStatus) +
        m_decodeStatusBuf.m_decErrorStatusOffset +
        sizeof(uint32_t) * 2;

    MHW_MI_STORE_REGISTER_MEM_PARAMS regParams;
    regParams.presStoreBuffer = &m_decodeStatusBuf.m_statusBuffer;
    regParams.dwOffset = errStatusOffset;
    regParams.dwRegister = mmioRegistersHcp ?
        mmioRegistersHcp->hcpCabacStatusRegOffset : mmioRegistersMfx->mfxErrorFlagsRegOffset;
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiStoreRegisterMemCmd(
        cmdBuffer,
        &regParams));

    //MB Count
    uint32_t mbCountOffset =
        currIndex * sizeof(CodechalDecodeStatus) +
        m_decodeStatusBuf.m_decMBCountOffset +
        sizeof(uint32_t) * 2;

    regParams.presStoreBuffer = &m_decodeStatusBuf.m_statusBuffer;
    regParams.dwOffset = mbCountOffset;
    regParams.dwRegister = mmioRegistersHcp ?
        mmioRegistersHcp->hcpDecStatusRegOffset : mmioRegistersMfx->mfxMBCountRegOffset;
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiStoreRegisterMemCmd(
        cmdBuffer,
        &regParams));

    return eStatus;
}

MOS_STATUS CodechalDecodeVp9G11::SetAndPopulateVEHintParams(
    PMOS_COMMAND_BUFFER       primCmdBuf)
{
    MOS_STATUS                      eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    if (static_cast<MhwVdboxMfxInterfaceG11*>(m_mfxInterface)->IsScalabilitySupported() && MOS_VE_SUPPORTED(m_osInterface))
    {
        CODECHAL_DECODE_SCALABILITY_SETHINT_PARMS scalSetParms;
        if (!MOS_VE_CTXBASEDSCHEDULING_SUPPORTED(m_osInterface))
        {
            scalSetParms.bNeedSyncWithPrevious       = true;
            scalSetParms.bSameEngineAsLastSubmission = false;
            scalSetParms.bSFCInUse                   = false;
        }
        CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHalDecodeScalability_SetHintParams(m_scalabilityState, &scalSetParms));
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

MOS_STATUS CodechalDecodeVp9G11::DetermineSendProlgwithFrmTracking(
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

uint32_t CodechalDecodeVp9G11::RequestedSpaceSize(uint32_t requestedSize)
{
    if (static_cast<MhwVdboxMfxInterfaceG11*>(m_mfxInterface)->IsScalabilitySupported())
    {
        //primary cmd buffer only including cmd buffer header .
        return COMMAND_BUFFER_RESERVED_SPACE * 2;
    }
    else
    {
        return requestedSize;
    }
}

MOS_STATUS CodechalDecodeVp9G11::VerifyExtraSpace(
    uint32_t requestedSize,
    uint32_t additionalSizeNeeded)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    if (static_cast<MhwVdboxMfxInterfaceG11*>(m_mfxInterface)->IsScalabilitySupported())
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

MOS_STATUS CodechalDecodeVp9G11 :: DecodeStateLevel()
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
        //Frame tracking functionality is called at the start of primary command buffer.
        CODECHAL_DECODE_CHK_STATUS_RETURN(SendPrologWithFrameTracking(
            &primCmdBuffer, true));
    }

    PMOS_COMMAND_BUFFER cmdBufferInUse = &primCmdBuffer;
    MOS_COMMAND_BUFFER  scdryCmdBuffer;
    auto                mmioRegisters = m_hwInterface->GetMfxInterface()->GetMmioRegisters(m_vdboxIndex);

    if (CodecHalDecodeScalabilityIsScalableMode(m_scalabilityState) && MOS_VE_SUPPORTED(m_osInterface))
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHalDecodeScalability_GetCmdBufferToUse(
            m_scalabilityState,
            &scdryCmdBuffer,
            &cmdBufferInUse));

        if (cmdBufferInUse == &scdryCmdBuffer)
        {
            //send prolog at the start of a secondary cmd buffer
            CODECHAL_DECODE_CHK_STATUS_RETURN(SendPrologWithFrameTracking(cmdBufferInUse, false));
        }

        HalOcaInterface::On1stLevelBBStart(scdryCmdBuffer, *m_osInterface->pOsContext, m_osInterface->CurrentGpuContextHandle, *m_miInterface, *mmioRegisters);
    }
    else
    {
        HalOcaInterface::On1stLevelBBStart(primCmdBuffer, *m_osInterface->pOsContext, m_osInterface->CurrentGpuContextHandle, *m_miInterface, *mmioRegisters);
    }

    auto pipeModeSelectParams =
        static_cast<PMHW_VDBOX_PIPE_MODE_SELECT_PARAMS_G11>(m_picMhwParams.PipeModeSelectParams);
    *pipeModeSelectParams = {}; 
    auto pipeBufAddrParams =
        static_cast<PMHW_VDBOX_PIPE_BUF_ADDR_PARAMS_G11>(m_picMhwParams.PipeBufAddrParams);
    *pipeBufAddrParams = {};

    CODECHAL_DECODE_CHK_STATUS_RETURN(InitPicStateMhwParams());

    bool drmStartStatusFlag = true;
    if (CodecHalDecodeScalabilityIsScalableMode(m_scalabilityState))
    {
        // Put all DRM/Start Status related cmd into FE VDBOX
        drmStartStatusFlag = CodecHalDecodeScalabilityIsFEPhase(m_scalabilityState);

        CodecHalDecodeScalablity_DecPhaseToHwWorkMode(
            pipeModeSelectParams->MultiEngineMode,
            pipeModeSelectParams->PipeWorkMode);

        pipeBufAddrParams->presCABACSyntaxStreamOutBuffer =
            m_scalabilityState->presCABACStreamOutBuffer;
        pipeBufAddrParams->presIntraPredUpRightColStoreBuffer =
            &m_scalabilityState->resIntraPredUpRightColStoreBuffer;
        pipeBufAddrParams->presIntraPredLeftReconColStoreBuffer =
            &m_scalabilityState->resIntraPredLeftReconColStoreBuffer;
    }

    if (drmStartStatusFlag)
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
        CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHalDecodeScalability_FEBESync(
            m_scalabilityState,
            cmdBufferInUse));
        if (m_perfFEBETimingEnabled && CodecHalDecodeScalabilityIsLastCompletePhase(m_scalabilityState))
        {
            CODECHAL_DECODE_CHK_STATUS_RETURN(m_perfProfiler->AddPerfCollectStartCmd((void *)this, m_osInterface, m_miInterface, &scdryCmdBuffer));
        }
    }

    if (CodecHalDecodeScalabilityIsBEPhase(m_scalabilityState))
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(
            m_miInterface->AddWatchdogTimerStartCmd(cmdBufferInUse));
    }

    CODECHAL_DECODE_CHK_STATUS_RETURN(AddPicStateMhwCmds(
        cmdBufferInUse));

    if (CodecHalDecodeScalabilityIsBEPhase(m_scalabilityState))
    {
        MHW_VDBOX_HCP_TILE_CODING_PARAMS_G11  hcpTileCodingParam;
        CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHalDecodeScalability_CalculateHcpTileCodingParams<MHW_VDBOX_HCP_TILE_CODING_PARAMS_G11>(
            m_scalabilityState,
            m_vp9PicParams,
            &hcpTileCodingParam));
        CODECHAL_DECODE_CHK_STATUS_RETURN(static_cast<MhwVdboxHcpInterfaceG11*>(m_hcpInterface)->AddHcpTileCodingCmd(
            cmdBufferInUse,
            &hcpTileCodingParam));
    }

    m_osInterface->pfnReturnCommandBuffer(m_osInterface, &primCmdBuffer, 0);
    if (CodecHalDecodeScalabilityIsScalableMode(m_scalabilityState) && MOS_VE_SUPPORTED(m_osInterface))
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHalDecodeScalability_ReturnSdryCmdBuffer(m_scalabilityState, &scdryCmdBuffer));
    }

    return eStatus;
}

MOS_STATUS CodechalDecodeVp9G11 :: DecodePrimitiveLevel()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    PERF_UTILITY_AUTO(__FUNCTION__, PERF_DECODE, PERF_LEVEL_HAL);

    CODECHAL_DECODE_FUNCTION_ENTER;

    // Bitstream is incomplete, don't do any decoding work.
    if (m_incompletePicture)
    {
        eStatus = MOS_STATUS_SUCCESS;
        return eStatus;
    }

    CODECHAL_DECODE_CHK_COND_RETURN(
        (m_vdboxIndex > m_mfxInterface->GetMaxVdboxIndex()),
        "ERROR - vdbox index exceed the maximum");

    CODECHAL_DECODE_CHK_NULL_RETURN(m_osInterface);

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
        CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHalDecodeScalability_GetCmdBufferToUse(
            m_scalabilityState,
            &scdryCmdBuffer,
            &cmdBufferInUse));
    }

    // store CS ENGINE ID register
    if (static_cast<MhwVdboxMfxInterfaceG11*>(m_mfxInterface)->IsScalabilitySupported() && MOS_VE_SUPPORTED(m_osInterface))
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHalDecodeScalability_ReadCSEngineIDReg(
            m_scalabilityState,
            &m_decodeStatusBuf,
            cmdBufferInUse));
    }

    //no slice level command for Huc based DRM and scalability decode BE phases
    if (m_cencBuf == nullptr && !CodecHalDecodeScalabilityIsBEPhase(m_scalabilityState))
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
        CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHalDecodeScalablity_SetFECabacStreamoutOverflowStatus(
            m_scalabilityState,
            cmdBufferInUse));

        CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHalDecodeScalability_SignalFE2BESemaphore(
            m_scalabilityState,
            cmdBufferInUse));

        //hcp decode status read at end of FE to support scalability
        if (m_statusQueryReportingEnabled)
        {
            EndStatusReportForFE(cmdBufferInUse);
            if (m_perfFEBETimingEnabled)
            {
                CODECHAL_DECODE_CHK_STATUS_RETURN(m_perfProfiler->AddPerfCollectEndCmd((void *)this, m_osInterface, m_miInterface, &scdryCmdBuffer));
            }
        }
    }

    //Sync for decode completion in scalable mode
    if (CodecHalDecodeScalabilityIsBEPhase(m_scalabilityState))
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

            // VP9 plug-in/out was NOT fully enabled; this is just to make sure driver would not crash in CodecHal_DecodeEndFrame(),
            // which requires the value of DecodeStatusReport.presCurrDecodedPic
            CODECHAL_DEBUG_TOOL(
                decodeStatusReport.m_frameType = m_perfType;
            )

            if (CodecHalDecodeScalabilityIsScalableMode(m_scalabilityState))
            {
                CODECHAL_DECODE_CHK_STATUS_RETURN(EndStatusReport(
                    decodeStatusReport,
                    cmdBufferInUse));
            }
            else
            {
                CODECHAL_DECODE_CHK_STATUS_RETURN(CodechalDecode::EndStatusReport(
                    decodeStatusReport,
                    cmdBufferInUse));
            }
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
        CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHalDecodeScalability_ReturnSdryCmdBuffer(m_scalabilityState, &scdryCmdBuffer));
    }

    CODECHAL_DEBUG_TOOL(

        if (CodecHalDecodeScalabilityIsScalableMode(m_scalabilityState)) {
            CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHalDecodeScalability_DbgDumpCmdBuffer(
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

    bool syncCompleteFrame = m_copyDataBufferInUse;
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
        submitCommand = CodecHalDecodeScalabilityIsToSubmitCmdBuffer(m_scalabilityState);
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
            CodecHalDecodeScalability_DecPhaseToSubmissionType(m_scalabilityState,cmdBufferInUse);
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
            resetStatusReport = CodecHalDecodeScalabilityIsFinalBEPhase(m_scalabilityState);
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

    return eStatus;
}

MOS_STATUS CodechalDecodeVp9G11 :: AllocateStandard (
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

    MHW_VDBOX_STATE_CMDSIZE_PARAMS_G11      stateCmdSizeParams;
    stateCmdSizeParams.bHucDummyStream = false;
    stateCmdSizeParams.bScalableMode   = static_cast<MhwVdboxMfxInterfaceG11*>(m_mfxInterface)->IsScalabilitySupported();
    //FE has more commands than BE.

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
        if (static_cast<MhwVdboxMfxInterfaceG11*>(m_mfxInterface)->IsScalabilitySupported())
        {
            m_scalabilityState = (PCODECHAL_DECODE_SCALABILITY_STATE)MOS_AllocAndZeroMemory(sizeof(CODECHAL_DECODE_SCALABILITY_STATE));
            CODECHAL_DECODE_CHK_NULL_RETURN(m_scalabilityState);
            //scalability initialize
            CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHalDecodeScalability_InitializeState(
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
    m_picMhwParams.PipeModeSelectParams = MOS_New(MHW_VDBOX_PIPE_MODE_SELECT_PARAMS_G11);
    m_picMhwParams.PipeBufAddrParams = MOS_New(MHW_VDBOX_PIPE_BUF_ADDR_PARAMS_G11);
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

MOS_STATUS CodechalDecodeVp9G11::SetCencBatchBuffer(
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
    if (!(CodecHalDecodeScalabilityIsBEPhase(m_scalabilityState) && MOS_VE_SUPPORTED(m_osInterface)))
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

