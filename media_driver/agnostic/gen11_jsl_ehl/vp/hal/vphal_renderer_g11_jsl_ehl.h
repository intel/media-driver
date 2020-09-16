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
//! \file     vphal_renderer_g11_jsl_ehl.h
//! \brief    The header file of VPHAL top level rendering component
//! \details  The top renderer is responsible for coordinating the sequence of calls to low level renderers, e.g. DNDI or Comp
//!
#ifndef __VPHAL_RENDERER_G11_JSL_EHL_H__
#define __VPHAL_RENDERER_G11_JSL_EHL_H__

#include "vphal_renderer_g11.h"

//!
//! \brief VPHAL renderer Gen11 class
//!
class VphalRendererG11JslEhl : public VphalRendererG11
{
public:
    //!
    //! \brief    VphalRendererG11JslEhl constructor
    //! \details  Based on the HW and OS info, initialize the renderer interfaces
    //! \param    [in] pRenderHal
    //!           Pointer to RenderHal Interface Structure
    //! \param    [in,out] pStatus
    //!           Pointer to the MOS_STATUS flag.
    //!           Will assign this flag to MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    VphalRendererG11JslEhl(
        PRENDERHAL_INTERFACE                pRenderHal,
        MOS_STATUS                          *pStatus) :
        VphalRendererG11(pRenderHal, pStatus)
    {
    }

    //!
    //! \brief    VPHAL renderer destructor
    //! \details  Destory the resources allocated for the renderers
    //!           including VEBOX and Composite.
    //! \param    [in,out] VphalRenderer* pRenderer
    //!           VPHAL renderer pointer
    //! \return   void
    //!
    ~VphalRendererG11JslEhl()
    {
    }

    //!
    //! \brief    Allocate render components
    //! \details  Allocate render components
    //! \param    [in] pVeboxInterface
    //!           Pointer to Vebox Interface Structure
    //! \param    [in] pSfcInterface
    //!           Pointer to SFC interface Structure
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    MOS_STATUS AllocateRenderComponents(
        PMHW_VEBOX_INTERFACE                pVeboxInterface,
        PMHW_SFC_INTERFACE                  pSfcInterface);

    //!
    //! \brief    Initialize the KDLL parameters
    //! \details  Initialize the KDLL parameters
    //! \return   MOS_STATUS
    //!
    MOS_STATUS InitKdllParam();

protected:

    //!
    //! \brief    Render Pre-composite process
    //! \details  Render Pre-composite process
    //! \param    [in,out] Pointer to the VPHAL render params data
    //!           pRenderParam
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    MOS_STATUS RenderPreComposite(
        PVPHAL_RENDER_PARAMS    pRenderParams)
    {
        return MOS_STATUS_SUCCESS;
    }
};

#endif // __VPHAL_RENDER_EXT_G11ICL_H__
