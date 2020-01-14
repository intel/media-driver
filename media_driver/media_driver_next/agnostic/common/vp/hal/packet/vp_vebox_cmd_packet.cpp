/*
* Copyright (c) 2018-2019, Intel Corporation
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
//! \file     vp_vebox_cmd_packet.cpp
//! \brief    vebox packet which used in by mediapipline.
//! \details  vebox packet provide the structures and generate the cmd buffer which mediapipline will used.
//!

#include "vp_vebox_cmd_packet.h"
#include "vp_utils.h"
#include "mos_resource_defs.h"
#include "hal_oca_interface.h"

namespace vp {

void VpVeboxCmdPacket::SetupSurfaceStates(
    bool                                    bDiVarianceEnable,
    PVPHAL_VEBOX_SURFACE_STATE_CMD_PARAMS   pVeboxSurfaceStateCmdParams)
{
    VP_PUBLIC_CHK_NULL_NO_STATUS_RETURN(pVeboxSurfaceStateCmdParams);
    MOS_ZeroMemory(pVeboxSurfaceStateCmdParams, sizeof(VPHAL_VEBOX_SURFACE_STATE_CMD_PARAMS));
    MOS_UNUSED(bDiVarianceEnable);
    pVeboxSurfaceStateCmdParams->pSurfInput  = m_veboxPacketSurface.pCurrInput;
    pVeboxSurfaceStateCmdParams->pSurfOutput = GetSurfOutput(m_PacketCaps.bDI);
    pVeboxSurfaceStateCmdParams->pSurfSTMM = m_veboxPacketSurface.pSTMMInput;
    pVeboxSurfaceStateCmdParams->pSurfDNOutput = m_veboxPacketSurface.pDenoisedCurrOutput;
    pVeboxSurfaceStateCmdParams->bDIEnable = m_PacketCaps.bDI;
}

MOS_STATUS VpVeboxCmdPacket::SetupVeboxState(
    bool                        bDiVarianceEnable,
    PMHW_VEBOX_STATE_CMD_PARAMS pVeboxStateCmdParams)
{
    PMHW_VEBOX_MODE         pVeboxMode   = nullptr;
    MOS_STATUS              eStatus      = MOS_STATUS_SUCCESS;

    VP_RENDER_CHK_NULL_RETURN(pVeboxStateCmdParams);
    VP_RENDER_CHK_NULL_RETURN(m_hwInterface);

    pVeboxMode = &pVeboxStateCmdParams->VeboxMode;

    VP_RENDER_CHK_NULL_RETURN(pVeboxMode);

    MOS_ZeroMemory(pVeboxStateCmdParams, sizeof(*pVeboxStateCmdParams));

    if (m_IsSfcUsed)
    {
        pVeboxMode->GlobalIECPEnable = true;
    }
    else
    {
        pVeboxMode->GlobalIECPEnable = m_PacketCaps.bIECP;
    }

    pVeboxMode->DIEnable = bDiVarianceEnable;

    pVeboxMode->SFCParallelWriteEnable = m_IsSfcUsed &&
                                            (m_PacketCaps.bDN || bDiVarianceEnable);
    pVeboxMode->DNEnable = m_PacketCaps.bDN;
    pVeboxMode->DNDIFirstFrame = (!m_bRefValid && (pVeboxMode->DNEnable || pVeboxMode->DIEnable));
    pVeboxMode->DIOutputFrames = m_DIOutputFrames;
    pVeboxMode->DisableEncoderStatistics = true;
    pVeboxMode->DisableTemporalDenoiseFilter = false;

    pVeboxStateCmdParams->bUseVeboxHeapKernelResource = UseKernelResource();

    //SetupChromaSampling(&pVeboxStateCmdParams->ChromaSampling);

    // Initialize VEBOX chroma sitting to bypass
    pVeboxStateCmdParams->ChromaSampling.BypassChromaUpsampling = 1;
    pVeboxStateCmdParams->ChromaSampling.ChromaUpsamplingCoSitedHorizontalOffset = 0;
    pVeboxStateCmdParams->ChromaSampling.ChromaUpsamplingCoSitedVerticalOffset = 0;
    pVeboxStateCmdParams->ChromaSampling.BypassChromaDownsampling = 1;
    pVeboxStateCmdParams->ChromaSampling.ChromaDownsamplingCoSitedHorizontalOffset = 0;
    pVeboxStateCmdParams->ChromaSampling.ChromaDownsamplingCoSitedVerticalOffset = 0;

    // Permanent program limitation that should go in all the configurations of SKLGT which have 2 VEBOXes (i.e. GT3 & GT4)
    // VEBOX1 should be disabled whenever there is an VE-SFC workload.
    // This is because we have only one SFC all the GT configurations and that SFC is tied to VEBOX0.Hence the programming restriction.
    if (m_IsSfcUsed)
    {
        pVeboxMode->SingleSliceVeboxEnable = 1;
    }
    else
    {
        pVeboxMode->SingleSliceVeboxEnable = 0;
    }
    return eStatus;
}

MOS_STATUS VpVeboxCmdPacket::InitCmdBufferWithVeParams(
    PRENDERHAL_INTERFACE pRenderHal,
    MOS_COMMAND_BUFFER & CmdBuffer,
    PRENDERHAL_GENERIC_PROLOG_PARAMS pGenericPrologParams)
{
    MOS_STATUS                              eStatus = MOS_STATUS_SUCCESS;
    RENDERHAL_GENERIC_PROLOG_PARAMS_G12     genericPrologParamsG12 = {};

    genericPrologParamsG12.bEnableMediaFrameTracking      = pGenericPrologParams->bEnableMediaFrameTracking;
    genericPrologParamsG12.bMmcEnabled                    = pGenericPrologParams->bMmcEnabled;
    genericPrologParamsG12.dwMediaFrameTrackingAddrOffset = pGenericPrologParams->dwMediaFrameTrackingAddrOffset;
    genericPrologParamsG12.dwMediaFrameTrackingTag        = pGenericPrologParams->dwMediaFrameTrackingTag;
    genericPrologParamsG12.presMediaFrameTrackingSurface  = pGenericPrologParams->presMediaFrameTrackingSurface;

    genericPrologParamsG12.VEngineHintParams.BatchBufferCount = 2;
    //genericPrologParamsG12.VEngineHintParams.resScalableBatchBufs[0] = CmdBuffer[0].OsResource;
    //genericPrologParamsG12.VEngineHintParams.resScalableBatchBufs[1] = CmdBuffer[1].OsResource;
    genericPrologParamsG12.VEngineHintParams.UsingFrameSplit = true;
    genericPrologParamsG12.VEngineHintParams.UsingSFC = false;
    genericPrologParamsG12.VEngineHintParams.EngineInstance[0] = 0;
    genericPrologParamsG12.VEngineHintParams.EngineInstance[1] = 1;
    genericPrologParamsG12.VEngineHintParams.NeedSyncWithPrevious = true;
    genericPrologParamsG12.VEngineHintParams.SameEngineAsLastSubmission = true;

    pRenderHal->pOsInterface->VEEnable = false;

    // Initialize command buffer and insert prolog
    VP_RENDER_CHK_STATUS_RETURN(pRenderHal->pfnInitCommandBuffer(
        pRenderHal,
        &CmdBuffer,
        (PRENDERHAL_GENERIC_PROLOG_PARAMS)&genericPrologParamsG12));

    return eStatus;
}

MOS_STATUS VpVeboxCmdPacket::InitSfcStateParams()
{
    if (!m_sfcRenderData.sfcStateParams)
    {
        m_sfcRenderData.sfcStateParams = (MHW_SFC_STATE_PARAMS_G12*)MOS_AllocAndZeroMemory(sizeof(MHW_SFC_STATE_PARAMS_G12));
    }
    else
    {
        MOS_ZeroMemory(m_sfcRenderData.sfcStateParams, sizeof(MHW_SFC_STATE_PARAMS_G12));
    }

    if (!m_sfcRenderData.sfcStateParams)
    {
        VP_RENDER_ASSERTMESSAGE("No Space for params allocation");
        return MOS_STATUS_NO_SPACE;
    }

    return MOS_STATUS_SUCCESS;
}

//!
//! \brief    Vebox initialize STMM History
//! \details  Initialize STMM History surface
//! Description:
//!   This function is used by VEBox for initializing
//!   the STMM surface.  The STMM / Denoise history is a custom surface used
//!   for both input and output. Each cache line contains data for 4 4x4s.
//!   The STMM for each 4x4 is 8 bytes, while the denoise history is 1 byte
//!   and the chroma denoise history is 1 byte for each U and V.
//!   Byte    Data\n
//!   0       STMM for 2 luma values at luma Y=0, X=0 to 1\n
//!   1       STMM for 2 luma values at luma Y=0, X=2 to 3\n
//!   2       Luma Denoise History for 4x4 at 0,0\n
//!   3       Not Used\n
//!   4-5     STMM for luma from X=4 to 7\n
//!   6       Luma Denoise History for 4x4 at 0,4\n
//!   7       Not Used\n
//!   8-15    Repeat for 4x4s at 0,8 and 0,12\n
//!   16      STMM for 2 luma values at luma Y=1,X=0 to 1\n
//!   17      STMM for 2 luma values at luma Y=1, X=2 to 3\n
//!   18      U Chroma Denoise History\n
//!   19      Not Used\n
//!   20-31   Repeat for 3 4x4s at 1,4, 1,8 and 1,12\n
//!   32      STMM for 2 luma values at luma Y=2,X=0 to 1\n
//!   33      STMM for 2 luma values at luma Y=2, X=2 to 3\n
//!   34      V Chroma Denoise History\n
//!   35      Not Used\n
//!   36-47   Repeat for 3 4x4s at 2,4, 2,8 and 2,12\n
//!   48      STMM for 2 luma values at luma Y=3,X=0 to 1\n
//!   49      STMM for 2 luma values at luma Y=3, X=2 to 3\n
//!   50-51   Not Used\n
//!   36-47   Repeat for 3 4x4s at 3,4, 3,8 and 3,12\n
//! \param    [in] iSurfaceIndex
//!           Index of STMM surface array
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VpVeboxCmdPacket::InitSTMMHistory(
    int32_t                      iSurfaceIndex)
{
    MOS_STATUS          eStatus;
    uint32_t            dwSize;
    int32_t             x, y;
    uint8_t*            pByte;
    MOS_LOCK_PARAMS     LockFlags;

    eStatus         = MOS_STATUS_SUCCESS;

    MOS_ZeroMemory(&LockFlags, sizeof(MOS_LOCK_PARAMS));

    LockFlags.WriteOnly    = 1;
    LockFlags.TiledAsTiled = 1; // Set TiledAsTiled flag for STMM surface initialization.

    // Lock the surface for writing
    pByte = (uint8_t*)m_allocator->Lock(
        &STMMSurfaces[iSurfaceIndex].OsResource,
        &LockFlags);

    VPHAL_RENDER_CHK_NULL(pByte);

    dwSize = STMMSurfaces[iSurfaceIndex].dwWidth >> 2;

    // Fill STMM surface with DN history init values.
    for (y = 0; y < (int32_t)STMMSurfaces[iSurfaceIndex].dwHeight; y++)
    {
        for (x = 0; x < (int32_t)dwSize; x++)
        {
            MOS_FillMemory(pByte, 2, DNDI_HISTORY_INITVALUE);
            // skip denosie history init.
            pByte += 4;
        }

        pByte += STMMSurfaces[iSurfaceIndex].dwPitch - STMMSurfaces[iSurfaceIndex].dwWidth;
    }

    // Unlock the surface
    VPHAL_RENDER_CHK_STATUS(m_allocator->UnLock(&STMMSurfaces[iSurfaceIndex].OsResource));

finish:
    return eStatus;
}

bool VpVeboxCmdPacket::IsFormatMMCSupported(MOS_FORMAT Format)
{
    // Check if Sample Format is supported
    if ((Format != Format_YUY2) &&
        (Format != Format_Y210) &&
        (Format != Format_Y410) &&
        (Format != Format_Y216) &&
        (Format != Format_Y416) &&
        (Format != Format_P010) &&
        (Format != Format_P016) &&
        (Format != Format_AYUV) &&
        (Format != Format_NV21) &&
        (Format != Format_NV12) &&
        (Format != Format_UYVY) &&
        (Format != Format_YUYV) &&
        (Format != Format_R10G10B10A2)   &&
        (Format != Format_B10G10R10A2)   &&
        (Format != Format_A8B8G8R8)      &&
        (Format != Format_X8B8G8R8)      &&
        (Format != Format_A8R8G8B8)      &&
        (Format != Format_X8R8G8B8)      &&
        (Format != Format_A16B16G16R16F) &&
        (Format != Format_A16R16G16B16F))
    {
      VP_RENDER_NORMALMESSAGE("Unsupported Format '0x%08x' for VEBOX MMC ouput.", Format);
      return false;
    }

    return true;
}

MOS_STATUS VpVeboxCmdPacket::SetSfcMmcParams()
{
    VP_FUNC_CALL();

    VP_PUBLIC_CHK_NULL_RETURN(m_renderTarget);
    VP_PUBLIC_CHK_NULL_RETURN(m_sfcRenderData.sfcStateParams);

    if (m_renderTarget->CompressionMode               &&
        IsFormatMMCSupported(m_renderTarget->Format)  &&
        m_renderTarget->TileType == MOS_TILE_Y        &&
        m_mmc->IsMmcEnabled())
    {
        m_sfcRenderData.sfcStateParams->bMMCEnable = true;
        m_sfcRenderData.sfcStateParams->MMCMode    = m_renderTarget->CompressionMode;
    }
    else
    {
        m_sfcRenderData.sfcStateParams->bMMCEnable = false;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpVeboxCmdPacket::SetScalingParams(PSFC_SCALING_PARAMS scalingParams)
{
    VP_RENDER_CHK_NULL_RETURN(scalingParams);
    // Scaing only can be apply to SFC path
    if (m_PacketCaps.bSFC)
    {
        // Adjust output width/height according to rotation.
        if (VPHAL_ROTATION_90                   == m_sfcRenderData.SfcRotation ||
            VPHAL_ROTATION_270                  == m_sfcRenderData.SfcRotation ||
            VPHAL_ROTATE_90_MIRROR_VERTICAL     == m_sfcRenderData.SfcRotation ||
            VPHAL_ROTATE_90_MIRROR_HORIZONTAL   == m_sfcRenderData.SfcRotation)
        {
            m_sfcRenderData.sfcStateParams->dwOutputFrameWidth         = scalingParams->dwOutputFrameHeight;
            m_sfcRenderData.sfcStateParams->dwOutputFrameHeight        = scalingParams->dwOutputFrameWidth;
        }
        else
        {
            m_sfcRenderData.sfcStateParams->dwOutputFrameWidth         = scalingParams->dwOutputFrameWidth;
            m_sfcRenderData.sfcStateParams->dwOutputFrameHeight        = scalingParams->dwOutputFrameHeight;
        }
        m_sfcRenderData.sfcStateParams->dwInputFrameHeight             = scalingParams->dwInputFrameHeight;
        m_sfcRenderData.sfcStateParams->dwInputFrameWidth              = scalingParams->dwInputFrameWidth;

        m_sfcRenderData.sfcStateParams->dwAVSFilterMode                = scalingParams->dwAVSFilterMode;
        m_sfcRenderData.sfcStateParams->dwSourceRegionHeight           = scalingParams->dwSourceRegionHeight;
        m_sfcRenderData.sfcStateParams->dwSourceRegionWidth            = scalingParams->dwSourceRegionWidth;
        m_sfcRenderData.sfcStateParams->dwSourceRegionVerticalOffset   = scalingParams->dwSourceRegionVerticalOffset;
        m_sfcRenderData.sfcStateParams->dwSourceRegionHorizontalOffset = scalingParams->dwSourceRegionHorizontalOffset;
        m_sfcRenderData.sfcStateParams->dwScaledRegionHeight           = scalingParams->dwScaledRegionHeight;
        m_sfcRenderData.sfcStateParams->dwScaledRegionWidth            = scalingParams->dwScaledRegionWidth;
        m_sfcRenderData.sfcStateParams->dwScaledRegionVerticalOffset   = scalingParams->dwScaledRegionVerticalOffset;
        m_sfcRenderData.sfcStateParams->dwScaledRegionHorizontalOffset = scalingParams->dwScaledRegionHorizontalOffset;
        m_sfcRenderData.sfcStateParams->fAVSXScalingRatio              = scalingParams->fAVSXScalingRatio;
        m_sfcRenderData.sfcStateParams->fAVSYScalingRatio              = scalingParams->fAVSYScalingRatio;

        m_sfcRenderData.bScaling = ((scalingParams->fAVSXScalingRatio == 1.0F) && (scalingParams->fAVSYScalingRatio == 1.0F)) ?
            false : true;

        m_sfcRenderData.fScaleX = scalingParams->fAVSXScalingRatio;
        m_sfcRenderData.fScaleY = scalingParams->fAVSYScalingRatio;

        // ColorFill/Alpha settings
        m_sfcRenderData.pColorFillParams            = &(scalingParams->sfcColorfillParams);
        m_sfcRenderData.sfcStateParams->fAlphaPixel = scalingParams->sfcColorfillParams.fAlphaPixel;
        m_sfcRenderData.sfcStateParams->fColorFillAPixel  = scalingParams->sfcColorfillParams.fColorFillAPixel;
        m_sfcRenderData.sfcStateParams->fColorFillUGPixel = scalingParams->sfcColorfillParams.fColorFillUGPixel;
        m_sfcRenderData.sfcStateParams->fColorFillVBPixel = scalingParams->sfcColorfillParams.fColorFillVBPixel;
        m_sfcRenderData.sfcStateParams->fColorFillYRPixel = scalingParams->sfcColorfillParams.fColorFillYRPixel;
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
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpVeboxCmdPacket::SetSfcCSCParams(PSFC_CSC_PARAMS cscParams)
{
    VP_RENDER_CHK_NULL_RETURN(cscParams);

    if (m_PacketCaps.bSFC)
    {
        m_sfcRenderData.bIEF           = cscParams->bIEFEnable;
        m_sfcRenderData.bCSC           = cscParams->bCSCEnabled;
        m_sfcRenderData.pIefParams     = cscParams->iefParams;
        m_sfcRenderData.SfcInputCspace = cscParams->inputColorSpcase;
        m_sfcRenderData.SfcInputFormat = cscParams->inputFormat;

        // ARGB8,ABGR10,A16B16G16R16,VYUY and YVYU output format need to enable swap
        if (cscParams->outputFormat == Format_X8R8G8B8 ||
            cscParams->outputFormat == Format_A8R8G8B8 ||
            cscParams->outputFormat == Format_R10G10B10A2 ||
            cscParams->outputFormat == Format_A16B16G16R16 ||
            cscParams->outputFormat == Format_VYUY ||
            cscParams->outputFormat == Format_YVYU)
        {
            m_sfcRenderData.sfcStateParams->bRGBASwapEnable = true;
        }
        else
        {
            m_sfcRenderData.sfcStateParams->bRGBASwapEnable = false;
        }
        m_sfcRenderData.sfcStateParams->bInputColorSpace = cscParams->bInputColorSpace;

        // Chromasitting config
        // config SFC chroma up sampling
        m_sfcRenderData.bForcePolyPhaseCoefs   = cscParams->bChromaUpSamplingEnable;
        m_sfcRenderData.SfcSrcChromaSiting     = cscParams->sfcSrcChromaSiting;
        m_sfcRenderData.inputChromaSubSampling = cscParams->inputChromaSubSampling;

        // 8-Tap chroma filter enabled or not
        m_sfcRenderData.sfcStateParams->b8tapChromafiltering = cscParams->b8tapChromafiltering;

        // config SFC chroma down sampling
        m_sfcRenderData.chromaDownSamplingHorizontalCoef = cscParams->chromaDownSamplingHorizontalCoef;
        m_sfcRenderData.chromaDownSamplingVerticalCoef   = cscParams->chromaDownSamplingVerticalCoef;

    }
    else
    {
        VP_RENDER_NORMALMESSAGE("CSC/IEF for Output is enabled in SFC, pls recheck the features enabling in SFC");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpVeboxCmdPacket::SetSfcRotMirParams(PSFC_ROT_MIR_PARAMS rotMirParams)
{
    VP_RENDER_CHK_NULL_RETURN(rotMirParams);

    if (m_PacketCaps.bSFC)
    {
        m_sfcRenderData.SfcRotation   = rotMirParams->rotationMode;
        m_sfcRenderData.bMirrorEnable = rotMirParams->bMirrorEnable;
        m_sfcRenderData.mirrorType  = rotMirParams->mirrorType;

        // Adjust output width/height according to rotation.
        if (VPHAL_ROTATION_90                   == m_sfcRenderData.SfcRotation ||
            VPHAL_ROTATION_270                  == m_sfcRenderData.SfcRotation ||
            VPHAL_ROTATE_90_MIRROR_VERTICAL     == m_sfcRenderData.SfcRotation ||
            VPHAL_ROTATE_90_MIRROR_HORIZONTAL   == m_sfcRenderData.SfcRotation)
        {
            uint32_t width = m_sfcRenderData.sfcStateParams->dwOutputFrameWidth;
            m_sfcRenderData.sfcStateParams->dwOutputFrameWidth  = m_sfcRenderData.sfcStateParams->dwOutputFrameHeight;
            m_sfcRenderData.sfcStateParams->dwOutputFrameHeight = width;
        }
    }
    else
    {
        VP_RENDER_NORMALMESSAGE("CSC/IEF for Output is enabled in SFC, pls recheck the features enabling in SFC");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    return MOS_STATUS_SUCCESS;
}

//!
//! \brief    Vebox Populate VEBOX parameters
//! \details  Populate the Vebox VEBOX state parameters to VEBOX RenderData
//! \param    [in] pLumaParams
//!           Pointer to Luma DN and DI parameter
//! \param    [in] pChromaParams
//!           Pointer to Chroma DN parameter
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VpVeboxCmdPacket::PopulateDNDIParams(
    PVPHAL_SAMPLER_STATE_DNDI_PARAM pLumaParams,
    PVPHAL_DNUV_PARAMS              pChromaParams)
{
    PMHW_VEBOX_DNDI_PARAMS          pVeboxDNDIParams;
    PVPHAL_VEBOX_RENDER_DATA        pRenderData = GetLastExecRenderData();

    // Populate the VEBOX VEBOX parameters
    pVeboxDNDIParams = &pRenderData->VeboxDNDIParams;

    // DI and Luma Denoise Params
    if (pLumaParams != nullptr)
    {
        if (m_PacketCaps.bDN)
        {
            pVeboxDNDIParams->dwDenoiseASDThreshold     = pLumaParams->dwDenoiseASDThreshold;
            pVeboxDNDIParams->dwDenoiseHistoryDelta     = pLumaParams->dwDenoiseHistoryDelta;
            pVeboxDNDIParams->dwDenoiseMaximumHistory   = pLumaParams->dwDenoiseMaximumHistory;
            pVeboxDNDIParams->dwDenoiseSTADThreshold    = pLumaParams->dwDenoiseSTADThreshold;
            pVeboxDNDIParams->dwDenoiseSCMThreshold     = pLumaParams->dwDenoiseSCMThreshold;
            pVeboxDNDIParams->dwDenoiseMPThreshold      = pLumaParams->dwDenoiseMPThreshold;
            pVeboxDNDIParams->dwLTDThreshold            = pLumaParams->dwLTDThreshold;
            pVeboxDNDIParams->dwTDThreshold             = pLumaParams->dwTDThreshold;
            pVeboxDNDIParams->dwGoodNeighborThreshold   = pLumaParams->dwGoodNeighborThreshold;
            pVeboxDNDIParams->bProgressiveDN            = pLumaParams->bProgressiveDN;
        }
        pVeboxDNDIParams->dwFMDFirstFieldCurrFrame      = pLumaParams->dwFMDFirstFieldCurrFrame;
        pVeboxDNDIParams->dwFMDSecondFieldPrevFrame     = pLumaParams->dwFMDSecondFieldPrevFrame;
        pVeboxDNDIParams->bDNDITopFirst                 = pLumaParams->bDNDITopFirst;
    }

    // Only need to reverse bDNDITopFirst for no reference case, no need to reverse it for having refrenece case
    if (!pRenderData->bRefValid)
    {
        pVeboxDNDIParams->bDNDITopFirst                 = pRenderData->bTopField;
    }

    // Chroma Denoise Params
    if (pRenderData->bChromaDenoise && pChromaParams != nullptr)
    {
        pVeboxDNDIParams->dwChromaSTADThreshold     = pChromaParams->dwSTADThresholdU; // Use U threshold for now
        pVeboxDNDIParams->dwChromaLTDThreshold      = pChromaParams->dwLTDThresholdU;  // Use U threshold for now
        pVeboxDNDIParams->dwChromaTDThreshold       = pChromaParams->dwTDThresholdU;   // Use U threshold for now
        pVeboxDNDIParams->bChromaDNEnable           = pRenderData->bChromaDenoise;
    }

    pRenderData->GetVeboxStateParams()->pVphalVeboxDndiParams = pVeboxDNDIParams;

    return MOS_STATUS_SUCCESS;
}

//!
//! \brief    Vebox Set FMD parameter
//! \details  Set up the FMD parameters for DNDI State
//! \param    [out] pLumaParams
//!           Pointer to DNDI Param for set FMD parameters
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VpVeboxCmdPacket::SetFMDParams(
    PVPHAL_SAMPLER_STATE_DNDI_PARAM     pLumaParams)
{
    PVPHAL_VEBOX_RENDER_DATA         pRenderData = GetLastExecRenderData();
    MOS_STATUS                       eStatus = MOS_STATUS_SUCCESS;

    VPHAL_RENDER_CHK_NULL(pLumaParams);

#if VEBOX_AUTO_DENOISE_SUPPORTED
    if (pRenderData->bProgressive && pRenderData->bAutoDenoise)
    {
        // out1 = Cur1st + Cur2nd
        pLumaParams->dwFMDFirstFieldCurrFrame =
            MEDIASTATE_DNDI_FIELDCOPY_NEXT;
        // out2 = Prv1st + Prv2nd
        pLumaParams->dwFMDSecondFieldPrevFrame =
            MEDIASTATE_DNDI_FIELDCOPY_PREV;
    }
    else
#endif
    {
        pLumaParams->dwFMDFirstFieldCurrFrame =
            MEDIASTATE_DNDI_DEINTERLACE;
        pLumaParams->dwFMDSecondFieldPrevFrame =
            MEDIASTATE_DNDI_DEINTERLACE;
    }

finish:
    return eStatus;
}

MOS_STATUS VpVeboxCmdPacket::SetVeboxDNDIParams(PVPHAL_SURFACE        pSrcSurface)
{
    MOS_STATUS                       eStatus;
    VPHAL_SAMPLER_STATE_DNDI_PARAM   lumaParams;
    VPHAL_DNUV_PARAMS                chromaParams;
    PVPHAL_SAMPLER_STATE_DNDI_PARAM  pLumaParams;
    PVPHAL_DNUV_PARAMS               pChromaParams;
    PVPHAL_VEBOX_RENDER_DATA         pRenderData = GetLastExecRenderData();

    eStatus             = MOS_STATUS_SUCCESS;
    pLumaParams         = &lumaParams;     // Params for DI and LumaDN
    pChromaParams       = &chromaParams;   // Params for ChromaDN

    MOS_ZeroMemory(pLumaParams, sizeof(VPHAL_SAMPLER_STATE_DNDI_PARAM));
    MOS_ZeroMemory(pChromaParams, sizeof(VPHAL_DNUV_PARAMS));

    // Set Luma and Chroma DNDI params
    VPHAL_RENDER_CHK_STATUS(SetDNDIParams(
        pSrcSurface,
        pLumaParams,
        pChromaParams));

    if (!m_bRefValid)
    {
        // setting LTDThreshold = TDThreshold = 0 forces SpatialDenoiseOnly
        pLumaParams->dwLTDThreshold   = 0;
        pLumaParams->dwTDThreshold    = 0;
    }

    if (m_PacketCaps.bDN)
    {
        pLumaParams->bDNEnable = true;

        if (pRenderData->bProgressive)
        {
            pLumaParams->bProgressiveDN = true;
        }
    }

    if (m_PacketCaps.bDI)
    {
        pLumaParams->bDIEnable = true;
        pLumaParams->bDNDITopFirst = pRenderData->bTFF;
    }

    SetFMDParams(pLumaParams);

    PopulateDNDIParams(
        pLumaParams,
        pChromaParams);

finish:
    return eStatus;
}

MOS_STATUS VpVeboxCmdPacket::SetupDiIecpState(
    bool                        bDiScdEnable,
    PMHW_VEBOX_DI_IECP_CMD_PARAMS   pVeboxDiIecpCmdParams)
{
    uint32_t                            dwWidth;
    uint32_t                            dwHeight;
    MHW_VEBOX_SURFACE_PARAMS            MhwVeboxSurfaceParam;
    PMHW_VEBOX_INTERFACE                pVeboxInterface;
    MHW_VEBOX_SURFACE_CNTL_PARAMS       VeboxSurfCntlParams;
    PVPHAL_SURFACE                      pSurface;
    MOS_STATUS                          eStatus     = MOS_STATUS_SUCCESS;

    VP_RENDER_CHK_NULL_RETURN(m_hwInterface);
    pVeboxInterface = m_hwInterface->m_veboxInterface;

    VP_RENDER_CHK_NULL_RETURN(pVeboxInterface);
    VP_RENDER_CHK_NULL_RETURN(m_veboxPacketSurface.pCurrInput);
    //VP_RENDER_CHK_NULL_RETURN(m_veboxPacketSurface.pCurrOutput);
    VP_RENDER_CHK_NULL_RETURN(m_veboxPacketSurface.pStatisticsOutput);

    MOS_ZeroMemory(pVeboxDiIecpCmdParams, sizeof(*pVeboxDiIecpCmdParams));

    // Align dwEndingX with surface state
    VP_RENDER_CHK_STATUS_RETURN(InitVeboxSurfaceParams(
                                    m_veboxPacketSurface.pCurrInput, &MhwVeboxSurfaceParam));
    VP_RENDER_CHK_STATUS_RETURN(pVeboxInterface->VeboxAdjustBoundary(
                                    &MhwVeboxSurfaceParam,
                                    &dwWidth,
                                    &dwHeight,
                                    m_PacketCaps.bDI));

    pVeboxDiIecpCmdParams->dwStartingX = 0;
    pVeboxDiIecpCmdParams->dwEndingX   = dwWidth - 1;

    pVeboxDiIecpCmdParams->pOsResCurrInput         = &m_veboxPacketSurface.pCurrInput->OsResource;
    pVeboxDiIecpCmdParams->dwCurrInputSurfOffset   = m_veboxPacketSurface.pCurrInput->dwOffset;
    pVeboxDiIecpCmdParams->CurrInputSurfCtrl.Value = m_DnDiSurfMemObjCtl.CurrentInputSurfMemObjCtl;

    // Update control bits for current surface
    if (m_mmc->IsMmcEnabled())
    {
        pSurface = m_veboxPacketSurface.pCurrInput;
        MOS_ZeroMemory(&VeboxSurfCntlParams, sizeof(VeboxSurfCntlParams));
        VeboxSurfCntlParams.bIsCompressed       = pSurface->bIsCompressed;
        VeboxSurfCntlParams.CompressionMode     = pSurface->CompressionMode;
        VP_RENDER_CHK_STATUS_RETURN(pVeboxInterface->AddVeboxSurfaceControlBits(
                                        &VeboxSurfCntlParams,
                                        (uint32_t *)&(pVeboxDiIecpCmdParams->CurrInputSurfCtrl.Value)));
    }

    // Reference surface
    if (m_veboxPacketSurface.pPrevInput)
    {
        pVeboxDiIecpCmdParams->pOsResPrevInput          = &m_veboxPacketSurface.pPrevInput->OsResource;
        pVeboxDiIecpCmdParams->dwPrevInputSurfOffset    = m_veboxPacketSurface.pPrevInput->dwOffset;
        pVeboxDiIecpCmdParams->PrevInputSurfCtrl.Value  = m_DnDiSurfMemObjCtl.PreviousInputSurfMemObjCtl;

        // Update control bits for PreviousSurface surface
        if (m_mmc->IsMmcEnabled())
        {
            pSurface = m_veboxPacketSurface.pPrevInput;
            MOS_ZeroMemory(&VeboxSurfCntlParams, sizeof(VeboxSurfCntlParams));
            VeboxSurfCntlParams.bIsCompressed       = pSurface->bIsCompressed;
            VeboxSurfCntlParams.CompressionMode     = pSurface->CompressionMode;
            VP_RENDER_CHK_STATUS_RETURN(pVeboxInterface->AddVeboxSurfaceControlBits(
                &VeboxSurfCntlParams,
                (uint32_t *)&(pVeboxDiIecpCmdParams->PrevInputSurfCtrl.Value)));
        }
    }

    // VEBOX final output surface
    if (m_veboxPacketSurface.pCurrOutput)
    {
        pVeboxDiIecpCmdParams->pOsResCurrOutput         = &m_veboxPacketSurface.pCurrOutput->OsResource;
        pVeboxDiIecpCmdParams->dwCurrOutputSurfOffset   = m_veboxPacketSurface.pCurrOutput->dwOffset;
        pVeboxDiIecpCmdParams->CurrOutputSurfCtrl.Value = m_DnDiSurfMemObjCtl.CurrentOutputSurfMemObjCtl;

        // Update control bits for Current Output Surf
        if (m_mmc->IsMmcEnabled())
        {
            pSurface = m_veboxPacketSurface.pCurrOutput;
            MOS_ZeroMemory(&VeboxSurfCntlParams, sizeof(VeboxSurfCntlParams));
            VeboxSurfCntlParams.bIsCompressed   = pSurface->bIsCompressed;
            VeboxSurfCntlParams.CompressionMode = pSurface->CompressionMode;
            VP_RENDER_CHK_STATUS_RETURN(pVeboxInterface->AddVeboxSurfaceControlBits(
                &VeboxSurfCntlParams,
                (uint32_t *)&(pVeboxDiIecpCmdParams->CurrOutputSurfCtrl.Value)));
        }
    }

    if (m_veboxPacketSurface.pPrevOutput)
    {
        pVeboxDiIecpCmdParams->pOsResPrevOutput         = &m_veboxPacketSurface.pPrevOutput->OsResource;
        pVeboxDiIecpCmdParams->PrevOutputSurfCtrl.Value = m_DnDiSurfMemObjCtl.CurrentOutputSurfMemObjCtl;

        // Update control bits for PrevOutput surface
        if (m_mmc->IsMmcEnabled())
        {
            pSurface = m_veboxPacketSurface.pPrevOutput;
            MOS_ZeroMemory(&VeboxSurfCntlParams, sizeof(VeboxSurfCntlParams));
            VeboxSurfCntlParams.bIsCompressed   = pSurface->bIsCompressed;
            VeboxSurfCntlParams.CompressionMode = pSurface->CompressionMode;
            VP_RENDER_CHK_STATUS_RETURN(pVeboxInterface->AddVeboxSurfaceControlBits(
                &VeboxSurfCntlParams,
                (uint32_t *)&(pVeboxDiIecpCmdParams->PrevOutputSurfCtrl.Value)));
        }
    }

    // DN intermediate output surface
    if (m_veboxPacketSurface.pDenoisedCurrOutput)
    {
        pVeboxDiIecpCmdParams->pOsResDenoisedCurrOutput         = &m_veboxPacketSurface.pDenoisedCurrOutput->OsResource;
        pVeboxDiIecpCmdParams->DenoisedCurrOutputSurfCtrl.Value = m_DnDiSurfMemObjCtl.DnOutSurfMemObjCtl;

        // Update control bits for DenoisedCurrOutputSurf surface
        if (m_mmc->IsMmcEnabled())
        {
            pSurface = m_veboxPacketSurface.pDenoisedCurrOutput;
            MOS_ZeroMemory(&VeboxSurfCntlParams, sizeof(VeboxSurfCntlParams));
            VeboxSurfCntlParams.bIsCompressed   = pSurface->bIsCompressed;
            VeboxSurfCntlParams.CompressionMode = pSurface->CompressionMode;
            VPHAL_RENDER_CHK_STATUS(pVeboxInterface->AddVeboxSurfaceControlBits(
                &VeboxSurfCntlParams,
                (uint32_t *)&(pVeboxDiIecpCmdParams->DenoisedCurrOutputSurfCtrl.Value)));
        }
    }

    // STMM surface
    if (m_veboxPacketSurface.pSTMMInput && m_veboxPacketSurface.pSTMMOutput)
    {
        // STMM in
        pVeboxDiIecpCmdParams->pOsResStmmInput         = &m_veboxPacketSurface.pSTMMInput->OsResource;
        pVeboxDiIecpCmdParams->StmmInputSurfCtrl.Value = m_DnDiSurfMemObjCtl.STMMInputSurfMemObjCtl;

        // Update control bits for stmm input surface
        if (m_mmc->IsMmcEnabled())
        {
            pSurface = m_veboxPacketSurface.pSTMMInput;
            MOS_ZeroMemory(&VeboxSurfCntlParams, sizeof(VeboxSurfCntlParams));
            VeboxSurfCntlParams.bIsCompressed   = pSurface->bIsCompressed;
            VeboxSurfCntlParams.CompressionMode = pSurface->CompressionMode;
            VPHAL_RENDER_CHK_STATUS(pVeboxInterface->AddVeboxSurfaceControlBits(
                &VeboxSurfCntlParams,
                (uint32_t *)&(pVeboxDiIecpCmdParams->StmmInputSurfCtrl.Value)));
        }

        // STMM out
        pVeboxDiIecpCmdParams->pOsResStmmOutput         = &m_veboxPacketSurface.pSTMMOutput->OsResource;
        pVeboxDiIecpCmdParams->StmmOutputSurfCtrl.Value = m_DnDiSurfMemObjCtl.STMMOutputSurfMemObjCtl;

        // Update control bits for stmm output surface
        if (m_mmc->IsMmcEnabled())
        {
            pSurface = m_veboxPacketSurface.pSTMMOutput;
            MOS_ZeroMemory(&VeboxSurfCntlParams, sizeof(VeboxSurfCntlParams));
            VeboxSurfCntlParams.bIsCompressed   = pSurface->bIsCompressed;
            VeboxSurfCntlParams.CompressionMode = pSurface->CompressionMode;
            VPHAL_RENDER_CHK_STATUS(pVeboxInterface->AddVeboxSurfaceControlBits(
                &VeboxSurfCntlParams,
                (uint32_t *)&(pVeboxDiIecpCmdParams->StmmOutputSurfCtrl.Value)));
        }
    }

    pVeboxDiIecpCmdParams->pOsResStatisticsOutput         = &m_veboxPacketSurface.pStatisticsOutput->OsResource;
    pVeboxDiIecpCmdParams->StatisticsOutputSurfCtrl.Value = m_DnDiSurfMemObjCtl.StatisticsOutputSurfMemObjCtl;

finish:
    return eStatus;
}

bool VpVeboxCmdPacket::UseKernelResource()
{
    return false;
}

MOS_STATUS VpVeboxCmdPacket::InitVeboxSurfaceParams(
    PVPHAL_SURFACE                 pVpHalVeboxSurface,
    PMHW_VEBOX_SURFACE_PARAMS      pMhwVeboxSurface)
{
    MOS_STATUS                   eStatus = MOS_STATUS_SUCCESS;

    VP_RENDER_CHK_NULL_RETURN(pVpHalVeboxSurface);
    VP_RENDER_CHK_NULL_RETURN(pMhwVeboxSurface);

    MOS_ZeroMemory(pMhwVeboxSurface, sizeof(*pMhwVeboxSurface));
    pMhwVeboxSurface->bActive                = true;
    pMhwVeboxSurface->Format                 = pVpHalVeboxSurface->Format;
    pMhwVeboxSurface->dwWidth                = pVpHalVeboxSurface->dwWidth;
    pMhwVeboxSurface->dwHeight               = pVpHalVeboxSurface->dwHeight;
    pMhwVeboxSurface->dwPitch                = pVpHalVeboxSurface->dwPitch;
    pMhwVeboxSurface->dwBitDepth             = pVpHalVeboxSurface->dwDepth;
    pMhwVeboxSurface->TileType               = pVpHalVeboxSurface->TileType;
    pMhwVeboxSurface->TileModeGMM            = pVpHalVeboxSurface->TileModeGMM;
    pMhwVeboxSurface->bGMMTileEnabled        = pVpHalVeboxSurface->bGMMTileEnabled;
    pMhwVeboxSurface->rcMaxSrc               = pVpHalVeboxSurface->rcMaxSrc;
    pMhwVeboxSurface->pOsResource            = &pVpHalVeboxSurface->OsResource;
    pMhwVeboxSurface->bIsCompressed          = pVpHalVeboxSurface->bIsCompressed;

    if (pVpHalVeboxSurface->dwPitch > 0)
    {
        pMhwVeboxSurface->dwUYoffset = ((pVpHalVeboxSurface->UPlaneOffset.iSurfaceOffset - pVpHalVeboxSurface->YPlaneOffset.iSurfaceOffset) / pVpHalVeboxSurface->dwPitch)
                                       + pVpHalVeboxSurface->UPlaneOffset.iYOffset;
    }
    return eStatus;
}

PVPHAL_SURFACE VpVeboxCmdPacket::GetSurfOutput(bool   bDiVarianceEnable)
{
    PVPHAL_SURFACE                          pSurface = nullptr;
    PVPHAL_VEBOX_RENDER_DATA                pRenderData = GetLastExecRenderData();

    if (IS_VPHAL_OUTPUT_PIPE_VEBOX(pRenderData))                    // Vebox output pipe
    {
        pSurface = pRenderData->pRenderTarget;
    }
    else if (m_IsSfcUsed)                 // Write to SFC
    {
        // Vebox o/p should not be written to memory
        pSurface = nullptr;
    }
    else
    {
        VP_RENDER_ASSERTMESSAGE("Unable to determine Vebox Output Surface.");
    }

    return pSurface;
}

MOS_STATUS VpVeboxCmdPacket::SendVeboxCmd(MOS_COMMAND_BUFFER* commandBuffer)
{
    MOS_STATUS                              eStatus;
    int32_t                                 iRemaining;
    MHW_VEBOX_DI_IECP_CMD_PARAMS            VeboxDiIecpCmdParams;
    VPHAL_VEBOX_SURFACE_STATE_CMD_PARAMS    VeboxSurfaceStateCmdParams;
    MHW_VEBOX_SURFACE_STATE_CMD_PARAMS      MhwVeboxSurfaceStateCmdParams;
    MHW_VEBOX_STATE_CMD_PARAMS              VeboxStateCmdParams;
    MHW_MI_FLUSH_DW_PARAMS                  FlushDwParams;
    PMHW_VEBOX_INTERFACE                    pVeboxInterface;
    RENDERHAL_GENERIC_PROLOG_PARAMS         GenericPrologParams;
    MOS_RESOURCE                            GpuStatusBuffer;

    eStatus                 = MOS_STATUS_SUCCESS;
    pVeboxInterface         = m_hwInterface->m_veboxInterface;
    iRemaining              = 0;

    VP_RENDER_CHK_NULL_RETURN(commandBuffer);

    eStatus = PrepareVeboxCmd(
                  commandBuffer,
                  GenericPrologParams,
                  GpuStatusBuffer,
                  iRemaining);

    if (eStatus != MOS_STATUS_SUCCESS)
    {
        CmdErrorHanlde(commandBuffer, iRemaining);
    }
    else
    {
        eStatus = RenderVeboxCmd(
                      commandBuffer,
                      VeboxDiIecpCmdParams,
                      VeboxSurfaceStateCmdParams,
                      MhwVeboxSurfaceStateCmdParams,
                      VeboxStateCmdParams,
                      FlushDwParams,
                      &GenericPrologParams);
        if (eStatus != MOS_STATUS_SUCCESS)
        {
          // Failed -> discard all changes in Command Buffer
            CmdErrorHanlde(commandBuffer, iRemaining);
        }
    }

    return eStatus;
}

void VpVeboxCmdPacket::CmdErrorHanlde(
    MOS_COMMAND_BUFFER  *CmdBuffer,
    int32_t             &iRemaining)
{
    int32_t     i= 0;

    VP_PUBLIC_CHK_NULL_NO_STATUS_RETURN(CmdBuffer);
    // Buffer overflow - display overflow size
    if (CmdBuffer->iRemaining < 0)
    {
        VP_RENDER_ASSERTMESSAGE("Command Buffer overflow by %d bytes", CmdBuffer->iRemaining);
    }

    // Move command buffer back to beginning
    i = iRemaining - CmdBuffer->iRemaining;
    CmdBuffer->iRemaining = iRemaining;
    CmdBuffer->iOffset -= i;
    CmdBuffer->pCmdPtr = CmdBuffer->pCmdBase + CmdBuffer->iOffset / sizeof(uint32_t);
}

MOS_STATUS VpVeboxCmdPacket::PrepareVeboxCmd(
    MOS_COMMAND_BUFFER*                      CmdBuffer,
    RENDERHAL_GENERIC_PROLOG_PARAMS&         GenericPrologParams,
    MOS_RESOURCE&                            GpuStatusBuffer,
    int32_t&                                 iRemaining)
{
    MOS_STATUS                              eStatus      = MOS_STATUS_SUCCESS;
    PMOS_INTERFACE                          pOsInterface = m_hwInterface->m_osInterface;
    PVPHAL_VEBOX_RENDER_DATA                pRenderData  = GetLastExecRenderData();

    VP_RENDER_CHK_NULL_RETURN(CmdBuffer);
    VP_RENDER_CHK_NULL_RETURN(pOsInterface);
    VP_RENDER_CHK_NULL_RETURN(m_currentSurface);

    // Set initial state
    iRemaining = CmdBuffer->iRemaining;

    //---------------------------
    // Set Performance Tags
    //---------------------------
    VP_RENDER_CHK_STATUS_RETURN(VeboxSetPerfTag(m_currentSurface->Format));
    pOsInterface->pfnResetPerfBufferID(pOsInterface);
    pOsInterface->pfnSetPerfTag(pOsInterface, pRenderData->PerfTag);

    MOS_ZeroMemory(&GenericPrologParams, sizeof(GenericPrologParams));

    // Linux will do nothing here since currently no frame tracking support
   #ifndef EMUL
     if(pOsInterface->bEnableKmdMediaFrameTracking)
     {
         // Get GPU Status buffer
         VP_RENDER_CHK_STATUS_RETURN(pOsInterface->pfnGetGpuStatusBufferResource(pOsInterface, &GpuStatusBuffer));

         // Register the buffer
         VP_RENDER_CHK_STATUS_RETURN(pOsInterface->pfnRegisterResource(pOsInterface, &GpuStatusBuffer, true, true));

         GenericPrologParams.bEnableMediaFrameTracking = true;
         GenericPrologParams.presMediaFrameTrackingSurface = &GpuStatusBuffer;
         GenericPrologParams.dwMediaFrameTrackingTag = pOsInterface->pfnGetGpuStatusTag(pOsInterface, pOsInterface->CurrentGpuContextOrdinal);
         GenericPrologParams.dwMediaFrameTrackingAddrOffset = pOsInterface->pfnGetGpuStatusTagOffset(pOsInterface, pOsInterface->CurrentGpuContextOrdinal);

         // Increment GPU Status Tag
         pOsInterface->pfnIncrementGpuStatusTag(pOsInterface, pOsInterface->CurrentGpuContextOrdinal);
     }
   #endif

    return eStatus;
}

MOS_STATUS VpVeboxCmdPacket::RenderVeboxCmd(
    MOS_COMMAND_BUFFER                      *CmdBuffer,
    MHW_VEBOX_DI_IECP_CMD_PARAMS            &VeboxDiIecpCmdParams,
    VPHAL_VEBOX_SURFACE_STATE_CMD_PARAMS    &VeboxSurfaceStateCmdParams,
    MHW_VEBOX_SURFACE_STATE_CMD_PARAMS      &MhwVeboxSurfaceStateCmdParams,
    MHW_VEBOX_STATE_CMD_PARAMS              &VeboxStateCmdParams,
    MHW_MI_FLUSH_DW_PARAMS                  &FlushDwParams,
    PRENDERHAL_GENERIC_PROLOG_PARAMS        pGenericPrologParams)
{
    MOS_STATUS                              eStatus = MOS_STATUS_SUCCESS;
    PRENDERHAL_INTERFACE                    pRenderHal;
    PMOS_INTERFACE                          pOsInterface;
    PMHW_MI_INTERFACE                       pMhwMiInterface;
    PMHW_VEBOX_INTERFACE                    pVeboxInterface;
    bool                                    bDiVarianceEnable;
    const MHW_VEBOX_HEAP                    *pVeboxHeap = nullptr;
    PVPHAL_VEBOX_RENDER_DATA                pRenderData = GetLastExecRenderData();
    MediaPerfProfiler                       *pPerfProfiler;
    MOS_CONTEXT                             *pOsContext = nullptr;
    PMHW_MI_MMIOREGISTERS                   pMmioRegisters = nullptr;

    VP_RENDER_CHK_NULL_RETURN(m_hwInterface->m_renderHal);
    VP_RENDER_CHK_NULL_RETURN(m_hwInterface->m_mhwMiInterface);
    VP_RENDER_CHK_NULL_RETURN(m_hwInterface->m_osInterface);
    VP_RENDER_CHK_NULL_RETURN(m_hwInterface->m_veboxInterface);
    VP_RENDER_CHK_NULL_RETURN(m_hwInterface->m_osInterface->pOsContext);
    VP_RENDER_CHK_NULL_RETURN(m_hwInterface->m_mhwMiInterface->GetMmioRegisters());

    pRenderHal              = m_hwInterface->m_renderHal;
    pMhwMiInterface         = m_hwInterface->m_mhwMiInterface;
    pOsInterface            = m_hwInterface->m_osInterface;
    pVeboxInterface         = m_hwInterface->m_veboxInterface;
    pPerfProfiler           = pRenderHal->pPerfProfiler;
    pOsContext              = m_hwInterface->m_osInterface->pOsContext;
    pMmioRegisters          = pMhwMiInterface->GetMmioRegisters();

    VP_RENDER_CHK_STATUS_RETURN(pVeboxInterface->GetVeboxHeapInfo(&pVeboxHeap));
    VP_RENDER_CHK_NULL_RETURN(pVeboxHeap);
    VP_RENDER_CHK_NULL_RETURN(CmdBuffer);

#ifdef _MMC_SUPPORTED

    MhwVeboxInterfaceG12 *pVeboxInterfaceExt;
    pVeboxInterfaceExt = (MhwVeboxInterfaceG12 *)pVeboxInterface;

    VP_RENDER_CHK_STATUS_RETURN(pVeboxInterfaceExt->setVeboxPrologCmd(pMhwMiInterface, CmdBuffer));
#endif

    HalOcaInterface::On1stLevelBBStart(*CmdBuffer, *pOsContext, pOsInterface->CurrentGpuContextHandle,
        *pMhwMiInterface, *pMmioRegisters);
    // Add vphal param to log.
    HalOcaInterface::DumpVphalParam(*CmdBuffer, *pOsContext, pRenderHal->pVphalOcaDumper);

    // Initialize command buffer and insert prolog
    VP_RENDER_CHK_STATUS_RETURN(InitCmdBufferWithVeParams(pRenderHal, *CmdBuffer, pGenericPrologParams));

    VP_RENDER_CHK_STATUS_RETURN(pPerfProfiler->AddPerfCollectStartCmd((void*)pRenderHal, pOsInterface, pRenderHal->pMhwMiInterface, CmdBuffer));

    bDiVarianceEnable = m_PacketCaps.bDI;

    SetupSurfaceStates(
        bDiVarianceEnable,
        &VeboxSurfaceStateCmdParams);

    SetupVeboxState(
        bDiVarianceEnable,
        &VeboxStateCmdParams);

    VP_RENDER_CHK_STATUS_RETURN(SetupDiIecpState(
        bDiVarianceEnable,
        &VeboxDiIecpCmdParams));

    VP_RENDER_CHK_STATUS_RETURN(IsCmdParamsValid(
        VeboxStateCmdParams,
        VeboxDiIecpCmdParams));

    //---------------------------------
    // Initialize Vebox Surface State Params
    //---------------------------------
    VP_RENDER_CHK_STATUS_RETURN(InitVeboxSurfaceStateCmdParams(
        &VeboxSurfaceStateCmdParams, &MhwVeboxSurfaceStateCmdParams));

    //---------------------------------
    // Send CMD: Vebox_State
    //---------------------------------
    VP_RENDER_CHK_STATUS_RETURN(pVeboxInterface->AddVeboxState(
        CmdBuffer,
        &VeboxStateCmdParams,
        0));

    //---------------------------------
    // Send CMD: Vebox_Surface_State
    //---------------------------------
    VP_RENDER_CHK_STATUS_RETURN(pVeboxInterface->AddVeboxSurfaces(
        CmdBuffer,
        &MhwVeboxSurfaceStateCmdParams));

    //---------------------------------
    // Send CMD: SFC pipe commands
    //---------------------------------
    if (m_IsSfcUsed)
    {

        VP_RENDER_CHK_NULL_RETURN(m_sfcRender);

        VP_RENDER_CHK_STATUS_RETURN(m_sfcRender->SetupSfcState(
            &m_sfcRenderData,
            m_renderTarget));

        VP_RENDER_CHK_STATUS_RETURN(m_sfcRender->SendSfcCmd(
                                pRenderData,
                                CmdBuffer));
    }

    HalOcaInterface::OnDispatch(*CmdBuffer, *pOsContext, *pMhwMiInterface, *pMmioRegisters);

    //---------------------------------
    // Send CMD: Vebox_DI_IECP
    //---------------------------------
    VP_RENDER_CHK_STATUS_RETURN(pVeboxInterface->AddVeboxDiIecp(
                                  CmdBuffer,
                                  &VeboxDiIecpCmdParams));

    //---------------------------------
    // Write GPU Status Tag for Tag based synchronization
    //---------------------------------
    if (!pOsInterface->bEnableKmdMediaFrameTracking)
    {
        VP_RENDER_CHK_STATUS_RETURN(SendVecsStatusTag(
                                      pMhwMiInterface,
                                      pOsInterface,
                                      CmdBuffer));
    }

    //---------------------------------
    // Write Sync tag for Vebox Heap Synchronization
    // If KMD frame tracking is on, the synchronization of Vebox Heap will use Status tag which
    // is updated using KMD frame tracking.
    //---------------------------------
    if (!pOsInterface->bEnableKmdMediaFrameTracking)
    {
        MOS_ZeroMemory(&FlushDwParams, sizeof(FlushDwParams));
        FlushDwParams.pOsResource                   = (PMOS_RESOURCE)&pVeboxHeap->DriverResource;
        FlushDwParams.dwResourceOffset              = pVeboxHeap->uiOffsetSync;
        FlushDwParams.dwDataDW1                     = pVeboxHeap->dwNextTag;
        VP_RENDER_CHK_STATUS_RETURN(pMhwMiInterface->AddMiFlushDwCmd(
                                      CmdBuffer,
                                      &FlushDwParams));
    }

    VP_RENDER_CHK_STATUS_RETURN(pPerfProfiler->AddPerfCollectEndCmd((void*)pRenderHal, pOsInterface, pRenderHal->pMhwMiInterface, CmdBuffer));

    HalOcaInterface::On1stLevelBBEnd(*CmdBuffer, *pOsContext);

    if (pOsInterface->bNoParsingAssistanceInKmd)
    {
        VP_RENDER_CHK_STATUS_RETURN(pMhwMiInterface->AddMiBatchBufferEnd(
                                      CmdBuffer,
                                      nullptr));
    }
    else if (RndrCommonIsMiBBEndNeeded(pOsInterface))
    {
        // Add Batch Buffer end command (HW/OS dependent)
        VP_RENDER_CHK_STATUS_RETURN(pMhwMiInterface->AddMiBatchBufferEnd(
            CmdBuffer,
            nullptr));
    }

    return eStatus;

}

MOS_STATUS VpVeboxCmdPacket::InitVeboxSurfaceStateCmdParams(
    PVPHAL_VEBOX_SURFACE_STATE_CMD_PARAMS    pVpHalVeboxSurfaceStateCmdParams,
    PMHW_VEBOX_SURFACE_STATE_CMD_PARAMS      pMhwVeboxSurfaceStateCmdParams)
{
    MOS_STATUS                       eStatus = MOS_STATUS_SUCCESS;

    VP_RENDER_CHK_NULL_RETURN(pVpHalVeboxSurfaceStateCmdParams);
    VP_RENDER_CHK_NULL_RETURN(pMhwVeboxSurfaceStateCmdParams);

    MOS_ZeroMemory(pMhwVeboxSurfaceStateCmdParams, sizeof(*pMhwVeboxSurfaceStateCmdParams));

    pMhwVeboxSurfaceStateCmdParams->bDIEnable = pVpHalVeboxSurfaceStateCmdParams->bDIEnable;

    if (pVpHalVeboxSurfaceStateCmdParams->pSurfInput)
    {
        VP_RENDER_CHK_STATUS_RETURN(InitVeboxSurfaceParams(
                                      pVpHalVeboxSurfaceStateCmdParams->pSurfInput,
                                      &pMhwVeboxSurfaceStateCmdParams->SurfInput));
        pMhwVeboxSurfaceStateCmdParams->SurfInput.dwYoffset = pVpHalVeboxSurfaceStateCmdParams->pSurfInput->YPlaneOffset.iYOffset;
    }
    if (pVpHalVeboxSurfaceStateCmdParams->pSurfOutput)
    {
        pMhwVeboxSurfaceStateCmdParams->bOutputValid = true;
        VP_RENDER_CHK_STATUS_RETURN(InitVeboxSurfaceParams(
                                      pVpHalVeboxSurfaceStateCmdParams->pSurfOutput,
                                      &pMhwVeboxSurfaceStateCmdParams->SurfOutput));
        pMhwVeboxSurfaceStateCmdParams->SurfOutput.dwYoffset = pVpHalVeboxSurfaceStateCmdParams->pSurfOutput->YPlaneOffset.iYOffset;
    }
    if (pVpHalVeboxSurfaceStateCmdParams->pSurfSTMM)
    {
        VP_RENDER_CHK_STATUS_RETURN(InitVeboxSurfaceParams(
                                      pVpHalVeboxSurfaceStateCmdParams->pSurfSTMM,
                                      &pMhwVeboxSurfaceStateCmdParams->SurfSTMM));
    }

    return eStatus;
}

MOS_STATUS VpVeboxCmdPacket::SendVecsStatusTag(
    PMHW_MI_INTERFACE                   pMhwMiInterface,
    PMOS_INTERFACE                      pOsInterface,
    PMOS_COMMAND_BUFFER                 pCmdBuffer)
{
    MOS_RESOURCE                        GpuStatusBuffer;
    MHW_MI_FLUSH_DW_PARAMS              FlushDwParams;
    MOS_STATUS                          eStatus = MOS_STATUS_SUCCESS;

    //------------------------------------
    VP_RENDER_CHK_NULL_RETURN(pMhwMiInterface);
    VP_RENDER_CHK_NULL_RETURN(pOsInterface);
    VP_RENDER_CHK_NULL_RETURN(pCmdBuffer);

    // Get GPU Status buffer
    pOsInterface->pfnGetGpuStatusBufferResource(pOsInterface, &GpuStatusBuffer);

    // Register the buffer
    VP_RENDER_CHK_STATUS_RETURN(pOsInterface->pfnRegisterResource(
                                  pOsInterface,
                                  &GpuStatusBuffer,
                                  true,
                                  true));

    MOS_ZeroMemory(&FlushDwParams, sizeof(FlushDwParams));
    FlushDwParams.pOsResource       = &GpuStatusBuffer;
    FlushDwParams.dwResourceOffset  = pOsInterface->pfnGetGpuStatusTagOffset(pOsInterface, MOS_GPU_CONTEXT_VEBOX);
    FlushDwParams.dwDataDW1         = pOsInterface->pfnGetGpuStatusTag(pOsInterface, MOS_GPU_CONTEXT_VEBOX);
    VP_RENDER_CHK_STATUS_RETURN(pMhwMiInterface->AddMiFlushDwCmd(
                                  pCmdBuffer,
                                  &FlushDwParams));

    // Increase buffer tag for next usage
    pOsInterface->pfnIncrementGpuStatusTag(pOsInterface, MOS_GPU_CONTEXT_VEBOX);

    return eStatus;
}

bool VpVeboxCmdPacket::RndrCommonIsMiBBEndNeeded(
    PMOS_INTERFACE           pOsInterface)
{
    bool needed = false;

    if (nullptr == pOsInterface)
        return false;

    return needed;
}

MOS_STATUS VpVeboxCmdPacket::Init()
{
    MOS_STATUS    eStatus = MOS_STATUS_SUCCESS;
    VP_RENDER_CHK_NULL_RETURN(m_hwInterface);
    VP_RENDER_CHK_NULL_RETURN(m_hwInterface->m_skuTable);

    if (m_sfcRender == nullptr)
    {
        m_sfcRender = MOS_New(SfcRenderBase,
            m_hwInterface->m_osInterface,
            m_hwInterface->m_sfcInterface,
            m_allocator);
        VP_CHK_SPACE_NULL_RETURN(m_sfcRender);

    }

    VP_RENDER_CHK_STATUS_RETURN(InitSfcStateParams());

#ifdef MOVE_TO_HWFILTER
    if (!m_currentSurface)
    {
        m_currentSurface = (VPHAL_SURFACE*)MOS_AllocAndZeroMemory(sizeof(VPHAL_SURFACE));
        VP_CHK_SPACE_NULL_RETURN(m_currentSurface);
    }

    if (!m_previousSurface)
    {
        m_previousSurface = (VPHAL_SURFACE*)MOS_AllocAndZeroMemory(sizeof(VPHAL_SURFACE));
        VP_CHK_SPACE_NULL_RETURN(m_previousSurface);
    }

    if (!m_renderTarget)
    {
        m_renderTarget = (VPHAL_SURFACE*)MOS_AllocAndZeroMemory(sizeof(VPHAL_SURFACE));
        VP_CHK_SPACE_NULL_RETURN(m_renderTarget);
    }

    for (uint32_t i = 0; i < VP_MAX_NUM_FFDI_SURFACES; i++)
    {
        if (!FFDISurfaces[i])
        {
            FFDISurfaces[i] = (VPHAL_SURFACE*)MOS_AllocAndZeroMemory(sizeof(VPHAL_SURFACE));
            if (!FFDISurfaces[i])
            {
                return MOS_STATUS_NO_SPACE;
            }
        }
    }

    for (uint32_t i = 0; i < VP_NUM_FFDN_SURFACES; i++)
    {
        if (!FFDNSurfaces[i])
        {
            FFDNSurfaces[i] = (VPHAL_SURFACE*)MOS_AllocAndZeroMemory(sizeof(VPHAL_SURFACE));
            if (!FFDNSurfaces[i])
            {
                return MOS_STATUS_NO_SPACE;
            }
        }
    }
#endif

    return eStatus;
}
MOS_STATUS VpVeboxCmdPacket::PacketInit(
    PVPHAL_SURFACE      pSrcSurface,
    PVPHAL_SURFACE      pOutputSurface,
    VP_EXECUTE_CAPS     packetCaps)
{
    VP_RENDER_CHK_NULL_RETURN(pSrcSurface);
    VP_RENDER_CHK_NULL_RETURN(pOutputSurface);
    VP_FUNC_CALL();
    m_PacketCaps      = packetCaps;

    VP_RENDER_CHK_STATUS_RETURN(Init());

    // init packet surface params.
    MOS_ZeroMemory(&m_veboxPacketSurface, sizeof(VEBOX_PACKET_SURFACE_PARAMS));

    m_veboxPacketSurface.pCurrInput        = m_currentSurface;
    m_veboxPacketSurface.pStatisticsOutput = &m_veboxStatisticsSurface;
    m_veboxPacketSurface.pCurrOutput       = nullptr;

    if (packetCaps.bSFC)
    {
        m_IsSfcUsed = true;
    }

    VP_RENDER_CHK_STATUS_RETURN(SetupVeboxRenderMode0(pSrcSurface, pOutputSurface));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpVeboxCmdPacket::Submit(MOS_COMMAND_BUFFER* commandBuffer, uint8_t packetPhase)
{
    MOS_STATUS    eStatus = MOS_STATUS_SUCCESS;
    PVPHAL_VEBOX_RENDER_DATA    pRenderData = GetLastExecRenderData();
    VP_FUNC_CALL();
    VP_RENDER_CHK_STATUS_RETURN(SendVeboxCmd(commandBuffer));

#ifdef MOVE_TO_HWFILTER
    //--------------------------------------------------------------------------
    // ffDN and ffDNDI cases
    //--------------------------------------------------------------------------
    VP_RENDER_CHK_NULL_RETURN(pRenderData);
    if (pRenderData->bDenoise)
    {
        CopySurfaceValue(m_currentSurface, FFDNSurfaces[pRenderData->iCurDNOut]);
    }

    if ((pRenderData->bDeinterlace ||
        !pRenderData->bRefValid)   &&
        pRenderData->bSameSamples  &&
        IS_VPHAL_OUTPUT_PIPE_SFC(pRenderData))
    {
        CopySurfaceValue(m_currentSurface, FFDNSurfaces[(pRenderData->iCurDNOut + 1) & 1]);
    }
    else
    {
        //--------------------------------------------------------------------------
        // Swap buffers for next iteration
        //--------------------------------------------------------------------------
        m_iCurDNIndex     = (pRenderData->iCurDNOut + 1) & 1;
        m_iCurStmmIndex   = (m_iCurStmmIndex + 1) & 1;
    }

    // Set the first frame flag
    if (m_bFirstFrame)
    {
        m_bFirstFrame = false;
    }

#endif

    return eStatus;
}

MOS_STATUS VpVeboxCmdPacket::SetupVeboxRenderMode0(
    PVPHAL_SURFACE           pSrcSurface,
    PVPHAL_SURFACE           pOutputSurface)
{
    MOS_STATUS            eStatus = MOS_STATUS_SUCCESS;
    PVPHAL_VEBOX_RENDER_DATA    pRenderData = GetLastExecRenderData();

    VP_RENDER_CHK_NULL_RETURN(pRenderData);
    VP_RENDER_CHK_NULL_RETURN(pSrcSurface);
    VP_RENDER_CHK_NULL_RETURN(pOutputSurface);
    VP_RENDER_CHK_NULL_RETURN(m_allocator);

    // Ensure the input is ready to be read
    // Currently, mos RegisterResourcere cannot sync the 3d resource.
    // Temporaly, call sync resource to do the sync explicitly.
    m_allocator->SyncOnResource(
        &pSrcSurface->OsResource,
        false);

#ifdef MOVE_TO_HWFILTER
    // Set current DN output buffer
    pRenderData->iCurDNOut = 0;

    // Set the FMD output frames
    pRenderData->iFrame0   = 0;
    pRenderData->iFrame1   = 1;

    // Setup Motion history for DI
    // for ffDI, ffDN and ffDNDI cases
    pRenderData->iCurHistIn  = (m_iCurStmmIndex) & 1;
    pRenderData->iCurHistOut = (m_iCurStmmIndex + 1) & 1;

    // Set current src = current primary input
    CopySurfaceValue(m_currentSurface, pSrcSurface);
    CopySurfaceValue(m_renderTarget, pOutputSurface);

    m_currentSurface->rcMaxSrc = m_currentSurface->rcSrc;

    // Allocate Resources if needed
    VP_RENDER_CHK_STATUS_RETURN(AllocateResources());
    m_iCurFrameID = pSrcSurface->FrameID;
#endif

    // Setup, Copy and Update VEBOX State
    VP_RENDER_CHK_STATUS_RETURN(CopyAndUpdateVeboxState(pSrcSurface));

    return eStatus;

}

#ifdef MOVE_TO_HWFILTER
MOS_STATUS VpVeboxCmdPacket::AllocateResources()
{
    MHW_VEBOX_SURFACE_PARAMS      MhwVeboxSurfaceParam;
    uint32_t               dwWidth;
    uint32_t               dwHeight;
    uint32_t               dwSize;
    MOS_FORMAT             format;
    MOS_TILE_TYPE          TileType;
    bool                   bSurfCompressed = false;
    bool                   bAllocated = false;
    MOS_STATUS             eStatus = MOS_STATUS_SUCCESS;
    int32_t                i = 0;

    MOS_RESOURCE_MMC_MODE       SurfCompressionMode     = MOS_MMC_DISABLED;
    PVPHAL_VEBOX_RENDER_DATA    pRenderData             = GetLastExecRenderData();

    VP_RENDER_CHK_NULL_RETURN(m_hwInterface->m_veboxInterface);
    VP_RENDER_CHK_NULL_RETURN(m_allocator);

    GetOutputSurfParams(format, TileType);

    // Allocate FFDI surfaces----------------------------------------------
    if (m_PacketCaps.bDI || (m_PacketCaps.bDN && IS_VPHAL_OUTPUT_PIPE_SFC(pRenderData)))
    {
        VPHAL_CSPACE        ColorSpace;
        VPHAL_SAMPLE_TYPE   SampleType;

        GetFFDISurfParams(ColorSpace, SampleType);

        for (i = 0; i < m_iNumFFDISurfaces; i++)
        {
            m_allocator->ReAllocateSurface(
                  FFDISurfaces[i],
                  "VeboxFFDISurface",
                  format,
                  MOS_GFXRES_2D,
                  TileType,
                  m_currentSurface->dwWidth,
                  m_currentSurface->dwHeight,
                  bSurfCompressed,
                  SurfCompressionMode,
                  bAllocated);

            FFDISurfaces[i]->SampleType = SampleType;

            // Copy rect sizes so that if input surface state needs to adjust,
            // output surface can be adjustted also.
            FFDISurfaces[i]->rcSrc = m_currentSurface->rcSrc;
            FFDISurfaces[i]->rcDst = m_currentSurface->rcDst;
            // Copy max src rect
            FFDISurfaces[i]->rcMaxSrc = m_currentSurface->rcMaxSrc;

            // Copy Rotation, it's used in setting SFC state
            FFDISurfaces[i]->Rotation = m_currentSurface->Rotation;

            FFDISurfaces[i]->ColorSpace = ColorSpace;
        }
    }

    // Allocate FFDN surfaces---------------------------------------------------
    if (m_PacketCaps.bDN)
    {
        for (i = 0; i < VP_NUM_FFDN_SURFACES; i++)
        {
            m_allocator->ReAllocateSurface(
                  FFDNSurfaces[i],
                  "VeboxFFDNSurface",
                  format,
                  MOS_GFXRES_2D,
                  TileType,
                  m_currentSurface->dwWidth,
                  m_currentSurface->dwHeight,
                  bSurfCompressed,
                  SurfCompressionMode,
                  bAllocated);

            // if allocated, pVeboxState->PreviousSurface is not valid for DN reference.
            if (bAllocated)
            {
                // If DI is enabled, try to use app's reference if provided
                if (pRenderData->bRefValid                         &&
                    pRenderData->bDeinterlace                      &&
                    (m_currentSurface->pBwdRef  != nullptr) &&
                    (FFDNSurfaces[i]->dwPitch == m_currentSurface->pBwdRef->dwPitch))
                {
                    CopySurfaceValue(m_previousSurface, m_currentSurface->pBwdRef);
                }
                else
                {
                    pRenderData->bRefValid = false;
                }
            }

            // DN's output format should be same to input
            FFDNSurfaces[i]->SampleType = m_currentSurface->SampleType;

            // Copy rect sizes so that if input surface state needs to adjust,
            // output surface can be adjustted also.
            FFDNSurfaces[i]->rcSrc    = m_currentSurface->rcSrc;
            FFDNSurfaces[i]->rcDst    = m_currentSurface->rcDst;
            // Copy max src rect
            FFDNSurfaces[i]->rcMaxSrc = m_currentSurface->rcMaxSrc;

            // Set Colorspace of FFDN
            FFDNSurfaces[i]->ColorSpace = m_currentSurface->ColorSpace;

            // Copy FrameID and parameters, as DN output will be used as next blt's current
            FFDNSurfaces[i]->FrameID            = m_currentSurface->FrameID;
            FFDNSurfaces[i]->pDenoiseParams     = m_currentSurface->pDenoiseParams;
        }
    }

    // Adjust the rcMaxSrc of pRenderTarget when Vebox output is enabled
    if (IS_VPHAL_OUTPUT_PIPE_VEBOX(pRenderData) && pRenderData->pRenderTarget)
    {
        pRenderData->pRenderTarget->rcMaxSrc = m_currentSurface->rcMaxSrc;
    }

    // Allocate STMM (Spatial-Temporal Motion Measure) Surfaces------------------
    if (m_PacketCaps.bDI || m_PacketCaps.bDN)
    {
        for (i = 0; i < VP_NUM_STMM_SURFACES; i++)
        {
            m_allocator->ReAllocateSurface(
                  &STMMSurfaces[i],
                  "VeboxSTMMSurface",
                  Format_STMM,
                  MOS_GFXRES_2D,
                  MOS_TILE_Y,
                  m_currentSurface->dwWidth,
                  m_currentSurface->dwHeight,
                  false,
                  MOS_MMC_DISABLED,
                  bAllocated);

            if (bAllocated)
            {
                VPHAL_RENDER_CHK_STATUS(InitSTMMHistory(i));
            }
        }
    }

    // Allocate Statistics State Surface----------------------------------------
    // Width to be a aligned on 64 bytes and height is 1/4 the height
    // Per frame information written twice per frame for 2 slices
    // Surface to be a rectangle aligned with dwWidth to get proper dwSize
    VP_RENDER_CHK_STATUS_RETURN(InitVeboxSurfaceParams(
                            m_currentSurface, &MhwVeboxSurfaceParam));
    VP_RENDER_CHK_STATUS_RETURN(m_hwInterface->m_veboxInterface->VeboxAdjustBoundary(
        &MhwVeboxSurfaceParam,
        &dwWidth,
        &dwHeight,
        false));

    dwWidth     = MOS_ALIGN_CEIL(dwWidth, 64);
    dwHeight    = MOS_ROUNDUP_DIVIDE(dwHeight, 4) +
                  MOS_ROUNDUP_DIVIDE(256 * sizeof(uint32_t), dwWidth);
    dwSize      = dwWidth * dwHeight;

    VP_RENDER_CHK_STATUS_RETURN(m_allocator->ReAllocateSurface(
                &m_veboxStatisticsSurface,
                "VeboxStatisticsSurface",
                Format_Buffer,
                MOS_GFXRES_BUFFER,
                MOS_TILE_LINEAR,
                dwSize,
                1,
                false,
                MOS_MMC_DISABLED,
                bAllocated));

    if (bAllocated)
    {
        // initialize Statistics Surface
        eStatus = m_allocator->OsFillResource(
                      &(m_veboxStatisticsSurface.OsResource),
                      dwSize,
                      0);
    }

    if (eStatus != MOS_STATUS_SUCCESS) {
        FreeResources();
    }

finish:
    return eStatus;
}

MOS_STATUS VpVeboxCmdPacket::GetOutputSurfParams(
    MOS_FORMAT    &Format,
    MOS_TILE_TYPE &TileType)
{
    PVPHAL_VEBOX_RENDER_DATA pRenderData = GetLastExecRenderData();

    if (pRenderData->bDeinterlace)
    {
        //set vebox output as NV12 if render target is non_YUY2 for saving bandwidth.
        if (pRenderData->pRenderTarget->Format == Format_YUY2)
        {
            Format = Format_YUY2;
        }
        else
        {
            Format = Format_NV12;
        }
        TileType = MOS_TILE_Y;
    }
    else
    {
        Format =  m_currentSurface->Format;
        TileType = m_currentSurface->TileType;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpVeboxCmdPacket::GetFFDISurfParams(
    VPHAL_CSPACE      &ColorSpace,
    VPHAL_SAMPLE_TYPE &SampleType)
{
    PVPHAL_VEBOX_RENDER_DATA pRenderData = GetLastExecRenderData();

    VP_RENDER_CHK_NULL_RETURN(pRenderData);
    VP_RENDER_CHK_NULL_RETURN(m_currentSurface);

    if (IS_VPHAL_OUTPUT_PIPE_SFC(pRenderData))
    {
        ColorSpace = m_sfcRenderData.SfcInputCspace;
    }
    else
    {
        ColorSpace = m_currentSurface->ColorSpace;
    }

    // When IECP is enabled and Bob or interlaced scaling is selected for interlaced input,
    // output surface's SampleType should be same to input's. Bob is being
    // done in Composition part
    if ((pRenderData->bIECP &&
        (m_currentSurface->pDeinterlaceParams                         &&
         m_currentSurface->pDeinterlaceParams->DIMode == DI_MODE_BOB)) ||
         m_currentSurface->bInterlacedScaling)
    {
        SampleType = m_currentSurface->SampleType;
    }
    else
    {
        SampleType = SAMPLE_PROGRESSIVE;
    }

    return MOS_STATUS_SUCCESS;
}
//!
//! \brief    Set DI output frame
//! \details  Choose 2nd Field of Previous frame, 1st Field of Current frame
//!           or both frames
//! \param    [in] pRenderData
//!           Pointer to Render data
//! \param    [in] pVeboxState
//!           Pointer to Vebox State
//! \param    [in] pVeboxMode
//!           Pointer to Vebox Mode
//! \return   GFX_MEDIA_VEBOX_DI_OUTPUT_MODE
//!           Return Previous/Current/Both frames
//!
GFX_MEDIA_VEBOX_DI_OUTPUT_MODE VpVeboxCmdPacket::SetDIOutputFrame(
    PVPHAL_VEBOX_RENDER_DATA pRenderData,
    PMHW_VEBOX_MODE          pVeboxMode)
{
    // for 30i->30fps + SFC
    if (IS_VPHAL_OUTPUT_PIPE_SFC(pRenderData) && !pRenderData->b60fpsDi)
    {
        // Set BLT1's Current DI Output as BLT2's input, it is always under Mode0
        // BLT1 output 1st field of current frame for the following cases:
        if (pVeboxMode->DNDIFirstFrame                                                            ||
            (m_currentSurface->SampleType == SAMPLE_INTERLEAVED_ODD_FIRST_BOTTOM_FIELD) ||
            (m_currentSurface->SampleType == SAMPLE_INTERLEAVED_EVEN_FIRST_TOP_FIELD)   ||
            (m_currentSurface->SampleType == SAMPLE_SINGLE_TOP_FIELD)                   ||
            (m_currentSurface->SampleType == SAMPLE_PROGRESSIVE))
        {
            return MEDIA_VEBOX_DI_OUTPUT_CURRENT;
        }
        else
        {
            // First sample output - 2nd field of the previous frame
            return MEDIA_VEBOX_DI_OUTPUT_PREVIOUS;
        }
    }
    // for 30i->60fps or other 30i->30fps cases
    else
    {
        if (IS_VPHAL_OUTPUT_PIPE_VEBOX(pRenderData))
        {
            // Align with the logic in SetupDiIecpState. The previous output surface is not needed for OUTPUT_PIPE_VEBOX case.
            return MEDIA_VEBOX_DI_OUTPUT_CURRENT;
        }
        else
        {
            return pVeboxMode->DNDIFirstFrame ?
                MEDIA_VEBOX_DI_OUTPUT_CURRENT :
                MEDIA_VEBOX_DI_OUTPUT_BOTH;
        }
    }
}

void VpVeboxCmdPacket::CopySurfaceValue(
    PVPHAL_SURFACE              pTargetSurface,
    PVPHAL_SURFACE              pSourceSurface)
{
    if (pTargetSurface == nullptr)
    {
        VP_RENDER_ASSERTMESSAGE("Input pTargetSurface is null");
        return;
    }
    *pTargetSurface = *pSourceSurface;
}

MOS_STATUS VpVeboxCmdPacket::SetupDiIecpStateForOutputSurf(
    bool                                    bDiScdEnable,
    PMHW_VEBOX_DI_IECP_CMD_PARAMS           pVeboxDiIecpCmdParams)
{
    PVPHAL_SURFACE                pSurface;
    PMHW_VEBOX_INTERFACE          pVeboxInterface;
    MHW_VEBOX_SURFACE_CNTL_PARAMS VeboxSurfCntlParams;
    PVPHAL_VEBOX_RENDER_DATA      pRenderData = GetLastExecRenderData();
    MOS_STATUS                    eStatus     = MOS_STATUS_SUCCESS;

    VP_RENDER_CHK_NULL_RETURN(m_hwInterface);
    pVeboxInterface = m_hwInterface->m_veboxInterface;
    VP_RENDER_CHK_NULL_RETURN(pVeboxInterface);

    // VEBOX final output surface
    if (IS_VPHAL_OUTPUT_PIPE_VEBOX(pRenderData))
    {
        VP_RENDER_CHK_NULL_RETURN(pVeboxDiIecpCmdParams);
        pVeboxDiIecpCmdParams->pOsResCurrOutput         = &pRenderData->pRenderTarget->OsResource;
        pVeboxDiIecpCmdParams->dwCurrOutputSurfOffset   = pRenderData->pRenderTarget->dwOffset;
        pVeboxDiIecpCmdParams->CurrOutputSurfCtrl.Value = m_DnDiSurfMemObjCtl.CurrentOutputSurfMemObjCtl;

        // Update control bits for Current Output Surf
        if (m_mmc->IsMmcEnabled())
        {
            pSurface = pRenderData->pRenderTarget;
            MOS_ZeroMemory(&VeboxSurfCntlParams, sizeof(VeboxSurfCntlParams));
            VeboxSurfCntlParams.bIsCompressed   = pSurface->bIsCompressed;
            VeboxSurfCntlParams.CompressionMode = pSurface->CompressionMode;
            VP_RENDER_CHK_STATUS_RETURN(pVeboxInterface->AddVeboxSurfaceControlBits(
                &VeboxSurfCntlParams,
                (uint32_t *)&(pVeboxDiIecpCmdParams->CurrOutputSurfCtrl.Value)));
        }
    }
    else if (bDiScdEnable)
    {
        pVeboxDiIecpCmdParams->pOsResCurrOutput         = &FFDISurfaces[pRenderData->iFrame1]->OsResource;
        pVeboxDiIecpCmdParams->CurrOutputSurfCtrl.Value = m_DnDiSurfMemObjCtl.CurrentOutputSurfMemObjCtl;

        // Update control bits for Current Output Surf
        if (m_mmc->IsMmcEnabled())
        {
            pSurface = FFDISurfaces[pRenderData->iFrame1];
            MOS_ZeroMemory(&VeboxSurfCntlParams, sizeof(VeboxSurfCntlParams));
            VeboxSurfCntlParams.bIsCompressed   = pSurface->bIsCompressed;
            VeboxSurfCntlParams.CompressionMode = pSurface->CompressionMode;
            VP_RENDER_CHK_STATUS_RETURN(pVeboxInterface->AddVeboxSurfaceControlBits(
                &VeboxSurfCntlParams,
                (uint32_t *)&(pVeboxDiIecpCmdParams->CurrOutputSurfCtrl.Value)));
        }

        pVeboxDiIecpCmdParams->pOsResPrevOutput         = &FFDISurfaces[pRenderData->iFrame0]->OsResource;
        pVeboxDiIecpCmdParams->PrevOutputSurfCtrl.Value = m_DnDiSurfMemObjCtl.CurrentOutputSurfMemObjCtl;

        // Update control bits for PrevOutput surface
        if (m_mmc->IsMmcEnabled())
        {
            pSurface = FFDISurfaces[pRenderData->iFrame0];
            MOS_ZeroMemory(&VeboxSurfCntlParams, sizeof(VeboxSurfCntlParams));
            VeboxSurfCntlParams.bIsCompressed   = pSurface->bIsCompressed;
            VeboxSurfCntlParams.CompressionMode = pSurface->CompressionMode;
            VP_RENDER_CHK_STATUS_RETURN(pVeboxInterface->AddVeboxSurfaceControlBits(
                &VeboxSurfCntlParams,
                (uint32_t *)&(pVeboxDiIecpCmdParams->PrevOutputSurfCtrl.Value)));
        }
    }
    else if (IsIECPEnabled()) // IECP output surface without DI
    {
        pVeboxDiIecpCmdParams->pOsResCurrOutput         = &FFDISurfaces[pRenderData->iCurDNOut]->OsResource;
        pVeboxDiIecpCmdParams->CurrOutputSurfCtrl.Value = m_DnDiSurfMemObjCtl.CurrentOutputSurfMemObjCtl;

        // Update control bits for CurrOutputSurf surface
        if (m_mmc->IsMmcEnabled())
        {
            pSurface = FFDISurfaces[pRenderData->iCurDNOut];
            MOS_ZeroMemory(&VeboxSurfCntlParams, sizeof(VeboxSurfCntlParams));
            VeboxSurfCntlParams.bIsCompressed   = pSurface->bIsCompressed;
            VeboxSurfCntlParams.CompressionMode = pSurface->CompressionMode;
            VP_RENDER_CHK_STATUS_RETURN(pVeboxInterface->AddVeboxSurfaceControlBits(
                &VeboxSurfCntlParams,
                (uint32_t *)&(pVeboxDiIecpCmdParams->CurrOutputSurfCtrl.Value)));
        }
    }

    return eStatus;
}

#endif

MOS_STATUS VpVeboxCmdPacket::FreeResources()
{
    VP_RENDER_CHK_NULL_RETURN(m_allocator);

#ifdef MOVE_TO_HWFILTER
    // Free FFDI surfaces
    for (int i = 0; i < m_iNumFFDISurfaces; i++)
    {
        if (FFDISurfaces[i])
        {
            m_allocator->FreeResource(&(FFDISurfaces[i]->OsResource));
        }
    }

    // Free FFDN surfaces
    for (int i = 0; i < VP_NUM_FFDN_SURFACES; i++)
    {
        if (FFDNSurfaces[i])
        {
            m_allocator->FreeResource(&(FFDNSurfaces[i]->OsResource));
        }
    }

    // Free STMM surfaces
    for (int i = 0; i < VP_NUM_STMM_SURFACES; i++)
    {
        m_allocator->FreeResource(&(STMMSurfaces[i].OsResource));
    }

    // Free Statistics data surface for VEBOX
    m_allocator->FreeResource(&m_veboxStatisticsSurface.OsResource);
#endif

    return MOS_STATUS_SUCCESS;
}

VpVeboxCmdPacket::VpVeboxCmdPacket(
    MediaTask * task,
    PVP_MHWINTERFACE hwInterface,
    PVpAllocator &allocator,
    VPMediaMemComp *mmc):
    VpCmdPacket(task, hwInterface, allocator, mmc, VP_PIPELINE_PACKET_FF)
{

}

VpVeboxCmdPacket:: ~VpVeboxCmdPacket()
{
    if (m_sfcRenderData.sfcStateParams)
    {
        MOS_FreeMemAndSetNull(m_sfcRenderData.sfcStateParams);
    }

    MOS_Delete(m_sfcRender);
    MOS_Delete(m_lastExecRenderData);

#ifdef MOVE_TO_HWFILTER
    FreeResources();
    MOS_FreeMemAndSetNull(m_currentSurface);
    MOS_FreeMemAndSetNull(m_previousSurface);
    MOS_FreeMemAndSetNull(m_renderTarget);

    for (uint32_t i = 0; i < VP_MAX_NUM_FFDI_SURFACES; i++)
    {
        if (FFDISurfaces[i])
        {
            MOS_FreeMemAndSetNull(FFDISurfaces[i]);
        }
    }

    for (uint32_t i = 0; i < VP_NUM_FFDN_SURFACES; i++)
    {
        if (FFDNSurfaces[i])
        {
            MOS_FreeMemAndSetNull(FFDNSurfaces[i]);
        }
    }
#endif
}

MOS_STATUS VpVeboxCmdPacket::SetDNPacketParams(
    PVEBOX_DN_PARAMS        pDNParams)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    m_bRefValid      = pDNParams->bRefValid;
    m_veboxPacketSurface.pCurrInput        = pDNParams->VeboxObliParams.pCurrInput;
    m_veboxPacketSurface.pCurrOutput       = pDNParams->VeboxObliParams.pCurrOutput;
    m_veboxPacketSurface.pStatisticsOutput = pDNParams->VeboxObliParams.pStatisticsOutput;
    m_veboxPacketSurface.pLaceOrAceOrRgbHistogram = pDNParams->VeboxObliParams.pLaceOrAceOrRgbHistogram;
    m_veboxPacketSurface.pDenoisedCurrOutput = pDNParams->pDenoisedCurrOutput;
    m_veboxPacketSurface.pPrevInput        = pDNParams->pPrevInput;
    m_veboxPacketSurface.pSTMMInput        = pDNParams->pSTMMInput;
    m_veboxPacketSurface.pSTMMOutput       = pDNParams->pSTMMOutput;

    return eStatus;
}

MOS_STATUS VpVeboxCmdPacket::CopyAndUpdateVeboxState(
    PVPHAL_SURFACE           pSrcSurface)
{
    PVPHAL_VEBOX_RENDER_DATA pRenderData = GetLastExecRenderData();
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    VP_RENDER_CHK_NULL_RETURN(pRenderData);
    VP_RENDER_CHK_NULL_RETURN(pSrcSurface);

    // Setup VEBOX State
    VP_RENDER_CHK_STATUS_RETURN(SetupIndirectStates(
            pSrcSurface,
            IS_VPHAL_OUTPUT_PIPE_VEBOX(pRenderData) ?
            pRenderData->pRenderTarget              :
            FFDISurfaces[0]));

    // Copy VEBOX State
    VP_RENDER_CHK_STATUS_RETURN(CopyVeboxStates());

    // Update VEBOX State
    VP_RENDER_CHK_STATUS_RETURN(UpdateVeboxStates(pSrcSurface));

    return eStatus;
}


//!
//! \brief    Vebox Copy Vebox state heap, intended for HM or IDM
//! \details  Copy Vebox state heap between different memory
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VpVeboxCmdPacket::CopyVeboxStates()
{
    return MOS_STATUS_SUCCESS;  // no need to copy, always use driver resource in clear memory
}

//!
//! \brief    Calculate offsets of statistics surface address based on the
//!           functions which were enabled in the previous call,
//!           and store the width and height of the per-block statistics into DNDI_STATE
//! \details
//! Layout of Statistics surface when Temporal DI enabled
//!     --------------------------------------------------------------\n
//!     | 16 bytes for x=0, Y=0       | 16 bytes for x=16, Y=0       | ...\n
//!     |-------------------------------------------------------------\n
//!     | 16 bytes for x=0, Y=4       | ...\n
//!     |------------------------------\n
//!     | ...\n
//!     |------------------------------\n
//!     | 16 bytes for x=0, Y=height-4| ...\n
//!     |-----------------------------------------------Pitch----------------------------------------------------------\n
//!     | 256 DW of ACE histogram Slice 0 (Previous)| 17 DW Reserved         | 2 DW STD0 | 2 DW GCC0 | 11 DW Reserved |\n
//!     |--------------------------------------------------------------------------------------------------------------\n
//!     | 256 DW of ACE histogram Slice 0 (Current) | 11 DW FMD0 | 6 DW GNE0 | 2 DW STD0 | 2 DW GCC0 | 11 DW Reserved |\n
//!     |--------------------------------------------------------------------------------------------------------------\n
//!     | 256 DW of ACE histogram Slice 1 (Previous)| 17 DW Reserved         | 2 DW STD1 | 2 DW GCC1 | 11 DW Reserved |\n
//!     |--------------------------------------------------------------------------------------------------------------\n
//!     | 256 DW of ACE histogram Slice 1 (Current) | 11 DW FMD1 | 6 DW GNE1 | 2 DW STD1 | 2 DW GCC1 | 11 DW Reserved |\n
//!     ---------------------------------------------------------------------------------------------------------------\n
//!
//! Layout of Statistics surface when DN or Spatial DI enabled (and Temporal DI disabled)
//!     --------------------------------------------------------------\n
//!     | 16 bytes for x=0, Y=0       | 16 bytes for x=16, Y=0       | ...\n
//!     |-------------------------------------------------------------\n
//!     | 16 bytes for x=0, Y=4       | ...\n
//!     |------------------------------\n
//!     | ...\n
//!     |------------------------------\n
//!     | 16 bytes for x=0, Y=height-4| ...\n
//!     |-----------------------------------------------Pitch----------------------------------------------------------\n
//!     | 256 DW of ACE histogram Slice 0 (Input)   | 11 DW FMD0 | 6 DW GNE0 | 2 DW STD0 | 2 DW GCC0 | 11 DW Reserved |\n
//!     |--------------------------------------------------------------------------------------------------------------\n
//!     | 256 DW of ACE histogram Slice 1 (Input)   | 11 DW FMD1 | 6 DW GNE1 | 2 DW STD1 | 2 DW GCC1 | 11 DW Reserved |\n
//!     ---------------------------------------------------------------------------------------------------------------\n
//!
//! Layout of Statistics surface when both DN and DI are disabled
//!     ------------------------------------------------Pitch----------------------------------------------------------\n
//!     | 256 DW of ACE histogram Slice 0 (Input)   | 17 DW Reserved         | 2 DW STD0 | 2 DW GCC0 | 11 DW Reserved |\n
//!     |--------------------------------------------------------------------------------------------------------------\n
//!     | 256 DW of ACE histogram Slice 1 (Input)   | 17 DW Reserved         | 2 DW STD1 | 2 DW GCC1 | 11 DW Reserved |\n
//!     ---------------------------------------------------------------------------------------------------------------\n
//! \param    [out] pStatSlice0Offset
//!           Statistics surface Slice 0 base pointer
//! \param    [out] pStatSlice1Offset
//!           Statistics surface Slice 1 base pointer
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VpVeboxCmdPacket::GetStatisticsSurfaceOffsets(
    int32_t*                    pStatSlice0Offset,
    int32_t*                    pStatSlice1Offset)
{
    uint32_t    uiPitch;
    int32_t     iOffset;
    MOS_STATUS  eStatus;

    eStatus     = MOS_STATUS_UNKNOWN;
    uiPitch     = 0;

    // Query platform dependent size of per frame information
    VP_RENDER_CHK_STATUS(QueryStatLayout(
        VEBOX_STAT_QUERY_PER_FRAME_SIZE, &uiPitch));

    // Get the base address of Frame based statistics for each slice
    if (m_PacketCaps.bDI || m_PacketCaps.bIECP) // VEBOX, VEBOX+IECP
    {
        // Frame based statistics begins after Encoder statistics
        iOffset = m_dwVeboxPerBlockStatisticsWidth *
                  m_dwVeboxPerBlockStatisticsHeight;

        *pStatSlice0Offset = iOffset + uiPitch;                                     // Slice 0 current frame
        *pStatSlice1Offset = iOffset + uiPitch * 3;                                 // Slice 1 current frame
    }
    else if (m_PacketCaps.bDN || m_PacketCaps.bIECP) // DN, DN_IECP, SpatialDI
    {
        // Frame based statistics begins after Encoder statistics
        iOffset = m_dwVeboxPerBlockStatisticsWidth *
                  m_dwVeboxPerBlockStatisticsHeight;

        *pStatSlice0Offset = iOffset;                                               // Slice 0 input frame
        *pStatSlice1Offset = iOffset + uiPitch;                                     // Slice 1 input frame
    }
    else // IECP only
    {
        *pStatSlice0Offset = 0;                                                     // Slice 0 input frame
        *pStatSlice1Offset = uiPitch;                                               // Slice 1 input frame
    }

finish:
    return eStatus;
}

//!
//! \brief    Vebox state heap update for auto mode features
//! \details  Update Vebox indirect states for auto mode features
//! \param    [in] pSrcSurface
//!           Pointer to input surface of Vebox
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VpVeboxCmdPacket::UpdateVeboxStates(
    PVPHAL_SURFACE              pSrcSurface)
{
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpVeboxCmdPacket::SetupIndirectStates(
    PVPHAL_SURFACE              pSrcSurface,
    PVPHAL_SURFACE              pOutSurface)
{
    PMHW_VEBOX_INTERFACE            pVeboxInterface    = nullptr;
    MHW_VEBOX_IECP_PARAMS           VeboxIecpParams    = {};
    MHW_VEBOX_GAMUT_PARAMS          VeboxGamutParams   = {};
    PVPHAL_VEBOX_RENDER_DATA        pRenderData        = GetLastExecRenderData();

    VP_RENDER_CHK_NULL_RETURN(pRenderData);
    VP_RENDER_CHK_NULL_RETURN(pSrcSurface);

    pVeboxInterface = m_hwInterface->m_veboxInterface;
    VP_RENDER_CHK_NULL_RETURN(pVeboxInterface);

    // Initialize structure before using
    MOS_ZeroMemory(&VeboxIecpParams, sizeof(MHW_VEBOX_IECP_PARAMS));
    MOS_ZeroMemory(&VeboxGamutParams, sizeof(MHW_VEBOX_GAMUT_PARAMS));
    // Gamma is default to 2.2 since SDR uses 2.2
    VeboxGamutParams.InputGammaValue    = MHW_GAMMA_2P2;
    VeboxGamutParams.OutputGammaValue   = MHW_GAMMA_2P2;

    //----------------------------------
    // Allocate and reset VEBOX state
    //----------------------------------
    VP_RENDER_CHK_STATUS_RETURN(pVeboxInterface->AssignVeboxState());

    // Set VEBOX State Params
    if (pRenderData->bDeinterlace       ||
        pRenderData->bQueryVariance     ||
        pRenderData->bDenoise           ||
        pRenderData->bChromaDenoise)
    {
        VP_RENDER_CHK_STATUS_RETURN(SetVeboxDNDIParams(pSrcSurface));
    }

    // Set IECP State Params
    if (pRenderData->bIECP)
    {
       /* Wait for Filter to set IECP Prarams to here.
        VP_RENDER_CHK_STATUS_RETURN(SetIECPParams(
                                    pSrcSurface,
                                    pOutSurface,
                                    pRenderData,
                                    &VeboxIecpParams));
        */
        VP_RENDER_CHK_STATUS_RETURN(pVeboxInterface->AddVeboxIecpState(
              &VeboxIecpParams));
    }

    if (pRenderData->GetVeboxStateParams()->pVphalVeboxDndiParams)
    {
        MhwVeboxInterfaceG12 *pVeboxInterfaceExt12;
        pVeboxInterfaceExt12 = (MhwVeboxInterfaceG12 *)pVeboxInterface;
        //VP_RENDER_CHK_STATUS_RETURN(pVeboxInterfaceExt12->SetVeboxChromaParams(
        //    (MhwVeboxInterfaceG12::MHW_VEBOX_CHROMA_PARAMS *)&pRenderData->VeboxChromaParams));
        VP_RENDER_CHK_STATUS_RETURN(pVeboxInterface->AddVeboxDndiState(
            pRenderData->GetVeboxStateParams()->pVphalVeboxDndiParams));
    }

    return MOS_STATUS_SUCCESS;
}

void VpVeboxCmdPacket::VeboxGetBeCSCMatrix(
  PVPHAL_SURFACE pSrcSurface,
  PVPHAL_SURFACE pOutSurface)
{
    float                       fTemp[3];

    // Get the matrix to use for conversion
    VpHal_GetCscMatrix(
        pSrcSurface->ColorSpace,
        pOutSurface->ColorSpace,
        m_fCscCoeff,
        m_fCscInOffset,
        m_fCscOutOffset);

    // Vebox CSC converts RGB input to YUV for SFC
    // Vebox only supports A8B8G8R8 input, swap the 1st and 3rd
    // columns of the transfer matrix for A8R8G8B8 and X8R8G8B8
    // This only happens when SFC output is used
    if ((pSrcSurface->Format == Format_A8R8G8B8) ||
      (pSrcSurface->Format == Format_X8R8G8B8))
    {
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
}

MOS_STATUS VpVeboxCmdPacket::IsCmdParamsValid(
    const MHW_VEBOX_STATE_CMD_PARAMS        &VeboxStateCmdParams,
    const MHW_VEBOX_DI_IECP_CMD_PARAMS      &VeboxDiIecpCmdParams)
{
    const MHW_VEBOX_MODE    &veboxMode          = VeboxStateCmdParams.VeboxMode;

    if (veboxMode.DIEnable)
    {
        if (nullptr == VeboxDiIecpCmdParams.pOsResPrevOutput &&
            (MEDIA_VEBOX_DI_OUTPUT_PREVIOUS == veboxMode.DIOutputFrames || MEDIA_VEBOX_DI_OUTPUT_BOTH == veboxMode.DIOutputFrames))
        {
            return MOS_STATUS_INVALID_PARAMETER;
        }
        if (nullptr == VeboxDiIecpCmdParams.pOsResCurrOutput &&
            (MEDIA_VEBOX_DI_OUTPUT_CURRENT == veboxMode.DIOutputFrames || MEDIA_VEBOX_DI_OUTPUT_BOTH == veboxMode.DIOutputFrames))
        {
            return MOS_STATUS_INVALID_PARAMETER;
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpVeboxCmdPacket::VeboxSetPerfTag(
    MOS_FORMAT   srcFmt)
{
    MOS_STATUS                eStatus = MOS_STATUS_SUCCESS;
    PVPHAL_PERFTAG            pPerfTag;
    PVPHAL_VEBOX_RENDER_DATA  pRenderData = GetLastExecRenderData();

    pPerfTag = &pRenderData->PerfTag;

    switch (srcFmt)
    {
        case Format_NV12:
            *pPerfTag = VPHAL_NV12_420CP;
            break;
        CASE_PA_FORMAT:
            *pPerfTag = VPHAL_PA_422CP;
            break;

        case Format_P010:
            // P010 Input Support for VEBOX, SFC
            *pPerfTag = VPHAL_VEBOX_P010;
            break;

        case Format_P016:
            // P016 Input Support for VEBOX, SFC
            *pPerfTag = VPHAL_VEBOX_P016;
            break;

        case Format_P210:
            // P210 Input Support for VEBOX, SFC
            *pPerfTag = VPHAL_VEBOX_P210;
            break;

        case Format_P216:
            // P216 Input Support for VEBOX, SFC
            *pPerfTag = VPHAL_VEBOX_P216;
            break;

        case Format_Y210:
            // Y210 Input Support for VEBOX, SFC
            *pPerfTag = VPHAL_VEBOX_Y210;
            break;

        case Format_Y216:
            // Y216 Input Support for VEBOX, SFC
            *pPerfTag = VPHAL_VEBOX_Y216;
            break;

        case Format_Y410:
            // Y410 Input Support for VEBOX, SFC
            *pPerfTag = VPHAL_VEBOX_Y410;
            break;

        case Format_Y416:
            // Y416 Input Support for VEBOX, SFC
            *pPerfTag = VPHAL_VEBOX_Y416;
            break;

        CASE_RGB32_FORMAT:
        case Format_AYUV:
        case Format_A16B16G16R16:
        case Format_A16R16G16B16:
        case Format_A16B16G16R16F:
        case Format_A16R16G16B16F:
            *pPerfTag = VPHAL_NONE;
            break;

        default:
            VPHAL_RENDER_ASSERTMESSAGE("Format Not found.");
            *pPerfTag = VPHAL_NONE;
            eStatus = MOS_STATUS_INVALID_PARAMETER;
    } // switch (srcFmt)

    return eStatus;
}

}

