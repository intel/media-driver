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
//! \file     renderhal_linux.cpp
//! \brief    Linux-specific functions in Render Engine state heap manager for VP and CM
//! \details  Linux-specific Render Engine state heap management interfaces
//!
#include "mos_os.h"
#include "renderhal.h"

void RenderHal_SetupPrologParams(
    PRENDERHAL_INTERFACE              renderHal,
    RENDERHAL_GENERIC_PROLOG_PARAMS  *prologParams,
    PMOS_RESOURCE                     osResource,
    uint32_t                          offset,
    uint32_t                          tag)
{
    return;
}

//!
//! \brief    Get Surface Info from OsResource
//! \details  Update surface info in PRENDERHAL_SURFACE based on allocated OsResource
//! \param    PMOS_INTERFACE pOsInterface
//!           [in] Pointer to MOS_INTERFACE
//! \param    PRENDERHAL_GET_SURFACE_INFO pInfo
//!           [in] Pointer to RENDERHAL_GET_SURFACE_INFO
//! \param    PMOS_SURFACE pSurface
//!           [in/out] Pointer to PMOS_SURFACE
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS RenderHal_GetSurfaceInfo(
    PMOS_INTERFACE               pOsInterface,
    PRENDERHAL_GET_SURFACE_INFO  pInfo,
    PMOS_SURFACE                 pSurface)
{

    MOS_STATUS             eStatus = MOS_STATUS_UNKNOWN;

    MHW_RENDERHAL_ASSERT(pOsInterface);
    MHW_RENDERHAL_ASSERT(pSurface);

    PMOS_RESOURCE       pResource = &pSurface->OsResource;
    MOS_SURFACE         ResDetails;

    MHW_RENDERHAL_ASSERT(!Mos_ResourceIsNull(&pSurface->OsResource));
    MOS_ZeroMemory(&ResDetails, sizeof(MOS_SURFACE));
    ResDetails.dwArraySlice = pInfo->ArraySlice;
    ResDetails.dwMipSlice   = pInfo->MipSlice;
    ResDetails.S3dChannel   = pInfo->S3dChannel;
    ResDetails.Format       = pSurface->Format;
    MHW_RENDERHAL_CHK_STATUS(pOsInterface->pfnGetResourceInfo(pOsInterface, &pSurface->OsResource, &ResDetails));

    if (ResDetails.Format == Format_420O)
    {
        ResDetails.Format = Format_NV12;
    }

    // Get resource information
    pSurface->dwWidth         = ResDetails.dwWidth;
    pSurface->dwHeight        = ResDetails.dwHeight;
    pSurface->dwPitch         = ResDetails.dwPitch;
    pSurface->dwSlicePitch    = ResDetails.dwSlicePitch;
    pSurface->dwQPitch        = ResDetails.dwQPitch;
    pSurface->dwDepth         = ResDetails.dwDepth;
    pSurface->TileType        = ResDetails.TileType;
    pSurface->TileModeGMM     = ResDetails.TileModeGMM;
    pSurface->bGMMTileEnabled = ResDetails.bGMMTileEnabled;
    pSurface->bOverlay        = ResDetails.bOverlay;
    pSurface->bFlipChain      = ResDetails.bFlipChain;
    pSurface->Format          = ResDetails.Format;
    pSurface->bCompressible   = ResDetails.bCompressible;
    pSurface->bIsCompressed   = ResDetails.bIsCompressed;
    pSurface->CompressionMode = ResDetails.CompressionMode;

    MHW_RENDERHAL_CHK_STATUS(pOsInterface->pfnGetMemoryCompressionMode(pOsInterface,
        &pSurface->OsResource, &pSurface->MmcState));

        MHW_RENDERHAL_CHK_STATUS(pOsInterface->pfnGetMemoryCompressionFormat(pOsInterface,
        &pSurface->OsResource, &pSurface->CompressionFormat));

    if (IS_RGB32_FORMAT(pSurface->Format) ||
        IS_RGB16_FORMAT(pSurface->Format) ||
        IS_RGB128_FORMAT(pSurface->Format)||
        pSurface->Format == Format_RGB    ||
        pSurface->Format == Format_Y410)
    {
        pSurface->dwOffset                    = ResDetails.RenderOffset.RGB.BaseOffset;
        pSurface->YPlaneOffset.iSurfaceOffset = ResDetails.RenderOffset.RGB.BaseOffset;
        pSurface->YPlaneOffset.iXOffset       = ResDetails.RenderOffset.RGB.XOffset;
        pSurface->YPlaneOffset.iYOffset       = ResDetails.RenderOffset.RGB.YOffset;
    }
    else // YUV or PL3_RGB
    {
        // Get Y plane information (plane offset, X/Y offset)
        pSurface->dwOffset                        = ResDetails.RenderOffset.YUV.Y.BaseOffset;
        pSurface->YPlaneOffset.iSurfaceOffset     = ResDetails.RenderOffset.YUV.Y.BaseOffset;
        pSurface->YPlaneOffset.iXOffset           = ResDetails.RenderOffset.YUV.Y.XOffset;
        pSurface->YPlaneOffset.iYOffset           = ResDetails.RenderOffset.YUV.Y.YOffset;
        pSurface->YPlaneOffset.iLockSurfaceOffset = ResDetails.LockOffset.YUV.Y;

        // Get U/UV plane information (plane offset, X/Y offset)
        pSurface->UPlaneOffset.iSurfaceOffset     = ResDetails.RenderOffset.YUV.U.BaseOffset;
        pSurface->UPlaneOffset.iXOffset           = ResDetails.RenderOffset.YUV.U.XOffset;
        pSurface->UPlaneOffset.iYOffset           = ResDetails.RenderOffset.YUV.U.YOffset;
        pSurface->UPlaneOffset.iLockSurfaceOffset = ResDetails.LockOffset.YUV.U;

        // Get V plane information (plane offset, X/Y offset)
        pSurface->VPlaneOffset.iSurfaceOffset     = ResDetails.RenderOffset.YUV.V.BaseOffset;
        pSurface->VPlaneOffset.iXOffset           = ResDetails.RenderOffset.YUV.V.XOffset;
        pSurface->VPlaneOffset.iYOffset           = ResDetails.RenderOffset.YUV.V.YOffset;
        pSurface->VPlaneOffset.iLockSurfaceOffset = ResDetails.LockOffset.YUV.V;
    }

    eStatus = MOS_STATUS_SUCCESS;
    goto finish;

finish:
    return eStatus;
}

//!
//! \brief    Send Surfaces PatchList
//! \details  Send Surface State commands
//! \param    PRENDERHAL_INTERFACE pRenderHal
//!           [in] Pointer to Hardware Interface Structure
//! \param    PMOS_COMMAND_BUFFER pCmdBuffer
//!           [in] Pointer to Command Buffer
//! \return   MOS_STATUS
//!
MOS_STATUS RenderHal_SendSurfaces_PatchList(
    PRENDERHAL_INTERFACE    pRenderHal,
    PMOS_COMMAND_BUFFER     pCmdBuffer)
{
    PMOS_INTERFACE                pOsInterface;
    PRENDERHAL_STATE_HEAP         pStateHeap;
    uint8_t                       *pIndirectState;
    uint32_t                      IndirectStateBase;
    uint32_t                      IndirectStateSize;
    int32_t                       iBindingTableOffs;
    MHW_BINDING_TABLE_SEND_PARAMS SendBtParams;
    MHW_SURFACE_STATE_SEND_PARAMS SendSurfaceParams;
    int32_t                       iSurfacesPerBT;
    int32_t                       i;
    int32_t                       j;
    MOS_STATUS                    eStatus = MOS_STATUS_SUCCESS;

    //----------------------------------------------
    MHW_RENDERHAL_CHK_NULL(pRenderHal);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pStateHeap);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pMhwStateHeap);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pOsInterface);
    //----------------------------------------------

    pStateHeap          = pRenderHal->pStateHeap;
    pOsInterface        = pRenderHal->pOsInterface;
    iSurfacesPerBT      = pRenderHal->StateHeapSettings.iSurfacesPerBT;

    // Get offset and size of indirect state in command buffer
    MHW_RENDERHAL_CHK_STATUS(pOsInterface->pfnGetIndirectState(pOsInterface, &IndirectStateBase, &IndirectStateSize));
    pIndirectState = (uint8_t*)pCmdBuffer->pCmdBase + IndirectStateBase;

    // Null Patch is only enabled for Media Patchless
    SendSurfaceParams.bNeedNullPatch = MEDIA_IS_SKU(pOsInterface->pfnGetSkuTable(pOsInterface), FtrMediaPatchless);
    SendSurfaceParams.pIndirectStateBase  = pIndirectState;
    SendSurfaceParams.iIndirectStateBase  = IndirectStateBase;

    // Send binding tables and surface states for all phases
    SendBtParams.iSurfaceStateBase = pStateHeap->iSurfaceStateOffset;
    iBindingTableOffs = pStateHeap->iBindingTableOffset;
    for (i = pStateHeap->iCurrentBindingTable; i > 0; i--,
         iBindingTableOffs += pStateHeap->iBindingTableSize)
    {
        // Binding tables entries (input/output)
        SendBtParams.pBindingTableSource = pStateHeap->pSshBuffer + iBindingTableOffs;
        SendBtParams.pBindingTableTarget = pIndirectState         + iBindingTableOffs;
        for (j = iSurfacesPerBT; j > 0; j--)
        {
            // Send BT to indirect heap, retrieve surface state associated with BT entry
            pRenderHal->pMhwStateHeap->SendBindingTableEntry(&SendBtParams);

            // Function returns Params.iSurfaceState for BT entry (-1 if "Copy=0") and iSurfaceStateOffset
            if (SendBtParams.iSurfaceState < 0) continue;

            SendSurfaceParams.pSurfaceToken       = (uint8_t*) &pStateHeap->pSurfaceEntry[SendBtParams.iSurfaceState].SurfaceToken;
            SendSurfaceParams.pSurfaceStateSource = (uint8_t*) pStateHeap->pSurfaceEntry[SendBtParams.iSurfaceState].pSurfaceState;
            SendSurfaceParams.iSurfaceStateOffset = SendBtParams.iSurfaceStateOffset;

            //           It should consider MHW instead of VPHAL type, but currently the function only supports VPHAL type.

            pRenderHal->pfnSendSurfaceStateEntry(pRenderHal, pCmdBuffer, &SendSurfaceParams);
        }
    }

finish:
    MHW_RENDERHAL_ASSERT(eStatus == MOS_STATUS_SUCCESS);
    return eStatus;
}

//!
//! \brief    Set surface state token
//! \details  Set surface state token
//! \param    [in] pRenderHal
//!           pointer to render hal 
//! \param    [in] Surface token parameters 
//!           Surface token parameters 
//! \param    void  *pSurfaceStateToken
//!           [in/out] pointer to surface state token
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success, else fail reason
//!
MOS_STATUS RenderHal_SetSurfaceStateToken(
    PRENDERHAL_INTERFACE        pRenderHal,
    PMHW_SURFACE_TOKEN_PARAMS   pParams,
    void                        *pSurfaceStateToken)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_MI_CHK_NULL(pSurfaceStateToken);
    MHW_MI_CHK_NULL(pParams);
    MHW_MI_CHK_NULL(pParams->pOsSurface);

    SURFACE_STATE_TOKEN_COMMON *pTokenState = (SURFACE_STATE_TOKEN_COMMON*)pSurfaceStateToken;
    PMOS_INTERFACE pOsInterface = pRenderHal->pOsInterface;
    PMOS_RESOURCE  pOsResource  = &(pParams->pOsSurface->OsResource);

    int32_t  iAllocationIndex =  pOsInterface->pfnGetResourceAllocationIndex(pOsInterface, pOsResource);

    // Initialize Token State
    *pTokenState = g_cInit_SURFACE_STATE_TOKEN_COMMON;

    pTokenState->DW1.SurfaceAllocationIndex = iAllocationIndex;
    MHW_ASSERT(pTokenState->DW1.SurfaceAllocationIndex != MOS_INVALID_ALLOC_INDEX);
    pTokenState->DW3.RenderTargetEnable = pParams->bRenderTarget;
    pTokenState->DW3.YUVPlane = pParams->YUVPlane;

    MOS_HW_COMMAND HwCommandType;
    if (pParams->bSurfaceTypeAvs)
    {
        pTokenState->DW3.SurfaceStateType = MEDIASTATE_BTS_DI_SAMPLE8x8_VME_TYPE;
        HwCommandType = MOS_SURFACE_STATE_ADV;
    }
    else
    {
        pTokenState->DW3.SurfaceStateType = MEDIASTATE_BTS_DEFAULT_TYPE;
        HwCommandType = MOS_SURFACE_STATE;
    }

    pTokenState->DW0.DriverID = HwCommandType;

    pTokenState->DW2.SurfaceOffset = pParams->dwSurfaceOffset;

    if (pOsInterface->bUsesGfxAddress)
    {
        uint64_t ui64GfxAddress = 0;
        if ( pOsResource->user_provided_va != 0 )
        {
            ui64GfxAddress = pOsResource->user_provided_va;
        }
        else
        {
            ui64GfxAddress = pOsInterface->pfnGetResourceGfxAddress( pOsInterface, pOsResource ) + pTokenState->DW2.SurfaceOffset;
        }
        pTokenState->DW4.SurfaceBaseAddress = ( uint32_t )( ui64GfxAddress & 0x00000000FFFFFFFF );
        pTokenState->DW5.SurfaceBaseAddress64 = ( uint32_t )( ( ui64GfxAddress & 0x0000FFFF00000000 ) >> 32 );
    }

    pTokenState->pResourceInfo = (void *)pOsResource;

    return eStatus;
}

//!
//! \brief    Get Y Offset according to the planeOffset struct and surface pitch
//! \details  Get Y Offset according to the planeOffset struct and surface pitch
//! \param    pOsInterface
//!           [in] pointer to OS Interface
//! \param    pSurface
//!           [in] Pointers to Surface
//! \return   uint16_t
//!           [out] the Y offset
//!
uint16_t RenderHal_CalculateYOffset(PMOS_INTERFACE pOsInterface, PMOS_RESOURCE pOsResource)
{
    // This is for MMCD/Non-MMCD, GMM will allocate the surface 32 height align surface, the the UV offset will not equal to the surface height.
    MOS_SURFACE    ResDetails;
    uint16_t       UYoffset = 0;

    MHW_RENDERHAL_ASSERT(!Mos_ResourceIsNull(pOsResource));
    MHW_RENDERHAL_ASSERT(pOsInterface);
    MOS_ZeroMemory(&ResDetails, sizeof(MOS_SURFACE));

    pOsInterface->pfnGetResourceInfo(pOsInterface, pOsResource, &ResDetails);

    if (ResDetails.dwPitch)
    {
        UYoffset = (uint16_t)((ResDetails.RenderOffset.YUV.U.BaseOffset - ResDetails.RenderOffset.YUV.Y.BaseOffset) / ResDetails.dwPitch + ResDetails.RenderOffset.YUV.U.YOffset);
        return MOS_MAX(UYoffset, (uint16_t)ResDetails.dwHeight);
    }
    else
    {
        return (uint16_t)ResDetails.dwHeight;
    }
}

//!
//! \brief    Allocate Debug Surface
//! \details  Allocate surface for ISA ASM debugging
//! \param    PRENDERHAL_INTERFACE pRenderHal
//!           [in] Pointer to Hardware Interface Structure
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success
//!           Error code otherwise
//!
MOS_STATUS RenderHal_AllocateDebugSurface(
    PRENDERHAL_INTERFACE     pRenderHal)
{
    MHW_RENDERHAL_UNUSED(pRenderHal);
    return MOS_STATUS_SUCCESS;
}

//!
//! \brief    Setup Debug Surface State
//! \details  Setup surface state for ISA ASM Debug surface
//! \param    PRENDERHAL_INTERFACE pRenderHal
//!           [in] Pointer to Hardware Interface Structure
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success
//!           Error code otherwise
//!
MOS_STATUS RenderHal_SetupDebugSurfaceState(
    PRENDERHAL_INTERFACE    pRenderHal)
{
    MHW_RENDERHAL_UNUSED(pRenderHal);
    return MOS_STATUS_SUCCESS;
}

//!
//! \brief    Free Debug Surface
//! \param    PRENDERHAL_INTERFACE pRenderHal
//!           [in] Pointer to Hardware Interface Structure
//! \return   void
//!
void RenderHal_FreeDebugSurface(
    PRENDERHAL_INTERFACE     pRenderHal)
{
    MHW_RENDERHAL_UNUSED(pRenderHal);
    return;
}

//!
//! \brief    Load Debug Kernel
//! \param    PRENDERHAL_INTERFACE pRenderHal
//!           [in] Pointer to Hardware Interface Structure
//! \param    PMHW_KERNEL_PARAM pSipKernel
//!           [in] Pointer to Debug (Sip) Kernel Parameters
//! \return   int32_t
//!           0
//!
int32_t RenderHal_LoadDebugKernel(
    PRENDERHAL_INTERFACE    pRenderHal,
    PMHW_KERNEL_PARAM       pSipKernel)
{
    MHW_RENDERHAL_UNUSED(pRenderHal);
    MHW_RENDERHAL_UNUSED(pSipKernel);
    return 0;
}

//!
//! \brief    Load Debug Kernel
//! \param    PRENDERHAL_INTERFACE pRenderHal
//!           [in] Pointer to Hardware Interface Structure
//! \param    void   *pSipKernel
//!           [in] Pointer to Debug (Sip) Kernel binary
//! \param    uint32_t dwSipSize
//!           [in] Debug (Sip) Kernel size
//! \return   MOS_STATUS
//!
MOS_STATUS RenderHal_LoadSipKernel(
    PRENDERHAL_INTERFACE    pRenderHal,
    void                    *pSipKernel,
    uint32_t                dwSipSize)
{
    MHW_RENDERHAL_UNUSED(pRenderHal);
    MHW_RENDERHAL_UNUSED(pSipKernel);
    MHW_RENDERHAL_UNUSED(dwSipSize);
    return MOS_STATUS_SUCCESS;
}

//!
//! \brief    Send SIP state command
//! \param    PRENDERHAL_INTERFACE pRenderHal
//!           [in] Pointer to Hardware Interface Structure
//! \param    PMOS_COMMAND_BUFFER pCmdBuffer
//!           [in] Pointer to Command Buffer
//! \return   MOS_STATUS
//!
MOS_STATUS RenderHal_SendSipStateCmd(
    PRENDERHAL_INTERFACE    pRenderHal,
    PMOS_COMMAND_BUFFER     pCmdBuffer)
{
    MHW_RENDERHAL_UNUSED(pRenderHal);
    MHW_RENDERHAL_UNUSED(pCmdBuffer);
    return MOS_STATUS_SUCCESS;
}

//!
//! \brief    Add debug control commands
//! \param    PRENDERHAL_INTERFACE pRenderHal
//!           [in] Pointer to Hardware Interface Structure
//! \param    PMOS_COMMAND_BUFFER pCmdBuffer
//!           [in] Pointer to Command Buffer
//! \return   MOS_STATUS
//!
MOS_STATUS RenderHal_AddDebugControl(
    PRENDERHAL_INTERFACE    pRenderHal,
    PMOS_COMMAND_BUFFER     pCmdBuffer)
{
    MHW_RENDERHAL_UNUSED(pRenderHal);
    MHW_RENDERHAL_UNUSED(pCmdBuffer);
    return MOS_STATUS_SUCCESS;
}

//!
//! \brief    Init Special Interface
//! \details  Initializes RenderHal Interface structure, responsible for HW
//!           abstraction of HW Rendering Engine for CM(MDF) and VP.
//! \param    PRENDERHAL_INTERFACE pRenderHal
//!           [in] Pointer to RenderHal Interface Structure
//!
void RenderHal_InitInterfaceEx(PRENDERHAL_INTERFACE pRenderHal)
{
    // No special APIs
    MHW_RENDERHAL_UNUSED(pRenderHal);
    return;
}

//!
//! \brief    Issue command to write timestamp
//! \param    [in] pRenderHal
//! \param    [in] pCmdBuffer
//! \param    [in] bStartTime
//! \return   MOS_STATUS
//!
MOS_STATUS RenderHal_SendTimingData(
    PRENDERHAL_INTERFACE         pRenderHal,
    PMOS_COMMAND_BUFFER          pCmdBuffer,
    bool                         bStartTime)
{
    return MOS_STATUS_SUCCESS;
}
