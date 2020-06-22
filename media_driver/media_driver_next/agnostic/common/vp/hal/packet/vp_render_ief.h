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
//! \file     vp_render_ief.h
//! \brief    VP IEF feature interface and structure
//! \details  VP IEF feature interface and structure
//!
#ifndef __VP_RENDER_IEF_H__
#define __VP_RENDER_IEF_H__

#include "vp_sfc_common.h"

//!
//! Class Ief
//! \brief IEF extension components
//!
namespace vp{

class Ief
{
public:
    //! \brief    IEF Constructor
    //! \param    [in] pSource
    //!           Pointer to VPHAL Surface
    //!
    Ief(
        PVPHAL_SURFACE                  pSource);

    //! \brief    IEF Constructor
    //! \param    [in] IefParams
    //!           Pointer to VPHAL Surface IEF Params
    //! \param    [in] format
    //!           IEF Format
    //!
    Ief(
        PVPHAL_IEF_PARAMS               IefParams,
        MOS_FORMAT                      format);

    //! \brief    IEF Destructor
    //! \details  IEF Destructor
    //!
    virtual ~Ief();

    //!
    //! \brief    Set HW State(SFC) according to IEF parameter
    //! \param    [in,out] pSfcStateParams
    //!           Pointer to MHW SFC state
    //! \param    [in,out] pSfcIefStateParams
    //!           Pointer to MHW SFC IEF state
    //! \return   MOS_STATUS
    //!
    MOS_STATUS SetHwState(
        PMHW_SFC_STATE_PARAMS           pSfcStateParams,
        PMHW_SFC_IEF_STATE_PARAMS       pSfcIefStateParams);

    //!
    //! \brief    Set HW State(Sampler) according to IEF parameter
    //! \param    [in,out] pSamplerStateParams
    //!           Pointer to MHW Sampler state
    //! \return   MOS_STATUS
    //!
    MOS_STATUS SetHwState(
        PMHW_SAMPLER_STATE_PARAM        pSamplerStateParams);

protected:
    //!
    //! \brief    Calculate IEF parameters
    //! \details  Calculate IEF parameters
    //! \return   MOS_STATUS
    //!
    MOS_STATUS CalculateIefParams();

    // VPHAL surface contain required IEF parameter and format info
    PVPHAL_IEF_PARAMS               m_iefParams = nullptr;
    MOS_FORMAT                      m_format    = Format_Any;

    // IEF parameters
    uint16_t                        m_wIEFFactor = 0;

    // common HW state IEF fields
    uint32_t                        m_dwR5xCoefficient = 0;
    uint32_t                        m_dwR5cxCoefficient = 0;
    uint32_t                        m_dwR5cCoefficient = 0;
    uint32_t                        m_dwR3xCoefficient = 0;
    uint32_t                        m_dwR3cCoefficient = 0;

private:
    //detail
    static const uint32_t           k_DetailStrongEdgeWeight = 7;
    static const uint32_t           k_DetailRegularEdgeWeight = 2;

    static const uint32_t           k_IefMaxItem = 64;
    //IEF R5X coefficient array
    static const uint32_t           g_R5x[k_IefMaxItem];
    //IEF R5CX coefficient array
    static const uint32_t           g_R5cx[k_IefMaxItem];
    //IEF R5C coefficient array
    static const uint32_t           g_R5c[k_IefMaxItem];
    //IEF R3X coefficient array
    static const uint32_t           g_R3x[k_IefMaxItem];
    //brief  Const IEF R3C coefficient array
    static const uint32_t           g_R3c[k_IefMaxItem];
};

}
#endif // __VP_RENDER_IEF_H__
