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
//! \file     vphal_render_ief.h
//! \brief    VPHAL IEF feature interface and structure
//! \details  VPHAL IEF feature interface and structure
//!
#ifndef __VPHAL_RENDER_IEF_H__
#define __VPHAL_RENDER_IEF_H__

#include "vphal_common.h"
#include "mhw_sfc.h"

// Detail filter
#define DETAIL_STRONG_EDGE_WEIGHT                   7
#define DETAIL_NON_EDGE_WEIGHT                      1
#define DETAIL_REGULAR_EDGE_WEIGHT                  2
#define DETAIL_WEAK_EDGE_THRESHOLD                  1
#define DETAIL_STRONG_EDGE_THRESHOLD                8

//!
//! \brief  IEF Parameters
//!
#define VPHAL_IEF_MIN                       0
#define VPHAL_IEF_MAX                       64
#define VPHAL_IEF_DEFAULT                   0
#define VPHAL_IEF_STEP                      1
#define VPHAL_IEF_OPTIMAL                   44 // for perfect SD-HQV Score
#define VPHAL_IEF_UPPER                     (VPHAL_IEF_MAX - 1 - VPHAL_IEF_OPTIMAL)
#define VPHAL_IEF_VERYSTRONG                63

extern const uint32_t R5x[VPHAL_IEF_MAX];
extern const uint32_t R5cx[VPHAL_IEF_MAX];
extern const uint32_t R5c[VPHAL_IEF_MAX];
extern const uint32_t R3x[VPHAL_IEF_MAX];
extern const uint32_t R3c[VPHAL_IEF_MAX];

//!
//! Class Ief
//! \brief IEF extension components
//!
class Ief
{
public:
    //! \brief    IEF Constructor
    //! \param    [in] pSource
    //!           Pointer to VPHAL Surface
    //!
    Ief(
        PVPHAL_SURFACE                  pSource);

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
    PVPHAL_SURFACE                  m_pSource;

    // IEF parameters
    uint16_t                        m_wIEFFactor;

    // common HW state IEF fields
    uint32_t                        m_dwR5xCoefficient;
    uint32_t                        m_dwR5cxCoefficient;
    uint32_t                        m_dwR5cCoefficient;
    uint32_t                        m_dwR3xCoefficient;
    uint32_t                        m_dwR3cCoefficient;
};

#endif // __VPHAL_RENDER_IEF_H__
