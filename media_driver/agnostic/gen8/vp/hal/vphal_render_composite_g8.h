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
//! \file     vphal_render_composite_g8.h
//! \brief    Common interface and structure used in Composite
//! \details  Common interface and structure used in Composite
//!
#ifndef __VPHAL_RENDER_COMPOSITE_G8_H__
#define __VPHAL_RENDER_COMPOSITE_G8_H__

#include "vphal_render_composite.h"

//!
//! \brief Class to VPHAL Composite G8 render
//!
class CompositeStateG8 : virtual public CompositeState
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
    CompositeStateG8(
        PMOS_INTERFACE                      pOsInterface,
        PRENDERHAL_INTERFACE                pRenderHal,
        PVPHAL_RNDR_PERF_DATA               pPerfData,
        const VPHAL_COMPOSITE_CACHE_CNTL    &compositeCacheCntl,
        MOS_STATUS                          *peStatus) :
        CompositeState(pOsInterface, pRenderHal, pPerfData, compositeCacheCntl, peStatus)
    {
        m_bSamplerSupportRotation       = false;
        m_bFallbackIefPatch             = false;
        m_bKernelSupportDualOutput      = false;
        m_bKernelSupportHdcDW           = true;
        m_bAvsTableCoeffExtraEnabled    = false;
        m_bAvsTableBalancedFilter       = false;
        m_bApplyTwoLayersCompOptimize   = true;
        m_bYV12iAvsScaling              = false;
        m_bEnableSamplerLumakey         = false;    // Disable sampler lumakey on Gen8.

        if (*peStatus != MOS_STATUS_SUCCESS)
        {
            // super class constructor failed, return directly
            return;
        }
        m_AvsCoeffsCache.Init(POLYPHASE_Y_COEFFICIENT_TABLE_SIZE_G8, POLYPHASE_UV_COEFFICIENT_TABLE_SIZE_G8);
        *peStatus = VpHal_RndrCommonInitAVSParams(&m_AvsParameters,
                                                  POLYPHASE_Y_COEFFICIENT_TABLE_SIZE_G8,
                                                  POLYPHASE_UV_COEFFICIENT_TABLE_SIZE_G8);
    }

    //!
    //! \brief    Composite render Destructor
    //! \details  Destroy Composite render and release all related RenderState resources
    //!
    virtual ~CompositeStateG8()
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
    //! \brief    Get Plane Offset override parameter for Kernel solution
    //! \details  Get Y/UV Plane Offset override parameters for Kernel solution
    //! \param    [in,out] pRenderHalSurface
    //!           Pointer to Render Hal Surface
    //! \param    [in] pParams
    //!           Pointer to Surface State Params
    //! \param    [out] pOverride
    //!           Pointer to override param that provides adjustments to
    //!           Y, UV plane offsets, used for kernel solution in a few cases.
    //! \return   RENDERHAL_OFFSET_OVERRIDE
    //!           return pointer to RENDERHAL_OFFSET_OVERRIDE if need, otherwise return nullptr.
    //!
    virtual PRENDERHAL_OFFSET_OVERRIDE GetPlaneOffsetOverrideParam(
        PRENDERHAL_SURFACE              pRenderHalSurface,
        PRENDERHAL_SURFACE_STATE_PARAMS pParams,
        PRENDERHAL_OFFSET_OVERRIDE      pOverride);

    //!
    //! \brief    Get Thread Count
    //! \details  Get Thread Count for VFE state parameter
    //! \param    [in] pRenderingData
    //!           Pointer to Composite state
    //! \param    [in] pTarget
    //!           Pointer to target surface
    //! \return   int32_t
    //!           return the thread count
    //!
    virtual int32_t GetThreadCountForVfeState(
        PVPHAL_RENDERING_DATA_COMPOSITE     pRenderingData,
        PVPHAL_SURFACE                      pTarget);

private:
    static const uint32_t waBdwGt2ThreadLimit = 96;
};

#endif // __VPHAL_RENDER_COMPOSITE_G8_H__
