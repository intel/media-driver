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
#ifndef __VP_PIPELINE_ADAPTER_H__
#define __VP_PIPELINE_ADAPTER_H__

#include "vp_pipeline.h"
#include "vp_pipeline_common.h"
#include "vp_pipeline_adapter_base.h"

const SseuSetting VpHalDefaultSSEUTable[baseKernelMaxNumID] =
    {
        // Slice    Sub-Slice       EU      Rsvd(freq)
        {2, 3, 8, 0},  // COMBINED_FC_KERNEL Default

        // 2 VEBOX KERNELS
        {2, 3, 8, 0},  // VEBOX_SECUREBLOCKCOPY_KERNEL,
        {2, 3, 8, 0},  // VEBOX_UPDATEDNSTATE_KERNEL,
};

class VpPipelineAdapter : public VpPipelineAdapterBase
{
public:
    VpPipelineAdapter(
        vp::VpPlatformInterface &vpPlatformInterface,
        MOS_STATUS              &eStatus);

    //!
    //! \brief    VpPipelineAdapter Destuctor
    //! \details  Destroys VpPipelineG12Adapter and all internal states and objects
    //! \return   void
    //!
    virtual ~VpPipelineAdapter();

    //!
    //! \brief    Performs VP Rendering
    //! \details  Performs VP Rendering
    //!           - call default render of video
    //! \param    [in] pcRenderParams
    //!           Pointer to Render Params
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS Render(
        PCVPHAL_RENDER_PARAMS   pcRenderParams) = 0;

    //!
    //! \brief    Allocate VP Resources
    //! \details  Allocate VP Resources
    //!           - Allocate and initialize HW states
    //!           - Allocate and initialize renderer states
    //! \param    [in] pVpHalSettings
    //!           Pointer to VPHAL Settings
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS Allocate(
        const VpSettings *pVpHalSettings) = 0;

    //!
    //! \brief    Get Status Report
    //! \details  Get Status Report, will return back to app indicating if related frame id is done by gpu
    //! \param    [out] pQueryReport
    //!           Pointer to pQueryReport, the status query report array.
    //! \param    [in] wStatusNum
    //!           The size of array pQueryReport.
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    virtual MOS_STATUS GetStatusReport(
        PQUERY_STATUS_REPORT_APP  pQueryReport,
        uint16_t                  numStatus) = 0;

    //!
    //! \brief    Get feature reporting
    //! \details  Get feature reporting
    //! \return   VphalFeatureReport*
    //!           Pointer to VPHAL_FEATURE_REPOR: rendering features reported
    //!
    virtual VphalFeatureReport*       GetRenderFeatureReport() = 0;

    //!
    //! \brief  Finish the execution for each frame
    //! \details Finish the execution for each frame
    //! \param  [in] params
    //!         Pointer to PVP_PIPELINE_PARAMS
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Execute(PVP_PIPELINE_PARAMS params) = 0;

    virtual void Destroy();
    virtual bool IsOclFCEnabled()
    {
        return m_vpPipeline->IsOclFCEnabled();
    }

protected:
    //!
    //! \brief    Allocate VP Resources
    //! \details  Allocate VP Resources
    //!           - Allocate and initialize HW states
    //!           - Allocate and initialize renderer states
    //! \param    [in] pVpHalSettings
    //!           Pointer to VPHAL Settings
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS Init(
        const VpSettings *pVpHalSettings, VP_MHWINTERFACE vpMhwinterface);

    //!
    //! \brief  Finish the execution for each frame
    //! \details Finish the execution for each frame
    //! \param  [in] params
    //!         Pointer to PVP_PIPELINE_PARAMS
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Execute(PVP_PIPELINE_PARAMS params, PRENDERHAL_INTERFACE renderHal);

    std::shared_ptr<vp::VpPipeline>    m_vpPipeline = {};

    VP_PIPELINE_PARAMS                 m_vpPipelineParams = {};   //!< vp Pipeline params
    bool                               m_bApgEnabled = false;    //!< VP APG path enabled

MEDIA_CLASS_DEFINE_END(VpPipelineAdapter)
};
#endif // !__VP_PIPELINE_ADAPTER_H__

