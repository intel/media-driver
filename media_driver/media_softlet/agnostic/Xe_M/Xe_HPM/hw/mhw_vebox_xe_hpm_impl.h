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
//! \file     mhw_vebox_xe_hpm_impl.h
//! \brief    MHW vebox interface common base for Xe_HPG
//! \details
//!

#ifndef __MHW_VEBOX_XE_HPM_IMPL_H__
#define __MHW_VEBOX_XE_HPM_IMPL_H__

#include "mhw_vebox_impl.h"
#include "mhw_vebox_hwcmd_xe_hpm.h"
#include "mhw_vebox_itf.h"
#include "mhw_impl.h"

namespace mhw
{
namespace vebox
{
namespace xe_hpm
{
class Impl : public vebox::Impl<mhw::vebox::xe_hpm::Cmd>
{
public:
    Impl(PMOS_INTERFACE osItf) : base_t(osItf)
    {
        MHW_FUNCTION_ENTER;

        dwLumaStadTh = 3200;
        dwChromaStadTh = 1600;
        bTGNEEnable = false;
        bHVSAutoBdrateEnable = false;
        dw4X4TGNEThCnt = 576;
        
        if (osItf && osItf->pfnGetSkuTable)
        {
            MEDIA_FEATURE_TABLE* m_skuTable = osItf->pfnGetSkuTable(osItf);
        
            if (m_skuTable)
            {
                m_veboxScalabilitywith4K = MEDIA_IS_SKU(m_skuTable, FtrVeboxScalabilitywith4K);
            }
            else
            {
                MHW_ASSERTMESSAGE("m_skuTable is null ptr");
            }
        }
        else
        {
            MHW_ASSERTMESSAGE("pOsInterface or pfnGetSkuTable is null ptr");
        }
    };

    MOS_STATUS IecpStateInitialization(mhw::vebox::xe_hpm::Cmd::VEBOX_IECP_STATE_CMD *pVeboxIecpState)
    {
        // Re-set the values
        pVeboxIecpState->StdSteState.DW5.InvMarginVyl = 3300;
        pVeboxIecpState->StdSteState.DW5.InvSkinTypesMargin = 1638;
        pVeboxIecpState->StdSteState.DW12.B3U = 140;
        pVeboxIecpState->StdSteState.DW27.Hues0Dark = 256;
        pVeboxIecpState->StdSteState.DW27.Hues1Dark = 0;

        pVeboxIecpState->AceState.DW0.LaceHistogramSize = 1;

        pVeboxIecpState->TccState.DW0.Satfactor1 = 160;
        pVeboxIecpState->TccState.DW0.Satfactor2 = 160;
        pVeboxIecpState->TccState.DW0.Satfactor3 = 160;
        pVeboxIecpState->TccState.DW1.Satfactor4 = 160;
        pVeboxIecpState->TccState.DW1.Satfactor5 = 160;
        pVeboxIecpState->TccState.DW1.Satfactor6 = 160;

        pVeboxIecpState->GamutState.DW2.CmS = 640;
        pVeboxIecpState->GamutState.DW3.AG = 26;
        pVeboxIecpState->GamutState.DW4.AB = 26;
        pVeboxIecpState->GamutState.DW5.RS = 768;
        pVeboxIecpState->GamutState.DW6.CmI = 192;
        pVeboxIecpState->GamutState.DW7.RI = 128;

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS SetVeboxIecpStateBecsc(
        mhw::vebox::xe_hpm::Cmd::VEBOX_IECP_STATE_CMD *pVeboxIecpState,
        MHW_VEBOX_IECP_PARAMS *pVeboxIecpParams,
        bool                                   bEnableFECSC)
    {
        MHW_CAPPIPE_PARAMS  *m_capPipeParams = nullptr;
        MOS_FORMAT          dstFormat;

        MHW_CHK_NULL_RETURN(pVeboxIecpState);
        MHW_CHK_NULL_RETURN(pVeboxIecpParams);

        m_capPipeParams = &pVeboxIecpParams->CapPipeParams;
        dstFormat = pVeboxIecpParams->dstFormat;
        //m_veboxIecpPar = (mhw::vebox::VEBOX_IECP_STATE_PAR *) pVeboxIecpParams;

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

                if (IS_RGB_SWAP(dstFormat))
                {
                    //pVeboxIecpState->CscState.DW0.YuvChannelSwap = true; // ???
                }

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

                if (IS_RGB_SWAP(dstFormat))
                {
                    //pVeboxIecpState->CscState.DW0.YuvChannelSwap = true;  // ???
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
                else
                {
                    MHW_ASSERT(false);
                }
            }
        }
        else if (pVeboxIecpParams->bCSCEnable)
        {
            pVeboxIecpState->CscState.DW0.TransformEnable = true;

            if (IS_RGB_SWAP(dstFormat))
            {
                //pVeboxIecpState->CscState.DW0.YuvChannelSwap = true; // ???
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

    MOS_STATUS SetVeboxSurfaceControlBits(
        MHW_VEBOX_SURFACE_CNTL_PARAMS *pVeboxSurfCntlParams,
        uint32_t                      *pSurfCtrlBits)
    {
        PLATFORM   Platform = {};
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        mhw::vebox::xe_hpm::Cmd::VEB_DI_IECP_COMMAND_SURFACE_CONTROL_BITS_CMD *pVeboxSurfCtrlBits;

        MHW_CHK_NULL_RETURN(pVeboxSurfCntlParams);
        MHW_CHK_NULL_RETURN(pSurfCtrlBits);
        MHW_CHK_NULL_RETURN(this->m_osItf);

        this->m_osItf->pfnGetPlatform(this->m_osItf, &Platform);

        pVeboxSurfCtrlBits = (mhw::vebox::xe_hpm::Cmd::VEB_DI_IECP_COMMAND_SURFACE_CONTROL_BITS_CMD *)pSurfCtrlBits;

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

    MOS_STATUS SetVeboxVertexTable(
        MHW_CSPACE           ColorSpace)
    {
        MHW_VEBOX_HEAP          *pVeboxHeap;
        uint32_t                uiOffset;
        uint32_t                uSize;
        MOS_STATUS              eStatus = MOS_STATUS_SUCCESS;
        typename mhw::vebox::xe_hpm::Cmd::VEBOX_VERTEX_TABLE_CMD* pVertexTable;

        MHW_CHK_NULL_RETURN(m_veboxHeap);

        pVeboxHeap = m_veboxHeap;
        uiOffset = pVeboxHeap->uiCurState * pVeboxHeap->uiInstanceSize;
        uSize = sizeof(typename mhw::vebox::xe_hpm::Cmd::VEBOX_VERTEX_TABLE_CMD);
        pVertexTable = (typename mhw::vebox::xe_hpm::Cmd::VEBOX_VERTEX_TABLE_CMD*)(pVeboxHeap->pLockedDriverResourceMem +
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
            par.dwRegister        = miItf->GetMmioInterfaces(mhw::mi::MHW_MMIO_VE0_AUX_TABLE_BASE_LOW);
            miItf->MHW_ADDCMD_F(MI_LOAD_REGISTER_IMM)(CmdBuffer);

            par.dwData     = ((auxTableBaseAddr >> 32) & 0xffffffff);
            par.dwRegister = miItf->GetMmioInterfaces(mhw::mi::MHW_MMIO_VE0_AUX_TABLE_BASE_HIGH);
            miItf->MHW_ADDCMD_F(MI_LOAD_REGISTER_IMM)(CmdBuffer);
        }

        return eStatus;
    }

    MOS_STATUS SetVeboxHdrState(
        PMHW_VEBOX_IECP_PARAMS pVeboxIecpParams)
    {
        MOS_STATUS                                eStatus        = MOS_STATUS_SUCCESS;
        mhw::vebox::xe_hpm::Cmd::VEBOX_HDR_STATE_CMD * pVeboxHdrState = nullptr;
        mhw::vebox::xe_hpm::Cmd::VEBOX_IECP_STATE_CMD *pIecpState     = nullptr;
        MHW_VEBOX_HEAP                            *pVeboxHeap     = nullptr;
        uint32_t                                  uiOffset       = 0;

        MHW_CHK_NULL_RETURN(pVeboxIecpParams);
        MHW_CHK_NULL_RETURN(m_veboxHeap);

        pVeboxHeap = m_veboxHeap;
        uiOffset   = pVeboxHeap->uiCurState * pVeboxHeap->uiInstanceSize;

        pVeboxHdrState =
            (mhw::vebox::xe_hpm::Cmd::VEBOX_HDR_STATE_CMD *)(pVeboxHeap->pLockedDriverResourceMem +
                                                           pVeboxHeap->uiHdrStateOffset +
                                                           uiOffset);

        pIecpState =
            (mhw::vebox::xe_hpm::Cmd::VEBOX_IECP_STATE_CMD *)(pVeboxHeap->pLockedDriverResourceMem +
                                                            pVeboxHeap->uiIecpStateOffset +
                                                            uiOffset);

        MHW_CHK_NULL_RETURN(pVeboxHdrState);
        MHW_CHK_NULL_RETURN(pIecpState);

        // Program 1DLUT in Inverse Gamma with 1024 entries / 16bit precision from API level
        if (pVeboxIecpParams->s1DLutParams.bActive && (pVeboxIecpParams->s1DLutParams.LUTSize == 1024))
        {
            // HW provides 4K 1DLUT inverse gamma
            mhw::vebox::xe_hpm::Cmd::VEBOX_HDR_INV_GAMMA_CORRECTION_STATE_CMD *pInverseGamma = pVeboxHdrState->PRGBCorrectedValue;
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

            mhw::vebox::xe_hpm::Cmd::VEBOX_HDR_FWD_GAMMA_CORRECTION_STATE_CMD *pForwardGamma = pVeboxHdrState->ForwardGammaLUTvalue;
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

        return eStatus;
    }

    MOS_STATUS VeboxInterface_BT2020YUVToRGB(
        PMHW_VEBOX_HEAP             pVeboxHeapInput,
        PMHW_VEBOX_IECP_PARAMS      pVeboxIecpParams,
        PMHW_VEBOX_GAMUT_PARAMS     pVeboxGamutParams)
    {
        mhw::vebox::xe_hpm::Cmd::VEBOX_IECP_STATE_CMD* pIecpState;
        MHW_VEBOX_HEAP*                        pVeboxHeap;
        uint32_t                                uiOffset;
        MOS_STATUS                              eStatus = MOS_STATUS_NULL_POINTER;

        MHW_CHK_NULL_RETURN(pVeboxHeapInput);

        MOS_UNUSED(pVeboxIecpParams);
        MOS_UNUSED(pVeboxGamutParams);

        pVeboxHeap = pVeboxHeapInput;
        MHW_CHK_NULL_RETURN(pVeboxHeap);

        uiOffset = pVeboxHeap->uiCurState * pVeboxHeap->uiInstanceSize;
        pIecpState = (mhw::vebox::xe_hpm::Cmd::VEBOX_IECP_STATE_CMD*)(pVeboxHeap->pLockedDriverResourceMem + pVeboxHeap->uiIecpStateOffset + uiOffset);

        MHW_CHK_NULL_RETURN(pIecpState);

        pIecpState->CscState.DW0.TransformEnable = true;

        if (pVeboxGamutParams->ColorSpace == MHW_CSpace_BT2020)                   // Limited->Full
        {
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

                pIecpState->CscState.DW9.OffsetIn1 = MOS_BITFIELD_VALUE((uint32_t)-2048, 16);
                pIecpState->CscState.DW10.OffsetIn2 = MOS_BITFIELD_VALUE((uint32_t)-16384, 16);
                pIecpState->CscState.DW11.OffsetIn3 = MOS_BITFIELD_VALUE((uint32_t)-16384, 16);

                pIecpState->CscState.DW9.OffsetOut1 = 0;
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

                pIecpState->CscState.DW9.OffsetIn1 = MOS_BITFIELD_VALUE((uint32_t)-2048, 16);
                pIecpState->CscState.DW10.OffsetIn2 = MOS_BITFIELD_VALUE((uint32_t)-16384, 16);
                pIecpState->CscState.DW11.OffsetIn3 = MOS_BITFIELD_VALUE((uint32_t)-16384, 16);

                pIecpState->CscState.DW9.OffsetOut1 = 0;
                pIecpState->CscState.DW10.OffsetOut2 = 0;
                pIecpState->CscState.DW11.OffsetOut3 = 0;
            }
        }
        else if (pVeboxGamutParams->ColorSpace == MHW_CSpace_BT2020_FullRange)    // Full->Full
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

            pIecpState->CscState.DW9.OffsetIn1 = 0;
            pIecpState->CscState.DW10.OffsetIn2 = MOS_BITFIELD_VALUE((uint32_t)-16384, 16);
            pIecpState->CscState.DW11.OffsetIn3 = MOS_BITFIELD_VALUE((uint32_t)-16384, 16);

            pIecpState->CscState.DW9.OffsetOut1 = 0;
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

    MOS_STATUS SetVeboxGamutState(
        PMHW_VEBOX_IECP_PARAMS  pVeboxIecpParams,
        PMHW_VEBOX_GAMUT_PARAMS pVeboxGamutParams)
    {
        MHW_VEBOX_HEAP  *pVeboxHeap = nullptr;
        uint32_t        uiOffset = 0;
        uint32_t        i;
        double          dInverseGamma = 0;
        double          dForwardGamma = 1.0;    // init as 1.0 as default to avoid divisor be 0
        MOS_STATUS      eStatus = MOS_STATUS_SUCCESS;
        uint16_t        usGE_Values[256][8] = { 0 };
        bool            bEnableCCM = false;

        MHW_1DLUT_PARAMS                                     *p1DLutParams = nullptr;
        mhw::vebox::xe_hpm::Cmd::VEBOX_IECP_STATE_CMD* pIecpState;
        mhw::vebox::xe_hpm::Cmd::VEBOX_GAMUT_CONTROL_STATE_CMD* pGamutState, gamutCmd;
        mhw::vebox::xe_hpm::Cmd::Gamut_Expansion_Gamma_Correction_CMD* pVeboxGEGammaCorrection, VeboxGEGammaCorrection;
        pVeboxHeap = m_veboxHeap;
        uiOffset = pVeboxHeap->uiCurState * pVeboxHeap->uiInstanceSize;

        pIecpState =
            (mhw::vebox::xe_hpm::Cmd::VEBOX_IECP_STATE_CMD*)(pVeboxHeap->pLockedDriverResourceMem +
                pVeboxHeap->uiIecpStateOffset +
                uiOffset);

        pVeboxGEGammaCorrection =
            (mhw::vebox::xe_hpm::Cmd::Gamut_Expansion_Gamma_Correction_CMD*)(pVeboxHeap->pLockedDriverResourceMem +
                pVeboxHeap->uiGamutStateOffset +
                uiOffset);

        // Must initialize VeboxIecpState even if it is not used because GCE
        // requires GlobalIECP enable bit to be turned on
        if (!pVeboxIecpParams)
        {
            // Re-set the values
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
                pGamutState->DW15.D1Out = pVeboxGamutParams->iDout;
                pGamutState->DW15.DOutDefault = pVeboxGamutParams->iDoutDefault;
                pGamutState->DW15.DInDefault = pVeboxGamutParams->iDinDefault;
                pGamutState->DW16.D1In = pVeboxGamutParams->iDin;
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
        if (pVeboxGamutParams->GExpMode != MHW_GAMUT_MODE_NONE)
        {
            // Need to convert YUV input to RGB before GE
            pIecpState->CscState.DW0.TransformEnable = true;
            if (pVeboxGamutParams->ColorSpace == MHW_CSpace_BT601 ||
                pVeboxGamutParams->ColorSpace == MHW_CSpace_xvYCC601 ||
                pVeboxGamutParams->ColorSpace == MHW_CSpace_BT601_FullRange)
            {
                pIecpState->CscState.DW0.C0 = 1192;
                pIecpState->CscState.DW1.C1 = MOS_BITFIELD_VALUE((uint32_t)-2, 19);
                pIecpState->CscState.DW2.C2 = 1634;
                pIecpState->CscState.DW3.C3 = 1192;
                pIecpState->CscState.DW4.C4 = MOS_BITFIELD_VALUE((uint32_t)-401, 19);
                pIecpState->CscState.DW5.C5 = MOS_BITFIELD_VALUE((uint32_t)-833, 19);
                pIecpState->CscState.DW6.C6 = 1192;
                pIecpState->CscState.DW7.C7 = 2066;
                pIecpState->CscState.DW8.C8 = MOS_BITFIELD_VALUE((uint32_t)-1, 19);
                pIecpState->CscState.DW9.OffsetIn1 = MOS_BITFIELD_VALUE((uint32_t)-64, 16);
                pIecpState->CscState.DW9.OffsetOut1 = 0;
                pIecpState->CscState.DW10.OffsetIn2 = MOS_BITFIELD_VALUE((uint32_t)-512, 16);
                pIecpState->CscState.DW10.OffsetOut2 = 0;
                pIecpState->CscState.DW11.OffsetIn3 = MOS_BITFIELD_VALUE((uint32_t)-512, 16);
                pIecpState->CscState.DW11.OffsetOut3 = 0;
            }
            else if (pVeboxGamutParams->ColorSpace == MHW_CSpace_BT709 ||
                pVeboxGamutParams->ColorSpace == MHW_CSpace_xvYCC709 ||
                pVeboxGamutParams->ColorSpace == MHW_CSpace_BT709_FullRange)
            {
                pIecpState->CscState.DW0.C0 = 1192;
                pIecpState->CscState.DW1.C1 = MOS_BITFIELD_VALUE((uint32_t)-1, 19);
                pIecpState->CscState.DW2.C2 = 1835;
                pIecpState->CscState.DW3.C3 = 1192;
                pIecpState->CscState.DW4.C4 = MOS_BITFIELD_VALUE((uint32_t)-218, 19);
                pIecpState->CscState.DW5.C5 = MOS_BITFIELD_VALUE((uint32_t)-537, 19);
                pIecpState->CscState.DW6.C6 = 1192;
                pIecpState->CscState.DW7.C7 = 2164;
                pIecpState->CscState.DW8.C8 = 1;
                pIecpState->CscState.DW9.OffsetIn1 = MOS_BITFIELD_VALUE((uint32_t)-64, 16);
                pIecpState->CscState.DW9.OffsetOut1 = 0;
                pIecpState->CscState.DW10.OffsetIn2 = MOS_BITFIELD_VALUE((uint32_t)-512, 16);
                pIecpState->CscState.DW10.OffsetOut2 = 0;
                pIecpState->CscState.DW11.OffsetIn3 = MOS_BITFIELD_VALUE((uint32_t)-512, 16);
                pIecpState->CscState.DW11.OffsetOut3 = 0;
            }
            else
            {
                MHW_ASSERTMESSAGE("Unknown primary");
            }

            if (pVeboxGamutParams->GExpMode == MHW_GAMUT_MODE_BASIC)
            {
                pGamutState->DW0.GlobalModeEnable = true;
                pGamutState->DW1.CmW = 1023;
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
            if (IS_RGB_SWAP(pVeboxGamutParams->dstFormat))
            {
                //pIecpState->CscState.DW0.YuvChannelSwap = true;
            }
            if (pVeboxGamutParams->ColorSpace == MHW_CSpace_BT601 ||
                pVeboxGamutParams->ColorSpace == MHW_CSpace_xvYCC601 ||
                pVeboxGamutParams->ColorSpace == MHW_CSpace_BT601_FullRange)
            {
                pIecpState->CscState.DW0.C0 = 76309;
                pIecpState->CscState.DW1.C1 = 0;
                pIecpState->CscState.DW2.C2 = 104597;
                pIecpState->CscState.DW3.C3 = 76309;
                pIecpState->CscState.DW4.C4 = MOS_BITFIELD_VALUE((uint32_t)-25675, 19);
                pIecpState->CscState.DW5.C5 = MOS_BITFIELD_VALUE((uint32_t)-53279, 19);
                pIecpState->CscState.DW6.C6 = 76309;
                pIecpState->CscState.DW7.C7 = 132201;
                pIecpState->CscState.DW8.C8 = 0;
                pIecpState->CscState.DW9.OffsetIn1 = MOS_BITFIELD_VALUE((uint32_t)-2048, 16);
                pIecpState->CscState.DW9.OffsetOut1 = 0;
                pIecpState->CscState.DW10.OffsetIn2 = MOS_BITFIELD_VALUE((uint32_t)-16384, 16);
                pIecpState->CscState.DW10.OffsetOut2 = 0;
                pIecpState->CscState.DW11.OffsetIn3 = MOS_BITFIELD_VALUE((uint32_t)-16384, 16);
                pIecpState->CscState.DW11.OffsetOut3 = 0;
            }
            else if (pVeboxGamutParams->ColorSpace == MHW_CSpace_BT709 ||
                pVeboxGamutParams->ColorSpace == MHW_CSpace_xvYCC709 ||
                pVeboxGamutParams->ColorSpace == MHW_CSpace_BT709_FullRange)
            {
                pIecpState->CscState.DW0.C0 = 76309;
                pIecpState->CscState.DW1.C1 = 0;
                pIecpState->CscState.DW2.C2 = 117489;
                pIecpState->CscState.DW3.C3 = 76309;
                pIecpState->CscState.DW4.C4 = MOS_BITFIELD_VALUE((uint32_t)-13975, 19);
                pIecpState->CscState.DW5.C5 = MOS_BITFIELD_VALUE((uint32_t)-34925, 19);
                pIecpState->CscState.DW6.C6 = 76309;
                pIecpState->CscState.DW7.C7 = 138438;
                pIecpState->CscState.DW8.C8 = 0;
                pIecpState->CscState.DW9.OffsetIn1 = MOS_BITFIELD_VALUE((uint32_t)-2048, 16);
                pIecpState->CscState.DW9.OffsetOut1 = 0;
                pIecpState->CscState.DW10.OffsetIn2 = MOS_BITFIELD_VALUE((uint32_t)-16384, 16);
                pIecpState->CscState.DW10.OffsetOut2 = 0;
                pIecpState->CscState.DW11.OffsetIn3 = MOS_BITFIELD_VALUE((uint32_t)-16384, 16);
                pIecpState->CscState.DW11.OffsetOut3 = 0;
            }
            else if (pVeboxGamutParams->ColorSpace == MHW_CSpace_BT2020)
            {
                VeboxInterface_BT2020YUVToRGB(m_veboxHeap, pVeboxIecpParams, pVeboxGamutParams);
            }
            else
            {
                MHW_ASSERTMESSAGE("Unknown primary");
            }

            // CCM is needed for CSC(BT2020->BT709/BT601 or vice versa with Different Gamma).
            bEnableCCM = (pVeboxGamutParams->InputGammaValue == pVeboxGamutParams->OutputGammaValue) ? false : true;
            pGamutState->DW0.GlobalModeEnable = true;
            pGamutState->DW1.CmW = 1023;
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
                pGamutState->DW1.C0 = 65536;
                pGamutState->DW0.C1 = 0;
                pGamutState->DW3.C2 = 0;
                pGamutState->DW2.C3 = 0;
                pGamutState->DW5.C4 = 65536;
                pGamutState->DW4.C5 = 0;
                pGamutState->DW7.C6 = 0;
                pGamutState->DW6.C7 = 0;
                pGamutState->DW8.C8 = 65536;
                pGamutState->DW9.OffsetInR = 0;
                pGamutState->DW10.OffsetInG = 0;
                pGamutState->DW11.OffsetInB = 0;
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

                pIecpState->CcmState.DW1.C0 = p1DLutParams->pCCM[0];
                pIecpState->CcmState.DW0.C1 = MOS_BITFIELD_VALUE((uint32_t)p1DLutParams->pCCM[1], 27);
                pIecpState->CcmState.DW3.C2 = MOS_BITFIELD_VALUE((uint32_t)p1DLutParams->pCCM[2], 27);
                pIecpState->CcmState.DW2.C3 = MOS_BITFIELD_VALUE((uint32_t)p1DLutParams->pCCM[3], 27);
                pIecpState->CcmState.DW5.C4 = p1DLutParams->pCCM[4];
                pIecpState->CcmState.DW4.C5 = MOS_BITFIELD_VALUE((uint32_t)p1DLutParams->pCCM[5], 27);
                pIecpState->CcmState.DW7.C6 = MOS_BITFIELD_VALUE((uint32_t)p1DLutParams->pCCM[6], 27);
                pIecpState->CcmState.DW6.C7 = MOS_BITFIELD_VALUE((uint32_t)p1DLutParams->pCCM[7], 27);
                pIecpState->CcmState.DW8.C8 = p1DLutParams->pCCM[8];
                pIecpState->CcmState.DW9.OffsetInR = p1DLutParams->pCCM[9];
                pIecpState->CcmState.DW10.OffsetInG = p1DLutParams->pCCM[10];
                pIecpState->CcmState.DW11.OffsetInB = p1DLutParams->pCCM[11];
                pIecpState->CcmState.DW12.OffsetOutR = p1DLutParams->pCCM[12];
                pIecpState->CcmState.DW13.OffsetOutG = p1DLutParams->pCCM[13];
                pIecpState->CcmState.DW14.OffsetOutB = p1DLutParams->pCCM[14];

                pGamutState->DW0.GlobalModeEnable = false;
                // Still need to set CSC params here
                VeboxInterface_BT2020YUVToRGB(m_veboxHeap, pVeboxIecpParams, pVeboxGamutParams);
                return MOS_STATUS_SUCCESS;
            }

            pGamutState->DW0.GlobalModeEnable = true;
            pGamutState->DW1.CmW = 1023;  // Colorimetric accurate image
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
                usGE_Values[i][0] = (uint16_t)m_BT2020InvPixelValue[i];
                usGE_Values[i][1] =
                    usGE_Values[i][2] =
                    usGE_Values[i][3] = (uint16_t)m_BT2020InvGammaLUT[i];

                usGE_Values[i][4] = (uint16_t)m_BT2020InvGammaLUT[i];
                usGE_Values[i][5] =
                    usGE_Values[i][6] =
                    usGE_Values[i][7] = (uint16_t)m_BT2020FwdGammaLUT[i];
            }
            // Copy two UNT16 to one DW(UNT32).
            MOS_SecureMemcpy(pVeboxGEGammaCorrection, sizeof(uint32_t) * 1024, usGE_Values, sizeof(uint16_t) * 8 * 256);
            // Back end CSC setting, need to convert BT2020 YUV input to RGB before GE
            VeboxInterface_BT2020YUVToRGB(m_veboxHeap, pVeboxIecpParams, pVeboxGamutParams);
        }
        else if (pVeboxIecpParams && pVeboxIecpParams->s1DLutParams.bActive)
        {
            uint16_t in_val = 0, vchan1_y = 0, vchan2_u = 0, vchan3_v = 0;
            uint32_t nIndex = 0;
            uint16_t* pForwardGamma = (uint16_t*)pVeboxIecpParams->s1DLutParams.p1DLUT;
            MHW_CHK_NULL_RETURN(pForwardGamma);

            // Gamut Expansion setting
            pGamutState->DW0.GlobalModeEnable = true;
            pGamutState->DW1.CmW = 1023;
            dInverseGamma = 1.0;

            for (uint32_t i = 0; i < pVeboxIecpParams->s1DLutParams.LUTSize; i++)
            {
                usGE_Values[i][0] = 257 * i;
                usGE_Values[i][1] =
                    usGE_Values[i][2] =
                    usGE_Values[i][3] = 257 * i;

                nIndex = 4 * i;
                in_val = pForwardGamma[nIndex];
                vchan1_y = pForwardGamma[nIndex + 1];
                vchan2_u = pForwardGamma[nIndex + 2];
                vchan3_v = pForwardGamma[nIndex + 3];

                // ayuv: in_val, vchan1_y, vchan2_u, vchan3_v
                usGE_Values[i][4] = (i == 0) ? 0 : ((i == 255) ? 0xffff : in_val);
                usGE_Values[i][5] = vchan1_y;
                usGE_Values[i][6] = vchan2_u;
                usGE_Values[i][7] = vchan3_v;
            }
            pGamutState->DW1.C0 = 65536;
            pGamutState->DW0.C1 = 0;
            pGamutState->DW3.C2 = 0;
            pGamutState->DW2.C3 = 0;
            pGamutState->DW5.C4 = 65536;
            pGamutState->DW4.C5 = 0;
            pGamutState->DW7.C6 = 0;
            pGamutState->DW6.C7 = 0;
            pGamutState->DW8.C8 = 65536;
            pGamutState->DW9.OffsetInR = 0;
            pGamutState->DW10.OffsetInG = 0;
            pGamutState->DW11.OffsetInB = 0;
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
        return MOS_STATUS_SUCCESS;
    }


    MOS_STATUS VeboxInterface_H2SManualMode(
        MHW_VEBOX_HEAP              *pVeboxHeapInput,
        MHW_VEBOX_IECP_PARAMS       *pVeboxIecpParams,
        MHW_VEBOX_GAMUT_PARAMS      *pVeboxGamutParams)
    {
        MHW_VEBOX_HEAP                          *pVeboxHeap;
        uint32_t                                uiOffset;

        mhw::vebox::xe_hpm::Cmd::VEBOX_IECP_STATE_CMD* pIecpState;
        mhw::vebox::xe_hpm::Cmd::VEBOX_GAMUT_CONTROL_STATE_CMD* pGamutState;
        mhw::vebox::xe_hpm::Cmd::Gamut_Expansion_Gamma_Correction_CMD* pVeboxGEGammaCorrection, VeboxGeGammaCorrection;
        MOS_STATUS                              eStatus = MOS_STATUS_SUCCESS;

        // HDR H2S algorithm related
        int32_t                                 iToneMappingX[5] = { 40, 200, 1000, 2000, 4000 };
        int32_t                                 iToneMappingY[4] = { 2500, 5000, 10000, 10000 };
        float                                   fPivotX[5] = { 0.0, 0.0, 0.0, 0.0, 0.0 };
        float                                   fPivotY[4] = { 0.0, 0.0, 0.0, 0.0 };
        float                                   fSlope[5] = { 0.0, 0.0, 0.0, 0.0, 0.0 };
        float                                   fMaxCLL = 0.0;
        // OETF parameters, corresponding to input
        uint32_t                                uiOETF[HDR_OETF_1DLUT_POINT_NUMBER] = { 0 };
        uint16_t                                usGE_Values[256][8] = { 0 };

        MHW_CHK_NULL_RETURN(pVeboxGamutParams);
        MHW_CHK_NULL_RETURN(pVeboxHeapInput);

        pVeboxHeap = pVeboxHeapInput;
        uiOffset = pVeboxHeap->uiCurState * pVeboxHeap->uiInstanceSize;
        pIecpState = (mhw::vebox::xe_hpm::Cmd::VEBOX_IECP_STATE_CMD*)(pVeboxHeap->pLockedDriverResourceMem + pVeboxHeap->uiIecpStateOffset + uiOffset);
        pVeboxGEGammaCorrection = (mhw::vebox::xe_hpm::Cmd::Gamut_Expansion_Gamma_Correction_CMD*)(pVeboxHeap->pLockedDriverResourceMem
            + pVeboxHeap->uiGamutStateOffset + uiOffset);
        fMaxCLL = (65535 * (float)pVeboxGamutParams->uiMaxCLL) / 10000;

        MHW_CHK_NULL_RETURN(pIecpState);
        MHW_CHK_NULL_RETURN(pVeboxGEGammaCorrection);

        // Must initialize VeboxIecpState even if it is not used because GCE
        // requires GlobalIECP enable bit to be turned on
        if (!pVeboxIecpParams)
        {
            //IecpStateInitialization(pIecpState);
        }
        pGamutState = &pIecpState->GamutState;

        for (int32_t i = 0; i < 4; i++)
        {
            fPivotX[i] = (iToneMappingY[i] < 10000) ? (65535 * (float)iToneMappingX[i]) / 10000 : MOS_MIN((65535 * (float)iToneMappingX[i]) / 10000, fMaxCLL);
            fPivotY[i] = (65535 * (float)iToneMappingY[i]) / 10000;
        }
        fPivotX[4] = MOS_MIN((65535 * (float)iToneMappingX[4]) / 10000, fMaxCLL);

        // Slope
        fSlope[0] = fPivotX[0] > 0 ? (float)(fPivotY[0] / fPivotX[0]) : 0;
        fPivotY[0] = fSlope[0] * fPivotX[0];
        for (int32_t i = 1; i < 4; i++)
        {
            fSlope[i] = (fPivotX[i] - fPivotX[i - 1]) > 0 ? (float)(fPivotY[i] - fPivotY[i - 1]) / (fPivotX[i] - fPivotX[i - 1]) : 0;
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
        uiOETF[0] = 0;
        uiOETF[255] = 65535;

        // Back end CSC setting, need to convert BT2020 YUV input to RGB before GE
        VeboxInterface_BT2020YUVToRGB(pVeboxHeap, pVeboxIecpParams, pVeboxGamutParams);

        // Global setting
        pGamutState->DW0.GlobalModeEnable = true;
        pGamutState->DW1.CmW = 1023; // Colorimetric accurate image

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

    MOS_STATUS VeboxAdjustBoundary(
        PMHW_VEBOX_SURFACE_PARAMS pCurrSurf,
        uint32_t *                pdwSurfaceWidth,
        uint32_t *                pdwSurfaceHeight,
        bool                      bDIEnable,
        bool                      b3DlutEnable)
    {
        MOS_STATUS           eStatus   = MOS_STATUS_SUCCESS;
        MEDIA_FEATURE_TABLE *pSkuTable = nullptr;

        MHW_CHK_NULL_RETURN(pdwSurfaceWidth);
        MHW_CHK_NULL_RETURN(pdwSurfaceHeight);
        MHW_CHK_NULL_RETURN(this->m_osItf);

        pSkuTable = this->m_osItf->pfnGetSkuTable(this->m_osItf);
        MHW_CHK_NULL_RETURN(pSkuTable);

        MHW_CHK_STATUS_RETURN(AdjustBoundary(pCurrSurf, pdwSurfaceWidth, pdwSurfaceHeight, bDIEnable));

        // match the vebox width with sfc input width to fix corruption issue when sfc scalability enabled in emu
        if (m_veboxScalabilityEnabled && m_usingSfc && this->m_osItf->bSimIsActive)
        {
            *pdwSurfaceWidth = MOS_ALIGN_CEIL(*pdwSurfaceWidth, 16);
            *pdwSurfaceHeight = MOS_ALIGN_CEIL(*pdwSurfaceHeight, 4);
        }

        if (b3DlutEnable && MEDIA_IS_SKU(pSkuTable, FtrHeight8AlignVE3DLUTDualPipe))
        {
            *pdwSurfaceHeight = MOS_ALIGN_CEIL(*pdwSurfaceHeight, 8);
            MHW_NORMALMESSAGE("Align Frame Height as 8x due to 3DlutEnable");
        }

        return eStatus;
    }

    //!
    //! \brief      Add VEBOX ACE/LACE States
    //! \details    Add Vebox ACE, Lace states
    //! \param      [in] pVeboxIecpParams
    //!             Pointer to VEBOX IECP State Params
    //! \return     voild
    //!
    void SetVeboxAceLaceState(
        MHW_VEBOX_IECP_PARAMS* pVeboxIecpParams,
        mhw::vebox::xe_hpm::Cmd::VEBOX_IECP_STATE_CMD* pVeboxIecpState)
    {
        MHW_ACE_PARAMS* pAceParams;
        MHW_VEBOX_HEAP* pVeboxHeap;
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

    MOS_STATUS SetVeboxIecpAceState(
        PMHW_VEBOX_IECP_PARAMS pVeboxIecpParams)
    {
        MHW_ACE_PARAMS* pAceParams;
        MHW_LACE_PARAMS* pLaceParams;
        MHW_VEBOX_HEAP* pVeboxHeap;
        int32_t          uiOffset;
        MOS_STATUS       eStatus = MOS_STATUS_SUCCESS;
        const uint32_t   uiFullRangeYOffsetInU16 = 0;
        const uint32_t   uiLimitedRangeYOffsetInU16 = 4096;
        const uint32_t   uiUOffsetInU16 = 32768;
        const uint32_t   uiVOffsetInU16 = 32768;

        mhw::vebox::xe_hpm::Cmd::VEBOX_IECP_STATE_CMD* pVeboxIecpState;

        MHW_CHK_NULL_RETURN(pVeboxIecpParams);
        MHW_CHK_NULL_RETURN(m_veboxHeap);

        pVeboxHeap = m_veboxHeap;
        uiOffset = pVeboxHeap->uiCurState * pVeboxHeap->uiInstanceSize;

        pVeboxIecpState = (mhw::vebox::xe_hpm::Cmd::VEBOX_IECP_STATE_CMD*)(pVeboxHeap->pLockedDriverResourceMem +
            pVeboxHeap->uiIecpStateOffset +
            uiOffset);

        MHW_CHK_NULL_RETURN(pVeboxIecpState);

        SetVeboxAceLaceState(pVeboxIecpParams, pVeboxIecpState);

        if (pVeboxIecpParams->ColorPipeParams.bActive &&
            pVeboxIecpParams->ColorPipeParams.bEnableLACE)
        {
            pLaceParams = &pVeboxIecpParams->ColorPipeParams.LaceParams;

            pVeboxIecpState->AceState.DW0.MinAceLuma = pLaceParams->wMinAceLuma;
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

            pVeboxIecpState->AceState.DW15.LaceGammaCurveBias0 = m_laceColorCorrection.colorWeightLut.iBias[0];
            pVeboxIecpState->AceState.DW15.LaceGammaCurvePoint0 = m_laceColorCorrection.colorWeightLut.iPoint[0];
            pVeboxIecpState->AceState.DW15.LaceGammaCurveSlope0 = m_laceColorCorrection.colorWeightLut.iSlope[0];

            pVeboxIecpState->AceState.DW16.LaceGammaCurveBias1 = m_laceColorCorrection.colorWeightLut.iBias[1];
            pVeboxIecpState->AceState.DW16.LaceGammaCurvePoint1 = m_laceColorCorrection.colorWeightLut.iPoint[1];
            pVeboxIecpState->AceState.DW16.LaceGammaCurveSlope1 = m_laceColorCorrection.colorWeightLut.iSlope[1];

            pVeboxIecpState->AceState.DW17.LaceGammaCurveBias2 = m_laceColorCorrection.colorWeightLut.iBias[2];
            pVeboxIecpState->AceState.DW17.LaceGammaCurvePoint2 = m_laceColorCorrection.colorWeightLut.iPoint[2];
            pVeboxIecpState->AceState.DW17.LaceGammaCurveSlope2 = m_laceColorCorrection.colorWeightLut.iSlope[2];

            pVeboxIecpState->AceState.DW18.LaceGammaCurveBias3 = m_laceColorCorrection.colorWeightLut.iBias[3];
            pVeboxIecpState->AceState.DW18.LaceGammaCurvePoint3 = m_laceColorCorrection.colorWeightLut.iPoint[3];
            pVeboxIecpState->AceState.DW18.LaceGammaCurveSlope3 = m_laceColorCorrection.colorWeightLut.iSlope[3];

            pVeboxIecpState->AceState.DW19.LaceGammaCurveBias4 = m_laceColorCorrection.colorWeightLut.iBias[4];
            pVeboxIecpState->AceState.DW19.LaceGammaCurvePoint4 = m_laceColorCorrection.colorWeightLut.iPoint[4];
            pVeboxIecpState->AceState.DW19.LaceGammaCurveSlope4 = m_laceColorCorrection.colorWeightLut.iSlope[4];

            pVeboxIecpState->AceState.DW20.LaceGammaCurveBias5 = m_laceColorCorrection.colorWeightLut.iBias[5];
            pVeboxIecpState->AceState.DW20.LaceGammaCurvePoint5 = m_laceColorCorrection.colorWeightLut.iPoint[5];
            pVeboxIecpState->AceState.DW20.LaceGammaCurveSlope5 = m_laceColorCorrection.colorWeightLut.iSlope[5];

            pVeboxIecpState->AceState.DW21.LaceGammaCurveBias6 = m_laceColorCorrection.colorWeightLut.iBias[6];
            pVeboxIecpState->AceState.DW21.LaceGammaCurvePoint6 = m_laceColorCorrection.colorWeightLut.iPoint[6];
            pVeboxIecpState->AceState.DW21.LaceGammaCurveSlope6 = m_laceColorCorrection.colorWeightLut.iSlope[6];

            pVeboxIecpState->AceState.DW22.LaceGammaCurveBias7 = m_laceColorCorrection.colorWeightLut.iBias[7];
            pVeboxIecpState->AceState.DW22.LaceGammaCurvePoint7 = m_laceColorCorrection.colorWeightLut.iPoint[7];
            pVeboxIecpState->AceState.DW22.LaceGammaCurveSlope7 = m_laceColorCorrection.colorWeightLut.iSlope[7];

            pVeboxIecpState->AceState.DW23.LaceGammaCurveBias8 = m_laceColorCorrection.colorWeightLut.iBias[8];
            pVeboxIecpState->AceState.DW23.LaceGammaCurvePoint8 = m_laceColorCorrection.colorWeightLut.iPoint[8];
            pVeboxIecpState->AceState.DW23.LaceGammaCurveSlope8 = m_laceColorCorrection.colorWeightLut.iSlope[8];

            pVeboxIecpState->AceState.DW24.LaceGammaCurveBias9 = m_laceColorCorrection.colorWeightLut.iBias[9];
            pVeboxIecpState->AceState.DW24.LaceGammaCurvePoint9 = m_laceColorCorrection.colorWeightLut.iPoint[9];
            pVeboxIecpState->AceState.DW24.LaceGammaCurveSlope9 = m_laceColorCorrection.colorWeightLut.iSlope[9];

            pVeboxIecpState->AceState.DW25.LaceGammaCurveBias10 = m_laceColorCorrection.colorWeightLut.iBias[10];
            pVeboxIecpState->AceState.DW25.LaceGammaCurvePoint10 = m_laceColorCorrection.colorWeightLut.iPoint[10];
            pVeboxIecpState->AceState.DW25.LaceGammaCurveSlope10 = m_laceColorCorrection.colorWeightLut.iSlope[10];

            pVeboxIecpState->AceState.DW26.LaceGammaCurveBias11 = m_laceColorCorrection.colorWeightLut.iBias[11];
            pVeboxIecpState->AceState.DW26.LaceGammaCurvePoint11 = m_laceColorCorrection.colorWeightLut.iPoint[11];
            pVeboxIecpState->AceState.DW26.LaceGammaCurveSlope11 = m_laceColorCorrection.colorWeightLut.iSlope[11];

            pVeboxIecpState->AceState.DW27.LaceGammaCurveBias12 = m_laceColorCorrection.colorWeightLut.iBias[12];
            pVeboxIecpState->AceState.DW27.LaceGammaCurvePoint12 = m_laceColorCorrection.colorWeightLut.iPoint[12];
            pVeboxIecpState->AceState.DW27.LaceGammaCurveSlope12 = m_laceColorCorrection.colorWeightLut.iSlope[12];

            pVeboxIecpState->AceState.DW28.LaceGammaCurveBias13 = m_laceColorCorrection.colorWeightLut.iBias[13];
            pVeboxIecpState->AceState.DW28.LaceGammaCurvePoint13 = m_laceColorCorrection.colorWeightLut.iPoint[13];
            pVeboxIecpState->AceState.DW28.LaceGammaCurveSlope13 = m_laceColorCorrection.colorWeightLut.iSlope[13];

            pVeboxIecpState->AceState.DW29.LaceGammaCurveBias14 = m_laceColorCorrection.colorWeightLut.iBias[14];
            pVeboxIecpState->AceState.DW29.LaceGammaCurvePoint14 = m_laceColorCorrection.colorWeightLut.iPoint[14];
            pVeboxIecpState->AceState.DW29.LaceGammaCurveSlope14 = m_laceColorCorrection.colorWeightLut.iSlope[14];

            pVeboxIecpState->AceState.DW30.LaceGammaCurveBias15 = m_laceColorCorrection.colorWeightLut.iBias[15];
            pVeboxIecpState->AceState.DW30.LaceGammaCurvePoint15 = m_laceColorCorrection.colorWeightLut.iPoint[15];
            pVeboxIecpState->AceState.DW30.LaceGammaCurveSlope15 = m_laceColorCorrection.colorWeightLut.iSlope[15];
        }

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
             mhw::vebox::xe_hpm::Cmd::VEBOX_STD_STE_STATE_CMD* pVeboxStdSteState,
            MHW_COLORPIPE_PARAMS* pColorPipeParams)
        {
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

            // Enable Skin Score Output surface to be written by Vebox
            pVeboxStdSteState->DW1.StdScoreOutput = pColorPipeParams->bEnableLACE && pColorPipeParams->LaceParams.bSTD;

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
        mhw::vebox::xe_hpm::Cmd::VEBOX_TCC_STATE_CMD* pVeboxTccState,
        MHW_COLORPIPE_PARAMS                    *pColorPipeParams)
    {
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
        mhw::vebox::xe_hpm::Cmd::VEBOX_ACE_LACE_STATE_CMD* pVeboxAceLaceState,
        mhw::vebox::xe_hpm::Cmd::VEBOX_ALPHA_AOI_STATE_CMD* pVeboxAlphaAoiState,
        bool                                             bEnableLACE)
    {
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
    //! \brief      Set Vebox Iecp State FECSC
    //! \details    Set Front-End CSC part of the VEBOX IECP States
    //! \param      [in] pVeboxIecpState
    //!             Pointer to VEBOX IECP States
    //! \param      [in] pVeboxIecpParams
    //!             Pointer to VEBOX IECP State Params
    //! \return     void
    //!
    void SetVeboxIecpStateFecsc(
        mhw::vebox::xe_hpm::Cmd::VEBOX_FRONT_END_CSC_STATE_CMD *pVeboxFecscState,
        MHW_VEBOX_IECP_PARAMS                *pVeboxIecpParams)
    {
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
        mhw::vebox::xe_hpm::Cmd::VEBOX_IECP_STATE_CMD* pVeboxIecpState,
        MHW_CAPPIPE_PARAMS                        *pCapPipeParams,
        const unsigned int                         uCoeffValue)
    {
        mhw::vebox::xe_hpm::Cmd::VEBOX_CCM_STATE_CMD *pCcm = nullptr;

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
        mhw::vebox::xe_hpm::Cmd::VEBOX_PROCAMP_STATE_CMD *pVeboxProcampState,
        MHW_PROCAMP_PARAMS                           *pProcAmpParams)
    {
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
        MHW_CAPPIPE_PARAMS *pCapPipeParams)
    {
        mhw::vebox::xe_hpm::Cmd::VEBOX_CAPTURE_PIPE_STATE_CMD *pVeboxCapPipeState, CapPipCmd;

        MHW_VEBOX_HEAP *pVeboxHeap;
        uint32_t        uiOffset;

        MHW_CHK_NULL_NO_STATUS_RETURN(pCapPipeParams);
        MHW_CHK_NULL_NO_STATUS_RETURN(m_veboxHeap);

        pVeboxHeap = m_veboxHeap;
        uiOffset   = pVeboxHeap->uiCurState * pVeboxHeap->uiInstanceSize;

        pVeboxCapPipeState =
            (mhw::vebox::xe_hpm::Cmd::VEBOX_CAPTURE_PIPE_STATE_CMD *)(pVeboxHeap->pLockedDriverResourceMem +
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

    MOS_STATUS SetVeboxIecpState(
        PMHW_VEBOX_IECP_PARAMS pVeboxIecpParams)
    {
        bool                   bEnableFECSC = false;
        PMHW_FORWARD_GAMMA_SEG pFwdGammaSeg;
        uint8_t *              p3DLUT;
        MHW_VEBOX_HEAP         *pVeboxHeap;
        uint32_t               uiOffset;
        MOS_STATUS             eStatus = MOS_STATUS_SUCCESS;

        mhw::vebox::xe_hpm::Cmd::VEBOX_IECP_STATE_CMD *pVeboxIecpState;

        MHW_CHK_NULL_RETURN(pVeboxIecpParams);
        MHW_CHK_NULL_RETURN(m_veboxHeap);

        pVeboxHeap      = m_veboxHeap;
        uiOffset        = pVeboxHeap->uiCurState * pVeboxHeap->uiInstanceSize;
        pVeboxIecpState = (mhw::vebox::xe_hpm::Cmd::VEBOX_IECP_STATE_CMD *)(pVeboxHeap->pLockedDriverResourceMem +
                                                                          pVeboxHeap->uiIecpStateOffset +
                                                                          uiOffset);

        MHW_CHK_NULL_RETURN(pVeboxIecpState);

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

    MOS_STATUS SetVeboxDndiState(
            PMHW_VEBOX_DNDI_PARAMS pVeboxDndiParams)
    {
        MHW_VEBOX_HEAP  *pVeboxHeap = nullptr;
        uint32_t        uiOffset   = 0;
        MOS_STATUS      eStatus    = MOS_STATUS_SUCCESS;

       mhw::vebox::xe_hpm::Cmd::VEBOX_DNDI_STATE_CMD *pVeboxDndiState, mVeboxDndiState;

        MHW_CHK_NULL_RETURN(pVeboxDndiParams);
        MHW_CHK_NULL_RETURN(m_veboxHeap);
        pVeboxHeap = m_veboxHeap;

        uiOffset = pVeboxHeap->uiCurState * pVeboxHeap->uiInstanceSize;
        pVeboxDndiState =
            (mhw::vebox::xe_hpm::Cmd::VEBOX_DNDI_STATE_CMD *)(pVeboxHeap->pLockedDriverResourceMem +
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

        if (bHVSAutoBdrateEnable)
        {
            if (bTGNEEnable)
            {
                pVeboxDndiState->DW3.TemporalGneEnable = bTGNEEnable;
                pVeboxDndiState->DW4.BlockNoiseEstimateNoiseThreshold = 720;
                pVeboxDndiState->DW6.BlockNoiseEstimateEdgeThreshold = 200;
                pVeboxDndiState->DW30.EightDirectionEdgeThreshold = 3200;
                pVeboxDndiState->DW30.ValidPixelThreshold = 336;
                pVeboxDndiState->DW33.MaxSobelThreshold = 448;
                pVeboxDndiState->DW49.ChromaStadTh = dwChromaStadTh;
                pVeboxDndiState->DW49.LumaStadTh = dwLumaStadTh;
                pVeboxDndiState->DW50.LumaUniformityHighTh2 = 0;
                pVeboxDndiState->DW50.LumaUniformityHighTh1 = 9;
                pVeboxDndiState->DW50.LumaUniformityLowTh2 = 2;
                pVeboxDndiState->DW50.LumaUniformityLowTh1 = 2;
                pVeboxDndiState->DW51.ChromaUniformityHighTh2 = 0;
                pVeboxDndiState->DW51.ChromaUniformityHighTh1 = 9;
                pVeboxDndiState->DW51.ChromaUniformityLowTh2 = 2;
                pVeboxDndiState->DW51.ChromaUniformityLowTh1 = 1;
                pVeboxDndiState->DW52._4X4TemporalGneThresholdCount = dw4X4TGNEThCnt;
            }
            else
            {
                pVeboxDndiState->DW3.TemporalGneEnable = 0;
                pVeboxDndiState->DW4.BlockNoiseEstimateNoiseThreshold = 720;
                pVeboxDndiState->DW6.BlockNoiseEstimateEdgeThreshold = 200;
                pVeboxDndiState->DW30.EightDirectionEdgeThreshold = 3200;
                pVeboxDndiState->DW30.ValidPixelThreshold = 336;
                pVeboxDndiState->DW33.MaxSobelThreshold = 448;
                pVeboxDndiState->DW49.ChromaStadTh = 0;
                pVeboxDndiState->DW49.LumaStadTh = 0;
                pVeboxDndiState->DW50.LumaUniformityHighTh2 = 0;
                pVeboxDndiState->DW50.LumaUniformityHighTh1 = 0;
                pVeboxDndiState->DW50.LumaUniformityLowTh2 = 0;
                pVeboxDndiState->DW50.LumaUniformityLowTh1 = 0;
                pVeboxDndiState->DW51.ChromaUniformityHighTh2 = 0;
                pVeboxDndiState->DW51.ChromaUniformityHighTh1 = 0;
                pVeboxDndiState->DW51.ChromaUniformityLowTh2 = 0;
                pVeboxDndiState->DW51.ChromaUniformityLowTh1 = 0;
                pVeboxDndiState->DW52._4X4TemporalGneThresholdCount = 0;
            }
        }
        else if (bHVSAutoSubjectiveEnable)
        {
            if (bTGNEEnable)
            {
                pVeboxDndiState->DW3.TemporalGneEnable = bTGNEEnable;
                pVeboxDndiState->DW4.BlockNoiseEstimateNoiseThreshold = 720;
                pVeboxDndiState->DW6.BlockNoiseEstimateEdgeThreshold = 200;
                pVeboxDndiState->DW30.EightDirectionEdgeThreshold = 3200;
                pVeboxDndiState->DW30.ValidPixelThreshold = 336;
                pVeboxDndiState->DW33.MaxSobelThreshold = 1440;
                pVeboxDndiState->DW49.ChromaStadTh = dwChromaStadTh;
                pVeboxDndiState->DW49.LumaStadTh = dwLumaStadTh;
                pVeboxDndiState->DW50.LumaUniformityHighTh2 = 50;
                pVeboxDndiState->DW50.LumaUniformityHighTh1 = 15;
                pVeboxDndiState->DW50.LumaUniformityLowTh2 = 2;
                pVeboxDndiState->DW50.LumaUniformityLowTh1 = 2;
                pVeboxDndiState->DW51.ChromaUniformityHighTh2 = 30;
                pVeboxDndiState->DW51.ChromaUniformityHighTh1 = 15;
                pVeboxDndiState->DW51.ChromaUniformityLowTh2 = 2;
                pVeboxDndiState->DW51.ChromaUniformityLowTh1 = 1;
                pVeboxDndiState->DW52._4X4TemporalGneThresholdCount = dw4X4TGNEThCnt;
            }
            else
            {
                pVeboxDndiState->DW3.TemporalGneEnable = 0;
                pVeboxDndiState->DW4.BlockNoiseEstimateNoiseThreshold = 720;
                pVeboxDndiState->DW6.BlockNoiseEstimateEdgeThreshold = 200;
                pVeboxDndiState->DW30.EightDirectionEdgeThreshold = 3200;
                pVeboxDndiState->DW30.ValidPixelThreshold = 336;
                pVeboxDndiState->DW33.MaxSobelThreshold = 1440;
                pVeboxDndiState->DW49.ChromaStadTh = 0;
                pVeboxDndiState->DW49.LumaStadTh = 0;
                pVeboxDndiState->DW50.LumaUniformityHighTh2 = 0;
                pVeboxDndiState->DW50.LumaUniformityHighTh1 = 0;
                pVeboxDndiState->DW50.LumaUniformityLowTh2 = 0;
                pVeboxDndiState->DW50.LumaUniformityLowTh1 = 0;
                pVeboxDndiState->DW51.ChromaUniformityHighTh2 = 0;
                pVeboxDndiState->DW51.ChromaUniformityHighTh1 = 0;
                pVeboxDndiState->DW51.ChromaUniformityLowTh2 = 0;
                pVeboxDndiState->DW51.ChromaUniformityLowTh1 = 0;
                pVeboxDndiState->DW52._4X4TemporalGneThresholdCount = 0;
            }
        }
        else
        {
            if (bTGNEEnable)
            {
                pVeboxDndiState->DW3.TemporalGneEnable = bTGNEEnable;
                pVeboxDndiState->DW4.BlockNoiseEstimateNoiseThreshold = 320;
                pVeboxDndiState->DW49.ChromaStadTh = dwChromaStadTh;
                pVeboxDndiState->DW49.LumaStadTh = dwLumaStadTh;
                pVeboxDndiState->DW50.LumaUniformityHighTh2 = 50;
                pVeboxDndiState->DW50.LumaUniformityHighTh1 = 10;
                pVeboxDndiState->DW50.LumaUniformityLowTh2 = 2;
                pVeboxDndiState->DW50.LumaUniformityLowTh1 = 1;
                pVeboxDndiState->DW51.ChromaUniformityHighTh2 = 30;
                pVeboxDndiState->DW51.ChromaUniformityHighTh1 = 15;
                pVeboxDndiState->DW51.ChromaUniformityLowTh2 = 2;
                pVeboxDndiState->DW51.ChromaUniformityLowTh1 = 1;
                pVeboxDndiState->DW52._4X4TemporalGneThresholdCount = dw4X4TGNEThCnt;
            }
            else
            {
                pVeboxDndiState->DW3.TemporalGneEnable = 0;
                pVeboxDndiState->DW4.BlockNoiseEstimateNoiseThreshold = 320;
                pVeboxDndiState->DW49.ChromaStadTh = 0;
                pVeboxDndiState->DW49.LumaStadTh = 0;
                pVeboxDndiState->DW50.LumaUniformityHighTh2 = 0;
                pVeboxDndiState->DW50.LumaUniformityHighTh1 = 0;
                pVeboxDndiState->DW50.LumaUniformityLowTh2 = 0;
                pVeboxDndiState->DW50.LumaUniformityLowTh1 = 0;
                pVeboxDndiState->DW51.ChromaUniformityHighTh2 = 0;
                pVeboxDndiState->DW51.ChromaUniformityHighTh1 = 0;
                pVeboxDndiState->DW51.ChromaUniformityLowTh2 = 0;
                pVeboxDndiState->DW51.ChromaUniformityLowTh1 = 0;
                pVeboxDndiState->DW52._4X4TemporalGneThresholdCount = 0;
            }
        }

        return eStatus;
    }

    _MHW_SETCMD_OVERRIDE_DECL(VEBOX_STATE)
    {
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

            HalOcaInterface::OnIndirectState(*this->m_currentCmdBuf, *pOsContext, ResourceParams.presResource, ResourceParams.dwOffset, false, m_veboxSettings.uiDndiStateSize);

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

            HalOcaInterface::OnIndirectState(*this->m_currentCmdBuf, *pOsContext, ResourceParams.presResource, ResourceParams.dwOffset, false, m_veboxSettings.uiIecpStateSize);

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

                HalOcaInterface::OnIndirectState(*this->m_currentCmdBuf, *pOsContext, ResourceParams.presResource, ResourceParams.dwOffset, false, m_veboxSettings.uiHdrStateSize);
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

                HalOcaInterface::OnIndirectState(*this->m_currentCmdBuf, *pOsContext, ResourceParams.presResource, ResourceParams.dwOffset, false, m_veboxSettings.uiGamutStateSize);
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

            HalOcaInterface::OnIndirectState(*this->m_currentCmdBuf, *pOsContext, ResourceParams.presResource, ResourceParams.dwOffset, false, m_veboxSettings.uiVertexTableSize);

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

            HalOcaInterface::OnIndirectState(*this->m_currentCmdBuf, *pOsContext, ResourceParams.presResource, ResourceParams.dwOffset, false, m_veboxSettings.uiCapturePipeStateSize);

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

            HalOcaInterface::OnIndirectState(*this->m_currentCmdBuf, *pOsContext, ResourceParams.presResource, ResourceParams.dwOffset, false, m_veboxSettings.uiGammaCorrectionStateSize);

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

            HalOcaInterface::OnIndirectState(*this->m_currentCmdBuf, *pOsContext, ResourceParams.presResource, 0, true, 0);
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
            MOS_MP_RESOURCE_USAGE_DEFAULT,
            pOsInterface->pfnGetGmmClientContext(pOsInterface)))
            .DwordValue;

        //cmd.DW17.EncDataControlFor3DLUT = 0;

        cmd.DW17.ArbitrationPriorityControlForLut3D = params.LUT3D.ArbitrationPriorityControl;
        // In GmmCachePolicyExt.h, Gen9/Gen10/Gen11/Gen12+ has the same definition for MEMORY_OBJECT_CONTROL_STATE.
        // In MHW_MEMORY_OBJECT_CONTROL_PARAMS, we only defined Gen9 which intended to use for Gen9 later, so reuse Gen9 index.
        //cmd.DW17.Lut3DMOCStable = params.Vebox3DLookUpTablesSurfCtrl.Gen9.Index;
        cmd.DW18.Lut3DEnable = params.LUT3D.Lut3dEnable;
        cmd.DW18.Lut3DSize = params.LUT3D.Lut3dSize;

        cmd.DW18.ChromaUpsamplingCoSitedHorizontalOffset = params.ChromaSampling.ChromaUpsamplingCoSitedHorizontalOffset;
        cmd.DW18.ChromaUpsamplingCoSitedVerticalOffset = params.ChromaSampling.ChromaUpsamplingCoSitedVerticalOffset;
        cmd.DW18.ChromaDownsamplingCoSitedHorizontalOffset = params.ChromaSampling.ChromaDownsamplingCoSitedHorizontalOffset;
        cmd.DW18.ChromaDownsamplingCoSitedVerticalOffset = params.ChromaSampling.ChromaDownsamplingCoSitedVerticalOffset;
        cmd.DW18.BypassChromaUpsampling = params.ChromaSampling.BypassChromaUpsampling;
        cmd.DW18.BypassChromaDownsampling = params.ChromaSampling.BypassChromaDownsampling;

        return eStatus;
    }

    _MHW_SETCMD_OVERRIDE_DECL(VEBOX_TILING_CONVERT)
    {
        _MHW_SETCMD_CALLBASE(VEBOX_TILING_CONVERT);

        mhw::vebox::xe_hpm::Cmd::VEB_DI_IECP_COMMAND_SURFACE_CONTROL_BITS_CMD veboxInputSurfCtrlBits, veboxOutputSurfCtrlBits;
        MHW_RESOURCE_PARAMS                               resourceParams = {0};
        PMOS_RESOURCE                           surface = nullptr;

        MHW_CHK_NULL_RETURN(this->m_currentCmdBuf);
        MHW_CHK_NULL_RETURN(this->m_osItf);

        auto& veboxSurfaceStateCmdParams = MHW_GETPAR_F(VEBOX_SURFACE_STATE)();
        MHW_CHK_NULL_RETURN(&veboxSurfaceStateCmdParams.SurfInput);
        MHW_CHK_NULL_RETURN(&veboxSurfaceStateCmdParams.SurfOutput);

        PMOS_RESOURCE inputSurface  = veboxSurfaceStateCmdParams.SurfInput.pOsResource;
        PMOS_RESOURCE outputSurface = veboxSurfaceStateCmdParams.SurfOutput.pOsResource;

        MHW_CHK_NULL_RETURN(inputSurface);
        MHW_CHK_NULL_RETURN(outputSurface);

        // Set up VEB_DI_IECP_COMMAND_SURFACE_CONTROL_BITS
        veboxInputSurfCtrlBits  = {};
        veboxOutputSurfCtrlBits = {};

        veboxInputSurfCtrlBits.DW0.IndexToMemoryObjectControlStateMocsTables =
            veboxOutputSurfCtrlBits.DW0.IndexToMemoryObjectControlStateMocsTables =
                (this->m_osItf->pfnCachePolicyGetMemoryObject(
                     MOS_MP_RESOURCE_USAGE_DEFAULT,
                    this->m_osItf->pfnGetGmmClientContext(this->m_osItf)))
                    .Gen12.Index;

        // Set Input surface compression status
        if (veboxSurfaceStateCmdParams.SurfInput.CompressionMode != MOS_MMC_DISABLED)
        {
            veboxInputSurfCtrlBits.DW0.MemoryCompressionEnable = true;

            if (veboxSurfaceStateCmdParams.SurfInput.CompressionMode == MOS_MMC_RC)
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

        // Set Output surface compression status
        if (outputSurface)
        {
            // Double Buffer copy
            surface = outputSurface;

            if (veboxSurfaceStateCmdParams.SurfOutput.CompressionMode != MOS_MMC_DISABLED)
            {
                veboxInputSurfCtrlBits.DW0.MemoryCompressionEnable = true;

                if (veboxSurfaceStateCmdParams.SurfInput.CompressionMode == MOS_MMC_RC)
                {
                    veboxInputSurfCtrlBits.DW0.CompressionType = 1;
                }
                else
                {
                    veboxInputSurfCtrlBits.DW0.CompressionType = 0;
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
        }
        else
        {
            // In-Place Resolve
            surface = inputSurface;

            if (veboxSurfaceStateCmdParams.SurfInput.CompressionMode == MOS_MMC_MC)
            {
                veboxOutputSurfCtrlBits.DW0.MemoryCompressionEnable = true;
                veboxOutputSurfCtrlBits.DW0.CompressionType         = 1;
            }
        }

        MOS_ZeroMemory(&resourceParams, sizeof(MHW_RESOURCE_PARAMS));
        InitMocsParams(resourceParams, &cmd.DW1_2.Value[0], 1, 6);
        resourceParams.presResource  = inputSurface;
        resourceParams.HwCommandType = MOS_VEBOX_TILING_CONVERT;

        // set up DW[2:1], input graphics address
        resourceParams.dwLocationInCmd = 1;
        resourceParams.pdwCmd          = &(cmd.DW1_2.Value[0]);
        resourceParams.bIsWritable     = false;
        resourceParams.dwOffset        = veboxSurfaceStateCmdParams.SurfInput.dwOffset + veboxInputSurfCtrlBits.DW0.Value;
        MHW_CHK_STATUS_RETURN(AddResourceToCmd(this->m_osItf, this->m_currentCmdBuf, &resourceParams));

        MOS_ZeroMemory(&resourceParams, sizeof(MHW_RESOURCE_PARAMS));
        InitMocsParams(resourceParams, &cmd.DW3_4.Value[0], 1, 6);

        if (outputSurface)
        {
            resourceParams.presResource = outputSurface;
        }
        else
        {
            resourceParams.presResource = inputSurface;
        }

        resourceParams.HwCommandType = MOS_VEBOX_TILING_CONVERT;

        // set up DW[4:3], output graphics address
        resourceParams.dwLocationInCmd = 3;
        resourceParams.pdwCmd          = &(cmd.DW3_4.Value[0]);
        resourceParams.bIsWritable     = true;
        resourceParams.dwOffset =
            (outputSurface != nullptr ? veboxSurfaceStateCmdParams.SurfOutput.dwOffset : veboxSurfaceStateCmdParams.SurfInput.dwOffset) + veboxOutputSurfCtrlBits.DW0.Value;
        MHW_CHK_STATUS_RETURN(AddResourceToCmd(this->m_osItf, this->m_currentCmdBuf, &resourceParams));

        return MOS_STATUS_SUCCESS;
    }

    _MHW_SETCMD_OVERRIDE_DECL(VEB_DI_IECP)
    {
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
                mhw::vebox::xe_hpm::Cmd::VEB_DI_IECP_COMMAND_SURFACE_CONTROL_BITS_CMD* pSurfCtrlBits;
                pSurfCtrlBits = (mhw::vebox::xe_hpm::Cmd::VEB_DI_IECP_COMMAND_SURFACE_CONTROL_BITS_CMD*)&params.CurrInputSurfCtrl.Value;
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

            if (m_numofVebox == 2)
            {
                if (m_indexofVebox == MHW_VEBOX_INDEX_0)
                {
                    cmd.DW1.EndingX = iMediumX - 1;
                    cmd.DW1.StartingX = params.dwStartingX;
                }
                else if (m_indexofVebox == MHW_VEBOX_INDEX_1)
                {
                    cmd.DW1.EndingX = params.dwEndingX;
                    cmd.DW1.StartingX = iMediumX;
                }
                else
                {
                    MHW_ASSERTMESSAGE("Unsupported Vebox Scalability Settings");
                }
            }
            else if (m_numofVebox == 3)
            {
                if (m_indexofVebox == MHW_VEBOX_INDEX_0)
                {
                    cmd.DW1.EndingX = iMediumX - 1;
                    cmd.DW1.StartingX = params.dwStartingX;
                }
                else if (m_indexofVebox == MHW_VEBOX_INDEX_1)
                {
                    cmd.DW1.EndingX = 2 * iMediumX - 1;
                    cmd.DW1.StartingX = iMediumX;
                }
                else if (m_indexofVebox == MHW_VEBOX_INDEX_2)
                {
                    cmd.DW1.EndingX = params.dwEndingX;
                    cmd.DW1.StartingX = 2 * iMediumX;
                }
                else
                {
                    MHW_ASSERTMESSAGE("Unsupported Vebox Scalability Settings");
                }
            }
            else if (m_numofVebox == 4)
            {
                if (m_indexofVebox == MHW_VEBOX_INDEX_0)
                {
                    cmd.DW1.EndingX = iMediumX - 1;
                    cmd.DW1.StartingX = params.dwStartingX;
                }
                else if (m_indexofVebox == MHW_VEBOX_INDEX_1)
                {
                    cmd.DW1.EndingX = 2 * iMediumX - 1;
                    cmd.DW1.StartingX = iMediumX;
                }
                else if (m_indexofVebox == MHW_VEBOX_INDEX_2)
                {
                    cmd.DW1.EndingX = 3 * iMediumX - 1;
                    cmd.DW1.StartingX = 2 * iMediumX;
                }
                else if (m_indexofVebox == MHW_VEBOX_INDEX_3)
                {
                    cmd.DW1.EndingX = params.dwEndingX;
                    cmd.DW1.StartingX = 3 * iMediumX;
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

                    if (m_indexofVebox >= MHW_VEBOX_INDEX_1)
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

        return MOS_STATUS_SUCCESS;
    }

protected:
    using base_t = vebox::Impl<mhw::vebox::xe_hpm::Cmd>;
MEDIA_CLASS_DEFINE_END(mhw__vebox__xe_hpm__Impl)
};

}  // namespace xe_hpg
}  // namespace render
}  // namespace mhw

#endif  // __MHW_RENDER_XE_HPG_IMPL_H__
