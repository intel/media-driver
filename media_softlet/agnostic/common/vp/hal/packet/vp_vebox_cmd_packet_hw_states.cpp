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
//! \file     vp_vebox_cmd_packet_hw_states.cpp
//! \brief    vebox packet for hw states which used in by mediapipline.
//! \details  vebox packet provide the structures and generate the cmd buffer which mediapipline will used.
//!

#include "vp_vebox_cmd_packet.h"

#define SURFACE_DW_UY_OFFSET(pSurface) \
    ((pSurface) != nullptr ? ((pSurface)->UPlaneOffset.iSurfaceOffset - (pSurface)->dwOffset) / (pSurface)->dwPitch + (pSurface)->UPlaneOffset.iYOffset : 0)

#define SURFACE_DW_VY_OFFSET(pSurface) \
    ((pSurface) != nullptr ? ((pSurface)->VPlaneOffset.iSurfaceOffset - (pSurface)->dwOffset) / (pSurface)->dwPitch + (pSurface)->VPlaneOffset.iYOffset : 0)

namespace vp
{
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

MOS_STATUS VpVeboxCmdPacket::SetupHDRUnifiedForHDR(mhw::vebox::VEBOX_STATE_PAR &veboxStateCmdParams)
{
    PMHW_VEBOX_MODE    pVeboxMode   = nullptr;

    pVeboxMode   = &veboxStateCmdParams.VeboxMode;
 
    pVeboxMode->Hdr1K1DLut = true;
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
    pLUT3D->InterpolationMethod        = Get3DLutInterpolationMethod(VPHAL_3DLUT_INTERPOLATION_DEFAULT);
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
    pLUT3D->InterpolationMethod           = external3DLutParams->InterpolationMethod;
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

MOS_STATUS VpVeboxCmdPacket::SetVeboxDiIecp(
    PMOS_COMMAND_BUFFER                pCmdBufferInUse)

{
    VP_RENDER_CHK_NULL_RETURN(pCmdBufferInUse);
    VP_RENDER_CHK_NULL_RETURN(m_veboxItf);

    VP_RENDER_CHK_STATUS_RETURN(m_veboxItf->MHW_ADDCMD_F(VEB_DI_IECP)(pCmdBufferInUse, nullptr));

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

MOS_STATUS VpVeboxCmdPacket::SetVeboxProCmd(
    MOS_COMMAND_BUFFER*   CmdBuffer)
{
    VP_RENDER_CHK_NULL_RETURN(m_miItf);

    VP_RENDER_CHK_STATUS_RETURN(m_miItf->AddVeboxMMIOPrologCmd(CmdBuffer));

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

MOS_STATUS VpVeboxCmdPacket::SetVeboxSurfaces(
    PMOS_COMMAND_BUFFER                 pCmdBufferInUse,
    PMHW_VEBOX_SURFACE_STATE_CMD_PARAMS pMhwVeboxSurfaceStateCmdParams)

{
    VP_RENDER_CHK_NULL_RETURN(pCmdBufferInUse);
    VP_RENDER_CHK_NULL_RETURN(m_veboxItf);

    VP_RENDER_CHK_STATUS_RETURN(m_veboxItf->AddVeboxSurfaces(pCmdBufferInUse, pMhwVeboxSurfaceStateCmdParams));

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
}