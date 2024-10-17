/*
* Copyright (c) 2022-2024, Intel Corporation
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

#include <stdint.h>
#include "mos_defs.h"
#include "vp_common.h"
#include "vp_common_tools.h"
#include "mhw_vebox.h"
#include "mhw_sfc.h"
#include "igfxfmid.h"
#include "media_class_trace.h"
#include "media_skuwa_specific.h"
#include "mos_os.h"
#include "mos_os_specific.h"
#include "vp_feature_report.h"
#include "mos_os_cp_interface_specific.h"

class MediaScalability;
class MediaContext;

using VphalFeatureReport = VpFeatureReport;

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
//! Structure VpSettings
//! \brief VPHAL Settings - controls allocation of internal resources in VPHAL
//!
struct VpSettings
{
    //!
    //! \brief    VpSettings Constructor
    //! \details  Creates instance of VpSettings
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
    bool     clearVideoViewMode;      //!< Perf Optimize for ClearVideoView DDI
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

    MediaScalability *m_singlePipeScalability;
    MediaScalability *m_multiPipeScalability;

    MOS_STATUS (*pfnCreateSinglePipe)(
    void *hwInterface, MediaContext *mediaContext, uint8_t componentType);

    MOS_STATUS (*pfnCreateMultiPipe)(
    void *hwInterface, MediaContext *mediaContext, uint8_t componentType);

    bool m_bIsMediaSfcInterfaceInUse = false;
};

using VP_MHWINTERFACE  = _VP_MHWINTERFACE;
using PVP_MHWINTERFACE = VP_MHWINTERFACE *;

class VpBase
{
public:
    VpBase();

    virtual ~VpBase();

    // factory function
    static VpBase* VphalStateFactory(
        PMOS_INTERFACE     osInterface,
        MOS_CONTEXT_HANDLE osDriverContext,
        MOS_STATUS         *eStatus,
        bool               clearViewMode = false);

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

    virtual VphalFeatureReport *GetRenderFeatureReport() = 0;

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

    virtual bool IsOclFCEnabled()
    {
        return false;
    }

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
