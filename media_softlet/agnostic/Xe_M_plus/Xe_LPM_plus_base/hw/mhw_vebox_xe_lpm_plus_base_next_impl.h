/*
* Copyright (c) 2022, Intel Corporation
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
//! \file     mhw_vebox_xe_lpm_plus_base_next_impl.h
//! \brief    MHW vebox interface common base for Xe_LPM_PLUS
//! \details
//!


#ifndef __MHW_VEBOX_XE_LPM_PLUS_BASE_NEXT_IMPL_H__
#define __MHW_VEBOX_XE_LPM_PLUS_BASE_NEXT_IMPL_H__

#include "mhw_vebox.h"
#include "mhw_vebox_impl.h"
#include "mhw_vebox_hwcmd_xe_lpm_plus_next.h"
#include "mhw_vebox_itf.h"
#include "mhw_impl.h"
#include "mos_solo_generic.h"
#include "mos_oca_rtlog_mgr.h"

//!
//! \brief Macro for Vebox Scalable
//!
#define MHW_VEBOX_MAX_PIPE_SIZE_XE_LPM_PLUS_BASE                         4096
#define MHW_VEBOX_MAX_SEMAPHORE_NUM_XE_LPM_PLUS_BASE                     4
#define MHW_VEBOX_TIMESTAMP_CNTS_PER_SEC_XE_LPM_PLUS_BASE                12000048
#define MHW_VEBOX_4K_PIC_WIDTH_XE_LPM_PLUS_BASE                          3840
#define MHW_VEBOX_4K_PIC_HEIGHT_XE_LPM_PLUS_BASE                         2160
#define MHW_VEBOX_TIMESTAMP_PER_TICK_IN_NS_XE_LPM_PLUS_BASE              83.333f
#define MHW_LACE_COLOR_COMPENSATION_LUT_POINT_NUMBER_XE_LPM_PLUS_BASE    16
#define MHW_VEBOX_SLIM_IPU_DN_CMD_SIZE_INUSE_XE_LPM_PLUS_BASE            34

namespace mhw
{
namespace vebox
{
namespace xe_lpm_plus_next
{
class Impl : public vebox::Impl<mhw::vebox::xe_lpm_plus_next::Cmd>
{
public:
    Impl(PMOS_INTERFACE osItf) : base_t(osItf)
    {
        MHW_FUNCTION_ENTER;
    };

    MOS_STATUS IecpStateInitialization(mhw::vebox::xe_lpm_plus_next::Cmd::VEBOX_IECP_STATE_CMD *pVeboxIecpState)
    {
        MHW_FUNCTION_ENTER;

        mhw::vebox::xe_lpm_plus_next::Cmd::VEBOX_IECP_STATE_CMD IecpState;
        *pVeboxIecpState = IecpState;

        // Re-set the values
        pVeboxIecpState->StdSteState.DW5.InvMarginVyl       = 3300;
        pVeboxIecpState->StdSteState.DW5.InvSkinTypesMargin = 1638;
        pVeboxIecpState->StdSteState.DW12.B3U               = 140;
        pVeboxIecpState->StdSteState.DW27.Hues0Dark         = 256;
        pVeboxIecpState->StdSteState.DW27.Hues1Dark         = 0;

        pVeboxIecpState->AceState.DW0.LaceHistogramSize     = 1;

        pVeboxIecpState->TccState.DW0.Satfactor1            = 160;
        pVeboxIecpState->TccState.DW0.Satfactor2            = 160;
        pVeboxIecpState->TccState.DW0.Satfactor3            = 160;
        pVeboxIecpState->TccState.DW1.Satfactor4            = 160;
        pVeboxIecpState->TccState.DW1.Satfactor5            = 160;
        pVeboxIecpState->TccState.DW1.Satfactor6            = 160;

        pVeboxIecpState->GamutState.DW2.CmS                 = 640;
        pVeboxIecpState->GamutState.DW3.AG                  = 26;
        pVeboxIecpState->GamutState.DW4.AB                  = 26;
        pVeboxIecpState->GamutState.DW5.RS                  = 768;
        pVeboxIecpState->GamutState.DW6.CmI                 = 192;
        pVeboxIecpState->GamutState.DW7.RI                  = 128;

        return MOS_STATUS_SUCCESS;
    }

        MOS_STATUS IecpStateInitializationforFDFB(mhw::vebox::xe_lpm_plus_next::Cmd::VEBOX_IECP_STATE_CMD *pVeboxIecpState)
    {
        MHW_FUNCTION_ENTER;

        mhw::vebox::xe_lpm_plus_next::Cmd::VEBOX_IECP_STATE_CMD IecpState;
        *pVeboxIecpState = IecpState;

        // Re-set the values
        pVeboxIecpState->StdSteState.DW0.StdEnable          = 1;
        pVeboxIecpState->StdSteState.DW0.OutputControl      = 1;
        pVeboxIecpState->StdSteState.DW1.StdScoreOutput     = 0;
        pVeboxIecpState->StdSteState.DW3.VyStdEnable        = 1;
        pVeboxIecpState->StdSteState.DW5.InvSkinTypesMargin = 20;
        pVeboxIecpState->AceState.DW0.AceEnable             = 0;
        pVeboxIecpState->AceState.DW0.LaceHistogramEnable   = 1;

        pVeboxIecpState->StdSteState.DW5.InvMarginVyl       = 3300;
        pVeboxIecpState->StdSteState.DW5.InvSkinTypesMargin = 1638;
        pVeboxIecpState->StdSteState.DW12.B3U               = 140;
        pVeboxIecpState->StdSteState.DW15.Satp1             = 122;
        pVeboxIecpState->StdSteState.DW15.Satb1             = 1016;
        pVeboxIecpState->StdSteState.DW19.Hueb1             = 1016;
        pVeboxIecpState->StdSteState.DW27.Hues0Dark         = 256;
        pVeboxIecpState->StdSteState.DW27.Hues1Dark         = 0;

        pVeboxIecpState->TccState.DW0.Satfactor1 = 160;
        pVeboxIecpState->TccState.DW0.Satfactor2 = 160;
        pVeboxIecpState->TccState.DW0.Satfactor3 = 160;
        pVeboxIecpState->TccState.DW1.Satfactor4 = 160;
        pVeboxIecpState->TccState.DW1.Satfactor5 = 160;
        pVeboxIecpState->TccState.DW1.Satfactor6 = 160;

        pVeboxIecpState->AlphaAoiState.DW1.AoiMaxX = 0;
        pVeboxIecpState->AlphaAoiState.DW2.AoiMaxY = 0;

        return MOS_STATUS_SUCCESS;
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
        mhw::vebox::xe_lpm_plus_next::Cmd::VEBOX_FRONT_END_CSC_STATE_CMD *pVeboxFecscState,
        PMHW_VEBOX_IECP_PARAMS                pVeboxIecpParams)
    {
        MHW_FUNCTION_ENTER;

        MHW_CAPPIPE_PARAMS *pCapPipeParams = nullptr;

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

    MOS_STATUS SetVeboxIecpStateBecsc(
        mhw::vebox::xe_lpm_plus_next::Cmd::VEBOX_IECP_STATE_CMD *pVeboxIecpState,
        PMHW_VEBOX_IECP_PARAMS                                  pVeboxIecpParams,
        bool                                         bEnableFECSC)
    {
        MHW_FUNCTION_ENTER;

        MHW_CAPPIPE_PARAMS *m_capPipeParams = nullptr;
        MOS_FORMAT          dstFormat;

        MHW_CHK_NULL_RETURN(pVeboxIecpState);
        MHW_CHK_NULL_RETURN(pVeboxIecpParams);

        m_capPipeParams = &pVeboxIecpParams->CapPipeParams;
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

        MHW_CHK_NULL_RETURN(m_capPipeParams);
        if (m_capPipeParams->bActive)
        {
            // Application controlled CSC operation
            if (m_capPipeParams->BECSCParams.bActive)
            {
                pVeboxIecpState->CscState.DW0.TransformEnable = true;

                // Coeff is S2.16, so multiply the floating value by 65536
                SET_COEFS(
                    ((uint32_t)(m_capPipeParams->BECSCParams.Matrix[0][0] * 65536)),
                    ((uint32_t)(m_capPipeParams->BECSCParams.Matrix[0][1] * 65536)),
                    ((uint32_t)(m_capPipeParams->BECSCParams.Matrix[0][2] * 65536)),
                    ((uint32_t)(m_capPipeParams->BECSCParams.Matrix[1][0] * 65536)),
                    ((uint32_t)(m_capPipeParams->BECSCParams.Matrix[1][1] * 65536)),
                    ((uint32_t)(m_capPipeParams->BECSCParams.Matrix[1][2] * 65536)),
                    ((uint32_t)(m_capPipeParams->BECSCParams.Matrix[2][0] * 65536)),
                    ((uint32_t)(m_capPipeParams->BECSCParams.Matrix[2][1] * 65536)),
                    ((uint32_t)(m_capPipeParams->BECSCParams.Matrix[2][2] * 65536)));
                SET_INPUT_OFFSETS(
                    ((uint32_t)m_capPipeParams->BECSCParams.PreOffset[0]),
                    ((uint32_t)m_capPipeParams->BECSCParams.PreOffset[1]),
                    ((uint32_t)m_capPipeParams->BECSCParams.PreOffset[2]));
                SET_OUTPUT_OFFSETS(
                    ((uint32_t)m_capPipeParams->BECSCParams.PostOffset[0]),
                    ((uint32_t)m_capPipeParams->BECSCParams.PostOffset[1]),
                    ((uint32_t)m_capPipeParams->BECSCParams.PostOffset[2]));
            }
            // YUV 4:4:4 CSC to xBGR or xRGB
            else if ((bEnableFECSC || (pVeboxIecpParams->srcFormat == Format_AYUV)) &&
                     (IS_RGB_FORMAT(dstFormat)))
            {
                pVeboxIecpState->CscState.DW0.TransformEnable = true;

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
                else
                {
                    MHW_ASSERT(false);
                }
            }
        }
        else if (pVeboxIecpParams->bCSCEnable)
        {
            pVeboxIecpState->CscState.DW0.TransformEnable = true;

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

        if (pVeboxIecpParams->dstFormat == Format_Y416)
        {
            pVeboxIecpState->AlphaAoiState.DW0.ColorPipeAlpha = pVeboxIecpParams->wAlphaValue;
        }
        else
        {
            // Alpha is U16, but the SW alpha is calculated as 8bits,
            // so left shift it 8bits to be in the position of MSB
            pVeboxIecpState->AlphaAoiState.DW0.ColorPipeAlpha = pVeboxIecpParams->wAlphaValue * 256;
        }

#undef SET_COEFS
#undef SET_INPUT_OFFSETS
#undef SET_OUTPUT_OFFSETS

        return MOS_STATUS_SUCCESS;
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
        mhw::vebox::xe_lpm_plus_next::Cmd::VEBOX_IECP_STATE_CMD* pVeboxIecpState,
        PMHW_CAPPIPE_PARAMS                        pCapPipeParams,
        const unsigned int                         uCoeffValue)
    {
        MHW_FUNCTION_ENTER;

        mhw::vebox::xe_lpm_plus_next::Cmd::VEBOX_CCM_STATE_CMD *pCcm = nullptr;

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
        mhw::vebox::xe_lpm_plus_next::Cmd::VEBOX_PROCAMP_STATE_CMD *pVeboxProcampState,
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
    //! \brief      Add VEBOX Capture pipe state
    //! \details    Add VEBOX Capture pipe state
    //! \param      [in] pCapPipeParams
    //!             Pointer to capture pipe parameters
    //! \return     void
    //!
    void SetVeboxCapPipeState(
        PMHW_CAPPIPE_PARAMS pCapPipeParams)
    {
        MHW_FUNCTION_ENTER;

        mhw::vebox::xe_lpm_plus_next::Cmd::VEBOX_CAPTURE_PIPE_STATE_CMD *pVeboxCapPipeState, CapPipCmd;

        PMHW_VEBOX_HEAP pVeboxHeap;
        uint32_t        uiOffset;

        MHW_CHK_NULL_NO_STATUS_RETURN(pCapPipeParams);
        MHW_CHK_NULL_NO_STATUS_RETURN(m_veboxHeap);

        pVeboxHeap = m_veboxHeap;
        uiOffset   = pVeboxHeap->uiCurState * pVeboxHeap->uiInstanceSize;

        pVeboxCapPipeState =
            (mhw::vebox::xe_lpm_plus_next::Cmd::VEBOX_CAPTURE_PIPE_STATE_CMD *)(pVeboxHeap->pLockedDriverResourceMem +
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
    //! \brief      Add VEBOX ACE/LACE States
    //! \details    Add Vebox ACE, Lace states
    //! \param      [in] pVeboxIecpParams
    //!             Pointer to VEBOX IECP State Params
    //! \return     voild
    //!
    void SetVeboxAceLaceState(
        PMHW_VEBOX_IECP_PARAMS                    pVeboxIecpParams,
        mhw::vebox::xe_lpm_plus_next::Cmd::VEBOX_IECP_STATE_CMD *pVeboxIecpState)
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

    MOS_STATUS SetVeboxVertexTable(
        MHW_CSPACE           ColorSpace)
    {
        MHW_FUNCTION_ENTER;

        MHW_VEBOX_HEAP          *pVeboxHeap;
        uint32_t                uiOffset;
        uint32_t                uSize;
        MOS_STATUS              eStatus = MOS_STATUS_SUCCESS;
        typename mhw::vebox::xe_lpm_plus_next::Cmd::VEBOX_VERTEX_TABLE_CMD* pVertexTable;

        MHW_CHK_NULL_RETURN(m_veboxHeap);

        pVeboxHeap = m_veboxHeap;
        uiOffset = pVeboxHeap->uiCurState * pVeboxHeap->uiInstanceSize;
        uSize = sizeof(typename mhw::vebox::xe_lpm_plus_next::Cmd::VEBOX_VERTEX_TABLE_CMD);
        pVertexTable = (typename mhw::vebox::xe_lpm_plus_next::Cmd::VEBOX_VERTEX_TABLE_CMD*)(pVeboxHeap->pLockedDriverResourceMem +
            pVeboxHeap->uiVertexTableOffset +
            uiOffset);
        memset(pVertexTable, 0, uSize);
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

        return eStatus;
    }

    MOS_STATUS setVeboxPrologCmd(
        std::shared_ptr<mhw::mi::Itf> miItf,
        PMOS_COMMAND_BUFFER CmdBuffer)
    {
        MHW_FUNCTION_ENTER;

        MOS_STATUS eStatus          = MOS_STATUS_SUCCESS;
        uint64_t   auxTableBaseAddr = 0;

        MHW_CHK_NULL_RETURN(miItf);
        MHW_CHK_NULL_RETURN(CmdBuffer);
        MHW_CHK_NULL_RETURN(this->m_osItf);

        auxTableBaseAddr = this->m_osItf->pfnGetAuxTableBaseAddr(this->m_osItf);

        if (auxTableBaseAddr)
        {
            auto& par = miItf->MHW_GETPAR_F(MI_LOAD_REGISTER_IMM)();
            par = {};
            par.dwData     = (auxTableBaseAddr & 0xffffffff);
            par.dwRegister = miItf->GetMmioInterfaces(mhw::mi::MHW_MMIO_VE0_AUX_TABLE_BASE_LOW);
            miItf->MHW_ADDCMD_F(MI_LOAD_REGISTER_IMM)(CmdBuffer);

            par.dwData     = ((auxTableBaseAddr >> 32) & 0xffffffff);
            par.dwRegister = miItf->GetMmioInterfaces(mhw::mi::MHW_MMIO_VE0_AUX_TABLE_BASE_HIGH);
            miItf->MHW_ADDCMD_F(MI_LOAD_REGISTER_IMM)(CmdBuffer);
        }

        return eStatus;
    }

    MOS_STATUS AddVeboxHdrState(
        PMHW_VEBOX_IECP_PARAMS pVeboxIecpParams) override
    {
        MHW_FUNCTION_ENTER;

        MOS_STATUS                                eStatus        = MOS_STATUS_SUCCESS;
        mhw::vebox::xe_lpm_plus_next::Cmd::VEBOX_HDR_STATE_CMD * pVeboxHdrState = nullptr;
        mhw::vebox::xe_lpm_plus_next::Cmd::VEBOX_IECP_STATE_CMD *pIecpState     = nullptr;
        MHW_VEBOX_HEAP                            *pVeboxHeap     = nullptr;
        uint32_t                                  uiOffset       = 0;

        MHW_CHK_NULL_RETURN(pVeboxIecpParams);
        MHW_CHK_NULL_RETURN(m_veboxHeap);

        pVeboxHeap = m_veboxHeap;
        uiOffset   = pVeboxHeap->uiCurState * pVeboxHeap->uiInstanceSize;

        pVeboxHdrState =
            (mhw::vebox::xe_lpm_plus_next::Cmd::VEBOX_HDR_STATE_CMD *)(pVeboxHeap->pLockedDriverResourceMem +
                                                           pVeboxHeap->uiHdrStateOffset +
                                                           uiOffset);

        pIecpState =
            (mhw::vebox::xe_lpm_plus_next::Cmd::VEBOX_IECP_STATE_CMD *)(pVeboxHeap->pLockedDriverResourceMem +
                                                            pVeboxHeap->uiIecpStateOffset +
                                                            uiOffset);

        MHW_CHK_NULL_RETURN(pVeboxHdrState);
        MHW_CHK_NULL_RETURN(pIecpState);

        // Program 1DLUT in Inverse Gamma with 1024 entries / 16bit precision from API level
        if (pVeboxIecpParams->s1DLutParams.bActive && (pVeboxIecpParams->s1DLutParams.LUTSize == 1024))
        {
            // HW provides 4K 1DLUT inverse gamma
            mhw::vebox::xe_lpm_plus_next::Cmd::VEBOX_HDR_INV_GAMMA_CORRECTION_STATE_CMD *pInverseGamma = pVeboxHdrState->PRGBCorrectedValue;
            uint16_t *                                                       p1DLut        = (uint16_t *)pVeboxIecpParams->s1DLutParams.p1DLUT;
            for (uint32_t i = 0; i < pVeboxIecpParams->s1DLutParams.LUTSize; i++)
            {
                pInverseGamma[i].DW0.Value                               = 0;
                pInverseGamma[i].DW1.InverseRChannelGammaCorrectionValue = (uint32_t)(p1DLut[4 * i + 1]);  // 32 bit precision
                pInverseGamma[i].DW2.InverseGChannelGammaCorrectionValue = (uint32_t)(p1DLut[4 * i + 2]);  // 32 bit precision
                pInverseGamma[i].DW3.InverseBChannelGammaCorrectionValue = (uint32_t)(p1DLut[4 * i + 3]);  // 32 bit precision
            }
            for (uint32_t i = pVeboxIecpParams->s1DLutParams.LUTSize; i < 4096; i++)
            {
                pInverseGamma[i].DW0.Value                               = 0;
                pInverseGamma[i].DW1.InverseRChannelGammaCorrectionValue = 0;
                pInverseGamma[i].DW2.InverseGChannelGammaCorrectionValue = 0;
                pInverseGamma[i].DW3.InverseBChannelGammaCorrectionValue = 0;
            }

            pVeboxHdrState->DW17440.ToneMappingEnable = false;

            mhw::vebox::xe_lpm_plus_next::Cmd::VEBOX_HDR_FWD_GAMMA_CORRECTION_STATE_CMD *pForwardGamma = pVeboxHdrState->ForwardGammaLUTvalue;
            for (uint32_t i = 0; i < 254; i++)
            {
                pForwardGamma[i].DW0.PointValueForForwardGammaLut        = 256 * i;
                pForwardGamma[i].DW1.ForwardRChannelGammaCorrectionValue = 256 * i;
                pForwardGamma[i].DW2.ForwardGChannelGammaCorrectionValue = 256 * i;
                pForwardGamma[i].DW3.ForwardBChannelGammaCorrectionValue = 256 * i;
            }

            pForwardGamma[254].DW0.PointValueForForwardGammaLut        = 0xffff;
            pForwardGamma[254].DW1.ForwardRChannelGammaCorrectionValue = 0xffff;
            pForwardGamma[254].DW2.ForwardGChannelGammaCorrectionValue = 0xffff;
            pForwardGamma[254].DW3.ForwardBChannelGammaCorrectionValue = 0xffff;

            pForwardGamma[255].DW0.PointValueForForwardGammaLut        = 0xffffffff;
            pForwardGamma[255].DW1.ForwardRChannelGammaCorrectionValue = 0xffff;
            pForwardGamma[255].DW2.ForwardGChannelGammaCorrectionValue = 0xffff;
            pForwardGamma[255].DW3.ForwardBChannelGammaCorrectionValue = 0xffff;

            // Program CCM as identity matrix
            pIecpState->CcmState.DW0.ColorCorrectionMatrixEnable = false;
            pIecpState->CcmState.DW1.C0                          = 0x400000;
            pIecpState->CcmState.DW0.C1                          = 0;
            pIecpState->CcmState.DW3.C2                          = 0;
            pIecpState->CcmState.DW2.C3                          = 0;
            pIecpState->CcmState.DW5.C4                          = 0x400000;
            pIecpState->CcmState.DW4.C5                          = 0;
            pIecpState->CcmState.DW7.C6                          = 0;
            pIecpState->CcmState.DW6.C7                          = 0;
            pIecpState->CcmState.DW8.C8                          = 0x400000;
            pIecpState->CcmState.DW9.OffsetInR                   = 0;
            pIecpState->CcmState.DW10.OffsetInG                  = 0;
            pIecpState->CcmState.DW11.OffsetInB                  = 0;
            pIecpState->CcmState.DW12.OffsetOutR                 = 0;
            pIecpState->CcmState.DW13.OffsetOutG                 = 0;
            pIecpState->CcmState.DW14.OffsetOutB                 = 0;
        }
        else if (pVeboxIecpParams->bCcmCscEnable)
        {
            uint32_t nLutInBitDepth     = 12;
            uint32_t nLutOutBitDepth    = 32;
            uint64_t maxValLutIn        = (((uint64_t)1) << nLutInBitDepth) - 1;
            uint64_t maxValLutOut       = (((uint64_t)1) << nLutOutBitDepth) - 1;

            // HW provides 4K 1DLUT inverse gamma and fill in with identity
            mhw::vebox::xe_lpm_plus_next::Cmd::VEBOX_HDR_INV_GAMMA_CORRECTION_STATE_CMD *pInverseGamma = pVeboxHdrState->PRGBCorrectedValue;
            for (uint32_t i = 0; i < 4096; i++)
            {
                float x = (float)(i) / maxValLutIn;
                uint32_t nCorrectedValue = (i < 4095) ? (uint32_t)(x * maxValLutOut + 0.5) : (uint32_t)(maxValLutOut);
                pInverseGamma[i].DW0.Value = 0;
                pInverseGamma[i].DW1.InverseRChannelGammaCorrectionValue = nCorrectedValue;
                pInverseGamma[i].DW2.InverseGChannelGammaCorrectionValue = nCorrectedValue;
                pInverseGamma[i].DW3.InverseBChannelGammaCorrectionValue = nCorrectedValue;
            }
            pVeboxHdrState->DW17440.ToneMappingEnable = false;
            pIecpState->CcmState.DW0.ColorCorrectionMatrixEnable = false;
            if ((pVeboxIecpParams->ColorSpace == MHW_CSpace_BT709) ||
                (pVeboxIecpParams->ColorSpace == MHW_CSpace_BT709_FullRange))
            {
                pIecpState->CcmState.DW1.C0 = 0x00009937;
                pIecpState->CcmState.DW0.C1 = 0x000115f6;
                pIecpState->CcmState.DW3.C2 = 0;
                pIecpState->CcmState.DW2.C3 = 0x00009937;
                pIecpState->CcmState.DW5.C4 = 0x07ffe3f1;
                pIecpState->CcmState.DW4.C5 = 0x07ffb9e0;
                pIecpState->CcmState.DW7.C6 = 0x00009937;
                pIecpState->CcmState.DW6.C7 = 0;
                pIecpState->CcmState.DW8.C8 = 0x0000ebe6;
                pIecpState->CcmState.DW9.OffsetInR   = (pVeboxIecpParams->ColorSpace == MHW_CSpace_BT709) ? 0xf8000000 : 0;
                pIecpState->CcmState.DW10.OffsetInG  = (pVeboxIecpParams->ColorSpace == MHW_CSpace_BT709) ? 0xc0000000 : 0;
                pIecpState->CcmState.DW11.OffsetInB  = (pVeboxIecpParams->ColorSpace == MHW_CSpace_BT709) ? 0xc0000000 : 0;
                pIecpState->CcmState.DW12.OffsetOutR = 0;
                pIecpState->CcmState.DW13.OffsetOutG = 0;
                pIecpState->CcmState.DW14.OffsetOutB = 0;
            }
            else if ((pVeboxIecpParams->ColorSpace == MHW_CSpace_BT2020) ||
                        (pVeboxIecpParams->ColorSpace == MHW_CSpace_BT2020_FullRange))
            {
                pIecpState->CcmState.DW1.C0 = 0x00009937;
                pIecpState->CcmState.DW0.C1 = 0x000119d4;
                pIecpState->CcmState.DW3.C2 = 0;
                pIecpState->CcmState.DW2.C3 = 0x00009937;
                pIecpState->CcmState.DW5.C4 = 0x07ffe75a;
                pIecpState->CcmState.DW4.C5 = 0x07ffaa6a;
                pIecpState->CcmState.DW7.C6 = 0x00009937;
                pIecpState->CcmState.DW6.C7 = 0;
                pIecpState->CcmState.DW8.C8 = 0x0000dce4;
                pIecpState->CcmState.DW9.OffsetInR   = (pVeboxIecpParams->ColorSpace == MHW_CSpace_BT2020) ? 0xf8000000 : 0;
                pIecpState->CcmState.DW10.OffsetInG  = (pVeboxIecpParams->ColorSpace == MHW_CSpace_BT2020) ? 0xc0000000 : 0;
                pIecpState->CcmState.DW11.OffsetInB  = (pVeboxIecpParams->ColorSpace == MHW_CSpace_BT2020) ? 0xc0000000 : 0;
                pIecpState->CcmState.DW12.OffsetOutR = 0;
                pIecpState->CcmState.DW13.OffsetOutG = 0;
                pIecpState->CcmState.DW14.OffsetOutB = 0;
            }
            else
            {
                MHW_ASSERTMESSAGE("Unsupported Input Color Space!");
            }
        }

        return eStatus;
    }

    MOS_STATUS SetVeboxGamutState(
        PMHW_VEBOX_IECP_PARAMS  pVeboxIecpParams,
        PMHW_VEBOX_GAMUT_PARAMS pVeboxGamutParams) override
    {
        MHW_FUNCTION_ENTER;

        MHW_VEBOX_HEAP  *pVeboxHeap;
        uint32_t        uiOffset;
        uint32_t        i;
        double          dInverseGamma       = 0;
        double          dForwardGamma       = 1.0;  // init as 1.0 as default to avoid divisor be 0
        MOS_STATUS      eStatus             = MOS_STATUS_SUCCESS;
        uint16_t        usGE_Values[256][8] = {0};
        bool            bEnableCCM          = false;

        MHW_1DLUT_PARAMS                                            *p1DLutParams = nullptr;
        mhw::vebox::xe_lpm_plus_next::Cmd::VEBOX_IECP_STATE_CMD *    pIecpState;
        mhw::vebox::xe_lpm_plus_next::Cmd::VEBOX_GAMUT_CONTROL_STATE_CMD *pGamutState, gamutCmd;
        mhw::vebox::xe_lpm_plus_next::Cmd::Gamut_Expansion_Gamma_Correction_CMD *pVeboxGEGammaCorrection, VeboxGEGammaCorrection;

        MHW_CHK_NULL_RETURN(pVeboxGamutParams);
        MHW_CHK_NULL_RETURN(pVeboxIecpParams);
        MHW_CHK_NULL_RETURN(m_veboxHeap);

        pVeboxHeap = m_veboxHeap;
        uiOffset   = pVeboxHeap->uiCurState * pVeboxHeap->uiInstanceSize;

        pIecpState =
            (mhw::vebox::xe_lpm_plus_next::Cmd::VEBOX_IECP_STATE_CMD *)(pVeboxHeap->pLockedDriverResourceMem +
                                                            pVeboxHeap->uiIecpStateOffset +
                                                            uiOffset);
        pVeboxGEGammaCorrection =
            (mhw::vebox::xe_lpm_plus_next::Cmd::Gamut_Expansion_Gamma_Correction_CMD *)(pVeboxHeap->pLockedDriverResourceMem +
                                                                            pVeboxHeap->uiGamutStateOffset +
                                                                            uiOffset);

        MHW_CHK_NULL_RETURN(pIecpState);
        MHW_CHK_NULL_RETURN(pVeboxGEGammaCorrection);

        // Must initialize VeboxIecpState even if it is not used because GCE
        // requires GlobalIECP enable bit to be turned on
        if (!pVeboxIecpParams)
        {
            IecpStateInitialization(pIecpState);
        }
        pGamutState = &pIecpState->GamutState;
        MHW_CHK_NULL_RETURN(pGamutState);

        if (pVeboxGamutParams->GCompMode != MHW_GAMUT_MODE_NONE)
        {
            if (pVeboxGamutParams->GCompMode == MHW_GAMUT_MODE_BASIC)
            {
                pGamutState->DW15.Fullrangemappingenable = false;

                if (pVeboxGamutParams->GCompBasicMode == gamutCmd.GCC_BASICMODESELECTION_SCALINGFACTOR)
                {
                    pGamutState->DW17.GccBasicmodeselection = gamutCmd.GCC_BASICMODESELECTION_SCALINGFACTOR;
                    pGamutState->DW17.Basicmodescalingfactor =
                        pVeboxGamutParams->iBasicModeScalingFactor;
                }
            }
            else if (pVeboxGamutParams->GCompMode == MHW_GAMUT_MODE_ADVANCED)
            {
                pGamutState->DW15.Fullrangemappingenable = true;
                pGamutState->DW15.D1Out                  = pVeboxGamutParams->iDout;
                pGamutState->DW15.DOutDefault            = pVeboxGamutParams->iDoutDefault;
                pGamutState->DW15.DInDefault             = pVeboxGamutParams->iDinDefault;
                pGamutState->DW16.D1In                   = pVeboxGamutParams->iDin;
            }
            else
            {
                MHW_ASSERTMESSAGE("Invalid GAMUT MODE");
            }

            // Set Vertex Table if Gamut Compression is enabled
            SetVeboxVertexTable(pVeboxGamutParams->ColorSpace);
        }

        // Initialize the Gamut_Expansion_Gamma_Correction.
        *pVeboxGEGammaCorrection = VeboxGEGammaCorrection;
        if (pVeboxGamutParams->bColorBalance)
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

            pGamutState->DW0.GlobalModeEnable = true;
            pGamutState->DW1.CmW              = 1023;

            pGamutState->DW1.C0 = pVeboxGamutParams->Matrix[0][0];
            pGamutState->DW0.C1 = pVeboxGamutParams->Matrix[0][1];
            pGamutState->DW3.C2 = pVeboxGamutParams->Matrix[0][2];
            pGamutState->DW2.C3 = pVeboxGamutParams->Matrix[1][0];
            pGamutState->DW5.C4 = pVeboxGamutParams->Matrix[1][1];
            pGamutState->DW4.C5 = pVeboxGamutParams->Matrix[1][2];
            pGamutState->DW7.C6 = pVeboxGamutParams->Matrix[2][0];
            pGamutState->DW6.C7 = pVeboxGamutParams->Matrix[2][1];
            pGamutState->DW8.C8 = pVeboxGamutParams->Matrix[2][2];
            pGamutState->DW9.OffsetInR   = 0;
            pGamutState->DW10.OffsetInG  = 0;
            pGamutState->DW11.OffsetInB  = 0;
            pGamutState->DW12.OffsetOutR = 0;
            pGamutState->DW13.OffsetOutG = 0;
            pGamutState->DW14.OffsetOutB = 0;

            for (i = 0; i < 256; i++)
            {
                usGE_Values[i][0] = 257 * i;
                usGE_Values[i][1] =
                    usGE_Values[i][2] =
                        usGE_Values[i][3] = 257 * i;

                usGE_Values[i][4] = 257 * i;
                usGE_Values[i][5] =
                    usGE_Values[i][6] =
                        usGE_Values[i][7] = 257 * i;
            }
            // Copy two uint16_t to one DW (UNT32).
            MOS_SecureMemcpy(pVeboxGEGammaCorrection, sizeof(uint32_t) * 1024, usGE_Values, sizeof(uint16_t) * 8 * 256);
        }
        else if (pVeboxGamutParams->GExpMode != MHW_GAMUT_MODE_NONE)
        {
            // Need to convert YUV input to RGB before GE
            pIecpState->CscState.DW0.TransformEnable = true;
            if (pVeboxGamutParams->ColorSpace == MHW_CSpace_BT601 ||
                pVeboxGamutParams->ColorSpace == MHW_CSpace_xvYCC601 ||
                pVeboxGamutParams->ColorSpace == MHW_CSpace_BT601_FullRange)
            {
                pIecpState->CscState.DW0.C0          = 1192;
                pIecpState->CscState.DW1.C1          = MOS_BITFIELD_VALUE((uint32_t)-2, 19);
                pIecpState->CscState.DW2.C2          = 1634;
                pIecpState->CscState.DW3.C3          = 1192;
                pIecpState->CscState.DW4.C4          = MOS_BITFIELD_VALUE((uint32_t)-401, 19);
                pIecpState->CscState.DW5.C5          = MOS_BITFIELD_VALUE((uint32_t)-833, 19);
                pIecpState->CscState.DW6.C6          = 1192;
                pIecpState->CscState.DW7.C7          = 2066;
                pIecpState->CscState.DW8.C8          = MOS_BITFIELD_VALUE((uint32_t)-1, 19);
                pIecpState->CscState.DW9.OffsetIn1   = MOS_BITFIELD_VALUE((uint32_t)-64, 16);
                pIecpState->CscState.DW9.OffsetOut1  = 0;
                pIecpState->CscState.DW10.OffsetIn2  = MOS_BITFIELD_VALUE((uint32_t)-512, 16);
                pIecpState->CscState.DW10.OffsetOut2 = 0;
                pIecpState->CscState.DW11.OffsetIn3  = MOS_BITFIELD_VALUE((uint32_t)-512, 16);
                pIecpState->CscState.DW11.OffsetOut3 = 0;
            }
            else if (pVeboxGamutParams->ColorSpace == MHW_CSpace_BT709 ||
                     pVeboxGamutParams->ColorSpace == MHW_CSpace_xvYCC709 ||
                     pVeboxGamutParams->ColorSpace == MHW_CSpace_BT709_FullRange)
            {
                pIecpState->CscState.DW0.C0          = 1192;
                pIecpState->CscState.DW1.C1          = MOS_BITFIELD_VALUE((uint32_t)-1, 19);
                pIecpState->CscState.DW2.C2          = 1835;
                pIecpState->CscState.DW3.C3          = 1192;
                pIecpState->CscState.DW4.C4          = MOS_BITFIELD_VALUE((uint32_t)-218, 19);
                pIecpState->CscState.DW5.C5          = MOS_BITFIELD_VALUE((uint32_t)-537, 19);
                pIecpState->CscState.DW6.C6          = 1192;
                pIecpState->CscState.DW7.C7          = 2164;
                pIecpState->CscState.DW8.C8          = 1;
                pIecpState->CscState.DW9.OffsetIn1   = MOS_BITFIELD_VALUE((uint32_t)-64, 16);
                pIecpState->CscState.DW9.OffsetOut1  = 0;
                pIecpState->CscState.DW10.OffsetIn2  = MOS_BITFIELD_VALUE((uint32_t)-512, 16);
                pIecpState->CscState.DW10.OffsetOut2 = 0;
                pIecpState->CscState.DW11.OffsetIn3  = MOS_BITFIELD_VALUE((uint32_t)-512, 16);
                pIecpState->CscState.DW11.OffsetOut3 = 0;
            }
            else
            {
                MHW_ASSERTMESSAGE("Unknown primary");
            }

            if (pVeboxGamutParams->GExpMode == MHW_GAMUT_MODE_BASIC)
            {
                pGamutState->DW0.GlobalModeEnable = true;
                pGamutState->DW1.CmW              = 1023;
            }
            else if (pVeboxGamutParams->GExpMode == MHW_GAMUT_MODE_ADVANCED)
            {
                pGamutState->DW0.GlobalModeEnable = false;
            }
            else
            {
                MHW_ASSERTMESSAGE("Invalid GAMUT MODE");
            }

            pGamutState->DW1.C0 = pVeboxGamutParams->Matrix[0][0];
            pGamutState->DW0.C1 = pVeboxGamutParams->Matrix[0][1];
            pGamutState->DW3.C2 = pVeboxGamutParams->Matrix[0][2];
            pGamutState->DW2.C3 = pVeboxGamutParams->Matrix[1][0];
            pGamutState->DW5.C4 = pVeboxGamutParams->Matrix[1][1];
            pGamutState->DW4.C5 = pVeboxGamutParams->Matrix[1][2];
            pGamutState->DW7.C6 = pVeboxGamutParams->Matrix[2][0];
            pGamutState->DW6.C7 = pVeboxGamutParams->Matrix[2][1];
            pGamutState->DW8.C8 = pVeboxGamutParams->Matrix[2][2];
        }
        else if (pVeboxGamutParams->bGammaCorr)
        {
            // Need to convert YUV input to RGB before Gamma Correction
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
            else if (pVeboxGamutParams->ColorSpace == MHW_CSpace_BT2020||
                     pVeboxGamutParams->ColorSpace == MHW_CSpace_BT2020_FullRange)
            {
                VeboxInterface_BT2020YUVToRGB(m_veboxHeap, pVeboxIecpParams, pVeboxGamutParams);
            }
            else
            {
                MHW_ASSERTMESSAGE("Unknown primary");
            }

            // CCM is needed for CSC(BT2020->BT709/BT601 or vice versa with Different Gamma).
            bEnableCCM                        = (pVeboxGamutParams->InputGammaValue == pVeboxGamutParams->OutputGammaValue) ? false : true;
            pGamutState->DW0.GlobalModeEnable = true;
            pGamutState->DW1.CmW              = 1023;
            if ((pVeboxGamutParams->ColorSpace == MHW_CSpace_BT2020) && bEnableCCM)
            {
                if (pVeboxGamutParams->dstColorSpace == MHW_CSpace_BT709)
                {
                    pGamutState->DW1.C0 = 108190;
                    pGamutState->DW0.C1 = MOS_BITFIELD_VALUE((uint32_t)-38288, 21);
                    pGamutState->DW3.C2 = MOS_BITFIELD_VALUE((uint32_t)-4747, 21);
                    pGamutState->DW2.C3 = MOS_BITFIELD_VALUE((uint32_t)-7967, 21);
                    pGamutState->DW5.C4 = 74174;
                    pGamutState->DW4.C5 = MOS_BITFIELD_VALUE((uint32_t)-557, 21);
                    pGamutState->DW7.C6 = MOS_BITFIELD_VALUE((uint32_t)-1198, 21);
                    pGamutState->DW6.C7 = MOS_BITFIELD_VALUE((uint32_t)-6587, 21);
                    pGamutState->DW8.C8 = 73321;
                }
                else
                {
                    pGamutState->DW1.C0 = 116420;
                    pGamutState->DW0.C1 = MOS_BITFIELD_VALUE((uint32_t)-45094, 21);
                    pGamutState->DW3.C2 = MOS_BITFIELD_VALUE((uint32_t)-5785, 21);
                    pGamutState->DW2.C3 = MOS_BITFIELD_VALUE((uint32_t)-10586, 21);
                    pGamutState->DW5.C4 = 77814;
                    pGamutState->DW4.C5 = MOS_BITFIELD_VALUE((uint32_t)-1705, 21);
                    pGamutState->DW7.C6 = MOS_BITFIELD_VALUE((uint32_t)-1036, 21);
                    pGamutState->DW6.C7 = MOS_BITFIELD_VALUE((uint32_t)-6284, 21);
                    pGamutState->DW8.C8 = 72864;
                }
            }
            else
            {
                pGamutState->DW1.C0          = 65536;
                pGamutState->DW0.C1          = 0;
                pGamutState->DW3.C2          = 0;
                pGamutState->DW2.C3          = 0;
                pGamutState->DW5.C4          = 65536;
                pGamutState->DW4.C5          = 0;
                pGamutState->DW7.C6          = 0;
                pGamutState->DW6.C7          = 0;
                pGamutState->DW8.C8          = 65536;
                pGamutState->DW9.OffsetInR   = 0;
                pGamutState->DW10.OffsetInG  = 0;
                pGamutState->DW11.OffsetInB  = 0;
                pGamutState->DW12.OffsetOutR = 0;
                pGamutState->DW13.OffsetOutG = 0;
                pGamutState->DW14.OffsetOutB = 0;
            }

            if (pVeboxGamutParams->InputGammaValue == MHW_GAMMA_1P0)
            {
                dInverseGamma = 1.0;
            }
            else if (pVeboxGamutParams->InputGammaValue == MHW_GAMMA_2P2)
            {
                dInverseGamma = 2.2;
            }
            else if (pVeboxGamutParams->InputGammaValue == MHW_GAMMA_2P6)
            {
                dInverseGamma = 2.6;
            }
            else
            {
                MHW_ASSERTMESSAGE("Invalid InputGammaValue");
            }

            if (pVeboxGamutParams->OutputGammaValue == MHW_GAMMA_1P0)
            {
                dForwardGamma = 1.0;
            }
            else if (pVeboxGamutParams->OutputGammaValue == MHW_GAMMA_2P2)
            {
                dForwardGamma = 2.2;
            }
            else if (pVeboxGamutParams->OutputGammaValue == MHW_GAMMA_2P6)
            {
                dForwardGamma = 2.6;
            }
            else
            {
                MHW_ASSERTMESSAGE("Invalid OutputGammaValue");
            }
            if ((pVeboxGamutParams->InputGammaValue == MHW_GAMMA_1P0) && (pVeboxGamutParams->OutputGammaValue == MHW_GAMMA_1P0))
            {
                for (i = 0; i < 256; i++)
                {
                    usGE_Values[i][0] = 257 * i;
                    usGE_Values[i][1] =
                    usGE_Values[i][2] =
                    usGE_Values[i][3] = 257 * i;

                    usGE_Values[i][4] = 257 * i;
                    usGE_Values[i][5] =
                    usGE_Values[i][6] =
                    usGE_Values[i][7] = 257 * i;
                }
                // Copy two uint16_t to one DW (UNT32).
                MOS_SecureMemcpy(pVeboxGEGammaCorrection, sizeof(uint32_t) * 1024, usGE_Values, sizeof(uint16_t) * 8 * 256);
            }
            else
            {
                for (i = 0; i < 255; i++)
                {
                    usGE_Values[i][0] = 256 * i;
                    usGE_Values[i][1] =
                    usGE_Values[i][2] =
                    usGE_Values[i][3] = (uint16_t)MOS_F_ROUND(pow((double)((double)i / 256), dInverseGamma) * 65536);

                    usGE_Values[i][4] = 256 * i;
                    usGE_Values[i][5] =
                    usGE_Values[i][6] =
                    usGE_Values[i][7] = (uint16_t)MOS_F_ROUND(pow((double)((double)i / 256), 1 / dForwardGamma) * 65536);
                }
                // Copy two uint16_t to one DW (UNT32).
                MOS_SecureMemcpy(pVeboxGEGammaCorrection, sizeof(uint32_t) * 1020, usGE_Values, sizeof(uint16_t) * 8 * 255);
            }
        }
        else if (pVeboxGamutParams->bH2S)
        {
            VeboxInterface_H2SManualMode(m_veboxHeap, pVeboxIecpParams, pVeboxGamutParams);
        }
        else if (pVeboxGamutParams->ColorSpace == MHW_CSpace_BT2020 ||
                 pVeboxGamutParams->ColorSpace == MHW_CSpace_BT2020_FullRange)  // BT2020 CSC case
        {
            if (pVeboxIecpParams->s1DLutParams.bActive)
            {
                //CCM setting if 1Dlut VEBOX HDR enabled
                p1DLutParams = &pVeboxIecpParams->s1DLutParams;

                pIecpState->CcmState.DW1.C0          = p1DLutParams->pCCM[0];
                pIecpState->CcmState.DW0.C1          = MOS_BITFIELD_VALUE((uint32_t)p1DLutParams->pCCM[1], 27);
                pIecpState->CcmState.DW3.C2          = MOS_BITFIELD_VALUE((uint32_t)p1DLutParams->pCCM[2], 27);
                pIecpState->CcmState.DW2.C3          = MOS_BITFIELD_VALUE((uint32_t)p1DLutParams->pCCM[3], 27);
                pIecpState->CcmState.DW5.C4          = p1DLutParams->pCCM[4];
                pIecpState->CcmState.DW4.C5          = MOS_BITFIELD_VALUE((uint32_t)p1DLutParams->pCCM[5], 27);
                pIecpState->CcmState.DW7.C6          = MOS_BITFIELD_VALUE((uint32_t)p1DLutParams->pCCM[6], 27);
                pIecpState->CcmState.DW6.C7          = MOS_BITFIELD_VALUE((uint32_t)p1DLutParams->pCCM[7], 27);
                pIecpState->CcmState.DW8.C8          = p1DLutParams->pCCM[8];
                pIecpState->CcmState.DW9.OffsetInR   = p1DLutParams->pCCM[9];
                pIecpState->CcmState.DW10.OffsetInG  = p1DLutParams->pCCM[10];
                pIecpState->CcmState.DW11.OffsetInB  = p1DLutParams->pCCM[11];
                pIecpState->CcmState.DW12.OffsetOutR = p1DLutParams->pCCM[12];
                pIecpState->CcmState.DW13.OffsetOutG = p1DLutParams->pCCM[13];
                pIecpState->CcmState.DW14.OffsetOutB = p1DLutParams->pCCM[14];

                pGamutState->DW0.GlobalModeEnable = false;
                // Still need to set CSC params here
                VeboxInterface_BT2020YUVToRGB(m_veboxHeap, pVeboxIecpParams, pVeboxGamutParams);

                return eStatus;
            }

            pGamutState->DW0.GlobalModeEnable = true;
            pGamutState->DW1.CmW              = 1023;  // Colorimetric accurate image
            if (pVeboxGamutParams->dstColorSpace == MHW_CSpace_BT601)
            {
                pGamutState->DW1.C0 = 116420;
                pGamutState->DW0.C1 = MOS_BITFIELD_VALUE((uint32_t)-45094, 21);
                pGamutState->DW3.C2 = MOS_BITFIELD_VALUE((uint32_t)-5785, 21);
                pGamutState->DW2.C3 = MOS_BITFIELD_VALUE((uint32_t)-10586, 21);
                pGamutState->DW5.C4 = 77814;
                pGamutState->DW4.C5 = MOS_BITFIELD_VALUE((uint32_t)-1705, 21);
                pGamutState->DW7.C6 = MOS_BITFIELD_VALUE((uint32_t)-1036, 21);
                pGamutState->DW6.C7 = MOS_BITFIELD_VALUE((uint32_t)-6284, 21);
                pGamutState->DW8.C8 = 72864;
            }
            else  //BT709, sRGB has same chromaticity CIE 1931
            {
                pGamutState->DW1.C0 = 108190;
                pGamutState->DW0.C1 = MOS_BITFIELD_VALUE((uint32_t)-38288, 21);
                pGamutState->DW3.C2 = MOS_BITFIELD_VALUE((uint32_t)-4747, 21);
                pGamutState->DW2.C3 = MOS_BITFIELD_VALUE((uint32_t)-7967, 21);
                pGamutState->DW5.C4 = 74174;
                pGamutState->DW4.C5 = MOS_BITFIELD_VALUE((uint32_t)-557, 21);
                pGamutState->DW7.C6 = MOS_BITFIELD_VALUE((uint32_t)-1198, 21);
                pGamutState->DW6.C7 = MOS_BITFIELD_VALUE((uint32_t)-6587, 21);
                pGamutState->DW8.C8 = 73321;
            }

            for (i = 0; i < 256; i++)
            {
                usGE_Values[i][0] = (uint16_t)g_Vebox_BT2020_Inverse_Pixel_Value[i];
                usGE_Values[i][1] =
                    usGE_Values[i][2] =
                        usGE_Values[i][3] = (uint16_t)g_Vebox_BT2020_Inverse_Gamma_LUT[i];

                usGE_Values[i][4] = (uint16_t)g_Vebox_BT2020_Forward_Pixel_Value[i];
                usGE_Values[i][5] =
                    usGE_Values[i][6] =
                        usGE_Values[i][7] = (uint16_t)g_Vebox_BT2020_Forward_Gamma_LUT[i];
            }
            // Copy two UNT16 to one DW(UNT32).
            MOS_SecureMemcpy(pVeboxGEGammaCorrection, sizeof(uint32_t) * 1024, usGE_Values, sizeof(uint16_t) * 8 * 256);
            // Back end CSC setting, need to convert BT2020 YUV input to RGB before GE
            VeboxInterface_BT2020YUVToRGB(m_veboxHeap, pVeboxIecpParams, pVeboxGamutParams);
        }
        else if (pVeboxIecpParams && pVeboxIecpParams->s1DLutParams.bActive && (pVeboxIecpParams->s1DLutParams.LUTSize == 256))
        {
            uint16_t  in_val = 0, vchan1_y = 0, vchan2_u = 0, vchan3_v = 0;
            uint32_t  nIndex        = 0;
            uint16_t *pForwardGamma = (uint16_t *)pVeboxIecpParams->s1DLutParams.p1DLUT;
            MHW_CHK_NULL_RETURN(pForwardGamma);

            // Gamut Expansion setting
            pGamutState->DW0.GlobalModeEnable = true;
            pGamutState->DW1.CmW              = 1023;
            dInverseGamma                     = 1.0;

            for (uint32_t i = 0; i < pVeboxIecpParams->s1DLutParams.LUTSize; i++)
            {
                usGE_Values[i][0] = 257 * i;
                usGE_Values[i][1] =
                    usGE_Values[i][2] =
                        usGE_Values[i][3] = 257 * i;

                nIndex   = 4 * i;
                in_val   = pForwardGamma[nIndex];
                vchan1_y = pForwardGamma[nIndex + 1];
                vchan2_u = pForwardGamma[nIndex + 2];
                vchan3_v = pForwardGamma[nIndex + 3];

                // ayuv: in_val, vchan1_y, vchan2_u, vchan3_v
                usGE_Values[i][4] = (i == 0) ? 0 : ((i == 255) ? 0xffff : in_val);
                usGE_Values[i][5] = vchan1_y;
                usGE_Values[i][6] = vchan2_u;
                usGE_Values[i][7] = vchan3_v;
            }
            pGamutState->DW1.C0          = 65536;
            pGamutState->DW0.C1          = 0;
            pGamutState->DW3.C2          = 0;
            pGamutState->DW2.C3          = 0;
            pGamutState->DW5.C4          = 65536;
            pGamutState->DW4.C5          = 0;
            pGamutState->DW7.C6          = 0;
            pGamutState->DW6.C7          = 0;
            pGamutState->DW8.C8          = 65536;
            pGamutState->DW9.OffsetInR   = 0;
            pGamutState->DW10.OffsetInG  = 0;
            pGamutState->DW11.OffsetInB  = 0;
            pGamutState->DW12.OffsetOutR = 0;
            pGamutState->DW13.OffsetOutG = 0;
            pGamutState->DW14.OffsetOutB = 0;
            // Copy two uint16_t to one DW (UNT32).
            MOS_SecureMemcpy(pVeboxGEGammaCorrection, sizeof(uint32_t) * 1024, usGE_Values, sizeof(uint16_t) * 8 * 256);
        }
        else
        {
            MHW_ASSERTMESSAGE("Unknown branch!");
        }

        return eStatus;
    }

    MOS_STATUS ForceGNEParams(uint8_t *pDndiSate)
    {
        MHW_FUNCTION_ENTER;
        MHW_CHK_NULL_RETURN(pDndiSate);
        mhw::vebox::xe_lpm_plus_next::Cmd::VEBOX_DNDI_STATE_CMD *pVeboxDndiState = (mhw::vebox::xe_lpm_plus_next::Cmd::VEBOX_DNDI_STATE_CMD *)pDndiSate;

        //used by both SGNE and TGNE
        pVeboxDndiState->DW6.BlockNoiseEstimateEdgeThreshold  = 900;
        pVeboxDndiState->DW4.BlockNoiseEstimateNoiseThreshold = 720;
        pVeboxDndiState->DW30.EightDirectionEdgeThreshold     = 1800;

        //SGNE
        pVeboxDndiState->DW31.LargeSobelThreshold       = 1290;
        pVeboxDndiState->DW33.MaxSobelThreshold         = 1440;
        pVeboxDndiState->DW31.SmallSobelThreshold       = 480;
        pVeboxDndiState->DW32.BlockSigmaDiffThreshold   = dwBSDThreshold;
        pVeboxDndiState->DW31.SmallSobelCountThreshold  = 6;
        pVeboxDndiState->DW32.LargeSobelCountThreshold  = 6;
        pVeboxDndiState->DW32.MedianSobelCountThreshold = 40;

        //TGNE
        pVeboxDndiState->DW50.LumaUniformityLowTh1  = 1;
        pVeboxDndiState->DW50.LumaUniformityLowTh2  = 1;
        pVeboxDndiState->DW50.LumaUniformityHighTh1 = 6;
        pVeboxDndiState->DW50.LumaUniformityHighTh2 = 0;
        pVeboxDndiState->DW49.LumaStadTh            = 250;

        //Chroma
        pVeboxDndiState->DW8.ChromaDenoiseMovingPixelThreshold               = 2;  //m_chromaParams.dwHotPixelThresholdChromaV;
        pVeboxDndiState->DW8.ChromaDenoiseAsdThreshold                       = 512;
        pVeboxDndiState->DW8.ChromaDenoiseThresholdForSumOfComplexityMeasure = 512;

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS SetVeboxTGNEDndiState(uint8_t *pDndiSate)
    {
        MHW_FUNCTION_ENTER;
        MHW_CHK_NULL_RETURN(pDndiSate);
        mhw::vebox::xe_lpm_plus_next::Cmd::VEBOX_DNDI_STATE_CMD *pVeboxDndiState = (mhw::vebox::xe_lpm_plus_next::Cmd::VEBOX_DNDI_STATE_CMD *)pDndiSate;

        if (bHVSAutoBdrateEnable)
        {
            pVeboxDndiState->DW3.TemporalGneEnable              = bTGNEEnable;
            pVeboxDndiState->DW30.ValidPixelThreshold           = 336;
            pVeboxDndiState->DW52._4X4TemporalGneThresholdCount = dw4X4TGNEThCnt;
            pVeboxDndiState->DW2.InitialDenoiseHistory          = dwHistoryInit;
            pVeboxDndiState->DW33.MaxSobelThreshold             = 448;  //for SGNE
            //for chroma
            pVeboxDndiState->DW49.ChromaStadTh            = dwChromaStadTh;
            pVeboxDndiState->DW51.ChromaUniformityHighTh2 = 0;
            pVeboxDndiState->DW51.ChromaUniformityHighTh1 = 9;
            pVeboxDndiState->DW51.ChromaUniformityLowTh2  = 2;
            pVeboxDndiState->DW51.ChromaUniformityLowTh1  = 1;

            ForceGNEParams((uint8_t *)pVeboxDndiState);
            pVeboxDndiState->DW2.InitialDenoiseHistory = dwHistoryInit;
        }
        else if (bHVSAutoSubjectiveEnable)
        {
            pVeboxDndiState->DW3.TemporalGneEnable                = bTGNEEnable;
            pVeboxDndiState->DW4.BlockNoiseEstimateNoiseThreshold = 720;
            pVeboxDndiState->DW6.BlockNoiseEstimateEdgeThreshold  = 200;
            pVeboxDndiState->DW30.EightDirectionEdgeThreshold     = 3200;
            pVeboxDndiState->DW30.ValidPixelThreshold             = 336;
            pVeboxDndiState->DW33.MaxSobelThreshold               = 1440;
            pVeboxDndiState->DW49.ChromaStadTh                    = dwChromaStadTh;
            pVeboxDndiState->DW49.LumaStadTh                      = dwLumaStadTh;
            pVeboxDndiState->DW50.LumaUniformityHighTh2           = 50;
            pVeboxDndiState->DW50.LumaUniformityHighTh1           = 15;
            pVeboxDndiState->DW50.LumaUniformityLowTh2            = 2;
            pVeboxDndiState->DW50.LumaUniformityLowTh1            = 2;
            pVeboxDndiState->DW51.ChromaUniformityHighTh2         = 30;
            pVeboxDndiState->DW51.ChromaUniformityHighTh1         = 15;
            pVeboxDndiState->DW51.ChromaUniformityLowTh2          = 2;
            pVeboxDndiState->DW51.ChromaUniformityLowTh1          = 1;
            pVeboxDndiState->DW52._4X4TemporalGneThresholdCount   = dw4X4TGNEThCnt;
        }
        else
        {
            pVeboxDndiState->DW3.TemporalGneEnable                = bTGNEEnable;
            pVeboxDndiState->DW4.BlockNoiseEstimateNoiseThreshold = 320;
            pVeboxDndiState->DW49.ChromaStadTh                    = dwChromaStadTh;
            pVeboxDndiState->DW49.LumaStadTh                      = dwLumaStadTh;
            pVeboxDndiState->DW50.LumaUniformityHighTh2           = 50;
            pVeboxDndiState->DW50.LumaUniformityHighTh1           = 10;
            pVeboxDndiState->DW50.LumaUniformityLowTh2            = 2;
            pVeboxDndiState->DW50.LumaUniformityLowTh1            = 1;
            pVeboxDndiState->DW51.ChromaUniformityHighTh2         = 30;
            pVeboxDndiState->DW51.ChromaUniformityHighTh1         = 15;
            pVeboxDndiState->DW51.ChromaUniformityLowTh2          = 2;
            pVeboxDndiState->DW51.ChromaUniformityLowTh1          = 1;
            pVeboxDndiState->DW52._4X4TemporalGneThresholdCount   = dw4X4TGNEThCnt;
        }
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS SetVeboxSGNEDndiState(uint8_t *pDndiSate)
    {
        MHW_FUNCTION_ENTER;
        MHW_CHK_NULL_RETURN(pDndiSate);
        mhw::vebox::xe_lpm_plus_next::Cmd::VEBOX_DNDI_STATE_CMD *pVeboxDndiState = (mhw::vebox::xe_lpm_plus_next::Cmd::VEBOX_DNDI_STATE_CMD *)pDndiSate;

        if (bHVSAutoBdrateEnable)
        {
            pVeboxDndiState->DW3.TemporalGneEnable     = 0;
            pVeboxDndiState->DW30.ValidPixelThreshold  = 336;
            pVeboxDndiState->DW33.MaxSobelThreshold    = 448;
            pVeboxDndiState->DW2.InitialDenoiseHistory = dwHistoryInit;

            pVeboxDndiState->DW49.ChromaStadTh                  = 0;
            pVeboxDndiState->DW51.ChromaUniformityHighTh2       = 0;
            pVeboxDndiState->DW51.ChromaUniformityHighTh1       = 0;
            pVeboxDndiState->DW51.ChromaUniformityLowTh2        = 0;
            pVeboxDndiState->DW51.ChromaUniformityLowTh1        = 0;
            pVeboxDndiState->DW52._4X4TemporalGneThresholdCount = 0;

            ForceGNEParams((uint8_t *)pVeboxDndiState);

            pVeboxDndiState->DW49.LumaStadTh            = 0;
            pVeboxDndiState->DW50.LumaUniformityHighTh2 = 0;
            pVeboxDndiState->DW50.LumaUniformityHighTh1 = 0;
            pVeboxDndiState->DW50.LumaUniformityLowTh2  = 0;
            pVeboxDndiState->DW50.LumaUniformityLowTh1  = 0;
        }
        else if (bHVSAutoSubjectiveEnable)
        {
            pVeboxDndiState->DW3.TemporalGneEnable                = 0;
            pVeboxDndiState->DW4.BlockNoiseEstimateNoiseThreshold = 720;
            pVeboxDndiState->DW6.BlockNoiseEstimateEdgeThreshold  = 200;
            pVeboxDndiState->DW30.EightDirectionEdgeThreshold     = 3200;
            pVeboxDndiState->DW30.ValidPixelThreshold             = 336;
            pVeboxDndiState->DW33.MaxSobelThreshold               = 1440;
            pVeboxDndiState->DW49.ChromaStadTh                    = 0;
            pVeboxDndiState->DW49.LumaStadTh                      = 0;
            pVeboxDndiState->DW50.LumaUniformityHighTh2           = 0;
            pVeboxDndiState->DW50.LumaUniformityHighTh1           = 0;
            pVeboxDndiState->DW50.LumaUniformityLowTh2            = 0;
            pVeboxDndiState->DW50.LumaUniformityLowTh1            = 0;
            pVeboxDndiState->DW51.ChromaUniformityHighTh2         = 0;
            pVeboxDndiState->DW51.ChromaUniformityHighTh1         = 0;
            pVeboxDndiState->DW51.ChromaUniformityLowTh2          = 0;
            pVeboxDndiState->DW51.ChromaUniformityLowTh1          = 0;
            pVeboxDndiState->DW52._4X4TemporalGneThresholdCount   = 0;
        }
        else
        {
            pVeboxDndiState->DW3.TemporalGneEnable                = 0;
            pVeboxDndiState->DW4.BlockNoiseEstimateNoiseThreshold = 320;
            pVeboxDndiState->DW49.ChromaStadTh                    = 0;
            pVeboxDndiState->DW49.LumaStadTh                      = 0;
            pVeboxDndiState->DW50.LumaUniformityHighTh2           = 0;
            pVeboxDndiState->DW50.LumaUniformityHighTh1           = 0;
            pVeboxDndiState->DW50.LumaUniformityLowTh2            = 0;
            pVeboxDndiState->DW50.LumaUniformityLowTh1            = 0;
            pVeboxDndiState->DW51.ChromaUniformityHighTh2         = 0;
            pVeboxDndiState->DW51.ChromaUniformityHighTh1         = 0;
            pVeboxDndiState->DW51.ChromaUniformityLowTh2          = 0;
            pVeboxDndiState->DW51.ChromaUniformityLowTh1          = 0;
            pVeboxDndiState->DW52._4X4TemporalGneThresholdCount   = 0;
        }

        return MOS_STATUS_SUCCESS;
    }

MOS_STATUS DumpDNDIStates(uint8_t *pDndiSate)
    {
        MHW_FUNCTION_ENTER;
        MHW_CHK_NULL_RETURN(pDndiSate);
        mhw::vebox::xe_lpm_plus_next::Cmd::VEBOX_DNDI_STATE_CMD *pVeboxDndiState = (mhw::vebox::xe_lpm_plus_next::Cmd::VEBOX_DNDI_STATE_CMD *)pDndiSate;

        MHW_VERBOSEMESSAGE("VeboxDndiState, DW 34-47 is DI related DW, others is DN.");
        MHW_VERBOSEMESSAGE("DW0:DenoiseMaximumHistory %d, DenoiseStadThreshold %d", pVeboxDndiState->DW0.DenoiseMaximumHistory, pVeboxDndiState->DW0.DenoiseStadThreshold);
        MHW_VERBOSEMESSAGE("DW1:DenoiseAsdThreshold %d, DenoiseHistoryIncrease %d, DenoiseMovingPixelThreshold %d", pVeboxDndiState->DW1.DenoiseAsdThreshold, pVeboxDndiState->DW1.DenoiseHistoryIncrease, pVeboxDndiState->DW1.DenoiseMovingPixelThreshold);
        MHW_VERBOSEMESSAGE("DW2:InitialDenoiseHistory %d, TemporalDifferenceThreshold %d", pVeboxDndiState->DW2.InitialDenoiseHistory, pVeboxDndiState->DW2.TemporalDifferenceThreshold);
        MHW_VERBOSEMESSAGE("DW3:HotPixelCountLuma %d, LowTemporalDifferenceThreshold %d, ProgressiveDn %d, TemporalGneEnable %d", pVeboxDndiState->DW3.HotPixelCountLuma, pVeboxDndiState->DW3.LowTemporalDifferenceThreshold, pVeboxDndiState->DW3.ProgressiveDn, pVeboxDndiState->DW3.TemporalGneEnable);
        MHW_VERBOSEMESSAGE("DW4:BlockNoiseEstimateNoiseThreshold %d, DenoiseThresholdForSumOfComplexityMeasureLuma %d, DW4.HotPixelThresholdLuma %d", pVeboxDndiState->DW4.BlockNoiseEstimateNoiseThreshold, pVeboxDndiState->DW4.DenoiseThresholdForSumOfComplexityMeasureLuma, pVeboxDndiState->DW4.HotPixelThresholdLuma);
        MHW_VERBOSEMESSAGE("DW5:ChromaDenoiseStadThreshold %d, HotPixelCountChromaU %d, HotPixelThresholdChromaU %d", pVeboxDndiState->DW5.ChromaDenoiseStadThreshold, pVeboxDndiState->DW5.HotPixelCountChromaU, pVeboxDndiState->DW5.HotPixelThresholdChromaU);
        MHW_VERBOSEMESSAGE("DW6:BlockNoiseEstimateEdgeThreshold %d, ChromaDenoiseEnable %d, ChromaTemporalDifferenceThreshold %d", pVeboxDndiState->DW6.BlockNoiseEstimateEdgeThreshold, pVeboxDndiState->DW6.ChromaDenoiseEnable, pVeboxDndiState->DW6.ChromaTemporalDifferenceThreshold);
        MHW_VERBOSEMESSAGE("DW7:ChromaLowTemporalDifferenceThreshold %d, HotPixelCountChromaV %d, HotPixelThresholdChromaV %d", pVeboxDndiState->DW7.ChromaLowTemporalDifferenceThreshold, pVeboxDndiState->DW7.HotPixelCountChromaV, pVeboxDndiState->DW7.HotPixelThresholdChromaV);
        MHW_VERBOSEMESSAGE("DW8:ChromaDenoiseAsdThreshold %d, ChromaDenoiseMovingPixelThreshold %d, ChromaDenoiseThresholdForSumOfComplexityMeasure %d", pVeboxDndiState->DW8.ChromaDenoiseAsdThreshold, pVeboxDndiState->DW8.ChromaDenoiseMovingPixelThreshold, pVeboxDndiState->DW8.ChromaDenoiseThresholdForSumOfComplexityMeasure);
        MHW_VERBOSEMESSAGE("DW9:DnyWr040 %d, DnyWr140 %d, DnyWr240 %d, DnyWr340 %d, DnyWr440 %d, DnyWr540 %d", pVeboxDndiState->DW9.DnyWr040, pVeboxDndiState->DW9.DnyWr140, pVeboxDndiState->DW9.DnyWr240, pVeboxDndiState->DW9.DnyWr340, pVeboxDndiState->DW9.DnyWr440, pVeboxDndiState->DW9.DnyWr540);
        MHW_VERBOSEMESSAGE("DW10:DnyThmax120 %d, DnyThmin120 %d, DW11: DnyDynThmin120 %d, DnyPrt5120 %d", pVeboxDndiState->DW10.DnyThmax120, pVeboxDndiState->DW10.DnyThmin120, pVeboxDndiState->DW11.DnyDynThmin120, pVeboxDndiState->DW11.DnyPrt5120);
        MHW_VERBOSEMESSAGE("DW12:DnyPrt3120 %d, DnyPrt4120 %d, DW13:DnyPrt1120 %d, DnyPrt2120 %d", pVeboxDndiState->DW12.DnyPrt3120, pVeboxDndiState->DW12.DnyPrt4120, pVeboxDndiState->DW13.DnyPrt1120, pVeboxDndiState->DW13.DnyPrt2120);
        MHW_VERBOSEMESSAGE("DW14:DnyPrt0120 %d, DnyWd2040 %d, DnyWd2140 %d, DnyWd2240 %d", pVeboxDndiState->DW14.DnyPrt0120, pVeboxDndiState->DW14.DnyWd2040, pVeboxDndiState->DW14.DnyWd2140, pVeboxDndiState->DW14.DnyWd2240);
        MHW_VERBOSEMESSAGE("DW15:DnyWd0040 %d, DnyWd0140 %d, DnyWd0240 %d, DnyWd1040 %d, DnyWd1140 %d, DnyWd1240 %d", pVeboxDndiState->DW15.DnyWd0040, pVeboxDndiState->DW15.DnyWd0140, pVeboxDndiState->DW15.DnyWd0240, pVeboxDndiState->DW15.DnyWd1040, pVeboxDndiState->DW15.DnyWd1140, pVeboxDndiState->DW15.DnyWd1240);
        MHW_VERBOSEMESSAGE("DW16:DnuWr040 %d, DnuWr140 %d, DnuWr240 %d, DnuWr340 %d, DnuWr440 %d, DnuWr540 %d", pVeboxDndiState->DW16.DnuWr040, pVeboxDndiState->DW16.DnuWr140, pVeboxDndiState->DW16.DnuWr240, pVeboxDndiState->DW16.DnuWr340, pVeboxDndiState->DW16.DnuWr440, pVeboxDndiState->DW16.DnuWr540);
        MHW_VERBOSEMESSAGE("DW17:DnuThmax120 %d, DnuThmin120 %d", pVeboxDndiState->DW17.DnuThmax120, pVeboxDndiState->DW17.DnuThmin120);
        MHW_VERBOSEMESSAGE("DW18:DnuDynThmin120 %d, DnuPrt5120 %d", pVeboxDndiState->DW18.DnuDynThmin120, pVeboxDndiState->DW18.DnuPrt5120);
        MHW_VERBOSEMESSAGE("DW19:DnuPrt3120 %d, DnuPrt4120 %d, DW20:DnuPrt1120 %d, DnuPrt2120 %d", pVeboxDndiState->DW19.DnuPrt3120, pVeboxDndiState->DW19.DnuPrt4120, pVeboxDndiState->DW20.DnuPrt1120, pVeboxDndiState->DW20.DnuPrt2120);
        MHW_VERBOSEMESSAGE("DW21:DnuPrt0120 %d, DnuWd2040 %d, DnuWd2140 %d, DnuWd2240 %d", pVeboxDndiState->DW21.DnuPrt0120, pVeboxDndiState->DW21.DnuWd2040, pVeboxDndiState->DW21.DnuWd2140, pVeboxDndiState->DW21.DnuWd2240);
        MHW_VERBOSEMESSAGE("DW22:DnuWd0040 %d, DnuWd0140 %d, DnuWd0240 %d, DnuWd1040 %d, DnuWd1140 %d, DnuWd1240 %d", pVeboxDndiState->DW22.DnuWd0040, pVeboxDndiState->DW22.DnuWd0140, pVeboxDndiState->DW22.DnuWd0240, pVeboxDndiState->DW22.DnuWd1040, pVeboxDndiState->DW22.DnuWd1140, pVeboxDndiState->DW22.DnuWd1240);
        MHW_VERBOSEMESSAGE("DW23:DnvWr040 %d, DnvWr240 %d, DnvWr340 %d, DnvWr440 %d, DnvWr5140 %d, DnvWr540", pVeboxDndiState->DW23.DnvWr040, pVeboxDndiState->DW23.DnvWr240, pVeboxDndiState->DW23.DnvWr340, pVeboxDndiState->DW23.DnvWr440, pVeboxDndiState->DW23.DnvWr5140, pVeboxDndiState->DW23.DnvWr540);
        MHW_VERBOSEMESSAGE("DW24:DnvThmax120 %d, DnvThmin120 %d, DW25:DnvDynThmin120 %d, DnvPrt5120 %d", pVeboxDndiState->DW24.DnvThmax120, pVeboxDndiState->DW24.DnvThmin120, pVeboxDndiState->DW25.DnvDynThmin120, pVeboxDndiState->DW25.DnvPrt5120);
        MHW_VERBOSEMESSAGE("DW26:DnvPrt3120 %d, DnvPrt4120 %d, DW27:DnvPrt1120 %d, DnvPrt2120 %d", pVeboxDndiState->DW26.DnvPrt3120, pVeboxDndiState->DW26.DnvPrt4120, pVeboxDndiState->DW27.DnvPrt1120, pVeboxDndiState->DW27.DnvPrt2120);
        MHW_VERBOSEMESSAGE("DW28:DnvPrt0120 %d, DnvWd2040 %d, DnvWd2140 %d, DnvWd2240 %d", pVeboxDndiState->DW28.DnvPrt0120, pVeboxDndiState->DW28.DnvWd2040, pVeboxDndiState->DW28.DnvWd2140, pVeboxDndiState->DW28.DnvWd2240);
        MHW_VERBOSEMESSAGE("DW29:DnvWd0040 %d, DnvWd0140 %d, DnvWd0240 %d, DnvWd1040 %d, DnvWd1140 %d, DnvWd1240 %d", pVeboxDndiState->DW29.DnvWd0040, pVeboxDndiState->DW29.DnvWd0140, pVeboxDndiState->DW29.DnvWd0240, pVeboxDndiState->DW29.DnvWd1040, pVeboxDndiState->DW29.DnvWd1140, pVeboxDndiState->DW29.DnvWd1240);
        MHW_VERBOSEMESSAGE("DW30:EightDirectionEdgeThreshold %d, ValidPixelThreshold %d", pVeboxDndiState->DW30.EightDirectionEdgeThreshold, pVeboxDndiState->DW30.ValidPixelThreshold);
        MHW_VERBOSEMESSAGE("DW31:LargeSobelThreshold %d, SmallSobelCountThreshold %d, SmallSobelThreshold %d", pVeboxDndiState->DW31.LargeSobelThreshold, pVeboxDndiState->DW31.SmallSobelCountThreshold, pVeboxDndiState->DW31.SmallSobelThreshold);
        MHW_VERBOSEMESSAGE("DW32:BlockSigmaDiffThreshold %d, LargeSobelCountThreshold %d, MedianSobelCountThreshold %d, DW33:MaxSobelThreshold %d", pVeboxDndiState->DW32.BlockSigmaDiffThreshold, pVeboxDndiState->DW32.LargeSobelCountThreshold, pVeboxDndiState->DW32.MedianSobelCountThreshold, pVeboxDndiState->DW33.MaxSobelThreshold);
        MHW_VERBOSEMESSAGE("DW34:SmoothMvThreshold %d, SadTightThreshold %d, ContentAdaptiveThresholdSlope %d, StmmC2 %d, SignBitForSmoothMvThreshold %d, SignBitForMaximumStmm %d, SignBitForMinimumStmm %d, Reserved1104 %d",
            pVeboxDndiState->DW34.SmoothMvThreshold,
            pVeboxDndiState->DW34.SadTightThreshold,
            pVeboxDndiState->DW34.ContentAdaptiveThresholdSlope,
            pVeboxDndiState->DW34.StmmC2,
            pVeboxDndiState->DW34.SignBitForSmoothMvThreshold,
            pVeboxDndiState->DW34.SignBitForMaximumStmm,
            pVeboxDndiState->DW34.SignBitForMinimumStmm,
            pVeboxDndiState->DW34.Reserved1104);
        MHW_VERBOSEMESSAGE("DW35:MaximumStmm %d, MultiplierForVecm %d, Reserved1134 %d, BlendingConstantAcrossTimeForSmallValuesOfStmm %d, BlendingConstantAcrossTimeForLargeValuesOfStmm %d, StmmBlendingConstantSelect %d",
            pVeboxDndiState->DW35.MaximumStmm,
            pVeboxDndiState->DW35.MultiplierForVecm,
            pVeboxDndiState->DW35.Reserved1134,
            pVeboxDndiState->DW35.BlendingConstantAcrossTimeForSmallValuesOfStmm,
            pVeboxDndiState->DW35.BlendingConstantAcrossTimeForLargeValuesOfStmm,
            pVeboxDndiState->DW35.StmmBlendingConstantSelect);
        MHW_VERBOSEMESSAGE("DW36:FmdTemporalDifferenceThreshold %d, LumatdmWt %d, ChromatdmWt %d, StmmOutputShift %d, StmmShiftUp %d, StmmShiftDown %d, MinimumStmm %d",
            pVeboxDndiState->DW36.FmdTemporalDifferenceThreshold,
            pVeboxDndiState->DW36.LumatdmWt,
            pVeboxDndiState->DW36.ChromatdmWt,
            pVeboxDndiState->DW36.StmmOutputShift,
            pVeboxDndiState->DW36.StmmShiftUp,
            pVeboxDndiState->DW36.StmmShiftDown,
            pVeboxDndiState->DW36.MinimumStmm);
        MHW_VERBOSEMESSAGE("DW37:CoringThresholdForSvcm %d, DeltabitValueForSvcm %d, Reserved1196 %d, CoringThresholdForShcm %d, DeltabitValueForShcm %d, Reserved1212 %d",
            pVeboxDndiState->DW37.CoringThresholdForSvcm,
            pVeboxDndiState->DW37.DeltabitValueForSvcm,
            pVeboxDndiState->DW37.Reserved1196,
            pVeboxDndiState->DW37.CoringThresholdForShcm,
            pVeboxDndiState->DW37.DeltabitValueForShcm,
            pVeboxDndiState->DW37.Reserved1212);
        MHW_VERBOSEMESSAGE("DW38:Reserved1216 %d, DnDiTopFirst %d, Reserved1220 %d, McdiEnable %d, FmdTearThreshold %d, CatThreshold %d, Fmd2VerticalDifferenceThreshold %d, Fmd1VerticalDifferenceThreshold %d",
            pVeboxDndiState->DW38.Reserved1216,
            pVeboxDndiState->DW38.DnDiTopFirst,
            pVeboxDndiState->DW38.Reserved1220,
            pVeboxDndiState->DW38.McdiEnable,
            pVeboxDndiState->DW38.FmdTearThreshold,
            pVeboxDndiState->DW38.CatThreshold,
            pVeboxDndiState->DW38.Fmd2VerticalDifferenceThreshold,
            pVeboxDndiState->DW38.Fmd1VerticalDifferenceThreshold);
        MHW_VERBOSEMESSAGE("DW39:SadTha %d, SadThb %d, ProgressiveCadenceReconstructionFor1StFieldOfCurrentFrame %d, McPixelConsistencyThreshold %d, ProgressiveCadenceReconstructionForSecondFieldOfPreviousFrame %d, Reserved1266 %d, NeighborPixelThreshold %d, ChromaSmallerWindowForTdm %d, LumaSmallerWindowForTdm %d, Fastercovergence %d, Reserved1274 %d",
            pVeboxDndiState->DW39.SadTha,
            pVeboxDndiState->DW39.SadThb,
            pVeboxDndiState->DW39.ProgressiveCadenceReconstructionFor1StFieldOfCurrentFrame,
            pVeboxDndiState->DW39.McPixelConsistencyThreshold,
            pVeboxDndiState->DW39.ProgressiveCadenceReconstructionForSecondFieldOfPreviousFrame,
            pVeboxDndiState->DW39.Reserved1266,
            pVeboxDndiState->DW39.NeighborPixelThreshold,
            pVeboxDndiState->DW39.ChromaSmallerWindowForTdm,
            pVeboxDndiState->DW39.LumaSmallerWindowForTdm,
            pVeboxDndiState->DW39.Fastercovergence,
            pVeboxDndiState->DW39.Reserved1274);
        MHW_VERBOSEMESSAGE("DW40:SadWt0 %d, SadWt1 %d, SadWt2 %d, SadWt3 %d", pVeboxDndiState->DW40.SadWt0, pVeboxDndiState->DW40.SadWt1, pVeboxDndiState->DW40.SadWt2, pVeboxDndiState->DW40.SadWt3);
        MHW_VERBOSEMESSAGE("DW41:SadWt4 %d, SadWt6 %d, CoringThresholdForLumaSadCalculation %d, CoringThresholdForChromaSadCalculation %d", pVeboxDndiState->DW41.SadWt4, pVeboxDndiState->DW41.SadWt6, pVeboxDndiState->DW41.CoringThresholdForLumaSadCalculation, pVeboxDndiState->DW41.CoringThresholdForChromaSadCalculation);
        MHW_VERBOSEMESSAGE("DW42:ParDiffcheckslackthreshold %d, ParTearinghighthreshold %d, ParTearinglowthreshold %d, ParDirectioncheckth %d, ParSyntheticcontentcheck %d, ParLocalcheck %d, ParUsesyntheticcontentmedian %d, BypassDeflicker %d, Reserved1375 %d",
            pVeboxDndiState->DW42.ParDiffcheckslackthreshold,
            pVeboxDndiState->DW42.ParTearinghighthreshold,
            pVeboxDndiState->DW42.ParTearinglowthreshold,
            pVeboxDndiState->DW42.ParDirectioncheckth,
            pVeboxDndiState->DW42.ParSyntheticcontentcheck,
            pVeboxDndiState->DW42.ParLocalcheck,
            pVeboxDndiState->DW42.ParUsesyntheticcontentmedian,
            pVeboxDndiState->DW42.BypassDeflicker,
            pVeboxDndiState->DW42.Reserved1375);
        MHW_VERBOSEMESSAGE("DW43:Lpfwtlut0 %d, Lpfwtlut1 %d, Lpfwtlut2 %d, Lpfwtlut3 %d", pVeboxDndiState->DW43.Lpfwtlut0, pVeboxDndiState->DW43.Lpfwtlut1, pVeboxDndiState->DW43.Lpfwtlut2, pVeboxDndiState->DW43.Lpfwtlut3);
        MHW_VERBOSEMESSAGE("DW44:Lpfwtlut4 %d, Lpfwtlut5 %d, Lpfwtlut6 %d, Lpfwtlut7 %d", pVeboxDndiState->DW44.Lpfwtlut4, pVeboxDndiState->DW44.Lpfwtlut5, pVeboxDndiState->DW44.Lpfwtlut6, pVeboxDndiState->DW44.Lpfwtlut7);
        MHW_VERBOSEMESSAGE("DW45:TdmUvThreshold %d, HvUvThreshold %d, TdmHarmonicFactorSynthetic %d, TdmHarmonicFactorNatural %d, SynthticFrame %d, SyntheticContentThreshold %d", pVeboxDndiState->DW45.TdmUvThreshold, pVeboxDndiState->DW45.HvUvThreshold, pVeboxDndiState->DW45.TdmHarmonicFactorSynthetic, pVeboxDndiState->DW45.TdmHarmonicFactorNatural, pVeboxDndiState->DW45.SynthticFrame, pVeboxDndiState->DW45.SyntheticContentThreshold);
        MHW_VERBOSEMESSAGE("DW46:SvcmHarmonicFactorSynthetic %d, ShcmHarmonicFactorSynthetic %d, SvcmHarmonicFactorNatural %d, ShcmHarmonicFactorNatural %d, HarmonicCounterThreshold %d, MaxHarmonicCounterThreshold %d, NaturalContentThreshold %d, Reserved1501 %d",
            pVeboxDndiState->DW46.SvcmHarmonicFactorSynthetic,
            pVeboxDndiState->DW46.ShcmHarmonicFactorSynthetic,
            pVeboxDndiState->DW46.SvcmHarmonicFactorNatural,
            pVeboxDndiState->DW46.ShcmHarmonicFactorNatural,
            pVeboxDndiState->DW46.HarmonicCounterThreshold,
            pVeboxDndiState->DW46.MaxHarmonicCounterThreshold,
            pVeboxDndiState->DW46.NaturalContentThreshold,
            pVeboxDndiState->DW46.Reserved1501);
        MHW_VERBOSEMESSAGE("DW47:MaximumValue %d, DW48:ShiftingValue %d, HvYThreshold %d, NumInlinerNumeratorThreshold %d, NumInlinerDenominatorThreshold %d, Reserved1556 %d", pVeboxDndiState->DW47.MaximumValue, pVeboxDndiState->DW48.ShiftingValue, pVeboxDndiState->DW48.HvYThreshold, pVeboxDndiState->DW48.NumInlinerNumeratorThreshold, pVeboxDndiState->DW48.NumInlinerDenominatorThreshold, pVeboxDndiState->DW48.Reserved1556);
        MHW_VERBOSEMESSAGE("DW49:ChromaStadTh %d, LumaStadTh %d", pVeboxDndiState->DW49.ChromaStadTh, pVeboxDndiState->DW49.LumaStadTh);
        MHW_VERBOSEMESSAGE("DW50:LumaUniformityHighTh1 %d, LumaUniformityHighTh2 %d, LumaUniformityLowTh1 %d, LumaUniformityLowTh2 %d", pVeboxDndiState->DW50.LumaUniformityHighTh1, pVeboxDndiState->DW50.LumaUniformityHighTh2, pVeboxDndiState->DW50.LumaUniformityLowTh1, pVeboxDndiState->DW50.LumaUniformityLowTh2);
        MHW_VERBOSEMESSAGE("DW51: ChromaUniformityHighTh1 %d, ChromaUniformityHighTh2 %d, ChromaUniformityLowTh1 %d, ChromaUniformityLowTh2 %d", pVeboxDndiState->DW51.ChromaUniformityHighTh1, pVeboxDndiState->DW51.ChromaUniformityHighTh2, pVeboxDndiState->DW51.ChromaUniformityLowTh1, pVeboxDndiState->DW51.ChromaUniformityLowTh2);
        MHW_VERBOSEMESSAGE("DW52:_4X4TemporalGneThresholdCount %d", pVeboxDndiState->DW52._4X4TemporalGneThresholdCount);

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS SetVeboxDndiState(
            PMHW_VEBOX_DNDI_PARAMS pVeboxDndiParams) override
    {
        MHW_FUNCTION_ENTER;

        MHW_VEBOX_HEAP  *pVeboxHeap = nullptr;
        uint32_t        uiOffset   = 0;
        MOS_STATUS      eStatus    = MOS_STATUS_SUCCESS;

       mhw::vebox::xe_lpm_plus_next::Cmd::VEBOX_DNDI_STATE_CMD *pVeboxDndiState, mVeboxDndiState;

       MHW_CHK_NULL_RETURN(pVeboxDndiParams);
       MHW_CHK_NULL_RETURN(m_veboxHeap);
       pVeboxHeap = m_veboxHeap;

        uiOffset = pVeboxHeap->uiCurState * pVeboxHeap->uiInstanceSize;
        pVeboxDndiState =
            (mhw::vebox::xe_lpm_plus_next::Cmd::VEBOX_DNDI_STATE_CMD *)(pVeboxHeap->pLockedDriverResourceMem +
                                                            pVeboxHeap->uiDndiStateOffset +
                                                            uiOffset);

        MHW_CHK_NULL_RETURN(pVeboxDndiState);

        *pVeboxDndiState = mVeboxDndiState;

        pVeboxDndiState->DW0.DenoiseMaximumHistory                         = pVeboxDndiParams->dwDenoiseMaximumHistory;
        pVeboxDndiState->DW0.DenoiseStadThreshold                          = pVeboxDndiParams->dwDenoiseSTADThreshold;
        pVeboxDndiState->DW1.DenoiseAsdThreshold                           = pVeboxDndiParams->dwDenoiseASDThreshold;
        pVeboxDndiState->DW1.DenoiseHistoryIncrease                        = pVeboxDndiParams->dwDenoiseHistoryDelta;
        pVeboxDndiState->DW1.DenoiseMovingPixelThreshold                   = pVeboxDndiParams->dwDenoiseMPThreshold;
        pVeboxDndiState->DW2.TemporalDifferenceThreshold                   = pVeboxDndiParams->dwTDThreshold;
        pVeboxDndiState->DW3.LowTemporalDifferenceThreshold                = pVeboxDndiParams->dwLTDThreshold;
        pVeboxDndiState->DW3.ProgressiveDn                                 = pVeboxDndiParams->bProgressiveDN;
        pVeboxDndiState->DW3.HotPixelCountLuma                             = pVeboxDndiParams->dwHotPixelCount;
        pVeboxDndiState->DW4.DenoiseThresholdForSumOfComplexityMeasureLuma = pVeboxDndiParams->dwDenoiseSCMThreshold;
        pVeboxDndiState->DW4.HotPixelThresholdLuma                         = pVeboxDndiParams->dwHotPixelThreshold;
        pVeboxDndiState->DW5.ChromaDenoiseStadThreshold                    = pVeboxDndiParams->dwChromaSTADThreshold;
        pVeboxDndiState->DW5.HotPixelCountChromaU                          = m_chromaParams.dwHotPixelCountChromaU;
        pVeboxDndiState->DW5.HotPixelThresholdChromaU                      = m_chromaParams.dwHotPixelThresholdChromaU;
        pVeboxDndiState->DW6.ChromaDenoiseEnable                           = pVeboxDndiParams->bChromaDNEnable;
        pVeboxDndiState->DW6.ChromaTemporalDifferenceThreshold             = pVeboxDndiParams->dwChromaTDThreshold;
        pVeboxDndiState->DW7.ChromaLowTemporalDifferenceThreshold          = pVeboxDndiParams->dwChromaLTDThreshold;
        pVeboxDndiState->DW7.HotPixelCountChromaV                          = m_chromaParams.dwHotPixelCountChromaV;
        pVeboxDndiState->DW7.HotPixelThresholdChromaV                      = m_chromaParams.dwHotPixelThresholdChromaV;
        pVeboxDndiState->DW8.ChromaDenoiseMovingPixelThreshold             = m_chromaParams.dwHotPixelThresholdChromaV;

        pVeboxDndiState->DW9.DnyWr040 = pVeboxDndiParams->dwPixRangeWeight[0];
        pVeboxDndiState->DW9.DnyWr140 = pVeboxDndiParams->dwPixRangeWeight[1];
        pVeboxDndiState->DW9.DnyWr240 = pVeboxDndiParams->dwPixRangeWeight[2];
        pVeboxDndiState->DW9.DnyWr340 = pVeboxDndiParams->dwPixRangeWeight[3];
        pVeboxDndiState->DW9.DnyWr440 = pVeboxDndiParams->dwPixRangeWeight[4];
        pVeboxDndiState->DW9.DnyWr540 = pVeboxDndiParams->dwPixRangeWeight[5];

        pVeboxDndiState->DW11.DnyPrt5120 = pVeboxDndiParams->dwPixRangeThreshold[5];
        pVeboxDndiState->DW12.DnyPrt4120 = pVeboxDndiParams->dwPixRangeThreshold[4];
        pVeboxDndiState->DW12.DnyPrt3120 = pVeboxDndiParams->dwPixRangeThreshold[3];
        pVeboxDndiState->DW13.DnyPrt2120 = pVeboxDndiParams->dwPixRangeThreshold[2];
        pVeboxDndiState->DW13.DnyPrt1120 = pVeboxDndiParams->dwPixRangeThreshold[1];
        pVeboxDndiState->DW14.DnyPrt0120 = pVeboxDndiParams->dwPixRangeThreshold[0];

        pVeboxDndiState->DW16.DnuWr040 = m_chromaParams.dwPixRangeWeightChromaU[0];
        pVeboxDndiState->DW16.DnuWr140 = m_chromaParams.dwPixRangeWeightChromaU[1];
        pVeboxDndiState->DW16.DnuWr240 = m_chromaParams.dwPixRangeWeightChromaU[2];
        pVeboxDndiState->DW16.DnuWr340 = m_chromaParams.dwPixRangeWeightChromaU[3];
        pVeboxDndiState->DW16.DnuWr440 = m_chromaParams.dwPixRangeWeightChromaU[4];
        pVeboxDndiState->DW16.DnuWr540 = m_chromaParams.dwPixRangeWeightChromaU[5];

        pVeboxDndiState->DW18.DnuPrt5120 = m_chromaParams.dwPixRangeThresholdChromaU[5];
        pVeboxDndiState->DW19.DnuPrt4120 = m_chromaParams.dwPixRangeThresholdChromaU[4];
        pVeboxDndiState->DW19.DnuPrt3120 = m_chromaParams.dwPixRangeThresholdChromaU[3];
        pVeboxDndiState->DW20.DnuPrt2120 = m_chromaParams.dwPixRangeThresholdChromaU[2];
        pVeboxDndiState->DW20.DnuPrt1120 = m_chromaParams.dwPixRangeThresholdChromaU[1];
        pVeboxDndiState->DW21.DnuPrt0120 = m_chromaParams.dwPixRangeThresholdChromaU[0];

        pVeboxDndiState->DW23.DnvWr040  = m_chromaParams.dwPixRangeWeightChromaV[0];
        pVeboxDndiState->DW23.DnvWr5140 = m_chromaParams.dwPixRangeWeightChromaV[1];
        pVeboxDndiState->DW23.DnvWr240  = m_chromaParams.dwPixRangeWeightChromaV[2];
        pVeboxDndiState->DW23.DnvWr340  = m_chromaParams.dwPixRangeWeightChromaV[3];
        pVeboxDndiState->DW23.DnvWr440  = m_chromaParams.dwPixRangeWeightChromaV[4];
        pVeboxDndiState->DW23.DnvWr540  = m_chromaParams.dwPixRangeWeightChromaV[5];

        pVeboxDndiState->DW25.DnvPrt5120 = m_chromaParams.dwPixRangeThresholdChromaV[5];
        pVeboxDndiState->DW26.DnvPrt4120 = m_chromaParams.dwPixRangeThresholdChromaV[4];
        pVeboxDndiState->DW26.DnvPrt3120 = m_chromaParams.dwPixRangeThresholdChromaV[3];
        pVeboxDndiState->DW27.DnvPrt2120 = m_chromaParams.dwPixRangeThresholdChromaV[2];
        pVeboxDndiState->DW27.DnvPrt1120 = m_chromaParams.dwPixRangeThresholdChromaV[1];
        pVeboxDndiState->DW28.DnvPrt0120 = m_chromaParams.dwPixRangeThresholdChromaV[0];

        pVeboxDndiState->DW38.DnDiTopFirst = pVeboxDndiParams->bDNDITopFirst;

        pVeboxDndiState->DW39.ProgressiveCadenceReconstructionFor1StFieldOfCurrentFrame     = pVeboxDndiParams->dwFMDFirstFieldCurrFrame;
        pVeboxDndiState->DW39.ProgressiveCadenceReconstructionForSecondFieldOfPreviousFrame = pVeboxDndiParams->dwFMDSecondFieldPrevFrame;

        // Improved Deinterlacing
        pVeboxDndiState->DW36.LumatdmWt   = pVeboxDndiParams->dwLumaTDMWeight;
        pVeboxDndiState->DW36.ChromatdmWt = pVeboxDndiParams->dwChromaTDMWeight;

        pVeboxDndiState->DW37.CoringThresholdForSvcm = pVeboxDndiParams->dwSVCMThreshold;
        pVeboxDndiState->DW37.DeltabitValueForSvcm   = pVeboxDndiParams->dwSVCMDelta;
        pVeboxDndiState->DW37.CoringThresholdForShcm = pVeboxDndiParams->dwSHCMThreshold;
        pVeboxDndiState->DW37.DeltabitValueForShcm   = pVeboxDndiParams->dwSHCMDelta;

        pVeboxDndiState->DW39.ChromaSmallerWindowForTdm = pVeboxDndiParams->bTDMChromaSmallerWindow;
        pVeboxDndiState->DW39.LumaSmallerWindowForTdm   = pVeboxDndiParams->bTDMLumaSmallerWindow;
        pVeboxDndiState->DW39.Fastercovergence          = pVeboxDndiParams->bFasterConvergence;

        pVeboxDndiState->DW40.SadWt0 = pVeboxDndiParams->dwSADWT0;
        pVeboxDndiState->DW40.SadWt1 = pVeboxDndiParams->dwSADWT1;
        pVeboxDndiState->DW40.SadWt2 = pVeboxDndiParams->dwSADWT2;
        pVeboxDndiState->DW40.SadWt3 = pVeboxDndiParams->dwSADWT3;
        pVeboxDndiState->DW41.SadWt4 = pVeboxDndiParams->dwSADWT4;
        pVeboxDndiState->DW41.SadWt6 = pVeboxDndiParams->dwSADWT6;

        pVeboxDndiState->DW41.CoringThresholdForLumaSadCalculation   = pVeboxDndiParams->dwLumaTDMCoringThreshold;
        pVeboxDndiState->DW41.CoringThresholdForChromaSadCalculation = pVeboxDndiParams->dwChromaTDMCoringThreshold;

        pVeboxDndiState->DW42.ParDiffcheckslackthreshold   = pVeboxDndiParams->dwDiffCheckSlackThreshold;
        pVeboxDndiState->DW42.ParTearinghighthreshold      = pVeboxDndiParams->dwTearingHighThreshold;
        pVeboxDndiState->DW42.ParTearinglowthreshold       = pVeboxDndiParams->dwTearingLowThreshold;
        pVeboxDndiState->DW42.ParDirectioncheckth          = pVeboxDndiParams->dwDirectionCheckThreshold;
        pVeboxDndiState->DW42.ParSyntheticcontentcheck     = pVeboxDndiParams->bSyntheticContentCheck;
        pVeboxDndiState->DW42.ParLocalcheck                = pVeboxDndiParams->bLocalCheck;
        pVeboxDndiState->DW42.ParUsesyntheticcontentmedian = pVeboxDndiParams->bUseSyntheticContentMedian;
        pVeboxDndiState->DW42.BypassDeflicker              = pVeboxDndiParams->bBypassDeflickerFilter;

        pVeboxDndiState->DW43.Lpfwtlut0 = pVeboxDndiParams->dwLPFWtLUT0;
        pVeboxDndiState->DW43.Lpfwtlut1 = pVeboxDndiParams->dwLPFWtLUT1;
        pVeboxDndiState->DW43.Lpfwtlut2 = pVeboxDndiParams->dwLPFWtLUT2;
        pVeboxDndiState->DW43.Lpfwtlut3 = pVeboxDndiParams->dwLPFWtLUT3;
        pVeboxDndiState->DW44.Lpfwtlut4 = pVeboxDndiParams->dwLPFWtLUT4;
        pVeboxDndiState->DW44.Lpfwtlut5 = pVeboxDndiParams->dwLPFWtLUT5;
        pVeboxDndiState->DW44.Lpfwtlut6 = pVeboxDndiParams->dwLPFWtLUT6;
        pVeboxDndiState->DW44.Lpfwtlut7 = pVeboxDndiParams->dwLPFWtLUT7;

        pVeboxDndiState->DW10.DnyThmin120    = 512;
        pVeboxDndiState->DW10.DnyThmax120    = 2048;
        pVeboxDndiState->DW11.DnyDynThmin120 = 256;

        pVeboxDndiState->DW14.DnyWd2040 = 10;
        pVeboxDndiState->DW14.DnyWd2140 = 10;
        pVeboxDndiState->DW14.DnyWd2240 = 8;
        pVeboxDndiState->DW15.DnyWd0040 = 12;
        pVeboxDndiState->DW15.DnyWd0140 = 12;
        pVeboxDndiState->DW15.DnyWd0240 = 10;
        pVeboxDndiState->DW15.DnyWd1040 = 12;
        pVeboxDndiState->DW15.DnyWd1140 = 11;
        pVeboxDndiState->DW15.DnyWd1240 = 10;

        pVeboxDndiState->DW17.DnuThmin120    = 512;
        pVeboxDndiState->DW17.DnuThmax120    = 2048;
        pVeboxDndiState->DW18.DnuDynThmin120 = 256;

        pVeboxDndiState->DW21.DnuWd2040 = 10;
        pVeboxDndiState->DW21.DnuWd2140 = 10;
        pVeboxDndiState->DW21.DnuWd2240 = 8;
        pVeboxDndiState->DW22.DnuWd0040 = 12;
        pVeboxDndiState->DW22.DnuWd0140 = 12;
        pVeboxDndiState->DW22.DnuWd0240 = 10;
        pVeboxDndiState->DW22.DnuWd1040 = 12;
        pVeboxDndiState->DW22.DnuWd1140 = 11;
        pVeboxDndiState->DW22.DnuWd1240 = 10;

        pVeboxDndiState->DW24.DnvThmin120    = 512;
        pVeboxDndiState->DW24.DnvThmax120    = 2048;
        pVeboxDndiState->DW25.DnvDynThmin120 = 256;

        pVeboxDndiState->DW28.DnvWd2040 = 10;
        pVeboxDndiState->DW28.DnvWd2140 = 10;
        pVeboxDndiState->DW28.DnvWd2240 = 8;
        pVeboxDndiState->DW29.DnvWd0040 = 12;
        pVeboxDndiState->DW29.DnvWd0140 = 12;
        pVeboxDndiState->DW29.DnvWd0240 = 10;
        pVeboxDndiState->DW29.DnvWd1040 = 12;
        pVeboxDndiState->DW29.DnvWd1140 = 11;
        pVeboxDndiState->DW29.DnvWd1240 = 10;

        pVeboxDndiState->DW31.SmallSobelCountThreshold  = 6;
        pVeboxDndiState->DW32.LargeSobelCountThreshold  = 6;
        pVeboxDndiState->DW32.MedianSobelCountThreshold = 40;

        pVeboxDndiState->DW34.StmmC2                                         = 2;
        pVeboxDndiState->DW35.MaximumStmm                                    = 150;
        pVeboxDndiState->DW35.MultiplierForVecm                              = 30;
        pVeboxDndiState->DW35.BlendingConstantAcrossTimeForSmallValuesOfStmm = 125;
        pVeboxDndiState->DW35.BlendingConstantAcrossTimeForLargeValuesOfStmm = 64;

        pVeboxDndiState->DW36.FmdTemporalDifferenceThreshold = 175;
        pVeboxDndiState->DW36.StmmOutputShift                = 5;
        pVeboxDndiState->DW36.StmmShiftUp                    = 1;
        pVeboxDndiState->DW36.MinimumStmm                    = 118;

        pVeboxDndiState->DW38.McdiEnable                      = 1;
        pVeboxDndiState->DW38.FmdTearThreshold                = 2;
        pVeboxDndiState->DW38.Fmd2VerticalDifferenceThreshold = 100;
        pVeboxDndiState->DW38.Fmd1VerticalDifferenceThreshold = 16;

        pVeboxDndiState->DW45.SynthticFrame = pVeboxDndiParams->bSyntheticFrame;

        // copy the DW0-DW33 SLIM_IPU_DN_PARAMS to VEBOX_DNDI_STATE, DW34-DW48 for DI according to DI DDI setting.
        if (pVeboxDndiParams->bEnableSlimIPUDenoise)
        {
            uint32_t slimIpuDnCmdSize = MHW_VEBOX_SLIM_IPU_DN_CMD_SIZE_INUSE * sizeof(pVeboxDndiState->DW0);  //buffer size in use for SLIM IPU DN

            if (nullptr == pVeboxDndiParams->pSystemMem || pVeboxDndiParams->MemSizeInBytes != sizeof(*pVeboxDndiState) || pVeboxDndiParams->MemSizeInBytes < slimIpuDnCmdSize)
            {
                MHW_ASSERTMESSAGE("SlimIPUDenoise size is invaild");
                return MOS_STATUS_INVALID_PARAMETER;
            }

            MOS_SecureMemcpy(pVeboxDndiState, sizeof(*pVeboxDndiState), pVeboxDndiParams->pSystemMem, slimIpuDnCmdSize);  // only copy dw0 - dw33 for DN

            pVeboxDndiState->DW3.ProgressiveDn = pVeboxDndiParams->bProgressiveDN;
        }

        if (pVeboxDndiParams->bSCDEnable)
        {
            pVeboxDndiState->DW34.SignBitForMinimumStmm       = 1;
            pVeboxDndiState->DW34.SignBitForMaximumStmm       = 1;
            pVeboxDndiState->DW34.SignBitForSmoothMvThreshold = 1;
        }
        else
        {
            pVeboxDndiState->DW34.SignBitForMinimumStmm       = 0;
            pVeboxDndiState->DW34.SignBitForMaximumStmm       = 0;
            pVeboxDndiState->DW34.SignBitForSmoothMvThreshold = 0;
        }

        // GNE setting
        if (bTGNEEnable)
        {
            SetVeboxTGNEDndiState((uint8_t *)pVeboxDndiState);
        }
        else
        {
            SetVeboxSGNEDndiState((uint8_t *)pVeboxDndiState);
        }

        DumpDNDIStates((uint8_t *)pVeboxDndiState);

        return eStatus;
    }

    //!
    //! \brief      Set Vebox Iecp  STDSTE State
    //! \details    Set STE part of the VEBOX IECP States
    //! \param      [in] pVeboxStdSteState
    //!             Pointer to VEBOX IECP STD/E States
    //! \param      [in] pColorPipeParams
    //!             Pointer to COLOR PIPE Params
    //! \return     void
    //!
     MOS_STATUS  SetVeboxIecpStateSTE(
         mhw::vebox::xe_lpm_plus_next::Cmd::VEBOX_STD_STE_STATE_CMD* pVeboxStdSteState,
         PMHW_COLORPIPE_PARAMS pColorPipeParams)
    {
        MHW_FUNCTION_ENTER;

        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        MHW_CHK_NULL_RETURN(pVeboxStdSteState);
        MHW_CHK_NULL_RETURN(pColorPipeParams);

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
        else if (pColorPipeParams->bEnableSTD)
        {
            if (nullptr == pColorPipeParams->StdParams.param || pColorPipeParams->StdParams.paraSizeInBytes > pVeboxStdSteState->byteSize)
            {
                MHW_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
            }

            MOS_SecureMemcpy(pVeboxStdSteState, pColorPipeParams->StdParams.paraSizeInBytes, pColorPipeParams->StdParams.param, pColorPipeParams->StdParams.paraSizeInBytes);
     
        }

        // Enable Skin Score Output surface to be written by Vebox
        pVeboxStdSteState->DW1.StdScoreOutput = (pColorPipeParams->bEnableLACE && pColorPipeParams->LaceParams.bSTD) || (pColorPipeParams->bEnableSTD);

        return eStatus;
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
        mhw::vebox::xe_lpm_plus_next::Cmd::VEBOX_TCC_STATE_CMD* pVeboxTccState,
        PMHW_COLORPIPE_PARAMS                    pColorPipeParams)
    {
        MHW_FUNCTION_ENTER;

        MHW_CHK_NULL_NO_STATUS_RETURN(pVeboxTccState);
        MHW_CHK_NULL_NO_STATUS_RETURN(pColorPipeParams);

        // TCC allows users to custom the color scheme by choosing different
        // grades of saturation for each of the six basic colors (red, green,
        // blue, magenta, yellow, and cyan).
        pVeboxTccState->DW0.TccEnable = true;
        pVeboxTccState->DW0.Satfactor1 = pColorPipeParams->TccParams.Magenta;
        pVeboxTccState->DW0.Satfactor2 = pColorPipeParams->TccParams.Red;
        pVeboxTccState->DW0.Satfactor3 = pColorPipeParams->TccParams.Yellow;
        pVeboxTccState->DW1.Satfactor4 = pColorPipeParams->TccParams.Green;
        pVeboxTccState->DW1.Satfactor5 = pColorPipeParams->TccParams.Cyan;
        pVeboxTccState->DW1.Satfactor6 = pColorPipeParams->TccParams.Blue;
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
        mhw::vebox::xe_lpm_plus_next::Cmd::VEBOX_ACE_LACE_STATE_CMD* pVeboxAceLaceState,
        mhw::vebox::xe_lpm_plus_next::Cmd::VEBOX_ALPHA_AOI_STATE_CMD* pVeboxAlphaAoiState,
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

    MOS_STATUS SetVeboxIecpState(
        PMHW_VEBOX_IECP_PARAMS pVeboxIecpParams) override
    {
        MHW_FUNCTION_ENTER;

        bool                   bEnableFECSC = false;
        PMHW_FORWARD_GAMMA_SEG pFwdGammaSeg;
        uint8_t *              p3DLUT;
        MHW_VEBOX_HEAP         *pVeboxHeap;
        uint32_t               uiOffset;
        MOS_STATUS             eStatus = MOS_STATUS_SUCCESS;

        mhw::vebox::xe_lpm_plus_next::Cmd::VEBOX_IECP_STATE_CMD *pVeboxIecpState;

        MHW_CHK_NULL_RETURN(pVeboxIecpParams);
        MHW_CHK_NULL_RETURN(m_veboxHeap);

        pVeboxHeap      = m_veboxHeap;
        uiOffset        = pVeboxHeap->uiCurState * pVeboxHeap->uiInstanceSize;
        pVeboxIecpState = (mhw::vebox::xe_lpm_plus_next::Cmd::VEBOX_IECP_STATE_CMD *)(pVeboxHeap->pLockedDriverResourceMem +
                                                                          pVeboxHeap->uiIecpStateOffset +
                                                                          uiOffset);

        MHW_CHK_NULL_RETURN(pVeboxIecpState);
        if (pVeboxIecpParams->iecpstateforFDFB)
        {
            IecpStateInitializationforFDFB(pVeboxIecpState);
            return MOS_STATUS_SUCCESS;
        }
        else
        {
            IecpStateInitialization(pVeboxIecpState);
        }

        if (pVeboxIecpParams->ColorPipeParams.bActive)
        {
            // Enable STD/E (Skin Tone Detection/Enhancement)
            SetVeboxIecpStateSTE(
                &pVeboxIecpState->StdSteState,
                &pVeboxIecpParams->ColorPipeParams);

            // Enable TCC (Total Color Control)
            if (pVeboxIecpParams->ColorPipeParams.bEnableTCC)
            {
                SetVeboxIecpStateTCC(
                    &pVeboxIecpState->TccState,
                    &pVeboxIecpParams->ColorPipeParams);
            }
        }

        // Enable ACE (Automatic Contrast Enhancement). Enable LACE if it's enabled.
        if (pVeboxIecpParams->bAce ||
            (pVeboxIecpParams->ColorPipeParams.bActive &&
                pVeboxIecpParams->ColorPipeParams.bEnableACE))
        {
            SetVeboxIecpStateACELACE(
                &pVeboxIecpState->AceState,
                &pVeboxIecpState->AlphaAoiState,
                (pVeboxIecpParams->ColorPipeParams.bEnableLACE == true) ? true : false);
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
                SetVeboxIecpStateFecsc(&pVeboxIecpState->FrontEndCsc, pVeboxIecpParams);
            }

            // Enable Color Correction Matrix
            if (pVeboxIecpParams->CapPipeParams.ColorCorrectionParams.bActive)
            {
                SetVeboxIecpStateCcm(
                    pVeboxIecpState,
                    &pVeboxIecpParams->CapPipeParams,
                    65536);
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
            SetVeboxIecpStateProcAmp(
                &pVeboxIecpState->ProcampState,
                &pVeboxIecpParams->ProcAmpParams);
        }

        if (pVeboxIecpParams && pVeboxIecpParams->CapPipeParams.bActive)
        {
            SetVeboxCapPipeState(
                &pVeboxIecpParams->CapPipeParams);
        }

        if (pVeboxIecpParams &&
            pVeboxIecpParams->CapPipeParams.bActive &&
            pVeboxIecpParams->CapPipeParams.FwdGammaParams.bActive)
        {
            pFwdGammaSeg =
                (PMHW_FORWARD_GAMMA_SEG)(pVeboxHeap->pLockedDriverResourceMem +
                                         pVeboxHeap->uiGammaCorrectionStateOffset +
                                         uiOffset);

            MHW_CHK_NULL_RETURN(pFwdGammaSeg);
            MOS_SecureMemcpy(
                pFwdGammaSeg,
                sizeof(MHW_FORWARD_GAMMA_SEG) * MHW_FORWARD_GAMMA_SEGMENT_CONTROL_POINT,
                &pVeboxIecpParams->CapPipeParams.FwdGammaParams.Segment[0],
                sizeof(MHW_FORWARD_GAMMA_SEG) * MHW_FORWARD_GAMMA_SEGMENT_CONTROL_POINT);
        }

        return eStatus;
    }

    MOS_STATUS SetVeboxIecpAceState(
        PMHW_VEBOX_IECP_PARAMS pVeboxIecpParams) override
    {
        MHW_FUNCTION_ENTER;

        MHW_ACE_PARAMS   *pAceParams;
        MHW_LACE_PARAMS  *pLaceParams;
        MHW_VEBOX_HEAP   *pVeboxHeap;
        int32_t          uiOffset;
        MOS_STATUS       eStatus                    = MOS_STATUS_SUCCESS;
        const uint32_t   uiFullRangeYOffsetInU16    = 0;
        const uint32_t   uiLimitedRangeYOffsetInU16 = 4096;
        const uint32_t   uiUOffsetInU16             = 32768;
        const uint32_t   uiVOffsetInU16             = 32768;

        mhw::vebox::xe_lpm_plus_next::Cmd::VEBOX_IECP_STATE_CMD *pVeboxIecpState;

        MHW_CHK_NULL_RETURN(pVeboxIecpParams);
        MHW_CHK_NULL_RETURN(m_veboxHeap);

        pVeboxHeap = m_veboxHeap;
        uiOffset   = pVeboxHeap->uiCurState * pVeboxHeap->uiInstanceSize;

        pVeboxIecpState = (mhw::vebox::xe_lpm_plus_next::Cmd::VEBOX_IECP_STATE_CMD *)(pVeboxHeap->pLockedDriverResourceMem +
                                                                          pVeboxHeap->uiIecpStateOffset +
                                                                          uiOffset);

        MHW_CHK_NULL_RETURN(pVeboxIecpState);

        SetVeboxAceLaceState(pVeboxIecpParams, pVeboxIecpState);

        if (pVeboxIecpParams->ColorPipeParams.bActive &&
            pVeboxIecpParams->ColorPipeParams.bEnableLACE)
        {
            pLaceParams = &pVeboxIecpParams->ColorPipeParams.LaceParams;

            pVeboxIecpState->AceState.DW0.MinAceLuma  = pLaceParams->wMinAceLuma;
            pVeboxIecpState->AceState.DW12.MaxAceLuma = pLaceParams->wMaxAceLuma;

            pVeboxIecpState->AceState.DW13.LaceColorCorrectionEnable = m_laceColorCorrection.bColorCorrectionEnable;
            if (m_laceColorCorrection.bYUVFullRange)
            {
                pVeboxIecpState->AceState.DW13.LaceYOffset = uiFullRangeYOffsetInU16;
                pVeboxIecpState->AceState.DW14.LaceUOffset = uiUOffsetInU16;
                pVeboxIecpState->AceState.DW14.LaceVOffset = uiVOffsetInU16;
            }
            else
            {
                pVeboxIecpState->AceState.DW13.LaceYOffset = uiLimitedRangeYOffsetInU16;
                pVeboxIecpState->AceState.DW14.LaceUOffset = uiUOffsetInU16;
                pVeboxIecpState->AceState.DW14.LaceVOffset = uiVOffsetInU16;
            }

            pVeboxIecpState->AceState.DW15.LaceGammaCurveBias0  = m_laceColorCorrection.colorWeightLut.iBias[0];
            pVeboxIecpState->AceState.DW15.LaceGammaCurvePoint0 = m_laceColorCorrection.colorWeightLut.iPoint[0];
            pVeboxIecpState->AceState.DW15.LaceGammaCurveSlope0 = m_laceColorCorrection.colorWeightLut.iSlope[0];

            pVeboxIecpState->AceState.DW16.LaceGammaCurveBias1  = m_laceColorCorrection.colorWeightLut.iBias[1];
            pVeboxIecpState->AceState.DW16.LaceGammaCurvePoint1 = m_laceColorCorrection.colorWeightLut.iPoint[1];
            pVeboxIecpState->AceState.DW16.LaceGammaCurveSlope1 = m_laceColorCorrection.colorWeightLut.iSlope[1];

            pVeboxIecpState->AceState.DW17.LaceGammaCurveBias2  = m_laceColorCorrection.colorWeightLut.iBias[2];
            pVeboxIecpState->AceState.DW17.LaceGammaCurvePoint2 = m_laceColorCorrection.colorWeightLut.iPoint[2];
            pVeboxIecpState->AceState.DW17.LaceGammaCurveSlope2 = m_laceColorCorrection.colorWeightLut.iSlope[2];

            pVeboxIecpState->AceState.DW18.LaceGammaCurveBias3  = m_laceColorCorrection.colorWeightLut.iBias[3];
            pVeboxIecpState->AceState.DW18.LaceGammaCurvePoint3 = m_laceColorCorrection.colorWeightLut.iPoint[3];
            pVeboxIecpState->AceState.DW18.LaceGammaCurveSlope3 = m_laceColorCorrection.colorWeightLut.iSlope[3];

            pVeboxIecpState->AceState.DW19.LaceGammaCurveBias4  = m_laceColorCorrection.colorWeightLut.iBias[4];
            pVeboxIecpState->AceState.DW19.LaceGammaCurvePoint4 = m_laceColorCorrection.colorWeightLut.iPoint[4];
            pVeboxIecpState->AceState.DW19.LaceGammaCurveSlope4 = m_laceColorCorrection.colorWeightLut.iSlope[4];

            pVeboxIecpState->AceState.DW20.LaceGammaCurveBias5  = m_laceColorCorrection.colorWeightLut.iBias[5];
            pVeboxIecpState->AceState.DW20.LaceGammaCurvePoint5 = m_laceColorCorrection.colorWeightLut.iPoint[5];
            pVeboxIecpState->AceState.DW20.LaceGammaCurveSlope5 = m_laceColorCorrection.colorWeightLut.iSlope[5];

            pVeboxIecpState->AceState.DW21.LaceGammaCurveBias6  = m_laceColorCorrection.colorWeightLut.iBias[6];
            pVeboxIecpState->AceState.DW21.LaceGammaCurvePoint6 = m_laceColorCorrection.colorWeightLut.iPoint[6];
            pVeboxIecpState->AceState.DW21.LaceGammaCurveSlope6 = m_laceColorCorrection.colorWeightLut.iSlope[6];

            pVeboxIecpState->AceState.DW22.LaceGammaCurveBias7  = m_laceColorCorrection.colorWeightLut.iBias[7];
            pVeboxIecpState->AceState.DW22.LaceGammaCurvePoint7 = m_laceColorCorrection.colorWeightLut.iPoint[7];
            pVeboxIecpState->AceState.DW22.LaceGammaCurveSlope7 = m_laceColorCorrection.colorWeightLut.iSlope[7];

            pVeboxIecpState->AceState.DW23.LaceGammaCurveBias8  = m_laceColorCorrection.colorWeightLut.iBias[8];
            pVeboxIecpState->AceState.DW23.LaceGammaCurvePoint8 = m_laceColorCorrection.colorWeightLut.iPoint[8];
            pVeboxIecpState->AceState.DW23.LaceGammaCurveSlope8 = m_laceColorCorrection.colorWeightLut.iSlope[8];

            pVeboxIecpState->AceState.DW24.LaceGammaCurveBias9  = m_laceColorCorrection.colorWeightLut.iBias[9];
            pVeboxIecpState->AceState.DW24.LaceGammaCurvePoint9 = m_laceColorCorrection.colorWeightLut.iPoint[9];
            pVeboxIecpState->AceState.DW24.LaceGammaCurveSlope9 = m_laceColorCorrection.colorWeightLut.iSlope[9];

            pVeboxIecpState->AceState.DW25.LaceGammaCurveBias10  = m_laceColorCorrection.colorWeightLut.iBias[10];
            pVeboxIecpState->AceState.DW25.LaceGammaCurvePoint10 = m_laceColorCorrection.colorWeightLut.iPoint[10];
            pVeboxIecpState->AceState.DW25.LaceGammaCurveSlope10 = m_laceColorCorrection.colorWeightLut.iSlope[10];

            pVeboxIecpState->AceState.DW26.LaceGammaCurveBias11  = m_laceColorCorrection.colorWeightLut.iBias[11];
            pVeboxIecpState->AceState.DW26.LaceGammaCurvePoint11 = m_laceColorCorrection.colorWeightLut.iPoint[11];
            pVeboxIecpState->AceState.DW26.LaceGammaCurveSlope11 = m_laceColorCorrection.colorWeightLut.iSlope[11];

            pVeboxIecpState->AceState.DW27.LaceGammaCurveBias12  = m_laceColorCorrection.colorWeightLut.iBias[12];
            pVeboxIecpState->AceState.DW27.LaceGammaCurvePoint12 = m_laceColorCorrection.colorWeightLut.iPoint[12];
            pVeboxIecpState->AceState.DW27.LaceGammaCurveSlope12 = m_laceColorCorrection.colorWeightLut.iSlope[12];

            pVeboxIecpState->AceState.DW28.LaceGammaCurveBias13  = m_laceColorCorrection.colorWeightLut.iBias[13];
            pVeboxIecpState->AceState.DW28.LaceGammaCurvePoint13 = m_laceColorCorrection.colorWeightLut.iPoint[13];
            pVeboxIecpState->AceState.DW28.LaceGammaCurveSlope13 = m_laceColorCorrection.colorWeightLut.iSlope[13];

            pVeboxIecpState->AceState.DW29.LaceGammaCurveBias14  = m_laceColorCorrection.colorWeightLut.iBias[14];
            pVeboxIecpState->AceState.DW29.LaceGammaCurvePoint14 = m_laceColorCorrection.colorWeightLut.iPoint[14];
            pVeboxIecpState->AceState.DW29.LaceGammaCurveSlope14 = m_laceColorCorrection.colorWeightLut.iSlope[14];

            pVeboxIecpState->AceState.DW30.LaceGammaCurveBias15  = m_laceColorCorrection.colorWeightLut.iBias[15];
            pVeboxIecpState->AceState.DW30.LaceGammaCurvePoint15 = m_laceColorCorrection.colorWeightLut.iPoint[15];
            pVeboxIecpState->AceState.DW30.LaceGammaCurveSlope15 = m_laceColorCorrection.colorWeightLut.iSlope[15];
        }

        return eStatus;
    }

    MOS_STATUS VeboxInterface_BT2020YUVToRGB(
        PMHW_VEBOX_HEAP         pVeboxHeapInput,
        PMHW_VEBOX_IECP_PARAMS  pVeboxIecpParams,
        PMHW_VEBOX_GAMUT_PARAMS pVeboxGamutParams)
    {
        MHW_FUNCTION_ENTER;

        mhw::vebox::xe_lpm_plus_next::Cmd::VEBOX_IECP_STATE_CMD *pIecpState;
        MHW_VEBOX_HEAP                              *pVeboxHeap;
        uint32_t                                     uiOffset;
        MOS_STATUS                                   eStatus = MOS_STATUS_NULL_POINTER;

        MHW_CHK_NULL_RETURN(pVeboxHeapInput);

        MOS_UNUSED(pVeboxIecpParams);
        MOS_UNUSED(pVeboxGamutParams);

        pVeboxHeap = pVeboxHeapInput;
        MHW_CHK_NULL_RETURN(pVeboxHeap);

        uiOffset   = pVeboxHeap->uiCurState * pVeboxHeap->uiInstanceSize;
        pIecpState = (mhw::vebox::xe_lpm_plus_next::Cmd::VEBOX_IECP_STATE_CMD *)(pVeboxHeap->pLockedDriverResourceMem + pVeboxHeap->uiIecpStateOffset + uiOffset);

        MHW_CHK_NULL_RETURN(pIecpState);

        pIecpState->CscState.DW0.TransformEnable = true;

        if (pVeboxGamutParams->ColorSpace == MHW_CSpace_BT2020)  // Limited->Full
        {
            MHW_CHK_NULL_RETURN(pVeboxIecpParams);
            if (pVeboxIecpParams->s1DLutParams.bActive)
            {
                // The updated value for TGL VEBOX HDR and Fp16 path
                pIecpState->CscState.DW0.C0 = 76533;
                pIecpState->CscState.DW1.C1 = 0;
                pIecpState->CscState.DW2.C2 = 110337;
                pIecpState->CscState.DW3.C3 = 76533;
                pIecpState->CscState.DW4.C4 = MOS_BITFIELD_VALUE((uint32_t)-12312, 19);
                pIecpState->CscState.DW5.C5 = MOS_BITFIELD_VALUE((uint32_t)-42751, 19);
                pIecpState->CscState.DW6.C6 = 76533;
                pIecpState->CscState.DW7.C7 = 140776;
                pIecpState->CscState.DW8.C8 = 0;

                pIecpState->CscState.DW9.OffsetIn1  = MOS_BITFIELD_VALUE((uint32_t)-2048, 16);
                pIecpState->CscState.DW10.OffsetIn2 = MOS_BITFIELD_VALUE((uint32_t)-16384, 16);
                pIecpState->CscState.DW11.OffsetIn3 = MOS_BITFIELD_VALUE((uint32_t)-16384, 16);

                pIecpState->CscState.DW9.OffsetOut1  = 0;
                pIecpState->CscState.DW10.OffsetOut2 = 0;
                pIecpState->CscState.DW11.OffsetOut3 = 0;
            }
            else
            {
                pIecpState->CscState.DW0.C0 = 76607;
                pIecpState->CscState.DW1.C1 = 0;
                pIecpState->CscState.DW2.C2 = 110443;
                pIecpState->CscState.DW3.C3 = 76607;
                pIecpState->CscState.DW4.C4 = MOS_BITFIELD_VALUE((uint32_t)-12325, 19);
                pIecpState->CscState.DW5.C5 = MOS_BITFIELD_VALUE((uint32_t)-42793, 19);
                pIecpState->CscState.DW6.C6 = 76607;
                pIecpState->CscState.DW7.C7 = 140911;
                pIecpState->CscState.DW8.C8 = 0;

                pIecpState->CscState.DW9.OffsetIn1  = MOS_BITFIELD_VALUE((uint32_t)-2048, 16);
                pIecpState->CscState.DW10.OffsetIn2 = MOS_BITFIELD_VALUE((uint32_t)-16384, 16);
                pIecpState->CscState.DW11.OffsetIn3 = MOS_BITFIELD_VALUE((uint32_t)-16384, 16);

                pIecpState->CscState.DW9.OffsetOut1  = 0;
                pIecpState->CscState.DW10.OffsetOut2 = 0;
                pIecpState->CscState.DW11.OffsetOut3 = 0;
            }
        }
        else if (pVeboxGamutParams->ColorSpace == MHW_CSpace_BT2020_FullRange)  // Full->Full
        {
            pIecpState->CscState.DW0.C0 = 65536;
            pIecpState->CscState.DW1.C1 = 0;
            pIecpState->CscState.DW2.C2 = 96639;
            pIecpState->CscState.DW3.C3 = 65536;
            pIecpState->CscState.DW4.C4 = MOS_BITFIELD_VALUE((uint32_t)-10784, 19);
            pIecpState->CscState.DW5.C5 = MOS_BITFIELD_VALUE((uint32_t)-37444, 19);
            pIecpState->CscState.DW6.C6 = 65536;
            pIecpState->CscState.DW7.C7 = 123299;
            pIecpState->CscState.DW8.C8 = 0;

            pIecpState->CscState.DW9.OffsetIn1  = 0;
            pIecpState->CscState.DW10.OffsetIn2 = MOS_BITFIELD_VALUE((uint32_t)-16384, 16);
            pIecpState->CscState.DW11.OffsetIn3 = MOS_BITFIELD_VALUE((uint32_t)-16384, 16);

            pIecpState->CscState.DW9.OffsetOut1  = 0;
            pIecpState->CscState.DW10.OffsetOut2 = 0;
            pIecpState->CscState.DW11.OffsetOut3 = 0;
        }
        else
        {
            MHW_ASSERTMESSAGE("Unsupported BeCSC input color space");
        }

        eStatus = MOS_STATUS_SUCCESS;

        return eStatus;
    }

    MOS_STATUS VeboxInterface_H2SManualMode(
        PMHW_VEBOX_HEAP         pVeboxHeapInput,
        PMHW_VEBOX_IECP_PARAMS  pVeboxIecpParams,
        PMHW_VEBOX_GAMUT_PARAMS pVeboxGamutParams)
    {
        MHW_FUNCTION_ENTER;

        MHW_VEBOX_HEAP *pVeboxHeap;
        uint32_t        uiOffset;

        mhw::vebox::xe_lpm_plus_next::Cmd::VEBOX_IECP_STATE_CMD *    pIecpState;
        mhw::vebox::xe_lpm_plus_next::Cmd::VEBOX_GAMUT_CONTROL_STATE_CMD *pGamutState;
        mhw::vebox::xe_lpm_plus_next::Cmd::Gamut_Expansion_Gamma_Correction_CMD *pVeboxGEGammaCorrection, VeboxGeGammaCorrection;
        MOS_STATUS                                                   eStatus = MOS_STATUS_SUCCESS;

        // HDR H2S algorithm related
        int32_t iToneMappingX[5] = {40, 200, 1000, 2000, 4000};
        int32_t iToneMappingY[4] = {2500, 5000, 10000, 10000};
        float   fPivotX[5]       = {0.0, 0.0, 0.0, 0.0, 0.0};
        float   fPivotY[4]       = {0.0, 0.0, 0.0, 0.0};
        float   fSlope[5]        = {0.0, 0.0, 0.0, 0.0, 0.0};
        float   fMaxCLL          = 0.0;
        // OETF parameters, corresponding to input
        uint32_t uiOETF[HDR_OETF_1DLUT_POINT_NUMBER] = {0};
        uint16_t usGE_Values[256][8]                 = {0};

        MHW_CHK_NULL_RETURN(pVeboxGamutParams);
        MHW_CHK_NULL_RETURN(pVeboxHeapInput);

        pVeboxHeap              = pVeboxHeapInput;
        uiOffset                = pVeboxHeap->uiCurState * pVeboxHeap->uiInstanceSize;
        pIecpState              = (mhw::vebox::xe_lpm_plus_next::Cmd::VEBOX_IECP_STATE_CMD *)(pVeboxHeap->pLockedDriverResourceMem + pVeboxHeap->uiIecpStateOffset + uiOffset);
        pVeboxGEGammaCorrection = (mhw::vebox::xe_lpm_plus_next::Cmd::Gamut_Expansion_Gamma_Correction_CMD *)(pVeboxHeap->pLockedDriverResourceMem + pVeboxHeap->uiGamutStateOffset + uiOffset);
        fMaxCLL                 = (65535 * (float)pVeboxGamutParams->uiMaxCLL) / 10000;

        MHW_CHK_NULL_RETURN(pIecpState);
        MHW_CHK_NULL_RETURN(pVeboxGEGammaCorrection);

        // Must initialize VeboxIecpState even if it is not used because GCE
        // requires GlobalIECP enable bit to be turned on
        if (!pVeboxIecpParams)
        {
            IecpStateInitialization(pIecpState);
        }
        pGamutState = &pIecpState->GamutState;

        for (int32_t i = 0; i < 4; i++)
        {
            fPivotX[i] = (iToneMappingY[i] < 10000) ? (65535 * (float)iToneMappingX[i]) / 10000 : MOS_MIN((65535 * (float)iToneMappingX[i]) / 10000, fMaxCLL);
            fPivotY[i] = (65535 * (float)iToneMappingY[i]) / 10000;
        }
        fPivotX[4] = MOS_MIN((65535 * (float)iToneMappingX[4]) / 10000, fMaxCLL);

        // Slope
        fSlope[0]  = fPivotX[0] > 0 ? (float)(fPivotY[0] / fPivotX[0]) : 0;
        fPivotY[0] = fSlope[0] * fPivotX[0];
        for (int32_t i = 1; i < 4; i++)
        {
            fSlope[i]  = (fPivotX[i] - fPivotX[i - 1]) > 0 ? (float)(fPivotY[i] - fPivotY[i - 1]) / (fPivotX[i] - fPivotX[i - 1]) : 0;
            fPivotY[i] = fSlope[i] * (fPivotX[i] - fPivotX[i - 1]) + fPivotY[i - 1];
        }
        fSlope[4] = (fPivotX[4] - fPivotX[3]) > 0 ? (float)(65535 - fPivotY[3]) / (fPivotX[4] - fPivotX[3]) : 0;

        // Linear Operation
        for (int32_t n = 1; n < HDR_OETF_1DLUT_POINT_NUMBER; n++)
        {
            if ((float)g_Hdr_ColorCorrect_OETF_Rec709_Input[n] < fSlope[0] * fPivotX[0])
            {
                uiOETF[n] = (uint32_t)((float)(g_Hdr_ColorCorrect_OETF_Rec709_Input[n]) / fSlope[0]);
            }
            else if ((float)g_Hdr_ColorCorrect_OETF_Rec709_Input[n] < fSlope[1] * (fPivotX[1] - fPivotX[0]) + fPivotY[0])
            {
                uiOETF[n] = (uint32_t)(((float)(g_Hdr_ColorCorrect_OETF_Rec709_Input[n]) - fPivotY[0]) / fSlope[1] + fPivotX[0]);
            }
            else if ((float)g_Hdr_ColorCorrect_OETF_Rec709_Input[n] < fSlope[2] * (fPivotX[2] - fPivotX[1]) + fPivotY[1])
            {
                uiOETF[n] = (uint32_t)(((float)g_Hdr_ColorCorrect_OETF_Rec709_Input[n] - fPivotY[1]) / fSlope[2] + fPivotX[1]);
            }
            else if ((float)g_Hdr_ColorCorrect_OETF_Rec709_Input[n] < fSlope[3] * (fPivotX[3] - fPivotX[2]) + fPivotY[2])
            {
                uiOETF[n] = (uint32_t)(((float)g_Hdr_ColorCorrect_OETF_Rec709_Input[n] - fPivotY[2]) / fSlope[3] + fPivotX[2]);
            }
            else
            {
                uiOETF[n] = (uint32_t)(((float)g_Hdr_ColorCorrect_OETF_Rec709_Input[n] - fPivotY[3]) / fSlope[4] + fPivotX[3]);
            }
        }
        uiOETF[0]   = 0;
        uiOETF[255] = 65535;

        // Back end CSC setting, need to convert BT2020 YUV input to RGB before GE
        MHW_CHK_STATUS_RETURN(VeboxInterface_BT2020YUVToRGB(pVeboxHeap, pVeboxIecpParams, pVeboxGamutParams));

        // Global setting
        pGamutState->DW0.GlobalModeEnable = true;
        pGamutState->DW1.CmW              = 1023;  // Colorimetric accurate image

        // CCM
        pGamutState->DW1.C0 = 108822;
        pGamutState->DW0.C1 = MOS_BITFIELD_VALUE((uint32_t)-38511, 21);
        pGamutState->DW3.C2 = MOS_BITFIELD_VALUE((uint32_t)-4774, 21);
        pGamutState->DW2.C3 = MOS_BITFIELD_VALUE((uint32_t)-8163, 21);
        pGamutState->DW5.C4 = 74246;
        pGamutState->DW4.C5 = MOS_BITFIELD_VALUE((uint32_t)-547, 21);
        pGamutState->DW7.C6 = MOS_BITFIELD_VALUE((uint32_t)-1190, 21);
        pGamutState->DW6.C7 = MOS_BITFIELD_VALUE((uint32_t)-6592, 21);
        pGamutState->DW8.C8 = 73317;

        // Gamma Expansion
        *pVeboxGEGammaCorrection = VeboxGeGammaCorrection;
        for (int32_t i = 0; i < 255; i++)
        {
            usGE_Values[i][0] = g_Hdr_ColorCorrect_EOTF_SMPTE_ST2084_Input[i];
            usGE_Values[i][1] = g_Hdr_ColorCorrect_EOTF_SMPTE_ST2084_Output[i];
            usGE_Values[i][2] = g_Hdr_ColorCorrect_EOTF_SMPTE_ST2084_Output[i];
            usGE_Values[i][3] = g_Hdr_ColorCorrect_EOTF_SMPTE_ST2084_Output[i];

            usGE_Values[i][4] = (uint16_t)uiOETF[i];
            usGE_Values[i][5] = g_Hdr_ColorCorrect_OETF_Rec709_Output[i];
            usGE_Values[i][6] = g_Hdr_ColorCorrect_OETF_Rec709_Output[i];
            usGE_Values[i][7] = g_Hdr_ColorCorrect_OETF_Rec709_Output[i];
        }
        // Keep the last 4 DWs' value as defult 65535.See mhw_vebox_g10_X::Gamut_Expansion_Gamma_Correction_CMD();
        MOS_SecureMemcpy(pVeboxGEGammaCorrection, sizeof(uint32_t) * 1020, usGE_Values, sizeof(uint16_t) * 2040);

        return eStatus;
    }

    MOS_STATUS SetVeboxSurfaceControlBits(
        PMHW_VEBOX_SURFACE_CNTL_PARAMS pVeboxSurfCntlParams,
        uint32_t *                     pSurfCtrlBits) override
    {
        MHW_FUNCTION_ENTER;

        PLATFORM   Platform = {};
        MOS_STATUS eStatus  = MOS_STATUS_SUCCESS;

        mhw::vebox::xe_lpm_plus_next::Cmd::VEB_DI_IECP_COMMAND_SURFACE_CONTROL_BITS_CMD *pVeboxSurfCtrlBits;
        MEDIA_FEATURE_TABLE *                                                skuTable = nullptr;

        MHW_CHK_NULL_RETURN(pVeboxSurfCntlParams);
        MHW_CHK_NULL_RETURN(pSurfCtrlBits);
        MHW_CHK_NULL_RETURN(this->m_osItf);

        this->m_osItf->pfnGetPlatform(this->m_osItf, &Platform);
        skuTable = this->m_osItf->pfnGetSkuTable(this->m_osItf);
        MHW_CHK_NULL_RETURN(skuTable);

        pVeboxSurfCtrlBits = (mhw::vebox::xe_lpm_plus_next::Cmd::VEB_DI_IECP_COMMAND_SURFACE_CONTROL_BITS_CMD *)pSurfCtrlBits;

        if (pVeboxSurfCntlParams->CompressionMode != MOS_MMC_DISABLED)
        {
            MHW_CHK_NULL_RETURN(pVeboxSurfCtrlBits);
            pVeboxSurfCtrlBits->DW0.MemoryCompressionEnable = 1;

            if (pVeboxSurfCntlParams->CompressionMode == MOS_MMC_RC)
            {
                pVeboxSurfCtrlBits->DW0.CompressionType = 1;
            }
        }

        return eStatus;
    }

    MOS_STATUS VeboxInputFormat(
        PMHW_VEBOX_SURFACE_PARAMS pCurrSurf,
        uint32_t                  *dwFormat)
    {
        MHW_FUNCTION_ENTER;

        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;
        mhw::vebox::xe_lpm_plus_next::Cmd::VEBOX_SURFACE_STATE_CMD VeboxSurfaceState;

        switch (pCurrSurf->Format)
        {
        case Format_NV12:
            *dwFormat = VeboxSurfaceState.SURFACE_FORMAT_PLANAR4208;
            break;
    
        case Format_YUYV:
        case Format_YUY2:
            *dwFormat = VeboxSurfaceState.SURFACE_FORMAT_YCRCBNORMAL;
            break;
    
        case Format_UYVY:
            *dwFormat = VeboxSurfaceState.SURFACE_FORMAT_YCRCBSWAPY;
            break;
    
        case Format_AYUV:
            *dwFormat = VeboxSurfaceState.SURFACE_FORMAT_PACKED444A8;
            break;
    
        case Format_Y416:
            *dwFormat = VeboxSurfaceState.SURFACE_FORMAT_PACKED44416;
            break;
    
        case Format_Y410:
            *dwFormat = VeboxSurfaceState.SURFACE_FORMAT_PACKED44410;
            break;
    
        case Format_YVYU:
            *dwFormat = VeboxSurfaceState.SURFACE_FORMAT_YCRCBSWAPUV;
            break;
    
        case Format_VYUY:
            *dwFormat = VeboxSurfaceState.SURFACE_FORMAT_YCRCBSWAPUVY;
            break;
    
        case Format_A8B8G8R8:
        case Format_X8B8G8R8:
            *dwFormat = VeboxSurfaceState.SURFACE_FORMAT_R8G8B8A8UNORMR8G8B8A8UNORMSRGB;
            break;
    
        case Format_A16B16G16R16:
        case Format_A16R16G16B16:
        case Format_A16B16G16R16F:
        case Format_A16R16G16B16F:
            *dwFormat = VeboxSurfaceState.SURFACE_FORMAT_R16G16B16A16;
            break;
    
        case Format_L8:
        case Format_P8:
        case Format_Y8:
            *dwFormat = VeboxSurfaceState.SURFACE_FORMAT_Y8UNORM;
            break;
    
        case Format_IRW0:
            *dwFormat = VeboxSurfaceState.SURFACE_FORMAT_BAYERPATTERN;
            break;
    
        case Format_IRW1:
            *dwFormat = VeboxSurfaceState.SURFACE_FORMAT_BAYERPATTERN;
            break;
    
        case Format_IRW2:
            *dwFormat = VeboxSurfaceState.SURFACE_FORMAT_BAYERPATTERN;
            break;
    
        case Format_IRW3:
            *dwFormat = VeboxSurfaceState.SURFACE_FORMAT_BAYERPATTERN;
            break;
    
        case Format_IRW4:
            *dwFormat = VeboxSurfaceState.SURFACE_FORMAT_BAYERPATTERN;
            break;
    
        case Format_IRW5:
            *dwFormat = VeboxSurfaceState.SURFACE_FORMAT_BAYERPATTERN;
            break;
    
        case Format_IRW6:
            *dwFormat = VeboxSurfaceState.SURFACE_FORMAT_BAYERPATTERN;
            break;
    
        case Format_IRW7:
            *dwFormat = VeboxSurfaceState.SURFACE_FORMAT_BAYERPATTERN;
            break;
    
        case Format_P010:
        case Format_P016:
            *dwFormat = VeboxSurfaceState.SURFACE_FORMAT_PLANAR42016;
            break;
    
        case Format_A8R8G8B8:
        case Format_X8R8G8B8:
            *dwFormat = VeboxSurfaceState.SURFACE_FORMAT_R8G8B8A8UNORMR8G8B8A8UNORMSRGB;
            break;
    
        case Format_R10G10B10A2:
        case Format_B10G10R10A2:
            *dwFormat = VeboxSurfaceState.SURFACE_FORMAT_R10G10B10A2UNORMR10G10B10A2UNORMSRGB;
            break;
    
        case Format_Y216:
        case Format_Y210:
            *dwFormat = VeboxSurfaceState.SURFACE_FORMAT_PACKED42216;
            break;
    
        case Format_P216:
        case Format_P210:
            *dwFormat = VeboxSurfaceState.SURFACE_FORMAT_PLANAR42216;
            break;
    
        case Format_Y16S:
        case Format_Y16U:
            *dwFormat = VeboxSurfaceState.SURFACE_FORMAT_Y16UNORM;
            break;
    
        default:
            MHW_ASSERTMESSAGE("Unsupported format.");
            break;
        }
    
        return eStatus;
    }

    MOS_STATUS AddVeboxTilingConvert(
            PMOS_COMMAND_BUFFER       cmdBuffer,
            PMHW_VEBOX_SURFACE_PARAMS inSurParams,
            PMHW_VEBOX_SURFACE_PARAMS outSurParams) override
    {
        MHW_FUNCTION_ENTER;

        MOS_STATUS                                                          eStatus = MOS_STATUS_SUCCESS;
        PMOS_RESOURCE                                                       surface = nullptr;
        PMOS_RESOURCE                                                       inputSurface;
        PMOS_RESOURCE                                                       outputSurface;
        mhw::vebox::xe_lpm_plus_next::Cmd::VEB_DI_IECP_COMMAND_SURFACE_CONTROL_BITS_CMD veboxInputSurfCtrlBits, veboxOutputSurfCtrlBits;
        mhw::vebox::xe_lpm_plus_next::Cmd::VEBOX_TILING_CONVERT_CMD                     cmd;
        MHW_RESOURCE_PARAMS                                                 ResourceParams = {0};

        MHW_CHK_NULL_RETURN(cmdBuffer);
        MHW_CHK_NULL_RETURN(this->m_osItf);
        MHW_CHK_NULL_RETURN(inSurParams);
        MHW_CHK_NULL_RETURN(outSurParams);

        inputSurface  = inSurParams->pOsResource;
        outputSurface = outSurParams->pOsResource;

        MHW_CHK_NULL_RETURN(inputSurface);
        MHW_CHK_NULL_RETURN(outputSurface);

        // Set up VEB_DI_IECP_COMMAND_SURFACE_CONTROL_BITS
        MOS_ZeroMemory(&veboxInputSurfCtrlBits, sizeof(veboxInputSurfCtrlBits));
        MOS_ZeroMemory(&veboxOutputSurfCtrlBits, sizeof(veboxOutputSurfCtrlBits));

        // Set Input surface compression status
        if (inSurParams->CompressionMode != MOS_MMC_DISABLED)
        {
            veboxInputSurfCtrlBits.DW0.MemoryCompressionEnable = true;

            if (inSurParams->CompressionMode == MOS_MMC_RC)
            {
                veboxInputSurfCtrlBits.DW0.CompressionType = 1;
            }
            else
            {
                veboxInputSurfCtrlBits.DW0.CompressionType = 0;
            }
        }

        switch (inputSurface->TileType)
        {
        case MOS_TILE_YF:
            veboxInputSurfCtrlBits.DW0.TiledResourceModeForOutputFrameSurfaceBaseAddress = TRMODE_TILEYF;
            break;
        case MOS_TILE_YS:
            veboxInputSurfCtrlBits.DW0.TiledResourceModeForOutputFrameSurfaceBaseAddress = TRMODE_TILEYS;
            break;
        default:
            veboxInputSurfCtrlBits.DW0.TiledResourceModeForOutputFrameSurfaceBaseAddress = TRMODE_NONE;
            break;
        }

        surface                                             = outputSurface;
        veboxOutputSurfCtrlBits.DW0.MemoryCompressionEnable = false;
        veboxOutputSurfCtrlBits.DW0.CompressionType         = 0;

        if (outSurParams->CompressionMode != MOS_MMC_DISABLED)
        {
            veboxOutputSurfCtrlBits.DW0.MemoryCompressionEnable = true;

            if (outSurParams->CompressionMode == MOS_MMC_RC)
            {
                veboxOutputSurfCtrlBits.DW0.CompressionType = 1;
            }
            else
            {
                veboxOutputSurfCtrlBits.DW0.CompressionType = 0;
            }
        }

        if (surface)
        {
            switch (surface->TileType)
            {
            case MOS_TILE_YF:
                veboxOutputSurfCtrlBits.DW0.TiledResourceModeForOutputFrameSurfaceBaseAddress = TRMODE_TILEYF;
                break;
            case MOS_TILE_YS:
                veboxOutputSurfCtrlBits.DW0.TiledResourceModeForOutputFrameSurfaceBaseAddress = TRMODE_TILEYS;
                break;
            default:
                veboxOutputSurfCtrlBits.DW0.TiledResourceModeForOutputFrameSurfaceBaseAddress = TRMODE_NONE;
                break;
            }
        }

        veboxInputSurfCtrlBits.DW0.IndexToMemoryObjectControlStateMocsTables =
            veboxOutputSurfCtrlBits.DW0.IndexToMemoryObjectControlStateMocsTables =
            (this->m_osItf->pfnCachePolicyGetMemoryObject(
                MOS_HW_RESOURCE_USAGE_VP_INTERNAL_READ_WRITE_FF,
                this->m_osItf->pfnGetGmmClientContext(this->m_osItf)))
            .XE_LPG.Index;

        MOS_ZeroMemory(&ResourceParams, sizeof(MHW_RESOURCE_PARAMS));
        InitMocsParams(ResourceParams, &cmd.DW1_2.Value[0], 1, 6);
        ResourceParams.presResource  = inputSurface;
        ResourceParams.HwCommandType = MOS_VEBOX_TILING_CONVERT;

        // set up DW[2:1], input graphics address
        ResourceParams.dwLocationInCmd = 1;
        ResourceParams.pdwCmd          = &(cmd.DW1_2.Value[0]);
        ResourceParams.bIsWritable     = false;
        ResourceParams.dwOffset        = inSurParams->dwOffset + veboxInputSurfCtrlBits.DW0.Value;
        MHW_CHK_STATUS_RETURN(AddResourceToCmd(
            this->m_osItf,
            cmdBuffer,
            &ResourceParams));

        cmd.DW1_2.InputSurfaceControlBits = veboxInputSurfCtrlBits.DW0.Value;

        MOS_ZeroMemory(&ResourceParams, sizeof(MHW_RESOURCE_PARAMS));
        InitMocsParams(ResourceParams, &cmd.DW3_4.Value[0], 1, 6);
        ResourceParams.presResource = outputSurface;
        ResourceParams.HwCommandType = MOS_VEBOX_TILING_CONVERT;

        // set up DW[4:3], output graphics address
        ResourceParams.dwLocationInCmd = 3;
        ResourceParams.pdwCmd          = &(cmd.DW3_4.Value[0]);
        ResourceParams.bIsWritable     = true;
        ResourceParams.dwOffset        = outSurParams->dwOffset + veboxOutputSurfCtrlBits.DW0.Value;;
        MHW_CHK_STATUS_RETURN(AddResourceToCmd(
            this->m_osItf,
            cmdBuffer,
            &ResourceParams));

        cmd.DW3_4.OutputSurfaceControlBits = veboxOutputSurfCtrlBits.DW0.Value;

        m_osItf->pfnAddCommand(cmdBuffer, &cmd, cmd.byteSize);

        return eStatus;
    }

    void SetVeboxSurfaces(
        PMHW_VEBOX_SURFACE_PARAMS                  pInputSurfaceParam,
        PMHW_VEBOX_SURFACE_PARAMS                  pOutputSurfaceParam,
        PMHW_VEBOX_SURFACE_PARAMS                  pDerivedSurfaceParam,
        PMHW_VEBOX_SURFACE_PARAMS                  pSkinScoreSurfaceParam,
        bool                                       bIsOutputSurface,
        bool                                       bDIEnable,
        PMOS_COMMAND_BUFFER                        pCmdBufferInUse)
    {
        MHW_FUNCTION_ENTER;

        uint32_t dwFormat             = 0;
        uint32_t dwInputFormat        = 0;
        uint32_t dwSurfaceWidth       = 0;
        uint32_t dwSurfaceHeight      = 0;
        uint32_t dwSurfacePitch       = 0;
        bool     bHalfPitchForChroma  = false;
        bool     bInterleaveChroma    = false;
        uint16_t wUXOffset            = 0;
        uint16_t wUYOffset            = 0;
        uint16_t wVXOffset            = 0;
        uint16_t wVYOffset            = 0;
        uint8_t  bBayerOffset         = 0;
        uint8_t  bBayerStride         = 0;
        uint8_t  bBayerInputAlignment = 0;
        PMHW_VEBOX_SURFACE_PARAMS pSurfaceParam = nullptr;

        mhw::vebox::xe_lpm_plus_next::Cmd::VEBOX_SURFACE_STATE_CMD VeboxSurfaceState;

        MHW_CHK_NULL_NO_STATUS_RETURN(pInputSurfaceParam);
        MHW_CHK_NULL_NO_STATUS_RETURN(pOutputSurfaceParam);

        if (!bIsOutputSurface)
        {
            pSurfaceParam = pInputSurfaceParam;
        }
        else
        {
            pSurfaceParam = pOutputSurfaceParam;
        }

        switch (pSurfaceParam->Format)
        {
        case Format_NV12:
            dwFormat = VeboxSurfaceState.SURFACE_FORMAT_PLANAR4208;
            bInterleaveChroma = true;
            wUYOffset = (uint16_t)pSurfaceParam->dwUYoffset;
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

        case Format_Y410:
            dwFormat = VeboxSurfaceState.SURFACE_FORMAT_PACKED44410;
            break;

        case Format_YVYU:
            dwFormat = VeboxSurfaceState.SURFACE_FORMAT_YCRCBSWAPUV;
            break;

        case Format_VYUY:
            dwFormat = VeboxSurfaceState.SURFACE_FORMAT_YCRCBSWAPUVY;
            break;

        case Format_A8B8G8R8:
        case Format_X8B8G8R8:
            dwFormat = VeboxSurfaceState.SURFACE_FORMAT_R8G8B8A8UNORMR8G8B8A8UNORMSRGB;
            break;

        case Format_A16B16G16R16:
        case Format_A16R16G16B16:
        case Format_A16B16G16R16F:
        case Format_A16R16G16B16F:
            dwFormat = VeboxSurfaceState.SURFACE_FORMAT_R16G16B16A16;
            break;

        case Format_L8:
        case Format_P8:
        case Format_Y8:
            dwFormat = VeboxSurfaceState.SURFACE_FORMAT_Y8UNORM;
            break;

        case Format_IRW0:
            dwFormat = VeboxSurfaceState.SURFACE_FORMAT_BAYERPATTERN;
            bBayerOffset = VeboxSurfaceState.BAYER_PATTERN_OFFSET_PIXELATX0_Y0ISBLUE;
            bBayerStride = VeboxSurfaceState.BAYER_PATTERN_FORMAT_16_BITINPUTATA16_BITSTRIDE;
            break;

        case Format_IRW1:
            dwFormat = VeboxSurfaceState.SURFACE_FORMAT_BAYERPATTERN;
            bBayerOffset = VeboxSurfaceState.BAYER_PATTERN_OFFSET_PIXELATX0_Y0ISRED;
            bBayerStride = VeboxSurfaceState.BAYER_PATTERN_FORMAT_16_BITINPUTATA16_BITSTRIDE;
            break;

        case Format_IRW2:
            dwFormat = VeboxSurfaceState.SURFACE_FORMAT_BAYERPATTERN;
            bBayerOffset = VeboxSurfaceState.BAYER_PATTERN_OFFSET_PIXELATX0_Y0ISGREEN_PIXELATX1_Y0ISRED;
            bBayerStride = VeboxSurfaceState.BAYER_PATTERN_FORMAT_16_BITINPUTATA16_BITSTRIDE;
            break;

        case Format_IRW3:
            dwFormat = VeboxSurfaceState.SURFACE_FORMAT_BAYERPATTERN;
            bBayerOffset = VeboxSurfaceState.BAYER_PATTERN_OFFSET_PIXELATX0_Y0ISGREEN_PIXELATX1_Y0ISBLUE;
            bBayerStride = VeboxSurfaceState.BAYER_PATTERN_FORMAT_16_BITINPUTATA16_BITSTRIDE;
            break;

        case Format_IRW4:
            dwFormat = VeboxSurfaceState.SURFACE_FORMAT_BAYERPATTERN;
            bBayerOffset = VeboxSurfaceState.BAYER_PATTERN_OFFSET_PIXELATX0_Y0ISBLUE;
            bBayerStride = VeboxSurfaceState.BAYER_PATTERN_FORMAT_8_BITINPUTATA8_BITSTRIDE;
            break;

        case Format_IRW5:
            dwFormat = VeboxSurfaceState.SURFACE_FORMAT_BAYERPATTERN;
            bBayerOffset = VeboxSurfaceState.BAYER_PATTERN_OFFSET_PIXELATX0_Y0ISRED;
            bBayerStride = VeboxSurfaceState.BAYER_PATTERN_FORMAT_8_BITINPUTATA8_BITSTRIDE;
            break;

        case Format_IRW6:
            dwFormat = VeboxSurfaceState.SURFACE_FORMAT_BAYERPATTERN;
            bBayerOffset = VeboxSurfaceState.BAYER_PATTERN_OFFSET_PIXELATX0_Y0ISGREEN_PIXELATX1_Y0ISRED;
            bBayerStride = VeboxSurfaceState.BAYER_PATTERN_FORMAT_8_BITINPUTATA8_BITSTRIDE;
            break;

        case Format_IRW7:
            dwFormat = VeboxSurfaceState.SURFACE_FORMAT_BAYERPATTERN;
            bBayerOffset = VeboxSurfaceState.BAYER_PATTERN_OFFSET_PIXELATX0_Y0ISGREEN_PIXELATX1_Y0ISBLUE;
            bBayerStride = VeboxSurfaceState.BAYER_PATTERN_FORMAT_8_BITINPUTATA8_BITSTRIDE;
            break;

        case Format_P010:
        case Format_P016:
            dwFormat = VeboxSurfaceState.SURFACE_FORMAT_PLANAR42016;
            bInterleaveChroma = true;
            wUYOffset = (uint16_t)pSurfaceParam->dwUYoffset;
            break;

        case Format_A8R8G8B8:
        case Format_X8R8G8B8:
            if (bIsOutputSurface)
            {
                dwFormat = VeboxSurfaceState.SURFACE_FORMAT_B8G8R8A8UNORM;
            }
            else
            {
                dwFormat = VeboxSurfaceState.SURFACE_FORMAT_R8G8B8A8UNORMR8G8B8A8UNORMSRGB;
            }
            break;

        case Format_R10G10B10A2:
        case Format_B10G10R10A2:
            dwFormat = VeboxSurfaceState.SURFACE_FORMAT_R10G10B10A2UNORMR10G10B10A2UNORMSRGB;
            break;

        case Format_Y216:
        case Format_Y210:
            dwFormat = VeboxSurfaceState.SURFACE_FORMAT_PACKED42216;
            break;

        case Format_P216:
        case Format_P210:
            dwFormat = VeboxSurfaceState.SURFACE_FORMAT_PLANAR42216;
            wUYOffset = (uint16_t)pSurfaceParam->dwUYoffset;
            break;

        case Format_Y16S:
        case Format_Y16U:
            dwFormat = VeboxSurfaceState.SURFACE_FORMAT_Y16UNORM;
            break;

        default:
            MHW_ASSERTMESSAGE("Unsupported format.");
            return;
            break;
        }

        if (!bIsOutputSurface)
        {
            // camera pipe will use 10/12/14 for LSB, 0 for MSB. For other pipeline,
            // dwBitDepth is inherited from pSrc->dwDepth which may not among (0,10,12,14)
            // For such cases should use MSB as default value.
            switch (pSurfaceParam->dwBitDepth)
            {
            case 10:
                bBayerInputAlignment = VeboxSurfaceState.BAYER_INPUT_ALIGNMENT_10BITLSBALIGNEDDATA;
                break;

            case 12:
                bBayerInputAlignment = VeboxSurfaceState.BAYER_INPUT_ALIGNMENT_12BITLSBALIGNEDDATA;
                break;

            case 14:
                bBayerInputAlignment = VeboxSurfaceState.BAYER_INPUT_ALIGNMENT_14BITLSBALIGNEDDATA;
                break;

            case 0:
            default:
                bBayerInputAlignment = VeboxSurfaceState.BAYER_INPUT_ALIGNMENT_MSBALIGNEDDATA;
                break;
            }
        }
        else
        {
            bBayerInputAlignment = VeboxSurfaceState.BAYER_INPUT_ALIGNMENT_MSBALIGNEDDATA;
        }

        // adjust boundary for vebox
        VeboxAdjustBoundary(
            pSurfaceParam,
            &dwSurfaceWidth,
            &dwSurfaceHeight,
            bDIEnable);

        dwSurfacePitch = (pSurfaceParam->TileType == MOS_TILE_LINEAR) ? MOS_ALIGN_CEIL(pSurfaceParam->dwPitch, MHW_VEBOX_LINEAR_PITCH) : pSurfaceParam->dwPitch;

        // Setup Surface State
        auto& par = MHW_GETPAR_F(VEBOX_SURFACE_STATE)();
        par = {};
        par.SurfaceIdentification = bIsOutputSurface;
        par.SurfaceFormat         = dwFormat;
        par.Width                 = dwSurfaceWidth - 1;
        par.Height                = dwSurfaceHeight - 1;
        par.HalfPitchForChroma    = bHalfPitchForChroma;
        par.InterleaveChroma      = bInterleaveChroma;
        par.BayerInputAlignment   = bBayerInputAlignment;
        par.BayerPatternOffset    = bBayerOffset;
        par.BayerPatternFormat    = bBayerStride;
        par.SurfacePitch          = dwSurfacePitch - 1;
        par.TileMode              = MosGetHWTileType(pSurfaceParam->TileType, pSurfaceParam->TileModeGMM, pSurfaceParam->bGMMTileEnabled);
        par.XOffsetForU           = wUXOffset;
        par.YOffsetForU           = wUYOffset;
        par.XOffsetForV           = wVXOffset;
        par.YOffsetForV           = wVYOffset;

        // May fix this for stereo surfaces
        par.YOffsetForFrame = pSurfaceParam->dwYoffset;
        par.XOffsetForFrame = 0;

        par.DerivedSurfacePitch                    = pDerivedSurfaceParam->dwPitch - 1;
        par.SurfacePitchForSkinScoreOutputSurfaces = (bIsOutputSurface && pSkinScoreSurfaceParam->bActive) ? (pSkinScoreSurfaceParam->dwPitch - 1) : 0;
        par.CompressionFormat                      = pSurfaceParam->dwCompressionFormat;

        if (bIsOutputSurface && (pInputSurfaceParam->Format == pOutputSurfaceParam->Format))
        {
            VeboxInputFormat(pInputSurfaceParam, &dwInputFormat);
            par.SurfaceFormat = dwInputFormat;
        }

        MHW_ADDCMD_F(VEBOX_SURFACE_STATE)(pCmdBufferInUse);

        return;
    }

    MOS_STATUS AddVeboxSurfaces(
        PMOS_COMMAND_BUFFER                 pCmdBufferInUse,
        PMHW_VEBOX_SURFACE_STATE_CMD_PARAMS pVeboxSurfaceStateCmdParams) override
    {
        MHW_FUNCTION_ENTER;

        MOS_STATUS eStatus;
        bool       bOutputValid;

        MHW_CHK_NULL_RETURN(pCmdBufferInUse);
        MHW_CHK_NULL_RETURN(this->m_osItf);
        MHW_CHK_NULL_RETURN(pVeboxSurfaceStateCmdParams);
        MHW_CHK_NULL_RETURN(this->m_osItf->pfnGetMemoryCompressionFormat);

        eStatus = MOS_STATUS_SUCCESS;
        bOutputValid = pVeboxSurfaceStateCmdParams->bOutputValid;

        if (!pVeboxSurfaceStateCmdParams->SurfInput.dwCompressionFormat)
        {
            this->m_osItf->pfnGetMemoryCompressionFormat(this->m_osItf, pVeboxSurfaceStateCmdParams->SurfInput.pOsResource, &pVeboxSurfaceStateCmdParams->SurfInput.dwCompressionFormat);
        }

        // Setup Surface State for Input surface
        SetVeboxSurfaces(
            &pVeboxSurfaceStateCmdParams->SurfInput,
            &pVeboxSurfaceStateCmdParams->SurfOutput,
            &pVeboxSurfaceStateCmdParams->SurfSTMM,
            nullptr,
            false,
            pVeboxSurfaceStateCmdParams->bDIEnable,
            pCmdBufferInUse);

        // Setup Surface State for Output surface
        if (bOutputValid)
        {
            if (!pVeboxSurfaceStateCmdParams->SurfOutput.dwCompressionFormat)
                this->m_osItf->pfnGetMemoryCompressionFormat(this->m_osItf, pVeboxSurfaceStateCmdParams->SurfOutput.pOsResource, &pVeboxSurfaceStateCmdParams->SurfOutput.dwCompressionFormat);

            SetVeboxSurfaces(
                &pVeboxSurfaceStateCmdParams->SurfInput,
                &pVeboxSurfaceStateCmdParams->SurfOutput,
                &pVeboxSurfaceStateCmdParams->SurfDNOutput,
                &pVeboxSurfaceStateCmdParams->SurfSkinScoreOutput,
                true,
                pVeboxSurfaceStateCmdParams->bDIEnable,
                pCmdBufferInUse);
        }

        return eStatus;
    }

#if (_DEBUG || _RELEASE_INTERNAL)
    MOS_STATUS ValidateVeboxScalabilityConfig()
    {
        MHW_FUNCTION_ENTER;

        MEDIA_ENGINE_INFO mediaSysInfo = {};
        MOS_FORCE_VEBOX   eForceVebox;
        bool              bScalableVEMode;
        bool              bUseVE1, bUseVE2, bUseVE3, bUseVE4;
        MOS_STATUS        eStatus = MOS_STATUS_SUCCESS;

        MHW_CHK_NULL_RETURN(this->m_osItf);

        eForceVebox = this->m_osItf->eForceVebox;
        bScalableVEMode = ((this->m_osItf->bVeboxScalabilityMode) ? true : false);
        eStatus = this->m_osItf->pfnGetMediaEngineInfo(this->m_osItf, mediaSysInfo);
        MHW_CHK_STATUS_RETURN(eStatus);

        if (eForceVebox != MOS_FORCE_VEBOX_NONE &&
            eForceVebox != MOS_FORCE_VEBOX_1 &&
            eForceVebox != MOS_FORCE_VEBOX_2 &&
            eForceVebox != MOS_FORCE_VEBOX_1_2 &&
            eForceVebox != MOS_FORCE_VEBOX_1_2_3 &&
            eForceVebox != MOS_FORCE_VEBOX_1_2_3_4)
        {
            eStatus = MOS_STATUS_INVALID_PARAMETER;
            MHW_ASSERTMESSAGE("eForceVebox value is invalid.");
            return eStatus;
        }

        if (!bScalableVEMode &&
            (eForceVebox == MOS_FORCE_VEBOX_1_2 ||
                eForceVebox == MOS_FORCE_VEBOX_1_2_3 ||
                eForceVebox == MOS_FORCE_VEBOX_1_2_3_4))
        {
            eStatus = MOS_STATUS_INVALID_PARAMETER;
            MHW_ASSERTMESSAGE("eForceVebox value is not consistent with scalability mode.");
            return eStatus;
        }

        if (bScalableVEMode && !m_veboxScalabilitySupported)
        {
            eStatus = MOS_STATUS_INVALID_PARAMETER;
            MHW_ASSERTMESSAGE("scalability mode is not allowed on current platform!");
            return eStatus;
        }

        bUseVE1 = bUseVE2 = bUseVE3 = bUseVE4 = false;
        if (eForceVebox == MOS_FORCE_VEBOX_NONE)
        {
            bUseVE1 = true;
        }
        else
        {
            MHW_VEBOX_IS_VEBOX_SPECIFIED_IN_CONFIG(eForceVebox, MOS_FORCE_VEBOX_1, MOS_FORCEVEBOX_VEBOXID_BITSNUM, MOS_FORCEVEBOX_MASK, bUseVE1);
            MHW_VEBOX_IS_VEBOX_SPECIFIED_IN_CONFIG(eForceVebox, MOS_FORCE_VEBOX_2, MOS_FORCEVEBOX_VEBOXID_BITSNUM, MOS_FORCEVEBOX_MASK, bUseVE2);
            MHW_VEBOX_IS_VEBOX_SPECIFIED_IN_CONFIG(eForceVebox, MOS_FORCE_VEBOX_3, MOS_FORCEVEBOX_VEBOXID_BITSNUM, MOS_FORCEVEBOX_MASK, bUseVE3);
            MHW_VEBOX_IS_VEBOX_SPECIFIED_IN_CONFIG(eForceVebox, MOS_FORCE_VEBOX_4, MOS_FORCEVEBOX_VEBOXID_BITSNUM, MOS_FORCEVEBOX_MASK, bUseVE4);
        }

        if (!mediaSysInfo.VEBoxInfo.IsValid ||
            (uint32_t)(bUseVE1 + bUseVE2 + bUseVE3 + bUseVE4) > mediaSysInfo.VEBoxInfo.NumberOfVEBoxEnabled)
        {
            eStatus = MOS_STATUS_INVALID_PARAMETER;
            MHW_ASSERTMESSAGE("the forced VEBOX is not enabled in current platform.");
            return eStatus;
        }

        return eStatus;
    }
#endif

    _MHW_SETCMD_OVERRIDE_DECL(VEBOX_STATE)
    {
       MHW_FUNCTION_ENTER;

        _MHW_SETCMD_CALLBASE(VEBOX_STATE);

        MOS_STATUS                             eStatus;
        PMOS_INTERFACE                         pOsInterface;
        PMOS_CONTEXT                           pOsContext = nullptr;
        PMOS_RESOURCE                          pVeboxParamResource = nullptr;
        PMOS_RESOURCE                          pVeboxHeapResource = nullptr;
        MHW_VEBOX_HEAP* pVeboxHeap;
        uint32_t                               uiInstanceBaseAddr = 0;
        MHW_RESOURCE_PARAMS                    ResourceParams = {};
        MOS_ALLOC_GFXRES_PARAMS                AllocParamsForBufferLinear = {};
        uint32_t                               *pIndirectState            = nullptr;

        MHW_CHK_NULL_RETURN(this->m_osItf);
        MHW_CHK_NULL_RETURN(this->m_osItf->pOsContext);
        MHW_CHK_NULL_RETURN(this->m_currentCmdBuf);

        // Initialize
        eStatus = MOS_STATUS_SUCCESS;
        pOsInterface = this->m_osItf;
        pOsContext = this->m_osItf->pOsContext;

        if (!params.bNoUseVeboxHeap)
        {
            MHW_CHK_NULL_RETURN(m_veboxHeap);

            pVeboxHeap = m_veboxHeap;
            if (params.bCmBuffer)
            {
                pVeboxParamResource = params.pVeboxParamSurf;
            }
            else
            {
                pVeboxHeapResource = params.bUseVeboxHeapKernelResource ? &pVeboxHeap->KernelResource : &pVeboxHeap->DriverResource;

                // Calculate the instance base address
                uiInstanceBaseAddr = pVeboxHeap->uiInstanceSize * pVeboxHeap->uiCurState;
            }

            TraceIndirectStateInfo(*this->m_currentCmdBuf, *pOsContext, params.bCmBuffer, params.bUseVeboxHeapKernelResource);

            MOS_ZeroMemory(&ResourceParams, sizeof(ResourceParams));
            if (params.bCmBuffer)
            {
                ResourceParams.presResource = pVeboxParamResource;
                ResourceParams.dwOffset = pVeboxHeap->uiDndiStateOffset;
            }
            else
            {
                ResourceParams.presResource = pVeboxHeapResource;
                ResourceParams.dwOffset = pVeboxHeap->uiDndiStateOffset + uiInstanceBaseAddr;
            }
            ResourceParams.pdwCmd = &(cmd.DW2.Value);
            ResourceParams.dwLocationInCmd = 2;
            ResourceParams.HwCommandType = MOS_VEBOX_STATE;

            MHW_CHK_STATUS_RETURN(AddResourceToCmd(
                pOsInterface,
                this->m_currentCmdBuf,
                &ResourceParams));
#if MOS_COMMAND_BUFFER_DUMP_SUPPORTED
            if (*ResourceParams.pdwCmd != 0 || *(ResourceParams.pdwCmd + 1) != 0)
            {
                // add DNDI indirect state dump
                pIndirectState = (uint32_t *)(pVeboxHeap->pLockedDriverResourceMem + ResourceParams.dwOffset);
                pOsInterface->pfnAddIndirectState(pOsInterface,
                    sizeof(mhw::vebox::xe_lpm_plus_next::Cmd::VEBOX_DNDI_STATE_CMD),
                    pIndirectState,
                    ResourceParams.pdwCmd,
                    ResourceParams.pdwCmd + 1,
                    "VEBOX_DNDI_STATE_CMD");
            }
#endif
            HalOcaInterfaceNext::OnIndirectState(*this->m_currentCmdBuf, (MOS_CONTEXT_HANDLE)pOsContext, ResourceParams.presResource, ResourceParams.dwOffset, false, m_veboxSettings.uiDndiStateSize);

            MOS_ZeroMemory(&ResourceParams, sizeof(ResourceParams));
            if (params.bCmBuffer)
            {
                ResourceParams.presResource = pVeboxParamResource;
                ResourceParams.dwOffset = pVeboxHeap->uiIecpStateOffset;
            }
            else
            {
                ResourceParams.presResource = pVeboxHeapResource;
                ResourceParams.dwOffset = pVeboxHeap->uiIecpStateOffset + uiInstanceBaseAddr;
            }
            ResourceParams.pdwCmd = &(cmd.DW4.Value);
            ResourceParams.dwLocationInCmd = 4;
            ResourceParams.HwCommandType = MOS_VEBOX_STATE;
            ResourceParams.dwSharedMocsOffset = 1 - ResourceParams.dwLocationInCmd;

            MHW_CHK_STATUS_RETURN(AddResourceToCmd(
                pOsInterface,
                this->m_currentCmdBuf,
                &ResourceParams));
#if MOS_COMMAND_BUFFER_DUMP_SUPPORTED
            if (*ResourceParams.pdwCmd != 0 || *(ResourceParams.pdwCmd + 1) != 0)
            {
                // add IECP indirect state dump
                pIndirectState = (uint32_t *)(pVeboxHeap->pLockedDriverResourceMem + ResourceParams.dwOffset);
                pOsInterface->pfnAddIndirectState(pOsInterface,
                    sizeof(mhw::vebox::xe_lpm_plus_next::Cmd::VEBOX_IECP_STATE_CMD),
                    pIndirectState,
                    ResourceParams.pdwCmd,
                    ResourceParams.pdwCmd + 1,
                    "VEBOX_IECP_STATE_CMD");
            }
#endif
            HalOcaInterfaceNext::OnIndirectState(*this->m_currentCmdBuf, (MOS_CONTEXT_HANDLE)pOsContext, ResourceParams.presResource, ResourceParams.dwOffset, false, m_veboxSettings.uiIecpStateSize);

            // Gamut Expansion, HDR and Forward Gamma Correction are mutually exclusive.
            if (params.VeboxMode.Hdr1DLutEnable)
            {
                // If HDR is enabled, this points to a buffer containing the HDR state.
                MOS_ZeroMemory(&ResourceParams, sizeof(ResourceParams));
                if (params.bCmBuffer)
                {
                    ResourceParams.presResource = pVeboxParamResource;
                    ResourceParams.dwOffset = pVeboxHeap->uiHdrStateOffset;
                }
                else
                {
                    ResourceParams.presResource = pVeboxHeapResource;
                    ResourceParams.dwOffset = pVeboxHeap->uiHdrStateOffset + uiInstanceBaseAddr;
                }
                ResourceParams.pdwCmd = &(cmd.DW6.Value);
                ResourceParams.dwLocationInCmd = 6;
                ResourceParams.HwCommandType = MOS_VEBOX_STATE;
                ResourceParams.dwSharedMocsOffset = 1 - ResourceParams.dwLocationInCmd;

                MHW_CHK_STATUS_RETURN(AddResourceToCmd(
                    pOsInterface,
                    this->m_currentCmdBuf,
                    &ResourceParams));
#if MOS_COMMAND_BUFFER_DUMP_SUPPORTED
                if (*ResourceParams.pdwCmd != 0 || *(ResourceParams.pdwCmd + 1) != 0)
                {
                    // add HDR indirect state dump
                    pIndirectState = (uint32_t *)(pVeboxHeap->pLockedDriverResourceMem + ResourceParams.dwOffset);
                    pOsInterface->pfnAddIndirectState(pOsInterface,
                        sizeof(mhw::vebox::xe_lpm_plus_next::Cmd::VEBOX_HDR_STATE_CMD),
                        pIndirectState,
                        ResourceParams.pdwCmd,
                        ResourceParams.pdwCmd + 1,
                        "VEBOX_HDR_STATE_CMD");
                }
#endif
                HalOcaInterfaceNext::OnIndirectState(*this->m_currentCmdBuf, (MOS_CONTEXT_HANDLE)pOsContext, ResourceParams.presResource, ResourceParams.dwOffset, false, m_veboxSettings.uiHdrStateSize);
            }
            else
            {
                // If Gamut Expansion is enabled, this points to a buffer containing the Gamut Expansion Gamma Correction state.
                MOS_ZeroMemory(&ResourceParams, sizeof(ResourceParams));
                if (params.bCmBuffer)
                {
                    ResourceParams.presResource = pVeboxParamResource;
                    ResourceParams.dwOffset = pVeboxHeap->uiGamutStateOffset;
                }
                else
                {
                    ResourceParams.presResource = pVeboxHeapResource;
                    ResourceParams.dwOffset = pVeboxHeap->uiGamutStateOffset + uiInstanceBaseAddr;
                }
                ResourceParams.pdwCmd = &(cmd.DW6.Value);
                ResourceParams.dwLocationInCmd = 6;
                ResourceParams.HwCommandType = MOS_VEBOX_STATE;
                ResourceParams.dwSharedMocsOffset = 1 - ResourceParams.dwLocationInCmd;

                MHW_CHK_STATUS_RETURN(AddResourceToCmd(
                    pOsInterface,
                    this->m_currentCmdBuf,
                    &ResourceParams));
#if MOS_COMMAND_BUFFER_DUMP_SUPPORTED
                if (*ResourceParams.pdwCmd != 0 || *(ResourceParams.pdwCmd + 1) != 0)
                {
                    // add Gamut Expansion Gamma Correctionindirect state dump
                    pIndirectState = (uint32_t *)(pVeboxHeap->pLockedDriverResourceMem + ResourceParams.dwOffset);
                    pOsInterface->pfnAddIndirectState(pOsInterface,
                        sizeof(mhw::vebox::xe_lpm_plus_next::Cmd::Gamut_Expansion_Gamma_Correction_CMD),
                        pIndirectState,
                        ResourceParams.pdwCmd,
                        ResourceParams.pdwCmd + 1,
                        "Gamut_Expansion_Gamma_Correction_CMD");
                }
#endif

                HalOcaInterfaceNext::OnIndirectState(*this->m_currentCmdBuf, (MOS_CONTEXT_HANDLE)pOsContext, ResourceParams.presResource, ResourceParams.dwOffset, false, m_veboxSettings.uiGamutStateSize);
            }

            MOS_ZeroMemory(&ResourceParams, sizeof(ResourceParams));
            if (params.bCmBuffer)
            {
                ResourceParams.presResource = pVeboxParamResource;
                ResourceParams.dwOffset = pVeboxHeap->uiVertexTableOffset;
            }
            else
            {
                ResourceParams.presResource = pVeboxHeapResource;
                ResourceParams.dwOffset = pVeboxHeap->uiVertexTableOffset + uiInstanceBaseAddr;
            }
            ResourceParams.pdwCmd = &(cmd.DW8.Value);
            ResourceParams.dwLocationInCmd = 8;
            ResourceParams.HwCommandType = MOS_VEBOX_STATE;
            ResourceParams.dwSharedMocsOffset = 1 - ResourceParams.dwLocationInCmd;

            MHW_CHK_STATUS_RETURN(AddResourceToCmd(
                pOsInterface,
                this->m_currentCmdBuf,
                &ResourceParams));
#if MOS_COMMAND_BUFFER_DUMP_SUPPORTED
            if (*ResourceParams.pdwCmd != 0 || *(ResourceParams.pdwCmd + 1) != 0)
            {
                // add Vertex Table state dump
                pIndirectState = (uint32_t *)(pVeboxHeap->pLockedDriverResourceMem + ResourceParams.dwOffset);
                pOsInterface->pfnAddIndirectState(pOsInterface,
                    sizeof(mhw::vebox::xe_lpm_plus_next::Cmd::VEBOX_VERTEX_TABLE_CMD),
                    pIndirectState,
                    ResourceParams.pdwCmd,
                    ResourceParams.pdwCmd + 1,
                    "VEBOX_VERTEX_TABLE_CMD");
            }
#endif
            HalOcaInterfaceNext::OnIndirectState(*this->m_currentCmdBuf, (MOS_CONTEXT_HANDLE)pOsContext, ResourceParams.presResource, ResourceParams.dwOffset, false, m_veboxSettings.uiVertexTableSize);

            MOS_ZeroMemory(&ResourceParams, sizeof(ResourceParams));
            if (params.bCmBuffer)
            {
                ResourceParams.presResource = pVeboxParamResource;
                ResourceParams.dwOffset = pVeboxHeap->uiCapturePipeStateOffset;
            }
            else
            {
                ResourceParams.presResource = pVeboxHeapResource;
                ResourceParams.dwOffset = pVeboxHeap->uiCapturePipeStateOffset + uiInstanceBaseAddr;
            }

            ResourceParams.pdwCmd = &(cmd.DW10.Value);
            ResourceParams.dwLocationInCmd = 10;
            ResourceParams.HwCommandType = MOS_VEBOX_STATE;
            ResourceParams.dwSharedMocsOffset = 1 - ResourceParams.dwLocationInCmd;

            MHW_CHK_STATUS_RETURN(AddResourceToCmd(
                pOsInterface,
                this->m_currentCmdBuf,
                &ResourceParams));
#if MOS_COMMAND_BUFFER_DUMP_SUPPORTED
            if (*ResourceParams.pdwCmd != 0 || *(ResourceParams.pdwCmd + 1) != 0)
            {
                // add CAPTURE PIPE STATE CMD state dump
                pIndirectState = (uint32_t *)(pVeboxHeap->pLockedDriverResourceMem + ResourceParams.dwOffset);
                pOsInterface->pfnAddIndirectState(pOsInterface,
                    sizeof(mhw::vebox::xe_lpm_plus_next::Cmd::VEBOX_CAPTURE_PIPE_STATE_CMD),
                    pIndirectState,
                    ResourceParams.pdwCmd,
                    ResourceParams.pdwCmd + 1,
                    "VEBOX_CAPTURE_PIPE_STATE_CMD");
            }
#endif
            HalOcaInterfaceNext::OnIndirectState(*this->m_currentCmdBuf, (MOS_CONTEXT_HANDLE)pOsContext, ResourceParams.presResource, ResourceParams.dwOffset, false, m_veboxSettings.uiCapturePipeStateSize);

            if (params.pLaceLookUpTables)
            {
                MOS_ZeroMemory(&ResourceParams, sizeof(ResourceParams));
                ResourceParams.presResource = params.pLaceLookUpTables;
                ResourceParams.dwOffset = 0;
                ResourceParams.pdwCmd = &(cmd.DW12.Value);
                ResourceParams.dwLocationInCmd = 12;
                ResourceParams.HwCommandType = MOS_VEBOX_STATE;
                ResourceParams.dwSharedMocsOffset = 1 - ResourceParams.dwLocationInCmd;

                MHW_CHK_STATUS_RETURN(AddResourceToCmd(
                    pOsInterface,
                    this->m_currentCmdBuf,
                    &ResourceParams));
            }

            MOS_ZeroMemory(&ResourceParams, sizeof(ResourceParams));
            if (params.bCmBuffer)
            {
                ResourceParams.presResource = pVeboxParamResource;
                ResourceParams.dwOffset = pVeboxHeap->uiGammaCorrectionStateOffset;
            }
            else
            {
                ResourceParams.presResource = pVeboxHeapResource;
                ResourceParams.dwOffset = pVeboxHeap->uiGammaCorrectionStateOffset + uiInstanceBaseAddr;
            }
            ResourceParams.pdwCmd = &(cmd.DW14_15.Value[0]);
            ResourceParams.dwLocationInCmd = 14;
            ResourceParams.HwCommandType = MOS_VEBOX_STATE;
            ResourceParams.dwSharedMocsOffset = 1 - ResourceParams.dwLocationInCmd;

            MHW_CHK_STATUS_RETURN(AddResourceToCmd(
                pOsInterface,
                this->m_currentCmdBuf,
                &ResourceParams));
#if MOS_COMMAND_BUFFER_DUMP_SUPPORTED
            if (*ResourceParams.pdwCmd != 0 || *(ResourceParams.pdwCmd + 1) != 0)
            {
                // add Gamma Correction state dump
                pIndirectState = (uint32_t *)(pVeboxHeap->pLockedDriverResourceMem + ResourceParams.dwOffset);
                pOsInterface->pfnAddIndirectState(pOsInterface,
                    sizeof(PMHW_FORWARD_GAMMA_SEG),
                    pIndirectState,
                    ResourceParams.pdwCmd,
                    ResourceParams.pdwCmd + 1,
                    "PMHW_FORWARD_GAMMA_SEG");
            }
#endif

            HalOcaInterfaceNext::OnIndirectState(*this->m_currentCmdBuf, (MOS_CONTEXT_HANDLE)pOsContext, ResourceParams.presResource, ResourceParams.dwOffset, false, m_veboxSettings.uiGammaCorrectionStateSize);

            if (params.pVebox3DLookUpTables)
            {
                MOS_ZeroMemory(&ResourceParams, sizeof(ResourceParams));
                ResourceParams.presResource = params.pVebox3DLookUpTables;
                ResourceParams.dwOffset = 0;
                ResourceParams.pdwCmd = &(cmd.DW16.Value);
                ResourceParams.dwLocationInCmd = 16;
                ResourceParams.HwCommandType = MOS_VEBOX_STATE;
                ResourceParams.dwSharedMocsOffset = 1 - ResourceParams.dwLocationInCmd;

                MHW_CHK_STATUS_RETURN(AddResourceToCmd(
                    pOsInterface,
                    this->m_currentCmdBuf,
                    &ResourceParams));
            }
        }
        else
        {
            // Allocate Resource to avoid Page Fault issue since HW will access it
            if (Mos_ResourceIsNull(params.DummyIecpResource))
            {
                AllocParamsForBufferLinear.Type = MOS_GFXRES_BUFFER;
                AllocParamsForBufferLinear.TileType = MOS_TILE_LINEAR;
                AllocParamsForBufferLinear.Format = Format_Buffer;
                AllocParamsForBufferLinear.dwBytes = m_veboxSettings.uiIecpStateSize;
                AllocParamsForBufferLinear.pBufName = "DummyIecpResource";
                AllocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_VP_INTERNAL_READ_WRITE_FF;

                MHW_CHK_STATUS_RETURN(pOsInterface->pfnAllocateResource(
                    pOsInterface,
                    &AllocParamsForBufferLinear,
                    params.DummyIecpResource));
            }

            MOS_ZeroMemory(&ResourceParams, sizeof(ResourceParams));
            ResourceParams.presResource = params.DummyIecpResource;
            ResourceParams.dwOffset = 0;
            ResourceParams.pdwCmd = &(cmd.DW4.Value);
            ResourceParams.dwLocationInCmd = 4;
            ResourceParams.HwCommandType = MOS_VEBOX_STATE;
            ResourceParams.dwSharedMocsOffset = 1 - ResourceParams.dwLocationInCmd;

            MHW_CHK_STATUS_RETURN(AddResourceToCmd(
                pOsInterface,
                this->m_currentCmdBuf,
                &ResourceParams));

            HalOcaInterfaceNext::OnIndirectState(*this->m_currentCmdBuf, (MOS_CONTEXT_HANDLE)pOsContext, ResourceParams.presResource, 0, true, 0);
        }

        cmd.DW1.ColorGamutExpansionEnable = params.VeboxMode.ColorGamutExpansionEnable;
        cmd.DW1.ColorGamutCompressionEnable = params.VeboxMode.ColorGamutCompressionEnable;
        cmd.DW1.GlobalIecpEnable = params.VeboxMode.GlobalIECPEnable;
        cmd.DW1.DnEnable = params.VeboxMode.DNEnable;
        cmd.DW1.DiEnable = params.VeboxMode.DIEnable;
        cmd.DW1.DnDiFirstFrame = params.VeboxMode.DNDIFirstFrame;
        cmd.DW1.DiOutputFrames = params.VeboxMode.DIOutputFrames;
        cmd.DW1.DemosaicEnable = params.VeboxMode.DemosaicEnable;
        cmd.DW1.VignetteEnable = params.VeboxMode.VignetteEnable;
        cmd.DW1.AlphaPlaneEnable = params.VeboxMode.AlphaPlaneEnable;
        cmd.DW1.HotPixelFilteringEnable = params.VeboxMode.HotPixelFilteringEnable;
        cmd.DW1.LaceCorrectionEnable = params.VeboxMode.LACECorrectionEnable;
        cmd.DW1.DisableEncoderStatistics = params.VeboxMode.DisableEncoderStatistics;
        cmd.DW1.DisableTemporalDenoiseFilter = params.VeboxMode.DisableTemporalDenoiseFilter;
        cmd.DW1.SinglePipeEnable = params.VeboxMode.SinglePipeIECPEnable;
        cmd.DW1.ScalarMode = params.VeboxMode.ScalarMode;
        cmd.DW1.ForwardGammaCorrectionEnable = params.VeboxMode.ForwardGammaCorrectionEnable;
        cmd.DW1.HdrEnable = params.VeboxMode.Hdr1DLutEnable;
        cmd.DW1.Fp16ModeEnable = params.VeboxMode.Fp16ModeEnable;
        cmd.DW1.StateSurfaceControlBits = (pOsInterface->pfnCachePolicyGetMemoryObject(
            MOS_HW_RESOURCE_USAGE_VP_INTERNAL_READ_WRITE_FF,
            pOsInterface->pfnGetGmmClientContext(pOsInterface))).DwordValue;

        cmd.DW17.EncDataControlFor3DLUT = 0;
        cmd.DW17.Lut3DMemoryLayoutControl = 0;  // Legacy layout
        cmd.DW17.ChannelMappingSwapForLut3D = params.LUT3D.ChannelMappingSwapForLut3D;  // B->Y, G->U, R->V for DV Perf

        cmd.DW17.ArbitrationPriorityControlForLut3D = params.LUT3D.ArbitrationPriorityControl;
        // In GmmCachePolicyExt.h, Gen9/Gen10/Gen11/Gen12/MTL+ has the same definition for MEMORY_OBJECT_CONTROL_STATE.
        // In MHW_MEMORY_OBJECT_CONTROL_PARAMS, we only defined Gen9 which intended to use for Gen9 later, so reuse Gen9 index.
        cmd.DW17.Lut3DMocsTable = params.Vebox3DLookUpTablesSurfCtrl.Gen9.Index;
        cmd.DW18.Lut3DEnable = params.LUT3D.Lut3dEnable;
        cmd.DW18.Lut3DSize = params.LUT3D.Lut3dSize;

        cmd.DW18.ChromaUpsamplingCoSitedHorizontalOffset = params.ChromaSampling.ChromaUpsamplingCoSitedHorizontalOffset;
        cmd.DW18.ChromaUpsamplingCoSitedVerticalOffset = params.ChromaSampling.ChromaUpsamplingCoSitedVerticalOffset;
        cmd.DW18.ChromaDownsamplingCoSitedHorizontalOffset = params.ChromaSampling.ChromaDownsamplingCoSitedHorizontalOffset;
        cmd.DW18.ChromaDownsamplingCoSitedVerticalOffset = params.ChromaSampling.ChromaDownsamplingCoSitedVerticalOffset;
        cmd.DW18.BypassChromaUpsampling = params.ChromaSampling.BypassChromaUpsampling;
        cmd.DW18.BypassChromaDownsampling = params.ChromaSampling.BypassChromaDownsampling;
        cmd.DW18._1DLutSize = params.VeboxMode.Hdr1K1DLut;

        return eStatus;
    }

_MHW_SETCMD_OVERRIDE_DECL(VEBOX_TILING_CONVERT)
{
    _MHW_SETCMD_CALLBASE(VEBOX_TILING_CONVERT);
    return MOS_STATUS_SUCCESS;
}

_MHW_SETCMD_OVERRIDE_DECL(VEB_DI_IECP)
{
    MHW_FUNCTION_ENTER;

    _MHW_SETCMD_CALLBASE(VEB_DI_IECP);
    MHW_RESOURCE_PARAMS resourceParams = {};

    MHW_CHK_NULL_RETURN(this->m_osItf);
    MHW_CHK_NULL_RETURN(this->m_currentCmdBuf);

    //MHW_CHK_NULL_RETURN_RETURN(pVeboxDiIecpCmdParams);
    MHW_ASSERT(MOS_IS_ALIGNED(params.dwCurrInputSurfOffset, MHW_PAGE_SIZE));  // offset should be aligned with 4KB
    MHW_ASSERT(MOS_IS_ALIGNED(params.dwPrevInputSurfOffset, MHW_PAGE_SIZE));  // offset should be aligned with 4KB

    if (params.pOsResCurrInput)
    {
        if (params.CurInputSurfMMCState != MOS_MEMCOMP_DISABLED)
        {
            mhw::vebox::xe_lpm_plus_next::Cmd::VEB_DI_IECP_COMMAND_SURFACE_CONTROL_BITS_CMD* pSurfCtrlBits;
            pSurfCtrlBits = (mhw::vebox::xe_lpm_plus_next::Cmd::VEB_DI_IECP_COMMAND_SURFACE_CONTROL_BITS_CMD*)&params.CurrInputSurfCtrl.Value;
            pSurfCtrlBits->DW0.MemoryCompressionEnable = 1;
            pSurfCtrlBits->DW0.CompressionType = pSurfCtrlBits->COMPRESSION_TYPE_MEDIACOMPRESSIONENABLED;
            if (params.CurInputSurfMMCState == MOS_MEMCOMP_RC)
            {
                pSurfCtrlBits->DW0.CompressionType = pSurfCtrlBits->COMPRESSION_TYPE_RENDERCOMPRESSIONENABLED;
            }
        }

        MOS_ZeroMemory(&resourceParams, sizeof(resourceParams));
        resourceParams.presResource = params.pOsResCurrInput;
        resourceParams.dwOffset = params.dwCurrInputSurfOffset + params.CurrInputSurfCtrl.Value;
        resourceParams.pdwCmd = &(cmd.DW2.Value);
        resourceParams.dwLocationInCmd = 2;
        resourceParams.HwCommandType = MOS_VEBOX_DI_IECP;

        MHW_CHK_STATUS_RETURN(AddResourceToCmd(
            this->m_osItf,
            this->m_currentCmdBuf,
            &resourceParams));
    }

    if (params.pOsResPrevInput)
    {
        MOS_ZeroMemory(&resourceParams, sizeof(resourceParams));
        resourceParams.presResource = params.pOsResPrevInput;
        resourceParams.dwOffset = params.PrevInputSurfCtrl.Value + params.dwPrevInputSurfOffset;
        resourceParams.pdwCmd = &(cmd.DW4.Value);
        resourceParams.dwLocationInCmd = 4;
        resourceParams.HwCommandType = MOS_VEBOX_DI_IECP;

        MHW_CHK_STATUS_RETURN(AddResourceToCmd(
            this->m_osItf,
            this->m_currentCmdBuf,
            &resourceParams));
    }

    if (params.pOsResStmmInput)
    {
        MOS_ZeroMemory(&resourceParams, sizeof(resourceParams));
        resourceParams.presResource = params.pOsResStmmInput;
        resourceParams.dwOffset = params.StmmInputSurfCtrl.Value;
        resourceParams.pdwCmd = &(cmd.DW6.Value);
        resourceParams.dwLocationInCmd = 6;
        resourceParams.HwCommandType = MOS_VEBOX_DI_IECP;

        MHW_CHK_STATUS_RETURN(AddResourceToCmd(
            this->m_osItf,
            this->m_currentCmdBuf,
            &resourceParams));
    }

    if (params.pOsResStmmOutput)
    {
        MOS_ZeroMemory(&resourceParams, sizeof(resourceParams));
        resourceParams.presResource = params.pOsResStmmOutput;
        resourceParams.dwOffset = params.StmmOutputSurfCtrl.Value;
        resourceParams.pdwCmd = &(cmd.DW8.Value);
        resourceParams.dwLocationInCmd = 8;
        resourceParams.bIsWritable = true;
        resourceParams.HwCommandType = MOS_VEBOX_DI_IECP;

        MHW_CHK_STATUS_RETURN(AddResourceToCmd(
            this->m_osItf,
            this->m_currentCmdBuf,
            &resourceParams));
    }

    if (params.pOsResDenoisedCurrOutput)
    {
        MOS_ZeroMemory(&resourceParams, sizeof(resourceParams));
        resourceParams.presResource = params.pOsResDenoisedCurrOutput;
        resourceParams.dwOffset = params.DenoisedCurrOutputSurfCtrl.Value;
        resourceParams.pdwCmd = &(cmd.DW10.Value);
        resourceParams.dwLocationInCmd = 10;
        resourceParams.bIsWritable = true;
        resourceParams.HwCommandType = MOS_VEBOX_DI_IECP;

        MHW_CHK_STATUS_RETURN(AddResourceToCmd(
            this->m_osItf,
            this->m_currentCmdBuf,
            &resourceParams));
    }

    if (params.pOsResCurrOutput)
    {
        MOS_ZeroMemory(&resourceParams, sizeof(resourceParams));
        resourceParams.presResource = params.pOsResCurrOutput;
        resourceParams.dwOffset = params.CurrOutputSurfCtrl.Value + params.dwCurrOutputSurfOffset;
        resourceParams.pdwCmd = &(cmd.DW12.Value);
        resourceParams.dwLocationInCmd = 12;
        resourceParams.bIsWritable = true;
        resourceParams.HwCommandType = MOS_VEBOX_DI_IECP;

        MHW_CHK_STATUS_RETURN(AddResourceToCmd(
            this->m_osItf,
            this->m_currentCmdBuf,
            &resourceParams));
    }

    if (params.pOsResPrevOutput)
    {
        MOS_ZeroMemory(&resourceParams, sizeof(resourceParams));
        resourceParams.presResource = params.pOsResPrevOutput;
        resourceParams.dwOffset = params.PrevOutputSurfCtrl.Value;
        resourceParams.pdwCmd = &(cmd.DW14.Value);
        resourceParams.dwLocationInCmd = 14;
        resourceParams.bIsWritable = true;
        resourceParams.HwCommandType = MOS_VEBOX_DI_IECP;

        MHW_CHK_STATUS_RETURN(AddResourceToCmd(
            this->m_osItf,
            this->m_currentCmdBuf,
            &resourceParams));
    }

    if (params.pOsResStatisticsOutput)
    {
        MOS_ZeroMemory(&resourceParams, sizeof(resourceParams));
        resourceParams.presResource = params.pOsResStatisticsOutput;
        resourceParams.dwOffset = params.StatisticsOutputSurfCtrl.Value;
        resourceParams.pdwCmd = &(cmd.DW16.Value);
        resourceParams.dwLocationInCmd = 16;
        resourceParams.bIsWritable = true;
        resourceParams.HwCommandType = MOS_VEBOX_DI_IECP;

        MHW_CHK_STATUS_RETURN(AddResourceToCmd(
            this->m_osItf,
            this->m_currentCmdBuf,
            &resourceParams));
    }

    if (params.pOsResAlphaOrVignette)
    {
        MOS_ZeroMemory(&resourceParams, sizeof(resourceParams));
        resourceParams.presResource = params.pOsResAlphaOrVignette;
        resourceParams.dwOffset = params.AlphaOrVignetteSurfCtrl.Value;
        resourceParams.pdwCmd = &(cmd.DW18.Value);
        resourceParams.dwLocationInCmd = 18;
        resourceParams.bIsWritable = true;
        resourceParams.HwCommandType = MOS_VEBOX_DI_IECP;

        MHW_CHK_STATUS_RETURN(AddResourceToCmd(
            this->m_osItf,
            this->m_currentCmdBuf,
            &resourceParams));
    }

    if (params.pOsResLaceOrAceOrRgbHistogram)
    {
        MOS_ZeroMemory(&resourceParams, sizeof(resourceParams));
        resourceParams.presResource = params.pOsResLaceOrAceOrRgbHistogram;
        resourceParams.dwOffset = params.LaceOrAceOrRgbHistogramSurfCtrl.Value;
        resourceParams.pdwCmd = &(cmd.DW20.Value);
        resourceParams.dwLocationInCmd = 20;
        resourceParams.bIsWritable = true;
        resourceParams.HwCommandType = MOS_VEBOX_DI_IECP;

        MHW_CHK_STATUS_RETURN(AddResourceToCmd(
            this->m_osItf,
            this->m_currentCmdBuf,
            &resourceParams));
    }

    if (params.pOsResSkinScoreSurface)
    {
        MOS_ZeroMemory(&resourceParams, sizeof(resourceParams));
        resourceParams.presResource = params.pOsResSkinScoreSurface;
        resourceParams.dwOffset = params.SkinScoreSurfaceSurfCtrl.Value;
        resourceParams.pdwCmd = &(cmd.DW22.Value);
        resourceParams.dwLocationInCmd = 22;
        resourceParams.bIsWritable = true;
        resourceParams.HwCommandType = MOS_VEBOX_DI_IECP;

        MHW_CHK_STATUS_RETURN(AddResourceToCmd(
            this->m_osItf,
            this->m_currentCmdBuf,
            &resourceParams));
    }

    if (m_veboxScalabilityEnabled == false)
    {
        cmd.DW1.EndingX = params.dwEndingX;
        cmd.DW1.StartingX = params.dwStartingX;
    }
    else
    {
        uint32_t iMediumX;
        MHW_ASSERT(params.dwEndingX >= m_numofVebox * 64 - 1);

        iMediumX = MOS_ALIGN_FLOOR(((params.dwEndingX + 1) / m_numofVebox), 64);
        iMediumX = MOS_CLAMP_MIN_MAX(iMediumX, 64, (params.dwEndingX - 63));

        if (m_numofVebox > 1)
        {
            if (m_indexofVebox == MHW_VEBOX_STARTING_INDEX)
            {
                cmd.DW1.EndingX   = iMediumX - 1;
                cmd.DW1.StartingX = params.dwStartingX;
            }
            else if (m_indexofVebox == m_numofVebox - 1)
            {
                cmd.DW1.EndingX = params.dwEndingX;
                cmd.DW1.StartingX = m_indexofVebox * iMediumX;
            }
            else if (m_indexofVebox < m_numofVebox - 1)
            {
                cmd.DW1.EndingX = (m_indexofVebox + 1) * iMediumX - 1;
                cmd.DW1.StartingX = m_indexofVebox * iMediumX;
            }
            else
            {
                MHW_ASSERTMESSAGE("Unsupported Vebox Scalability Settings");
            }
        }

        if (m_usingSfc)
        {
            cmd.DW1.SplitWorkloadEnable = true;

            if ((params.dwEndingX + 1) != m_numofVebox * iMediumX)
            {
                if (m_indexofVebox < m_numofVebox - 1)
                {
                    cmd.DW1.EndingX += 64;
                }

                if (m_indexofVebox > MHW_VEBOX_STARTING_INDEX)
                {
                    cmd.DW1.StartingX += 64;
                }
            }
        }
        else
        {
            cmd.DW1.SplitWorkloadEnable = false;
        }

        cmd.DW24.OutputEndingX = cmd.DW1.EndingX;
        cmd.DW24.OutputStartingX = cmd.DW1.StartingX;

        if (m_usingSfc)
        {
            // Use left overfetch for sfc split
            if (cmd.DW1.StartingX >= 64)
            {
                cmd.DW1.StartingX -= 64;
            }
        }

        MT_LOG3(MT_VP_MHW_VE_SCALABILITY, MT_NORMAL, MT_VP_MHW_VE_SCALABILITY_EN, m_veboxScalabilityEnabled,
            MT_VP_MHW_VE_SCALABILITY_USE_SFC, m_usingSfc, MT_VP_MHW_VE_SCALABILITY_IDX, m_indexofVebox);

        MHW_NORMALMESSAGE("VEBOX%d STATE: startx %d endx %d", m_indexofVebox, cmd.DW1.StartingX, cmd.DW1.EndingX);
        MHW_NORMALMESSAGE("VEBOX%d STATE: output startx %d endx %d", m_indexofVebox, cmd.DW24.OutputStartingX, cmd.DW24.OutputEndingX);
    }

    cmd.DW26.StartingY = params.dwStartingY;
    cmd.DW26.EndingY   = params.dwEndingY;

    return MOS_STATUS_SUCCESS;

}


protected:
    using base_t = vebox::Impl<mhw::vebox::xe_lpm_plus_next::Cmd>;

MEDIA_CLASS_DEFINE_END(mhw__vebox__xe_lpm_plus_next__Impl)
};

}  // namespace xe_hpg
}  // namespace render
}  // namespace mhw

#endif  // __MHW_RENDER_XE_HPG_IMPL_H__
