/*
* Copyright (c) 2018, Intel Corporation
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
    uint32_t                    width;
    uint32_t                    height;
    MOS_ALLOC_GFXRES_PARAMS     allocParamsForBufferLinear;
    uint32_t                    size = 0, sizeLace = 0, sizeNoLace = 0;
    MOS_STATUS                  eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;
    CODECHAL_ENCODE_CHK_NULL_RETURN(params);

    height = m_inputSurface->dwHeight;
    width = m_inputSurface->dwWidth;

    params->dwStartingX = 0;
    params->dwEndingX = width - 1;
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
        size = CODECHAL_SFC_VEBOX_RGB_HISTOGRAM_SIZE_G11;
        size += CODECHAL_SFC_VEBOX_RGB_ACE_HISTOGRAM_SIZE_RESERVED_G11;
        sizeLace = MOS_ROUNDUP_DIVIDE(height, 64) *
            MOS_ROUNDUP_DIVIDE(width, 64)  *
            CODECHAL_SFC_VEBOX_LACE_HISTOGRAM_256_BIN_PER_BLOCK;

        sizeNoLace = CODECHAL_SFC_VEBOX_ACE_HISTOGRAM_SIZE_PER_FRAME_PER_SLICE *
            CODECHAL_SFC_NUM_FRAME_PREVIOUS_CURRENT                   *
            CODECHAL_SFC_VEBOX_MAX_SLICES_G11;

        size += MOS_MAX(sizeLace, sizeNoLace);

        MOS_ZeroMemory(&allocParamsForBufferLinear, sizeof(MOS_ALLOC_GFXRES_PARAMS));
        allocParamsForBufferLinear.Type = MOS_GFXRES_BUFFER;
        allocParamsForBufferLinear.TileType = MOS_TILE_LINEAR;
        allocParamsForBufferLinear.Format = Format_Buffer;
        allocParamsForBufferLinear.dwBytes = size;
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
        width = MOS_ALIGN_CEIL(width, 64);
        height = MOS_ROUNDUP_DIVIDE(height, 4) + MOS_ROUNDUP_DIVIDE(CODECHAL_SFC_VEBOX_STATISTICS_SIZE_G11 * sizeof(uint32_t), width);
        size = width * height;

        MOS_ZeroMemory(&allocParamsForBufferLinear, sizeof(MOS_ALLOC_GFXRES_PARAMS));
        allocParamsForBufferLinear.Type = MOS_GFXRES_BUFFER;
        allocParamsForBufferLinear.TileType = MOS_TILE_LINEAR;
        allocParamsForBufferLinear.Format = Format_Buffer;
        allocParamsForBufferLinear.dwBytes = size;
        allocParamsForBufferLinear.pBufName = "ResStatisticsOutput";

        m_osInterface->pfnAllocateResource(
            m_osInterface,
            &allocParamsForBufferLinear,
            &m_resStatisticsOutput);
    }

    params->pOsResStatisticsOutput = &m_resStatisticsOutput;

    return eStatus;
}