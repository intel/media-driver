/*
* Copyright (c) 2017, Intel Corporation
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
//! \file     vphal_render_vebox_tcc.cpp
//! \brief    VPHAL VEBOX IECP Total Color Control (TCC) interfaces
//! \details  VPHAL VEBOX IECP Total Color Control (TCC) interfaces
//!

#include "vphal_render_vebox_tcc.h"

#if VPHAL_RENDER_VEBOX_TCC_ENABLE

//!
//! \brief    Vebox set IECP parameter
//! \details  Set Vebox IECP state parameter
//! \param    [in] pSrcSurface
//!           Pointer to input surface of Vebox
//! \param    [in,out] pRenderData
//!           Pointer to Vebox Render Data
//! \return   void
//!
void VPHAL_VEBOX_IECP_TCC::SetParams(
    PVPHAL_SURFACE              pSrcSurface,
    PVPHAL_VEBOX_RENDER_DATA    pRenderData)
{
    PVPHAL_VEBOX_IECP_PARAMS    pVphalVeboxIecpParams;

    pVphalVeboxIecpParams = pRenderData->GetVeboxIECPParams();

    if (pSrcSurface->pColorPipeParams && pRenderData->bColorPipe)
    {
        pVphalVeboxIecpParams->pColorPipeParams = pSrcSurface->pColorPipeParams;

        pRenderData->GetVeboxStateParams()->pVphalVeboxIecpParams = pVphalVeboxIecpParams;
    }
}

//!
//! \brief    Init Vebox IECP parameter
//! \param    [in] pVphalVeboxIecpParams
//!           Pointer to input Vphal Iecp parameters
//! \param    [in,out] pMhwVeboxIecpParams
//!           Pointer to Mhw Iecp parameters
//! \return   void
//!
void VPHAL_VEBOX_IECP_TCC::InitParams(
    PVPHAL_VEBOX_IECP_PARAMS        pVphalVeboxIecpParams,
    PMHW_VEBOX_IECP_PARAMS          pMhwVeboxIecpParams)
{
    PVPHAL_COLORPIPE_PARAMS         pVpHalColorPipeParams;
    PMHW_COLORPIPE_PARAMS           pMhwColorPipeParams;

    pVpHalColorPipeParams = pVphalVeboxIecpParams->pColorPipeParams;
    pMhwColorPipeParams = &pMhwVeboxIecpParams->ColorPipeParams;
    if (pVpHalColorPipeParams)
    {
        pMhwColorPipeParams->bActive            = true;
        pMhwColorPipeParams->bEnableTCC         = pVpHalColorPipeParams->bEnableTCC;
        pMhwColorPipeParams->TccParams.Red      = pVpHalColorPipeParams->TccParams.Red;
        pMhwColorPipeParams->TccParams.Green    = pVpHalColorPipeParams->TccParams.Green;
        pMhwColorPipeParams->TccParams.Blue     = pVpHalColorPipeParams->TccParams.Blue;
        pMhwColorPipeParams->TccParams.Cyan     = pVpHalColorPipeParams->TccParams.Cyan;
        pMhwColorPipeParams->TccParams.Magenta  = pVpHalColorPipeParams->TccParams.Magenta;
        pMhwColorPipeParams->TccParams.Yellow   = pVpHalColorPipeParams->TccParams.Yellow;
    }
}

#endif // VPHAL_RENDER_VEBOX_TCC_ENABLE