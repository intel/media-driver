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
//! \file     vphal_render_composite_g11_jsl_ehl.h
//! \brief    Common interface and structure used in Composite
//! \details  Common interface and structure used in Composite
//!
#ifndef __VPHAL_RENDER_COMPOSITE_G11_JSL_EHL_H__
#define __VPHAL_RENDER_COMPOSITE_G11_JSL_EHL_H__

#include "vphal_render_composite_g11.h"

//!
//! \brief Class to VPHAL Composite G11 JSL and EHL render
//!
class CompositeStateG11JslEhl : public CompositeStateG11
{
public:
    //!
    //! \brief    Composite Constructor
    //! \details  Construct Composite render and allocate member data structure
    //! \param    [in] pOsInterface
    //!           Pointer to MOS interface structure
    //! \param    [in] pRenderHal
    //!           Pointer to RenderHal interface structure
    //! \param    [in] pPerfData
    //!           Pointer to performance data structure
    //! \param    compositeCacheCntl
    //!           [in] Composite Cache Control Data
    //! \param    [in,out] peStatus
    //!           Pointer to MOS status
    //!
    CompositeStateG11JslEhl(
        PMOS_INTERFACE                      pOsInterface,
        PRENDERHAL_INTERFACE                pRenderHal,
        PVPHAL_RNDR_PERF_DATA               pPerfData,
        const VPHAL_COMPOSITE_CACHE_CNTL    &compositeCacheCntl,
        MOS_STATUS                          *peStatus) :
        CompositeState(pOsInterface, pRenderHal, pPerfData, compositeCacheCntl, peStatus),
        CompositeStateG11(pOsInterface, pRenderHal, pPerfData, compositeCacheCntl, peStatus)
    {

    }

    //!
    //! \brief    Composite render Destructor
    //! \details  Destroy Composite render and release all related RenderState resources
    //!
    virtual ~CompositeStateG11JslEhl()
    {
    }
};

#endif // __VPHAL_RENDER_COMPOSITE_G11_JSL_EHL_H__
