/*
* Copyright (c) 2009-2022, Intel Corporation
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
//! \file     vphal.h
//! \brief    vphal interface clarification
//! \details  vphal interface clarification inlcuding:
//!           some marcro, enum, structure, function
//!
#ifndef __VPHAL_H__
#define __VPHAL_H__

#include "mos_utilities.h"
#include "mos_util_debug.h"
#include "mhw_vebox.h"
#include "mhw_sfc.h"
#include "vp_pipeline_adapter_base.h"
#include "vphal_common_hdr.h"
#include "vp_common.h"
#include "vp_base.h"
#include "renderhal_legacy.h"

// YUV input ranges
#define YUV_RANGE_FROM_DDI              3

// RGB input ranges
#define RGB_RANGE_16_235                1
#define RGB_RANGE_0_255                 0

//------------------------------------------------------------------------------
// Simplified macros for debug message, Assert, Null check and MOS eStatus check
// within VPhal without the need to explicitly pass comp and sub-comp name
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Macros specific to MOS_VP_SUBCOMP_HW sub-comp
//------------------------------------------------------------------------------
#define VPHAL_HW_ASSERT(_expr)                                                       \
    MOS_ASSERT(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_HW, _expr)

#define VPHAL_HW_ASSERTMESSAGE(_message, ...)                                        \
    MOS_ASSERTMESSAGE(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_HW, _message, ##__VA_ARGS__)

#define VPHAL_HW_NORMALMESSAGE(_message, ...)                                        \
    MOS_NORMALMESSAGE(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_HW, _message, ##__VA_ARGS__)

#define VPHAL_HW_VERBOSEMESSAGE(_message, ...)                                       \
    MOS_VERBOSEMESSAGE(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_HW, _message, ##__VA_ARGS__)

#define VPHAL_HW_FUNCTION_ENTER                                                      \
    MOS_FUNCTION_ENTER(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_HW)

#define VPHAL_HW_CHK_STATUS(_stmt)                                                   \
    MOS_CHK_STATUS(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_HW, _stmt)

#define VPHAL_HW_CHK_NULL(_ptr)                                                      \
    MOS_CHK_NULL(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_HW, _ptr)

#define VPHAL_HW_CHK_NULL_NO_STATUS(_ptr)                                            \
    MOS_CHK_NULL_NO_STATUS(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_HW, _ptr)

//------------------------------------------------------------------------------
// Macros specific to MOS_VP_SUBCOMP_PUBLIC sub-comp
//------------------------------------------------------------------------------
#define VPHAL_PUBLIC_ASSERT(_expr)                                                   \
    MOS_ASSERT(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_PUBLIC, _expr)

#define VPHAL_PUBLIC_ASSERTMESSAGE(_message, ...)                                    \
    MOS_ASSERTMESSAGE(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_PUBLIC, _message, ##__VA_ARGS__)

#define VPHAL_PUBLIC_NORMALMESSAGE(_message, ...)                                    \
    MOS_NORMALMESSAGE(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_PUBLIC, _message, ##__VA_ARGS__)

#define VPHAL_PUBLIC_VERBOSEMESSAGE(_message, ...)                                   \
    MOS_VERBOSEMESSAGE(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_PUBLIC, _message, ##__VA_ARGS__)

#define VPHAL_PUBLIC_FUNCTION_ENTER                                                  \
    MOS_FUNCTION_ENTER(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_PUBLIC)

#define VPHAL_PUBLIC_CHK_STATUS(_stmt)                                               \
    MOS_CHK_STATUS(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_PUBLIC, _stmt)

#define VPHAL_PUBLIC_CHK_STATUS_RETURN(_stmt)                                        \
    MOS_CHK_STATUS_RETURN(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_PUBLIC, _stmt)

#define VPHAL_PUBLIC_CHK_NULL(_ptr)                                                  \
    MOS_CHK_NULL(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_PUBLIC, _ptr)

#define VPHAL_PUBLIC_CHK_NULL_NO_STATUS(_ptr)                                        \
    MOS_CHK_NULL_NO_STATUS(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_PUBLIC, _ptr)

#define VPHAL_PUBLIC_CHK_NULL_NO_STATUS_RETURN(_ptr)                                 \
    MOS_CHK_NULL_NO_STATUS_RETURN(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_PUBLIC, _ptr)

//------------------------------------------------------------------------------
// Macros specific to MOS_VP_SUBCOMP_DEBUG sub-comp
//------------------------------------------------------------------------------
#define VPHAL_DEBUG_ASSERT(_expr)                                                    \
    MOS_ASSERT(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_DEBUG, _expr)

#define VPHAL_DEBUG_ASSERTMESSAGE(_message, ...)                                     \
    MOS_ASSERTMESSAGE(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_DEBUG, _message, ##__VA_ARGS__)

#define VPHAL_DEBUG_NORMALMESSAGE(_message, ...)                                     \
    MOS_NORMALMESSAGE(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_DEBUG, _message, ##__VA_ARGS__)

#define VPHAL_DEBUG_VERBOSEMESSAGE(_message, ...)                                    \
    MOS_VERBOSEMESSAGE(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_DEBUG, _message, ##__VA_ARGS__)

#define VPHAL_DEBUG_FUNCTION_ENTER                                                   \
    MOS_FUNCTION_ENTER(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_DEBUG)

#define VPHAL_DEBUG_FUNCTION_EXIT                                                    \
    MOS_FUNCTION_EXIT(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_DEBUG)

#define VPHAL_DEBUG_CHK_STATUS(_stmt)                                                \
    MOS_CHK_STATUS(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_DEBUG, _stmt)

#define VPHAL_DEBUG_CHK_NULL(_ptr)                                                   \
    MOS_CHK_NULL(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_DEBUG, _ptr)

#define VPHAL_DEBUG_CHK_NULL_NO_STATUS(_ptr)                                         \
    MOS_CHK_NULL_NO_STATUS(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_DEBUG, _ptr)

//------------------------------------------------------------------------------
// Macros specific to MOS_VP_SUBCOMP_RENDER sub-comp
//------------------------------------------------------------------------------
#define VPHAL_RENDER_ASSERT(_expr)                                                   \
    MOS_ASSERT(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_RENDER, _expr)

#define VPHAL_RENDER_ASSERTMESSAGE(_message, ...)                                    \
    MOS_ASSERTMESSAGE(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_RENDER, _message, ##__VA_ARGS__)

#define VPHAL_RENDER_NORMALMESSAGE(_message, ...)                                    \
    MOS_NORMALMESSAGE(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_RENDER, _message, ##__VA_ARGS__)

#define VPHAL_RENDER_VERBOSEMESSAGE(_message, ...)                                   \
    MOS_VERBOSEMESSAGE(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_RENDER, _message, ##__VA_ARGS__)

#define VPHAL_RENDER_FUNCTION_ENTER                                                  \
    MOS_FUNCTION_ENTER(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_RENDER)

#define VPHAL_RENDER_EXITMESSAGE(_message, ...)                                      \
    MOS_DEBUGMESSAGE(MOS_MESSAGE_LVL_FUNCTION_EXIT, MOS_COMPONENT_VP, MOS_VP_SUBCOMP_RENDER, _message, ##__VA_ARGS__)

#define VPHAL_RENDER_CHK_STATUS(_stmt)                                               \
    MOS_CHK_STATUS(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_RENDER, _stmt)

#define VPHAL_RENDER_CHK_STATUS_RETURN(_stmt)                                        \
    MOS_CHK_STATUS_RETURN(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_RENDER, _stmt)

#define VPHAL_RENDER_CHK_STATUS_MESSAGE(_stmt, _message, ...)                        \
    MOS_CHK_STATUS_MESSAGE(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_RENDER, _stmt, _message, ##__VA_ARGS__)

#define VPHAL_RENDER_CHK_NULL(_ptr)                                                  \
    MOS_CHK_NULL(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_RENDER, _ptr)

#define VPHAL_RENDER_CHK_NULL_RETURN(_ptr)                                           \
    MOS_CHK_NULL_RETURN(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_RENDER, _ptr)

#define VPHAL_RENDER_CHK_NULL_NO_STATUS(_ptr)                                        \
    MOS_CHK_NULL_NO_STATUS(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_RENDER, _ptr)
//!
//! \def VPHAL_RENDER_CHK_NULL_NO_STATUS_RETURN(_ptr)
//!  MOS_ASSERTMESSAGE \a _ptr with MOS utility comp/subcomp info without returning a status
//!
#define VPHAL_RENDER_CHK_NULL_NO_STATUS_RETURN(_ptr)                                 \
    MOS_CHK_NULL_NO_STATUS_RETURN(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_RENDER, _ptr)

//------------------------------------------------------------------------------
// Macros specific to MOS_VP_SUBCOMP_DDI sub-comp
//------------------------------------------------------------------------------
#define VP_DDI_ASSERT(_expr)                                                         \
    MOS_ASSERT(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_DDI, _expr)

#define VP_DDI_ASSERTMESSAGE(_message, ...)                                          \
    MOS_ASSERTMESSAGE(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_DDI, _message, ##__VA_ARGS__)

#define VP_DDI_NORMALMESSAGE(_message, ...)                                          \
    MOS_NORMALMESSAGE(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_DDI, _message, ##__VA_ARGS__)

#define VP_DDI_VERBOSEMESSAGE(_message, ...)                                         \
    MOS_VERBOSEMESSAGE(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_DDI, _message, ##__VA_ARGS__)

#define VP_DDI_FUNCTION_ENTER                                                        \
    MOS_FUNCTION_ENTER(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_DDI)

#define VP_DDI_CHK_STATUS(_stmt)                                                     \
    MOS_CHK_STATUS(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_DDI, _stmt)

#define VP_DDI_CHK_STATUS_MESSAGE(_stmt, _message, ...)                              \
    MOS_CHK_STATUS_MESSAGE(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_DDI, _stmt, _message, ##__VA_ARGS__)

#define VP_DDI_CHK_NULL(_ptr)                                                        \
    MOS_CHK_NULL(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_DDI, _ptr)

#define VP_DDI_CHK_NULL_NO_STATUS(_ptr)                                              \
    MOS_CHK_NULL_NO_STATUS(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_DDI, _ptr)

#define VPHAL_DDI_CHK_HR(_ptr)                                                       \
    MOS_CHK_HR(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_DDI, _ptr)

#define VPHAL_DDI_CHK_NULL_WITH_HR(_ptr)                                             \
    MOS_CHK_NULL_WITH_HR(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_DDI, _ptr)

//!
//! \brief VPHAL SS/EU setting
//!
struct VphalSseuSetting
{
    uint8_t   numSlices;
    uint8_t   numSubSlices;
    uint8_t   numEUs;
    uint8_t   reserved;       // Place holder for frequency setting
};

//!
//! \brief Gpu context entry
//!
struct VPHAL_GPU_CONTEXT_ENTRY
{
    MOS_GPU_CONTEXT    gpuCtxForMos          = MOS_GPU_CONTEXT_MAX;
    GPU_CONTEXT_HANDLE gpuContextHandle      = MOS_GPU_CONTEXT_INVALID_HANDLE;
    void*              pGpuContext           = nullptr;
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
using VphalSettings =  VpSettings;
#pragma pack(push)
#pragma pack(1)

#pragma pack(pop)


//!
//! Class VphalState
//! \brief VPHAL class definition
//!
class VphalState : public VpBase
{
public:
    // Perf Optimize for ClearVideoView DDI
    bool m_clearVideoViewMode = false;

    // factory function
    static VphalState* VphalStateFactory(
        PMOS_INTERFACE          pOsInterface,
        PMOS_CONTEXT            pOsDriverContext,
        MOS_STATUS              *peStatus);

    //!
    //! \brief    VphalState Constructor
    //! \details  Creates instance of VphalState
    //!           - Caller must call Allocate to allocate all VPHAL states and objects.
    //! \param    [in] pOsInterface
    //!           OS interface, if provided externally - may be nullptr
    //! \param    [in] pOsDriverContext
    //!           OS driver context (UMD context, pShared, ...)
    //! \param    [in,out] peStatus
    //!           Pointer to the MOS_STATUS flag.
    //!           Will assign this flag to MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    VphalState(
        PMOS_INTERFACE          pOsInterface,
        MOS_STATUS              *peStatus);

    //!
    //! \brief    Copy constructor
    //!
    VphalState(const VphalState&) = delete;

    //!
    //! \brief    Copy assignment operator
    //!
    VphalState& operator=(const VphalState&) = delete;

    //!
    //! \brief    VphalState Destuctor
    //! \details  Destroys VPHAL and all internal states and objects
    //!
    virtual ~VphalState();

    //!
    //! \brief    Allocate VPHAL Resources
    //! \details  Allocate VPHAL Resources
    //!           - Allocate and initialize HW states
    //!           - Allocate and initialize renderer states
    //! \param    [in] pVpHalSettings
    //!           Pointer to VPHAL Settings
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS Allocate(
        const VphalSettings *pVpHalSettings) override;

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
        PCVPHAL_RENDER_PARAMS pcRenderParams) override;

    //!
    //! \brief    Get feature reporting from renderer
    //! \details  Get feature reporting from renderer
    //! \return   VphalFeatureReport*
    //!           Pointer to VPHAL_FEATURE_REPOR: rendering features reported
    //!
    virtual VphalFeatureReport*       GetRenderFeatureReport() override;

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
        PQUERY_STATUS_REPORT_APP        pQueryReport,
        uint16_t                 numStatus) override;

    //!
    //! \brief    Get Status Report's entry length from head to tail
    //! \details  Get Status Report's entry length from head to tail
    //! \param    [out] puiLength
    //!           Pointer to the entry length
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS GetStatusReportEntryLength(
        uint32_t                         *puiLength) override;

    virtual PLATFORM &GetPlatform() override
    {
        return m_platform;
    }

    virtual MEDIA_FEATURE_TABLE *GetSkuTable() override
    {
        return m_skuTable;
    }

    MEDIA_WA_TABLE* GetWaTable()
    {
        return m_waTable;
    }

    virtual PMOS_INTERFACE GetOsInterface() override
    {
        return m_osInterface;
    }

    virtual PRENDERHAL_INTERFACE GetRenderHal() override
    {
        return m_renderHal;
    }

    PMHW_VEBOX_INTERFACE GetVeboxInterface()
    {
        return m_veboxInterface;
    }

    MhwCpInterface* GetCpInterface()
    {
        return m_cpInterface;
    }

    PMHW_SFC_INTERFACE GetSfcInterface()
    {
        return m_sfcInterface;
    }

    VphalRenderer* GetRenderer()
    {
        return m_renderer;
    }

    VPHAL_STATUS_TABLE& GetStatusTable()
    {
        return m_statusTable;
    }

    void SetMhwVeboxInterface(MhwVeboxInterface* veboxInterface)
    {
        if (veboxInterface == nullptr)
        {
            return;
        }

        if (m_veboxInterface != nullptr)
        {
            m_veboxItf = std::static_pointer_cast<mhw::vebox::Itf>(m_veboxInterface->GetNewVeboxInterface());
            MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

            if (m_veboxItf)
            {
                eStatus = m_veboxItf->DestroyHeap();
                if (eStatus != MOS_STATUS_SUCCESS)
                {
                    VPHAL_PUBLIC_ASSERTMESSAGE("Failed to destroy Vebox Interface, eStatus:%d.\n", eStatus);
                }
            }

            eStatus = m_veboxInterface->DestroyHeap();
            MOS_Delete(m_veboxInterface);
            m_veboxInterface = nullptr;
            if (eStatus != MOS_STATUS_SUCCESS)
            {
                VPHAL_PUBLIC_ASSERTMESSAGE("Failed to destroy Vebox Interface, eStatus:%d.\n", eStatus);
            }
        }

        m_veboxInterface = veboxInterface;
        m_veboxItf = std::static_pointer_cast<mhw::vebox::Itf>(veboxInterface->GetNewVeboxInterface());
    }

    void SetMhwSfcInterface(MhwSfcInterface* sfcInterface)
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

    virtual MOS_STATUS GetVpMhwInterface(
        VP_MHWINTERFACE &vpMhwinterface);

protected:
    // Internals
    PLATFORM                    m_platform;
    MEDIA_FEATURE_TABLE         *m_skuTable;
    MEDIA_WA_TABLE              *m_waTable;

    // States
    PMOS_INTERFACE              m_osInterface;
    PRENDERHAL_INTERFACE_LEGACY m_renderHal;
    PMHW_VEBOX_INTERFACE        m_veboxInterface;
    MhwCpInterface              *m_cpInterface;
    PMHW_SFC_INTERFACE          m_sfcInterface;
    VphalRenderer               *m_renderer;
    std::shared_ptr<mhw::vebox::Itf> m_veboxItf = nullptr;

    // Render GPU context/node
    MOS_GPU_NODE                m_renderGpuNode;
    MOS_GPU_CONTEXT             m_renderGpuContext;

    // StatusTable indicating if command is done by gpu or not
    VPHAL_STATUS_TABLE          m_statusTable = {};

    MediaUserSettingSharedPtr   m_userSettingPtr = nullptr;  //!< usersettingInstance

    // Same MOS_GPU_CONTEXT may be created in MediaContext with a different handle,
    // which will cause the gpuContext created in VphalState missed to be destroyed during
    // m_osInterface being destroyed. m_gpuContextCheckList is used to store gpu contexts
    // which may encounter such case.
    std::vector<VPHAL_GPU_CONTEXT_ENTRY> m_gpuContextCheckList;

    //!
    //! \brief    Create instance of VphalRenderer
    //! \details  Create instance of VphalRenderer
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS CreateRenderer() = 0;

    virtual bool IsApoEnabled() override
    {
        return false;
    }

    virtual bool IsRenderContextBasedSchedulingNeeded()
    {
        return false;
    }

private:
    //!
    //! \brief    Put GPU context entry
    //! \details  Put GPU context entry in the m_gpuContextCheckList
    //! \param    MOS_GPU_CONTEXT mosGpuConext
    //!           [in] Mos GPU context
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    MOS_STATUS AddGpuContextToCheckList(
        MOS_GPU_CONTEXT mosGpuConext);

    //!
    //! \brief    Destroy GPU context entry with invalid handle
    //! \details  Release these GPU context overwritten by MediaContext
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    MOS_STATUS DestroyGpuContextWithInvalidHandle();

    //!
    //! \brief    Check whether GPU context is reused or not
    //! \details  Check whether GPU context is reused or not
    //! \param    MOS_GPU_CONTEXT mosGpuConext
    //!           [in] Mos GPU context
    //! \return   bool
    //!           Return true if is reused, otherwise false
    //!
    bool IsGpuContextReused(
        MOS_GPU_CONTEXT mosGpuContext);
};

#endif  // __VPHAL_H__
