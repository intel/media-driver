/*
* Copyright (c) 2021, Intel Corporation
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
//! \file     vp_pipeline_adapter_base.h
//! \brief    vp pipeline adapter base clarification
//! \details  vp pipeline adapter base clarification inlcuding:
//!           some marcro, enum, structure, function
//!
#ifndef __VP_PIPELINE_ADAPTER_BASE_H__
#define __VP_PIPELINE_ADAPTER_BASE_H__
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
};

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
    VpSettings()    : maxPhases(0),
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
    uint32_t clearVideoViewMode;     //!< Perf Optimize for ClearVideoView DDI
};

//!
//! \brief Deinterlace Mode enum
//!
typedef enum
{
    VPDDI_PROGRESSIVE = 0,  //!< Progressive mode
    VPDDI_BOB         = 1,  //!< BOB DI mode
    VPDDI_ADI         = 2   //!< ADI mode
} DI_MODE;

//!
//! \brief Scaling Mode enum
//!
typedef enum
{
    VPDDI_SCALING                = 0,  //!< Bilinear scaling
    VPDDI_ADVANCEDSCALING        = 1,  //!< AVS scaling
    VPDDI_SUPERRESOLUTIONSCALING = 2   //!< Super scaling
} SCALING_MODE;

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

//!
//! Class VpPipelineAdapterBase
//! \brief VP_INTERFACE class definition
//!
class VpPipelineAdapterBase
{
public:
    // factory function
    static VpPipelineAdapterBase *VphalStateFactory(
        PMOS_INTERFACE pOsInterface,
        PMOS_CONTEXT   pOsDriverContext,
        MOS_STATUS *   peStatus);

    //!
    //! \brief    VpPipelineAdapterBase Constructor
    //! \details  Creates instance of VpPipelineAdapterBase
    //!           - Caller must call Allocate to allocate all VPHAL states and objects.
    //! \param    [in] pOsInterface
    //!           OS interface, if provided externally - may be nullptr
    //! \param    [in] pOsDriverContext
    //!           OS driver context (UMD context, pShared, ...)
    //! \param    [in,out] peStatus
    //!           Pointer to the MOS_STATUS flag.
    //!           Will assign this flag to MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    VpPipelineAdapterBase(
        vp::VpPlatformInterface &vpPlatformInterface,
        MOS_STATUS &eStatus);

    virtual MOS_STATUS GetVpMhwInterface(
        VP_MHWINTERFACE &vpMhwinterface);

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
        PCVPHAL_RENDER_PARAMS pcRenderParams) = 0;

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
        PQUERY_STATUS_REPORT_APP pQueryReport,
        uint16_t                 numStatus);

    virtual VpFeatureReport *GetRenderFeatureReport() = 0;

    //!
    //! \brief    VpPipelineAdapterBase Destuctor
    //! \details  Destroys VpPipelineAdapterBase and all internal states and objects
    //! \return   void
    //!
    virtual ~VpPipelineAdapterBase();

    PMOS_INTERFACE GetOsInterface()
    {
        return m_pOsInterface;
    }

    void SetMhwVeboxInterface(MhwVeboxInterface *veboxInterface)
    {
        MOS_STATUS                  eStatus = MOS_STATUS_SUCCESS;
        if (veboxInterface == nullptr)
        {
            return;
        }

        if (m_veboxInterface != nullptr)
        {
            m_veboxItf = std::static_pointer_cast<mhw::vebox::Itf>(m_veboxInterface->GetNewVeboxInterface());

            if (m_veboxItf)
            {
                eStatus = m_veboxItf->DestroyHeap();
            }
            else
            {
                eStatus = m_veboxInterface->DestroyHeap();
            }

            MOS_Delete(m_veboxInterface);
            m_veboxInterface = nullptr;
            m_veboxItf       = nullptr;
            if (eStatus != MOS_STATUS_SUCCESS)
            {
                VP_PUBLIC_ASSERTMESSAGE("Failed to destroy Vebox Interface, eStatus:%d.\n", eStatus);
            }
        }

        m_veboxInterface = veboxInterface;
        m_veboxItf = std::static_pointer_cast<mhw::vebox::Itf>(veboxInterface->GetNewVeboxInterface());
    }

    void SetMhwSfcInterface(MhwSfcInterface *sfcInterface)
    {
        if (sfcInterface == nullptr)
        {
            return;
        }

        if (m_sfcInterface != nullptr)
        {
            MOS_Delete(m_sfcInterface);
            m_sfcInterface = nullptr;
        }

        m_sfcInterface = sfcInterface;
    }

    HANDLE m_gpuAppTaskEvent = nullptr;

protected:
    // Internals
    PLATFORM             m_platform = {};
    MEDIA_FEATURE_TABLE *m_skuTable = nullptr;
    MEDIA_WA_TABLE *     m_waTable  = nullptr;

    // States
    PMOS_INTERFACE       m_pOsInterface = nullptr;
    PRENDERHAL_INTERFACE m_vprenderHal   = nullptr;
    PMHW_VEBOX_INTERFACE m_veboxInterface = nullptr;
    MhwCpInterface *     m_cpInterface    = nullptr;
    PMHW_SFC_INTERFACE   m_sfcInterface   = nullptr;
    std::shared_ptr<mhw::vebox::Itf> m_veboxItf = nullptr;

    // StatusTable indicating if command is done by gpu or not
    VPHAL_STATUS_TABLE       m_statusTable = {};
    vp::VpPlatformInterface &m_vpPlatformInterface;  //!< vp platform interface. Should be destroyed during deconstruction.

MEDIA_CLASS_DEFINE_END(VpPipelineAdapterBase)
};

#endif  // __VP_PIPELINE_ADAPTER_BASE_H__