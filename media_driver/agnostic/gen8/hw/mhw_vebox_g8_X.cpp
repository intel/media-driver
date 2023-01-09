/*
* Copyright (c) 2014-2018, Intel Corporation
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
//! \file     mhw_vebox_g8_X.cpp
//! \brief    Constructs vebox commands on Gen8-based platforms
//! \details  Each client facing function both creates a HW command and adds
//!           that command to a command or batch buffer.
//!

#include "mhw_vebox_g8_X.h"

MhwVeboxInterfaceG8::MhwVeboxInterfaceG8(
    PMOS_INTERFACE pInputInterface)
    : MhwVeboxInterfaceGeneric(pInputInterface)
{
    MHW_FUNCTION_ENTER;
    m_veboxSettings = g_Vebox_Settings_g8;
}

void  MhwVeboxInterfaceG8::SetVeboxIecpStateCcm(
    mhw_vebox_g8_X::VEBOX_IECP_STATE_CMD *pVeboxIecpState,
    PMHW_CAPPIPE_PARAMS                  pCapPipeParams,
    const unsigned int                   uCoeffValue)
{
    MHW_CHK_NULL_NO_STATUS_RETURN(pVeboxIecpState);
    MHW_CHK_NULL_NO_STATUS_RETURN(pCapPipeParams);

    pVeboxIecpState->AceState.DW0.FullImageHistogram          = true;
    pVeboxIecpState->CcmState.DW0.ColorCorrectionMatrixEnable = true;

    // CleanupAfterSKLDone-AutoWhiteBalanceNotSupported
    //
    //if (pCapPipeParams->ColorCorrectionParams.bActive &&
    //    !pCapPipeParams->WhiteBalanceParams.bActive)
    // Matrix for Color Correction provided by the application
    if (pCapPipeParams->ColorCorrectionParams.bActive)
    {
        // Coeff is S8.12, so multiply the floating value by 4096
        pVeboxIecpState->CcmState.DW0.C1CoefficientOf3X3TransformMatrix =
            (uint32_t)(pCapPipeParams->ColorCorrectionParams.CCM[0][1] * uCoeffValue);
        pVeboxIecpState->CcmState.DW1.C0CoefficientOf3X3TransformMatrix =
            (uint32_t)(pCapPipeParams->ColorCorrectionParams.CCM[0][0] * uCoeffValue);
        pVeboxIecpState->CcmState.DW2.C3CoefficientOf3X3TransformMatrix =
            (uint32_t)(pCapPipeParams->ColorCorrectionParams.CCM[1][0] * uCoeffValue);
        pVeboxIecpState->CcmState.DW3.C2CoefficientOf3X3TransformMatrix =
            (uint32_t)(pCapPipeParams->ColorCorrectionParams.CCM[0][2] * uCoeffValue);
        pVeboxIecpState->CcmState.DW4.C5CoefficientOf3X3TransformMatrix =
            (uint32_t)(pCapPipeParams->ColorCorrectionParams.CCM[1][2] * uCoeffValue);
        pVeboxIecpState->CcmState.DW5.C4CoefficientOf3X3TransformMatrix =
            (uint32_t)(pCapPipeParams->ColorCorrectionParams.CCM[1][1] * uCoeffValue);
        pVeboxIecpState->CcmState.DW6.C7CoefficientOf3X3TransformMatrix =
            (uint32_t)(pCapPipeParams->ColorCorrectionParams.CCM[2][1] * uCoeffValue);
        pVeboxIecpState->CcmState.DW7.C6CoefficientOf3X3TransformMatrix =
            (uint32_t)(pCapPipeParams->ColorCorrectionParams.CCM[2][0] * uCoeffValue);
        pVeboxIecpState->CcmState.DW8.C8CoefficientOf3X3TransformMatrix =
            (uint32_t)(pCapPipeParams->ColorCorrectionParams.CCM[2][2] * uCoeffValue);
    }

}

void MhwVeboxInterfaceG8::SetVeboxIecpStateBecsc(
    mhw_vebox_g8_X::VEBOX_IECP_STATE_CMD *pVeboxIecpState,
    PMHW_VEBOX_IECP_PARAMS                pVeboxIecpParams,
    bool                                  bEnableFECSC)
{
    PMHW_CAPPIPE_PARAMS pCapPipeParams;
    MOS_FORMAT          dstFormat;

    MHW_CHK_NULL_NO_STATUS_RETURN(pVeboxIecpState);
    MHW_CHK_NULL_NO_STATUS_RETURN(pVeboxIecpParams);

    pCapPipeParams = &pVeboxIecpParams->CapPipeParams;
    dstFormat      = pVeboxIecpParams->dstFormat;

    if (pCapPipeParams->bActive)
    {
        // Enable BECSC if YUV->RGB conversion is needed or RGB->BGR swap is needed
        if ((bEnableFECSC && IS_RGB_FORMAT(dstFormat)) || dstFormat == Format_A8R8G8B8)
        {
            pVeboxIecpState->CscState.DW0.TransformEnable = true;
        }

        if (bEnableFECSC)
        {
            pVeboxIecpState->CscState.DW0.C0 = 1192;
            pVeboxIecpState->CscState.DW1.C3 = 1192;
            pVeboxIecpState->CscState.DW3.C6 = 1192;

            pVeboxIecpState->CscState.DW5.OffsetIn1  = MOS_BITFIELD_VALUE((uint32_t)-64, 11);
            pVeboxIecpState->CscState.DW5.OffsetOut1 = 0;
            pVeboxIecpState->CscState.DW6.OffsetIn2  = MOS_BITFIELD_VALUE((uint32_t)-512, 11);
            pVeboxIecpState->CscState.DW6.OffsetOut2 = 0;
            pVeboxIecpState->CscState.DW7.OffsetIn3  = MOS_BITFIELD_VALUE((uint32_t)-512, 11);
            pVeboxIecpState->CscState.DW7.OffsetOut3 = 0;

            if (dstFormat == Format_A8B8G8R8)  // Convert YUV->RGB
            {
                if (pVeboxIecpParams->ColorSpace == MHW_CSpace_BT601)
                {
                    pVeboxIecpState->CscState.DW0.C1 = MOS_BITFIELD_VALUE((uint32_t)-2, 13);
                    pVeboxIecpState->CscState.DW1.C2 = 1634;

                    pVeboxIecpState->CscState.DW2.C4 = MOS_BITFIELD_VALUE((uint32_t)-401, 13);
                    pVeboxIecpState->CscState.DW2.C5 = MOS_BITFIELD_VALUE((uint32_t)-833, 13);

                    pVeboxIecpState->CscState.DW3.C7 = 2066;
                    pVeboxIecpState->CscState.DW4.C8 = MOS_BITFIELD_VALUE((uint32_t)-1, 13);
                }
                else if (pVeboxIecpParams->ColorSpace == MHW_CSpace_BT709)
                {
                    pVeboxIecpState->CscState.DW0.C1 = MOS_BITFIELD_VALUE((uint32_t)-1, 13);
                    pVeboxIecpState->CscState.DW1.C2 = 1835;
                    pVeboxIecpState->CscState.DW2.C4 = MOS_BITFIELD_VALUE((uint32_t)-218, 13);
                    pVeboxIecpState->CscState.DW2.C5 = MOS_BITFIELD_VALUE((uint32_t)-537, 13);

                    pVeboxIecpState->CscState.DW3.C7 = 2164;
                    pVeboxIecpState->CscState.DW4.C8 = 1;
                }
                else
                {
                    MHW_ASSERT(false);
                }
            }
            else  // if (dstFormat == Format_A8R8G8B8) convert YUV->BGR
            {
                if (pVeboxIecpParams->ColorSpace == MHW_CSpace_BT601)
                {
                    pVeboxIecpState->CscState.DW0.C1 = 2066;
                    pVeboxIecpState->CscState.DW1.C2 = MOS_BITFIELD_VALUE((uint32_t)-1, 13);
                    pVeboxIecpState->CscState.DW2.C4 = MOS_BITFIELD_VALUE((uint32_t)-401, 13);
                    pVeboxIecpState->CscState.DW2.C5 = MOS_BITFIELD_VALUE((uint32_t)-833, 13);

                    pVeboxIecpState->CscState.DW3.C7 = MOS_BITFIELD_VALUE((uint32_t)-2, 13);
                    pVeboxIecpState->CscState.DW4.C8 = 1634;
                }
                else if (pVeboxIecpParams->ColorSpace == MHW_CSpace_BT709)
                {
                    pVeboxIecpState->CscState.DW0.C1 = 2164;
                    pVeboxIecpState->CscState.DW1.C2 = 1;
                    pVeboxIecpState->CscState.DW2.C4 = MOS_BITFIELD_VALUE((uint32_t)-218, 13);
                    pVeboxIecpState->CscState.DW2.C5 = MOS_BITFIELD_VALUE((uint32_t)-537, 13);

                    pVeboxIecpState->CscState.DW3.C7 = MOS_BITFIELD_VALUE((uint32_t)-1, 13);
                    pVeboxIecpState->CscState.DW4.C8 = 1835;
                }
                else
                {
                    MHW_ASSERT(false);
                }
            }
        }
        else if (dstFormat == Format_A8R8G8B8)  // RGB->BGR
        {
            pVeboxIecpState->CscState.DW0.C0         = 0;
            pVeboxIecpState->CscState.DW0.C1         = 0;
            pVeboxIecpState->CscState.DW1.C2         = 1024;
            pVeboxIecpState->CscState.DW1.C3         = 0;
            pVeboxIecpState->CscState.DW2.C4         = 1024;
            pVeboxIecpState->CscState.DW2.C5         = 0;
            pVeboxIecpState->CscState.DW3.C6         = 1024;
            pVeboxIecpState->CscState.DW3.C7         = 0;
            pVeboxIecpState->CscState.DW4.C8         = 0;
            pVeboxIecpState->CscState.DW5.OffsetIn1  = 0;
            pVeboxIecpState->CscState.DW5.OffsetOut1 = 0;
            pVeboxIecpState->CscState.DW6.OffsetIn2  = 0;
            pVeboxIecpState->CscState.DW6.OffsetOut2 = 0;
            pVeboxIecpState->CscState.DW7.OffsetIn3  = 0;
            pVeboxIecpState->CscState.DW7.OffsetOut3 = 0;
        }
    }
    else if (pVeboxIecpParams->bCSCEnable)
    {
        // Set CSC Params
        pVeboxIecpState->CscState.DW0.TransformEnable = true;

        pVeboxIecpState->CscState.DW0.C0 = (uint32_t)MOS_F_ROUND(pVeboxIecpParams->pfCscCoeff[0] * 1024.0F);  // S2.10
        pVeboxIecpState->CscState.DW0.C1 = (uint32_t)MOS_F_ROUND(pVeboxIecpParams->pfCscCoeff[1] * 1024.0F);  // S2.10
        pVeboxIecpState->CscState.DW1.C2 = (uint32_t)MOS_F_ROUND(pVeboxIecpParams->pfCscCoeff[2] * 1024.0F);  // S2.10
        pVeboxIecpState->CscState.DW1.C3 = (uint32_t)MOS_F_ROUND(pVeboxIecpParams->pfCscCoeff[3] * 1024.0F);  // S2.10
        pVeboxIecpState->CscState.DW2.C4 = (uint32_t)MOS_F_ROUND(pVeboxIecpParams->pfCscCoeff[4] * 1024.0F);  // S2.10
        pVeboxIecpState->CscState.DW2.C5 = (uint32_t)MOS_F_ROUND(pVeboxIecpParams->pfCscCoeff[5] * 1024.0F);  // S2.10

        pVeboxIecpState->CscState.DW3.C6 = (uint32_t)MOS_F_ROUND(pVeboxIecpParams->pfCscCoeff[6] * 1024.0F);  // S2.10
        pVeboxIecpState->CscState.DW3.C7 = (uint32_t)MOS_F_ROUND(pVeboxIecpParams->pfCscCoeff[7] * 1024.0F);  // S2.10
        pVeboxIecpState->CscState.DW4.C8 = (uint32_t)MOS_F_ROUND(pVeboxIecpParams->pfCscCoeff[8] * 1024.0F);  // S2.10

        // Offset is S10, but the SW offsets are calculated as 8bits, so left shift them 2bits to be in the position of MSB
        pVeboxIecpState->CscState.DW5.OffsetIn1 = (uint32_t)MOS_F_ROUND(pVeboxIecpParams->pfCscInOffset[0] * 4.0F);  // S10
        pVeboxIecpState->CscState.DW6.OffsetIn2 = (uint32_t)MOS_F_ROUND(pVeboxIecpParams->pfCscInOffset[1] * 4.0F);  // S10
        pVeboxIecpState->CscState.DW7.OffsetIn3 = (uint32_t)MOS_F_ROUND(pVeboxIecpParams->pfCscInOffset[2] * 4.0F);  // S10

        pVeboxIecpState->CscState.DW5.OffsetOut1 = (uint32_t)MOS_F_ROUND(pVeboxIecpParams->pfCscOutOffset[0] * 4.0F);  // S10
        pVeboxIecpState->CscState.DW6.OffsetOut2 = (uint32_t)MOS_F_ROUND(pVeboxIecpParams->pfCscOutOffset[1] * 4.0F);  // S10
        pVeboxIecpState->CscState.DW7.OffsetOut3 = (uint32_t)MOS_F_ROUND(pVeboxIecpParams->pfCscOutOffset[2] * 4.0F);  // S10

        pVeboxIecpState->AlphaAoiState.DW0.AlphaFromStateSelect = pVeboxIecpParams->bAlphaEnable;

        // Alpha is U12, but the SW alpha is calculated as 8bits, so left shift it 4bits to be in the position of MSB
        pVeboxIecpState->AlphaAoiState.DW0.ColorPipeAlpha = pVeboxIecpParams->wAlphaValue * 16;  // U12
    }
}

MOS_STATUS MhwVeboxInterfaceG8::VeboxAdjustBoundary(
    PMHW_VEBOX_SURFACE_PARAMS pSurfaceParam,
    uint32_t                  *pdwSurfaceWidth,
    uint32_t                  *pdwSurfaceHeight,
    bool                      bDIEnable)
{
    uint16_t                    wWidthAlignUnit;
    uint16_t                    wHeightAlignUnit;
    MOS_STATUS                  eStatus = MOS_STATUS_SUCCESS;

    MHW_CHK_NULL(pSurfaceParam);
    MHW_CHK_NULL(pdwSurfaceWidth);
    MHW_CHK_NULL(pdwSurfaceHeight);

    // initialize
    wHeightAlignUnit = 1;
    wWidthAlignUnit = 1;

    switch (pSurfaceParam->Format)
    {
    case Format_NV12:
        wHeightAlignUnit = bDIEnable ? 4 : 2;
        wWidthAlignUnit = 2;
        break;

    case Format_YUYV:
    case Format_YUY2:
    case Format_UYVY:
    case Format_YVYU:
    case Format_VYUY:
        wHeightAlignUnit = bDIEnable ? 2 : 1;
        wWidthAlignUnit = 2;
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

finish:
    return eStatus;
}

MOS_STATUS MhwVeboxInterfaceG8::AddVeboxState(
    PMOS_COMMAND_BUFFER         pCmdBuffer,
    PMHW_VEBOX_STATE_CMD_PARAMS pVeboxStateCmdParams,
    bool                        bUseCmBuffer)
{
    MOS_STATUS                       eStatus;
    PMOS_INTERFACE                   pOsInterface;
    PMOS_RESOURCE                    pVeboxParamResource = nullptr;
    PMOS_RESOURCE                    pVeboxHeapResource  = nullptr;
    PMHW_VEBOX_HEAP                  pVeboxHeap;
    PMHW_VEBOX_MODE                  pVeboxMode;
    uint32_t                         uiInstanceBaseAddr  = 0;
    MHW_RESOURCE_PARAMS              ResourceParams;
    mhw_vebox_g8_X::VEBOX_STATE_CMD  cmd;

    MHW_CHK_NULL(m_osInterface);
    MHW_CHK_NULL(m_veboxHeap);
    MHW_CHK_NULL(pCmdBuffer);
    MHW_CHK_NULL(pVeboxStateCmdParams);

    // Initialize
    eStatus      = MOS_STATUS_SUCCESS;
    pOsInterface = m_osInterface;
    pVeboxHeap   = m_veboxHeap;
    pVeboxMode   = &pVeboxStateCmdParams->VeboxMode;
    if (bUseCmBuffer)
    {
        pVeboxParamResource = pVeboxStateCmdParams->pVeboxParamSurf;
    }
    else
    {
        pVeboxHeapResource = pVeboxStateCmdParams->bUseVeboxHeapKernelResource ? &pVeboxHeap->KernelResource : &pVeboxHeap->DriverResource;
        // Calculate the instance base address
        uiInstanceBaseAddr = pVeboxHeap->uiInstanceSize * pVeboxHeap->uiCurState;
    }

    cmd.DW1.DownsampleMethod422to420 = 1;
    cmd.DW1.DownsampleMethod444to422 = 1;

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

    cmd.DW1.ColorGamutExpansionEnable   = pVeboxMode->ColorGamutExpansionEnable;
    cmd.DW1.ColorGamutCompressionEnable = pVeboxMode->ColorGamutCompressionEnable;
    cmd.DW1.GlobalIecpEnable            = pVeboxMode->GlobalIECPEnable;
    cmd.DW1.DnEnable                    = pVeboxMode->DNEnable;
    cmd.DW1.DiEnable                    = pVeboxMode->DIEnable;
    cmd.DW1.DnDiFirstFrame              = pVeboxMode->DNDIFirstFrame;
    cmd.DW1.DiOutputFrames              = pVeboxMode->DIOutputFrames;
    cmd.DW1.DemosaicEnable              = pVeboxMode->DemosaicEnable;
    cmd.DW1.VignetteEnable              = pVeboxMode->VignetteEnable;
    cmd.DW1.AlphaPlaneEnable            = pVeboxMode->AlphaPlaneEnable;
    cmd.DW1.HotPixelFilteringEnable     = pVeboxMode->HotPixelFilteringEnable;
    cmd.DW1.SingleSliceVeboxEnable      = pVeboxMode->SingleSliceVeboxEnable;

    pOsInterface->pfnAddCommand(pCmdBuffer, &cmd, cmd.byteSize);

finish:
    return eStatus;
}

MOS_STATUS MhwVeboxInterfaceG8::AddVeboxSurfaces(
    PMOS_COMMAND_BUFFER                 pCmdBuffer,
    PMHW_VEBOX_SURFACE_STATE_CMD_PARAMS pVeboxSurfaceStateCmdParams)
{
    mhw_vebox_g8_X::VEBOX_SURFACE_STATE_CMD cmd1, cmd2;

    MHW_CHK_NULL_RETURN(m_osInterface);
    MHW_CHK_NULL_RETURN(pCmdBuffer);
    MHW_CHK_NULL_RETURN(pVeboxSurfaceStateCmdParams);

    // Setup Surface State for Input surface
    SetVeboxSurfaces(
        &pVeboxSurfaceStateCmdParams->SurfInput,
        nullptr,
        nullptr,
        &cmd1,
        false,
        pVeboxSurfaceStateCmdParams->bDIEnable);
    m_osInterface->pfnAddCommand(pCmdBuffer, &cmd1, cmd1.byteSize);

    // Setup Surface State for Output surface
    SetVeboxSurfaces(
        &pVeboxSurfaceStateCmdParams->SurfOutput,
        nullptr,
        nullptr,
        &cmd2,
        true,
        pVeboxSurfaceStateCmdParams->bDIEnable);
    m_osInterface->pfnAddCommand(pCmdBuffer, &cmd2, cmd2.byteSize);

    return MOS_STATUS_SUCCESS;
}

void MhwVeboxInterfaceG8::SetVeboxSurfaces(
    PMHW_VEBOX_SURFACE_PARAMS                pSurfaceParam,
    PMHW_VEBOX_SURFACE_PARAMS                pDerivedSurfaceParam,
    PMHW_VEBOX_SURFACE_PARAMS                pSkinScoreSurfaceParam,
    mhw_vebox_g8_X::VEBOX_SURFACE_STATE_CMD *pVeboxSurfaceState,
    bool                                     bIsOutputSurface,
    bool                                     bDIEnable)
{
    uint32_t dwFormat;
    uint32_t dwSurfaceWidth;
    uint32_t dwSurfaceHeight;
    uint32_t dwSurfacePitch;
    bool  bHalfPitchForChroma;
    bool  bInterleaveChroma;
    uint16_t wUXOffset;
    uint16_t wUYOffset;
    uint16_t wVXOffset;
    uint16_t wVYOffset;
    uint8_t bBayerOffset;
    uint8_t bBayerStride;

    mhw_vebox_g8_X::VEBOX_SURFACE_STATE_CMD VeboxSurfaceState;

    if ((pSurfaceParam == nullptr) || (pVeboxSurfaceState == nullptr))
    {
        MHW_ASSERTMESSAGE("Invalid Input Parameter: pSurfaceParam is nullptr OR pVeboxSurfaceState is nullptr");
        return;
    }

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
        wUYOffset         = (uint16_t)pSurfaceParam->dwHeight;
        break;

    case Format_YUYV:
    case Format_YUY2:
        dwFormat = VeboxSurfaceState.SURFACE_FORMAT_YCRCBNORMAL;
        break;

    case Format_UYVY:
        dwFormat = VeboxSurfaceState.SURFACE_FORMAT_YCRCBSWAPY;
        break;

    case Format_YVYU:
        dwFormat = VeboxSurfaceState.SURFACE_FORMAT_YCRCBSWAPUV;
        break;

    case Format_VYUY:
        dwFormat = VeboxSurfaceState.SURFACE_FORMAT_YCRCBSWAPUVY;
        break;

    case Format_AYUV:
        dwFormat = VeboxSurfaceState.SURFACE_FORMAT_PACKED444A8;
        break;

    case Format_A8R8G8B8:
    case Format_X8R8G8B8:
    case Format_A8B8G8R8:
    case Format_X8B8G8R8:
        dwFormat = VeboxSurfaceState.SURFACE_FORMAT_R8G8B8A8UNORMR8G8B8A8UNORMSRGB;
        break;

    case Format_L8:
    case Format_P8:
        dwFormat = VeboxSurfaceState.SURFACE_FORMAT_Y8UNORM;
        break;

    case Format_IRW0:
        dwFormat     = VeboxSurfaceState.SURFACE_FORMAT_BAYERPATTERN;
        bBayerOffset = VeboxSurfaceState.BAYER_PATTERN_OFFSET_PIXELATX0_Y0ISBLUE;
        bBayerStride = VeboxSurfaceState.BAYER_PATTERN_FORMAT_12OR10_BITINPUTATA16_BITSTRIDEVALIDDATAISINTHEMSBS;
        break;

    case Format_IRW1:
        dwFormat     = VeboxSurfaceState.SURFACE_FORMAT_BAYERPATTERN;
        bBayerOffset = VeboxSurfaceState.BAYER_PATTERN_OFFSET_PIXELATX0_Y0ISRED;
        bBayerStride = VeboxSurfaceState.BAYER_PATTERN_FORMAT_12OR10_BITINPUTATA16_BITSTRIDEVALIDDATAISINTHEMSBS;
        break;

    case Format_IRW2:
        dwFormat     = VeboxSurfaceState.SURFACE_FORMAT_BAYERPATTERN;
        bBayerOffset = VeboxSurfaceState.BAYER_PATTERN_OFFSET_PIXELATX0_Y0ISGREEN_PIXELATX1_Y0ISRED;
        bBayerStride = VeboxSurfaceState.BAYER_PATTERN_FORMAT_12OR10_BITINPUTATA16_BITSTRIDEVALIDDATAISINTHEMSBS;
        break;

    case Format_IRW3:
        dwFormat     = VeboxSurfaceState.SURFACE_FORMAT_BAYERPATTERN;
        bBayerOffset = VeboxSurfaceState.BAYER_PATTERN_OFFSET_PIXELATX0_Y0ISGREEN_PIXELATX1_Y0ISBLUE;
        bBayerStride = VeboxSurfaceState.BAYER_PATTERN_FORMAT_12OR10_BITINPUTATA16_BITSTRIDEVALIDDATAISINTHEMSBS;
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

    // Re-set DW0
    pVeboxSurfaceState->DW0.DwordLength = 4;

    pVeboxSurfaceState->DW1.SurfaceIdentification = bIsOutputSurface;
    pVeboxSurfaceState->DW2.Width                 = dwSurfaceWidth - 1;
    pVeboxSurfaceState->DW2.Height                = dwSurfaceHeight - 1;
    pVeboxSurfaceState->DW3.HalfPitchForChroma    = bHalfPitchForChroma;
    pVeboxSurfaceState->DW3.InterleaveChroma      = bInterleaveChroma;
    pVeboxSurfaceState->DW3.SurfaceFormat         = dwFormat;
    pVeboxSurfaceState->DW3.BayerPatternOffset    = bBayerOffset;
    pVeboxSurfaceState->DW3.BayerPatternFormat    = bBayerStride;
    pVeboxSurfaceState->DW3.SurfacePitch          = dwSurfacePitch - 1;
    pVeboxSurfaceState->DW3.TiledSurface          = (pSurfaceParam->TileType != MOS_TILE_LINEAR)
                                               ? true
                                               : false;
    pVeboxSurfaceState->DW3.TileWalk = (pSurfaceParam->TileType == MOS_TILE_Y)
                                           ? VeboxSurfaceState.TILE_WALK_TILEWALKYMAJOR
                                           : VeboxSurfaceState.TILE_WALK_TILEWALKXMAJOR;
    pVeboxSurfaceState->DW4.XOffsetForU = wUXOffset;
    pVeboxSurfaceState->DW4.YOffsetForU = wUYOffset;
    pVeboxSurfaceState->DW5.XOffsetForV = wVXOffset;
    pVeboxSurfaceState->DW5.YOffsetForV = wVYOffset;

finish:
    return;
}

MOS_STATUS MhwVeboxInterfaceG8::AddVeboxDiIecp(
    PMOS_COMMAND_BUFFER           pCmdBuffer,
    PMHW_VEBOX_DI_IECP_CMD_PARAMS pVeboxDiIecpCmdParams)
{
    MOS_STATUS          eStatus;;
    MHW_RESOURCE_PARAMS ResourceParams;
    mhw_vebox_g8_X::VEB_DI_IECP_CMD  cmd;

    MHW_CHK_NULL(m_osInterface);
    MHW_CHK_NULL(pCmdBuffer);
    MHW_CHK_NULL(pVeboxDiIecpCmdParams);
    MHW_ASSERT(MOS_IS_ALIGNED(pVeboxDiIecpCmdParams->dwCurrInputSurfOffset, MHW_PAGE_SIZE));  // offset should be aligned with 4KB
    MHW_ASSERT(MOS_IS_ALIGNED(pVeboxDiIecpCmdParams->dwPrevInputSurfOffset, MHW_PAGE_SIZE));  // offset should be aligned with 4KB

    // Initialize
    eStatus      = MOS_STATUS_SUCCESS;

    if (pVeboxDiIecpCmdParams->pOsResCurrInput)
    {
        MOS_ZeroMemory(&ResourceParams, sizeof(ResourceParams));
        ResourceParams.presResource    = pVeboxDiIecpCmdParams->pOsResCurrInput;
        ResourceParams.dwOffset        = pVeboxDiIecpCmdParams->CurrInputSurfCtrl.Value + pVeboxDiIecpCmdParams->dwCurrInputSurfOffset;
        ResourceParams.pdwCmd          = & (cmd.DW2.Value);
        ResourceParams.dwLocationInCmd = 2;
        ResourceParams.HwCommandType   = MOS_VEBOX_DI_IECP;

        MHW_CHK_STATUS(pfnAddResourceToCmd(
            m_osInterface,
            pCmdBuffer,
            &ResourceParams));
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
            m_osInterface,
            pCmdBuffer,
            &ResourceParams));
    }

    // Patch STMM Input
    if (pVeboxDiIecpCmdParams->pOsResStmmInput)
    {
        MOS_ZeroMemory(&ResourceParams, sizeof(ResourceParams));
        ResourceParams.presResource    = pVeboxDiIecpCmdParams->pOsResStmmInput;
        ResourceParams.dwOffset        = pVeboxDiIecpCmdParams->StmmInputSurfCtrl.Value;
        ResourceParams.pdwCmd          = & (cmd.DW6.Value);
        ResourceParams.dwLocationInCmd = 6;
        ResourceParams.HwCommandType   = MOS_VEBOX_DI_IECP;

        MHW_CHK_STATUS(pfnAddResourceToCmd(
            m_osInterface,
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
            m_osInterface,
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
            m_osInterface,
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
            m_osInterface,
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
            m_osInterface,
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
            m_osInterface,
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
            m_osInterface,
            pCmdBuffer,
            &ResourceParams));
    }

    cmd.DW1.EndingX   = pVeboxDiIecpCmdParams->dwEndingX;
    cmd.DW1.StartingX = pVeboxDiIecpCmdParams->dwStartingX;

    m_osInterface->pfnAddCommand(pCmdBuffer, &cmd, cmd.byteSize);

finish:
    return eStatus;
}

MOS_STATUS MhwVeboxInterfaceG8::AddVeboxDndiState(
    PMHW_VEBOX_DNDI_PARAMS pVeboxDndiParams)
{
    PMHW_VEBOX_HEAP                       pVeboxHeap;
    uint32_t                              uiOffset;
    MOS_STATUS                            eStatus = MOS_STATUS_SUCCESS;
    mhw_vebox_g8_X::VEBOX_DNDI_STATE_CMD *pVeboxDndiState, VeboxDndiState;

    MHW_CHK_NULL(pVeboxDndiParams);
    MHW_CHK_NULL(m_veboxHeap);

    pVeboxHeap      = m_veboxHeap;
    uiOffset        = pVeboxHeap->uiCurState * pVeboxHeap->uiInstanceSize;
    pVeboxDndiState = (mhw_vebox_g8_X::VEBOX_DNDI_STATE_CMD *)(pVeboxHeap->pLockedDriverResourceMem +
                                                               pVeboxHeap->uiDndiStateOffset +
                                                               uiOffset);

    *pVeboxDndiState = VeboxDndiState;

    pVeboxDndiState->DW0.DenoiseAsdThreshold =
        pVeboxDndiParams->dwDenoiseASDThreshold;
    pVeboxDndiState->DW0.DenoiseHistoryIncrease =
        pVeboxDndiParams->dwDenoiseHistoryDelta;
    pVeboxDndiState->DW0.DenoiseMaximumHistory =
        pVeboxDndiParams->dwDenoiseMaximumHistory;
    pVeboxDndiState->DW0.DenoiseStadThreshold =
        pVeboxDndiParams->dwDenoiseSTADThreshold;
    pVeboxDndiState->DW1.DenoiseThresholdForSumOfComplexityMeasure =
        pVeboxDndiParams->dwDenoiseSCMThreshold;
    pVeboxDndiState->DW1.DenoiseMovingPixelThreshold =
        pVeboxDndiParams->dwDenoiseMPThreshold;
    pVeboxDndiState->DW1.LowTemporalDifferenceThreshold =
        pVeboxDndiParams->dwLTDThreshold;
    pVeboxDndiState->DW1.TemporalDifferenceThreshold =
        pVeboxDndiParams->dwTDThreshold;
    pVeboxDndiState->DW2.GoodNeighborThreshold =
        pVeboxDndiParams->dwGoodNeighborThreshold;
    pVeboxDndiState->DW2.BlockNoiseEstimateNoiseThreshold =
        ABSMAXABSMIN_THRESHOLD_DEFAULT_G8;
    pVeboxDndiState->DW6.DnDiTopFirst =
        pVeboxDndiParams->bDNDITopFirst;
    pVeboxDndiState->DW6.ProgressiveDn =
        pVeboxDndiParams->bProgressiveDN;
    pVeboxDndiState->DW7.ProgressiveCadenceReconstructionFor1StFieldOfCurrentFrame =
        pVeboxDndiParams->dwFMDFirstFieldCurrFrame;
    pVeboxDndiState->DW7.ProgressiveCadenceReconstructionFor2NdFieldOfPreviousFrame =
        pVeboxDndiParams->dwFMDSecondFieldPrevFrame;

    // Chroma Denoise
    pVeboxDndiState->DW8.ChromaLowTemporalDifferenceThreshold =
        pVeboxDndiParams->dwChromaLTDThreshold;
    pVeboxDndiState->DW8.ChromaTemporalDifferenceThreshold =
        pVeboxDndiParams->dwChromaTDThreshold;
    pVeboxDndiState->DW8.ChromaDenoiseStadThreshold =
        pVeboxDndiParams->dwChromaSTADThreshold;
    pVeboxDndiState->DW8.ChromaDenoiseEnable =
        pVeboxDndiParams->bChromaDNEnable;

    // Hot Pixel
    pVeboxDndiState->DW9.HotPixelThreshold =
        pVeboxDndiParams->dwHotPixelThreshold;
    pVeboxDndiState->DW9.HotPixelCount =
        pVeboxDndiParams->dwHotPixelCount;

    pVeboxDndiState->DW1.StmmC2            = 2;
    pVeboxDndiState->DW3.MaximumStmm       = 150;
    pVeboxDndiState->DW3.MultiplierForVecm = 30;
    pVeboxDndiState->DW4.SdiDelta          = 5;
    pVeboxDndiState->DW4.SdiThreshold      = 100;
    pVeboxDndiState->DW4.StmmOutputShift   = 5;
    pVeboxDndiState->DW4.StmmShiftUp       = 1;
    pVeboxDndiState->DW4.MinimumStmm       = 118;

    pVeboxDndiState->DW5.FmdTemporalDifferenceThreshold   = 175;
    pVeboxDndiState->DW5.SdiFallbackMode2ConstantAngle2X1 = 37;
    pVeboxDndiState->DW5.SdiFallbackMode1T2Constant       = 100;
    pVeboxDndiState->DW5.SdiFallbackMode1T1Constant       = 50;

    pVeboxDndiState->DW6.McdiEnable                      = 1;
    pVeboxDndiState->DW6.FmdTearThreshold                = 2;
    pVeboxDndiState->DW6.Fmd2VerticalDifferenceThreshold = 100;
    pVeboxDndiState->DW6.Fmd1VerticalDifferenceThreshold = 16;

    MHW_ASSERT(pVeboxDndiState->DW6.Value & 0xFF);

finish:
    return eStatus;
}

MOS_STATUS MhwVeboxInterfaceG8::AddVeboxGamutState(
    PMHW_VEBOX_IECP_PARAMS  pVeboxIecpParams,
    PMHW_VEBOX_GAMUT_PARAMS pVeboxGamutParams)
{
    PMHW_VEBOX_HEAP        pVeboxHeap;
    uint32_t               uiOffset;
    MOS_STATUS             eStatus = MOS_STATUS_SUCCESS;

    mhw_vebox_g8_X::VEBOX_IECP_STATE_CMD  *pIecpState;
    mhw_vebox_g8_X::VEBOX_GAMUT_STATE_CMD *pGamutState, gamutCmd;
    MHW_CHK_NULL(pVeboxGamutParams);
    MHW_CHK_NULL(m_veboxHeap);

    pVeboxHeap = m_veboxHeap;
    uiOffset   = pVeboxHeap->uiCurState * pVeboxHeap->uiInstanceSize;

    pIecpState =
        (mhw_vebox_g8_X::VEBOX_IECP_STATE_CMD *)(pVeboxHeap->pLockedDriverResourceMem +
                                                 pVeboxHeap->uiIecpStateOffset +
                                                 uiOffset);
    pGamutState =
        (mhw_vebox_g8_X::VEBOX_GAMUT_STATE_CMD *)(pVeboxHeap->pLockedDriverResourceMem +
                                 pVeboxHeap->uiGamutStateOffset +
                                 uiOffset);
    MHW_CHK_NULL(pIecpState);
    MHW_CHK_NULL(pGamutState);

    // Must initialize VeboxIecpState firstly, even if it is not used because GCE
    // requires GlobalIECP enable bit to be turned on
    if (!pVeboxIecpParams)
    {
        IecpStateInitialization(pIecpState);
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
        MhwVeboxInterfaceGeneric<mhw_vebox_g8_X>::AddVeboxVertexTable(pVeboxGamutParams->ColorSpace);
    }

    if (pVeboxGamutParams->GExpMode != MHW_GAMUT_MODE_NONE)
    {
        // Need to convert YUV input to RGB before GE
        pIecpState->CscState.DW0.TransformEnable = true;
        pIecpState->CscState.DW0.C0              = 1192;
        pIecpState->CscState.DW1.C3              = 1192;
        pIecpState->CscState.DW3.C6              = 1192;

        pIecpState->CscState.DW5.OffsetIn1  = MOS_BITFIELD_VALUE((uint32_t)-64, 11);
        pIecpState->CscState.DW5.OffsetOut1 = 0;
        pIecpState->CscState.DW6.OffsetIn2  = MOS_BITFIELD_VALUE((uint32_t)-512, 11);
        pIecpState->CscState.DW6.OffsetOut2 = 0;
        pIecpState->CscState.DW7.OffsetIn3  = MOS_BITFIELD_VALUE((uint32_t)-512, 11);
        pIecpState->CscState.DW7.OffsetOut3 = 0;

        if (pVeboxGamutParams->ColorSpace == MHW_CSpace_BT601 ||
            pVeboxGamutParams->ColorSpace == MHW_CSpace_xvYCC601 ||
            pVeboxGamutParams->ColorSpace == MHW_CSpace_BT601_FullRange)
        {
            pIecpState->CscState.DW0.C1 = MOS_BITFIELD_VALUE((uint32_t)-2, 13);
            pIecpState->CscState.DW1.C2 = 1634;

            pIecpState->CscState.DW2.C4 = MOS_BITFIELD_VALUE((uint32_t)-401, 13);
            pIecpState->CscState.DW2.C5 = MOS_BITFIELD_VALUE((uint32_t)-833, 13);

            pIecpState->CscState.DW3.C7 = 2066;
            pIecpState->CscState.DW4.C8 = MOS_BITFIELD_VALUE((uint32_t)-1, 13);
        }
        else if (pVeboxGamutParams->ColorSpace == MHW_CSpace_BT709 ||
                 pVeboxGamutParams->ColorSpace == MHW_CSpace_xvYCC709 ||
                 pVeboxGamutParams->ColorSpace == MHW_CSpace_BT709_FullRange)
        {
            pIecpState->CscState.DW0.C1 = MOS_BITFIELD_VALUE((uint32_t)-1, 13);
            pIecpState->CscState.DW1.C2 = 1835;
            pIecpState->CscState.DW2.C4 = MOS_BITFIELD_VALUE((uint32_t)-218, 13);
            pIecpState->CscState.DW2.C5 = MOS_BITFIELD_VALUE((uint32_t)-537, 13);

            pIecpState->CscState.DW3.C7 = 2164;
            pIecpState->CscState.DW4.C8 = 1;
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

        pIecpState->CscState.DW0.C0 = 1192;
        pIecpState->CscState.DW1.C3 = 1192;
        pIecpState->CscState.DW3.C6 = 1192;

        pIecpState->CscState.DW5.OffsetIn1  = MOS_BITFIELD_VALUE((uint32_t)-64, 11);
        pIecpState->CscState.DW5.OffsetOut1 = 0;
        pIecpState->CscState.DW6.OffsetIn2  = MOS_BITFIELD_VALUE((uint32_t)-512, 11);
        pIecpState->CscState.DW6.OffsetOut2 = 0;
        pIecpState->CscState.DW7.OffsetIn3  = MOS_BITFIELD_VALUE((uint32_t)-512, 11);
        pIecpState->CscState.DW7.OffsetOut3 = 0;

        if (pVeboxGamutParams->ColorSpace == MHW_CSpace_BT601 ||
            pVeboxGamutParams->ColorSpace == MHW_CSpace_xvYCC601 ||
            pVeboxGamutParams->ColorSpace == MHW_CSpace_BT601_FullRange)
        {
            pIecpState->CscState.DW0.C1 = MOS_BITFIELD_VALUE((uint32_t)-2, 13);
            pIecpState->CscState.DW1.C2 = 1634;

            pIecpState->CscState.DW2.C4 = MOS_BITFIELD_VALUE((uint32_t)-401, 13);
            pIecpState->CscState.DW2.C5 = MOS_BITFIELD_VALUE((uint32_t)-833, 13);

            pIecpState->CscState.DW3.C7 = 2066;
            pIecpState->CscState.DW4.C8 = MOS_BITFIELD_VALUE((uint32_t)-1, 13);
        }
        else if (pVeboxGamutParams->ColorSpace == MHW_CSpace_BT709 ||
                 pVeboxGamutParams->ColorSpace == MHW_CSpace_xvYCC709 ||
                 pVeboxGamutParams->ColorSpace == MHW_CSpace_BT709_FullRange)
        {
            pIecpState->CscState.DW0.C1 = MOS_BITFIELD_VALUE((uint32_t)-1, 13);
            pIecpState->CscState.DW1.C2 = 1835;
            pIecpState->CscState.DW2.C4 = MOS_BITFIELD_VALUE((uint32_t)-218, 13);
            pIecpState->CscState.DW2.C5 = MOS_BITFIELD_VALUE((uint32_t)-537, 13);

            pIecpState->CscState.DW3.C7 = 2164;
            pIecpState->CscState.DW4.C8 = 1;
        }
        else
        {
            MHW_ASSERTMESSAGE("Unknown primary");
        }

        pGamutState->DW0.GlobalModeEnable = true;
        pGamutState->DW0.CmW              = 1023;
        pGamutState->DW3.C0               = 4096;
        pGamutState->DW3.C1               = 0;
        pGamutState->DW4.C2               = 0;
        pGamutState->DW4.C3               = 0;
        pGamutState->DW5.C4               = 4096;
        pGamutState->DW5.C5               = 0;
        pGamutState->DW6.C6               = 0;
        pGamutState->DW6.C7               = 0;
        pGamutState->DW7.C8               = 4096;
        pGamutState->DW32.OffsetInR       = 0;
        pGamutState->DW32.OffsetInG       = 0;
        pGamutState->DW33.OffsetInB       = 0;
        pGamutState->DW33.OffsetOutB      = 0;
        pGamutState->DW34.OffsetOutR      = 0;
        pGamutState->DW34.OffsetOutG      = 0;

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

finish:
    return eStatus;
}

MOS_STATUS MhwVeboxInterfaceG8::AddVeboxIecpState(
    PMHW_VEBOX_IECP_PARAMS pVeboxIecpParams)
{
    bool                                          bEnableFECSC = false;
    PMHW_VEBOX_HEAP                               pVeboxHeap;
    uint32_t                                      uiOffset;
    MOS_STATUS                                    eStatus = MOS_STATUS_SUCCESS;
    mhw_vebox_g8_X::VEBOX_IECP_STATE_CMD *        pVeboxIecpState;
    mhw_vebox_g8_X::VEBOX_CAPTURE_PIPE_STATE_CMD *pVeboxCapPipeState, CapPipeStatecmd;

    MHW_CHK_NULL(pVeboxIecpParams);
    MHW_CHK_NULL(m_veboxHeap);

    pVeboxHeap      = m_veboxHeap;
    uiOffset        = pVeboxHeap->uiCurState * pVeboxHeap->uiInstanceSize;
    pVeboxIecpState = (mhw_vebox_g8_X::VEBOX_IECP_STATE_CMD *)(pVeboxHeap->pLockedDriverResourceMem +
                                                               pVeboxHeap->uiIecpStateOffset +
                                                               uiOffset);
    MHW_CHK_NULL(pVeboxIecpState);
    IecpStateInitialization(pVeboxIecpState);

    if (pVeboxIecpParams->ColorPipeParams.bActive)
    {
        // Enable STD/E (Skin Tone Detection/Enhancement)
        MhwVeboxInterfaceGeneric<mhw_vebox_g8_X>::SetVeboxIecpStateSTE(&pVeboxIecpState->StdSteState,
            &pVeboxIecpParams->ColorPipeParams);

        // Enable ACE (Automatic Contrast Enhancement)
        if (pVeboxIecpParams->ColorPipeParams.bEnableACE)
        {
            pVeboxIecpState->AceState.DW0.AceEnable          = true;
            pVeboxIecpState->AceState.DW0.FullImageHistogram = true;
        }

        // Enable TCC (Total Color Control)
        if (pVeboxIecpParams->ColorPipeParams.bEnableTCC)
        {
            MhwVeboxInterfaceGeneric<mhw_vebox_g8_X>::SetVeboxIecpStateTCC(&pVeboxIecpState->TccState,
                &pVeboxIecpParams->ColorPipeParams);
        }
    }

    if (pVeboxIecpParams->CapPipeParams.bActive)
    {
        // IECP needs to operate in YUV space
        if (pVeboxIecpParams->dstFormat == Format_NV12 ||
            pVeboxIecpParams->ProcAmpParams.bActive ||
            pVeboxIecpParams->ColorPipeParams.bActive)
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
            SetVeboxIecpStateFecsc(&pVeboxIecpState->FrontEndCsc, pVeboxIecpParams);
        }

        // Set Black Level Correction
        pVeboxIecpState->BlackLevelCorrectionState.DW0.BlackPointOffsetR = pVeboxIecpParams->CapPipeParams.BlackLevelParams.R;
        pVeboxIecpState->BlackLevelCorrectionState.DW1.BlackPointOffsetG = pVeboxIecpParams->CapPipeParams.BlackLevelParams.G0;
        pVeboxIecpState->BlackLevelCorrectionState.DW1.BlackPointOffsetB = pVeboxIecpParams->CapPipeParams.BlackLevelParams.B;

        // Enable Color Correction Matrix
        if (pVeboxIecpParams->CapPipeParams.ColorCorrectionParams.bActive ||
            pVeboxIecpParams->CapPipeParams.WhiteBalanceParams.bActive)
        {
            SetVeboxIecpStateCcm(pVeboxIecpState, &pVeboxIecpParams->CapPipeParams, 4096);
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
        MhwVeboxInterfaceGeneric<mhw_vebox_g8_X>::SetVeboxIecpStateProcAmp(
            &pVeboxIecpState->ProcampState,
            &pVeboxIecpParams->ProcAmpParams);
    }

    pVeboxCapPipeState =
        (mhw_vebox_g8_X::VEBOX_CAPTURE_PIPE_STATE_CMD *)(pVeboxHeap->pLockedDriverResourceMem +
                                                         pVeboxHeap->uiCapturePipeStateOffset +
                                                         uiOffset);
    MHW_CHK_NULL(pVeboxCapPipeState);
    *pVeboxCapPipeState = CapPipeStatecmd;

finish:
    return eStatus;
}

void MhwVeboxInterfaceG8::SetVeboxIecpStateFecsc(
    mhw_vebox_g8_X::VEBOX_FRONT_END_CSC_STATE_CMD *pVeboxFecscState,
    PMHW_VEBOX_IECP_PARAMS                         pVeboxIecpParams)
{
    PMHW_CAPPIPE_PARAMS pCapPipeParams;

    MHW_CHK_NULL_NO_STATUS_RETURN(pVeboxFecscState);
    MHW_CHK_NULL_NO_STATUS_RETURN(pVeboxIecpParams);

    pCapPipeParams = &pVeboxIecpParams->CapPipeParams;

    pVeboxFecscState->DW0.FfrontEndCScTransformEnable = true;

    if (pCapPipeParams->FECSCParams.bActive)
    {
        // Coeff is S2.10, so multiply the floating value by 1024
        pVeboxFecscState->DW0.FecscC0TransformCoefficient   = (uint32_t)(pCapPipeParams->FECSCParams.Matrix[0][0] * 1024);
        pVeboxFecscState->DW0.FecscC1TransformCoefficient   = (uint32_t)(pCapPipeParams->FECSCParams.Matrix[0][1] * 1024);
        pVeboxFecscState->DW1.FecScC2TransformCoefficient   = (uint32_t)(pCapPipeParams->FECSCParams.Matrix[0][2] * 1024);
        pVeboxFecscState->DW1.FecScC3TransformCoefficient   = (uint32_t)(pCapPipeParams->FECSCParams.Matrix[1][0] * 1024);
        pVeboxFecscState->DW2.FecScC4TransformCoefficient   = (uint32_t)(pCapPipeParams->FECSCParams.Matrix[1][1] * 1024);
        pVeboxFecscState->DW2.FecScC5TransformCoefficient   = (uint32_t)(pCapPipeParams->FECSCParams.Matrix[1][2] * 1024);
        pVeboxFecscState->DW3.FecScC6TransformCoefficient   = (uint32_t)(pCapPipeParams->FECSCParams.Matrix[2][0] * 1024);
        pVeboxFecscState->DW3.FecScC7TransformCoefficient   = (uint32_t)(pCapPipeParams->FECSCParams.Matrix[2][1] * 1024);
        pVeboxFecscState->DW4.FecScC8TransformCoefficient   = (uint32_t)(pCapPipeParams->FECSCParams.Matrix[2][2] * 1024);
        pVeboxFecscState->DW5.FecScOffsetIn1OffsetInForYR   = (uint32_t)(pCapPipeParams->FECSCParams.PreOffset[0] * 1024);
        pVeboxFecscState->DW5.FecScOffsetOut1OffsetOutForYR = (uint32_t)(pCapPipeParams->FECSCParams.PostOffset[0] * 1024);
        pVeboxFecscState->DW6.FecScOffsetIn2OffsetInForUG   = (uint32_t)(pCapPipeParams->FECSCParams.PreOffset[1] * 1024);
        pVeboxFecscState->DW6.FecScOffsetOut2OffsetOutForUG = (uint32_t)(pCapPipeParams->FECSCParams.PostOffset[1] * 1024);
        pVeboxFecscState->DW7.FecScOffsetIn3OffsetInForVB   = (uint32_t)(pCapPipeParams->FECSCParams.PreOffset[2] * 1024);
        pVeboxFecscState->DW7.FecScOffsetOut3OffsetOutForVB = (uint32_t)(pCapPipeParams->FECSCParams.PostOffset[2] * 1024);
    }
    else if (pVeboxIecpParams->ColorSpace == MHW_CSpace_BT601)
    {
        pVeboxFecscState->DW0.FecscC0TransformCoefficient = 263;
        pVeboxFecscState->DW0.FecscC1TransformCoefficient = 516;
        pVeboxFecscState->DW1.FecScC2TransformCoefficient = 100;
        pVeboxFecscState->DW1.FecScC3TransformCoefficient = MOS_BITFIELD_VALUE((uint32_t)-152, 13);
        pVeboxFecscState->DW2.FecScC4TransformCoefficient = MOS_BITFIELD_VALUE((uint32_t)-298, 13);
        pVeboxFecscState->DW2.FecScC5TransformCoefficient = 450;
        pVeboxFecscState->DW3.FecScC6TransformCoefficient = 450;
        pVeboxFecscState->DW3.FecScC7TransformCoefficient = MOS_BITFIELD_VALUE((uint32_t)-377, 13);
        pVeboxFecscState->DW4.FecScC8TransformCoefficient = MOS_BITFIELD_VALUE((uint32_t)-73, 13);

        pVeboxFecscState->DW5.FecScOffsetIn1OffsetInForYR   = 0;
        pVeboxFecscState->DW5.FecScOffsetOut1OffsetOutForYR = 64;
        pVeboxFecscState->DW6.FecScOffsetIn2OffsetInForUG   = 0;
        pVeboxFecscState->DW6.FecScOffsetOut2OffsetOutForUG = 512;
        pVeboxFecscState->DW7.FecScOffsetIn3OffsetInForVB   = 0;
        pVeboxFecscState->DW7.FecScOffsetOut3OffsetOutForVB = 512;
    }
    else if (pVeboxIecpParams->ColorSpace == MHW_CSpace_BT709)
    {
        pVeboxFecscState->DW0.FecscC0TransformCoefficient = 187;
        pVeboxFecscState->DW0.FecscC1TransformCoefficient = 629;
        pVeboxFecscState->DW1.FecScC2TransformCoefficient = 63;
        pVeboxFecscState->DW1.FecScC3TransformCoefficient = MOS_BITFIELD_VALUE((uint32_t)-103, 13);
        pVeboxFecscState->DW2.FecScC4TransformCoefficient = MOS_BITFIELD_VALUE((uint32_t)-346, 13);
        pVeboxFecscState->DW2.FecScC5TransformCoefficient = 450;
        pVeboxFecscState->DW3.FecScC6TransformCoefficient = 450;
        pVeboxFecscState->DW3.FecScC7TransformCoefficient = MOS_BITFIELD_VALUE((uint32_t)-409, 13);
        pVeboxFecscState->DW4.FecScC8TransformCoefficient = MOS_BITFIELD_VALUE((uint32_t)-41, 13);

        pVeboxFecscState->DW5.FecScOffsetIn1OffsetInForYR   = 0;
        pVeboxFecscState->DW5.FecScOffsetOut1OffsetOutForYR = 64;
        pVeboxFecscState->DW6.FecScOffsetIn2OffsetInForUG   = 0;
        pVeboxFecscState->DW6.FecScOffsetOut2OffsetOutForUG = 512;
        pVeboxFecscState->DW7.FecScOffsetIn3OffsetInForVB   = 0;
        pVeboxFecscState->DW7.FecScOffsetOut3OffsetOutForVB = 512;
    }
    else
    {
        MHW_ASSERT(false);
    }
}

MOS_STATUS MhwVeboxInterfaceG8::AddVeboxIecpAceState(
    PMHW_VEBOX_IECP_PARAMS pVeboxIecpParams)
{
    PMHW_ACE_PARAMS pAceParams;
    PMHW_VEBOX_HEAP pVeboxHeap;
    int32_t         uiOffset;
    MOS_STATUS      eStatus = MOS_STATUS_SUCCESS;

    mhw_vebox_g8_X::VEBOX_IECP_STATE_CMD *pVeboxIecpState;

    MHW_CHK_NULL(pVeboxIecpParams);
    MHW_CHK_NULL(m_veboxHeap);

    pVeboxHeap = m_veboxHeap;
    uiOffset   = pVeboxHeap->uiCurState * pVeboxHeap->uiInstanceSize;

    pVeboxIecpState = (mhw_vebox_g8_X::VEBOX_IECP_STATE_CMD *)(pVeboxHeap->pLockedDriverResourceMem +
                                                               pVeboxHeap->uiIecpStateOffset +
                                                               uiOffset);
    MHW_CHK_NULL(pVeboxIecpState);

    MhwVeboxInterfaceGeneric<mhw_vebox_g8_X>::SetVeboxAceLaceState(pVeboxIecpParams, pVeboxIecpState);

finish:
    return eStatus;
}

MOS_STATUS MhwVeboxInterfaceG8::GetVeboxAce_FullImageHistogram(
    uint32_t *pFullImageHistogram)
{
    MOS_STATUS      eStatus = MOS_STATUS_SUCCESS;
    PMHW_VEBOX_HEAP pVeboxHeap;
    int32_t         uiOffset;

    mhw_vebox_g8_X::VEBOX_IECP_STATE_CMD *pVeboxIecpState;

    MHW_CHK_NULL(pFullImageHistogram);
    MHW_CHK_NULL(m_veboxHeap);

    pVeboxHeap = m_veboxHeap;
    uiOffset   = pVeboxHeap->uiCurState * pVeboxHeap->uiInstanceSize;

    pVeboxIecpState = (mhw_vebox_g8_X::VEBOX_IECP_STATE_CMD *)(pVeboxHeap->pLockedDriverResourceMem +
                                                               pVeboxHeap->uiIecpStateOffset +
                                                               uiOffset);
    MHW_CHK_NULL(pVeboxIecpState);
    *pFullImageHistogram = pVeboxIecpState->AceState.DW0.FullImageHistogram;

finish:
    return eStatus;
}

void MhwVeboxInterfaceG8::IecpStateInitialization(
    mhw_vebox_g8_X::VEBOX_IECP_STATE_CMD  *pVeboxIecpState)
{
    MHW_CHK_NULL_NO_STATUS_RETURN(pVeboxIecpState);

    mhw_vebox_g8_X::VEBOX_IECP_STATE_CMD IecpState;
    *pVeboxIecpState = IecpState;

    // Initialize the values to default for media driver.
    pVeboxIecpState->StdSteState.DW5.InvMarginVyl = 3300;
    pVeboxIecpState->StdSteState.DW5.InvSkinTypesMargin = 1638;

    pVeboxIecpState->StdSteState.DW12.B3U = 140;
    pVeboxIecpState->StdSteState.DW15.Satp1 = 122;
    pVeboxIecpState->StdSteState.DW15.Satb1 = 1016;
    pVeboxIecpState->StdSteState.DW19.Hueb1 = 1016;
    pVeboxIecpState->StdSteState.DW27.Hues0Dark = 256;
    pVeboxIecpState->StdSteState.DW27.Hues1Dark = 0;

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

void MhwVeboxInterfaceG8::GamutStateInitialization(
    mhw_vebox_g8_X::VEBOX_GAMUT_STATE_CMD  *pGamutState)
{
    MHW_CHK_NULL_NO_STATUS_RETURN(pGamutState);

    mhw_vebox_g8_X::VEBOX_GAMUT_STATE_CMD cmd;
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