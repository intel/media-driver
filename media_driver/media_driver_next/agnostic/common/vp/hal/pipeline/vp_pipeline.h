/*
* Copyright (c) 2018-2019, Intel Corporation
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

    virtual MOS_STATUS GetStatusReport(void *status, uint16_t numStatus) override;

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
    //! \brief  Initialize the vp pipeline
    //! \param  [in] settings
    //!         Pointer to the initialize settings
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Initialize(void *settings);

    //!
    //! \brief  Create the packet factory
    //! \return PacketFactory *
    //!         pointer to PacketFactory instance.
    //!
    virtual PacketFactory *CreatePacketFactory() = 0;

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
    //! \brief  prepare execution Context for vp pipeline
    //! \param  [in] params
    //!         Pointer to VP pipeline params
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS PrepareVpExeContext();

    //!
    //! \brief  Create VP FilterList
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS CreateFilterList();

    //!
    //! \brief  Execute VP Filter List
    //! \param  [in] executeCaps
    //!         Pointer to execute enginie
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS ExecuteFilter(
        VP_EXECUTE_CAPS executeEngine);

    //!
    //! \brief  Delete VP FilterList
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS DeleteFilter();

    //!
    //! \brief  query filter is existing in filter pool
    //! \param  [in] filterId
    //!         Filter Id
    //! \return bool
    //!         true if exists, else not
    //!
    bool QueryVpFilterExist(FilterType filterId);

    //!
    //! \brief  Register filter into filter pool
    //! \param  [in] filterId
    //!         Filter Id
    //! \param  [in] filter
    //!         Pointer to created filter
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS RegisterVpFilter(FilterType filterId, VpFilter *filter);

    //!
    //! \brief  Execute Vp Pipeline, and generate VP Filters
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS ExecuteVpPipeline();

    //!
    //! \brief  Active Video Processing Packets
    //! \param  [in] packetID
    //!         Packet Id
    //! \param  [in] immediateSubmit
    //!         submit immediate
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS ActivateVideoProcessingPackets(uint32_t packetId, bool immediateSubmit);

    //!
    //! \brief  Allocate Vp Pipeline Packets based on the caps of filter
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AllocateVpPackets(VP_EXECUTE_CAPS *engineCaps) = 0;

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

    virtual MOS_STATUS CheckFeatures(void *params, bool &bapgFuncSupported);

protected:
    std::map<FilterType, VpFilter *> m_AdvfilterList;  //!< filter list: will allcaote a filterList for Render engine

    PVP_PIPELINE_PARAMS    m_pvpParams              = nullptr;  //!< vp Pipeline params
    PVP_MHWINTERFACE       m_pvpMhwInterface        = nullptr;  //!< vp Pipeline Mhw Interface
    VP_EXECUTE_CAPS        m_vpPipelineCaps         = {};       //!< vp Pipeline Engine execute caps
    VPHAL_OUTPUT_PIPE_MODE m_vpOutputPipe           = VPHAL_OUTPUT_PIPE_MODE_COMP;
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
    bool                   m_bEnableFeatureManagerNext = true;
    bool                   m_bBypassSwFilterPipe    = false;
};

struct _VP_SFC_PACKET_PARAMS
{
    PSFC_SCALING_PARAMS sfcScalingParams;  //!< Params for SFC Scaling
};

using VP_SFC_PACKET_PARAMS = _VP_SFC_PACKET_PARAMS;
}  // namespace vp

#endif