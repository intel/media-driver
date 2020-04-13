/*
* Copyright (c) 2016-2019, Intel Corporation
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
//! \file     codechal_decode_scalability.cpp
//! \brief    Implements the decode interface extension for decode scalability.
//! \details  Implements all functions required by CodecHal for scalability decoding.
//!

#include "codechal_decoder.h"
#include "codechal_decode_scalability.h"
#include "mos_util_user_interface.h"
#include "mos_solo_generic.h"
#include "mos_os_virtualengine_next.h"

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
static MOS_STATUS CodecHalDecodeScalability_CalculateScdryCmdBufIndex(
    PCODECHAL_DECODE_SCALABILITY_STATE  pScalabilityState,
    uint32_t                           *pdwBufIdxPlus1)
{
    uint32_t                        HcpDecPhaseForBufIdx;
    MOS_STATUS                      eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    CODECHAL_DECODE_CHK_NULL_RETURN(pScalabilityState);
    CODECHAL_DECODE_CHK_NULL_RETURN(pdwBufIdxPlus1);

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
static MOS_STATUS CodecHalDecodeScalability_CheckDecPhaseValidity(
    PCODECHAL_DECODE_SCALABILITY_STATE  pScalabilityState,
    uint32_t                            HcpDecPhase)
{
    bool                bInValidPhase = false;
    MOS_STATUS          eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    CODECHAL_DECODE_CHK_NULL_RETURN(pScalabilityState);

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
        case CODECHAL_HCP_DECODE_PHASE_BE1:
        case CODECHAL_HCP_DECODE_PHASE_RESERVED:
            if (!pScalabilityState->bScalableDecodeMode)
            {
                bInValidPhase = true;
            }
            else if (pScalabilityState->ucScalablePipeNum < (HcpDecPhase - CODECHAL_HCP_DECODE_PHASE_BE0 + 1))
            {
                bInValidPhase = true;
            }
            break;
        default:
            bInValidPhase = true;
            break;
    }

    if (bInValidPhase)
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        CODECHAL_DECODE_ASSERTMESSAGE("invalid decode phase : %d !", HcpDecPhase);
    }

    return eStatus;
}

//!
//! \brief    Allocate fixed size resources for scalability decode
//! \details  Allocate fixed size resources for scalability decode
//! \param    [in]  pScalabilityState
//!                pointer to scalability decode state
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success, else fail reason
//!
MOS_STATUS CodecHalDecodeScalability_AllocateResources_FixedSizes(
    PCODECHAL_DECODE_SCALABILITY_STATE  pScalabilityState)
{
    PMOS_INTERFACE          pOsInterface;
    MOS_ALLOC_GFXRES_PARAMS AllocParamsForBufferLinear;
    MOS_LOCK_PARAMS         LockFlagsWriteOnly;
    uint8_t                *pData;
    MOS_STATUS              eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    CODECHAL_DECODE_CHK_NULL_RETURN(pScalabilityState);
    CODECHAL_DECODE_CHK_NULL_RETURN(pScalabilityState->pHwInterface);
    CODECHAL_DECODE_CHK_NULL_RETURN(pScalabilityState->pHwInterface->GetOsInterface());

    pOsInterface = pScalabilityState->pHwInterface->GetOsInterface();

    MOS_ZeroMemory(&LockFlagsWriteOnly, sizeof(MOS_LOCK_PARAMS));
    LockFlagsWriteOnly.WriteOnly = 1;

    // initiate allocation paramters
    MOS_ZeroMemory(&AllocParamsForBufferLinear, sizeof(MOS_ALLOC_GFXRES_PARAMS));
    AllocParamsForBufferLinear.Type = MOS_GFXRES_BUFFER;
    AllocParamsForBufferLinear.TileType = MOS_TILE_LINEAR;
    AllocParamsForBufferLinear.Format = Format_Buffer;

    if (pScalabilityState->Standard == CODECHAL_HEVC)//Confirmed by HW that VP9 does not need this buffer
    {
        //for Scalability --- Slice State Stream Out Buffer
        AllocParamsForBufferLinear.dwBytes = CODECHAL_HEVC_MAX_NUM_SLICES_LVL_6 * pScalabilityState->sliceStateCLs * CODECHAL_CACHELINE_SIZE;
        AllocParamsForBufferLinear.pBufName = "SliceStateStreamOut";

        eStatus = (MOS_STATUS)pOsInterface->pfnAllocateResource(
            pOsInterface,
            &AllocParamsForBufferLinear,
            &pScalabilityState->resSliceStateStreamOutBuffer);

        if (eStatus != MOS_STATUS_SUCCESS)
        {
            CODECHAL_DECODE_ASSERTMESSAGE("Failed to allocate SliceState StreamOut Buffer.");
            return eStatus;
        }
    }

    //Semaphore memory for BEs to start at the same time
    AllocParamsForBufferLinear.dwBytes = sizeof(uint32_t);
    AllocParamsForBufferLinear.pBufName = "BESemaphoreMemory";

    eStatus = (MOS_STATUS)pOsInterface->pfnAllocateResource(
        pOsInterface,
        &AllocParamsForBufferLinear,
        &pScalabilityState->resSemaMemBEs);

    if (eStatus != MOS_STATUS_SUCCESS)
    {
        CODECHAL_DECODE_ASSERTMESSAGE("Failed to allocate BE Semaphore memory.");
        return eStatus;
    }

    pData = (uint8_t*)pOsInterface->pfnLockResource(
        pOsInterface,
        &pScalabilityState->resSemaMemBEs,
        &LockFlagsWriteOnly);

    CODECHAL_DECODE_CHK_NULL_RETURN(pData);

    MOS_ZeroMemory(pData, sizeof(uint32_t));

    CODECHAL_DECODE_CHK_STATUS_RETURN(pOsInterface->pfnUnlockResource(
        pOsInterface,
        &pScalabilityState->resSemaMemBEs));

    AllocParamsForBufferLinear.dwBytes = sizeof(uint32_t);
    AllocParamsForBufferLinear.pBufName = "DelayMinusMemory";

    eStatus = (MOS_STATUS)pOsInterface->pfnAllocateResource(
        pOsInterface,
        &AllocParamsForBufferLinear,
        &pScalabilityState->resDelayMinus);

    if (eStatus != MOS_STATUS_SUCCESS)
    {
        CODECHAL_DECODE_ASSERTMESSAGE("Failed to allocate delay minus memory.");
        return eStatus;
    }

    pData = (uint8_t*)pOsInterface->pfnLockResource(
        pOsInterface,
        &pScalabilityState->resDelayMinus,
        &LockFlagsWriteOnly);

    CODECHAL_DECODE_CHK_NULL_RETURN(pData);

    MOS_ZeroMemory(pData, sizeof(uint32_t));

    CODECHAL_DECODE_CHK_STATUS_RETURN(pOsInterface->pfnUnlockResource(
        pOsInterface,
        &pScalabilityState->resDelayMinus));

    if (pScalabilityState->pHwInterface->GetMfxInterface()->GetNumVdbox() > 2)
    {
        if (pScalabilityState->bFESeparateSubmission)
        {
            //SW Semaphore sync object for FE/BE synchronization
            CODECHAL_DECODE_CHK_STATUS_RETURN(pOsInterface->pfnCreateSyncResource(pOsInterface, &pScalabilityState->resFeBeSyncObject));
        }
        else if (pOsInterface->bUseHwSemaForResSyncInVE)
        {
            //Semaphore memory for FE /BE synchronization
            AllocParamsForBufferLinear.dwBytes = sizeof(uint32_t);
            AllocParamsForBufferLinear.pBufName = "FEBESemaphMemory";

            eStatus = (MOS_STATUS)pOsInterface->pfnAllocateResource(
                pOsInterface,
                &AllocParamsForBufferLinear,
                &pScalabilityState->resSemaMemFEBE);

            if (eStatus != MOS_STATUS_SUCCESS)
            {
                CODECHAL_DECODE_ASSERTMESSAGE("Failed to allocate FEBE Semaph memory.");
                return eStatus;
            }

            pData = (uint8_t*)pOsInterface->pfnLockResource(
                pOsInterface,
                &pScalabilityState->resSemaMemFEBE,
                &LockFlagsWriteOnly);

            CODECHAL_DECODE_CHK_NULL_RETURN(pData);

            MOS_ZeroMemory(pData, sizeof(uint32_t));

            CODECHAL_DECODE_CHK_STATUS_RETURN(pOsInterface->pfnUnlockResource(
                pOsInterface,
                &pScalabilityState->resSemaMemFEBE));
        }
    }

    AllocParamsForBufferLinear.dwBytes = sizeof(CODECHAL_DECODE_SCALABILITY_FE_STATUS);
    AllocParamsForBufferLinear.pBufName = "FEStatusBuffer";
    eStatus = (MOS_STATUS)pOsInterface->pfnAllocateResource(
        pOsInterface,
        &AllocParamsForBufferLinear,
        &pScalabilityState->resFEStatusBuffer);

    if (eStatus != MOS_STATUS_SUCCESS)
    {
        CODECHAL_DECODE_ASSERTMESSAGE("Failed to allocate FE status Buffer.");
        return eStatus;
    }

    //Semaphore memory for frame decode completion synchronization
    AllocParamsForBufferLinear.dwBytes = sizeof(uint32_t);
    AllocParamsForBufferLinear.pBufName = "CompletionSemaphMemory";

    eStatus = (MOS_STATUS)pOsInterface->pfnAllocateResource(
        pOsInterface,
        &AllocParamsForBufferLinear,
        &pScalabilityState->resSemaMemCompletion);

    if (eStatus != MOS_STATUS_SUCCESS)
    {
        CODECHAL_DECODE_ASSERTMESSAGE("Failed to allocate Completion Semaph memory.");
        return eStatus;
    }

    pData = (uint8_t*)pOsInterface->pfnLockResource(
        pOsInterface,
        &pScalabilityState->resSemaMemCompletion,
        &LockFlagsWriteOnly);

    CODECHAL_DECODE_CHK_NULL_RETURN(pData);

    MOS_ZeroMemory(pData, sizeof(uint32_t));

    CODECHAL_DECODE_CHK_STATUS_RETURN(pOsInterface->pfnUnlockResource(
        pOsInterface,
        &pScalabilityState->resSemaMemCompletion));

    return eStatus;
}

//!
//! \brief    Get secondary cmd buffer
//! \param    [in]  pScalabilityState
//!                Scalability decode state
//! \param    [in] pSdryCmdBuf
//!                secondary cmd buffer address
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success, else fail reason
//!
static MOS_STATUS CodecHalDecodeScalability_GetVESecondaryCmdBuffer(
    PCODECHAL_DECODE_SCALABILITY_STATE  pScalabilityState,
    PMOS_COMMAND_BUFFER                 pSdryCmdBuf)
{
    PMOS_INTERFACE                  pOsInterface;
    uint32_t                        HcpDecPhase;
    uint32_t                        dwBufIdxPlus1 = 0;
    MOS_STATUS                      eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    CODECHAL_DECODE_CHK_NULL_RETURN(pScalabilityState);
    CODECHAL_DECODE_CHK_NULL_RETURN(pSdryCmdBuf);
    CODECHAL_DECODE_CHK_NULL_RETURN(pScalabilityState->pHwInterface);
    CODECHAL_DECODE_CHK_NULL_RETURN(pScalabilityState->pHwInterface->GetOsInterface());

    pOsInterface        = pScalabilityState->pHwInterface->GetOsInterface();
    HcpDecPhase         = pScalabilityState->HcpDecPhase;

    //calculate bufidx for getting secondary cmd buffer.
    CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHalDecodeScalability_CalculateScdryCmdBufIndex(pScalabilityState, &dwBufIdxPlus1));
    //Check if valid decode phase
    CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHalDecodeScalability_CheckDecPhaseValidity(pScalabilityState, HcpDecPhase));

    //Get batch buffer according to current decode phase
    switch (HcpDecPhase)
    {
        case CodechalDecode::CodechalHcpDecodePhaseLegacyS2L:
            //Note: no break here, S2L and FE commands put in one secondary command buffer.
        case CODECHAL_HCP_DECODE_PHASE_FE:
            if(!pScalabilityState->bFESeparateSubmission)
            {
                CODECHAL_DECODE_CHK_STATUS_RETURN(pOsInterface->pfnGetCommandBuffer(pOsInterface, pSdryCmdBuf, dwBufIdxPlus1));
                CODECHAL_DECODE_CHK_NULL_RETURN(pSdryCmdBuf);
            }
            else
            {
                //if FE separate submission, S2L and FE cmd buffer are in primary cmd buffer, shall not call this function to get secondary cmd buffer
                eStatus = MOS_STATUS_INVALID_PARAMETER;
                CODECHAL_DECODE_ASSERTMESSAGE("S2L or FE does not need secondary cmd buffer in FE separate submission!");
            }
            break;
        case CODECHAL_HCP_DECODE_PHASE_BE0:
        case CODECHAL_HCP_DECODE_PHASE_BE1:
        case CODECHAL_HCP_DECODE_PHASE_RESERVED:
            CODECHAL_DECODE_CHK_STATUS_RETURN(pOsInterface->pfnGetCommandBuffer(pOsInterface, pSdryCmdBuf, dwBufIdxPlus1));
            CODECHAL_DECODE_CHK_NULL_RETURN(pSdryCmdBuf);
            break;
        default:
            //never comes here because other decode phase already checked invalid in function CodecHalDecodeScalability_CheckDecPhaseValidity,
            eStatus = MOS_STATUS_INVALID_PARAMETER;
            break;
    }

    return eStatus;
}

MOS_STATUS CodecHalDecodeScalability_AllocateResources_VariableSizes(
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

    pOsInterface = pScalabilityState->pHwInterface->GetOsInterface();

    // initiate allocation paramters
    MOS_ZeroMemory(&AllocParamsForBufferLinear, sizeof(MOS_ALLOC_GFXRES_PARAMS));
    AllocParamsForBufferLinear.Type     = MOS_GFXRES_BUFFER;
    AllocParamsForBufferLinear.TileType = MOS_TILE_LINEAR;
    AllocParamsForBufferLinear.Format   = Format_Buffer;

    if (pScalabilityState->Standard == CODECHAL_HEVC)//Confirmed by HW that this buffer is not used in VP9 decoding
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(pScalabilityState->pfnIsHcpBufferReallocNeeded(pScalabilityState->pHwInterface,
                                                                                 MHW_VDBOX_HCP_INTERNAL_BUFFER_MV_UP_RT_COL,
                                                                                 pAllocParam));
        if (pAllocParam->bNeedBiggerSize || Mos_ResourceIsNull(&pScalabilityState->resMvUpRightColStoreBuffer))
        {
            if (!Mos_ResourceIsNull(&pScalabilityState->resMvUpRightColStoreBuffer))
            {
                pOsInterface->pfnFreeResource(
                    pOsInterface,
                    &pScalabilityState->resMvUpRightColStoreBuffer);
            }
            // MV UpperRight Column Store Buffer
            CODECHAL_DECODE_CHK_STATUS_RETURN(pScalabilityState->pfnGetHcpBufferSize(pScalabilityState->pHwInterface,
                                                                             MHW_VDBOX_HCP_INTERNAL_BUFFER_MV_UP_RT_COL,
                                                                             pHcpBufSizeParam));
            AllocParamsForBufferLinear.dwBytes  = pHcpBufSizeParam->dwBufferSize;
            AllocParamsForBufferLinear.pBufName = "MVUpperRightColumnStore";

            eStatus = (MOS_STATUS)pOsInterface->pfnAllocateResource(
                pOsInterface,
                &AllocParamsForBufferLinear,
                &pScalabilityState->resMvUpRightColStoreBuffer);

            if (eStatus != MOS_STATUS_SUCCESS)
            {
                CODECHAL_DECODE_ASSERTMESSAGE("Failed to allocate MV UpperRight Column Store Buffer.");
                return eStatus;
            }
        }
    }

    CODECHAL_DECODE_CHK_STATUS_RETURN(pScalabilityState->pfnIsHcpBufferReallocNeeded(pScalabilityState->pHwInterface,
                                                                             MHW_VDBOX_HCP_INTERNAL_BUFFER_INTRA_PRED_UP_RIGHT_COL,
                                                                             pAllocParam));
    if (pAllocParam->bNeedBiggerSize || Mos_ResourceIsNull(&pScalabilityState->resIntraPredUpRightColStoreBuffer))
    {
        if (!Mos_ResourceIsNull(&pScalabilityState->resIntraPredUpRightColStoreBuffer))
        {
            pOsInterface->pfnFreeResource(
                pOsInterface,
                &pScalabilityState->resIntraPredUpRightColStoreBuffer);
        }
        // IntraPred UpperRight Column Store Buffer
        CODECHAL_DECODE_CHK_STATUS_RETURN(pScalabilityState->pfnGetHcpBufferSize(pScalabilityState->pHwInterface,
                                                                         MHW_VDBOX_HCP_INTERNAL_BUFFER_INTRA_PRED_UP_RIGHT_COL,
                                                                         pHcpBufSizeParam));
        AllocParamsForBufferLinear.dwBytes  = pHcpBufSizeParam->dwBufferSize;
         AllocParamsForBufferLinear.pBufName = "IntraPredUpperRightColumnStore";

        eStatus = (MOS_STATUS)pOsInterface->pfnAllocateResource(
            pOsInterface,
            &AllocParamsForBufferLinear,
            &pScalabilityState->resIntraPredUpRightColStoreBuffer);

        if (eStatus != MOS_STATUS_SUCCESS)
        {
            CODECHAL_DECODE_ASSERTMESSAGE("Failed to allocate IntraPred UpperRight Column Store Buffer.");
            return eStatus;
        }
    }

    CODECHAL_DECODE_CHK_STATUS_RETURN(pScalabilityState->pfnIsHcpBufferReallocNeeded(pScalabilityState->pHwInterface,
                                                                             MHW_VDBOX_HCP_INTERNAL_BUFFER_INTRA_PRED_LFT_RECON_COL,
                                                                             pAllocParam));
    if (pAllocParam->bNeedBiggerSize || Mos_ResourceIsNull(&pScalabilityState->resIntraPredLeftReconColStoreBuffer))
    {
        if (!Mos_ResourceIsNull(&pScalabilityState->resIntraPredLeftReconColStoreBuffer))
        {
            pOsInterface->pfnFreeResource(
                pOsInterface,
                &pScalabilityState->resIntraPredLeftReconColStoreBuffer);
        }
        // IntraPred Left Recon Column Store Buffer
        CODECHAL_DECODE_CHK_STATUS_RETURN(pScalabilityState->pfnGetHcpBufferSize(pScalabilityState->pHwInterface,
                                                                         MHW_VDBOX_HCP_INTERNAL_BUFFER_INTRA_PRED_LFT_RECON_COL,
                                                                         pHcpBufSizeParam));
        AllocParamsForBufferLinear.dwBytes  = pHcpBufSizeParam->dwBufferSize;
        AllocParamsForBufferLinear.pBufName = "IntraPredLeftReconColumnStore";

        eStatus = (MOS_STATUS)pOsInterface->pfnAllocateResource(
            pOsInterface,
            &AllocParamsForBufferLinear,
            &pScalabilityState->resIntraPredLeftReconColStoreBuffer);

        if (eStatus != MOS_STATUS_SUCCESS)
        {
            CODECHAL_DECODE_ASSERTMESSAGE("Failed to allocate IntraPred Left Recon Column Store Buffer.");
            return eStatus;
        }
    }

    CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHalDecodeScalability_AllocateCABACStreamOutBuffer(pScalabilityState,
                                                                            pHcpBufSizeParam,
                                                                            pAllocParam,
                                                                            &pScalabilityState->resCABACSyntaxStreamOutBuffer[0]));

    pScalabilityState->presCABACStreamOutBuffer = &pScalabilityState->resCABACSyntaxStreamOutBuffer[0];

    return eStatus;
}

MOS_STATUS CodecHalDecodeScalability_AllocateCABACStreamOutBuffer(
    PCODECHAL_DECODE_SCALABILITY_STATE   pScalabilityState,
    PMHW_VDBOX_HCP_BUFFER_SIZE_PARAMS    pHcpBufSizeParam,
    PMHW_VDBOX_HCP_BUFFER_REALLOC_PARAMS pAllocParam,
    PMOS_RESOURCE                        presCABACStreamOutBuffer)
{
    PMOS_INTERFACE          pOsInterface;
    MOS_ALLOC_GFXRES_PARAMS AllocParamsForBufferLinear;
    MOS_STATUS              eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    CODECHAL_DECODE_CHK_NULL_RETURN(pScalabilityState);
    CODECHAL_DECODE_CHK_NULL_RETURN(pScalabilityState->pHwInterface);
    CODECHAL_DECODE_CHK_NULL_RETURN(pScalabilityState->pHwInterface->GetOsInterface());
    CODECHAL_DECODE_CHK_NULL_RETURN(pHcpBufSizeParam);
    CODECHAL_DECODE_CHK_NULL_RETURN(pAllocParam);
    CODECHAL_DECODE_CHK_NULL_RETURN(presCABACStreamOutBuffer);

    pOsInterface = pScalabilityState->pHwInterface->GetOsInterface();

    // initiate allocation paramters
    MOS_ZeroMemory(&AllocParamsForBufferLinear, sizeof(MOS_ALLOC_GFXRES_PARAMS));
    AllocParamsForBufferLinear.Type     = MOS_GFXRES_BUFFER;
    AllocParamsForBufferLinear.TileType = MOS_TILE_LINEAR;
    AllocParamsForBufferLinear.Format   = Format_Buffer;

    CODECHAL_DECODE_CHK_STATUS_RETURN(pScalabilityState->pfnIsHcpBufferReallocNeeded(pScalabilityState->pHwInterface,
                                                                             MHW_VDBOX_HCP_INTERNAL_BUFFER_CABAC_STREAMOUT,
                                                                             pAllocParam));
    if (pAllocParam->bNeedBiggerSize || Mos_ResourceIsNull(presCABACStreamOutBuffer))
    {
        if (!Mos_ResourceIsNull(presCABACStreamOutBuffer))
        {
            pOsInterface->pfnFreeResource(
                pOsInterface,
                presCABACStreamOutBuffer);
        }

        CODECHAL_DECODE_CHK_STATUS_RETURN(pScalabilityState->pfnGetHcpBufferSize(pScalabilityState->pHwInterface,
                                                                         MHW_VDBOX_HCP_INTERNAL_BUFFER_CABAC_STREAMOUT,
                                                                         pHcpBufSizeParam));

        pScalabilityState->dwCABACSyntaxStreamOutBufferSize = pHcpBufSizeParam->dwBufferSize;

        AllocParamsForBufferLinear.dwBytes = pHcpBufSizeParam->dwBufferSize;
        AllocParamsForBufferLinear.pBufName = "CABACStreamOutBuffer";

        eStatus = (MOS_STATUS)pOsInterface->pfnAllocateResource(
            pOsInterface,
            &AllocParamsForBufferLinear,
            presCABACStreamOutBuffer);

        if (eStatus != MOS_STATUS_SUCCESS)
        {
            CODECHAL_DECODE_ASSERTMESSAGE("Failed to allocate CABAC StreamOut Buffer.");
            return eStatus;
        }
    }

    return eStatus;
}

void CodecHalDecodeScalability_Destroy (
    PCODECHAL_DECODE_SCALABILITY_STATE  pScalabilityState)
{
    PMOS_INTERFACE      pOsInterface;

    CODECHAL_DECODE_FUNCTION_ENTER;

    CODECHAL_DECODE_CHK_NULL_NO_STATUS_RETURN(pScalabilityState);
    CODECHAL_DECODE_CHK_NULL_NO_STATUS_RETURN(pScalabilityState->pHwInterface);
    CODECHAL_DECODE_CHK_NULL_NO_STATUS_RETURN(pScalabilityState->pHwInterface->GetOsInterface());
    pOsInterface = pScalabilityState->pHwInterface->GetOsInterface();

    pOsInterface->pfnFreeResource(
        pOsInterface,
        &pScalabilityState->resSliceStateStreamOutBuffer);
    pOsInterface->pfnFreeResource(
        pOsInterface,
        &pScalabilityState->resMvUpRightColStoreBuffer);
    pOsInterface->pfnFreeResource(
        pOsInterface,
        &pScalabilityState->resIntraPredLeftReconColStoreBuffer);
    pOsInterface->pfnFreeResource(
        pOsInterface,
        &pScalabilityState->resIntraPredUpRightColStoreBuffer);
    for (int i = 0; i < CODECHAL_HCP_STREAMOUT_BUFFER_MAX_NUM; i++)
    {
        pOsInterface->pfnFreeResource(
            pOsInterface,
            &pScalabilityState->resCABACSyntaxStreamOutBuffer[i]);
    }
    pOsInterface->pfnFreeResource(
        pOsInterface,
        &pScalabilityState->resSemaMemBEs);
    pOsInterface->pfnFreeResource(
        pOsInterface,
        &pScalabilityState->resDelayMinus);
    if (pOsInterface->bUseHwSemaForResSyncInVE)
    {
        pOsInterface->pfnFreeResource(
            pOsInterface,
            &pScalabilityState->resSemaMemFEBE);
    }
    pOsInterface->pfnFreeResource(
        pOsInterface,
        &pScalabilityState->resFEStatusBuffer);
    pOsInterface->pfnDestroySyncResource(pOsInterface, &pScalabilityState->resFeBeSyncObject);

    pOsInterface->pfnFreeResource(
        pOsInterface,
        &pScalabilityState->resSemaMemCompletion);

    return;
}

MOS_STATUS CodecHalDecodeScalability_GetCmdBufferToUse(
    PCODECHAL_DECODE_SCALABILITY_STATE  pScalabilityState,
    PMOS_COMMAND_BUFFER                 pScdryCmdBuf,
    PMOS_COMMAND_BUFFER                 *ppCmdBufToUse)
{
    MOS_STATUS                      eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    CODECHAL_DECODE_CHK_NULL_RETURN(pScalabilityState);
    CODECHAL_DECODE_CHK_NULL_RETURN(ppCmdBufToUse);

    if (!CodecHalDecodeScalabilityIsFESeparateSubmission(pScalabilityState) ||
        CodecHalDecodeScalabilityIsBEPhase(pScalabilityState))
    {
        pScalabilityState->bUseSecdryCmdBuffer = true;
        CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHalDecodeScalability_GetVESecondaryCmdBuffer(pScalabilityState, pScdryCmdBuf));
        *ppCmdBufToUse = pScdryCmdBuf;
    }
    else
    {
        pScalabilityState->bUseSecdryCmdBuffer = false;
    }

    return eStatus;
}

MOS_STATUS CodecHalDecodeScalability_ReturnSdryCmdBuffer(
    PCODECHAL_DECODE_SCALABILITY_STATE  pScalabilityState,
    PMOS_COMMAND_BUFFER                 pSdryCmdBuf)
{
    PMOS_INTERFACE                  pOsInterface;
    uint32_t                        HcpDecPhase;
    uint32_t                        dwBufIdxPlus1;
    MOS_STATUS                      eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    CODECHAL_DECODE_CHK_NULL_RETURN(pScalabilityState);
    CODECHAL_DECODE_CHK_NULL_RETURN(pSdryCmdBuf);
    CODECHAL_DECODE_CHK_NULL_RETURN(pScalabilityState->pHwInterface);
    CODECHAL_DECODE_CHK_NULL_RETURN(pScalabilityState->pHwInterface->GetOsInterface());

    pOsInterface = pScalabilityState->pHwInterface->GetOsInterface();

    if (!pScalabilityState->bUseSecdryCmdBuffer)
    {
        return eStatus;
    }

    HcpDecPhase         = pScalabilityState->HcpDecPhase;

    //calculate bufidx for getting secondary cmd buffer.
    CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHalDecodeScalability_CalculateScdryCmdBufIndex(pScalabilityState, &dwBufIdxPlus1));
    //Check if valid decode phase
    CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHalDecodeScalability_CheckDecPhaseValidity(pScalabilityState, HcpDecPhase));

    //Get batch buffer according to current decode phase
    switch (HcpDecPhase)
    {
        case CodechalDecode::CodechalHcpDecodePhaseLegacyS2L:
            //Note: no break here, S2L and FE commands put in one secondary command buffer.
        case CODECHAL_HCP_DECODE_PHASE_FE:
            if(!pScalabilityState->bFESeparateSubmission)
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
        case CODECHAL_HCP_DECODE_PHASE_BE0:
        case CODECHAL_HCP_DECODE_PHASE_BE1:
        case CODECHAL_HCP_DECODE_PHASE_RESERVED:
            pOsInterface->pfnReturnCommandBuffer(pOsInterface, pSdryCmdBuf, dwBufIdxPlus1);
            break;
        default:
            //never comes here because other decode phase already checked invalid in function CodecHalDecodeScalability_CheckDecPhaseValidity,
            eStatus = MOS_STATUS_INVALID_PARAMETER;
            break;
    }

    return eStatus;
}

MOS_STATUS CodecHalDecodeScalablity_SetFECabacStreamoutOverflowStatus(
    PCODECHAL_DECODE_SCALABILITY_STATE  pScalabilityState,
    PMOS_COMMAND_BUFFER                 pCmdBufferInUse)
{
    MhwMiInterface                      *pMiInterface;
    CodechalHwInterface                 *pHwInterface;
    MmioRegistersMfx                    *pMmioRegistersMfx;
    MmioRegistersHcp                    *pMmioRegistersHcp;
    MHW_MI_STORE_REGISTER_MEM_PARAMS    StoreRegParams;
    MHW_MI_STORE_DATA_PARAMS            StoreDataParams;
    MHW_MI_LOAD_REGISTER_REG_PARAMS     LoadRegRegParams;
    MHW_MI_LOAD_REGISTER_IMM_PARAMS     LoadRegisterImmParams;
    MHW_MI_FLUSH_DW_PARAMS              FlushDwParams;
    MHW_MI_MATH_PARAMS                  MiMathParams;
    MHW_MI_ALU_PARAMS                   MiAluParams[4];
    MOS_STATUS                          eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    CODECHAL_DECODE_CHK_NULL_RETURN(pCmdBufferInUse);
    CODECHAL_DECODE_CHK_NULL_RETURN(pScalabilityState);

    CODECHAL_DECODE_CHK_NULL_RETURN(pScalabilityState->pHwInterface);
    CODECHAL_DECODE_CHK_NULL_RETURN(pScalabilityState->pHwInterface->GetMiInterface());
    pMiInterface  = pScalabilityState->pHwInterface->GetMiInterface();
    pHwInterface = pScalabilityState->pHwInterface;

    //relative MMIO addressing
    pMmioRegistersMfx   = pHwInterface->GetMfxInterface()->GetMmioRegisters(MHW_VDBOX_NODE_1);
    pMmioRegistersHcp   = pHwInterface->GetHcpInterface()->GetMmioRegisters(MHW_VDBOX_NODE_1);

    MOS_ZeroMemory(&FlushDwParams, sizeof(FlushDwParams));
    CODECHAL_DECODE_CHK_STATUS_RETURN(pMiInterface->AddMiFlushDwCmd(pCmdBufferInUse, &FlushDwParams));

    // store stream out size register to general purpose register0 - Hi=0, Lo=FE stream out size
    MOS_ZeroMemory(&LoadRegRegParams, sizeof(LoadRegRegParams));
    LoadRegRegParams.dwSrcRegister = pMmioRegistersHcp->hcpDebugFEStreamOutSizeRegOffset;
    LoadRegRegParams.dwDstRegister = pMmioRegistersMfx->generalPurposeRegister0LoOffset;
    CODECHAL_DECODE_CHK_STATUS_RETURN(pMiInterface->AddMiLoadRegisterRegCmd(
        pCmdBufferInUse,
        &LoadRegRegParams));
    MOS_ZeroMemory(&LoadRegisterImmParams, sizeof(LoadRegisterImmParams));
    LoadRegisterImmParams.dwData        = 0;
    LoadRegisterImmParams.dwRegister    = pMmioRegistersMfx->generalPurposeRegister0HiOffset;
    CODECHAL_DECODE_CHK_STATUS_RETURN(pMiInterface->AddMiLoadRegisterImmCmd(
        pCmdBufferInUse,
        &LoadRegisterImmParams));

    // load allocated size to general purpose register4 - Hi = 0
    MOS_ZeroMemory(&LoadRegisterImmParams, sizeof(LoadRegisterImmParams));
    LoadRegisterImmParams.dwData        = pScalabilityState->dwCABACSyntaxStreamOutBufferSize;
    LoadRegisterImmParams.dwRegister    = pMmioRegistersMfx->generalPurposeRegister4LoOffset;
    CODECHAL_DECODE_CHK_STATUS_RETURN(pMiInterface->AddMiLoadRegisterImmCmd(
        pCmdBufferInUse,
        &LoadRegisterImmParams));
    MOS_ZeroMemory(&LoadRegisterImmParams, sizeof(LoadRegisterImmParams));
    LoadRegisterImmParams.dwData        = 0;
    LoadRegisterImmParams.dwRegister    = pMmioRegistersMfx->generalPurposeRegister4HiOffset;
    CODECHAL_DECODE_CHK_STATUS_RETURN(pMiInterface->AddMiLoadRegisterImmCmd(
        pCmdBufferInUse,
        &LoadRegisterImmParams));

    //perform the sub operation(reported size - allocated size)
    MOS_ZeroMemory(&MiMathParams, sizeof(MiMathParams));
    MOS_ZeroMemory(&MiAluParams, sizeof(MiAluParams));
    // load     srcA, reg0
    MiAluParams[0].AluOpcode    = MHW_MI_ALU_LOAD;
    MiAluParams[0].Operand1     = MHW_MI_ALU_SRCA;
    MiAluParams[0].Operand2     = MHW_MI_ALU_GPREG0;
    // load     srcB, reg4
    MiAluParams[1].AluOpcode    = MHW_MI_ALU_LOAD;
    MiAluParams[1].Operand1     = MHW_MI_ALU_SRCB;
    MiAluParams[1].Operand2     = MHW_MI_ALU_GPREG4;
    // sub      srcA, srcB
    MiAluParams[2].AluOpcode    = MHW_MI_ALU_SUB;
    MiAluParams[2].Operand1     = MHW_MI_ALU_SRCB;
    MiAluParams[2].Operand2     = MHW_MI_ALU_GPREG4;
    // store      reg0, CF
    MiAluParams[3].AluOpcode    = MHW_MI_ALU_STORE;
    MiAluParams[3].Operand1     = MHW_MI_ALU_GPREG0;
    MiAluParams[3].Operand2     = MHW_MI_ALU_CF;
    MiMathParams.pAluPayload    = MiAluParams;
    MiMathParams.dwNumAluParams = 4; // four ALU commands needed for this substract opertaion. see following ALU commands.
    CODECHAL_DECODE_CHK_STATUS_RETURN(pMiInterface->AddMiMathCmd(
        pCmdBufferInUse,
        &MiMathParams));
    // store the carry flag of (reported size - allocated size),
    // if reported size < allocated size,  the carry flag will be 0xFFFFFFFF, else carry flag will be 0x0.
    MOS_ZeroMemory(&StoreRegParams, sizeof(StoreRegParams));
    StoreRegParams.presStoreBuffer      = &pScalabilityState->resFEStatusBuffer;
    StoreRegParams.dwOffset             = CODECHAL_OFFSETOF(CODECHAL_DECODE_SCALABILITY_FE_STATUS, dwCarryFlagOfReportedSizeMinusAllocSize);
    StoreRegParams.dwRegister           = pMmioRegistersMfx->generalPurposeRegister0LoOffset;
    CODECHAL_DECODE_CHK_STATUS_RETURN(pMiInterface->AddMiStoreRegisterMemCmd(pCmdBufferInUse, &StoreRegParams));

    return eStatus;
}

MOS_STATUS CodecHalDecodeScalablity_GetFEReportedCabacStreamoutBufferSize(
    PCODECHAL_DECODE_SCALABILITY_STATE  pScalabilityState,
    PMOS_COMMAND_BUFFER                 pCmdBufferInUse)
{
    MhwMiInterface                      *pMiInterface;
    CodechalHwInterface                 *pHwInterface;
    MmioRegistersHcp                    *pMmioRegistersHcp;
    MHW_MI_STORE_REGISTER_MEM_PARAMS    StoreRegParams;
    MHW_MI_FLUSH_DW_PARAMS              FlushDwParams;
    MOS_STATUS                          eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    CODECHAL_DECODE_CHK_NULL_RETURN(pCmdBufferInUse);
    CODECHAL_DECODE_CHK_NULL_RETURN(pScalabilityState);

    CODECHAL_DECODE_CHK_NULL_RETURN(pScalabilityState->pHwInterface);
    pMiInterface = pScalabilityState->pHwInterface->GetMiInterface();
    pHwInterface = pScalabilityState->pHwInterface;

    //relative MMIO addressing
    pMmioRegistersHcp = pHwInterface->GetHcpInterface()->GetMmioRegisters(MHW_VDBOX_NODE_1);

    MOS_ZeroMemory(&FlushDwParams, sizeof(FlushDwParams));
    CODECHAL_DECODE_CHK_STATUS_RETURN(pMiInterface->AddMiFlushDwCmd(pCmdBufferInUse, &FlushDwParams));

    //store the cabac streamout buff size in register into mem
    MOS_ZeroMemory(&StoreRegParams, sizeof(StoreRegParams));
    StoreRegParams.presStoreBuffer = &pScalabilityState->resFEStatusBuffer;
    StoreRegParams.dwOffset = CODECHAL_OFFSETOF(CODECHAL_DECODE_SCALABILITY_FE_CABAC_STREAMOUT_BUFF_SIZE, dwCabacStreamoutBuffSize);
    StoreRegParams.dwRegister = pMmioRegistersHcp->hcpDebugFEStreamOutSizeRegOffset;
    CODECHAL_DECODE_CHK_STATUS_RETURN(pMiInterface->AddMiStoreRegisterMemCmd(pCmdBufferInUse, &StoreRegParams));

    return eStatus;
}

MOS_STATUS CodecHalDecodeScalability_DetermineDecodePhase(
    PCODECHAL_DECODE_SCALABILITY_STATE  pScalabilityState,
    uint32_t                            *pHcpDecPhase)
{
    uint32_t                        CurPhase;
    MOS_STATUS                      eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    CODECHAL_DECODE_CHK_NULL_RETURN(pScalabilityState);
    CODECHAL_DECODE_CHK_NULL_RETURN(pHcpDecPhase);

    CurPhase = *pHcpDecPhase;
    //Check if valid decode phase
    CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHalDecodeScalability_CheckDecPhaseValidity(pScalabilityState, CurPhase));

    if (CodecHalDecodeScalabilityIsScalableMode(pScalabilityState))
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
            case CODECHAL_HCP_DECODE_PHASE_BE0:
                *pHcpDecPhase = CODECHAL_HCP_DECODE_PHASE_BE1;
                break;
            case CODECHAL_HCP_DECODE_PHASE_BE1:
                *pHcpDecPhase = CODECHAL_HCP_DECODE_PHASE_RESERVED;
                break;
            default:
                //never comes here because other decode phase already checked invalid in function CodecHalDecodeScalability_CheckDecPhaseValidity,
                eStatus = MOS_STATUS_INVALID_PARAMETER;
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
                *pHcpDecPhase = CodechalDecode::CodechalHcpDecodePhaseLegacyLong;
                break;
            default:
                //never comes here because other decode phase already checked invalid in function CodecHalDecodeScalability_CheckDecPhaseValidity,
                eStatus = MOS_STATUS_INVALID_PARAMETER;
                break;
        }
    }

    pScalabilityState->HcpDecPhase = *pHcpDecPhase;

    return eStatus;
}

MOS_STATUS CodecHalDecodeScalability_DetermineSendWatchdogTimerStart(
    PCODECHAL_DECODE_SCALABILITY_STATE  pScalabilityState,
    bool                                *pbSend)
{
    uint32_t                        HcpDecPhase;
    MOS_STATUS                      eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    CODECHAL_DECODE_CHK_NULL_RETURN(pScalabilityState);
    HcpDecPhase  = pScalabilityState->HcpDecPhase;

    //Check if valid decode phase
    CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHalDecodeScalability_CheckDecPhaseValidity(pScalabilityState, HcpDecPhase));

    switch (HcpDecPhase)
    {
        case CodechalDecode::CodechalHcpDecodePhaseLegacyS2L:
            *pbSend = true;
            break;
        case CODECHAL_HCP_DECODE_PHASE_FE:
            *pbSend = !pScalabilityState->bShortFormatInUse;
            break;
        case CODECHAL_HCP_DECODE_PHASE_BE0:
        case CODECHAL_HCP_DECODE_PHASE_BE1:
        case CODECHAL_HCP_DECODE_PHASE_RESERVED:
            *pbSend = true;
            break;
        default:
            //never comes here because other decode phase already checked invalid in function CodecHalDecodeScalability_CheckDecPhaseValidity,
            eStatus = MOS_STATUS_INVALID_PARAMETER;
            break;
    }

    return eStatus;
}

MOS_STATUS CodecHalDecodeScalability_SwitchGpuContext(
    PCODECHAL_DECODE_SCALABILITY_STATE  pScalabilityState)
{
    PMOS_INTERFACE                  pOsInterface;
    MOS_GPU_CONTEXT                 GpuContext;
    uint32_t                        HcpDecPhase;
    bool                            bFESepSwitchContextFlag = false;
    MOS_STATUS                      eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    CODECHAL_DECODE_CHK_NULL_RETURN(pScalabilityState);
    CODECHAL_DECODE_CHK_NULL_RETURN(pScalabilityState->pHwInterface);
    CODECHAL_DECODE_CHK_NULL_RETURN(pScalabilityState->pHwInterface->GetOsInterface());

    pOsInterface = pScalabilityState->pHwInterface->GetOsInterface();
    HcpDecPhase  = pScalabilityState->HcpDecPhase;

    if (pScalabilityState->bFESeparateSubmission)
    {
        if (CodecHalDecodeScalability1stPhaseofSubmission(pScalabilityState))
        {
            bFESepSwitchContextFlag = true;
            if (pScalabilityState->bFESeparateSubmission && HcpDecPhase == CODECHAL_HCP_DECODE_PHASE_BE0)
            {
                GpuContext = pScalabilityState->VideoContext;
            }
            else
            {
                GpuContext = pScalabilityState->VideoContextForFE;
            }
        }
    }

#if (_DEBUG || _RELEASE_INTERNAL)
    if (CodecHalDecodeScalability1stDecPhase(pScalabilityState))
    {
        // report in-use
        MOS_USER_FEATURE_VALUE_WRITE_DATA UserFeatureWriteData;
        MOS_ZeroMemory(&UserFeatureWriteData, sizeof(UserFeatureWriteData));
        UserFeatureWriteData.Value.i32Data = pScalabilityState->bFESeparateSubmission;
        UserFeatureWriteData.ValueID = __MEDIA_USER_FEATURE_VALUE_SCALABILITY_FE_SEPARATE_SUBMISSION_IN_USE_ID;
        MOS_UserFeature_WriteValues_ID(nullptr, &UserFeatureWriteData, 1);
    }
#endif

    if (bFESepSwitchContextFlag)
    {
        CODECHAL_DECODE_VERBOSEMESSAGE("Change Decode GPU Ctxt to %d.", GpuContext);

        // Switch GPU context
        CODECHAL_DECODE_CHK_STATUS_RETURN(pOsInterface->pfnSetGpuContext(pOsInterface, GpuContext));
        // Reset allocation list and house keeping
        pOsInterface->pfnResetOsStates(pOsInterface);
    }

    return eStatus;
}

MOS_STATUS CodecHalDecodeScalability_InitSemaMemResources(
    PCODECHAL_DECODE_SCALABILITY_STATE  pScalabilityState,
    PMOS_COMMAND_BUFFER                 pCmdBuffer)
{
    CodechalHwInterface      *pHwInterface;
    MhwMiInterface           *pMiInterface;
    MOS_STATUS               eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    CODECHAL_DECODE_CHK_NULL_RETURN(pScalabilityState);
    CODECHAL_DECODE_CHK_NULL_RETURN(pScalabilityState->pHwInterface);
    CODECHAL_DECODE_CHK_NULL_RETURN(pScalabilityState->pHwInterface->GetMiInterface());
    pHwInterface       = pScalabilityState->pHwInterface;
    pMiInterface = pHwInterface->GetMiInterface();

    MHW_MI_STORE_DATA_PARAMS StoreDataParams;
    MOS_ZeroMemory(&StoreDataParams, sizeof(StoreDataParams));

    if (!Mos_ResourceIsNull(&pScalabilityState->resSemaMemCompletion))
    {
        StoreDataParams.pOsResource       = &pScalabilityState->resSemaMemCompletion;
        StoreDataParams.dwResourceOffset  = 0;
        StoreDataParams.dwValue           = 0;
        CODECHAL_DECODE_CHK_STATUS_RETURN(pMiInterface->AddMiStoreDataImmCmd(
            pCmdBuffer,
            &StoreDataParams));
    }

    return eStatus;

}

MOS_STATUS CodecHalDecodeScalability_DecidePipeNum(
    PCODECHAL_DECODE_SCALABILITY_STATE         pScalState,
    PCODECHAL_DECODE_SCALABILITY_INIT_PARAMS   pInitParams)
{
    PMOS_VIRTUALENGINE_INTERFACE pVEInterface;
    MOS_STATUS                   eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    CODECHAL_DECODE_CHK_NULL_RETURN(pScalState);
    CODECHAL_DECODE_CHK_NULL_RETURN(pScalState->pVEInterface);
    CODECHAL_DECODE_CHK_NULL_RETURN(pInitParams);

    pVEInterface = pScalState->pVEInterface;

    pScalState->ucScalablePipeNum = CODECHAL_DECODE_HCP_Legacy_PIPE_NUM_1;
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
                pScalState->ucScalablePipeNum = CODECHAL_DECODE_HCP_SCALABLE_PIPE_NUM_RESERVED;
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
                                && CodechalDecodeResolutionEqualLargerThan5k(pInitParams->u32PicWidthInPixel, pInitParams->u32PicHeightInPixel)))
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
                                && CodechalDecodeResolutionEqualLargerThan5k(pInitParams->u32PicWidthInPixel, pInitParams->u32PicHeightInPixel)))
                    {
                        pScalState->ucScalablePipeNum = CODECHAL_DECODE_HCP_SCALABLE_PIPE_NUM_2;
                    }
                }
            }
        }
    }

    return eStatus;
}

MOS_STATUS CodechalDecodeScalability_MapPipeNumToLRCACount(
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
    case CODECHAL_DECODE_HCP_SCALABLE_PIPE_NUM_RESERVED:
        *LRCACount = pScalState->bFESeparateSubmission ? 3 : 4;
        break;
    case CODECHAL_DECODE_HCP_SCALABLE_PIPE_NUM_2:
        // on GT2 or debug override enabled, FE separate submission = false, FE run on the same engine of BEs;
        // on GT3, FE separate submission = true, scalability submission includes only BEs.
        *LRCACount = 2;
        break;
    case CODECHAL_DECODE_HCP_Legacy_PIPE_NUM_1:
        *LRCACount = 1;
        break;
    default:
        CODECHAL_DECODE_ASSERTMESSAGE("invalid pipe number.")
            return MOS_STATUS_INVALID_PARAMETER;
    }

    if (*LRCACount > pScalState->ucNumVdbox)
    {
        CODECHAL_DECODE_ASSERTMESSAGE("LRCA count can not exceed vdbox number.");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    return eStatus;
}

MOS_STATUS CodechalDecodeScalability_ChkGpuCtxReCreation(
    PCODECHAL_DECODE_SCALABILITY_STATE         pScalabilityState,
    PMOS_GPUCTX_CREATOPTIONS_ENHANCED          CurgpuCtxCreatOpts)
{
    PMOS_INTERFACE      pOsInterface;
    bool                changed = false;
    uint32_t            NewLRCACount = 0, PreLRCACount = 0;
    MOS_STATUS          eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    CODECHAL_DECODE_CHK_NULL_RETURN(pScalabilityState);
    CODECHAL_DECODE_CHK_NULL_RETURN(CurgpuCtxCreatOpts);

    pOsInterface    = pScalabilityState->pHwInterface->GetOsInterface();

#if (_DEBUG || _RELEASE_INTERNAL)
    if (pOsInterface->bEnableDbgOvrdInVE)
    {
        changed = false;
    }
    else
#endif
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(pScalabilityState->pfnMapPipeNumToLRCACount(
            pScalabilityState,
            &NewLRCACount));

        if (CurgpuCtxCreatOpts->LRCACount != NewLRCACount)
        {
            changed = true;
            PreLRCACount = CurgpuCtxCreatOpts->LRCACount;
            CurgpuCtxCreatOpts->LRCACount = NewLRCACount;
        }
        else
        {
            changed = false;
        }
    }

    if (changed)
    {
         auto contextToCreate = MOS_GPU_CONTEXT_VIDEO;

         switch (NewLRCACount)
         {
         case 2:
             contextToCreate = pScalabilityState->VideoContextForMP;
             break;
         case 3:
             contextToCreate = pScalabilityState->VideoContextFor3P;
             break;
         default:
             contextToCreate = pScalabilityState->VideoContextForSP;
             break;
         }

         CODECHAL_DECODE_CHK_STATUS_RETURN(pOsInterface->pfnCreateGpuContext(
             pOsInterface,
             contextToCreate,
             MOS_GPU_NODE_VIDEO,
             CurgpuCtxCreatOpts));
         CODECHAL_DECODE_CHK_STATUS_RETURN(pOsInterface->pfnRegisterBBCompleteNotifyEvent(
             pOsInterface,
             contextToCreate));

        // Switch across single pipe/ scalable mode gpu contexts
        MOS_GPU_CONTEXT GpuContext = contextToCreate;
        CODECHAL_DECODE_VERBOSEMESSAGE("Change Decode GPU Ctxt to %d.", GpuContext);
        CODECHAL_DECODE_CHK_STATUS_RETURN(pOsInterface->pfnSetGpuContext(pOsInterface, GpuContext));
        // Reset allocation list and house keeping
        pOsInterface->pfnResetOsStates(pOsInterface);
        // Gpu context re-use
        pScalabilityState->VideoContext = GpuContext;
    }

    return eStatus;
}

#if (_DEBUG || _RELEASE_INTERNAL)
MOS_STATUS CodechalDecodeScalability_DebugOvrdDecidePipeNum(
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
        if (veInterface->GetEngineCount() == 1)
        {
            pScalState->ucScalablePipeNum = CODECHAL_DECODE_HCP_Legacy_PIPE_NUM_1;
        }
        else if (veInterface->GetEngineCount() == 2)
        {
            //engine count = 2, only support FE run on the same engine as one of BE for now.
            pScalState->ucScalablePipeNum = CODECHAL_DECODE_HCP_SCALABLE_PIPE_NUM_2;
        }
        else if (veInterface->GetEngineCount() == 4 &&
                 veInterface->GetEngineLogicId(3) != veInterface->GetEngineLogicId(0) &&
                 veInterface->GetEngineLogicId(3) != veInterface->GetEngineLogicId(1) &&
                 veInterface->GetEngineLogicId(3) != veInterface->GetEngineLogicId(2))
        {
            pScalState->ucScalablePipeNum = CODECHAL_DECODE_HCP_SCALABLE_PIPE_NUM_RESERVED;
        }
        else
        {
            CODECHAL_DECODE_ASSERTMESSAGE("invalid parameter settings in debug override.");
            return MOS_STATUS_INVALID_PARAMETER;
        }

        return eStatus;
    }

    // debug override for virtual tile
    if (pVEInterface->ucEngineCount == 1)
    {
        pScalState->ucScalablePipeNum = CODECHAL_DECODE_HCP_Legacy_PIPE_NUM_1;
    }
    else if (pVEInterface->ucEngineCount == 2)
    {
        //engine count = 2, only support FE run on the same engine as one of BE for now.
        pScalState->ucScalablePipeNum = CODECHAL_DECODE_HCP_SCALABLE_PIPE_NUM_2;
    }
    else if (pVEInterface->ucEngineCount == 4 &&
        pVEInterface->EngineLogicId[3] != pVEInterface->EngineLogicId[0] &&
        pVEInterface->EngineLogicId[3] != pVEInterface->EngineLogicId[1] &&
        pVEInterface->EngineLogicId[3] != pVEInterface->EngineLogicId[2])
    {
        pScalState->ucScalablePipeNum = CODECHAL_DECODE_HCP_SCALABLE_PIPE_NUM_RESERVED;
    }
    else
    {
        CODECHAL_DECODE_ASSERTMESSAGE("invalid parameter settings in debug override.");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    return eStatus;
}
#endif

MOS_STATUS CodechalDecodeScalability_ConstructParmsForGpuCtxCreation(
    PCODECHAL_DECODE_SCALABILITY_STATE         pScalState,
    PMOS_GPUCTX_CREATOPTIONS_ENHANCED          gpuCtxCreatOpts,
    CodechalSetting *                          codecHalSetting)
{
    PMOS_INTERFACE                           pOsInterface;
    CODECHAL_DECODE_SCALABILITY_INIT_PARAMS  initParams;
    MOS_STATUS                               eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    CODECHAL_DECODE_CHK_NULL_RETURN(pScalState);
    CODECHAL_DECODE_CHK_NULL_RETURN(pScalState->pHwInterface);
    CODECHAL_DECODE_CHK_NULL_RETURN(gpuCtxCreatOpts);
    CODECHAL_DECODE_CHK_NULL_RETURN(codecHalSetting);
    bool sfcInUse = codecHalSetting->sfcInUseHinted && codecHalSetting->downsamplingHinted
                       && (MEDIA_IS_SKU(pScalState->pHwInterface->GetSkuTable(), FtrSFCPipe)
                       && !MEDIA_IS_SKU(pScalState->pHwInterface->GetSkuTable(), FtrDisableVDBox2SFC));
    pOsInterface    = pScalState->pHwInterface->GetOsInterface();
    MEDIA_FEATURE_TABLE *m_skuTable = pOsInterface->pfnGetSkuTable(pOsInterface);
#if (_DEBUG || _RELEASE_INTERNAL)
    if (pOsInterface->bEnableDbgOvrdInVE)
    {
        PMOS_VIRTUALENGINE_INTERFACE pVEInterface = pScalState->pVEInterface;
        CODECHAL_DECODE_CHK_NULL_RETURN(pVEInterface);
        gpuCtxCreatOpts->DebugOverride      = true;
        if (MEDIA_IS_SKU(m_skuTable, FtrSfcScalability))
        {
            gpuCtxCreatOpts->UsingSFC = false;// this param ignored when dbgoverride enabled
        }
        else
        {
            gpuCtxCreatOpts->UsingSFC = sfcInUse;  // this param ignored when dbgoverride enabled
        }
        CODECHAL_DECODE_CHK_STATUS_RETURN(pScalState->pfnDebugOvrdDecidePipeNum(pScalState));

        if (g_apoMosEnabled)
        {
            CODECHAL_DECODE_CHK_NULL_RETURN(pVEInterface->veInterface);
            for (uint32_t i = 0; i < pVEInterface->veInterface->GetEngineCount(); i++)
            {
                gpuCtxCreatOpts->EngineInstance[i] = pVEInterface->veInterface->GetEngineLogicId(i);
            }
        }
        else
        {
            for (uint32_t i = 0; i < pVEInterface->ucEngineCount; i++)
            {
                gpuCtxCreatOpts->EngineInstance[i] = pVEInterface->EngineLogicId[i];
            }
        }
    }
    else
#endif
    {
        if (MEDIA_IS_SKU(m_skuTable, FtrSfcScalability))
        {
            gpuCtxCreatOpts->UsingSFC = false;
        }
        else
        {
            gpuCtxCreatOpts->UsingSFC = sfcInUse;
        }

        MOS_ZeroMemory(&initParams, sizeof(initParams));
        initParams.u32PicWidthInPixel   = MOS_ALIGN_CEIL(codecHalSetting->width, 8);
        initParams.u32PicHeightInPixel  = MOS_ALIGN_CEIL(codecHalSetting->height, 8);
        if (((codecHalSetting->standard == CODECHAL_VP9) || (codecHalSetting->standard == CODECHAL_HEVC))
                && (codecHalSetting->chromaFormat == HCP_CHROMA_FORMAT_YUV420))
        {
            initParams.format = Format_NV12;
            if (codecHalSetting->lumaChromaDepth == CODECHAL_LUMA_CHROMA_DEPTH_10_BITS)
            {
                initParams.format = Format_P010;
            }
        }
        initParams.usingSFC             = sfcInUse;
        initParams.usingSecureDecode    = codecHalSetting->DecodeEncType();
        CODECHAL_DECODE_CHK_STATUS_RETURN(pScalState->pfnDecidePipeNum(
            pScalState,
            &initParams));
    }

    CODECHAL_DECODE_CHK_STATUS_RETURN(pScalState->pfnMapPipeNumToLRCACount(
        pScalState,
        &gpuCtxCreatOpts->LRCACount));

    return eStatus;
}

MOS_STATUS CodecHalDecodeScalability_InitScalableParams(
    PCODECHAL_DECODE_SCALABILITY_STATE         pScalabilityState,
    PCODECHAL_DECODE_SCALABILITY_INIT_PARAMS   pInitParams,
    uint16_t                                   *pucDecPassNum)
{
    PMOS_INTERFACE                  pOsInterface;
    PMOS_VIRTUALENGINE_INTERFACE    pVEInterface;
    MOS_STATUS                      eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    CODECHAL_DECODE_CHK_NULL_RETURN(pScalabilityState);
    CODECHAL_DECODE_CHK_NULL_RETURN(pScalabilityState->pHwInterface);
    CODECHAL_DECODE_CHK_NULL_RETURN(pScalabilityState->pHwInterface->GetOsInterface());

    pOsInterface    = pScalabilityState->pHwInterface->GetOsInterface();
    pVEInterface    = pScalabilityState->pVEInterface;

    if (!pOsInterface->bSupportVirtualEngine)
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        CODECHAL_DECODE_ASSERTMESSAGE("Scalability decode must run with virtual engine interface.\n");
        return eStatus;
    }

    pScalabilityState->bScalableDecodeMode = false; // initialized to false

#if (_DEBUG || _RELEASE_INTERNAL)
    if (pOsInterface->bEnableDbgOvrdInVE)
    {
        if (!MOS_VE_CTXBASEDSCHEDULING_SUPPORTED(pOsInterface))
        {
            if (g_apoMosEnabled)
            {
                CODECHAL_DECODE_CHK_NULL_RETURN(pVEInterface->veInterface);
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
        CODECHAL_DECODE_CHK_STATUS_RETURN(pScalabilityState->pfnDecidePipeNum(pScalabilityState, pInitParams));
    }

    // Decide scalable mode or single pipe mode
    if (pScalabilityState->ucScalablePipeNum > 1 && pOsInterface->frameSplit)
    {
        pScalabilityState->bScalableDecodeMode = true;
    }

    CODECHAL_DECODE_CHK_NULL_RETURN(pucDecPassNum);
    // Decide Decode pass number - pucDecPassNum
    if (pScalabilityState->bScalableDecodeMode)
    {
        *pucDecPassNum = pScalabilityState->ucScalablePipeNum + 1; // FE + all BEs
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

    return eStatus;
}

MOS_STATUS CodecHalDecodeScalability_SetHintParams(
    PCODECHAL_DECODE_SCALABILITY_STATE         pScalabilityState,
    PCODECHAL_DECODE_SCALABILITY_SETHINT_PARMS pSetHintParms)
{
    PMOS_VIRTUALENGINE_INTERFACE    pVEInterface;
    MOS_VIRTUALENGINE_SET_PARAMS    VEParams;
    PMOS_INTERFACE                  pOsInterface;
    MOS_STATUS                      eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    CODECHAL_DECODE_CHK_NULL_RETURN(pScalabilityState);
    CODECHAL_DECODE_CHK_NULL_RETURN(pSetHintParms);
    CODECHAL_DECODE_CHK_NULL_RETURN(pScalabilityState->pHwInterface);
    CODECHAL_DECODE_CHK_NULL_RETURN(pScalabilityState->pHwInterface->GetOsInterface());

    pOsInterface    = pScalabilityState->pHwInterface->GetOsInterface();
    pVEInterface    = pScalabilityState->pVEInterface;

    MOS_ZeroMemory(&VEParams, sizeof(VEParams));
    if(!MOS_VE_CTXBASEDSCHEDULING_SUPPORTED(pOsInterface))
    {
        //not used by VE2.0
        VEParams.bNeedSyncWithPrevious       = pSetHintParms->bNeedSyncWithPrevious;
        VEParams.bSameEngineAsLastSubmission = pSetHintParms->bSameEngineAsLastSubmission;
        VEParams.bSFCInUse                   = pSetHintParms->bSFCInUse;
    }

    VEParams.ucScalablePipeNum  = pScalabilityState->ucScalablePipeNum;

    if (pScalabilityState->bScalableDecodeMode)
    {
        if (pScalabilityState->bFESeparateSubmission)
        {
            //set Hint parameter for FE submission
            VEParams.bScalableMode = false;
            if (pVEInterface->pfnVESetHintParams)
            {
                CODECHAL_DECODE_CHK_STATUS_RETURN(pVEInterface->pfnVESetHintParams(pVEInterface, &VEParams));
            }
        }

        VEParams.bScalableMode = true;
        VEParams.bHaveFrontEndCmds = (pScalabilityState->bFESeparateSubmission ? false : true);
        if (pVEInterface->pfnVESetHintParams)
        {
            CODECHAL_DECODE_CHK_STATUS_RETURN(pVEInterface->pfnVESetHintParams(pVEInterface, &VEParams));
        }
    }
    else
    {
        VEParams.bScalableMode = false;
        if (pVEInterface->pfnVESetHintParams)
        {
            CODECHAL_DECODE_CHK_STATUS_RETURN(pVEInterface->pfnVESetHintParams(pVEInterface, &VEParams));
        }
    }

    return eStatus;
}

#if (_DEBUG || _RELEASE_INTERNAL)
MOS_STATUS CodecHalDecodeScalability_DbgDumpCmdBuffer(
    CodechalDecode                      *pDecoder,
    PCODECHAL_DECODE_SCALABILITY_STATE  pScalabilityState,
    CodechalDebugInterface              *debugInterface,
    PMOS_COMMAND_BUFFER                 pPrimCmdBuf)
{
    MOS_STATUS                      eStatus = MOS_STATUS_SUCCESS;
    MOS_COMMAND_BUFFER              ScdryCmdBuffer;
    PMOS_COMMAND_BUFFER             pCmdBufferInUse;

    CODECHAL_DECODE_FUNCTION_ENTER;

    CODECHAL_DECODE_CHK_NULL_RETURN(pDecoder);
    CODECHAL_DECODE_CHK_NULL_RETURN(pScalabilityState);
    CODECHAL_DECODE_CHK_NULL_RETURN(pPrimCmdBuf);

    if (!pScalabilityState->bScalableDecodeMode)
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        CODECHAL_DECODE_ASSERTMESSAGE("Invalid decode parameters!");
        return eStatus;
    }

    if (CodecHalDecodeScalabilityIsFESeparateSubmission(pScalabilityState)
        && CodecHalDecodeScalabilityIsFEPhase(pScalabilityState))
    {
        pCmdBufferInUse = pPrimCmdBuf;
    }
    else
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHalDecodeScalability_GetVESecondaryCmdBuffer(pScalabilityState, &ScdryCmdBuffer));
        pCmdBufferInUse = &ScdryCmdBuffer;
    }

    CODECHAL_DECODE_CHK_STATUS_RETURN(debugInterface->DumpCmdBuffer(
        pCmdBufferInUse,
        CODECHAL_NUM_MEDIA_STATES,
        "_DEC"));

    return eStatus;
}
#endif

MOS_STATUS CodecHalDecodeScalability_PopulateHintParams(
    PCODECHAL_DECODE_SCALABILITY_STATE  pScalabilityState,
    PMOS_COMMAND_BUFFER                 pPrimCmdBuf)
{
    MOS_STATUS                      eStatus = MOS_STATUS_SUCCESS;
    PMOS_CMD_BUF_ATTRI_VE          pAttriVe;

    CODECHAL_DECODE_FUNCTION_ENTER;

    CODECHAL_DECODE_CHK_NULL_RETURN(pScalabilityState);
    CODECHAL_DECODE_CHK_NULL_RETURN(pPrimCmdBuf);
    pAttriVe = (PMOS_CMD_BUF_ATTRI_VE)(pPrimCmdBuf->Attributes.pAttriVe);

    if (pAttriVe)
    {
        if ((CodecHalDecodeScalabilityIsScalableMode(pScalabilityState) &&
             !CodecHalDecodeScalabilityIsFESeparateSubmission(pScalabilityState)) ||
            (CodecHalDecodeScalabilityIsFESeparateSubmission(pScalabilityState) &&
             CodecHalDecodeScalabilityIsBEPhase(pScalabilityState)))
        {
            CODECHAL_DECODE_CHK_NULL_RETURN(pScalabilityState->pScalHintParms);
            pAttriVe->VEngineHintParams = *(pScalabilityState->pScalHintParms);
        }
        else
        {
            CODECHAL_DECODE_CHK_NULL_RETURN(pScalabilityState->pSingleHintParms);
            pAttriVe->VEngineHintParams = *(pScalabilityState->pSingleHintParms);
        }

        pAttriVe->bUseVirtualEngineHint = true;
    }

    return eStatus;
}

MOS_STATUS CodecHalDecodeScalability_SignalFE2BESemaphore(
    PCODECHAL_DECODE_SCALABILITY_STATE  pScalabilityState,
    PMOS_COMMAND_BUFFER                 pCmdBufferInUse)
{
    PMOS_INTERFACE                      pOsInterface;
    MhwMiInterface                      *pMiInterface;
    MOS_SYNC_PARAMS                     SyncParams;
    MOS_STATUS                          eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    CODECHAL_DECODE_CHK_NULL_RETURN(pScalabilityState);
    CODECHAL_DECODE_CHK_NULL_RETURN(pScalabilityState->pHwInterface);
    CODECHAL_DECODE_CHK_NULL_RETURN(pScalabilityState->pHwInterface->GetOsInterface());
    CODECHAL_DECODE_CHK_NULL_RETURN(pScalabilityState->pHwInterface->GetMiInterface());
    CODECHAL_DECODE_CHK_NULL_RETURN(pCmdBufferInUse);

    pOsInterface  = pScalabilityState->pHwInterface->GetOsInterface();
    pMiInterface  = pScalabilityState->pHwInterface->GetMiInterface();

    // FE semaphore to BE0 for FE/BE Sync.
    if (pScalabilityState->bScalableDecodeMode && pScalabilityState->ucNumVdbox > 2)
    {
        // When FE separate submission enabled, use SW semaphore between FE/BE0. Otherwise use HW semaphore
        if (pScalabilityState->bFESeparateSubmission)
        {
            SyncParams = g_cInitSyncParams;
            SyncParams.GpuContext = pScalabilityState->VideoContextForFE;
            SyncParams.presSyncResource = &pScalabilityState->resFeBeSyncObject;
            CODECHAL_DECODE_CHK_STATUS_RETURN(pOsInterface->pfnEngineSignal(pOsInterface, &SyncParams));
        }
        else
        {
            //post HW semaphore (FE-BE) after FE completion , mi atomic increase 1
            CODECHAL_DECODE_CHK_STATUS_RETURN(pScalabilityState->pHwInterface->SendMiAtomicDwordCmd(&pScalabilityState->resSemaMemFEBE, 1, MHW_MI_ATOMIC_INC, pCmdBufferInUse));
        }
    }

    return eStatus;
}

MOS_STATUS CodecHalDecodeScalability_FEBESync(
    PCODECHAL_DECODE_SCALABILITY_STATE  pScalabilityState,
    PMOS_COMMAND_BUFFER                 pCmdBufferInUse)
{
    PMOS_INTERFACE                      pOsInterface;
    MhwMiInterface                      *pMiInterface;
    uint32_t                            HcpDecPhase;
    MOS_STATUS                          eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    CODECHAL_DECODE_CHK_NULL_RETURN(pScalabilityState);
    CODECHAL_DECODE_CHK_NULL_RETURN(pScalabilityState->pHwInterface);
    CODECHAL_DECODE_CHK_NULL_RETURN(pScalabilityState->pHwInterface->GetOsInterface());
    CODECHAL_DECODE_CHK_NULL_RETURN(pScalabilityState->pHwInterface->GetMiInterface());
    CODECHAL_DECODE_CHK_NULL_RETURN(pCmdBufferInUse);

    pOsInterface        = pScalabilityState->pHwInterface->GetOsInterface();
    pMiInterface        = pScalabilityState->pHwInterface->GetMiInterface();
    HcpDecPhase         = pScalabilityState->HcpDecPhase;

    //FE& BE0 Sync.
    if (HcpDecPhase == CODECHAL_HCP_DECODE_PHASE_BE0 && pScalabilityState->pHwInterface->GetMfxInterface()->GetNumVdbox() > 2)
    {
        if (pScalabilityState->bFESeparateSubmission)
        {
            MOS_SYNC_PARAMS SyncParams;

            SyncParams = g_cInitSyncParams;
            SyncParams.GpuContext = pScalabilityState->VideoContext;
            SyncParams.presSyncResource = &pScalabilityState->resFeBeSyncObject;

            CODECHAL_DECODE_CHK_STATUS_RETURN(pOsInterface->pfnEngineWait(pOsInterface, &SyncParams));
        }
        else
        {
            pMiInterface->AddWatchdogTimerStopCmd(pCmdBufferInUse);

            CODECHAL_DECODE_CHK_STATUS_RETURN(pScalabilityState->pHwInterface->SendHwSemaphoreWaitCmd(&pScalabilityState->resSemaMemFEBE, 1, MHW_MI_SAD_EQUAL_SDD, pCmdBufferInUse));
            //reset semaphore. mi atomic decrease 1
            CODECHAL_DECODE_CHK_STATUS_RETURN(pScalabilityState->pHwInterface->SendMiAtomicDwordCmd(&pScalabilityState->resSemaMemFEBE, 1, MHW_MI_ATOMIC_DEC, pCmdBufferInUse));
        }
    }

    if (CodecHalDecodeScalabilityIsBEPhase(pScalabilityState))
    {
        // Stop Watchdog before BEs wait
        pMiInterface->AddWatchdogTimerStopCmd(pCmdBufferInUse);

        //HW Semaphore for BEs Starting at the same time
        CODECHAL_DECODE_CHK_STATUS_RETURN(pScalabilityState->pHwInterface->SendMiAtomicDwordCmd(&pScalabilityState->resSemaMemBEs, 1, MHW_MI_ATOMIC_INC, pCmdBufferInUse));
        CODECHAL_DECODE_CHK_STATUS_RETURN(pScalabilityState->pHwInterface->SendHwSemaphoreWaitCmd(
            &pScalabilityState->resSemaMemBEs,
            pScalabilityState->ucScalablePipeNum,
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
        CODECHAL_DECODE_CHK_STATUS_RETURN(pScalabilityState->pHwInterface->SendMiAtomicDwordCmd(&pScalabilityState->resSemaMemBEs, 1, MHW_MI_ATOMIC_DEC, pCmdBufferInUse));

        // Condidtional BB END for streamout buffer writing over allocated size
        CODECHAL_DECODE_CHK_STATUS_RETURN(pScalabilityState->pHwInterface->SendCondBbEndCmd(
            &pScalabilityState->resFEStatusBuffer,
            CODECHAL_OFFSETOF(CODECHAL_DECODE_SCALABILITY_FE_STATUS, dwCarryFlagOfReportedSizeMinusAllocSize),
            0,
            true,
            pCmdBufferInUse));

    }

    return eStatus;
}

MOS_STATUS CodecHalDecodeScalability_BEsCompletionSync(
    PCODECHAL_DECODE_SCALABILITY_STATE  pScalabilityState,
    PMOS_COMMAND_BUFFER                 pCmdBufferInUse)
{
    MhwMiInterface             *pMiInterface;
    uint32_t                   HcpDecPhase;
    MOS_STATUS                 eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    CODECHAL_DECODE_CHK_NULL_RETURN(pScalabilityState);
    CODECHAL_DECODE_CHK_NULL_RETURN(pScalabilityState->pHwInterface);
    CODECHAL_DECODE_CHK_NULL_RETURN(pScalabilityState->pHwInterface->GetMiInterface());
    CODECHAL_DECODE_CHK_NULL_RETURN(pCmdBufferInUse);
    HcpDecPhase        = pScalabilityState->HcpDecPhase;

    if (CodecHalDecodeScalabilityIsLastCompletePhase(pScalabilityState))
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(pScalabilityState->pHwInterface->SendHwSemaphoreWaitCmd(
            &pScalabilityState->resSemaMemCompletion,
            pScalabilityState->ucScalablePipeNum - 1,
            MHW_MI_SAD_EQUAL_SDD,
            pCmdBufferInUse));

        for (int i = 0; i < pScalabilityState->ucScalablePipeNum - 1; i++)
        {
            CODECHAL_DECODE_CHK_STATUS_RETURN(pScalabilityState->pHwInterface->SendMiAtomicDwordCmd(
                &pScalabilityState->resSemaMemCompletion,
                1,
                MHW_MI_ATOMIC_DEC,
                pCmdBufferInUse));
        }
    }
    else
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(pScalabilityState->pHwInterface->SendMiAtomicDwordCmd(
            &pScalabilityState->resSemaMemCompletion,
            1,
            MHW_MI_ATOMIC_INC,
            pCmdBufferInUse));
    }

    return eStatus;
}

MOS_STATUS CodecHalDecodeScalability_ReadCSEngineIDReg(
    PCODECHAL_DECODE_SCALABILITY_STATE pScalabilityState,
    CodechalDecodeStatusBuffer        *pDecodeStatusBuf,
    PMOS_COMMAND_BUFFER                pCmdBufferInUse)
{
    MHW_MI_STORE_REGISTER_MEM_PARAMS  StoreRegParams;
    MhwMiInterface                    *pMiInterface;
    MmioRegistersHcp                  *pMmioRegisters;
    uint8_t                           ucPhaseIndex = 0;
    uint32_t                          dwOffset = 0;
    uint32_t                          dwCurrIndex = 0;
    uint32_t                          dwPreIndex = 0;
    MOS_STATUS                        eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    CODECHAL_DECODE_CHK_NULL_RETURN(pDecodeStatusBuf);
    CODECHAL_DECODE_CHK_NULL_RETURN(pScalabilityState);
    CODECHAL_DECODE_CHK_NULL_RETURN(pScalabilityState->pHwInterface);
    CODECHAL_DECODE_CHK_NULL_RETURN(pScalabilityState->pHwInterface->GetMiInterface());

    pMiInterface = pScalabilityState->pHwInterface->GetMiInterface();
    pMmioRegisters = pScalabilityState->pHwInterface->GetHcpInterface()->GetMmioRegisters(MHW_VDBOX_NODE_1); //relative mmio addressing.

    ucPhaseIndex = (pScalabilityState->HcpDecPhase < CODECHAL_HCP_DECODE_PHASE_FE) ?
        0 : (pScalabilityState->HcpDecPhase - CODECHAL_HCP_DECODE_PHASE_FE);

    if (ucPhaseIndex >= CODECHAL_HCP_DECODE_SCALABLE_MAX_PHASE_NUM)
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        CODECHAL_DECODE_ASSERTMESSAGE("Invalid HCP decode phase!");
        return eStatus;
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

    dwOffset = (dwCurrIndex * sizeof(CodechalDecodeStatus)) +
        pDecodeStatusBuf->m_csEngineIdOffset + sizeof(uint32_t)* ucPhaseIndex +
        sizeof(uint32_t)* 2;

    StoreRegParams.presStoreBuffer  = &pDecodeStatusBuf->m_statusBuffer;
    StoreRegParams.dwOffset         = dwOffset;
    StoreRegParams.dwRegister       = pMmioRegisters->csEngineIdOffset;
    CODECHAL_DECODE_CHK_STATUS_RETURN(pMiInterface->AddMiStoreRegisterMemCmd(pCmdBufferInUse, &StoreRegParams));

    return eStatus;
}

MOS_STATUS IsHevcBufferReallocNeeded(
    CodechalHwInterface                  *hwInterface,
    MHW_VDBOX_HCP_INTERNAL_BUFFER_TYPE   bufferType,
    PMHW_VDBOX_HCP_BUFFER_REALLOC_PARAMS reallocParam)
{
    return hwInterface->GetHcpInterface()->IsHevcBufferReallocNeeded(bufferType, reallocParam);
}

MOS_STATUS GetHevcBufferSize(
    CodechalHwInterface                 *hwInterface,
    MHW_VDBOX_HCP_INTERNAL_BUFFER_TYPE  bufferType,
    PMHW_VDBOX_HCP_BUFFER_SIZE_PARAMS   hcpBufSizeParam)
{
    return hwInterface->GetHcpInterface()->GetHevcBufferSize(bufferType, hcpBufSizeParam);
}

MOS_STATUS IsVp9BufferReallocNeeded(
    CodechalHwInterface                  *hwInterface,
    MHW_VDBOX_HCP_INTERNAL_BUFFER_TYPE   bufferType,
    PMHW_VDBOX_HCP_BUFFER_REALLOC_PARAMS reallocParam)
{
    return hwInterface->GetHcpInterface()->IsVp9BufferReallocNeeded(bufferType, reallocParam);
}

MOS_STATUS GetVp9BufferSize(
    CodechalHwInterface                 *hwInterface,
    MHW_VDBOX_HCP_INTERNAL_BUFFER_TYPE  bufferType,
    PMHW_VDBOX_HCP_BUFFER_SIZE_PARAMS   hcpBufSizeParam)
{
    return hwInterface->GetHcpInterface()->GetVp9BufferSize(bufferType, hcpBufSizeParam);
}

MOS_STATUS CodecHalDecodeScalability_InitializeState (
    CodechalDecode                     *pDecoder,
    PCODECHAL_DECODE_SCALABILITY_STATE  pScalabilityState,
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

    CODECHAL_DECODE_CHK_NULL_RETURN(pDecoder);
    CODECHAL_DECODE_CHK_NULL_RETURN(pScalabilityState);
    CODECHAL_DECODE_CHK_NULL_RETURN(hwInterface);
    osInterface = hwInterface->GetOsInterface();
    CODECHAL_DECODE_CHK_NULL_RETURN(osInterface);

    vdboxMfxInterface = hwInterface->GetMfxInterface();
    CODECHAL_DECODE_CHK_NULL_RETURN(vdboxMfxInterface);
    vdboxNum = vdboxMfxInterface->GetNumVdbox();

    if (vdboxNum < 2
#if (_DEBUG || _RELEASE_INTERNAL)
        || !osInterface->bHcpDecScalabilityMode
#endif
    )
   {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        CODECHAL_DECODE_ASSERTMESSAGE("not support scalability on this platform.");
        return eStatus;
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

    MOS_ZeroMemory(&UserFeatureData, sizeof(UserFeatureData));
    MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_SCALABILITY_OVERRIDE_SPLIT_WIDTH_IN_MINCB,
        &UserFeatureData);
    pScalabilityState->dbgOvrdWidthInMinCb = UserFeatureData.u32Data;
#endif

    // enable FE separate submission by default in multi-pipe mode
    if (hwInterface->GetMfxInterface()->GetNumVdbox() > 2)
    {
        pScalabilityState->bFESeparateSubmission = true;
    }
    else
    {
        // no benefit to enable FE separate submission on 2 vdbox config.
        pScalabilityState->bFESeparateSubmission = false;
    }

#if (_DEBUG || _RELEASE_INTERNAL)
    if (osInterface->bEnableDbgOvrdInVE || Mos_Solo_IsInUse(osInterface))
    {
        //if DbgOverride is enabled, FE separate submission is not supported
        pScalabilityState->bFESeparateSubmission = false;
    }
#endif

    if (pScalabilityState->bFESeparateSubmission)
    {
        MOS_GPU_CONTEXT         GpuContext = MOS_VE_CTXBASEDSCHEDULING_SUPPORTED(osInterface) ? MOS_GPU_CONTEXT_VIDEO : MOS_GPU_CONTEXT_VIDEO4;
        GpuContext = MOS_VE_MULTINODESCALING_SUPPORTED(osInterface) ? MOS_GPU_CONTEXT_VIDEO4 : GpuContext;

        MHW_VDBOX_GPUNODE_LIMIT gpuNodeLimit;
        CODECHAL_DECODE_CHK_STATUS_RETURN(vdboxMfxInterface->FindGpuNodeToUse(
            &gpuNodeLimit));
        MOS_GPU_NODE videoGpuNode = (MOS_GPU_NODE)(gpuNodeLimit.dwGpuNodeToUse);

        MOS_GPUCTX_CREATOPTIONS createOpts;
        CODECHAL_DECODE_CHK_STATUS_RETURN(osInterface->pfnCreateGpuContext(
            osInterface,
            GpuContext,
            videoGpuNode,
            &createOpts));
        pScalabilityState->VideoContextForFE = GpuContext;
    }

    pScalabilityState->Standard              = pDecoder->GetStandard();
    pScalabilityState->VideoContext          = pDecoder->GetVideoContext();
    pScalabilityState->bShortFormatInUse     = bShortFormat;
    pScalabilityState->ucNumVdbox            = vdboxNum;
    pScalabilityState->pHwInterface          = hwInterface;

    //virtual engine init with scalability
    MOS_ZeroMemory(&VEInitParms, sizeof(VEInitParms));
    VEInitParms.bScalabilitySupported     = true;
    VEInitParms.bFESeparateSubmit         = pScalabilityState->bFESeparateSubmission;
    VEInitParms.ucMaxNumPipesInUse        = (vdboxNum == 2) ? 2 : 3;
    VEInitParms.ucNumOfSdryCmdBufSets     = CODECHAL_SCALABILITY_DECODE_SECONDARY_CMDBUFSET_NUM;
    VEInitParms.ucMaxNumOfSdryCmdBufInOneFrame = (pScalabilityState->bFESeparateSubmission) ? VEInitParms.ucMaxNumPipesInUse : (VEInitParms.ucMaxNumPipesInUse + 1);
    CODECHAL_DECODE_CHK_STATUS_RETURN(Mos_VirtualEngineInterface_Initialize(osInterface, &VEInitParms));
    pScalabilityState->pVEInterface = pVEInterface = osInterface->pVEInterf;

    if (pVEInterface->pfnVEGetHintParams)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(pVEInterface->pfnVEGetHintParams(pVEInterface, true, &pScalabilityState->pScalHintParms));
    }
    if (pVEInterface->pfnVEGetHintParams)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(pVEInterface->pfnVEGetHintParams(pVEInterface, false, &pScalabilityState->pSingleHintParms));
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

    if (pDecoder->GetStandard() == CODECHAL_HEVC)
    {
        pScalabilityState->pfnGetHcpBufferSize         = GetHevcBufferSize;
        pScalabilityState->pfnIsHcpBufferReallocNeeded = IsHevcBufferReallocNeeded;
    }
    else if (pDecoder->GetStandard() == CODECHAL_VP9)
    {
        pScalabilityState->pfnGetHcpBufferSize         = GetVp9BufferSize;
        pScalabilityState->pfnIsHcpBufferReallocNeeded = IsVp9BufferReallocNeeded;
    }
    else
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        CODECHAL_DECODE_ASSERTMESSAGE("unsupported decode format for scalability mode.");
        return eStatus;
    }

    pScalabilityState->bToggleCABACStreamOutBuffer = false;
    pScalabilityState->sliceStateCLs = CODECHAL_SCALABILITY_SLICE_STATE_CACHELINES_PER_SLICE;
    pScalabilityState->pfnDecidePipeNum = CodecHalDecodeScalability_DecidePipeNum;
    pScalabilityState->pfnMapPipeNumToLRCACount = CodechalDecodeScalability_MapPipeNumToLRCACount;
#if (_DEBUG || _RELEASE_INTERNAL)
    pScalabilityState->pfnDebugOvrdDecidePipeNum = CodechalDecodeScalability_DebugOvrdDecidePipeNum;
#endif

    CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHalDecodeScalability_AllocateResources_FixedSizes(pScalabilityState));

    return eStatus;
}

bool CodecHalDecodeScalabilityIsToSubmitCmdBuffer(
    PCODECHAL_DECODE_SCALABILITY_STATE pScalabilityState)
{
    if (pScalabilityState == nullptr)
    {
        return false;
    }
    else
    {
        return (CodecHalDecodeScalabilityIsFinalBEPhase(pScalabilityState) ||
            (pScalabilityState->HcpDecPhase == CODECHAL_HCP_DECODE_PHASE_FE && pScalabilityState->bFESeparateSubmission));
    }
}

void CodecHalDecodeScalability_DecPhaseToSubmissionType(
    PCODECHAL_DECODE_SCALABILITY_STATE pScalabilityState,
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
        case CODECHAL_HCP_DECODE_PHASE_BE1:
            pCmdBuffer->iSubmissionType = SUBMISSION_TYPE_MULTI_PIPE_SLAVE | SUBMISSION_TYPE_MULTI_PIPE_FLAGS_LAST_PIPE;
            break;
        case CODECHAL_HCP_DECODE_PHASE_RESERVED:
        default:
            pCmdBuffer->iSubmissionType = SUBMISSION_TYPE_MULTI_PIPE_ALONE;
            break;
    }
}