/*
* Copyright (c) 2017-2021, Intel Corporation
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
//! \file     mhw_vebox_generic.h
//! \brief    MHW interface templates for render engine commands
//! \details  Impelements shared HW command construction functions across all platforms as templates
//!

#ifndef __MHW_VEBOX_GENERIC_H__
#define __MHW_VEBOX_GENERIC_H__

#include "mhw_vebox.h"

template <class TVeboxCmds>
class MhwVeboxInterfaceGeneric : public MhwVeboxInterface
{
public:
    //!
    //! \brief    Initializes the Vebox interface
    //! \details  Internal MHW function to initialize all function pointers and some parameters
    //!           Assumes that the caller has checked pointer validity and whether or not an
    //!           addressing method has been selected in the OS interface (bUsesGfxAddress or
    //!           bUsesPatchList).
    //! \param    [in] pOsInterface
    //!           OS interface, must be valid
    //!
    MhwVeboxInterfaceGeneric(
        PMOS_INTERFACE pOsInterface) : MhwVeboxInterface(pOsInterface)
    {
        MHW_FUNCTION_ENTER;
    }

    virtual ~MhwVeboxInterfaceGeneric() { MHW_FUNCTION_ENTER; }

    MOS_STATUS AddVeboxVertexTable(
        MHW_CSPACE           ColorSpace)
    {
        MHW_FUNCTION_ENTER;
        PMHW_VEBOX_HEAP         pVeboxHeap;
        uint32_t                uiOffset;
        uint32_t                uSize;
        MOS_STATUS              eStatus = MOS_STATUS_SUCCESS;

        typename TVeboxCmds::VEBOX_VERTEX_TABLE_CMD *pVertexTable;
        MHW_CHK_NULL(m_veboxHeap);

        pVeboxHeap   = m_veboxHeap;
        uiOffset     = pVeboxHeap->uiCurState * pVeboxHeap->uiInstanceSize;
        uSize        = sizeof(typename TVeboxCmds::VEBOX_VERTEX_TABLE_CMD);
        pVertexTable = (typename TVeboxCmds::VEBOX_VERTEX_TABLE_CMD *)(pVeboxHeap->pLockedDriverResourceMem +
                                                 pVeboxHeap->uiVertexTableOffset +
                                                 uiOffset);
        MOS_ZeroMemory(pVertexTable, uSize);
        if (ColorSpace == MHW_CSpace_BT601 || ColorSpace == MHW_CSpace_xvYCC601)
        {
            MOS_SecureMemcpy(pVertexTable, uSize, g_VeboxVertexTableBT601, uSize);
        }
        else if (ColorSpace == MHW_CSpace_BT709 || ColorSpace == MHW_CSpace_xvYCC709)
        {
            MOS_SecureMemcpy(pVertexTable, uSize, g_VeboxVertexTableBT709, uSize);
        }
        else
        {
            MHW_NORMALMESSAGE("Unhandled ColorSpace");
        }

    finish:
        return eStatus;
    }

    virtual MOS_STATUS AddVeboxSurfaces(
        PMOS_COMMAND_BUFFER                     pCmdBuffer,
        PMHW_VEBOX_SURFACE_STATE_CMD_PARAMS     pVeboxSurfaceStateCmdParams)
    {
        MHW_FUNCTION_ENTER;
        MOS_STATUS eStatus;
        bool       bOutputValid;

        typename TVeboxCmds::VEBOX_SURFACE_STATE_CMD cmd1, cmd2;

        MEDIA_FEATURE_TABLE    *pSkuTable = nullptr;
        MHW_CHK_NULL(m_osInterface);
        pSkuTable = m_osInterface->pfnGetSkuTable(m_osInterface);
        MHW_CHK_NULL(pSkuTable);
        MHW_CHK_NULL(pCmdBuffer);
        MHW_CHK_NULL(pVeboxSurfaceStateCmdParams);

        eStatus      = MOS_STATUS_SUCCESS;
        bOutputValid = pVeboxSurfaceStateCmdParams->bOutputValid;

        // Setup Surface State for Input surface
        SetVeboxSurfaces(
            &pVeboxSurfaceStateCmdParams->SurfInput,
            &pVeboxSurfaceStateCmdParams->SurfSTMM,
            nullptr,
            &cmd1,
            false,
            pVeboxSurfaceStateCmdParams->bDIEnable);

        if (pVeboxSurfaceStateCmdParams->b3DlutEnable && MEDIA_IS_SKU(pSkuTable, FtrHeight8AlignVE3DLUTDualPipe))
        {
            cmd1.DW2.Height = MOS_ALIGN_CEIL((cmd1.DW2.Height + 1), 8) - 1;
            MHW_NORMALMESSAGE("Align Input Height as 8x due to 3DlutEnable");
        }

        m_osInterface->pfnAddCommand(pCmdBuffer, &cmd1, cmd1.byteSize);
        MHW_NORMALMESSAGE("Vebox input Height: %d, Width: %d;", cmd1.DW2.Height, cmd1.DW2.Width);

        // Setup Surface State for Output surface
        if (bOutputValid)
        {
            SetVeboxSurfaces(
                &pVeboxSurfaceStateCmdParams->SurfOutput,
                &pVeboxSurfaceStateCmdParams->SurfDNOutput,
                &pVeboxSurfaceStateCmdParams->SurfSkinScoreOutput,
                &cmd2,
                true,
                pVeboxSurfaceStateCmdParams->bDIEnable);

            // Reset Output Format When Input/Output Format are the same
            if (pVeboxSurfaceStateCmdParams->SurfInput.Format == pVeboxSurfaceStateCmdParams->SurfOutput.Format)
            {
                cmd2.DW3.SurfaceFormat = cmd1.DW3.SurfaceFormat;
            }

            if (pVeboxSurfaceStateCmdParams->b3DlutEnable && MEDIA_IS_SKU(pSkuTable, FtrHeight8AlignVE3DLUTDualPipe))
            {
                cmd2.DW2.Height = MOS_ALIGN_CEIL((cmd2.DW2.Height + 1), 8) - 1;
                MHW_NORMALMESSAGE("Align Output Height as 8x due to 3DlutEnable");
            }

            m_osInterface->pfnAddCommand(pCmdBuffer, &cmd2, cmd2.byteSize);
            MHW_NORMALMESSAGE("Vebox output Height: %d, Width: %d;", cmd2.DW2.Height, cmd2.DW2.Width);
        }

    finish:
        return eStatus;
    }

        //!
    //! \brief    Add Vebox Tiling Convert Control Bits
    //! \details  Add Vebox Tiling Convert Control Bits
    //! \param    [in] cmdBuffer
    //!           Pointers to the HW Cmd buffer
    //! \param    [in] inSurParams
    //!           Pointer to input vebox surface params
    //! \param    [in] outSurParams
    //!           Pointer to output vebox surface params
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    virtual MOS_STATUS AddVeboxTilingConvert(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMHW_VEBOX_SURFACE_PARAMS        inSurParams,
        PMHW_VEBOX_SURFACE_PARAMS        outSurParams)
    {
        return MOS_STATUS_SUCCESS;
    }

    virtual MOS_STATUS setVeboxPrologCmd(
        PMHW_MI_INTERFACE   mhwMiInterface,
        PMOS_COMMAND_BUFFER cmdBuffer)
    {
        return MOS_STATUS_SUCCESS;
    }

protected:
    //!
    //! \brief      Set Vebox Iecp  STDSTE State
    //! \details    Set STE part of the VEBOX IECP States
    //! \param      [in] pVeboxStdSteState
    //!             Pointer to VEBOX IECP STD/E States
    //! \param      [in] pColorPipeParams
    //!             Pointer to COLOR PIPE Params
    //! \return     void
    //!
    virtual MOS_STATUS  SetVeboxIecpStateSTE(
        typename TVeboxCmds::VEBOX_STD_STE_STATE_CMD     *pVeboxStdSteState,
        PMHW_COLORPIPE_PARAMS                            pColorPipeParams)
    {
        MHW_FUNCTION_ENTER;
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        MHW_CHK_NULL(pVeboxStdSteState);
        MHW_CHK_NULL(pColorPipeParams);

        // STD detects the skin like colors and passes a grade of skin tone
        // color to STE (Skin Tone Enhancement). STD operates in the YUV color
        // space.The level of skin tone detection is determined through skin
        // tone factors in UV plane. If skin tone detection in VY plane is also
        // enabled, the final skin tone factor is given by the minimum of STD
        // in the (U, V) plane and (V, Y) plane.
        //
        // The skin tone factor will also be passed to ACE and TCC to indicate
        // the strength of skin tone likelihood.
        pVeboxStdSteState->DW0.StdEnable = true;

        // Enable skin tone detection in VY plane
        pVeboxStdSteState->DW3.VyStdEnable = true;

        // Enable STE (Skin Tone Enhancement)
        // STE modify the saturation and hue of the pixels which were detected
        // as the skin-tone pixels by STD
        if (pColorPipeParams->bEnableSTE &&
            pColorPipeParams->SteParams.dwSTEFactor > 0)
        {
            pVeboxStdSteState->DW0.SteEnable = true;

            if (pColorPipeParams->SteParams.dwSTEFactor <= MHW_STE_OPTIMAL)
            {
                pVeboxStdSteState->DW15.Satb1 = MOS_BITFIELD_VALUE((uint32_t)-8, 10);
                pVeboxStdSteState->DW15.Satp3 = 31;
                pVeboxStdSteState->DW15.Satp2 = 6;
                pVeboxStdSteState->DW15.Satp1 = pColorPipeParams->SteParams.satP1;

                pVeboxStdSteState->DW16.Sats0 = pColorPipeParams->SteParams.satS0;
                pVeboxStdSteState->DW16.Satb3 = 124;
                pVeboxStdSteState->DW16.Satb2 = 8;

                pVeboxStdSteState->DW17.Sats2 = 297;
                pVeboxStdSteState->DW17.Sats1 = pColorPipeParams->SteParams.satS1;

                pVeboxStdSteState->DW18.Huep3 = 14;
                pVeboxStdSteState->DW18.Huep2 = 6;
                pVeboxStdSteState->DW18.Huep1 = MOS_BITFIELD_VALUE((uint32_t)-6, 7);
                pVeboxStdSteState->DW18.Sats3 = 256;

                pVeboxStdSteState->DW19.Hueb3 = 56;
                pVeboxStdSteState->DW19.Hueb2 = 8;
                pVeboxStdSteState->DW19.Hueb1 = MOS_BITFIELD_VALUE((uint32_t)-8, 10);

                pVeboxStdSteState->DW20.Hues1 = 85;
                pVeboxStdSteState->DW20.Hues0 = 384;

                pVeboxStdSteState->DW21.Hues3 = 256;
                pVeboxStdSteState->DW21.Hues2 = 384;
            }
            else  // if (pColorPipeParams->SteParams.dwSTEFactor > MHW_STE_OPTIMAL)
            {
                pVeboxStdSteState->DW15.Satb1 = 0;
                pVeboxStdSteState->DW15.Satp3 = 31;
                pVeboxStdSteState->DW15.Satp2 = 31;
                pVeboxStdSteState->DW15.Satp1 = pColorPipeParams->SteParams.satP1;

                pVeboxStdSteState->DW16.Sats0 = pColorPipeParams->SteParams.satS0;
                pVeboxStdSteState->DW16.Satb3 = 124;
                pVeboxStdSteState->DW16.Satb2 = 124;

                pVeboxStdSteState->DW17.Sats2 = 256;
                pVeboxStdSteState->DW17.Sats1 = pColorPipeParams->SteParams.satS1;

                pVeboxStdSteState->DW18.Huep3 = 14;
                pVeboxStdSteState->DW18.Huep2 = 14;
                pVeboxStdSteState->DW18.Huep1 = 14;
                pVeboxStdSteState->DW18.Sats3 = 256;

                pVeboxStdSteState->DW19.Hueb3 = 56;
                pVeboxStdSteState->DW19.Hueb2 = 56;
                pVeboxStdSteState->DW19.Hueb1 = 56;

                pVeboxStdSteState->DW20.Hues1 = 256;
                pVeboxStdSteState->DW20.Hues0 = 256;

                pVeboxStdSteState->DW21.Hues3 = 256;
                pVeboxStdSteState->DW21.Hues2 = 256;
            }
        }

        if (pColorPipeParams->bEnableSTD)
        {
            if (nullptr == pColorPipeParams->StdParams.param || pColorPipeParams->StdParams.paraSizeInBytes > pVeboxStdSteState->byteSize)
            {
                MHW_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
            }

            MOS_SecureMemcpy(pVeboxStdSteState, pColorPipeParams->StdParams.paraSizeInBytes, pColorPipeParams->StdParams.param, pColorPipeParams->StdParams.paraSizeInBytes);
        }
    finish:
        return eStatus;
    }

    //!
    //! \brief      Set Vebox Iecp ACE/LACE State
    //! \details    Set VEBOX IECP Ace and Lace States
    //! \param      [in, out] pVeboxAceLaceState
    //!             Pointer to VEBOX IECP ACE/LACE States
    //! \param      [in, out] pVeboxAlphaAoiState
    //!             Pointer to VEBOX IECP ALPHA/AOI States
    //! \param      [in] bEnableLACE
    //!             LACE enabled or not
    //! \return     void
    //!
    void SetVeboxIecpStateACELACE(
        typename TVeboxCmds::VEBOX_ACE_LACE_STATE_CMD    *pVeboxAceLaceState,
        typename TVeboxCmds::VEBOX_ALPHA_AOI_STATE_CMD   *pVeboxAlphaAoiState,
        bool                                             bEnableLACE)
    {
        MHW_FUNCTION_ENTER;
        MHW_CHK_NULL_NO_STATUS_RETURN(pVeboxAceLaceState);
        MHW_CHK_NULL_NO_STATUS_RETURN(pVeboxAlphaAoiState);

        // ACE improves the overall contrast of the image and emphasizing
        // details when relevant (such as in dark areas)
        pVeboxAceLaceState->DW0.AceEnable = true;

        // On SKL FullImageHistogram is set in Alpha/AOI state
        pVeboxAlphaAoiState->DW0.FullImageHistogram = true;

        if (bEnableLACE)
        {
            // Enable Block level histogram generation
            pVeboxAceLaceState->DW0.LaceHistogramEnable = true;
        }
        else
        {
            pVeboxAceLaceState->DW0.LaceHistogramEnable = false;
        }
    }

    //!
    //! \brief      Set VEBOX IECP TCC State
    //! \details    Set Vebox TCC state
    //! \param      [in] pVeboxTccState
    //!             Pointer to VEBOX IECP TCC States
    //! \param      [in] pColorPipeParams
    //!             Pointer to COLOR PIPE Params
    //! \return     void
    //!
    void SetVeboxIecpStateTCC(
        typename TVeboxCmds::VEBOX_TCC_STATE_CMD *pVeboxTccState,
        PMHW_COLORPIPE_PARAMS                    pColorPipeParams)
    {
        MHW_FUNCTION_ENTER;
        MHW_CHK_NULL_NO_STATUS_RETURN(pVeboxTccState);
        MHW_CHK_NULL_NO_STATUS_RETURN(pColorPipeParams);

        // TCC allows users to custom the color scheme by choosing different
        // grades of saturation for each of the six basic colors (red, green,
        // blue, magenta, yellow, and cyan).
        pVeboxTccState->DW0.TccEnable  = true;
        pVeboxTccState->DW0.Satfactor1 = pColorPipeParams->TccParams.Magenta;
        pVeboxTccState->DW0.Satfactor2 = pColorPipeParams->TccParams.Red;
        pVeboxTccState->DW0.Satfactor3 = pColorPipeParams->TccParams.Yellow;
        pVeboxTccState->DW1.Satfactor4 = pColorPipeParams->TccParams.Green;
        pVeboxTccState->DW1.Satfactor5 = pColorPipeParams->TccParams.Cyan;
        pVeboxTccState->DW1.Satfactor6 = pColorPipeParams->TccParams.Blue;
    }

    //!
    //! \brief      Set Vebox Iecp State FECSC
    //! \details    Set Front-End CSC part of the VEBOX IECP States
    //! \param      [in] pVeboxIecpState
    //!             Pointer to VEBOX IECP States
    //! \param      [in] pVeboxIecpParams
    //!             Pointer to VEBOX IECP State Params
    //! \return     void
    //!
    void SetVeboxIecpStateFecsc(
        typename TVeboxCmds::VEBOX_FRONT_END_CSC_STATE_CMD *pVeboxFecscState,
        PMHW_VEBOX_IECP_PARAMS                pVeboxIecpParams)
    {
        MHW_FUNCTION_ENTER;
        PMHW_CAPPIPE_PARAMS pCapPipeParams = nullptr;

        MHW_CHK_NULL_NO_STATUS_RETURN(pVeboxFecscState);
        MHW_CHK_NULL_NO_STATUS_RETURN(pVeboxIecpParams);

        pCapPipeParams = &pVeboxIecpParams->CapPipeParams;
        MHW_CHK_NULL_NO_STATUS_RETURN(pCapPipeParams);
        MHW_ASSERT(pCapPipeParams->bActive);

#define SET_COEFS(_c0, _c1, _c2, _c3, _c4, _c5, _c6, _c7, _c8)   \
    {                                                            \
        pVeboxFecscState->DW0.FecscC0TransformCoefficient = _c0; \
        pVeboxFecscState->DW1.FecscC1TransformCoefficient = _c1; \
        pVeboxFecscState->DW2.FecscC2TransformCoefficient = _c2; \
        pVeboxFecscState->DW3.FecscC3TransformCoefficient = _c3; \
        pVeboxFecscState->DW4.FecscC4TransformCoefficient = _c4; \
        pVeboxFecscState->DW5.FecscC5TransformCoefficient = _c5; \
        pVeboxFecscState->DW6.FecscC6TransformCoefficient = _c6; \
        pVeboxFecscState->DW7.FecscC7TransformCoefficient = _c7; \
        pVeboxFecscState->DW8.FecscC8TransformCoefficient = _c8; \
    }

#define SET_INPUT_OFFSETS(_in1, _in2, _in3)                         \
    {                                                               \
        pVeboxFecscState->DW9.FecScOffsetIn1OffsetInForYR   = _in1; \
        pVeboxFecscState->DW10.FecScOffsetIn2OffsetOutForUG = _in2; \
        pVeboxFecscState->DW11.FecScOffsetIn3OffsetOutForVB = _in3; \
    }

#define SET_OUTPUT_OFFSETS(_out1, _out2, _out3)                       \
    {                                                                 \
        pVeboxFecscState->DW9.FecScOffsetOut1OffsetOutForYR  = _out1; \
        pVeboxFecscState->DW10.FecScOffsetOut2OffsetOutForUG = _out2; \
        pVeboxFecscState->DW11.FecScOffsetOut3OffsetOutForVB = _out3; \
    }

        pVeboxFecscState->DW0.FrontEndCscTransformEnable = true;

        if (pCapPipeParams->FECSCParams.bActive)
        {
            // Coeff is S2.16, so multiply the floating value by 65536
            SET_COEFS(
                ((uint32_t)(pCapPipeParams->FECSCParams.Matrix[0][0] * 65536)),
                ((uint32_t)(pCapPipeParams->FECSCParams.Matrix[0][1] * 65536)),
                ((uint32_t)(pCapPipeParams->FECSCParams.Matrix[0][2] * 65536)),
                ((uint32_t)(pCapPipeParams->FECSCParams.Matrix[1][0] * 65536)),
                ((uint32_t)(pCapPipeParams->FECSCParams.Matrix[1][1] * 65536)),
                ((uint32_t)(pCapPipeParams->FECSCParams.Matrix[1][2] * 65536)),
                ((uint32_t)(pCapPipeParams->FECSCParams.Matrix[2][0] * 65536)),
                ((uint32_t)(pCapPipeParams->FECSCParams.Matrix[2][1] * 65536)),
                ((uint32_t)(pCapPipeParams->FECSCParams.Matrix[2][2] * 65536)));
            SET_INPUT_OFFSETS(
                ((uint32_t)pCapPipeParams->FECSCParams.PreOffset[0]),
                ((uint32_t)pCapPipeParams->FECSCParams.PreOffset[1]),
                ((uint32_t)pCapPipeParams->FECSCParams.PreOffset[2]));
            SET_OUTPUT_OFFSETS(
                ((uint32_t)pCapPipeParams->FECSCParams.PostOffset[0]),
                ((uint32_t)pCapPipeParams->FECSCParams.PostOffset[1]),
                ((uint32_t)pCapPipeParams->FECSCParams.PostOffset[2]));
        }
        else if (pVeboxIecpParams->bFeCSCEnable)
        {
            // Coeff is S2.16, so multiply the floating value by 65536
            SET_COEFS(
                ((uint32_t)(pVeboxIecpParams->pfFeCscCoeff[0] * 65536)),
                ((uint32_t)(pVeboxIecpParams->pfFeCscCoeff[1] * 65536)),
                ((uint32_t)(pVeboxIecpParams->pfFeCscCoeff[2] * 65536)),
                ((uint32_t)(pVeboxIecpParams->pfFeCscCoeff[3] * 65536)),
                ((uint32_t)(pVeboxIecpParams->pfFeCscCoeff[4] * 65536)),
                ((uint32_t)(pVeboxIecpParams->pfFeCscCoeff[5] * 65536)),
                ((uint32_t)(pVeboxIecpParams->pfFeCscCoeff[6] * 65536)),
                ((uint32_t)(pVeboxIecpParams->pfFeCscCoeff[7] * 65536)),
                ((uint32_t)(pVeboxIecpParams->pfFeCscCoeff[8] * 65536)));
            SET_INPUT_OFFSETS(
                ((uint32_t)(pVeboxIecpParams->pfFeCscInOffset[0] * 128.0F)),
                ((uint32_t)(pVeboxIecpParams->pfFeCscInOffset[1] * 128.0F)),
                ((uint32_t)(pVeboxIecpParams->pfFeCscInOffset[2] * 128.0F)));
            SET_OUTPUT_OFFSETS(
                ((uint32_t)(pVeboxIecpParams->pfFeCscOutOffset[0] * 128.0F)),
                ((uint32_t)(pVeboxIecpParams->pfFeCscOutOffset[1] * 128.0F)),
                ((uint32_t)(pVeboxIecpParams->pfFeCscOutOffset[2] * 128.0F)));
        }
        else if (pVeboxIecpParams->ColorSpace == MHW_CSpace_BT601)
        {
            SET_COEFS(16843, 33030, 6423, MOS_BITFIELD_VALUE((uint32_t)-9698, 19), MOS_BITFIELD_VALUE((uint32_t)-19070, 19), 28770, 28770, MOS_BITFIELD_VALUE((uint32_t)-24116, 19), MOS_BITFIELD_VALUE((uint32_t)-4652, 19));
            SET_INPUT_OFFSETS(0, 0, 0);
            SET_OUTPUT_OFFSETS(2048, 16384, 16384);
        }
        else if (pVeboxIecpParams->ColorSpace == MHW_CSpace_BT709)
        {
            SET_COEFS(11993, 40239, 4063, MOS_BITFIELD_VALUE((uint32_t)-6618, 19), MOS_BITFIELD_VALUE((uint32_t)-22216, 19), 28770, 28770, MOS_BITFIELD_VALUE((uint32_t)-26148, 19), MOS_BITFIELD_VALUE((uint32_t)-2620, 19));
            SET_INPUT_OFFSETS(0, 0, 0);
            SET_OUTPUT_OFFSETS(2048, 16384, 16384);
        }
        else if (pVeboxIecpParams->ColorSpace == MHW_CSpace_sRGB)
        {
            SET_COEFS(13932, 46871, 4731, MOS_BITFIELD_VALUE((uint32_t)-7508, 19), MOS_BITFIELD_VALUE((uint32_t)-25260, 19), 32768, 32768, MOS_BITFIELD_VALUE((uint32_t)-29764, 19), MOS_BITFIELD_VALUE((uint32_t)-3005, 19));
            SET_INPUT_OFFSETS(0, 0, 0);
            SET_OUTPUT_OFFSETS(0, 16384, 16384);
        }
        else
        {
            MHW_ASSERT(0);
        }

#undef SET_COEFS
#undef SET_INPUT_OFFSETS
#undef SET_OUTPUT_OFFSETS
    }

    //!
    //! \brief      Set Vebox Iecp State Color Correction Matrix
    //! \details    Set Color Correction Matrix of the VEBOX IECP States
    //! \param      [in] pVeboxIecpState
    //!             Pointer to VEBOX IECP States
    //! \param      [in] pCapPipeParams
    //!             Pointer to Capture Pipe Params
    //! \param      [in] const unsigned int uCoeffValue
    //!             the coeffs's miltiply floating value.
    //!                  Coeff is S4.12, then multiply the floating value by 4096
    //!                  Coeff is S4.16, then multiply the floating value by 65536
    //! \return     void
    //!
    void  SetVeboxIecpStateCcm(
        typename TVeboxCmds::VEBOX_IECP_STATE_CMD* pVeboxIecpState,
        PMHW_CAPPIPE_PARAMS                        pCapPipeParams,
        const unsigned int                         uCoeffValue)
    {
        MHW_FUNCTION_ENTER;
        typename TVeboxCmds::VEBOX_CCM_STATE_CMD *pCcm = nullptr;

        MHW_CHK_NULL_NO_STATUS_RETURN(pVeboxIecpState);
        MHW_CHK_NULL_NO_STATUS_RETURN(pCapPipeParams);

        pCcm = &pVeboxIecpState->CcmState;

        if (pCapPipeParams->ColorCorrectionParams.bActive)
        {
            MHW_CHK_NULL_NO_STATUS_RETURN(pCcm);
            pCcm->DW0.ColorCorrectionMatrixEnable = true;

            pCcm->DW0.C1 =
                (uint32_t)(pCapPipeParams->ColorCorrectionParams.CCM[0][1] * uCoeffValue);
            pCcm->DW1.C0 =
                (uint32_t)(pCapPipeParams->ColorCorrectionParams.CCM[0][0] * uCoeffValue);
            pCcm->DW2.C3 =
                (uint32_t)(pCapPipeParams->ColorCorrectionParams.CCM[1][0] * uCoeffValue);
            pCcm->DW3.C2 =
                (uint32_t)(pCapPipeParams->ColorCorrectionParams.CCM[0][2] * uCoeffValue);
            pCcm->DW4.C5 =
                (uint32_t)(pCapPipeParams->ColorCorrectionParams.CCM[1][2] * uCoeffValue);
            pCcm->DW5.C4 =
                (uint32_t)(pCapPipeParams->ColorCorrectionParams.CCM[1][1] * uCoeffValue);
            pCcm->DW6.C7 =
                (uint32_t)(pCapPipeParams->ColorCorrectionParams.CCM[2][1] * uCoeffValue);
            pCcm->DW7.C6 =
                (uint32_t)(pCapPipeParams->ColorCorrectionParams.CCM[2][0] * uCoeffValue);
            pCcm->DW8.C8 =
                (uint32_t)(pCapPipeParams->ColorCorrectionParams.CCM[2][2] * uCoeffValue);
        }
    }

    //!
    //! \brief      Set Vebox Iecp State ProcAmp 
    //! \details    Set ProcAmp part of the VEBOX IECP States
    //! \param      [in] pVeboxProcampState
    //!             Pointer to VEBOX IECP Procamp States
    //! \param      [in] pProcAmpParams
    //!             Pointer to ProcAmp Params
    //! \return     void
    //!
    void SetVeboxIecpStateProcAmp(
        typename TVeboxCmds::VEBOX_PROCAMP_STATE_CMD *pVeboxProcampState,
        PMHW_PROCAMP_PARAMS                           pProcAmpParams)
    {
        MHW_FUNCTION_ENTER;
        MHW_CHK_NULL_NO_STATUS_RETURN(pVeboxProcampState);
        MHW_CHK_NULL_NO_STATUS_RETURN(pProcAmpParams);

        pVeboxProcampState->DW0.ProcampEnable = true;
        pVeboxProcampState->DW0.Brightness    = pProcAmpParams->brightness;  // S7.4
        pVeboxProcampState->DW0.Contrast      = pProcAmpParams->contrast;    // U4.7
        pVeboxProcampState->DW1.SinCS         = pProcAmpParams->sinCS;       // S7.8
        pVeboxProcampState->DW1.CosCS         = pProcAmpParams->cosCS;       // S7.8
    }

    //!
    //! \brief      Add VEBOX ACE/LACE States
    //! \details    Add Vebox ACE, Lace states
    //! \param      [in] pVeboxIecpParams
    //!             Pointer to VEBOX IECP State Params
    //! \return     voild
    //!
    void SetVeboxAceLaceState(
        PMHW_VEBOX_IECP_PARAMS                    pVeboxIecpParams,
        typename TVeboxCmds::VEBOX_IECP_STATE_CMD *pVeboxIecpState)
    {
        MHW_FUNCTION_ENTER;
        PMHW_ACE_PARAMS             pAceParams;
        PMHW_VEBOX_HEAP             pVeboxHeap;
        int32_t                     uiOffset;
        MOS_STATUS                  eStatus = MOS_STATUS_SUCCESS;

        MHW_CHK_NULL_NO_STATUS_RETURN(pVeboxIecpParams);

        if (pVeboxIecpParams->ColorPipeParams.bActive &&
            pVeboxIecpParams->ColorPipeParams.bEnableACE)
        {
            MHW_CHK_NULL_NO_STATUS_RETURN(pVeboxIecpState);
            pAceParams = &pVeboxIecpParams->AceParams;
            MHW_ASSERT(pAceParams->bActive);

            // Y is input, use wPWLF_X, Y4 as Max. Populate Y5-Ymax with max Y to
            // avoid artifact
            pVeboxIecpState->AceState.DW1.Ymin = pAceParams->wACEPWLF_X[0] >> 2;
            pVeboxIecpState->AceState.DW1.Y1 = pAceParams->wACEPWLF_X[1] >> 2;
            pVeboxIecpState->AceState.DW1.Y2 = pAceParams->wACEPWLF_X[2] >> 2;
            pVeboxIecpState->AceState.DW1.Y3 = pAceParams->wACEPWLF_X[3] >> 2;
            pVeboxIecpState->AceState.DW2.Y4 =
                pVeboxIecpState->AceState.DW2.Y5 =
                pVeboxIecpState->AceState.DW2.Y6 =
                pVeboxIecpState->AceState.DW2.Y7 =
                pVeboxIecpState->AceState.DW3.Y8 =
                pVeboxIecpState->AceState.DW3.Y9 =
                pVeboxIecpState->AceState.DW3.Y10 =
                pVeboxIecpState->AceState.DW3.Ymax = pAceParams->wACEPWLF_X[4] >> 2;

            // the bias is the output, therefore the Y in algorithm
            pVeboxIecpState->AceState.DW4.B1 = pAceParams->wACEPWLF_Y[1] >> 2;
            pVeboxIecpState->AceState.DW4.B2 = pAceParams->wACEPWLF_Y[2] >> 2;
            pVeboxIecpState->AceState.DW4.B3 = pAceParams->wACEPWLF_Y[3] >> 2;

            // need to double check if slope is set incorrectly
            pVeboxIecpState->AceState.DW7.S0 = pAceParams->wACEPWLF_S[0];
            pVeboxIecpState->AceState.DW7.S1 = pAceParams->wACEPWLF_S[1];
            pVeboxIecpState->AceState.DW8.S2 = pAceParams->wACEPWLF_S[2];
            pVeboxIecpState->AceState.DW8.S3 = pAceParams->wACEPWLF_S[3];
        }
    }

    //!
    //! \brief      Add VEBOX Capture pipe state
    //! \details    Add VEBOX Capture pipe state
    //! \param      [in] pCapPipeParams
    //!             Pointer to capture pipe parameters
    //! \return     void
    //!
    void AddVeboxCapPipeState(
        PMHW_CAPPIPE_PARAMS pCapPipeParams)
    {
        MHW_FUNCTION_ENTER;
        typename TVeboxCmds::VEBOX_CAPTURE_PIPE_STATE_CMD *pVeboxCapPipeState, CapPipCmd;

        PMHW_VEBOX_HEAP pVeboxHeap;
        uint32_t        uiOffset;

        MHW_CHK_NULL_NO_STATUS_RETURN(pCapPipeParams);
        MHW_CHK_NULL_NO_STATUS_RETURN(m_veboxHeap);

        pVeboxHeap = m_veboxHeap;
        uiOffset   = pVeboxHeap->uiCurState * pVeboxHeap->uiInstanceSize;

        pVeboxCapPipeState =
            (typename TVeboxCmds::VEBOX_CAPTURE_PIPE_STATE_CMD *)(pVeboxHeap->pLockedDriverResourceMem +
                                                                  pVeboxHeap->uiCapturePipeStateOffset +
                                                                  uiOffset);
        MHW_CHK_NULL_NO_STATUS_RETURN(pVeboxCapPipeState);
        *pVeboxCapPipeState = CapPipCmd;

        if (pCapPipeParams->BlackLevelParams.bActive)
        {
            pVeboxCapPipeState->DW2.BlackPointCorrectionEnable = true;
            // Red
            pVeboxCapPipeState->DW2.BlackPointOffsetRedMsb =
                (pCapPipeParams->BlackLevelParams.R & MOS_BITFIELD_BIT_N(16)) >> 16;
            pVeboxCapPipeState->DW3.BlackPointOffsetRed =
                pCapPipeParams->BlackLevelParams.R & MOS_MASK(0, 15);
            // Green Top
            pVeboxCapPipeState->DW2.BlackPointOffsetGreenTopMsb =
                (pCapPipeParams->BlackLevelParams.G1 & MOS_BITFIELD_BIT_N(16)) >> 16;
            pVeboxCapPipeState->DW3.BlackPointOffsetGreenTop =
                pCapPipeParams->BlackLevelParams.G1 & MOS_MASK(0, 15);
            // Green Bottom
            pVeboxCapPipeState->DW2.BlackPointOffsetGreenBottomMsb =
                (pCapPipeParams->BlackLevelParams.G0 & MOS_BITFIELD_BIT_N(16)) >> 16;
            pVeboxCapPipeState->DW4.BlackPointOffsetGreenBottom =
                pCapPipeParams->BlackLevelParams.G0 & MOS_MASK(0, 15);
            // Blue
            pVeboxCapPipeState->DW2.BlackPointOffsetBlueMsb =
                (pCapPipeParams->BlackLevelParams.B & MOS_BITFIELD_BIT_N(16)) >> 16;
            pVeboxCapPipeState->DW4.BlackPointOffsetBlue =
                pCapPipeParams->BlackLevelParams.B & MOS_MASK(0, 15);
        }

        if (pCapPipeParams->WhiteBalanceParams.bActive &&
            pCapPipeParams->WhiteBalanceParams.Mode == MHW_WB_MANUAL)
        {
            pVeboxCapPipeState->DW2.WhiteBalanceCorrectionEnable = true;

            // Is U4.12, so multiply the floating value by 4096
            // Red
            pVeboxCapPipeState->DW5.WhiteBalanceRedCorrection =
                (uint32_t)(pCapPipeParams->WhiteBalanceParams.RedCorrection * 4096);

            // Greep Top
            pVeboxCapPipeState->DW5.WhiteBalanceGreenTopCorrection =
                (uint32_t)(pCapPipeParams->WhiteBalanceParams.GreenTopCorrection * 4096);

            // Green Bottom
            pVeboxCapPipeState->DW6.WhiteBalanceGreenBottomCorrection =
                (uint32_t)(pCapPipeParams->WhiteBalanceParams.GreenBottomCorrection * 4096);

            // Blue
            pVeboxCapPipeState->DW6.WhiteBalanceBlueCorrection =
                (uint32_t)(pCapPipeParams->WhiteBalanceParams.BlueCorrection * 4096);
        }
    }

    //!
    //! \brief      Set Vebox Surface State
    //! \details    Set Vebox surface state
    //! \param      [in] pSurfaceParam
    //!             Pointer to surface
    //! \param      [in] pDerivedSurfaceParam
    //!             Pointer to Derived surface
    //! \param      [in] pSkinScoreSurfaceParam
    //!             Pointer to Skin Score output surface
    //! \param      [in] pVeboxSurfaceState
    //!             Pointer to VEBOX surface state cmd
    //! \param      [in] bIsOutputSurface
    //!             Indicates whether surface is input or output
    //! \param      [in] bDIEnable
    //!             True if DI is enabled
    //! \return     void
    //!
    virtual void SetVeboxSurfaces(
        PMHW_VEBOX_SURFACE_PARAMS                     pSurfaceParam,
        PMHW_VEBOX_SURFACE_PARAMS                     pDerivedSurfaceParam,
        PMHW_VEBOX_SURFACE_PARAMS                     pSkinScoreSurfaceParam,
        typename TVeboxCmds::VEBOX_SURFACE_STATE_CMD *pVeboxSurfaceState,
        bool                                          bIsOutputSurface,
        bool                                          bDIEnable) = 0;

private:
    //!
    //! Vertext Table for BT601
    //!
    const unsigned short g_VeboxVertexTableBT601[512][2] =
    {
        //{  Cv,   Lv}
        { 1936, 666 },{ 1932, 673 },{ 1928, 680 },{ 1924, 687 },
        { 1920, 694 },{ 1917, 701 },{ 1913, 708 },{ 1910, 715 },
        { 1906, 722 },{ 1903, 729 },{ 1900, 736 },{ 1897, 743 },
        { 1894, 749 },{ 1891, 756 },{ 1888, 763 },{ 1885, 770 },
        { 1882, 776 },{ 1880, 783 },{ 1877, 790 },{ 1875, 796 },
        { 1872, 803 },{ 1870, 810 },{ 1868, 816 },{ 1866, 823 },
        { 1864, 830 },{ 1862, 836 },{ 1860, 843 },{ 1858, 849 },
        { 1856, 856 },{ 1854, 862 },{ 1853, 869 },{ 1851, 875 },
        { 1849, 882 },{ 1848, 888 },{ 1847, 895 },{ 1845, 901 },
        { 1844, 908 },{ 1843, 914 },{ 1842, 921 },{ 1841, 927 },
        { 1840, 933 },{ 1839, 940 },{ 1838, 946 },{ 1838, 953 },
        { 1837, 959 },{ 1836, 965 },{ 1836, 972 },{ 1835, 978 },
        { 1835, 985 },{ 1835, 991 },{ 1834, 997 },{ 1834, 1004 },
        { 1834, 1010 },{ 1834, 1017 },{ 1834, 1023 },{ 1834, 1029 },
        { 1834, 1036 },{ 1835, 1042 },{ 1835, 1048 },{ 1835, 1055 },
        { 1836, 1061 },{ 1836, 1068 },{ 1837, 1074 },{ 1837, 1080 },
        { 1838, 1087 },{ 1839, 1093 },{ 1840, 1100 },{ 1841, 1106 },
        { 1842, 1112 },{ 1843, 1119 },{ 1844, 1125 },{ 1845, 1132 },
        { 1847, 1138 },{ 1848, 1145 },{ 1849, 1151 },{ 1851, 1158 },
        { 1852, 1164 },{ 1854, 1171 },{ 1856, 1177 },{ 1858, 1184 },
        { 1859, 1190 },{ 1861, 1197 },{ 1863, 1203 },{ 1866, 1210 },
        { 1868, 1217 },{ 1870, 1223 },{ 1872, 1230 },{ 1875, 1236 },
        { 1877, 1243 },{ 1880, 1250 },{ 1882, 1257 },{ 1885, 1263 },
        { 1888, 1270 },{ 1891, 1277 },{ 1894, 1284 },{ 1897, 1290 },
        { 1900, 1297 },{ 1903, 1304 },{ 1906, 1311 },{ 1910, 1318 },
        { 1913, 1325 },{ 1916, 1332 },{ 1920, 1339 },{ 1924, 1346 },
        { 1928, 1353 },{ 1931, 1360 },{ 1935, 1367 },{ 1939, 1374 },
        { 1944, 1381 },{ 1948, 1388 },{ 1952, 1396 },{ 1957, 1403 },
        { 1961, 1410 },{ 1966, 1417 },{ 1970, 1425 },{ 1975, 1432 },
        { 1980, 1439 },{ 1985, 1447 },{ 1990, 1454 },{ 1995, 1462 },
        { 2001, 1470 },{ 2006, 1477 },{ 2011, 1485 },{ 2017, 1492 },
        { 2023, 1500 },{ 2029, 1508 },{ 2035, 1516 },{ 2041, 1524 },
        { 2047, 1532 },{ 2053, 1540 },{ 2059, 1548 },{ 2066, 1556 },
        { 2073, 1564 },{ 2079, 1572 },{ 2086, 1580 },{ 2093, 1588 },
        { 2100, 1597 },{ 2108, 1605 },{ 2115, 1614 },{ 2123, 1622 },
        { 2130, 1631 },{ 2138, 1639 },{ 2146, 1648 },{ 2154, 1657 },
        { 2162, 1666 },{ 2171, 1674 },{ 2179, 1683 },{ 2186, 1691 },
        { 2179, 1688 },{ 2172, 1684 },{ 2164, 1681 },{ 2157, 1678 },
        { 2151, 1674 },{ 2144, 1671 },{ 2137, 1668 },{ 2131, 1664 },
        { 2124, 1661 },{ 2118, 1658 },{ 2112, 1655 },{ 2106, 1651 },
        { 2100, 1648 },{ 2094, 1645 },{ 2088, 1642 },{ 2083, 1639 },
        { 2077, 1636 },{ 2072, 1633 },{ 2067, 1629 },{ 2061, 1626 },
        { 2056, 1623 },{ 2051, 1620 },{ 2046, 1617 },{ 2042, 1614 },
        { 2037, 1611 },{ 2032, 1608 },{ 2028, 1605 },{ 2024, 1602 },
        { 2019, 1599 },{ 2015, 1596 },{ 2011, 1593 },{ 2007, 1590 },
        { 2003, 1588 },{ 1999, 1585 },{ 1996, 1582 },{ 1992, 1579 },
        { 1988, 1576 },{ 1985, 1573 },{ 1981, 1570 },{ 1978, 1568 },
        { 1975, 1565 },{ 1972, 1562 },{ 1969, 1559 },{ 1966, 1556 },
        { 1963, 1554 },{ 1960, 1551 },{ 1957, 1548 },{ 1955, 1545 },
        { 1952, 1542 },{ 1950, 1540 },{ 1947, 1537 },{ 1945, 1534 },
        { 1943, 1531 },{ 1941, 1529 },{ 1939, 1526 },{ 1937, 1523 },
        { 1935, 1521 },{ 1933, 1518 },{ 1931, 1515 },{ 1929, 1513 },
        { 1928, 1510 },{ 1926, 1507 },{ 1925, 1504 },{ 1923, 1502 },
        { 1922, 1499 },{ 1921, 1496 },{ 1919, 1494 },{ 1918, 1491 },
        { 1917, 1488 },{ 1916, 1486 },{ 1916, 1483 },{ 1915, 1481 },
        { 1914, 1478 },{ 1913, 1475 },{ 1913, 1473 },{ 1912, 1470 },
        { 1912, 1467 },{ 1911, 1465 },{ 1911, 1462 },{ 1911, 1459 },
        { 1911, 1457 },{ 1911, 1454 },{ 1911, 1451 },{ 1911, 1449 },
        { 1911, 1446 },{ 1911, 1444 },{ 1911, 1441 },{ 1912, 1438 },
        { 1912, 1436 },{ 1912, 1433 },{ 1913, 1430 },{ 1914, 1428 },
        { 1914, 1425 },{ 1915, 1422 },{ 1916, 1420 },{ 1917, 1417 },
        { 1918, 1414 },{ 1919, 1412 },{ 1920, 1409 },{ 1921, 1406 },
        { 1923, 1404 },{ 1924, 1401 },{ 1925, 1398 },{ 1927, 1396 },
        { 1929, 1393 },{ 1930, 1390 },{ 1932, 1388 },{ 1934, 1385 },
        { 1936, 1382 },{ 1938, 1380 },{ 1940, 1377 },{ 1942, 1374 },
        { 1944, 1371 },{ 1946, 1369 },{ 1949, 1366 },{ 1951, 1363 },
        { 1954, 1360 },{ 1956, 1358 },{ 1959, 1355 },{ 1962, 1352 },
        { 1964, 1349 },{ 1967, 1347 },{ 1970, 1344 },{ 1973, 1341 },
        { 1977, 1338 },{ 1980, 1335 },{ 1983, 1332 },{ 1987, 1330 },
        { 1990, 1327 },{ 1994, 1324 },{ 1998, 1321 },{ 2001, 1318 },
        { 2005, 1315 },{ 2009, 1312 },{ 2013, 1309 },{ 2017, 1306 },
        { 2022, 1303 },{ 2026, 1300 },{ 2030, 1298 },{ 2035, 1295 },
        { 2040, 1292 },{ 2044, 1289 },{ 2049, 1285 },{ 2054, 1282 },
        { 2059, 1279 },{ 2064, 1276 },{ 2069, 1273 },{ 2075, 1270 },
        { 2080, 1267 },{ 2086, 1264 },{ 2091, 1261 },{ 2097, 1258 },
        { 2103, 1254 },{ 2109, 1251 },{ 2115, 1248 },{ 2121, 1245 },
        { 2128, 1241 },{ 2134, 1238 },{ 2141, 1235 },{ 2147, 1232 },
        { 2154, 1228 },{ 2161, 1225 },{ 2153, 1241 },{ 2145, 1259 },
        { 2137, 1276 },{ 2128, 1293 },{ 2120, 1309 },{ 2113, 1326 },
        { 2105, 1343 },{ 2097, 1359 },{ 2090, 1375 },{ 2083, 1392 },
        { 2075, 1408 },{ 2068, 1424 },{ 2062, 1440 },{ 2055, 1455 },
        { 2048, 1471 },{ 2042, 1486 },{ 2035, 1502 },{ 2029, 1517 },
        { 2023, 1533 },{ 2016, 1548 },{ 2011, 1563 },{ 2005, 1578 },
        { 1999, 1593 },{ 1993, 1608 },{ 1988, 1622 },{ 1982, 1637 },
        { 1977, 1652 },{ 1972, 1666 },{ 1967, 1681 },{ 1962, 1695 },
        { 1957, 1709 },{ 1952, 1723 },{ 1947, 1738 },{ 1942, 1752 },
        { 1938, 1766 },{ 1933, 1780 },{ 1929, 1793 },{ 1925, 1807 },
        { 1921, 1821 },{ 1917, 1835 },{ 1913, 1848 },{ 1909, 1862 },
        { 1905, 1875 },{ 1901, 1889 },{ 1897, 1902 },{ 1894, 1916 },
        { 1890, 1929 },{ 1887, 1942 },{ 1884, 1956 },{ 1881, 1969 },
        { 1877, 1982 },{ 1874, 1995 },{ 1871, 2008 },{ 1868, 2021 },
        { 1866, 2034 },{ 1863, 2047 },{ 1860, 2060 },{ 1858, 2073 },
        { 1855, 2086 },{ 1853, 2098 },{ 1850, 2111 },{ 1848, 2124 },
        { 1846, 2136 },{ 1844, 2149 },{ 1842, 2162 },{ 1840, 2174 },
        { 1838, 2187 },{ 1836, 2200 },{ 1834, 2212 },{ 1832, 2225 },
        { 1831, 2237 },{ 1829, 2250 },{ 1828, 2262 },{ 1826, 2274 },
        { 1825, 2287 },{ 1824, 2299 },{ 1822, 2312 },{ 1821, 2324 },
        { 1820, 2336 },{ 1819, 2349 },{ 1818, 2361 },{ 1818, 2373 },
        { 1817, 2385 },{ 1816, 2398 },{ 1815, 2410 },{ 1815, 2422 },
        { 1814, 2435 },{ 1814, 2447 },{ 1813, 2459 },{ 1813, 2471 },
        { 1813, 2483 },{ 1813, 2496 },{ 1813, 2508 },{ 1813, 2520 },
        { 1813, 2532 },{ 1813, 2545 },{ 1813, 2557 },{ 1813, 2569 },
        { 1814, 2581 },{ 1814, 2594 },{ 1814, 2606 },{ 1815, 2618 },
        { 1816, 2630 },{ 1816, 2643 },{ 1817, 2655 },{ 1818, 2667 },
        { 1819, 2679 },{ 1819, 2692 },{ 1820, 2704 },{ 1822, 2716 },
        { 1823, 2729 },{ 1824, 2741 },{ 1825, 2754 },{ 1827, 2766 },
        { 1828, 2778 },{ 1829, 2791 },{ 1831, 2803 },{ 1833, 2816 },
        { 1834, 2828 },{ 1836, 2841 },{ 1838, 2853 },{ 1840, 2866 },
        { 1842, 2879 },{ 1844, 2891 },{ 1846, 2904 },{ 1848, 2917 },
        { 1851, 2929 },{ 1853, 2942 },{ 1856, 2955 },{ 1858, 2968 },
        { 1861, 2981 },{ 1863, 2994 },{ 1866, 3006 },{ 1869, 3019 },
        { 1872, 3032 },{ 1875, 3046 },{ 1878, 3059 },{ 1881, 3072 },
        { 1884, 3085 },{ 1888, 3098 },{ 1891, 3111 },{ 1895, 3125 },
        { 1898, 3138 },{ 1902, 3152 },{ 1906, 3165 },{ 1909, 3179 },
        { 1913, 3192 },{ 1917, 3206 },{ 1921, 3220 },{ 1926, 3233 },
        { 1930, 3247 },{ 1934, 3261 },{ 1939, 3275 },{ 1943, 3289 },
        { 1948, 3303 },{ 1953, 3317 },{ 1958, 3332 },{ 1962, 3346 },
        { 1968, 3360 },{ 1973, 3375 },{ 1978, 3389 },{ 1983, 3404 },
        { 1989, 3418 },{ 1994, 3433 },{ 2000, 3448 },{ 2006, 3463 },
        { 2012, 3478 },{ 2018, 3493 },{ 2024, 3508 },{ 2030, 3524 },
        { 2036, 3539 },{ 2043, 3554 },{ 2049, 3570 },{ 2056, 3586 },
        { 2063, 3601 },{ 2070, 3617 },{ 2073, 3627 },{ 2066, 3619 },
        { 2060, 3611 },{ 2054, 3603 },{ 2047, 3595 },{ 2041, 3587 },
        { 2035, 3579 },{ 2029, 3571 },{ 2023, 3563 },{ 2017, 3556 },
        { 2012, 3548 },{ 2006, 3540 },{ 2001, 3533 },{ 1996, 3525 },
        { 1990, 3517 },{ 1985, 3510 },{ 1980, 3503 },{ 1975, 3495 },
        { 1971, 3488 },{ 1966, 3480 },{ 1961, 3473 },{ 1957, 3466 },
        { 1952, 3459 },{ 1948, 3451 },{ 1944, 3444 },{ 1940, 3437 }
    };

    //!
    //! Vertex Table
    //!
    const unsigned short g_VeboxVertexTableBT709[512][2] =
    {
        //{  Cv,   Lv}
        { 2005, 375 },{ 2002, 381 },{ 2000, 386 },{ 1997, 391 },
        { 1994, 396 },{ 1992, 401 },{ 1990, 407 },{ 1987, 412 },
        { 1985, 417 },{ 1983, 422 },{ 1981, 427 },{ 1979, 432 },
        { 1977, 437 },{ 1975, 442 },{ 1973, 447 },{ 1972, 453 },
        { 1970, 458 },{ 1969, 463 },{ 1967, 468 },{ 1966, 473 },
        { 1965, 478 },{ 1963, 483 },{ 1962, 488 },{ 1961, 493 },
        { 1960, 498 },{ 1959, 503 },{ 1959, 508 },{ 1958, 513 },
        { 1957, 518 },{ 1957, 523 },{ 1956, 528 },{ 1956, 533 },
        { 1955, 538 },{ 1955, 543 },{ 1955, 548 },{ 1954, 553 },
        { 1954, 558 },{ 1954, 563 },{ 1954, 568 },{ 1955, 572 },
        { 1955, 577 },{ 1955, 582 },{ 1955, 587 },{ 1956, 592 },
        { 1956, 597 },{ 1957, 602 },{ 1957, 607 },{ 1958, 612 },
        { 1959, 617 },{ 1960, 622 },{ 1961, 627 },{ 1962, 632 },
        { 1963, 637 },{ 1964, 642 },{ 1965, 647 },{ 1967, 652 },
        { 1968, 657 },{ 1970, 662 },{ 1971, 668 },{ 1973, 673 },
        { 1974, 678 },{ 1976, 683 },{ 1978, 688 },{ 1980, 693 },
        { 1982, 698 },{ 1984, 703 },{ 1986, 708 },{ 1989, 713 },
        { 1991, 719 },{ 1993, 724 },{ 1996, 729 },{ 1998, 734 },
        { 2001, 739 },{ 2004, 744 },{ 2007, 750 },{ 2010, 755 },
        { 2012, 760 },{ 2016, 766 },{ 2019, 771 },{ 2022, 776 },
        { 2025, 781 },{ 2029, 787 },{ 2032, 792 },{ 2036, 798 },
        { 2040, 803 },{ 2043, 808 },{ 2047, 814 },{ 2051, 819 },
        { 2055, 825 },{ 2059, 830 },{ 2064, 836 },{ 2068, 841 },
        { 2072, 847 },{ 2077, 853 },{ 2082, 858 },{ 2086, 864 },
        { 2091, 869 },{ 2096, 875 },{ 2101, 881 },{ 2106, 887 },
        { 2112, 892 },{ 2117, 898 },{ 2122, 904 },{ 2128, 910 },
        { 2134, 916 },{ 2139, 922 },{ 2145, 928 },{ 2151, 934 },
        { 2157, 940 },{ 2164, 946 },{ 2170, 952 },{ 2177, 958 },
        { 2183, 964 },{ 2190, 971 },{ 2197, 977 },{ 2204, 983 },
        { 2211, 989 },{ 2218, 996 },{ 2225, 1002 },{ 2233, 1009 },
        { 2241, 1015 },{ 2248, 1022 },{ 2256, 1028 },{ 2264, 1035 },
        { 2272, 1042 },{ 2281, 1049 },{ 2289, 1055 },{ 2298, 1062 },
        { 2307, 1069 },{ 2316, 1076 },{ 2325, 1083 },{ 2334, 1090 },
        { 2344, 1097 },{ 2353, 1104 },{ 2363, 1112 },{ 2373, 1119 },
        { 2383, 1126 },{ 2393, 1134 },{ 2404, 1141 },{ 2414, 1149 },
        { 2425, 1156 },{ 2436, 1164 },{ 2433, 1165 },{ 2423, 1162 },
        { 2412, 1160 },{ 2402, 1157 },{ 2393, 1154 },{ 2383, 1152 },
        { 2374, 1149 },{ 2364, 1147 },{ 2355, 1145 },{ 2346, 1142 },
        { 2337, 1140 },{ 2329, 1137 },{ 2320, 1135 },{ 2312, 1132 },
        { 2304, 1130 },{ 2296, 1128 },{ 2288, 1125 },{ 2280, 1123 },
        { 2272, 1121 },{ 2265, 1119 },{ 2257, 1116 },{ 2250, 1114 },
        { 2243, 1112 },{ 2236, 1110 },{ 2229, 1107 },{ 2223, 1105 },
        { 2216, 1103 },{ 2209, 1101 },{ 2203, 1099 },{ 2197, 1097 },
        { 2191, 1095 },{ 2185, 1092 },{ 2179, 1090 },{ 2173, 1088 },
        { 2167, 1086 },{ 2162, 1084 },{ 2156, 1082 },{ 2151, 1080 },
        { 2146, 1078 },{ 2141, 1076 },{ 2136, 1074 },{ 2131, 1072 },
        { 2126, 1070 },{ 2121, 1068 },{ 2117, 1066 },{ 2112, 1064 },
        { 2108, 1062 },{ 2103, 1060 },{ 2099, 1058 },{ 2095, 1056 },
        { 2091, 1054 },{ 2087, 1052 },{ 2083, 1050 },{ 2079, 1048 },
        { 2076, 1046 },{ 2072, 1045 },{ 2069, 1043 },{ 2065, 1041 },
        { 2062, 1039 },{ 2059, 1037 },{ 2056, 1035 },{ 2053, 1033 },
        { 2050, 1031 },{ 2047, 1030 },{ 2044, 1028 },{ 2041, 1026 },
        { 2039, 1024 },{ 2036, 1022 },{ 2034, 1020 },{ 2031, 1019 },
        { 2029, 1017 },{ 2027, 1015 },{ 2025, 1013 },{ 2023, 1011 },
        { 2021, 1009 },{ 2019, 1008 },{ 2017, 1006 },{ 2015, 1004 },
        { 2014, 1002 },{ 2012, 1001 },{ 2010, 999 },{ 2009, 997 },
        { 2008, 995 },{ 2006, 993 },{ 2005, 992 }, { 2004, 990  },
        { 2003, 988 },{ 2002, 986 },{ 2001, 984 }, { 2000, 983  },
        { 2000, 981 },{ 1999, 979 },{ 1999, 977 },{ 1998, 976  },
        { 1998, 974 },{ 1997, 972 },{ 1997, 970 },{ 1997, 969  },
        { 1997, 967 },{ 1997, 965 },{ 1997, 963 },{ 1997, 962  },
        { 1997, 960 },{ 1997, 958 },{ 1998, 956 },{ 1998, 955  },
        { 1998, 953 },{ 1999, 951 },{ 2000, 949 },{ 2000, 947  },
        { 2001, 946 },{ 2002, 944 },{ 2003, 942 },{ 2004, 940  },
        { 2005, 939 },{ 2006, 937 },{ 2007, 935 },{ 2009, 933  },
        { 2010, 931 },{ 2012, 930 },{ 2013, 928 },{ 2015, 926  },
        { 2016, 924 },{ 2018, 923 },{ 2020, 921 },{ 2022, 919  },
        { 2024, 917 },{ 2026, 915 },{ 2028, 913 },{ 2031, 912  },
        { 2033, 910 },{ 2035, 908 },{ 2038, 906 },{ 2041, 904  },
        { 2043, 902 },{ 2046, 901 },{ 2049, 899 },{ 2052, 897  },
        { 2055, 895 },{ 2058, 893 },{ 2061, 891 },{ 2064, 889  },
        { 2068, 888 },{ 2071, 886 },{ 2075, 884 },{ 2078, 882  },
        { 2082, 880 },{ 2086, 878 },{ 2090, 876 },{ 2094, 874  },
        { 2098, 872 },{ 2098, 876 },{ 2089, 895 },{ 2080, 914  },
        { 2070, 933 },{ 2061, 952 },{ 2052, 970 },{ 2044, 989  },
        { 2035, 1007 },{ 2027, 1025 },{ 2019, 1043 },{ 2010, 1061 },
        { 2002, 1078 },{ 1995, 1096 },{ 1987, 1113 },{ 1979, 1130 },
        { 1972, 1148 },{ 1964, 1164 },{ 1957, 1181 },{ 1950, 1198 },
        { 1943, 1215 },{ 1936, 1231 },{ 1930, 1247 },{ 1923, 1264 },
        { 1917, 1280 },{ 1910, 1296 },{ 1904, 1312 },{ 1898, 1328 },
        { 1892, 1343 },{ 1886, 1359 },{ 1880, 1374 },{ 1875, 1390 },
        { 1869, 1405 },{ 1863, 1420 },{ 1858, 1436 },{ 1853, 1451 },
        { 1848, 1466 },{ 1843, 1481 },{ 1838, 1495 },{ 1833, 1510 },
        { 1828, 1525 },{ 1823, 1539 },{ 1819, 1554 },{ 1814, 1568 },
        { 1810, 1583 },{ 1805, 1597 },{ 1801, 1611 },{ 1797, 1625 },
        { 1793, 1640 },{ 1789, 1654 },{ 1785, 1668 },{ 1781, 1681 },
        { 1777, 1695 },{ 1774, 1709 },{ 1770, 1723 },{ 1767, 1737 },
        { 1763, 1750 },{ 1760, 1764 },{ 1756, 1777 },{ 1753, 1791 },
        { 1750, 1804 },{ 1747, 1818 },{ 1744, 1831 },{ 1741, 1844 },
        { 1739, 1857 },{ 1736, 1871 },{ 1733, 1884 },{ 1731, 1897 },
        { 1728, 1910 },{ 1726, 1923 },{ 1723, 1936 },{ 1721, 1949 },
        { 1719, 1962 },{ 1717, 1975 },{ 1715, 1988 },{ 1713, 2001 },
        { 1711, 2014 },{ 1709, 2026 },{ 1707, 2039 },{ 1705, 2052 },
        { 1703, 2065 },{ 1702, 2077 },{ 1700, 2090 },{ 1699, 2103 },
        { 1697, 2115 },{ 1696, 2128 },{ 1695, 2140 },{ 1694, 2153 },
        { 1693, 2165 },{ 1691, 2178 },{ 1690, 2191 },{ 1689, 2203 },
        { 1689, 2216 },{ 1688, 2228 },{ 1687, 2240 },{ 1686, 2253 },
        { 1686, 2265 },{ 1685, 2278 },{ 1685, 2290 },{ 1684, 2303 },
        { 1684, 2315 },{ 1684, 2327 },{ 1683, 2340 },{ 1683, 2352 },
        { 1683, 2365 },{ 1683, 2377 },{ 1683, 2389 },{ 1683, 2402 },
        { 1683, 2414 },{ 1683, 2427 },{ 1684, 2439 },{ 1684, 2451 },
        { 1684, 2464 },{ 1685, 2476 },{ 1685, 2489 },{ 1686, 2501 },
        { 1687, 2514 },{ 1687, 2526 },{ 1688, 2539 },{ 1689, 2551 },
        { 1690, 2564 },{ 1691, 2576 },{ 1692, 2589 },{ 1693, 2601 },
        { 1694, 2614 },{ 1695, 2626 },{ 1697, 2639 },{ 1698, 2651 },
        { 1699, 2664 },{ 1701, 2677 },{ 1702, 2689 },{ 1704, 2702 },
        { 1706, 2715 },{ 1708, 2728 },{ 1709, 2740 },{ 1711, 2753 },
        { 1713, 2766 },{ 1715, 2779 },{ 1717, 2792 },{ 1720, 2805 },
        { 1722, 2818 },{ 1724, 2831 },{ 1726, 2844 },{ 1729, 2857 },
        { 1731, 2870 },{ 1734, 2883 },{ 1737, 2896 },{ 1740, 2909 },
        { 1742, 2923 },{ 1745, 2936 },{ 1748, 2949 },{ 1751, 2963 },
        { 1754, 2976 },{ 1758, 2990 },{ 1761, 3003 },{ 1764, 3017 },
        { 1768, 3030 },{ 1771, 3044 },{ 1775, 3058 },{ 1779, 3072 },
        { 1782, 3086 },{ 1786, 3100 },{ 1790, 3114 },{ 1794, 3128 },
        { 1798, 3142 },{ 1802, 3156 },{ 1807, 3170 },{ 1811, 3184 },
        { 1816, 3199 },{ 1820, 3213 },{ 1825, 3228 },{ 1830, 3243 },
        { 1834, 3257 },{ 1839, 3272 },{ 1844, 3287 },{ 1849, 3302 },
        { 1855, 3317 },{ 1860, 3332 },{ 1865, 3347 },{ 1871, 3362 },
        { 1877, 3378 },{ 1882, 3393 },{ 1888, 3409 },{ 1894, 3424 },
        { 1900, 3440 },{ 1906, 3456 },{ 1913, 3472 },{ 1919, 3488 },
        { 1925, 3504 },{ 1932, 3520 },{ 1939, 3537 },{ 1946, 3553 },
        { 1953, 3570 },{ 1960, 3587 },{ 1967, 3604 },{ 1974, 3621 },
        { 1982, 3638 },{ 1990, 3655 },{ 1997, 3672 },{ 2005, 3690 },
        { 2013, 3708 },{ 2021, 3725 },{ 2030, 3743 },{ 2038, 3761 },
        { 2047, 3780 },{ 2056, 3798 },{ 2053, 3795 },{ 2049, 3790 },
        { 2045, 3784 },{ 2041, 3779 },{ 2037, 3774 },{ 2034, 3768 },
        { 2030, 3763 },{ 2027, 3758 },{ 2023, 3752 },{ 2020, 3747 },
        { 2017, 3742 },{ 2014, 3736 },{ 2011, 3731 },{ 2008, 3726 }
    };
};
#endif  // __MHW_VEBOX_GENERIC_H__
