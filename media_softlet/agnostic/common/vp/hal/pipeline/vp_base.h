/*
* Copyright (c) 2022, Intel Corporation
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
//! \file     vp_base.h
//! \brief    vp base clarification
//! \details  vp base clarification inlcuding:
//!           some marcro, enum, structure, function
//!
#ifndef __VP_BASE_H__
#define __VP_BASE_H__
#include "vp_common.h"
#include "vphal_common_tools.h"
#include "mhw_vebox.h"
#include "mhw_sfc.h"
#include "vp_utils.h"
#include "media_interfaces_mhw.h"
#include "vp_feature_report.h"
#include "mhw_vebox_itf.h"

namespace vp
{
class VpPlatformInterface;
class VpUserFeatureControl;
};  // namespace vp

class VpExtIntfBase;

//-----------------------------------------------------------------------------
// VPHAL-DDI RENDERING INTERFACE
//
//      Params that may apply to more than one layer are part of VPHAL_SURFACE
//      DDI layers must set this interface before calling pfnRender
//-----------------------------------------------------------------------------
//!
//! Structure VphalSettings
//! \brief VPHAL Settings - controls allocation of internal resources in VPHAL
//!
struct VpSettings
{
    //!
    //! \brief    VphalSettings Constructor
    //! \details  Creates instance of VphalSettings
    //!
    VpSettings() : maxPhases(0),
                   mediaStates(0),
                   sameSampleThreshold(0),
                   disableDnDi(0),
                   kernelUpdate(0),
                   disableHdr(0),
                   veboxParallelExecution(0),
                   clearVideoViewMode(false){};

    int32_t  maxPhases;
    int32_t  mediaStates;
    int32_t  sameSampleThreshold;
    uint32_t disableDnDi;             //!< For validation purpose
    uint32_t kernelUpdate;            //!< For VEBox Copy and Update kernels
    uint32_t disableHdr;              //!< Disable Hdr
    uint32_t veboxParallelExecution;  //!< Control VEBox parallel execution with render engine
    uint32_t clearVideoViewMode;      //!< Perf Optimize for ClearVideoView DDI
};

struct _VP_MHWINTERFACE
{
    // Internals
    PLATFORM             m_platform;
    MEDIA_FEATURE_TABLE *m_skuTable;
    MEDIA_WA_TABLE *     m_waTable;

    // States
    PMOS_INTERFACE           m_osInterface;
    PRENDERHAL_INTERFACE     m_renderHal;
    PMHW_VEBOX_INTERFACE     m_veboxInterface;
    MhwCpInterface *         m_cpInterface;
    PMHW_SFC_INTERFACE       m_sfcInterface;
    PMHW_MI_INTERFACE        m_mhwMiInterface;
    vp::VpPlatformInterface *m_vpPlatformInterface;
    void *                   m_settings;
    VpFeatureReport *        m_reporting;

    // Render GPU context/node
    MOS_GPU_NODE    m_renderGpuNode;
    MOS_GPU_CONTEXT m_renderGpuContext;

    // vp Pipeline workload status report
    PVPHAL_STATUS_TABLE m_statusTable;

    void *m_debugInterface;
    vp::VpUserFeatureControl *m_userFeatureControl;
};

using VP_MHWINTERFACE  = _VP_MHWINTERFACE;

class VpBase
{
public:
    VpBase();

    virtual ~VpBase();

    // factory function
    static VpBase* VphalStateFactory(
        PMOS_INTERFACE osInterface,
        PMOS_CONTEXT   osDriverContext,
        MOS_STATUS     *eStatus);

    //!
    //! \brief    Allocate VpPipelineAdapterBase Resources
    //! \details  Allocate VpPipelineAdapterBase Resources
    //!           - Allocate and initialize HW states
    //!           - Allocate and initialize renderer states
    //! \param    [in] pVpHalSettings
    //!           Pointer to VPHAL Settings
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS Allocate(
        const VpSettings *pVpSettings) = 0;

    virtual VpFeatureReport* GetRenderFeatureReport() = 0;

    virtual PLATFORM& GetPlatform() = 0;

    virtual MEDIA_FEATURE_TABLE* GetSkuTable() = 0;

    virtual PMOS_INTERFACE GetOsInterface() = 0;

    virtual PRENDERHAL_INTERFACE GetRenderHal() = 0;

    virtual MOS_STATUS Render(
        PCVPHAL_RENDER_PARAMS pcRenderParams) = 0;

    virtual MOS_STATUS GetStatusReport(
        PQUERY_STATUS_REPORT_APP pQueryReport,
        uint16_t                 numStatus) = 0;
    
    virtual MOS_STATUS GetStatusReportEntryLength(
        uint32_t                         *puiLength) = 0;

    HANDLE m_gpuAppTaskEvent = nullptr;

    VpExtIntfBase *extIntf = nullptr;

protected:
    virtual bool IsApoEnabled()
    {
        return false;
    }

MEDIA_CLASS_DEFINE_END(VpBase)
};

class VpExtIntfBase
{
public:
    VpExtIntfBase(VpBase *hal) : m_hal(hal)
    {
    }

    virtual ~VpExtIntfBase()
    {
    }

protected:
    VpBase *m_hal = nullptr;

MEDIA_CLASS_DEFINE_END(VpExtIntfBase)
};

#endif  // __VPBASE_H__