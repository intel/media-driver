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
//! \file     vphal_render_vebox_ste.cpp
//! \brief    VPHAL VEBOX IECP Skin Tone Detect & Enhancement (STD/E) interfaces
//! \details  VPHAL VEBOX IECP Skin Tone Detect & Enhancement (STD/E) interfaces
//!

#include "vphal_render_vebox_ste.h"

#if VPHAL_RENDER_VEBOX_STE_ENABLE

#define MHW_STE_FACTOR_MAX        9  // STE factor is 0 ~ 9

const uint32_t   satP1[MHW_STE_FACTOR_MAX + 1] = {
    0x00000000, 0xfffffffe, 0xfffffffc, 0xfffffffa, 0xfffffff6, 0xfffffff4, 0xfffffff2, 0xfffffff0, 0xffffffee, 0xffffffec };

const uint32_t   satS0[MHW_STE_FACTOR_MAX + 1] = {
    0x000000ef, 0x00000100, 0x00000113, 0x00000129, 0x0000017a, 0x000001a2, 0x000001d3, 0x00000211, 0x00000262, 0x000002d1 };

const uint32_t   satS1[MHW_STE_FACTOR_MAX + 1] = {
    0x000000ab, 0x00000080, 0x00000066, 0x00000055, 0x000000c2, 0x000000b9, 0x000000b0, 0x000000a9, 0x000000a2, 0x0000009c };

//!
//! \brief    Vebox set IECP parameter
//! \details  Set Vebox IECP state parameter
//! \param    [in] pSrcSurface
//!           Pointer to input surface of Vebox
//! \param    [in,out] pRenderData
//!           Pointer to Vebox Render Data
//! \return   void
//!
void VPHAL_VEBOX_IECP_STE::SetParams(
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
void VPHAL_VEBOX_IECP_STE::InitParams(
    PVPHAL_VEBOX_IECP_PARAMS        pVphalVeboxIecpParams,
    PMHW_VEBOX_IECP_PARAMS          pMhwVeboxIecpParams)
{
    PVPHAL_COLORPIPE_PARAMS         pVpHalColorPipeParams;
    PMHW_COLORPIPE_PARAMS           pMhwColorPipeParams;

    pVpHalColorPipeParams = pVphalVeboxIecpParams->pColorPipeParams;
    pMhwColorPipeParams = &pMhwVeboxIecpParams->ColorPipeParams;
    if (pVpHalColorPipeParams)
    {
        if (pVpHalColorPipeParams->SteParams.dwSTEFactor > MHW_STE_FACTOR_MAX)
        {
            pVpHalColorPipeParams->SteParams.dwSTEFactor = MHW_STE_FACTOR_MAX;
        }

        pMhwColorPipeParams->bActive               = true;
        pMhwColorPipeParams->bEnableSTE            = pVpHalColorPipeParams->bEnableSTE;
        pMhwColorPipeParams->SteParams.dwSTEFactor = pVpHalColorPipeParams->SteParams.dwSTEFactor;
        pMhwColorPipeParams->SteParams.satP1       = satP1[pVpHalColorPipeParams->SteParams.dwSTEFactor];
        pMhwColorPipeParams->SteParams.satS0       = satS0[pVpHalColorPipeParams->SteParams.dwSTEFactor];
        pMhwColorPipeParams->SteParams.satS1       = satS1[pVpHalColorPipeParams->SteParams.dwSTEFactor];
    }
}

#endif // VPHAL_RENDER_VEBOX_STE_ENABLE