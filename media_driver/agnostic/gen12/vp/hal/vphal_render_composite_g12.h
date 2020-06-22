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
//! \file     vphal_render_composite_g12.h
//! \brief    Common interface and structure used in Composite
//! \details  Common interface and structure used in Composite
//!
#ifndef __VPHAL_RENDER_COMPOSITE_G12_H__
#define __VPHAL_RENDER_COMPOSITE_G12_H__

#include "vphal_render_composite.h"
#include "hal_kerneldll.h"
#include "renderhal_platform_interface.h"

//!
//! \brief Class to VPHAL Composite G12 render
//!
class CompositeStateG12 : virtual public CompositeState
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
    CompositeStateG12(
        PMOS_INTERFACE                      pOsInterface,
        PRENDERHAL_INTERFACE                pRenderHal,
        PVPHAL_RNDR_PERF_DATA               pPerfData,
        const VPHAL_COMPOSITE_CACHE_CNTL    &compositeCacheCntl,
        MOS_STATUS                          *peStatus) :
        CompositeState(pOsInterface, pRenderHal, pPerfData, compositeCacheCntl, peStatus)
    {
        m_bSamplerSupportRotation       = true;     // On Gen9+, Rotation is done in sampler.
        m_bFallbackIefPatch             = true;
        m_bKernelSupportDualOutput      = true;
        m_bKernelSupportHdcDW           = false;
        m_bAvsTableCoeffExtraEnabled    = true;
        m_bAvsTableBalancedFilter       = true;
        m_bApplyTwoLayersCompOptimize   = false;
        m_bYV12iAvsScaling              = true;     // On Gen9+, iAVS scaling can support YV12 input format
        m_bEnableSamplerLumakey         = false;    // Disable sampler lumakey on Gen12.

        if (*peStatus != MOS_STATUS_SUCCESS)
        {
            // super class constructor failed, return directly
            return;
        }
        m_AvsCoeffsCache.Init(POLYPHASE_Y_COEFFICIENT_TABLE_SIZE_G10, POLYPHASE_UV_COEFFICIENT_TABLE_SIZE_G10);
        *peStatus = VpHal_RndrCommonInitAVSParams(&m_AvsParameters,
                                                  POLYPHASE_Y_COEFFICIENT_TABLE_SIZE_G10,
                                                  POLYPHASE_UV_COEFFICIENT_TABLE_SIZE_G10);
    }

    //!
    //! \brief    Composite render Destructor
    //! \details  Destroy Composite render and release all related RenderState resources
    //!
    virtual ~CompositeStateG12()
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
    //! \brief    Initialize Composite render
    //! \param    [in] pSettings
    //!           Pointer to VPHAL Settings
    //! \param    [in] pKernelDllState
    //!           Pointer to KernelDLL State
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful
    //!
    virtual MOS_STATUS Initialize(
        const VphalSettings    *pSettings,
        Kdll_State             *pKernelDllState)
    {
        // Set CMFC extension Kdll function pointers
        KernelDll_SetupFunctionPointers_Ext(pKernelDllState);

        return CompositeState::Initialize(pSettings, pKernelDllState);
    }
};

#endif // __VPHAL_RENDER_COMPOSITE_G12_H__
