/*
* Copyright (c) 2017-2019, Intel Corporation
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
//! \file     vphal_renderer_g12_tgllp.h
//! \brief    The header file of VPHAL top level rendering component
//! \details  The top renderer is responsible for coordinating the sequence of calls to low level renderers, e.g. DNDI or Comp
//!
#ifndef __VPHAL_RENDERER_G12TGLLP_H__
#define __VPHAL_RENDERER_G12TGLLP_H__

#include "vphal_renderer_g12.h"

//!
//! \brief VPHAL renderer Gen12 class
//!
class VphalRendererG12Tgllp : public VphalRendererG12
{
public:
    //!
    //! \brief    VphalRendererG12Tgllp constructor
    //! \details  Based on the HW and OS info, initialize the renderer interfaces
    //! \param    [in] pRenderHal
    //!           Pointer to RenderHal Interface Structure
    //! \param    [in,out] pStatus
    //!           Pointer to the MOS_STATUS flag.
    //!           Will assign this flag to MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    VphalRendererG12Tgllp(
        PRENDERHAL_INTERFACE                pRenderHal,
        MOS_STATUS                          *pStatus) :
        VphalRendererG12(pRenderHal, pStatus)
    {
       bEnableCMFC = true;
    }

    //!
    //! \brief    VPHAL renderer destructor
    //! \details  Destory the resources allocated for the renderers
    //!           including VEBOX and Composite.
    //! \param    [in,out] VphalRenderer* pRenderer
    //!           VPHAL renderer pointer
    //! \return   VOID
    //!
    ~VphalRendererG12Tgllp()
    {
    }

    //!
    //! \brief    Initialize the KDLL parameters
    //! \details  Initialize the KDLL parameters
    //! \return   MOS_STATUS
    //!
    MOS_STATUS InitKdllParam();
};

#endif // __VPHAL_RENDER_G12TGLLP_H__
