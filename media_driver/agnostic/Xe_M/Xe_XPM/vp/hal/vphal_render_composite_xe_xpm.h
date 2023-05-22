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
//! \file     vphal_render_composite_xe_xpm.h
//! \brief    Common interface and structure used in Composite
//! \details  Common interface and structure used in Composite
//!
#ifndef __VPHAL_RENDER_COMPOSITE_XE_XPM_H__
#define __VPHAL_RENDER_COMPOSITE_XE_XPM_H__

#include "vphal_render_composite.h"
#include "hal_kerneldll.h"
#include "renderhal_platform_interface.h"
#include "vphal_render_composite_g12.h"
//!
//! \brief Class to VPHAL Composite G12HP render
//!
class CompositeStateXe_Xpm : virtual public CompositeStateG12
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
    CompositeStateXe_Xpm (
        PMOS_INTERFACE                      pOsInterface,
        PRENDERHAL_INTERFACE                pRenderHal,
        PVPHAL_RNDR_PERF_DATA               pPerfData,
        const VPHAL_COMPOSITE_CACHE_CNTL    &compositeCacheCntl,
        MOS_STATUS                          *peStatus) :
        CompositeState(pOsInterface, pRenderHal, pPerfData, compositeCacheCntl, peStatus),
        CompositeStateG12(pOsInterface, pRenderHal, pPerfData, compositeCacheCntl, peStatus)
    {
        if ((pRenderHal == nullptr) && (peStatus != nullptr))
        {
            *peStatus = MOS_STATUS_NULL_POINTER;
             return;
        }
        m_bFtrComputeWalker             = pRenderHal->pRenderHalPltInterface->IsComputeContextInUse(pRenderHal);

        if (m_bFtrComputeWalker)
        {
            m_need3DSampler = true;
        }
    }

    //!
    //! \brief    Composite render Destructor
    //! \details  Destroy Composite render and release all related RenderState resources
    //!
    virtual ~CompositeStateXe_Xpm()
    {
    }

protected:

    //!
    //! \brief    Get Sampler Index associated with a surface state for composite
    //! \param    [in] pSurface
    //!           point to input Surface
    //! \param    [in] pEntry
    //!           Pointer to Surface state
    //! \param    [out] pSamplerIndex
    //!           Pointer to Sampler Index
    //! \param    [out] pSamplerType
    //!           Pointer to Sampler Type
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise MOS_STATUS_UNKNOWN
    //!
    virtual MOS_STATUS GetSamplerIndex(
        PVPHAL_SURFACE                      pSurface,
        PRENDERHAL_SURFACE_STATE_ENTRY      pEntry,
        int32_t*                            pSamplerIndex,
        PMHW_SAMPLER_TYPE                   pSamplerType) override;

    //!
    //! \brief    Check whether the 3Dsampler use for Y plane
    //! \param    [in] SamplerID
    //!           sampler ID
    //! \return   bool
    //!           Return true if the 3Dsampler use for Y plane, otherwise fase
    //!
    virtual bool IsSamplerIDForY(
        int32_t                            SamplerID) override;

    //!
    //! \brief    set Sampler status
    //! \param    [in] pSurface
    //!           input Surface
    //! \param    [in] Layer
    //!           composition layer
    //! \param    [in] pStatic
    //!           Pointer to static data
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise MOS_STATUS_UNKNOWN
    //!
    virtual MOS_STATUS Set3DSamplerStatus(
        PVPHAL_SURFACE                 pSurface,
        uint8_t                        Layer,
        MEDIA_OBJECT_KA2_STATIC_DATA   *pStatic) override;

    //!
    //! \brief    update Inline Data status
    //! \param    [in] pSurface
    //!           input Surface
    //! \param    [in] pStatic
    //!           Pointer to static data
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise MOS_STATUS_UNKNOWN
    //!
    virtual MOS_STATUS UpdateInlineDataStatus(
       PVPHAL_SURFACE                 pSurface,
       MEDIA_OBJECT_KA2_STATIC_DATA   *pStatic) override;

    //!
    //! \brief    Get intermediate surface output
    //! \param    pOutput
    //!           [in] Pointer to Intermediate Output Surface
    //! \return   PVPHAL_SURFACE
    //!           Return the chose output
    //!
    virtual MOS_STATUS GetIntermediateOutput(PVPHAL_SURFACE &output) override;

    //!
    //! \brief    Decompress the Surface
    //! \details  Decompress the interlaced Surface which is in the RC compression mode
    //! \param    [in,out] pSource
    //!           Pointer to Source Surface
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS DecompressInterlacedSurf(PVPHAL_SURFACE pSource) override;

    //!
    //! \brief    Update SamplerStateParams associated with a surface state for composite
    //! \param    [in] pSamplerStateParams
    //!           Pointer to SamplerStateParams
    //! \param    [in] pEntry
    //!           Pointer to Surface state
    //! \param    [in] pRenderData
    //!           Pointer to RenderData
    //! \param    [in] uLayerNum
    //!           Layer total number
    //! \param    [in] SamplerFilterMode
    //!           SamplerFilterMode to be set
    //! \param    [out] pSamplerIndex
    //!           Pointer to Sampler Index
    //! \param    [out] pSurface
    //!           point to Surface
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise MOS_STATUS_UNKNOWN
    //!
    virtual MOS_STATUS SetSamplerFilterMode(
        PMHW_SAMPLER_STATE_PARAM       &pSamplerStateParams,
        PRENDERHAL_SURFACE_STATE_ENTRY  pEntry,
        PVPHAL_RENDERING_DATA_COMPOSITE pRenderData,
        uint32_t                        uLayerNum,
        MHW_SAMPLER_FILTER_MODE         SamplerFilterMode,
        int32_t                        *pSamplerIndex,
        PVPHAL_SURFACE                  pSource) override;
};

#endif // __VPHAL_RENDER_COMPOSITE_XE_XPM_H__
