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
//! \file     vphal_renderer_g12.h
//! \brief    The header file of VPHAL top level rendering component
//! \details  The top renderer is responsible for coordinating the sequence of calls to low level renderers, e.g. DNDI or Comp
//!
#ifndef __VPHAL_RENDERER_G12_H__
#define __VPHAL_RENDERER_G12_H__

#include "vphal_renderer.h"

const VphalSseuSetting VpHalDefaultSSEUTableG12[baseKernelMaxNumID] =
{
    // Slice    Sub-Slice       EU      Rsvd(freq)
    { 2,            3,          8,          0 },     // COMBINED_FC_KERNEL Default

    // 2 VEBOX KERNELS
    { 2,            3,          8,          0 },     // VEBOX_SECUREBLOCKCOPY_KERNEL,
    { 2,            3,          8,          0 },     // VEBOX_UPDATEDNSTATE_KERNEL,
};

//!
//! \brief VPHAL renderer Gen12 class
//!
class VphalRendererG12 : public VphalRenderer
{
public:
    bool                        bEnableCMFC;                                    //!< Enable CM based FC for GEN12
    //!
    //! \brief    VphalRendererG8 constructor
    //! \details  Based on the HW and OS info, initialize the renderer interfaces
    //! \param    [in,out] pRenderHal
    //!           Pointer to RenderHal Interface Structure
    //! \param    [in,out] pStatus
    //!           Pointer to the MOS_STATUS flag.
    //!           Will assign this flag to MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    VphalRendererG12(
        PRENDERHAL_INTERFACE                pRenderHal,
        MOS_STATUS                          *pStatus) :
        VphalRenderer(pRenderHal, pStatus)
    {
        // Set SSEUTable
        pRenderHal->sseuTable = VpHalDefaultSSEUTableG12;
    }

    //!
    //! \brief    VPHAL renderer destructor
    //! \details  Destory the resources allocated for the renderers
    //!           including VEBOX and Composite.
    //! \param    [in,out] VphalRenderer* pRenderer
    //!           VPHAL renderer pointer
    //! \return   VOID
    //!
    virtual ~VphalRendererG12()
    {
    }

    //!
    //! \brief    Get Renderer Cache Settings
    //! \details  Get cache settings for various VP features
    //! \param    [in] pOsInterface
    //!           OS Interface
    //! \param    [in] pPlatform
    //!           Platform Pointer
    //! \param    [in] pSkuTable
    //!           SKU feature table
    //! \param    [in,out] pSettings
    //!           Pointer to Render Cache Control Settings
    //! \return   void
    //!
    virtual void GetCacheCntl(
        PMOS_INTERFACE                      pOsInterface,
        PLATFORM                            *pPlatform,
        MEDIA_FEATURE_TABLE                 *pSkuTable,
        PVPHAL_RENDER_CACHE_CNTL            pSettings);

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
    virtual MOS_STATUS AllocateRenderComponents(
        PMHW_VEBOX_INTERFACE                pVeboxInterface,
        PMHW_SFC_INTERFACE                  pSfcInterface);

    //!
    //! \brief    Initialize the KDLL parameters
    //! \details  Initialize the KDLL parameters
    //! \return   MOS_STATUS
    //!
    virtual MOS_STATUS InitKdllParam() = 0;

};

#endif // __VPHAL_RENDER_G12_H__
