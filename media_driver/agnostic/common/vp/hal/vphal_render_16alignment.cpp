/*
*
* Copyright (c) Intel Corporation (2018 - 2019).
*
* INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
* LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
* ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
* PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
* DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
* PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
* including liability for infringement of any proprietary rights, relating to
* use of the code. No license, express or implied, by estoppel or otherwise,
* to any intellectual property rights is granted herein.
*
*
* File Name  : vphal_render_16alignment.cpp
*
* Abstract   : Video Surface 16 bytes alignment for Video Processing
*
* Environment: ubuntu
*
* Notes      : This module contains video surface 16 bytes alignment definitions
*              for VPHAL
*
*/
//!
//! \file     vphal_render_16alignment.cpp
//! \brief    Surface alignment as 16 bytes
//! \details  Unified VP HAL Surface 16 bytes alignment module interfaces
//!
#include "vphal_render_16alignment.h"
#include "vphal_debug.h"
#include "vpkrnheader.h"
#include "vphal_render_composite.h"
#include "vphal_render_ief.h"
#include "vphal_renderer.h"

#define AVS_SAMPLE_INDEX        1
#define TD_SAMPLE_INDEX         1 // 3D sampler

#define ALIGN16_SRC_Y_INDEX     0
#define ALIGN16_SRC_U_INDEX     1
#define ALIGN16_SRC_UV_INDEX    1
#define ALIGN16_SRC_V_INDEX     2
#define ALIGN16_TRG_INDEX       3
#define ALIGN16_TRG_Y_INDEX     3
#define ALIGN16_TRG_U_INDEX     4
#define ALIGN16_TRG_UV_INDEX    4
#define ALIGN16_TRG_V_INDEX     5
//!
//! \brief 16 Bytes Alignment Kernel params for Gen9 Media Walker
//!
static const RENDERHAL_KERNEL_PARAM g_16Align_MW_KernelParam[2] =
{
/*    GRF_Count
      |  BT_Count
      |  |    Sampler_Count
      |  |    |  Thread_Count
      |  |    |  |                             GRF_Start_Register
      |  |    |  |                             |   CURBE_Length
      |  |    |  |                             |   |   block_width
      |  |    |  |                             |   |   |    block_height
      |  |    |  |                             |   |   |    |   blocks_x
      |  |    |  |                             |   |   |    |   |   blocks_y
      |  |    |  |                             |   |   |    |   |   |*/
    { 4, 34,  1, VPHAL_USE_MEDIA_THREADS_MAX,  0,  2,  16,  16,  1,  1 },    // NV12 and YUY2
    { 4, 34,  1, VPHAL_USE_MEDIA_THREADS_MAX,  0,  2,  32,  8,  1,  1 },     // YV12 only
};

//!
//! \brief    16Align load the curbe data
//! \details  Curbe data for 16Align
//! \param    PVPHAL_16_ALIGN_STATE p16AlignState
//!           [in] Pointer to the 16Align State
//! \param    PVPHAL_16_ALIGN_RENDER_DATA pRenderData
//!           [in] Pointer to 16Align render data
//! \param    int32_t* piCurbeOffset
//!           [out] Pointer to curbe data offset
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VpHal_16AlignLoadStaticData(
    PVPHAL_16_ALIGN_STATE           p16AlignState,    
    PVPHAL_16_ALIGN_RENDER_DATA     pRenderData,
    int32_t*                        piCurbeOffset)
{
    PRENDERHAL_INTERFACE                    pRenderHal;
    MEDIA_WALKER_16ALIGN_STATIC_DATA        WalkerStatic;
    MOS_STATUS                              eStatus;
    int32_t                                 iCurbeLength;

    VPHAL_RENDER_CHK_NULL(p16AlignState);
    VPHAL_RENDER_CHK_NULL(p16AlignState->pRenderHal);
    eStatus          = MOS_STATUS_SUCCESS;
    pRenderHal       = p16AlignState->pRenderHal;

    // Set relevant static data
    MOS_ZeroMemory(&WalkerStatic, sizeof(MEDIA_WALKER_16ALIGN_STATIC_DATA));
    if (pRenderData->ScalingRatio_H < 0.0625f || pRenderData->ScalingRatio_V < 0.0625f)
    {
        WalkerStatic.DW0.Sampler_Index = TD_SAMPLE_INDEX;
        WalkerStatic.DW11.ScalingMode  = 0;
        WalkerStatic.DW12.Original_X   = 0;
        WalkerStatic.DW13.Original_Y   = 0;
    }
    else
    {
        WalkerStatic.DW0.Sampler_Index = AVS_SAMPLE_INDEX;
        WalkerStatic.DW11.ScalingMode  = 1;
    }

    switch (p16AlignState->pSource->Format)
    {
        case Format_NV12:
            WalkerStatic.DW1.pSrcSurface_Y    = ALIGN16_SRC_Y_INDEX;
            WalkerStatic.DW2.pSrcSurface_UV   = ALIGN16_SRC_UV_INDEX;
            WalkerStatic.DW9.Input_Format     = 0;
            break;
        case Format_YUY2:
            WalkerStatic.DW1.pSrcSurface_YUY2 = ALIGN16_SRC_Y_INDEX;
            WalkerStatic.DW9.Input_Format     = 1;
            break;
        case Format_YV12:
            WalkerStatic.DW1.pSrcSurface_Y    = ALIGN16_SRC_Y_INDEX;
            WalkerStatic.DW2.pSrcSurface_U    = ALIGN16_SRC_U_INDEX;
            WalkerStatic.DW3.pSrcSurface_V    = ALIGN16_SRC_V_INDEX;
            WalkerStatic.DW9.Input_Format     = 2;
            break;
        default:
            VPHAL_RENDER_ASSERTMESSAGE("16 align input format doesn't support.");
            eStatus = MOS_STATUS_INVALID_PARAMETER;
            break;
    }
    WalkerStatic.DW7.ScalingStep_H            = pRenderData->ScalingStep_H;
    WalkerStatic.DW8.ScalingStep_V            = pRenderData->ScalingStep_V;
#if defined(LINUX)
    WalkerStatic.DW10.Output_Pitch            = p16AlignState->pTarget->OsResource.iPitch;
#endif
    switch (p16AlignState->pTarget->Format)
    {
        case Format_NV12:
            WalkerStatic.DW4.pOutSurface_Y    = ALIGN16_TRG_Y_INDEX;
            WalkerStatic.DW5.pOutSurface_UV   = ALIGN16_TRG_UV_INDEX;
            WalkerStatic.DW9.Output_Format    = 0;
#if defined(LINUX)
            WalkerStatic.DW10.Output_UVOffset  = p16AlignState->pTarget->OsResource.iHeight;
#endif
            break;
        case Format_YUY2:
            WalkerStatic.DW4.pOutSurface      = ALIGN16_TRG_INDEX;
            WalkerStatic.DW9.Output_Format    = 1;
            break;
        case Format_YV12:
            WalkerStatic.DW4.pOutSurface_Y    = ALIGN16_TRG_Y_INDEX;
            WalkerStatic.DW5.pOutSurface_U    = ALIGN16_TRG_U_INDEX;
            WalkerStatic.DW6.pOutSurface_V    = ALIGN16_TRG_V_INDEX;
            WalkerStatic.DW9.Output_Format    = 2;
#if defined(LINUX)
            WalkerStatic.DW10.Output_UVOffset  = p16AlignState->pTarget->OsResource.iHeight;
#endif
            break;
        default:
            VPHAL_RENDER_ASSERTMESSAGE("16 align output format doesn't support.");
            eStatus = MOS_STATUS_INVALID_PARAMETER;
            break;
    }

    iCurbeLength = sizeof(MEDIA_WALKER_16ALIGN_STATIC_DATA);

    *piCurbeOffset = pRenderHal->pfnLoadCurbeData(
        pRenderHal,
        pRenderData->pMediaState,
        &WalkerStatic,
        iCurbeLength);

    if (*piCurbeOffset < 0)
    {
        eStatus = MOS_STATUS_UNKNOWN;
        goto finish;
    }

finish:
    VPHAL_RENDER_ASSERT(eStatus == MOS_STATUS_SUCCESS);
    return eStatus;
}

//!
//! \brief    16Align kernel setup
//! \details  Kernel setup for bitcopy
//! \param    PVPHAL_16_ALIGN_STATE p16AlignState
//!           [in] Pointer to the 16Align State
//! \param    PVPHAL_16_ALIGN_RENDER_DATA pRenderData
//!           [in] Pointer to 16Align render data
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VpHal_16AlignSetupKernel(
    PVPHAL_16_ALIGN_STATE        p16AlignState,
    PVPHAL_16_ALIGN_RENDER_DATA  pRenderData)
{
    MOS_STATUS      eStatus;
    Kdll_CacheEntry *pCacheEntryTable;

    VPHAL_RENDER_CHK_NULL(p16AlignState);
    eStatus             = MOS_STATUS_SUCCESS;
    pCacheEntryTable    = 
        p16AlignState->pKernelDllState->ComponentKernelCache.pCacheEntries;

    // Set the Kernel Parameters
    pRenderData->pKernelParam   = p16AlignState->pKernelParamTable;
    pRenderData->PerfTag        = VPHAL_NONE;

    // Set curbe & inline data size
    pRenderData->iCurbeLength   = pRenderData->pKernelParam->CURBE_Length * GRF_SIZE;

    // Set Kernel entry
    pRenderData->KernelEntry.iKUID     = IDR_VP_1_1_16aligned;
    pRenderData->KernelEntry.iKCID     = -1;
    pRenderData->KernelEntry.iSize     = pCacheEntryTable[IDR_VP_1_1_16aligned].iSize;
    pRenderData->KernelEntry.pBinary   = pCacheEntryTable[IDR_VP_1_1_16aligned].pBinary;

finish:
    return eStatus;
}

//!
//! \brief    Recalculate Sampler Avs 8x8 Horizontal/Vertical scaling table
//! \details  Recalculate Sampler Avs 8x8 Horizontal/Vertical scaling table
//! \param    MOS_FORMAT SrcFormat
//!           [in] Source Format
//! \param    float fScale
//!           [in] Horizontal or Vertical Scale Factor
//! \param    bool bVertical
//!           [in] true if Vertical Scaling, else Horizontal Scaling
//! \param    uint32_t dwChromaSiting
//!           [in] Chroma Siting
//! \param    bool bBalancedFilter
//!           [in] true if Gen9+, balanced filter
//! \param    bool b8TapAdaptiveEnable
//!           [in] true if 8Tap Adaptive Enable
//! \param    PVPHAL_AVS_PARAMS pAvsParams
//!           [in/out] Pointer to AVS Params
//! \return   MOS_STATUS
//!
static MOS_STATUS VpHal_16AlignSamplerAvsCalcScalingTable(
    MOS_FORMAT                      SrcFormat,
    float                           fScale,
    bool                            bVertical,
    uint32_t                        dwChromaSiting,
    bool                            bBalancedFilter,
    bool                            b8TapAdaptiveEnable,
    PMHW_AVS_PARAMS                 pAvsParams)
{
    MOS_STATUS                      eStatus = MOS_STATUS_SUCCESS;
    MHW_PLANE                       Plane;
    int32_t                         iUvPhaseOffset;
    uint32_t                        dwHwPhrase;
    uint32_t                        YCoefTableSize;
    uint32_t                        UVCoefTableSize;
    float                           fScaleParam;
    int32_t*                        piYCoefsParam;
    int32_t*                        piUVCoefsParam;
    float                           fHPStrength;

    VPHAL_RENDER_CHK_NULL(pAvsParams);
    VPHAL_RENDER_CHK_NULL(pAvsParams->piYCoefsY);
    VPHAL_RENDER_CHK_NULL(pAvsParams->piYCoefsX);
    VPHAL_RENDER_CHK_NULL(pAvsParams->piUVCoefsY);
    VPHAL_RENDER_CHK_NULL(pAvsParams->piUVCoefsX);

    if (bBalancedFilter)
    {
        YCoefTableSize      = POLYPHASE_Y_COEFFICIENT_TABLE_SIZE_G9;
        UVCoefTableSize     = POLYPHASE_UV_COEFFICIENT_TABLE_SIZE_G9;
        dwHwPhrase          = NUM_HW_POLYPHASE_TABLES_G9;
    }
    else
    {
        YCoefTableSize      = POLYPHASE_Y_COEFFICIENT_TABLE_SIZE_G8;
        UVCoefTableSize     = POLYPHASE_UV_COEFFICIENT_TABLE_SIZE_G8;
        dwHwPhrase          = MHW_NUM_HW_POLYPHASE_TABLES;
    }

    fHPStrength = 0.0F;
    piYCoefsParam   = bVertical ? pAvsParams->piYCoefsY : pAvsParams->piYCoefsX;
    piUVCoefsParam  = bVertical ? pAvsParams->piUVCoefsY : pAvsParams->piUVCoefsX;
    fScaleParam     = bVertical ? pAvsParams->fScaleY : pAvsParams->fScaleX;

    // Recalculate Horizontal or Vertical scaling table
    if (SrcFormat != pAvsParams->Format || fScale != fScaleParam)
    {
        MOS_ZeroMemory(piYCoefsParam, YCoefTableSize);
        MOS_ZeroMemory(piUVCoefsParam, UVCoefTableSize);

        // 4-tap filtering for RGB format G-channel if 8tap adaptive filter is not enabled.
        Plane = (IS_RGB32_FORMAT(SrcFormat) && !b8TapAdaptiveEnable) ? MHW_U_PLANE : MHW_Y_PLANE;
        if (bVertical)
        {
            pAvsParams->fScaleY = fScale;
        }
        else
        {
            pAvsParams->fScaleX = fScale;
        }

        // For 1x scaling in horizontal direction, use special coefficients for filtering
        // we don't do this when bForcePolyPhaseCoefs flag is set
        if (fScale == 1.0F && !pAvsParams->bForcePolyPhaseCoefs)
        {
            VPHAL_RENDER_CHK_STATUS(Mhw_SetNearestModeTable(
                piYCoefsParam,
                Plane,
                bBalancedFilter));
            // If the 8-tap adaptive is enabled for all channel, then UV/RB use the same coefficient as Y/G
            // So, coefficient for UV/RB channels caculation can be passed
            if (!b8TapAdaptiveEnable)
            {
                VPHAL_RENDER_CHK_STATUS(Mhw_SetNearestModeTable(
                    piUVCoefsParam,
                    MHW_U_PLANE,
                    bBalancedFilter));
            }
        }
        else
        {
            // Clamp the Scaling Factor if > 1.0x
            fScale = MOS_MIN(1.0F, fScale);

            VPHAL_RENDER_CHK_STATUS(Mhw_CalcPolyphaseTablesY(
                piYCoefsParam,
                fScale,
                Plane,
                SrcFormat,
                fHPStrength,
                true,
                dwHwPhrase));

            // If the 8-tap adaptive is enabled for all channel, then UV/RB use the same coefficient as Y/G
            // So, coefficient for UV/RB channels caculation can be passed
            if (!b8TapAdaptiveEnable)
            {
                if (!bBalancedFilter)
                {
                    VPHAL_RENDER_CHK_STATUS(Mhw_CalcPolyphaseTablesY(
                        piUVCoefsParam,
                        fScale,
                        MHW_U_PLANE,
                        SrcFormat,
                        fHPStrength,
                        true,
                        dwHwPhrase));
                }
                else
                {
                    // If Chroma Siting info is present
                    if (dwChromaSiting & (bVertical ? MHW_CHROMA_SITING_VERT_TOP : MHW_CHROMA_SITING_HORZ_LEFT))
                    {
                        // No Chroma Siting
                        VPHAL_RENDER_CHK_STATUS(Mhw_CalcPolyphaseTablesUV(
                            piUVCoefsParam,
                            2.0F,
                            fScale));
                    }
                    else
                    {
                        // Chroma siting offset needs to be added
                        if (dwChromaSiting & (bVertical ? MHW_CHROMA_SITING_VERT_CENTER : MHW_CHROMA_SITING_HORZ_CENTER))
                        {
                            iUvPhaseOffset = MOS_UF_ROUND(0.5F * 16.0F);   // U0.4
                        }
                        else //if (ChromaSiting & (bVertical ? MHW_CHROMA_SITING_VERT_BOTTOM : MHW_CHROMA_SITING_HORZ_RIGHT))
                        {
                            iUvPhaseOffset = MOS_UF_ROUND(1.0F * 16.0F);   // U0.4
                        }

                        VPHAL_RENDER_CHK_STATUS(Mhw_CalcPolyphaseTablesUVOffset(
                            piUVCoefsParam,
                            3.0F,
                            fScale,
                            iUvPhaseOffset));
                    }
                }
            }
        }
    }

finish:
    return eStatus;
}

//!
//! \brief    Set Sampler Avs 8x8 Table for LGCA
//! \details  Set Sampler Avs 8x8 Table for LGCA
//! \param    PRENDERHAL_INTERFACE pRenderHal
//!           [in] Pointer to RenderHal Interface Structure
//! \param    PMHW_SAMPLER_STATE_PARAM pSamplerStateParams
//!           [in] Pointer to Sampler State Params
//! \param    PMHW_AVS_PARAMS pAvsParams
//!           [in/out] Pointer to AVS Params
//! \param    MOS_FORMAT SrcFormat
//!           [in] Source Format
//! \return   MOS_STATUS
//!
static MOS_STATUS VpHal_16AlignSetSamplerAvsTableParam(
    PRENDERHAL_INTERFACE            pRenderHal,
    PMHW_SAMPLER_STATE_PARAM        pSamplerStateParams,
    PMHW_AVS_PARAMS                 pAvsParams,
    MOS_FORMAT                      SrcFormat,
    float                           fScaleX,
    float                           fScaleY,
    uint32_t                        dwChromaSiting)
{
    MOS_STATUS                   eStatus = MOS_STATUS_SUCCESS;
    bool                         bBalancedFilter;
    PMHW_SAMPLER_AVS_TABLE_PARAM pMhwSamplerAvsTableParam;

    VPHAL_RENDER_CHK_NULL(pRenderHal);
    VPHAL_RENDER_CHK_NULL(pSamplerStateParams);
    VPHAL_RENDER_CHK_NULL(pAvsParams);
    if (pAvsParams->piUVCoefsX == nullptr || pAvsParams->piYCoefsY  == nullptr ||
        pAvsParams->piYCoefsX  == nullptr || pAvsParams->piUVCoefsY == nullptr ||
        pAvsParams             == nullptr)
    {
        VPHAL_RENDER_ASSERTMESSAGE("meet null ptr!");
    }

    pMhwSamplerAvsTableParam = pSamplerStateParams->Avs.pMhwSamplerAvsTableParam;

    pMhwSamplerAvsTableParam->b8TapAdaptiveEnable         = pSamplerStateParams->Avs.b8TapAdaptiveEnable;
    pMhwSamplerAvsTableParam->byteTransitionArea8Pixels   = MEDIASTATE_AVS_TRANSITION_AREA_8_PIXELS;
    pMhwSamplerAvsTableParam->byteTransitionArea4Pixels   = MEDIASTATE_AVS_TRANSITION_AREA_4_PIXELS;
    pMhwSamplerAvsTableParam->byteMaxDerivative8Pixels    = MEDIASTATE_AVS_MAX_DERIVATIVE_8_PIXELS;
    pMhwSamplerAvsTableParam->byteMaxDerivative4Pixels    = MEDIASTATE_AVS_MAX_DERIVATIVE_4_PIXELS;
    pMhwSamplerAvsTableParam->byteDefaultSharpnessLevel   = MEDIASTATE_AVS_SHARPNESS_LEVEL_SHARP;

    // Enable Adaptive Filtering, if it is being upscaled
    // in either direction. we must check for this before clamping the SF.
    if ((IS_YUV_FORMAT(SrcFormat) && (fScaleX > 1.0F || fScaleY > 1.0F)) ||
        pMhwSamplerAvsTableParam->b8TapAdaptiveEnable)
    {
        pMhwSamplerAvsTableParam->bBypassXAdaptiveFiltering = false;
        pMhwSamplerAvsTableParam->bBypassYAdaptiveFiltering = false;
        if (pMhwSamplerAvsTableParam->b8TapAdaptiveEnable)
        {
            pMhwSamplerAvsTableParam->bAdaptiveFilterAllChannels = true;

            if (IS_RGB_FORMAT(SrcFormat))
            {
                pMhwSamplerAvsTableParam->bEnableRGBAdaptive     = true;
            }
        }
    }
    else
    {
        pMhwSamplerAvsTableParam->bBypassXAdaptiveFiltering = true;
        pMhwSamplerAvsTableParam->bBypassYAdaptiveFiltering = true;
    }

    // No changes to AVS parameters -> skip
    if (SrcFormat == pAvsParams->Format &&
        fScaleX == pAvsParams->fScaleX &&
        fScaleY == pAvsParams->fScaleY)
    {
        goto finish;
    }

    // AVS Coefficients don't change for Scaling Factors > 1.0x
    // Hence recalculation is avoided
    if (fScaleX > 1.0F && pAvsParams->fScaleX > 1.0F)
    {
        pAvsParams->fScaleX = fScaleX;
    }

    // AVS Coefficients don't change for Scaling Factors > 1.0x
    // Hence recalculation is avoided
    if (fScaleY > 1.0F && pAvsParams->fScaleY > 1.0F)
    {
        pAvsParams->fScaleY = fScaleY;
    }

    bBalancedFilter = true;
    // Recalculate Horizontal scaling table
    VPHAL_HW_CHK_STATUS(VpHal_16AlignSamplerAvsCalcScalingTable(
        SrcFormat,
        fScaleX,
        false,
        dwChromaSiting,
        bBalancedFilter,
        pMhwSamplerAvsTableParam->b8TapAdaptiveEnable ? true : false,
        pAvsParams));

    // Recalculate Vertical scaling table
    VPHAL_HW_CHK_STATUS(VpHal_16AlignSamplerAvsCalcScalingTable(
        SrcFormat,
        fScaleY,
        true,
        dwChromaSiting,
        bBalancedFilter,
        pMhwSamplerAvsTableParam->b8TapAdaptiveEnable ? true : false,
        pAvsParams));

    pMhwSamplerAvsTableParam->bIsCoeffExtraEnabled = true;
    // Save format used to calculate AVS parameters
    pAvsParams->Format                             = SrcFormat;
    pMhwSamplerAvsTableParam->b4TapGY              = (IS_RGB32_FORMAT(SrcFormat) && !pMhwSamplerAvsTableParam->b8TapAdaptiveEnable);
    pMhwSamplerAvsTableParam->b4TapRBUV            = (!pMhwSamplerAvsTableParam->b8TapAdaptiveEnable);

    VpHal_RenderCommonSetAVSTableParam(pAvsParams, pMhwSamplerAvsTableParam);

finish:
    return eStatus;
}

//!
//! \brief    16Align setup HW states
//! \details  Setup HW states for 16Align
//! \param    PVPHAL_16_ALIGN_STATE p16AlignState
//!           [in] Pointer to the 16Align State
//! \param    PVPHAL_16_ALIGN_RENDER_DATA pRenderData
//!           [in/out] Pointer to 16Align render data
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VpHal_16AlignSetSamplerStates(
    PVPHAL_16_ALIGN_STATE        p16AlignState,
    PVPHAL_16_ALIGN_RENDER_DATA  pRenderData)
{
    MOS_STATUS                  eStatus;
    PRENDERHAL_INTERFACE        pRenderHal;
    PMHW_SAMPLER_STATE_PARAM    pSamplerStateParams;

    VPHAL_PUBLIC_CHK_NULL(p16AlignState);
    VPHAL_PUBLIC_CHK_NULL(pRenderData);

    pRenderHal = p16AlignState->pRenderHal;

    VPHAL_PUBLIC_CHK_NULL(pRenderHal);
    pSamplerStateParams                          = &pRenderData->SamplerStateParams;
    pSamplerStateParams->bInUse                  = true;
    pSamplerStateParams->SamplerType             = MHW_SAMPLER_TYPE_AVS;
    pSamplerStateParams->Avs.AvsType             = false;
    pSamplerStateParams->Avs.bEnableIEF          = false;
    pSamplerStateParams->Avs.b8TapAdaptiveEnable = false;
    pSamplerStateParams->Avs.bHdcDwEnable        = true;
    pSamplerStateParams->Avs.bEnableAVS          = true;
    pSamplerStateParams->Avs.WeakEdgeThr         = DETAIL_WEAK_EDGE_THRESHOLD;
    pSamplerStateParams->Avs.StrongEdgeThr       = DETAIL_STRONG_EDGE_THRESHOLD;
    pSamplerStateParams->Avs.StrongEdgeWght      = DETAIL_STRONG_EDGE_WEIGHT;
    pSamplerStateParams->Avs.RegularWght         = DETAIL_REGULAR_EDGE_WEIGHT;
    pSamplerStateParams->Avs.NonEdgeWght         = DETAIL_NON_EDGE_WEIGHT;
    pSamplerStateParams->Avs.pMhwSamplerAvsTableParam = &p16AlignState->mhwSamplerAvsTableParam;

    VPHAL_RENDER_CHK_STATUS(VpHal_16AlignSetSamplerAvsTableParam(
                    pRenderHal,
                    pSamplerStateParams,
                    pRenderData->pAVSParameters,
                    p16AlignState->pSource->Format,
                    pRenderData->ScalingRatio_H,
                    pRenderData->ScalingRatio_V,
                    MHW_CHROMA_SITING_HORZ_LEFT | MHW_CHROMA_SITING_VERT_TOP));

    eStatus = pRenderHal->pfnSetSamplerStates(
        pRenderHal,
        pRenderData->iMediaID,
        pSamplerStateParams,
        1);

finish:
    return eStatus;
}

//!
//! \brief    16Align setup HW states
//! \details  Setup HW states for 16Align
//! \param    PVPHAL_16_ALIGN_STATE p16AlignState
//!           [in] Pointer to the 16Align State
//! \param    PVPHAL_16_ALIGN_RENDER_DATA pRenderData
//!           [in/out] Pointer to 16Align render data
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VpHal_16AlignSetupHwStates(
    PVPHAL_16_ALIGN_STATE        p16AlignState,
    PVPHAL_16_ALIGN_RENDER_DATA  pRenderData)
{
    PRENDERHAL_INTERFACE        pRenderHal;
    int32_t                     iKrnAllocation;
    int32_t                     iCurbeOffset;
    MOS_STATUS                  eStatus;
    int32_t                     iThreadCount;
    MHW_KERNEL_PARAM            MhwKernelParam;

    VPHAL_RENDER_CHK_NULL(p16AlignState);
    VPHAL_RENDER_CHK_NULL(pRenderData);

    eStatus                     = MOS_STATUS_SUCCESS;
    pRenderHal                  = p16AlignState->pRenderHal;
    VPHAL_RENDER_CHK_NULL(pRenderHal);

    // Allocate and reset media state
    pRenderData->pMediaState = pRenderHal->pfnAssignMediaState(pRenderHal, (RENDERHAL_COMPONENT)RENDERHAL_COMPONENT_16ALIGN);
    VPHAL_RENDER_CHK_NULL(pRenderData->pMediaState);

    // Allocate and reset SSH instance
    VPHAL_RENDER_CHK_STATUS(pRenderHal->pfnAssignSshInstance(pRenderHal));

    // Assign and Reset Binding Table
    VPHAL_RENDER_CHK_STATUS(pRenderHal->pfnAssignBindingTable(
            pRenderHal,
            &pRenderData->iBindingTable));

    // Setup surface states
    VPHAL_RENDER_CHK_STATUS(p16AlignState->pfnSetupSurfaceStates(
            p16AlignState,
            pRenderData));

    // load static data
    VPHAL_RENDER_CHK_STATUS(p16AlignState->pfnLoadStaticData(
            p16AlignState,
            pRenderData,
            &iCurbeOffset));

    if (p16AlignState->pPerfData->CompMaxThreads.bEnabled)
    {
        iThreadCount = p16AlignState->pPerfData->CompMaxThreads.uiVal;
    }
    else
    {
        iThreadCount = pRenderData->pKernelParam->Thread_Count;
    }

    // Setup VFE State params.
    VPHAL_RENDER_CHK_STATUS(pRenderHal->pfnSetVfeStateParams(
        pRenderHal,
        MEDIASTATE_DEBUG_COUNTER_FREE_RUNNING,
        iThreadCount,
        pRenderData->iCurbeLength,
        pRenderData->iInlineLength,
        nullptr));

    // Load kernel to GSH
    INIT_MHW_KERNEL_PARAM(MhwKernelParam, &pRenderData->KernelEntry);
    iKrnAllocation = pRenderHal->pfnLoadKernel(
        pRenderHal,
        pRenderData->pKernelParam,
        &MhwKernelParam,
        nullptr);

    if (iKrnAllocation < 0) 
    {
        eStatus = MOS_STATUS_UNKNOWN;
        goto finish;
    }

    // Allocate Media ID, link to kernel
    pRenderData->iMediaID = pRenderHal->pfnAllocateMediaID(
        pRenderHal,
        iKrnAllocation,
        pRenderData->iBindingTable,
        iCurbeOffset,
        (pRenderData->pKernelParam->CURBE_Length << 5),
        0,
        nullptr);
    
    if (pRenderData->iMediaID < 0) 
    {
        eStatus = MOS_STATUS_UNKNOWN;
        goto finish;
    }

    // Set Sampler states for this Media ID
    VPHAL_RENDER_CHK_STATUS(p16AlignState->pfnSetSamplerStates(
        p16AlignState, 
        pRenderData));

finish:
    VPHAL_RENDER_ASSERT(eStatus == MOS_STATUS_SUCCESS);
    return eStatus;
}

//!
//! \brief    16Align media walker setup
//! \details  Media walker setup for bitcopy
//! \param    PVPHAL_16_ALIGN_STATE p16AlignState
//!           [in] Pointer to the 16Align State
//! \param    PVPHAL_16_ALIGN_RENDER_DATA pRenderData
//!           [in] Pointer to 16Align render data
//! \param    PMHW_WALKER_PARAMS pWalkerParams
//!           [in/out] Pointer to Walker params
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VpHal_16AlignRenderMediaWalker(
    PVPHAL_16_ALIGN_STATE        p16AlignState,    
    PVPHAL_16_ALIGN_RENDER_DATA  pRenderData,
    PMHW_WALKER_PARAMS               pWalkerParams)
{
    PRENDERHAL_INTERFACE            pRenderHal;
    uint32_t                        dwWidth;
    uint32_t                        dwHeight;
    MOS_STATUS                      eStatus;

    eStatus     = MOS_STATUS_SUCCESS;
    pRenderHal  = p16AlignState->pRenderHal;

    // Calculate how many media object commands are needed.
    dwWidth  = MOS_ALIGN_CEIL((p16AlignState->pTarget->rcSrc.right  - 
                           p16AlignState->pTarget->rcSrc.left), 
                           pRenderData->pKernelParam->block_width);
    dwHeight = MOS_ALIGN_CEIL((p16AlignState->pTarget->rcSrc.bottom - 
                           p16AlignState->pTarget->rcSrc.top),  
                           pRenderData->pKernelParam->block_height);

    pRenderData->iBlocksX = dwWidth  / pRenderData->pKernelParam->block_width;
    pRenderData->iBlocksY = dwHeight / pRenderData->pKernelParam->block_height;

    // Set walker cmd params - Rasterscan
    MOS_ZeroMemory(pWalkerParams, sizeof(*pWalkerParams));

    pWalkerParams->InterfaceDescriptorOffset    = pRenderData->iMediaID;

    pWalkerParams->dwGlobalLoopExecCount        = 1;
    pWalkerParams->dwLocalLoopExecCount         = pRenderData->iBlocksY - 1;
    
    pWalkerParams->GlobalResolution.x           = pRenderData->iBlocksX;
    pWalkerParams->GlobalResolution.y           = pRenderData->iBlocksY;
    
    pWalkerParams->GlobalStart.x                = 0;
    pWalkerParams->GlobalStart.y                = 0;

    pWalkerParams->GlobalOutlerLoopStride.x     = pRenderData->iBlocksX;
    pWalkerParams->GlobalOutlerLoopStride.y     = 0;

    pWalkerParams->GlobalInnerLoopUnit.x        = 0;
    pWalkerParams->GlobalInnerLoopUnit.y        = pRenderData->iBlocksY;

    pWalkerParams->BlockResolution.x            = pRenderData->iBlocksX;
    pWalkerParams->BlockResolution.y            = pRenderData->iBlocksY;
   
    pWalkerParams->LocalStart.x                 = 0;
    pWalkerParams->LocalStart.y                 = 0;

    pWalkerParams->LocalEnd.x                   = pRenderData->iBlocksX - 1;
    pWalkerParams->LocalEnd.y                   = 0;

    pWalkerParams->LocalOutLoopStride.x         = 0;
    pWalkerParams->LocalOutLoopStride.y         = 1;

    pWalkerParams->LocalInnerLoopUnit.x         = 1;
    pWalkerParams->LocalInnerLoopUnit.y         = 0;
    
    return eStatus;
}

//!
//! \brief    16Align renderer
//! \details  Renderer function for 16Align
//! \param    PVPHAL_16_ALIGN_STATE p16AlignState
//!           [in] Pointer to the 16Align State
//! \param    PVPHAL_RENDER_PARAMS pRenderParams
//!           [in] Pointer to 16Align render params
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VpHal_16AlignRender(
    PVPHAL_16_ALIGN_STATE p16AlignState,
    PVPHAL_RENDER_PARAMS     pRenderParams)
{
    MOS_STATUS                              eStatus;
    PRENDERHAL_INTERFACE                    pRenderHal;
    PMOS_INTERFACE                          pOsInterface;
    MHW_WALKER_PARAMS                       WalkerParams;
    VPHAL_16_ALIGN_RENDER_DATA              RenderData;
    PRENDERHAL_L3_CACHE_SETTINGS            pCacheSettings = nullptr;
    uint32_t                                dwInputRegionHeight;
    uint32_t                                dwInputRegionWidth;
    uint32_t                                dwOutputRegionHeight;
    uint32_t                                dwOutputRegionWidth;

    VPHAL_RENDER_ASSERT(p16AlignState);
    VPHAL_RENDER_ASSERT(pRenderParams);
    VPHAL_RENDER_ASSERT(p16AlignState->pOsInterface);
    VPHAL_RENDER_ASSERT(p16AlignState->pRenderHal);
    VPHAL_RENDER_ASSERT(p16AlignState->pPerfData);

    eStatus                     = MOS_STATUS_SUCCESS;
    pOsInterface                = p16AlignState->pOsInterface;
    pRenderHal                  = p16AlignState->pRenderHal;
    MOS_ZeroMemory(&RenderData, sizeof(RenderData));

    // Reset reporting
    p16AlignState->Reporting.InitReportValue();

    // Reset states before rendering
    pOsInterface->pfnResetOsStates(pOsInterface);
    VPHAL_RENDER_CHK_STATUS(pRenderHal->pfnReset(pRenderHal));
    pOsInterface->pfnResetPerfBufferID(pOsInterface);   // reset once per frame

    VPHAL_DBG_STATE_DUMPPER_SET_CURRENT_STAGE(VPHAL_DBG_STAGE_COMP);

    // Configure cache settings for this render operation 
    pCacheSettings      = &pRenderHal->L3CacheSettings;
    MOS_ZeroMemory(pCacheSettings, sizeof(*pCacheSettings));
    pCacheSettings->bOverride                  = true;
    pCacheSettings->bL3CachingEnabled          = p16AlignState->SurfMemObjCtl.bL3CachingEnabled;

    if (p16AlignState->pPerfData->L3SQCReg1Override.bEnabled)
    {
        pCacheSettings->bSqcReg1Override       = true;
        pCacheSettings->dwSqcReg1              = p16AlignState->pPerfData->L3SQCReg1Override.uiVal;
    }

    if (p16AlignState->pPerfData->L3CntlReg2Override.bEnabled)
    {
        pCacheSettings->bCntlReg2Override      = true;
        pCacheSettings->dwCntlReg2             = p16AlignState->pPerfData->L3CntlReg2Override.uiVal;
    }

    if (p16AlignState->pPerfData->L3CntlReg3Override.bEnabled)
    {
        pCacheSettings->bCntlReg3Override      = true;
        pCacheSettings->dwCntlReg3             = p16AlignState->pPerfData->L3CntlReg3Override.uiVal;
    }

    if (p16AlignState->pPerfData->L3LRA1RegOverride.bEnabled)
    {
        pCacheSettings->bLra1RegOverride       = true;
        pCacheSettings->dwLra1Reg              = p16AlignState->pPerfData->L3LRA1RegOverride.uiVal;
    }

    // Setup Source/Target surface and get the Source width/height for
    p16AlignState->pSource           = pRenderParams->pSrc[0];
    p16AlignState->pTarget           = pRenderParams->pTarget[0];
    dwInputRegionWidth               = p16AlignState->pSource->rcSrc.right  - p16AlignState->pSource->rcSrc.left;
    dwInputRegionHeight              = p16AlignState->pSource->rcSrc.bottom - p16AlignState->pSource->rcSrc.top;
    dwOutputRegionWidth              = p16AlignState->pSource->rcDst.right  - p16AlignState->pSource->rcDst.left;
    dwOutputRegionHeight             = p16AlignState->pSource->rcDst.bottom - p16AlignState->pSource->rcDst.top;

    RenderData.ScalingStep_H        = (float)1.0 / (float)dwOutputRegionWidth;
    RenderData.ScalingStep_V        = (float)1.0 / (float)dwOutputRegionHeight;
    RenderData.ScalingRatio_H       = (float)dwOutputRegionWidth / (float)dwInputRegionWidth;
    RenderData.ScalingRatio_V       = (float)dwOutputRegionHeight / (float)dwInputRegionHeight;

    RenderData.pAVSParameters = &p16AlignState->AVSParameters;
    RenderData.SamplerStateParams.Avs.pMhwSamplerAvsTableParam = &RenderData.mhwSamplerAvsTableParam;

    p16AlignState->pKernelParamTable =
        (PRENDERHAL_KERNEL_PARAM)((p16AlignState->pTarget->Format != Format_YV12)?&g_16Align_MW_KernelParam[0]:&g_16Align_MW_KernelParam[1]);

    // Ensure input can be read
    pOsInterface->pfnSyncOnResource(
        pOsInterface, 
        &p16AlignState->pSource->OsResource,
        pOsInterface->CurrentGpuContextOrdinal,
        false);

    // Ensure the output can be written
    pOsInterface->pfnSyncOnResource(
        pOsInterface, 
        &p16AlignState->pTarget->OsResource,
        pOsInterface->CurrentGpuContextOrdinal,
        true);

    // Setup copy kernel
    VPHAL_RENDER_CHK_STATUS(p16AlignState->pfnSetupKernel(
            p16AlignState,
            &RenderData));

    // Submit HW States and Commands
    VPHAL_RENDER_CHK_STATUS(VpHal_16AlignSetupHwStates(
            p16AlignState, 
            &RenderData));

    // Set perftag information
    pOsInterface->pfnResetPerfBufferID(pOsInterface);
    pOsInterface->pfnSetPerfTag(pOsInterface, RenderData.PerfTag);

    VPHAL_RENDER_CHK_STATUS(VpHal_16AlignRenderMediaWalker(
            p16AlignState,    
            &RenderData,
            &WalkerParams));

    VPHAL_DBG_STATE_DUMPPER_DUMP_GSH(pRenderHal);
    VPHAL_DBG_STATE_DUMPPER_DUMP_SSH(pRenderHal);

    VPHAL_RENDER_CHK_STATUS(VpHal_RndrSubmitCommands(
        pRenderHal, 
        nullptr,
        p16AlignState->bNullHwRender16Align,
        &WalkerParams,
        nullptr,
        &p16AlignState->StatusTableUpdateParams,
        kernelUserPtr));

finish:
    MOS_ZeroMemory(pCacheSettings, sizeof(*pCacheSettings));
    VPHAL_RENDER_ASSERT(eStatus == MOS_STATUS_SUCCESS);
    VPHAL_RENDER_NORMALMESSAGE("finish 16alignment process!");
    return eStatus;
}

//!
//! \brief    16Align Destroy state
//! \details  Function to destroy 16Align state
//! \param    PVPHAL_16_ALIGN_STATE p16AlignState
//!           [in] Pointer to the 16Align State
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VpHal_16AlignDestroy(
    PVPHAL_16_ALIGN_STATE    p16AlignState)
{
    MOS_STATUS eStatus;
    eStatus = MOS_STATUS_SUCCESS;
    VPHAL_RENDER_CHK_NULL(p16AlignState);
    VpHal_RenderDestroyAVSParams(&p16AlignState->AVSParameters);
    MOS_UNUSED(p16AlignState);

finish:
    return eStatus;
}

//!
//! \brief    16Align kernel state Initializations
//! \details  Kernel state Initializations for 16Align
//! \param    PVPHAL_16_ALIGN_STATE p16AlignState
//!           [in] Pointer to the 16Align State
//! \param    const VphalSettings* pSettings
//!           [in] Pointer to VPHAL Setting
//! \param    Kdll_State pKernelDllState
//!           [in/out] Pointer to bitcopy kernel Dll state
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VpHal_16AlignInitialize(
    PVPHAL_16_ALIGN_STATE    p16AlignState,
    const VphalSettings      *pSettings,
    Kdll_State               *pKernelDllState)
{
    MOS_NULL_RENDERING_FLAGS  NullRenderingFlags;

    VPHAL_RENDER_ASSERT(p16AlignState);
    VPHAL_RENDER_ASSERT(p16AlignState->pOsInterface);

    NullRenderingFlags            = 
                    p16AlignState->pOsInterface->pfnGetNullHWRenderFlags(p16AlignState->pOsInterface);
    p16AlignState->bNullHwRender16Align =
                    NullRenderingFlags.VPLgca ||
                    NullRenderingFlags.VPGobal;

    // Setup interface to KDLL
    p16AlignState->pKernelDllState   = pKernelDllState;
    VpHal_RenderInitAVSParams(&p16AlignState->AVSParameters,
            POLYPHASE_Y_COEFFICIENT_TABLE_SIZE_G9,
            POLYPHASE_UV_COEFFICIENT_TABLE_SIZE_G9);

    return MOS_STATUS_SUCCESS;
}

//!
//! \brief    16alignment setup surface states
//! \details  Setup surface states for 16Align
//! \param    PVPHAL_16_ALIGN_STATE p16AlignState
//!           [in] Pointer to the 16Align State
//! \param    PVPHAL_16_ALIGN_RENDER_DATA pRenderData
//!           [in] Pointer to 16Align render data
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VpHal_16AlignSetupSurfaceStates(
    PVPHAL_16_ALIGN_STATE        p16AlignState,    
    PVPHAL_16_ALIGN_RENDER_DATA  pRenderData)
{
    PRENDERHAL_INTERFACE            pRenderHal;
    RENDERHAL_SURFACE_STATE_PARAMS  SurfaceParams;
    MOS_STATUS                      eStatus;
    PRENDERHAL_SURFACE_STATE_ENTRY  pSurfaceEntry;

    eStatus             = MOS_STATUS_SUCCESS;
    pRenderHal          = p16AlignState->pRenderHal;
    uint32_t width      = p16AlignState->pTarget->dwWidth;
#if defined(LINUX)
    uint32_t dwSize     = p16AlignState->pTarget->dwHeight * p16AlignState->pTarget->OsResource.iPitch;
#endif
    MOS_FORMAT format   = p16AlignState->pTarget->Format;

    // Source surface
    MOS_ZeroMemory(&SurfaceParams, sizeof(SurfaceParams));

    SurfaceParams.bAVS              = true;
    SurfaceParams.Boundary          = RENDERHAL_SS_BOUNDARY_SRCRECT;
    SurfaceParams.bRenderTarget     = false;
    SurfaceParams.MemObjCtl         = 
        p16AlignState->SurfMemObjCtl.SourceSurfMemObjCtl;
    SurfaceParams.Type              = RENDERHAL_SURFACE_TYPE_ADV_G9;
    SurfaceParams.bWidthInDword_Y   = false;
    SurfaceParams.bWidthInDword_UV  = false;
    SurfaceParams.bWidth16Align     = false;

    VPHAL_RENDER_CHK_STATUS(VpHal_CommonSetSurfaceForHwAccess(
                pRenderHal,
                p16AlignState->pSource,
                &p16AlignState->RenderHalSource,
                &SurfaceParams,
                pRenderData->iBindingTable,
                ALIGN16_SRC_Y_INDEX,
                false));

    // for 1 sampler access YV12 3plane input, Y plane should use the R8 sampler type, the same as U,V plane
    // for 3 samplers access YV12 3plane input, Y plane should use Y8 sampler type
    // 16-alignment kernel always uses 1-sampler, legacy FC kernel always uses 3-sampler
    if (p16AlignState->pSource->Format == Format_YV12)
    {
        uint32_t * pSrcPlaneYSampler  = nullptr;
        pSurfaceEntry       = &pRenderHal->pStateHeap->pSurfaceEntry[0]; // input Y plane
        pSrcPlaneYSampler   = (uint32_t*)pSurfaceEntry->pSurfaceState + 2; // DW2
        *pSrcPlaneYSampler  = (*pSrcPlaneYSampler & 0x07FFFFFF) | (0x0B<<27);
    }

    // Target surface
    SurfaceParams.MemObjCtl         =
        p16AlignState->SurfMemObjCtl.TargetSurfMemObjCtl;
    SurfaceParams.Type              = pRenderHal->SurfaceTypeDefault;
    SurfaceParams.bRenderTarget     = true;
    SurfaceParams.bAVS              = false;
    SurfaceParams.Boundary          = RENDERHAL_SS_BOUNDARY_DSTRECT;

    // reset the output surface format as Raw and calculate the surface size.
    p16AlignState->pTarget->Format      = Format_RAW;
#if defined(LINUX)
    switch (format)
    {
        case Format_NV12:
            for (int i = 0; i < 2; i++)
            {
                p16AlignState->pTarget->dwWidth = (i==0)?dwSize:dwSize/2;
                p16AlignState->pTarget->dwWidth = MOS_ALIGN_CEIL(p16AlignState->pTarget->dwWidth, 128);
                VPHAL_RENDER_CHK_STATUS(VpHal_CommonSetBufferSurfaceForHwAccess(
                    pRenderHal,
                    p16AlignState->pTarget,
                    &p16AlignState->RenderHalTarget,
                    &SurfaceParams,
                    pRenderData->iBindingTable,
                    (i==0)?ALIGN16_TRG_Y_INDEX:ALIGN16_TRG_UV_INDEX,
                    true));

                // add UV offset which was missed in raw buffer common configuration.
                if (i > 0)
                {
                    dwSize          = MOS_ALIGN_CEIL(dwSize, 128);
                    pSurfaceEntry   = &pRenderHal->pStateHeap->pSurfaceEntry[pRenderHal->pStateHeap->iCurrentSurfaceState-1]; // fetch the surface plane
                    pSurfaceEntry->SurfaceToken.DW2.SurfaceOffset = dwSize;
                }
            }
            break;
        case Format_YUY2:
            p16AlignState->pTarget->dwWidth = dwSize * 2;
            p16AlignState->pTarget->dwWidth = MOS_ALIGN_CEIL(p16AlignState->pTarget->dwWidth, 128);
            VPHAL_RENDER_CHK_STATUS(VpHal_CommonSetBufferSurfaceForHwAccess(
                pRenderHal,
                p16AlignState->pTarget,
                &p16AlignState->RenderHalTarget,
                &SurfaceParams,
                pRenderData->iBindingTable,
                ALIGN16_TRG_INDEX,
                true));
            break;
        case Format_YV12:
            // YV12 should be allocated as 3 linear buffer for every Y U V output plane.
            for (int i = 0; i < 3; i++)
            {
                p16AlignState->pTarget->dwWidth = (i == 0)?dwSize:dwSize/4;
                p16AlignState->pTarget->dwWidth = MOS_ALIGN_CEIL(p16AlignState->pTarget->dwWidth, 128);
                VPHAL_RENDER_CHK_STATUS(VpHal_CommonSetBufferSurfaceForHwAccess(
                    pRenderHal,
                    p16AlignState->pTarget,
                    &p16AlignState->RenderHalTarget,
                    &SurfaceParams,
                    pRenderData->iBindingTable,
                    (i==0)?ALIGN16_TRG_Y_INDEX:((i==1)?ALIGN16_TRG_U_INDEX:ALIGN16_TRG_V_INDEX),
                    true));

                // add U, V offset which was missed in raw buffer common configuration.
                // recalculate U, V offset based on 16aligned pitch.
                if (i > 0)
                {
                    dwSize          = MOS_ALIGN_CEIL(dwSize, 128);
                    pSurfaceEntry   = &pRenderHal->pStateHeap->pSurfaceEntry[pRenderHal->pStateHeap->iCurrentSurfaceState-1]; // fetch the surface plane
                    pSurfaceEntry->SurfaceToken.DW2.SurfaceOffset = (i == 1)?(dwSize*5/4):dwSize;
                }
            }
            break;
        default:
            VPHAL_RENDER_ASSERTMESSAGE("16 align output format doesn't support.");
            eStatus = MOS_STATUS_INVALID_PARAMETER;
            break;
    }

    // resotre the target format and width for curbe data.
    p16AlignState->pTarget->Format      = format;
    p16AlignState->pTarget->dwWidth     = width;
#endif

finish:
    VPHAL_RENDER_ASSERT(eStatus == MOS_STATUS_SUCCESS);
    return eStatus;
}

//!
//! \brief    16Align interface Initializations
//! \details  Interface Initializations for 16Align
//! \param    PVPHAL_16_ALIGN_STATE p16AlignState
//!           [in] Pointer to the 16Align State
//! \param    PRENDERHAL_INTERFACE pRenderHal
//!           [in/out] Pointer to RenderHal Interface Structure
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VpHal_16AlignInitInterface(
    PVPHAL_16_ALIGN_STATE    p16AlignState,
    PRENDERHAL_INTERFACE        pRenderHal)
{
    PMOS_INTERFACE                  pOsInterface;
    MOS_STATUS                      eStatus;

    eStatus      = MOS_STATUS_SUCCESS;
    pOsInterface = pRenderHal->pOsInterface;

    // Connect renderer to other VPHAL components (HW/OS interfaces)
    p16AlignState->pRenderHal      = pRenderHal;
    p16AlignState->pOsInterface    = pOsInterface;
    p16AlignState->pSkuTable       = pRenderHal->pSkuTable;

    // Setup functions
    p16AlignState->pfnInitialize         = VpHal_16AlignInitialize;
    p16AlignState->pfnDestroy            = VpHal_16AlignDestroy;
    p16AlignState->pfnRender             = VpHal_16AlignRender;
    p16AlignState->pfnSetupSurfaceStates = VpHal_16AlignSetupSurfaceStates;

    // States
    p16AlignState->bFtrMediaWalker       = 
        p16AlignState->pRenderHal->pfnGetMediaWalkerStatus(p16AlignState->pRenderHal) ? true : false;

    p16AlignState->pfnLoadStaticData     = VpHal_16AlignLoadStaticData;
    p16AlignState->pfnSetupKernel        = VpHal_16AlignSetupKernel;
    p16AlignState->pfnSetSamplerStates   = VpHal_16AlignSetSamplerStates;

    return eStatus;
}

//!
//! \brief    check 16 bytes alignment whether can be processed
//! \details  check 16 bytes alignment whether can be processed
//! \param    PVPHAL_RENDER_PARAMS  pRenderParams
//!           [in] Pointer to VPHAL render parameter
//! \return   bool
//!           Return true if 16 bytes alignment can be processed, otherwise false
//!
bool VpHal_RndrIs16Align(
    PVPHAL_RENDER_PARAMS    pRenderParams)
{
    PVPHAL_SURFACE  pSource;
    PVPHAL_SURFACE  pTarget;
    bool            b16alignment = false;

    pSource = pRenderParams->pSrc[0];
    pTarget = pRenderParams->pTarget[0];

    if (pRenderParams->uSrcCount == 1                           &&
        pRenderParams->uDstCount == 1                           &&
        pRenderParams->pConstriction == nullptr                 &&
        (pSource->pBlendingParams == nullptr                    ||
         (pSource->pBlendingParams != nullptr                   &&
          pSource->pBlendingParams->BlendType == BLEND_NONE))   &&
        pSource->pLumaKeyParams == nullptr                      &&
        pSource->pProcampParams == nullptr                      &&
        pSource->pIEFParams == nullptr                          &&
        pSource->bInterlacedScaling == false                    &&
        pSource->bFieldWeaving == false                         &&
        pSource->pDenoiseParams == nullptr                      &&
        pSource->pColorPipeParams == nullptr                    &&
        !(pSource->pDeinterlaceParams                           &&
          pSource->pDeinterlaceParams->DIMode == DI_MODE_BOB))
    {
        b16alignment = ((pSource->Format == Format_NV12         ||
                         pSource->Format == Format_YUY2         ||
                         pSource->Format == Format_YV12)        && 
                       (pTarget->Format == Format_NV12          || 
                        pTarget->Format == Format_YUY2          ||
                        pTarget->Format == Format_YV12));
        // for YV12 output, input should be YV12.
        if (pTarget->Format == Format_YV12)
        {
            return (pSource->Format == Format_YV12);
        }
    }

    return b16alignment;
}
