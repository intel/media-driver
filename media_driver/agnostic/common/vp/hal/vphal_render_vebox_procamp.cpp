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
//! \file     vphal_render_vebox_procamp.cpp
//! \brief    VPHAL VEBOX IECP ProcAmp interfaces
//! \details  VPHAL VEBOX IECP ProcAmp interfaces
//!

#include "vphal_render_vebox_procamp.h"

#if VPHAL_RENDER_VEBOX_PROCAMP_ENABLE

//!
//! \brief    Vebox set IECP parameter
//! \details  Set Vebox IECP state parameter
//! \param    [in] pSrcSurface
//!           Pointer to input surface of Vebox
//! \param    [in,out] pRenderData
//!           Pointer to Vebox Render Data
//! \return   void
//!
void VPHAL_VEBOX_IECP_ProcAmp::SetParams(
    PVPHAL_SURFACE              pSrcSurface,
    PVPHAL_VEBOX_RENDER_DATA    pRenderData)
{
    PVPHAL_VEBOX_IECP_PARAMS    pVphalVeboxIecpParams;

    pVphalVeboxIecpParams = pRenderData->GetVeboxIECPParams();

    // Check whether ProcAmp is enabled in Vebox
    if (pRenderData->bProcamp)
    {
        pVphalVeboxIecpParams->pProcAmpParams = pSrcSurface->pProcampParams;

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
void VPHAL_VEBOX_IECP_ProcAmp::InitParams(
    PVPHAL_VEBOX_IECP_PARAMS        pVphalVeboxIecpParams,
    PMHW_VEBOX_IECP_PARAMS          pMhwVeboxIecpParams)
{
    PVPHAL_PROCAMP_PARAMS           pVpHalProcAmpParams;
    PMHW_PROCAMP_PARAMS             pMhwProcAmpParams;

    pVpHalProcAmpParams = pVphalVeboxIecpParams->pProcAmpParams;
    pMhwProcAmpParams = &pMhwVeboxIecpParams->ProcAmpParams;
    if (pVpHalProcAmpParams)
    {
        pMhwProcAmpParams->bActive    = true;
        pMhwProcAmpParams->bEnabled   = pVpHalProcAmpParams->bEnabled;
        pMhwProcAmpParams->brightness = (uint32_t)MOS_F_ROUND(pVpHalProcAmpParams->fBrightness * 16.0F);  // S7.4
        pMhwProcAmpParams->contrast   = (uint32_t)MOS_UF_ROUND(pVpHalProcAmpParams->fContrast * 128.0F);  // U4.7
        pMhwProcAmpParams->sinCS      = (uint32_t)MOS_F_ROUND(sin(MHW_DEGREE_TO_RADIAN(pVpHalProcAmpParams->fHue)) *
                                                         pVpHalProcAmpParams->fContrast *
                                                         pVpHalProcAmpParams->fSaturation * 256.0F);  // S7.8
        pMhwProcAmpParams->cosCS      = (uint32_t)MOS_F_ROUND(cos(MHW_DEGREE_TO_RADIAN(pVpHalProcAmpParams->fHue)) *
                                                         pVpHalProcAmpParams->fContrast *
                                                         pVpHalProcAmpParams->fSaturation * 256.0F);  // S7.8
    }
}

#endif // VPHAL_RENDER_VEBOX_PROCAMP_ENABLE