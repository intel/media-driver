/*
* Copyright (c) 2018-2022, Intel Corporation
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
#include "vp_dumper.h"
#include "vp_debug_interface.h"
#include "vp_feature_manager.h"
#include "vp_packet_shared_context.h"
#include "vp_kernelset.h"
#include "vp_packet_reuse_manager.h"

namespace vp
{

class PacketFactory;
class PacketPipeFactory;
class VpResourceManager;
class SwFilterFeatureHandler;

enum PIPELINE_PARAM_TYPE
{
    PIPELINE_PARAM_TYPE_LEGACY = 1,
    PIPELINE_PARAM_TYPE_MEDIA_SFC_INTERFACE,
};

struct VP_PARAMS
{
    PIPELINE_PARAM_TYPE type;
    union
    {
        PVP_PIPELINE_PARAMS renderParams;
        VEBOX_SFC_PARAMS    *sfcParams;
    };
};

class VpPipelineParamFactory
{
public:
    VpPipelineParamFactory(){};
    virtual ~VpPipelineParamFactory()
    {
        while (!m_Pool.empty())
        {
            PVP_PIPELINE_PARAMS param = m_Pool.back();
            m_Pool.pop_back();
            MOS_Delete(param);
        }
    }

    virtual PVP_PIPELINE_PARAMS Clone(PVP_PIPELINE_PARAMS param)
    {
        PVP_PIPELINE_PARAMS paramDst = nullptr;

        if (m_Pool.empty())
        {
            paramDst = MOS_New(VP_PIPELINE_PARAMS);
            *paramDst = *param;
        }
        else
        {
            paramDst = m_Pool.back();
            if (paramDst)
            {
                m_Pool.pop_back();
                *paramDst = *param;
            }
        }
        return paramDst;
    }

    virtual MOS_STATUS Destroy(PVP_PIPELINE_PARAMS &param)
    {
        if (param == nullptr)
        {
            return MOS_STATUS_SUCCESS;
        }
        m_Pool.push_back(param);
        param = nullptr;
        return MOS_STATUS_SUCCESS;
    }

    std::vector<PVP_PIPELINE_PARAMS> m_Pool;

MEDIA_CLASS_DEFINE_END(vp__VpPipelineParamFactory)
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
    //! \brief  Destory the tempSurface and release internal resources
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
#if (_DEBUG || _RELEASE_INTERNAL)
    MOS_STATUS DestroySurface();
#endif

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

    //!
    //! \brief    Check whether VEBOX-SFC Format Supported
    //! \details  Check whether VEBOX-SFC Format Supported.
    //! \param    inputFormat
    //!           [in] Format of Input Frame
    //! \param    outputFormat
    //!           [in] Format of Output Frame
    //! \return   bool
    //!           Return true if supported, otherwise failed
    //!
    bool IsVeboxSfcFormatSupported(MOS_FORMAT formatInput, MOS_FORMAT formatOutput);

    virtual MOS_STATUS ProcessBypassHandler(PVP_PIPELINE_PARAMS renderParams, bool &isBypassNeeded)
    {
        return MOS_STATUS_SUCCESS;
    }

    virtual MOS_STATUS CreateVPDebugInterface();

    VpUserFeatureControl *GetUserFeatureControl()
    {
        return m_userFeatureControl;
    }

    VpAllocator *GetAllocator()
    {
        return m_allocator;
    }

    virtual bool IsOclFCEnabled()
    {
        return m_vpMhwInterface.m_userFeatureControl->EnableOclFC();
    }

    // for debug purpose
#if (_DEBUG || _RELEASE_INTERNAL)
    //!
    //! \brief  replace output surface from Tile-Y to Linear
    //! \param  [in] params
    //!         Pointer to VP pipeline params
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SurfaceReplace(PVP_PIPELINE_PARAMS params);
    virtual VPHAL_SURFACE *AllocateTempTargetSurface(VPHAL_SURFACE *m_tempTargetSurface);
#endif

    static const uint32_t m_scalability_threshWidth  = 4096;
    static const uint32_t m_scalability_threshHeight = 2880;

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
    //! \brief  prepare execution params for vp scalability pipeline
    //! \param  [in] params
    //!         Pointer to VP scalability pipeline params
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS PrepareVpPipelineScalabilityParams(VEBOX_SFC_PARAMS* params);


    //!
    //! \brief  prepare execution params for vp scalability pipeline
    //! \param  [in] params
    //!         Pointer to VP scalability pipeline params
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS PrepareVpPipelineScalabilityParams(PVP_PIPELINE_PARAMS params);

    //!
    //! \brief  prepare execution params for vp scalability pipeline
    //! \param  [in] params
    //!         src and dst surface's width and height
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS PrepareVpPipelineScalabilityParams(uint32_t srcWidth, uint32_t srcHeight, uint32_t dstWidth, uint32_t dstHeight);

    //!
    //! \brief  Execute Vp Pipeline, and generate VP Filters
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS ExecuteVpPipeline();

    //!
    //! \brief  updated Execute Vp Pipeline status
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS UpdateExecuteStatus(uint32_t frameCn);

    //!
    //! \brief  Create SwFilterPipe
    //! \param  [in] params
    //!         Pointer to the input parameters
    //! \param  [out] swFilterPipe
    //!         Pointer to swFilterPipe
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS CreateSwFilterPipe(VP_PARAMS &params, std::vector<SwFilterPipe*> &swFilterPipe);

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
    virtual MOS_STATUS CreateFeatureManager(VpResourceManager * vpResourceManager);

    virtual MOS_STATUS CreateSinglePipeContext();

//!
//! \brief  create media kernel sets
//! \return MOS_STATUS
//!         MOS_STATUS_SUCCESS if success, else fail reason
//!
    virtual MOS_STATUS CreateVpKernelSets();

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

    virtual MOS_STATUS CreateReport()
    {
        m_reporting        = MOS_New(VphalFeatureReport);
        VP_PUBLIC_CHK_NULL_RETURN(m_reporting);
        m_reporting->owner = this;
        return MOS_STATUS_SUCCESS;
    }

    virtual MOS_STATUS CreateUserFeatureControl();

    //!
    //! \brief  set Predication Params
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetPredicationParams(PVP_PIPELINE_PARAMS params)
    {
        return MOS_STATUS_SUCCESS;
    }

    virtual MOS_STATUS UpdateFrameTracker();

#if (_DEBUG || _RELEASE_INTERNAL)
    //!
    //! \brief  Report INTER_FRAME_MEMORY_NINJA_START_COUNTER and INTER_FRAME_MEMORY_NINJA_END_COUNTER
    //!         INTER_FRAME_MEMORY_NINJA_START_COUNTER will be reported in Prepare() function
    //!         INTER_FRAME_MEMORY_NINJA_END_COUNTER will be reported in UserFeatureReport() function which runs in EXecute()
    //! \param
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS ReportIFNCC(bool bStart);
#endif

    //!
    //! \brief  set Video Processing Settings
    //! \param  [in] params
    //!         Pointer to the input parameters
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetVideoProcessingSettings(void* settings)
    {
        if (!settings)
        {
            VP_PUBLIC_NORMALMESSAGE("No VPP Settings needed, driver to handle features behavious by self");
            return MOS_STATUS_SUCCESS;
        }

        VP_SETTINGS* vpSettings = (VP_SETTINGS*)settings;

        if (m_vpSettings == nullptr)
        {
            m_vpSettings = (VP_SETTINGS*)MOS_AllocAndZeroMemory(sizeof(VP_SETTINGS));

            if (m_vpSettings == nullptr)
            {
                VP_PUBLIC_ASSERTMESSAGE("No Space, VP Settings created failed");
                return MOS_STATUS_NO_SPACE;
            }
        }

        *m_vpSettings = *vpSettings;

        return MOS_STATUS_SUCCESS;
    }

    //!
    //! \brief  Judge if it is gt test environment
    //! \return bool
    //!         true if success, else false
    //!
    virtual bool IsGtEnv()
    {
        return false;
    }

    //!
    //! \brief  Judge if it is gt test environment
    //! \return bool
    //!         true if success, else false
    //!
    virtual bool IsMultiple()
    {
        return (m_numVebox > 1) ? true : false;
    }

    MOS_STATUS UpdateVeboxNumberforScalability();

    MOS_STATUS ExecuteSingleswFilterPipe(VpSinglePipeContext *singlePipeCtx, SwFilterPipe *&pipe, PacketPipe *pPacketPipe, VpFeatureManagerNext *featureManagerNext);
    MOS_STATUS UpdateRectForNegtiveDstTopLeft(PVP_PIPELINE_PARAMS params);

protected:
    VP_PARAMS              m_pvpParams              = {};   //!< vp Pipeline params
    VP_MHWINTERFACE        m_vpMhwInterface         = {};   //!< vp Pipeline Mhw Interface

    uint8_t                m_numVebox               = 0;
    uint8_t                m_numVeboxOriginal       = 0;
    uint32_t               m_forceMultiplePipe      = 0;
    VpAllocator           *m_allocator              = nullptr;  //!< vp Pipeline allocator
    VPMediaMemComp        *m_mmc                    = nullptr;  //!< vp Pipeline mmc

    // For user feature report
    VphalFeatureReport    *m_reporting              = nullptr;  //!< vp Pipeline user feature report

    VPStatusReport        *m_statusReport           = nullptr;  //!< vp Pipeline status report

#if (_DEBUG || _RELEASE_INTERNAL)
    VpDebugInterface      *m_debugInterface         = nullptr;
#endif
    bool                   m_currentFrameAPGEnabled = false;
    PacketFactory         *m_pPacketFactory         = nullptr;
    PacketPipeFactory     *m_pPacketPipeFactory     = nullptr;
    VpKernelSet           *m_kernelSet              = nullptr;
    VPFeatureManager      *m_paramChecker           = nullptr;
    VP_PACKET_SHARED_CONTEXT *m_packetSharedContext = nullptr;
    VpInterface           *m_vpInterface            = nullptr;
#if (_DEBUG || _RELEASE_INTERNAL)
    VPHAL_SURFACE         *m_tempTargetSurface      = nullptr;
#endif
    VP_SETTINGS           *m_vpSettings = nullptr;
    VpUserFeatureControl  *m_userFeatureControl = nullptr;
    std::vector<VpSinglePipeContext *> m_vpPipeContexts     = {};
    VpPipelineParamFactory            *m_pipelineParamFactory = nullptr;
    bool                               m_reportOnceFlag       = true;

    MEDIA_CLASS_DEFINE_END(vp__VpPipeline)
};

struct _VP_SFC_PACKET_PARAMS
{
    PSFC_SCALING_PARAMS sfcScalingParams;  //!< Params for SFC Scaling
};

using VP_SFC_PACKET_PARAMS = _VP_SFC_PACKET_PARAMS;

class VpSinglePipeContext
{
public:
    VpSinglePipeContext();
    virtual ~VpSinglePipeContext();

    virtual MOS_STATUS Init(PMOS_INTERFACE osInterface, VpAllocator *allocator, VphalFeatureReport *reporting, vp::VpPlatformInterface *vpPlatformInterface, PacketPipeFactory *packetPipeFactory, VpUserFeatureControl *userFeatureControl, MediaCopyWrapper *mediaCopyWrapper);

    //!
    //! \brief  create reource manager
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS CreateResourceManager(PMOS_INTERFACE osInterface, VpAllocator *allocator, VphalFeatureReport *reporting, vp::VpPlatformInterface *vpPlatformInterface, vp::VpUserFeatureControl *userFeatureControl, MediaCopyWrapper *mediaCopyWrapper);

    virtual MOS_STATUS CreatePacketReuseManager(PacketPipeFactory *pPacketPipeFactory, VpUserFeatureControl *userFeatureControl);

    virtual VpPacketReuseManager *NewVpPacketReuseManagerObj(PacketPipeFactory *packetPipeFactory, VpUserFeatureControl *userFeatureControl)
    {
        return packetPipeFactory && userFeatureControl ? MOS_New(VpPacketReuseManager, *packetPipeFactory, *userFeatureControl) : nullptr;
    }

    VpPacketReuseManager *GetPacketReUseManager()
    {
        return m_packetReuseMgr;
    }

    VpResourceManager *GetVpResourceManager()
    {
        return m_resourceManager;
    }

    bool IsPacketReUsed()
    {
        return m_packetReused;
    }

    bool IsVeboxInUse()
    {
        return m_veboxFeatureInuse;
    }

    uint32_t GetFrameCounter()
    {
        return m_frameCounter;
    }

    void AddFrameCount()
    {
        m_frameCounter++;
    }

    void InitializeOutputPipe()
    {
        m_vpOutputPipe      = VPHAL_OUTPUT_PIPE_MODE_INVALID;
        m_veboxFeatureInuse = false;
    }

    VPHAL_OUTPUT_PIPE_MODE GetOutputPipe()
    {
        return m_vpOutputPipe;
    }

    void SetOutputPipeMode(VPHAL_OUTPUT_PIPE_MODE mode)
    {
        m_vpOutputPipe = mode;
    }

    void SetPacketReused(bool isReused)
    {
        m_packetReused = isReused;
    }

    void SetIsVeboxFeatureInuse(bool isInuse)
    {
        m_veboxFeatureInuse = isInuse;
    }

protected:
    VpPacketReuseManager *m_packetReuseMgr  = nullptr;
    VpResourceManager    *m_resourceManager = nullptr;
    // Surface dumper fields (counter and specification)
    uint32_t               m_frameCounter      = 0;
    bool                   m_packetReused      = false;  //!< true is packet reused.
    VPHAL_OUTPUT_PIPE_MODE m_vpOutputPipe      = VPHAL_OUTPUT_PIPE_MODE_INVALID;
    bool                   m_veboxFeatureInuse = false;
    MEDIA_CLASS_DEFINE_END(vp__VpSinglePipeContext)
};

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

        type = (FeatureType)(type & FEATURE_TYPE_MASK);

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

    MOS_STATUS SwitchResourceManager(VpResourceManager *resManager)
    {
        m_resourceManager = resManager;
        return MOS_STATUS_SUCCESS;
    }

private:
    SwFilterPipeFactory m_swFilterPipeFactory;
    HwFilterPipeFactory m_hwFilterPipeFactory;
    HwFilterFactory     m_hwFilterFactory;

    PVP_MHWINTERFACE    m_hwInterface;
    VpAllocator& m_allocator;
    VpResourceManager* m_resourceManager;
    std::map<FeatureType, SwFilterFeatureHandler*>* m_swFilterHandler = nullptr;

MEDIA_CLASS_DEFINE_END(vp__VpInterface)
};
}  // namespace vp

#endif
