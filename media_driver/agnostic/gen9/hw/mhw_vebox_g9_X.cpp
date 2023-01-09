/*
* Copyright (c) 2014-2019, Intel Corporation
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
//! \file     mhw_vebox_g9_X.cpp
//! \brief    Constructs vebox commands on Gen9-based platforms
//! \details  Each client facing function both creates a HW command and adds
//!           that command to a command or batch buffer.
//!

#include "mhw_vebox_g9_X.h"
#include "hal_oca_interface.h"

MhwVeboxInterfaceG9::MhwVeboxInterfaceG9(
    PMOS_INTERFACE pInputInterface)
    : MhwVeboxInterfaceGeneric(pInputInterface)
{
    MHW_FUNCTION_ENTER;
    m_veboxSettings = g_Vebox_Settings_g9;
}

void MhwVeboxInterfaceG9::SetVeboxIecpStateBecsc(
    mhw_vebox_g9_X::VEBOX_IECP_STATE_CMD *pVeboxIecpState,
    PMHW_VEBOX_IECP_PARAMS                pVeboxIecpParams,
    bool                                  bEnableFECSC)
{
    PMHW_CAPPIPE_PARAMS pCapPipeParams;
    MOS_FORMAT          dstFormat;

    MHW_CHK_NULL_NO_STATUS_RETURN(pVeboxIecpState);
    MHW_CHK_NULL_NO_STATUS_RETURN(pVeboxIecpParams);

    pCapPipeParams = &pVeboxIecpParams->CapPipeParams;
    dstFormat      = pVeboxIecpParams->dstFormat;

#define SET_COEFS(_c0, _c1, _c2, _c3, _c4, _c5, _c6, _c7, _c8) \
    {                                                          \
        pVeboxIecpState->CscState.DW0.C0 = _c0;                \
        pVeboxIecpState->CscState.DW1.C1 = _c1;                \
        pVeboxIecpState->CscState.DW2.C2 = _c2;                \
        pVeboxIecpState->CscState.DW3.C3 = _c3;                \
        pVeboxIecpState->CscState.DW4.C4 = _c4;                \
        pVeboxIecpState->CscState.DW5.C5 = _c5;                \
        pVeboxIecpState->CscState.DW6.C6 = _c6;                \
        pVeboxIecpState->CscState.DW7.C7 = _c7;                \
        pVeboxIecpState->CscState.DW8.C8 = _c8;                \
    }

#define SET_INPUT_OFFSETS(_in1, _in2, _in3)              \
    {                                                    \
        pVeboxIecpState->CscState.DW9.OffsetIn1  = _in1; \
        pVeboxIecpState->CscState.DW10.OffsetIn2 = _in2; \
        pVeboxIecpState->CscState.DW11.OffsetIn3 = _in3; \
    }

#define SET_OUTPUT_OFFSETS(_out1, _out2, _out3)            \
    {                                                      \
        pVeboxIecpState->CscState.DW9.OffsetOut1  = _out1; \
        pVeboxIecpState->CscState.DW10.OffsetOut2 = _out2; \
        pVeboxIecpState->CscState.DW11.OffsetOut3 = _out3; \
    }

    if (pCapPipeParams->bActive)
    {
        // Application controlled CSC operation
        if (pCapPipeParams->BECSCParams.bActive)
        {
            pVeboxIecpState->CscState.DW0.TransformEnable = true;

            if (IS_RGB_SWAP(dstFormat))
            {
                pVeboxIecpState->CscState.DW0.YuvChannelSwap = true;
            }

            // Coeff is S2.16, so multiply the floating value by 65536
            SET_COEFS(
                ((uint32_t)(pCapPipeParams->BECSCParams.Matrix[0][0] * 65536)),
                ((uint32_t)(pCapPipeParams->BECSCParams.Matrix[0][1] * 65536)),
                ((uint32_t)(pCapPipeParams->BECSCParams.Matrix[0][2] * 65536)),
                ((uint32_t)(pCapPipeParams->BECSCParams.Matrix[1][0] * 65536)),
                ((uint32_t)(pCapPipeParams->BECSCParams.Matrix[1][1] * 65536)),
                ((uint32_t)(pCapPipeParams->BECSCParams.Matrix[1][2] * 65536)),
                ((uint32_t)(pCapPipeParams->BECSCParams.Matrix[2][0] * 65536)),
                ((uint32_t)(pCapPipeParams->BECSCParams.Matrix[2][1] * 65536)),
                ((uint32_t)(pCapPipeParams->BECSCParams.Matrix[2][2] * 65536)));
            SET_INPUT_OFFSETS(
                ((uint32_t)pCapPipeParams->BECSCParams.PreOffset[0]),
                ((uint32_t)pCapPipeParams->BECSCParams.PreOffset[1]),
                ((uint32_t)pCapPipeParams->BECSCParams.PreOffset[2]));
            SET_OUTPUT_OFFSETS(
                ((uint32_t)pCapPipeParams->BECSCParams.PostOffset[0]),
                ((uint32_t)pCapPipeParams->BECSCParams.PostOffset[1]),
                ((uint32_t)pCapPipeParams->BECSCParams.PostOffset[2]));
        }
        // YUV 4:4:4 CSC to xBGR or xRGB
        else if ((bEnableFECSC || (pVeboxIecpParams->srcFormat == Format_AYUV)) &&
                 (IS_RGB_FORMAT(dstFormat)))
        {
            pVeboxIecpState->CscState.DW0.TransformEnable = true;

            if (IS_RGB_SWAP(dstFormat))
            {
                pVeboxIecpState->CscState.DW0.YuvChannelSwap = true;
            }

            // CSC matrix to convert YUV 4:4:4 to xBGR. e.g. Format_A8B8G8R8. In the
            // event that dstFormat is xRGB, driver sets R & B channel swapping via
            // CscState.DW0.YuvChannelSwap so a separate matrix is not needed.

            if (pVeboxIecpParams->ColorSpace == MHW_CSpace_BT601)
            {
                SET_COEFS(76284, 0, 104595, 76284, MOS_BITFIELD_VALUE((uint32_t)-25689, 19), MOS_BITFIELD_VALUE((uint32_t)-53280, 19), 76284, 132186, 0);

                SET_INPUT_OFFSETS(MOS_BITFIELD_VALUE((uint32_t)-2048, 16),
                    MOS_BITFIELD_VALUE((uint32_t)-16384, 16),
                    MOS_BITFIELD_VALUE((uint32_t)-16384, 16));
                SET_OUTPUT_OFFSETS(0, 0, 0);
            }
            else if (pVeboxIecpParams->ColorSpace == MHW_CSpace_BT709)
            {
                SET_COEFS(76284, 0, 117506, 76284, MOS_BITFIELD_VALUE((uint32_t)-13958, 19), MOS_BITFIELD_VALUE((uint32_t)-34930, 19), 76284, 138412, 0);

                SET_INPUT_OFFSETS(MOS_BITFIELD_VALUE((uint32_t)-2048, 16),
                    MOS_BITFIELD_VALUE((uint32_t)-16384, 16),
                    MOS_BITFIELD_VALUE((uint32_t)-16384, 16));
                SET_OUTPUT_OFFSETS(0, 0, 0);
            }
            else if (pVeboxIecpParams->ColorSpace == MHW_CSpace_sRGB)
            {
                SET_COEFS(65536, 0, 103206, 65536, MOS_BITFIELD_VALUE((uint32_t)-12277, 19), MOS_BITFIELD_VALUE((uint32_t)-30679, 19), 65536, 121608, 0);

                SET_INPUT_OFFSETS(0,
                    MOS_BITFIELD_VALUE((uint32_t)-16384, 16),
                    MOS_BITFIELD_VALUE((uint32_t)-16384, 16));
                SET_OUTPUT_OFFSETS(0, 0, 0);
            }
            else
            {
                MHW_ASSERT(false);
            }
        }
        // xBGR to xRGB
        else if ((dstFormat == Format_A8R8G8B8) ||
                 (dstFormat == Format_X8R8G8B8) ||
                 (dstFormat == Format_A16R16G16B16))
        {
            pVeboxIecpState->CscState.DW0.YuvChannelSwap = true;
        }
    }
    else if (pVeboxIecpParams->bCSCEnable)
    {
        pVeboxIecpState->CscState.DW0.TransformEnable = true;

        if (IS_RGB_SWAP(dstFormat))
        {
            pVeboxIecpState->CscState.DW0.YuvChannelSwap = true;
        }

        // Coeff is S2.16, so multiply the floating value by 65536
        SET_COEFS(
            ((uint32_t)MOS_F_ROUND(pVeboxIecpParams->pfCscCoeff[0] * 65536.0F)),
            ((uint32_t)MOS_F_ROUND(pVeboxIecpParams->pfCscCoeff[1] * 65536.0F)),
            ((uint32_t)MOS_F_ROUND(pVeboxIecpParams->pfCscCoeff[2] * 65536.0F)),
            ((uint32_t)MOS_F_ROUND(pVeboxIecpParams->pfCscCoeff[3] * 65536.0F)),
            ((uint32_t)MOS_F_ROUND(pVeboxIecpParams->pfCscCoeff[4] * 65536.0F)),
            ((uint32_t)MOS_F_ROUND(pVeboxIecpParams->pfCscCoeff[5] * 65536.0F)),
            ((uint32_t)MOS_F_ROUND(pVeboxIecpParams->pfCscCoeff[6] * 65536.0F)),
            ((uint32_t)MOS_F_ROUND(pVeboxIecpParams->pfCscCoeff[7] * 65536.0F)),
            ((uint32_t)MOS_F_ROUND(pVeboxIecpParams->pfCscCoeff[8] * 65536.0F)));

        // Offset is S15, but the SW offsets are calculated as 8bits,
        // so left shift them 7bits to be in the position of MSB
        SET_INPUT_OFFSETS(
            ((uint32_t)MOS_F_ROUND(pVeboxIecpParams->pfCscInOffset[0] * 128.0F)),
            ((uint32_t)MOS_F_ROUND(pVeboxIecpParams->pfCscInOffset[1] * 128.0F)),
            ((uint32_t)MOS_F_ROUND(pVeboxIecpParams->pfCscInOffset[2] * 128.0F)));
        SET_OUTPUT_OFFSETS(
            ((uint32_t)MOS_F_ROUND(pVeboxIecpParams->pfCscOutOffset[0] * 128.0F)),
            ((uint32_t)MOS_F_ROUND(pVeboxIecpParams->pfCscOutOffset[1] * 128.0F)),
            ((uint32_t)MOS_F_ROUND(pVeboxIecpParams->pfCscOutOffset[2] * 128.0F)));
    }

    pVeboxIecpState->AlphaAoiState.DW0.AlphaFromStateSelect = pVeboxIecpParams->bAlphaEnable;

    // Alpha is U16, but the SW alpha is calculated as 8bits,
    // so left shift it 8bits to be in the position of MSB
    pVeboxIecpState->AlphaAoiState.DW0.ColorPipeAlpha = pVeboxIecpParams->wAlphaValue * 256;

#undef SET_COEFS
#undef SET_INPUT_OFFSETS
#undef SET_OUTPUT_OFFSETS
}

void MhwVeboxInterfaceG9::SetVeboxSurfaces(
    PMHW_VEBOX_SURFACE_PARAMS                pSurfaceParam,
    PMHW_VEBOX_SURFACE_PARAMS                pDerivedSurfaceParam,
    PMHW_VEBOX_SURFACE_PARAMS                pSkinScoreSurfaceParam,
    mhw_vebox_g9_X::VEBOX_SURFACE_STATE_CMD *pVeboxSurfaceState,
    bool                                     bIsOutputSurface,
    bool                                     bDIEnable)
{
    uint32_t                                dwFormat;
    uint32_t                                dwSurfaceWidth;
    uint32_t                                dwSurfaceHeight;
    uint32_t                                dwSurfacePitch;
    bool                                    bHalfPitchForChroma;
    bool                                    bInterleaveChroma;
    uint16_t                                wUXOffset;
    uint16_t                                wUYOffset;
    uint16_t                                wVXOffset;
    uint16_t                                wVYOffset;
    uint8_t                                 bBayerOffset;
    uint8_t                                 bBayerStride;
    mhw_vebox_g9_X::VEBOX_SURFACE_STATE_CMD VeboxSurfaceState;

    MHW_CHK_NULL_NO_STATUS_RETURN(pSurfaceParam);
    MHW_CHK_NULL_NO_STATUS_RETURN(pVeboxSurfaceState);

    // Initialize
    dwSurfaceWidth      = 0;
    dwSurfaceHeight     = 0;
    dwSurfacePitch      = 0;
    bHalfPitchForChroma = false;
    bInterleaveChroma   = false;
    wUXOffset           = 0;
    wUYOffset           = 0;
    wVXOffset           = 0;
    wVYOffset           = 0;
    bBayerOffset        = 0;
    bBayerStride        = 0;

    *pVeboxSurfaceState = VeboxSurfaceState;

    switch (pSurfaceParam->Format)
    {
    case Format_NV12:
        dwFormat          = VeboxSurfaceState.SURFACE_FORMAT_PLANAR4208;
        bInterleaveChroma = true;
        wUYOffset         = (uint16_t)pSurfaceParam->dwUYoffset;
        break;

    case Format_YUYV:
    case Format_YUY2:
        dwFormat = VeboxSurfaceState.SURFACE_FORMAT_YCRCBNORMAL;
        break;

    case Format_UYVY:
        dwFormat = VeboxSurfaceState.SURFACE_FORMAT_YCRCBSWAPY;
        break;

    case Format_AYUV:
        dwFormat = VeboxSurfaceState.SURFACE_FORMAT_PACKED444A8;
        break;

    case Format_Y416:
        dwFormat = VeboxSurfaceState.SURFACE_FORMAT_PACKED44416;
        break;

    case Format_YVYU:
        dwFormat = VeboxSurfaceState.SURFACE_FORMAT_YCRCBSWAPUV;
        break;

    case Format_VYUY:
        dwFormat = VeboxSurfaceState.SURFACE_FORMAT_YCRCBSWAPUVY;
        break;

    case Format_A8R8G8B8:
    case Format_X8R8G8B8:
    case Format_A8B8G8R8:
    case Format_X8B8G8R8:
        dwFormat = VeboxSurfaceState.SURFACE_FORMAT_R8G8B8A8UNORMR8G8B8A8UNORMSRGB;
        break;

    case Format_A16B16G16R16:
    case Format_A16R16G16B16:
        dwFormat = VeboxSurfaceState.SURFACE_FORMAT_R16G16B16A16;
        break;

    case Format_L8:
    case Format_P8:
        dwFormat = VeboxSurfaceState.SURFACE_FORMAT_Y8UNORM;
        break;

    case Format_IRW0:
        dwFormat     = VeboxSurfaceState.SURFACE_FORMAT_BAYERPATTERN;
        bBayerOffset = VeboxSurfaceState.BAYER_PATTERN_OFFSET_PIXELATX0_Y0ISBLUE;
        bBayerStride = VeboxSurfaceState.BAYER_PATTERN_FORMAT_16_BITINPUTATA16_BITSTRIDE;
        break;

    case Format_IRW1:
        dwFormat     = VeboxSurfaceState.SURFACE_FORMAT_BAYERPATTERN;
        bBayerOffset = VeboxSurfaceState.BAYER_PATTERN_OFFSET_PIXELATX0_Y0ISRED;
        bBayerStride = VeboxSurfaceState.BAYER_PATTERN_FORMAT_16_BITINPUTATA16_BITSTRIDE;
        break;

    case Format_IRW2:
        dwFormat     = VeboxSurfaceState.SURFACE_FORMAT_BAYERPATTERN;
        bBayerOffset = VeboxSurfaceState.BAYER_PATTERN_OFFSET_PIXELATX0_Y0ISGREEN_PIXELATX1_Y0ISRED;
        bBayerStride = VeboxSurfaceState.BAYER_PATTERN_FORMAT_16_BITINPUTATA16_BITSTRIDE;
        break;

    case Format_IRW3:
        dwFormat     = VeboxSurfaceState.SURFACE_FORMAT_BAYERPATTERN;
        bBayerOffset = VeboxSurfaceState.BAYER_PATTERN_OFFSET_PIXELATX0_Y0ISGREEN_PIXELATX1_Y0ISBLUE;
        bBayerStride = VeboxSurfaceState.BAYER_PATTERN_FORMAT_16_BITINPUTATA16_BITSTRIDE;
        break;

    case Format_IRW4:
        dwFormat     = VeboxSurfaceState.SURFACE_FORMAT_BAYERPATTERN;
        bBayerOffset = VeboxSurfaceState.BAYER_PATTERN_OFFSET_PIXELATX0_Y0ISBLUE;
        bBayerStride = VeboxSurfaceState.BAYER_PATTERN_FORMAT_8_BITINPUTATA8_BITSTRIDE;
        break;

    case Format_IRW5:
        dwFormat     = VeboxSurfaceState.SURFACE_FORMAT_BAYERPATTERN;
        bBayerOffset = VeboxSurfaceState.BAYER_PATTERN_OFFSET_PIXELATX0_Y0ISRED;
        bBayerStride = VeboxSurfaceState.BAYER_PATTERN_FORMAT_8_BITINPUTATA8_BITSTRIDE;
        break;

    case Format_IRW6:
        dwFormat     = VeboxSurfaceState.SURFACE_FORMAT_BAYERPATTERN;
        bBayerOffset = VeboxSurfaceState.BAYER_PATTERN_OFFSET_PIXELATX0_Y0ISGREEN_PIXELATX1_Y0ISRED;
        bBayerStride = VeboxSurfaceState.BAYER_PATTERN_FORMAT_8_BITINPUTATA8_BITSTRIDE;
        break;

    case Format_IRW7:
        dwFormat     = VeboxSurfaceState.SURFACE_FORMAT_BAYERPATTERN;
        bBayerOffset = VeboxSurfaceState.BAYER_PATTERN_OFFSET_PIXELATX0_Y0ISGREEN_PIXELATX1_Y0ISBLUE;
        bBayerStride = VeboxSurfaceState.BAYER_PATTERN_FORMAT_8_BITINPUTATA8_BITSTRIDE;
        break;

    case Format_P010:
    case Format_P016:
        dwFormat          = VeboxSurfaceState.SURFACE_FORMAT_PLANAR42016;
        bInterleaveChroma = true;
        wUYOffset         = (uint16_t)pSurfaceParam->dwUYoffset;
        break;

    case Format_R10G10B10A2:
    case Format_B10G10R10A2:
        dwFormat = VeboxSurfaceState.SURFACE_FORMAT_R10G10B10A2UNORMR10G10B10A2UNORMSRGB;
        break;

    default:
        MHW_ASSERTMESSAGE("Unsupported format.");
        goto finish;
        break;
    }

    // adjust boundary for vebox
    VeboxAdjustBoundary(
        pSurfaceParam,
        &dwSurfaceWidth,
        &dwSurfaceHeight,
        bDIEnable);

    dwSurfacePitch = (pSurfaceParam->TileType == MOS_TILE_LINEAR) ? MOS_ALIGN_CEIL(pSurfaceParam->dwPitch, MHW_VEBOX_LINEAR_PITCH) : pSurfaceParam->dwPitch;

    pVeboxSurfaceState->DW1.SurfaceIdentification = bIsOutputSurface;
    pVeboxSurfaceState->DW2.Width                 = dwSurfaceWidth - 1;
    pVeboxSurfaceState->DW2.Height                = dwSurfaceHeight - 1;
    pVeboxSurfaceState->DW3.HalfPitchForChroma    = bHalfPitchForChroma;
    pVeboxSurfaceState->DW3.InterleaveChroma      = bInterleaveChroma;
    pVeboxSurfaceState->DW3.SurfaceFormat         = dwFormat;
    pVeboxSurfaceState->DW3.BayerPatternOffset    = bBayerOffset;
    pVeboxSurfaceState->DW3.BayerPatternFormat    = bBayerStride;
    pVeboxSurfaceState->DW3.SurfacePitch          = dwSurfacePitch - 1;
    pVeboxSurfaceState->DW3.TiledSurface          = (pSurfaceParam->TileType != MOS_TILE_LINEAR) ? true : false;
    pVeboxSurfaceState->DW3.TileWalk              = (pSurfaceParam->TileType == MOS_TILE_Y)
                                                    ? VeboxSurfaceState.TILE_WALK_TILEWALKYMAJOR
                                                    : VeboxSurfaceState.TILE_WALK_TILEWALKXMAJOR;
    pVeboxSurfaceState->DW4.XOffsetForU           = wUXOffset;
    pVeboxSurfaceState->DW4.YOffsetForU           = wUYOffset;
    pVeboxSurfaceState->DW5.XOffsetForV           = wVXOffset;
    pVeboxSurfaceState->DW5.YOffsetForV           = wVYOffset;

    // May fix this for stereo surfaces
    pVeboxSurfaceState->DW6.YOffsetForFrame = pSurfaceParam->dwYoffset;
    pVeboxSurfaceState->DW6.XOffsetForFrame = 0;

    pVeboxSurfaceState->DW7.DerivedSurfacePitch                    = pDerivedSurfaceParam->dwPitch - 1;
    pVeboxSurfaceState->DW8.SurfacePitchForSkinScoreOutputSurfaces = (bIsOutputSurface && pSkinScoreSurfaceParam->bActive) ? (pSkinScoreSurfaceParam->dwPitch - 1) : 0;

finish:
    return;
}

MOS_STATUS MhwVeboxInterfaceG9::GetVeboxAce_FullImageHistogram(
    uint32_t *pFullImageHistogram)
{
    MOS_STATUS                            eStatus = MOS_STATUS_SUCCESS;
    PMHW_VEBOX_HEAP                       pVeboxHeap;
    int32_t                               uiOffset;
    mhw_vebox_g9_X::VEBOX_IECP_STATE_CMD *pVeboxIecpState;

    MHW_CHK_NULL(pFullImageHistogram);
    MHW_CHK_NULL(m_veboxHeap);

    pVeboxHeap = m_veboxHeap;
    uiOffset   = pVeboxHeap->uiCurState * pVeboxHeap->uiInstanceSize;

    pVeboxIecpState = (mhw_vebox_g9_X::VEBOX_IECP_STATE_CMD *)(pVeboxHeap->pLockedDriverResourceMem +
                                                               pVeboxHeap->uiIecpStateOffset +
                                                               uiOffset);
    MHW_CHK_NULL(pVeboxIecpState);
    *pFullImageHistogram = pVeboxIecpState->AlphaAoiState.DW0.FullImageHistogram;

finish:
    return eStatus;
}

MOS_STATUS MhwVeboxInterfaceG9::AddVeboxState(
    PMOS_COMMAND_BUFFER         pCmdBuffer,
    PMHW_VEBOX_STATE_CMD_PARAMS pVeboxStateCmdParams,
    bool                        bUseCmBuffer)
{
    MOS_STATUS                      eStatus;
    PMOS_INTERFACE                  pOsInterface;
    PMOS_CONTEXT                    pOsContext = nullptr;
    PMOS_RESOURCE                   pVeboxParamResource = nullptr;
    PMOS_RESOURCE                   pVeboxHeapResource = nullptr;
    PMHW_VEBOX_MODE                 pVeboxMode;
    uint32_t                        uiInstanceBaseAddr = 0;
    MHW_RESOURCE_PARAMS             ResourceParams;
    PMHW_VEBOX_HEAP                 pVeboxHeap;
    MOS_ALLOC_GFXRES_PARAMS         AllocParamsForBufferLinear;
    mhw_vebox_g9_X::VEBOX_STATE_CMD cmd;

    MHW_CHK_NULL(m_osInterface);
    MHW_CHK_NULL(m_osInterface->pOsContext);
    MHW_CHK_NULL(pCmdBuffer);
    MHW_CHK_NULL(pVeboxStateCmdParams);

    // Initialize
    eStatus      = MOS_STATUS_SUCCESS;
    pOsInterface = m_osInterface;
    pOsContext   = m_osInterface->pOsContext;
    pVeboxMode   = &pVeboxStateCmdParams->VeboxMode;

    cmd.DW1.DownsampleMethod422to420 = 1;
    cmd.DW1.DownsampleMethod444to422 = 1;

    if (!pVeboxStateCmdParams->bNoUseVeboxHeap)
    {
        MHW_CHK_NULL(m_veboxHeap);

        pVeboxHeap = m_veboxHeap;
        if (bUseCmBuffer)
            pVeboxParamResource = pVeboxStateCmdParams->pVeboxParamSurf;
        else
        {
            pVeboxHeap         = m_veboxHeap;
            pVeboxHeapResource = pVeboxStateCmdParams->bUseVeboxHeapKernelResource ? &pVeboxHeap->KernelResource : &pVeboxHeap->DriverResource;
            // Calculate the instance base address
            uiInstanceBaseAddr = pVeboxHeap->uiInstanceSize * pVeboxHeap->uiCurState;
        }

        TraceIndirectStateInfo(*pCmdBuffer, *pOsContext, bUseCmBuffer, pVeboxStateCmdParams->bUseVeboxHeapKernelResource);

        MOS_ZeroMemory(&ResourceParams, sizeof(ResourceParams));
        if (bUseCmBuffer)
        {
            ResourceParams.presResource = pVeboxParamResource;
            ResourceParams.dwOffset     = pVeboxHeap->uiDndiStateOffset;
        }
        else
        {
            ResourceParams.presResource = pVeboxHeapResource;
            ResourceParams.dwOffset     = pVeboxHeap->uiDndiStateOffset + uiInstanceBaseAddr;
        }
        ResourceParams.pdwCmd          = & (cmd.DW2.Value);
        ResourceParams.dwLocationInCmd = 2;
        ResourceParams.HwCommandType   = MOS_VEBOX_STATE;

        MHW_CHK_STATUS(pfnAddResourceToCmd(
            pOsInterface,
            pCmdBuffer,
            &ResourceParams));

        HalOcaInterface::OnIndirectState(*pCmdBuffer, (MOS_CONTEXT_HANDLE)pOsContext, ResourceParams.presResource, ResourceParams.dwOffset, false, m_veboxSettings.uiDndiStateSize);

        MOS_ZeroMemory(&ResourceParams, sizeof(ResourceParams));
        if (bUseCmBuffer)
        {
            ResourceParams.presResource = pVeboxParamResource;
            ResourceParams.dwOffset     = pVeboxHeap->uiIecpStateOffset;
        }
        else
        {
            ResourceParams.presResource = pVeboxHeapResource;
            ResourceParams.dwOffset     = pVeboxHeap->uiIecpStateOffset + uiInstanceBaseAddr;
        }
        ResourceParams.pdwCmd             = & (cmd.DW4.Value);
        ResourceParams.dwLocationInCmd    = 4;
        ResourceParams.HwCommandType      = MOS_VEBOX_STATE;
        ResourceParams.dwSharedMocsOffset = 1 - ResourceParams.dwLocationInCmd;

        MHW_CHK_STATUS(pfnAddResourceToCmd(
            pOsInterface,
            pCmdBuffer,
            &ResourceParams));

        HalOcaInterface::OnIndirectState(*pCmdBuffer, (MOS_CONTEXT_HANDLE)pOsContext, ResourceParams.presResource, ResourceParams.dwOffset, false, m_veboxSettings.uiIecpStateSize);

        MOS_ZeroMemory(&ResourceParams, sizeof(ResourceParams));

        if (bUseCmBuffer)
        {
            ResourceParams.presResource = pVeboxParamResource;
            ResourceParams.dwOffset     = pVeboxHeap->uiGamutStateOffset;
        }
        else
        {
            ResourceParams.presResource = pVeboxHeapResource;
            ResourceParams.dwOffset     = pVeboxHeap->uiGamutStateOffset + uiInstanceBaseAddr;
        }
        ResourceParams.pdwCmd             = & (cmd.DW6.Value);
        ResourceParams.dwLocationInCmd    = 6;
        ResourceParams.HwCommandType      = MOS_VEBOX_STATE;
        ResourceParams.dwSharedMocsOffset = 1 - ResourceParams.dwLocationInCmd;

        MHW_CHK_STATUS(pfnAddResourceToCmd(
            pOsInterface,
            pCmdBuffer,
            &ResourceParams));

        HalOcaInterface::OnIndirectState(*pCmdBuffer, (MOS_CONTEXT_HANDLE)pOsContext, ResourceParams.presResource, ResourceParams.dwOffset, false, m_veboxSettings.uiGamutStateSize);

        MOS_ZeroMemory(&ResourceParams, sizeof(ResourceParams));
        if (bUseCmBuffer)
        {
            ResourceParams.presResource = pVeboxParamResource;
            ResourceParams.dwOffset     = pVeboxHeap->uiVertexTableOffset;
        }
        else
        {
            ResourceParams.presResource = pVeboxHeapResource;
            ResourceParams.dwOffset     = pVeboxHeap->uiVertexTableOffset + uiInstanceBaseAddr;
        }
        ResourceParams.pdwCmd             = & (cmd.DW8.Value);
        ResourceParams.dwLocationInCmd    = 8;
        ResourceParams.HwCommandType      = MOS_VEBOX_STATE;
        ResourceParams.dwSharedMocsOffset = 1 - ResourceParams.dwLocationInCmd;

        MHW_CHK_STATUS(pfnAddResourceToCmd(
            pOsInterface,
            pCmdBuffer,
            &ResourceParams));

        HalOcaInterface::OnIndirectState(*pCmdBuffer, (MOS_CONTEXT_HANDLE)pOsContext, ResourceParams.presResource, ResourceParams.dwOffset, false, m_veboxSettings.uiVertexTableSize);

        MOS_ZeroMemory(&ResourceParams, sizeof(ResourceParams));
        if (bUseCmBuffer)
        {
            ResourceParams.presResource = pVeboxParamResource;
            ResourceParams.dwOffset     = pVeboxHeap->uiCapturePipeStateOffset;
        }
        else
        {
            ResourceParams.presResource = pVeboxHeapResource;
            ResourceParams.dwOffset     = pVeboxHeap->uiCapturePipeStateOffset + uiInstanceBaseAddr;
        }
        ResourceParams.pdwCmd             = & (cmd.DW10.Value);
        ResourceParams.dwLocationInCmd    = 10;
        ResourceParams.HwCommandType      = MOS_VEBOX_STATE;
        ResourceParams.dwSharedMocsOffset = 1 - ResourceParams.dwLocationInCmd;

        MHW_CHK_STATUS(pfnAddResourceToCmd(
            pOsInterface,
            pCmdBuffer,
            &ResourceParams));

        HalOcaInterface::OnIndirectState(*pCmdBuffer, (MOS_CONTEXT_HANDLE)pOsContext, ResourceParams.presResource, ResourceParams.dwOffset, false, m_veboxSettings.uiCapturePipeStateSize);

        if (pVeboxStateCmdParams->pLaceLookUpTables)
        {
            MOS_ZeroMemory(&ResourceParams, sizeof(ResourceParams));
            ResourceParams.presResource       = pVeboxStateCmdParams->pLaceLookUpTables;
            ResourceParams.dwOffset           = 0;
            ResourceParams.pdwCmd             = & (cmd.DW12.Value);
            ResourceParams.dwLocationInCmd    = 12;
            ResourceParams.HwCommandType      = MOS_VEBOX_STATE;
            ResourceParams.dwSharedMocsOffset = 1 - ResourceParams.dwLocationInCmd;

            MHW_CHK_STATUS(pfnAddResourceToCmd(
                pOsInterface,
                pCmdBuffer,
                &ResourceParams));
        }

        MOS_ZeroMemory(&ResourceParams, sizeof(ResourceParams));
        if (bUseCmBuffer)
        {
            ResourceParams.presResource = pVeboxParamResource;
            ResourceParams.dwOffset     = pVeboxHeap->uiGammaCorrectionStateOffset;
        }
        else
        {
            ResourceParams.presResource = pVeboxHeapResource;
            ResourceParams.dwOffset     = pVeboxHeap->uiGammaCorrectionStateOffset + uiInstanceBaseAddr;
        }
        ResourceParams.pdwCmd             = & (cmd.DW14_15.Value[0]);
        ResourceParams.dwLocationInCmd    = 14;
        ResourceParams.HwCommandType      = MOS_VEBOX_STATE;
        ResourceParams.dwSharedMocsOffset = 1 - ResourceParams.dwLocationInCmd;

        MHW_CHK_STATUS(pfnAddResourceToCmd(
            pOsInterface,
            pCmdBuffer,
            &ResourceParams));

        HalOcaInterface::OnIndirectState(*pCmdBuffer, (MOS_CONTEXT_HANDLE)pOsContext, ResourceParams.presResource, ResourceParams.dwOffset, false, m_veboxSettings.uiGammaCorrectionStateSize);
    }
    else
    {
        // Allocate Resource to avoid Page Fault issue since HW will access it
        if (Mos_ResourceIsNull(&pVeboxStateCmdParams->DummyIecpResource))
        {
            MOS_ZeroMemory(&AllocParamsForBufferLinear, sizeof(MOS_ALLOC_GFXRES_PARAMS));

            AllocParamsForBufferLinear.Type     = MOS_GFXRES_BUFFER;
            AllocParamsForBufferLinear.TileType = MOS_TILE_LINEAR;
            AllocParamsForBufferLinear.Format   = Format_Buffer;
            AllocParamsForBufferLinear.dwBytes  = m_veboxSettings.uiIecpStateSize;
            AllocParamsForBufferLinear.pBufName = "DummyIecpResource";

            MHW_CHK_STATUS(pOsInterface->pfnAllocateResource(
                pOsInterface,
                &AllocParamsForBufferLinear,
                &pVeboxStateCmdParams->DummyIecpResource));
        }

        MOS_ZeroMemory(&ResourceParams, sizeof(ResourceParams));
        ResourceParams.presResource = &pVeboxStateCmdParams->DummyIecpResource;
        ResourceParams.dwOffset = 0;
        ResourceParams.pdwCmd =  &(cmd.DW4.Value);
        ResourceParams.dwLocationInCmd = 4;
        ResourceParams.HwCommandType = MOS_VEBOX_STATE;
        ResourceParams.dwSharedMocsOffset = 1 - ResourceParams.dwLocationInCmd;

        MHW_CHK_STATUS(pfnAddResourceToCmd(
            pOsInterface,
            pCmdBuffer,
            &ResourceParams));

        HalOcaInterface::OnIndirectState(*pCmdBuffer, (MOS_CONTEXT_HANDLE)pOsContext, ResourceParams.presResource, 0, true, 0);
    }

    cmd.DW1.ColorGamutExpansionEnable    = pVeboxMode->ColorGamutExpansionEnable;
    cmd.DW1.ColorGamutCompressionEnable  = pVeboxMode->ColorGamutCompressionEnable;
    cmd.DW1.GlobalIecpEnable             = pVeboxMode->GlobalIECPEnable;
    cmd.DW1.DnEnable                     = pVeboxMode->DNEnable;
    cmd.DW1.DiEnable                     = pVeboxMode->DIEnable;
    cmd.DW1.DnDiFirstFrame               = pVeboxMode->DNDIFirstFrame;
    cmd.DW1.DiOutputFrames               = pVeboxMode->DIOutputFrames;
    cmd.DW1.DemosaicEnable               = pVeboxMode->DemosaicEnable;
    cmd.DW1.VignetteEnable               = pVeboxMode->VignetteEnable;
    cmd.DW1.AlphaPlaneEnable             = pVeboxMode->AlphaPlaneEnable;
    cmd.DW1.HotPixelFilteringEnable      = pVeboxMode->HotPixelFilteringEnable;
    cmd.DW1.SingleSliceVeboxEnable       = pVeboxMode->SingleSliceVeboxEnable;
    cmd.DW1.LaceCorrectionEnable         = pVeboxMode->LACECorrectionEnable;
    cmd.DW1.DisableEncoderStatistics     = pVeboxMode->DisableEncoderStatistics;
    cmd.DW1.DisableTemporalDenoiseFilter = pVeboxMode->DisableTemporalDenoiseFilter;
    cmd.DW1.SinglePipeEnable             = pVeboxMode->SinglePipeIECPEnable;
    cmd.DW1.ForwardGammaCorrectionEnable = pVeboxMode->ForwardGammaCorrectionEnable;

    pOsInterface->pfnAddCommand(pCmdBuffer, &cmd, cmd.byteSize);

finish:
    return eStatus;
}

MOS_STATUS MhwVeboxInterfaceG9::AddVeboxGamutState(
    PMHW_VEBOX_IECP_PARAMS  pVeboxIecpParams,
    PMHW_VEBOX_GAMUT_PARAMS pVeboxGamutParams)

{
    PMHW_VEBOX_HEAP        pVeboxHeap;
    uint32_t               uiOffset;
    MOS_STATUS             eStatus = MOS_STATUS_SUCCESS;

    mhw_vebox_g9_X::VEBOX_IECP_STATE_CMD  *pIecpState, IecpStateCmd;
    mhw_vebox_g9_X::VEBOX_GAMUT_STATE_CMD *pGamutState, gamutCmd;
    MHW_CHK_NULL(pVeboxGamutParams);
    MHW_CHK_NULL(m_veboxHeap);

    pVeboxHeap = m_veboxHeap;
    uiOffset   = pVeboxHeap->uiCurState * pVeboxHeap->uiInstanceSize;

    pIecpState =
        (mhw_vebox_g9_X::VEBOX_IECP_STATE_CMD *)(pVeboxHeap->pLockedDriverResourceMem +
                                                 pVeboxHeap->uiIecpStateOffset +
                                                 uiOffset);
    pGamutState =
        (mhw_vebox_g9_X::VEBOX_GAMUT_STATE_CMD *)(pVeboxHeap->pLockedDriverResourceMem +
                                 pVeboxHeap->uiGamutStateOffset +
                                 uiOffset);

    MHW_CHK_NULL(pIecpState);
    MHW_CHK_NULL(pGamutState);

    // Must initialize VeboxIecpState even if it is not used because GCE
    // requires GlobalIECP enable bit to be turned on
    if (!pVeboxIecpParams)
    {
        *pIecpState = IecpStateCmd;
    }
    // Initializte the GammatSate;
    GamutStateInitialization(pGamutState);

    if (pVeboxGamutParams->GCompMode != MHW_GAMUT_MODE_NONE)
    {
        if (pVeboxGamutParams->GCompMode == MHW_GAMUT_MODE_BASIC)
        {
            pGamutState->DW35.Fullrangemappingenable = false;

            if (pVeboxGamutParams->GCompBasicMode == gamutCmd.GCC_BASICMODESELECTION_SCALINGFACTOR)
            {
                pGamutState->DW37.GccBasicmodeselection = gamutCmd.GCC_BASICMODESELECTION_SCALINGFACTOR;
                pGamutState->DW37.Basicmodescalingfactor =
                    pVeboxGamutParams->iBasicModeScalingFactor;
            }
        }
        else if (pVeboxGamutParams->GCompMode == MHW_GAMUT_MODE_ADVANCED)
        {
            pGamutState->DW35.Fullrangemappingenable = true;
            pGamutState->DW35.D1Out                  = pVeboxGamutParams->iDout;
            pGamutState->DW35.DOutDefault            = pVeboxGamutParams->iDoutDefault;
            pGamutState->DW35.DInDefault             = pVeboxGamutParams->iDinDefault;
            pGamutState->DW36.D1In                   = pVeboxGamutParams->iDin;
        }

        // Set Vertex Table if Gamut Compression is enabled
        MhwVeboxInterfaceGeneric<mhw_vebox_g9_X>::AddVeboxVertexTable(pVeboxGamutParams->ColorSpace);
    }

    if (pVeboxGamutParams->GExpMode != MHW_GAMUT_MODE_NONE)
    {
        // Need to convert YUV input to RGB before GE
        pIecpState->CscState.DW0.TransformEnable = true;
        if (pVeboxGamutParams->ColorSpace == MHW_CSpace_BT601 ||
            pVeboxGamutParams->ColorSpace == MHW_CSpace_xvYCC601 ||
            pVeboxGamutParams->ColorSpace == MHW_CSpace_BT601_FullRange)
        {
            pIecpState->CscState.DW0.C0          = 76309;
            pIecpState->CscState.DW1.C1          = 0;
            pIecpState->CscState.DW2.C2          = 104597;
            pIecpState->CscState.DW3.C3          = 76309;
            pIecpState->CscState.DW4.C4          = MOS_BITFIELD_VALUE((uint32_t)-25675, 19);
            pIecpState->CscState.DW5.C5          = MOS_BITFIELD_VALUE((uint32_t)-53279, 19);
            pIecpState->CscState.DW6.C6          = 76309;
            pIecpState->CscState.DW7.C7          = 132201;
            pIecpState->CscState.DW8.C8          = 0;
            pIecpState->CscState.DW9.OffsetIn1   = MOS_BITFIELD_VALUE((uint32_t)-2048, 16);
            pIecpState->CscState.DW9.OffsetOut1  = 0;
            pIecpState->CscState.DW10.OffsetIn2  = MOS_BITFIELD_VALUE((uint32_t)-16384, 16);
            pIecpState->CscState.DW10.OffsetOut2 = 0;
            pIecpState->CscState.DW11.OffsetIn3  = MOS_BITFIELD_VALUE((uint32_t)-16384, 16);
            pIecpState->CscState.DW11.OffsetOut3 = 0;
        }
        else if (pVeboxGamutParams->ColorSpace == MHW_CSpace_BT709 ||
                 pVeboxGamutParams->ColorSpace == MHW_CSpace_xvYCC709 ||
                 pVeboxGamutParams->ColorSpace == MHW_CSpace_BT709_FullRange)
        {
            pIecpState->CscState.DW0.C0          = 76309;
            pIecpState->CscState.DW1.C1          = 0;
            pIecpState->CscState.DW2.C2          = 117489;
            pIecpState->CscState.DW3.C3          = 76309;
            pIecpState->CscState.DW4.C4          = MOS_BITFIELD_VALUE((uint32_t)-13975, 19);
            pIecpState->CscState.DW5.C5          = MOS_BITFIELD_VALUE((uint32_t)-34925, 19);
            pIecpState->CscState.DW6.C6          = 76309;
            pIecpState->CscState.DW7.C7          = 138438;
            pIecpState->CscState.DW8.C8          = 0;
            pIecpState->CscState.DW9.OffsetIn1   = MOS_BITFIELD_VALUE((uint32_t)-2048, 16);
            pIecpState->CscState.DW9.OffsetOut1  = 0;
            pIecpState->CscState.DW10.OffsetIn2  = MOS_BITFIELD_VALUE((uint32_t)-16384, 16);
            pIecpState->CscState.DW10.OffsetOut2 = 0;
            pIecpState->CscState.DW11.OffsetIn3  = MOS_BITFIELD_VALUE((uint32_t)-16384, 16);
            pIecpState->CscState.DW11.OffsetOut3 = 0;
        }
        else
        {
            MHW_ASSERTMESSAGE("Unknown primary");
        }

        if (pVeboxGamutParams->GExpMode == MHW_GAMUT_MODE_BASIC)
        {
            pGamutState->DW0.GlobalModeEnable = true;
            pGamutState->DW0.CmW              = 1023;  // Colorimetric accurate image
        }
        else if (pVeboxGamutParams->GExpMode == MHW_GAMUT_MODE_ADVANCED)
        {
            pGamutState->DW0.GlobalModeEnable = false;
        }

        pGamutState->DW3.C0 = pVeboxGamutParams->Matrix[0][0];
        pGamutState->DW3.C1 = pVeboxGamutParams->Matrix[0][1];
        pGamutState->DW4.C2 = pVeboxGamutParams->Matrix[0][2];
        pGamutState->DW4.C3 = pVeboxGamutParams->Matrix[1][0];
        pGamutState->DW5.C4 = pVeboxGamutParams->Matrix[1][1];
        pGamutState->DW5.C5 = pVeboxGamutParams->Matrix[1][2];
        pGamutState->DW6.C6 = pVeboxGamutParams->Matrix[2][0];
        pGamutState->DW6.C7 = pVeboxGamutParams->Matrix[2][1];
        pGamutState->DW7.C8 = pVeboxGamutParams->Matrix[2][2];
    }
    else if (pVeboxGamutParams->bGammaCorr)
    {
        // Need to convert YUV input to RGB before Gamma Correction
        pIecpState->CscState.DW0.TransformEnable = true;
        if (IS_RGB_SWAP(pVeboxGamutParams->dstFormat))
        {
            pIecpState->CscState.DW0.YuvChannelSwap = true;
        }
        if (pVeboxGamutParams->ColorSpace == MHW_CSpace_BT601 ||
            pVeboxGamutParams->ColorSpace == MHW_CSpace_xvYCC601 ||
            pVeboxGamutParams->ColorSpace == MHW_CSpace_BT601_FullRange)
        {
            pIecpState->CscState.DW0.C0          = 76309;
            pIecpState->CscState.DW1.C1          = 0;
            pIecpState->CscState.DW2.C2          = 104597;
            pIecpState->CscState.DW3.C3          = 76309;
            pIecpState->CscState.DW4.C4          = MOS_BITFIELD_VALUE((uint32_t)-25675, 19);
            pIecpState->CscState.DW5.C5          = MOS_BITFIELD_VALUE((uint32_t)-53279, 19);
            pIecpState->CscState.DW6.C6          = 76309;
            pIecpState->CscState.DW7.C7          = 132201;
            pIecpState->CscState.DW8.C8          = 0;
            pIecpState->CscState.DW9.OffsetIn1   = MOS_BITFIELD_VALUE((uint32_t)-2048, 16);
            pIecpState->CscState.DW9.OffsetOut1  = 0;
            pIecpState->CscState.DW10.OffsetIn2  = MOS_BITFIELD_VALUE((uint32_t)-16384, 16);
            pIecpState->CscState.DW10.OffsetOut2 = 0;
            pIecpState->CscState.DW11.OffsetIn3  = MOS_BITFIELD_VALUE((uint32_t)-16384, 16);
            pIecpState->CscState.DW11.OffsetOut3 = 0;
        }
        else if (pVeboxGamutParams->ColorSpace == MHW_CSpace_BT709 ||
                 pVeboxGamutParams->ColorSpace == MHW_CSpace_xvYCC709 ||
                 pVeboxGamutParams->ColorSpace == MHW_CSpace_BT709_FullRange)
        {
            pIecpState->CscState.DW0.C0          = 76309;
            pIecpState->CscState.DW1.C1          = 0;
            pIecpState->CscState.DW2.C2          = 117489;
            pIecpState->CscState.DW3.C3          = 76309;
            pIecpState->CscState.DW4.C4          = MOS_BITFIELD_VALUE((uint32_t)-13975, 19);
            pIecpState->CscState.DW5.C5          = MOS_BITFIELD_VALUE((uint32_t)-34925, 19);
            pIecpState->CscState.DW6.C6          = 76309;
            pIecpState->CscState.DW7.C7          = 138438;
            pIecpState->CscState.DW8.C8          = 0;
            pIecpState->CscState.DW9.OffsetIn1   = MOS_BITFIELD_VALUE((uint32_t)-2048, 16);
            pIecpState->CscState.DW9.OffsetOut1  = 0;
            pIecpState->CscState.DW10.OffsetIn2  = MOS_BITFIELD_VALUE((uint32_t)-16384, 16);
            pIecpState->CscState.DW10.OffsetOut2 = 0;
            pIecpState->CscState.DW11.OffsetIn3  = MOS_BITFIELD_VALUE((uint32_t)-16384, 16);
            pIecpState->CscState.DW11.OffsetOut3 = 0;
        }
        else if (pVeboxGamutParams->ColorSpace == MHW_CSpace_BT2020)
        {
            pIecpState->CscState.DW0.C0          = 76309;
            pIecpState->CscState.DW1.C1          = 0;
            pIecpState->CscState.DW2.C2          = 110014;
            pIecpState->CscState.DW3.C3          = 76309;
            pIecpState->CscState.DW4.C4          = MOS_BITFIELD_VALUE((uint32_t)-12277, 19);
            pIecpState->CscState.DW5.C5          = MOS_BITFIELD_VALUE((uint32_t)-42626, 19);
            pIecpState->CscState.DW6.C6          = 76309;
            pIecpState->CscState.DW7.C7          = 140363;
            pIecpState->CscState.DW8.C8          = 0;
            pIecpState->CscState.DW9.OffsetIn1   = MOS_BITFIELD_VALUE((uint32_t)-2048, 16);
            pIecpState->CscState.DW10.OffsetIn2  = MOS_BITFIELD_VALUE((uint32_t)-16384, 16);
            pIecpState->CscState.DW11.OffsetIn3  = MOS_BITFIELD_VALUE((uint32_t)-16384, 16);
            pIecpState->CscState.DW9.OffsetOut1  = 0;
            pIecpState->CscState.DW10.OffsetOut2 = 0;
            pIecpState->CscState.DW11.OffsetOut3 = 0;
        }
        else
        {
            MHW_ASSERTMESSAGE("Unknown primary");
        }

        pGamutState->DW0.GlobalModeEnable = true;
        pGamutState->DW0.CmW              = 1023;
        if (pVeboxGamutParams->ColorSpace == MHW_CSpace_BT2020)
        {
            if (pVeboxGamutParams->dstColorSpace == MHW_CSpace_BT709)
            {
                pGamutState->DW3.C0 = 6762;
                pGamutState->DW3.C1 = MOS_BITFIELD_VALUE((uint32_t)-2393, 15);
                pGamutState->DW4.C2 = MOS_BITFIELD_VALUE((uint32_t)-297, 15);
                pGamutState->DW4.C3 = MOS_BITFIELD_VALUE((uint32_t)-498, 15);
                pGamutState->DW5.C4 = 4636;
                pGamutState->DW5.C5 = MOS_BITFIELD_VALUE((uint32_t)-35, 15);
                pGamutState->DW6.C6 = MOS_BITFIELD_VALUE((uint32_t)-75, 15);
                pGamutState->DW6.C7 = MOS_BITFIELD_VALUE((uint32_t)-412, 15);
                pGamutState->DW7.C8 = 4583;
            }
            else
            {
                pGamutState->DW3.C0 = 7276;
                pGamutState->DW3.C1 = MOS_BITFIELD_VALUE((uint32_t)-2818, 15);
                pGamutState->DW4.C2 = MOS_BITFIELD_VALUE((uint32_t)-362, 15);
                pGamutState->DW4.C3 = MOS_BITFIELD_VALUE((uint32_t)-662, 15);
                pGamutState->DW5.C4 = 4863;
                pGamutState->DW5.C5 = MOS_BITFIELD_VALUE((uint32_t)-107, 15);
                pGamutState->DW6.C6 = MOS_BITFIELD_VALUE((uint32_t)-65, 15);
                pGamutState->DW6.C7 = MOS_BITFIELD_VALUE((uint32_t)-393, 15);
                pGamutState->DW7.C8 = 4554;
            }
        }
        else
        {
            pGamutState->DW3.C0          = 4096;
            pGamutState->DW3.C1          = 0;
            pGamutState->DW4.C2          = 0;
            pGamutState->DW4.C3          = 0;
            pGamutState->DW5.C4          = 4096;
            pGamutState->DW5.C5          = 0;
            pGamutState->DW6.C6          = 0;
            pGamutState->DW6.C7          = 0;
            pGamutState->DW7.C8          = 4096;
            pGamutState->DW32.OffsetInR  = 0;
            pGamutState->DW32.OffsetInG  = 0;
            pGamutState->DW33.OffsetInB  = 0;
            pGamutState->DW33.OffsetOutB = 0;
            pGamutState->DW34.OffsetOutR = 0;
            pGamutState->DW34.OffsetOutG = 0;
        }

        if (pVeboxGamutParams->InputGammaValue == MHW_GAMMA_1P0)
        {
            pGamutState->DW20.PwlInvGammaPoint1  = 21;
            pGamutState->DW20.PwlInvGammaPoint2  = 43;
            pGamutState->DW20.PwlInvGammaPoint3  = 64;
            pGamutState->DW20.PwlInvGammaPoint4  = 85;
            pGamutState->DW21.PwlInvGammaPoint5  = 106;
            pGamutState->DW21.PwlInvGammaPoint6  = 128;
            pGamutState->DW21.PwlInvGammaPoint7  = 149;
            pGamutState->DW21.PwlInvGammaPoint8  = 170;
            pGamutState->DW22.PwlInvGammaPoint9  = 191;
            pGamutState->DW22.PwlInvGammaPoint10 = 213;
            pGamutState->DW22.PwlInvGammaPoint11 = 234;
            pGamutState->DW23.PwlInvGammaBias1   = 21;
            pGamutState->DW23.PwlInvGammaBias2   = 43;
            pGamutState->DW23.PwlInvGammaBias3   = 64;
            pGamutState->DW23.PwlInvGammaBias4   = 85;
            pGamutState->DW24.PwlInvGammaBias5   = 106;
            pGamutState->DW24.PwlInvGammaBias6   = 128;
            pGamutState->DW24.PwlInvGammaBias7   = 149;
            pGamutState->DW24.PwlInvGammaBias8   = 170;
            pGamutState->DW25.PwlInvGammaBias9   = 191;
            pGamutState->DW25.PwlInvGammaBias10  = 213;
            pGamutState->DW25.PwlInvGammaBias11  = 234;
            pGamutState->DW26.PwlInvGammaSlope0  = 256;
            pGamutState->DW26.PwlInvGammaSlope1  = 256;
            pGamutState->DW27.PwlInvGammaSlope2  = 256;
            pGamutState->DW27.PwlInvGammaSlope3  = 256;
            pGamutState->DW28.PwlInvGammaSlope4  = 256;
            pGamutState->DW28.PwlInvGammaSlope5  = 256;
            pGamutState->DW29.PwlInvGammaSlope6  = 256;
            pGamutState->DW29.PwlInvGammaSlope7  = 256;
            pGamutState->DW30.PwlInvGammaSlope8  = 256;
            pGamutState->DW30.PwlInvGammaSlope9  = 256;
            pGamutState->DW31.PwlInvGammaSlope10 = 256;
            pGamutState->DW31.PwlInvGammaSlope11 = 256;
        }
        else if (pVeboxGamutParams->InputGammaValue == MHW_GAMMA_2P2)
        {
            pGamutState->DW20.PwlInvGammaPoint1  = 40;
            pGamutState->DW20.PwlInvGammaPoint2  = 56;
            pGamutState->DW20.PwlInvGammaPoint3  = 80;
            pGamutState->DW20.PwlInvGammaPoint4  = 104;
            pGamutState->DW21.PwlInvGammaPoint5  = 128;
            pGamutState->DW21.PwlInvGammaPoint6  = 149;
            pGamutState->DW21.PwlInvGammaPoint7  = 170;
            pGamutState->DW21.PwlInvGammaPoint8  = 191;
            pGamutState->DW22.PwlInvGammaPoint9  = 207;
            pGamutState->DW22.PwlInvGammaPoint10 = 223;
            pGamutState->DW22.PwlInvGammaPoint11 = 239;
            pGamutState->DW23.PwlInvGammaBias1   = 4;
            pGamutState->DW23.PwlInvGammaBias2   = 9;
            pGamutState->DW23.PwlInvGammaBias3   = 20;
            pGamutState->DW23.PwlInvGammaBias4   = 35;
            pGamutState->DW24.PwlInvGammaBias5   = 55;
            pGamutState->DW24.PwlInvGammaBias6   = 78;
            pGamutState->DW24.PwlInvGammaBias7   = 105;
            pGamutState->DW24.PwlInvGammaBias8   = 135;
            pGamutState->DW25.PwlInvGammaBias9   = 161;
            pGamutState->DW25.PwlInvGammaBias10  = 190;
            pGamutState->DW25.PwlInvGammaBias11  = 221;
            pGamutState->DW26.PwlInvGammaSlope0  = 28;
            pGamutState->DW26.PwlInvGammaSlope1  = 76;
            pGamutState->DW27.PwlInvGammaSlope2  = 115;
            pGamutState->DW27.PwlInvGammaSlope3  = 165;
            pGamutState->DW28.PwlInvGammaSlope4  = 218;
            pGamutState->DW28.PwlInvGammaSlope5  = 270;
            pGamutState->DW29.PwlInvGammaSlope6  = 320;
            pGamutState->DW29.PwlInvGammaSlope7  = 372;
            pGamutState->DW30.PwlInvGammaSlope8  = 419;
            pGamutState->DW30.PwlInvGammaSlope9  = 459;
            pGamutState->DW31.PwlInvGammaSlope10 = 500;
            pGamutState->DW31.PwlInvGammaSlope11 = 542;
        }
        else if (pVeboxGamutParams->InputGammaValue == MHW_GAMMA_2P6)
        {
            pGamutState->DW20.PwlInvGammaPoint1  = 40;
            pGamutState->DW20.PwlInvGammaPoint2  = 56;
            pGamutState->DW20.PwlInvGammaPoint3  = 80;
            pGamutState->DW20.PwlInvGammaPoint4  = 104;
            pGamutState->DW21.PwlInvGammaPoint5  = 128;
            pGamutState->DW21.PwlInvGammaPoint6  = 149;
            pGamutState->DW21.PwlInvGammaPoint7  = 170;
            pGamutState->DW21.PwlInvGammaPoint8  = 191;
            pGamutState->DW22.PwlInvGammaPoint9  = 207;
            pGamutState->DW22.PwlInvGammaPoint10 = 223;
            pGamutState->DW22.PwlInvGammaPoint11 = 239;
            pGamutState->DW23.PwlInvGammaBias1   = 2;
            pGamutState->DW23.PwlInvGammaBias2   = 5;
            pGamutState->DW23.PwlInvGammaBias3   = 12;
            pGamutState->DW23.PwlInvGammaBias4   = 25;
            pGamutState->DW24.PwlInvGammaBias5   = 42;
            pGamutState->DW24.PwlInvGammaBias6   = 63;
            pGamutState->DW24.PwlInvGammaBias7   = 89;
            pGamutState->DW24.PwlInvGammaBias8   = 121;
            pGamutState->DW25.PwlInvGammaBias9   = 149;
            pGamutState->DW25.PwlInvGammaBias10  = 180;
            pGamutState->DW25.PwlInvGammaBias11  = 216;
            pGamutState->DW26.PwlInvGammaSlope0  = 13;
            pGamutState->DW26.PwlInvGammaSlope1  = 46;
            pGamutState->DW27.PwlInvGammaSlope2  = 80;
            pGamutState->DW27.PwlInvGammaSlope3  = 130;
            pGamutState->DW28.PwlInvGammaSlope4  = 188;
            pGamutState->DW28.PwlInvGammaSlope5  = 250;
            pGamutState->DW29.PwlInvGammaSlope6  = 314;
            pGamutState->DW29.PwlInvGammaSlope7  = 384;
            pGamutState->DW30.PwlInvGammaSlope8  = 449;
            pGamutState->DW30.PwlInvGammaSlope9  = 507;
            pGamutState->DW31.PwlInvGammaSlope10 = 569;
            pGamutState->DW31.PwlInvGammaSlope11 = 633;
        }
        else
        {
            MHW_ASSERTMESSAGE("Unknown InputGammaValue");
        }

        if (pVeboxGamutParams->OutputGammaValue == MHW_GAMMA_1P0)
        {
            pGamutState->DW8.PwlGammaPoint1   = 21;
            pGamutState->DW8.PwlGammaPoint2   = 43;
            pGamutState->DW8.PwlGammaPoint3   = 64;
            pGamutState->DW8.PwlGammaPoint4   = 85;
            pGamutState->DW9.PwlGammaPoint5   = 106;
            pGamutState->DW9.PwlGammaPoint6   = 128;
            pGamutState->DW9.PwlGammaPoint7   = 149;
            pGamutState->DW9.PwlGammaPoint8   = 170;
            pGamutState->DW10.PwlGammaPoint9  = 191;
            pGamutState->DW10.PwlGammaPoint10 = 213;
            pGamutState->DW10.PwlGammaPoint11 = 234;
            pGamutState->DW11.PwlGammaBias1   = 21;
            pGamutState->DW11.PwlGammaBias2   = 43;
            pGamutState->DW11.PwlGammaBias3   = 64;
            pGamutState->DW11.PwlGammaBias4   = 85;
            pGamutState->DW12.PwlGammaBias5   = 106;
            pGamutState->DW12.PwlGammaBias6   = 128;
            pGamutState->DW12.PwlGammaBias7   = 149;
            pGamutState->DW12.PwlGammaBias8   = 170;
            pGamutState->DW13.PwlGammaBias9   = 191;
            pGamutState->DW13.PwlGammaBias10  = 213;
            pGamutState->DW13.PwlGammaBias11  = 234;
            pGamutState->DW14.PwlGammaSlope0  = 256;
            pGamutState->DW14.PwlGammaSlope1  = 256;
            pGamutState->DW15.PwlGammaSlope2  = 256;
            pGamutState->DW15.PwlGammaSlope3  = 256;
            pGamutState->DW16.PwlGammaSlope4  = 256;
            pGamutState->DW16.PwlGammaSlope5  = 256;
            pGamutState->DW17.PwlGammaSlope6  = 256;
            pGamutState->DW17.PwlGammaSlope7  = 256;
            pGamutState->DW18.PwlGammaSlope8  = 256;
            pGamutState->DW18.PwlGammaSlope9  = 256;
            pGamutState->DW19.PwlGammaSlope10 = 256;
            pGamutState->DW19.PwlGammaSlope11 = 256;
        }
        else if (pVeboxGamutParams->OutputGammaValue == MHW_GAMMA_2P2)
        {
            pGamutState->DW8.PwlGammaPoint1   = 4;
            pGamutState->DW8.PwlGammaPoint2   = 10;
            pGamutState->DW8.PwlGammaPoint3   = 16;
            pGamutState->DW8.PwlGammaPoint4   = 27;
            pGamutState->DW9.PwlGammaPoint5   = 44;
            pGamutState->DW9.PwlGammaPoint6   = 64;
            pGamutState->DW9.PwlGammaPoint7   = 96;
            pGamutState->DW9.PwlGammaPoint8   = 128;
            pGamutState->DW10.PwlGammaPoint9  = 159;
            pGamutState->DW10.PwlGammaPoint10 = 191;
            pGamutState->DW10.PwlGammaPoint11 = 223;
            pGamutState->DW11.PwlGammaBias1   = 39;
            pGamutState->DW11.PwlGammaBias2   = 58;
            pGamutState->DW11.PwlGammaBias3   = 72;
            pGamutState->DW11.PwlGammaBias4   = 91;
            pGamutState->DW12.PwlGammaBias5   = 115;
            pGamutState->DW12.PwlGammaBias6   = 136;
            pGamutState->DW12.PwlGammaBias7   = 163;
            pGamutState->DW12.PwlGammaBias8   = 186;
            pGamutState->DW13.PwlGammaBias9   = 206;
            pGamutState->DW13.PwlGammaBias10  = 224;
            pGamutState->DW13.PwlGammaBias11  = 240;
            pGamutState->DW14.PwlGammaSlope0  = 255;
            pGamutState->DW14.PwlGammaSlope1  = 2474;
            pGamutState->DW15.PwlGammaSlope2  = 852;
            pGamutState->DW15.PwlGammaSlope3  = 596;
            pGamutState->DW16.PwlGammaSlope4  = 455;
            pGamutState->DW16.PwlGammaSlope5  = 345;
            pGamutState->DW17.PwlGammaSlope6  = 273;
            pGamutState->DW17.PwlGammaSlope7  = 221;
            pGamutState->DW18.PwlGammaSlope8  = 183;
            pGamutState->DW18.PwlGammaSlope9  = 160;
            pGamutState->DW19.PwlGammaSlope10 = 143;
            pGamutState->DW19.PwlGammaSlope11 = 130;
        }
        else if (pVeboxGamutParams->OutputGammaValue == MHW_GAMMA_2P6)
        {
            pGamutState->DW8.PwlGammaPoint1   = 4;
            pGamutState->DW8.PwlGammaPoint2   = 10;
            pGamutState->DW8.PwlGammaPoint3   = 16;
            pGamutState->DW8.PwlGammaPoint4   = 27;
            pGamutState->DW9.PwlGammaPoint5   = 44;
            pGamutState->DW9.PwlGammaPoint6   = 64;
            pGamutState->DW9.PwlGammaPoint7   = 96;
            pGamutState->DW9.PwlGammaPoint8   = 128;
            pGamutState->DW10.PwlGammaPoint9  = 159;
            pGamutState->DW10.PwlGammaPoint10 = 191;
            pGamutState->DW10.PwlGammaPoint11 = 223;
            pGamutState->DW11.PwlGammaBias1   = 52;
            pGamutState->DW11.PwlGammaBias2   = 73;
            pGamutState->DW11.PwlGammaBias3   = 88;
            pGamutState->DW11.PwlGammaBias4   = 107;
            pGamutState->DW12.PwlGammaBias5   = 130;
            pGamutState->DW12.PwlGammaBias6   = 150;
            pGamutState->DW12.PwlGammaBias7   = 175;
            pGamutState->DW12.PwlGammaBias8   = 195;
            pGamutState->DW13.PwlGammaBias9   = 213;
            pGamutState->DW13.PwlGammaBias10  = 228;
            pGamutState->DW13.PwlGammaBias11  = 242;
            pGamutState->DW14.PwlGammaSlope0  = 3309;
            pGamutState->DW14.PwlGammaSlope1  = 932;
            pGamutState->DW15.PwlGammaSlope2  = 622;
            pGamutState->DW15.PwlGammaSlope3  = 459;
            pGamutState->DW16.PwlGammaSlope4  = 335;
            pGamutState->DW16.PwlGammaSlope5  = 257;
            pGamutState->DW17.PwlGammaSlope6  = 203;
            pGamutState->DW17.PwlGammaSlope7  = 164;
            pGamutState->DW18.PwlGammaSlope8  = 141;
            pGamutState->DW18.PwlGammaSlope9  = 124;
            pGamutState->DW19.PwlGammaSlope10 = 112;
            pGamutState->DW19.PwlGammaSlope11 = 103;
        }
        else
        {
            MHW_ASSERTMESSAGE("Unknown OutputGammaValue");
        }
    }
    else if ((pVeboxGamutParams->ColorSpace == MHW_CSpace_BT2020 ||
                 (pVeboxGamutParams->ColorSpace == MHW_CSpace_BT2020_FullRange)) &&  // Need to refine
             pVeboxGamutParams->dstColorSpace != MHW_CSpace_BT2020_RGB &&            // BT2020 CSC case
             pVeboxGamutParams->dstColorSpace != MHW_CSpace_BT2020_stRGB)
    {
        // Gamut Expansion setting
        pGamutState->DW0.GlobalModeEnable = true;
        pGamutState->DW0.CmW              = 1023;  // Colorimetric accurate image

        if (pVeboxGamutParams->dstColorSpace == MHW_CSpace_BT709)
        {
            pGamutState->DW3.C0 = 6762;
            pGamutState->DW3.C1 = MOS_BITFIELD_VALUE((uint32_t)-2393, 15);
            pGamutState->DW4.C2 = MOS_BITFIELD_VALUE((uint32_t)-297, 15);
            pGamutState->DW4.C3 = MOS_BITFIELD_VALUE((uint32_t)-498, 15);
            pGamutState->DW5.C4 = 4636;
            pGamutState->DW5.C5 = MOS_BITFIELD_VALUE((uint32_t)-35, 15);
            pGamutState->DW6.C6 = MOS_BITFIELD_VALUE((uint32_t)-75, 15);
            pGamutState->DW6.C7 = MOS_BITFIELD_VALUE((uint32_t)-412, 15);
            pGamutState->DW7.C8 = 4583;
        }
        else
        {
            pGamutState->DW3.C0 = 7276;
            pGamutState->DW3.C1 = MOS_BITFIELD_VALUE((uint32_t)-2818, 15);
            pGamutState->DW4.C2 = MOS_BITFIELD_VALUE((uint32_t)-362, 15);
            pGamutState->DW4.C3 = MOS_BITFIELD_VALUE((uint32_t)-662, 15);
            pGamutState->DW5.C4 = 4863;
            pGamutState->DW5.C5 = MOS_BITFIELD_VALUE((uint32_t)-107, 15);
            pGamutState->DW6.C6 = MOS_BITFIELD_VALUE((uint32_t)-65, 15);
            pGamutState->DW6.C7 = MOS_BITFIELD_VALUE((uint32_t)-393, 15);
            pGamutState->DW7.C8 = 4554;
        }

        pGamutState->DW8.PwlGammaPoint1   = 5;
        pGamutState->DW8.PwlGammaPoint2   = 27;
        pGamutState->DW8.PwlGammaPoint3   = 50;
        pGamutState->DW8.PwlGammaPoint4   = 73;
        pGamutState->DW9.PwlGammaPoint5   = 96;
        pGamutState->DW9.PwlGammaPoint6   = 118;
        pGamutState->DW9.PwlGammaPoint7   = 141;
        pGamutState->DW9.PwlGammaPoint8   = 164;
        pGamutState->DW10.PwlGammaPoint9  = 187;
        pGamutState->DW10.PwlGammaPoint10 = 209;
        pGamutState->DW10.PwlGammaPoint11 = 232;

        pGamutState->DW11.PwlGammaBias1  = 21;
        pGamutState->DW11.PwlGammaBias2  = 77;
        pGamutState->DW11.PwlGammaBias3  = 110;
        pGamutState->DW11.PwlGammaBias4  = 134;
        pGamutState->DW12.PwlGammaBias5  = 155;
        pGamutState->DW12.PwlGammaBias6  = 173;
        pGamutState->DW12.PwlGammaBias7  = 190;
        pGamutState->DW12.PwlGammaBias8  = 204;
        pGamutState->DW13.PwlGammaBias9  = 218;
        pGamutState->DW13.PwlGammaBias10 = 231;
        pGamutState->DW13.PwlGammaBias11 = 243;

        pGamutState->DW14.PwlGammaSlope0  = 1156;
        pGamutState->DW14.PwlGammaSlope1  = 637;
        pGamutState->DW15.PwlGammaSlope2  = 361;
        pGamutState->DW15.PwlGammaSlope3  = 278;
        pGamutState->DW16.PwlGammaSlope4  = 233;
        pGamutState->DW16.PwlGammaSlope5  = 204;
        pGamutState->DW17.PwlGammaSlope6  = 184;
        pGamutState->DW17.PwlGammaSlope7  = 168;
        pGamutState->DW18.PwlGammaSlope8  = 156;
        pGamutState->DW18.PwlGammaSlope9  = 146;
        pGamutState->DW19.PwlGammaSlope10 = 137;
        pGamutState->DW19.PwlGammaSlope11 = 130;

        pGamutState->DW20.PwlInvGammaPoint1  = 21;
        pGamutState->DW20.PwlInvGammaPoint2  = 42;
        pGamutState->DW20.PwlInvGammaPoint3  = 63;
        pGamutState->DW20.PwlInvGammaPoint4  = 85;
        pGamutState->DW21.PwlInvGammaPoint5  = 106;
        pGamutState->DW21.PwlInvGammaPoint6  = 127;
        pGamutState->DW21.PwlInvGammaPoint7  = 148;
        pGamutState->DW21.PwlInvGammaPoint8  = 170;
        pGamutState->DW22.PwlInvGammaPoint9  = 191;
        pGamutState->DW22.PwlInvGammaPoint10 = 212;
        pGamutState->DW22.PwlInvGammaPoint11 = 234;

        pGamutState->DW23.PwlInvGammaBias1  = 5;
        pGamutState->DW23.PwlInvGammaBias2  = 11;
        pGamutState->DW23.PwlInvGammaBias3  = 20;
        pGamutState->DW23.PwlInvGammaBias4  = 32;
        pGamutState->DW24.PwlInvGammaBias5  = 47;
        pGamutState->DW24.PwlInvGammaBias6  = 66;
        pGamutState->DW24.PwlInvGammaBias7  = 88;
        pGamutState->DW24.PwlInvGammaBias8  = 114;
        pGamutState->DW25.PwlInvGammaBias9  = 143;
        pGamutState->DW25.PwlInvGammaBias10 = 177;
        pGamutState->DW25.PwlInvGammaBias11 = 214;

        pGamutState->DW26.PwlInvGammaSlope0  = 57;
        pGamutState->DW26.PwlInvGammaSlope1  = 73;
        pGamutState->DW27.PwlInvGammaSlope2  = 108;
        pGamutState->DW27.PwlInvGammaSlope3  = 145;
        pGamutState->DW28.PwlInvGammaSlope4  = 185;
        pGamutState->DW28.PwlInvGammaSlope5  = 225;
        pGamutState->DW29.PwlInvGammaSlope6  = 267;
        pGamutState->DW29.PwlInvGammaSlope7  = 310;
        pGamutState->DW30.PwlInvGammaSlope8  = 355;
        pGamutState->DW30.PwlInvGammaSlope9  = 400;
        pGamutState->DW31.PwlInvGammaSlope10 = 446;
        pGamutState->DW31.PwlInvGammaSlope11 = 494;

        pGamutState->DW32.OffsetInR  = 0;
        pGamutState->DW32.OffsetInG  = 0;
        pGamutState->DW33.OffsetInB  = 0;
        pGamutState->DW33.OffsetOutB = 0;
        pGamutState->DW34.OffsetOutG = 0;
        pGamutState->DW34.OffsetOutR = 0;

        // Back end CSC setting
        // Need to convert BT2020 YUV input to RGB before GE
        pIecpState->CscState.DW0.TransformEnable = true;

        if (IS_RGB_SWAP(pVeboxGamutParams->dstFormat))
        {
            pIecpState->CscState.DW0.YuvChannelSwap = true;
        }

        pIecpState->CscState.DW0.C0 = 76309;
        pIecpState->CscState.DW1.C1 = 0;
        pIecpState->CscState.DW2.C2 = 110014;
        pIecpState->CscState.DW3.C3 = 76309;
        pIecpState->CscState.DW4.C4 = MOS_BITFIELD_VALUE((uint32_t)-12277, 19);
        pIecpState->CscState.DW5.C5 = MOS_BITFIELD_VALUE((uint32_t)-42626, 19);
        pIecpState->CscState.DW6.C6 = 76309;
        pIecpState->CscState.DW7.C7 = 140363;
        pIecpState->CscState.DW8.C8 = 0;

        pIecpState->CscState.DW9.OffsetIn1   = MOS_BITFIELD_VALUE((uint32_t)-2048, 16);
        pIecpState->CscState.DW10.OffsetIn2  = MOS_BITFIELD_VALUE((uint32_t)-16384, 16);
        pIecpState->CscState.DW11.OffsetIn3  = MOS_BITFIELD_VALUE((uint32_t)-16384, 16);
        pIecpState->CscState.DW9.OffsetOut1  = 0;
        pIecpState->CscState.DW10.OffsetOut2 = 0;
        pIecpState->CscState.DW11.OffsetOut3 = 0;
    }
    else if (pVeboxGamutParams->ColorSpace == MHW_CSpace_BT2020 &&
             (pVeboxGamutParams->dstColorSpace == MHW_CSpace_BT2020_RGB ||
              pVeboxGamutParams->dstColorSpace == MHW_CSpace_BT2020_stRGB))  // BT2020 YUV to BT2020 RGB CSC
    {
        // Back end CSC setting
        // Convert BT2020 YUV input to BT2020 RGB
        pIecpState->CscState.DW0.TransformEnable = true;

        if (IS_RGB_SWAP(pVeboxGamutParams->dstFormat))
        {
            pIecpState->CscState.DW0.YuvChannelSwap = true;
        }

        if (pVeboxGamutParams->dstColorSpace == MHW_CSpace_BT2020_stRGB)
        {
            pIecpState->CscState.DW0.C0 = 65536;
            pIecpState->CscState.DW1.C1 = 0;
            pIecpState->CscState.DW2.C2 = 94482;
            pIecpState->CscState.DW3.C3 = 65536;
            pIecpState->CscState.DW4.C4 = MOS_BITFIELD_VALUE((uint32_t)-10543, 19);
            pIecpState->CscState.DW5.C5 = MOS_BITFIELD_VALUE((uint32_t)-36608, 19);
            pIecpState->CscState.DW6.C6 = 65536;
            pIecpState->CscState.DW7.C7 = 120547;
            pIecpState->CscState.DW8.C8 = 0;

            pIecpState->CscState.DW9.OffsetIn1  = MOS_BITFIELD_VALUE((uint32_t)-2048, 16);
            pIecpState->CscState.DW10.OffsetIn2 = MOS_BITFIELD_VALUE((uint32_t)-16384, 16);
            pIecpState->CscState.DW11.OffsetIn3 = MOS_BITFIELD_VALUE((uint32_t)-16384, 16);

            pIecpState->CscState.DW9.OffsetOut1  = 2048;
            pIecpState->CscState.DW10.OffsetOut2 = 2048;
            pIecpState->CscState.DW11.OffsetOut3 = 2048;
        }
        else
        {
            pIecpState->CscState.DW0.C0 = 76309;
            pIecpState->CscState.DW1.C1 = 0;
            pIecpState->CscState.DW2.C2 = 110014;
            pIecpState->CscState.DW3.C3 = 76309;
            pIecpState->CscState.DW4.C4 = MOS_BITFIELD_VALUE((uint32_t)-12277, 19);
            pIecpState->CscState.DW5.C5 = MOS_BITFIELD_VALUE((uint32_t)-42626, 19);
            pIecpState->CscState.DW6.C6 = 76309;
            pIecpState->CscState.DW7.C7 = 140363;
            pIecpState->CscState.DW8.C8 = 0;

            pIecpState->CscState.DW9.OffsetIn1  = MOS_BITFIELD_VALUE((uint32_t)-2048, 16);
            pIecpState->CscState.DW10.OffsetIn2 = MOS_BITFIELD_VALUE((uint32_t)-16384, 16);
            pIecpState->CscState.DW11.OffsetIn3 = MOS_BITFIELD_VALUE((uint32_t)-16384, 16);

            pIecpState->CscState.DW9.OffsetOut1  = 0;
            pIecpState->CscState.DW10.OffsetOut2 = 0;
            pIecpState->CscState.DW11.OffsetOut3 = 0;
        }
    }
    else
    {
        MHW_ASSERTMESSAGE("Unknown Handled ColorSpace");
    }

finish:
    return eStatus;
}

MOS_STATUS MhwVeboxInterfaceG9::AddVeboxIecpAceState(
    PMHW_VEBOX_IECP_PARAMS pVeboxIecpParams)
{
    PMHW_ACE_PARAMS  pAceParams;
    PMHW_LACE_PARAMS pLaceParams;
    PMHW_VEBOX_HEAP  pVeboxHeap;
    int32_t          uiOffset;
    MOS_STATUS       eStatus = MOS_STATUS_SUCCESS;

    mhw_vebox_g9_X::VEBOX_IECP_STATE_CMD *pVeboxIecpState;

    MHW_CHK_NULL(pVeboxIecpParams);
    MHW_CHK_NULL(m_veboxHeap);

    pVeboxHeap = m_veboxHeap;
    uiOffset   = pVeboxHeap->uiCurState * pVeboxHeap->uiInstanceSize;

    pVeboxIecpState = (mhw_vebox_g9_X::VEBOX_IECP_STATE_CMD *)(pVeboxHeap->pLockedDriverResourceMem +
                                                               pVeboxHeap->uiIecpStateOffset +
                                                               uiOffset);
    MHW_CHK_NULL(pVeboxIecpState);

    MhwVeboxInterfaceGeneric<mhw_vebox_g9_X>::SetVeboxAceLaceState(pVeboxIecpParams, pVeboxIecpState);

    if (pVeboxIecpParams->ColorPipeParams.bActive &&
        pVeboxIecpParams->ColorPipeParams.bEnableLACE)
    {
        pLaceParams = &pVeboxIecpParams->ColorPipeParams.LaceParams;

        pVeboxIecpState->AceState.DW0.MinAceLuma  = pLaceParams->wMinAceLuma;
        pVeboxIecpState->AceState.DW12.MaxAceLuma = pLaceParams->wMaxAceLuma;
    }

finish:
    return eStatus;
}

MOS_STATUS MhwVeboxInterfaceG9::AddVeboxDiIecp(
    PMOS_COMMAND_BUFFER           pCmdBuffer,
    PMHW_VEBOX_DI_IECP_CMD_PARAMS pVeboxDiIecpCmdParams)
{
    MOS_STATUS                      eStatus;
    PMOS_INTERFACE                  pOsInterface;
    MHW_RESOURCE_PARAMS             ResourceParams;
    mhw_vebox_g9_X::VEB_DI_IECP_CMD cmd;

    MHW_CHK_NULL(m_osInterface);
    MHW_CHK_NULL(pCmdBuffer);
    MHW_CHK_NULL(pVeboxDiIecpCmdParams);
    MHW_ASSERT(MOS_IS_ALIGNED(pVeboxDiIecpCmdParams->dwCurrInputSurfOffset, MHW_PAGE_SIZE));  // offset should be aligned with 4KB
    MHW_ASSERT(MOS_IS_ALIGNED(pVeboxDiIecpCmdParams->dwPrevInputSurfOffset, MHW_PAGE_SIZE));  // offset should be aligned with 4KB

    // Initialize
    eStatus      = MOS_STATUS_SUCCESS;
    pOsInterface = m_osInterface;

    if (pVeboxDiIecpCmdParams->pOsResCurrInput)
    {
        cmd.DW2.CurrentFrameSurfaceControlBitsMemoryCompressionEnable =
            (pVeboxDiIecpCmdParams->CurInputSurfMMCState != MOS_MEMCOMP_DISABLED) ? 1 : 0;
        cmd.DW2.CurrentFrameSurfaceControlBitsMemoryCompressionMode =
            (pVeboxDiIecpCmdParams->CurInputSurfMMCState == MOS_MEMCOMP_HORIZONTAL) ? 0 : 1;

        MOS_ZeroMemory(&ResourceParams, sizeof(ResourceParams));
        ResourceParams.dwLsbNum        = MHW_VEBOX_DI_IECP_SHIFT;
        ResourceParams.presResource    = pVeboxDiIecpCmdParams->pOsResCurrInput;
        ResourceParams.dwOffset        = pVeboxDiIecpCmdParams->dwCurrInputSurfOffset;
        ResourceParams.pdwCmd          = & (cmd.DW2.Value);
        ResourceParams.dwLocationInCmd = 2;
        ResourceParams.HwCommandType   = MOS_VEBOX_DI_IECP;
        MHW_CHK_STATUS(pfnAddResourceToCmd(
            pOsInterface,
            pCmdBuffer,
            &ResourceParams));
    }

    // Remove this after VPHAL moving to new cmd definition --- assign MOCS/MMC bits directly
    if (pVeboxDiIecpCmdParams->CurInputSurfMMCState == 0)
    {
        // bit 0 ~ 10 is MOCS/MMC bits
        cmd.DW2.Value = (cmd.DW2.Value & 0xFFFFF800) + pVeboxDiIecpCmdParams->CurrInputSurfCtrl.Value;
    }

    if (pVeboxDiIecpCmdParams->pOsResPrevInput)
    {
        MOS_ZeroMemory(&ResourceParams, sizeof(ResourceParams));
        ResourceParams.presResource    = pVeboxDiIecpCmdParams->pOsResPrevInput;
        ResourceParams.dwOffset        = pVeboxDiIecpCmdParams->PrevInputSurfCtrl.Value + pVeboxDiIecpCmdParams->dwPrevInputSurfOffset;
        ResourceParams.pdwCmd          = & (cmd.DW4.Value);
        ResourceParams.dwLocationInCmd = 4;
        ResourceParams.HwCommandType   = MOS_VEBOX_DI_IECP;

        MHW_CHK_STATUS(pfnAddResourceToCmd(
            pOsInterface,
            pCmdBuffer,
            &ResourceParams));
    }

    if (pVeboxDiIecpCmdParams->pOsResStmmInput)
    {
        MOS_ZeroMemory(&ResourceParams, sizeof(ResourceParams));
        ResourceParams.presResource    = pVeboxDiIecpCmdParams->pOsResStmmInput;
        ResourceParams.dwOffset        = pVeboxDiIecpCmdParams->StmmInputSurfCtrl.Value;
        ResourceParams.pdwCmd          = & (cmd.DW6.Value);
        ResourceParams.dwLocationInCmd = 6;
        ResourceParams.HwCommandType   = MOS_VEBOX_DI_IECP;

        MHW_CHK_STATUS(pfnAddResourceToCmd(
            pOsInterface,
            pCmdBuffer,
            &ResourceParams));
    }

    if (pVeboxDiIecpCmdParams->pOsResStmmOutput)
    {
        MOS_ZeroMemory(&ResourceParams, sizeof(ResourceParams));
        ResourceParams.presResource    = pVeboxDiIecpCmdParams->pOsResStmmOutput;
        ResourceParams.dwOffset        = pVeboxDiIecpCmdParams->StmmOutputSurfCtrl.Value;
        ResourceParams.pdwCmd          = & (cmd.DW8.Value);
        ResourceParams.dwLocationInCmd = 8;
        ResourceParams.bIsWritable     = true;
        ResourceParams.HwCommandType   = MOS_VEBOX_DI_IECP;

        MHW_CHK_STATUS(pfnAddResourceToCmd(
            pOsInterface,
            pCmdBuffer,
            &ResourceParams));
    }

    if (pVeboxDiIecpCmdParams->pOsResDenoisedCurrOutput)
    {
        MOS_ZeroMemory(&ResourceParams, sizeof(ResourceParams));
        ResourceParams.presResource    = pVeboxDiIecpCmdParams->pOsResDenoisedCurrOutput;
        ResourceParams.dwOffset        = pVeboxDiIecpCmdParams->DenoisedCurrOutputSurfCtrl.Value;
        ResourceParams.pdwCmd          = & (cmd.DW10.Value);
        ResourceParams.dwLocationInCmd = 10;
        ResourceParams.bIsWritable     = true;
        ResourceParams.HwCommandType   = MOS_VEBOX_DI_IECP;

        MHW_CHK_STATUS(pfnAddResourceToCmd(
            pOsInterface,
            pCmdBuffer,
            &ResourceParams));
    }

    if (pVeboxDiIecpCmdParams->pOsResCurrOutput)
    {
        MOS_ZeroMemory(&ResourceParams, sizeof(ResourceParams));
        ResourceParams.presResource    = pVeboxDiIecpCmdParams->pOsResCurrOutput;
        ResourceParams.dwOffset        = pVeboxDiIecpCmdParams->CurrOutputSurfCtrl.Value + pVeboxDiIecpCmdParams->dwCurrOutputSurfOffset;
        ResourceParams.pdwCmd          = & (cmd.DW12.Value);
        ResourceParams.dwLocationInCmd = 12;
        ResourceParams.bIsWritable     = true;
        ResourceParams.HwCommandType   = MOS_VEBOX_DI_IECP;

        MHW_CHK_STATUS(pfnAddResourceToCmd(
            pOsInterface,
            pCmdBuffer,
            &ResourceParams));
    }

    if (pVeboxDiIecpCmdParams->pOsResPrevOutput)
    {
        MOS_ZeroMemory(&ResourceParams, sizeof(ResourceParams));
        ResourceParams.presResource    = pVeboxDiIecpCmdParams->pOsResPrevOutput;
        ResourceParams.dwOffset        = pVeboxDiIecpCmdParams->PrevOutputSurfCtrl.Value;
        ResourceParams.pdwCmd          = & (cmd.DW14.Value);
        ResourceParams.dwLocationInCmd = 14;
        ResourceParams.bIsWritable     = true;
        ResourceParams.HwCommandType   = MOS_VEBOX_DI_IECP;

        MHW_CHK_STATUS(pfnAddResourceToCmd(
            pOsInterface,
            pCmdBuffer,
            &ResourceParams));
    }

    if (pVeboxDiIecpCmdParams->pOsResStatisticsOutput)
    {
        MOS_ZeroMemory(&ResourceParams, sizeof(ResourceParams));
        ResourceParams.presResource    = pVeboxDiIecpCmdParams->pOsResStatisticsOutput;
        ResourceParams.dwOffset        = pVeboxDiIecpCmdParams->StatisticsOutputSurfCtrl.Value;
        ResourceParams.pdwCmd          = & (cmd.DW16.Value);
        ResourceParams.dwLocationInCmd = 16;
        ResourceParams.bIsWritable     = true;
        ResourceParams.HwCommandType   = MOS_VEBOX_DI_IECP;

        MHW_CHK_STATUS(pfnAddResourceToCmd(
            pOsInterface,
            pCmdBuffer,
            &ResourceParams));
    }

    if (pVeboxDiIecpCmdParams->pOsResAlphaOrVignette)
    {
        MOS_ZeroMemory(&ResourceParams, sizeof(ResourceParams));
        ResourceParams.presResource    = pVeboxDiIecpCmdParams->pOsResAlphaOrVignette;
        ResourceParams.dwOffset        = pVeboxDiIecpCmdParams->AlphaOrVignetteSurfCtrl.Value;
        ResourceParams.pdwCmd          = & (cmd.DW18.Value);
        ResourceParams.dwLocationInCmd = 18;
        ResourceParams.bIsWritable     = true;
        ResourceParams.HwCommandType   = MOS_VEBOX_DI_IECP;

        MHW_CHK_STATUS(pfnAddResourceToCmd(
            pOsInterface,
            pCmdBuffer,
            &ResourceParams));
    }

    if (pVeboxDiIecpCmdParams->pOsResLaceOrAceOrRgbHistogram)
    {
        MOS_ZeroMemory(&ResourceParams, sizeof(ResourceParams));
        ResourceParams.presResource    = pVeboxDiIecpCmdParams->pOsResLaceOrAceOrRgbHistogram;
        ResourceParams.dwOffset        = pVeboxDiIecpCmdParams->LaceOrAceOrRgbHistogramSurfCtrl.Value;
        ResourceParams.pdwCmd          = & (cmd.DW20.Value);
        ResourceParams.dwLocationInCmd = 20;
        ResourceParams.bIsWritable     = true;
        ResourceParams.HwCommandType   = MOS_VEBOX_DI_IECP;

        MHW_CHK_STATUS(pfnAddResourceToCmd(
            pOsInterface,
            pCmdBuffer,
            &ResourceParams));
    }

    if (pVeboxDiIecpCmdParams->pOsResSkinScoreSurface)
    {
        MOS_ZeroMemory(&ResourceParams, sizeof(ResourceParams));
        ResourceParams.presResource    = pVeboxDiIecpCmdParams->pOsResSkinScoreSurface;
        ResourceParams.dwOffset        = pVeboxDiIecpCmdParams->SkinScoreSurfaceSurfCtrl.Value;
        ResourceParams.pdwCmd          = & (cmd.DW22.Value);
        ResourceParams.dwLocationInCmd = 22;
        ResourceParams.bIsWritable     = true;
        ResourceParams.HwCommandType   = MOS_VEBOX_DI_IECP;

        MHW_CHK_STATUS(pfnAddResourceToCmd(
            pOsInterface,
            pCmdBuffer,
            &ResourceParams));
    }

    cmd.DW1.EndingX   = pVeboxDiIecpCmdParams->dwEndingX;
    cmd.DW1.StartingX = pVeboxDiIecpCmdParams->dwStartingX;

    pOsInterface->pfnAddCommand(pCmdBuffer, &cmd, cmd.byteSize);

finish:
    return eStatus;
}

MOS_STATUS MhwVeboxInterfaceG9::AddVeboxDndiState(
    PMHW_VEBOX_DNDI_PARAMS pVeboxDndiParams)
{
    PMHW_VEBOX_HEAP pVeboxHeap;
    uint32_t        uiOffset;
    MOS_STATUS      eStatus = MOS_STATUS_SUCCESS;

    mhw_vebox_g9_X::VEBOX_DNDI_STATE_CMD *pVeboxDndiState, DndiStateCmd;

    MHW_CHK_NULL(pVeboxDndiParams);
    MHW_CHK_NULL(m_veboxHeap);

    pVeboxHeap = m_veboxHeap;
    uiOffset   = pVeboxHeap->uiCurState * pVeboxHeap->uiInstanceSize;
    pVeboxDndiState =
        (mhw_vebox_g9_X::VEBOX_DNDI_STATE_CMD *)(pVeboxHeap->pLockedDriverResourceMem +
                                                 pVeboxHeap->uiDndiStateOffset +
                                                 uiOffset);
    MHW_CHK_NULL(pVeboxDndiState);
    *pVeboxDndiState = DndiStateCmd;

    pVeboxDndiState->DW0.DenoiseMovingPixelThreshold =
        pVeboxDndiParams->dwDenoiseMPThreshold;
    pVeboxDndiState->DW0.DenoiseHistoryIncrease =
        pVeboxDndiParams->dwDenoiseHistoryDelta;
    pVeboxDndiState->DW0.DenoiseMaximumHistory =
        pVeboxDndiParams->dwDenoiseMaximumHistory;
    pVeboxDndiState->DW0.DenoiseStadThreshold =
        pVeboxDndiParams->dwDenoiseSTADThreshold;
    pVeboxDndiState->DW1.DenoiseAsdThreshold =
        pVeboxDndiParams->dwDenoiseASDThreshold;
    pVeboxDndiState->DW1.LowTemporalDifferenceThreshold =
        pVeboxDndiParams->dwLTDThreshold;
    pVeboxDndiState->DW1.TemporalDifferenceThreshold =
        pVeboxDndiParams->dwTDThreshold;
    pVeboxDndiState->DW2.DenoiseThresholdForSumOfComplexityMeasure =
        pVeboxDndiParams->dwDenoiseSCMThreshold;
    pVeboxDndiState->DW2.ProgressiveDn =
        pVeboxDndiParams->bProgressiveDN;

    //DW5 to DW11 for new 5x5 spatial denoise filter on Gen9
    pVeboxDndiState->DW5.DnWr0 =
        pVeboxDndiParams->dwPixRangeWeight[0];
    pVeboxDndiState->DW5.DnWr1 =
        pVeboxDndiParams->dwPixRangeWeight[1];
    pVeboxDndiState->DW5.DnWr2 =
        pVeboxDndiParams->dwPixRangeWeight[2];
    pVeboxDndiState->DW5.DnWr3 =
        pVeboxDndiParams->dwPixRangeWeight[3];
    pVeboxDndiState->DW5.DnWr4 =
        pVeboxDndiParams->dwPixRangeWeight[4];
    pVeboxDndiState->DW5.DnWr5 =
        pVeboxDndiParams->dwPixRangeWeight[5];

    pVeboxDndiState->DW7.DnPrt5 =
        pVeboxDndiParams->dwPixRangeThreshold[5];
    pVeboxDndiState->DW8.DnPrt4 =
        pVeboxDndiParams->dwPixRangeThreshold[4];
    pVeboxDndiState->DW8.DnPrt3 =
        pVeboxDndiParams->dwPixRangeThreshold[3];
    pVeboxDndiState->DW9.DnPrt2 =
        pVeboxDndiParams->dwPixRangeThreshold[2];
    pVeboxDndiState->DW9.DnPrt1 =
        pVeboxDndiParams->dwPixRangeThreshold[1];
    pVeboxDndiState->DW10.DnPrt0 =
        pVeboxDndiParams->dwPixRangeThreshold[0];

    pVeboxDndiState->DW16.DnDiTopFirst =
        pVeboxDndiParams->bDNDITopFirst;

    pVeboxDndiState->DW17.ProgressiveCadenceReconstructionFor1StFieldOfCurrentFrame =
        pVeboxDndiParams->dwFMDFirstFieldCurrFrame;
    pVeboxDndiState->DW17.ProgressiveCadenceReconstructionFor2NdFieldOfPreviousFrame =
        pVeboxDndiParams->dwFMDSecondFieldPrevFrame;

    // Chroma Denoise
    pVeboxDndiState->DW4.ChromaLowTemporalDifferenceThreshold =
        pVeboxDndiParams->dwChromaLTDThreshold;
    pVeboxDndiState->DW4.ChromaTemporalDifferenceThreshold =
        pVeboxDndiParams->dwChromaTDThreshold;
    pVeboxDndiState->DW4.ChromaDenoiseStadThreshold =
        pVeboxDndiParams->dwChromaSTADThreshold;
    pVeboxDndiState->DW4.ChromaDenoiseEnable =
        pVeboxDndiParams->bChromaDNEnable;

    // Hot Pixel
    pVeboxDndiState->DW3.HotPixelThreshold =
        pVeboxDndiParams->dwHotPixelThreshold;
    pVeboxDndiState->DW3.HotPixelCount =
        pVeboxDndiParams->dwHotPixelCount;

    pVeboxDndiState->DW3.BlockNoiseEstimateNoiseThreshold = ABSMAXABSMIN_THRESHOLD_DEFAULT_G9;

    pVeboxDndiState->DW6.DnThmin = 512;
    pVeboxDndiState->DW6.DnThmax = 2048;

    pVeboxDndiState->DW7.DnDynThmin = 256;

    pVeboxDndiState->DW10.DnWd20 = 10;
    pVeboxDndiState->DW10.DnWd21 = 10;
    pVeboxDndiState->DW10.DnWd22 = 8;
    pVeboxDndiState->DW11.DnWd00 = 12;
    pVeboxDndiState->DW11.DnWd01 = 12;
    pVeboxDndiState->DW11.DnWd02 = 10;
    pVeboxDndiState->DW11.DnWd10 = 12;
    pVeboxDndiState->DW11.DnWd11 = 11;
    pVeboxDndiState->DW11.DnWd12 = 10;

    pVeboxDndiState->DW12.StmmC2 = 2;

    pVeboxDndiState->DW13.MaximumStmm                                    = 150;
    pVeboxDndiState->DW13.MultiplierForVecm                              = 30;
    pVeboxDndiState->DW13.BlendingConstantAcrossTimeForSmallValuesOfStmm = 125;
    pVeboxDndiState->DW13.BlendingConstantAcrossTimeForLargeValuesOfStmm = 64;

    pVeboxDndiState->DW14.SdiDelta        = 5;
    pVeboxDndiState->DW14.SdiThreshold    = 100;
    pVeboxDndiState->DW14.StmmOutputShift = 5;
    pVeboxDndiState->DW14.StmmShiftUp     = 1;
    pVeboxDndiState->DW14.MinimumStmm     = 118;

    pVeboxDndiState->DW15.FmdTemporalDifferenceThreshold   = 175;
    pVeboxDndiState->DW15.SdiFallbackMode2ConstantAngle2X1 = 37;
    pVeboxDndiState->DW15.SdiFallbackMode1T2Constant       = 100;
    pVeboxDndiState->DW15.SdiFallbackMode1T1Constant       = 50;

    pVeboxDndiState->DW16.McdiEnable                      = 1;
    pVeboxDndiState->DW16.FmdTearThreshold                = 2;
    pVeboxDndiState->DW16.Fmd2VerticalDifferenceThreshold = 100;
    pVeboxDndiState->DW16.Fmd1VerticalDifferenceThreshold = 16;

finish:
    return eStatus;
}

MOS_STATUS MhwVeboxInterfaceG9::AddVeboxIecpState(
    PMHW_VEBOX_IECP_PARAMS pVeboxIecpParams)
{
    bool                                  bEnableFECSC = false;
    bool                                  bEnableLACE  = false;
    PMHW_FORWARD_GAMMA_SEG                pFwdGammaSeg;
    PMHW_VEBOX_HEAP                       pVeboxHeap;
    uint32_t                              uiOffset;
    MOS_STATUS                            eStatus = MOS_STATUS_SUCCESS;
    mhw_vebox_g9_X::VEBOX_IECP_STATE_CMD  *pVeboxIecpState, IecpState;

    MHW_CHK_NULL(pVeboxIecpParams);
    MHW_CHK_NULL(m_veboxHeap);

    pVeboxHeap      = m_veboxHeap;
    uiOffset        = pVeboxHeap->uiCurState * pVeboxHeap->uiInstanceSize;
    pVeboxIecpState = (mhw_vebox_g9_X::VEBOX_IECP_STATE_CMD *)(pVeboxHeap->pLockedDriverResourceMem +
                                                               pVeboxHeap->uiIecpStateOffset +
                                                               uiOffset);
    MHW_CHK_NULL(pVeboxIecpState);

    *pVeboxIecpState = IecpState;
    IecpStateInitialization(pVeboxIecpState);

    if (pVeboxIecpParams->ColorPipeParams.bActive)
    {
        // Enable STD/E (Skin Tone Detection/Enhancement)
        SetVeboxIecpStateSTE(
            &pVeboxIecpState->StdSteState,
            &pVeboxIecpParams->ColorPipeParams);

        // Enable ACE (Automatic Contrast Enhancement). Enable LACE if it's enabled.
        if (pVeboxIecpParams->ColorPipeParams.bEnableACE)
        {
            MhwVeboxInterfaceGeneric<mhw_vebox_g9_X>::SetVeboxIecpStateACELACE(
                &pVeboxIecpState->AceState,
                &pVeboxIecpState->AlphaAoiState,
                (pVeboxIecpParams->ColorPipeParams.bEnableLACE == true)? true : false);
        }

        // Enable TCC (Total Color Control)
        if (pVeboxIecpParams->ColorPipeParams.bEnableTCC)
        {
            MhwVeboxInterfaceGeneric<mhw_vebox_g9_X>::SetVeboxIecpStateTCC(
                &pVeboxIecpState->TccState,
                &pVeboxIecpParams->ColorPipeParams);
        }
    }

    if (pVeboxIecpParams->CapPipeParams.bActive)
    {
        // IECP needs to operate in YUV space
        if ((pVeboxIecpParams->srcFormat != Format_AYUV) &&
            (pVeboxIecpParams->dstFormat == Format_AYUV ||
                pVeboxIecpParams->dstFormat == Format_Y416 ||
                pVeboxIecpParams->ProcAmpParams.bActive ||
                pVeboxIecpParams->ColorPipeParams.bActive))
        {
            bEnableFECSC = true;
        }
        else if (pVeboxIecpParams->srcFormat == Format_A16B16G16R16 &&
                 (pVeboxIecpParams->ProcAmpParams.bActive ||
                     pVeboxIecpParams->ColorPipeParams.bActive))
        {
            bEnableFECSC = true;
        }
        else if (pVeboxIecpParams->CapPipeParams.FECSCParams.bActive)
        {
            bEnableFECSC = true;
        }
        else
        {
            bEnableFECSC = false;
        }

        // Enable Front End CSC so that input to IECP will be in YUV color space
        if (bEnableFECSC)
        {
            MhwVeboxInterfaceGeneric<mhw_vebox_g9_X>::SetVeboxIecpStateFecsc(&pVeboxIecpState->FrontEndCsc, pVeboxIecpParams);
        }

        // Enable Color Correction Matrix
        if (pVeboxIecpParams->CapPipeParams.ColorCorrectionParams.bActive)
        {
            MhwVeboxInterfaceGeneric<mhw_vebox_g9_X>::SetVeboxIecpStateCcm(
                pVeboxIecpState,
                &pVeboxIecpParams->CapPipeParams,
                4096);
        }
    }

    // Enable Back End CSC for capture pipeline or Vebox output pipe
    if (pVeboxIecpParams->CapPipeParams.bActive ||
        pVeboxIecpParams->bCSCEnable)
    {
        SetVeboxIecpStateBecsc(
            pVeboxIecpState,
            pVeboxIecpParams,
            bEnableFECSC);
    }

    // Enable ProcAmp
    if (pVeboxIecpParams->ProcAmpParams.bActive &&
        pVeboxIecpParams->ProcAmpParams.bEnabled)
    {
        MhwVeboxInterfaceGeneric<mhw_vebox_g9_X>::SetVeboxIecpStateProcAmp(
            &pVeboxIecpState->ProcampState,
            &pVeboxIecpParams->ProcAmpParams);
    }

    if (pVeboxIecpParams && pVeboxIecpParams->CapPipeParams.bActive)
    {
        MhwVeboxInterfaceGeneric<mhw_vebox_g9_X>::AddVeboxCapPipeState(&pVeboxIecpParams->CapPipeParams);
    }

    if (pVeboxIecpParams &&
        pVeboxIecpParams->CapPipeParams.bActive &&
        pVeboxIecpParams->CapPipeParams.FwdGammaParams.bActive)
    {
        pFwdGammaSeg =
            (PMHW_FORWARD_GAMMA_SEG)(pVeboxHeap->pLockedDriverResourceMem +
                                     pVeboxHeap->uiGammaCorrectionStateOffset +
                                     uiOffset);
        MHW_CHK_NULL(pFwdGammaSeg);
        MOS_SecureMemcpy(
            pFwdGammaSeg,
            sizeof(MHW_FORWARD_GAMMA_SEG) * MHW_FORWARD_GAMMA_SEGMENT_CONTROL_POINT_G9,
            &pVeboxIecpParams->CapPipeParams.FwdGammaParams.Segment[0],
            sizeof(MHW_FORWARD_GAMMA_SEG) * MHW_FORWARD_GAMMA_SEGMENT_CONTROL_POINT_G9);
    }

finish:
    return eStatus;
}

MOS_STATUS MhwVeboxInterfaceG9::VeboxAdjustBoundary(
    PMHW_VEBOX_SURFACE_PARAMS pSurfaceParam,
    uint32_t                  *pdwSurfaceWidth,
    uint32_t                  *pdwSurfaceHeight,
    bool                      bDIEnable)
{
    uint16_t   wWidthAlignUnit;
    uint16_t   wHeightAlignUnit;
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_CHK_NULL(pSurfaceParam);
    MHW_CHK_NULL(pdwSurfaceWidth);
    MHW_CHK_NULL(pdwSurfaceHeight);

    // initialize
    wHeightAlignUnit = 1;
    wWidthAlignUnit  = 1;

    switch (pSurfaceParam->Format)
    {
        case Format_NV12:
            wHeightAlignUnit = bDIEnable ? 4 : 2;
            wWidthAlignUnit  = 2;
            break;

        case Format_YUYV:
        case Format_YUY2:
        case Format_UYVY:
        case Format_YVYU:
        case Format_VYUY:
            wHeightAlignUnit = bDIEnable ? 2 : 1;
            wWidthAlignUnit  = 2;
            break;

        case Format_AYUV:
        case Format_Y416:
            wHeightAlignUnit = 1;
            wWidthAlignUnit  = 2;
            break;

            // For other formats, we will not do any special alignment
        case Format_A8R8G8B8:
        case Format_X8R8G8B8:
        case Format_A8B8G8R8:
        case Format_X8B8G8R8:
        case Format_L8:
        default:
            break;
    }

    //When Crop being used in vebox, source surface height/width is updated in VeboxAdjustBoundary(), and the rcMaxSrc is used for crop rectangle.
    //But in dynamic Crop case, if the rcMaxSrc is larger than the rcSrc, the input pdwSurfaceHeight/pdwSurfaceWidth will be the input surface size.
    //And if the target surface size is smaller than input surface, it may lead to pagefault issue . So in Vebox Crop case, we set the pdwSurfaceHeight/pdwSurfaceWidth
    //with rcSrc to ensure Vebox input size is same with target Dstrec.
    if (pSurfaceParam->bVEBOXCroppingUsed)
    {
        *pdwSurfaceHeight = MOS_ALIGN_CEIL(
            MOS_MIN(pSurfaceParam->dwHeight, MOS_MAX((uint32_t)pSurfaceParam->rcSrc.bottom, MHW_VEBOX_MIN_HEIGHT)),
            wHeightAlignUnit);
        *pdwSurfaceWidth = MOS_ALIGN_CEIL(
            MOS_MIN(pSurfaceParam->dwWidth, MOS_MAX((uint32_t)pSurfaceParam->rcSrc.right, MHW_VEBOX_MIN_WIDTH)),
            wWidthAlignUnit);
        MHW_NORMALMESSAGE("bVEBOXCroppingUsed = true, pSurfaceParam->rcSrc.bottom: %d, pSurfaceParam->rcSrc.right: %d; pdwSurfaceHeight: %d, pdwSurfaceWidth: %d;",
            (uint32_t)pSurfaceParam->rcSrc.bottom,
            (uint32_t)pSurfaceParam->rcSrc.right,
            *pdwSurfaceHeight,
            *pdwSurfaceWidth);
        MT_LOG5(MT_VP_MHW_VE_ADJUST_SURFPARAM, MT_NORMAL, MT_VP_RENDER_VE_CROPPING, 1, MT_RECT_BOTTOM, pSurfaceParam->rcSrc.bottom, 
            MT_RECT_RIGHT, pSurfaceParam->rcSrc.right, MT_SURF_HEIGHT, *pdwSurfaceHeight, MT_SURF_WIDTH, *pdwSurfaceWidth);
    }
    else
    {
        // Align width and height with max src renctange with consideration of
        // these conditions:
        // The minimum of width/height should equal to or larger than
        // MHW_VEBOX_MIN_WIDTH/HEIGHT. The maximum of width/heigh should equal
        // to or smaller than surface width/height
        *pdwSurfaceHeight = MOS_ALIGN_CEIL(
            MOS_MIN(pSurfaceParam->dwHeight, MOS_MAX((uint32_t)pSurfaceParam->rcMaxSrc.bottom, MHW_VEBOX_MIN_HEIGHT)),
            wHeightAlignUnit);
        *pdwSurfaceWidth = MOS_ALIGN_CEIL(
            MOS_MIN(pSurfaceParam->dwWidth, MOS_MAX((uint32_t)pSurfaceParam->rcMaxSrc.right, MHW_VEBOX_MIN_WIDTH)),
            wWidthAlignUnit);
        MHW_NORMALMESSAGE("bVEBOXCroppingUsed = false, pSurfaceParam->rcMaxSrc.bottom: %d, pSurfaceParam->rcMaxSrc.right: %d; pdwSurfaceHeight: %d, pdwSurfaceWidth: %d;",
            (uint32_t)pSurfaceParam->rcMaxSrc.bottom,
            (uint32_t)pSurfaceParam->rcMaxSrc.right,
            *pdwSurfaceHeight,
            *pdwSurfaceWidth);
    }


finish:
    return eStatus;
}

void MhwVeboxInterfaceG9::IecpStateInitialization(
    mhw_vebox_g9_X::VEBOX_IECP_STATE_CMD  *pVeboxIecpState)
{
    MHW_CHK_NULL_NO_STATUS_RETURN(pVeboxIecpState);
    // Initialize the values to default for media driver.
    pVeboxIecpState->StdSteState.DW5.InvMarginVyl       = 3300;
    pVeboxIecpState->StdSteState.DW5.InvSkinTypesMargin = 1638;

    pVeboxIecpState->StdSteState.DW12.B3U       = 140;
    pVeboxIecpState->StdSteState.DW15.Satp1     = 122;
    pVeboxIecpState->StdSteState.DW15.Satb1     = 1016;
    pVeboxIecpState->StdSteState.DW19.Hueb1     = 1016;
    pVeboxIecpState->StdSteState.DW27.Hues0Dark = 256;
    pVeboxIecpState->StdSteState.DW27.Hues1Dark = 0;

    pVeboxIecpState->AceState.DW0.LaceHistogramSize = 1;

    pVeboxIecpState->TccState.DW0.Satfactor1 = 160;
    pVeboxIecpState->TccState.DW0.Satfactor2 = 160;
    pVeboxIecpState->TccState.DW0.Satfactor3 = 160;
    pVeboxIecpState->TccState.DW1.Satfactor4 = 160;
    pVeboxIecpState->TccState.DW1.Satfactor5 = 160;
    pVeboxIecpState->TccState.DW1.Satfactor6 = 160;

    pVeboxIecpState->AlphaAoiState.DW1.AoiMaxX = 0;
    pVeboxIecpState->AlphaAoiState.DW2.AoiMaxY = 0;

    return;
}

void MhwVeboxInterfaceG9::GamutStateInitialization(
    mhw_vebox_g9_X::VEBOX_GAMUT_STATE_CMD  *pGamutState)
{
    MHW_CHK_NULL_NO_STATUS_RETURN(pGamutState);

    mhw_vebox_g9_X::VEBOX_GAMUT_STATE_CMD cmd;
    *pGamutState = cmd;

    pGamutState->DW1.AB  = 26;
    pGamutState->DW1.AG  = 26;
    pGamutState->DW1.CmS = 640;

    pGamutState->DW2.RI  = 128;
    pGamutState->DW2.CmI = 192;
    pGamutState->DW2.RS  = 768;

    pGamutState->DW3.C0 = 2792;
    pGamutState->DW3.C1 = 1141;
    pGamutState->DW4.C2 = 34;
    pGamutState->DW4.C3 = 71;
    pGamutState->DW5.C4 = 3663;
    pGamutState->DW5.C5 = MOS_BITFIELD_VALUE((uint32_t)-52, 15);
    pGamutState->DW6.C6 = MOS_BITFIELD_VALUE((uint32_t)-12, 15);
    pGamutState->DW6.C7 = 168;
    pGamutState->DW7.C8 = 3434;

    pGamutState->DW14.PwlGammaSlope0  = 3328;
    pGamutState->DW14.PwlGammaSlope1  = 2560;
    pGamutState->DW15.PwlGammaSlope2  = 1280;
    pGamutState->DW15.PwlGammaSlope3  = 960;
    pGamutState->DW16.PwlGammaSlope4  = 658;
    pGamutState->DW16.PwlGammaSlope5  = 512;
    pGamutState->DW17.PwlGammaSlope6  = 368;
    pGamutState->DW17.PwlGammaSlope7  = 278;
    pGamutState->DW18.PwlGammaSlope8  = 215;
    pGamutState->DW18.PwlGammaSlope9  = 179;
    pGamutState->DW19.PwlGammaSlope10 = 151;
    pGamutState->DW19.PwlGammaSlope11 = 124;

    pGamutState->DW26.PwlInvGammaSlope0  = 26;
    pGamutState->DW26.PwlInvGammaSlope1  = 72;
    pGamutState->DW27.PwlInvGammaSlope2  = 107;
    pGamutState->DW27.PwlInvGammaSlope3  = 151;
    pGamutState->DW28.PwlInvGammaSlope4  = 195;
    pGamutState->DW28.PwlInvGammaSlope5  = 243;
    pGamutState->DW29.PwlInvGammaSlope6  = 305;
    pGamutState->DW29.PwlInvGammaSlope7  = 337;
    pGamutState->DW30.PwlInvGammaSlope8  = 404;
    pGamutState->DW30.PwlInvGammaSlope9  = 445;
    pGamutState->DW31.PwlInvGammaSlope10 = 498;
    pGamutState->DW31.PwlInvGammaSlope11 = 555;

    pGamutState->DW33.OffsetOutB = MOS_BITFIELD_VALUE((uint32_t)-1246, 15);
    pGamutState->DW34.OffsetOutR = MOS_BITFIELD_VALUE((uint32_t)-974, 15);
    pGamutState->DW34.OffsetOutG = MOS_BITFIELD_VALUE((uint32_t)-983, 15);
}

MOS_STATUS MhwVeboxInterfaceG9::AddVeboxSurfaceControlBits(
    PMHW_VEBOX_SURFACE_CNTL_PARAMS pVeboxSurfCntlParams,
    uint32_t                       *pSurfCtrlBits)
{
    PLATFORM   Platform = {};
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    mhw_vebox_g9_X::VEB_DI_IECP_COMMAND_SURFACE_CONTROL_BITS_CMD *pVeboxSurfCtrlBits;

    MHW_CHK_NULL(pVeboxSurfCntlParams);
    MHW_CHK_NULL(pSurfCtrlBits);
    MHW_CHK_NULL(m_osInterface);

    m_osInterface->pfnGetPlatform(m_osInterface, &Platform);

    pVeboxSurfCtrlBits = (mhw_vebox_g9_X::VEB_DI_IECP_COMMAND_SURFACE_CONTROL_BITS_CMD *)pSurfCtrlBits;

    if (pVeboxSurfCntlParams->bIsCompressed)
    {
        pVeboxSurfCtrlBits->DW0.MemoryCompressionEnable = 1;

        if (pVeboxSurfCntlParams->CompressionMode == MOS_MMC_VERTICAL)
        {
            pVeboxSurfCtrlBits->DW0.MemoryCompressionMode = 1;
        }
    }

finish:
    return eStatus;
}
