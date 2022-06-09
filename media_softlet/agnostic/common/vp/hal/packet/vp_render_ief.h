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
//! \file     vp_render_ief.h
//! \brief    VP IEF feature interface and structure
//! \details  VP IEF feature interface and structure
//!
#ifndef __VP_RENDER_IEF_H__
#define __VP_RENDER_IEF_H__

#include <stdint.h>
#include "mos_defs.h"
#include "media_class_trace.h"
#include "mhw_sfc.h"
#include "mhw_state_heap.h"
#include "mos_resource_defs.h"
#include "vp_common.h"
namespace mhw
{
namespace sfc
{
struct SFC_IEF_STATE_PAR;
}
}  // namespace mhw
namespace mhw
{
namespace sfc
{
struct SFC_STATE_PAR;
}
}  // namespace mhw

//!
//! Class VpIef
//! \brief IEF extension components
//!
namespace vp{

class VpIef
{
public:
    //! \brief    IEF Constructor
    //!
    VpIef();

    //! \brief    IEF Destructor
    //! \details  IEF Destructor
    //!
    virtual ~VpIef();

    //!
    //! \brief    Set HW State(SFC) according to IEF parameter
    //! \param    [in,out] pSfcStateParams
    //!           Pointer to MHW SFC state
    //! \param    [in,out] pSfcIefStateParams
    //!           Pointer to MHW SFC IEF state
    //! \return   MOS_STATUS
    //!
     virtual MOS_STATUS SetHwState(
        PMHW_SFC_STATE_PARAMS           pSfcStateParams,
        PMHW_SFC_IEF_STATE_PARAMS       pSfcIefStateParams);

     virtual MOS_STATUS SetHwState(
         mhw::sfc::SFC_STATE_PAR           *pSfcStateParams,
         mhw::sfc::SFC_IEF_STATE_PAR       *pSfcIefStateParams);

    //!
    //! \brief    Set HW State(Sampler) according to IEF parameter
    //! \param    [in,out] pSamplerStateParams
    //!           Pointer to MHW Sampler state
    //! \return   MOS_STATUS
    //!
     virtual MOS_STATUS SetHwState(
        PMHW_SAMPLER_STATE_PARAM        pSamplerStateParams);

    //!
    //! \brief    IEF Initialization
    //! \details  Initialize member varible in IEF object.
    //! \param    [in] iefParams
    //!           Pointer to VPHAL Surface IEF Params
    //! \param    [in] format
    //!           IEF Format
    //! \param    fScaleX
    //!           [in] width scaling ratio
    //! \param    fScaleY
    //!           [in] height scaling ratio
    //!
    virtual void Init(
        PVPHAL_IEF_PARAMS   iefParams,
        MOS_FORMAT          format,
        float               scaleX,
        float               scaleY);

protected:
    //!
    //! \brief    Calculate IEF parameters
    //! \details  Calculate IEF parameters
    //! \return   MOS_STATUS
    //!
     virtual MOS_STATUS CalculateIefParams();

    // VPHAL surface contain required IEF parameter and format info
    PVPHAL_IEF_PARAMS               m_iefParams                 = nullptr;
    MOS_FORMAT                      m_format                    = Format_Invalid;
    // IEF parameters
    uint16_t                        m_iefFactor                 = 0;
    float                           m_scaleX                    = 0.0;
    float                           m_scaleY                    = 0.0;
    // common HW state IEF fields
    uint32_t                        m_r5xCoefficient            = 0;
    uint32_t                        m_r5cxCoefficient           = 0;
    uint32_t                        m_r5cCoefficient            = 0;
    uint32_t                        m_r3xCoefficient            = 0;
    uint32_t                        m_r3cCoefficient            = 0;
    //detail
    static const uint32_t           s_detailStrongEdgeWeight    = 7;
    static const uint32_t           s_detailRegularEdgeWeight   = 2;
    static const uint32_t           s_iefMaxItem                = 64;
    //IEF R5X coefficient array
    static const uint32_t           s_r5x[s_iefMaxItem];
    //IEF R5CX coefficient array
    static const uint32_t           s_r5cx[s_iefMaxItem];
    //IEF R5C coefficient array
    static const uint32_t           s_r5c[s_iefMaxItem];
    //IEF R3X coefficient array
    static const uint32_t           s_r3x[s_iefMaxItem];
    //brief  Const IEF R3C coefficient array
    static const uint32_t           s_r3c[s_iefMaxItem];

MEDIA_CLASS_DEFINE_END(vp__VpIef)
};

}
#endif // __VP_RENDER_IEF_H__
