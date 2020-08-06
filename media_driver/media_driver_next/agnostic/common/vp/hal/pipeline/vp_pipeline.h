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
class SwFilterFeatureHandler;

enum PIPELINE_PARAM_TYPE
{
    PIPELINE_PARAM_TYPE_LEGACY = 1,
};

struct VP_PARAMS
{
    PIPELINE_PARAM_TYPE type;
    union
    {
        PVP_PIPELINE_PARAMS renderParams;
    };
};

class VpPipeline : public MediaPipeline
{
public:
    VpPipeline(PMOS_INTERFACE osInterface);

    virtual ~VpPipeline();

    //!
    //! \brief  Initialize the vp pipeline
    //! \param  [in] mhwInterface
    //!         Pointer to VP_MHWINTERFACE
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Init(void *mhwInterface) override;

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
    //! \brief  get Status Report Table
    //! \return PVPHAL_STATUS_TABLE
    //!         Pointers to status Table
    //!
    PVPHAL_STATUS_TABLE GetStatusReportTable()
    {
        return m_vpMhwInterface.m_statusTable;
    }

    //!
    //! \brief  User Feature Key Report
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS UserFeatureReport() override;

    virtual VphalFeatureReport *GetFeatureReport()
    {
        return m_reporting;
    }

protected:

    //!
    //! \brief  prepare execution params for vp pipeline
    //! \param  [in] params
    //!         Pointer to VP pipeline params
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS PrepareVpPipelineParams(PVP_PIPELINE_PARAMS params);

    //!
    //! \brief  Execute Vp Pipeline, and generate VP Filters
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS ExecuteVpPipeline();

    //!
    //! \brief  Create SwFilterPipe
    //! \param  [in] params
    //!         Pointer to the input parameters
    //! \param  [out] swFilterPipe
    //!         Pointer to swFilterPipe
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS CreateSwFilterPipe(VP_PARAMS &params, SwFilterPipe *&swFilterPipe);

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

    //!
    //! \brief  create packet shared context
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS CreatePacketSharedContext();

    //!
    //! \brief  create feature report
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS CreateFeatureReport();

protected:
    VP_PARAMS              m_pvpParams              = {};   //!< vp Pipeline params
    VP_MHWINTERFACE        m_vpMhwInterface         = {};   //!< vp Pipeline Mhw Interface

    uint8_t                m_numVebox               = 0;
    bool                   m_forceMultiplePipe      = false;
    VpAllocator           *m_allocator              = nullptr;  //!< vp Pipeline allocator
    VPMediaMemComp        *m_mmc                    = nullptr;  //!< vp Pipeline mmc

    // For user feature report
    VphalFeatureReport    *m_reporting              = nullptr;  //!< vp Pipeline user feature report
    VPHAL_OUTPUT_PIPE_MODE m_vpOutputPipe           = VPHAL_OUTPUT_PIPE_MODE_INVALID;
    bool                   m_veboxFeatureInuse      = false;

    VPStatusReport        *m_statusReport           = nullptr;  //!< vp Pipeline status report
    // Surface dumper fields (counter and specification)
    uint32_t               m_frameCounter           = 0;
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
    VpInterface           *m_vpInterface            = nullptr;
};

struct _VP_SFC_PACKET_PARAMS
{
    PSFC_SCALING_PARAMS sfcScalingParams;  //!< Params for SFC Scaling
};

using VP_SFC_PACKET_PARAMS = _VP_SFC_PACKET_PARAMS;

class VpInterface
{
public:
    VpInterface(PVP_MHWINTERFACE pHwInterface, VpAllocator& allocator, VpResourceManager* resourceManager) :
        m_swFilterPipeFactory(*this),
        m_hwFilterPipeFactory(*this),
        m_hwFilterFactory(*this),
        m_hwInterface(pHwInterface),
        m_allocator(allocator),
        m_resourceManager(resourceManager),
        m_swFilterHandler(nullptr) // setting when create feature manager
    {
    }

    virtual ~VpInterface()
    {
    }

    SwFilterPipeFactory& GetSwFilterPipeFactory()
    {
        return m_swFilterPipeFactory;
    }

    void SetSwFilterHandlers(std::map<FeatureType, SwFilterFeatureHandler*>& swFilterHandler)
    {
        m_swFilterHandler = &swFilterHandler;
    }

    SwFilterFeatureHandler* GetSwFilterHandler(FeatureType type)
    {
         if (!m_swFilterHandler)
         {
             return nullptr;
         }

        auto handler = m_swFilterHandler->find(type);

        if (handler != m_swFilterHandler->end())
        {
            return handler->second;
        }
        else
        {
            return nullptr;
        }
    }

    std::map<FeatureType, SwFilterFeatureHandler*>* GetSwFilterHandlerMap()
    {
        return m_swFilterHandler;
    }

    HwFilterPipeFactory& GetHwFilterPipeFactory()
    {
        return m_hwFilterPipeFactory;
    }

    HwFilterFactory& GetHwFilterFactory()
    {
        return m_hwFilterFactory;
    }

    VpAllocator& GetAllocator()
    {
        return m_allocator;
    }

    VpResourceManager* GetResourceManager()
    {
        return m_resourceManager;
    }

    PVP_MHWINTERFACE GetHwInterface()
    {
        return m_hwInterface;
    }

private:
    SwFilterPipeFactory m_swFilterPipeFactory;
    HwFilterPipeFactory m_hwFilterPipeFactory;
    HwFilterFactory     m_hwFilterFactory;

    PVP_MHWINTERFACE    m_hwInterface;
    VpAllocator& m_allocator;
    VpResourceManager* m_resourceManager;
    std::map<FeatureType, SwFilterFeatureHandler*>* m_swFilterHandler = nullptr;
};
}  // namespace vp

#endif