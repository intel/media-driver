/*
* Copyright (c) 2025, Intel Corporation
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
//! \file     vp_vebox_cmd_packet_feature_handling.cpp
//! \brief    vebox packet for feature handling which used in by mediapipline.
//! \details  vebox packet provide the structures and generate the cmd buffer which mediapipline will used.
//!

#include "vp_vebox_cmd_packet.h"
#include "vp_hal_ddi_utils.h"

namespace vp
{

bool VpVeboxCmdPacket::IsTopField(VPHAL_SAMPLE_TYPE sampleType)
{
    return sampleType == SAMPLE_INTERLEAVED_EVEN_FIRST_TOP_FIELD ||
        sampleType == SAMPLE_INTERLEAVED_ODD_FIRST_TOP_FIELD;
}

bool VpVeboxCmdPacket::IsTopFieldFirst(VPHAL_SAMPLE_TYPE sampleType)
{
    return m_DNDIFirstFrame ?
        IsTopField(sampleType) :    // For no reference case, just do DI for current field.
        (sampleType == SAMPLE_INTERLEAVED_EVEN_FIRST_TOP_FIELD ||
         sampleType == SAMPLE_INTERLEAVED_EVEN_FIRST_BOTTOM_FIELD);
}

MOS_STATUS VpVeboxCmdPacket::CheckTGNEValid(
    uint32_t *pStatSlice0GNEPtr,
    uint32_t *pStatSlice1GNEPtr,
    uint32_t *pQuery)
{
    VP_FUNC_CALL();
    MOS_STATUS                eStatus            = MOS_STATUS_SUCCESS;
    uint32_t                  bGNECountLumaValid = 0;
    uint32_t                  dwTGNEoffset       = 0;
    VP_PACKET_SHARED_CONTEXT *sharedContext      = (VP_PACKET_SHARED_CONTEXT *)m_packetSharedContext;

    VP_PUBLIC_CHK_NULL_RETURN(sharedContext);

    auto &tgneParams = sharedContext->tgneParams;
    dwTGNEoffset     = (VP_VEBOX_STATISTICS_SURFACE_TGNE_OFFSET - VP_VEBOX_STATISTICS_SURFACE_GNE_OFFSET) / 4;

    if (m_bTgneEnable)
    {
        bGNECountLumaValid = (pStatSlice0GNEPtr[dwTGNEoffset + 3] & 0x80000000) || (pStatSlice1GNEPtr[dwTGNEoffset + 3] & 0x80000000);

        VP_PUBLIC_NORMALMESSAGE("TGNE:bGNECountLumaValid %x", bGNECountLumaValid);

        if (bGNECountLumaValid)
        {
            *pQuery      = VP_VEBOX_STATISTICS_SURFACE_TGNE_OFFSET;
            m_bTgneValid = true;

            if (tgneParams.bTgneFirstFrame)
            {
                tgneParams.bTgneFirstFrame = false;
            }
        }
        else
        {
            *pQuery      = VP_VEBOX_STATISTICS_SURFACE_GNE_OFFSET;
            m_bTgneValid = false;
        }
    }
    else
    {
        *pQuery                    = VP_VEBOX_STATISTICS_SURFACE_GNE_OFFSET;
        m_bTgneValid               = false;
        tgneParams.bTgneFirstFrame = true;
    }

    VP_PUBLIC_NORMALMESSAGE("TGNE:bTGNEValid %x", m_bTgneValid);
    return eStatus;
}

//!
//! \brief    Vebox Populate VEBOX parameters
//! \details  Populate the Vebox VEBOX state parameters to VEBOX RenderData
//! \param    [in] bDnEnabled
//!           true if DN being enabled
//! \param    [in] pChromaParams
//!           true to Chroma DN being enabled
//! \param    [in] pLumaParams
//!           Pointer to Luma DN and DI parameter
//! \param    [in] pChromaParams
//!           Pointer to Chroma DN parameter
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VpVeboxCmdPacket::ConfigDnLumaChromaParams(
    bool                            bDnEnabled,
    bool                            bChromaDenoise,
    PVP_SAMPLER_STATE_DN_PARAM      pLumaParams,
    PVPHAL_DNUV_PARAMS              pChromaParams
    )
{
    VP_FUNC_CALL();

    VpVeboxRenderData               *pRenderData = GetLastExecRenderData();

    VP_PUBLIC_CHK_NULL_RETURN(pRenderData);

    MHW_VEBOX_DNDI_PARAMS &veboxDNDIParams = pRenderData->GetDNDIParams();

    // Luma Denoise Params
    if (bDnEnabled && pLumaParams)
    {
        veboxDNDIParams.dwDenoiseASDThreshold     = pLumaParams->dwDenoiseASDThreshold;
        veboxDNDIParams.dwDenoiseHistoryDelta     = pLumaParams->dwDenoiseHistoryDelta;
        veboxDNDIParams.dwDenoiseMaximumHistory   = pLumaParams->dwDenoiseMaximumHistory;
        veboxDNDIParams.dwDenoiseSTADThreshold    = pLumaParams->dwDenoiseSTADThreshold;
        veboxDNDIParams.dwDenoiseSCMThreshold     = pLumaParams->dwDenoiseSCMThreshold;
        veboxDNDIParams.dwDenoiseMPThreshold      = pLumaParams->dwDenoiseMPThreshold;
        veboxDNDIParams.dwLTDThreshold            = pLumaParams->dwLTDThreshold;
        veboxDNDIParams.dwTDThreshold             = pLumaParams->dwTDThreshold;
        veboxDNDIParams.dwGoodNeighborThreshold   = pLumaParams->dwGoodNeighborThreshold;
    }

    // Chroma Denoise Params
    if (bChromaDenoise && pChromaParams)
    {
        veboxDNDIParams.dwChromaSTADThreshold     = pChromaParams->dwSTADThresholdU; // Use U threshold for now
        veboxDNDIParams.dwChromaLTDThreshold      = pChromaParams->dwLTDThresholdU;  // Use U threshold for now
        veboxDNDIParams.dwChromaTDThreshold       = pChromaParams->dwTDThresholdU;   // Use U threshold for now
    }

    return MOS_STATUS_SUCCESS;
}

//!
//! \brief    Configure FMD parameter
//! \details  Configure FMD parameters for DNDI State
//! \param    [in] bProgressive
//!           true if sample being progressive
//! \param    [in] bAutoDenoise
//!           true if auto denoise being enabled
//! \param    [out] pLumaParams
//!           Pointer to DNDI Param for set FMD parameters
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VpVeboxCmdPacket::ConfigFMDParams(bool bProgressive, bool bAutoDenoise, bool bFmdEnabled)
{
    VP_FUNC_CALL();

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpVeboxCmdPacket::ConfigureDenoiseParams(VpVeboxRenderData *renderData, float fDenoiseFactor)
{
    // ConfigureDenoiseParams() just includes logic that both used in SetDenoiseParams and UpdateDenoiseParams.
    // If the logic won't be used in UpdateDenoiseParams, please just add the logic into SetDenoiseParams.
    VP_FUNC_CALL();
    VP_RENDER_CHK_NULL_RETURN(renderData);
    VP_SAMPLER_STATE_DN_PARAM lumaParams   = {};
    VPHAL_DNUV_PARAMS         chromaParams = {};

    GetDnLumaParams(renderData->DN.bDnEnabled, renderData->DN.bAutoDetect, fDenoiseFactor, m_PacketCaps.bRefValid, &lumaParams);
    GetDnChromaParams(renderData->DN.bChromaDnEnabled, renderData->DN.bAutoDetect, fDenoiseFactor, &chromaParams);

    // Setup Denoise Params
    ConfigLumaPixRange(renderData->DN.bDnEnabled, renderData->DN.bAutoDetect, fDenoiseFactor);
    ConfigChromaPixRange(renderData->DN.bChromaDnEnabled, renderData->DN.bAutoDetect, fDenoiseFactor);
    ConfigDnLumaChromaParams(renderData->DN.bDnEnabled, renderData->DN.bChromaDnEnabled, &lumaParams, &chromaParams);
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpVeboxCmdPacket::ConfigureProcampParams(VpVeboxRenderData *renderData, bool bEnableProcamp, float fBrightness, float fContrast, float fHue, float fSaturation)
{
    // ConfigureProcampParams() just includes logic that both used in SetProcampParams and UpdateProcampParams.
    // If the logic won't be used in UpdateProcampParams, please just add the logic into SetProcampParams.
    VP_FUNC_CALL();
    VP_RENDER_CHK_NULL_RETURN(renderData);
    MHW_VEBOX_IECP_PARAMS &mhwVeboxIecpParams = renderData->GetIECPParams();
    if (bEnableProcamp)
    {
        renderData->IECP.PROCAMP.bProcampEnabled    = true;
        mhwVeboxIecpParams.ProcAmpParams.bActive    = true;
        mhwVeboxIecpParams.ProcAmpParams.bEnabled   = true;
        mhwVeboxIecpParams.ProcAmpParams.brightness = (uint32_t)MOS_F_ROUND(fBrightness * 16.0F);  // S7.4
        mhwVeboxIecpParams.ProcAmpParams.contrast   = (uint32_t)MOS_UF_ROUND(fContrast * 128.0F);  // U4.7
        mhwVeboxIecpParams.ProcAmpParams.sinCS      = (uint32_t)MOS_F_ROUND(sin(MHW_DEGREE_TO_RADIAN(fHue)) *
                                                                       fContrast * fSaturation * 256.0F);                                    // S7.8
        mhwVeboxIecpParams.ProcAmpParams.cosCS      = (uint32_t)MOS_F_ROUND(cos(MHW_DEGREE_TO_RADIAN(fHue)) * fContrast * fSaturation * 256.0F);  // S7.8
    }
    else
    {
        renderData->IECP.PROCAMP.bProcampEnabled  = false;
        mhwVeboxIecpParams.ProcAmpParams.bActive  = false;
        mhwVeboxIecpParams.ProcAmpParams.bEnabled = false;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpVeboxCmdPacket::ConfigureSteParams(VpVeboxRenderData *renderData, bool bEnableSte, uint32_t dwSTEFactor, bool bEnableStd, uint32_t stdParaSizeInBytes, void *stdParam)
{
    VP_FUNC_CALL();
    MHW_VEBOX_IECP_PARAMS &mhwVeboxIecpParams = renderData->GetIECPParams();

    if (bEnableSte)
    {
        renderData->IECP.STE.bSteEnabled              = true;
        mhwVeboxIecpParams.ColorPipeParams.bActive    = true;
        mhwVeboxIecpParams.ColorPipeParams.bEnableSTE = true;
        if (dwSTEFactor > MHW_STE_FACTOR_MAX)
        {
            mhwVeboxIecpParams.ColorPipeParams.SteParams.dwSTEFactor = MHW_STE_FACTOR_MAX;
            mhwVeboxIecpParams.ColorPipeParams.SteParams.satP1       = m_satP1Table[MHW_STE_FACTOR_MAX];
            mhwVeboxIecpParams.ColorPipeParams.SteParams.satS0       = m_satS0Table[MHW_STE_FACTOR_MAX];
            mhwVeboxIecpParams.ColorPipeParams.SteParams.satS1       = m_satS1Table[MHW_STE_FACTOR_MAX];
        }
        else
        {
            mhwVeboxIecpParams.ColorPipeParams.SteParams.dwSTEFactor = dwSTEFactor;
            mhwVeboxIecpParams.ColorPipeParams.SteParams.satP1       = m_satP1Table[dwSTEFactor];
            mhwVeboxIecpParams.ColorPipeParams.SteParams.satS0       = m_satS0Table[dwSTEFactor];
            mhwVeboxIecpParams.ColorPipeParams.SteParams.satS1       = m_satS1Table[dwSTEFactor];
        }
    }
    else if (bEnableStd)
    {
        renderData->IECP.STE.bStdEnabled                             = true;
        mhwVeboxIecpParams.ColorPipeParams.bActive                   = true;
        mhwVeboxIecpParams.ColorPipeParams.bEnableSTD                = true;
        mhwVeboxIecpParams.ColorPipeParams.StdParams.paraSizeInBytes = stdParaSizeInBytes;
        mhwVeboxIecpParams.ColorPipeParams.StdParams.param           = stdParam;
    }
    else
    {
        renderData->IECP.STE.bSteEnabled              = false;
        mhwVeboxIecpParams.ColorPipeParams.bEnableSTE = false;
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpVeboxCmdPacket::ConfigureTccParams(VpVeboxRenderData *renderData, bool bEnableTcc, uint8_t magenta, uint8_t red, uint8_t yellow, uint8_t green, uint8_t cyan, uint8_t blue)
{
    // ConfigureTccParams() just includes logic that both used in SetTccParams and UpdateTccParams.
    // If the logic won't be used in UpdateTccParams, please just add the logic into SetTccParams.
    VP_FUNC_CALL();
    MHW_VEBOX_IECP_PARAMS &mhwVeboxIecpParams = renderData->GetIECPParams();
    if (bEnableTcc)
    {
        renderData->IECP.TCC.bTccEnabled                     = true;
        mhwVeboxIecpParams.ColorPipeParams.bActive           = true;
        mhwVeboxIecpParams.ColorPipeParams.bEnableTCC        = true;
        mhwVeboxIecpParams.ColorPipeParams.TccParams.Magenta = magenta;
        mhwVeboxIecpParams.ColorPipeParams.TccParams.Red     = red;
        mhwVeboxIecpParams.ColorPipeParams.TccParams.Yellow  = yellow;
        mhwVeboxIecpParams.ColorPipeParams.TccParams.Green   = green;
        mhwVeboxIecpParams.ColorPipeParams.TccParams.Cyan    = cyan;
        mhwVeboxIecpParams.ColorPipeParams.TccParams.Blue    = blue;
    }
    else
    {
        renderData->IECP.TCC.bTccEnabled              = false;
        mhwVeboxIecpParams.ColorPipeParams.bEnableTCC = false;
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpVeboxCmdPacket::Init3DLutTable(PVP_SURFACE surf3DLut)
{
    VP_FUNC_CALL();
    VpVeboxRenderData *renderData   = GetLastExecRenderData();

    VP_RENDER_CHK_NULL_RETURN(renderData);

    if (!renderData->HDR3DLUT.is3DLutTableFilled)
    {
        VP_RENDER_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
    }

    VP_RENDER_NORMALMESSAGE("3DLut table is calculated by kernel.");

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpVeboxCmdPacket::QueryStatLayoutGNE(
    VEBOX_STAT_QUERY_TYPE QueryType,
    uint32_t             *pQuery,
    uint8_t              *pStatSlice0Base,
    uint8_t              *pStatSlice1Base)
{
    VP_FUNC_CALL();

    VP_PUBLIC_CHK_NULL_RETURN(pQuery);
    // Query platform dependent GNE offset
    VP_PUBLIC_CHK_STATUS_RETURN(QueryStatLayout(
        VEBOX_STAT_QUERY_GNE_OFFEST,
        pQuery));

    // check TGNE is valid or not.
    VP_PUBLIC_CHK_STATUS_RETURN(VpVeboxCmdPacket::CheckTGNEValid(
        (uint32_t *)(pStatSlice0Base + *pQuery),
        (uint32_t *)(pStatSlice1Base + *pQuery),
        pQuery));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpVeboxCmdPacket::SetCgcParams(
    PVEBOX_CGC_PARAMS                    cgcParams)
{
    VP_FUNC_CALL();

    VP_PUBLIC_CHK_NULL_RETURN(cgcParams);

    VpVeboxRenderData* pRenderData = GetLastExecRenderData();
    VP_PUBLIC_CHK_NULL_RETURN(pRenderData);

    MHW_VEBOX_GAMUT_PARAMS& mhwVeboxGamutParams = pRenderData->GetGamutParams();

    bool  bAdvancedMode = (cgcParams->GCompMode == GAMUT_MODE_ADVANCED) ? true : false;
    bool  bypassGComp = false;

    if (cgcParams->bBt2020ToRGB)
    {
        // Init GC params
        pRenderData->IECP.CGC.bCGCEnabled = true;
        mhwVeboxGamutParams.ColorSpace    = VpHalCspace2MhwCspace(cgcParams->inputColorSpace);
        mhwVeboxGamutParams.dstColorSpace = VpHalCspace2MhwCspace(cgcParams->outputColorSpace);
        mhwVeboxGamutParams.srcFormat     = cgcParams->inputFormat;
        mhwVeboxGamutParams.dstFormat     = cgcParams->outputFormat;
        mhwVeboxGamutParams.GCompMode     = MHW_GAMUT_MODE_NONE;
        mhwVeboxGamutParams.GExpMode      = MHW_GAMUT_MODE_NONE;
        mhwVeboxGamutParams.bGammaCorr    = false;
    }
    else
    {
        if (cgcParams->bEnableCGC && cgcParams->GCompMode != GAMUT_MODE_NONE)
        {
            VP_RENDER_ASSERTMESSAGE("Bypass GamutComp.");
        }
        pRenderData->IECP.CGC.bCGCEnabled = false;
        mhwVeboxGamutParams.GCompMode = MHW_GAMUT_MODE_NONE;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpVeboxCmdPacket::SetDiParams(PVEBOX_DI_PARAMS diParams)
{
    VP_FUNC_CALL();

    MOS_STATUS                      eStatus         = MOS_STATUS_SUCCESS;
    VpVeboxRenderData               *renderData     = GetLastExecRenderData();
    VP_SAMPLER_STATE_DN_PARAM       lumaParams      = {};
    VPHAL_DNUV_PARAMS               chromaParams    = {};

    VP_PUBLIC_CHK_NULL_RETURN(diParams);
    VP_PUBLIC_CHK_NULL_RETURN(renderData);

    renderData->DI.value            = 0;
    renderData->DI.bDeinterlace     = diParams->bDiEnabled;
    renderData->DI.bQueryVariance   = diParams->bEnableQueryVariance;
    renderData->DI.bTFF             = IsTopFieldFirst(diParams->sampleTypeInput);
    renderData->DI.bFmdEnabled      = diParams->enableFMD;

    // for 30i->30fps + SFC
    if (m_PacketCaps.bSFC && !diParams->b60fpsDi)
    {
        // Set BLT1's Current DI Output as BLT2's input, it is always under Mode0
        // BLT1 output 1st field of current frame for the following cases:
        if (m_DNDIFirstFrame                                                            ||
            (diParams->sampleTypeInput == SAMPLE_INTERLEAVED_ODD_FIRST_BOTTOM_FIELD) ||
            (diParams->sampleTypeInput == SAMPLE_INTERLEAVED_EVEN_FIRST_TOP_FIELD)   ||
            (diParams->sampleTypeInput == SAMPLE_SINGLE_TOP_FIELD)                   ||
            (diParams->sampleTypeInput == SAMPLE_PROGRESSIVE))
        {
            m_DIOutputFrames = MEDIA_VEBOX_DI_OUTPUT_CURRENT;
        }
        else
        {
            // First sample output - 2nd field of the previous frame
            m_DIOutputFrames = MEDIA_VEBOX_DI_OUTPUT_PREVIOUS;
        }
    }
    // for 30i->60fps or other 30i->30fps cases
    else
    {
            m_DIOutputFrames = m_DNDIFirstFrame ?
                MEDIA_VEBOX_DI_OUTPUT_CURRENT :
                MEDIA_VEBOX_DI_OUTPUT_BOTH;
    }

    VP_PUBLIC_CHK_STATUS_RETURN(SetDiParams(diParams->bDiEnabled, diParams->bSCDEnabled, diParams->bHDContent, diParams->sampleTypeInput, renderData->GetDNDIParams()));

    return eStatus;
}

MOS_STATUS VpVeboxCmdPacket::SetDiParams(bool bDiEnabled, bool bSCDEnabled, bool bHDContent, VPHAL_SAMPLE_TYPE sampleTypeInput, MHW_VEBOX_DNDI_PARAMS &param)
{
    VP_FUNC_CALL();

    if (!bDiEnabled)
    {
        return MOS_STATUS_SUCCESS;
    }

    param.bDNDITopFirst              = IsTopFieldFirst(sampleTypeInput);
    param.dwLumaTDMWeight            = VPHAL_VEBOX_DI_LUMA_TDM_WEIGHT_NATUAL;
    param.dwChromaTDMWeight          = VPHAL_VEBOX_DI_CHROMA_TDM_WEIGHT_NATUAL;
    param.dwSHCMDelta                = VPHAL_VEBOX_DI_SHCM_DELTA_NATUAL;
    param.dwSHCMThreshold            = VPHAL_VEBOX_DI_SHCM_THRESHOLD_NATUAL;
    param.dwSVCMDelta                = VPHAL_VEBOX_DI_SVCM_DELTA_NATUAL;
    param.dwSVCMThreshold            = VPHAL_VEBOX_DI_SVCM_THRESHOLD_NATUAL;
    param.bFasterConvergence         = false;
    param.bTDMLumaSmallerWindow      = false;
    param.bTDMChromaSmallerWindow    = false;
    param.dwLumaTDMCoringThreshold   = VPHAL_VEBOX_DI_LUMA_TDM_CORING_THRESHOLD_NATUAL;
    param.dwChromaTDMCoringThreshold = VPHAL_VEBOX_DI_CHROMA_TDM_CORING_THRESHOLD_NATUAL;
    param.bBypassDeflickerFilter     = true;
    param.bUseSyntheticContentMedian = false;
    param.bLocalCheck                = true;
    param.bSyntheticContentCheck     = false;
    param.dwDirectionCheckThreshold  = VPHAL_VEBOX_DI_DIRECTION_CHECK_THRESHOLD_NATUAL;
    param.dwTearingLowThreshold      = VPHAL_VEBOX_DI_TEARING_LOW_THRESHOLD_NATUAL;
    param.dwTearingHighThreshold     = VPHAL_VEBOX_DI_TEARING_HIGH_THRESHOLD_NATUAL;
    param.dwDiffCheckSlackThreshold  = VPHAL_VEBOX_DI_DIFF_CHECK_SLACK_THRESHOLD_NATUAL;
    param.dwSADWT0                   = VPHAL_VEBOX_DI_SAD_WT0_NATUAL;
    param.dwSADWT1                   = VPHAL_VEBOX_DI_SAD_WT1_NATUAL;
    param.dwSADWT2                   = VPHAL_VEBOX_DI_SAD_WT2_NATUAL;
    param.dwSADWT3                   = VPHAL_VEBOX_DI_SAD_WT3_NATUAL;
    param.dwSADWT4                   = VPHAL_VEBOX_DI_SAD_WT4_NATUAL;
    param.dwSADWT6                   = VPHAL_VEBOX_DI_SAD_WT6_NATUAL;
    param.bSCDEnable                 = bSCDEnabled;

    if (bHDContent)
    {
        param.dwLPFWtLUT0 = VPHAL_VEBOX_DI_LPFWTLUT0_HD_NATUAL;
        param.dwLPFWtLUT1 = VPHAL_VEBOX_DI_LPFWTLUT1_HD_NATUAL;
        param.dwLPFWtLUT2 = VPHAL_VEBOX_DI_LPFWTLUT2_HD_NATUAL;
        param.dwLPFWtLUT3 = VPHAL_VEBOX_DI_LPFWTLUT3_HD_NATUAL;
        param.dwLPFWtLUT4 = VPHAL_VEBOX_DI_LPFWTLUT4_HD_NATUAL;
        param.dwLPFWtLUT5 = VPHAL_VEBOX_DI_LPFWTLUT5_HD_NATUAL;
        param.dwLPFWtLUT6 = VPHAL_VEBOX_DI_LPFWTLUT6_HD_NATUAL;
        param.dwLPFWtLUT7 = VPHAL_VEBOX_DI_LPFWTLUT7_HD_NATUAL;
    }
    else
    {
        param.dwLPFWtLUT0 = VPHAL_VEBOX_DI_LPFWTLUT0_SD_NATUAL;
        param.dwLPFWtLUT1 = VPHAL_VEBOX_DI_LPFWTLUT1_SD_NATUAL;
        param.dwLPFWtLUT2 = VPHAL_VEBOX_DI_LPFWTLUT2_SD_NATUAL;
        param.dwLPFWtLUT3 = VPHAL_VEBOX_DI_LPFWTLUT3_SD_NATUAL;
        param.dwLPFWtLUT4 = VPHAL_VEBOX_DI_LPFWTLUT4_SD_NATUAL;
        param.dwLPFWtLUT5 = VPHAL_VEBOX_DI_LPFWTLUT5_SD_NATUAL;
        param.dwLPFWtLUT6 = VPHAL_VEBOX_DI_LPFWTLUT6_SD_NATUAL;
        param.dwLPFWtLUT7 = VPHAL_VEBOX_DI_LPFWTLUT7_SD_NATUAL;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpVeboxCmdPacket::SetDnParams(
    PVEBOX_DN_PARAMS                    pDnParams)
{
    VP_FUNC_CALL();

    MOS_STATUS         eStatus     = MOS_STATUS_SUCCESS;
    VpVeboxRenderData *pRenderData = GetLastExecRenderData();

    VP_PUBLIC_CHK_NULL_RETURN(pDnParams);
    VP_PUBLIC_CHK_NULL_RETURN(pRenderData);
    VP_PUBLIC_CHK_NULL_RETURN(m_report);

    pRenderData->DN.bDnEnabled       = pDnParams->bDnEnabled;
    pRenderData->DN.bAutoDetect      = pDnParams->bAutoDetect;
    pRenderData->DN.bChromaDnEnabled = pDnParams->bChromaDenoise;
    pRenderData->DN.bHvsDnEnabled    = pDnParams->bEnableHVSDenoise;

    pRenderData->GetDNDIParams().bChromaDNEnable = pDnParams->bChromaDenoise;
    pRenderData->GetDNDIParams().bProgressiveDN  = pDnParams->bDnEnabled && pDnParams->bProgressive;
    pRenderData->GetHVSParams().QP               = pDnParams->HVSDenoise.QP;
    pRenderData->GetHVSParams().Mode             = pDnParams->HVSDenoise.Mode;
    pRenderData->GetHVSParams().Strength         = pDnParams->HVSDenoise.Strength;

    // ConfigureDenoiseParams() just includes logic that both used in SetDenoiseParams and UpdateDenoiseParams.
    // If the logic won't be used in UpdateDenoiseParams, please just add the logic into SetDenoiseParams.
    VP_PUBLIC_CHK_STATUS_RETURN(ConfigureDenoiseParams(pRenderData, pDnParams->fDenoiseFactor));
    // bDNDITopFirst in DNDI parameters need be configured during SetDIParams.

    m_report->GetFeatures().denoise = pRenderData->DN.bDnEnabled;
    return eStatus;
}

MOS_STATUS VpVeboxCmdPacket::SetHdrParams(PVEBOX_HDR_PARAMS hdrParams)
{
    VP_FUNC_CALL();
    VpVeboxRenderData     *pRenderData = GetLastExecRenderData();
    PMOS_INTERFACE        pOsInterface   = nullptr;
    PMHW_3DLUT_PARAMS     pLutParams     = nullptr;

    VP_PUBLIC_CHK_NULL_RETURN(m_hwInterface);
    VP_PUBLIC_CHK_NULL_RETURN(m_hwInterface->m_osInterface);
    VP_PUBLIC_CHK_NULL_RETURN(hdrParams);
    VP_RENDER_ASSERT(pRenderData);

    MHW_VEBOX_IECP_PARAMS  &mhwVeboxIecpParams  = pRenderData->GetIECPParams();
    MHW_VEBOX_GAMUT_PARAMS &mhwVeboxGamutParams = pRenderData->GetGamutParams();
    pOsInterface                       = m_hwInterface->m_osInterface;
    pRenderData->HDR3DLUT.bHdr3DLut    = true;

    pRenderData->HDR3DLUT.is3DLutTableFilled   = HDR_STAGE::HDR_STAGE_VEBOX_3DLUT_UPDATE == hdrParams->stage ||
                                                    HDR_STAGE::HDR_STAGE_VEBOX_3DLUT_NO_UPDATE == hdrParams->stage;
    pRenderData->HDR3DLUT.is3DLutTableUpdatedByKernel = HDR_STAGE::HDR_STAGE_VEBOX_3DLUT_UPDATE == hdrParams->stage;
    pRenderData->HDR3DLUT.isExternal3DLutTable    = HDR_STAGE::HDR_STAGE_VEBOX_EXTERNAL_3DLUT == hdrParams->stage;
    pRenderData->HDR3DLUT.uiMaxDisplayLum      = hdrParams->uiMaxDisplayLum;
    pRenderData->HDR3DLUT.uiMaxContentLevelLum = hdrParams->uiMaxContentLevelLum;
    pRenderData->HDR3DLUT.hdrMode              = hdrParams->hdrMode;
    pRenderData->HDR3DLUT.uiLutSize            = hdrParams->lutSize;

    if (!(hdrParams->stage == HDR_STAGE_VEBOX_EXTERNAL_3DLUT))
    {
        VP_RENDER_CHK_STATUS_RETURN(ValidateHDR3DLutParameters(pRenderData->HDR3DLUT.is3DLutTableFilled));
    }
    // Use Gamut
    mhwVeboxGamutParams.ColorSpace       = VpHalCspace2MhwCspace(hdrParams->srcColorSpace);
    mhwVeboxGamutParams.dstColorSpace    = VpHalCspace2MhwCspace(hdrParams->dstColorSpace);
    mhwVeboxGamutParams.dstFormat        = hdrParams->dstFormat;
    mhwVeboxGamutParams.bGammaCorr       = true;
    mhwVeboxGamutParams.InputGammaValue  = (MHW_GAMMA_VALUE)GAMMA_1P0;
    mhwVeboxGamutParams.OutputGammaValue = (MHW_GAMMA_VALUE)GAMMA_1P0;
    if (pRenderData->HDR3DLUT.hdrMode == VPHAL_HDR_MODE_TONE_MAPPING)
    {
        mhwVeboxGamutParams.bH2S     = true;
        mhwVeboxGamutParams.uiMaxCLL = (uint16_t)pRenderData->HDR3DLUT.uiMaxContentLevelLum;
    }
    else if (pRenderData->HDR3DLUT.hdrMode == VPHAL_HDR_MODE_H2H)
    {
        mhwVeboxGamutParams.bH2S     = false;
        mhwVeboxGamutParams.uiMaxCLL = 0;
    }

    mhwVeboxIecpParams.s3DLutParams.bActive   = true;

    if (hdrParams->isFp16Enable)
    {
        // When FP16 enable, it doesn't need Pre CSC, so set GammaCorr and H2S false.
        mhwVeboxGamutParams.bGammaCorr          = false;
        mhwVeboxGamutParams.bH2S                = false;
        mhwVeboxIecpParams.fp16Params.isActive  = true;
    }

    if (hdrParams->stage == HDR_STAGE_VEBOX_EXTERNAL_3DLUT)
    {
        if (hdrParams->external3DLutParams)
        {
            mhwVeboxIecpParams.s3DLutParams.LUTSize = hdrParams->external3DLutParams->LutSize;
            mhwVeboxIecpParams.s3DLutParams.InterpolationMethod = Get3DLutInterpolationMethod(hdrParams->external3DLutParams->InterpolationMethod);
            pRenderData->HDR3DLUT.external3DLutSurfResource = hdrParams->external3DLutParams->pExt3DLutSurface->OsResource;
        }
        else
        {
            VP_RENDER_ASSERTMESSAGE("hdrParams external3DLutParams is null.");
        }
    }

    //Report
    if (m_hwInterface->m_reporting)
    {
        if (pRenderData->HDR3DLUT.uiLutSize == LUT33_SEG_SIZE)
        {
            m_hwInterface->m_reporting->GetFeatures().hdrMode = (pRenderData->HDR3DLUT.hdrMode == VPHAL_HDR_MODE_H2H) ? VPHAL_HDR_MODE_H2H_VEBOX_3DLUT33 : VPHAL_HDR_MODE_TONE_MAPPING_VEBOX_3DLUT33;
        }
        else
        {
            m_hwInterface->m_reporting->GetFeatures().hdrMode = (pRenderData->HDR3DLUT.hdrMode == VPHAL_HDR_MODE_H2H) ? VPHAL_HDR_MODE_H2H_VEBOX_3DLUT : VPHAL_HDR_MODE_TONE_MAPPING_VEBOX_3DLUT;
        }
    }

    return MOS_STATUS_SUCCESS;
}
MOS_STATUS VpVeboxCmdPacket::SetProcampParams(
    PVEBOX_PROCAMP_PARAMS pProcampParams)
{
    VP_FUNC_CALL();

    VpVeboxRenderData *pRenderData = GetLastExecRenderData();
    VP_PUBLIC_CHK_NULL_RETURN(pProcampParams);
    VP_PUBLIC_CHK_NULL_RETURN(pRenderData);

    // ConfigureProcampParams() just includes logic that both used in SetProcampParams and UpdateProcampParams.
    // If the logic won't be used in UpdateProcampParams, please just add the logic into SetProcampParams.
    VP_PUBLIC_CHK_STATUS_RETURN(ConfigureProcampParams(pRenderData, pProcampParams->bEnableProcamp, pProcampParams->fBrightness, 
        pProcampParams->fContrast, pProcampParams->fHue, pProcampParams->fSaturation));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpVeboxCmdPacket::SetScalingParams(PSFC_SCALING_PARAMS scalingParams)
{
    VP_FUNC_CALL();

    VP_PUBLIC_CHK_NULL_RETURN(scalingParams);
    // Scaing only can be apply to SFC path
    if (m_PacketCaps.bSFC)
    {
        VP_PUBLIC_CHK_NULL_RETURN(m_sfcRender);
        VP_PUBLIC_CHK_STATUS_RETURN(m_sfcRender->SetScalingParams(scalingParams));

        //---------------------------------
        // Set SFC State:  mmc
        //---------------------------------
        SetSfcMmcParams();

        return MOS_STATUS_SUCCESS;
    }
    else
    {
        VP_RENDER_NORMALMESSAGE("Scaling is enabled in SFC, pls recheck the features enabling in SFC");
        return MOS_STATUS_INVALID_PARAMETER;
    }
}

MOS_STATUS VpVeboxCmdPacket::SetSfcCSCParams(PSFC_CSC_PARAMS cscParams)
{
    VP_FUNC_CALL();

    VP_PUBLIC_CHK_NULL_RETURN(cscParams);

    if (m_PacketCaps.bSFC)
    {
        VP_PUBLIC_CHK_NULL_RETURN(m_sfcRender);
        VP_PUBLIC_CHK_STATUS_RETURN(m_sfcRender->SetCSCParams(cscParams));
        return MOS_STATUS_SUCCESS;
    }
    else
    {
        VP_RENDER_NORMALMESSAGE("CSC/IEF for Output is enabled in SFC, pls recheck the features enabling in SFC");
        return MOS_STATUS_INVALID_PARAMETER;
    }
}

MOS_STATUS VpVeboxCmdPacket::SetVeboxCSCParams(PVEBOX_CSC_PARAMS cscParams)
{
    VP_FUNC_CALL();

    VP_RENDER_CHK_NULL_RETURN(cscParams);

    if (cscParams->blockType == VEBOX_CSC_BLOCK_TYPE::FRONT_END)
    {
        VP_RENDER_CHK_STATUS_RETURN(SetVeboxFeCSCParams(cscParams));
    }
    else //cscParams->blockType == VEBOX_CSC_BLOCK_TYPE::BACK_END
    {
        VP_RENDER_CHK_STATUS_RETURN(SetVeboxBeCSCParams(cscParams));
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpVeboxCmdPacket::SetVeboxFeCSCParams(PVEBOX_CSC_PARAMS cscParams)
{
    VP_FUNC_CALL();

    VP_RENDER_CHK_NULL_RETURN(cscParams);

    VpVeboxRenderData *pRenderData = GetLastExecRenderData();
    VP_RENDER_CHK_NULL_RETURN(pRenderData);

    pRenderData->IECP.FeCSC.bFeCSCEnabled = cscParams->bCSCEnabled;

    MHW_VEBOX_IECP_PARAMS &veboxIecpParams = pRenderData->GetIECPParams();

    if (m_CscInputCspace != cscParams->inputColorSpace ||
        m_CscOutputCspace != cscParams->outputColorSpace)
    {
        // For VE 3DLUT HDR cases, CSC params will be overriden in Vebox Interface_BT2020YUVToRGB
        // Get the matrix to use for conversion
        // For BeCSC, it will call VeboxGetBeCSCMatrix, because it may need to swap channel
        // For FeCSC, no need to swap channel, because no SFC is used when using FeCSC. Vebox will directly output 
        VpHal_GetCscMatrix(
            cscParams->inputColorSpace,
            cscParams->outputColorSpace,
            m_fCscCoeff,
            m_fCscInOffset,
            m_fCscOutOffset);

        veboxIecpParams.srcFormat  = cscParams->inputFormat;
        veboxIecpParams.dstFormat  = cscParams->outputFormat;
        veboxIecpParams.ColorSpace = (MHW_CSPACE)cscParams->inputColorSpace;
    }

    if (m_PacketCaps.bVebox &&
        m_PacketCaps.bFeCSC &&
        cscParams->bCSCEnabled)
    {
        veboxIecpParams.bFeCSCEnable     = true;
        veboxIecpParams.pfFeCscCoeff     = m_fCscCoeff;
        veboxIecpParams.pfFeCscInOffset  = m_fCscInOffset;
        veboxIecpParams.pfFeCscOutOffset = m_fCscOutOffset;
        if ((cscParams->outputFormat == Format_A16B16G16R16F) || (cscParams->outputFormat == Format_A16R16G16B16F))
        {
            // Use CCM to do CSC for FP16 VEBOX output
            veboxIecpParams.bFeCSCEnable  = false;
            veboxIecpParams.bCcmCscEnable = true;
        }
    }

    VP_RENDER_CHK_STATUS_RETURN(SetVeboxOutputAlphaParams(cscParams));
    VP_RENDER_CHK_STATUS_RETURN(SetVeboxChromasitingParams(cscParams));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpVeboxCmdPacket::SetVeboxBeCSCParams(PVEBOX_CSC_PARAMS cscParams)
{
    VP_FUNC_CALL();

    VP_RENDER_CHK_NULL_RETURN(cscParams);

    VpVeboxRenderData* pRenderData = GetLastExecRenderData();
    VP_RENDER_CHK_NULL_RETURN(pRenderData);

    pRenderData->IECP.BeCSC.bBeCSCEnabled = cscParams->bCSCEnabled;

    MHW_VEBOX_IECP_PARAMS& veboxIecpParams = pRenderData->GetIECPParams();

    if (m_CscInputCspace  != cscParams->inputColorSpace ||
        m_CscOutputCspace != cscParams->outputColorSpace)
    {
        // For VE 3DLUT HDR cases, CSC params will be overriden in Vebox Interface_BT2020YUVToRGB
        VeboxGetBeCSCMatrix(
            cscParams->inputColorSpace,
            cscParams->outputColorSpace,
            cscParams->inputFormat);

        veboxIecpParams.srcFormat  = cscParams->inputFormat;
        veboxIecpParams.dstFormat  = cscParams->outputFormat;
        veboxIecpParams.ColorSpace = (MHW_CSPACE)cscParams->inputColorSpace;
    }

    if (m_PacketCaps.bVebox &&
        m_PacketCaps.bBeCSC &&
        cscParams->bCSCEnabled)
    {
        veboxIecpParams.bCSCEnable     = true;
        veboxIecpParams.pfCscCoeff     = m_fCscCoeff;
        veboxIecpParams.pfCscInOffset  = m_fCscInOffset;
        veboxIecpParams.pfCscOutOffset = m_fCscOutOffset;
        if ((cscParams->outputFormat == Format_A16B16G16R16F) || (cscParams->outputFormat == Format_A16R16G16B16F))
        {
            // Use CCM to do CSC for FP16 VEBOX output
            veboxIecpParams.bCSCEnable    = false;
            veboxIecpParams.bCcmCscEnable = true;
        }
    }

    VP_RENDER_CHK_STATUS_RETURN(SetVeboxOutputAlphaParams(cscParams));
    VP_RENDER_CHK_STATUS_RETURN(SetVeboxChromasitingParams(cscParams));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpVeboxCmdPacket::SetVeboxOutputAlphaParams(PVEBOX_CSC_PARAMS cscParams)
{
    VP_FUNC_CALL();

    VP_RENDER_CHK_NULL_RETURN(cscParams);

    VpVeboxRenderData* pRenderData = GetLastExecRenderData();
    VP_RENDER_CHK_NULL_RETURN(pRenderData);

    MHW_VEBOX_IECP_PARAMS& veboxIecpParams = pRenderData->GetIECPParams();

    bool isAlphaFromStateSelect = IS_ALPHA_FORMAT(cscParams->outputFormat)  &&
                                cscParams->alphaParams                      &&
                                (!IS_ALPHA_FORMAT(cscParams->inputFormat)   ||
                                VPHAL_ALPHA_FILL_MODE_SOURCE_STREAM != cscParams->alphaParams->AlphaMode);

    VP_RENDER_NORMALMESSAGE("AlphaFromStateSelect = %d", isAlphaFromStateSelect);

    if (isAlphaFromStateSelect)
    {
        veboxIecpParams.bAlphaEnable = true;
    }
    else
    {
        veboxIecpParams.bAlphaEnable = false;
        return MOS_STATUS_SUCCESS;
    }

    MOS_FORMAT outFormat = cscParams->outputFormat;

    auto SetOpaqueAlphaValue = [&](uint16_t &alphaValue)
    {
        if (Format_Y416 == cscParams->outputFormat)
        {
            alphaValue = 0xffff;
        }
        else
        {
            alphaValue = 0xff;
        }
    };

    if (cscParams->alphaParams != nullptr)
    {
        switch (cscParams->alphaParams->AlphaMode)
        {
        case VPHAL_ALPHA_FILL_MODE_NONE:
            if (outFormat == Format_A8R8G8B8)
            {
                veboxIecpParams.wAlphaValue =
                    (uint8_t)(0xff * cscParams->alphaParams->fAlpha);
            }
            else
            {
                SetOpaqueAlphaValue(veboxIecpParams.wAlphaValue);
            }
            break;

            // VEBOX does not support Background Color
        case VPHAL_ALPHA_FILL_MODE_BACKGROUND:

            // VPHAL_ALPHA_FILL_MODE_SOURCE_STREAM case is hit when the
            // input does not have alpha
            // So we set Opaque alpha channel.
        case VPHAL_ALPHA_FILL_MODE_SOURCE_STREAM:
        case VPHAL_ALPHA_FILL_MODE_OPAQUE:
        default:
            SetOpaqueAlphaValue(veboxIecpParams.wAlphaValue);
            break;
        }
    }
    else
    {
        SetOpaqueAlphaValue(veboxIecpParams.wAlphaValue);
    }

    VP_RENDER_NORMALMESSAGE("wAlphaValue = %d", veboxIecpParams.wAlphaValue);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpVeboxCmdPacket::SetVeboxChromasitingParams(PVEBOX_CSC_PARAMS cscParams)
{
    VP_FUNC_CALL();

    VP_RENDER_CHK_NULL_RETURN(cscParams);

    VpVeboxRenderData* pRenderData = GetLastExecRenderData();
    VP_RENDER_CHK_NULL_RETURN(pRenderData);

    MHW_VEBOX_CHROMA_SAMPLING& veboxChromaSamplingParams = pRenderData->GetChromaSubSamplingParams();

    veboxChromaSamplingParams.BypassChromaDownsampling                  = cscParams->bypassCDS;
    veboxChromaSamplingParams.BypassChromaUpsampling                    = cscParams->bypassCUS;
    veboxChromaSamplingParams.ChromaDownsamplingCoSitedHorizontalOffset = cscParams->chromaDownSamplingHorizontalCoef;
    veboxChromaSamplingParams.ChromaDownsamplingCoSitedVerticalOffset   = cscParams->chromaDownSamplingVerticalCoef;
    veboxChromaSamplingParams.ChromaUpsamplingCoSitedHorizontalOffset   = cscParams->chromaUpSamplingHorizontalCoef;
    veboxChromaSamplingParams.ChromaUpsamplingCoSitedVerticalOffset     = cscParams->chromaUpSamplingVerticalCoef;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpVeboxCmdPacket::SetSfcRotMirParams(PSFC_ROT_MIR_PARAMS rotMirParams)
{
    VP_FUNC_CALL();

    VP_PUBLIC_CHK_NULL_RETURN(rotMirParams);

    if (m_PacketCaps.bSFC)
    {
        VP_PUBLIC_CHK_NULL_RETURN(m_sfcRender);
        VP_PUBLIC_CHK_STATUS_RETURN(m_sfcRender->SetRotMirParams(rotMirParams));
        return MOS_STATUS_SUCCESS;
    }
    else
    {
        VP_RENDER_NORMALMESSAGE("CSC/IEF for Output is enabled in SFC, pls recheck the features enabling in SFC");
        return MOS_STATUS_INVALID_PARAMETER;
    }

}

MOS_STATUS VpVeboxCmdPacket::SetSteParams(
    PVEBOX_STE_PARAMS                    pSteParams)
{
    VP_FUNC_CALL();

    VpVeboxRenderData *pRenderData = GetLastExecRenderData();
    VP_PUBLIC_CHK_NULL_RETURN(pSteParams);
    VP_PUBLIC_CHK_NULL_RETURN(pRenderData);

    // ConfigureSteParams() just includes logic that both used in SetSteParams and UpdateSteParams.
    // If the logic won't be used in UpdateSteParams, please just add the logic into SetSteParams.
    VP_PUBLIC_CHK_STATUS_RETURN(ConfigureSteParams(pRenderData, pSteParams->bEnableSTE, pSteParams->dwSTEFactor, pSteParams->bEnableSTD, pSteParams->STDParam.paraSizeInBytes, pSteParams->STDParam.param));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpVeboxCmdPacket::UpdateCscParams(FeatureParamCsc &params)
{
    VP_FUNC_CALL();
    // Csc only can be apply to SFC path
    if (m_PacketCaps.bSfcCsc)
    {
        VP_PUBLIC_CHK_STATUS_RETURN(m_sfcRender->UpdateCscParams(params));
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpVeboxCmdPacket::UpdateDenoiseParams(FeatureParamDenoise &params)
{
    VP_FUNC_CALL();

    VpVeboxRenderData *pRenderData = GetLastExecRenderData();
    VP_RENDER_CHK_NULL_RETURN(pRenderData);

    // ConfigureDenoiseParams() just includes logic that both used in SetDenoiseParams and UpdateDenoiseParams..
    VP_PUBLIC_CHK_STATUS_RETURN(ConfigureDenoiseParams(pRenderData, params.denoiseParams.fDenoiseFactor));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpVeboxCmdPacket::UpdateTccParams(FeatureParamTcc &params)
{
    VP_FUNC_CALL();
    VpVeboxRenderData *pRenderData = GetLastExecRenderData();
    VP_RENDER_CHK_NULL_RETURN(pRenderData);

    // ConfigureTccParams() just includes logic that both used in SetTccParams and UpdateTccParams.
    VP_PUBLIC_CHK_STATUS_RETURN(ConfigureTccParams(pRenderData, params.bEnableTCC, params.Magenta, params.Red, params.Yellow, params.Green, params.Cyan, params.Blue));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpVeboxCmdPacket::UpdateSteParams(FeatureParamSte &params)
{
    VP_FUNC_CALL();
    VpVeboxRenderData *pRenderData = GetLastExecRenderData();
    VP_RENDER_CHK_NULL_RETURN(pRenderData);

    // ConfigureSteParams() just includes logic that both used in SetSteParams and UpdateSteParams.
    VP_PUBLIC_CHK_STATUS_RETURN(ConfigureSteParams(pRenderData, params.bEnableSTE, params.dwSTEFactor, params.bEnableSTD, params.STDParam.paraSizeInBytes, params.STDParam.param));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpVeboxCmdPacket::UpdateProcampParams(FeatureParamProcamp &params)
{
    VP_FUNC_CALL();

    VpVeboxRenderData *pRenderData = GetLastExecRenderData();
    VP_RENDER_CHK_NULL_RETURN(pRenderData);
    PVPHAL_PROCAMP_PARAMS pProcampParams = params.procampParams;
    VP_RENDER_CHK_NULL_RETURN(pProcampParams);

    // ConfigureProcampParams() just includes logic that both used in SetProcampParams and UpdateProcampParams.
    VP_PUBLIC_CHK_STATUS_RETURN(ConfigureProcampParams(pRenderData, pProcampParams->bEnabled, pProcampParams->fBrightness, pProcampParams->fContrast, pProcampParams->fHue, pProcampParams->fSaturation));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpVeboxCmdPacket::SetTccParams(
    PVEBOX_TCC_PARAMS                    pTccParams)
{
    VP_FUNC_CALL();

    VpVeboxRenderData *pRenderData = GetLastExecRenderData();
    VP_PUBLIC_CHK_NULL_RETURN(pTccParams);
    VP_PUBLIC_CHK_NULL_RETURN(pRenderData);

    // ConfigureTccParams() just includes logic that both used in SetTccParams and UpdateTccParams.
    // If the logic won't be used in UpdateTccParams, please just add the logic into SetTccParams.
    VP_PUBLIC_CHK_STATUS_RETURN(ConfigureTccParams(pRenderData, pTccParams->bEnableTCC, pTccParams->Magenta, 
        pTccParams->Red, pTccParams->Yellow, pTccParams->Green, pTccParams->Cyan, pTccParams->Blue));
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpVeboxCmdPacket::ValidateHDR3DLutParameters(bool is3DLutTableFilled)
{
    VP_FUNC_CALL();
    if (!is3DLutTableFilled)
    {
        // 3DLut need be calculated and filled by 3DLut kernel.
        VP_RENDER_ASSERTMESSAGE("3DLut is not filled!");
        VP_RENDER_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
    }
    return MOS_STATUS_SUCCESS;
}

void VpVeboxCmdPacket::VeboxGetBeCSCMatrix(
    VPHAL_CSPACE    inputColorSpace,
    VPHAL_CSPACE    outputColorSpace,
    MOS_FORMAT      inputFormat)
{
    VP_FUNC_CALL();

    // Get the matrix to use for conversion
    VpHal_GetCscMatrix(
        inputColorSpace,
        outputColorSpace,
        m_fCscCoeff,
        m_fCscInOffset,
        m_fCscOutOffset);

    // Vebox CSC converts RGB input to YUV for SFC
    // Vebox only supports A8B8G8R8 input, swap the 1st and 3rd
    // columns of the transfer matrix for A8R8G8B8 and X8R8G8B8
    // This only happens when SFC output is used
    if (inputFormat == Format_A8R8G8B8 ||
        inputFormat == Format_X8R8G8B8)
    {
        if (m_PacketCaps.bSFC || inputColorSpace != outputColorSpace)
        {
            VP_RENDER_NORMALMESSAGE("Swap R and B for format %d, sfc %d, inputColorSpace %d, outputColorSpace %d",
                inputFormat, m_PacketCaps.bSFC, inputColorSpace, outputColorSpace);
            float   fTemp[3] = {};
            fTemp[0] = m_fCscCoeff[0];
            fTemp[1] = m_fCscCoeff[3];
            fTemp[2] = m_fCscCoeff[6];

            m_fCscCoeff[0] = m_fCscCoeff[2];
            m_fCscCoeff[3] = m_fCscCoeff[5];
            m_fCscCoeff[6] = m_fCscCoeff[8];

            m_fCscCoeff[2] = fTemp[0];
            m_fCscCoeff[5] = fTemp[1];
            m_fCscCoeff[8] = fTemp[2];
        }
        else
        {
            // Do not swap since no more process needed.
            VP_RENDER_NORMALMESSAGE("Not swap R and B for format %d, sfc %d, inputColorSpace %d, outputColorSpace %d",
                inputFormat, m_PacketCaps.bSFC, inputColorSpace, outputColorSpace);
        }
    }
}

MOS_STATUS VpVeboxCmdPacket::UpdateDnHVSParameters(
    uint32_t *pStatSlice0GNEPtr,
    uint32_t *pStatSlice1GNEPtr)
{
    VP_FUNC_CALL();
    uint32_t                  dwGNELuma = 0, dwGNEChromaU = 0, dwGNEChromaV = 0;
    uint32_t                  dwSGNELuma = 0, dwSGNEChromaU = 0, dwSGNEChromaV = 0;
    uint32_t                  dwGNECountLuma = 0, dwGNECountChromaU = 0, dwGNECountChromaV = 0;
    uint32_t                  dwSGNECountLuma = 0, dwSGNECountChromaU = 0, dwSGNECountChromaV;
    uint32_t                  resltn        = 0;
    int32_t                   sgne_offset   = 0;
    VP_PACKET_SHARED_CONTEXT *sharedContext = (VP_PACKET_SHARED_CONTEXT *)m_packetSharedContext;
    VpVeboxRenderData        *renderData    = GetLastExecRenderData();
    VP_PUBLIC_CHK_NULL_RETURN(renderData);
    VP_PUBLIC_CHK_NULL_RETURN(sharedContext);
    VP_PUBLIC_CHK_NULL_RETURN(m_hwInterface);
    VP_PUBLIC_CHK_NULL_RETURN(m_veboxItf);
    VP_PUBLIC_CHK_NULL_RETURN(m_hwInterface->m_waTable);
    VP_PUBLIC_CHK_NULL_RETURN(m_currentSurface);
    VP_PUBLIC_CHK_NULL_RETURN(m_currentSurface->osSurface);
    auto &tgneParams = sharedContext->tgneParams;
    auto &hvsParams  = sharedContext->hvsParams;
    resltn           = m_currentSurface->rcSrc.right * m_currentSurface->rcSrc.bottom;

    // Set HVS kernel params
    hvsParams.Format            = 0;
    hvsParams.Mode              = renderData->GetHVSParams().Mode;
    hvsParams.Fallback          = !m_bTgneValid && !sharedContext->isVeboxFirstFrame && !tgneParams.bTgneFirstFrame;
    hvsParams.EnableChroma      = renderData->DN.bChromaDnEnabled;
    hvsParams.FirstFrame        = sharedContext->isVeboxFirstFrame;
    hvsParams.TgneFirstFrame    = !sharedContext->isVeboxFirstFrame && tgneParams.bTgneFirstFrame;
    hvsParams.EnableTemporalGNE = m_bTgneEnable;
    hvsParams.Width             = (uint16_t)m_currentSurface->rcSrc.right;
    hvsParams.Height            = (uint16_t)m_currentSurface->rcSrc.bottom;

    // Set QP
    if (renderData->GetHVSParams().Mode == HVSDENOISE_MANUAL)
    {
        if (renderData->GetHVSParams().QP <= 18)
        {
            hvsParams.QP = 0;
        }
        else if (renderData->GetHVSParams().QP <= 22)
        {
            hvsParams.QP = 1;
        }
        else if (renderData->GetHVSParams().QP <= 27)
        {
            hvsParams.QP = 2;
        }
        else if (renderData->GetHVSParams().QP <= 32)
        {
            hvsParams.QP = 3;
        }
        else if (renderData->GetHVSParams().QP <= 37)
        {
            hvsParams.QP = 4;
        }
    }
    else
    {
        hvsParams.QP = renderData->GetHVSParams().QP;
    }

    // Combine the GNE in slice0 and slice1 to generate the global GNE and Count
    dwGNELuma         = pStatSlice0GNEPtr[0] + pStatSlice1GNEPtr[0];
    dwGNEChromaU      = pStatSlice0GNEPtr[1] + pStatSlice1GNEPtr[1];
    dwGNEChromaV      = pStatSlice0GNEPtr[2] + pStatSlice1GNEPtr[2];
    dwGNECountLuma    = pStatSlice0GNEPtr[3] + pStatSlice1GNEPtr[3];
    dwGNECountChromaU = pStatSlice0GNEPtr[4] + pStatSlice1GNEPtr[4];
    dwGNECountChromaV = pStatSlice0GNEPtr[5] + pStatSlice1GNEPtr[5];

    // Validate GNE
    if (dwGNELuma == 0xFFFFFFFF || dwGNECountLuma == 0xFFFFFFFF ||
        dwGNEChromaU == 0xFFFFFFFF || dwGNECountChromaU == 0xFFFFFFFF ||
        dwGNEChromaV == 0xFFFFFFFF || dwGNECountChromaV == 0xFFFFFFF)
    {
        VP_RENDER_ASSERTMESSAGE("Incorrect GNE / GNE count.");
        return MOS_STATUS_UNKNOWN;
    }

    // Set HVS Mode in Mhw interface
    if (renderData->GetHVSParams().Mode == HVSDENOISE_AUTO_BDRATE)
    {
        hvsParams.hVSAutoBdrateEnable = true;
        tgneParams.dwBSDThreshold      = (resltn < NOSIE_GNE_RESOLUTION_THRESHOLD) ? 240 : 135;
    }
    else if (renderData->GetHVSParams().Mode == HVSDENOISE_AUTO_SUBJECTIVE)
    {
        hvsParams.hVSAutoSubjectiveEnable = true;
    }
    else
    {
        hvsParams.hVSAutoBdrateEnable     = false;
        hvsParams.hVSAutoSubjectiveEnable = false;
    }
    m_veboxItf->SetgnHVSMode(hvsParams.hVSAutoBdrateEnable, hvsParams.hVSAutoSubjectiveEnable, tgneParams.dwBSDThreshold);

    if (m_bTgneEnable && !sharedContext->isVeboxFirstFrame && tgneParams.bTgneFirstFrame)  //Second frame
    {
        tgneParams.bTgneFirstFrame = false;  // next frame bTgneFirstFrame should be false

        // caculate GNE
        dwGNELuma    = dwGNELuma * 100 / (dwGNECountLuma + 1);
        dwGNEChromaU = dwGNEChromaU * 100 / (dwGNECountChromaU + 1);
        dwGNEChromaV = dwGNEChromaV * 100 / (dwGNECountChromaV + 1);

        // Set some mhw params
        tgneParams.bTgneEnable  = true;
        tgneParams.lumaStadTh   = 3200;
        tgneParams.chromaStadTh = 1600;

        if (MEDIA_IS_WA(m_hwInterface->m_waTable, Wa_1609102037) &&
            VpHalDDIUtils::GetSurfaceColorPack(m_currentSurface->osSurface->Format) == VPHAL_COLORPACK_444)
        {
            tgneParams.dw4X4TGNEThCnt = ((m_currentSurface->osSurface->dwWidth - 32) *
                                            (m_currentSurface->osSurface->dwHeight - 8)) /
                                        1600;
        }
        else
        {
            tgneParams.dw4X4TGNEThCnt = ((m_currentSurface->osSurface->dwWidth - 8) *
                                            (m_currentSurface->osSurface->dwHeight - 8)) /
                                        1600;
        }

        if (renderData->GetHVSParams().Mode == HVSDENOISE_AUTO_BDRATE)
        {
            tgneParams.lumaStadTh    = 250;
            tgneParams.chromaStadTh  = 250;
            tgneParams.dwHistoryInit = 27;
            GNELumaConsistentCheck(dwGNELuma, pStatSlice0GNEPtr, pStatSlice1GNEPtr);
            // caculate temporal GNE
            dwGlobalNoiseLevel_Temporal  = (dwGNELuma + 50) / 100;
            dwGlobalNoiseLevelU_Temporal = (dwGNEChromaU + 50) / 100;
            dwGlobalNoiseLevelV_Temporal = (dwGNEChromaV + 50) / 100;
        }
        m_veboxItf->SetgnHVSParams(
            tgneParams.bTgneEnable, tgneParams.lumaStadTh, tgneParams.chromaStadTh, tgneParams.dw4X4TGNEThCnt, tgneParams.dwHistoryInit, hvsParams.hVSfallback);

        hvsParams.Sgne_Level          = dwGNELuma;
        hvsParams.Sgne_LevelU         = dwGNEChromaU;
        hvsParams.Sgne_LevelV         = dwGNEChromaV;
        hvsParams.Sgne_Count          = dwGNECountLuma;
        hvsParams.Sgne_CountU         = dwGNECountChromaU;
        hvsParams.Sgne_CountV         = dwGNECountChromaV;
        hvsParams.PrevNslvTemporal    = -1;
        hvsParams.PrevNslvTemporalU   = -1;
        hvsParams.PrevNslvTemporalV   = -1;
        hvsParams.dwGlobalNoiseLevel  = dwGlobalNoiseLevel_Temporal;
        hvsParams.dwGlobalNoiseLevelU = dwGlobalNoiseLevelU_Temporal;
        hvsParams.dwGlobalNoiseLevelV = dwGlobalNoiseLevelV_Temporal;
    }
    else if (m_bTgneEnable && m_bTgneValid && !sharedContext->isVeboxFirstFrame)  //Middle frame
    {
        dwGNECountLuma    = (pStatSlice0GNEPtr[3] & 0x7FFFFFFF) + (pStatSlice1GNEPtr[3] & 0x7FFFFFFF);
        dwGNECountChromaU = (pStatSlice0GNEPtr[4] & 0x7FFFFFFF) + (pStatSlice1GNEPtr[4] & 0x7FFFFFFF);
        dwGNECountChromaV = (pStatSlice0GNEPtr[5] & 0x7FFFFFFF) + (pStatSlice1GNEPtr[5] & 0x7FFFFFFF);

        sgne_offset        = -12;
        dwSGNELuma         = pStatSlice0GNEPtr[sgne_offset] + pStatSlice1GNEPtr[sgne_offset];
        dwSGNEChromaU      = pStatSlice0GNEPtr[sgne_offset + 1] + pStatSlice1GNEPtr[sgne_offset + 1];
        dwSGNEChromaV      = pStatSlice0GNEPtr[sgne_offset + 2] + pStatSlice1GNEPtr[sgne_offset + 2];
        dwSGNECountLuma    = pStatSlice0GNEPtr[sgne_offset + 3] + pStatSlice1GNEPtr[sgne_offset + 3];
        dwSGNECountChromaU = pStatSlice0GNEPtr[sgne_offset + 4] + pStatSlice1GNEPtr[sgne_offset + 4];
        dwSGNECountChromaV = pStatSlice0GNEPtr[sgne_offset + 5] + pStatSlice1GNEPtr[sgne_offset + 5];

        // Validate TGNE
        if (dwGNECountLuma == 0 || dwGNECountChromaU == 0 || dwGNECountChromaV == 0 ||
            dwSGNELuma == 0xFFFFFFFF || dwSGNECountLuma == 0xFFFFFFFF ||
            dwSGNEChromaU == 0xFFFFFFFF || dwSGNECountChromaU == 0xFFFFFFFF ||
            dwSGNEChromaV == 0xFFFFFFFF || dwSGNECountChromaV == 0xFFFFFFF)
        {
            VP_RENDER_ASSERTMESSAGE("Incorrect GNE count.");
            return MOS_STATUS_UNKNOWN;
        }

        curNoiseLevel_Temporal  = dwGNELuma / dwGNECountLuma;
        curNoiseLevelU_Temporal = dwGNEChromaU / dwGNECountChromaU;
        curNoiseLevelV_Temporal = dwGNEChromaV / dwGNECountChromaV;

        if (hvsParams.hVSfallback)
        {
            // last frame is fallback frame, nosie level use current noise level
            dwGlobalNoiseLevel_Temporal  = curNoiseLevel_Temporal;
            dwGlobalNoiseLevelU_Temporal = curNoiseLevelU_Temporal;
            dwGlobalNoiseLevelV_Temporal = curNoiseLevelU_Temporal;
            hvsParams.hVSfallback       = false;
        }
        else
        {
            // last frame is tgne frame, noise level use previous and current noise level
            dwGlobalNoiseLevel_Temporal  = MOS_ROUNDUP_SHIFT(dwGlobalNoiseLevel_Temporal + curNoiseLevel_Temporal, 1);
            dwGlobalNoiseLevelU_Temporal = MOS_ROUNDUP_SHIFT(dwGlobalNoiseLevelU_Temporal + curNoiseLevelU_Temporal, 1);
            dwGlobalNoiseLevelV_Temporal = MOS_ROUNDUP_SHIFT(dwGlobalNoiseLevelV_Temporal + curNoiseLevelV_Temporal, 1);
        }

        //// Set mhw parameters
        tgneParams.bTgneEnable  = true;
        tgneParams.lumaStadTh   = (dwGlobalNoiseLevel_Temporal <= 1) ? 3200 : (tgneParams.lumaStadTh + (curNoiseLevel_Temporal << 1) + 1) >> 1;
        tgneParams.chromaStadTh = (dwGlobalNoiseLevelU_Temporal <= 1 || dwGlobalNoiseLevelV_Temporal <= 1) ? 1600 : (tgneParams.chromaStadTh + curNoiseLevelU_Temporal + curNoiseLevelV_Temporal + 1) >> 1;

        if (renderData->GetHVSParams().Mode == HVSDENOISE_AUTO_BDRATE)
        {
            tgneParams.lumaStadTh    = 250;
            tgneParams.chromaStadTh  = 250;
            tgneParams.dwHistoryInit = 27;
        }

        if (MEDIA_IS_WA(m_hwInterface->m_waTable, Wa_1609102037) &&
            VpHalDDIUtils::GetSurfaceColorPack(m_currentSurface->osSurface->Format) == VPHAL_COLORPACK_444)
        {
            tgneParams.dw4X4TGNEThCnt = ((m_currentSurface->osSurface->dwWidth - 32) *
                                            (m_currentSurface->osSurface->dwHeight - 8)) /
                                        1600;
        }
        else
        {
            tgneParams.dw4X4TGNEThCnt = ((m_currentSurface->osSurface->dwWidth - 8) *
                                            (m_currentSurface->osSurface->dwHeight - 8)) /
                                        1600;
        }
        m_veboxItf->SetgnHVSParams(
            tgneParams.bTgneEnable, tgneParams.lumaStadTh, tgneParams.chromaStadTh, tgneParams.dw4X4TGNEThCnt, tgneParams.dwHistoryInit, hvsParams.hVSfallback);

        hvsParams.Sgne_Level          = dwSGNELuma * 100 / (dwSGNECountLuma + 1);
        hvsParams.Sgne_LevelU         = dwSGNEChromaU * 100 / (dwSGNECountChromaU + 1);
        hvsParams.Sgne_LevelV         = dwSGNEChromaV * 100 / (dwSGNECountChromaV + 1);
        hvsParams.Sgne_Count          = dwSGNECountLuma;
        hvsParams.Sgne_CountU         = dwSGNECountChromaU;
        hvsParams.Sgne_CountV         = dwSGNECountChromaV;
        hvsParams.PrevNslvTemporal    = curNoiseLevel_Temporal;
        hvsParams.PrevNslvTemporalU   = curNoiseLevelU_Temporal;
        hvsParams.PrevNslvTemporalV   = curNoiseLevelV_Temporal;
        hvsParams.dwGlobalNoiseLevel  = dwGlobalNoiseLevel_Temporal;
        hvsParams.dwGlobalNoiseLevelU = dwGlobalNoiseLevelU_Temporal;
        hvsParams.dwGlobalNoiseLevelV = dwGlobalNoiseLevelV_Temporal;
    }
    else  //First frame and fallback frame
    {
        dwGNELuma    = dwGNELuma * 100 / (dwGNECountLuma + 1);
        dwGNEChromaU = dwGNEChromaU * 100 / (dwGNECountChromaU + 1);
        dwGNEChromaV = dwGNEChromaV * 100 / (dwGNECountChromaV + 1);

        GNELumaConsistentCheck(dwGNELuma, pStatSlice0GNEPtr, pStatSlice1GNEPtr);

        // Set some mhw params
        if (hvsParams.Fallback)
        {
            hvsParams.hVSfallback   = true;
            tgneParams.bTgneEnable   = true;
            tgneParams.dwHistoryInit = 32;
        }
        else
        {
            if (renderData->GetHVSParams().Mode == HVSDENOISE_AUTO_BDRATE)
            {
                tgneParams.dwHistoryInit = 32;
            }
            tgneParams.bTgneEnable    = false;
            tgneParams.lumaStadTh     = 0;
            tgneParams.chromaStadTh   = 0;
            tgneParams.dw4X4TGNEThCnt = 0;
        }
        m_veboxItf->SetgnHVSParams(
            tgneParams.bTgneEnable, tgneParams.lumaStadTh, tgneParams.chromaStadTh, tgneParams.dw4X4TGNEThCnt, tgneParams.dwHistoryInit, hvsParams.hVSfallback);

        hvsParams.Sgne_Level          = dwGNELuma;
        hvsParams.Sgne_LevelU         = dwGNEChromaU;
        hvsParams.Sgne_LevelV         = dwGNEChromaV;
        hvsParams.Sgne_Count          = dwGNECountLuma;
        hvsParams.Sgne_CountU         = dwGNECountChromaU;
        hvsParams.Sgne_CountV         = dwGNECountChromaV;
        hvsParams.PrevNslvTemporal    = -1;
        hvsParams.PrevNslvTemporalU   = -1;
        hvsParams.PrevNslvTemporalV   = -1;
        hvsParams.dwGlobalNoiseLevel  = dwGNELuma;
        hvsParams.dwGlobalNoiseLevelU = dwGNEChromaU;
        hvsParams.dwGlobalNoiseLevelV = dwGNEChromaV;
    }

    if (renderData->GetHVSParams().Mode == HVSDENOISE_MANUAL)
    {
        hvsParams.dwGlobalNoiseLevel  = renderData->GetHVSParams().Strength;
        hvsParams.dwGlobalNoiseLevelU = renderData->GetHVSParams().Strength;
        hvsParams.dwGlobalNoiseLevelV = renderData->GetHVSParams().Strength;
    }

    VP_PUBLIC_NORMALMESSAGE("HVS Kernel params: hvsParams.QP %d, hvsParams.Mode %d, hvsParams.Width %d, hvsParams.Height %d, hvsParams.Format %d", hvsParams.QP, hvsParams.Strength, hvsParams.Mode, hvsParams.Width, hvsParams.Height, hvsParams.Format);
    VP_PUBLIC_NORMALMESSAGE("HVS Kernel params: hvsParams.FirstFrame %d, hvsParams.TgneFirstFrame %d, hvsParams.EnableTemporalGNE %d, hvsParams.Fallback %d, hvsParams.EnableChroma %d", hvsParams.FirstFrame, hvsParams.TgneFirstFrame, hvsParams.EnableTemporalGNE, hvsParams.Fallback, hvsParams.EnableChroma);
    VP_PUBLIC_NORMALMESSAGE("HVS Kernel params: hvsParams.dwGlobalNoiseLevel %d, hvsParams.dwGlobalNoiseLevelU %d, hvsParams.dwGlobalNoiseLevelV %d", hvsParams.dwGlobalNoiseLevel, hvsParams.dwGlobalNoiseLevelU, hvsParams.dwGlobalNoiseLevelV);
    VP_PUBLIC_NORMALMESSAGE("HVS Kernel params: hvsParams.PrevNslvTemporal %d, hvsParams.PrevNslvTemporalU %d, hvsParams.PrevNslvTemporalV %d", hvsParams.PrevNslvTemporal, hvsParams.PrevNslvTemporalU, hvsParams.PrevNslvTemporalV);
    VP_PUBLIC_NORMALMESSAGE("HVS Kernel params: hvsParams.Sgne_Count %d, hvsParams.Sgne_CountU %d, hvsParams.Sgne_CountV %d", hvsParams.Sgne_Count, hvsParams.Sgne_CountU, hvsParams.Sgne_CountV);
    VP_PUBLIC_NORMALMESSAGE("HVS Kernel params: hvsParams.Sgne_Level %d, hvsParams.Sgne_LevelU %d, hvsParams.Sgne_LevelV %d", hvsParams.Sgne_Level, hvsParams.Sgne_LevelU, hvsParams.Sgne_LevelV);
    VP_PUBLIC_NORMALMESSAGE("HVS Kernel params: veboxInterface->dwLumaStadTh %d, veboxInterface->dwChromaStadTh %d", tgneParams.lumaStadTh, tgneParams.chromaStadTh);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpVeboxCmdPacket::SetupDNTableForHVS(
    mhw::vebox::VEBOX_STATE_PAR &veboxStateCmdParams)
{
    VP_FUNC_CALL();
    VpVeboxRenderData *renderData   = GetLastExecRenderData();
    PVP_SURFACE        surfHVSTable = GetSurface(SurfaceTypeHVSTable);
    VP_RENDER_CHK_NULL_RETURN(renderData);

    if (nullptr == surfHVSTable || !renderData->DN.bHvsDnEnabled)
    {
        // It is just for update HVS Kernel DNDI table.
        // it will not overwrite params in SetDnParams() for other DN feature.
        VP_RENDER_NORMALMESSAGE("HVS DN DI table not need be updated.");
        return MOS_STATUS_SUCCESS;
    }

    VP_RENDER_CHK_NULL_RETURN(m_allocator);
    VP_RENDER_CHK_NULL_RETURN(surfHVSTable);
    VP_RENDER_CHK_NULL_RETURN(surfHVSTable->osSurface);

    // Just HVS kernel stage will call this function.
    // And SetDnParams() only call before the first stage.
    // It wil not overwrite the params caculated here.
    VP_RENDER_NORMALMESSAGE("HVS DN DI table caculated by kernel.");

    uint8_t  *bufHVSSurface      = (uint8_t *)m_allocator->LockResourceForWrite(&surfHVSTable->osSurface->OsResource);
    uint32_t *bufHVSDenoiseParam = (uint32_t *)bufHVSSurface;
    VP_RENDER_CHK_NULL_RETURN(bufHVSSurface);
    VP_RENDER_CHK_NULL_RETURN(bufHVSDenoiseParam);

    VP_RENDER_NORMALMESSAGE("Set HVS Denoised Parameters to VEBOX DNDI params");
    // DW0
    renderData->GetDNDIParams().dwDenoiseMaximumHistory = (bufHVSDenoiseParam[0] & 0x000000ff);
    VP_RENDER_NORMALMESSAGE("HVS: renderData->GetDNDIParams().dwDenoiseMaximumHistory %d", renderData->GetDNDIParams().dwDenoiseMaximumHistory);
    renderData->GetDNDIParams().dwDenoiseSTADThreshold = (bufHVSDenoiseParam[0] & 0xfffe0000) >> 17;
    VP_RENDER_NORMALMESSAGE("HVS: renderData->GetDNDIParams().dwDenoiseSTADThreshold %d", renderData->GetDNDIParams().dwDenoiseSTADThreshold);
    // DW1
    renderData->GetDNDIParams().dwDenoiseASDThreshold = (bufHVSDenoiseParam[1] & 0x00000fff);
    VP_RENDER_NORMALMESSAGE("HVS: renderData->GetDNDIParams().dwDenoiseASDThreshold %d", renderData->GetDNDIParams().dwDenoiseASDThreshold);
    renderData->GetDNDIParams().dwDenoiseMPThreshold = (bufHVSDenoiseParam[1] & 0x0f800000) >> 23;
    VP_RENDER_NORMALMESSAGE("HVS: renderData->GetDNDIParams().dwDenoiseMPThreshold %d", renderData->GetDNDIParams().dwDenoiseMPThreshold);
    renderData->GetDNDIParams().dwDenoiseHistoryDelta = (bufHVSDenoiseParam[1] & 0xf0000000) >> 28;
    VP_RENDER_NORMALMESSAGE("HVS: renderData->GetDNDIParams().dwDenoiseHistoryDelta %d", renderData->GetDNDIParams().dwDenoiseHistoryDelta);
    // DW2
    renderData->GetDNDIParams().dwTDThreshold = (bufHVSDenoiseParam[2] & 0xfff00000) >> 20;
    VP_RENDER_NORMALMESSAGE("HVS: renderData->GetDNDIParams().dwTDThreshold %d", renderData->GetDNDIParams().dwTDThreshold);
    // DW3
    renderData->GetDNDIParams().dwLTDThreshold = (bufHVSDenoiseParam[3] & 0xfff00000) >> 20;
    VP_RENDER_NORMALMESSAGE("HVS: renderData->GetDNDIParams().dwLTDThreshold %d", renderData->GetDNDIParams().dwLTDThreshold);
    // DW4
    renderData->GetDNDIParams().dwDenoiseSCMThreshold = (bufHVSDenoiseParam[4] & 0xfff00000) >> 20;
    VP_RENDER_NORMALMESSAGE("HVS: renderData->GetDNDIParams().dwDenoiseSCMThreshold %d", renderData->GetDNDIParams().dwDenoiseSCMThreshold);
    // DW5
    renderData->GetDNDIParams().dwChromaSTADThreshold = (bufHVSDenoiseParam[5] & 0xfffe0000) >> 17;
    VP_RENDER_NORMALMESSAGE("HVS: renderData->GetDNDIParams().dwChromaSTADThreshold %d", renderData->GetDNDIParams().dwChromaSTADThreshold);
    // DW6
    renderData->GetDNDIParams().dwChromaTDThreshold = (bufHVSDenoiseParam[6] & 0xfff00000) >> 20;
    VP_RENDER_NORMALMESSAGE("HVS: renderData->GetDNDIParams().dwChromaTDThreshold %d", renderData->GetDNDIParams().dwChromaTDThreshold);
    // DW7
    renderData->GetDNDIParams().dwChromaLTDThreshold = (bufHVSDenoiseParam[7] & 0xfff00000) >> 20;
    VP_RENDER_NORMALMESSAGE("HVS: renderData->GetDNDIParams().dwChromaLTDThreshold %d", renderData->GetDNDIParams().dwChromaLTDThreshold);
    // DW9
    renderData->GetDNDIParams().dwPixRangeWeight[0] = (bufHVSDenoiseParam[9] & 0x0000001f);
    VP_RENDER_NORMALMESSAGE("HVS: renderData->GetDNDIParams().dwPixRangeWeight[0] %d", renderData->GetDNDIParams().dwPixRangeWeight[0]);
    renderData->GetDNDIParams().dwPixRangeWeight[1] = (bufHVSDenoiseParam[9] & 0x000003e0) >> 5;
    VP_RENDER_NORMALMESSAGE("HVS: renderData->GetDNDIParams().dwPixRangeWeight[1] %d", renderData->GetDNDIParams().dwPixRangeWeight[1]);
    renderData->GetDNDIParams().dwPixRangeWeight[2] = (bufHVSDenoiseParam[9] & 0x00007c00) >> 10;
    VP_RENDER_NORMALMESSAGE("HVS: renderData->GetDNDIParams().dwPixRangeWeight[2] %d", renderData->GetDNDIParams().dwPixRangeWeight[2]);
    renderData->GetDNDIParams().dwPixRangeWeight[3] = (bufHVSDenoiseParam[9] & 0x000f8000) >> 15;
    VP_RENDER_NORMALMESSAGE("HVS: renderData->GetDNDIParams().dwPixRangeWeight[3] %d", renderData->GetDNDIParams().dwPixRangeWeight[3]);
    renderData->GetDNDIParams().dwPixRangeWeight[4] = (bufHVSDenoiseParam[9] & 0x01f00000) >> 20;
    VP_RENDER_NORMALMESSAGE("HVS: renderData->GetDNDIParams().dwPixRangeWeight[4] %d", renderData->GetDNDIParams().dwPixRangeWeight[4]);
    renderData->GetDNDIParams().dwPixRangeWeight[5] = (bufHVSDenoiseParam[9] & 0x3e000000) >> 25;
    VP_RENDER_NORMALMESSAGE("HVS: renderData->GetDNDIParams().dwPixRangeWeight[5] %d", renderData->GetDNDIParams().dwPixRangeWeight[5]);
    // DW11
    renderData->GetDNDIParams().dwPixRangeThreshold[5] = (bufHVSDenoiseParam[11] & 0x1fff0000) >> 16;
    VP_RENDER_NORMALMESSAGE("HVS: renderData->GetDNDIParams().dwPixRangeThreshold[5] %d", renderData->GetDNDIParams().dwPixRangeThreshold[5]);
    // DW12
    renderData->GetDNDIParams().dwPixRangeThreshold[4] = (bufHVSDenoiseParam[12] & 0x1fff0000) >> 16;
    VP_RENDER_NORMALMESSAGE("HVS: renderData->GetDNDIParams().dwPixRangeThreshold[4] %d", renderData->GetDNDIParams().dwPixRangeThreshold[4]);
    renderData->GetDNDIParams().dwPixRangeThreshold[3] = (bufHVSDenoiseParam[12] & 0x00001fff);
    VP_RENDER_NORMALMESSAGE("HVS: renderData->GetDNDIParams().dwPixRangeThreshold[3] %d", renderData->GetDNDIParams().dwPixRangeThreshold[3]);
    // DW13
    renderData->GetDNDIParams().dwPixRangeThreshold[2] = (bufHVSDenoiseParam[13] & 0x1fff0000) >> 16;
    VP_RENDER_NORMALMESSAGE("HVS: renderData->GetDNDIParams().dwPixRangeThreshold[2] %d", renderData->GetDNDIParams().dwPixRangeThreshold[2]);
    renderData->GetDNDIParams().dwPixRangeThreshold[1] = (bufHVSDenoiseParam[13] & 0x00001fff);
    VP_RENDER_NORMALMESSAGE("HVS: renderData->GetDNDIParams().dwPixRangeThreshold[1] %d", renderData->GetDNDIParams().dwPixRangeThreshold[1]);
    // DW14
    renderData->GetDNDIParams().dwPixRangeThreshold[0] = (bufHVSDenoiseParam[14] & 0x1fff0000) >> 16;
    VP_RENDER_NORMALMESSAGE("HVS: renderData->GetDNDIParams().dwPixRangeThreshold[0] %d", renderData->GetDNDIParams().dwPixRangeThreshold[0]);
    // DW16
    veboxChromaParams.dwPixRangeWeightChromaU[0] = (bufHVSDenoiseParam[16] & 0x0000001f);
    VP_RENDER_NORMALMESSAGE("veboxChromaParams.dwPixRangeWeightChromaU[0] %d", veboxChromaParams.dwPixRangeWeightChromaU[0]);
    veboxChromaParams.dwPixRangeWeightChromaU[1] = (bufHVSDenoiseParam[16] & 0x000003e0) >> 5;
    VP_RENDER_NORMALMESSAGE("veboxChromaParams.dwPixRangeWeightChromaU[1] %d", veboxChromaParams.dwPixRangeWeightChromaU[1]);
    veboxChromaParams.dwPixRangeWeightChromaU[2] = (bufHVSDenoiseParam[16] & 0x00007c00) >> 10;
    VP_RENDER_NORMALMESSAGE("veboxChromaParams.dwPixRangeWeightChromaU[2] %d", veboxChromaParams.dwPixRangeWeightChromaU[2]);
    veboxChromaParams.dwPixRangeWeightChromaU[3] = (bufHVSDenoiseParam[16] & 0x000f8000) >> 15;
    VP_RENDER_NORMALMESSAGE("veboxChromaParams.dwPixRangeWeightChromaU[3] %d", veboxChromaParams.dwPixRangeWeightChromaU[3]);
    veboxChromaParams.dwPixRangeWeightChromaU[4] = (bufHVSDenoiseParam[16] & 0x01f00000) >> 20;
    VP_RENDER_NORMALMESSAGE("veboxChromaParams.dwPixRangeWeightChromaU[4] %d", veboxChromaParams.dwPixRangeWeightChromaU[4]);
    veboxChromaParams.dwPixRangeWeightChromaU[5] = (bufHVSDenoiseParam[16] & 0x3e000000) >> 25;
    VP_RENDER_NORMALMESSAGE("veboxChromaParams.dwPixRangeWeightChromaU[5] %d", veboxChromaParams.dwPixRangeWeightChromaU[5]);
    //DW18
    veboxChromaParams.dwPixRangeThresholdChromaU[5] = (bufHVSDenoiseParam[18] & 0x1fff0000) >> 16;
    VP_RENDER_NORMALMESSAGE("veboxChromaParams.dwPixRangeThresholdChromaU[5] %d", veboxChromaParams.dwPixRangeThresholdChromaU[5]);
    //DW19
    veboxChromaParams.dwPixRangeThresholdChromaU[4] = (bufHVSDenoiseParam[19] & 0x1fff0000) >> 16;
    VP_RENDER_NORMALMESSAGE("veboxChromaParams.dwPixRangeThresholdChromaU[4] %d", veboxChromaParams.dwPixRangeThresholdChromaU[4]);
    veboxChromaParams.dwPixRangeThresholdChromaU[3] = (bufHVSDenoiseParam[19] & 0x00001fff);
    VP_RENDER_NORMALMESSAGE("veboxChromaParams.dwPixRangeThresholdChromaU[3] %d", veboxChromaParams.dwPixRangeThresholdChromaU[3]);
    //DW20
    veboxChromaParams.dwPixRangeThresholdChromaU[2] = (bufHVSDenoiseParam[20] & 0x1fff0000) >> 16;
    VP_RENDER_NORMALMESSAGE("veboxChromaParams.dwPixRangeThresholdChromaU[2] %d", veboxChromaParams.dwPixRangeThresholdChromaU[2]);
    veboxChromaParams.dwPixRangeThresholdChromaU[1] = (bufHVSDenoiseParam[20] & 0x00001fff);
    VP_RENDER_NORMALMESSAGE("veboxChromaParams.dwPixRangeThresholdChromaU[1] %d", veboxChromaParams.dwPixRangeThresholdChromaU[1]);
    //DW21
    veboxChromaParams.dwPixRangeThresholdChromaU[0] = (bufHVSDenoiseParam[21] & 0x1fff0000) >> 16;
    VP_RENDER_NORMALMESSAGE("veboxChromaParams.dwPixRangeThresholdChromaU[0] %d", veboxChromaParams.dwPixRangeThresholdChromaU[0]);
    //DW23
    veboxChromaParams.dwPixRangeWeightChromaV[0] = (bufHVSDenoiseParam[23] & 0x0000001f);
    VP_RENDER_NORMALMESSAGE("veboxChromaParams.dwPixRangeWeightChromaV[0] %d", veboxChromaParams.dwPixRangeWeightChromaV[0]);
    veboxChromaParams.dwPixRangeWeightChromaV[1] = (bufHVSDenoiseParam[23] & 0x000003e0) >> 5;
    VP_RENDER_NORMALMESSAGE("veboxChromaParams.dwPixRangeWeightChromaV[1] %d", veboxChromaParams.dwPixRangeWeightChromaV[1]);
    veboxChromaParams.dwPixRangeWeightChromaV[2] = (bufHVSDenoiseParam[23] & 0x00007c00) >> 10;
    VP_RENDER_NORMALMESSAGE("veboxChromaParams.dwPixRangeWeightChromaV[2] %d", veboxChromaParams.dwPixRangeWeightChromaV[2]);
    veboxChromaParams.dwPixRangeWeightChromaV[3] = (bufHVSDenoiseParam[23] & 0x000f8000) >> 15;
    VP_RENDER_NORMALMESSAGE("veboxChromaParams.dwPixRangeWeightChromaV[3] %d", veboxChromaParams.dwPixRangeWeightChromaV[3]);
    veboxChromaParams.dwPixRangeWeightChromaV[4] = (bufHVSDenoiseParam[23] & 0x01f00000) >> 20;
    VP_RENDER_NORMALMESSAGE("veboxChromaParams.dwPixRangeWeightChromaV[4] %d", veboxChromaParams.dwPixRangeWeightChromaV[4]);
    veboxChromaParams.dwPixRangeWeightChromaV[5] = (bufHVSDenoiseParam[23] & 0x3e000000) >> 25;
    VP_RENDER_NORMALMESSAGE("veboxChromaParams.dwPixRangeWeightChromaV[5] %d", veboxChromaParams.dwPixRangeWeightChromaV[5]);
    //DW25
    veboxChromaParams.dwPixRangeThresholdChromaV[5] = (bufHVSDenoiseParam[25] & 0x1fff0000) >> 16;
    VP_RENDER_NORMALMESSAGE("veboxChromaParams.dwPixRangeThresholdChromaV[5] %d", veboxChromaParams.dwPixRangeThresholdChromaV[5]);
    //DW26
    veboxChromaParams.dwPixRangeThresholdChromaV[4] = (bufHVSDenoiseParam[26] & 0x1fff0000) >> 16;
    VP_RENDER_NORMALMESSAGE("veboxChromaParams.dwPixRangeThresholdChromaV[4] %d", veboxChromaParams.dwPixRangeThresholdChromaV[4]);
    veboxChromaParams.dwPixRangeThresholdChromaV[3] = (bufHVSDenoiseParam[26] & 0x00001fff);
    VP_RENDER_NORMALMESSAGE("veboxChromaParams.dwPixRangeThresholdChromaV[3] %d", veboxChromaParams.dwPixRangeThresholdChromaV[3]);
    //DW27
    veboxChromaParams.dwPixRangeThresholdChromaV[2] = (bufHVSDenoiseParam[27] & 0x1fff0000) >> 16;
    VP_RENDER_NORMALMESSAGE("veboxChromaParams.dwPixRangeThresholdChromaV[2] %d", veboxChromaParams.dwPixRangeThresholdChromaV[2]);
    veboxChromaParams.dwPixRangeThresholdChromaV[1] = (bufHVSDenoiseParam[27] & 0x00001fff);
    VP_RENDER_NORMALMESSAGE("veboxChromaParams.dwPixRangeThresholdChromaV[1] %d", veboxChromaParams.dwPixRangeThresholdChromaV[1]);
    //DW28
    veboxChromaParams.dwPixRangeThresholdChromaV[0] = (bufHVSDenoiseParam[28] & 0x1fff0000) >> 16;
    VP_RENDER_NORMALMESSAGE("veboxChromaParams.dwPixRangeThresholdChromaV[0] %d", veboxChromaParams.dwPixRangeThresholdChromaV[0]);

    VP_PUBLIC_CHK_STATUS_RETURN(m_allocator->UnLock(&surfHVSTable->osSurface->OsResource));

    //Set up the Vebox State in Clear Memory
    VP_PUBLIC_CHK_STATUS_RETURN(VpVeboxCmdPacket::AddVeboxDndiState());

    return MOS_STATUS_SUCCESS;
}
}