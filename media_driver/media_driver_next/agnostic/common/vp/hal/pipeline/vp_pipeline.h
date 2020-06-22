/*
* Copyright (c) 2018-2020, Intel Corporation
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
//! \file     vp_pipeline.h
//! \brief    Defines the interfaces for vp pipeline
//!           this file is for the base interface which is shared by all features.
//!

#ifndef __VP_PIPELINE_H__
#define __VP_PIPELINE_H__

#include <map>
#include "media_pipeline.h"
#include "vp_filter.h"
#include "vp_pipeline_common.h"
#include "vp_utils.h"
#include "vp_allocator.h"
#include "vp_status_report.h"
#include "vphal.h"
#include "vp_dumper.h"
#include "vp_feature_manager.h"
#include "vp_packet_shared_context.h"

namespace vp
{

class PacketFactory;
class PacketPipeFactory;
class VpResourceManager;

class VpPipeline : public MediaPipeline
{
public:
    VpPipeline(PMOS_INTERFACE osInterface, VphalFeatureReport *reporting);

    virtual ~VpPipeline();

    //!
    //! \brief  Initialize the vp pipeline
    //! \param  [in] settings
    //!         Pointer to the initialize settings
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Init(void *settings) override;

    //!
    //! \brief  Prepare interal parameters, should be invoked for each frame
    //! \param  [in] params
    //!         Pointer to the input parameters
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Prepare(void *params) override;

    //!
    //! \brief  Finish the execution for each frame
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Execute() override;

    //!
    //! \brief  Get media pipeline execution status
    //! \param  [out] status
    //!         The point to encode status
    //! \param  [in] numStatus
    //!         The requested number of status reports
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS GetStatusReport(void *status, uint16_t numStatus) override;

    //!
    //! \brief  Destory the media pipeline and release internal resources
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Destroy() override;


    //!
    //! \brief  set mhw interface for vp pipeline
    //! \param  [in] mhwInterface
    //!         Pointer to the set mhw interface
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetVpPipelineMhwInterfce(void *mhwInterface);

    //!
    //! \brief  get Status Report Table
    //! \return PVPHAL_STATUS_TABLE
    //!         Pointers to status Table
    //!
    PVPHAL_STATUS_TABLE GetStatusReportTable()
    {
        return m_pvpMhwInterface->m_statusTable;
    }

    //!
    //! \brief  User Feature Key Report
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS UserFeatureReport() override;

protected:

    //!
    //! \brief  prepare execution params for vp pipeline
    //! \param  [in] params
    //!         Pointer to VP pipeline params
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS PrepareVpPipelineParams(void *params);

    //!
    //! \brief  prepare execution policy for vp pipeline
    //! \param  [in] params
    //!         Pointer to VP pipeline params
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS PrepareVpExePipe();

    //!
    //! \brief  Execute Vp Pipeline, and generate VP Filters
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS ExecuteVpPipeline();

    //!
    //! \brief  Get System Vebox Number
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS GetSystemVeboxNumber();

    //!
    //! \brief  create media feature manager
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS CreateFeatureManager() override;

    //!
    //! \brief  create reource manager
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS CreateResourceManager();
    virtual MOS_STATUS CheckFeatures(void *params, bool &bapgFuncSupported);

    virtual MOS_STATUS CreatePacketSharedContext();

protected:
    PVP_PIPELINE_PARAMS    m_pvpParams              = nullptr;  //!< vp Pipeline params
    PVP_MHWINTERFACE       m_pvpMhwInterface        = nullptr;  //!< vp Pipeline Mhw Interface
    VP_EXECUTE_CAPS        m_vpPipelineCaps         = {};       //!< vp Pipeline Engine execute caps
    VPHAL_OUTPUT_PIPE_MODE m_vpOutputPipe           = VPHAL_OUTPUT_PIPE_MODE_INVALID;
    uint8_t                m_numVebox               = 0;
    bool                   m_forceMultiplePipe      = false;
    VpAllocator           *m_allocator              = nullptr;  //!< vp Pipeline allocator
    VPMediaMemComp        *m_mmc                    = nullptr;  //!< vp Pipeline mmc
    VphalFeatureReport    *m_reporting              = nullptr;  //!< vp Pipeline user feature report
    VPStatusReport        *m_statusReport           = nullptr;  //!< vp Pipeline status report
    // Surface dumper fields (counter and specification)
    uint32_t               uiFrameCounter           = 0;
#if (_DEBUG || _RELEASE_INTERNAL)
    VpSurfaceDumper       *m_surfaceDumper          = nullptr;
    VpParameterDumper     *m_parameterDumper        = nullptr;
#endif
    bool                   m_currentFrameAPGEnabled = false;
    PacketFactory         *m_pPacketFactory         = nullptr;
    PacketPipeFactory     *m_pPacketPipeFactory     = nullptr;
    VpResourceManager     *m_resourceManager        = nullptr;
    VPFeatureManager      *m_paramChecker           = nullptr;
    VP_PACKET_SHARED_CONTEXT *m_packetSharedContext = nullptr;
};

struct _VP_SFC_PACKET_PARAMS
{
    PSFC_SCALING_PARAMS sfcScalingParams;  //!< Params for SFC Scaling
};

using VP_SFC_PACKET_PARAMS = _VP_SFC_PACKET_PARAMS;
}  // namespace vp

#endif