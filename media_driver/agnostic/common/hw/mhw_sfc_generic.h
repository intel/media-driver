/*
* Copyright (c) 2017-2018, Intel Corporation
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
//! \file     mhw_sfc_generic.h
//! \brief    MHW interface templates for render engine commands
//! \details  Impelements shared HW command construction functions across all platforms as templates
//!

#ifndef __MHW_SFC_GENERIC_H__
#define __MHW_SFC_GENERIC_H__

#include "mhw_sfc.h"

template <class TSfcCmds>
class MhwSfcInterfaceGeneric : public MhwSfcInterface
{
public:

    MhwSfcInterfaceGeneric(PMOS_INTERFACE pOsInterface) : MhwSfcInterface(pOsInterface)
    {
        MHW_FUNCTION_ENTER;
    }

    virtual ~MhwSfcInterfaceGeneric() { MHW_FUNCTION_ENTER; }

    MOS_STATUS AddSfcLock(
        PMOS_COMMAND_BUFFER            pCmdBuffer,
        PMHW_SFC_LOCK_PARAMS           pSfcLockParams)
    {
        typename TSfcCmds::SFC_LOCK_CMD cmd;

        MHW_CHK_NULL_RETURN(pCmdBuffer);
        MHW_CHK_NULL_RETURN(pSfcLockParams);

        cmd.DW1.VeSfcPipeSelect                    = (pSfcLockParams->sfcPipeMode == SFC_PIPE_MODE_VEBOX) ? 1 : 0;
        cmd.DW1.PreScaledOutputSurfaceOutputEnable = pSfcLockParams->bOutputToMemory ? 1 : 0;

        MHW_CHK_STATUS_RETURN(Mos_AddCommand(pCmdBuffer, &cmd, cmd.byteSize));
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AddSfcFrameStart(
        PMOS_COMMAND_BUFFER            pCmdBuffer,
        uint8_t                        sfcPipeMode)
    {
        typename TSfcCmds::SFC_FRAME_START_CMD cmd;
        MHW_CHK_NULL_RETURN(pCmdBuffer);
        MHW_CHK_STATUS_RETURN(Mos_AddCommand(pCmdBuffer, &cmd, cmd.byteSize));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AddSfcIefState(
        PMOS_COMMAND_BUFFER            pCmdBuffer,
        PMHW_SFC_IEF_STATE_PARAMS      pSfcIefStateParams)
    {
        typename TSfcCmds::SFC_IEF_STATE_CMD cmd;

        MHW_CHK_NULL_RETURN(pCmdBuffer);
        MHW_CHK_NULL_RETURN(pSfcIefStateParams);

        // Init SFC_IEF_STATE_CMD
        cmd.DW1.GainFactor            = 0;
        cmd.DW1.R3XCoefficient        = 0;
        cmd.DW1.R3CCoefficient        = 0;
        cmd.DW2.GlobalNoiseEstimation = 0;
        cmd.DW2.R5XCoefficient        = 0;
        cmd.DW2.R5CxCoefficient       = 0;
        cmd.DW2.R5CCoefficient        = 0;

        cmd.DW3.StdSinAlpha  = 101;
        cmd.DW3.StdCosAlpha  = 79;
        cmd.DW5.DiamondAlpha = 100;
        cmd.DW7.InvMarginVyl = 3300;
        cmd.DW8.InvMarginVyu = 1600;
        cmd.DW10.S0L         = MOS_BITFIELD_VALUE((uint32_t)-5, 11);
        cmd.DW10.YSlope2     = 31;
        cmd.DW12.YSlope1     = 31;
        cmd.DW14.S0U         = 256;
        cmd.DW15.S1U         = 113;
        cmd.DW15.S2U         = MOS_BITFIELD_VALUE((uint32_t)-179, 11);

        // Set IEF Params
        if (pSfcIefStateParams->bIEFEnable)
        {
            cmd.DW1.GainFactor          = pSfcIefStateParams->dwGainFactor;
            cmd.DW1.StrongEdgeThreshold = pSfcIefStateParams->StrongEdgeThreshold;
            cmd.DW1.R3XCoefficient      = pSfcIefStateParams->dwR3xCoefficient;
            cmd.DW1.R3CCoefficient      = pSfcIefStateParams->dwR3cCoefficient;
            cmd.DW2.StrongEdgeWeight    = pSfcIefStateParams->StrongEdgeWeight;
            cmd.DW2.RegularWeight       = pSfcIefStateParams->RegularWeight;
            cmd.DW2.R5XCoefficient      = pSfcIefStateParams->dwR5xCoefficient;
            cmd.DW2.R5CxCoefficient     = pSfcIefStateParams->dwR5cxCoefficient;
            cmd.DW2.R5CCoefficient      = pSfcIefStateParams->dwR5cCoefficient;
            cmd.DW4.VyStdEnable         = pSfcIefStateParams->bVYSTDEnable;
            cmd.DW5.SkinDetailFactor    = pSfcIefStateParams->bSkinDetailFactor;
        }

        // Set CSC Params
        if (pSfcIefStateParams->bCSCEnable)
        {
            MHW_CHK_NULL_RETURN(pSfcIefStateParams->pfCscCoeff);
            MHW_CHK_NULL_RETURN(pSfcIefStateParams->pfCscInOffset);
            MHW_CHK_NULL_RETURN(pSfcIefStateParams->pfCscOutOffset);
            cmd.DW16.TransformEnable = true;

            cmd.DW16.C0 = (uint32_t)MOS_F_ROUND(pSfcIefStateParams->pfCscCoeff[0] * 1024.0F);  // S2.10
            cmd.DW16.C1 = (uint32_t)MOS_F_ROUND(pSfcIefStateParams->pfCscCoeff[1] * 1024.0F);  // S2.10
            cmd.DW17.C2 = (uint32_t)MOS_F_ROUND(pSfcIefStateParams->pfCscCoeff[2] * 1024.0F);  // S2.10

            cmd.DW17.C3 = (uint32_t)MOS_F_ROUND(pSfcIefStateParams->pfCscCoeff[3] * 1024.0F);  // S2.10
            cmd.DW18.C4 = (uint32_t)MOS_F_ROUND(pSfcIefStateParams->pfCscCoeff[4] * 1024.0F);  // S2.10
            cmd.DW18.C5 = (uint32_t)MOS_F_ROUND(pSfcIefStateParams->pfCscCoeff[5] * 1024.0F);  // S2.10

            cmd.DW19.C6 = (uint32_t)MOS_F_ROUND(pSfcIefStateParams->pfCscCoeff[6] * 1024.0F);  // S2.10
            cmd.DW19.C7 = (uint32_t)MOS_F_ROUND(pSfcIefStateParams->pfCscCoeff[7] * 1024.0F);  // S2.10
            cmd.DW20.C8 = (uint32_t)MOS_F_ROUND(pSfcIefStateParams->pfCscCoeff[8] * 1024.0F);  // S2.10

            cmd.DW21.OffsetIn1 = (uint32_t)MOS_F_ROUND(pSfcIefStateParams->pfCscInOffset[0] * 4.0F);  // S8.2
            cmd.DW22.OffsetIn2 = (uint32_t)MOS_F_ROUND(pSfcIefStateParams->pfCscInOffset[1] * 4.0F);  // S8.2
            cmd.DW23.OffsetIn3 = (uint32_t)MOS_F_ROUND(pSfcIefStateParams->pfCscInOffset[2] * 4.0F);  // S8.2

            cmd.DW21.OffsetOut1 = (uint32_t)MOS_F_ROUND(pSfcIefStateParams->pfCscOutOffset[0] * 4.0F);  // S8.2
            cmd.DW22.OffsetOut2 = (uint32_t)MOS_F_ROUND(pSfcIefStateParams->pfCscOutOffset[1] * 4.0F);  // S8.2
            cmd.DW23.OffsetOut3 = (uint32_t)MOS_F_ROUND(pSfcIefStateParams->pfCscOutOffset[2] * 4.0F); // S8.2
        }

        MHW_CHK_STATUS_RETURN(Mos_AddCommand(pCmdBuffer, &cmd, cmd.byteSize));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AddSfcAvsChromaTable(
        PMOS_COMMAND_BUFFER             pCmdBuffer,
        PMHW_SFC_AVS_CHROMA_TABLE       pChromaTable)
    {
        PSFC_AVS_CHROMA_FILTER_COEFF                      pChromaCoeff;
        typename TSfcCmds::SFC_AVS_CHROMA_Coeff_Table_CMD cmd;

        MHW_CHK_NULL_RETURN(pCmdBuffer);
        MHW_CHK_NULL_RETURN(pChromaTable);
        pChromaCoeff = pChromaTable->ChromaTable;

        // Copy programmed State tables into the command
        MHW_CHK_STATUS_RETURN(MOS_SecureMemcpy(
            &(cmd.DW1),
            sizeof(SFC_AVS_CHROMA_FILTER_COEFF)* 32,
            pChromaCoeff,
            sizeof(SFC_AVS_CHROMA_FILTER_COEFF)* 32));

        MHW_CHK_STATUS_RETURN(Mos_AddCommand(pCmdBuffer, &cmd, cmd.byteSize));

        return MOS_STATUS_SUCCESS;
    }

   MOS_STATUS AddSfcAvsLumaTable(
       PMOS_COMMAND_BUFFER             pCmdBuffer,
       PMHW_SFC_AVS_LUMA_TABLE         pLumaTable)
   {
       PSFC_AVS_LUMA_FILTER_COEFF                      pLumaCoeff;
       typename TSfcCmds::SFC_AVS_LUMA_Coeff_Table_CMD cmd;

       MHW_CHK_NULL_RETURN(pCmdBuffer);
       MHW_CHK_NULL_RETURN(pLumaTable);

       pLumaCoeff = pLumaTable->LumaTable;

       // Copy programmed State tables into the command
       MHW_CHK_STATUS_RETURN(MOS_SecureMemcpy(
           &(cmd.DW1),
           sizeof(SFC_AVS_LUMA_FILTER_COEFF)* 32,
           pLumaCoeff,
           sizeof(SFC_AVS_LUMA_FILTER_COEFF)* 32));

       MHW_CHK_STATUS_RETURN(Mos_AddCommand(pCmdBuffer, &cmd, cmd.byteSize));

       return MOS_STATUS_SUCCESS;
   }

   MOS_STATUS AddSfcState(
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
       typename TSfcCmds::SFC_STATE_CMD cmd;

       MHW_CHK_NULL_RETURN(pCmdBuffer);
       MHW_CHK_NULL_RETURN(pSfcStateParams);
       MHW_CHK_NULL_RETURN(pOutSurface);

       pOsInterface = m_osInterface;

       MHW_CHK_NULL_RETURN(pOsInterface);
       pWaTable = pOsInterface->pfnGetWaTable(pOsInterface);
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

       cmd.DW1.SfcPipeMode               = pSfcStateParams->sfcPipeMode;
       cmd.DW1.SfcInputChromaSubSampling = pSfcStateParams->dwInputChromaSubSampling;
       cmd.DW1.VdVeInputOrderingMode     = pSfcStateParams->dwVDVEInputOrderingMode;

       // Set DW2
       cmd.DW2.InputFrameResolutionWidth = pSfcStateParams->dwInputFrameWidth - 1;
       cmd.DW2.InputFrameResolutionHeight = pSfcStateParams->dwInputFrameHeight - 1;

       switch (pSfcStateParams->OutputFrameFormat)
       {
           case Format_AYUV: //AYUV 4:4:4 (8:8:8:8 MSB-A:Y:U:V)
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
               bInterleaveChroma = true;
               wUYOffset = (uint16_t)pOutSurface->dwHeight;
               break;
           case Format_YUY2:
               cmd.DW3.OutputSurfaceFormatType = cmd.OUTPUT_SURFACE_FORMAT_TYPE_YUYV;
               break;
           case Format_UYVY:
               cmd.DW3.OutputSurfaceFormatType = cmd.OUTPUT_SURFACE_FORMAT_TYPE_UYVY;
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

       // Vertical line issue for SFC 270 degree rotation & NV12 output
       // HW requires Scaled Region Size Width < Output Frame Width && Scaled Region Size Height < Output Frame Height.
       // HW Design team recommends to program both Output Frame Width/Height to the maximum value of both for the new SW WA.
       if (MEDIA_IS_WA(pWaTable, WaSFC270DegreeRotation) &&
           cmd.DW4.RotationMode == MHW_ROTATION_270 &&
           cmd.DW3.OutputSurfaceFormatType == cmd.OUTPUT_SURFACE_FORMAT_TYPE_NV12)
       {
           MHW_NORMALMESSAGE("SW WA vertical line issue for SFC 270 degree rotation & NV12 output.");
           MHW_NORMALMESSAGE("SFC_STATE before SW WA: OutputFrameWidth = %d, OutputFrameHeight = %d, ScaledRegionSizeWidth = %d, ScaledRegionSizeHeight = %d.",
               cmd.DW7.OutputFrameWidth, cmd.DW7.OutputFrameHeight,
               cmd.DW8.ScaledRegionSizeWidth, cmd.DW8.ScaledRegionSizeHeight);
           if (cmd.DW7.OutputFrameWidth > cmd.DW7.OutputFrameHeight)
           {
               cmd.DW7.OutputFrameHeight = cmd.DW7.OutputFrameWidth;
           }
           else
           {
               cmd.DW7.OutputFrameWidth = cmd.DW7.OutputFrameHeight;
           }
           MHW_NORMALMESSAGE("SFC_STATE after SW WA: OutputFrameWidth = %d, OutputFrameHeight = %d, ScaledRegionSizeWidth = %d, ScaledRegionSizeHeight = %d.",
               cmd.DW7.OutputFrameWidth, cmd.DW7.OutputFrameHeight,
               cmd.DW8.ScaledRegionSizeWidth, cmd.DW8.ScaledRegionSizeHeight);
       }

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
                                                      = (uint32_t)m_avsLineBufferCtrl.Gen9.Index;
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
       cmd.DW30.OutputSurfaceYOffsetForU = wUYOffset;
       cmd.DW30.OutputSurfaceXOffsetForU = wUXOffset;
       cmd.DW31.OutputSurfaceYOffsetForV = wVYOffset;
       cmd.DW31.OutputSurfaceXOffsetForV = wVXOffset;

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

   MOS_STATUS AddSfcAvsState(
       PMOS_COMMAND_BUFFER             pCmdBuffer,
       PMHW_SFC_AVS_STATE              pSfcAvsState)
   {
       typename TSfcCmds::SFC_AVS_STATE_CMD cmd;
       MHW_CHK_NULL_RETURN(pCmdBuffer);

       // Inilizatialied the SFC_AVS_STATE_CMD
       cmd.DW1.TransitionAreaWith8Pixels = 5;
       cmd.DW1.TransitionAreaWith4Pixels = 4;
       cmd.DW1.SharpnessLevel            = 255;

       cmd.DW2.MaxDerivativePoint8       = 20;
       cmd.DW2.MaxDerivative4Pixels      = 7;

       MHW_CHK_STATUS_RETURN(Mos_AddCommand(pCmdBuffer, &cmd, cmd.byteSize));
       return MOS_STATUS_SUCCESS;
   }
};

#endif  // __MHW_SFC_GENERIC_H__
