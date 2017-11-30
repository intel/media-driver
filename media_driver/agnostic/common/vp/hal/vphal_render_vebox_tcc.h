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
//! \file     vphal_render_vebox_tcc.h
//! \brief    VPHAL VEBOX IECP Total Color Control (TCC) interfaces
//! \details  VPHAL VEBOX IECP Total Color Control (TCC) interfaces
//!
#ifndef __VPHAL_RENDER_VEBOX_TCC_H__
#define __VPHAL_RENDER_VEBOX_TCC_H__

#include "vphal_render_common.h"
#include "vphal_render_vebox_base.h"
#include "vphal_render_vebox_iecp.h"

#define VPHAL_RENDER_VEBOX_TCC_ENABLE   true
#if VPHAL_RENDER_VEBOX_TCC_ENABLE

class VPHAL_VEBOX_IECP_TCC : public VPHAL_VEBOX_IECP_FILTER
{
public:
    ~VPHAL_VEBOX_IECP_TCC() {}

    //!
    //! \brief    Vebox set IECP parameter
    //! \details  Set Vebox IECP state parameter
    //! \param    [in] pSrcSurface
    //!           Pointer to input surface of Vebox
    //! \param    [in,out] pRenderData
    //!           Pointer to Vebox Render Data
    //! \return   void
    //!
    void SetParams(
        PVPHAL_SURFACE              pSrcSurface,
        PVPHAL_VEBOX_RENDER_DATA    pRenderData);

    //!
    //! \brief    Init Vebox IECP parameter
    //! \param    [in] pVphalVeboxIecpParams
    //!           Pointer to input Vphal Iecp parameters
    //! \param    [in,out] pMhwVeboxIecpParams
    //!           Pointer to Mhw Iecp parameters
    //! \return   void
    //!
    void InitParams(
        PVPHAL_VEBOX_IECP_PARAMS        pVphalVeboxIecpParams,
        PMHW_VEBOX_IECP_PARAMS          pMhwVeboxIecpParams);
};
typedef VPHAL_VEBOX_IECP_TCC * PVPHAL_VEBOX_IECP_TCC;

#endif  // VPHAL_RENDER_VEBOX_TCC_ENABLE

#endif  //__VPHAL_RENDER_VEBOX_TCC_H__