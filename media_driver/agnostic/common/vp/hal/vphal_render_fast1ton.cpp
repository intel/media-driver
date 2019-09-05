/*
*
* Copyright (c) Intel Corporation (2018).
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
* File Name  : vphal_render_fast1ton.cpp
*
* Abstract   : Video Surface multi scaling output Video Processing
*
* Environment: linux
*
* Notes      : This module contains video surface multi scaling output definitions
*              for VPHAL
*
*/
//!
//! \file     vphal_render_fast1ton.cpp
//! \brief    Multi scaling output Surface alignment as 16 or non-16 bytes
//! \details  Unified VP HAL multi scaling output Surface 16 or no-16 bytes alignment module interfaces
//!
#include "vphal_render_fast1ton.h"
#include "vphal_debug.h"
#include "vpkrnheader.h"
#include "vphal_render_composite.h"
#include "vphal_render_ief.h"
#include "vphal_renderer.h"

#define AVS_SAMPLE_INDEX0         1
#define AVS_SAMPLE_INDEX1         3
#define AVS_SAMPLE_INDEX2         5

#define FAST1TON_SRC_INDEX        0
// output1
#define FAST1TON_DST_INDEX0       1     // non-16 aligned
#define FAST1TON_DST_Y_INDEX0     1     // 16 aligned
#define FAST1TON_DST_UV_INDEX0    2     // 16 aligned
// output2
#define FAST1TON_DST_INDEX1       3     // non-16 aligned
#define FAST1TON_DST_Y_INDEX1     3     // 16 aligned
#define FAST1TON_DST_UV_INDEX1    4     // 16 aligned
// output3
#define FAST1TON_DST_INDEX2     5     // non-16 aligned
#define FAST1TON_DST_Y_INDEX2     5     // 16 aligned
#define FAST1TON_DST_UV_INDEX2    6     // 16 aligned

#define ALIGN16_DST0      1
#define ALIGN16_DST1      (1<<1)
#define ALIGN16_DST2      (1<<2)
//!
//! \brief fast 1toN Kernel params for Gen9 Media Walker
//!
static const RENDERHAL_KERNEL_PARAM g_fast1toN_MW_KernelParam[1] =
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
    { 4, 34,  3, VPHAL_USE_MEDIA_THREADS_MAX,  0,  4,  16,  16,  1,  1 },    // R8
};

//!
//! \brief    fast 1toN load the curbe data
//! \details  Curbe data for fast 1toN
//! \param    PVPHAL_FAST1TON_STATE pFast1toNState
//!           [in] Pointer to the fast 1toN State
//! \param    PVPHAL_FAST1TON_RENDER_DATA pRenderData
//!           [in] Pointer to fast 1toN render data
//! \param    int32_t* piCurbeOffset
//!           [out] Pointer to curbe data offset
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VpHal_Fast1toNLoadStaticData(
    PVPHAL_FAST1TON_STATE           pFast1toNState,    
    PVPHAL_FAST1TON_RENDER_DATA     pRenderData,
    int32_t*                        piCurbeOffset)
{
    PRENDERHAL_INTERFACE                    pRenderHal;
    MEDIA_WALKER_FAST1TON_STATIC_DATA       WalkerStatic;
    MOS_STATUS                              eStatus;
    int32_t                                 iCurbeLength;

    VPHAL_RENDER_CHK_NULL(pFast1toNState);
    VPHAL_RENDER_CHK_NULL(pFast1toNState->pRenderHal);
    eStatus          = MOS_STATUS_SUCCESS;
    pRenderHal       = pFast1toNState->pRenderHal;

    // Set relevant static data
    MOS_ZeroMemory(&WalkerStatic, sizeof(MEDIA_WALKER_FAST1TON_STATIC_DATA));
    if (pFast1toNState->pTarget[0])
    {
        WalkerStatic.DW0.Sampler_Index0      = AVS_SAMPLE_INDEX0;
        if (pFast1toNState->Aligned16[0])
        {
            WalkerStatic.DW4.Dst_Index0      = FAST1TON_DST_INDEX0;
            WalkerStatic.DW10.Dst_16Aligned |= ALIGN16_DST0;
#if defined(LINUX)
            WalkerStatic.DW10.Dst_pitch0     = pFast1toNState->pTarget[0]->OsResource.iPitch;
            WalkerStatic.DW12.Dst_UVOffset0  = pFast1toNState->pTarget[0]->OsResource.iHeight;
#endif
        }
        else
        {
            WalkerStatic.DW4.Dst_Y_Index0    = FAST1TON_DST_Y_INDEX0;
            WalkerStatic.DW5.Dst_UV_Index0   = FAST1TON_DST_UV_INDEX0;
            WalkerStatic.DW10.Dst_pitch0     = pFast1toNState->pTarget[0]->dwPitch;
            WalkerStatic.DW12.Dst_UVOffset0  = pFast1toNState->pTarget[0]->dwHeight;
        }
        WalkerStatic.DW14.ScalingStep_H0     = pRenderData->ScalingStep_H[0];
        WalkerStatic.DW15.ScalingStep_V0     = pRenderData->ScalingStep_V[0];
    }

    if (pFast1toNState->pTarget[1])
    {
        WalkerStatic.DW1.Sampler_Index1      = AVS_SAMPLE_INDEX1;
        if (pFast1toNState->Aligned16[1])
        {
            WalkerStatic.DW6.Dst_Index1      = FAST1TON_DST_INDEX1;
            WalkerStatic.DW10.Dst_16Aligned |= ALIGN16_DST1;
#if defined(LINUX)
            WalkerStatic.DW11.Dst_pitch1     = pFast1toNState->pTarget[1]->OsResource.iPitch;
            WalkerStatic.DW12.Dst_UVOffset1  = pFast1toNState->pTarget[1]->OsResource.iHeight;
#endif
        }
        else
        {
            WalkerStatic.DW6.Dst_Y_Index1    = FAST1TON_DST_Y_INDEX1;
            WalkerStatic.DW7.Dst_UV_Index1   = FAST1TON_DST_UV_INDEX1;
            WalkerStatic.DW11.Dst_pitch1     = pFast1toNState->pTarget[1]->dwPitch;
            WalkerStatic.DW12.Dst_UVOffset1  = pFast1toNState->pTarget[1]->dwHeight;
        }
        WalkerStatic.DW16.ScalingStep_H1     = pRenderData->ScalingStep_H[1];
        WalkerStatic.DW17.ScalingStep_V1     = pRenderData->ScalingStep_V[1];
    }

    if (pFast1toNState->pTarget[2])
    {
        WalkerStatic.DW2.Sampler_Index2      = AVS_SAMPLE_INDEX2;
        if (pFast1toNState->Aligned16[2])
        {
            WalkerStatic.DW8.Dst_Index2      = FAST1TON_DST_INDEX2;
            WalkerStatic.DW10.Dst_16Aligned |= ALIGN16_DST2;
#if defined(LINUX)
            WalkerStatic.DW11.Dst_pitch2     = pFast1toNState->pTarget[2]->OsResource.iPitch;
            WalkerStatic.DW13.Dst_UVOffset2  = pFast1toNState->pTarget[2]->OsResource.iHeight;
#endif
        }
        else
        {
            WalkerStatic.DW8.Dst_Y_Index2    = FAST1TON_DST_Y_INDEX2;
            WalkerStatic.DW9.Dst_UV_Index2   = FAST1TON_DST_UV_INDEX2;
            WalkerStatic.DW11.Dst_pitch2     = pFast1toNState->pTarget[2]->dwPitch;
            WalkerStatic.DW13.Dst_UVOffset2  = pFast1toNState->pTarget[2]->dwHeight;
        }
        WalkerStatic.DW18.ScalingStep_H2     = pRenderData->ScalingStep_H[2];
        WalkerStatic.DW19.ScalingStep_V2     = pRenderData->ScalingStep_V[2];
    }
    WalkerStatic.DW3.Src_Index               = FAST1TON_SRC_INDEX;

    iCurbeLength = sizeof(MEDIA_WALKER_FAST1TON_STATIC_DATA);

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
//! \brief    fast 1toN kernel setup
//! \details  Kernel setup for bitcopy
//! \param    PVPHAL_FAST1TON_STATE pFast1toNState
//!           [in] Pointer to the fast 1toN State
//! \param    PVPHAL_FAST1TON_RENDER_DATA pRenderData
//!           [in] Pointer to fast 1toN render data
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VpHal_Fast1toNSetupKernel(
    PVPHAL_FAST1TON_STATE        pFast1toNState,
    PVPHAL_FAST1TON_RENDER_DATA  pRenderData)
{
    MOS_STATUS      eStatus;
    Kdll_CacheEntry *pCacheEntryTable;

    VPHAL_RENDER_CHK_NULL(pFast1toNState);
    eStatus             = MOS_STATUS_SUCCESS;
    pCacheEntryTable    = 
        pFast1toNState->pKernelDllState->ComponentKernelCache.pCacheEntries;

    // Set the Kernel Parameters
    pRenderData->pKernelParam   = pFast1toNState->pKernelParamTable;
    pRenderData->PerfTag        = VPHAL_NONE;

    // Set curbe & inline data size
    pRenderData->iCurbeLength   = pRenderData->pKernelParam->CURBE_Length * GRF_SIZE;

    // Set Kernel entry
    pRenderData->KernelEntry.iKUID     = IDR_VP_fast_avs_1_to_n;
    pRenderData->KernelEntry.iKCID     = -1;
    pRenderData->KernelEntry.iSize     = pCacheEntryTable[IDR_VP_fast_avs_1_to_n].iSize;
    pRenderData->KernelEntry.pBinary   = pCacheEntryTable[IDR_VP_fast_avs_1_to_n].pBinary;

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
static MOS_STATUS VpHal_Fast1toNSamplerAvsCalcScalingTable(
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
static MOS_STATUS VpHal_Fast1toNSetSamplerAvsTableParam(
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
    VPHAL_HW_CHK_STATUS(VpHal_Fast1toNSamplerAvsCalcScalingTable(
        SrcFormat,
        fScaleX,
        false,
        dwChromaSiting,
        bBalancedFilter,
        pMhwSamplerAvsTableParam->b8TapAdaptiveEnable ? true : false,
        pAvsParams));

    // Recalculate Vertical scaling table
    VPHAL_HW_CHK_STATUS(VpHal_Fast1toNSamplerAvsCalcScalingTable(
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
//! \brief    fast 1toN setup HW states
//! \details  Setup HW states for fast 1toN
//! \param    PVPHAL_FAST1TON_STATE pFast1toNState
//!           [in] Pointer to the fast 1toN State
//! \param    PVPHAL_FAST1TON_RENDER_DATA pRenderData
//!           [in/out] Pointer to fast 1toN render data
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VpHal_Fast1toNSetSamplerStates(
    PVPHAL_FAST1TON_STATE        pFast1toNState,
    PVPHAL_FAST1TON_RENDER_DATA  pRenderData)
{
    MOS_STATUS                  eStatus;
    PRENDERHAL_INTERFACE        pRenderHal;
    PMHW_SAMPLER_STATE_PARAM    pSamplerStateParams;
    uint32_t                    index;

    VPHAL_PUBLIC_CHK_NULL(pFast1toNState);
    VPHAL_PUBLIC_CHK_NULL(pRenderData);

    pRenderHal = pFast1toNState->pRenderHal;
    VPHAL_PUBLIC_CHK_NULL(pRenderHal);

    for (index = 0; index < pFast1toNState->uDstCount; index++)
    {
        pSamplerStateParams                          = &pRenderData->SamplerStateParams[index];
        pSamplerStateParams->bInUse                  = true;
        pSamplerStateParams->SamplerType             = MHW_SAMPLER_TYPE_AVS;
        pSamplerStateParams->Avs.bHdcDwEnable        = true;
        pSamplerStateParams->Avs.b8TapAdaptiveEnable = false;
        pSamplerStateParams->Avs.bEnableAVS          = true;
        pSamplerStateParams->Avs.WeakEdgeThr         = DETAIL_WEAK_EDGE_THRESHOLD;
        pSamplerStateParams->Avs.StrongEdgeThr       = DETAIL_STRONG_EDGE_THRESHOLD;
        pSamplerStateParams->Avs.StrongEdgeWght      = DETAIL_STRONG_EDGE_WEIGHT;
        pSamplerStateParams->Avs.RegularWght         = DETAIL_REGULAR_EDGE_WEIGHT;
        pSamplerStateParams->Avs.NonEdgeWght         = DETAIL_NON_EDGE_WEIGHT;
        pSamplerStateParams->Unorm.SamplerFilterMode      = MHW_SAMPLER_FILTER_NEAREST;
        pSamplerStateParams->Avs.pMhwSamplerAvsTableParam = &pFast1toNState->mhwSamplerAvsTableParam[index];

        VPHAL_RENDER_CHK_STATUS(VpHal_Fast1toNSetSamplerAvsTableParam(
                        pRenderHal,
                        pSamplerStateParams,
                        pRenderData->pAVSParameters[index],
                        pFast1toNState->pSource->Format,
                        pRenderData->ScalingRatio_H[index],
                        pRenderData->ScalingRatio_V[index],
                        MHW_CHROMA_SITING_HORZ_LEFT | MHW_CHROMA_SITING_VERT_TOP));
    }

    eStatus = pRenderHal->pfnSetSamplerStates(
        pRenderHal,
        pRenderData->iMediaID,
        &pRenderData->SamplerStateParams[0],
        pFast1toNState->uDstCount);

finish:
    return eStatus;
}

//!
//! \brief    fast 1toN setup HW states
//! \details  Setup HW states for fast 1toN
//! \param    PVPHAL_FAST1TON_STATE pFast1toNState
//!           [in] Pointer to the fast 1toN State
//! \param    PVPHAL_FAST1TON_RENDER_DATA pRenderData
//!           [in/out] Pointer to fast 1toN render data
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VpHal_Fast1toNSetupHwStates(
    PVPHAL_FAST1TON_STATE        pFast1toNState,
    PVPHAL_FAST1TON_RENDER_DATA  pRenderData)
{
    PRENDERHAL_INTERFACE        pRenderHal;
    int32_t                     iKrnAllocation;
    int32_t                     iCurbeOffset;
    MOS_STATUS                  eStatus;
    int32_t                     iThreadCount;
    MHW_KERNEL_PARAM            MhwKernelParam;

    VPHAL_RENDER_CHK_NULL(pFast1toNState);
    VPHAL_RENDER_CHK_NULL(pRenderData);

    eStatus                     = MOS_STATUS_SUCCESS;
    pRenderHal                  = pFast1toNState->pRenderHal;
    VPHAL_RENDER_CHK_NULL(pRenderHal);

    // Allocate and reset media state
    pRenderData->pMediaState = pRenderHal->pfnAssignMediaState(pRenderHal, (RENDERHAL_COMPONENT)RENDERHAL_COMPONENT_FAST1TON);
    VPHAL_RENDER_CHK_NULL(pRenderData->pMediaState);

    // Allocate and reset SSH instance
    VPHAL_RENDER_CHK_STATUS(pRenderHal->pfnAssignSshInstance(pRenderHal));

    // Assign and Reset Binding Table
    VPHAL_RENDER_CHK_STATUS(pRenderHal->pfnAssignBindingTable(
            pRenderHal,
            &pRenderData->iBindingTable));

    // Setup surface states
    VPHAL_RENDER_CHK_STATUS(pFast1toNState->pfnSetupSurfaceStates(
            pFast1toNState,
            pRenderData));

    // load static data
    VPHAL_RENDER_CHK_STATUS(pFast1toNState->pfnLoadStaticData(
            pFast1toNState,
            pRenderData,
            &iCurbeOffset));

    if (pFast1toNState->pPerfData->CompMaxThreads.bEnabled)
    {
        iThreadCount = pFast1toNState->pPerfData->CompMaxThreads.uiVal;
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
    VPHAL_RENDER_CHK_STATUS(pFast1toNState->pfnSetSamplerStates(
        pFast1toNState, 
        pRenderData));

finish:
    VPHAL_RENDER_ASSERT(eStatus == MOS_STATUS_SUCCESS);
    return eStatus;
}

//!
//! \brief    fast 1toN media walker setup
//! \details  Media walker setup for bitcopy
//! \param    PVPHAL_FAST1TON_STATE pFast1toNState
//!           [in] Pointer to the fast 1toN State
//! \param    PVPHAL_FAST1TON_RENDER_DATA pRenderData
//!           [in] Pointer to fast 1toN render data
//! \param    PMHW_WALKER_PARAMS pWalkerParams
//!           [in/out] Pointer to Walker params
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VpHal_Fast1toNRenderMediaWalker(
    PVPHAL_FAST1TON_STATE        pFast1toNState,    
    PVPHAL_FAST1TON_RENDER_DATA  pRenderData,
    PMHW_WALKER_PARAMS               pWalkerParams)
{
    PRENDERHAL_INTERFACE            pRenderHal;
    int32_t                         dwWidth  = 0;
    int32_t                         dwHeight = 0;
    MOS_STATUS                      eStatus;

    eStatus     = MOS_STATUS_SUCCESS;
    pRenderHal  = pFast1toNState->pRenderHal;

    // Calculate how many media object commands are needed.
    // Using the Max output size to calculate the iBlock.
    for (uint32_t i = 0; i < pFast1toNState->uDstCount; i++)
    {
        dwWidth  = MOS_MAX(MOS_ALIGN_CEIL((pFast1toNState->pTarget[i]->rcSrc.right  - 
                           pFast1toNState->pTarget[i]->rcSrc.left), 
                           pRenderData->pKernelParam->block_width), dwWidth);
        dwHeight = MOS_MAX(MOS_ALIGN_CEIL((pFast1toNState->pTarget[i]->rcSrc.bottom - 
                       pFast1toNState->pTarget[i]->rcSrc.top),  
                       pRenderData->pKernelParam->block_height), dwHeight);
    }

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
//! \brief    fast 1toN renderer
//! \details  Renderer function for fast 1toN
//! \param    PVPHAL_FAST1TON_STATE pFast1toNState
//!           [in] Pointer to the fast 1toN State
//! \param    PVPHAL_RENDER_PARAMS pRenderParams
//!           [in] Pointer to fast 1toN render params
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VpHal_Fast1toNRender(
    PVPHAL_FAST1TON_STATE    pFast1toNState,
    PVPHAL_RENDER_PARAMS     pRenderParams)
{
    MOS_STATUS                              eStatus;
    PRENDERHAL_INTERFACE                    pRenderHal;
    PMOS_INTERFACE                          pOsInterface;
    MHW_WALKER_PARAMS                       WalkerParams;
    VPHAL_FAST1TON_RENDER_DATA              RenderData;
    PRENDERHAL_L3_CACHE_SETTINGS            pCacheSettings = nullptr;
    uint32_t                                dwInputRegionHeight;
    uint32_t                                dwInputRegionWidth;
    uint32_t                                dwOutputRegionHeight;
    uint32_t                                dwOutputRegionWidth;
    uint32_t                                index;

    VPHAL_RENDER_CHK_NULL(pFast1toNState);
    VPHAL_RENDER_CHK_NULL(pRenderParams);
    VPHAL_RENDER_CHK_NULL(pFast1toNState->pOsInterface);
    VPHAL_RENDER_CHK_NULL(pFast1toNState->pRenderHal);
    VPHAL_RENDER_CHK_NULL(pFast1toNState->pPerfData);

    eStatus                     = MOS_STATUS_SUCCESS;
    pOsInterface                = pFast1toNState->pOsInterface;
    pRenderHal                  = pFast1toNState->pRenderHal;
    MOS_ZeroMemory(&RenderData, sizeof(RenderData));

    // Reset reporting
    pFast1toNState->Reporting.InitReportValue();

    // Reset states before rendering
    pOsInterface->pfnResetOsStates(pOsInterface);
    VPHAL_RENDER_CHK_STATUS(pRenderHal->pfnReset(pRenderHal));
    pOsInterface->pfnResetPerfBufferID(pOsInterface);   // reset once per frame
    for (index = 0; index < MAX_1TON_SUPPORT; index++)
    {
        pFast1toNState->pTarget[index]   = nullptr;
        pFast1toNState->Aligned16[index] = 0;
    }

    VPHAL_DBG_STATE_DUMPPER_SET_CURRENT_STAGE(VPHAL_DBG_STAGE_COMP);

    // Configure cache settings for this render operation 
    pCacheSettings      = &pRenderHal->L3CacheSettings;
    MOS_ZeroMemory(pCacheSettings, sizeof(*pCacheSettings));
    pCacheSettings->bOverride                  = true;
    pCacheSettings->bL3CachingEnabled          = pFast1toNState->SurfMemObjCtl.bL3CachingEnabled;

    if (pFast1toNState->pPerfData->L3SQCReg1Override.bEnabled)
    {
        pCacheSettings->bSqcReg1Override       = true;
        pCacheSettings->dwSqcReg1              = pFast1toNState->pPerfData->L3SQCReg1Override.uiVal;
    }

    if (pFast1toNState->pPerfData->L3CntlReg2Override.bEnabled)
    {
        pCacheSettings->bCntlReg2Override      = true;
        pCacheSettings->dwCntlReg2             = pFast1toNState->pPerfData->L3CntlReg2Override.uiVal;
    }

    if (pFast1toNState->pPerfData->L3CntlReg3Override.bEnabled)
    {
        pCacheSettings->bCntlReg3Override      = true;
        pCacheSettings->dwCntlReg3             = pFast1toNState->pPerfData->L3CntlReg3Override.uiVal;
    }

    if (pFast1toNState->pPerfData->L3LRA1RegOverride.bEnabled)
    {
        pCacheSettings->bLra1RegOverride       = true;
        pCacheSettings->dwLra1Reg              = pFast1toNState->pPerfData->L3LRA1RegOverride.uiVal;
    }

    // Setup Source/Target surface and get the Source width/height
    pFast1toNState->pSource                 = pRenderParams->pSrc[0];
    dwInputRegionWidth                      = pFast1toNState->pSource->rcSrc.right  - pFast1toNState->pSource->rcSrc.left;
    dwInputRegionHeight                     = pFast1toNState->pSource->rcSrc.bottom - pFast1toNState->pSource->rcSrc.top;
    pFast1toNState->uDstCount               = pRenderParams->uDstCount;
    for (index = 0; index < pFast1toNState->uDstCount; index++)
    {
        pFast1toNState->pTarget[index]      = pRenderParams->pTarget[index];
        pFast1toNState->Aligned16[index]    = pRenderParams->pTarget[index]->b16UsrPtr;
        dwOutputRegionWidth                 = pFast1toNState->pTarget[index]->rcSrc.right  - pFast1toNState->pTarget[index]->rcSrc.left;
        dwOutputRegionHeight                = pFast1toNState->pTarget[index]->rcSrc.bottom - pFast1toNState->pTarget[index]->rcSrc.top;
        RenderData.ScalingStep_H[index]     = (float)1.0 / (float)dwOutputRegionWidth;
        RenderData.ScalingStep_V[index]     = (float)1.0 / (float)dwOutputRegionHeight;
        RenderData.ScalingRatio_H[index]    = (float)dwOutputRegionWidth / (float)dwInputRegionWidth;
        RenderData.ScalingRatio_V[index]    = (float)dwOutputRegionHeight / (float)dwInputRegionHeight;
        RenderData.pAVSParameters[index]    = &pFast1toNState->AVSParameters[index];
        RenderData.SamplerStateParams[index].Avs.pMhwSamplerAvsTableParam = &RenderData.mhwSamplerAvsTableParam[index];
    }

    // Ensure input can be read
    pOsInterface->pfnSyncOnResource(
        pOsInterface, 
        &pFast1toNState->pSource->OsResource,
        pOsInterface->CurrentGpuContextOrdinal,
        false);

    // Ensure the output can be written
    for (index = 0; index < pFast1toNState->uDstCount; index++)
    {
        pOsInterface->pfnSyncOnResource(
            pOsInterface, 
            &pFast1toNState->pTarget[index]->OsResource,
            pOsInterface->CurrentGpuContextOrdinal,
            true);
    }

    // Setup copy kernel
    VPHAL_RENDER_CHK_STATUS(pFast1toNState->pfnSetupKernel(
            pFast1toNState,
            &RenderData));

    // Submit HW States and Commands
    VPHAL_RENDER_CHK_STATUS(VpHal_Fast1toNSetupHwStates(
            pFast1toNState, 
            &RenderData));

    // Set perftag information
    pOsInterface->pfnResetPerfBufferID(pOsInterface);
    pOsInterface->pfnSetPerfTag(pOsInterface, RenderData.PerfTag);

    VPHAL_RENDER_CHK_STATUS(VpHal_Fast1toNRenderMediaWalker(
            pFast1toNState,    
            &RenderData,
            &WalkerParams));

    VPHAL_DBG_STATE_DUMPPER_DUMP_GSH(pRenderHal);
    VPHAL_DBG_STATE_DUMPPER_DUMP_SSH(pRenderHal);

    VPHAL_RENDER_CHK_STATUS(VpHal_RndrSubmitCommands(
        pRenderHal, 
        nullptr,
        pFast1toNState->bNullHwRenderfast1toN,
        &WalkerParams,
        nullptr,
        &pFast1toNState->StatusTableUpdateParams,
        kernelFast1toN,
        0,
        nullptr,
        true));

finish:
    MOS_ZeroMemory(pCacheSettings, sizeof(*pCacheSettings));
    VPHAL_RENDER_ASSERT(eStatus == MOS_STATUS_SUCCESS);
    return eStatus;
}

//!
//! \brief    fast 1toN Destroy state
//! \details  Function to destroy fast 1toN state
//! \param    PVPHAL_FAST1TON_STATE pFast1toNState
//!           [in] Pointer to the fast 1toN State
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VpHal_Fast1toNDestroy(
    PVPHAL_FAST1TON_STATE    pFast1toNState)
{
    MOS_STATUS  eStatus;
    uint32_t    index;
    eStatus = MOS_STATUS_SUCCESS;
    VPHAL_RENDER_CHK_NULL(pFast1toNState);
    for (index = 0; index < MAX_1TON_SUPPORT; index++)
    {
        VpHal_RenderDestroyAVSParams(&pFast1toNState->AVSParameters[index]);
    }
    MOS_UNUSED(pFast1toNState);

finish:
    return eStatus;
}

//!
//! \brief    fast 1toN kernel state Initializations
//! \details  Kernel state Initializations for fast 1toN
//! \param    PVPHAL_FAST1TON_STATE pFast1toNState
//!           [in] Pointer to the fast 1toN State
//! \param    const VphalSettings* pSettings
//!           [in] Pointer to VPHAL Setting
//! \param    Kdll_State pKernelDllState
//!           [in/out] Pointer to bitcopy kernel Dll state
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VpHal_Fast1toNInitialize(
    PVPHAL_FAST1TON_STATE    pFast1toNState,
    const VphalSettings      *pSettings,
    Kdll_State               *pKernelDllState)
{
    MOS_NULL_RENDERING_FLAGS  NullRenderingFlags;
    uint32_t                  index;

    VPHAL_RENDER_ASSERT(pFast1toNState);
    VPHAL_RENDER_ASSERT(pFast1toNState->pOsInterface);

    NullRenderingFlags            = 
                    pFast1toNState->pOsInterface->pfnGetNullHWRenderFlags(pFast1toNState->pOsInterface);
    pFast1toNState->bNullHwRenderfast1toN =
                    NullRenderingFlags.VPLgca ||
                    NullRenderingFlags.VPGobal;

    // Setup interface to KDLL
    pFast1toNState->pKernelDllState   = pKernelDllState;
    for (index = 0; index < MAX_1TON_SUPPORT; index++)
    {
        VpHal_RenderInitAVSParams(&pFast1toNState->AVSParameters[index],
                POLYPHASE_Y_COEFFICIENT_TABLE_SIZE_G9,
                POLYPHASE_UV_COEFFICIENT_TABLE_SIZE_G9);
    }

    return MOS_STATUS_SUCCESS;
}

//!
//! \brief    fast 1toN setup surface states
//! \details  Setup surface states for fast 1toN
//! \param    PVPHAL_FAST1TON_STATE pFast1toNState
//!           [in] Pointer to the fast 1toN State
//! \param    PVPHAL_FAST1TON_RENDER_DATA pRenderData
//!           [in] Pointer to fast 1toN render data
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VpHal_Fast1toNSetupSurfaceStates(
    PVPHAL_FAST1TON_STATE        pFast1toNState,    
    PVPHAL_FAST1TON_RENDER_DATA  pRenderData)
{
    PRENDERHAL_INTERFACE            pRenderHal;
    RENDERHAL_SURFACE_STATE_PARAMS  SurfaceParams;
    MOS_STATUS                      eStatus;
    uint32_t                        index;
    uint32_t                        width  = 0;
    MOS_FORMAT                      format = Format_NV12;
    int32_t                         iBTEntry;
    eStatus             = MOS_STATUS_SUCCESS;
    pRenderHal          = pFast1toNState->pRenderHal;

    // Source surface
    MOS_ZeroMemory(&SurfaceParams, sizeof(SurfaceParams));

    SurfaceParams.bAVS              = true;
    SurfaceParams.Boundary          = RENDERHAL_SS_BOUNDARY_SRCRECT;
    SurfaceParams.bRenderTarget     = false;
    SurfaceParams.MemObjCtl         = 
        pFast1toNState->SurfMemObjCtl.SourceSurfMemObjCtl;
    SurfaceParams.Type              = RENDERHAL_SURFACE_TYPE_ADV_G9;
    SurfaceParams.bWidthInDword_Y   = false;
    SurfaceParams.bWidthInDword_UV  = false;
    SurfaceParams.bWidth16Align     = false;
    
    VPHAL_RENDER_CHK_STATUS(VpHal_CommonSetSurfaceForHwAccess(
                pRenderHal,
                pFast1toNState->pSource, 
                &pFast1toNState->RenderHalSource, 
                &SurfaceParams, 
                pRenderData->iBindingTable, 
                FAST1TON_SRC_INDEX,
                false));

    // Target surface
    SurfaceParams.MemObjCtl         = 
        pFast1toNState->SurfMemObjCtl.TargetSurfMemObjCtl;
    SurfaceParams.Type              = pRenderHal->SurfaceTypeDefault;
    SurfaceParams.bRenderTarget     = true;
    SurfaceParams.bAVS              = false;
    SurfaceParams.Boundary          = RENDERHAL_SS_BOUNDARY_DSTRECT;

    for (index = 0; index < pFast1toNState->uDstCount; index++)
    {
        if (pFast1toNState->Aligned16[index])
        {
            // store and recalculate the target format and width
            format   = pFast1toNState->pTarget[index]->Format;
            width    = pFast1toNState->pTarget[index]->dwWidth;
            pFast1toNState->pTarget[index]->Format  = Format_RAW;
#if defined(LINUX)
            pFast1toNState->pTarget[index]->dwWidth = (pFast1toNState->pTarget[index]->dwHeight * pFast1toNState->pTarget[index]->OsResource.iPitch) * 3/2;
#endif
            pFast1toNState->pTarget[index]->dwWidth = MOS_ALIGN_CEIL(pFast1toNState->pTarget[index]->dwWidth, 128);
            iBTEntry = (index == 0)?FAST1TON_DST_INDEX0:((index == 1)?FAST1TON_DST_INDEX1:FAST1TON_DST_INDEX2);

            VPHAL_RENDER_CHK_STATUS(VpHal_CommonSetBufferSurfaceForHwAccess(
                        pRenderHal,
                        pFast1toNState->pTarget[index], 
                        &pFast1toNState->RenderHalTarget[index], 
                        &SurfaceParams, 
                        pRenderData->iBindingTable, 
                        iBTEntry,
                        true));

            // restore the target format and width
            pFast1toNState->pTarget[index]->Format  = format;
            pFast1toNState->pTarget[index]->dwWidth = width;
        }
        else
        {
            iBTEntry = (index == 0)?FAST1TON_DST_Y_INDEX0:((index == 1)?FAST1TON_DST_Y_INDEX1:FAST1TON_DST_Y_INDEX2);
            VPHAL_RENDER_CHK_STATUS(VpHal_CommonSetSurfaceForHwAccess(
                        pRenderHal,
                        pFast1toNState->pTarget[index], 
                        &pFast1toNState->RenderHalTarget[index], 
                        &SurfaceParams, 
                        pRenderData->iBindingTable, 
                        iBTEntry,
                        true));
        }
    }

finish:
    VPHAL_RENDER_ASSERT(eStatus == MOS_STATUS_SUCCESS);
    return eStatus;
}

//!
//! \brief    fast 1toN interface Initializations
//! \details  Interface Initializations for fast 1toN
//! \param    PVPHAL_FAST1TON_STATE pFast1toNState
//!           [in] Pointer to the fast 1toN State
//! \param    PRENDERHAL_INTERFACE pRenderHal
//!           [in/out] Pointer to RenderHal Interface Structure
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VpHal_Fast1toNInitInterface(
    PVPHAL_FAST1TON_STATE    pFast1toNState,
    PRENDERHAL_INTERFACE        pRenderHal)
{
    PMOS_INTERFACE                  pOsInterface;
    MOS_STATUS                      eStatus;

    eStatus      = MOS_STATUS_SUCCESS;
    pOsInterface = pRenderHal->pOsInterface;

    // Connect renderer to other VPHAL components (HW/OS interfaces)
    pFast1toNState->pRenderHal      = pRenderHal;
    pFast1toNState->pOsInterface    = pOsInterface;
    pFast1toNState->pSkuTable       = pRenderHal->pSkuTable;

    // Setup functions
    pFast1toNState->pfnInitialize         = VpHal_Fast1toNInitialize;
    pFast1toNState->pfnDestroy            = VpHal_Fast1toNDestroy;
    pFast1toNState->pfnRender             = VpHal_Fast1toNRender;
    pFast1toNState->pfnSetupSurfaceStates = VpHal_Fast1toNSetupSurfaceStates;

    // States
    pFast1toNState->pKernelParamTable     = (PRENDERHAL_KERNEL_PARAM)g_fast1toN_MW_KernelParam;
    pFast1toNState->bFtrMediaWalker       = 
        pFast1toNState->pRenderHal->pfnGetMediaWalkerStatus(pFast1toNState->pRenderHal) ? true : false;

    pFast1toNState->pfnLoadStaticData     = VpHal_Fast1toNLoadStaticData;
    pFast1toNState->pfnSetupKernel        = VpHal_Fast1toNSetupKernel;
    pFast1toNState->pfnSetSamplerStates   = VpHal_Fast1toNSetSamplerStates;

    return eStatus;
}

//!
//! \brief    check if intput/output is a fast 1toN case
//! \param    [in] pRenderParams
//!           Pointer to VPHAL render parameter
//! \param    [in] pSrcSurface
//!           Pointer to input surface.
//! \return   ture if this case match fas 1toN condition, otherwise return fasle.
//!
bool VpHal_RndrIsFast1toNSupport(
    PVPHAL_FAST1TON_STATE   pFast1toNState,
    PVPHAL_RENDER_PARAMS    pRenderParams,
    PVPHAL_SURFACE          pSrcSurface)
{
    bool        ret = true;
    uint32_t    uiIndex_out;
    float       ScalingRatio_H;
    float       ScalingRatio_V;

    if (!GFX_IS_RENDERCORE(pFast1toNState->pRenderHal->Platform, IGFX_GEN9_CORE))
    {
        ret = false;
        goto finish;
    }

    // fast 1toN only support scaling now.
    if (pRenderParams->pConstriction != nullptr              ||
        pSrcSurface->pLumaKeyParams  != nullptr              ||
        pSrcSurface->pProcampParams  != nullptr              ||
        pSrcSurface->pIEFParams      != nullptr              ||
        pSrcSurface->bInterlacedScaling                      ||
        pSrcSurface->bFieldWeaving                           ||
        pSrcSurface->pDenoiseParams != nullptr               ||
        pSrcSurface->pColorPipeParams != nullptr             ||
        pSrcSurface->pDeinterlaceParams != nullptr           ||
        !(pSrcSurface->pBlendingParams == nullptr            ||
         (pSrcSurface->pBlendingParams != nullptr            &&
          pSrcSurface->pBlendingParams->BlendType == BLEND_NONE)))
    {
        ret = false;
        goto finish;
    }
    if ((pSrcSurface->Format != Format_NV12) || (pRenderParams->uDstCount > MAX_1TON_SUPPORT)
            || (pRenderParams->uDstCount < 2))
    {
        ret = false;
        goto finish;
    }
    for (uiIndex_out = 0; uiIndex_out < pRenderParams->uDstCount; uiIndex_out++)
    {
        if (pRenderParams->pTarget[uiIndex_out]->Format != Format_NV12)
        {
            ret = false;
            goto finish;
        }
        // check scaling ratio
        ScalingRatio_H = (float)pRenderParams->pTarget[uiIndex_out]->dwHeight/(float)(pSrcSurface->rcSrc.bottom - pSrcSurface->rcSrc.top);
        ScalingRatio_V = (float)pRenderParams->pTarget[uiIndex_out]->dwWidth/(float)(pSrcSurface->rcSrc.right - pSrcSurface->rcSrc.left);
        if (ScalingRatio_H < 0.0625f || ScalingRatio_V < 0.0625f)
        {
            ret = false;
            goto finish;
        }
    }

finish:
    return ret;
}


