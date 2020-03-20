/*
* Copyright (c) 2016-2019, Intel Corporation
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
//! \file     vphal_render_renderstate.h
//! \brief    VPHAL RenderState class definition
//! \details  RenderState class defines the interface of render components
//!
#ifndef __VPHAL_RENDER_RENDERSTATE_H__
#define __VPHAL_RENDER_RENDERSTATE_H__

#include "vphal.h"
#include "vphal_render_common.h"
#include "hal_kerneldll.h"

//!
//! class RenderpassData
//! \brief transient data used in single render pass
//!
class RenderpassData
{
public:
    bool                    bCompNeeded;
    bool                    bHdrNeeded;

    uint32_t                uiSrcIndex;
    PVPHAL_SURFACE          pSrcSurface;                                        // pSrcSurface points to the original input surface
                                                                                // or the output from previous render
    PVPHAL_SURFACE          pOutSurface;                                        // pOutSurface points to the output surface
    bool                    bOutputGenerated;
    PVPHAL_SURFACE          pOriginalSrcSurface;                                // Pointer to original source surface that is not adv proc'd
    uint32_t                uiPrimaryIndex;
    PVPHAL_SURFACE          pPrimarySurface;                                    // Null of no primary passed by app.

    static const uint32_t   TempSurfaceAmount = 2;
    bool                    bSFCScalingOnly = false;                            // whehter use SFC replace AVS do scaling.

    RenderpassData() :
        bCompNeeded(false),
        bHdrNeeded(false),
        uiSrcIndex(0),
        pSrcSurface(nullptr),
        pOutSurface(nullptr),
        bOutputGenerated(false),
        pOriginalSrcSurface(nullptr),
        uiPrimaryIndex(0),
        pPrimarySurface(nullptr),
        uiOutSurfaceIndex(0),
        TempOutputSurfaces()
    {
    }

    virtual ~RenderpassData()
    {
        for (uint32_t i = 0; i < TempSurfaceAmount; i++)
        {
            MOS_FreeMemAndSetNull(TempOutputSurfaces[i]);
        }
    }

    virtual MOS_STATUS AllocateTempOutputSurfaces()
    {
        for (uint32_t i = 0 ; i < TempSurfaceAmount; i++ )
        {
            // only allocate if it is null
            if (TempOutputSurfaces[i] == nullptr)
            {
                TempOutputSurfaces[i] = (PVPHAL_SURFACE)MOS_AllocAndZeroMemory(sizeof(VPHAL_SURFACE));

                // if allocation failed
                if(TempOutputSurfaces[i] == nullptr)
                {
                    // free all allocated surfaces
                    while (i > 0)
                    {
                        MOS_FreeMemAndSetNull(TempOutputSurfaces[--i]);
                    }
                    return MOS_STATUS_NO_SPACE;
                }
            }
        }
        return MOS_STATUS_SUCCESS;
    }

    PVPHAL_SURFACE GetTempOutputSurface()
    {
        return TempOutputSurfaces[uiOutSurfaceIndex];
    }

    void MoveToNextTempOutputSurface()
    {
        uiOutSurfaceIndex++;
        uiOutSurfaceIndex %= TempSurfaceAmount;
    }

protected:
    uint32_t                uiOutSurfaceIndex;
    PVPHAL_SURFACE          TempOutputSurfaces[TempSurfaceAmount];

};

//!
//! Class RenderState
//! \brief abstract the interface of VP render components
//!
class RenderState
{
public:
    //!
    //! \brief    RenderState Constructor
    //! \details  Construct RenderState and allocate member data structure
    //! \param    [in] pOsInterface
    //!           Pointer to MOS interface structure
    //! \param    [in] pRenderHal
    //!           Pointer to RenderHal interface structure
    //! \param    [in] pPerfData
    //!           Pointer to performance data structure
    //! \param    [out] peStatus
    //!           Pointer to MOS status
    //!
    RenderState(
        PMOS_INTERFACE              pOsInterface,
        PRENDERHAL_INTERFACE        pRenderHal,
        PVPHAL_RNDR_PERF_DATA       pPerfData,
        MOS_STATUS                  *peStatus);

    //!
    //! \brief    Copy constructor
    //!
    RenderState(const RenderState&) = delete;

    //!
    //! \brief    Copy assignment operator
    //!
    RenderState& operator=(const RenderState&) = delete;

    //!
    //! \brief    RenderState Destructor
    //! \details  Destroy RenderState and release all related RenderState resources
    //!
    virtual ~RenderState() {
        MOS_Delete(m_reporting);
    };

    //!
    //! \brief    Initialize RenderState
    //! \param    [in] pSettings
    //!           Pointer to VPHAL Settings
    //! \param    [in] pKernelDllState
    //!           Pointer to KernelDLL State
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful
    //!
    virtual MOS_STATUS Initialize(
        const VphalSettings    *pSettings,
        Kdll_State             *pKernelDllState) = 0;

    //!
    //! \brief    RenderState Destroy
    //! \details  Destroy resource allocated by Render 
    //!
    virtual void Destroy()
    {
    };

    //!
    //! \brief    RenderState Rendering
    //! \details  VPHal RenderState entry
    //! \param    [in] pcRenderParams
    //!           Pointer to Render parameters
    //! \param    [in,out] pRenderPassData
    //!           Pointer to Render data
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS Render(
        PCVPHAL_RENDER_PARAMS  pcRenderParams,
        RenderpassData         *pRenderPassData) = 0;

    //!
    //! \brief    Judge if render is needed
    //! \details  Check Render parameter/data if render needed
    //! \param    [in] pcRenderParams
    //!           Pointer to Render parameters
    //! \param    [in,out] pRenderPassData
    //!           Pointer to Render data
    //! \return   bool
    //!           true if meeded. Else false
    //!
    virtual bool IsNeeded(
        PCVPHAL_RENDER_PARAMS  pcRenderParams,
        RenderpassData         *pRenderPassData) = 0;

    //!
    //! \brief    Judge if render support multiple stream rendering
    //! \details  Judge if render support multiple stream rendering
    //! \return   bool
    //!           true if supported. Else false
    //!
    virtual bool IsMultipleStreamSupported() = 0;

    //!
    //! \brief    Set Slice Shutdown Mode
    //! \param    [in] bSingleSlice
    //!           value of Slice Shutdown Mode
    //! \return   void
    //!
    void SetSingleSliceMode(
        bool                    bSingleSlice)
    {
        m_bSingleSlice = bSingleSlice;
    }

    //!
    //! \brief    get Performace data
    //! \details  get Performace data used by this render
    //! \return   PVPHAL_RNDR_PERF_DATA
    //!           Performace data used by this render
    //!
    PVPHAL_RNDR_PERF_DATA GetPerfData()
    {
        return m_pPerfData;
    }

    //!
    //! \brief    copy Report data
    //! \details  copy Report data from this render
    //! \param    [out] pReporting 
    //!           pointer to the Report data to copy data to
    //!
    virtual void CopyReporting(VphalFeatureReport* pReporting)
    {
    }

    //!
    //! \brief    get Sku table
    //! \details  get Sku table from this render
    //! \return   MEDIA_FEATURE_TABLE *
    //!           Sku table pointer
    //!
    MEDIA_FEATURE_TABLE *GetSkuTable()
    {
        return m_pSkuTable;
    }

    //!
    //! \brief    get RenderHal interface pointer
    //! \details  get RenderHal interface pointer from this render
    //! \return   PRENDERHAL_INTERFACE
    //!           RenderHal pointer
    //!
    PRENDERHAL_INTERFACE GetRenderHalInterface()
    {
        return m_pRenderHal;
    }

    //!
    //! \brief    get OS interface pointer
    //! \details  get OS interface pointer from this render
    //! \return   PRENDERHAL_INTERFACE
    //!           RenderHal pointer
    //!
    PMOS_INTERFACE GetOsInterface()
    {
        return m_pOsInterface;
    }

    //!
    //! \brief    Set status report parameters
    //! \param    [in] pRenderer
    //!           pointer to the renderer
    //! \param    [in] pRenderParams
    //!           pointer to the render parameters
    //!
    void SetStatusReportParams(
        VphalRenderer           *pRenderer,
        PVPHAL_RENDER_PARAMS    pRenderParams);

    //!
    //! \brief    Get render disable flag
    //! \details  Get render disable flag
    //! \return   bool
    //!           return true if Render is disabled,
    //!           return false otherwise.
    //!
    bool GetRenderDisableFlag()
    {
        return m_bDisableRender;
    };

    //!
    //! \brief    Set render disable flag
    //! \param    [in] bDisable
    //!           The diable flag. If parameter is true, it will disable render.
    //!
    void SetRenderDisableFlag(bool bDisable)
    {
        m_bDisableRender = bDisable;
    };

    // External components
    PMOS_INTERFACE              m_pOsInterface;
    PRENDERHAL_INTERFACE        m_pRenderHal;

protected:
    // External tables
    MEDIA_FEATURE_TABLE         *m_pSkuTable;
    MEDIA_WA_TABLE              *m_pWaTable;

    // Disable Render flag
    bool                        m_bDisableRender;

    // Slice Shutdown flag
    bool                        m_bSingleSlice;

    // Performance Related item
    PVPHAL_RNDR_PERF_DATA       m_pPerfData;

    // Feature reporting
    VphalFeatureReport          *m_reporting;

    // Status Buffer, Video Pre-Processing Only
    STATUS_TABLE_UPDATE_PARAMS  m_StatusTableUpdateParams = { 0 };

};

#endif // __VPHAL_RENDER_RENDERSTATE_H__
