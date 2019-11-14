/*
* Copyright (c) 2010-2019, Intel Corporation
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
//! \file      vphal_render_hdr_base.cpp
//! \brief     Unified VP HAL HDR Implementation 
//!
//!
//! \file     vphal_render_hdr_base.cpp
//! \brief    Common interface and structure used in HDR
//! \details  Common interface and structure used in HDR which are platform independent
//!

#include "vphal.h"
#include "vphal_renderer.h"
#include "vphal_render_hdr_base.h"
#include "renderhal_platform_interface.h"
#include "vphal_render_hdr_g9_base.h"

static bool sForceSplitFrame = false;

#if (_DEBUG || _RELEASE_INTERNAL)
static bool sEnableKernelDump = false;
#endif

//!
//! \brief    Initialize HDR state
//! \details  Initialize HDR state
//! \param    PVPHAL_HDR_STATE pHdrState
//!           [in,out] HDR State pointer
//! \param    const VphalSettings* pSettings
//!           [in] Pointer to VPHAL Setting
//! \param    Kdll_KernelCache * pKernelCache
//!           [in] Pointer to kernel cache
//! \return   void
//!
MOS_STATUS VpHal_HdrInitialize(
    PVPHAL_HDR_STATE         pHdrState,
    const VphalSettings      *pSettings,
    Kdll_State               *pKernelDllState)
{
    int32_t    i;
    uint32_t   dwSize = 0;
    bool       bAllocated = false;
    MOS_NULL_RENDERING_FLAGS    NullRenderingFlags;
    MOS_STATUS                  eStatus;
    PRENDERHAL_INTERFACE        pRenderHal;
    MOS_USER_FEATURE_VALUE_DATA UserFeatureData;

    eStatus = MOS_STATUS_SUCCESS;

    VPHAL_PUBLIC_CHK_NULL(pHdrState);
    VPHAL_PUBLIC_CHK_NULL(pHdrState->pOsInterface);
    VPHAL_PUBLIC_CHK_NULL(pHdrState->pSkuTable);
    VPHAL_PUBLIC_CHK_NULL(pKernelDllState);

    NullRenderingFlags          = 
                    pHdrState->pOsInterface->pfnGetNullHWRenderFlags(pHdrState->pOsInterface);
    pHdrState->bNullHwRenderHdr = false;
    pRenderHal                  = pHdrState->pRenderHal;

    VPHAL_PUBLIC_CHK_NULL(pRenderHal);

    // Setup disable render flag controlled by a user feature key for validation purpose
    pHdrState->bDisableRender = (pSettings->disableHdr) ? true : false;

    // Setup interface to KDLL
    pHdrState->pKernelCache   = &pKernelDllState->ComponentKernelCache;

    eStatus = MOS_STATUS_SUCCESS;

    pHdrState->uiSplitFramePortions = 1;

    // If user set the user feature key, then the uiSplitFramePortions specified will always be used,
    // no matter HW support preemption or not.
    // If it is not set, and HW doesn't support preemption, then the split portions
    // will be calculted based on resolution.
    if (!sForceSplitFrame)
    {
        if (MEDIA_IS_SKU(pHdrState->pSkuTable, FtrMediaMidBatchPreempt) ||
            MEDIA_IS_SKU(pHdrState->pSkuTable, FtrMediaThreadGroupLevelPreempt) ||
            MEDIA_IS_SKU(pHdrState->pSkuTable, FtrMediaMidThreadLevelPreempt))
        {
            pHdrState->uiSplitFramePortions = 1;
            sForceSplitFrame = true;
        }
    }

    pHdrState->bFtrComputeWalker = false;
    pHdrState->uiSplitFramePortions = 1;

    VpHal_HdrInitInterface_g9(pHdrState);            // Total number of slices
    
finish:
    return eStatus;
}

//!
//! \brief    Destroy HDR state
//! \details  Release local resources.
//! \param    PVPHAL_HDR_STATE pHdrState
//!           [in] Pointer to HDR state
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VpHal_HdrDestroy(
    PVPHAL_HDR_STATE    pHdrState)
{
    MOS_STATUS                  eStatus = MOS_STATUS_SUCCESS;
    VPHAL_PUBLIC_CHK_NULL(pHdrState);

    VpHal_HdrDestroyInterface_g9(pHdrState);

    // Free allocations
    if (pHdrState->pfnFreeResources)
    {
        pHdrState->pfnFreeResources(pHdrState);
    }

finish:
    return eStatus;
}

//!
//! \brief    Assemble the HDR kernel per layer stages
//! \details  Contruct a case id from the input information, and look up the configuration entry in the table.
//! \param    PVPHAL_HDR_STATE pHdrState
//!           [in] Pointer to HDR state
//! \param    PVPHAL_SURFACE pSource
//!           [in] Pointer to source surface
//! \param    PVPHAL_SURFACE pTarget
//!           [in] Pointer to target surface
//! \param    HDRStageConfigEntry *pConfigEntry
//!           [out] Pointer to configuration entry
//! \return   bool
//!           True if find proper configuration entry, otherwise false
//!
static bool Vphal_HdrToneMappingStagesAssemble(
    PVPHAL_HDR_STATE       pHdrState,
    PVPHAL_SURFACE         pSource,
    PVPHAL_SURFACE         pTarget,
    HDRStageConfigEntry   *pConfigEntry)
{
    HDRCaseID id = { 0 };

    VPHAL_RENDER_ASSERT(pHdrState);
    VPHAL_RENDER_ASSERT(pSource);
    VPHAL_RENDER_ASSERT(pTarget);
    VPHAL_RENDER_ASSERT(pConfigEntry);

    if (!pHdrState || !pSource || !pTarget || !pConfigEntry)
        return false;

    // Because FP16 format can represent both SDR or HDR, we need do judgement here.
    // We need this information because we dont have unified tone mapping algorithm for various scenarios(H2S/H2H).
    // To do this, we make two assumptions:
    // 1. This colorspace will be set to BT709/Gamma1.0 from APP, so such information can NOT be used to check HDR.
    // 2. If APP pass any HDR metadata, it indicates this is HDR.
    id.InputXDR     = (pSource->pHDRParams &&
                      ((pSource->pHDRParams->EOTF == VPHAL_HDR_EOTF_SMPTE_ST2084) || IS_RGB64_FLOAT_FORMAT(pSource->Format))) ? 1 : 0;
    id.InputGamut   = IS_COLOR_SPACE_BT2020(pSource->ColorSpace);
    id.OutputXDR    = (pTarget->pHDRParams &&
                      ((pTarget->pHDRParams->EOTF == VPHAL_HDR_EOTF_SMPTE_ST2084) || IS_RGB64_FLOAT_FORMAT(pTarget->Format))) ? 1 : 0;
    id.OutputGamut  = IS_COLOR_SPACE_BT2020(pTarget->ColorSpace);
    id.OutputLinear = IS_RGB64_FLOAT_FORMAT(pTarget->Format) ? 1 : 0;

    if (pHdrState->pHDRStageConfigTable)
    {
        pConfigEntry->value = pHdrState->pHDRStageConfigTable[id.index];
    }
    else
    {
        pConfigEntry->Invalid = 1;
    }

    if (pConfigEntry->Invalid == 1)
    {
        VPHAL_RENDER_ASSERTMESSAGE("Tone mapping stages assembling failed, please reexamine the usage case(case id %d)! "
            "If it is definitely a correct usage, please add an entry in HDRStageEnableTable.", id.index);
    }

    return (pConfigEntry->Invalid != 1);
}

//!
//! \brief    Update per layer pipeline states and return update mask for each layer
//! \details  Update per layer pipeline states and return update mask for each layer
//! \param    PVPHAL_HDR_STATE pHdrStatee
//!           [in] Pointer to HDR state
//! \param    uint32_t* pdwUpdateMask
//!           [out] Pointer to update mask
//! \return   MOS_STATUS
//!
MOS_STATUS VpHal_HdrUpdatePerLayerPipelineStates(
    PVPHAL_HDR_STATE    pHdrState,
    uint32_t*           pdwUpdateMask)
{
    MOS_STATUS           eStatus              = MOS_STATUS_UNKNOWN;
    uint32_t             i                    = 0;
    PVPHAL_SURFACE       pSrc                 = nullptr;
    PVPHAL_SURFACE       pTarget              = nullptr;
    VPHAL_HDR_LUT_MODE   CurrentLUTMode       = VPHAL_HDR_LUT_MODE_NONE;
    VPHAL_GAMMA_TYPE     CurrentEOTF          = VPHAL_GAMMA_NONE;            //!< EOTF
    VPHAL_GAMMA_TYPE     CurrentOETF          = VPHAL_GAMMA_NONE;            //!< OETF
    VPHAL_HDR_MODE       CurrentHdrMode       = VPHAL_HDR_MODE_NONE;      //!< Hdr Mode
    VPHAL_HDR_CCM_TYPE   CurrentCCM           = VPHAL_HDR_CCM_NONE;           //!< CCM Mode
    VPHAL_HDR_CCM_TYPE   CurrentCCMExt1       = VPHAL_HDR_CCM_NONE;       //!< CCM Ext1 Mode
    VPHAL_HDR_CCM_TYPE   CurrentCCMExt2       = VPHAL_HDR_CCM_NONE;       //!< CCM Ext2 Mode
    VPHAL_HDR_CSC_TYPE   CurrentPriorCSC      = VPHAL_HDR_CSC_NONE;      //!< Prior CSC Mode
    VPHAL_HDR_CSC_TYPE   CurrentPostCSC       = VPHAL_HDR_CSC_NONE;       //!< Post CSC Mode
    HDRStageConfigEntry  ConfigEntry          = { 0 };
    HDRStageEnables      StageEnables         = { 0 };

    VPHAL_RENDER_CHK_NULL(pHdrState);
    VPHAL_RENDER_CHK_NULL(pdwUpdateMask);
    VPHAL_PUBLIC_CHK_NULL(pHdrState->pTargetSurf[0]);

    *pdwUpdateMask = 0;

    pTarget = (PVPHAL_SURFACE)pHdrState->pTargetSurf[0];

    for (i = 0; i < VPHAL_MAX_HDR_INPUT_LAYER; i++)
    {
        if (pHdrState->pSrcSurf[i] == nullptr)
        {
            pHdrState->LUTMode[i]   = VPHAL_HDR_LUT_MODE_NONE;
            pHdrState->EOTFGamma[i] = VPHAL_GAMMA_NONE;
            pHdrState->OETFGamma[i] = VPHAL_GAMMA_NONE;
            pHdrState->CCM[i]       = VPHAL_HDR_CCM_NONE;
            pHdrState->CCMExt1[i]   = VPHAL_HDR_CCM_NONE;
            pHdrState->CCMExt2[i]   = VPHAL_HDR_CCM_NONE;
            pHdrState->HdrMode[i]   = VPHAL_HDR_MODE_NONE;
            pHdrState->PriorCSC[i]  = VPHAL_HDR_CSC_NONE;
            pHdrState->PostCSC[i]   = VPHAL_HDR_CSC_NONE;

            pHdrState->StageEnableFlags[i].value = 0;
            MOS_ZeroMemory(&pHdrState->HDRLastFrameSourceParams[i], sizeof(VPHAL_HDR_PARAMS));

            continue;
        }

        pSrc = (PVPHAL_SURFACE)pHdrState->pSrcSurf[i];

        CurrentLUTMode  = VPHAL_HDR_LUT_MODE_NONE;
        CurrentEOTF     = VPHAL_GAMMA_NONE;
        CurrentOETF     = VPHAL_GAMMA_NONE;
        CurrentHdrMode  = VPHAL_HDR_MODE_NONE;
        CurrentCCM      = VPHAL_HDR_CCM_NONE;
        CurrentCCMExt1  = VPHAL_HDR_CCM_NONE;
        CurrentCCMExt2  = VPHAL_HDR_CCM_NONE;
        CurrentPriorCSC = VPHAL_HDR_CSC_NONE;
        CurrentPostCSC  = VPHAL_HDR_CSC_NONE;

        if (!Vphal_HdrToneMappingStagesAssemble(pHdrState, pSrc, pTarget, &ConfigEntry))
        {
            eStatus = MOS_STATUS_INVALID_PARAMETER;
            goto finish;
        }

        CurrentHdrMode = (VPHAL_HDR_MODE)ConfigEntry.PWLF;
        CurrentCCM     = (VPHAL_HDR_CCM_TYPE)ConfigEntry.CCM;
        CurrentCCMExt1 = (VPHAL_HDR_CCM_TYPE)ConfigEntry.CCMExt1;
        CurrentCCMExt2 = (VPHAL_HDR_CCM_TYPE)ConfigEntry.CCMExt2;

        // So far only enable auto mode in H2S cases.
        if (CurrentHdrMode == VPHAL_HDR_MODE_TONE_MAPPING &&
            pSrc->pHDRParams                              &&
            pSrc->pHDRParams->bAutoMode                   &&
            pSrc->SurfType == SURF_IN_PRIMARY)
        {
            CurrentHdrMode = VPHAL_HDR_MODE_TONE_MAPPING_AUTO_MODE;
        }

        StageEnables.value             = 0;
        StageEnables.CCMEnable         = (CurrentCCM     != VPHAL_HDR_CCM_NONE ) ? 1 : 0;
        StageEnables.PWLFEnable        = (CurrentHdrMode != VPHAL_HDR_MODE_NONE) ? 1 : 0;
        StageEnables.CCMExt1Enable     = (CurrentCCMExt1 != VPHAL_HDR_CCM_NONE ) ? 1 : 0;
        StageEnables.CCMExt2Enable     = (CurrentCCMExt2 != VPHAL_HDR_CCM_NONE ) ? 1 : 0;
        StageEnables.GamutClamp1Enable = ConfigEntry.GamutClamp1;
        StageEnables.GamutClamp2Enable = ConfigEntry.GamutClamp2;

        if (IS_YUV_FORMAT(pSrc->Format) || pSrc->Format == Format_AYUV)
        {
            StageEnables.PriorCSCEnable = 1;
        }

        if (!IS_RGB64_FLOAT_FORMAT(pSrc->Format) &&
            (StageEnables.CCMEnable || StageEnables.PWLFEnable || StageEnables.CCMExt1Enable || StageEnables.CCMExt2Enable))
        {
            StageEnables.EOTFEnable = 1;
        }

        if (!IS_RGB64_FLOAT_FORMAT(pTarget->Format) && StageEnables.EOTFEnable)
        {
            StageEnables.OETFEnable = 1;
        }

        if (IS_YUV_FORMAT(pTarget->Format))
        {
            StageEnables.PostCSCEnable = 1;
        }

        if (pSrc->SurfType == SURF_IN_PRIMARY && pHdrState->GlobalLutMode != VPHAL_HDR_LUT_MODE_3D)
        {
            CurrentLUTMode = VPHAL_HDR_LUT_MODE_2D;
        }
        else
        {
            CurrentLUTMode = VPHAL_HDR_LUT_MODE_3D;
        }

        // Neither 1D nor 3D LUT is needed in linear output case.
        if (IS_RGB64_FLOAT_FORMAT(pHdrState->pTargetSurf[0]->Format))
        {
            CurrentLUTMode = VPHAL_HDR_LUT_MODE_NONE;
        }

        // EOTF/CCM/Tone Mapping/OETF require RGB input
        // So if prior CSC is needed, it will always be YUV to RGB conversion
        if (StageEnables.PriorCSCEnable)
        {
            if (pSrc->ColorSpace == CSpace_BT601)
            {
                CurrentPriorCSC = VPHAL_HDR_CSC_YUV_TO_RGB_BT601;
            }
            else if (pSrc->ColorSpace == CSpace_BT709)
            {
                CurrentPriorCSC = VPHAL_HDR_CSC_YUV_TO_RGB_BT709;
            }
            else if (pSrc->ColorSpace == CSpace_BT2020)
            {
                CurrentPriorCSC = VPHAL_HDR_CSC_YUV_TO_RGB_BT2020;
            }
            else if (pSrc->ColorSpace == CSpace_BT2020_FullRange)
            {
                CurrentPriorCSC = VPHAL_HDR_CSC_YUV_TO_RGB_BT2020;
            }
            else
            {
                VPHAL_RENDER_ASSERTMESSAGE("Color Space Not found.");
                eStatus = MOS_STATUS_INVALID_PARAMETER;
                goto finish;
            }
        }

        if (StageEnables.EOTFEnable)
        {
            if ((!pSrc->pHDRParams) ||
                (pSrc->pHDRParams &&
                 (pSrc->pHDRParams->EOTF == VPHAL_HDR_EOTF_TRADITIONAL_GAMMA_SDR ||
                  pSrc->pHDRParams->EOTF == VPHAL_HDR_EOTF_TRADITIONAL_GAMMA_HDR)))
            {
                // Mark tranditional HDR/SDR gamma as the same type
                CurrentEOTF = VPHAL_GAMMA_TRADITIONAL_GAMMA;
            }
            else if (pSrc->pHDRParams &&
                     pSrc->pHDRParams->EOTF == VPHAL_HDR_EOTF_SMPTE_ST2084)
            {
                CurrentEOTF = VPHAL_GAMMA_SMPTE_ST2084;
            }
            else if (pSrc->pHDRParams &&
                     pSrc->pHDRParams->EOTF == VPHAL_HDR_EOTF_BT1886)
            {
                CurrentEOTF = VPHAL_GAMMA_BT1886;
            }
            else
            {
                VPHAL_RENDER_ASSERTMESSAGE("Invalid EOTF setting for tone mapping");
                eStatus = MOS_STATUS_INVALID_PARAMETER;
                goto finish;
            }
        }

        if (StageEnables.OETFEnable)
        {
            if ((!pTarget->pHDRParams) ||
                (pTarget->pHDRParams &&
                 (pTarget->pHDRParams->EOTF == VPHAL_HDR_EOTF_TRADITIONAL_GAMMA_SDR ||
                  pTarget->pHDRParams->EOTF == VPHAL_HDR_EOTF_TRADITIONAL_GAMMA_HDR)))
            {
                CurrentOETF = VPHAL_GAMMA_SRGB;
            }
            else if (pTarget->pHDRParams &&
                     pTarget->pHDRParams->EOTF == VPHAL_HDR_EOTF_SMPTE_ST2084)
            {
                CurrentOETF = VPHAL_GAMMA_SMPTE_ST2084;
            }
            else
            {
                VPHAL_RENDER_ASSERTMESSAGE("Invalid EOTF setting for tone mapping");
                eStatus = MOS_STATUS_INVALID_PARAMETER;
                goto finish;
            }
        }

        // OETF will output RGB surface
        // So if post CSC is needed, it will always be RGB to YUV conversion
        if (StageEnables.PostCSCEnable)
        {
            if (pTarget->ColorSpace == CSpace_BT601)
            {
                CurrentPostCSC = VPHAL_HDR_CSC_RGB_TO_YUV_BT601;
            }
            else if (pTarget->ColorSpace == CSpace_BT709)
            {
                CurrentPostCSC = VPHAL_HDR_CSC_RGB_TO_YUV_BT709;
            }
            else if (pTarget->ColorSpace == CSpace_BT709_FullRange)
            {
                CurrentPostCSC = VPHAL_HDR_CSC_RGB_TO_YUV_BT709_FULLRANGE;
            }
            else if (pTarget->ColorSpace == CSpace_BT2020 ||
                     pTarget->ColorSpace == CSpace_BT2020_FullRange)
            {
                CurrentPostCSC = VPHAL_HDR_CSC_RGB_TO_YUV_BT2020;
            }
            else
            {
                VPHAL_RENDER_ASSERTMESSAGE("Color Space Not found.");
                eStatus = MOS_STATUS_INVALID_PARAMETER;
                goto finish;
            }
        }

        if (pHdrState->LUTMode[i]   != CurrentLUTMode  ||
            pHdrState->EOTFGamma[i] != CurrentEOTF     ||
            pHdrState->OETFGamma[i] != CurrentOETF     ||
            pHdrState->CCM[i]       != CurrentCCM      ||
            pHdrState->CCMExt1[i]   != CurrentCCMExt1  ||
            pHdrState->CCMExt2[i]   != CurrentCCMExt2  ||
            pHdrState->HdrMode[i]   != CurrentHdrMode  ||
            pHdrState->PriorCSC[i]  != CurrentPriorCSC ||
            pHdrState->PostCSC[i]   != CurrentPostCSC)
        {
            *pdwUpdateMask |= (1 << i);
        }
        
        if (pSrc->pHDRParams)
        {
            if (memcmp(pSrc->pHDRParams, &pHdrState->HDRLastFrameSourceParams[i], sizeof(VPHAL_HDR_PARAMS)))
            {
                *pdwUpdateMask |= (1 << i);
                pHdrState->HDRLastFrameSourceParams[i] = *pSrc->pHDRParams;
            }
        }
        else
        {
            MOS_ZeroMemory(&pHdrState->HDRLastFrameSourceParams[i], sizeof(VPHAL_HDR_PARAMS));
        }

        pHdrState->LUTMode[i]          = CurrentLUTMode;
        pHdrState->EOTFGamma[i]        = CurrentEOTF;
        pHdrState->OETFGamma[i]        = CurrentOETF;
        pHdrState->CCM[i]              = CurrentCCM;
        pHdrState->CCMExt1[i]          = CurrentCCMExt1;
        pHdrState->CCMExt2[i]          = CurrentCCMExt2;
        pHdrState->HdrMode[i]          = CurrentHdrMode;
        pHdrState->PriorCSC[i]         = CurrentPriorCSC;
        pHdrState->PostCSC[i]          = CurrentPostCSC;
        pHdrState->StageEnableFlags[i] = StageEnables;
    }

    if (pTarget->pHDRParams)
    {
        if (memcmp(pTarget->pHDRParams, &pHdrState->HDRLastFrameTargetParams, sizeof(VPHAL_HDR_PARAMS)))
        {
            *pdwUpdateMask |= (1 << VPHAL_MAX_HDR_INPUT_LAYER);
            pHdrState->HDRLastFrameTargetParams = *pTarget->pHDRParams;
        }
    }
    else
    {
        MOS_ZeroMemory(&pHdrState->HDRLastFrameTargetParams, sizeof(VPHAL_HDR_PARAMS));
    }
    pHdrState->dwUpdateMask = *pdwUpdateMask;
    eStatus = MOS_STATUS_SUCCESS;

finish:
    return eStatus;
}

//!
//! \brief    Checks to see if HDR is needed and supported
//! \details  Checks to see if HDR is needed and supported
//! \param    pRenderer
//            [in] Pointer to VphalRenderer
//! \param    pBeNeeded
//!           [out] 1 Needed 0 not Needed
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VpHal_HdrIsNeeded(
    VphalRenderer         *pRenderer,
    bool*                  pBeNeeded)
{
    MOS_STATUS                  eStatus;
    eStatus = MOS_STATUS_SUCCESS;

    VPHAL_PUBLIC_CHK_NULL(pRenderer);
    VPHAL_PUBLIC_CHK_NULL(pBeNeeded);

    // Check whether Hdr is supported by platform
    if (!MEDIA_IS_SKU(pRenderer->GetSkuTable(), FtrHDR) ||
         pRenderer->pHdrState->bDisableRender)
    {
        *pBeNeeded = false;
        VPHAL_RENDER_ASSERTMESSAGE("Hdr not enabled or disabled for this platform.");
        goto finish;
    }

    *pBeNeeded = true;

finish:
    return eStatus;
}

//!
//! \brief    Set up HDR Render Data
//! \details  Set up HDR render data, including kernel information, input surface's block size
//! \param    PVPHAL_HDR_STATE pHdrState
//!           [in] Pointer to HDR state
//! \param    PVPHAL_HDR_RENDER_DATA pRenderData
//!           [out] Pointer to HDR render data
//! \param    int32_t iKUID
//!           [in] Kernel unique ID
//! \param    int32_t iKDTIndex
//            [in] KDT index.
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VpHal_HdrSetupRenderData(
    PVPHAL_HDR_STATE        pHdrState,
    PVPHAL_HDR_RENDER_DATA  pRenderData,
    int32_t                 iKUID,
    int32_t                 iKDTIndex)
{
    int32_t                         iBlockWd;                                       // Block width
    int32_t                         iBlockHt;                                       // Block Height
    MOS_STATUS                      eStatus;                                        // Return code
    PRENDERHAL_INTERFACE            pRenderHal;
    Kdll_CacheEntry                 *pCacheEntryTable;                              // Kernel Cache Entry table
    PVPHAL_SURFACE                  pSrcSurface;
    uint32_t                        dwSrcWidth;
    uint32_t                        dwSrcHeight;
    uint32_t                        i;

    VPHAL_RENDER_CHK_NULL(pHdrState);
    VPHAL_RENDER_CHK_NULL(pRenderData);
    VPHAL_RENDER_CHK_NULL(pHdrState->pRenderHal);

    MOS_ZeroMemory(pRenderData, sizeof(VPHAL_HDR_RENDER_DATA));

    // Initialize Variables
    eStatus = MOS_STATUS_SUCCESS;

    if (iKDTIndex == KERNEL_HDR_MANDATORY_G9)
    {
        for (i = 0; i < pHdrState->uSourceCount; i++)
        {
            if (pHdrState->pSrcSurf[i])
            {
                if (pHdrState->pSrcSurf[i]->SurfType == SURF_IN_PRIMARY)
                {
                    if (pHdrState->pSrcSurf[i]->pIEFParams)
                    {
                        pRenderData->pIEFParams = pHdrState->pSrcSurf[i]->pIEFParams;
                    }


                    if (pHdrState->pSrcSurf[i]->Rotation == VPHAL_ROTATION_IDENTITY ||
                        pHdrState->pSrcSurf[i]->Rotation == VPHAL_ROTATION_180 ||
                        pHdrState->pSrcSurf[i]->Rotation == VPHAL_MIRROR_HORIZONTAL ||
                        pHdrState->pSrcSurf[i]->Rotation == VPHAL_MIRROR_VERTICAL)
                    {
                        pRenderData->fPrimaryLayerScaleX = (float)(pHdrState->pSrcSurf[i]->rcDst.right - pHdrState->pSrcSurf[i]->rcDst.left) /
                                                           (float)(pHdrState->pSrcSurf[i]->rcSrc.right - pHdrState->pSrcSurf[i]->rcSrc.left);
                        pRenderData->fPrimaryLayerScaleY = (float)(pHdrState->pSrcSurf[i]->rcDst.bottom - pHdrState->pSrcSurf[i]->rcDst.top) /
                                                           (float)(pHdrState->pSrcSurf[i]->rcSrc.bottom - pHdrState->pSrcSurf[i]->rcSrc.top);
                    }
                    else
                    {
                        // VPHAL_ROTATION_90 || VPHAL_ROTATION_270 || 
                        // VPHAL_ROTATE_90_MIRROR_HORIZONTAL || VPHAL_ROTATE_90_MIRROR_VERTICAL
                        pRenderData->fPrimaryLayerScaleX = (float)(pHdrState->pSrcSurf[i]->rcDst.right - pHdrState->pSrcSurf[i]->rcDst.left) /
                                                           (float)(pHdrState->pSrcSurf[i]->rcSrc.bottom - pHdrState->pSrcSurf[i]->rcSrc.top);
                        pRenderData->fPrimaryLayerScaleY = (float)(pHdrState->pSrcSurf[i]->rcDst.bottom - pHdrState->pSrcSurf[i]->rcDst.top) /
                                                           (float)(pHdrState->pSrcSurf[i]->rcSrc.right - pHdrState->pSrcSurf[i]->rcSrc.left);
                    }

                    pRenderData->PrimaryLayerFormat = pHdrState->pSrcSurf[i]->Format;
                }
            }
        }

        // Store pointer to Kernel Parameter
        pRenderData->pKernelParam[iKDTIndex] = &pHdrState->pKernelParamTable[iKDTIndex];
        pCacheEntryTable                     = pHdrState->pKernelCache->pCacheEntries;

        VPHAL_RENDER_CHK_NULL(pCacheEntryTable);

        // Set Parameters for Kernel Entry
        MOS_ZeroMemory(&pRenderData->KernelEntry[iKDTIndex], sizeof(Kdll_CacheEntry));

        // Set the curbe length
        pRenderData->iCurbeLength = (pRenderData->pKernelParam[iKDTIndex]->CURBE_Length) * GRF_SIZE;

        // Set Parameters for Kernel Entry
        pRenderData->KernelEntry[iKDTIndex].iKUID   = iKUID;
        pRenderData->KernelEntry[iKDTIndex].iKCID   = -1;
        pRenderData->KernelEntry[iKDTIndex].iSize   = pCacheEntryTable[iKUID].iSize;
        pRenderData->KernelEntry[iKDTIndex].pBinary = pCacheEntryTable[iKUID].pBinary;
        pRenderData->KernelEntry[iKDTIndex].szName  = pCacheEntryTable[iKUID].szName;

        pRenderData->PerfTag  = (VPHAL_PERFTAG)(VPHAL_HDR_GENERIC + pHdrState->uSourceCount);

        // Get per block resulution
        iBlockWd = pRenderData->pKernelParam[iKDTIndex]->block_width;
        iBlockHt = pRenderData->pKernelParam[iKDTIndex]->block_height;

        // Calcualte block numbers to process
        dwSrcWidth            = pHdrState->pTargetSurf[0]->rcDst.right - pHdrState->pTargetSurf[0]->rcDst.left;
        dwSrcHeight           = pHdrState->pTargetSurf[0]->rcDst.bottom - pHdrState->pTargetSurf[0]->rcDst.top;
        pRenderData->iBlocksX = (dwSrcWidth + iBlockWd - 1) / iBlockWd;
        pRenderData->iBlocksY = (dwSrcHeight + iBlockHt -1) / iBlockHt;

        // Set up Scoreboard parameters
        pRenderData->ScoreboardParams.ScoreboardMask = 0;
        pRenderData->ScoreboardParams.ScoreboardType = 1;

        // Set up AVS parameters
        pRenderData->pAVSParameters[0] = &pHdrState->AVSParameters[0];
        pRenderData->pAVSParameters[1] = &pHdrState->AVSParameters[1];
    }
    else if (iKDTIndex == KERNEL_HDR_PREPROCESS)
    {
        // Store pointer to Kernel Parameter
        pRenderData->pKernelParam[iKDTIndex] = &pHdrState->pKernelParamTable[iKDTIndex];
        pCacheEntryTable = pHdrState->pKernelCache->pCacheEntries;

        VPHAL_RENDER_CHK_NULL(pCacheEntryTable);

        // Set Parameters for Kernel Entry
        MOS_ZeroMemory(&pRenderData->KernelEntry[iKDTIndex], sizeof(Kdll_CacheEntry));

        // Set the curbe length
        pRenderData->iCurbeLength = (pRenderData->pKernelParam[iKDTIndex]->CURBE_Length) * GRF_SIZE;

        // Set Parameters for Kernel Entry
        pRenderData->KernelEntry[iKDTIndex].iKUID = iKUID;
        pRenderData->KernelEntry[iKDTIndex].iKCID = -1;
        pRenderData->KernelEntry[iKDTIndex].iSize = pCacheEntryTable[iKUID].iSize;
        pRenderData->KernelEntry[iKDTIndex].pBinary = pCacheEntryTable[iKUID].pBinary;
        pRenderData->KernelEntry[iKDTIndex].szName = pCacheEntryTable[iKUID].szName;

        // Set up Scoreboard parameters
        pRenderData->ScoreboardParams.ScoreboardMask = 0;
        pRenderData->ScoreboardParams.ScoreboardType = 1;
    }
    else
    {
        VPHAL_RENDER_ASSERTMESSAGE("Unknown HDR kernel");
        eStatus = MOS_STATUS_UNKNOWN;
        goto finish;
    }

finish:
    return eStatus;
}

//!
//! \brief    HDR HW States Setup
//! \details  Setup HW states for HDR
//! \param    PVPHAL_HDR_STATE pHdrState
//!           [in] Pointer to the HDR State
//! \param    PVPHAL_HDR_RENDER_DATA pRenderData
//!           [in,out] Pointer to HDR render data
//! \param    PVPHAL_HDR_RENDER_DATA pRenderData
//!           [in,out] Pointer to HDR render data
//! \param    uint32_t HDRKernelID
//!           [in] HDR Kernel ID
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VpHal_HdrSetupHwStates(
    PVPHAL_HDR_STATE            pHdrState,
    PVPHAL_HDR_RENDER_DATA      pRenderData,
    uint32_t                    HDRKernelID)
{
    PRENDERHAL_INTERFACE        pRenderHal           = nullptr;
    int32_t                     iKrnAllocation       = 0;
    int32_t                     iCurbeOffset         = 0;
    MOS_STATUS                  eStatus              = MOS_STATUS_SUCCESS;
    MHW_KERNEL_PARAM            MhwKernelParam       = {};
    PMOS_INTERFACE              pOsInterface         = nullptr;

    VPHAL_RENDER_CHK_NULL(pHdrState);
    VPHAL_RENDER_CHK_NULL(pRenderData);
    VPHAL_RENDER_CHK_NULL(pHdrState->pOsInterface);
    
    pRenderHal   = pHdrState->pRenderHal;
    pOsInterface = pHdrState->pOsInterface;
    VPHAL_RENDER_CHK_NULL(pRenderHal);

    //----------------------------------
    // Allocate and reset media state
    //----------------------------------
    pRenderData->pMediaState = pRenderHal->pfnAssignMediaState(pRenderHal, (RENDERHAL_COMPONENT)RENDERHAL_COMPONENT_HDR);
    MOS_OS_CHK_NULL(pRenderData->pMediaState);

    //----------------------------------
    // Allocate and reset SSH instance
    //----------------------------------
    VPHAL_RENDER_CHK_STATUS(pRenderHal->pfnAssignSshInstance(pRenderHal));

    //----------------------------------
    // Assign and Reset Binding Table
    //----------------------------------
    VPHAL_RENDER_CHK_STATUS(pRenderHal->pfnAssignBindingTable(
        pRenderHal, 
        &pRenderData->iBindingTable));

    //----------------------------------
    // Setup Surface states
    //----------------------------------
    VPHAL_RENDER_CHK_STATUS(pHdrState->pfnSetupSurfaceStates(
        pHdrState,
        pRenderData));

    //----------------------------------
    // Load Static data
    //----------------------------------
    VPHAL_RENDER_CHK_STATUS(pHdrState->pfnLoadStaticData(
        pHdrState,
        pRenderData,
        &iCurbeOffset));

    //----------------------------------
    // Setup VFE State params. Each Renderer MUST call pfnSetVfeStateParams().
    // See comment in VpHal_HwSetVfeStateParams() for details.
    //----------------------------------
    pRenderHal->pfnSetVfeStateParams(
        pRenderHal,
        MEDIASTATE_DEBUG_COUNTER_FREE_RUNNING,
        pRenderData->pKernelParam[HDRKernelID]->Thread_Count,
        pRenderData->iCurbeLength,
        0,
        nullptr);

    //----------------------------------
    // Load kernel to GSH
    //----------------------------------
    INIT_MHW_KERNEL_PARAM(MhwKernelParam, &pRenderData->KernelEntry[HDRKernelID]);    

    iKrnAllocation = pRenderHal->pfnLoadKernel(
        pRenderHal,
        pRenderData->pKernelParam[HDRKernelID],
        &MhwKernelParam,
        nullptr);

    if (iKrnAllocation < 0)
    {
        VPHAL_RENDER_ASSERTMESSAGE("HDR Load kernel to GSH failed");
        eStatus = MOS_STATUS_UNKNOWN;
        goto finish;
    }

    //----------------------------------
    // Allocate Media ID, link to kernel
    //----------------------------------
    pRenderData->iMediaID = pRenderHal->pfnAllocateMediaID(
        pRenderHal,
        iKrnAllocation,
        pRenderData->iBindingTable,
        iCurbeOffset,
        pRenderData->iCurbeLength,
        0,
        nullptr);

    if (pRenderData->iMediaID < 0) 
    {
        VPHAL_RENDER_ASSERTMESSAGE("HDR Allocate Media ID failed");
        eStatus = MOS_STATUS_UNKNOWN;
        goto finish;
    }

    //----------------------------------
    // Setup Sampler states
    //----------------------------------
    if (HDRKernelID != KERNEL_HDR_PREPROCESS)
    {
        VPHAL_RENDER_CHK_STATUS(pHdrState->pfnSetSamplerStates(
            pHdrState,
            pRenderData));
    }

finish:
    VPHAL_RENDER_ASSERT(eStatus == MOS_STATUS_SUCCESS);
    return eStatus;
}

//!
//! \brief    Setup media walker command for HDR
//! \details  Setup media walker command for HDR
//! \param    PVPHAL_HDR_STATE pHdrState
//!           [in] Pointer to HDR state
//! \param    PVPHAL_HDR_RENDER_DATA pRenderData
//!           [in] Pointer to render data
//! \param    PMHW_WALKER_PARAMS pWalkerParams
//!           [out] Pointer to media walker parameters
//! \param    int32_t iKDTIndex
//            [in] KDT index.
//! \param    uint32_t uiPortionIndex
//            [in] Frame split portion index.
//! \return   MOS_STATUS
//!
MOS_STATUS VpHal_PreprocessHdrSetupWalkerObject(
    PVPHAL_HDR_STATE            pHdrState,
    PVPHAL_HDR_RENDER_DATA      pRenderData,
    PMHW_WALKER_PARAMS          pWalkerParams,
    int32_t                     iKDTIndex,
    uint32_t                    uiPortionIndex)
{
    MOS_STATUS  eStatus       = MOS_STATUS_SUCCESS;
    uint32_t threadswidth     = 1;
    uint32_t threadsheight    = VPHAL_MAX_HDR_INPUT_LAYER;

    VPHAL_RENDER_CHK_NULL(pHdrState);
    VPHAL_RENDER_CHK_NULL(pRenderData);
    VPHAL_RENDER_CHK_NULL(pWalkerParams);

    // Setup Media Walker cmd. Raster scan with no dependency
    MOS_ZeroMemory(pWalkerParams, sizeof(MHW_WALKER_PARAMS));
    pWalkerParams->InterfaceDescriptorOffset = pRenderData->iMediaID;
    pWalkerParams->dwGlobalLoopExecCount = 1;
    pWalkerParams->dwLocalLoopExecCount  = 1;
    pWalkerParams->BlockResolution.x  = threadswidth;
    pWalkerParams->BlockResolution.y  = threadsheight;
    pWalkerParams->GlobalResolution.x = threadswidth;
    pWalkerParams->GlobalResolution.y = threadsheight;


    pWalkerParams->GlobalStart.x = 0;
    pWalkerParams->GlobalStart.y = 0;
    pWalkerParams->GlobalOutlerLoopStride.x = threadswidth;
    pWalkerParams->GlobalOutlerLoopStride.y = 0;
    pWalkerParams->GlobalInnerLoopUnit.x = 0;
    pWalkerParams->GlobalInnerLoopUnit.y = threadsheight;
    pWalkerParams->LocalStart.x = 0;
    pWalkerParams->LocalStart.y = 0;
    pWalkerParams->LocalOutLoopStride.x = 1;
    pWalkerParams->LocalOutLoopStride.y = 0;
    pWalkerParams->LocalInnerLoopUnit.x = 0;
    pWalkerParams->LocalInnerLoopUnit.y = 1;
    pWalkerParams->LocalEnd.x = 0;
    pWalkerParams->LocalEnd.y = threadsheight - 1;

    eStatus = MOS_STATUS_SUCCESS;

finish:
    return eStatus;
}

//!
//! \brief    Render GpGpu Walker Buffer
//! \details  Render GpGpu Walker Buffer, fill Walker static data fields and set walker
//!           cmd params
//! \param    [in] pHdrState
//!           Pointer to HdrState
//! \param    [in] pRenderingData
//!           Pointer to Rendering Data
//! \param    [in] pWalkerParams
//!           Pointer to Walker parameters
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise false
//!
MOS_STATUS Vphal_HdrSetupComputeWalker(
    PVPHAL_HDR_STATE                pHdrState,
    PVPHAL_HDR_RENDER_DATA          pRenderData,
    PMHW_GPGPU_WALKER_PARAMS        pWalkerParams)
{
    MOS_STATUS                          eStatus                 = MOS_STATUS_UNINITIALIZED;
    PRENDERHAL_INTERFACE                pRenderHal              = nullptr;
    PVPHAL_BB_COMP_ARGS                 pBbArgs                 = nullptr;
    bool                                bResult                 = false;
    int32_t                             iLayers                 = 0;
    uint32_t                            uiMediaWalkerBlockSize  = 0;
    uint32_t*                           pdwDestXYTopLeft        = nullptr;
    uint32_t*                           pdwDestXYBottomRight    = nullptr;
    RECT                                AlignedRect             = {};
    bool                                bVerticalPattern        = false;

    VPHAL_RENDER_CHK_NULL(pHdrState);
    VPHAL_RENDER_CHK_NULL(pHdrState->pTargetSurf[0]);
    VPHAL_RENDER_CHK_NULL(pHdrState->pSrcSurf[0]);
    VPHAL_RENDER_CHK_NULL(pRenderData);
    VPHAL_RENDER_CHK_NULL(pRenderHal);
    VPHAL_RENDER_CHK_NULL(pWalkerParams);

    pRenderHal              = pHdrState->pRenderHal;
    bVerticalPattern        = false;
    AlignedRect             = pHdrState->pTargetSurf[0]->rcDst;

    // Get media walker kernel block size
    uiMediaWalkerBlockSize  = pRenderHal->pHwSizes->dwSizeMediaWalkerBlock;

    // Calculate aligned output area in order to determine the total # blocks
    // to process in case of non-16x16 aligned target.
    AlignedRect.right       += uiMediaWalkerBlockSize - 1;
    AlignedRect.bottom      += uiMediaWalkerBlockSize - 1;
    AlignedRect.left        -= AlignedRect.left   % uiMediaWalkerBlockSize;
    AlignedRect.top         -= AlignedRect.top    % uiMediaWalkerBlockSize;
    AlignedRect.right       -= AlignedRect.right  % uiMediaWalkerBlockSize;
    AlignedRect.bottom      -= AlignedRect.bottom % uiMediaWalkerBlockSize;

    // Set walker cmd params - Rasterscan
    pWalkerParams->InterfaceDescriptorOffset = pRenderData->iMediaID;

    if (pHdrState->uSourceCount == 1 &&
        pHdrState->pSrcSurf[0]->TileType == MOS_TILE_LINEAR &&
        (pHdrState->pSrcSurf[0]->Rotation == VPHAL_ROTATION_90 || pHdrState->pSrcSurf[0]->Rotation == VPHAL_ROTATION_270))
    {
        pWalkerParams->GroupStartingX       = (AlignedRect.top / uiMediaWalkerBlockSize);
        pWalkerParams->GroupStartingY       = (AlignedRect.left / uiMediaWalkerBlockSize);
        pWalkerParams->GroupWidth           = pRenderData->iBlocksY;
        pWalkerParams->GroupHeight          = pRenderData->iBlocksX;
    }
    else
    {
        pWalkerParams->GroupStartingX       = (AlignedRect.left / uiMediaWalkerBlockSize);
        pWalkerParams->GroupStartingY       = (AlignedRect.top / uiMediaWalkerBlockSize);
        pWalkerParams->GroupWidth           = pRenderData->iBlocksX;
        pWalkerParams->GroupHeight          = pRenderData->iBlocksY;
    }

    pWalkerParams->ThreadWidth              = 1;
    pWalkerParams->ThreadHeight             = 1;
    pWalkerParams->ThreadDepth              = 1;
    pWalkerParams->IndirectDataStartAddress = pRenderData->iCurbeOffset;
    // Indirect Data Length is a multiple of 64 bytes (size of L3 cacheline). Bits [5:0] are zero.
    pWalkerParams->IndirectDataLength       = MOS_ALIGN_CEIL(pRenderData->iCurbeLength, 1 << MHW_COMPUTE_INDIRECT_SHIFT);
    pWalkerParams->BindingTableID           = pRenderData->iBindingTable;

    eStatus = MOS_STATUS_SUCCESS;

finish:
    return eStatus;
}

//!
//! \brief    HDR render
//! \details  Launch HDR kernel to render output picture
//! \param    PVPHAL_HDR_STATE pHdrState
//!           [in] Poniter to HDR state
//! \param    PVPHAL_RENDER_PARAMS pRenderParams
//!           [in,out] Pointer to Render parameters
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VpHal_HdrRender(
    PVPHAL_HDR_STATE             pHdrState,
    PVPHAL_RENDER_PARAMS         pRenderParams)
{
    PRENDERHAL_INTERFACE            pRenderHal              = nullptr;
    PMOS_INTERFACE                  pOsInterface            = nullptr;
    MHW_WALKER_PARAMS               WalkerParams            = {};
    PMHW_WALKER_PARAMS              pWalkerParams           = nullptr;
    MHW_GPGPU_WALKER_PARAMS         ComputeWalkerParams     = {};
    PMHW_GPGPU_WALKER_PARAMS        pComputeWalkerParams    = nullptr;
    int32_t                         iKUID                   = 0;
    int32_t                         iKDTIndex               = 0;
    uint32_t                        i                       = 0;
    bool                            bSupported              = true;
    MOS_STATUS                      eStatus                 = MOS_STATUS_UNKNOWN;
    uint32_t                        HdrKernel               = 0;
    VPHAL_HDR_RENDER_DATA           RenderData              = {};
    MOS_GPU_CONTEXT                 RenderGpuContext        = MOS_GPU_CONTEXT_RENDER;
    bool                            bLastSummit             = true;

    VPHAL_RENDER_FUNCTION_ENTER;

    VPHAL_RENDER_CHK_NULL(pHdrState);
    VPHAL_RENDER_CHK_NULL(pRenderParams);
    VPHAL_RENDER_CHK_NULL(pHdrState->pRenderHal);
    VPHAL_RENDER_CHK_NULL(pHdrState->pOsInterface);

    // Initialize Variables
    pRenderHal                = pHdrState->pRenderHal;
    pOsInterface              = pHdrState->pOsInterface;
    pWalkerParams             = &WalkerParams;
    pHdrState->uSourceCount   = 0;
    pHdrState->uTargetCount   = 0;
    pRenderParams             = (VPHAL_RENDER_PARAMS*)pRenderParams;

    RenderGpuContext          = pOsInterface ? (pOsInterface->CurrentGpuContextOrdinal) : MOS_GPU_CONTEXT_RENDER;
    pComputeWalkerParams      = nullptr;
    MOS_ZeroMemory(&ComputeWalkerParams, sizeof(ComputeWalkerParams));

    if (pRenderParams->uSrcCount > VPHAL_MAX_HDR_INPUT_LAYER)
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        goto finish;
    }
    if (pRenderParams->uDstCount > VPHAL_MAX_HDR_OUTPUT_LAYER)
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        goto finish;
    }

    HdrKernel = KERNEL_HDR_MANDATORY;
    
    for (i = 0; i < pRenderParams->uSrcCount; i++)
    {
        if (pRenderParams->pSrc[i] == nullptr)
        {
            continue;
        }

        VPHAL_RENDER_CHK_STATUS(pHdrState->pfnIsInputFormatSupported(pRenderParams->pSrc[i], &bSupported));

        if (!bSupported)
        {
            eStatus = MOS_STATUS_INVALID_PARAMETER;
            goto finish;
        }

        pHdrState->pSrcSurf[i] = pRenderParams->pSrc[i];
        pHdrState->uSourceCount++;

        // Ensure the input is ready to be read
        pOsInterface->pfnSyncOnResource(
            pOsInterface, 
            &pHdrState->pSrcSurf[i]->OsResource,
            RenderGpuContext,
            false);
    }

    for (i = 0; i < pRenderParams->uDstCount; i++)
    {
        if (pRenderParams->pTarget[i] == nullptr)
        {
            continue;
        }

        VPHAL_RENDER_CHK_STATUS(pHdrState->pfnIsOutputFormatSupported(pRenderParams->pTarget[i], &bSupported));

        if (!bSupported)
        {
            eStatus = MOS_STATUS_INVALID_PARAMETER;
            goto finish;
        }


        pHdrState->pTargetSurf[i] = pRenderParams->pTarget[i];
        pHdrState->uTargetCount++;

        // Ensure the output is ready to be written
        pOsInterface->pfnSyncOnResource(
            pOsInterface, 
            &pHdrState->pTargetSurf[i]->OsResource,
            RenderGpuContext,
            true);

        // Sync Render Target with Overlay Context
        if (pHdrState->pTargetSurf[i]->bOverlay)
        {
            pOsInterface->pfnSyncOnOverlayResource(
                pOsInterface,
                &pHdrState->pTargetSurf[i]->OsResource,
                RenderGpuContext);
        }
    }

    pHdrState->pColorFillParams = pRenderParams->pColorFillParams;
    // Allocate resources needed by Hdr
    VPHAL_RENDER_CHK_STATUS(pHdrState->pfnAllocateResources(pHdrState));
    VPHAL_RENDER_CHK_STATUS(VpHal_HdrPreprocess(pHdrState, pRenderParams));    

    for (i = 0; i < pHdrState->uiSplitFramePortions; i++)
    {
        // Reset states before rendering
        // (clear allocations, get GSH allocation index + any additional housekeeping)
        pOsInterface->pfnResetOsStates(pOsInterface);
        VPHAL_RENDER_CHK_STATUS(pRenderHal->pfnReset(pRenderHal));

        // Get the Kernel Parameter (Platform Specific)
        VPHAL_RENDER_CHK_STATUS(pHdrState->pfnGetKernelParam(
            HdrKernel,
            &iKUID,
            &iKDTIndex));

        // Setup Hdr render data
        VPHAL_RENDER_CHK_STATUS(VpHal_HdrSetupRenderData(
            pHdrState,
            &RenderData,
            iKUID,
            iKDTIndex));

        // Set up HW States and Commands
        VPHAL_RENDER_CHK_STATUS(VpHal_HdrSetupHwStates(pHdrState, &RenderData, iKDTIndex));

        // Set Perf Tag
        pOsInterface->pfnResetPerfBufferID(pOsInterface);
        pOsInterface->pfnSetPerfTag(pOsInterface, RenderData.PerfTag);

        if (pHdrState->bFtrComputeWalker)
        {
            // Setup Compute Walker
            pWalkerParams = nullptr;
            pComputeWalkerParams = &ComputeWalkerParams;

            VPHAL_RENDER_CHK_STATUS(Vphal_HdrSetupComputeWalker(
                pHdrState,
                &RenderData,
                &ComputeWalkerParams));
        }
        else
        {
            // Setup Media Walker Object
            VpHal_HdrSetupWalkerObject(
                pHdrState,
                &RenderData,
                &WalkerParams,
                iKDTIndex,
                i);
        }

        bLastSummit = (i == (pHdrState->uiSplitFramePortions - 1) ? true : false);
        // Submit all media states to HW
        VPHAL_RENDER_CHK_STATUS(VpHal_RndrSubmitCommands(
            pRenderHal,
            nullptr,
            pHdrState->bNullHwRenderHdr,
            pWalkerParams,
            pComputeWalkerParams,
            &pHdrState->StatusTableUpdateParams,
            (VpKernelID)kernelHdrMandatory,
            0,
            nullptr,
            bLastSummit));
    }

    eStatus = MOS_STATUS_SUCCESS;

finish:
    VPHAL_RENDER_EXITMESSAGE("eStatus %d", eStatus);
    return eStatus;
}

//!
//! \brief    Hdr init renderer interface
//! \details  Initializes the Hdr interface
//! \param    PVPHAL_HDR_STATE pHdrState
//!           [in] Pointer to Hdr state
//! \param    PRENDERHAL_INTERFACE pRenderHal
//!           [in] Pointer to RENDERHAL interface
//! \return   void
//!
MOS_STATUS VpHal_HdrInitInterface(
    PVPHAL_HDR_STATE          pHdrState,
    PRENDERHAL_INTERFACE      pRenderHal)
{
    MOS_STATUS                  eStatus         = MOS_STATUS_SUCCESS;
    PMOS_INTERFACE              pOsInterface    = nullptr;

    VPHAL_PUBLIC_CHK_NULL(pHdrState);
    VPHAL_PUBLIC_CHK_NULL(pRenderHal);

    MOS_ZeroMemory(pHdrState, sizeof(VPHAL_HDR_STATE));

    pOsInterface = pRenderHal->pOsInterface;

    VPHAL_PUBLIC_CHK_NULL(pOsInterface);

    // Set interface to OS and HW interfaces
    pHdrState->pRenderHal                 = pRenderHal;
    pHdrState->pOsInterface               = pOsInterface;
    pHdrState->pSkuTable                  = pOsInterface->pfnGetSkuTable(pOsInterface);

    // Setup Function Pointers
    pHdrState->pfnInitialize              = VpHal_HdrInitialize;
    pHdrState->pfnDestroy                 = VpHal_HdrDestroy;
    pHdrState->pfnRender                  = VpHal_HdrRender;
    pHdrState->pfnIsNeeded                = VpHal_HdrIsNeeded;

    eStatus = MOS_STATUS_SUCCESS;

finish:
    return eStatus;
}

//! \brief    Perform Rendering HDR step
//! \details  Check whether HDR is needed. When it's needed, perform HDR
//!           operation
//! \param    [in,out] pRenderer
//!           VPHAL renderer pointer
//! \param    [in,out] pRenderParams
//!           Pointer to VPHAL render parameter
//! \param    [in,out] pRenderPassData
//!           Pointer to the VPHAL render pass data
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VpHal_RndrRenderHDR(
    VphalRenderer           *pRenderer,
    PVPHAL_RENDER_PARAMS    pRenderParams,
    RenderpassData          *pRenderPassData)
{
    PRENDERHAL_INTERFACE    *pRenderHal = nullptr;
    MOS_STATUS              eStatus     = MOS_STATUS_SUCCESS;
    bool                    bEnabled    = false;

    VPHAL_RENDER_FUNCTION_ENTER;

    VPHAL_RENDER_CHK_NULL(pRenderer);
    VPHAL_RENDER_CHK_NULL(pRenderParams);
    VPHAL_RENDER_CHK_NULL(pRenderPassData);
    VPHAL_RENDER_CHK_NULL(pRenderer->pHdrState);

    pRenderHal = &pRenderer->pHdrState->pRenderHal;
    VPHAL_RENDER_CHK_NULL(pRenderHal);

    // Disable bEnableP010SinglePass for HDR path, to avoid AVS sampler and 1 planes 3D sampler path in kernel.
    // Kernel solution only support 2 planes rendering of 3D sampler.
    if ((*pRenderHal)->bEnableP010SinglePass)
    {
        bEnabled = true;
        (*pRenderHal)->bEnableP010SinglePass = false;
    }
    eStatus = pRenderer->pHdrState->pfnRender(pRenderer->pHdrState, pRenderParams);

    if (bEnabled)
       (*pRenderHal)->bEnableP010SinglePass = true;

finish:
    VPHAL_RENDER_EXITMESSAGE("eStatus %d", eStatus);
    return eStatus;
}

//!
//! \brief    Check if HDR path is needed
//! \details  Check if HDR path is needed
//! \param    [in] pRenderer
//!           VPHAL renderer pointer
//! \param    [in] pRenderParams
//!           Pointer to VPHAL render parameter
//! \param    [in] pRenderPassData
//!           Pointer to VPHAL render pass data
//! \return   bool
//!
bool VpHal_RndrIsHdrPathNeeded(
    VphalRenderer           *pRenderer,
    PVPHAL_RENDER_PARAMS    pRenderParams,
    RenderpassData          *pRenderPassData)
{
    if (!pRenderer || !pRenderParams || !pRenderPassData)
    {
        return false;
    }

    if (pRenderPassData->bHdrNeeded && pRenderer->pHdrState)
    {
        // Hdr kernel render will be disabled for 1 layer H2H bypass case for PnP optimization
        if (!pRenderer->pHdrState->bBypassHdrKernelPath)
        {
            return true;
        }
    }

    return false;
}

//!
//! \brief    Setup media walker command for HDR
//! \details  Setup media walker command for HDR
//! \param    PVPHAL_HDR_STATE pHdrState
//!           [in] Pointer to HDR state
//! \param    PVPHAL_HDR_RENDER_DATA pRenderData
//!           [in] Pointer to render data
//! \param    PMHW_WALKER_PARAMS pWalkerParams
//!           [out] Pointer to media walker parameters
//! \param    int32_t iKDTIndex
//            [in] KDT index.
//! \param    uint32_t uiPortionIndex
//            [in] Frame split portion index.
//! \return   MOS_STATUS
//!
MOS_STATUS VpHal_HdrSetupWalkerObject(
    PVPHAL_HDR_STATE            pHdrState,
    PVPHAL_HDR_RENDER_DATA      pRenderData,
    PMHW_WALKER_PARAMS          pWalkerParams,
    int32_t                     iKDTIndex,
    uint32_t                    uiPortionIndex)
{
    MOS_STATUS                  eStatus        =  MOS_STATUS_SUCCESS;
    RECT                        AlignedRect    = {};
    int32_t                     iBlockWd       = 0;                                       // Block Width
    int32_t                     iBlockHt       = 0;                                       // Block Height

    VPHAL_RENDER_CHK_NULL(pHdrState);
    VPHAL_RENDER_CHK_NULL(pHdrState->pTargetSurf[0]);
    VPHAL_RENDER_CHK_NULL(pRenderData);
    VPHAL_RENDER_CHK_NULL(pWalkerParams);

    AlignedRect = pHdrState->pTargetSurf[0]->rcDst;
    iBlockWd = pRenderData->pKernelParam[iKDTIndex]->block_width;
    iBlockHt = pRenderData->pKernelParam[iKDTIndex]->block_height;

    AlignedRect.right += iBlockWd - 1;
    AlignedRect.bottom += iBlockHt - 1;
    AlignedRect.left -= AlignedRect.left   % iBlockWd;
    AlignedRect.top -= AlignedRect.top    % iBlockHt;
    AlignedRect.right -= AlignedRect.right  % iBlockWd;
    AlignedRect.bottom -= AlignedRect.bottom % iBlockHt;

    // Setup Media Walker cmd. Raster scan with no dependency
    MOS_ZeroMemory(pWalkerParams, sizeof(MHW_WALKER_PARAMS));
    pWalkerParams->InterfaceDescriptorOffset = pRenderData->iMediaID;
    pWalkerParams->dwGlobalLoopExecCount = 1;
    pWalkerParams->dwLocalLoopExecCount = pRenderData->iBlocksX - 1;
    pWalkerParams->BlockResolution.x = pRenderData->iBlocksX;
    pWalkerParams->BlockResolution.y = pRenderData->iBlocksY;

    if (AlignedRect.left != 0 || AlignedRect.top != 0)
    {
        // if the rect starts from any other macro block other than the first
        // then the global resolution should be the whole frame and the global 
        // start should be the rect start.
        pWalkerParams->GlobalResolution.x =
            (AlignedRect.right / iBlockWd);
        pWalkerParams->GlobalResolution.y =
            (AlignedRect.bottom / iBlockHt);
    }
    else
    {
        pWalkerParams->GlobalResolution.x = pRenderData->iBlocksX;
        pWalkerParams->GlobalResolution.y = pRenderData->iBlocksY;
    }

    pWalkerParams->GlobalStart.x = (AlignedRect.left / iBlockWd);
    pWalkerParams->GlobalStart.y = (AlignedRect.top / iBlockHt);
    pWalkerParams->GlobalOutlerLoopStride.x = pRenderData->iBlocksX;
    pWalkerParams->GlobalOutlerLoopStride.y = 0;
    pWalkerParams->GlobalInnerLoopUnit.x = 0;
    pWalkerParams->GlobalInnerLoopUnit.y = pRenderData->iBlocksY;
    pWalkerParams->LocalStart.x = 0;
    pWalkerParams->LocalStart.y = 0;
    pWalkerParams->LocalOutLoopStride.x = 1;
    pWalkerParams->LocalOutLoopStride.y = 0;
    pWalkerParams->LocalInnerLoopUnit.x = 0;
    pWalkerParams->LocalInnerLoopUnit.y = 1;
    pWalkerParams->LocalEnd.x = 0;
    pWalkerParams->LocalEnd.y = pRenderData->iBlocksY - 1;

    if (pHdrState->uiSplitFramePortions > 1)
    {
        pWalkerParams->GlobalStart.x = MOS_MAX(MOS_ROUNDUP_DIVIDE(pWalkerParams->GlobalResolution.x, pHdrState->uiSplitFramePortions) * (uiPortionIndex),
            pWalkerParams->GlobalStart.x);
        pWalkerParams->GlobalResolution.x = MOS_MIN(MOS_ROUNDUP_DIVIDE(pWalkerParams->GlobalResolution.x, pHdrState->uiSplitFramePortions) * (uiPortionIndex + 1),
            pWalkerParams->GlobalResolution.x);
    }

    eStatus = MOS_STATUS_SUCCESS;

finish:
    return eStatus;
}

//!
//! \brief    HDR preprocess
//! \details  Launch HDR pre process kernel to render hdr coefficients surface
//! \param    PVPHAL_HDR_STATE pHdrState
//!           [in] Poniter to HDR state
//! \param    PVPHAL_RENDER_PARAMS pRenderParams
//!           [in,out] Pointer to Render parameters
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VpHal_HdrPreprocess(
    PVPHAL_HDR_STATE        pHdrState,
    PVPHAL_RENDER_PARAMS    pRenderParams)
{
    MOS_STATUS                    eStatus               = MOS_STATUS_UNKNOWN;
    PRENDERHAL_INTERFACE          pRenderHal            = nullptr;
    PMOS_INTERFACE                pOsInterface          = nullptr;
    uint32_t                      HdrKernel             = KERNEL_HDR_PREPROCESS;
    MOS_GPU_CONTEXT               RenderGpuContext      = MOS_GPU_CONTEXT_RENDER;
    int32_t                       iKUID                 = 0;
    int32_t                       iKDTIndex             = 0;
    VPHAL_HDR_RENDER_DATA         RenderData            = {};
    bool                          bLastSummit           = true;
    MHW_WALKER_PARAMS             WalkerParams          = {};
    PMHW_WALKER_PARAMS            pWalkerParams         = nullptr;
    MHW_GPGPU_WALKER_PARAMS       ComputeWalkerParams   = {};
    PMHW_GPGPU_WALKER_PARAMS      pComputeWalkerParams  = nullptr;
    uint32_t                      i                     = 0;
    int32_t                       iCurbeOffset          = 0;
    const uint32_t                HDRKernelID           = KERNEL_HDR_PREPROCESS;
    MHW_KERNEL_PARAM              MhwKernelParam        = {};
    int32_t                       iKrnAllocation        = 0;

    VPHAL_RENDER_FUNCTION_ENTER;

    VPHAL_RENDER_CHK_NULL(pHdrState);
    VPHAL_RENDER_CHK_NULL(pRenderParams);
    VPHAL_RENDER_CHK_NULL(pHdrState->pRenderHal);
    VPHAL_RENDER_CHK_NULL(pHdrState->pOsInterface);

    // HDR PreProcess Kernel is needed only if HDR metada is changed.
    if (!pHdrState->dwUpdateMask)
    {
        VPHAL_RENDER_EXITMESSAGE("pHdrState->dwUpdateMask is false, no need to update coefficients, exit with MOS_STATUS_SUCCESS!");
        return MOS_STATUS_SUCCESS;
    }

    // Initialize Variables
    pRenderHal          = pHdrState->pRenderHal;
    pOsInterface        = pHdrState->pOsInterface;
    RenderGpuContext    = pOsInterface->CurrentGpuContextOrdinal;

    // Reset states before rendering
    // (clear allocations, get GSH allocation index + any additional housekeeping)
    pOsInterface->pfnResetOsStates(pOsInterface);
    VPHAL_RENDER_CHK_STATUS(pRenderHal->pfnReset(pRenderHal));

    // Get the Kernel Parameter (Platform Specific)
    VPHAL_RENDER_CHK_STATUS(pHdrState->pfnGetKernelParam(
        HdrKernel,
        &iKUID,
        &iKDTIndex));

    // Setup Hdr render data
    VPHAL_RENDER_CHK_STATUS(VpHal_HdrSetupRenderData(
        pHdrState,
        &RenderData,
        iKUID,
        iKDTIndex));

    //----------------------------------
    // Allocate and reset media state
    //----------------------------------
    RenderData.pMediaState = pRenderHal->pfnAssignMediaState(pRenderHal, (RENDERHAL_COMPONENT)RENDERHAL_COMPONENT_HDR);
    MOS_OS_CHK_NULL(RenderData.pMediaState);

    //----------------------------------
    // Allocate and reset SSH instance
    //----------------------------------
    VPHAL_RENDER_CHK_STATUS(pRenderHal->pfnAssignSshInstance(pRenderHal));

    //----------------------------------
    // Assign and Reset Binding Table
    //----------------------------------
    VPHAL_RENDER_CHK_STATUS(pRenderHal->pfnAssignBindingTable(
        pRenderHal,
        &RenderData.iBindingTable));

    //----------------------------------
   // Setup Surface states
   //----------------------------------
    VPHAL_RENDER_CHK_STATUS(pHdrState->pfnSetupPreSurfaceStates(
        pHdrState,
        &RenderData));

    //----------------------------------
    // Load Static data
    //----------------------------------
    VPHAL_RENDER_CHK_STATUS(pHdrState->pfnLoadPreStaticData(
        pHdrState,
        &RenderData,
        &iCurbeOffset));

    //----------------------------------
    // Setup VFE State params. Each Renderer MUST call pfnSetVfeStateParams().
    // See comment in VpHal_HwSetVfeStateParams() for details.
    //----------------------------------
    pRenderHal->pfnSetVfeStateParams(
        pRenderHal,
        MEDIASTATE_DEBUG_COUNTER_FREE_RUNNING,
        RenderData.pKernelParam[HDRKernelID]->Thread_Count,
        RenderData.iCurbeLength,
        0,
        nullptr);

    //----------------------------------
    // Load kernel to GSH
    //----------------------------------
    INIT_MHW_KERNEL_PARAM(MhwKernelParam, &RenderData.KernelEntry[HDRKernelID]);

    iKrnAllocation = pRenderHal->pfnLoadKernel(
        pRenderHal,
        RenderData.pKernelParam[HDRKernelID],
        &MhwKernelParam,
        nullptr);

    if (iKrnAllocation < 0)
    {
        VPHAL_RENDER_ASSERTMESSAGE("HDR Load kernel to GSH failed");
        eStatus = MOS_STATUS_UNKNOWN;
        goto finish;
    }

    //----------------------------------
    // Allocate Media ID, link to kernel
    //----------------------------------
    RenderData.iMediaID = pRenderHal->pfnAllocateMediaID(
        pRenderHal,
        iKrnAllocation,
        RenderData.iBindingTable,
        iCurbeOffset,
        RenderData.iCurbeLength,
        0,
        nullptr);

    if (RenderData.iMediaID < 0)
    {
        VPHAL_RENDER_ASSERTMESSAGE("HDR Allocate Media ID failed");
        eStatus = MOS_STATUS_UNKNOWN;
        goto finish;
    }

    // Set Perf Tag
    pOsInterface->pfnResetPerfBufferID(pOsInterface);
    pOsInterface->pfnSetPerfTag(pOsInterface, RenderData.PerfTag);

    if (pHdrState->bFtrComputeWalker)
    {
        // Setup Compute Walker
        pWalkerParams = nullptr;
        pComputeWalkerParams = &ComputeWalkerParams;

        VPHAL_RENDER_CHK_STATUS(Vphal_HdrSetupComputeWalker(
            pHdrState,
            &RenderData,
            &ComputeWalkerParams));
    }
    else
    {
        // Setup Media Walker Object
        VpHal_PreprocessHdrSetupWalkerObject(
            pHdrState,
            &RenderData,
            &WalkerParams,
            iKDTIndex,
            0);
    }

    // Submit all media states to HW
    VPHAL_RENDER_CHK_STATUS(VpHal_RndrSubmitCommands(
        pRenderHal,
        nullptr,
        pHdrState->bNullHwRenderHdr,
        &WalkerParams,
        &ComputeWalkerParams,
        &pHdrState->StatusTableUpdateParams,
        (VpKernelID)kernelHdrPreprocess,
        0,
        nullptr,
        bLastSummit));

#if (_DEBUG || _RELEASE_INTERNAL)
    if (sEnableKernelDump)
    {
        VphalSurfaceDumper surfaceDumper(pOsInterface);
        std::string fileName("Preprocessed_HDRMandatory_coefficient_8x98");
        surfaceDumper.DumpSurfaceToFile(pOsInterface, &pHdrState->CoeffSurface, fileName.c_str(), 0, true, false, nullptr);
    }
#endif

    eStatus = MOS_STATUS_SUCCESS;

finish:
    VPHAL_RENDER_EXITMESSAGE("eStatus %d", eStatus);
    return eStatus;
}


//!
//! \brief    Calculate Yuv Range and Offest
//! \details  Calculate Yuv Range and Offest
//! \param    VPHAL_CSPACE cspace
//!           [in] Source color space
//! \param    float* pLumaOffset
//!           [out] Pointer to Luma Offset
//! \param    float* pLumaExcursion 
//!           [out] Pointer to Luma Excursion
//! \param    float* pChromaZero
//!           [out] Pointer to Chroma Offset
//! \param    float* pChromaExcursion
//!           [out] Pointer to Chroma Excursion
//! \return   MOS_STATUS
//!
MOS_STATUS VpHal_HdrGetYuvRangeAndOffset(
    VPHAL_CSPACE cspace,
    float*       pLumaOffset,
    float*       pLumaExcursion,
    float*       pChromaZero,
    float*       pChromaExcursion)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    VPHAL_PUBLIC_CHK_NULL(pLumaOffset);
    VPHAL_PUBLIC_CHK_NULL(pLumaExcursion);
    VPHAL_PUBLIC_CHK_NULL(pChromaZero);
    VPHAL_PUBLIC_CHK_NULL(pChromaExcursion);

    switch (cspace)
    {
    case CSpace_BT601_FullRange:
    case CSpace_BT709_FullRange:
    case CSpace_BT601Gray_FullRange:
        *pLumaOffset = 0.0f;
        *pLumaExcursion = 255.0f;
        *pChromaZero = 128.0f;
        *pChromaExcursion = 255.0f;
        break;

    case CSpace_BT601:
    case CSpace_BT709:
    case CSpace_xvYCC601: // since matrix is the same as 601, use the same range
    case CSpace_xvYCC709: // since matrix is the same as 709, use the same range
    case CSpace_BT601Gray:
    case CSpace_BT2020:
    case CSpace_BT2020_FullRange:
        *pLumaOffset = 16.0f;
        *pLumaExcursion = 219.0f;
        *pChromaZero = 128.0f;
        *pChromaExcursion = 224.0f;
        break;

    default:
        *pLumaOffset = 0.0f;
        *pLumaExcursion = 255.0f;
        *pChromaZero = 128.0f;
        *pChromaExcursion = 255.0f;
        break;
    }

    *pLumaOffset /= 255.0f;
    *pLumaExcursion /= 255.0f;
    *pChromaZero /= 255.0f;
    *pChromaExcursion /= 255.0f;

finish:
    return eStatus;
}

//!
//! \brief    Calculate Rgb Range and Offest
//! \details  Calculate Rgb Range and Offest
//! \param    VPHAL_CSPACE cspace
//!           [in] Source color space
//! \param    float* pLumaOffset
//!           [out] Pointer to Rgb Offset
//! \param    float* pLumaExcursion 
//!           [out] Pointer to Rgb Excursion
//! \return   MOS_STATUS
//!
MOS_STATUS VpHal_HdrGetRgbRangeAndOffset(
    VPHAL_CSPACE cspace,
    float*       pRgbOffset,
    float*       pRgbExcursion)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    VPHAL_PUBLIC_CHK_NULL(pRgbOffset);
    VPHAL_PUBLIC_CHK_NULL(pRgbExcursion);

    switch (cspace)
    {
    case CSpace_sRGB:
        *pRgbOffset = 0.0f;
        *pRgbExcursion = 255.0f;
        break;

    case CSpace_stRGB:
    case CSpace_BT2020_stRGB:
        *pRgbOffset = 16.0f;
        *pRgbExcursion = 219.0f;
        break;

    default:
        *pRgbOffset = 0.0f;
        *pRgbExcursion = 255.0f;
        break;
    }

    *pRgbOffset /= 255.0f;
    *pRgbExcursion /= 255.0f;

finish:
    return eStatus;
}

//!
//! \brief    Calculate Yuv To Rgb Matrix
//! \details  Calculate Yuv To Rgb Matrix
//! \param    VPHAL_CSPACE src
//!           [in] Source color space
//! \param    VPHAL_CSPACE dst
//!           [in] Dest color space
//! \param    float* pTransferMatrix
//!           [in] Pointer to input transfer matrix
//! \param    float* pOutMatrix
//!           [out] Pointer to output transfer matrix for curbe
//! \return   MOS_STATUS
//!
MOS_STATUS VpHal_HdrCalcYuvToRgbMatrix(
    VPHAL_CSPACE    src,
    VPHAL_CSPACE    dst,
    float*          pTransferMatrix,
    float*          pOutMatrix)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;
    float   Y_o = 0.0f, Y_e = 0.0f, C_z = 0.0f, C_e = 0.0f;
    float   R_o = 0.0f, R_e = 0.0f;

    VPHAL_PUBLIC_CHK_NULL(pTransferMatrix);
    VPHAL_PUBLIC_CHK_NULL(pOutMatrix);

    VpHal_HdrGetRgbRangeAndOffset(dst, &R_o, &R_e);
    VpHal_HdrGetYuvRangeAndOffset(src, &Y_o, &Y_e, &C_z, &C_e);

    // after + (3x3)(3x3)
    pOutMatrix[0] = pTransferMatrix[0] * R_e / Y_e;
    pOutMatrix[4] = pTransferMatrix[4] * R_e / Y_e;
    pOutMatrix[8] = pTransferMatrix[8] * R_e / Y_e;
    pOutMatrix[1] = pTransferMatrix[1] * R_e / C_e;
    pOutMatrix[5] = pTransferMatrix[5] * R_e / C_e;
    pOutMatrix[9] = pTransferMatrix[9] * R_e / C_e;
    pOutMatrix[2] = pTransferMatrix[2] * R_e / C_e;
    pOutMatrix[6] = pTransferMatrix[6] * R_e / C_e;
    pOutMatrix[10] = pTransferMatrix[10] * R_e / C_e;

    // (3x1) - (3x3)(3x3)(3x1)
    pOutMatrix[3] = R_o - (pOutMatrix[0] * Y_o + pOutMatrix[1] * C_z + pOutMatrix[2] * C_z);
    pOutMatrix[7] = R_o - (pOutMatrix[4] * Y_o + pOutMatrix[5] * C_z + pOutMatrix[6] * C_z);
    pOutMatrix[11] = R_o - (pOutMatrix[8] * Y_o + pOutMatrix[9] * C_z + pOutMatrix[10] * C_z);

finish:
    return eStatus;
}

//!
//! \brief    Calculate Rgb To Yuv Matrix
//! \details  Calculate Rgb To Yuv Matrix
//! \param    VPHAL_CSPACE src
//!           [in] Source color space
//! \param    VPHAL_CSPACE dst
//!           [in] Dest color space
//! \param    float* pTransferMatrix
//!           [in] Pointer to input transfer matrix
//! \param    float* pOutMatrix
//!           [out] Pointer to output transfer matrix for curbe
//! \return   MOS_STATUS
//!
MOS_STATUS VpHal_HdrCalcRgbToYuvMatrix(
    VPHAL_CSPACE    src,
    VPHAL_CSPACE    dst,
    float*          pTransferMatrix,
    float*          pOutMatrix)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;
    float   Y_o = 0.0f, Y_e = 0.0f, C_z = 0.0f, C_e = 0.0f;
    float   R_o = 0.0f, R_e = 0.0f;

    VPHAL_PUBLIC_CHK_NULL(pTransferMatrix);
    VPHAL_PUBLIC_CHK_NULL(pOutMatrix);

    VpHal_HdrGetRgbRangeAndOffset(src, &R_o, &R_e);
    VpHal_HdrGetYuvRangeAndOffset(dst, &Y_o, &Y_e, &C_z, &C_e);

    // multiplication of + onwards
    pOutMatrix[0] = pTransferMatrix[0] * Y_e / R_e;
    pOutMatrix[1] = pTransferMatrix[1] * Y_e / R_e;
    pOutMatrix[2] = pTransferMatrix[2] * Y_e / R_e;
    pOutMatrix[4] = pTransferMatrix[4] * C_e / R_e;
    pOutMatrix[5] = pTransferMatrix[5] * C_e / R_e;
    pOutMatrix[6] = pTransferMatrix[6] * C_e / R_e;
    pOutMatrix[8] = pTransferMatrix[8] * C_e / R_e;
    pOutMatrix[9] = pTransferMatrix[9] * C_e / R_e;
    pOutMatrix[10] = pTransferMatrix[10] * C_e / R_e;

    pOutMatrix[7] = Y_o - Y_e * R_o / R_e;
    pOutMatrix[3] = C_z;
    pOutMatrix[11] = C_z;

finish:
    return eStatus;
}

//!
//! \brief    Calculate CCM Matrix
//! \details  Calculate CCM Matrix
//! \param    float* pTransferMatrix
//!           [in] Pointer to input transfer matrix
//! \param    float* pOutMatrix
//!           [out] Pointer to output transfer matrix for curbe
//! \return   MOS_STATUS
//!
MOS_STATUS VpHal_HdrCalcCCMMatrix(
    float*          pTransferMatrix,
    float*          pOutMatrix)
{
    MOS_STATUS  eStatus = MOS_STATUS_SUCCESS;

    VPHAL_PUBLIC_CHK_NULL(pTransferMatrix);
    VPHAL_PUBLIC_CHK_NULL(pOutMatrix);

    // multiplication of + onwards
    pOutMatrix[0] = pTransferMatrix[1];
    pOutMatrix[1] = pTransferMatrix[2];
    pOutMatrix[2] = pTransferMatrix[0];
    pOutMatrix[4] = pTransferMatrix[5];
    pOutMatrix[5] = pTransferMatrix[6];
    pOutMatrix[6] = pTransferMatrix[4];
    pOutMatrix[8] = pTransferMatrix[9];
    pOutMatrix[9] = pTransferMatrix[10];
    pOutMatrix[10] = pTransferMatrix[8];

    pOutMatrix[3] = pTransferMatrix[11];
    pOutMatrix[7] = pTransferMatrix[3];
    pOutMatrix[11] = pTransferMatrix[7];

finish:
    return eStatus;
}