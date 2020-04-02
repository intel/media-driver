/*
* Copyright (c) 2009-2018, Intel Corporation
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
//! \file     vphal_ddi.c
//! \brief    VPHAL related utility functions that are needed in DDI layer
//! \details  Common utility functions for different DDI layers
//!
#include "vphal_ddi.h"
#include "vphal.h"

//!
//! \brief    Delete surface at DDI layer
//! \details  Free different parameter structures in surfaces and free surfaces
//!           at DDI layer, e.g. Sourc/Target/Bwd/Fwd surface
//! \param    [in,out] pSurf
//!           VPHAL surface pointer
//! \return   void
//!
void VpHal_DdiDeleteSurface(PVPHAL_SURFACE pSurf)
{
    if (pSurf)
    {
        if (pSurf->pBwdRef)
        {
            VpHal_DdiDeleteSurface(pSurf->pBwdRef);
        }

        if (pSurf->pFwdRef)
        {
            VpHal_DdiDeleteSurface(pSurf->pFwdRef);
        }

        MOS_SafeFreeMemory(pSurf->Palette.pPalette8);

        MOS_SafeFreeMemory(pSurf->pBlendingParams);

        MOS_SafeFreeMemory(pSurf->pLumaKeyParams);

        if (pSurf->pIEFParams)
        {
            MOS_SafeFreeMemory(pSurf->pIEFParams->pExtParam);
            MOS_SafeFreeMemory(pSurf->pIEFParams);
        }

        MOS_SafeFreeMemory(pSurf->pProcampParams);

        MOS_SafeFreeMemory(pSurf->pDeinterlaceParams);

        MOS_SafeFreeMemory(pSurf->pDenoiseParams);

        MOS_SafeFreeMemory(pSurf->pColorPipeParams);

        MOS_SafeFreeMemory(pSurf);
    }
}

//!
//! \brief    Destroy VPHAL rendering parameters
//! \details  Free source/target surface and other parameters
//! \param    [in,out] pRenderParams
//!           Render parameter pointer
//! \return   void
//!
void VpHal_DdiReleaseRenderParams(PVPHAL_RENDER_PARAMS pRenderParams)
{
    int i;

    MOS_SafeFreeMemory(pRenderParams->pColorFillParams);

    MOS_SafeFreeMemory(pRenderParams->pCompAlpha);

    MOS_SafeFreeMemory(pRenderParams->pConstriction);

    for (i = 0; i < VPHAL_MAX_SOURCES; i++)
    {
        if (pRenderParams->pSrc[i])
        {
            VpHal_DdiDeleteSurface(pRenderParams->pSrc[i]);
        }
    }

    for (i = 0; i < VPHAL_MAX_TARGETS; i++)
    {
        if (pRenderParams->pTarget[i])
        {
            VpHal_DdiDeleteSurface(pRenderParams->pTarget[i]);
        }
    }
}

//!
//! \brief    Judge whether the input procamp value is default or not
//! \details  If the procamp values requested are outside one step of the 
//!           default value(to handle precision errors), then return true
//! \param    [in] ProcAmpParameters
//!           ProcAmp Parameters
//! \return   bool
//!           - true  The input procamp value is not default
//!           - false The input procamp value is default
//!
bool VpHal_DdiProcAmpValuesNotDefault(VPHAL_PROCAMP_PARAMS ProcAmpParameters)
{
    if (OUT_OF_BOUNDS(
        ProcAmpParameters.fBrightness,
        PROCAMP_BRIGHTNESS_DEFAULT - PROCAMP_BRIGHTNESS_STEP,
        PROCAMP_BRIGHTNESS_DEFAULT + PROCAMP_BRIGHTNESS_STEP)  ||
        OUT_OF_BOUNDS(
        ProcAmpParameters.fContrast,
        PROCAMP_CONTRAST_DEFAULT - PROCAMP_CONTRAST_STEP,
        PROCAMP_CONTRAST_DEFAULT + PROCAMP_CONTRAST_STEP)      ||
        OUT_OF_BOUNDS(
        ProcAmpParameters.fHue,
        PROCAMP_HUE_DEFAULT - PROCAMP_HUE_STEP,
        PROCAMP_HUE_DEFAULT + PROCAMP_HUE_STEP)                ||
        OUT_OF_BOUNDS(
        ProcAmpParameters.fSaturation,
        PROCAMP_SATURATION_DEFAULT - PROCAMP_SATURATION_STEP,
        PROCAMP_SATURATION_DEFAULT + PROCAMP_SATURATION_STEP))
    {
        return true;
    }
    return false;
}

//!
//! \brief    Report mode of different features
//! \details  Report DI/Scaling/OutputPipe/FRC mode
//! \param    [in] pVpHalState
//!           VPHAL state pointer
//! \param    [in,out] pConfigValues
//!           Porinter to configuration report value structure,
//!           feature modes will be store in this structure.
//! \return   void
//!
void VpHal_DdiReportFeatureMode(
    VphalState          *pVpHalState,
    PVP_CONFIG          pConfigValues)
{
    VphalFeatureReport* pReport;

    // Get VPHAL feature reporting
    pReport = pVpHalState->GetRenderFeatureReport();
    VPHAL_PUBLIC_CHK_NULL_NO_STATUS_RETURN(pReport);

    // Report DI mode
    switch (pReport->DeinterlaceMode)
    {
        case VPHAL_DI_REPORT_BOB         :
        case VPHAL_DI_REPORT_ADI_BOB     :
            pConfigValues->dwCurrentDeinterlaceMode = VPDDI_BOB;
            break;
        case VPHAL_DI_REPORT_ADI         :
        case VPHAL_DI_REPORT_FMD         :
            pConfigValues->dwCurrentDeinterlaceMode = VPDDI_ADI;
            break;
        case VPHAL_DI_REPORT_PROGRESSIVE :
        default:
            pConfigValues->dwCurrentDeinterlaceMode = VPDDI_PROGRESSIVE;
            break;
    }

    // Report Scaling mode
    pConfigValues->dwCurrentScalingMode =
        (pReport->ScalingMode == VPHAL_SCALING_AVS) ? VPDDI_ADVANCEDSCALING :
            (pReport->ScalingMode > VPHAL_SCALING_AVS) ? VPDDI_SUPERRESOLUTIONSCALING : VPDDI_SCALING;

    // Report Output Pipe
    pConfigValues->dwCurrentOutputPipeMode = pReport->OutputPipeMode;

    // Report VE Feature In Use
    pConfigValues->dwCurrentVEFeatureInUse = pReport->VEFeatureInUse;

    // Report MMC status
    pConfigValues->dwVPMMCInUse              = pReport->VPMMCInUse;
    pConfigValues->dwRTCompressible          = pReport->RTCompressible;
    pConfigValues->dwRTCompressMode          = pReport->RTCompressMode;
    pConfigValues->dwFFDICompressible        = pReport->FFDICompressible;
    pConfigValues->dwFFDICompressMode        = pReport->FFDICompressMode;
    pConfigValues->dwFFDNCompressible        = pReport->FFDNCompressible;
    pConfigValues->dwFFDNCompressMode        = pReport->FFDNCompressMode;
    pConfigValues->dwSTMMCompressible        = pReport->STMMCompressible;
    pConfigValues->dwSTMMCompressMode        = pReport->STMMCompressMode;
    pConfigValues->dwScalerCompressible      = pReport->ScalerCompressible;
    pConfigValues->dwScalerCompressMode      = pReport->ScalerCompressMode;
    pConfigValues->dwPrimaryCompressible     = pReport->PrimaryCompressible;
    pConfigValues->dwPrimaryCompressMode     = pReport->PrimaryCompressMode;

    // Report In Place Compositon status
    pConfigValues->dwCurrentCompositionMode = pReport->CompositionMode;
    pConfigValues->dwCurrentScdMode         = pReport->DiScdMode;

    VP_DDI_NORMALMESSAGE("VP Feature Report: \
        OutputPipeMode %d, \
        VEFeatureInUse %d, \
        ScalingMode %d, \
        DeinterlaceMode %d, \
        VPMMCInUse %d, \
        RTCompressible %d, \
        RTCompressMode %d, \
        PrimaryCompressible %d, \
        PrimaryCompressMode %d, \
        CompositionMode %d",
        pReport->OutputPipeMode,
        pReport->VEFeatureInUse,
        pReport->ScalingMode,
        pReport->DeinterlaceMode,
        pReport->VPMMCInUse,
        pReport->RTCompressible,
        pReport->RTCompressMode,
        pReport->PrimaryCompressible,
        pReport->PrimaryCompressMode,
        pReport->CompositionMode
    );
}

//!
//! \brief    Set up split screen demo mode
//! \details  Allocate and initialize split-screen demo mode structure
//! \param    [in] splitDemoPosDdi
//!           The split demo position setting from DDI layer
//! \param    [in] splitDemoParaDdi
//!           The split demo parameters setting from DDI layer
//! \param    [in,out] splitScreenDemoModeParams
//!           Pointer to struct for split-screen demo mode parameters
//! \param    [in,out] disableDemoMode
//!           Return whether demo mode will be disable or not
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VpHal_DdiSetupSplitScreenDemoMode(
    uint32_t                                splitDemoPosDdi,
    uint32_t                                splitDemoParaDdi,
    PVPHAL_SPLIT_SCREEN_DEMO_MODE_PARAMS    *splitScreenDemoModeParams,
    bool                                    *disableDemoMode)
{
    MOS_STATUS                  eStatus;
    uint32_t                    splitScreenDemoPosition;
    uint32_t                    splitScreenDemoParameters;

    eStatus                     = MOS_STATUS_SUCCESS;
    splitScreenDemoPosition     = splitDemoPosDdi;
    splitScreenDemoParameters   = splitDemoParaDdi;

    //--------------------------
    // Set Demo Mode Parameters
    //--------------------------
    if (*splitScreenDemoModeParams == nullptr)
    {
        *splitScreenDemoModeParams = (PVPHAL_SPLIT_SCREEN_DEMO_MODE_PARAMS)MOS_AllocAndZeroMemory(sizeof(VPHAL_SPLIT_SCREEN_DEMO_MODE_PARAMS));
        VPHAL_PUBLIC_CHK_NULL(*splitScreenDemoModeParams);
    }

#if (_DEBUG || _RELEASE_INTERNAL)
    // If it is not enabled from DDI params, check if internal user feature key have settings
    if (splitScreenDemoPosition == SPLIT_SCREEN_DEMO_DISABLED &&
        splitScreenDemoParameters == 0)
    {
        MOS_USER_FEATURE_VALUE_DATA UserFeatureData;

        MOS_ZeroMemory(&UserFeatureData, sizeof(UserFeatureData));
        MOS_USER_FEATURE_INVALID_KEY_ASSERT(MOS_UserFeature_ReadValue_ID(
            nullptr,
            __MEDIA_USER_FEATURE_VALUE_SPLIT_SCREEN_DEMO_POSITION_ID,
            &UserFeatureData));
        splitScreenDemoPosition = UserFeatureData.u32Data;

        MOS_ZeroMemory(&UserFeatureData, sizeof(UserFeatureData));
        MOS_USER_FEATURE_INVALID_KEY_ASSERT(MOS_UserFeature_ReadValue_ID(
            nullptr,
            __MEDIA_USER_FEATURE_VALUE_SPLIT_SCREEN_DEMO_PARAMETERS_ID,
            &UserFeatureData));
        splitScreenDemoParameters = UserFeatureData.u32Data;
    }
#endif

    if ((splitScreenDemoPosition > SPLIT_SCREEN_DEMO_DISABLED) && (splitScreenDemoPosition < SPLIT_SCREEN_DEMO_END_POS_LIST))
    {
        (*splitScreenDemoModeParams)->Position            = (VPHAL_SPLIT_SCREEN_DEMO_POSITION)(splitScreenDemoPosition);
        (*splitScreenDemoModeParams)->bDisableACE         = (bool)((splitScreenDemoParameters & 0x0001) > 0);
        (*splitScreenDemoModeParams)->bDisableAVS         = (bool)((splitScreenDemoParameters & 0x0002) > 0);
        (*splitScreenDemoModeParams)->bDisableDN          = (bool)((splitScreenDemoParameters & 0x0004) > 0);
        (*splitScreenDemoModeParams)->bDisableFMD         = (bool)((splitScreenDemoParameters & 0x0008) > 0);
        (*splitScreenDemoModeParams)->bDisableIEF         = (bool)((splitScreenDemoParameters & 0x0010) > 0);
        (*splitScreenDemoModeParams)->bDisableProcamp     = (bool)((splitScreenDemoParameters & 0x0020) > 0);
        (*splitScreenDemoModeParams)->bDisableSTE         = (bool)((splitScreenDemoParameters & 0x0040) > 0);
        (*splitScreenDemoModeParams)->bDisableTCC         = (bool)((splitScreenDemoParameters & 0x0080) > 0);
        (*splitScreenDemoModeParams)->bDisableIS          = (bool)((splitScreenDemoParameters & 0x0100) > 0);
        (*splitScreenDemoModeParams)->bDisableDrDb        = (bool)((splitScreenDemoParameters & 0x0200) > 0);
        (*splitScreenDemoModeParams)->bDisableDNUV        = (bool)((splitScreenDemoParameters & 0x0400) > 0);
        (*splitScreenDemoModeParams)->bDisableFRC         = (bool)((splitScreenDemoParameters & 0x0800) > 0);
        (*splitScreenDemoModeParams)->bDisableLACE        = (bool)((splitScreenDemoParameters & 0x1000) > 0);
        *disableDemoMode = false;
    }
    else
    {
        *disableDemoMode = true;
    }

finish:
    return eStatus;
}

//!
//! \brief    Init IEF Params to their default value
//! \param    [out] pIEFParams
//!           The IEF Params struct to be initialized
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VpHal_DdiInitIEFParams(
    PVPHAL_IEF_PARAMS       pIEFParams)
{
    MOS_STATUS              eStatus = MOS_STATUS_SUCCESS;

    VPHAL_PUBLIC_CHK_NULL(pIEFParams);

    // Init default values for flows where user feature key is not available or used
    pIEFParams->bSkintoneTuned        = true;
    pIEFParams->bEmphasizeSkinDetail  = false;
    pIEFParams->bSmoothMode           = false;
    pIEFParams->StrongEdgeWeight      = IEF_STRONG_EDGE_WEIGHT;
    pIEFParams->RegularWeight         = IEF_REGULAR_WEIGHT;
    pIEFParams->StrongEdgeThreshold   = IEF_STRONG_EDGE_THRESHOLD;

finish:
    return eStatus;
}
