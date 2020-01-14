/*
// Copyright (C) 2017-2019, Intel Corporation
//
// Licensed under the Apache License,Version 2.0(the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
*/
//!
//! \file     codechal_decode_scalability_g12.cpp
//! \brief    Impelements the public interface for Gen12 Scalability Decode
//!
#include "codechal_decoder.h"
#include "codechal_decode_scalability_g12.h"
#include "media_user_settings_mgr_g12.h"
#include "mos_os_virtualengine_next.h"

//==<Functions>=======================================================
//!
//! \brief    calculate secondary cmd buffer index
//! \details  calculate secondary cmd buffer index to get or return secondary cmd buffer
//! \param    [in]  pScalabilityState
//!                pointer to scalability decode state
//! \param    [in]  pdwBufIdxPlus1
//!                pointer to buf index, will contain the returned buf index value.
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success, else fail reason
//!
static MOS_STATUS CodecHalDecodeScalability_CalculateScdryCmdBufIndex_G12(
    PCODECHAL_DECODE_SCALABILITY_STATE  pScalabilityStateBase,
    uint32_t                           *pdwBufIdxPlus1)
{
    uint32_t                        HcpDecPhaseForBufIdx;
    MOS_STATUS                      eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    PCODECHAL_DECODE_SCALABILITY_STATE_G12 pScalabilityState = static_cast<PCODECHAL_DECODE_SCALABILITY_STATE_G12>(pScalabilityStateBase);

    CODECHAL_DECODE_CHK_NULL(pScalabilityStateBase);
    CODECHAL_DECODE_CHK_NULL(pdwBufIdxPlus1);

    if (pScalabilityState->HcpDecPhase == CODECHAL_HCP_DECODE_PHASE_REAL_TILE)
    {
        *pdwBufIdxPlus1 = pScalabilityState->u8RtCurPipe + 1;
    }
    else
    {
        HcpDecPhaseForBufIdx = pScalabilityState->HcpDecPhase;
        if (pScalabilityState->HcpDecPhase == CodechalDecode::CodechalHcpDecodePhaseLegacyS2L)
        {
            //S2L commands put in the FE secondary command buffer.
            CODECHAL_DECODE_ASSERT(pScalabilityState->bShortFormatInUse);
            HcpDecPhaseForBufIdx = CODECHAL_HCP_DECODE_PHASE_FE;
        }

        //buffer index order is same as the buffer order in resScalableBatchBufs[] of MOS_VIRTUALENGINE_HINT_PARAMS
        *pdwBufIdxPlus1 = HcpDecPhaseForBufIdx - (pScalabilityState->bFESeparateSubmission ?
            CODECHAL_HCP_DECODE_PHASE_BE0 : CODECHAL_HCP_DECODE_PHASE_FE) + 1;
    }

finish:
    return eStatus;
}

//!
//! \brief    check if valid decode phase
//! \param    [in]  pScalabilityState
//!                pointer to scalability decode state
//! \param    [in]  HcpDecPhase
//!                Hcp Decode Phase
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if valid decode phase, else fail reason
//!
static MOS_STATUS CodecHalDecodeScalability_CheckDecPhaseValidity_G12(
    PCODECHAL_DECODE_SCALABILITY_STATE  pScalabilityStateBase,
    uint32_t                            HcpDecPhase)
{
    bool                bInValidPhase = false;
    MOS_STATUS          eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    PCODECHAL_DECODE_SCALABILITY_STATE_G12 pScalabilityState = static_cast<PCODECHAL_DECODE_SCALABILITY_STATE_G12>(pScalabilityStateBase);

    CODECHAL_DECODE_CHK_NULL(pScalabilityStateBase);

    switch (HcpDecPhase)
    {
    case CodechalDecode::CodechalHcpDecodePhaseInitialized:
        break;
    case CodechalDecode::CodechalHcpDecodePhaseLegacyS2L:
        if (!pScalabilityState->bShortFormatInUse)
        {
            bInValidPhase = true;
        }
        break;
    case CodechalDecode::CodechalHcpDecodePhaseLegacyLong:
        if (pScalabilityState->bScalableDecodeMode)
        {
            bInValidPhase = true;
        }
        break;
    case CODECHAL_HCP_DECODE_PHASE_FE:
    case CODECHAL_HCP_DECODE_PHASE_BE0:
        if (!pScalabilityState->bScalableDecodeMode)
        {
            bInValidPhase = true;
        }
        else if (pScalabilityState->ucScalablePipeNum < 2)
        {
            //at least 2 pipe
            bInValidPhase = true;
        }
        break;
    case CODECHAL_HCP_DECODE_PHASE_REAL_TILE:
        if (pScalabilityState->ucScalablePipeNum <= pScalabilityState->u8RtCurPipe)
        {
            bInValidPhase = true;
        }
        break;
    default:
        if(HcpDecPhase > CODECHAL_HCP_DECODE_PHASE_BE0 &&
            HcpDecPhase != CODECHAL_HCP_DECODE_PHASE_REAL_TILE)
        {
            if (!pScalabilityState->bScalableDecodeMode)
            {
                bInValidPhase = true;
            }
            else if (pScalabilityState->ucScalablePipeNum < (HcpDecPhase - CODECHAL_HCP_DECODE_PHASE_BE0 + 1))
            {
                bInValidPhase = true;
            }
        }
        else
        {
            bInValidPhase = true;
        }
        break;
    }

    if (bInValidPhase)
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        CODECHAL_DECODE_ASSERTMESSAGE("invalid decode phase : %d !", HcpDecPhase);
    }

finish:
    return eStatus;
}

 MOS_STATUS CodecHalDecodeScalability_GetVESecondaryCmdBuffer_G12(
    PCODECHAL_DECODE_SCALABILITY_STATE  pScalabilityState,
    PMOS_COMMAND_BUFFER                 pSdryCmdBuf)
{
    PMOS_INTERFACE                  pOsInterface;
    uint32_t                        HcpDecPhase;
    uint32_t                        dwBufIdxPlus1 = 0;
    MOS_STATUS                      eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    CODECHAL_DECODE_CHK_NULL(pScalabilityState);
    CODECHAL_DECODE_CHK_NULL(pSdryCmdBuf);
    CODECHAL_DECODE_CHK_NULL(pScalabilityState->pHwInterface);
    CODECHAL_DECODE_CHK_NULL(pScalabilityState->pHwInterface->GetOsInterface());

    pOsInterface = pScalabilityState->pHwInterface->GetOsInterface();
    HcpDecPhase = pScalabilityState->HcpDecPhase;

    //calculate bufidx for getting secondary cmd buffer.
    CODECHAL_DECODE_CHK_STATUS(CodecHalDecodeScalability_CalculateScdryCmdBufIndex_G12(pScalabilityState, &dwBufIdxPlus1));
    //Check if valid decode phase
    CODECHAL_DECODE_CHK_STATUS(CodecHalDecodeScalability_CheckDecPhaseValidity_G12(pScalabilityState, HcpDecPhase));

    //Get batch buffer according to current decode phase
    switch (HcpDecPhase)
    {
    case CodechalDecode::CodechalHcpDecodePhaseLegacyS2L:
        //Note: no break here, S2L and FE commands put in one secondary command buffer.
    case CODECHAL_HCP_DECODE_PHASE_FE:
        if (!pScalabilityState->bFESeparateSubmission)
        {
            CODECHAL_DECODE_CHK_STATUS(pOsInterface->pfnGetCommandBuffer(pOsInterface, pSdryCmdBuf, dwBufIdxPlus1));
            CODECHAL_DECODE_CHK_NULL(pSdryCmdBuf);
        }
        else
        {
            //if FE separate submission, S2L and FE cmd buffer are in primary cmd buffer, shall not call this function to get secondary cmd buffer
            eStatus = MOS_STATUS_INVALID_PARAMETER;
            CODECHAL_DECODE_ASSERTMESSAGE("S2L or FE does not need secondary cmd buffer in FE separate submission!");
        }
        break;
    default:
        if(((HcpDecPhase >= CODECHAL_HCP_DECODE_PHASE_BE0) && ((HcpDecPhase-CODECHAL_HCP_DECODE_PHASE_FE) <= pScalabilityState->ucScalablePipeNum)) ||
            (HcpDecPhase == CODECHAL_HCP_DECODE_PHASE_REAL_TILE))
        {
            CODECHAL_DECODE_CHK_STATUS(pOsInterface->pfnGetCommandBuffer(pOsInterface, pSdryCmdBuf, dwBufIdxPlus1));
            CODECHAL_DECODE_CHK_NULL(pSdryCmdBuf);
        }
        else
        {
            eStatus = MOS_STATUS_INVALID_PARAMETER;
        }
        break;
    }

finish:
    return eStatus;
}

MOS_STATUS CodecHalDecodeScalability_GetCmdBufferToUse_G12(
    PCODECHAL_DECODE_SCALABILITY_STATE  pScalabilityStateBase,
    PMOS_COMMAND_BUFFER                 pScdryCmdBuf,
    PMOS_COMMAND_BUFFER                 *ppCmdBufToUse)
{
    MOS_STATUS                      eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    PCODECHAL_DECODE_SCALABILITY_STATE_G12 pScalabilityState = static_cast<PCODECHAL_DECODE_SCALABILITY_STATE_G12>(pScalabilityStateBase);

    CODECHAL_DECODE_CHK_NULL(pScalabilityStateBase);
    CODECHAL_DECODE_CHK_NULL(ppCmdBufToUse);

    if (!CodecHalDecodeScalabilityIsFESeparateSubmission(pScalabilityState) ||
        CodecHalDecodeScalabilityIsBEPhaseG12(pScalabilityState))
    {
        pScalabilityState->bUseSecdryCmdBuffer = true;
        CODECHAL_DECODE_CHK_STATUS(CodecHalDecodeScalability_GetVESecondaryCmdBuffer_G12(pScalabilityState, pScdryCmdBuf));
        *ppCmdBufToUse = pScdryCmdBuf;
    }
    else
    {
        pScalabilityState->bUseSecdryCmdBuffer = false;
    }

finish:
    return eStatus;
}

MOS_STATUS CodecHalDecodeScalability_ReturnSdryCmdBuffer_G12(
    PCODECHAL_DECODE_SCALABILITY_STATE  pScalabilityState,
    PMOS_COMMAND_BUFFER                 pSdryCmdBuf)
{
    PMOS_INTERFACE                  pOsInterface;
    uint32_t                        HcpDecPhase;
    uint32_t                        dwBufIdxPlus1;
    MOS_STATUS                      eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    CODECHAL_DECODE_CHK_NULL(pScalabilityState);
    CODECHAL_DECODE_CHK_NULL(pSdryCmdBuf);
    CODECHAL_DECODE_CHK_NULL(pScalabilityState->pHwInterface);
    CODECHAL_DECODE_CHK_NULL(pScalabilityState->pHwInterface->GetOsInterface());

    pOsInterface = pScalabilityState->pHwInterface->GetOsInterface();

    if (!pScalabilityState->bUseSecdryCmdBuffer)
    {
        goto finish;
    }

    HcpDecPhase = pScalabilityState->HcpDecPhase;

    //calculate bufidx for getting secondary cmd buffer.
    CODECHAL_DECODE_CHK_STATUS(CodecHalDecodeScalability_CalculateScdryCmdBufIndex_G12(pScalabilityState, &dwBufIdxPlus1));
    //Check if valid decode phase
    CODECHAL_DECODE_CHK_STATUS(CodecHalDecodeScalability_CheckDecPhaseValidity_G12(pScalabilityState, HcpDecPhase));

    //Get batch buffer according to current decode phase
    switch (HcpDecPhase)
    {
    case CodechalDecode::CodechalHcpDecodePhaseLegacyS2L:
        //Note: no break here, S2L and FE commands put in one secondary command buffer.
    case CODECHAL_HCP_DECODE_PHASE_FE:
        if (!pScalabilityState->bFESeparateSubmission)
        {
            pOsInterface->pfnReturnCommandBuffer(pOsInterface, pSdryCmdBuf, dwBufIdxPlus1);
        }
        else
        {
            //if FE separate submission, S2L and FE cmd buffer are in primary cmd buffer, shall not call this function to get secondary cmd buffer
            eStatus = MOS_STATUS_INVALID_PARAMETER;
            CODECHAL_DECODE_ASSERTMESSAGE("S2L or FE does not need secondary cmd buffer in FE separate submission!");
        }
        break;
    default:
        if(((HcpDecPhase >= CODECHAL_HCP_DECODE_PHASE_BE0) && ((HcpDecPhase-CODECHAL_HCP_DECODE_PHASE_FE) <= pScalabilityState->ucScalablePipeNum)) ||
          (HcpDecPhase == CODECHAL_HCP_DECODE_PHASE_REAL_TILE))
        {
            pOsInterface->pfnReturnCommandBuffer(pOsInterface, pSdryCmdBuf, dwBufIdxPlus1);
        }
        else
        {
            eStatus = MOS_STATUS_INVALID_PARAMETER;
        }
        break;
    }

finish:
    return eStatus;
}

MOS_STATUS CodecHalDecodeScalability_DetermineDecodePhase_G12(
    PCODECHAL_DECODE_SCALABILITY_STATE  pScalabilityStateBase,
    uint32_t                            *pHcpDecPhase)
{
    uint32_t                        CurPhase;
    MOS_STATUS                      eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    PCODECHAL_DECODE_SCALABILITY_STATE_G12 pScalabilityState = static_cast<PCODECHAL_DECODE_SCALABILITY_STATE_G12>(pScalabilityStateBase);

    CODECHAL_DECODE_CHK_NULL(pScalabilityStateBase);
    CODECHAL_DECODE_CHK_NULL(pHcpDecPhase);

    CurPhase = *pHcpDecPhase;
    //Check if valid decode phase
    CODECHAL_DECODE_CHK_STATUS(CodecHalDecodeScalability_CheckDecPhaseValidity_G12(pScalabilityState, CurPhase));

    if (pScalabilityState->bIsRtMode)
    {
        switch (CurPhase)
        {
        case CodechalDecode::CodechalHcpDecodePhaseInitialized:
            if (pScalabilityState->bShortFormatInUse)
            {
                *pHcpDecPhase = CodechalDecode::CodechalHcpDecodePhaseLegacyS2L;
            }
            else
            {
                *pHcpDecPhase = CODECHAL_HCP_DECODE_PHASE_REAL_TILE;
            }
            break;
        case CodechalDecode::CodechalHcpDecodePhaseLegacyS2L:
        case CODECHAL_HCP_DECODE_PHASE_REAL_TILE:
            *pHcpDecPhase = CODECHAL_HCP_DECODE_PHASE_REAL_TILE;
            break;
        default:
            eStatus = MOS_STATUS_INVALID_PARAMETER;
            break;
        }
    }
    else if (CodecHalDecodeScalabilityIsScalableMode(pScalabilityState))
    {
        switch (CurPhase)
        {
        case CodechalDecode::CodechalHcpDecodePhaseInitialized:
            if (pScalabilityState->bShortFormatInUse)
            {
                *pHcpDecPhase = CodechalDecode::CodechalHcpDecodePhaseLegacyS2L;
            }
            else
            {
                *pHcpDecPhase = CODECHAL_HCP_DECODE_PHASE_FE;
            }
            break;
        case CodechalDecode::CodechalHcpDecodePhaseLegacyS2L:
            *pHcpDecPhase = CODECHAL_HCP_DECODE_PHASE_FE;
            break;
        case CODECHAL_HCP_DECODE_PHASE_FE:
            *pHcpDecPhase = CODECHAL_HCP_DECODE_PHASE_BE0;
            break;
        default:
            if(CurPhase >= CODECHAL_HCP_DECODE_PHASE_BE0 && ((CurPhase-CODECHAL_HCP_DECODE_PHASE_FE) <= pScalabilityState->ucScalablePipeNum))
            {
                *pHcpDecPhase = CurPhase + 1;
            }
            else
            {
                eStatus = MOS_STATUS_INVALID_PARAMETER;
            }
            break;
        }
    }
    else
    {
        switch (CurPhase)
        {
        case CodechalDecode::CodechalHcpDecodePhaseInitialized:
            if (pScalabilityState->bShortFormatInUse)
            {
                *pHcpDecPhase = CodechalDecode::CodechalHcpDecodePhaseLegacyS2L;
            }
            else
            {
                *pHcpDecPhase = CodechalDecode::CodechalHcpDecodePhaseLegacyLong;
            }
            break;
        case CodechalDecode::CodechalHcpDecodePhaseLegacyS2L:
        case CodechalDecode::CodechalHcpDecodePhaseLegacyLong:
            *pHcpDecPhase = CodechalDecode::CodechalHcpDecodePhaseLegacyLong;
            break;
        default:
            //never comes here because other decode phase already checked invalid in function CodecHalDecodeScalability_CheckDecPhaseValidity,
            eStatus = MOS_STATUS_INVALID_PARAMETER;
            break;
        }
    }

    pScalabilityState->HcpDecPhase = *pHcpDecPhase;

finish:
    return eStatus;
}

MOS_STATUS CodecHalDecodeScalability_InitScalableParams_G12(
    PCODECHAL_DECODE_SCALABILITY_STATE         pScalabilityStateBase,
    PCODECHAL_DECODE_SCALABILITY_INIT_PARAMS   pInitParamsBase,
    uint16_t                                   *pucDecPassNum)
{
    PMOS_INTERFACE                  pOsInterface;
    PMOS_VIRTUALENGINE_INTERFACE    pVEInterface;
    MOS_STATUS                      eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    PCODECHAL_DECODE_SCALABILITY_STATE_G12 pScalabilityState = static_cast<PCODECHAL_DECODE_SCALABILITY_STATE_G12>(pScalabilityStateBase);
    PCODECHAL_DECODE_SCALABILITY_INIT_PARAMS_G12 pInitParams = static_cast<PCODECHAL_DECODE_SCALABILITY_INIT_PARAMS_G12>(pInitParamsBase);

    CODECHAL_DECODE_CHK_NULL(pScalabilityStateBase);
    CODECHAL_DECODE_CHK_NULL(pScalabilityStateBase->pHwInterface);
    CODECHAL_DECODE_CHK_NULL(pScalabilityStateBase->pHwInterface->GetOsInterface());

    pOsInterface = pScalabilityStateBase->pHwInterface->GetOsInterface();
    pVEInterface = pScalabilityStateBase->pVEInterface;

    if (!pOsInterface->bSupportVirtualEngine)
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        CODECHAL_DECODE_ASSERTMESSAGE("Scalability decode must run with virtual engine interface.\n");
        goto finish;
    }

    pScalabilityState->bScalableDecodeMode = false; // initialized to false
    pScalabilityState->bIsRtMode = false;

#if (_DEBUG || _RELEASE_INTERNAL)
    if (pOsInterface->bEnableDbgOvrdInVE)
    {
        if (!MOS_VE_CTXBASEDSCHEDULING_SUPPORTED(pOsInterface))
        {
            if (g_apoMosEnabled)
            {
                CODECHAL_DECODE_CHK_NULL(pVEInterface->veInterface);
                pScalabilityState->ucScalablePipeNum = pVEInterface->veInterface->GetEngineCount() - 1;
            }
            else
            {
                pScalabilityState->ucScalablePipeNum = pVEInterface->ucEngineCount - 1;
            }
            pScalabilityState->bScalableDecodeMode = true;
        }
        else
        {
            // do nothing since pipe number already decided at the gpu context creation.
        }
    }
    else
#endif
    {
        // Decide pipe number
        CODECHAL_DECODE_CHK_STATUS(pScalabilityState->pfnDecidePipeNum(pScalabilityState, pInitParams));
    }

    // Decide scalable mode or single pipe mode
    if (pScalabilityState->ucScalablePipeNum > 1)
    {
        pScalabilityState->bScalableDecodeMode = true;
    }

    CODECHAL_DECODE_CHK_NULL(pucDecPassNum);
    // Decide Decode pass number - pucDecPassNum
    if (pScalabilityState->bScalableDecodeMode)
    {
        bool        bCanEnableRealTile = true;
        uint8_t     u8MaxTileColumn = HEVC_NUM_MAX_TILE_COLUMN;

#if (_DEBUG || _RELEASE_INTERNAL)
        bCanEnableRealTile = !pScalabilityState->bDisableRtMode;
        if (!pScalabilityState->bEnableRtMultiPhase)
            u8MaxTileColumn = pScalabilityState->ucScalablePipeNum;
#endif
        bCanEnableRealTile = bCanEnableRealTile && pInitParams->bIsTileEnabled && (pInitParams->u8NumTileColumns > 1) &&
            (pInitParams->u8NumTileColumns <= u8MaxTileColumn) && (pInitParams->u8NumTileRows <= HEVC_NUM_MAX_TILE_ROW);
        if (bCanEnableRealTile)
        {
            pScalabilityState->bIsRtMode = true;
            //WA: No FE for real tile decoding and need to disable FE separate submission
            pScalabilityState->bFESeparateSubmission = false;
            pScalabilityState->u8RtPhaseNum =
                (pInitParams->u8NumTileColumns + pScalabilityState->ucScalablePipeNum - 1) / pScalabilityState->ucScalablePipeNum;
            pScalabilityState->u8RtPipeInLastPhase = pInitParams->u8NumTileColumns - pScalabilityState->ucScalablePipeNum * (pScalabilityState->u8RtPhaseNum - 1);
            pScalabilityState->u8RtCurPipe = 0;
            pScalabilityState->u8RtCurPhase = 0;

            *pucDecPassNum = pInitParams->u8NumTileColumns;
        }
        else if (pInitParams->bIsSccDecoding) // No virtual tile support for SCC, fallback to legacy mode
        {
            pScalabilityState->bScalableDecodeMode = false;
            *pucDecPassNum = 1;
        }
        else
        {
            *pucDecPassNum = pScalabilityState->ucScalablePipeNum + 1; // FE + all BEs
        }
    }
    else
    {
        *pucDecPassNum = 1;
    }

    // Add one pass for S2L conversion in short format.
    if (pScalabilityState->bShortFormatInUse)
    {
        *pucDecPassNum = *pucDecPassNum + 1;
    }

    pScalabilityState->VideoContext = pInitParams->gpuCtxInUse;

finish:
    return eStatus;
}

MOS_STATUS CodecHalDecodeScalability_SetHintParams_G12(
    PCODECHAL_DECODE_SCALABILITY_STATE         pScalabilityStateBase,
    PCODECHAL_DECODE_SCALABILITY_SETHINT_PARMS pSetHintParms)
{
    PMOS_VIRTUALENGINE_INTERFACE    pVEInterface;
    MOS_VIRTUALENGINE_SET_PARAMS    VEParams;
    PMOS_INTERFACE                  pOsInterface;
    MOS_STATUS                      eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    PCODECHAL_DECODE_SCALABILITY_STATE_G12 pScalabilityState = static_cast<PCODECHAL_DECODE_SCALABILITY_STATE_G12>(pScalabilityStateBase);

    CODECHAL_DECODE_CHK_NULL(pScalabilityStateBase);
    CODECHAL_DECODE_CHK_NULL(pSetHintParms);
    CODECHAL_DECODE_CHK_NULL(pScalabilityStateBase->pHwInterface);
    CODECHAL_DECODE_CHK_NULL(pScalabilityStateBase->pHwInterface->GetOsInterface());

    pOsInterface = pScalabilityStateBase->pHwInterface->GetOsInterface();
    pVEInterface = pScalabilityStateBase->pVEInterface;

    MOS_ZeroMemory(&VEParams, sizeof(VEParams));
    if (!MOS_VE_CTXBASEDSCHEDULING_SUPPORTED(pOsInterface))
    {
        //not used by VE2.0
        VEParams.bNeedSyncWithPrevious = pSetHintParms->bNeedSyncWithPrevious;
        VEParams.bSameEngineAsLastSubmission = pSetHintParms->bSameEngineAsLastSubmission;
        VEParams.bSFCInUse = pSetHintParms->bSFCInUse;
    }

    VEParams.ucScalablePipeNum = pScalabilityState->ucScalablePipeNum;

    if (pScalabilityState->bScalableDecodeMode)
    {
        if (pScalabilityState->bFESeparateSubmission)
        {
            //set Hint parameter for FE submission
            VEParams.bScalableMode = false;
            if (pVEInterface->pfnVESetHintParams)
            {
                CODECHAL_DECODE_CHK_STATUS(pVEInterface->pfnVESetHintParams(pVEInterface, &VEParams));
            }
        }

        VEParams.bScalableMode = true;
        VEParams.bHaveFrontEndCmds = pScalabilityState->bIsRtMode ? false : (pScalabilityState->bFESeparateSubmission ? false : true);
        if (pVEInterface->pfnVESetHintParams)
        {
            CODECHAL_DECODE_CHK_STATUS(pVEInterface->pfnVESetHintParams(pVEInterface, &VEParams));
        }
    }
    else
    {
        VEParams.bScalableMode = false;
        if (pVEInterface->pfnVESetHintParams)
        {
            CODECHAL_DECODE_CHK_STATUS(pVEInterface->pfnVESetHintParams(pVEInterface, &VEParams));
        }
    }

finish:
    return eStatus;
}

#if (_DEBUG || _RELEASE_INTERNAL)
MOS_STATUS CodecHalDecodeScalability_DbgDumpCmdBuffer_G12(
    CodechalDecode                      *pDecoder,
    PCODECHAL_DECODE_SCALABILITY_STATE  pScalabilityState,
    CodechalDebugInterface              *debugInterface,
    PMOS_COMMAND_BUFFER                 pPrimCmdBuf)
{
    MOS_STATUS                      eStatus = MOS_STATUS_SUCCESS;
    MOS_COMMAND_BUFFER              ScdryCmdBuffer;
    PMOS_COMMAND_BUFFER             pCmdBufferInUse;

    CODECHAL_DECODE_FUNCTION_ENTER;

    CODECHAL_DECODE_CHK_NULL(pDecoder);
    CODECHAL_DECODE_CHK_NULL(pScalabilityState);
    CODECHAL_DECODE_CHK_NULL(pPrimCmdBuf);

    if (!pScalabilityState->bScalableDecodeMode)
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        CODECHAL_DECODE_ASSERTMESSAGE("Invalid decode parameters!");
        goto finish;
    }

    if (CodecHalDecodeScalabilityIsFESeparateSubmission(pScalabilityState)
        && CodecHalDecodeScalabilityIsFEPhase(pScalabilityState))
    {
        pCmdBufferInUse = pPrimCmdBuf;
    }
    else
    {
        CODECHAL_DECODE_CHK_STATUS(CodecHalDecodeScalability_GetVESecondaryCmdBuffer_G12(pScalabilityState, &ScdryCmdBuffer));
        pCmdBufferInUse = &ScdryCmdBuffer;
    }

    CODECHAL_DECODE_CHK_STATUS_RETURN(debugInterface->DumpCmdBuffer(
        pCmdBufferInUse,
        CODECHAL_NUM_MEDIA_STATES,
        "DEC"));

finish:
    return eStatus;
}
#endif

MOS_STATUS CodecHalDecodeScalability_FEBESync_G12(
    PCODECHAL_DECODE_SCALABILITY_STATE  pScalabilityStateBase,
    PMOS_COMMAND_BUFFER                 pCmdBufferInUse,
    bool phasedSubmission)
{
    PMOS_INTERFACE                      pOsInterface;
    MhwMiInterface                      *pMiInterface;
    uint32_t                            HcpDecPhase;
    MOS_STATUS                          eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    PCODECHAL_DECODE_SCALABILITY_STATE_G12 pScalabilityState = static_cast<PCODECHAL_DECODE_SCALABILITY_STATE_G12>(pScalabilityStateBase);

    CODECHAL_DECODE_CHK_NULL(pScalabilityStateBase);
    CODECHAL_DECODE_CHK_NULL(pScalabilityStateBase->pHwInterface);
    CODECHAL_DECODE_CHK_NULL(pScalabilityStateBase->pHwInterface->GetOsInterface());
    CODECHAL_DECODE_CHK_NULL(pScalabilityStateBase->pHwInterface->GetMiInterface());
    CODECHAL_DECODE_CHK_NULL(pCmdBufferInUse);

    pOsInterface = pScalabilityStateBase->pHwInterface->GetOsInterface();
    pMiInterface = pScalabilityState->pHwInterface->GetMiInterface();
    HcpDecPhase = pScalabilityStateBase->HcpDecPhase;

    //FE& BE0 Sync. to refine (ucNumVdbox > )for GT3
    if (HcpDecPhase == CODECHAL_HCP_DECODE_PHASE_BE0 && pScalabilityState->pHwInterface->GetMfxInterface()->GetNumVdbox() > 2)
    {
        if (pScalabilityState->bFESeparateSubmission)
        {
            MOS_SYNC_PARAMS SyncParams;

            SyncParams = g_cInitSyncParams;
            SyncParams.GpuContext = pScalabilityState->VideoContext;
            SyncParams.presSyncResource = &pScalabilityState->resFeBeSyncObject;

            CODECHAL_DECODE_CHK_STATUS(pOsInterface->pfnEngineWait(pOsInterface, &SyncParams));
        }
        else
        {
            CODECHAL_DECODE_CHK_STATUS(pScalabilityState->pHwInterface->SendHwSemaphoreWaitCmd(&pScalabilityState->resSemaMemFEBE, 1, MHW_MI_SAD_EQUAL_SDD, pCmdBufferInUse));
            //reset semaphore. mi atomic decrease 1
            CODECHAL_DECODE_CHK_STATUS(pScalabilityState->pHwInterface->SendMiAtomicDwordCmd(&pScalabilityState->resSemaMemFEBE, 1, MHW_MI_ATOMIC_DEC, pCmdBufferInUse));
        }
    }

    if (CodecHalDecodeScalabilityIsBEPhaseG12(pScalabilityState) ||
        CodecHalDecodeScalabilityIsFirstRealTilePhase(pScalabilityState))
    {
        // Stop Watchdog before BEs wait
        pMiInterface->AddWatchdogTimerStopCmd(pCmdBufferInUse);

        //HW Semaphore for BEs Starting at the same time
        CODECHAL_DECODE_CHK_STATUS(pScalabilityState->pHwInterface->SendMiAtomicDwordCmd(&pScalabilityState->resSemaMemBEs, 1, MHW_MI_ATOMIC_INC, pCmdBufferInUse));
        uint8_t u8PipeNum = pScalabilityState->ucScalablePipeNum;

        if (phasedSubmission && pScalabilityState->bIsRtMode)
        {
            if (pScalabilityState->u8RtCurPhase == pScalabilityState->u8RtPhaseNum - 1)
            {
                u8PipeNum = pScalabilityState->u8RtPipeInLastPhase;
            }
        }

        CODECHAL_DECODE_CHK_STATUS(pScalabilityState->pHwInterface->SendHwSemaphoreWaitCmd(
            &pScalabilityState->resSemaMemBEs,
            u8PipeNum,
            MHW_MI_SAD_EQUAL_SDD,
            pCmdBufferInUse));

        // Program some placeholder cmds to resolve the hazard between BEs sync
        MHW_MI_STORE_DATA_PARAMS dataParams;
        dataParams.pOsResource = &pScalabilityState->resDelayMinus;
        dataParams.dwResourceOffset = 0;
        dataParams.dwValue = 0xDE1A;
        for (uint32_t i = 0; i < pScalabilityState->numDelay; i++)
        {
            CODECHAL_DECODE_CHK_STATUS_RETURN(pScalabilityState->pHwInterface->GetMiInterface()->AddMiStoreDataImmCmd(
                pCmdBufferInUse,
                &dataParams));
        }

        //reset HW semaphore
        CODECHAL_DECODE_CHK_STATUS(pScalabilityState->pHwInterface->SendMiAtomicDwordCmd(&pScalabilityState->resSemaMemBEs, 1, MHW_MI_ATOMIC_DEC, pCmdBufferInUse));

        if (!pScalabilityState->bIsRtMode)
        {
            if (pScalabilityState->bIsEnableEndCurrentBatchBuffLevel)
            {
                // Enhanced Condidtional BB END for streamout buffer writing over allocated size
                CODECHAL_DECODE_CHK_STATUS(static_cast<CodechalHwInterfaceG12*>(pScalabilityState->pHwInterface)->SendCondBbEndCmd(
                    &pScalabilityState->resFEStatusBuffer,
                    CODECHAL_OFFSETOF(CODECHAL_DECODE_SCALABILITY_FE_CABAC_STREAMOUT_BUFF_SIZE, dwCabacStreamoutBuffSize),
                    pScalabilityState->dwCABACSyntaxStreamOutBufferSize,
                    true,
                    pScalabilityState->bIsEnableEndCurrentBatchBuffLevel,
                    mhw_mi_g12_X::MI_CONDITIONAL_BATCH_BUFFER_END_CMD::COMPARE_OPERATION_MADLESSTHANIDD,
                    pCmdBufferInUse));
                CODECHAL_DECODE_NORMALMESSAGE("pScalabilityState->bIsEnableEndCurrentBatchBuffLevel: %s\n", "true");
            }
            else
            {
                // Condidtional BB END for streamout buffer writing over allocated size
                CODECHAL_DECODE_CHK_STATUS(pScalabilityState->pHwInterface->SendCondBbEndCmd(
                    &pScalabilityState->resFEStatusBuffer,
                    CODECHAL_OFFSETOF(CODECHAL_DECODE_SCALABILITY_FE_STATUS, dwCarryFlagOfReportedSizeMinusAllocSize),
                    0,
                    true,
                    pCmdBufferInUse));
            }
        }
    }

finish:
    return eStatus;
}

MOS_STATUS CodecHalDecodeScalability_ReadCSEngineIDReg_G12(
    PCODECHAL_DECODE_SCALABILITY_STATE pScalabilityStateBase,
    CodechalDecodeStatusBuffer        *pDecodeStatusBuf,
    PMOS_COMMAND_BUFFER                pCmdBufferInUse)
{
    MHW_MI_STORE_REGISTER_MEM_PARAMS  StoreRegParams;
    MhwMiInterface                    *pMiInterface;
    MhwVdboxMfxInterface              *vdboxMfxInterface;
    MmioRegistersHcp                  *pMmioRegisters;
    uint8_t                           ucPhaseIndex;
    uint8_t                           hcpDecMaxPhaseNum;
    uint32_t                          dwOffset, dwCurrIndex, dwPreIndex;
    MOS_STATUS                        eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    PCODECHAL_DECODE_SCALABILITY_STATE_G12 pScalabilityState = static_cast<PCODECHAL_DECODE_SCALABILITY_STATE_G12>(pScalabilityStateBase);

    CODECHAL_DECODE_CHK_NULL(pDecodeStatusBuf);
    CODECHAL_DECODE_CHK_NULL(pScalabilityStateBase);
    CODECHAL_DECODE_CHK_NULL(pScalabilityStateBase->pHwInterface);
    CODECHAL_DECODE_CHK_NULL(pScalabilityStateBase->pHwInterface->GetMiInterface());

    pMiInterface = pScalabilityStateBase->pHwInterface->GetMiInterface();
    pMmioRegisters = pScalabilityStateBase->pHwInterface->GetHcpInterface()->GetMmioRegisters(MHW_VDBOX_NODE_1); //relative mmio addressing.

    vdboxMfxInterface = pScalabilityStateBase->pHwInterface->GetMfxInterface();
    CODECHAL_DECODE_CHK_NULL(vdboxMfxInterface);
    hcpDecMaxPhaseNum = vdboxMfxInterface->GetNumVdbox();

    if (pScalabilityState->HcpDecPhase == CODECHAL_HCP_DECODE_PHASE_REAL_TILE)
    {
        ucPhaseIndex = pScalabilityState->u8RtCurPipe;
        dwCurrIndex = pDecodeStatusBuf->m_currIndex;
    }
    else
    {
        ucPhaseIndex = (pScalabilityState->HcpDecPhase < CODECHAL_HCP_DECODE_PHASE_FE) ?
            0 : (pScalabilityState->HcpDecPhase - CODECHAL_HCP_DECODE_PHASE_FE);

        if (ucPhaseIndex > hcpDecMaxPhaseNum)
        {
            eStatus = MOS_STATUS_INVALID_PARAMETER;
            CODECHAL_DECODE_ASSERTMESSAGE("Invalid HCP decode phase!");
            goto finish;
        }

        if (CodecHalDecodeScalabilityIsScalableMode(pScalabilityState) &&
            pScalabilityState->HcpDecPhase > CODECHAL_HCP_DECODE_PHASE_BE0)
        {
            if (pDecodeStatusBuf->m_currIndex == 0)
            {
                dwPreIndex = CODECHAL_DECODE_STATUS_NUM - 1;
            }
            else
            {
                dwPreIndex = pDecodeStatusBuf->m_currIndex - 1;
            }
        }

        dwCurrIndex = (CodecHalDecodeScalabilityIsScalableMode(pScalabilityState) &&
            pScalabilityState->HcpDecPhase > CODECHAL_HCP_DECODE_PHASE_BE0) ?
            dwPreIndex : pDecodeStatusBuf->m_currIndex;
    }

    dwOffset = (dwCurrIndex * sizeof(CodechalDecodeStatus)) +
        pDecodeStatusBuf->m_csEngineIdOffset + sizeof(uint32_t)* ucPhaseIndex +
        sizeof(uint32_t) * 2;

    StoreRegParams.presStoreBuffer = &pDecodeStatusBuf->m_statusBuffer;
    StoreRegParams.dwOffset = dwOffset;
    StoreRegParams.dwRegister = pMmioRegisters->csEngineIdOffset;
    CODECHAL_DECODE_CHK_STATUS(pMiInterface->AddMiStoreRegisterMemCmd(pCmdBufferInUse, &StoreRegParams));

finish:
    return eStatus;
}

MOS_STATUS CodecHalDecodeScalability_InitializeState_G12(
    CodechalDecode                     *pDecoder,
    PCODECHAL_DECODE_SCALABILITY_STATE  pScalabilityStateBase,
    CodechalHwInterface                *hwInterface,
    bool                                bShortFormat)
{
    PMOS_VIRTUALENGINE_INTERFACE   pVEInterface;
    MOS_VIRTUALENGINE_INIT_PARAMS  VEInitParms;
    MOS_USER_FEATURE_VALUE_DATA    UserFeatureData;
    PMOS_INTERFACE                 osInterface;
    MhwVdboxMfxInterface           *vdboxMfxInterface;
    uint8_t                        vdboxNum;
    MOS_STATUS                     eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    PCODECHAL_DECODE_SCALABILITY_STATE_G12 pScalabilityState = static_cast<PCODECHAL_DECODE_SCALABILITY_STATE_G12>(pScalabilityStateBase);

    CODECHAL_DECODE_CHK_NULL(pDecoder);
    CODECHAL_DECODE_CHK_NULL(pScalabilityState);
    CODECHAL_DECODE_CHK_NULL(hwInterface);
    osInterface = hwInterface->GetOsInterface();
    CODECHAL_DECODE_CHK_NULL(osInterface);

    vdboxMfxInterface = hwInterface->GetMfxInterface();
    CODECHAL_DECODE_CHK_NULL(vdboxMfxInterface);
    vdboxNum = vdboxMfxInterface->GetNumVdbox();

    if (vdboxNum < 2
#if (_DEBUG || _RELEASE_INTERNAL)
        || !osInterface->bHcpDecScalabilityMode
#endif
        )
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        CODECHAL_DECODE_ASSERTMESSAGE("not support scalability on this platform.");
        goto finish;
    }

    pScalabilityState->VideoContextForSP = MOS_GPU_CONTEXT_VIDEO;
    pScalabilityState->VideoContextForMP = MOS_VE_MULTINODESCALING_SUPPORTED(osInterface) ? MOS_GPU_CONTEXT_VIDEO5 : MOS_GPU_CONTEXT_VDBOX2_VIDEO;
    pScalabilityState->VideoContextFor3P = MOS_VE_MULTINODESCALING_SUPPORTED(osInterface) ? MOS_GPU_CONTEXT_VIDEO7 : MOS_GPU_CONTEXT_VDBOX2_VIDEO2;

    pScalabilityState->numDelay = 15;

#if (_DEBUG || _RELEASE_INTERNAL)
    // Reg key of the threshold for mode switch single pipe <-> 2 pipe. Using pic width value to control mode switch for now
    MOS_ZeroMemory(&UserFeatureData, sizeof(UserFeatureData));
    MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_HCP_DECODE_MODE_SWITCH_THRESHOLD1_ID,
        &UserFeatureData);
    pScalabilityState->dwHcpDecModeSwtichTh1Width = UserFeatureData.u32Data;

    MOS_ZeroMemory(&UserFeatureData, sizeof(UserFeatureData));
    MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_HCP_DECODE_MODE_SWITCH_THRESHOLD2_ID,
        &UserFeatureData);
    pScalabilityState->dwHcpDecModeSwtichTh2Width = UserFeatureData.u32Data;

    // Reg key to control hevc real tile decoding
    MOS_ZeroMemory(&UserFeatureData, sizeof(UserFeatureData));
    MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_DISABLE_HEVC_REALTILE_DECODE_ID_G12,
        &UserFeatureData);
    pScalabilityState->bDisableRtMode = (UserFeatureData.u32Data != 0);

    // Reg key to control hevc real tile multi-phase decoding
    MOS_ZeroMemory(&UserFeatureData, sizeof(UserFeatureData));
    MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_ENABLE_HEVC_REALTILE_MULTI_PHASE_DECODE_ID_G12,
        &UserFeatureData);
    pScalabilityState->bEnableRtMultiPhase = (UserFeatureData.u32Data != 0);

    MOS_ZeroMemory(&UserFeatureData, sizeof(UserFeatureData));
    MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_SCALABILITY_OVERRIDE_SPLIT_WIDTH_IN_MINCB,
        &UserFeatureData);
    pScalabilityState->dbgOvrdWidthInMinCb = UserFeatureData.u32Data;

    MOS_ZeroMemory(&UserFeatureData, sizeof(UserFeatureData));
    MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_HCP_DECODE_USER_PIPE_NUM_ID_G12,
        &UserFeatureData);
    pScalabilityState->dbgOverUserPipeNum = (uint8_t)UserFeatureData.u32Data;
#endif

    // enable FE separate submission by default in multi-pipe mode
    if((vdboxNum > 2) && osInterface->phasedSubmission)
    {
        pScalabilityState->bFESeparateSubmission = true;
    }
    else if ((vdboxNum > 2)
#if (_DEBUG || _RELEASE_INTERNAL)
        && (((pScalabilityState->dbgOverUserPipeNum >= 3)
        && (pScalabilityState->dbgOverUserPipeNum <= vdboxNum)))
#endif
        )
    {
        pScalabilityState->bFESeparateSubmission = true;
    }
    else
    {
        pScalabilityState->bFESeparateSubmission = false;
    }

#if (_DEBUG || _RELEASE_INTERNAL)
    if (osInterface->bEnableDbgOvrdInVE)
    {
        //if DbgOverride is enabled, FE separate submission is not supported
        pScalabilityState->bFESeparateSubmission = false;
    }
#endif

    if (pScalabilityState->bFESeparateSubmission)
    {
        MOS_GPU_CONTEXT         GpuContext = MOS_VE_CTXBASEDSCHEDULING_SUPPORTED(osInterface) ? MOS_GPU_CONTEXT_VIDEO : MOS_GPU_CONTEXT_VIDEO4;
        GpuContext                         = MOS_VE_MULTINODESCALING_SUPPORTED(osInterface) ? MOS_GPU_CONTEXT_VIDEO4 : GpuContext;
        MOS_GPUCTX_CREATOPTIONS createOpts;

        CODECHAL_DECODE_CHK_STATUS(osInterface->pfnCreateGpuContext(
            osInterface,
            GpuContext,
            MOS_GPU_NODE_VIDEO,
            &createOpts));
        pScalabilityState->VideoContextForFE = GpuContext;
    }

    pScalabilityState->Standard = pDecoder->GetStandard();
    pScalabilityState->VideoContext = pDecoder->GetVideoContext();
    pScalabilityState->bShortFormatInUse = bShortFormat;
    pScalabilityState->ucNumVdbox = vdboxNum;
    pScalabilityState->pHwInterface = hwInterface;

    //virtual engine init with scalability
    MOS_ZeroMemory(&VEInitParms, sizeof(VEInitParms));
    VEInitParms.bScalabilitySupported = true;
    VEInitParms.bFESeparateSubmit = pScalabilityState->bFESeparateSubmission;
    VEInitParms.ucMaxNumPipesInUse = CodecHalDecodeMaxNumPipesInUseG12(vdboxNum);
    VEInitParms.ucNumOfSdryCmdBufSets = CODECHAL_SCALABILITY_DECODE_SECONDARY_CMDBUFSET_NUM;
    VEInitParms.ucMaxNumOfSdryCmdBufInOneFrame = pScalabilityState->bFESeparateSubmission ? VEInitParms.ucMaxNumPipesInUse : (VEInitParms.ucMaxNumPipesInUse + 1);
    CODECHAL_DECODE_CHK_STATUS(Mos_VirtualEngineInterface_Initialize(osInterface, &VEInitParms));
    pScalabilityState->pVEInterface = pVEInterface = osInterface->pVEInterf;

    if (pVEInterface->pfnVEGetHintParams)
    {
        CODECHAL_DECODE_CHK_STATUS(pVEInterface->pfnVEGetHintParams(pVEInterface, true, &pScalabilityState->pScalHintParms));
    }
    if (pVEInterface->pfnVEGetHintParams)
    {
        CODECHAL_DECODE_CHK_STATUS(pVEInterface->pfnVEGetHintParams(pVEInterface, false, &pScalabilityState->pSingleHintParms));
    }

#if (_DEBUG || _RELEASE_INTERNAL)
    MOS_ZeroMemory(&UserFeatureData, sizeof(UserFeatureData));
    MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_HCP_DECODE_ALWAYS_FRAME_SPLIT_ID,
        &UserFeatureData);
    pScalabilityState->bAlwaysFrameSplit = UserFeatureData.u32Data ? true : false;
#endif

    pScalabilityState->bIsEvenSplit = true;

    pScalabilityState->bIsEnableEndCurrentBatchBuffLevel = MEDIA_IS_SKU(hwInterface->GetSkuTable(), FtrConditionalBatchBuffEnd);

    if (pDecoder->GetStandard() == CODECHAL_HEVC)
    {
        pScalabilityState->pfnGetHcpBufferSize = GetHevcBufferSize;
        pScalabilityState->pfnIsHcpBufferReallocNeeded = IsHevcBufferReallocNeeded;
    }
    else if (pDecoder->GetStandard() == CODECHAL_VP9)
    {
        pScalabilityState->pfnGetHcpBufferSize = GetVp9BufferSize;
        pScalabilityState->pfnIsHcpBufferReallocNeeded = IsVp9BufferReallocNeeded;
    }
    else
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        CODECHAL_DECODE_ASSERTMESSAGE("unsupported decode format for scalability mode.");
        goto finish;
    }

    pScalabilityState->bToggleCABACStreamOutBuffer = false;
    pScalabilityState->pfnDecidePipeNum = CodecHalDecodeScalability_DecidePipeNum_G12;
    pScalabilityState->pfnMapPipeNumToLRCACount = CodechalDecodeScalability_MapPipeNumToLRCACount_G12;
#if (_DEBUG || _RELEASE_INTERNAL)
    pScalabilityState->pfnDebugOvrdDecidePipeNum = CodechalDecodeScalability_DebugOvrdDecidePipeNum_G12;
#endif

    pScalabilityState->sliceStateCLs = CODECHAL_SCALABILITY_SLICE_STATE_CACHELINES_PER_SLICE_TGL;
    CODECHAL_DECODE_CHK_STATUS(CodecHalDecodeScalability_AllocateResources_FixedSizes(pScalabilityState));

finish:
    return eStatus;
}

MOS_STATUS CodecHalDecodeScalability_AdvanceRealTilePass(
    PCODECHAL_DECODE_SCALABILITY_STATE  pScalabilityStateBase)
{
    auto eStatus = MOS_STATUS_SUCCESS;

    PCODECHAL_DECODE_SCALABILITY_STATE_G12 pScalabilityState = static_cast<PCODECHAL_DECODE_SCALABILITY_STATE_G12>(pScalabilityStateBase);

    CODECHAL_DECODE_CHK_NULL_RETURN(pScalabilityStateBase);
    CODECHAL_DECODE_ASSERT(pScalabilityState->bIsRtMode);

    pScalabilityState->u8RtCurPipe++;
    if (pScalabilityState->u8RtCurPipe >= pScalabilityState->ucScalablePipeNum)
    {
        pScalabilityState->u8RtCurPipe = 0;
        pScalabilityState->u8RtCurPhase++;
    }

    CODECHAL_DECODE_ASSERT(pScalabilityState->u8RtCurPhase < pScalabilityState->u8RtPhaseNum);

    return eStatus;
}

MOS_STATUS CodecHalDecodeScalability_GetCurrentRealTileColumnId(
    PCODECHAL_DECODE_SCALABILITY_STATE  pScalabilityStateBase,
    uint8_t                             &col)
{
    auto eStatus = MOS_STATUS_SUCCESS;

    PCODECHAL_DECODE_SCALABILITY_STATE_G12 pScalabilityState = static_cast<PCODECHAL_DECODE_SCALABILITY_STATE_G12>(pScalabilityStateBase);

    CODECHAL_DECODE_CHK_NULL_RETURN(pScalabilityStateBase);

    col = pScalabilityState->u8RtCurPipe + pScalabilityState->u8RtCurPhase * pScalabilityState->ucScalablePipeNum;
    return eStatus;
}

MOS_STATUS CodecHalDecodeScalability_SetSfcState(
    PCODECHAL_DECODE_SCALABILITY_STATE          scalabilityStateBase,
    void                                       *picParams,
    CODECHAL_RECTANGLE                         *srcRegion,
    CODECHAL_RECTANGLE                         *dstRegion,
    PCODECHAL_DECODE_SFC_SCALABILITY_PARAMS     sfcScalabilityParams)
{
    MOS_STATUS  eStatus = MOS_STATUS_SUCCESS;
    uint32_t    minCbSize, maxCbSize, widthInPixel, widthInMaxCb;
    uint32_t    srcStartX = 0, srcEndX = 0;
    uint32_t    engMode, tileType;
    uint32_t    tileColumnIndex = 0;
    uint32_t    tileColumnCount = 0;
    bool        isInput444;
    uint32_t    ildbXOffset;
    uint64_t    oneBySf;
    double      xLandingPoint;
    uint32_t    srcEndXTemp;
    uint32_t    xOffset;
    uint32_t    tileEndX;
    uint32_t    dstStartX, dstEndX;

    PCODECHAL_DECODE_SCALABILITY_STATE_G12 scalabilityState = static_cast<PCODECHAL_DECODE_SCALABILITY_STATE_G12>(scalabilityStateBase);

    CODECHAL_DECODE_CHK_NULL_RETURN(scalabilityStateBase);
    CODECHAL_DECODE_CHK_NULL_RETURN(sfcScalabilityParams);
    CODECHAL_DECODE_CHK_NULL_RETURN(srcRegion);
    CODECHAL_DECODE_CHK_NULL_RETURN(dstRegion);
    CODECHAL_DECODE_CHK_NULL_RETURN(picParams);

    if (!CodecHalDecodeScalabilityIsScalableMode(scalabilityState))
    {
        return MOS_STATUS_SUCCESS;
    }

    if (scalabilityState->Standard == CODECHAL_HEVC)
    {
        PCODEC_HEVC_PIC_PARAMS  hevcPicParams = (PCODEC_HEVC_PIC_PARAMS)picParams;

        minCbSize = 1 << (hevcPicParams->log2_min_luma_coding_block_size_minus3 + 3);
        maxCbSize = (minCbSize << hevcPicParams->log2_diff_max_min_luma_coding_block_size);
        widthInPixel = hevcPicParams->PicWidthInMinCbsY * minCbSize;
        widthInMaxCb = MOS_ROUNDUP_DIVIDE(widthInPixel, maxCbSize);
        isInput444 = (hevcPicParams->chroma_format_idc == HCP_CHROMA_FORMAT_YUV444);
    }
    else if (scalabilityState->Standard == CODECHAL_VP9)
    {
        PCODEC_VP9_PIC_PARAMS vp9PicParams = (PCODEC_VP9_PIC_PARAMS)picParams;

        minCbSize = CODEC_VP9_MIN_BLOCK_WIDTH;
        maxCbSize = CODEC_VP9_SUPER_BLOCK_WIDTH;
        widthInPixel = vp9PicParams->FrameWidthMinus1 + 1;
        widthInMaxCb = MOS_ROUNDUP_DIVIDE(widthInPixel, maxCbSize);
        isInput444 = (vp9PicParams->subsampling_x == 0 && vp9PicParams->subsampling_y == 0);
    }
    else
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        CODECHAL_DECODE_ASSERTMESSAGE("invalid codec type, only HEVC/VP9 are supported in scalability mode.");
        return eStatus;
    }

    if (scalabilityState->bIsRtMode)
    {
        PCODEC_HEVC_PIC_PARAMS  hevcPicParams = (PCODEC_HEVC_PIC_PARAMS)picParams;
        uint8_t col = scalabilityState->u8RtCurPipe + scalabilityState->u8RtCurPhase * scalabilityState->ucScalablePipeNum;

        if (scalabilityState->u8RtCurPipe == 0)
        {
            engMode = 1;
        }
        else if (scalabilityState->u8RtCurPipe == scalabilityState->ucScalablePipeNum - 1)
        {
            engMode = 2;
        }
        else
        {
            engMode = 3;
        }

        if (hevcPicParams->uniform_spacing_flag == 1)
        {
            srcStartX = (col * widthInMaxCb) / (hevcPicParams->num_tile_columns_minus1 + 1) * maxCbSize;
            srcEndX = ((col + 1) * widthInMaxCb) / (hevcPicParams->num_tile_columns_minus1 + 1) * maxCbSize - 1;
        }
        else
        {
            for (uint8_t i = 0; i < col; i++)
            {
                srcStartX += (hevcPicParams->column_width_minus1[i] + 1) * maxCbSize;
            }
            if (col == hevcPicParams->num_tile_columns_minus1)
            {
                srcEndX = srcRegion->X + srcRegion->Width - 1;
            }
            else
            {
                srcEndX = srcStartX + (hevcPicParams->column_width_minus1[col] + 1) * maxCbSize - 1;
            }
        }

        tileType = 0;
        tileColumnIndex = col;
        tileColumnCount = hevcPicParams->num_tile_columns_minus1 + 1;
    }
    else if (CodecHalDecodeScalabilityIsBEPhaseG12(scalabilityState))
    {
        uint8_t pipeIndex = scalabilityState->HcpDecPhase - CODECHAL_HCP_DECODE_PHASE_BE0;

        if (pipeIndex == 0)
        {
            engMode = 1;
        }
        else if (pipeIndex == scalabilityState->ucScalablePipeNum - 1)
        {
            engMode = 2;
        }
        else
        {
            engMode = 3;
        }

        srcStartX = pipeIndex * widthInMaxCb / scalabilityState->ucScalablePipeNum;
        srcStartX *= maxCbSize;
        if (pipeIndex == scalabilityState->ucScalablePipeNum - 1)
        {
            srcEndX = srcRegion->X + srcRegion->Width - 1;
        }
        else
        {
            srcEndX = (pipeIndex + 1) * widthInMaxCb / scalabilityState->ucScalablePipeNum;
            srcEndX = srcEndX * maxCbSize - 1;
        }
#if (_DEBUG || _RELEASE_INTERNAL)
        if (scalabilityState->dbgOvrdWidthInMinCb && scalabilityState->ucScalablePipeNum == 2)
        {
            if (pipeIndex == 1)
            {
                srcStartX = scalabilityState->dbgOvrdWidthInMinCb * minCbSize;
                srcEndX = srcRegion->X + srcRegion->Width - 1;
            }
            else
            {
                srcStartX = 0;
                srcEndX = scalabilityState->dbgOvrdWidthInMinCb * minCbSize - 1;
            }
        }
#endif

        tileType = 1;
        tileColumnIndex = pipeIndex;
        tileColumnCount = scalabilityState->ucScalablePipeNum;
    }
    else
    {
        engMode = 0;
        tileType = 0;
        srcStartX = 0;
        srcEndX = srcRegion->X + srcRegion->Width - 1;
    }

    // Clamp srcStartX, srcEndX into source region
    if (srcStartX < srcRegion->X)
        srcStartX = srcRegion->X;
    if (srcEndX > srcRegion->X + srcRegion->Width - 1)
        srcEndX = srcRegion->X + srcRegion->Width - 1;

    if (tileColumnIndex == 0)
    {
        scalabilityState->fistValidTileIndex = 0;
        scalabilityState->lastValidTileIndex = tileColumnCount - 1;
        scalabilityState->dstXLandingCount = 0;
    }

    ildbXOffset = (scalabilityState->Standard == CODECHAL_HEVC) ? 5 : 8;  // 1 : HEVC; 0 : VP9
    oneBySf = (uint64_t)((double)((uint64_t)(srcRegion->Width) * 524288 / (dstRegion->Width)));

    //------------------ start ildb offset correction -----------------------------//
    srcEndXTemp = srcEndX - ildbXOffset;

    //---------------------- destination startX determination logic ---------------//
    if ((srcRegion->X + srcRegion->Width - 1) <= srcEndXTemp)
    {
        xOffset = 0;
        tileEndX = (srcRegion->X + srcRegion->Width);
    }
    else
    {
        xOffset = isInput444 ? 3 : (scalabilityState->Standard == CODECHAL_HEVC) ? 8 : 11;
        tileEndX = srcEndXTemp;
    }

    while (true)
    {
        if (srcEndXTemp - srcRegion->X < (xOffset + 1))
        {
            dstEndX = 0;
            break;
        }
        if (scalabilityState->dstXLandingCount == 0)
        {
            scalabilityState->fistValidTileIndex = tileColumnIndex;
        }

        // xLandingpoint = (float)(((max(0, (((float)dest_cntX * (float)one_by_sf) + Xphaseshift)) + ((float)1 << (one_by sf_fraction_precision - beta_precision - 1))) / 524288) + SourceRegionHorizontalOffset)
        //
        {
            const uint32_t one_by_sf_fraction_precision = 19;
            const uint32_t beta_precision = 5;
            uint32_t xPhaseShift = MOS_CLAMP_MIN_MAX(MOS_F_ROUND((((double)srcRegion->Width / dstRegion->Width - 1.0) / 2.0) * 524288.0F), -(1 << (4 + 19)), ((1 << (4 + 19)) - 1));

            double tempDestCntx = (((double)scalabilityState->dstXLandingCount * (double)oneBySf) + xPhaseShift);
            if (tempDestCntx < 0)
            {
                tempDestCntx = 0;
            }
            xLandingPoint = (double)(((tempDestCntx + ((double)(1 << (one_by_sf_fraction_precision - beta_precision - 1)))) / 524288) + srcRegion->X);
        }

        if (xLandingPoint >= (double)(tileEndX - xOffset))
        {
            dstEndX = scalabilityState->dstXLandingCount - 1;
            break;
        }
        else
        {
            scalabilityState->dstXLandingCount++;
        }
    }

    if (xOffset == 0)
    {
        scalabilityState->lastValidTileIndex = tileColumnIndex;
    }

    // Last column end at destination region right border.
    if (tileColumnIndex == scalabilityState->lastValidTileIndex)
    {
        dstEndX = dstRegion->X + dstRegion->Width - 1;
    }

    if (tileColumnIndex <= scalabilityState->fistValidTileIndex)
        dstStartX = 0;
    else if (tileColumnIndex <= scalabilityState->lastValidTileIndex)
    {
        dstStartX = scalabilityState->lastDstEndX + 1;
    }
    else
    {
        dstStartX = 0;
        dstEndX = 0;
    }
    scalabilityState->lastDstEndX = dstEndX;

    sfcScalabilityParams->engineMode = engMode;
    sfcScalabilityParams->tileType = tileType;
    sfcScalabilityParams->srcStartX = srcStartX;
    sfcScalabilityParams->srcEndX = srcEndX;
    sfcScalabilityParams->dstStartX = dstStartX;
    sfcScalabilityParams->dstEndX = dstEndX;

    return eStatus;
}

bool CodecHalDecodeScalabilityIsToSubmitCmdBuffer_G12(
    PCODECHAL_DECODE_SCALABILITY_STATE pScalabilityStateBase)
{
    PCODECHAL_DECODE_SCALABILITY_STATE_G12 pScalabilityState = static_cast<PCODECHAL_DECODE_SCALABILITY_STATE_G12>(pScalabilityStateBase);

    if (pScalabilityState == nullptr)
    {
        return false;
    }
    else
    {
        return (CodecHalDecodeScalabilityIsFinalBEPhaseG12(pScalabilityState) ||
            (pScalabilityState->HcpDecPhase == CODECHAL_HCP_DECODE_PHASE_FE && pScalabilityState->bFESeparateSubmission) ||
            CodecHalDecodeScalabilityIsLastRealTilePass(pScalabilityState));
    }
}

MOS_STATUS CodecHalDecodeScalability_AllocateResources_VariableSizes_G12(
    PCODECHAL_DECODE_SCALABILITY_STATE   pScalabilityState,
    PMHW_VDBOX_HCP_BUFFER_SIZE_PARAMS    pHcpBufSizeParam,
    PMHW_VDBOX_HCP_BUFFER_REALLOC_PARAMS pAllocParam)
{
    MOS_ALLOC_GFXRES_PARAMS AllocParamsForBufferLinear;
    PMOS_INTERFACE          pOsInterface;
    MOS_STATUS              eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    CODECHAL_DECODE_CHK_NULL_RETURN(pScalabilityState);
    CODECHAL_DECODE_CHK_NULL_RETURN(pScalabilityState->pHwInterface);
    CODECHAL_DECODE_CHK_NULL_RETURN(pScalabilityState->pHwInterface->GetOsInterface());
    CODECHAL_DECODE_CHK_NULL_RETURN(pHcpBufSizeParam);
    CODECHAL_DECODE_CHK_NULL_RETURN(pAllocParam);

    CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHalDecodeScalability_AllocateResources_VariableSizes(pScalabilityState, pHcpBufSizeParam, pAllocParam));

    // for multi-pipe scalability mode
    if (pScalabilityState->ucNumVdbox > 2)
    {
        if (pScalabilityState->bFESeparateSubmission)
        {
            for (int i = 1; i < CODECHAL_HCP_STREAMOUT_BUFFER_MAX_NUM; i++)
            {
                CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHalDecodeScalability_AllocateCABACStreamOutBuffer(pScalabilityState,
                    pHcpBufSizeParam,
                    pAllocParam,
                    &pScalabilityState->resCABACSyntaxStreamOutBuffer[i]));
            }

            pScalabilityState->presCABACStreamOutBuffer =
                pScalabilityState->bToggleCABACStreamOutBuffer ? &pScalabilityState->resCABACSyntaxStreamOutBuffer[1] : &pScalabilityState->resCABACSyntaxStreamOutBuffer[0];

            pScalabilityState->bToggleCABACStreamOutBuffer = !pScalabilityState->bToggleCABACStreamOutBuffer;
        }
    }

    return eStatus;
}

MOS_STATUS CodecHalDecodeScalability_DecidePipeNum_G12(
    PCODECHAL_DECODE_SCALABILITY_STATE         pScalState,
    PCODECHAL_DECODE_SCALABILITY_INIT_PARAMS   pInitParams)
{
    PMOS_VIRTUALENGINE_INTERFACE pVEInterface;
    MOS_STATUS                   eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    CODECHAL_DECODE_CHK_NULL_RETURN(pScalState);
    CODECHAL_DECODE_CHK_NULL_RETURN(pScalState->pVEInterface);
    CODECHAL_DECODE_CHK_NULL_RETURN(pInitParams);

    pVEInterface                                                     = pScalState->pVEInterface;
    pScalState->ucScalablePipeNum                                    = CODECHAL_DECODE_HCP_Legacy_PIPE_NUM_1;
    PCODECHAL_DECODE_SCALABILITY_STATE_G12       pScalStateG12        = static_cast<PCODECHAL_DECODE_SCALABILITY_STATE_G12>(pScalState);
    PCODECHAL_DECODE_SCALABILITY_INIT_PARAMS_G12 pInitParamsG12       = static_cast<PCODECHAL_DECODE_SCALABILITY_INIT_PARAMS_G12>(pInitParams);
    uint8_t                                     u8MaxTileColumn      = HEVC_NUM_MAX_TILE_COLUMN;
    bool                                        bCanEnableRealTile   = true;
#if (_DEBUG || _RELEASE_INTERNAL)
    bCanEnableRealTile = !(static_cast<PCODECHAL_DECODE_SCALABILITY_STATE_G12>(pScalState))->bDisableRtMode;
    if (!pScalStateG12->bEnableRtMultiPhase)
        u8MaxTileColumn = 2;
#endif
    bCanEnableRealTile = bCanEnableRealTile && pInitParamsG12->bIsTileEnabled && (pInitParams->u8NumTileColumns > 1) &&
                         (pInitParams->u8NumTileColumns <= u8MaxTileColumn) && (pInitParams->u8NumTileRows <= HEVC_NUM_MAX_TILE_ROW);

    if (pInitParams->usingSFC)
    {
        //using SFC can only work in single pipe mode.
        return MOS_STATUS_SUCCESS;
    }

#if (_DEBUG || _RELEASE_INTERNAL)
    if (pScalState->bAlwaysFrameSplit)
    {
        if (pScalState->ucNumVdbox != 1)
        {
            if (pScalState->ucNumVdbox == 2)
            {
                pScalState->ucScalablePipeNum = CODECHAL_DECODE_HCP_SCALABLE_PIPE_NUM_2;
            }
            else
            {
                uint8_t dbgOverUserPipeNum    = pScalStateG12->dbgOverUserPipeNum;
                pScalState->ucScalablePipeNum = CODECHAL_DECODE_HCP_SCALABLE_PIPE_NUM_2;
                if ((dbgOverUserPipeNum >= CODECHAL_DECODE_HCP_SCALABLE_PIPE_NUM_2) && (dbgOverUserPipeNum <= pScalState->ucNumVdbox))
                {
                    pScalState->ucScalablePipeNum = dbgOverUserPipeNum;
                }
            }
        }
    }
    else
#endif
    {
        if (pScalState->ucNumVdbox != 1)
        {
            if (pScalState->ucNumVdbox == 2)
            {
                if (pScalState->dwHcpDecModeSwtichTh1Width != 0)
                {
                    if (pInitParams->u32PicWidthInPixel >= pScalState->dwHcpDecModeSwtichTh1Width)
                    {
                        pScalState->ucScalablePipeNum = CODECHAL_DECODE_HCP_SCALABLE_PIPE_NUM_2;
                    }
                }
                else if ((!CodechalDecodeNonRextFormat(pInitParams->format)
                                    && CodechalDecodeResolutionEqualLargerThan4k(pInitParams->u32PicWidthInPixel, pInitParams->u32PicHeightInPixel))
                                || (CodechalDecodeNonRextFormat(pInitParams->format)
                                    && CodechalDecodeResolutionEqualLargerThan5k(pInitParams->u32PicWidthInPixel, pInitParams->u32PicHeightInPixel))
                                || (bCanEnableRealTile && !pInitParams->usingSecureDecode))
                {
                    pScalState->ucScalablePipeNum = CODECHAL_DECODE_HCP_SCALABLE_PIPE_NUM_2;
                }

                if (pScalState->bIsEvenSplit == false)
                {
                    // disable scalability for clips with width less than split condition when MMC is on
                    if (pInitParams->u32PicWidthInPixel <= CODEC_SCALABILITY_FIRST_TILE_WIDTH_4K)
                    {
                        pScalState->ucScalablePipeNum = CODECHAL_DECODE_HCP_Legacy_PIPE_NUM_1;
                    }
                }
            }
            else
            {
                if (pScalState->dwHcpDecModeSwtichTh1Width != 0 &&
                    pScalState->dwHcpDecModeSwtichTh2Width != 0)
                {
                    if (pInitParams->u32PicWidthInPixel >= pScalState->dwHcpDecModeSwtichTh2Width)
                    {
                        pScalState->ucScalablePipeNum = CODECHAL_DECODE_HCP_SCALABLE_PIPE_NUM_RESERVED;
                    }
                    else if (pInitParams->u32PicWidthInPixel >= pScalState->dwHcpDecModeSwtichTh1Width)
                    {
                        pScalState->ucScalablePipeNum = CODECHAL_DECODE_HCP_SCALABLE_PIPE_NUM_2;
                    }
                }
                else
                {
                    if ((pInitParams->u32PicWidthInPixel * pInitParams->u32PicHeightInPixel) >= (CODECHAL_HCP_DECODE_SCALABLE_THRESHOLD4_WIDTH * CODECHAL_HCP_DECODE_SCALABLE_THRESHOLD4_HEIGHT))
                    {
                        pScalState->ucScalablePipeNum = CODECHAL_DECODE_HCP_SCALABLE_PIPE_NUM_RESERVED;
                    }
                    else if ((!CodechalDecodeNonRextFormat(pInitParams->format)
                                    && CodechalDecodeResolutionEqualLargerThan4k(pInitParams->u32PicWidthInPixel, pInitParams->u32PicHeightInPixel))
                                || (CodechalDecodeNonRextFormat(pInitParams->format)
                                    && CodechalDecodeResolutionEqualLargerThan5k(pInitParams->u32PicWidthInPixel, pInitParams->u32PicHeightInPixel))
                                || (bCanEnableRealTile && !pInitParams->usingSecureDecode))
                    {
                        pScalState->ucScalablePipeNum = CODECHAL_DECODE_HCP_SCALABLE_PIPE_NUM_2;
                    }
                }
            }
        }
    }

    return eStatus;
}

MOS_STATUS CodechalDecodeScalability_MapPipeNumToLRCACount_G12(
    PCODECHAL_DECODE_SCALABILITY_STATE   pScalState,
    uint32_t                             *LRCACount)
{
    MOS_STATUS    eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    CODECHAL_DECODE_CHK_NULL_RETURN(pScalState);
    CODECHAL_DECODE_CHK_NULL_RETURN(LRCACount);

    *LRCACount = 1; // initialzed to 1.

    switch (pScalState->ucScalablePipeNum)
    {
    case CODECHAL_DECODE_HCP_SCALABLE_PIPE_NUM_2:
        // on GT2 or debug override enabled, FE separate submission = false, FE run on the same engine of BEs;
        // on GT3, FE separate submission = true, scalability submission includes only BEs.
        *LRCACount = 2;
        break;
    case CODECHAL_DECODE_HCP_Legacy_PIPE_NUM_1:
        *LRCACount = 1;
        break;
    default:
        // in release driver bFESeparateSubmission is always false since this is on GT3 or GT4.
        // bFESeparateSubmission could be false if debug override enabled.
        if (pScalState->bFESeparateSubmission || (static_cast<PCODECHAL_DECODE_SCALABILITY_STATE_G12>(pScalState))->bIsRtMode)
        {
            *LRCACount = pScalState->ucScalablePipeNum;
        }
        else
        {
            *LRCACount = pScalState->ucScalablePipeNum + 1;
        }
    }

    if (*LRCACount > pScalState->ucNumVdbox)
    {
        CODECHAL_DECODE_ASSERTMESSAGE("LRCA count can not exceed vdbox number.");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    return eStatus;
}

#if (_DEBUG || _RELEASE_INTERNAL)
MOS_STATUS CodechalDecodeScalability_DebugOvrdDecidePipeNum_G12(
    PCODECHAL_DECODE_SCALABILITY_STATE         pScalState)
{
    PMOS_VIRTUALENGINE_INTERFACE pVEInterface;
    MOS_STATUS                   eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_CHK_NULL_RETURN(pScalState);
    CODECHAL_DECODE_CHK_NULL_RETURN(pScalState->pVEInterface);

    pVEInterface = pScalState->pVEInterface;

    if (g_apoMosEnabled)
    {
        CODECHAL_DECODE_CHK_NULL_RETURN(pVEInterface->veInterface);
        auto veInterface = pVEInterface->veInterface;
        if (veInterface->GetEngineCount() > pScalState->ucNumVdbox)
        {
            CODECHAL_DECODE_ASSERTMESSAGE("invalid parameter settings in debug override.");
            return MOS_STATUS_INVALID_PARAMETER;
        }
        else if (veInterface->GetEngineCount() == 1)
        {
            pScalState->ucScalablePipeNum = CODECHAL_DECODE_HCP_Legacy_PIPE_NUM_1;
        }
        else if (veInterface->GetEngineCount() == 2)
        {
            //engine count = 2, only support FE run on the same engine as one of BE for now.
            pScalState->ucScalablePipeNum = CODECHAL_DECODE_HCP_SCALABLE_PIPE_NUM_2;
        }
        else
        {
            bool bEngineLogicIdValid = false;
            for (uint8_t i = 0; i < veInterface->GetEngineCount() - 1; i++)
            {
                if (veInterface->GetEngineLogicId(veInterface->GetEngineCount() - 1) !=
                    veInterface->GetEngineLogicId(i))
                {
                    bEngineLogicIdValid = true;
                }
                else
                {
                    bEngineLogicIdValid = false;
                    break;
                }
            }
            if (bEngineLogicIdValid)
            {
                pScalState->ucScalablePipeNum = pScalState->ucNumVdbox - 1;
            }
        }

        return eStatus;
    }

    // debug override for virtual tile
    if (pVEInterface->ucEngineCount > pScalState->ucNumVdbox)
    {
        CODECHAL_DECODE_ASSERTMESSAGE("invalid parameter settings in debug override.");
        return MOS_STATUS_INVALID_PARAMETER;
    }
    else if (pVEInterface->ucEngineCount == 1)
    {
        pScalState->ucScalablePipeNum = CODECHAL_DECODE_HCP_Legacy_PIPE_NUM_1;
    }
    else if (pVEInterface->ucEngineCount == 2)
    {
        //engine count = 2, only support FE run on the same engine as one of BE for now.
        pScalState->ucScalablePipeNum = CODECHAL_DECODE_HCP_SCALABLE_PIPE_NUM_2;
    }
    else
    {
        bool bEngineLogicIdValid = false;
        for (uint8_t i = 0; i < pVEInterface->ucEngineCount - 1; i++)
        {
            if (pVEInterface->EngineLogicId[pVEInterface->ucEngineCount - 1] != pVEInterface->EngineLogicId[i])
            {
                bEngineLogicIdValid = true;
            }
            else
            {
                bEngineLogicIdValid = false;
                break;
            }
        }
        if (bEngineLogicIdValid)
        {
            pScalState->ucScalablePipeNum = pScalState->ucNumVdbox - 1;
        }
    }

    return eStatus;
}
#endif

void CodecHalDecodeScalability_DecPhaseToSubmissionType_G12(
    PCODECHAL_DECODE_SCALABILITY_STATE_G12 pScalabilityState,
    PMOS_COMMAND_BUFFER pCmdBuffer)
{
    switch (pScalabilityState->HcpDecPhase)
    {
        case CodechalDecode::CodechalHcpDecodePhaseLegacyS2L:
            //Note: no break here, S2L and FE commands put in one secondary command buffer.
        case CODECHAL_HCP_DECODE_PHASE_FE:
            pCmdBuffer->iSubmissionType = SUBMISSION_TYPE_MULTI_PIPE_ALONE;
            break;
        case CODECHAL_HCP_DECODE_PHASE_BE0:
            pCmdBuffer->iSubmissionType = SUBMISSION_TYPE_MULTI_PIPE_MASTER;
            break;
        case CODECHAL_HCP_DECODE_PHASE_REAL_TILE:
            if (pScalabilityState->u8RtCurPipe == 0)
            {
                pCmdBuffer->iSubmissionType = SUBMISSION_TYPE_MULTI_PIPE_MASTER;
            }
            else
            {
                pCmdBuffer->iSubmissionType = SUBMISSION_TYPE_MULTI_PIPE_SLAVE;
                pCmdBuffer->iSubmissionType |= ((pScalabilityState->u8RtCurPipe - 1) << SUBMISSION_TYPE_MULTI_PIPE_SLAVE_INDEX_SHIFT);
            }

            if (pScalabilityState->u8RtCurPhase == pScalabilityState->u8RtPhaseNum - 1)
            {
                if (pScalabilityState->u8RtCurPipe == pScalabilityState->u8RtPipeInLastPhase - 1)
                {
                    pCmdBuffer->iSubmissionType |= SUBMISSION_TYPE_MULTI_PIPE_FLAGS_LAST_PIPE;
                }
            }
            else
            {
                if (pScalabilityState->u8RtCurPipe == pScalabilityState->ucScalablePipeNum-1)
                {
                    pCmdBuffer->iSubmissionType |= SUBMISSION_TYPE_MULTI_PIPE_FLAGS_LAST_PIPE;
                }
            }
            break;
        default:
            if((pScalabilityState->HcpDecPhase > CODECHAL_HCP_DECODE_PHASE_BE0) &&
                ((pScalabilityState->HcpDecPhase - CODECHAL_HCP_DECODE_PHASE_FE) <= pScalabilityState->ucScalablePipeNum))
            {
                pCmdBuffer->iSubmissionType = SUBMISSION_TYPE_MULTI_PIPE_SLAVE;
                pCmdBuffer->iSubmissionType |= ((pScalabilityState->HcpDecPhase - CODECHAL_HCP_DECODE_PHASE_BE1) << SUBMISSION_TYPE_MULTI_PIPE_SLAVE_INDEX_SHIFT);
                if ((pScalabilityState->HcpDecPhase - CODECHAL_HCP_DECODE_PHASE_FE) == pScalabilityState->ucScalablePipeNum)
                {
                    pCmdBuffer->iSubmissionType |= SUBMISSION_TYPE_MULTI_PIPE_FLAGS_LAST_PIPE;
                }
            }
            else
            {
                pCmdBuffer->iSubmissionType = SUBMISSION_TYPE_MULTI_PIPE_ALONE;
            }
    }
}
