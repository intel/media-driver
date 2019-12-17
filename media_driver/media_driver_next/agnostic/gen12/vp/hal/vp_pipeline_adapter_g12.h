/*
* Copyright (c) 2018, Intel Corporation
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
#ifndef __VP_PIPELINE_ADAPTER_G12_H__
#define __VP_PIPELINE_ADAPTER_G12_H__

#include "vphal_g12_tgllp.h"
#include "vp_pipeline_g12.h"
#include "vp_pipeline_common.h"

class VpPipelineG12Adapter : public VphalStateG12Tgllp
{
public:
    VpPipelineG12Adapter(
        PMOS_INTERFACE          pOsInterface,
        PMOS_CONTEXT            pOsDriverContext,
        MOS_STATUS              *peStatus);

    //!
    //! \brief    VpPipelineG12Adapter Destuctor
    //! \details  Destroys VpPipelineG12Adapter and all internal states and objects
    //! \return   void
    //!
     ~VpPipelineG12Adapter()
     {
        Destroy();
     };

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
      const VphalSettings     *pVpHalSettings);

    virtual MOS_STATUS Execute(void    *params);

    virtual MOS_STATUS GetStatusReport(
        PQUERY_STATUS_REPORT_APP pQueryReport,
        uint16_t                 numStatus);

    virtual void Destroy();

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
        PCVPHAL_RENDER_PARAMS   pcRenderParams);

    MOS_STATUS Prepare(
        PCVPHAL_RENDER_PARAMS   pcRenderParams);

    //!
    //! \brief    Get feature reporting
    //! \details  Get feature reporting
    //! \return   VphalFeatureReport*
    //!           Pointer to VPHAL_FEATURE_REPOR: rendering features reported
    //!
    virtual VphalFeatureReport*       GetRenderFeatureReport();

protected:
    std::shared_ptr<vp::VpPipelineG12> m_vpPipeline = {};

    VP_MHWINTERFACE                    m_vpMhwinterface = {};   //!< vp Mhw Interface
    VP_PIPELINE_PARAMS                 m_vpPipelineParams = {};   //!< vp Pipeline params
    VphalFeatureReport                *m_reporting = nullptr;
    bool                               m_bApgEnabled = 0;    //!< VP APG path enabled

};
#endif // !__VP_PIPELINE_ADAPTER_G12_H__

