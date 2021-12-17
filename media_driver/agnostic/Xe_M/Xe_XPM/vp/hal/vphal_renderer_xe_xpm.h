/*===================== begin_copyright_notice ==================================

# Copyright (c) 2020-2021, Intel Corporation

# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
# OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
# ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
# OTHER DEALINGS IN THE SOFTWARE.

======================= end_copyright_notice ==================================*/
//!
//! \file     vphal_renderer_xe_xpm.h
//! \brief    The header file of VPHAL top level rendering component
//! \details  The top renderer is responsible for coordinating the sequence of calls to low level renderers, e.g. DNDI or Comp
//!
#ifndef __VPHAL_RENDERER_XE_XPM_H__
#define __VPHAL_RENDERER_XE_XPM_H__

#include "vphal_renderer_g12.h"
#include "vphal_debug_xe_xpm.h"

void KernelDll_ModifyFunctionPointers_g12hp(Kdll_State *pState);

//!
//! \brief VPHAL renderer Gen12 class
//!
class VphalRendererXe_Xpm : public VphalRendererG12
{
public:
    //!
    //! \brief    VphalRendererXe_Xpm constructor
    //! \details  Based on the HW and OS info, initialize the renderer interfaces
    //! \param    [in] pRenderHal
    //!           Pointer to RenderHal Interface Structure
    //! \param    [in,out] pStatus
    //!           Pointer to the MOS_STATUS flag.
    //!           Will assign this flag to MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    VphalRendererXe_Xpm(
        PRENDERHAL_INTERFACE                pRenderHal,
        MOS_STATUS                          *pStatus) :
        VphalRendererG12(pRenderHal, pStatus)
    {
        bEnableCMFC = true;

        m_modifyKdllFunctionPointers = KernelDll_ModifyFunctionPointers_g12hp;
    }

    //!
    //! \brief    VPHAL renderer destructor
    //! \details  Destory the resources allocated for the renderers
    //!           including VEBOX and Composite.
    //! \param    [in,out] VphalRenderer* pRenderer
    //!           VPHAL renderer pointer
    //! \return   VOID
    //!
    ~VphalRendererXe_Xpm()
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
    //! \brief    Main render function
    //! \details  The top level renderer function, which may contain multiple
    //!           passes of rendering
    //! \param    [in] pcRenderParams
    //!           Const pointer to VPHAL render parameter
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS Render(
        PCVPHAL_RENDER_PARAMS    pcRenderParams);

    //!
    //! \brief    Initialize the KDLL parameters
    //! \details  Initialize the KDLL parameters
    //! \return   MOS_STATUS
    //!
    MOS_STATUS InitKdllParam();

    //!
    //! \brief    Allocate surface dumper
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    MOS_STATUS CreateSurfaceDumper();
};

#endif // __VPHAL_RENDER_XE_XPM_H__
