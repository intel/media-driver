/*
* Copyright (c) 2018-2022, Intel Corporation
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
#include "hal_oca_interface_next.h"
#include "vp_render_sfc_m12.h"
#include "vp_render_ief.h"
#include "vp_feature_caps.h"
#include "vp_platform_interface.h"
#include "mhw_vebox_itf.h"
#include "mhw_mi_itf.h"
#include "mhw_mi_cmdpar.h"
#include "mhw_utilities.h"
#include "hal_oca_interface.h"

namespace vp {

#define INTERP(x0, x1, x, y0, y1)   ((uint32_t) floor(y0+(x-x0)*(y1-y0)/(double)(x1-x0)))

const uint32_t  VpVeboxCmdPacket::m_satP1Table[MHW_STE_FACTOR_MAX + 1] = {
    0x00000000, 0xfffffffe, 0xfffffffc, 0xfffffffa, 0xfffffff6, 0xfffffff4, 0xfffffff2, 0xfffffff0, 0xffffffee, 0xffffffec };

const uint32_t   VpVeboxCmdPacket::m_satS0Table[MHW_STE_FACTOR_MAX + 1] = {
    0x000000ef, 0x00000100, 0x00000113, 0x00000129, 0x0000017a, 0x000001a2, 0x000001d3, 0x00000211, 0x00000262, 0x000002d1 };

const uint32_t   VpVeboxCmdPacket::m_satS1Table[MHW_STE_FACTOR_MAX + 1] = {
    0x000000ab, 0x00000080, 0x00000066, 0x00000055, 0x000000c2, 0x000000b9, 0x000000b0, 0x000000a9, 0x000000a2, 0x0000009c };

void VpVeboxCmdPacket::SetupSurfaceStates(
    PVPHAL_VEBOX_SURFACE_STATE_CMD_PARAMS   pVeboxSurfaceStateCmdParams)
{
    VP_FUNC_CALL();

    VP_PUBLIC_CHK_NULL_NO_STATUS_RETURN(pVeboxSurfaceStateCmdParams);
    MOS_ZeroMemory(pVeboxSurfaceStateCmdParams, sizeof(VPHAL_VEBOX_SURFACE_STATE_CMD_PARAMS));
    pVeboxSurfaceStateCmdParams->pSurfInput    = m_veboxPacketSurface.pCurrInput;
    pVeboxSurfaceStateCmdParams->pSurfOutput   = m_veboxPacketSurface.pCurrOutput;
    pVeboxSurfaceStateCmdParams->pSurfSTMM     = m_veboxPacketSurface.pSTMMInput;
    pVeboxSurfaceStateCmdParams->pSurfDNOutput = m_veboxPacketSurface.pDenoisedCurrOutput;
    pVeboxSurfaceStateCmdParams->bDIEnable     = m_PacketCaps.bDI;
    pVeboxSurfaceStateCmdParams->b3DlutEnable  = m_PacketCaps.bHDR3DLUT;  // Need to consider cappipe
}

MOS_STATUS VpVeboxCmdPacket::Init3DLutTable(PVP_SURFACE surf3DLut)
{
    VP_FUNC_CALL();
    PVP_SURFACE        surf3DLut2D  = nullptr;
    VpVeboxRenderData *renderData   = GetLastExecRenderData();

    VP_RENDER_CHK_NULL_RETURN(renderData);

    if (!renderData->HDR3DLUT.is3DLutTableFilled)
    {
        VP_RENDER_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
    }

    // kernel already update this surface
    surf3DLut2D = GetSurface(SurfaceType3DLut2D);

    if (nullptr == surf3DLut2D)
    {
        // HDR_STAGE_VEBOX_3DLUT_NO_UPDATE should come here.
        VP_RENDER_NORMALMESSAGE("3DLut table is calculated by kernel and no need be updated.");
        return MOS_STATUS_SUCCESS;
    }

    VP_RENDER_CHK_NULL_RETURN(m_allocator);
    VP_RENDER_CHK_NULL_RETURN(renderData);
    VP_RENDER_CHK_NULL_RETURN(surf3DLut);
    VP_RENDER_CHK_NULL_RETURN(surf3DLut->osSurface);
    VP_RENDER_CHK_NULL_RETURN(surf3DLut2D->osSurface);

    VP_RENDER_NORMALMESSAGE("3DLut table is calculated by kernel.");

    uint8_t *buf3DLut = (uint8_t *)m_allocator->LockResourceForWrite(&surf3DLut->osSurface->OsResource);
    VP_RENDER_CHK_NULL_RETURN(buf3DLut);
    uint8_t *buf3DLut2D = (uint8_t *)m_allocator->LockResourceForWrite(&surf3DLut2D->osSurface->OsResource);
    VP_RENDER_CHK_NULL_RETURN(buf3DLut2D);

    uint32_t widthInByte = surf3DLut2D->osSurface->dwWidth * 4;
    uint32_t pitchInByte = surf3DLut2D->osSurface->dwPitch;
    uint32_t height      = surf3DLut2D->osSurface->dwHeight;
    uint32_t size        = surf3DLut->osSurface->dwSize;

    if (size < widthInByte * height)
    {
        VP_RENDER_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
    }

    uint32_t offset   = 0;
    uint32_t offset2D = 0;
    for (uint32_t h = 0; h < height;
         ++h, offset += widthInByte, offset2D += pitchInByte)
    {
        MOS_SecureMemcpy(buf3DLut + offset, widthInByte, buf3DLut2D + offset2D, widthInByte);
    }

    VP_PUBLIC_CHK_STATUS_RETURN(m_allocator->UnLock(&surf3DLut2D->osSurface->OsResource));
    VP_PUBLIC_CHK_STATUS_RETURN(m_allocator->UnLock(&surf3DLut->osSurface->OsResource));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpVeboxCmdPacket::SetupVebox3DLutForHDR(
    mhw::vebox::VEBOX_STATE_PAR &veboxStateCmdParams)
{
    PMHW_VEBOX_MODE   pVeboxMode = nullptr;
    PMHW_VEBOX_3D_LUT pLUT3D     = nullptr;
    PVP_SURFACE       surf3DLut  = GetSurface(SurfaceType3DLut);

    VP_RENDER_CHK_NULL_RETURN(m_surfMemCacheCtl);
    VP_RENDER_CHK_NULL_RETURN(surf3DLut);
    VP_RENDER_CHK_NULL_RETURN(surf3DLut->osSurface);

    VP_RENDER_CHK_STATUS_RETURN(Init3DLutTable(surf3DLut));

    pVeboxMode  = &veboxStateCmdParams.VeboxMode;
    pLUT3D      = &veboxStateCmdParams.LUT3D;

    pLUT3D->ArbitrationPriorityControl      = 0;
    pLUT3D->Lut3dEnable                     = true;
    // Config 3DLut size to 65 for HDR usage.
    pLUT3D->Lut3dSize                       = 2;

    veboxStateCmdParams.Vebox3DLookUpTablesSurfCtrl.Value =
        m_surfMemCacheCtl->DnDi.Vebox3DLookUpTablesSurfMemObjCtl;

    pVeboxMode->ColorGamutExpansionEnable       = true;

    veboxStateCmdParams.pVebox3DLookUpTables  = &surf3DLut->osSurface->OsResource;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpVeboxCmdPacket::SetupHDRLuts(
    mhw::vebox::VEBOX_STATE_PAR &veboxStateCmdParams)
{
    VP_FUNC_CALL();
    VpVeboxRenderData *renderData = GetLastExecRenderData();
    VP_RENDER_CHK_NULL_RETURN(renderData);

    if (renderData->HDR3DLUT.bHdr3DLut)
    {
        VP_RENDER_CHK_STATUS_RETURN(SetupVebox3DLutForHDR(veboxStateCmdParams));
    }
    else
    {
        veboxStateCmdParams.pVebox3DLookUpTables = nullptr;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpVeboxCmdPacket::SetupVeboxState(mhw::vebox::VEBOX_STATE_PAR& veboxStateCmdParams)
{
    VP_FUNC_CALL();

    PMHW_VEBOX_MODE         pVeboxMode   = nullptr;

    pVeboxMode = &veboxStateCmdParams.VeboxMode;
    VP_RENDER_CHK_NULL_RETURN(pVeboxMode);

    VpVeboxRenderData* pRenderData = GetLastExecRenderData();
    VP_RENDER_CHK_NULL_RETURN(pRenderData);

    MOS_ZeroMemory(&veboxStateCmdParams, sizeof(veboxStateCmdParams));

    // Always enable the global iecp to align with the legacy path.
    // For next step, need to enable it only when necessary.
    pVeboxMode->GlobalIECPEnable = true;

    pVeboxMode->DIEnable = m_PacketCaps.bDI;

    pVeboxMode->SFCParallelWriteEnable = m_IsSfcUsed &&
                                            (m_PacketCaps.bDN || m_PacketCaps.bDI);
    pVeboxMode->DNEnable = m_PacketCaps.bDN;
    pVeboxMode->DNDIFirstFrame = m_DNDIFirstFrame;
    pVeboxMode->DIOutputFrames = m_DIOutputFrames;
    pVeboxMode->DisableEncoderStatistics = true;
    pVeboxMode->DisableTemporalDenoiseFilter = false;

    if (!m_PacketCaps.bDI  &&
        m_PacketCaps.bDN   &&
        (IS_RGB_CSPACE(m_currentSurface->ColorSpace)))
    {
        // RGB DN must disable Temporal filter in Vebox
        pVeboxMode->DisableTemporalDenoiseFilter = true;
        pVeboxMode->GlobalIECPEnable             = true;
    }

    pVeboxMode->ColorGamutCompressionEnable = m_PacketCaps.bCGC && !m_PacketCaps.bBt2020ToRGB;
    pVeboxMode->ColorGamutExpansionEnable   = m_PacketCaps.bBt2020ToRGB;

    veboxStateCmdParams.bUseVeboxHeapKernelResource = UseKernelResource();

    //Set up Chroma Sampling
    veboxStateCmdParams.ChromaSampling = pRenderData->GetChromaSubSamplingParams();

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

    VP_RENDER_CHK_STATUS_RETURN(SetupHDRLuts(veboxStateCmdParams));

    veboxStateCmdParams.bCmBuffer = false;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpVeboxCmdPacket::InitCmdBufferWithVeParams(
    PRENDERHAL_INTERFACE pRenderHal,
    MOS_COMMAND_BUFFER & CmdBuffer,
    PRENDERHAL_GENERIC_PROLOG_PARAMS pGenericPrologParams)
{
    VP_FUNC_CALL();

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
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VpVeboxCmdPacket::InitSTMMHistory()
{
    VP_FUNC_CALL();

    MOS_STATUS          eStatus;
    uint32_t            dwSize;
    int32_t             x, y;
    uint8_t*            pByte;
    MOS_LOCK_PARAMS     LockFlags;
    PVP_SURFACE         stmmSurface = GetSurface(SurfaceTypeSTMMIn);
    eStatus         = MOS_STATUS_SUCCESS;

    VP_PUBLIC_CHK_NULL_RETURN(stmmSurface);
    VP_PUBLIC_CHK_NULL_RETURN(stmmSurface->osSurface);

    MOS_ZeroMemory(&LockFlags, sizeof(MOS_LOCK_PARAMS));

    LockFlags.WriteOnly    = 1;
    LockFlags.TiledAsTiled = 1; // Set TiledAsTiled flag for STMM surface initialization.

    // Lock the surface for writing
    pByte = (uint8_t*)m_allocator->Lock(
        &stmmSurface->osSurface->OsResource,
        &LockFlags);

    VPHAL_RENDER_CHK_NULL(pByte);

    dwSize = stmmSurface->osSurface->dwWidth >> 2;

    // Fill STMM surface with DN history init values.
    for (y = 0; y < (int32_t)stmmSurface->osSurface->dwHeight; y++)
    {
        for (x = 0; x < (int32_t)dwSize; x++)
        {
            MOS_FillMemory(pByte, 2, DNDI_HISTORY_INITVALUE);
            // skip denosie history init.
            pByte += 4;
        }

        pByte += stmmSurface->osSurface->dwPitch - stmmSurface->osSurface->dwWidth;
    }

    // Unlock the surface
    VPHAL_RENDER_CHK_STATUS(m_allocator->UnLock(&stmmSurface->osSurface->OsResource));

finish:
    return eStatus;
}

bool VpVeboxCmdPacket::IsFormatMMCSupported(MOS_FORMAT Format)
{
    VP_FUNC_CALL();

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
    VP_PUBLIC_CHK_NULL_RETURN(m_sfcRender);
    VP_PUBLIC_CHK_NULL_RETURN(m_renderTarget);
    VP_PUBLIC_CHK_NULL_RETURN(m_renderTarget->osSurface);
    VP_PUBLIC_CHK_NULL_RETURN(m_mmc);

    VP_PUBLIC_CHK_STATUS_RETURN(m_sfcRender->SetMmcParams(m_renderTarget->osSurface,
                                                        IsFormatMMCSupported(m_renderTarget->osSurface->Format),
                                                        m_mmc->IsMmcEnabled()));

    return MOS_STATUS_SUCCESS;
}

VP_SURFACE *VpVeboxCmdPacket::GetSurface(SurfaceType type)
{
    VP_FUNC_CALL();

    auto it = m_surfSetting.surfGroup.find(type);
    VP_SURFACE *surf = (m_surfSetting.surfGroup.end() != it) ? it->second : nullptr;
    if (SurfaceTypeVeboxCurrentOutput == type && nullptr == surf && !m_IsSfcUsed)
    {
        // Vebox output case.
        surf = m_renderTarget;
    }
    else if (SurfaceTypeVeboxInput == type && surf)
    {
        // The vp surface object for external surface will be destroyed by hw filter before packet submit.
        // Store the vp surface object inside packet.
        if (MOS_FAILED(m_allocator->CopyVpSurface(*m_currentSurface ,*surf)))
        {
            return nullptr;
        }
        m_currentSurface->rcMaxSrc = m_currentSurface->rcSrc;
        surf = m_currentSurface;
    }
    else if (SurfaceTypeVeboxPreviousInput == type && surf)
    {
        // The vp surface object for external surface will be destroyed by hw filter before packet submit.
        // Store the vp surface object inside packet.
        if (MOS_FAILED(m_allocator->CopyVpSurface(*m_previousSurface ,*surf)))
        {
            return nullptr;
        }
        surf = m_previousSurface;
    }
    return surf;
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

MOS_STATUS VpVeboxCmdPacket::SetVeboxBeCSCParams(PVEBOX_CSC_PARAMS cscParams)
{
    VP_FUNC_CALL();

    VP_RENDER_CHK_NULL_RETURN(cscParams);

    VpVeboxRenderData* pRenderData = GetLastExecRenderData();
    VP_RENDER_ASSERT(pRenderData);

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
    VP_RENDER_ASSERT(pRenderData);

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
    VP_RENDER_ASSERT(pRenderData);

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

MOS_STATUS VpVeboxCmdPacket::SetDnParams(
    PVEBOX_DN_PARAMS                    pDnParams)
{
    VP_FUNC_CALL();

    MOS_STATUS                      eStatus = MOS_STATUS_SUCCESS;
    VpVeboxRenderData               *pRenderData = GetLastExecRenderData();
    VP_SAMPLER_STATE_DN_PARAM       lumaParams = {};
    VPHAL_DNUV_PARAMS               chromaParams = {};

    VP_RENDER_ASSERT(pDnParams);
    VP_RENDER_ASSERT(pRenderData);

    pRenderData->DN.bDnEnabled = pDnParams->bDnEnabled;
    pRenderData->DN.bAutoDetect = pDnParams->bAutoDetect;
    pRenderData->DN.bChromaDnEnabled = pDnParams->bChromaDenoise;
    pRenderData->DN.bHvsDnEnabled    = pDnParams->bEnableHVSDenoise;

    pRenderData->GetDNDIParams().bChromaDNEnable = pDnParams->bChromaDenoise;
    pRenderData->GetDNDIParams().bProgressiveDN  = pDnParams->bDnEnabled && pDnParams->bProgressive;
    pRenderData->GetHVSParams().QP               = pDnParams->HVSDenoise.QP;
    pRenderData->GetHVSParams().Mode             = pDnParams->HVSDenoise.Mode;
    pRenderData->GetHVSParams().Strength         = pDnParams->HVSDenoise.Strength;

    GetDnLumaParams(pDnParams->bDnEnabled, pDnParams->bAutoDetect, pDnParams->fDenoiseFactor, m_PacketCaps.bRefValid, &lumaParams);
    GetDnChromaParams(pDnParams->bChromaDenoise, pDnParams->bAutoDetect, pDnParams->fDenoiseFactor, &chromaParams);

    // Setup Denoise Params
    ConfigLumaPixRange(pDnParams->bDnEnabled, pDnParams->bAutoDetect, pDnParams->fDenoiseFactor);
    ConfigChromaPixRange(pDnParams->bChromaDenoise, pDnParams->bAutoDetect, pDnParams->fDenoiseFactor);
    ConfigDnLumaChromaParams(pDnParams->bDnEnabled, pDnParams->bChromaDenoise, &lumaParams, &chromaParams);

    // bDNDITopFirst in DNDI parameters need be configured during SetDIParams.

    return eStatus;
}

MOS_STATUS VpVeboxCmdPacket::SetSteParams(
    PVEBOX_STE_PARAMS                    pSteParams)
{
    VP_FUNC_CALL();

    VpVeboxRenderData               *pRenderData = GetLastExecRenderData();
    MHW_VEBOX_IECP_PARAMS&           mhwVeboxIecpParams = pRenderData->GetIECPParams();

    VP_RENDER_ASSERT(pSteParams);
    VP_RENDER_ASSERT(pRenderData);

    if (pSteParams->bEnableSTE)
    {
        pRenderData->IECP.STE.bSteEnabled = true;
        mhwVeboxIecpParams.ColorPipeParams.bActive = true;
        mhwVeboxIecpParams.ColorPipeParams.bEnableSTE = true;

        if (pSteParams->dwSTEFactor > MHW_STE_FACTOR_MAX)
        {
            mhwVeboxIecpParams.ColorPipeParams.SteParams.dwSTEFactor = MHW_STE_FACTOR_MAX;
            mhwVeboxIecpParams.ColorPipeParams.SteParams.satP1 = m_satP1Table[MHW_STE_FACTOR_MAX];
            mhwVeboxIecpParams.ColorPipeParams.SteParams.satS0 = m_satS0Table[MHW_STE_FACTOR_MAX];
            mhwVeboxIecpParams.ColorPipeParams.SteParams.satS1 = m_satS1Table[MHW_STE_FACTOR_MAX];
        }
        else
        {
            mhwVeboxIecpParams.ColorPipeParams.SteParams.dwSTEFactor = pSteParams->dwSTEFactor;
            mhwVeboxIecpParams.ColorPipeParams.SteParams.satP1 = m_satP1Table[pSteParams->dwSTEFactor];
            mhwVeboxIecpParams.ColorPipeParams.SteParams.satS0 = m_satS0Table[pSteParams->dwSTEFactor];
            mhwVeboxIecpParams.ColorPipeParams.SteParams.satS1 = m_satS1Table[pSteParams->dwSTEFactor];
        }
    }
    else
    {
        pRenderData->IECP.STE.bSteEnabled = false;
        mhwVeboxIecpParams.ColorPipeParams.bEnableSTE = false;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpVeboxCmdPacket::SetTccParams(
    PVEBOX_TCC_PARAMS                    pTccParams)
{
    VP_FUNC_CALL();

    VpVeboxRenderData               *pRenderData = GetLastExecRenderData();
    MHW_VEBOX_IECP_PARAMS&           mhwVeboxIecpParams = pRenderData->GetIECPParams();

    VP_RENDER_ASSERT(pTccParams);
    VP_RENDER_ASSERT(pRenderData);

    if (pTccParams->bEnableTCC)
    {
        pRenderData->IECP.TCC.bTccEnabled = true;
        mhwVeboxIecpParams.ColorPipeParams.bActive = true;
        mhwVeboxIecpParams.ColorPipeParams.bEnableTCC = true;
        mhwVeboxIecpParams.ColorPipeParams.TccParams.Magenta = pTccParams->Magenta;
        mhwVeboxIecpParams.ColorPipeParams.TccParams.Red = pTccParams->Red;
        mhwVeboxIecpParams.ColorPipeParams.TccParams.Yellow = pTccParams->Yellow;
        mhwVeboxIecpParams.ColorPipeParams.TccParams.Green = pTccParams->Green;
        mhwVeboxIecpParams.ColorPipeParams.TccParams.Cyan = pTccParams->Cyan;
        mhwVeboxIecpParams.ColorPipeParams.TccParams.Blue = pTccParams->Blue;
    }
    else
    {
        pRenderData->IECP.TCC.bTccEnabled = false;
        mhwVeboxIecpParams.ColorPipeParams.bEnableTCC = false;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpVeboxCmdPacket::SetProcampParams(
    PVEBOX_PROCAMP_PARAMS                    pProcampParams)
{
    VP_FUNC_CALL();

    VpVeboxRenderData               *pRenderData = GetLastExecRenderData();
    MHW_VEBOX_IECP_PARAMS&           mhwVeboxIecpParams = pRenderData->GetIECPParams();

    VP_RENDER_ASSERT(pProcampParams);
    VP_RENDER_ASSERT(pRenderData);

    if (pProcampParams->bEnableProcamp)
    {
        pRenderData->IECP.PROCAMP.bProcampEnabled = true;
        mhwVeboxIecpParams.ProcAmpParams.bActive = true;
        mhwVeboxIecpParams.ProcAmpParams.bEnabled = true;
        mhwVeboxIecpParams.ProcAmpParams.brightness = (uint32_t)MOS_F_ROUND(pProcampParams->fBrightness * 16.0F);  // S7.4
        mhwVeboxIecpParams.ProcAmpParams.contrast   = (uint32_t)MOS_UF_ROUND(pProcampParams->fContrast * 128.0F);  // U4.7
        mhwVeboxIecpParams.ProcAmpParams.sinCS      = (uint32_t)MOS_F_ROUND(sin(MHW_DEGREE_TO_RADIAN(pProcampParams->fHue)) *
                                                         pProcampParams->fContrast *
                                                         pProcampParams->fSaturation * 256.0F);  // S7.8
        mhwVeboxIecpParams.ProcAmpParams.cosCS      = (uint32_t)MOS_F_ROUND(cos(MHW_DEGREE_TO_RADIAN(pProcampParams->fHue)) *
                                                         pProcampParams->fContrast *
                                                         pProcampParams->fSaturation * 256.0F);  // S7.8
    }
    else
    {
        pRenderData->IECP.PROCAMP.bProcampEnabled = false;
        mhwVeboxIecpParams.ProcAmpParams.bActive = false;
        mhwVeboxIecpParams.ProcAmpParams.bEnabled = false;
    }

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
    MHW_VEBOX_GAMUT_PARAMS &mhwVeboxGamutParams = pRenderData->GetGamutParams();
    pOsInterface                       = m_hwInterface->m_osInterface;
    pRenderData->HDR3DLUT.bHdr3DLut    = true;

    pRenderData->HDR3DLUT.is3DLutTableFilled   = HDR_STAGE::HDR_STAGE_VEBOX_3DLUT_UPDATE == hdrParams->stage ||
                                                    HDR_STAGE::HDR_STAGE_VEBOX_3DLUT_NO_UPDATE == hdrParams->stage;
    pRenderData->HDR3DLUT.uiMaxDisplayLum      = hdrParams->uiMaxDisplayLum;
    pRenderData->HDR3DLUT.uiMaxContentLevelLum = hdrParams->uiMaxContentLevelLum;
    pRenderData->HDR3DLUT.hdrMode              = hdrParams->hdrMode;

    VP_RENDER_CHK_STATUS_RETURN(ValidateHDR3DLutParameters(pRenderData->HDR3DLUT.is3DLutTableFilled));

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

    //Report
    if (m_hwInterface->m_reporting)
    {
        m_hwInterface->m_reporting->GetFeatures().hdrMode = (pRenderData->HDR3DLUT.hdrMode == VPHAL_HDR_MODE_H2H) ? VPHAL_HDR_MODE_H2H_VEBOX_3DLUT : VPHAL_HDR_MODE_TONE_MAPPING_VEBOX_3DLUT;
    }

    return MOS_STATUS_SUCCESS;
}

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

MOS_STATUS VpVeboxCmdPacket::SetVeboxSurfaceControlBits(
    MHW_VEBOX_SURFACE_CNTL_PARAMS       *pVeboxSurfCntlParams,
    uint32_t                            *pSurfCtrlBits)
{
    VP_RENDER_CHK_NULL_RETURN(m_veboxItf);

    VP_RENDER_CHK_STATUS_RETURN(m_veboxItf->SetVeboxSurfaceControlBits(
        pVeboxSurfCntlParams,
        pSurfCtrlBits));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpVeboxCmdPacket::SetupDiIecpState(
    bool                        bDiScdEnable,
    mhw::vebox::VEB_DI_IECP_PAR &veboxDiIecpCmdParams)
{
    VP_FUNC_CALL();

    uint32_t                            dwWidth                 = 0;
    uint32_t                            dwHeight                = 0;
    MHW_VEBOX_SURFACE_PARAMS            MhwVeboxSurfaceParam    = {};
    MHW_VEBOX_SURFACE_CNTL_PARAMS       VeboxSurfCntlParams     = {};
    PVP_SURFACE                         pSurface                = nullptr;
    MOS_STATUS                          eStatus                 = MOS_STATUS_SUCCESS;

    VP_RENDER_CHK_NULL_RETURN(m_hwInterface);

    VP_RENDER_CHK_NULL_RETURN(m_veboxPacketSurface.pCurrInput);
    VP_RENDER_CHK_NULL_RETURN(m_veboxPacketSurface.pCurrInput->osSurface);
    VP_RENDER_CHK_NULL_RETURN(m_veboxPacketSurface.pStatisticsOutput);
    VP_RENDER_CHK_NULL_RETURN(m_surfMemCacheCtl);
    VP_RENDER_CHK_NULL_RETURN(m_veboxItf);

    MOS_ZeroMemory(&veboxDiIecpCmdParams, sizeof(veboxDiIecpCmdParams));

    // Align dwEndingX with surface state
    VP_RENDER_CHK_STATUS_RETURN(InitVeboxSurfaceParams(
                                    m_veboxPacketSurface.pCurrInput, &MhwVeboxSurfaceParam));

    VP_RENDER_CHK_STATUS_RETURN(m_veboxItf->VeboxAdjustBoundary(
                                    &MhwVeboxSurfaceParam,
                                    &dwWidth,
                                    &dwHeight,
                                    m_PacketCaps.bDI));

    veboxDiIecpCmdParams.dwStartingX = 0;
    veboxDiIecpCmdParams.dwEndingX   = dwWidth-1;

    veboxDiIecpCmdParams.pOsResCurrInput         = &m_veboxPacketSurface.pCurrInput->osSurface->OsResource;
    veboxDiIecpCmdParams.dwCurrInputSurfOffset   = m_veboxPacketSurface.pCurrInput->osSurface->dwOffset;
    veboxDiIecpCmdParams.CurrInputSurfCtrl.Value = m_surfMemCacheCtl->DnDi.CurrentInputSurfMemObjCtl;

    // Update control bits for current surface
    if (m_mmc->IsMmcEnabled())
    {
        pSurface = m_veboxPacketSurface.pCurrInput;
        MOS_ZeroMemory(&VeboxSurfCntlParams, sizeof(VeboxSurfCntlParams));
        VeboxSurfCntlParams.bIsCompressed       = pSurface->osSurface->bIsCompressed;
        VeboxSurfCntlParams.CompressionMode     = pSurface->osSurface->CompressionMode;
        VPHAL_RENDER_CHK_STATUS(SetVeboxSurfaceControlBits(
            &VeboxSurfCntlParams,
            (uint32_t *)&(veboxDiIecpCmdParams.CurrInputSurfCtrl.Value)));
    }

    // Reference surface
    if (m_veboxPacketSurface.pPrevInput)
    {
        VP_RENDER_CHK_NULL_RETURN(m_veboxPacketSurface.pPrevInput->osSurface);
        veboxDiIecpCmdParams.pOsResPrevInput          = &m_veboxPacketSurface.pPrevInput->osSurface->OsResource;
        veboxDiIecpCmdParams.dwPrevInputSurfOffset    = m_veboxPacketSurface.pPrevInput->osSurface->dwOffset;
        veboxDiIecpCmdParams.PrevInputSurfCtrl.Value  = m_surfMemCacheCtl->DnDi.PreviousInputSurfMemObjCtl;

        // Update control bits for PastSurface surface
        if (m_mmc->IsMmcEnabled())
        {
            pSurface = m_veboxPacketSurface.pPrevInput;
            MOS_ZeroMemory(&VeboxSurfCntlParams, sizeof(VeboxSurfCntlParams));
            VeboxSurfCntlParams.bIsCompressed       = pSurface->osSurface->bIsCompressed;
            VeboxSurfCntlParams.CompressionMode     = pSurface->osSurface->CompressionMode;
            VPHAL_RENDER_CHK_STATUS(SetVeboxSurfaceControlBits(
                &VeboxSurfCntlParams,
                (uint32_t *)&(veboxDiIecpCmdParams.PrevInputSurfCtrl.Value)));
        }
    }

    // VEBOX final output surface
    if (m_veboxPacketSurface.pCurrOutput)
    {
        VP_RENDER_CHK_NULL_RETURN(m_veboxPacketSurface.pCurrOutput->osSurface);
        veboxDiIecpCmdParams.pOsResCurrOutput         = &m_veboxPacketSurface.pCurrOutput->osSurface->OsResource;
        veboxDiIecpCmdParams.dwCurrOutputSurfOffset   = m_veboxPacketSurface.pCurrOutput->osSurface->dwOffset;
        veboxDiIecpCmdParams.CurrOutputSurfCtrl.Value = m_surfMemCacheCtl->DnDi.CurrentOutputSurfMemObjCtl;

        // Update control bits for Current Output Surf
        if (m_mmc->IsMmcEnabled())
        {
            pSurface = m_veboxPacketSurface.pCurrOutput;
            MOS_ZeroMemory(&VeboxSurfCntlParams, sizeof(VeboxSurfCntlParams));
            VeboxSurfCntlParams.bIsCompressed   = pSurface->osSurface->bIsCompressed;
            VeboxSurfCntlParams.CompressionMode = pSurface->osSurface->CompressionMode;
            VPHAL_RENDER_CHK_STATUS(SetVeboxSurfaceControlBits(
                &VeboxSurfCntlParams,
                (uint32_t *)&(veboxDiIecpCmdParams.CurrOutputSurfCtrl.Value)));
        }
    }

    if (m_veboxPacketSurface.pPrevOutput)
    {
        VP_RENDER_CHK_NULL_RETURN(m_veboxPacketSurface.pPrevOutput->osSurface);
        veboxDiIecpCmdParams.pOsResPrevOutput         = &m_veboxPacketSurface.pPrevOutput->osSurface->OsResource;
        veboxDiIecpCmdParams.PrevOutputSurfCtrl.Value = m_surfMemCacheCtl->DnDi.CurrentOutputSurfMemObjCtl;

        // Update control bits for PrevOutput surface
        if (m_mmc->IsMmcEnabled())
        {
            pSurface = m_veboxPacketSurface.pPrevOutput;
            MOS_ZeroMemory(&VeboxSurfCntlParams, sizeof(VeboxSurfCntlParams));
            VeboxSurfCntlParams.bIsCompressed   = pSurface->osSurface->bIsCompressed;
            VeboxSurfCntlParams.CompressionMode = pSurface->osSurface->CompressionMode;
            VPHAL_RENDER_CHK_STATUS(SetVeboxSurfaceControlBits(
                &VeboxSurfCntlParams,
                (uint32_t *)&(veboxDiIecpCmdParams.PrevOutputSurfCtrl.Value)));
        }
    }

    // DN intermediate output surface
    if (m_veboxPacketSurface.pDenoisedCurrOutput)
    {
        VP_RENDER_CHK_NULL_RETURN(m_veboxPacketSurface.pDenoisedCurrOutput->osSurface);
        veboxDiIecpCmdParams.pOsResDenoisedCurrOutput         = &m_veboxPacketSurface.pDenoisedCurrOutput->osSurface->OsResource;
        veboxDiIecpCmdParams.DenoisedCurrOutputSurfCtrl.Value = m_surfMemCacheCtl->DnDi.DnOutSurfMemObjCtl;

        // Update control bits for DenoisedCurrOutputSurf surface
        if (m_mmc->IsMmcEnabled())
        {
            pSurface = m_veboxPacketSurface.pDenoisedCurrOutput;
            MOS_ZeroMemory(&VeboxSurfCntlParams, sizeof(VeboxSurfCntlParams));
            VeboxSurfCntlParams.bIsCompressed   = pSurface->osSurface->bIsCompressed;
            VeboxSurfCntlParams.CompressionMode = pSurface->osSurface->CompressionMode;
            VPHAL_RENDER_CHK_STATUS(SetVeboxSurfaceControlBits(
                &VeboxSurfCntlParams,
                (uint32_t *)&(veboxDiIecpCmdParams.DenoisedCurrOutputSurfCtrl.Value)));
        }
    }

    // STMM surface
    if (m_veboxPacketSurface.pSTMMInput && m_veboxPacketSurface.pSTMMOutput)
    {
        VP_RENDER_CHK_NULL_RETURN(m_veboxPacketSurface.pSTMMInput->osSurface);
        VP_RENDER_CHK_NULL_RETURN(m_veboxPacketSurface.pSTMMOutput->osSurface);

        // STMM in
        veboxDiIecpCmdParams.pOsResStmmInput         = &m_veboxPacketSurface.pSTMMInput->osSurface->OsResource;
        veboxDiIecpCmdParams.StmmInputSurfCtrl.Value = m_surfMemCacheCtl->DnDi.STMMInputSurfMemObjCtl;

        // Update control bits for stmm input surface
        if (m_mmc->IsMmcEnabled())
        {
            pSurface = m_veboxPacketSurface.pSTMMInput;
            MOS_ZeroMemory(&VeboxSurfCntlParams, sizeof(VeboxSurfCntlParams));
            VeboxSurfCntlParams.bIsCompressed   = pSurface->osSurface->bIsCompressed;
            VeboxSurfCntlParams.CompressionMode = pSurface->osSurface->CompressionMode;
            VPHAL_RENDER_CHK_STATUS(SetVeboxSurfaceControlBits(
                &VeboxSurfCntlParams,
                (uint32_t *)&(veboxDiIecpCmdParams.StmmInputSurfCtrl.Value)));
        }

        // STMM out
        veboxDiIecpCmdParams.pOsResStmmOutput         = &m_veboxPacketSurface.pSTMMOutput->osSurface->OsResource;
        veboxDiIecpCmdParams.StmmOutputSurfCtrl.Value = m_surfMemCacheCtl->DnDi.STMMOutputSurfMemObjCtl;

        // Update control bits for stmm output surface
        if (m_mmc->IsMmcEnabled())
        {
            pSurface = m_veboxPacketSurface.pSTMMOutput;
            MOS_ZeroMemory(&VeboxSurfCntlParams, sizeof(VeboxSurfCntlParams));
            VeboxSurfCntlParams.bIsCompressed   = pSurface->osSurface->bIsCompressed;
            VeboxSurfCntlParams.CompressionMode = pSurface->osSurface->CompressionMode;
            VPHAL_RENDER_CHK_STATUS(SetVeboxSurfaceControlBits(
                &VeboxSurfCntlParams,
                (uint32_t *)&(veboxDiIecpCmdParams.StmmOutputSurfCtrl.Value)));
        }
    }

    veboxDiIecpCmdParams.pOsResStatisticsOutput         = &m_veboxPacketSurface.pStatisticsOutput->osSurface->OsResource;
    veboxDiIecpCmdParams.StatisticsOutputSurfCtrl.Value = m_surfMemCacheCtl->DnDi.StatisticsOutputSurfMemObjCtl;

finish:
    return eStatus;
}

bool VpVeboxCmdPacket::UseKernelResource()
{
    VP_FUNC_CALL();

    return false;
}

MOS_STATUS VpVeboxCmdPacket::InitVeboxSurfaceParams(
    PVP_SURFACE                     pVpHalVeboxSurface,
    PMHW_VEBOX_SURFACE_PARAMS       pMhwVeboxSurface)
{
    VP_FUNC_CALL();

    MOS_STATUS                   eStatus = MOS_STATUS_SUCCESS;

    VP_RENDER_CHK_NULL_RETURN(pVpHalVeboxSurface);
    VP_RENDER_CHK_NULL_RETURN(pVpHalVeboxSurface->osSurface);
    VP_RENDER_CHK_NULL_RETURN(pMhwVeboxSurface);

    MOS_ZeroMemory(pMhwVeboxSurface, sizeof(*pMhwVeboxSurface));
    pMhwVeboxSurface->bActive                = true;
    pMhwVeboxSurface->Format                 = pVpHalVeboxSurface->osSurface->Format;
    pMhwVeboxSurface->dwWidth                = pVpHalVeboxSurface->osSurface->dwWidth;
    pMhwVeboxSurface->dwHeight               = pVpHalVeboxSurface->osSurface->dwHeight;
    pMhwVeboxSurface->dwPitch                = pVpHalVeboxSurface->osSurface->dwPitch;
    pMhwVeboxSurface->dwBitDepth             = pVpHalVeboxSurface->osSurface->dwDepth;
    pMhwVeboxSurface->TileType               = pVpHalVeboxSurface->osSurface->TileType;
    pMhwVeboxSurface->TileModeGMM            = pVpHalVeboxSurface->osSurface->TileModeGMM;
    pMhwVeboxSurface->bGMMTileEnabled        = pVpHalVeboxSurface->osSurface->bGMMTileEnabled;
    if (pVpHalVeboxSurface->rcMaxSrc.top == pVpHalVeboxSurface->rcMaxSrc.bottom ||
        pVpHalVeboxSurface->rcMaxSrc.left == pVpHalVeboxSurface->rcMaxSrc.right)
    {
        // If rcMaxSrc is invalid, just use rcSrc.
        pMhwVeboxSurface->rcMaxSrc           = pVpHalVeboxSurface->rcSrc;
    }
    else
    {
        pMhwVeboxSurface->rcMaxSrc           = pVpHalVeboxSurface->rcMaxSrc;
    }
    pMhwVeboxSurface->rcSrc                  = pVpHalVeboxSurface->rcSrc;
    pMhwVeboxSurface->bVEBOXCroppingUsed     = pVpHalVeboxSurface->bVEBOXCroppingUsed;
    pMhwVeboxSurface->pOsResource            = &pVpHalVeboxSurface->osSurface->OsResource;
    pMhwVeboxSurface->bIsCompressed          = pVpHalVeboxSurface->osSurface->bIsCompressed;

    if (pVpHalVeboxSurface->osSurface->dwPitch > 0)
    {
        pMhwVeboxSurface->dwUYoffset = ((pVpHalVeboxSurface->osSurface->UPlaneOffset.iSurfaceOffset - pVpHalVeboxSurface->osSurface->YPlaneOffset.iSurfaceOffset) / pVpHalVeboxSurface->osSurface->dwPitch)
                                       + pVpHalVeboxSurface->osSurface->UPlaneOffset.iYOffset;
    }
    return eStatus;
}

MOS_STATUS VpVeboxCmdPacket::SendVeboxCmd(MOS_COMMAND_BUFFER* commandBuffer)
{
    VP_FUNC_CALL();

    MOS_STATUS                              eStatus;
    int32_t                                 iRemaining;
    VPHAL_VEBOX_SURFACE_STATE_CMD_PARAMS    VeboxSurfaceStateCmdParams;
    MHW_VEBOX_SURFACE_STATE_CMD_PARAMS      MhwVeboxSurfaceStateCmdParams;
    MHW_MI_FLUSH_DW_PARAMS                  FlushDwParams;
    RENDERHAL_GENERIC_PROLOG_PARAMS         GenericPrologParams;

    eStatus                 = MOS_STATUS_SUCCESS;
    iRemaining              = 0;

    VP_RENDER_CHK_NULL_RETURN(commandBuffer);

    eStatus = PrepareVeboxCmd(
                  commandBuffer,
                  GenericPrologParams,
                  iRemaining);

    if (eStatus != MOS_STATUS_SUCCESS)
    {
        CmdErrorHanlde(commandBuffer, iRemaining);
    }
    else
    {
        eStatus = RenderVeboxCmd(
                      commandBuffer,
                      VeboxSurfaceStateCmdParams,
                      MhwVeboxSurfaceStateCmdParams,
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
    VP_FUNC_CALL();

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
    int32_t&                                 iRemaining)
{
    VP_FUNC_CALL();

    MOS_STATUS                              eStatus      = MOS_STATUS_SUCCESS;
    PMOS_INTERFACE                          pOsInterface = m_hwInterface->m_osInterface;
    VpVeboxRenderData                       *pRenderData  = GetLastExecRenderData();
    PMOS_RESOURCE                           gpuStatusBuffer = nullptr;

    VP_RENDER_CHK_NULL_RETURN(CmdBuffer);
    VP_RENDER_CHK_NULL_RETURN(pOsInterface);
    VP_RENDER_CHK_NULL_RETURN(m_currentSurface);
    VP_RENDER_CHK_NULL_RETURN(m_currentSurface->osSurface);

    // Set initial state
    iRemaining = CmdBuffer->iRemaining;

    //---------------------------
    // Set Performance Tags
    //---------------------------
    VP_RENDER_CHK_STATUS_RETURN(VeboxSetPerfTag());
    pOsInterface->pfnResetPerfBufferID(pOsInterface);
    pOsInterface->pfnSetPerfTag(pOsInterface, pRenderData->PerfTag);

    MOS_ZeroMemory(&GenericPrologParams, sizeof(GenericPrologParams));

    VP_RENDER_CHK_STATUS_RETURN(SetMediaFrameTracking(GenericPrologParams));

    return eStatus;
}

MOS_STATUS VpVeboxCmdPacket::SetVeboxProCmd(
    MOS_COMMAND_BUFFER*   CmdBuffer)
{
    VP_RENDER_CHK_NULL_RETURN(m_veboxItf);

    VP_RENDER_CHK_STATUS_RETURN(m_veboxItf->setVeboxPrologCmd(m_miItf, CmdBuffer));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpVeboxCmdPacket::SetVeboxIndex(
    uint32_t                            dwVeboxIndex,
    uint32_t                            dwVeboxCount,
    uint32_t                            dwUsingSFC)
{
    VP_RENDER_CHK_NULL_RETURN(m_veboxItf);

    m_veboxItf->SetVeboxIndex(dwVeboxIndex, dwVeboxCount, dwUsingSFC);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpVeboxCmdPacket::SetVeboxState(
    PMOS_COMMAND_BUFFER         pCmdBufferInUse)

{
    VP_RENDER_CHK_NULL_RETURN(pCmdBufferInUse);
    VP_RENDER_CHK_NULL_RETURN(m_veboxItf);

    m_veboxItf->MHW_ADDCMD_F(VEBOX_STATE)(pCmdBufferInUse, nullptr);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpVeboxCmdPacket::SetVeboxSurfaces(
    PMOS_COMMAND_BUFFER                 pCmdBufferInUse,
    PMHW_VEBOX_SURFACE_STATE_CMD_PARAMS pMhwVeboxSurfaceStateCmdParams)

{
    VP_RENDER_CHK_NULL_RETURN(pCmdBufferInUse);
    VP_RENDER_CHK_NULL_RETURN(m_veboxItf);

    m_veboxItf->AddVeboxSurfaces(pCmdBufferInUse, pMhwVeboxSurfaceStateCmdParams);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpVeboxCmdPacket::SetVeboxDiIecp(
    PMOS_COMMAND_BUFFER                pCmdBufferInUse)

{
    VP_RENDER_CHK_NULL_RETURN(pCmdBufferInUse);
    VP_RENDER_CHK_NULL_RETURN(m_veboxItf);

    m_veboxItf->MHW_ADDCMD_F(VEB_DI_IECP)(pCmdBufferInUse, nullptr);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpVeboxCmdPacket::RenderVeboxCmd(
    MOS_COMMAND_BUFFER                      *CmdBuffer,
    VPHAL_VEBOX_SURFACE_STATE_CMD_PARAMS    &VeboxSurfaceStateCmdParams,
    MHW_VEBOX_SURFACE_STATE_CMD_PARAMS      &MhwVeboxSurfaceStateCmdParams,
    MHW_MI_FLUSH_DW_PARAMS                  &FlushDwParams,
    PRENDERHAL_GENERIC_PROLOG_PARAMS        pGenericPrologParams)
{
    VP_FUNC_CALL();

    MOS_STATUS            eStatus = MOS_STATUS_SUCCESS;
    PRENDERHAL_INTERFACE  pRenderHal;
    PMOS_INTERFACE        pOsInterface;
    PMHW_MI_INTERFACE     pMhwMiInterface;
    bool                  bDiVarianceEnable;
    const MHW_VEBOX_HEAP *pVeboxHeap     = nullptr;
    VpVeboxRenderData *   pRenderData    = GetLastExecRenderData();
    MediaPerfProfiler *   pPerfProfiler  = nullptr;
    MOS_CONTEXT *         pOsContext     = nullptr;
    PMHW_MI_MMIOREGISTERS pMmioRegisters = nullptr;
    MOS_COMMAND_BUFFER    CmdBufferInUse;
    PMOS_COMMAND_BUFFER   pCmdBufferInUse = nullptr;
    uint32_t              curPipe         = 0;
    uint8_t               inputPipe       = 0;
    uint32_t              numPipe         = 1;
    bool                  bMultipipe      = false;

    VP_RENDER_CHK_NULL_RETURN(m_hwInterface->m_renderHal);
    VP_RENDER_CHK_NULL_RETURN(m_hwInterface->m_mhwMiInterface);
    VP_RENDER_CHK_NULL_RETURN(m_miItf);
    VP_RENDER_CHK_NULL_RETURN(m_hwInterface->m_osInterface);
    VP_RENDER_CHK_NULL_RETURN(m_hwInterface->m_osInterface->pOsContext);
    VP_RENDER_CHK_NULL_RETURN(m_miItf->GetMmioRegisters());
    VP_RENDER_CHK_NULL_RETURN(pRenderData);
    VP_RENDER_CHK_NULL_RETURN(CmdBuffer);
    VP_RENDER_CHK_NULL_RETURN(m_veboxItf);

    pRenderHal      = m_hwInterface->m_renderHal;
    pMhwMiInterface = m_hwInterface->m_mhwMiInterface;
    pOsInterface    = m_hwInterface->m_osInterface;
    pPerfProfiler   = pRenderHal->pPerfProfiler;
    pOsContext      = m_hwInterface->m_osInterface->pOsContext;
    pMmioRegisters  = m_miItf->GetMmioRegisters();
    pCmdBufferInUse = CmdBuffer;

    auto scalability = GetMediaScalability();
    VP_RENDER_CHK_NULL_RETURN(m_veboxItf);
    mhw::vebox::VEBOX_STATE_PAR& veboxStateCmdParams = m_veboxItf->MHW_GETPAR_F(VEBOX_STATE)();
    mhw::vebox::VEB_DI_IECP_PAR& veboxDiIecpCmdParams = m_veboxItf->MHW_GETPAR_F(VEB_DI_IECP)();

    VP_RENDER_CHK_STATUS_RETURN(m_veboxItf->GetVeboxHeapInfo(&pVeboxHeap));
    VP_RENDER_CHK_NULL_RETURN(pVeboxHeap);

#ifdef _MMC_SUPPORTED

    VP_RENDER_CHK_STATUS_RETURN(SetVeboxProCmd(CmdBuffer));

#endif

    // Initialize the scalability
    curPipe    = scalability->GetCurrentPipe();
    inputPipe  = (uint8_t)curPipe;
    numPipe    = scalability->GetPipeNumber();
    bMultipipe = (numPipe > 1) ? true : false;

    VP_RENDER_CHK_STATUS_RETURN(SetVeboxIndex(0, numPipe, m_IsSfcUsed));

    bDiVarianceEnable = m_PacketCaps.bDI;

    SetupSurfaceStates(
        &VeboxSurfaceStateCmdParams);

    SetupVeboxState(veboxStateCmdParams);

    VP_RENDER_CHK_STATUS_RETURN(SetupDiIecpState(
        bDiVarianceEnable,
        veboxDiIecpCmdParams));

    VP_RENDER_CHK_STATUS_RETURN(IsCmdParamsValid(
        veboxStateCmdParams,
        veboxDiIecpCmdParams,
        VeboxSurfaceStateCmdParams));

    // Initialize command buffer and insert prolog
    VP_RENDER_CHK_STATUS_RETURN(InitCmdBufferWithVeParams(pRenderHal, *CmdBuffer, pGenericPrologParams));

    //---------------------------------
    // Initialize Vebox Surface State Params
    //---------------------------------
    VP_RENDER_CHK_STATUS_RETURN(InitVeboxSurfaceStateCmdParams(
        &VeboxSurfaceStateCmdParams, &MhwVeboxSurfaceStateCmdParams));

    for (curPipe = 0; curPipe < numPipe; curPipe++)
    {
        if (bMultipipe)
        {
            // initialize the command buffer struct
            MOS_ZeroMemory(&CmdBufferInUse, sizeof(MOS_COMMAND_BUFFER));
            bool frameTrackingRequested = m_PacketCaps.lastSubmission && (numPipe - 1 == curPipe);
            scalability->SetCurrentPipeIndex((uint8_t)curPipe);
            scalability->GetCmdBuffer(&CmdBufferInUse, frameTrackingRequested);
            pCmdBufferInUse = &CmdBufferInUse;
        }
        else
        {
            pCmdBufferInUse = CmdBuffer;
        }

        VP_RENDER_CHK_STATUS_RETURN(SetVeboxIndex(curPipe, numPipe, m_IsSfcUsed));

        HalOcaInterfaceNext::On1stLevelBBStart(*pCmdBufferInUse, *pOsContext, pOsInterface->CurrentGpuContextHandle, m_miItf, *pMmioRegisters);

        char ocaMsg[] = "VP APG Vebox Packet";
        HalOcaInterfaceNext::TraceMessage(*pCmdBufferInUse, *pOsContext, ocaMsg, sizeof(ocaMsg));

        HalOcaInterfaceNext::TraceOcaSkuValue(*pCmdBufferInUse, *pOsInterface);

        // Add vphal param to log.
        HalOcaInterfaceNext::DumpVphalParam(*pCmdBufferInUse, *pOsContext, pRenderHal->pVphalOcaDumper);

        VP_RENDER_CHK_STATUS_RETURN(pPerfProfiler->AddPerfCollectStartCmd((void *)pRenderHal, pOsInterface, pRenderHal->pMhwMiInterface, pCmdBufferInUse));

        VP_RENDER_CHK_STATUS_RETURN(NullHW::StartPredicateNext(m_miItf, pCmdBufferInUse));

        // Add compressible info of input/output surface to log
        if (this->m_currentSurface && VeboxSurfaceStateCmdParams.pSurfOutput)
        {
            std::string info   = "in_comps = " + std::to_string(int(this->m_currentSurface->osSurface->bCompressible)) + ", out_comps = " + std::to_string(int(VeboxSurfaceStateCmdParams.pSurfOutput->osSurface->bCompressible));
            const char *ocaLog = info.c_str();
            HalOcaInterfaceNext::TraceMessage(*pCmdBufferInUse, *pOsContext, ocaLog, info.size());
        }

        if (bMultipipe)
        {
            // Insert prolog with VE params
#ifdef _MMC_SUPPORTED

            VP_RENDER_CHK_STATUS_RETURN(SetVeboxProCmd(pCmdBufferInUse));

#endif

            MHW_GENERIC_PROLOG_PARAMS genericPrologParams;
            MOS_ZeroMemory(&genericPrologParams, sizeof(genericPrologParams));
            genericPrologParams.pOsInterface  = pRenderHal->pOsInterface;
            genericPrologParams.pvMiInterface = pRenderHal->pMhwMiInterface;
            genericPrologParams.bMmcEnabled   = pGenericPrologParams ? pGenericPrologParams->bMmcEnabled : false;
            VP_RENDER_CHK_STATUS_RETURN(Mhw_SendGenericPrologCmdNext(pCmdBufferInUse, &genericPrologParams, m_miItf));

            VP_RENDER_CHK_STATUS_RETURN(scalability->SyncPipe(syncAllPipes, 0, pCmdBufferInUse));

            // Enable Watchdog Timer
            VP_RENDER_CHK_STATUS_RETURN(m_miItf->AddWatchdogTimerStartCmd(pCmdBufferInUse));

#if (_DEBUG || _RELEASE_INTERNAL)
            // Add noop for simu no output issue
            if (curPipe > 0)
            {
                SETPAR_AND_ADDCMD(MI_NOOP, m_miItf, pCmdBufferInUse);
                SETPAR_AND_ADDCMD(MI_NOOP, m_miItf, pCmdBufferInUse);
                SETPAR_AND_ADDCMD(MI_NOOP, m_miItf, pCmdBufferInUse);
                SETPAR_AND_ADDCMD(MI_NOOP, m_miItf, pCmdBufferInUse);
                if (m_IsSfcUsed)
                {
                    SETPAR_AND_ADDCMD(MI_NOOP, m_miItf, pCmdBufferInUse);
                    SETPAR_AND_ADDCMD(MI_NOOP, m_miItf, pCmdBufferInUse);
                    SETPAR_AND_ADDCMD(MI_NOOP, m_miItf, pCmdBufferInUse);
                    SETPAR_AND_ADDCMD(MI_NOOP, m_miItf, pCmdBufferInUse);
                }
            }
#endif
        }

        //---------------------------------
        // Send CMD: Vebox_State
        //---------------------------------
        VP_RENDER_CHK_STATUS_RETURN(SetVeboxState(
            pCmdBufferInUse));

        //---------------------------------
        // Send CMD: Vebox_Surface_State
        //---------------------------------
        VP_RENDER_CHK_STATUS_RETURN(SetVeboxSurfaces(
            pCmdBufferInUse,
            &MhwVeboxSurfaceStateCmdParams));

        //---------------------------------
        // Send CMD: SFC pipe commands
        //---------------------------------
        if (m_IsSfcUsed)
        {
            VP_RENDER_CHK_NULL_RETURN(m_sfcRender);

            VP_RENDER_CHK_STATUS_RETURN(m_sfcRender->SetSfcPipe(curPipe, numPipe));

            VP_RENDER_CHK_STATUS_RETURN(m_sfcRender->SetupSfcState(m_renderTarget));

            VP_RENDER_CHK_STATUS_RETURN(m_sfcRender->SendSfcCmd(
                (pRenderData->DI.bDeinterlace || pRenderData->DN.bDnEnabled),
                pCmdBufferInUse));
        }

        HalOcaInterfaceNext::OnDispatch(*pCmdBufferInUse, *pOsContext, m_miItf, *pMmioRegisters);

        //---------------------------------
        // Send CMD: Vebox_DI_IECP
        //---------------------------------
        VP_RENDER_CHK_STATUS_RETURN(SetVeboxDiIecp(
            pCmdBufferInUse));

        VP_RENDER_CHK_NULL_RETURN(pOsInterface);
        VP_RENDER_CHK_NULL_RETURN(pOsInterface->pfnGetSkuTable);
        auto *skuTable = pOsInterface->pfnGetSkuTable(pOsInterface);
        if (skuTable && MEDIA_IS_SKU(skuTable, FtrEnablePPCFlush))
        {
            // Add PPC fulsh
            auto &miFlushDwParams           = m_miItf->MHW_GETPAR_F(MI_FLUSH_DW)();
            miFlushDwParams                 = {};
            miFlushDwParams.bEnablePPCFlush = true;
            VP_RENDER_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_FLUSH_DW)(pCmdBufferInUse));
        }

        if (bMultipipe)
        {
            // MI FlushDw, for vebox output green block issue
            auto &params             = m_miItf->MHW_GETPAR_F(MI_FLUSH_DW)();
            params                   = {};
            m_miItf->MHW_ADDCMD_F(MI_FLUSH_DW)(pCmdBufferInUse);

            VP_RENDER_CHK_STATUS_RETURN(scalability->SyncPipe(syncAllPipes, 0, pCmdBufferInUse));
        }

        //---------------------------------
        // Write GPU Status Tag for Tag based synchronization
        //---------------------------------
        if (!pOsInterface->bEnableKmdMediaFrameTracking)
        {
            VP_RENDER_CHK_STATUS_RETURN(SendVecsStatusTag(
                pOsInterface,
                pCmdBufferInUse));
        }

        //---------------------------------
        // Write Sync tag for Vebox Heap Synchronization
        // If KMD frame tracking is on, the synchronization of Vebox Heap will use Status tag which
        // is updated using KMD frame tracking.
        //---------------------------------
        if (!pOsInterface->bEnableKmdMediaFrameTracking)
        {
            auto &params             = m_miItf->MHW_GETPAR_F(MI_FLUSH_DW)();
            params                   = {};
            params.pOsResource       = (PMOS_RESOURCE)&pVeboxHeap->DriverResource;
            params.dwResourceOffset  = pVeboxHeap->uiOffsetSync;
            params.dwDataDW1         = pVeboxHeap->dwNextTag;
            m_miItf->MHW_ADDCMD_F(MI_FLUSH_DW)(pCmdBufferInUse);
        }

        if (bMultipipe)
        {
            // Disable Watchdog Timer
            VP_RENDER_CHK_STATUS_RETURN(m_miItf->AddWatchdogTimerStopCmd(pCmdBufferInUse));
        }

        VP_RENDER_CHK_STATUS_RETURN(NullHW::StopPredicateNext(m_miItf, pCmdBufferInUse));

        VP_RENDER_CHK_STATUS_RETURN(pPerfProfiler->AddPerfCollectEndCmd((void *)pRenderHal, pOsInterface, pRenderHal->pMhwMiInterface, pCmdBufferInUse));

        HalOcaInterfaceNext::On1stLevelBBEnd(*pCmdBufferInUse, *pOsInterface);

        if (pOsInterface->bNoParsingAssistanceInKmd)
        {
            m_miItf->AddMiBatchBufferEnd(pCmdBufferInUse, nullptr);
        }
        else if (RndrCommonIsMiBBEndNeeded(pOsInterface))
        {
            // Add Batch Buffer end command (HW/OS dependent)
            m_miItf->AddMiBatchBufferEnd(pCmdBufferInUse, nullptr);
        }

        if (bMultipipe)
        {
            scalability->ReturnCmdBuffer(pCmdBufferInUse);
        }
    }

    if (bMultipipe)
    {
        scalability->SetCurrentPipeIndex(inputPipe);
        ReportUserSetting(m_userSettingPtr, __MEDIA_USER_FEATURE_VALUE_ENABLE_VEBOX_SCALABILITY_MODE, true, MediaUserSetting::Group::Sequence);
    }
    else
    {
        ReportUserSetting(m_userSettingPtr, __MEDIA_USER_FEATURE_VALUE_ENABLE_VEBOX_SCALABILITY_MODE, false, MediaUserSetting::Group::Sequence);
    }

    MT_LOG2(MT_VP_HAL_RENDER_VE, MT_NORMAL, MT_VP_MHW_VE_SCALABILITY_EN, bMultipipe, MT_VP_MHW_VE_SCALABILITY_USE_SFC, m_IsSfcUsed);

    return eStatus;
}

MOS_STATUS VpVeboxCmdPacket::InitVeboxSurfaceStateCmdParams(
    PVPHAL_VEBOX_SURFACE_STATE_CMD_PARAMS    pVpHalVeboxSurfaceStateCmdParams,
    PMHW_VEBOX_SURFACE_STATE_CMD_PARAMS      pMhwVeboxSurfaceStateCmdParams)
{
    VP_FUNC_CALL();

    MOS_STATUS                       eStatus = MOS_STATUS_SUCCESS;

    VP_RENDER_CHK_NULL_RETURN(pVpHalVeboxSurfaceStateCmdParams);
    VP_RENDER_CHK_NULL_RETURN(pMhwVeboxSurfaceStateCmdParams);

    MOS_ZeroMemory(pMhwVeboxSurfaceStateCmdParams, sizeof(*pMhwVeboxSurfaceStateCmdParams));

    pMhwVeboxSurfaceStateCmdParams->bDIEnable       = pVpHalVeboxSurfaceStateCmdParams->bDIEnable;
    pMhwVeboxSurfaceStateCmdParams->b3DlutEnable    = pVpHalVeboxSurfaceStateCmdParams->b3DlutEnable;

    if (pVpHalVeboxSurfaceStateCmdParams->pSurfInput)
    {
        VP_RENDER_CHK_NULL_RETURN(pVpHalVeboxSurfaceStateCmdParams->pSurfInput->osSurface);
        VP_RENDER_CHK_STATUS_RETURN(InitVeboxSurfaceParams(
                                      pVpHalVeboxSurfaceStateCmdParams->pSurfInput,
                                      &pMhwVeboxSurfaceStateCmdParams->SurfInput));
        pMhwVeboxSurfaceStateCmdParams->SurfInput.dwYoffset = pVpHalVeboxSurfaceStateCmdParams->pSurfInput->osSurface->YPlaneOffset.iYOffset;
        MT_LOG2(MT_VP_MHW_VE_SURFSTATE_INPUT, MT_NORMAL, MT_SURF_TILE_MODE, pVpHalVeboxSurfaceStateCmdParams->pSurfInput->osSurface->TileModeGMM,
            MT_SURF_MOS_FORMAT, pVpHalVeboxSurfaceStateCmdParams->pSurfInput->osSurface->Format);
    }
    if (pVpHalVeboxSurfaceStateCmdParams->pSurfOutput)
    {
        VP_RENDER_CHK_NULL_RETURN(pVpHalVeboxSurfaceStateCmdParams->pSurfOutput->osSurface);
        pMhwVeboxSurfaceStateCmdParams->bOutputValid = true;
        VP_RENDER_CHK_STATUS_RETURN(InitVeboxSurfaceParams(
                                      pVpHalVeboxSurfaceStateCmdParams->pSurfOutput,
                                      &pMhwVeboxSurfaceStateCmdParams->SurfOutput));
        pMhwVeboxSurfaceStateCmdParams->SurfOutput.dwYoffset = pVpHalVeboxSurfaceStateCmdParams->pSurfOutput->osSurface->YPlaneOffset.iYOffset;
        MT_LOG2(MT_VP_MHW_VE_SURFSTATE_OUT, MT_NORMAL, MT_SURF_TILE_MODE, pVpHalVeboxSurfaceStateCmdParams->pSurfOutput->osSurface->TileModeGMM,
            MT_SURF_MOS_FORMAT, pVpHalVeboxSurfaceStateCmdParams->pSurfOutput->osSurface->Format);
    }
    if (pVpHalVeboxSurfaceStateCmdParams->pSurfSTMM)
    {
        VP_RENDER_CHK_NULL_RETURN(pVpHalVeboxSurfaceStateCmdParams->pSurfSTMM->osSurface);
        VP_RENDER_CHK_STATUS_RETURN(InitVeboxSurfaceParams(
                                      pVpHalVeboxSurfaceStateCmdParams->pSurfSTMM,
                                      &pMhwVeboxSurfaceStateCmdParams->SurfSTMM));
        MT_LOG2(MT_VP_MHW_VE_SURFSTATE_STMM, MT_NORMAL, MT_SURF_TILE_MODE, pVpHalVeboxSurfaceStateCmdParams->pSurfSTMM->osSurface->TileModeGMM,
            MT_SURF_MOS_FORMAT, pVpHalVeboxSurfaceStateCmdParams->pSurfSTMM->osSurface->Format);
    }
    if (pVpHalVeboxSurfaceStateCmdParams->pSurfDNOutput)
    {
        VP_RENDER_CHK_NULL_RETURN(pVpHalVeboxSurfaceStateCmdParams->pSurfDNOutput->osSurface);
        VP_RENDER_CHK_STATUS_RETURN(InitVeboxSurfaceParams(
                                      pVpHalVeboxSurfaceStateCmdParams->pSurfDNOutput,
                                      &pMhwVeboxSurfaceStateCmdParams->SurfDNOutput));
        pMhwVeboxSurfaceStateCmdParams->SurfDNOutput.dwYoffset = pVpHalVeboxSurfaceStateCmdParams->pSurfDNOutput->osSurface->YPlaneOffset.iYOffset;
        MT_LOG2(MT_VP_MHW_VE_SURFSTATE_DNOUT, MT_NORMAL, MT_SURF_TILE_MODE, pVpHalVeboxSurfaceStateCmdParams->pSurfDNOutput->osSurface->TileModeGMM,
            MT_SURF_MOS_FORMAT, pVpHalVeboxSurfaceStateCmdParams->pSurfDNOutput->osSurface->Format);
    }
    if (pVpHalVeboxSurfaceStateCmdParams->pSurfSkinScoreOutput)
    {
        VP_RENDER_CHK_NULL_RETURN(pVpHalVeboxSurfaceStateCmdParams->pSurfSkinScoreOutput->osSurface);
        VP_RENDER_CHK_STATUS_RETURN(InitVeboxSurfaceParams(
                                      pVpHalVeboxSurfaceStateCmdParams->pSurfSkinScoreOutput,
                                      &pMhwVeboxSurfaceStateCmdParams->SurfSkinScoreOutput));
        MT_LOG2(MT_VP_MHW_VE_SURFSTATE_SKINSCORE, MT_NORMAL, MT_SURF_TILE_MODE, pVpHalVeboxSurfaceStateCmdParams->pSurfSkinScoreOutput->osSurface->TileModeGMM,
            MT_SURF_MOS_FORMAT, pVpHalVeboxSurfaceStateCmdParams->pSurfSkinScoreOutput->osSurface->Format);
    }

    if (m_inputDepth)
    {
        pMhwVeboxSurfaceStateCmdParams->SurfInput.dwBitDepth = m_inputDepth;
    }

    return eStatus;
}

MOS_STATUS VpVeboxCmdPacket::SendVecsStatusTag(
    PMOS_INTERFACE                      pOsInterface,
    PMOS_COMMAND_BUFFER                 pCmdBuffer)
{
    VP_FUNC_CALL();

    PMOS_RESOURCE                       gpuStatusBuffer = nullptr;
    MHW_MI_FLUSH_DW_PARAMS              FlushDwParams;
    MOS_STATUS                          eStatus = MOS_STATUS_SUCCESS;

    //------------------------------------
    VP_RENDER_CHK_NULL_RETURN(m_miItf);
    VP_RENDER_CHK_NULL_RETURN(pOsInterface);
    VP_RENDER_CHK_NULL_RETURN(pCmdBuffer);

    // Get GPU Status buffer
    pOsInterface->pfnGetGpuStatusBufferResource(pOsInterface, gpuStatusBuffer);
    VP_RENDER_CHK_NULL_RETURN(gpuStatusBuffer);

    // Register the buffer
    VP_RENDER_CHK_STATUS_RETURN(pOsInterface->pfnRegisterResource(
                                  pOsInterface,
                                  gpuStatusBuffer,
                                  true,
                                  true));

    VP_RENDER_CHK_NULL_RETURN(m_miItf);

    auto &params             = m_miItf->MHW_GETPAR_F(MI_FLUSH_DW)();
    params                   = {};
    params.pOsResource       = gpuStatusBuffer;;
    params.dwResourceOffset  = pOsInterface->pfnGetGpuStatusTagOffset(pOsInterface, MOS_GPU_CONTEXT_VEBOX);
    params.dwDataDW1         = pOsInterface->pfnGetGpuStatusTag(pOsInterface, MOS_GPU_CONTEXT_VEBOX);
    m_miItf->MHW_ADDCMD_F(MI_FLUSH_DW)(pCmdBuffer);

    // Increase buffer tag for next usage
    pOsInterface->pfnIncrementGpuStatusTag(pOsInterface, MOS_GPU_CONTEXT_VEBOX);

    return eStatus;
}

bool VpVeboxCmdPacket::RndrCommonIsMiBBEndNeeded(
    PMOS_INTERFACE           pOsInterface)
{
    VP_FUNC_CALL();

    bool needed = false;

    if (nullptr == pOsInterface)
        return false;

    return needed;
}

MOS_STATUS VpVeboxCmdPacket::InitSfcRender()
{
    VP_FUNC_CALL();

    if (nullptr == m_sfcRender)
    {
        VP_RENDER_CHK_NULL_RETURN(m_hwInterface);
        VP_RENDER_CHK_NULL_RETURN(m_hwInterface->m_vpPlatformInterface);
        VP_RENDER_CHK_STATUS_RETURN(m_hwInterface->m_vpPlatformInterface->CreateSfcRender(
            m_sfcRender,
            *m_hwInterface,
            m_allocator));
        VP_RENDER_CHK_NULL_RETURN(m_sfcRender);
    }
    VP_PUBLIC_CHK_STATUS_RETURN(m_sfcRender->Init());
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpVeboxCmdPacket::DumpVeboxStateHeap()
{
    VP_FUNC_CALL();

    MOS_STATUS    eStatus = MOS_STATUS_SUCCESS;
#if (_DEBUG || _RELEASE_INTERNAL)
    static uint32_t counter = 0;
    VP_SURFACE driverResource = {};
    VP_SURFACE kernelResource = {};
    MOS_SURFACE driverSurface = {};
    MOS_SURFACE kernelSurface = {};

    const MHW_VEBOX_HEAP* pVeboxHeap      = nullptr;
    VP_RENDER_CHK_NULL_RETURN(m_veboxItf);

    VpDebugInterface*     debuginterface  = (VpDebugInterface*)m_hwInterface->m_debugInterface;

    VP_RENDER_CHK_STATUS_RETURN(m_veboxItf->GetVeboxHeapInfo(&pVeboxHeap));

    VP_RENDER_CHK_NULL_RETURN(pVeboxHeap);


    driverResource.osSurface = &driverSurface;
    kernelResource.osSurface = &kernelSurface;

    driverResource.osSurface->OsResource = pVeboxHeap->DriverResource;
    kernelResource.osSurface->OsResource = pVeboxHeap->KernelResource;

    VPHAL_GET_SURFACE_INFO info = {};
    m_allocator->GetSurfaceInfo(&driverResource, info);
    m_allocator->GetSurfaceInfo(&kernelResource, info);


    VP_SURFACE_DUMP(debuginterface,
        &kernelResource,
        counter,
        0,
        VPHAL_DUMP_TYPE_VEBOX_DRIVERHEAP);

    VP_SURFACE_DUMP(debuginterface,
        &kernelResource,
        counter,
        0,
        VPHAL_DUMP_TYPE_VEBOX_KERNELHEAP);

    counter++;
finish:
#endif
    return eStatus;
}

MOS_STATUS VpVeboxCmdPacket::Init()
{
    VP_FUNC_CALL();

    MOS_STATUS    eStatus = MOS_STATUS_SUCCESS;
    VP_RENDER_CHK_NULL_RETURN(m_hwInterface);
    VP_RENDER_CHK_NULL_RETURN(m_hwInterface->m_skuTable);

    VP_RENDER_CHK_STATUS_RETURN(InitSfcRender());

    if (nullptr == m_currentSurface)
    {
        m_currentSurface = m_allocator->AllocateVpSurface();
        VP_CHK_SPACE_NULL_RETURN(m_currentSurface);
    }
    else
    {
        m_currentSurface->Clean();
    }

    if (nullptr == m_previousSurface)
    {
        m_previousSurface = m_allocator->AllocateVpSurface();
        VP_CHK_SPACE_NULL_RETURN(m_previousSurface);
    }
    else
    {
        m_previousSurface->Clean();
    }

    if (nullptr == m_renderTarget)
    {
        m_renderTarget = m_allocator->AllocateVpSurface();
        VP_CHK_SPACE_NULL_RETURN(m_renderTarget);
    }
    else
    {
        m_renderTarget->Clean();
    }

    MOS_ZeroMemory(&m_veboxPacketSurface, sizeof(VEBOX_PACKET_SURFACE_PARAMS));
    m_surfSetting.Clean();

    return eStatus;
}

MOS_STATUS VpVeboxCmdPacket::Prepare()
{
    VP_FUNC_CALL();

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    return eStatus;
}

MOS_STATUS VpVeboxCmdPacket::PrepareState()
{
    VP_FUNC_CALL();

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    if (m_packetResourcesdPrepared)
    {
        VP_RENDER_NORMALMESSAGE("Resource Prepared, skip this time");
        return MOS_STATUS_SUCCESS;
    }

    VP_RENDER_CHK_STATUS_RETURN(SetupIndirectStates());

    VP_RENDER_CHK_STATUS_RETURN(UpdateVeboxStates());

    m_packetResourcesdPrepared = true;

    return eStatus;
}

MOS_STATUS VpVeboxCmdPacket::AdjustBlockStatistics()
{
    VP_FUNC_CALL();

    if (m_surfSetting.dwVeboxPerBlockStatisticsWidth == 0 || m_surfSetting.dwVeboxPerBlockStatisticsHeight == 0)
    {
        VP_RENDER_NORMALMESSAGE("Not need update statistic block height and width");
        return MOS_STATUS_SUCCESS;
    }

    uint32_t                 dwWidth              = 0;
    uint32_t                 dwHeight             = 0;
    MHW_VEBOX_SURFACE_PARAMS mhwVeboxSurfaceParam = {};

    VP_RENDER_CHK_NULL_RETURN(m_veboxItf);
    VP_RENDER_CHK_NULL_RETURN(m_veboxPacketSurface.pCurrInput);

    // Align dwEndingX with surface state
    VP_RENDER_CHK_STATUS_RETURN(InitVeboxSurfaceParams(
        m_veboxPacketSurface.pCurrInput, &mhwVeboxSurfaceParam));

    // Update Statistics Block Height and Weight in surfsetting

    VP_RENDER_CHK_STATUS_RETURN(m_veboxItf->VeboxAdjustBoundary(
        &mhwVeboxSurfaceParam,
        &dwWidth,
        &dwHeight,
        m_PacketCaps.bDI));

    dwWidth  = MOS_ALIGN_CEIL(dwWidth, 64);
    dwHeight = MOS_ROUNDUP_DIVIDE(dwHeight, 4);

    if (dwWidth > m_surfSetting.dwVeboxPerBlockStatisticsWidth || dwHeight > m_surfSetting.dwVeboxPerBlockStatisticsHeight)
    {
        VP_RENDER_ASSERTMESSAGE("Adjust boundary width %d, height %d is larger than origin boundary width %d, height %d, invalid params",
            dwWidth,
            dwHeight,
            m_surfSetting.dwVeboxPerBlockStatisticsWidth,
            m_surfSetting.dwVeboxPerBlockStatisticsHeight);
        return MOS_STATUS_INVALID_PARAMETER;
    }
    else
    {
        VP_RENDER_NORMALMESSAGE("Adjust boundary width %d, height %d. Origin boundary width %d, height %d",
            dwWidth,
            dwHeight,
            m_surfSetting.dwVeboxPerBlockStatisticsWidth,
            m_surfSetting.dwVeboxPerBlockStatisticsHeight);

        m_surfSetting.dwVeboxPerBlockStatisticsHeight = dwHeight;
        m_surfSetting.dwVeboxPerBlockStatisticsWidth  = dwWidth;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpVeboxCmdPacket::PacketInit(
    VP_SURFACE                          *inputSurface,
    VP_SURFACE                          *outputSurface,
    VP_SURFACE                          *previousSurface,
    VP_SURFACE_SETTING                  &surfSetting,
    VP_EXECUTE_CAPS                     packetCaps)
{
    VP_FUNC_CALL();

    VpVeboxRenderData       *pRenderData = GetLastExecRenderData();
    m_packetResourcesdPrepared = false;

    VP_RENDER_CHK_NULL_RETURN(pRenderData);
    VP_RENDER_CHK_NULL_RETURN(inputSurface);
    VP_RENDER_CHK_NULL_RETURN(outputSurface);
    VP_RENDER_CHK_STATUS_RETURN(pRenderData->Init());

    m_PacketCaps      = packetCaps;

    VP_RENDER_CHK_STATUS_RETURN(Init());
    VP_RENDER_CHK_NULL_RETURN(m_allocator);
    VP_RENDER_CHK_NULL_RETURN(m_currentSurface);
    VP_RENDER_CHK_NULL_RETURN(m_renderTarget);
    VP_RENDER_CHK_NULL_RETURN(m_previousSurface);

    VP_RENDER_CHK_STATUS_RETURN(InitSurfMemCacheControl(packetCaps));

    m_IsSfcUsed = packetCaps.bSFC;

    //update VEBOX resource GMM resource usage type
    m_allocator->UpdateResourceUsageType(&inputSurface->osSurface->OsResource, MOS_HW_RESOURCE_USAGE_VP_INPUT_PICTURE_FF);
    m_allocator->UpdateResourceUsageType(&outputSurface->osSurface->OsResource, MOS_HW_RESOURCE_USAGE_VP_OUTPUT_PICTURE_FF);

    // Set current src = current primary input
    VP_PUBLIC_CHK_STATUS_RETURN(m_allocator->CopyVpSurface(*m_renderTarget ,*outputSurface));

    // Init packet surface params.
    m_surfSetting                                   = surfSetting;
    m_veboxPacketSurface.pCurrInput                 = GetSurface(SurfaceTypeVeboxInput);
    m_veboxPacketSurface.pStatisticsOutput          = GetSurface(SurfaceTypeStatistics);
    m_veboxPacketSurface.pCurrOutput                = GetSurface(SurfaceTypeVeboxCurrentOutput);
    m_veboxPacketSurface.pPrevInput                 = GetSurface(SurfaceTypeVeboxPreviousInput);
    m_veboxPacketSurface.pSTMMInput                 = GetSurface(SurfaceTypeSTMMIn);
    m_veboxPacketSurface.pSTMMOutput                = GetSurface(SurfaceTypeSTMMOut);
    m_veboxPacketSurface.pDenoisedCurrOutput        = GetSurface(SurfaceTypeDNOutput);
    m_veboxPacketSurface.pPrevOutput                = GetSurface(SurfaceTypeVeboxPreviousOutput);
    m_veboxPacketSurface.pAlphaOrVignette           = GetSurface(SurfaceTypeAlphaOrVignette);
    m_veboxPacketSurface.pLaceOrAceOrRgbHistogram   = GetSurface(SurfaceTypeLaceAceRGBHistogram);
    m_veboxPacketSurface.pSurfSkinScoreOutput       = GetSurface(SurfaceTypeSkinScore);

    VP_RENDER_CHK_NULL_RETURN(m_veboxPacketSurface.pCurrInput);
    VP_RENDER_CHK_NULL_RETURN(m_veboxPacketSurface.pStatisticsOutput);
    VP_RENDER_CHK_NULL_RETURN(m_veboxPacketSurface.pLaceOrAceOrRgbHistogram);

    m_DNDIFirstFrame    = (!m_PacketCaps.bRefValid && (m_PacketCaps.bDN || m_PacketCaps.bDI));
    m_DIOutputFrames    = MEDIA_VEBOX_DI_OUTPUT_CURRENT;

    auto curInput = m_veboxPacketSurface.pCurrInput;
    auto curOutput = m_veboxPacketSurface.pCurrOutput;
    if (!m_IsSfcUsed &&
        ((uint32_t)curInput->rcSrc.bottom < curInput->osSurface->dwHeight ||
        (uint32_t)curInput->rcSrc.right < curInput->osSurface->dwWidth))
    {
        curInput->bVEBOXCroppingUsed = true;
        VP_RENDER_NORMALMESSAGE("bVEBOXCroppingUsed = true, input: rcSrc.bottom: %d, rcSrc.right: %d; dwHeight: %d, dwHeight: %d;",
            (uint32_t)curInput->rcSrc.bottom,
            (uint32_t)curInput->rcSrc.right,
            curInput->osSurface->dwHeight,
            curInput->osSurface->dwWidth);

        if (curOutput)
        {
            curOutput->bVEBOXCroppingUsed = true;
            VP_RENDER_NORMALMESSAGE("                           output: rcSrc.bottom: %d, rcSrc.right: %d; dwHeight: %d, dwHeight: %d;",
                (uint32_t)curOutput->rcSrc.bottom,
                (uint32_t)curOutput->rcSrc.right,
                curOutput->osSurface->dwHeight,
                curOutput->osSurface->dwWidth);
        }
    }
    else
    {
        curInput->bVEBOXCroppingUsed = false;
        if (curOutput)
        {
            curOutput->bVEBOXCroppingUsed = false;
        }
    }

    // Adjust boundary for statistics surface block
    VP_RENDER_CHK_STATUS_RETURN(AdjustBlockStatistics());

    // Get Vebox Secure mode form policy
    m_useKernelResource = packetCaps.bSecureVebox;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpVeboxCmdPacket::Submit(MOS_COMMAND_BUFFER* commandBuffer, uint8_t packetPhase)
{
    VP_FUNC_CALL();

    MOS_STATUS    eStatus = MOS_STATUS_SUCCESS;
    VpVeboxRenderData   *pRenderData = GetLastExecRenderData();

    if (m_currentSurface && m_currentSurface->osSurface)
    {
        // Ensure the input is ready to be read
        // Currently, mos RegisterResourcere cannot sync the 3d resource.
        // Temporaly, call sync resource to do the sync explicitly.
        // Sync need be done after switching context.
#if MOS_MEDIASOLO_SUPPORTED
        if (!m_hwInterface->m_osInterface->bSoloInUse)
#endif
        {
            m_allocator->SyncOnResource(
                &m_currentSurface->osSurface->OsResource,
                false);
        }
    }

    // Send vebox command
    VP_RENDER_CHK_STATUS_RETURN(SendVeboxCmd(commandBuffer));

#if (_DEBUG || _RELEASE_INTERNAL)
    // Debug interface with state heap check
    VP_RENDER_CHK_STATUS_RETURN(DumpVeboxStateHeap())
#endif

    return eStatus;
}

void VpVeboxCmdPacket::CopySurfaceValue(
    VP_SURFACE                  *pTargetSurface,
    VP_SURFACE                  *pSourceSurface)
{
    VP_FUNC_CALL();

    if (pTargetSurface == nullptr)
    {
        VP_RENDER_ASSERTMESSAGE("Input pTargetSurface is null");
        return;
    }
    *pTargetSurface = *pSourceSurface;
}

VpVeboxCmdPacket::VpVeboxCmdPacket(
    MediaTask * task,
    PVP_MHWINTERFACE hwInterface,
    PVpAllocator &allocator,
    VPMediaMemComp *mmc) :
    CmdPacket(task),
    VpVeboxCmdPacketBase(task, hwInterface, allocator, mmc)
{
    m_veboxItf = hwInterface->m_vpPlatformInterface->GetMhwVeboxItf();
    m_miItf = hwInterface->m_vpPlatformInterface->GetMhwMiItf();
}

VpVeboxCmdPacket:: ~VpVeboxCmdPacket()
{
    VP_FUNC_CALL();

    MOS_Delete(m_sfcRender);
    MOS_Delete(m_lastExecRenderData);
    MOS_Delete(m_surfMemCacheCtl);

    m_allocator->DestroyVpSurface(m_currentSurface);
    m_allocator->DestroyVpSurface(m_previousSurface);
    m_allocator->DestroyVpSurface(m_renderTarget);
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
    VP_FUNC_CALL();

    uint32_t    uiPitch;
    int32_t     iOffset;
    MOS_STATUS  eStatus;

    eStatus     = MOS_STATUS_UNKNOWN;
    uiPitch     = 0;

    // Query platform dependent size of per frame information
    VP_RENDER_CHK_STATUS(QueryStatLayout(
        VEBOX_STAT_QUERY_PER_FRAME_SIZE, &uiPitch));

    // Get the base address of Frame based statistics for each slice
    if (m_PacketCaps.bDI) // VEBOX, VEBOX+IECP
    {
        // Frame based statistics begins after Encoder statistics
        iOffset = m_surfSetting.dwVeboxPerBlockStatisticsWidth *
                  m_surfSetting.dwVeboxPerBlockStatisticsHeight;

        *pStatSlice0Offset = iOffset + uiPitch;                                     // Slice 0 current frame
        *pStatSlice1Offset = iOffset + uiPitch * 3;                                 // Slice 1 current frame
    }
    else if (m_PacketCaps.bDN) // DN, DN_IECP, SpatialDI
    {
        // Frame based statistics begins after Encoder statistics
        iOffset = m_surfSetting.dwVeboxPerBlockStatisticsWidth *
                  m_surfSetting.dwVeboxPerBlockStatisticsHeight;

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

MOS_STATUS VpVeboxCmdPacket::AddVeboxDndiState()
{
    VP_FUNC_CALL();

    VpVeboxRenderData               *pRenderData     = GetLastExecRenderData();
    VP_RENDER_CHK_NULL_RETURN(m_veboxItf);
    VP_RENDER_CHK_NULL_RETURN(pRenderData);

    if (pRenderData->DN.bDnEnabled || pRenderData->DI.bDeinterlace || pRenderData->DI.bQueryVariance)
    {
        return m_veboxItf->SetVeboxDndiState(&pRenderData->GetDNDIParams());
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpVeboxCmdPacket::AddVeboxIECPState()
{
    VP_FUNC_CALL();

    VpVeboxRenderData*                pRenderData     = GetLastExecRenderData();

    VP_RENDER_CHK_NULL_RETURN(m_veboxItf);
    VP_RENDER_CHK_NULL_RETURN(pRenderData);

    if (pRenderData->IECP.IsIecpEnabled())
    {
        VP_PUBLIC_NORMALMESSAGE("IecpState is added. ace %d, lace %d, becsc %d, tcc %d, ste %d, procamp %d",
            pRenderData->IECP.ACE.bAceEnabled,
            pRenderData->IECP.LACE.bLaceEnabled,
            pRenderData->IECP.BeCSC.bBeCSCEnabled,
            pRenderData->IECP.TCC.bTccEnabled,
            pRenderData->IECP.STE.bSteEnabled,
            pRenderData->IECP.PROCAMP.bProcampEnabled);

        return m_veboxItf->SetVeboxIecpState(&pRenderData->GetIECPParams());
    }
    else
    {
        // BeCsc may not needed for AlphaFromStateSelect == 1 case.
        // Refer to IsBeCscNeededForAlphaFill for detail.
        VP_PUBLIC_NORMALMESSAGE("IecpState is not added with AlphaFromStateSelect %d",
            pRenderData->GetIECPParams().bAlphaEnable);
    }
    return MOS_STATUS_SUCCESS;
}

bool VpVeboxCmdPacket::IsVeboxGamutStateNeeded()
{
    VpVeboxRenderData *renderData = GetLastExecRenderData();
    return renderData ? renderData->HDR3DLUT.bHdr3DLut : false;
}

//!
//! \brief    Add vebox Gamut state
//! \details  Add vebox Gamut state
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VpVeboxCmdPacket::AddVeboxGamutState()
{
    VP_FUNC_CALL();

    VpVeboxRenderData *renderData = GetLastExecRenderData();
    VP_PUBLIC_CHK_NULL_RETURN(m_veboxItf);
    VP_PUBLIC_CHK_NULL_RETURN(renderData);

    if (IsVeboxGamutStateNeeded())
    {
        VP_PUBLIC_CHK_STATUS_RETURN(m_veboxItf->SetVeboxGamutState(
            &renderData->GetIECPParams(),
            &renderData->GetGamutParams()));
    }

    return MOS_STATUS_SUCCESS;
}

//!
//! \brief    Add vebox Hdr state
//! \details  Add vebox Hdr state
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VpVeboxCmdPacket::AddVeboxHdrState()
{
    VP_FUNC_CALL();

    VpVeboxRenderData    *renderData      = GetLastExecRenderData();
    VP_PUBLIC_CHK_NULL_RETURN(m_veboxItf);
    VP_PUBLIC_CHK_NULL_RETURN(renderData);
    MHW_VEBOX_IECP_PARAMS &mhwVeboxIecpParams = renderData->GetIECPParams();

    VP_PUBLIC_CHK_STATUS_RETURN(m_veboxItf->AddVeboxHdrState(&mhwVeboxIecpParams));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpVeboxCmdPacket::SetupIndirectStates()
{
    VP_FUNC_CALL();

    VpVeboxRenderData               *pRenderData    = GetLastExecRenderData();

    VP_RENDER_CHK_NULL_RETURN(pRenderData);
    VP_RENDER_CHK_NULL_RETURN(m_veboxItf);

    // Set FMD Params
    VP_RENDER_CHK_STATUS_RETURN(ConfigFMDParams(pRenderData->GetDNDIParams().bProgressiveDN, pRenderData->DN.bAutoDetect, pRenderData->DI.bFmdEnabled));

    //----------------------------------
    // Allocate and reset VEBOX state
    //----------------------------------
    VP_RENDER_CHK_STATUS_RETURN(m_veboxItf->AssignVeboxState());

    // Set IECP State
    VP_RENDER_CHK_STATUS_RETURN(AddVeboxIECPState());

    // Set DNDI State
    VP_RENDER_CHK_STATUS_RETURN(AddVeboxDndiState());

    // Set GAMUT State
    VP_RENDER_CHK_STATUS_RETURN(AddVeboxGamutState());

    // Set HDR State
    VP_RENDER_CHK_STATUS_RETURN(AddVeboxHdrState());

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

MOS_STATUS VpVeboxCmdPacket::IsCmdParamsValid(
    const mhw::vebox::VEBOX_STATE_PAR           &veboxStateCmdParams,
    const mhw::vebox::VEB_DI_IECP_PAR           &veboxDiIecpCmdParams,
    const VPHAL_VEBOX_SURFACE_STATE_CMD_PARAMS  &VeboxSurfaceStateCmdParams)
{
    VP_FUNC_CALL();

    const MHW_VEBOX_MODE    &veboxMode          = veboxStateCmdParams.VeboxMode;

    if (veboxMode.DIEnable)
    {
        if (nullptr == veboxDiIecpCmdParams.pOsResPrevOutput &&
            (MEDIA_VEBOX_DI_OUTPUT_PREVIOUS == veboxMode.DIOutputFrames || MEDIA_VEBOX_DI_OUTPUT_BOTH == veboxMode.DIOutputFrames))
        {
            MT_ERR1(MT_VP_HAL_RENDER_VE, MT_SURF_ALLOC_HANDLE, 0);
            return MOS_STATUS_INVALID_PARAMETER;
        }
        if (nullptr == veboxDiIecpCmdParams.pOsResCurrOutput &&
            (MEDIA_VEBOX_DI_OUTPUT_CURRENT == veboxMode.DIOutputFrames || MEDIA_VEBOX_DI_OUTPUT_BOTH == veboxMode.DIOutputFrames))
        {
            MT_ERR1(MT_VP_HAL_RENDER_VE, MT_SURF_ALLOC_HANDLE, 0);
            return MOS_STATUS_INVALID_PARAMETER;
        }
    }

    if (m_PacketCaps.bDN && !m_PacketCaps.bDI && !m_PacketCaps.bQueryVariance && !m_PacketCaps.bIECP)
    {
        if ((VeboxSurfaceStateCmdParams.pSurfInput->osSurface->TileModeGMM == VeboxSurfaceStateCmdParams.pSurfDNOutput->osSurface->TileModeGMM) &&
            (VeboxSurfaceStateCmdParams.pSurfInput->osSurface->dwPitch != VeboxSurfaceStateCmdParams.pSurfDNOutput->osSurface->dwPitch))
        {
            MT_ERR3(MT_VP_MHW_VE_SURFSTATE_INPUT, MT_SURF_TILE_MODE, VeboxSurfaceStateCmdParams.pSurfInput->osSurface->TileModeGMM,
                MT_SURF_PITCH, VeboxSurfaceStateCmdParams.pSurfInput->osSurface->dwPitch, MT_SURF_PITCH, VeboxSurfaceStateCmdParams.pSurfDNOutput->osSurface->dwPitch);
            return MOS_STATUS_INVALID_PARAMETER;
        }
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpVeboxCmdPacket::VeboxSetPerfTag()
{
    VP_FUNC_CALL();

    MOS_STATUS                  eStatus = MOS_STATUS_SUCCESS;
    PVPHAL_PERFTAG              pPerfTag = nullptr;
    VpVeboxRenderData           *pRenderData = GetLastExecRenderData();

    VP_PUBLIC_CHK_NULL_RETURN(pRenderData);
    VP_PUBLIC_CHK_NULL_RETURN(m_currentSurface);
    VP_PUBLIC_CHK_NULL_RETURN(m_currentSurface->osSurface);

    MOS_FORMAT srcFmt = m_currentSurface->osSurface->Format;

    pPerfTag = &pRenderData->PerfTag;

    switch (srcFmt)
    {
        case Format_NV12:
            return VeboxSetPerfTagNv12();

        CASE_PA_FORMAT:
            return VeboxSetPerfTagPaFormat();

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

MOS_STATUS VpVeboxCmdPacket::VeboxSetPerfTagNv12()
{
    VP_FUNC_CALL();

    MOS_STATUS                  eStatus = MOS_STATUS_SUCCESS;
    PVPHAL_PERFTAG              pPerfTag = nullptr;
    VpVeboxRenderData           *pRenderData = GetLastExecRenderData();

    VP_PUBLIC_CHK_NULL_RETURN(pRenderData);
    VP_PUBLIC_CHK_NULL_RETURN(m_renderTarget);
    VP_PUBLIC_CHK_NULL_RETURN(m_renderTarget->osSurface);

    MOS_FORMAT dstFormat = m_renderTarget->osSurface->Format;

    pPerfTag = &pRenderData->PerfTag;

    if (pRenderData->IsDiEnabled())
    {
        if (pRenderData->DN.bDnEnabled ||
            pRenderData->DN.bChromaDnEnabled)
        {
            if (IsIECPEnabled())
            {
                *pPerfTag = VPHAL_NV12_DNDI_422CP;
            }
            else
            {
                *pPerfTag = VPHAL_NV12_DNDI_PA;
            }
        }
        else
        {
            if (IsIECPEnabled())
            {
                *pPerfTag = VPHAL_PL_DI_422CP;
            }
            else
            {
                *pPerfTag = VPHAL_PL_DI_PA;
            }
        }
    }
    else
    {
        if (pRenderData->DN.bDnEnabled ||
            pRenderData->DN.bChromaDnEnabled)
        {
            if (IsOutputPipeVebox())
            {
                switch (dstFormat)
                {
                    case Format_NV12:
                        *pPerfTag = VPHAL_NV12_DN_420CP;
                        break;
                    CASE_PA_FORMAT:
                        *pPerfTag = VPHAL_NV12_DN_422CP;
                        break;
                    case Format_RGB32:
                        *pPerfTag = VPHAL_NV12_DN_RGB32CP;
                    case Format_A8R8G8B8:
                    case Format_A8B8G8R8:
                        *pPerfTag = VPHAL_NV12_DN_RGB32CP;
                        break;
                    case Format_P010:
                    case Format_P016:
                    case Format_Y410:
                    case Format_Y416:
                    case Format_Y210:
                    case Format_Y216:
                    case Format_AYUV:
                    case Format_Y8:
                    case Format_Y16S:
                    case Format_Y16U:
                        *pPerfTag = VPHAL_NONE;
                        break;
                    default:
                        VP_PUBLIC_ASSERTMESSAGE("Output Format Not found.");
                        return MOS_STATUS_INVALID_PARAMETER;
                }
            }
            else if (IsIECPEnabled())
            {
                *pPerfTag = VPHAL_NV12_DN_420CP;
            }
            else
            {
                *pPerfTag = VPHAL_NV12_DN_NV12;
            }
        }
        else
        {
            if (IsOutputPipeVebox())
            {
                switch (dstFormat)
                {
                    case Format_NV12:
                        *pPerfTag = VPHAL_NV12_420CP;
                        break;
                    CASE_PA_FORMAT:
                        *pPerfTag = VPHAL_NV12_422CP;
                        break;
                    case Format_RGB32:
                        *pPerfTag = VPHAL_NV12_RGB32CP;
                    case Format_A8R8G8B8:
                    case Format_A8B8G8R8:
                    case Format_R10G10B10A2:
                    case Format_B10G10R10A2:
                        *pPerfTag = VPHAL_NV12_RGB32CP;
                        break;
                    case Format_P010:
                    case Format_P016:
                    case Format_Y410:
                    case Format_Y416:
                    case Format_Y210:
                    case Format_Y216:
                    case Format_AYUV:
                    case Format_Y8:
                    case Format_Y16S:
                    case Format_Y16U:
                        *pPerfTag = VPHAL_NONE;
                        break;
                    default:
                        VPHAL_RENDER_ASSERTMESSAGE("Output Format Not found.");
                        return MOS_STATUS_INVALID_PARAMETER;
                }
            }
            else
            {
                *pPerfTag = VPHAL_NV12_420CP;
            }
        }
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpVeboxCmdPacket::VeboxSetPerfTagPaFormat()
{
    VP_FUNC_CALL();

    MOS_STATUS                  eStatus = MOS_STATUS_SUCCESS;
    PVPHAL_PERFTAG              pPerfTag = nullptr;
    VpVeboxRenderData           *pRenderData = GetLastExecRenderData();

    VP_PUBLIC_CHK_NULL_RETURN(pRenderData);
    VP_PUBLIC_CHK_NULL_RETURN(m_renderTarget);
    VP_PUBLIC_CHK_NULL_RETURN(m_renderTarget->osSurface);

    MOS_FORMAT dstFormat = m_renderTarget->osSurface->Format;

    pPerfTag = &pRenderData->PerfTag;

    if (pRenderData->IsDiEnabled())
    {
        if (pRenderData->DN.bDnEnabled ||
            pRenderData->DN.bChromaDnEnabled)
        {
            if (IsIECPEnabled())
            {
                *pPerfTag = VPHAL_PA_DNDI_422CP;
            }
            else
            {
                *pPerfTag = VPHAL_PA_DNDI_PA;
            }
        }
        else
        {
            if (IsIECPEnabled())
            {
                *pPerfTag = VPHAL_PA_DI_422CP;
            }
            else
            {
                *pPerfTag = VPHAL_PA_DI_PA;
            }
        }
    }
    else
    {
        if (pRenderData->DN.bDnEnabled ||
            pRenderData->DN.bChromaDnEnabled)
        {
            if (IsOutputPipeVebox())
            {
                switch (dstFormat)
                {
                    case Format_NV12:
                        *pPerfTag = VPHAL_PA_DN_420CP;
                        break;
                    CASE_PA_FORMAT:
                        *pPerfTag = VPHAL_PA_DN_422CP;
                        break;
                    case Format_RGB32:
                        *pPerfTag = VPHAL_PA_DN_RGB32CP;
                        break;
                    case Format_A8R8G8B8:
                    case Format_A8B8G8R8:
                    case Format_R10G10B10A2:
                    case Format_B10G10R10A2:
                        *pPerfTag = VPHAL_PA_RGB32CP;
                        break;
                    case Format_P010:
                    case Format_P016:
                    case Format_Y410:
                    case Format_Y416:
                    case Format_Y210:
                    case Format_Y216:
                    case Format_AYUV:
                    case Format_Y8:
                    case Format_Y16S:
                    case Format_Y16U:
                        *pPerfTag = VPHAL_NONE;
                        break;
                    default:
                        VPHAL_RENDER_ASSERTMESSAGE("Output Format Not found.");
                        return MOS_STATUS_INVALID_PARAMETER;
                }
            }
            else if (IsIECPEnabled())
            {
                *pPerfTag = VPHAL_PA_DN_422CP;
            }
            else
            {
                *pPerfTag = VPHAL_PA_DN_PA;
            }
        }
        else
        {
            if (IsOutputPipeVebox())
            {
                switch (dstFormat)
                {
                    case Format_NV12:
                        *pPerfTag = VPHAL_PA_420CP;
                        break;
                    CASE_PA_FORMAT:
                        *pPerfTag = VPHAL_PA_422CP;
                        break;
                    case Format_RGB32:
                        *pPerfTag = VPHAL_PA_RGB32CP;
                        break;
                    case Format_A8R8G8B8:
                    case Format_A8B8G8R8:
                    case Format_R10G10B10A2:
                    case Format_B10G10R10A2:
                        *pPerfTag = VPHAL_PA_RGB32CP;
                        break;
                    case Format_P010:
                    case Format_P016:
                    case Format_Y410:
                    case Format_Y416:
                    case Format_Y210:
                    case Format_Y216:
                    case Format_AYUV:
                    case Format_Y8:
                    case Format_Y16S:
                    case Format_Y16U:
                        *pPerfTag = VPHAL_NONE;
                        break;
                    default:
                        VPHAL_RENDER_ASSERTMESSAGE("Output Format Not found.");
                        return MOS_STATUS_INVALID_PARAMETER;
                }
            }
            else
            {
                *pPerfTag = VPHAL_PA_422CP;
            }
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpVeboxCmdPacket::UpdateVeboxStates()
{
    VP_FUNC_CALL();

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpVeboxCmdPacket::InitSurfMemCacheControl(VP_EXECUTE_CAPS packetCaps)
{
    VP_FUNC_CALL();

    MOS_HW_RESOURCE_DEF                 Usage           = MOS_HW_RESOURCE_DEF_MAX;
    MEMORY_OBJECT_CONTROL_STATE         MemObjCtrl      = {};
    PMOS_INTERFACE                      pOsInterface    = nullptr;
    PVP_VEBOX_CACHE_CNTL                pSettings       = nullptr;

    if (nullptr == m_surfMemCacheCtl)
    {
        m_surfMemCacheCtl = MOS_New(VP_VEBOX_CACHE_CNTL);
    }

    VP_PUBLIC_CHK_NULL_RETURN(m_surfMemCacheCtl);
    VP_PUBLIC_CHK_NULL_RETURN(m_hwInterface);
    VP_PUBLIC_CHK_NULL_RETURN(m_hwInterface->m_osInterface);

    MOS_ZeroMemory(m_surfMemCacheCtl, sizeof(VP_VEBOX_CACHE_CNTL));

    pOsInterface    = m_hwInterface->m_osInterface;
    pSettings       = m_surfMemCacheCtl;

    pSettings->bDnDi = true;

    if (pSettings->bDnDi)
    {
        pSettings->DnDi.bL3CachingEnabled = true;

        VPHAL_SET_SURF_MEMOBJCTL(pSettings->DnDi.CurrentInputSurfMemObjCtl,        MOS_MP_RESOURCE_USAGE_SurfaceState);
        VPHAL_SET_SURF_MEMOBJCTL(pSettings->DnDi.PreviousInputSurfMemObjCtl,       MOS_MP_RESOURCE_USAGE_SurfaceState);
        VPHAL_SET_SURF_MEMOBJCTL(pSettings->DnDi.STMMInputSurfMemObjCtl,           MOS_MP_RESOURCE_USAGE_SurfaceState);
        VPHAL_SET_SURF_MEMOBJCTL(pSettings->DnDi.STMMOutputSurfMemObjCtl,          MOS_MP_RESOURCE_USAGE_SurfaceState);
        VPHAL_SET_SURF_MEMOBJCTL(pSettings->DnDi.DnOutSurfMemObjCtl,               MOS_MP_RESOURCE_USAGE_SurfaceState);

        if (packetCaps.bVebox && !packetCaps.bSFC && !packetCaps.bRender)
        {
            // Disable cache for output surface in vebox only condition
            VPHAL_SET_SURF_MEMOBJCTL(pSettings->DnDi.CurrentOutputSurfMemObjCtl,    MOS_MP_RESOURCE_USAGE_DEFAULT);
        }
        else
        {
            VPHAL_SET_SURF_MEMOBJCTL(pSettings->DnDi.CurrentOutputSurfMemObjCtl,    MOS_MP_RESOURCE_USAGE_SurfaceState);
        }

        VPHAL_SET_SURF_MEMOBJCTL(pSettings->DnDi.StatisticsOutputSurfMemObjCtl,    MOS_MP_RESOURCE_USAGE_SurfaceState);
        VPHAL_SET_SURF_MEMOBJCTL(pSettings->DnDi.AlphaOrVignetteSurfMemObjCtl,     MOS_MP_RESOURCE_USAGE_SurfaceState);
        VPHAL_SET_SURF_MEMOBJCTL(pSettings->DnDi.LaceOrAceOrRgbHistogramSurfCtrl,  MOS_MP_RESOURCE_USAGE_SurfaceState);
        VPHAL_SET_SURF_MEMOBJCTL(pSettings->DnDi.SkinScoreSurfMemObjCtl,           MOS_MP_RESOURCE_USAGE_SurfaceState);
        VPHAL_SET_SURF_MEMOBJCTL(pSettings->DnDi.LaceLookUpTablesSurfMemObjCtl,    MOS_MP_RESOURCE_USAGE_SurfaceState);
        VPHAL_SET_SURF_MEMOBJCTL(pSettings->DnDi.Vebox3DLookUpTablesSurfMemObjCtl, MOS_MP_RESOURCE_USAGE_SurfaceState);
    }
    else
    {
        pSettings->DnDi.bL3CachingEnabled = false;

        VPHAL_SET_SURF_MEMOBJCTL(pSettings->DnDi.CurrentInputSurfMemObjCtl,        MOS_MP_RESOURCE_USAGE_DEFAULT);
        VPHAL_SET_SURF_MEMOBJCTL(pSettings->DnDi.PreviousInputSurfMemObjCtl,       MOS_MP_RESOURCE_USAGE_DEFAULT);
        VPHAL_SET_SURF_MEMOBJCTL(pSettings->DnDi.STMMInputSurfMemObjCtl,           MOS_MP_RESOURCE_USAGE_DEFAULT);
        VPHAL_SET_SURF_MEMOBJCTL(pSettings->DnDi.STMMOutputSurfMemObjCtl,          MOS_MP_RESOURCE_USAGE_DEFAULT);
        VPHAL_SET_SURF_MEMOBJCTL(pSettings->DnDi.DnOutSurfMemObjCtl,               MOS_MP_RESOURCE_USAGE_DEFAULT);
        VPHAL_SET_SURF_MEMOBJCTL(pSettings->DnDi.CurrentOutputSurfMemObjCtl,       MOS_MP_RESOURCE_USAGE_DEFAULT);
        VPHAL_SET_SURF_MEMOBJCTL(pSettings->DnDi.StatisticsOutputSurfMemObjCtl,    MOS_MP_RESOURCE_USAGE_DEFAULT);
        VPHAL_SET_SURF_MEMOBJCTL(pSettings->DnDi.AlphaOrVignetteSurfMemObjCtl,     MOS_MP_RESOURCE_USAGE_DEFAULT);
        VPHAL_SET_SURF_MEMOBJCTL(pSettings->DnDi.LaceOrAceOrRgbHistogramSurfCtrl,  MOS_MP_RESOURCE_USAGE_DEFAULT);
        VPHAL_SET_SURF_MEMOBJCTL(pSettings->DnDi.SkinScoreSurfMemObjCtl,           MOS_MP_RESOURCE_USAGE_DEFAULT);
        VPHAL_SET_SURF_MEMOBJCTL(pSettings->DnDi.LaceLookUpTablesSurfMemObjCtl,    MOS_MP_RESOURCE_USAGE_DEFAULT);
        VPHAL_SET_SURF_MEMOBJCTL(pSettings->DnDi.Vebox3DLookUpTablesSurfMemObjCtl, MOS_MP_RESOURCE_USAGE_DEFAULT);
    }

    if (pSettings->bLace)
    {
        VPHAL_SET_SURF_MEMOBJCTL(pSettings->Lace.FrameHistogramSurfaceMemObjCtl,                       MOS_MP_RESOURCE_USAGE_SurfaceState);
        VPHAL_SET_SURF_MEMOBJCTL(pSettings->Lace.AggregatedHistogramSurfaceMemObjCtl,                  MOS_MP_RESOURCE_USAGE_SurfaceState);
        VPHAL_SET_SURF_MEMOBJCTL(pSettings->Lace.StdStatisticsSurfaceMemObjCtl,                        MOS_MP_RESOURCE_USAGE_SurfaceState);
        VPHAL_SET_SURF_MEMOBJCTL(pSettings->Lace.PwlfInSurfaceMemObjCtl,                               MOS_MP_RESOURCE_USAGE_SurfaceState);
        VPHAL_SET_SURF_MEMOBJCTL(pSettings->Lace.PwlfOutSurfaceMemObjCtl,                              MOS_MP_RESOURCE_USAGE_SurfaceState);
        VPHAL_SET_SURF_MEMOBJCTL(pSettings->Lace.WeitCoefSurfaceMemObjCtl,                             MOS_MP_RESOURCE_USAGE_SurfaceState);
    }
    else
    {
        VPHAL_SET_SURF_MEMOBJCTL(pSettings->Lace.FrameHistogramSurfaceMemObjCtl,                       MOS_MP_RESOURCE_USAGE_DEFAULT);
        VPHAL_SET_SURF_MEMOBJCTL(pSettings->Lace.AggregatedHistogramSurfaceMemObjCtl,                  MOS_MP_RESOURCE_USAGE_DEFAULT);
        VPHAL_SET_SURF_MEMOBJCTL(pSettings->Lace.StdStatisticsSurfaceMemObjCtl,                        MOS_MP_RESOURCE_USAGE_DEFAULT);
        VPHAL_SET_SURF_MEMOBJCTL(pSettings->Lace.PwlfInSurfaceMemObjCtl,                               MOS_MP_RESOURCE_USAGE_DEFAULT);
        VPHAL_SET_SURF_MEMOBJCTL(pSettings->Lace.PwlfOutSurfaceMemObjCtl,                              MOS_MP_RESOURCE_USAGE_DEFAULT);
        VPHAL_SET_SURF_MEMOBJCTL(pSettings->Lace.WeitCoefSurfaceMemObjCtl,                             MOS_MP_RESOURCE_USAGE_DEFAULT);
        VPHAL_SET_SURF_MEMOBJCTL(pSettings->Lace.GlobalToneMappingCurveLUTSurfaceMemObjCtl,            MOS_MP_RESOURCE_USAGE_DEFAULT);
    }

    return MOS_STATUS_SUCCESS;
}

MHW_CSPACE VpVeboxCmdPacket::VpHalCspace2MhwCspace(VPHAL_CSPACE cspace)
{
    VP_FUNC_CALL();

    switch (cspace)
    {
        case CSpace_Source:
            return MHW_CSpace_Source;

        case CSpace_RGB:
            return MHW_CSpace_RGB;

        case CSpace_YUV:
            return MHW_CSpace_YUV;

        case CSpace_Gray:
            return MHW_CSpace_Gray;

        case CSpace_Any:
            return MHW_CSpace_Any;

        case CSpace_sRGB:
            return MHW_CSpace_sRGB;

        case CSpace_stRGB:
            return MHW_CSpace_stRGB;

        case CSpace_BT601:
            return MHW_CSpace_BT601;

        case CSpace_BT601_FullRange:
            return MHW_CSpace_BT601_FullRange;

        case CSpace_BT709:
            return MHW_CSpace_BT709;

        case CSpace_BT709_FullRange:
            return MHW_CSpace_BT709_FullRange;

        case CSpace_xvYCC601:
            return MHW_CSpace_xvYCC601;

        case CSpace_xvYCC709:
            return MHW_CSpace_xvYCC709;

        case CSpace_BT601Gray:
            return MHW_CSpace_BT601Gray;

        case CSpace_BT601Gray_FullRange:
            return MHW_CSpace_BT601Gray_FullRange;

        case CSpace_BT2020:
            return MHW_CSpace_BT2020;

        case CSpace_BT2020_RGB:
            return MHW_CSpace_BT2020_RGB;

        case CSpace_BT2020_FullRange:
            return MHW_CSpace_BT2020_FullRange;

        case CSpace_BT2020_stRGB:
            return MHW_CSpace_BT2020_stRGB;

        case CSpace_None:
        default:
            return MHW_CSpace_None;
    }
}

}

