/*===================== begin_copyright_notice ==================================

# Copyright (c) 2021, Intel Corporation

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
//! \file     vphal_render_composite_xe_xpm_plus.h
//! \brief    Common interface and structure used in Composite
//! \details  Common interface and structure used in Composite
//!
#ifndef __VPHAL_RENDER_COMPOSITE_XE_XPM_PLUS_H__
#define __VPHAL_RENDER_COMPOSITE_XE_XPM_PLUS_H__

#include "vphal_render_composite.h"
#include "hal_kerneldll.h"
#include "renderhal_platform_interface.h"
#include "vphal_render_composite_g12.h"
//!
//! \brief Class to VPHAL Composite G12HP render
//!
class CompositeStateXe_Xpm_Plus : virtual public CompositeStateG12
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
    CompositeStateXe_Xpm_Plus (
        PMOS_INTERFACE                      pOsInterface,
        PRENDERHAL_INTERFACE                pRenderHal,
        PVPHAL_RNDR_PERF_DATA               pPerfData,
        const VPHAL_COMPOSITE_CACHE_CNTL    &compositeCacheCntl,
        MOS_STATUS                          *peStatus);
 
    //!
    //! \brief    Composite render Destructor
    //! \details  Destroy Composite render and release all related RenderState resources
    //!
    virtual ~CompositeStateXe_Xpm_Plus()
    {
    }

    //!
    //! \brief    caculate kernel block size
    //! \param    [in, out] uiBlockSize
    //!           point to kernel block size
    //! \return   void
    //!
    void CaculateBlockSize(uint32_t* uiBlockSize);

    virtual void SetFilterScalingRatio(
     Kdll_Scalingratio* ScalingRatio);

    virtual MOS_STATUS RenderInit(
     PVPHAL_COMPOSITE_PARAMS           pCompParams,
     PVPHAL_RENDERING_DATA_COMPOSITE   pRenderingData);

    virtual bool RenderBufferComputeWalker(
     PMHW_BATCH_BUFFER               pBatchBuffer,
     PVPHAL_RENDERING_DATA_COMPOSITE pRenderingData,
     PMHW_GPGPU_WALKER_PARAMS        pWalkerParams);

//!
//! \brief    Submit Composite states
//! \details  Submit Composite states, including load CSC matrix, set NLAS Inline data,
//!           set background color, load Palettes, set output format, load kernel, load
//!           curbe data, set sampler state, set VFE State params, and etc
//! \param    [in] pRenderingData
//!           Pointer to Composite state
//! \return   bool
//!           Return TURE if successful, otherwise false
//!
    virtual bool SubmitStates(
     PVPHAL_RENDERING_DATA_COMPOSITE     pRenderingData);

};

#endif // __VPHAL_RENDER_COMPOSITE_XE_XPM_PLUS_H__
