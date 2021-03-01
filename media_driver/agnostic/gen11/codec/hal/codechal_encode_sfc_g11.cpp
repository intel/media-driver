/*
* Copyright (c) 2018-2021, Intel Corporation
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
//! \file     codechal_encode_sfc_g11.cpp
//! \brief    Implements the encode interface extension for CSC via VEBox/SFC.
//! \details  Downsampling in this case is supported by the VEBox fixed function HW unit.
//!

#include "codechal_encode_sfc_g11.h"
#include "codechal_encoder_base.h"
#include "mhw_sfc_g11_X.h"

MOS_STATUS CodecHalEncodeSfcG11::SetVeboxDiIecpParams(
    PMHW_VEBOX_DI_IECP_CMD_PARAMS         params)
{
    MOS_ALLOC_GFXRES_PARAMS allocParamsForBufferLinear;
    MOS_STATUS              eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;
    CODECHAL_ENCODE_CHK_NULL_RETURN(params);

    params->dwStartingX = 0;
    params->dwEndingX = m_inputSurfaceRegion.Width - 1; // Must be taken from SPS
    params->dwCurrInputSurfOffset = m_inputSurface->dwOffset;
    params->pOsResCurrInput = &m_inputSurface->OsResource;
    params->CurrInputSurfCtrl.Value = 0;  //Keep it here untill VPHAL moving to new CMD definition and remove this parameter definition.

    CodecHalGetResourceInfo(
        m_osInterface,
        m_inputSurface);
    params->CurInputSurfMMCState = (MOS_MEMCOMP_STATE)(m_inputSurface->CompressionMode);

    // Allocate Resource to avoid Page Fault issue since HW will access it
    if (Mos_ResourceIsNull(&m_resLaceOrAceOrRgbHistogram))
    {
        MOS_ZeroMemory(&allocParamsForBufferLinear, sizeof(MOS_ALLOC_GFXRES_PARAMS));
        allocParamsForBufferLinear.Type = MOS_GFXRES_BUFFER;
        allocParamsForBufferLinear.TileType = MOS_TILE_LINEAR;
        allocParamsForBufferLinear.Format = Format_Buffer;
        allocParamsForBufferLinear.dwBytes = GetResLaceOrAceOrRgbHistogramBufferSize();
        allocParamsForBufferLinear.pBufName = "ResLaceOrAceOrRgbHistogram";

        m_osInterface->pfnAllocateResource(
            m_osInterface,
            &allocParamsForBufferLinear,
            &m_resLaceOrAceOrRgbHistogram);
    }

    params->pOsResLaceOrAceOrRgbHistogram = &m_resLaceOrAceOrRgbHistogram;

    // Allocate Resource to avoid Page Fault issue since HW will access it
    if (Mos_ResourceIsNull(&m_resStatisticsOutput))
    {
        MOS_ZeroMemory(&allocParamsForBufferLinear, sizeof(MOS_ALLOC_GFXRES_PARAMS));
        allocParamsForBufferLinear.Type = MOS_GFXRES_BUFFER;
        allocParamsForBufferLinear.TileType = MOS_TILE_LINEAR;
        allocParamsForBufferLinear.Format = Format_Buffer;
        allocParamsForBufferLinear.dwBytes = GetSfcVeboxStatisticsSize();
        allocParamsForBufferLinear.pBufName = "ResStatisticsOutput";

        m_osInterface->pfnAllocateResource(
            m_osInterface,
            &allocParamsForBufferLinear,
            &m_resStatisticsOutput);
    }

    params->pOsResStatisticsOutput = &m_resStatisticsOutput;

    return eStatus;
}