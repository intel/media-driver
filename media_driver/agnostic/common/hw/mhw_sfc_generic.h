/*
* Copyright (c) 2017-2019, Intel Corporation
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

        MHW_CHK_NULL_RETURN(m_osInterface);
        MHW_CHK_NULL_RETURN(pCmdBuffer);
        MHW_CHK_NULL_RETURN(pSfcLockParams);

        cmd.DW1.VeSfcPipeSelect                    = (pSfcLockParams->sfcPipeMode == SFC_PIPE_MODE_VEBOX) ? 1 : 0;
        cmd.DW1.PreScaledOutputSurfaceOutputEnable = pSfcLockParams->bOutputToMemory ? 1 : 0;

        MHW_CHK_STATUS_RETURN(m_osInterface->pfnAddCommand(pCmdBuffer, &cmd, cmd.byteSize));
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AddSfcFrameStart(
        PMOS_COMMAND_BUFFER            pCmdBuffer,
        uint8_t                        sfcPipeMode)
    {
        typename TSfcCmds::SFC_FRAME_START_CMD cmd;
        MHW_CHK_NULL_RETURN(m_osInterface);
        MHW_CHK_NULL_RETURN(pCmdBuffer);
        MHW_CHK_STATUS_RETURN(m_osInterface->pfnAddCommand(pCmdBuffer, &cmd, cmd.byteSize));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AddSfcIefState(
        PMOS_COMMAND_BUFFER            pCmdBuffer,
        PMHW_SFC_IEF_STATE_PARAMS      pSfcIefStateParams)
    {
        typename TSfcCmds::SFC_IEF_STATE_CMD cmd;

        MHW_CHK_NULL_RETURN(m_osInterface);
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

        MHW_CHK_STATUS_RETURN(m_osInterface->pfnAddCommand(pCmdBuffer, &cmd, cmd.byteSize));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AddSfcAvsChromaTable(
        PMOS_COMMAND_BUFFER             pCmdBuffer,
        PMHW_SFC_AVS_CHROMA_TABLE       pChromaTable)
    {
        PSFC_AVS_CHROMA_FILTER_COEFF                      pChromaCoeff;
        typename TSfcCmds::SFC_AVS_CHROMA_Coeff_Table_CMD cmd;

        MHW_CHK_NULL_RETURN(m_osInterface);
        MHW_CHK_NULL_RETURN(pCmdBuffer);
        MHW_CHK_NULL_RETURN(pChromaTable);
        pChromaCoeff = pChromaTable->ChromaTable;

        // Copy programmed State tables into the command
        MHW_CHK_STATUS_RETURN(MOS_SecureMemcpy(
            &(cmd.DW1),
            sizeof(SFC_AVS_CHROMA_FILTER_COEFF)* 32,
            pChromaCoeff,
            sizeof(SFC_AVS_CHROMA_FILTER_COEFF)* 32));

        MHW_CHK_STATUS_RETURN(m_osInterface->pfnAddCommand(pCmdBuffer, &cmd, cmd.byteSize));

        return MOS_STATUS_SUCCESS;
    }

   MOS_STATUS AddSfcAvsLumaTable(
       PMOS_COMMAND_BUFFER             pCmdBuffer,
       PMHW_SFC_AVS_LUMA_TABLE         pLumaTable)
   {
       PSFC_AVS_LUMA_FILTER_COEFF                      pLumaCoeff;
       typename TSfcCmds::SFC_AVS_LUMA_Coeff_Table_CMD cmd;

       MHW_CHK_NULL_RETURN(m_osInterface);
       MHW_CHK_NULL_RETURN(pCmdBuffer);
       MHW_CHK_NULL_RETURN(pLumaTable);

       pLumaCoeff = pLumaTable->LumaTable;

       // Copy programmed State tables into the command
       MHW_CHK_STATUS_RETURN(MOS_SecureMemcpy(
           &(cmd.DW1),
           sizeof(SFC_AVS_LUMA_FILTER_COEFF)* 32,
           pLumaCoeff,
           sizeof(SFC_AVS_LUMA_FILTER_COEFF)* 32));

       MHW_CHK_STATUS_RETURN(m_osInterface->pfnAddCommand(pCmdBuffer, &cmd, cmd.byteSize));

       return MOS_STATUS_SUCCESS;
   }

   MOS_STATUS AddSfcAvsState(
       PMOS_COMMAND_BUFFER             pCmdBuffer,
       PMHW_SFC_AVS_STATE              pSfcAvsState)
   {
       typename TSfcCmds::SFC_AVS_STATE_CMD cmd;
       MHW_CHK_NULL_RETURN(m_osInterface);
       MHW_CHK_NULL_RETURN(pCmdBuffer);
       MHW_CHK_NULL_RETURN(pSfcAvsState);

       // Inilizatialied the SFC_AVS_STATE_CMD
       cmd.DW1.TransitionAreaWith8Pixels = 5;
       cmd.DW1.TransitionAreaWith4Pixels = 4;
       if (pSfcAvsState->dwAVSFilterMode == MEDIASTATE_SFC_AVS_FILTER_BILINEAR)
       {
           cmd.DW1.SharpnessLevel = 0;
       }
       else
       {
           cmd.DW1.SharpnessLevel = 255;
       }

       cmd.DW2.MaxDerivativePoint8       = 20;
       cmd.DW2.MaxDerivative4Pixels      = 7;

       MHW_CHK_STATUS_RETURN(m_osInterface->pfnAddCommand(pCmdBuffer, &cmd, cmd.byteSize));
       return MOS_STATUS_SUCCESS;
   }
};

#endif  // __MHW_SFC_GENERIC_H__
