/*
* Copyright (c) 2010-2019, Intel Corporation
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
//! \file     vphal_render_sfc_g12_base.h
//! \brief    The header file of the Gen12 VPHAL SFC rendering component
//! \details  The SFC renderer supports Scaling, IEF, CSC/ColorFill and Rotation.
//!           It's responsible for setting up HW states and generating the SFC
//!           commands.
//!
#ifndef __VPHAL_RENDER_SFC_G12_BASE_H__
#define __VPHAL_RENDER_SFC_G12_BASE_H__

#include "mhw_sfc_g12_X.h"
#include "vphal_render_sfc_base.h"

#if __VPHAL_SFC_SUPPORTED

class VphalSfcStateG12 :virtual public VphalSfcState
{
public:
    VphalSfcStateG12(
        PMOS_INTERFACE       osInterface,
        PRENDERHAL_INTERFACE renderHal,
        PMHW_SFC_INTERFACE   sfcInterface);

    virtual ~VphalSfcStateG12()
    {

    }

    //!
    //! \brief    Initialize sfc render data
    //! \return   void
    //!
    virtual void InitRenderData()
    {
        VphalSfcState::InitRenderData();
        m_renderData.SfcStateParams = (MHW_SFC_STATE_PARAMS_G12*)MOS_AllocAndZeroMemory(sizeof(MHW_SFC_STATE_PARAMS_G12));
    }

protected:
    virtual bool IsOutputCapable(
        bool            isColorFill,
        PVPHAL_SURFACE  src,
        PVPHAL_SURFACE  renderTarget);

    virtual bool IsInputFormatSupported(
        PVPHAL_SURFACE              srcSurface);

    virtual bool IsOutputFormatSupported(
        PVPHAL_SURFACE              outSurface);

    virtual void GetInputWidthHeightAlignUnit(
        MOS_FORMAT              inputFormat,
        MOS_FORMAT              outputFormat,
        uint16_t                &widthAlignUnit,
        uint16_t                &heightAlignUnit);

    virtual void DetermineCscParams(
        PVPHAL_SURFACE                  src,
        PVPHAL_SURFACE                  renderTarget);

    virtual void DetermineInputFormat(
        PVPHAL_SURFACE                  src,
        PVPHAL_VEBOX_RENDER_DATA        veboxRenderData);

    virtual MOS_STATUS SetSfcStateParams(
        PVPHAL_VEBOX_RENDER_DATA    pRenderData,
        PVPHAL_SURFACE              pSrcSurface,
        PVPHAL_SURFACE              pOutSurface);

    virtual bool IsOutputPipeSfcFeasible(
        PCVPHAL_RENDER_PARAMS       pcRenderParams,
        PVPHAL_SURFACE              pSrcSurface,
        PVPHAL_SURFACE              pRenderTarget);

    virtual void SetRenderingFlags(
        PVPHAL_COLORFILL_PARAMS         pColorFillParams,
        PVPHAL_ALPHA_PARAMS             pAlphaParams,
        PVPHAL_SURFACE                  pSrc,
        PVPHAL_SURFACE                  pRenderTarget,
        PVPHAL_VEBOX_RENDER_DATA        pRenderData);

    //!
    //! \brief    Set SFC MMC States
    //! \details  Update the SFC output MMC status
    //! \param    [in] renderData
    //!           Pointer to Render Data
    //! \param    [in] outSurface
    //!           Pointer to Sfc Output Surface
    //! \param    [in,out] sfcStateParams
    //!           Pointer to SFC State Params
    //! \return   MOS_STATUS
    //!
    virtual MOS_STATUS SetSfcMmcStatus(
        PVPHAL_VEBOX_RENDER_DATA    renderData,
        PVPHAL_SURFACE              outSurface,
        PMHW_SFC_STATE_PARAMS       sfcStateParams);

    //!
    //! \brief    Is the Format MMC Supported in SFC
    //! \details  Check whether input format is supported by memory compression
    //! \param    [in] Format
    //!           Surface format
    //! \return   true if supported, false if not
    //!
    virtual bool IsFormatMMCSupported(
        MOS_FORMAT                  Format);

    bool m_disableOutputCentering;                      //!< flag for whether to disable Output centering, for validation purpose
};

#endif // __VPHAL_SFC_SUPPORTED
#endif // __VPHAL_RENDER_SFC_G12_BASE_H__
