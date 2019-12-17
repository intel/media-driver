/*
* Copyright (c) 2016-2017, Intel Corporation
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
//! \file     vp_render_ief.cpp
//! \brief    VP IEF feature implementation
//! \details  VP IEF feature implementation
//!

#include "vp_render_ief.h"
#include "vp_utils.h"
#include "mos_resource_defs.h"

namespace vp{

const uint32_t   Ief::g_R5x[k_IefMaxItem] = {
    0,   0,   0,   0,   1,   1,   1,   1,   1,   1,   2,   2,   2,   2,   2,   2,
    3,   3,   3,   3,   3,   3,   4,   4,   4,   4,   4,   4,   4,   5,   5,   5,
    5,   5,   5,   6,   6,   6,   6,   6,   6,   7,   7,   7,   7,   8,  10,  11,
    12,  13,  15,  16,  17,  18,  20,  21,  22,  23,  25,  26,  27,  28,  30,  31
};

//!
//! \brief  Const IEF R5CX coefficient array
//!
const uint32_t   Ief::g_R5cx[k_IefMaxItem] = {
    0,   0,   0,   0,   1,   1,   1,   1,   1,   1,   2,   2,   2,   2,   2,   2,
    3,   3,   3,   3,   3,   3,   4,   4,   4,   4,   4,   4,   4,   5,   5,   5,
    5,   5,   5,   6,   6,   6,   6,   6,   6,   7,   7,   7,   7,   8,  10,  11,
    12,  13,  15,  16,  17,  18,  20,  21,  22,  23,  25,  26,  27,  28,  30,  31
};

//!
//! \brief  Const IEF R5C coefficient array
//!
const uint32_t   Ief::g_R5c[k_IefMaxItem] = {
    0,   0,   0,   0,   1,   1,   1,   1,   1,   1,   2,   2,   2,   2,   2,   2,
    3,   3,   3,   3,   3,   3,   4,   4,   4,   4,   4,   4,   4,   5,   5,   5,
    5,   5,   5,   6,   6,   6,   6,   6,   6,   7,   7,   7,   7,   8,  10,  11,
    12,  13,  15,  16,  17,  18,  20,  21,  22,  23,  25,  26,  27,  28,  30,  31
};

//!
//! \brief  Const IEF R3X coefficient array
//!
const uint32_t   Ief::g_R3x[k_IefMaxItem] = {
    0,   0,   0,   0,   0,   1,   1,   1,   1,   1,   1,   1,   1,   1,   2,   2,
    2,   2,   2,   2,   2,   2,   3,   3,   3,   3,   3,   3,   3,   3,   3,   4,
    4,   4,   4,   4,   4,   4,   4,   4,   5,   5,   5,   5,   5,   6,   6,   7,
    7,   8,   8,   9,  10,  10,  11,  11,  12,  13,  13,  14,  14,  15,  15,  16
};

//!
//! \brief  Const IEF R3C coefficient array
//!
const uint32_t   Ief::g_R3c[k_IefMaxItem] = {
    0,   0,   0,   0,   0,   1,   1,   1,   1,   1,   1,   1,   1,   1,   2,   2,
    2,   2,   2,   2,   2,   2,   3,   3,   3,   3,   3,   3,   3,   3,   3,   4,
    4,   4,   4,   4,   4,   4,   4,   4,   5,   5,   5,   5,   5,   6,   6,   7,
    7,   8,   8,   9,  10,  10,  11,  11,  12,  13,  13,  14,  14,  15,  15,  16
};

MOS_STATUS Ief::CalculateIefParams()
{
    PVPHAL_IEF_PARAMS   pIEFParams;
    MOS_STATUS          eStatus = MOS_STATUS_SUCCESS;

    VP_RENDER_CHK_NULL_RETURN(m_iefParams);

    pIEFParams = m_iefParams;
    if (pIEFParams == nullptr)
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }

    m_wIEFFactor = (uint16_t)pIEFParams->fIEFFactor;

    // HW supports 0 - 63, but driver reports 0 - 64. so we clamp to 63 here.
    if (m_wIEFFactor >= k_IefMaxItem)
    {
        m_wIEFFactor = k_IefMaxItem - 1;
    }

    m_dwR5xCoefficient  = g_R5x[m_wIEFFactor];
    m_dwR5cxCoefficient = g_R5cx[m_wIEFFactor];
    m_dwR5cCoefficient  = g_R5c[m_wIEFFactor];
    m_dwR3xCoefficient  = g_R3x[m_wIEFFactor];
    m_dwR3cCoefficient  = g_R3c[m_wIEFFactor];

    return eStatus;
}

MOS_STATUS Ief::SetHwState(
    PMHW_SAMPLER_STATE_PARAM        pSamplerStateParams)
{
    // Init default parameters
    // Set IEF params
    PVPHAL_IEF_PARAMS   pIEFParams  = nullptr;
    MOS_STATUS          eStatus     = MOS_STATUS_SUCCESS;

    VP_RENDER_CHK_NULL_RETURN(m_iefParams);

    pIEFParams = m_iefParams;
    VP_RENDER_CHK_NULL_RETURN(pIEFParams);

    // calculate IEF parameter
    eStatus = CalculateIefParams();
    if (eStatus != MOS_STATUS_SUCCESS)
    {
        VP_RENDER_ASSERTMESSAGE("CalculateIefParams failed.");
    }

    if (IS_PL3_FORMAT(m_format))
    {
        // Disable Skin Tone Tuned IEF Params for PL3 formats - not supported in HW
        pSamplerStateParams->Avs.bEnableSTDE        = false;
    }
    else
    {
        // Setup Skin Tone Tuned IEF Params (enable when IEF is enabled) [default]
        pSamplerStateParams->Avs.bEnableSTDE        = pIEFParams->bSkintoneTuned;
        pSamplerStateParams->Avs.bSkinDetailFactor  = pIEFParams->bEmphasizeSkinDetail;
    }

    pSamplerStateParams->Avs.StrongEdgeWght = (uint8_t)pIEFParams->StrongEdgeWeight;
    pSamplerStateParams->Avs.RegularWght    = (uint8_t)pIEFParams->RegularWeight;
    pSamplerStateParams->Avs.StrongEdgeThr  = (uint8_t)pIEFParams->StrongEdgeThreshold;

    pSamplerStateParams->Avs.bEnableIEF = true;
    pSamplerStateParams->Avs.wIEFFactor = m_wIEFFactor;
    pSamplerStateParams->Avs.GainFactor = m_wIEFFactor;

    pSamplerStateParams->Avs.wR5xCoefficient        = (uint16_t)m_dwR5xCoefficient;
    pSamplerStateParams->Avs.wR5cxCoefficient       = (uint16_t)m_dwR5cxCoefficient;
    pSamplerStateParams->Avs.wR5cCoefficient        = (uint16_t)m_dwR5cCoefficient;
    pSamplerStateParams->Avs.wR3xCoefficient        = (uint16_t)m_dwR3xCoefficient;
    pSamplerStateParams->Avs.wR3cCoefficient        = (uint16_t)m_dwR3cCoefficient;

    return eStatus;
}

MOS_STATUS Ief::SetHwState(
    PMHW_SFC_STATE_PARAMS           pSfcStateParams,
    PMHW_SFC_IEF_STATE_PARAMS       pSfcIefStateParams)
{
    PVPHAL_IEF_PARAMS   pIEFParams  = nullptr;
    MOS_STATUS          eStatus     = MOS_STATUS_SUCCESS;

    VP_RENDER_CHK_NULL_RETURN(m_iefParams);

    pIEFParams = m_iefParams;
    VP_RENDER_CHK_NULL_RETURN(pIEFParams);

    eStatus = CalculateIefParams();
    if (eStatus != MOS_STATUS_SUCCESS)
    {
        VP_RENDER_ASSERTMESSAGE("CalculateIefParams failed.");
    }

    // Init default parameters
    // Set IEF params
    pSfcStateParams->bIEFEnable               = true;
    pSfcIefStateParams->bIEFEnable            = true;
    pSfcIefStateParams->StrongEdgeWeight      = k_DetailStrongEdgeWeight;
    pSfcIefStateParams->RegularWeight         = k_DetailRegularEdgeWeight;
    pSfcIefStateParams->StrongEdgeThreshold   = IEF_STRONG_EDGE_THRESHOLD;

    // Set STE params
    pSfcStateParams->bSkinToneTunedIEFEnable  = true;
    pSfcIefStateParams->bSkinDetailFactor     = false;
    pSfcIefStateParams->bVYSTDEnable          = true;

    // Settings from user
    pSfcIefStateParams->StrongEdgeWeight      = (uint8_t)pIEFParams->StrongEdgeWeight;
    pSfcIefStateParams->RegularWeight         = (uint8_t)pIEFParams->RegularWeight;
    pSfcIefStateParams->StrongEdgeThreshold   = (uint8_t)pIEFParams->StrongEdgeThreshold;
    pSfcStateParams->bSkinToneTunedIEFEnable  = pIEFParams->bSkintoneTuned;
    pSfcIefStateParams->bSkinDetailFactor     = pIEFParams->bEmphasizeSkinDetail;

    // Set IEF params
    if (m_wIEFFactor > 0)
    {
        pSfcIefStateParams->dwGainFactor        = m_wIEFFactor;
        pSfcIefStateParams->dwR5xCoefficient    = m_dwR5xCoefficient;
        pSfcIefStateParams->dwR5cxCoefficient   = m_dwR5cxCoefficient;
        pSfcIefStateParams->dwR5cCoefficient    = m_dwR5cCoefficient;
        pSfcIefStateParams->dwR3xCoefficient    = m_dwR3xCoefficient;
        pSfcIefStateParams->dwR3cCoefficient    = m_dwR3cCoefficient;
    }

    return eStatus;
}

Ief::Ief(
    PVPHAL_SURFACE                  pSource) :
    m_wIEFFactor(0),
    m_dwR5xCoefficient(0),
    m_dwR5cxCoefficient(0),
    m_dwR5cCoefficient(0),
    m_dwR3xCoefficient(0),
    m_dwR3cCoefficient(0)
{
    m_format    = pSource->Format;
    m_iefParams = pSource->pIEFParams;
}

Ief::Ief(PVPHAL_IEF_PARAMS IefParams, MOS_FORMAT format) :
    m_iefParams(IefParams),
    m_format(format),
    m_wIEFFactor(0),
    m_dwR5xCoefficient(0),
    m_dwR5cxCoefficient(0),
    m_dwR5cCoefficient(0),
    m_dwR3xCoefficient(0),
    m_dwR3cCoefficient(0)
{

}

Ief::~Ief()
{
}

}