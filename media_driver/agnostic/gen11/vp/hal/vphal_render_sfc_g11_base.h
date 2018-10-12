/*
* Copyright (c) 2012-2018, Intel Corporation
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
//! \file     vphal_render_sfc_g11_base.h
//! \brief    The header file of the Gen 11 VPHAL SFC rendering component
//! \details  The SFC renderer supports Scaling, IEF, CSC/ColorFill and Rotation.
//!           It's responsible for setting up HW states and generating the SFC
//!           commands.
//!
#ifndef __VPHAL_RENDER_SFC_G11_BASE_H__
#define __VPHAL_RENDER_SFC_G11_BASE_H__

#include "mhw_sfc.h"
#include "vphal_render_sfc_base.h"

#if __VPHAL_SFC_SUPPORTED

class VphalSfcStateG11 :virtual public VphalSfcState
{
public:
    VphalSfcStateG11(
        PMOS_INTERFACE       osInterface,
        PRENDERHAL_INTERFACE renderHal,
        PMHW_SFC_INTERFACE   sfcInterface) :VphalSfcState(osInterface, renderHal, sfcInterface)
    {
    }

    virtual ~VphalSfcStateG11()
    {

    }

    //!
    //! \brief    Initialize sfc render data
    //! \return   void
    //!
    virtual void InitRenderData()
    {
        VphalSfcState::InitRenderData();
        m_renderData.SfcStateParams = (MHW_SFC_STATE_PARAMS*)MOS_AllocAndZeroMemory(sizeof(MHW_SFC_STATE_PARAMS));
    }

protected:
    virtual bool IsInputFormatSupported(
        PVPHAL_SURFACE              srcSurface);

    virtual bool IsOutputFormatSupported(
        PVPHAL_SURFACE              outSurface);

    virtual void GetInputWidthHeightAlignUnit(
        MOS_FORMAT              inputFormat,
        MOS_FORMAT              outputFormat,
        uint16_t                &widthAlignUnit,
        uint16_t                &heightAlignUnit);
};

#endif // __VPHAL_SFC_SUPPORTED

#endif // __VPHAL_RENDER_SFC_G11_BASE_H__
