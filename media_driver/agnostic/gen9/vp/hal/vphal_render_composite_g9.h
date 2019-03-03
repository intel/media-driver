/*
* Copyright (c) 2017-2018, Intel Corporation
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
//! \file     vphal_render_composite_g9.h
//! \brief    Common interface and structure used in Composite
//! \details  Common interface and structure used in Composite
//!
#ifndef __VPHAL_RENDER_COMPOSITE_G9_H__
#define __VPHAL_RENDER_COMPOSITE_G9_H__

#include "vphal_render_composite.h"

//!
//! \brief Class to VPHAL Composite G9 render
//!
class CompositeStateG9 : virtual public CompositeState
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
    //! \param    [in] compositeCacheCntl
    //!           Composite Cache Control Data
    //! \param    [in,out] peStatus
    //!           Pointer to MOS status
    //!
    CompositeStateG9(
        PMOS_INTERFACE                      pOsInterface,
        PRENDERHAL_INTERFACE                pRenderHal,
        PVPHAL_RNDR_PERF_DATA               pPerfData,
        const VPHAL_COMPOSITE_CACHE_CNTL    &compositeCacheCntl,
        MOS_STATUS                          *peStatus) :
        CompositeState(pOsInterface, pRenderHal, pPerfData, compositeCacheCntl, peStatus)
    {
        m_bSamplerSupportRotation       = true;     // On Gen9+, Rotation is done in sampler.
        m_bFallbackIefPatch             = false;
        m_bKernelSupportDualOutput      = true;
        m_bKernelSupportHdcDW           = true;
        m_bAvsTableCoeffExtraEnabled    = true;
        m_bAvsTableBalancedFilter       = true;
        m_bApplyTwoLayersCompOptimize   = false;
        m_bYV12iAvsScaling              = true;     // On Gen9+, iAVS scaling can support YV12 input format
        m_bEnableSamplerLumakey         = true;     // Enable sampler lumakey on Gen9.

        if (*peStatus != MOS_STATUS_SUCCESS)
        {
            // super class constructor failed, return directly
            return;
        }
        m_AvsCoeffsCache.Init(POLYPHASE_Y_COEFFICIENT_TABLE_SIZE_G9, POLYPHASE_UV_COEFFICIENT_TABLE_SIZE_G9);
        *peStatus = VpHal_RndrCommonInitAVSParams(&m_AvsParameters,
                                                  POLYPHASE_Y_COEFFICIENT_TABLE_SIZE_G9,
                                                  POLYPHASE_UV_COEFFICIENT_TABLE_SIZE_G9);
    }

    //!
    //! \brief    Composite render Destructor
    //! \details  Destroy Composite render and release all related RenderState resources
    //!
    virtual ~CompositeStateG9()
    {
    }

protected:
    //!
    //! \brief    Override Static Data
    //! \details  Override Static Data
    //! \param    [in] pRenderingData
    //!           Pointer to REnder Data
    //! \param    [in] pTarget
    //!           Pointer to Target Surface
    //! \param    [in,out] pStatic
    //!           Pointer to Static Data
    //! \return   VOID
    //!
    virtual void SubmitStatesFillGenSpecificStaticData(
        PVPHAL_RENDERING_DATA_COMPOSITE     pRenderingData,
        PVPHAL_SURFACE                      pTarget,
        MEDIA_OBJECT_KA2_STATIC_DATA        *pStatic);

    //!
    //! \brief    Check NV12 luma key sampler solution is needed or not
    //! \details  This WA is needed before Gen9 platforms
    //! \param    pSrc
    //!           [in] Pointer to Source Surface
    //! \param    pRenderHal
    //!           [in] Pointer to render hal
    //! \return   bool
    //!           Return TRUE if needed, otherwise FALSE
    //!
    virtual bool IsNV12SamplerLumakeyNeeded(PVPHAL_SURFACE pSrc, PRENDERHAL_INTERFACE pRenderHal)
    {
        MOS_UNUSED(pRenderHal);
        return (pSrc->bUseSamplerLumakey && pSrc->Format == Format_NV12) ? true : false;
    }
};

#endif // __VPHAL_RENDER_COMPOSITE_G9_H__
