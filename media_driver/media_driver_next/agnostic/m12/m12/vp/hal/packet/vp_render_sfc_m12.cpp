/*
* Copyright (c) 2020, Intel Corporation
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
//! \file     vp_render_sfc_m12.cpp
//! \brief    SFC rendering component
//! \details  The SFC renderer supports Scaling, IEF, CSC/ColorFill and Rotation.
//!           It's responsible for setting up HW states and generating the SFC
//!           commands.
//!

#include "vp_render_sfc_m12.h"
#include "mhw_sfc_g12_X.h"
#include "mos_defs.h"

using namespace vp;

SfcRenderM12::SfcRenderM12(
    VP_MHWINTERFACE &vpMhwinterface,
    PVpAllocator &allocator):
    SfcRenderBase(vpMhwinterface, allocator)
{
}

SfcRenderM12::~SfcRenderM12()
{
}

MOS_STATUS SfcRenderM12::SetupSfcState(
    PVP_SURFACE                     targetSurface)
{
    MOS_STATUS                eStatus = MOS_STATUS_SUCCESS;
    PMHW_SFC_STATE_PARAMS_G12 sfcStateParamsM12 = nullptr;

    VP_RENDER_CHK_STATUS_RETURN(SfcRenderBase::SetupSfcState(targetSurface));

    //Set SFD Line Buffer
    VP_RENDER_CHK_NULL_RETURN(m_renderData.sfcStateParams);
    sfcStateParamsM12 = static_cast<PMHW_SFC_STATE_PARAMS_G12>(m_renderData.sfcStateParams);
    VP_RENDER_CHK_NULL_RETURN(sfcStateParamsM12);
    sfcStateParamsM12->resSfdLineBuffer = Mos_ResourceIsNull(&m_SFDLineBufferSurface.OsResource) ? nullptr : &m_SFDLineBufferSurface.OsResource;

    return eStatus;
}

MOS_STATUS SfcRenderM12::InitSfcStateParams()
{
    if (nullptr == m_sfcStateParams)
    {
        m_sfcStateParams = (MHW_SFC_STATE_PARAMS_G12*)MOS_AllocAndZeroMemory(sizeof(MHW_SFC_STATE_PARAMS_G12));
    }
    else
    {
        MOS_ZeroMemory(m_sfcStateParams, sizeof(MHW_SFC_STATE_PARAMS_G12));
    }

    VP_PUBLIC_CHK_NULL_RETURN(m_sfcStateParams);

    m_renderData.sfcStateParams = m_sfcStateParams;

    return MOS_STATUS_SUCCESS;
}
