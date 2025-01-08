/*
* Copyright (c) 2018-2023, Intel Corporation
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
#include "vp_render_ief.h"
#include "vp_feature_caps.h"
#include "vp_platform_interface.h"
#include "vp_hal_ddi_utils.h"
#include "mhw_vebox_itf.h"
#include "mhw_mi_itf.h"
#include "mhw_mi_cmdpar.h"
#include "mhw_utilities.h"
#include "media_scalability_defs.h"
#include "renderhal_platform_interface_next.h"

#define SURFACE_DW_UY_OFFSET(pSurface) \
    ((pSurface) != nullptr ? ((pSurface)->UPlaneOffset.iSurfaceOffset - (pSurface)->dwOffset) / (pSurface)->dwPitch + (pSurface)->UPlaneOffset.iYOffset : 0)

#define SURFACE_DW_VY_OFFSET(pSurface) \
    ((pSurface) != nullptr ? ((pSurface)->VPlaneOffset.iSurfaceOffset - (pSurface)->dwOffset) / (pSurface)->dwPitch + (pSurface)->VPlaneOffset.iYOffset : 0)
namespace vp {

#define INTERP(x0, x1, x, y0, y1)   ((uint32_t) floor(y0+(x-x0)*(y1-y0)/(double)(x1-x0)))

const uint32_t  VpVeboxCmdPacket::m_satP1Table[MHW_STE_FACTOR_MAX + 1] = {
    0x00000000, 0xfffffffe, 0xfffffffc, 0xfffffffa, 0xfffffff6, 0xfffffff4, 0xfffffff2, 0xfffffff0, 0xffffffee, 0xffffffec };

const uint32_t   VpVeboxCmdPacket::m_satS0Table[MHW_STE_FACTOR_MAX + 1] = {
    0x000000ef, 0x00000100, 0x00000113, 0x00000129, 0x0000017a, 0x000001a2, 0x000001d3, 0x00000211, 0x00000262, 0x000002d1 };

const uint32_t   VpVeboxCmdPacket::m_satS1Table[MHW_STE_FACTOR_MAX + 1] = {
    0x000000ab, 0x00000080, 0x00000066, 0x00000055, 0x000000c2, 0x000000b9, 0x000000b0, 0x000000a9, 0x000000a2, 0x0000009c };

MOS_STATUS VpVeboxCmdPacket::SetupSurfaceStates(
    PVP_VEBOX_SURFACE_STATE_CMD_PARAMS   pVeboxSurfaceStateCmdParams)
{
    VP_FUNC_CALL();

    VP_RENDER_CHK_NULL_RETURN(pVeboxSurfaceStateCmdParams);
    MOS_ZeroMemory(pVeboxSurfaceStateCmdParams, sizeof(VP_VEBOX_SURFACE_STATE_CMD_PARAMS));
    pVeboxSurfaceStateCmdParams->pSurfInput    = m_veboxPacketSurface.pCurrInput;
    pVeboxSurfaceStateCmdParams->pSurfOutput   = m_veboxPacketSurface.pCurrOutput;
    pVeboxSurfaceStateCmdParams->pSurfSTMM     = m_veboxPacketSurface.pSTMMInput;
    pVeboxSurfaceStateCmdParams->pSurfDNOutput = m_veboxPacketSurface.pDenoisedCurrOutput;
    pVeboxSurfaceStateCmdParams->bDIEnable     = m_PacketCaps.bDI;
    pVeboxSurfaceStateCmdParams->b3DlutEnable  = m_PacketCaps.bHDR3DLUT;  // Need to consider cappipe

    if (pVeboxSurfaceStateCmdParams->pSurfOutput &&
        pVeboxSurfaceStateCmdParams->pSurfOutput->osSurface &&
        pVeboxSurfaceStateCmdParams->pSurfOutput->osSurface->OsResource.bUncompressedWriteNeeded)
    {
        VP_RENDER_NORMALMESSAGE("Force compression as RC for bUncompressedWriteNeeded being true");
        pVeboxSurfaceStateCmdParams->pSurfOutput->osSurface->CompressionMode = MOS_MMC_RC;
    }

    UpdateCpPrepareResources();
    return MOS_STATUS_SUCCESS;
}

void VpVeboxCmdPacket::UpdateCpPrepareResources()
{
    VP_FUNC_CALL();

    VpVeboxRenderData *pRenderData = GetLastExecRenderData();
    VP_PUBLIC_CHK_NULL_NO_STATUS_RETURN(pRenderData);
    // For 3DLut usage, it update in CpPrepareResources() for kernel usage, should
    // reupdate here. For other feature usage, it already update in vp_pipeline
    if (pRenderData->HDR3DLUT.is3DLutTableUpdatedByKernel == true)
    {
        VP_RENDER_NORMALMESSAGE("Update CP Prepare Resource for 3DLut kernel.");
        PMOS_RESOURCE source[VPHAL_MAX_SOURCES] = {nullptr};
        PMOS_RESOURCE target[VPHAL_MAX_TARGETS] = {nullptr};

        if ((nullptr != m_hwInterface->m_osInterface) &&
            (nullptr != m_hwInterface->m_osInterface->osCpInterface))
        {
            VP_SURFACE *surf = GetSurface(SurfaceTypeVeboxInput);
            VP_PUBLIC_CHK_NULL_NO_STATUS_RETURN(surf);
            source[0] = &(surf->osSurface->OsResource);

            VP_PUBLIC_CHK_NULL_NO_STATUS_RETURN(m_renderTarget);
            target[0] = &(m_renderTarget->osSurface->OsResource);

            m_hwInterface->m_osInterface->osCpInterface->PrepareResources((void **)source, 1, (void **)target, 1);
        }
    }
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

MOS_STATUS VpVeboxCmdPacket::SetupVeboxExternal3DLutforHDR(
    mhw::vebox::VEBOX_STATE_PAR &veboxStateCmdParams)
{
    VP_RENDER_NORMALMESSAGE("Init 3DLut table surface by external API.");
    PMHW_VEBOX_MODE    pVeboxMode   = nullptr;
    PMHW_VEBOX_3D_LUT  pLUT3D       = nullptr;
    PMHW_3DLUT_PARAMS  external3DLutParams = nullptr;
    PMOS_INTERFACE     osInterface  = nullptr;
    VpVeboxRenderData *pRenderData  = GetLastExecRenderData();
    external3DLutParams                    = &(pRenderData->GetIECPParams().s3DLutParams);

    VP_RENDER_CHK_NULL_RETURN(m_surfMemCacheCtl);
    VP_RENDER_CHK_NULL_RETURN(pRenderData);

    VP_PUBLIC_CHK_NULL_RETURN(external3DLutParams);
    VP_PUBLIC_CHK_NULL_RETURN(m_hwInterface->m_osInterface);
    osInterface = m_hwInterface->m_osInterface;

    pVeboxMode = &veboxStateCmdParams.VeboxMode;
    pLUT3D     = &veboxStateCmdParams.LUT3D;

    VP_PUBLIC_CHK_NULL_RETURN(pLUT3D);
    pLUT3D->ArbitrationPriorityControl    = 0;
    pLUT3D->Lut3dEnable                   = true;
    pVeboxMode->ColorGamutExpansionEnable = true;

    pLUT3D->Lut3dSize = 0;
    switch (external3DLutParams->LUTSize)
    {
    case 17:
        pLUT3D->Lut3dSize = 1;
        break;
    case 65:
        pLUT3D->Lut3dSize = 2;
        break;
    case 45:
        pLUT3D->Lut3dSize = 3;
        break;
    case 33:
    default:
        pLUT3D->Lut3dSize = 0;
        break;
    }
    VP_RENDER_CHK_STATUS_RETURN(osInterface->pfnRegisterResource(
        osInterface,
        &(pRenderData->HDR3DLUT.external3DLutSurfResource),
        false,
        true));

    veboxStateCmdParams.Vebox3DLookUpTablesSurfCtrl.Value =
        m_surfMemCacheCtl->DnDi.Vebox3DLookUpTablesSurfMemObjCtl;
    veboxStateCmdParams.pVebox3DLookUpTables = &(pRenderData->HDR3DLUT.external3DLutSurfResource);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpVeboxCmdPacket::Add1DLutState(PVP_SURFACE &surface, PMHW_1DLUT_PARAMS p1DLutParams)
{
    VP_FUNC_CALL();
    if (m_PacketCaps.b1K1DLutInited)
    {
        VP_RENDER_NORMALMESSAGE("1K1DLut Surface is reused for HDR");
        return MOS_STATUS_SUCCESS;
    }
    VP_RENDER_CHK_NULL_RETURN(surface);
    void *sur = (void *)m_allocator->LockResourceForWrite(&surface->osSurface->OsResource);
    VP_PUBLIC_CHK_NULL_RETURN(sur);
    VpVeboxRenderData *pRenderData = GetLastExecRenderData();
    VP_PUBLIC_CHK_NULL_RETURN(m_veboxItf);
    VP_PUBLIC_CHK_NULL_RETURN(pRenderData);

    if (m_veboxItf)
    {
        m_veboxItf->Add1DLutState(sur, p1DLutParams);
        VP_RENDER_NORMALMESSAGE("1K1DLut Surface is inited for HDR");
    }

    VP_PUBLIC_CHK_STATUS_RETURN(m_allocator->UnLock(&surface->osSurface->OsResource));
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpVeboxCmdPacket::SetupVebox3DLutForHDR(mhw::vebox::VEBOX_STATE_PAR &veboxStateCmdParams)
{
    PMHW_VEBOX_MODE       pVeboxMode     = nullptr;
    PMHW_VEBOX_3D_LUT     pLUT3D         = nullptr;
    PMHW_1DLUT_PARAMS     p1DLutParams   = nullptr;
    PVP_SURFACE           surf3DLut      = GetSurface(SurfaceType3DLut);
    VpVeboxRenderData    *pRenderData    = GetLastExecRenderData();

    VP_RENDER_CHK_NULL_RETURN(m_surfMemCacheCtl);
    VP_RENDER_CHK_NULL_RETURN(surf3DLut);
    VP_RENDER_CHK_NULL_RETURN(surf3DLut->osSurface);
    VP_RENDER_CHK_NULL_RETURN(pRenderData);
    VP_RENDER_CHK_STATUS_RETURN(Init3DLutTable(surf3DLut));

    pVeboxMode                         = &veboxStateCmdParams.VeboxMode;
    pLUT3D                             = &veboxStateCmdParams.LUT3D;
    p1DLutParams                       = &(pRenderData->GetIECPParams().s1DLutParams);

    VP_RENDER_CHK_NULL_RETURN(pVeboxMode);
    VP_RENDER_CHK_NULL_RETURN(p1DLutParams);
    VP_RENDER_CHK_NULL_RETURN(pLUT3D);

    pLUT3D->ArbitrationPriorityControl = 0;
    pLUT3D->Lut3dEnable                = true;
    // Config 3DLut size to 65 for HDR10 usage.
    pLUT3D->Lut3dSize = 2;
    if (pRenderData->HDR3DLUT.uiLutSize == 33)
    {
        pLUT3D->Lut3dSize = 0;  // 33x33x33
    }

    veboxStateCmdParams.Vebox3DLookUpTablesSurfCtrl.Value = m_surfMemCacheCtl->DnDi.Vebox3DLookUpTablesSurfMemObjCtl;

    //Use 1K1DLUT instead of Gamut Expansion
    pVeboxMode->ColorGamutExpansionEnable = false;

    PVP_SURFACE surface = GetSurface(SurfaceType1k1dLut);
    VP_RENDER_CHK_NULL_RETURN(surface);
    VP_RENDER_CHK_STATUS_RETURN(Add1DLutState(surface, nullptr));

    veboxStateCmdParams.pVebox1DLookUpTables = &(surface->osSurface->OsResource);
    pVeboxMode->Hdr1K1DLut                   = true;
    pVeboxMode->Hdr1DLutEnable               = true;
    VP_RENDER_CHK_STATUS_RETURN(m_veboxItf->SetDisableHistogram(&pRenderData->GetIECPParams()));

    veboxStateCmdParams.pVebox3DLookUpTables = &surf3DLut->osSurface->OsResource;

    VP_RENDER_CHK_STATUS_RETURN(SetupHDRUnifiedForHDR(veboxStateCmdParams));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpVeboxCmdPacket::SetupHDRUnifiedForHDR(mhw::vebox::VEBOX_STATE_PAR &veboxStateCmdParams)
{
    PMHW_VEBOX_MODE    pVeboxMode   = nullptr;

    pVeboxMode   = &veboxStateCmdParams.VeboxMode;
 
    pVeboxMode->Hdr1K1DLut = true;
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpVeboxCmdPacket::SetupHDRLuts(
    mhw::vebox::VEBOX_STATE_PAR &veboxStateCmdParams)
{
    VP_FUNC_CALL();
    VpVeboxRenderData *renderData = GetLastExecRenderData();
    VP_RENDER_CHK_NULL_RETURN(renderData);

    if (renderData->HDR3DLUT.isExternal3DLutTable)
    {
        VP_RENDER_CHK_STATUS_RETURN(SetupVeboxExternal3DLutforHDR(veboxStateCmdParams));
        VP_PUBLIC_NORMALMESSAGE("3DLUT table setup by API.");
        return MOS_STATUS_SUCCESS;
    }

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
    PMHW_FP16_PARAMS        fp16Params = nullptr;

    pVeboxMode = &veboxStateCmdParams.VeboxMode;
    VP_RENDER_CHK_NULL_RETURN(pVeboxMode);

    VpVeboxRenderData* pRenderData = GetLastExecRenderData();
    VP_RENDER_CHK_NULL_RETURN(pRenderData);

    fp16Params = &(pRenderData->GetIECPParams().fp16Params);
    VP_RENDER_CHK_NULL_RETURN(fp16Params);

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
    VP_RENDER_CHK_STATUS_RETURN(SetupDNTableForHVS(veboxStateCmdParams));

    if (fp16Params->isActive == 1)
    {
        VP_RENDER_CHK_STATUS_RETURN(SetupVeboxFP16State(veboxStateCmdParams));
    }

    veboxStateCmdParams.bCmBuffer = false;

    MHW_VEBOX_IECP_PARAMS& veboxIecpParams = pRenderData->GetIECPParams();
    // CCM for CSC needs HDR state
    pVeboxMode->Hdr1DLutEnable |= veboxIecpParams.bCcmCscEnable;
    // Only fp16 output needs CCM for CSC
    pVeboxMode->Fp16ModeEnable |= veboxIecpParams.bCcmCscEnable;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpVeboxCmdPacket::InitCmdBufferWithVeParams(
    PRENDERHAL_INTERFACE pRenderHal,
    MOS_COMMAND_BUFFER & CmdBuffer,
    PRENDERHAL_GENERIC_PROLOG_PARAMS pGenericPrologParams)
{
    VP_FUNC_CALL();

    MOS_STATUS                              eStatus = MOS_STATUS_SUCCESS;
    RENDERHAL_GENERIC_PROLOG_PARAMS_NEXT    genericPrologParams = {};

    genericPrologParams.bEnableMediaFrameTracking      = pGenericPrologParams->bEnableMediaFrameTracking;
    genericPrologParams.bMmcEnabled                    = pGenericPrologParams->bMmcEnabled;
    genericPrologParams.dwMediaFrameTrackingAddrOffset = pGenericPrologParams->dwMediaFrameTrackingAddrOffset;
    genericPrologParams.dwMediaFrameTrackingTag        = pGenericPrologParams->dwMediaFrameTrackingTag;
    genericPrologParams.presMediaFrameTrackingSurface  = pGenericPrologParams->presMediaFrameTrackingSurface;

    genericPrologParams.VEngineHintParams.BatchBufferCount = 2;
    //genericPrologParams.VEngineHintParams.resScalableBatchBufs[0] = CmdBuffer[0].OsResource;
    //genericPrologParams.VEngineHintParams.resScalableBatchBufs[1] = CmdBuffer[1].OsResource;
    genericPrologParams.VEngineHintParams.UsingFrameSplit = true;
    genericPrologParams.VEngineHintParams.UsingSFC = false;
    genericPrologParams.VEngineHintParams.EngineInstance[0] = 0;
    genericPrologParams.VEngineHintParams.EngineInstance[1] = 1;
    genericPrologParams.VEngineHintParams.NeedSyncWithPrevious = true;
    genericPrologParams.VEngineHintParams.SameEngineAsLastSubmission = true;

    pRenderHal->pOsInterface->VEEnable = false;

    // Initialize command buffer and insert prolog
    VP_RENDER_CHK_STATUS_RETURN(pRenderHal->pfnInitCommandBuffer(
        pRenderHal,
        &CmdBuffer,
        (PRENDERHAL_GENERIC_PROLOG_PARAMS)&genericPrologParams));

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

    VP_RENDER_CHK_NULL(pByte);

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
    VP_RENDER_CHK_STATUS(m_allocator->UnLock(&stmmSurface->osSurface->OsResource));

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

    // Decompress resource if surfaces need write from a un-align offset
    if ((m_renderTarget->osSurface->CompressionMode != MOS_MMC_DISABLED) && m_sfcRender->IsSFCUncompressedWriteNeeded(m_renderTarget))
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;
        MOS_SURFACE details = {};

        eStatus = m_osInterface->pfnGetResourceInfo(m_osInterface, &m_renderTarget->osSurface->OsResource, &details);

        if (eStatus != MOS_STATUS_SUCCESS)
        {
            VP_RENDER_ASSERTMESSAGE("Get SFC target surface resource info failed.");
        }

        if (!m_renderTarget->osSurface->OsResource.bUncompressedWriteNeeded)
        {
            eStatus = m_osInterface->pfnDecompResource(m_osInterface, &m_renderTarget->osSurface->OsResource);

            if (eStatus != MOS_STATUS_SUCCESS)
            {
                VP_RENDER_ASSERTMESSAGE("inplace decompression failed for sfc target.");
            }
            else
            {
                VP_RENDER_NORMALMESSAGE("inplace decompression enabled for sfc target RECT is not compression block align.");
                m_renderTarget->osSurface->OsResource.bUncompressedWriteNeeded = 1;
            }
        }
    }

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
        VP_RENDER_CHK_STATUS(SetVeboxSurfaceControlBits(
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
            VP_RENDER_CHK_STATUS(SetVeboxSurfaceControlBits(
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
            VP_RENDER_CHK_STATUS(SetVeboxSurfaceControlBits(
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
            VP_RENDER_CHK_STATUS(SetVeboxSurfaceControlBits(
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
            VP_RENDER_CHK_STATUS(SetVeboxSurfaceControlBits(
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
            VP_RENDER_CHK_STATUS(SetVeboxSurfaceControlBits(
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
            VP_RENDER_CHK_STATUS(SetVeboxSurfaceControlBits(
                &VeboxSurfCntlParams,
                (uint32_t *)&(veboxDiIecpCmdParams.StmmOutputSurfCtrl.Value)));
        }
    }

    veboxDiIecpCmdParams.pOsResStatisticsOutput         = &m_veboxPacketSurface.pStatisticsOutput->osSurface->OsResource;
    veboxDiIecpCmdParams.StatisticsOutputSurfCtrl.Value = m_surfMemCacheCtl->DnDi.StatisticsOutputSurfMemObjCtl;

    if (m_veboxPacketSurface.pLaceOrAceOrRgbHistogram->osSurface)
    {
        veboxDiIecpCmdParams.pOsResLaceOrAceOrRgbHistogram =
            &m_veboxPacketSurface.pLaceOrAceOrRgbHistogram->osSurface->OsResource;
        veboxDiIecpCmdParams.LaceOrAceOrRgbHistogramSurfCtrl.Value =
            m_surfMemCacheCtl->DnDi.LaceOrAceOrRgbHistogramSurfCtrl;
    }
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
    VP_VEBOX_SURFACE_STATE_CMD_PARAMS    VeboxSurfaceStateCmdParams;
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
    VP_RENDER_CHK_NULL_RETURN(pRenderData);

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
    VP_RENDER_CHK_NULL_RETURN(m_miItf);

    VP_RENDER_CHK_STATUS_RETURN(m_miItf->AddVeboxMMIOPrologCmd(CmdBuffer));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpVeboxCmdPacket::SetVeboxIndex(
    uint32_t                            dwVeboxIndex,
    uint32_t                            dwVeboxCount,
    uint32_t                            dwUsingSFC)
{
    VP_RENDER_CHK_NULL_RETURN(m_veboxItf);

    VP_RENDER_CHK_STATUS_RETURN(m_veboxItf->SetVeboxIndex(dwVeboxIndex, dwVeboxCount, dwUsingSFC));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpVeboxCmdPacket::SetVeboxState(
    PMOS_COMMAND_BUFFER         pCmdBufferInUse)

{
    VP_RENDER_CHK_NULL_RETURN(pCmdBufferInUse);
    VP_RENDER_CHK_NULL_RETURN(m_veboxItf);

    VP_RENDER_CHK_STATUS_RETURN(m_veboxItf->MHW_ADDCMD_F(VEBOX_STATE)(pCmdBufferInUse, nullptr));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpVeboxCmdPacket::SetVeboxSurfaces(
    PMOS_COMMAND_BUFFER                 pCmdBufferInUse,
    PMHW_VEBOX_SURFACE_STATE_CMD_PARAMS pMhwVeboxSurfaceStateCmdParams)

{
    VP_RENDER_CHK_NULL_RETURN(pCmdBufferInUse);
    VP_RENDER_CHK_NULL_RETURN(m_veboxItf);

    VP_RENDER_CHK_STATUS_RETURN(m_veboxItf->AddVeboxSurfaces(pCmdBufferInUse, pMhwVeboxSurfaceStateCmdParams));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpVeboxCmdPacket::SetVeboxDiIecp(
    PMOS_COMMAND_BUFFER                pCmdBufferInUse)

{
    VP_RENDER_CHK_NULL_RETURN(pCmdBufferInUse);
    VP_RENDER_CHK_NULL_RETURN(m_veboxItf);

    VP_RENDER_CHK_STATUS_RETURN(m_veboxItf->MHW_ADDCMD_F(VEB_DI_IECP)(pCmdBufferInUse, nullptr));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpVeboxCmdPacket::RenderVeboxCmd(
    MOS_COMMAND_BUFFER                      *CmdBuffer,
    VP_VEBOX_SURFACE_STATE_CMD_PARAMS    &VeboxSurfaceStateCmdParams,
    MHW_VEBOX_SURFACE_STATE_CMD_PARAMS      &MhwVeboxSurfaceStateCmdParams,
    MHW_MI_FLUSH_DW_PARAMS                  &FlushDwParams,
    PRENDERHAL_GENERIC_PROLOG_PARAMS        pGenericPrologParams)
{
    VP_FUNC_CALL();

    MOS_STATUS            eStatus = MOS_STATUS_SUCCESS;
    PRENDERHAL_INTERFACE  pRenderHal;
    PMOS_INTERFACE        pOsInterface;
    bool                  bDiVarianceEnable;
    const MHW_VEBOX_HEAP *pVeboxHeap     = nullptr;
    VpVeboxRenderData *   pRenderData    = GetLastExecRenderData();
    MOS_CONTEXT *         pOsContext     = nullptr;
    PMHW_MI_MMIOREGISTERS pMmioRegisters = nullptr;
    MOS_COMMAND_BUFFER    CmdBufferInUse;
    PMOS_COMMAND_BUFFER   pCmdBufferInUse = nullptr;
    uint32_t              curPipe         = 0;
    uint8_t               inputPipe       = 0;
    uint32_t              numPipe         = 1;
    bool                  bMultipipe      = false;

    VP_RENDER_CHK_NULL_RETURN(m_hwInterface);
    VP_RENDER_CHK_NULL_RETURN(m_hwInterface->m_renderHal);
    VP_RENDER_CHK_NULL_RETURN(m_miItf);
    VP_RENDER_CHK_NULL_RETURN(m_hwInterface->m_osInterface);
    VP_RENDER_CHK_NULL_RETURN(m_hwInterface->m_osInterface->pOsContext);
    VP_RENDER_CHK_NULL_RETURN(m_miItf->GetMmioRegisters());
    VP_RENDER_CHK_NULL_RETURN(pRenderData);
    VP_RENDER_CHK_NULL_RETURN(CmdBuffer);
    VP_RENDER_CHK_NULL_RETURN(m_veboxItf);

    pRenderHal      = m_hwInterface->m_renderHal;
    pOsInterface    = m_hwInterface->m_osInterface;
    pOsContext      = m_hwInterface->m_osInterface->pOsContext;
    pMmioRegisters  = m_miItf->GetMmioRegisters();
    pCmdBufferInUse = CmdBuffer;

    auto report      = (VpFeatureReport *)(m_hwInterface->m_reporting);
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

    VP_RENDER_CHK_STATUS_RETURN(SetupSurfaceStates(
        &VeboxSurfaceStateCmdParams));

    VP_RENDER_CHK_STATUS_RETURN(SetupVeboxState(veboxStateCmdParams));

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

        AddCommonOcaMessage(pCmdBufferInUse, (MOS_CONTEXT_HANDLE)pOsContext, pOsInterface, pRenderHal, pMmioRegisters);

        VP_RENDER_CHK_STATUS_RETURN(pRenderHal->pRenderHalPltInterface->AddPerfCollectStartCmd(pRenderHal, pOsInterface, pCmdBufferInUse));

        VP_RENDER_CHK_STATUS_RETURN(NullHW::StartPredicateNext(pOsInterface, m_miItf, pCmdBufferInUse));
#if (_DEBUG || _RELEASE_INTERNAL)
        VP_RENDER_CHK_STATUS_RETURN(StoreCSEngineIdRegMem(pCmdBufferInUse, pVeboxHeap));
#endif
        // Add compressible info of input/output surface to log
        if (this->m_currentSurface && VeboxSurfaceStateCmdParams.pSurfOutput)
        {
            std::string info   = "in_comps = " + std::to_string(int(this->m_currentSurface->osSurface->bCompressible)) + ", out_comps = " + std::to_string(int(VeboxSurfaceStateCmdParams.pSurfOutput->osSurface->bCompressible));
            const char *ocaLog = info.c_str();
            HalOcaInterfaceNext::TraceMessage(*pCmdBufferInUse, (MOS_CONTEXT_HANDLE)pOsContext, ocaLog, info.size());
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

            bool enableVeboxOutputSurf = false;
            if (m_vpUserFeatureControl)
            {
                enableVeboxOutputSurf = m_vpUserFeatureControl->IsVeboxOutputSurfEnabled();
            }

            VP_RENDER_CHK_STATUS_RETURN(m_sfcRender->SendSfcCmd(
                (enableVeboxOutputSurf || pRenderData->DI.bDeinterlace || pRenderData->DN.bDnEnabled),
                pCmdBufferInUse));
        }

        pRenderHal->pRenderHalPltInterface->OnDispatch(pRenderHal, pCmdBufferInUse, pOsInterface, pMmioRegisters);
        //HalOcaInterfaceNext::OnDispatch(*pCmdBufferInUse, *pOsContext, m_miItf, *pMmioRegisters);

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

        if (m_PacketCaps.enableSFCLinearOutputByTileConvert)
        {
            VP_RENDER_CHK_STATUS_RETURN(AddTileConvertStates(CmdBuffer, MhwVeboxSurfaceStateCmdParams));
            report->GetFeatures().sfcLinearOutputByTileConvert = true;
        }

        //---------------------------------
        // Write GPU Status Tag for Tag based synchronization
        //---------------------------------
#if !EMUL
        if (!pOsInterface->bEnableKmdMediaFrameTracking)
        {
            VP_RENDER_CHK_STATUS_RETURN(SendVecsStatusTag(
                pOsInterface,
                pCmdBufferInUse));
        }
#endif

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

        VP_RENDER_CHK_STATUS_RETURN(NullHW::StopPredicateNext(pOsInterface, m_miItf, pCmdBufferInUse));

        VP_RENDER_CHK_STATUS_RETURN(pRenderHal->pRenderHalPltInterface->AddPerfCollectEndCmd(pRenderHal, pOsInterface, pCmdBufferInUse));

#if (_DEBUG || _RELEASE_INTERNAL)
        VP_RENDER_CHK_STATUS_RETURN(StallBatchBuffer(pCmdBufferInUse));
#endif

        HalOcaInterfaceNext::On1stLevelBBEnd(*pCmdBufferInUse, *pOsInterface);

        if (pOsInterface->bNoParsingAssistanceInKmd)
        {
            VP_RENDER_CHK_STATUS_RETURN(m_miItf->AddMiBatchBufferEnd(pCmdBufferInUse, nullptr));
        }
        else if (RndrCommonIsMiBBEndNeeded(pOsInterface))
        {
            // Add Batch Buffer end command (HW/OS dependent)
            VP_RENDER_CHK_STATUS_RETURN(m_miItf->AddMiBatchBufferEnd(pCmdBufferInUse, nullptr));
        }

        if (bMultipipe)
        {
            VP_RENDER_CHK_STATUS_RETURN(scalability->ReturnCmdBuffer(pCmdBufferInUse));
        }
    }

    if (bMultipipe)
    {
        scalability->SetCurrentPipeIndex(inputPipe);
    }

    report->GetFeatures().VeboxScalability  = bMultipipe;

    MT_LOG2(MT_VP_HAL_RENDER_VE, MT_NORMAL, MT_VP_MHW_VE_SCALABILITY_EN, bMultipipe, MT_VP_MHW_VE_SCALABILITY_USE_SFC, m_IsSfcUsed);

    return eStatus;
}
// Meida copy has the same logic, when Format_R10G10B10A2/Format_B10G10R10A2, the output is AYUV, and in this WA there is corruption for these two format.
MOS_FORMAT VpVeboxCmdPacket::AdjustFormatForTileConvert(MOS_FORMAT format)
{
    if (format == Format_R10G10B10A2 ||
        format == Format_B10G10R10A2 ||
        format == Format_Y410 ||
        format == Format_Y210)
    {
        // RGB10 not supported without IECP. Re-map RGB10/RGB10 as AYUV
        // Y410/Y210 has HW issue. Remap to AYUV.
        return Format_AYUV;
    }
    else if (format == Format_A8)
    {
        return Format_P8;
    }
    else
    {
        return format;
    }
}

MOS_STATUS VpVeboxCmdPacket::AddTileConvertStates(MOS_COMMAND_BUFFER *CmdBuffer, MHW_VEBOX_SURFACE_STATE_CMD_PARAMS &MhwVeboxSurfaceStateCmdParams)
{
    auto &flushDwParams = m_miItf->MHW_GETPAR_F(MI_FLUSH_DW)();
    flushDwParams       = {};
    VP_RENDER_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_FLUSH_DW)(CmdBuffer));
    // Prepare Vebox_Surface_State, surface input/and output are the same but the compressed status.
    VP_RENDER_CHK_STATUS_RETURN(InitVeboxSurfaceStateCmdParamsForTileConvert(&MhwVeboxSurfaceStateCmdParams, m_renderTarget->osSurface, m_originalOutput->osSurface));

    //---------------------------------
    // Send CMD: Vebox_Surface_State
    //---------------------------------
    VP_RENDER_CHK_STATUS_RETURN(m_veboxItf->AddVeboxSurfaces(
        CmdBuffer,
        &MhwVeboxSurfaceStateCmdParams));

    HalOcaInterfaceNext::OnDispatch(*CmdBuffer, *m_osInterface, m_miItf, *m_miItf->GetMmioRegisters());

    //---------------------------------
    // Send CMD: Vebox_Tiling_Convert
    //---------------------------------
    VP_RENDER_CHK_STATUS_RETURN(m_veboxItf->AddVeboxTilingConvert(CmdBuffer, &MhwVeboxSurfaceStateCmdParams.SurfInput, &MhwVeboxSurfaceStateCmdParams.SurfOutput));
    flushDwParams = {};
    VP_RENDER_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_FLUSH_DW)(CmdBuffer));
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpVeboxCmdPacket::InitVeboxSurfaceStateCmdParamsForTileConvert(
    PMHW_VEBOX_SURFACE_STATE_CMD_PARAMS mhwVeboxSurfaceStateCmdParams,
    PMOS_SURFACE                        inputSurface,
    PMOS_SURFACE                        outputSurface)
{
    MOS_STATUS status               = MOS_STATUS_SUCCESS;
    bool       inputIsLinearBuffer  = false;
    bool       outputIsLinearBuffer = false;
    uint32_t   bpp                  = 1;
    uint32_t   inputWidth           = 0;
    uint32_t   outputWidth          = 0;

    VP_RENDER_CHK_NULL_RETURN(inputSurface);
    VP_RENDER_CHK_NULL_RETURN(outputSurface);
    VP_RENDER_CHK_NULL_RETURN(mhwVeboxSurfaceStateCmdParams);

    MOS_ZeroMemory(mhwVeboxSurfaceStateCmdParams, sizeof(*mhwVeboxSurfaceStateCmdParams));

    mhwVeboxSurfaceStateCmdParams->SurfInput.bActive = mhwVeboxSurfaceStateCmdParams->SurfOutput.bActive = true;
    mhwVeboxSurfaceStateCmdParams->SurfInput.dwBitDepth = mhwVeboxSurfaceStateCmdParams->SurfOutput.dwBitDepth = inputSurface->dwDepth;
    mhwVeboxSurfaceStateCmdParams->SurfInput.dwHeight                                                          = mhwVeboxSurfaceStateCmdParams->SurfOutput.dwHeight =
        MOS_MIN(inputSurface->dwHeight, ((outputSurface != nullptr) ? outputSurface->dwHeight : inputSurface->dwHeight));
    mhwVeboxSurfaceStateCmdParams->SurfInput.dwWidth = mhwVeboxSurfaceStateCmdParams->SurfOutput.dwWidth =
        MOS_MIN(inputSurface->dwWidth, ((outputSurface != nullptr) ? outputSurface->dwWidth : inputSurface->dwWidth));
    mhwVeboxSurfaceStateCmdParams->SurfInput.Format = mhwVeboxSurfaceStateCmdParams->SurfOutput.Format = AdjustFormatForTileConvert(inputSurface->Format);

    MOS_SURFACE inputDetails, outputDetails;
    MOS_ZeroMemory(&inputDetails, sizeof(inputDetails));
    MOS_ZeroMemory(&outputDetails, sizeof(outputDetails));
    inputDetails.Format  = Format_Invalid;
    outputDetails.Format = Format_Invalid;

    VP_RENDER_CHK_STATUS_RETURN(m_osInterface->pfnGetResourceInfo(
        m_osInterface,
        &inputSurface->OsResource,
        &inputDetails));

    VP_RENDER_CHK_STATUS_RETURN(m_osInterface->pfnGetResourceInfo(
        m_osInterface,
        &outputSurface->OsResource,
        &outputDetails));

    // Following settings are enabled only when outputSurface is availble
    inputIsLinearBuffer  = (inputDetails.dwHeight == 1) ? true : false;
    outputIsLinearBuffer = (outputDetails.dwHeight == 1) ? true : false;

    inputWidth  = inputSurface->dwWidth;
    outputWidth = outputSurface->dwWidth;

    if (inputIsLinearBuffer)
    {
        bpp = outputDetails.dwPitch / outputDetails.dwWidth;
        if (outputDetails.dwPitch % outputDetails.dwWidth != 0)
        {
            inputWidth = outputDetails.dwPitch / bpp;
        }
    }
    else if (outputIsLinearBuffer)
    {
        bpp = inputDetails.dwPitch / inputDetails.dwWidth;
        if (inputDetails.dwPitch % inputDetails.dwWidth != 0)
        {
            outputWidth = inputDetails.dwPitch / bpp;
        }
    }
    else
    {
        VP_RENDER_NORMALMESSAGE("2D to 2D, no need for bpp setting.");
    }

    if (inputSurface->dwPitch > 0 &&
        (inputSurface->Format == Format_P010 ||
            inputSurface->Format == Format_P016 ||
            inputSurface->Format == Format_NV12))
    {
        mhwVeboxSurfaceStateCmdParams->SurfInput.dwUYoffset = (!inputIsLinearBuffer) ? SURFACE_DW_UY_OFFSET(inputSurface) : inputSurface->dwHeight;

        mhwVeboxSurfaceStateCmdParams->SurfOutput.dwUYoffset = (!outputIsLinearBuffer) ? SURFACE_DW_UY_OFFSET(outputSurface) : outputSurface->dwHeight;
    }

    mhwVeboxSurfaceStateCmdParams->SurfInput.rcMaxSrc.left = mhwVeboxSurfaceStateCmdParams->SurfOutput.rcMaxSrc.left = 0;
    mhwVeboxSurfaceStateCmdParams->SurfInput.rcMaxSrc.right = mhwVeboxSurfaceStateCmdParams->SurfOutput.rcMaxSrc.right = (long)inputSurface->dwWidth;
    mhwVeboxSurfaceStateCmdParams->SurfInput.rcMaxSrc.top = mhwVeboxSurfaceStateCmdParams->SurfOutput.rcMaxSrc.top = 0;
    mhwVeboxSurfaceStateCmdParams->SurfInput.rcMaxSrc.bottom = mhwVeboxSurfaceStateCmdParams->SurfOutput.rcMaxSrc.bottom = (long)inputSurface->dwHeight;
    mhwVeboxSurfaceStateCmdParams->bOutputValid                                                                          = true;

    //double buffer resolve
    mhwVeboxSurfaceStateCmdParams->SurfInput.TileType         = inputSurface->TileType;
    mhwVeboxSurfaceStateCmdParams->SurfInput.TileModeGMM      = inputSurface->TileModeGMM;
    mhwVeboxSurfaceStateCmdParams->SurfInput.bGMMTileEnabled  = inputSurface->bGMMTileEnabled;
    mhwVeboxSurfaceStateCmdParams->SurfOutput.TileType        = outputSurface->TileType;
    mhwVeboxSurfaceStateCmdParams->SurfOutput.TileModeGMM     = outputSurface->TileModeGMM;
    mhwVeboxSurfaceStateCmdParams->SurfOutput.bGMMTileEnabled = outputSurface->bGMMTileEnabled;
    mhwVeboxSurfaceStateCmdParams->SurfInput.dwOffset         = inputSurface->dwOffset;
    mhwVeboxSurfaceStateCmdParams->SurfOutput.dwOffset        = outputSurface->dwOffset;

    // When surface is 1D but processed as 2D, fake a min(pitch, width) is needed as the pitch API passed may less surface width in 1D surface
    mhwVeboxSurfaceStateCmdParams->SurfInput.dwPitch              = (inputIsLinearBuffer) ? MOS_MIN(inputWidth * bpp, inputSurface->dwPitch) : inputSurface->dwPitch;
    mhwVeboxSurfaceStateCmdParams->SurfOutput.dwPitch             = (outputIsLinearBuffer) ? MOS_MIN(outputWidth * bpp, outputSurface->dwPitch) : outputSurface->dwPitch;
    mhwVeboxSurfaceStateCmdParams->SurfInput.pOsResource          = &(inputSurface->OsResource);
    mhwVeboxSurfaceStateCmdParams->SurfOutput.pOsResource         = &(outputSurface->OsResource);
    mhwVeboxSurfaceStateCmdParams->SurfInput.dwYoffset            = inputSurface->YPlaneOffset.iYOffset;
    mhwVeboxSurfaceStateCmdParams->SurfOutput.dwYoffset           = outputSurface->YPlaneOffset.iYOffset;
    mhwVeboxSurfaceStateCmdParams->SurfInput.dwCompressionFormat  = inputSurface->CompressionFormat;
    mhwVeboxSurfaceStateCmdParams->SurfOutput.dwCompressionFormat = outputSurface->CompressionFormat;
    mhwVeboxSurfaceStateCmdParams->SurfInput.CompressionMode      = inputSurface->CompressionMode;
    mhwVeboxSurfaceStateCmdParams->SurfOutput.CompressionMode     = outputSurface->CompressionMode;

    return status;
}


void VpVeboxCmdPacket::AddCommonOcaMessage(PMOS_COMMAND_BUFFER pCmdBufferInUse, MOS_CONTEXT_HANDLE pOsContext, PMOS_INTERFACE pOsInterface, PRENDERHAL_INTERFACE pRenderHal, PMHW_MI_MMIOREGISTERS pMmioRegisters)
{
    VP_FUNC_CALL();

    HalOcaInterfaceNext::On1stLevelBBStart(*pCmdBufferInUse, pOsContext, pOsInterface->CurrentGpuContextHandle, m_miItf, *pMmioRegisters);

    char ocaMsg[] = "VP APG Vebox Packet";
    HalOcaInterfaceNext::TraceMessage(*pCmdBufferInUse, pOsContext, ocaMsg, sizeof(ocaMsg));

    VpVeboxRenderData *pRenderData = GetLastExecRenderData();
    if (pRenderData)
    {
        MHW_VEBOX_IECP_PARAMS IecpParams = pRenderData->GetIECPParams();
        if (pRenderData->IECP.STE.bStdEnabled && IecpParams.ColorPipeParams.StdParams.param)
        {
            char ocaMsg_std[] = "Customized STD state is used";
            HalOcaInterfaceNext::TraceMessage(*pCmdBufferInUse, pOsContext, ocaMsg_std, sizeof(ocaMsg_std));
        }
    }

    HalOcaInterfaceNext::TraceOcaSkuValue(*pCmdBufferInUse, *pOsInterface);

    // Add vphal param to log.
    HalOcaInterfaceNext::DumpVphalParam(*pCmdBufferInUse, pOsContext, pRenderHal->pVphalOcaDumper);

    if (m_vpUserFeatureControl)
    {
        HalOcaInterfaceNext::DumpVpUserFeautreControlInfo(*pCmdBufferInUse, pOsContext, m_vpUserFeatureControl->GetOcaFeautreControlInfo());
    }

}

MOS_STATUS VpVeboxCmdPacket::InitVeboxSurfaceStateCmdParams(
    PVP_VEBOX_SURFACE_STATE_CMD_PARAMS    pVpHalVeboxSurfaceStateCmdParams,
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
    VP_RENDER_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_FLUSH_DW)(pCmdBuffer));

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
#if USE_VP_DEBUG_TOOL
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
    VP_RENDER_CHK_STATUS_RETURN(m_allocator->GetSurfaceInfo(&driverResource, info));
    VP_RENDER_CHK_STATUS_RETURN(m_allocator->GetSurfaceInfo(&kernelResource, info));


    VP_SURFACE_DUMP(debuginterface,
        &kernelResource,
        counter,
        0,
        VPHAL_DUMP_TYPE_VEBOX_DRIVERHEAP,
        VPHAL_SURF_DUMP_DDI_VP_BLT);

    VP_SURFACE_DUMP(debuginterface,
        &kernelResource,
        counter,
        0,
        VPHAL_DUMP_TYPE_VEBOX_KERNELHEAP,
        VPHAL_SURF_DUMP_DDI_VP_BLT);

    counter++;
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

    if (nullptr == m_originalOutput)
    {
        m_originalOutput = m_allocator->AllocateVpSurface();
        VP_CHK_SPACE_NULL_RETURN(m_originalOutput);
    }
    else
    {
        m_originalOutput->Clean();
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

    if (m_packetResourcesPrepared)
    {
        VP_RENDER_NORMALMESSAGE("Resource Prepared, skip this time");
        return MOS_STATUS_SUCCESS;
    }

    VP_RENDER_CHK_STATUS_RETURN(SetupIndirectStates());

    VP_RENDER_CHK_STATUS_RETURN(UpdateVeboxStates());

    if (m_veboxItf)
    {
        const MHW_VEBOX_HEAP *veboxHeap = nullptr;

        VP_RENDER_CHK_STATUS_RETURN(m_veboxItf->GetVeboxHeapInfo(&veboxHeap));
        VP_RENDER_CHK_NULL_RETURN(veboxHeap);

        m_veboxHeapCurState = veboxHeap->uiCurState;
    }

    m_packetResourcesPrepared = true;

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

MOS_STATUS VpVeboxCmdPacket::SetUpdatedExecuteResource(
    VP_SURFACE                          *inputSurface,
    VP_SURFACE                          *outputSurface,
    VP_SURFACE                          *previousSurface,
    VP_SURFACE_SETTING                  &surfSetting)
{
    VP_FUNC_CALL();
    VP_RENDER_CHK_NULL_RETURN(inputSurface);
    VP_RENDER_CHK_NULL_RETURN(outputSurface);
    VP_RENDER_CHK_NULL_RETURN(inputSurface->osSurface);
    VP_RENDER_CHK_NULL_RETURN(outputSurface->osSurface);
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

    // Adjust boundary for statistics surface block
    VP_RENDER_CHK_STATUS_RETURN(AdjustBlockStatistics());

    if (m_PacketCaps.bSFC)
    {
        VP_RENDER_CHK_STATUS_RETURN(SetSfcMmcParams());
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
    m_packetResourcesPrepared = false;

    VP_RENDER_CHK_NULL_RETURN(pRenderData);
    VP_RENDER_CHK_NULL_RETURN(inputSurface);
    VP_RENDER_CHK_NULL_RETURN(outputSurface);
    VP_RENDER_CHK_STATUS_RETURN(pRenderData->Init());

    m_PacketCaps      = packetCaps;
    VP_RENDER_NORMALMESSAGE("m_PacketCaps %x", m_PacketCaps.value);

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
    m_veboxPacketSurface.pInnerTileConvertInput     = GetSurface(SurfaceTypeInnerTileConvertInput);
    // Set current src = current primary input
    if (m_veboxPacketSurface.pInnerTileConvertInput)
    {
        VP_PUBLIC_CHK_STATUS_RETURN(m_allocator->CopyVpSurface(*m_renderTarget, *m_veboxPacketSurface.pInnerTileConvertInput));

        VP_PUBLIC_CHK_STATUS_RETURN(m_allocator->CopyVpSurface(*m_originalOutput, *outputSurface));
    }
    else
    {
        VP_PUBLIC_CHK_STATUS_RETURN(m_allocator->CopyVpSurface(*m_renderTarget, *outputSurface));
    }

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

    // use the StateIndex which was saved in PrepareState() to resolve the 2pass sfc mismatch issue
    if (m_veboxItf)
    {
        m_veboxItf->SetVeboxHeapStateIndex(m_veboxHeapCurState);
    }

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
    VpCmdPacket(task, hwInterface, allocator, mmc, VP_PIPELINE_PACKET_VEBOX),
    VpVeboxCmdPacketBase(task, hwInterface, allocator, mmc)
{
    VP_PUBLIC_CHK_NULL_NO_STATUS_RETURN(hwInterface);
    VP_PUBLIC_CHK_NULL_NO_STATUS_RETURN(hwInterface->m_vpPlatformInterface);
    m_veboxItf = hwInterface->m_vpPlatformInterface->GetMhwVeboxItf();
    m_miItf = hwInterface->m_vpPlatformInterface->GetMhwMiItf();
    m_vpUserFeatureControl = hwInterface->m_userFeatureControl;
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
    m_allocator->DestroyVpSurface(m_originalOutput);
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
        mhw::vebox::MHW_VEBOX_CHROMA_PARAMS veboxChromaPar = {};

        VP_RENDER_CHK_STATUS_RETURN(MOS_SecureMemcpy(&veboxChromaPar,
            sizeof(veboxChromaPar),
            &veboxChromaParams,
            sizeof(veboxChromaParams)));
        VP_RENDER_CHK_STATUS_RETURN(m_veboxItf->SetVeboxChromaParams(&veboxChromaPar));
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
        VP_PUBLIC_NORMALMESSAGE("IecpState is added. ace %d, lace %d, becsc %d, fecsc %d, tcc %d, ste %d, procamp %d, std %d, gamut % d",
            pRenderData->IECP.ACE.bAceEnabled,
            pRenderData->IECP.LACE.bLaceEnabled,
            pRenderData->IECP.BeCSC.bBeCSCEnabled,
            pRenderData->IECP.FeCSC.bFeCSCEnabled,
            pRenderData->IECP.TCC.bTccEnabled,
            pRenderData->IECP.STE.bSteEnabled,
            pRenderData->IECP.PROCAMP.bProcampEnabled,
            pRenderData->IECP.STE.bStdEnabled,
            pRenderData->IECP.CGC.bCGCEnabled);

        return m_veboxItf->SetVeboxIecpState(&pRenderData->GetIECPParams());
    }
    else
    {
        // BeCsc may not needed for AlphaFromStateSelect == 1 case.
        // Refer to IsBeCscNeededForAlphaFill for detail.
        VP_PUBLIC_NORMALMESSAGE("IecpState is not added with AlphaFromStateSelect %d",
            pRenderData->GetIECPParams().bAlphaEnable);
        return m_veboxItf->SetDisableHistogram(&pRenderData->GetIECPParams());
    }
}

bool VpVeboxCmdPacket::IsVeboxGamutStateNeeded()
{
    VP_FUNC_CALL();

    VpVeboxRenderData *renderData = GetLastExecRenderData();

    if (renderData)
    {
        return renderData->HDR3DLUT.bHdr3DLut || renderData->IECP.CGC.bCGCEnabled;
    }
    else
    {
        return false;
    }
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
    const VP_VEBOX_SURFACE_STATE_CMD_PARAMS  &VeboxSurfaceStateCmdParams)
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
            *pPerfTag = VPHAL_VEBOX_RGB32;
            break;

        case Format_AYUV:
            *pPerfTag = VPHAL_VEBOX_AYUV;
            break;

        case Format_A16B16G16R16:
        case Format_A16R16G16B16:
        case Format_A16B16G16R16F:
        case Format_A16R16G16B16F:
            *pPerfTag = VPHAL_VEBOX_RGB64;
            break;

        default:
            VP_RENDER_ASSERTMESSAGE("Format Not found.");
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
                    case Format_A16B16G16R16F:
                    case Format_A16R16G16B16F:
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
                        break;
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
                    case Format_A16B16G16R16F:
                    case Format_A16R16G16B16F:
                        *pPerfTag = VPHAL_NONE;
                        break;
                    default:
                        VP_RENDER_ASSERTMESSAGE("Output Format Not found.");
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
                    case Format_A16B16G16R16F:
                    case Format_A16R16G16B16F:
                        *pPerfTag = VPHAL_NONE;
                        break;
                    default:
                        VP_RENDER_ASSERTMESSAGE("Output Format Not found.");
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
                    case Format_A16B16G16R16F:
                    case Format_A16R16G16B16F:
                        *pPerfTag = VPHAL_NONE;
                        break;
                    default:
                        VP_RENDER_ASSERTMESSAGE("Output Format Not found.");
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

//!
//! \brief    Vebox get statistics surface base
//! \details  Calculate address of statistics surface address based on the
//!           functions which were enabled in the previous call.
//! \param    uint8_t* pStat
//!           [in] Pointer to Statistics surface
//! \param    uint8_t* * pStatSlice0Base
//!           [out] Statistics surface Slice 0 base pointer
//! \param    uint8_t* * pStatSlice1Base
//!           [out] Statistics surface Slice 1 base pointer
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VpVeboxCmdPacket::GetStatisticsSurfaceBase(
    uint8_t  *pStat,
    uint8_t **pStatSlice0Base,
    uint8_t **pStatSlice1Base)
{
    VP_FUNC_CALL();

    int32_t    iOffsetSlice0, iOffsetSlice1;
    MOS_STATUS eStatus;

    eStatus = MOS_STATUS_UNKNOWN;

    // Calculate the offsets of Slice0 and Slice1
    VP_RENDER_CHK_STATUS(VpVeboxCmdPacket::GetStatisticsSurfaceOffsets(
        &iOffsetSlice0,
        &iOffsetSlice1));

    *pStatSlice0Base = pStat + iOffsetSlice0;  // Slice 0 current frame
    *pStatSlice1Base = pStat + iOffsetSlice1;  // Slice 1 current frame

finish:
    return eStatus;
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

MOS_STATUS VpVeboxCmdPacket::GNELumaConsistentCheck(
    uint32_t &dwGNELuma,
    uint32_t *pStatSlice0GNEPtr,
    uint32_t *pStatSlice1GNEPtr)
{
    uint32_t dwGNEChromaU = 0, dwGNEChromaV = 0;
    uint32_t dwGNECountChromaU = 0, dwGNECountChromaV = 0;
    VP_PUBLIC_CHK_NULL_RETURN(pStatSlice0GNEPtr);
    VP_PUBLIC_CHK_NULL_RETURN(pStatSlice1GNEPtr);

    // Combine the GNE in slice0 and slice1 to generate the global GNE and Count
    dwGNEChromaU = pStatSlice0GNEPtr[1] + pStatSlice1GNEPtr[1];
    dwGNEChromaV = pStatSlice0GNEPtr[2] + pStatSlice1GNEPtr[2];

    dwGNECountChromaU = pStatSlice0GNEPtr[4] + pStatSlice1GNEPtr[4];
    dwGNECountChromaV = pStatSlice0GNEPtr[5] + pStatSlice1GNEPtr[5];

    // Validate GNE
    if (dwGNEChromaU == 0xFFFFFFFF || dwGNECountChromaU == 0xFFFFFFFF ||
        dwGNEChromaV == 0xFFFFFFFF || dwGNECountChromaV == 0xFFFFFFFF)
    {
        VP_RENDER_ASSERTMESSAGE("Incorrect GNE / GNE count.");
        return MOS_STATUS_UNKNOWN;
    }

    dwGNEChromaU = dwGNEChromaU * 100 / (dwGNECountChromaU + 1);
    dwGNEChromaV = dwGNEChromaV * 100 / (dwGNECountChromaV + 1);
    VP_RENDER_NORMALMESSAGE("Consistent Check: dwGNEChromaU %d  dwGNEChromaV %d", dwGNEChromaU, dwGNEChromaV);
    if ((dwGNEChromaU < NOSIE_GNE_CHROMA_THRESHOLD) &&
        (dwGNEChromaV < NOSIE_GNE_CHROMA_THRESHOLD) &&
        (dwGNEChromaU != 0) &&
        (dwGNEChromaV != 0) &&
        (dwGNELuma > NOSIE_GNE_LUMA_THRESHOLD))
    {
        dwGNELuma = dwGNELuma >> 2;
    }

    return MOS_STATUS_SUCCESS;
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

MOS_STATUS VpVeboxCmdPacket::UpdateVeboxStates()
{
    VP_FUNC_CALL();
    MOS_STATUS         eStatus;
    uint8_t           *pStat = nullptr;
    uint8_t           *pStatSlice0Base, *pStatSlice1Base;
    uint32_t           dwQuery = 0;
    MOS_LOCK_PARAMS    LockFlags;
    VpVeboxRenderData *renderData = GetLastExecRenderData();

    VP_PUBLIC_CHK_NULL_RETURN(renderData);
    VP_PUBLIC_CHK_NULL_RETURN(m_veboxPacketSurface.pStatisticsOutput);
    VP_PUBLIC_CHK_NULL_RETURN(m_veboxPacketSurface.pStatisticsOutput->osSurface);
    VP_PUBLIC_CHK_NULL_RETURN(m_sfcRender);

    eStatus = MOS_STATUS_SUCCESS;

    if (!renderData->DN.bHvsDnEnabled)
    {
        // no need to update, direct return.
        return MOS_STATUS_SUCCESS;
    }

    // Update DN State in CPU
    MOS_ZeroMemory(&LockFlags, sizeof(MOS_LOCK_PARAMS));
    LockFlags.ReadOnly = 1;

    // Get Statistic surface
    pStat = (uint8_t *)m_allocator->Lock(
        &m_veboxPacketSurface.pStatisticsOutput->osSurface->OsResource,
        &LockFlags);

    VP_PUBLIC_CHK_NULL_RETURN(pStat);

    VP_RENDER_CHK_STATUS_RETURN(GetStatisticsSurfaceBase(
        pStat,
        &pStatSlice0Base,
        &pStatSlice1Base));

    // Query platform dependent GNE offset
    VP_RENDER_CHK_STATUS_RETURN(QueryStatLayoutGNE(
        VEBOX_STAT_QUERY_GNE_OFFEST,
        &dwQuery,
        pStatSlice0Base,
        pStatSlice1Base));

#if VEBOX_AUTO_DENOISE_SUPPORTED
    VP_RENDER_CHK_STATUS_RETURN(UpdateDnHVSParameters(
        (uint32_t *)(pStatSlice0Base + dwQuery),
        (uint32_t *)(pStatSlice1Base + dwQuery)));
#endif

    // unlock the statistic surface
    VP_RENDER_CHK_STATUS_RETURN(m_allocator->UnLock(
        &m_veboxPacketSurface.pStatisticsOutput->osSurface->OsResource));
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

        VPHAL_SET_SURF_MEMOBJCTL(pSettings->DnDi.CurrentInputSurfMemObjCtl,     MOS_HW_RESOURCE_USAGE_VP_INPUT_PICTURE_FF);
        VPHAL_SET_SURF_MEMOBJCTL(pSettings->DnDi.PreviousInputSurfMemObjCtl,    MOS_HW_RESOURCE_USAGE_VP_INPUT_PICTURE_FF);
        VPHAL_SET_SURF_MEMOBJCTL(pSettings->DnDi.STMMInputSurfMemObjCtl,        MOS_HW_RESOURCE_USAGE_VP_INPUT_PICTURE_FF);
        VPHAL_SET_SURF_MEMOBJCTL(pSettings->DnDi.STMMOutputSurfMemObjCtl,       MOS_HW_RESOURCE_USAGE_VP_OUTPUT_PICTURE_FF);
        VPHAL_SET_SURF_MEMOBJCTL(pSettings->DnDi.DnOutSurfMemObjCtl,            MOS_HW_RESOURCE_USAGE_VP_OUTPUT_PICTURE_FF);

        if (packetCaps.bVebox && !packetCaps.bSFC && !packetCaps.bRender)
        {
            // Disable cache for output surface in vebox only condition
            VPHAL_SET_SURF_MEMOBJCTL(pSettings->DnDi.CurrentOutputSurfMemObjCtl, MOS_HW_RESOURCE_USAGE_VP_OUTPUT_PICTURE_FF);
        }
        else
        {
            VPHAL_SET_SURF_MEMOBJCTL(pSettings->DnDi.CurrentOutputSurfMemObjCtl, MOS_HW_RESOURCE_USAGE_VP_OUTPUT_PICTURE_FF);
        }

        VPHAL_SET_SURF_MEMOBJCTL(pSettings->DnDi.StatisticsOutputSurfMemObjCtl,     MOS_HW_RESOURCE_USAGE_VP_OUTPUT_PICTURE_FF);
        VPHAL_SET_SURF_MEMOBJCTL(pSettings->DnDi.AlphaOrVignetteSurfMemObjCtl,      MOS_HW_RESOURCE_USAGE_VP_INTERNAL_READ_WRITE_FF);
        VPHAL_SET_SURF_MEMOBJCTL(pSettings->DnDi.LaceOrAceOrRgbHistogramSurfCtrl,   MOS_HW_RESOURCE_USAGE_VP_INTERNAL_READ_WRITE_FF);
        VPHAL_SET_SURF_MEMOBJCTL(pSettings->DnDi.SkinScoreSurfMemObjCtl,            MOS_HW_RESOURCE_USAGE_VP_INTERNAL_READ_WRITE_FF);
        VPHAL_SET_SURF_MEMOBJCTL(pSettings->DnDi.LaceLookUpTablesSurfMemObjCtl,     MOS_HW_RESOURCE_USAGE_VP_INTERNAL_READ_WRITE_FF);
        VPHAL_SET_SURF_MEMOBJCTL(pSettings->DnDi.Vebox3DLookUpTablesSurfMemObjCtl,  MOS_HW_RESOURCE_USAGE_VP_INTERNAL_READ_WRITE_FF);
    }
    else
    {
        pSettings->DnDi.bL3CachingEnabled = false;

        VPHAL_SET_SURF_MEMOBJCTL(pSettings->DnDi.CurrentInputSurfMemObjCtl,             MOS_HW_RESOURCE_USAGE_VP_INPUT_PICTURE_FF);
        VPHAL_SET_SURF_MEMOBJCTL(pSettings->DnDi.PreviousInputSurfMemObjCtl,            MOS_HW_RESOURCE_USAGE_VP_INPUT_PICTURE_FF);
        VPHAL_SET_SURF_MEMOBJCTL(pSettings->DnDi.STMMInputSurfMemObjCtl,                MOS_HW_RESOURCE_USAGE_VP_INPUT_PICTURE_FF);
        VPHAL_SET_SURF_MEMOBJCTL(pSettings->DnDi.STMMOutputSurfMemObjCtl,               MOS_HW_RESOURCE_USAGE_VP_OUTPUT_PICTURE_FF);
        VPHAL_SET_SURF_MEMOBJCTL(pSettings->DnDi.DnOutSurfMemObjCtl,                    MOS_HW_RESOURCE_USAGE_VP_OUTPUT_PICTURE_FF);
        VPHAL_SET_SURF_MEMOBJCTL(pSettings->DnDi.CurrentOutputSurfMemObjCtl,            MOS_HW_RESOURCE_USAGE_VP_OUTPUT_PICTURE_FF);
        VPHAL_SET_SURF_MEMOBJCTL(pSettings->DnDi.StatisticsOutputSurfMemObjCtl,         MOS_HW_RESOURCE_USAGE_VP_OUTPUT_PICTURE_FF);
        VPHAL_SET_SURF_MEMOBJCTL(pSettings->DnDi.AlphaOrVignetteSurfMemObjCtl,          MOS_HW_RESOURCE_USAGE_VP_INTERNAL_READ_WRITE_FF);
        VPHAL_SET_SURF_MEMOBJCTL(pSettings->DnDi.LaceOrAceOrRgbHistogramSurfCtrl,       MOS_HW_RESOURCE_USAGE_VP_INTERNAL_READ_WRITE_FF);
        VPHAL_SET_SURF_MEMOBJCTL(pSettings->DnDi.SkinScoreSurfMemObjCtl,                MOS_HW_RESOURCE_USAGE_VP_INTERNAL_READ_WRITE_FF);
        VPHAL_SET_SURF_MEMOBJCTL(pSettings->DnDi.LaceLookUpTablesSurfMemObjCtl,         MOS_HW_RESOURCE_USAGE_VP_INTERNAL_READ_WRITE_FF);
        VPHAL_SET_SURF_MEMOBJCTL(pSettings->DnDi.Vebox3DLookUpTablesSurfMemObjCtl,      MOS_HW_RESOURCE_USAGE_VP_INTERNAL_READ_WRITE_FF);
    }

    if (pSettings->bLace)
    {
        VPHAL_SET_SURF_MEMOBJCTL(pSettings->Lace.FrameHistogramSurfaceMemObjCtl,        MOS_HW_RESOURCE_USAGE_VP_INTERNAL_READ_WRITE_FF);
        VPHAL_SET_SURF_MEMOBJCTL(pSettings->Lace.AggregatedHistogramSurfaceMemObjCtl,   MOS_HW_RESOURCE_USAGE_VP_INTERNAL_READ_WRITE_FF);
        VPHAL_SET_SURF_MEMOBJCTL(pSettings->Lace.StdStatisticsSurfaceMemObjCtl,         MOS_HW_RESOURCE_USAGE_VP_INTERNAL_READ_WRITE_FF);
        VPHAL_SET_SURF_MEMOBJCTL(pSettings->Lace.PwlfInSurfaceMemObjCtl,                MOS_HW_RESOURCE_USAGE_VP_INPUT_PICTURE_FF);
        VPHAL_SET_SURF_MEMOBJCTL(pSettings->Lace.PwlfOutSurfaceMemObjCtl,               MOS_HW_RESOURCE_USAGE_VP_OUTPUT_PICTURE_FF);
        VPHAL_SET_SURF_MEMOBJCTL(pSettings->Lace.WeitCoefSurfaceMemObjCtl,              MOS_HW_RESOURCE_USAGE_VP_INTERNAL_READ_WRITE_FF);
    }
    else
    {
        VPHAL_SET_SURF_MEMOBJCTL(pSettings->Lace.FrameHistogramSurfaceMemObjCtl,            MOS_HW_RESOURCE_USAGE_VP_INTERNAL_READ_WRITE_FF);
        VPHAL_SET_SURF_MEMOBJCTL(pSettings->Lace.AggregatedHistogramSurfaceMemObjCtl,       MOS_HW_RESOURCE_USAGE_VP_INTERNAL_READ_WRITE_FF);
        VPHAL_SET_SURF_MEMOBJCTL(pSettings->Lace.StdStatisticsSurfaceMemObjCtl,             MOS_HW_RESOURCE_USAGE_VP_INTERNAL_READ_WRITE_FF);
        VPHAL_SET_SURF_MEMOBJCTL(pSettings->Lace.PwlfInSurfaceMemObjCtl,                    MOS_HW_RESOURCE_USAGE_VP_INPUT_PICTURE_FF);
        VPHAL_SET_SURF_MEMOBJCTL(pSettings->Lace.PwlfOutSurfaceMemObjCtl,                   MOS_HW_RESOURCE_USAGE_VP_OUTPUT_PICTURE_FF);
        VPHAL_SET_SURF_MEMOBJCTL(pSettings->Lace.WeitCoefSurfaceMemObjCtl,                  MOS_HW_RESOURCE_USAGE_VP_INTERNAL_READ_WRITE_FF);
        VPHAL_SET_SURF_MEMOBJCTL(pSettings->Lace.GlobalToneMappingCurveLUTSurfaceMemObjCtl, MOS_HW_RESOURCE_USAGE_VP_INTERNAL_READ_WRITE_FF);
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

