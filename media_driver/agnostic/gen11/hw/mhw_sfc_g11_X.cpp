/*
* Copyright (c) 2015-2018, Intel Corporation
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
//! \file     mhw_sfc_g11_X.cpp
//! \brief    Constructs sfc commands on Gen11-based platforms
//! \details  Each client facing function both creates a HW command and adds
//!           that command to a command or batch buffer.
//!

#include "mhw_sfc_g11_X.h"

MOS_STATUS MhwSfcInterfaceG11::AddSfcState(
    PMOS_COMMAND_BUFFER            pCmdBuffer,
    PMHW_SFC_STATE_PARAMS          pSfcStateParams,
    PMHW_SFC_OUT_SURFACE_PARAMS    pOutSurface)
{
    PMOS_INTERFACE              pOsInterface;
    bool                        bHalfPitchForChroma;
    bool                        bInterleaveChroma;
    uint16_t                    wUXOffset;
    uint16_t                    wUYOffset;
    uint16_t                    wVXOffset;
    uint16_t                    wVYOffset;
    MHW_RESOURCE_PARAMS         ResourceParams;
    MEDIA_WA_TABLE              *pWaTable = nullptr;

    mhw_sfc_g11_X::SFC_STATE_CMD cmd;

    MHW_CHK_NULL_RETURN(pCmdBuffer);
    MHW_CHK_NULL_RETURN(pSfcStateParams);
    MHW_CHK_NULL_RETURN(pOutSurface);

    pOsInterface = m_osInterface;
    MHW_CHK_NULL_RETURN(pOsInterface);
    pWaTable     = pOsInterface->pfnGetWaTable(pOsInterface);
    MHW_CHK_NULL_RETURN(pWaTable);

    bHalfPitchForChroma = false;
    bInterleaveChroma   = false;
    wUXOffset           = 0;
    wUYOffset           = 0;
    wVXOffset           = 0;
    wVYOffset           = 0;

    // Check input/output size
    MHW_ASSERT(pSfcStateParams->dwInputFrameWidth   >= MHW_SFC_MIN_WIDTH);
    MHW_ASSERT(pSfcStateParams->dwInputFrameHeight  >= MHW_SFC_MIN_HEIGHT);
    MHW_ASSERT(pSfcStateParams->dwOutputFrameWidth  <= MHW_SFC_MAX_WIDTH);
    MHW_ASSERT(pSfcStateParams->dwOutputFrameHeight <= MHW_SFC_MAX_HEIGHT);

    // Set DW1
    cmd.DW1.SfcPipeMode                  = pSfcStateParams->sfcPipeMode;
    cmd.DW1.SfcInputChromaSubSampling    = pSfcStateParams->dwInputChromaSubSampling;
    cmd.DW1.VdVeInputOrderingMode        = pSfcStateParams->dwVDVEInputOrderingMode;

    // Set DW2
    cmd.DW2.InputFrameResolutionWidth    = pSfcStateParams->dwInputFrameWidth - 1;
    cmd.DW2.InputFrameResolutionHeight   = pSfcStateParams->dwInputFrameHeight - 1;

    // Set DW3
    cmd.DW3.OutputChromaDownsamplingCoSitingPositionVerticalDirection   = pSfcStateParams->dwChromaDownSamplingVerticalCoef;
    cmd.DW3.OutputChromaDownsamplingCoSitingPositionHorizontalDirection = pSfcStateParams->dwChromaDownSamplingHorizontalCoef;

    switch(pSfcStateParams->OutputFrameFormat)
    {
        case Format_AYUV:
            cmd.DW3.OutputSurfaceFormatType = cmd.OUTPUT_SURFACE_FORMAT_TYPE_AYUV;
            break;
        case Format_X8R8G8B8:
        case Format_A8R8G8B8:
            // RGBASwapEnable is true only when CSC is enabled.
            // The OutputSurfaceFormatType is set as A8B8G8R8 for X8R8G8B8 and A8R8G8B8 output.
            cmd.DW3.RgbaChannelSwapEnable = pSfcStateParams->bRGBASwapEnable;
        case Format_X8B8G8R8:
        case Format_A8B8G8R8:
            cmd.DW3.OutputSurfaceFormatType = cmd.OUTPUT_SURFACE_FORMAT_TYPE_A8B8G8R8;
            break;
        case Format_R10G10B10A2:
            cmd.DW3.RgbaChannelSwapEnable = pSfcStateParams->bRGBASwapEnable;
        case Format_B10G10R10A2:
            cmd.DW3.OutputSurfaceFormatType = cmd.OUTPUT_SURFACE_FORMAT_TYPE_A2R10G10B10;
            break;
        case Format_R5G6B5:
            cmd.DW3.OutputSurfaceFormatType = cmd.OUTPUT_SURFACE_FORMAT_TYPE_R5G6B5;
            break;
        case Format_NV12:
            cmd.DW3.OutputSurfaceFormatType = cmd.OUTPUT_SURFACE_FORMAT_TYPE_NV12;
            bInterleaveChroma   = true;
            wUYOffset           = (uint16_t)pOutSurface->dwUYoffset;
            break;
        case Format_YUY2:
            cmd.DW3.OutputSurfaceFormatType = cmd.OUTPUT_SURFACE_FORMAT_TYPE_YUYV;
            break;
        case Format_UYVY:
            cmd.DW3.OutputSurfaceFormatType = cmd.OUTPUT_SURFACE_FORMAT_TYPE_UYVY;
            break;
        case Format_P010:
            cmd.DW3.OutputSurfaceFormatType = cmd.OUTPUT_SURFACE_FORMAT_TYPE_P016;
            cmd.DW4.Bitdepth                = 0;
            wUYOffset                       = (uint16_t)pOutSurface->dwUYoffset;
            break;
        case Format_P016:
            cmd.DW3.OutputSurfaceFormatType = cmd.OUTPUT_SURFACE_FORMAT_TYPE_P016;
            cmd.DW4.Bitdepth                = 1;
            wUYOffset                       = (uint16_t)pOutSurface->dwUYoffset;
            break;
        default:
            MHW_ASSERTMESSAGE("Unknown Output Format.");
            return MOS_STATUS_UNKNOWN;
    }

    // Set DW4
    cmd.DW4.IefEnable                    = pSfcStateParams->bIEFEnable;
    cmd.DW4.SkinToneTunedIefEnable       = pSfcStateParams->bSkinToneTunedIEFEnable;
    cmd.DW4.AvsFilterMode                = pSfcStateParams->dwAVSFilterMode;
    cmd.DW4.AdaptiveFilterForAllChannels = (pSfcStateParams->dwAVSFilterMode == cmd.AVS_FILTER_MODE_8X8POLY_PHASEFILTERBILINEAR_ADAPTIVE) ?
                                                      true : false;
    cmd.DW4.AvsScalingEnable             = ((pSfcStateParams->fAVSXScalingRatio == 1.0F) &&
                                                      (pSfcStateParams->fAVSYScalingRatio == 1.0F)) ? false : true;
    cmd.DW4.BypassYAdaptiveFiltering     = pSfcStateParams->bBypassYAdaptiveFilter;
    cmd.DW4.BypassXAdaptiveFiltering     = pSfcStateParams->bBypassXAdaptiveFilter;
    cmd.DW4.ChromaUpsamplingEnable       = pSfcStateParams->bAVSChromaUpsamplingEnable;
    cmd.DW4.RotationMode                 = pSfcStateParams->RotationMode;
    cmd.DW4.ColorFillEnable              = pSfcStateParams->bColorFillEnable;
    cmd.DW4.CscEnable                    = pSfcStateParams->bCSCEnable;

    if (pSfcStateParams->bMirrorEnable)
    {
        cmd.DW4.Value                   |= (uint32_t) (1 << 14) & 0x00004000;                                          // Mirror Mode
        cmd.DW4.Value                   |= (uint32_t) (pSfcStateParams->dwMirrorType << 13) & 0x00002000;              // Mirror Type
    }

    // Set DW5, DW6, DW7, DW8, DW9
    cmd.DW5.SourceRegionWidth            = pSfcStateParams->dwSourceRegionWidth - 1;
    cmd.DW5.SourceRegionHeight           = pSfcStateParams->dwSourceRegionHeight - 1;
    cmd.DW6.SourceRegionHorizontalOffset = pSfcStateParams->dwSourceRegionHorizontalOffset;
    cmd.DW6.SourceRegionVerticalOffset   = pSfcStateParams->dwSourceRegionVerticalOffset;
    cmd.DW7.OutputFrameWidth             = pSfcStateParams->dwOutputFrameWidth + pOutSurface->dwSurfaceXOffset - 1;
    cmd.DW7.OutputFrameHeight            = pSfcStateParams->dwOutputFrameHeight + pOutSurface->dwSurfaceYOffset - 1;
    cmd.DW8.ScaledRegionSizeWidth        = pSfcStateParams->dwScaledRegionWidth - 1;
    cmd.DW8.ScaledRegionSizeHeight       = pSfcStateParams->dwScaledRegionHeight - 1;
    cmd.DW9.ScaledRegionHorizontalOffset = pSfcStateParams->dwScaledRegionHorizontalOffset + pOutSurface->dwSurfaceXOffset;
    cmd.DW9.ScaledRegionVerticalOffset   = pSfcStateParams->dwScaledRegionVerticalOffset + pOutSurface->dwSurfaceYOffset;

    // Set DW10
    cmd.DW10.GrayBarPixelUG              = MOS_CLAMP_MIN_MAX(MOS_F_ROUND(pSfcStateParams->fColorFillUGPixel * 1024.0F), 0, 1023); // U10
    cmd.DW10.GrayBarPixelYR              = MOS_CLAMP_MIN_MAX(MOS_F_ROUND(pSfcStateParams->fColorFillYRPixel * 1024.0F), 0, 1023); // U10

    // Set DW11
    cmd.DW11.GrayBarPixelA               = MOS_CLAMP_MIN_MAX(MOS_F_ROUND(pSfcStateParams->fColorFillAPixel * 1024.0F), 0, 1023); // U10
    cmd.DW11.GrayBarPixelVB              = MOS_CLAMP_MIN_MAX(MOS_F_ROUND(pSfcStateParams->fColorFillVBPixel * 1024.0F), 0, 1023); // U10

    // Set DW13
    cmd.DW13.AlphaDefaultValue           = MOS_CLAMP_MIN_MAX(MOS_F_ROUND(pSfcStateParams->fAlphaPixel * 1024.0F), 0, 1023); // U10

    // Set DW14
    cmd.DW14.ScalingFactorHeight         = MOS_UF_ROUND((1.0F / pSfcStateParams->fAVSYScalingRatio) * 131072.0F); // U4.17

    // Set DW15
    cmd.DW15.ScalingFactorWidth          = MOS_UF_ROUND((1.0F / pSfcStateParams->fAVSXScalingRatio) * 131072.0f); // U4.17

    // Set DW19
    cmd.DW19.OutputFrameSurfaceBaseAddressMemoryCompressionEnable                   = pSfcStateParams->bMMCEnable;
    cmd.DW19.OutputFrameSurfaceBaseAddressIndexToMemoryObjectControlStateMocsTables = m_outputSurfCtrl.Gen9.Index;

    if (pSfcStateParams->MMCMode == MOS_MMC_VERTICAL)
    {
        cmd.DW19.OutputFrameSurfaceBaseAddressMemoryCompressionMode = 1;
    }

    // Set DW22
    cmd.DW22.AvsLineBufferBaseAddressIndexToMemoryObjectControlStateMocsTables
                                                   = m_avsLineBufferCtrl.Gen9.Index;
    // Set DW25
    cmd.DW25.IefLineBufferBaseAddressIndexToMemoryObjectControlStateMocsTables
                                                   = m_iefLineBufferCtrl.Gen9.Index;
    // Set DW29
    cmd.DW29.OutputSurfaceTileWalk       = (pOutSurface->TileType == MOS_TILE_Y) ?
                                                      true : false;
    cmd.DW29.OutputSurfaceTiled          = (pOutSurface->TileType != MOS_TILE_LINEAR) ?
                                                      true : false;
    cmd.DW29.OutputSurfaceHalfPitchForChroma
                                                   = bHalfPitchForChroma;
    cmd.DW29.OutputSurfacePitch          = pOutSurface->dwPitch - 1;
    cmd.DW29.OutputSurfaceInterleaveChromaEnable
                                                   = bInterleaveChroma;
    cmd.DW29.OutputSurfaceFormat         = cmd.DW3.OutputSurfaceFormatType;

    // Set DW30, DW31
    cmd.DW30.OutputSurfaceYOffsetForU    = wUYOffset;
    cmd.DW30.OutputSurfaceXOffsetForU    = wUXOffset;
    cmd.DW31.OutputSurfaceYOffsetForV    = wVYOffset;
    cmd.DW31.OutputSurfaceXOffsetForV    = wVXOffset;

    if (pSfcStateParams->pOsResOutputSurface)
    {
        MOS_ZeroMemory(&ResourceParams, sizeof(ResourceParams));
        ResourceParams.presResource                = pSfcStateParams->pOsResOutputSurface;
        ResourceParams.pdwCmd                      = &(cmd.DW17.Value);
        ResourceParams.dwLocationInCmd             = 17;
        ResourceParams.HwCommandType               = MOS_SFC_STATE;
        ResourceParams.bIsWritable                 = true;
        ResourceParams.dwOffset                    = pSfcStateParams->dwOutputSurfaceOffset;

        MHW_CHK_STATUS_RETURN(pfnAddResourceToCmd(
          pOsInterface,
          pCmdBuffer,
          &ResourceParams));
    }

    if (pSfcStateParams->pOsResAVSLineBuffer)
    {
        MOS_ZeroMemory(&ResourceParams, sizeof(ResourceParams));
        ResourceParams.presResource                = pSfcStateParams->pOsResAVSLineBuffer;
        ResourceParams.pdwCmd                      = &(cmd.DW20.Value);
        ResourceParams.dwLocationInCmd             = 20;
        ResourceParams.HwCommandType               = MOS_SFC_STATE;
        ResourceParams.bIsWritable                 = true;

        MHW_CHK_STATUS_RETURN(pfnAddResourceToCmd(
          pOsInterface,
          pCmdBuffer,
          &ResourceParams));
    }

    if (pSfcStateParams->pOsResIEFLineBuffer)
    {
        MOS_ZeroMemory(&ResourceParams, sizeof(ResourceParams));
        ResourceParams.presResource                = pSfcStateParams->pOsResIEFLineBuffer;
        ResourceParams.pdwCmd                      = &(cmd.DW23.Value);
        ResourceParams.dwLocationInCmd             = 23;
        ResourceParams.HwCommandType               = MOS_SFC_STATE;
        ResourceParams.bIsWritable                 = true;

        MHW_CHK_STATUS_RETURN(pfnAddResourceToCmd(
          pOsInterface,
          pCmdBuffer,
          &ResourceParams));
    }

    MHW_CHK_STATUS_RETURN(Mos_AddCommand(pCmdBuffer, &cmd, cmd.byteSize));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MhwSfcInterfaceG11::AddSfcAvsState(
    PMOS_COMMAND_BUFFER             pCmdBuffer,
    PMHW_SFC_AVS_STATE              pSfcAvsState)
{
    MHW_CHK_NULL_RETURN(pCmdBuffer);

    mhw_sfc_g11_X::SFC_AVS_STATE_CMD *cmdPtr;
    cmdPtr = (mhw_sfc_g11_X::SFC_AVS_STATE_CMD *)pCmdBuffer->pCmdPtr;

    MHW_CHK_STATUS_RETURN(MhwSfcInterfaceGeneric::AddSfcAvsState(pCmdBuffer, pSfcAvsState));

    if (pSfcAvsState)
    {
        MHW_CHK_NULL_RETURN(cmdPtr);
        cmdPtr->DW3.InputHorizontalSitingValueSpecifiesTheHorizontalSitingOfTheInput = pSfcAvsState->dwInputHorizontalSiting;
        cmdPtr->DW3.InputVerticalSitingSpecifiesTheVerticalSitingOfTheInput          = pSfcAvsState->dwInputVerticalSitting;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MhwSfcInterfaceG11::SetSfcSamplerTable(
    PMHW_SFC_AVS_LUMA_TABLE      pLumaTable, 
    PMHW_SFC_AVS_CHROMA_TABLE    pChromaTable, 
    PMHW_AVS_PARAMS              pAvsParams, 
    MOS_FORMAT                   SrcFormat, 
    float                        fScaleX, 
    float                        fScaleY, 
    uint32_t                     dwChromaSiting, 
    bool                         bUse8x8Filter)
{
    float       fHPStrength;
    int32_t     *piYCoefsX, *piYCoefsY;
    int32_t     *piUVCoefsX, *piUVCoefsY;
    MHW_PLANE   Plane;

    MHW_CHK_NULL_RETURN(pLumaTable);
    MHW_CHK_NULL_RETURN(pChromaTable);
    MHW_CHK_NULL_RETURN(pAvsParams);

    fHPStrength = 0.0F;
    piYCoefsX = pAvsParams->piYCoefsX;
    piYCoefsY = pAvsParams->piYCoefsY;
    piUVCoefsX = pAvsParams->piUVCoefsX;
    piUVCoefsY = pAvsParams->piUVCoefsY;

    //  Skip calculation if no changes to AVS parameters
    if (SrcFormat == pAvsParams->Format  &&
        fScaleX == pAvsParams->fScaleX &&
        fScaleY == pAvsParams->fScaleY)
    {
        return MOS_STATUS_SUCCESS;
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

    // Recalculate Horizontal scaling table
    if (SrcFormat != pAvsParams->Format || fScaleX != pAvsParams->fScaleX)
    {
        MOS_ZeroMemory(
            piYCoefsX,
            8 * 32 * sizeof(int32_t));

        MOS_ZeroMemory(
            piUVCoefsX,
            4 * 32 * sizeof(int32_t));

        // 4-tap filtering for RGB format G-channel.
        Plane = IS_RGB32_FORMAT(SrcFormat) ? MHW_U_PLANE : MHW_Y_PLANE;

        pAvsParams->fScaleX = fScaleX;

        // For 1x scaling in horizontal direction and not force polyphase coefs, use special coefficients for filtering
        if (fScaleX == 1.0F && !pAvsParams->bForcePolyPhaseCoefs)
        {
            MHW_CHK_STATUS_RETURN(Mhw_SetNearestModeTable(
                piYCoefsX,
                Plane,
                true));

            MHW_CHK_STATUS_RETURN(Mhw_SetNearestModeTable(
                piUVCoefsX,
                MHW_U_PLANE,
                true));
        }
        else
        {
            // Clamp the Scaling Factor if > 1.0x
            fScaleX = MOS_MIN(1.0F, fScaleX);

            MHW_CHK_STATUS_RETURN(Mhw_CalcPolyphaseTablesY(
                piYCoefsX,
                fScaleX,
                Plane,
                SrcFormat,
                fHPStrength,
                bUse8x8Filter,
                NUM_HW_POLYPHASE_TABLES));
        }

        // If Chroma Siting info is present
        if (dwChromaSiting & MHW_CHROMA_SITING_HORZ_LEFT)
        {
            // No Chroma Siting
            MHW_CHK_STATUS_RETURN(Mhw_CalcPolyphaseTablesUV(
                piUVCoefsX,
                2.0F,
                fScaleX));
        }
        else
        {
            // Chroma siting offset will be add in the HW cmd
            MHW_CHK_STATUS_RETURN(Mhw_CalcPolyphaseTablesUV(
                piUVCoefsX,
                3.0F,
                fScaleX));
        }
    }

    // Recalculate Vertical scaling table
    if (SrcFormat != pAvsParams->Format || fScaleY != pAvsParams->fScaleY)
    {
        MOS_ZeroMemory(piYCoefsY, 8 * 32 * sizeof(int32_t));

        MOS_ZeroMemory(piUVCoefsY, 4 * 32 * sizeof(int32_t));

        // 4-tap filtering for RGB format G-channel.
        Plane = IS_RGB32_FORMAT(SrcFormat) ? MHW_U_PLANE : MHW_Y_PLANE;

        pAvsParams->fScaleY = fScaleY;

        // For 1x scaling in vertical direction and not force polyphase coefs, use special coefficients for filtering
        if (fScaleY == 1.0F && !pAvsParams->bForcePolyPhaseCoefs)
        {
            MHW_CHK_STATUS_RETURN(Mhw_SetNearestModeTable(
                piYCoefsY,
                Plane,
                true));

            MHW_CHK_STATUS_RETURN(Mhw_SetNearestModeTable(
                piUVCoefsY,
                MHW_U_PLANE,
                true));
        }
        else
        {
            // Clamp the Scaling Factor if > 1.0x
            fScaleY = MOS_MIN(1.0F, fScaleY);

            MHW_CHK_STATUS_RETURN(Mhw_CalcPolyphaseTablesY(
                piYCoefsY,
                fScaleY,
                Plane,
                SrcFormat,
                fHPStrength,
                bUse8x8Filter,
                NUM_HW_POLYPHASE_TABLES));
        }

        // If Chroma Siting info is present
        if (dwChromaSiting & MHW_CHROMA_SITING_VERT_TOP)
        {
            // No Chroma Siting
            MHW_CHK_STATUS_RETURN(Mhw_CalcPolyphaseTablesUV(
                piUVCoefsY,
                2.0F,
                fScaleY));
        }
        else
        {
            // Chroma siting offset will be add in the HW cmd
            MHW_CHK_STATUS_RETURN(Mhw_CalcPolyphaseTablesUV(
                piUVCoefsY,
                3.0F,
                fScaleY));
        }
    }

    // Save format used to calculate AVS parameters
    pAvsParams->Format = SrcFormat;

    SetSfcAVSLumaTable(
        SrcFormat,
        pLumaTable->LumaTable,
        piYCoefsX,
        piYCoefsY,
        false);

    SetSfcAVSChromaTable(
        pChromaTable->ChromaTable,
        piUVCoefsX,
        piUVCoefsY);

    return MOS_STATUS_SUCCESS;
}

MhwSfcInterfaceG11::MhwSfcInterfaceG11(PMOS_INTERFACE pOsInterface) : MhwSfcInterfaceGeneric(pOsInterface)
{
    if (m_osInterface == nullptr)
    {
         MHW_ASSERTMESSAGE("Invalid Input Paramete: m_osInterface is nullptr");
        return;
    }

    // Get Memory control object directly from MOS.
    // If any override is needed, something like pfnOverrideMemoryObjectCtrl() / pfnComposeSurfaceCacheabilityControl()
    // will need to be implemented.
    m_outputSurfCtrl.Value = m_osInterface->pfnCachePolicyGetMemoryObject(
        MOS_MHW_RESOURCE_USAGE_Sfc_CurrentOutputSurface,
        m_osInterface->pfnGetGmmClientContext(m_osInterface)).DwordValue;

    m_avsLineBufferCtrl.Value = m_osInterface->pfnCachePolicyGetMemoryObject(
        MOS_MHW_RESOURCE_USAGE_Sfc_AvsLineBufferSurface,
        m_osInterface->pfnGetGmmClientContext(m_osInterface)).DwordValue;
    m_iefLineBufferCtrl.Value = m_osInterface->pfnCachePolicyGetMemoryObject(
        MOS_MHW_RESOURCE_USAGE_Sfc_IefLineBufferSurface,
        m_osInterface->pfnGetGmmClientContext(m_osInterface)).DwordValue;
}
