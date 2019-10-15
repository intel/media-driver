/*
* Copyright (c) 2009-2019, Intel Corporation
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
//! \file     vphal_renderer.h
//! \brief    The header file of VPHAL top level rendering component
//! \details  The top renderer is responsible for coordinating the sequence of calls to low level renderers, e.g. DNDI or Comp
//!
#ifndef __VPHAL_RENDERER_H__
#define __VPHAL_RENDERER_H__

#include "mos_os.h"
#include "vphal.h"
#include "vphal_render_common.h"
#include "vphal_render_renderstate.h"
#include "vphal_render_vebox_base.h"
#include "vphal_render_16alignment.h"
#include "vphal_render_fast1ton.h"
#include "vphal_debug.h"
#include "vphal_render_hdr_base.h"

#define VPHAL_RNDR_TEMP_OUT_SURFS            2

// Slice Shutdown Modes
#define VPHAL_SSD_DEFAULT 0
#define VPHAL_SSD_ENABLE  1
#define VPHAL_SSD_DISABLE 2

//!
//! \brief MACRO to dump surface for the given layer
//!
#define VPHAL_RNDR_DUMP_SURF(pRenderer, uiLayer, Location, pSurface)            \
    VPHAL_DBG_SURFACE_DUMP(                                                     \
    pRenderer->m_surfaceDumper,                                                \
    pSurface,                                                                   \
    pRenderer->uiFrameCounter,                                                  \
    uiLayer,                                                                    \
    Location)

//!
//! \brief MACRO to dump all layers
//!
#define VPHAL_RNDR_DUMP_SURF_PTR_ARRAY(pRenderer, pSurf, surfMax, surfCount, Location)      \
    VPHAL_DBG_SURFACE_PTRS_DUMP(                                                \
    pRenderer->m_surfaceDumper,                                                \
    pSurf,                                                                      \
    surfMax,                                                                    \
    surfCount,                                                                  \
    pRenderer->uiFrameCounter,                                                  \
    Location)

//!
//! \brief MACRO to set status report params
//!
#if (_DEBUG || _RELEASE_INTERNAL)
#define VPHAL_RNDR_SET_STATUS_REPORT_PARAMS(pState, pRenderer, pRenderParams) do {                \
    if (pState)                                                                                   \
    {                                                                                             \
        bool bSurfIsRenderTarget = (pRenderParams->pTarget[0]->SurfType == SURF_OUT_RENDERTARGET);\
        (pState)->StatusTableUpdateParams.bReportStatus       = (pRenderParams->bReportStatus);   \
        (pState)->StatusTableUpdateParams.bSurfIsRenderTarget = bSurfIsRenderTarget;              \
        (pState)->StatusTableUpdateParams.pStatusTable        = pRenderer->m_statusTable;          \
        (pState)->StatusTableUpdateParams.StatusFeedBackID    = pRenderParams->StatusFeedBackID;  \
        (pState)->StatusTableUpdateParams.bTriggerGPUHang     = pRenderParams->bTriggerGPUHang;   \
    }                                                                                             \
} while(0)
#else
#define VPHAL_RNDR_SET_STATUS_REPORT_PARAMS(pState, pRenderer, pRenderParams) do {                \
    if (pState)                                                                                   \
    {                                                                                             \
        bool bSurfIsRenderTarget = (pRenderParams->pTarget[0]->SurfType == SURF_OUT_RENDERTARGET);\
        (pState)->StatusTableUpdateParams.bReportStatus       = (pRenderParams->bReportStatus);   \
        (pState)->StatusTableUpdateParams.bSurfIsRenderTarget = bSurfIsRenderTarget;              \
        (pState)->StatusTableUpdateParams.pStatusTable        = pRenderer->m_statusTable;         \
        (pState)->StatusTableUpdateParams.StatusFeedBackID    = pRenderParams->StatusFeedBackID;  \
    }                                                                                             \
} while(0)
#endif

//!
//! \brief Render ID enum
//!
enum VPHAL_RENDER_ID
{
    VPHAL_RENDER_ID_VEBOX = 0,
    VPHAL_RENDER_ID_VEBOX2,
    VPHAL_RENDER_ID_COMPOSITE,
    VPHAL_RENDER_ID_COUNT                   //!< Keep this line at the end
};
C_ASSERT(VPHAL_RENDER_ID_COUNT == 3);      //!< When adding, update assert

//!
//! \brief VPHAL renderer class
//!
class VphalRenderer
{
public:
    // 16 Bytes Alignment state
    VPHAL_16_ALIGN_STATE        Align16State;
    // Fast 1toN state
    VPHAL_FAST1TON_STATE        Fast1toNState;
    // Rendering engines
    VPHAL_VEBOX_EXEC_STATE      VeboxExecState[VPHAL_MAX_CHANNELS];             //!< Vebox Execution State

    RenderState                 *pRender[VPHAL_RENDER_ID_COUNT];

    // VpHal surfaces
    PVPHAL_SURFACE              pPrimaryFwdRef[VPHAL_MAX_FUTURE_FRAMES];

    bool                        bVeboxUsedForCapPipe;                           //!< VEBOX used for CapPipe

    // Stereo state.
    uint32_t                    uiCurrentChannel;                               //!< 0=StereoLeft or nonStereo, 1=StereoRight. N/A in nonStereo

    // Compositing Kernel DLL/Search state
    const Kdll_RuleEntry        *pKernelDllRules;
    Kdll_State                  *pKernelDllState;

    // Compositing Kernel buffer and size
    const void                  *pcKernelBin;
    uint32_t                    dwKernelBinSize;

    // CM Compositing Kernel patch file buffer and size
    const void                  *pcFcPatchBin;
    uint32_t                    dwFcPatchBinSize;

    // Surface dumper fields (counter and specification)
    uint32_t                    uiFrameCounter;
#if (_DEBUG || _RELEASE_INTERNAL)
    VphalSurfaceDumper          *m_surfaceDumper;
    VphalParameterDumper        *m_parameterDumper;
#endif

    // StatusTable indicating if command is done by gpu or not
    PVPHAL_STATUS_TABLE          m_statusTable;

    // max src rectangle
    RECT                        maxSrcRect;

    // Intermediate surface, currently two usages:
    // 1) It is for viedo surveillance usage, when applying AVS for multiple surfaces;
    // 2) It could be VEBOX output or input for HDR processing;
    VPHAL_SURFACE               IntermediateSurface = {};
    PVPHAL_HDR_STATE            pHdrState;

protected:
    // Renderer private data
    PRENDERHAL_INTERFACE        m_pRenderHal;
    PMOS_INTERFACE              m_pOsInterface;

    // Auxiliary
    MEDIA_FEATURE_TABLE           *m_pSkuTable;

    void (*m_modifyKdllFunctionPointers)(PKdll_State);

    uint32_t                    uiSsdControl;                                   //!< Slice Shutdown Control - read from User feature keys
    bool                        bDpRotationUsed;                                //!< Dataport-based rotation Used Flag
    bool                        bSkuDisableVpFor4K;                             //!< Disable VP features for 4K
    bool                        bSkuDisableLaceFor4K;                           //!< Disable LACE for 4K
    bool                        bSkuDisableDNFor4K;                             //!< Disable DN for 4K

    // VDI performance data
    VPHAL_RNDR_PERF_DATA        PerfData;

    // Renderer feature reporting
    VphalFeatureReport          *m_reporting;
public:
    //!
    //! \brief    VphalRenderer constructor
    //! \details  Based on the HW and OS info, initialize the renderer interfaces
    //! \param    [in] pRenderHal
    //!           Pointer to RenderHal Interface Structure
    //! \param    [in,out] pStatus
    //!           Pointer to the MOS_STATUS flag.
    //!           Will assign this flag to MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    VphalRenderer(
        PRENDERHAL_INTERFACE                pRenderHal,
        MOS_STATUS                          *pStatus);

    //!
    //! \brief    Copy constructor
    //!
    VphalRenderer(const VphalRenderer&) = delete;

    //!
    //! \brief    Copy assignment operator
    //!
    VphalRenderer& operator=(const VphalRenderer&) = delete;

    //!
    //! \brief    VPHAL renderer destructor
    //! \details  Destory the resources allocated for the renderers
    //!           including VEBOX and Composite.
    //!
    virtual ~VphalRenderer();

    //!
    //! \brief    Get Renderer Cache Settings
    //! \details  Get cache settings for various VP features
    //! \param    [in] pOsInterface
    //!           OS Interface
    //! \param    [in] pPlatform
    //!           Platform Pointer
    //! \param    [in] pSkuTable
    //!           SKU feature table
    //! \param    [in,out] pSettings
    //!           Pointer to Render Cache Control Settings
    //! \return   void
    //!
    virtual void GetCacheCntl(
        PMOS_INTERFACE                      pOsInterface,
        PLATFORM                            *pPlatform,
        MEDIA_FEATURE_TABLE                 *pSkuTable,
        PVPHAL_RENDER_CACHE_CNTL            pSettings) = 0;

    //!
    //! \brief    Allocate render components
    //! \param    [in] pVeboxInterface
    //!           Pointer to Vebox Interface Structure
    //! \param    [in] pSfcInterface
    //!           Pointer to SFC interface Structure
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS AllocateRenderComponents(
        PMHW_VEBOX_INTERFACE                pVeboxInterface,
        PMHW_SFC_INTERFACE                  pSfcInterface) = 0;

    //!
    //! \brief    Initialize the VPHAL renderer
    //! \details  Initialize all the renderers supported including VEBOX, Composite.
    //! \param    [in] pSettings
    //!           Const pointer to VPHAL settings
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS Initialize(
        const VphalSettings                 *pSettings);

    //!
    //! \brief    Main render function
    //! \details  The top level renderer function, which may contain multiple
    //!           passes of rendering
    //! \param    [in] pcRenderParams
    //!           Const pointer to VPHAL render parameter
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS Render(
        PCVPHAL_RENDER_PARAMS   pcRenderParams);

    VphalFeatureReport* GetReport() {
        return m_reporting;
    }

    PMOS_INTERFACE GetOsInterface() {
        return m_pOsInterface;
    }

    MEDIA_FEATURE_TABLE* GetSkuTable() {
        return m_pSkuTable;
    }

    void SetStatusReportTable(PVPHAL_STATUS_TABLE statusTable)
    {
        m_statusTable = statusTable;
    }

    //!
    //! \brief    Initialize the KDLL parameters
    //! \details  Initialize the KDLL parameters
    //! \return   MOS_STATUS
    //!
    virtual MOS_STATUS InitKdllParam() = 0;

    //!
    //! \brief    Update Render Gpu Context
    //! \details  Update Render Gpu Context
    //! \param    [in] renderGpuContext
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS UpdateRenderGpuContext(MOS_GPU_CONTEXT renderGpuContext);

protected:
    //!
    //! \brief    Prepare input surface list for top level render processing
    //! \details  Prepare the inputs, e.g. adjust src/dst rectangles of stereo input or allocate
    //!           and copy intermediate surfaces.
    //! \param    [in,out] pRenderParams
    //!           Pointer to VPHAL render parameter
    //! \param    [in,out] pSrcLeft
    //!           Pointer to left frame list
    //! \param    [in,out] pSrcRight
    //!           Pointer to right frame list
    //! \param    [out] puiRenderPasses
    //!           Pointer to times of the rendering.
    //!                 The value is 2 for S3D and 1 for the other cases.
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS PrepareSources(
        PVPHAL_RENDER_PARAMS    pRenderParams,
        PVPHAL_SURFACE          *pSrcLeft,
        PVPHAL_SURFACE          *pSrcRight,
        uint32_t                *puiRenderPasses);

    //!
    //! \brief    Render function for the pass
    //! \details  The render function coordinates the advanced renderers and basic
    //!           renders in one pass
    //! \param    [in,out] pRenderParams
    //!           Pointer to VPHAL render parameter
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS RenderPass(
        PVPHAL_RENDER_PARAMS    pRenderParams);

    //!
    //! \brief    Release intermediate surfaces
    //! \details  Release intermediate surfaces created for main render function
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS FreeIntermediateSurfaces();

    //!
    //! \brief    Process render parameter
    //! \param    [in,out] pRenderParams
    //!           Pointer to VPHAL render parameter
    //! \param    [in,out] pRenderPassData
    //!           Pointer to the VPHAL render pass data
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    MOS_STATUS ProcessRenderParameter(
        PVPHAL_RENDER_PARAMS    pRenderParams,
        RenderpassData          *pRenderPassData);

    //!
    //! \brief    Adjust surface parameter
    //! \param    [in] pRenderParams
    //!           Pointer to VPHAL render parameter
    //! \param    [in,out] pSrcSurface
    //!           Pointer to VPHAL surface
    //! \param    [in] pGtSystemInfo
    //!           Pointer to GT system information structure
    //! \param    [in] bHybridDecoderFlag
    //!           Hybrid Decoder or not
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    MOS_STATUS AdjustSurfaceParam(
        PVPHAL_RENDER_PARAMS    pRenderParams,
        PVPHAL_SURFACE          pSrcSurface,
        MEDIA_SYSTEM_INFO       *pGtSystemInfo,
        bool                    bHybridDecoderFlag);

    //!
    //! \brief    Render single stream
    //! \param    [in] pRenderParams
    //!           Pointer to VPHAL render parameter
    //! \param    [in,out] pRenderPassData
    //!           Pointer to the VPHAL render pass data
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS RenderSingleStream(
        PVPHAL_RENDER_PARAMS    pRenderParams,
        RenderpassData          *pRenderPassData);

    //!
    //! \brief    Compose input streams as fast 1toN
    //! \details  Use composite render to multi output streams
    //! \param    [in] pRenderParams
    //!           Pointer to VPHAL render parameter
    //! \param    [in,out] pRenderPassData
    //!           Pointer to the VPHAL render pass data
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    MOS_STATUS RenderFast1toNComposite(
        PVPHAL_RENDER_PARAMS    pRenderParams,
        RenderpassData          *pRenderPassData);

    //!
    //! \brief    Compose input streams
    //! \details  Use composite render to compose input streams
    //! \param    [in] pRenderParams
    //!           Pointer to VPHAL render parameter
    //! \param    [in,out] pRenderPassData
    //!           Pointer to the VPHAL render pass data
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    MOS_STATUS RenderComposite(
        PVPHAL_RENDER_PARAMS    pRenderParams,
        RenderpassData          *pRenderPassData);

    //!
    //! \brief    Get surface info for all input source
    //! \details  Get surface info for the input surface and its reference surfaces
    //! \param    [in] pRenderParams
    //!           Pointer to VPHAL render parameter
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    MOS_STATUS GetSurfaceInfoForSrc(
        PVPHAL_RENDER_PARAMS    pRenderParams);

    //!
    //! \brief    Check if Vphal renderer support some formats
    //! \param    [in] pcRenderParams
    //!           Const pointer to VPHAL render parameter
    //! \return   bool
    //!           Return true if successful, false failed
    //!
    bool IsFormatSupported(
        PCVPHAL_RENDER_PARAMS   pcRenderParams);

    //!
    //! \brief    Update report data
    //! \details  Update report data from each feature render
    //! \param    [in] pRenderParams
    //!           Pointer to VPHAL render parameter
    //! \param    [in,out] pRenderPassData
    //!           Pointer to the VPHAL render pass data
    //!
    void UpdateReport(
        PVPHAL_RENDER_PARAMS    pRenderParams,
        RenderpassData          *pRenderPassData);

    //!
    //! \brief    Allocate debug dumper
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS AllocateDebugDumper();

    //!
    //! \brief    Get Hdr path needed flag
    //! \details  Get Hdr path needed flag
    //! \param    pRenderParams
    //!           [in] Pointer to VPHAL render parameter
    //! \param    pRenderPassData
    //!           [in,out] Pointer to the VPHAL render pass data
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS GetHdrPathNeededFlag(
        PVPHAL_RENDER_PARAMS    pRenderParams,
        RenderpassData          *pRenderPassData);
};

#define VPHAL_RENDERER_GET_CACHE_CNTL(obj, pOsInterface, pPlatform, pSkuTable, pSettings)   \
                obj->GetCacheCntl(pOsInterface, pPlatform, pSkuTable, pSettings)

//!
//! \brief    Initialize AVS parameters shared by Renderers
//! \details  Initialize the members of the AVS parameter and allocate memory for its coefficient tables
//! \param    [in,out] pAVS_Params
//!           Pointer to VPHAL AVS parameter
//! \param    [in] uiYCoeffTableSize
//!           Size of the Y coefficient table
//! \param    [in] uiUVCoeffTableSize
//!           Size of the UV coefficient table
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VpHal_RenderInitAVSParams(
    PMHW_AVS_PARAMS             pAVS_Params,
    uint32_t                    uiYCoeffTableSize,
    uint32_t                    uiUVCoeffTableSize);

//!
//! \brief    Destroy AVS parameters shared by Renderers
//! \details  Free the memory of AVS parameter's coefficient tables
//! \param    [in,out] pAVS_Params
//!           Pointer to VPHAL AVS parameter
//! \return   void
//!
void VpHal_RenderDestroyAVSParams(
    PMHW_AVS_PARAMS             pAVS_Params);

//!
//! \brief    Search from existing BBs for a match. If none, allocate new BB
//! \details  Based on the params of the BB, search the BB table and try to get
//!           the best match. If none, try to get an old unused BB to reuse. If
//!           still none, allocate one new BB
//! \param    [in] pBatchBufferTable
//!           Pointer to the BB table to be searched
//! \param    [in] pInputBbParams
//!           Pointer to the BB params required for the BB needed
//! \param    [in] iBbSize
//!           The BB size required for the BB needed
//! \param    [in] pRenderHal
//!           Pointer to RenderHal Interface Structure
//! \param    [out] ppBatchBuffer
//!           Pointer to the addr of the available BB. Point to nullptr if there's no
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VpHal_RenderAllocateBB(
    PVPHAL_BATCH_BUFFER_TABLE   pBatchBufferTable,
    PVPHAL_BATCH_BUFFER_PARAMS  pInputBbParams,
    int32_t                     iBbSize,
    PRENDERHAL_INTERFACE        pRenderHal,
    PMHW_BATCH_BUFFER           *ppBatchBuffer);

//!
//! \brief    Update max src rect in VphalRenderer and primary surface based
//!           on src rectangle info from primary video
//! \details  Add max src rect for consistent statistics surface layout. Update
//!           the max src rect of the surface and its reference surfaces
//! \param    [in,out] pRenderer
//!           VPHAL renderer pointer
//! \param    [in,out] pSurface
//!           Pointer to the surface
//! \return   void
//!
void VpHal_RenderInitMaxRect(
    VphalRenderer               *pRenderer,
    PVPHAL_SURFACE              pSurface);

//!
//! \brief    Get the size in byte from that in pixel
//! \details  Size_in_byte = size_in_pixel x byte/pixel
//! \param    [in] pOsInterface
//!           Pointer to OS interface
//! \param    [in] Format
//!           The format which determines the value of byte/pixel
//! \param    [in] dwPixels
//!           The size in pixel
//! \return   uint32_t
//!           Return the size in byte
//!
uint32_t VpHal_PixelsToBytes(
    PMOS_INTERFACE      pOsInterface,
    MOS_FORMAT          Format,
    uint32_t            dwPixels);

//!
//! \brief    Save/Restore fwd references for the primary
//! \details  Based on the flag passed in to save or restore the forward references
//!           of the primary
//! \param    [in,out] pRenderer
//!           VPHAL renderer pointer
//! \param    [in,out] pPrimarySurf
//!           Pointer to the primary surface
//! \param    [in] bSave
//!           Save - true or restore - false the fwd references
//! \return   void
//!
void VpHal_SaveRestorePrimaryFwdRefs(
    VphalRenderer           *pRenderer,
    PVPHAL_SURFACE          pPrimarySurf,
    bool                    bSave);

//!
//! \brief    Align the src/dst surface rectangle and surface width/height
//! \details  The surface rects and width/height need to be aligned according to the surface format
//! \param    [in,out] pSurface
//!           Pointer to the surface
//! \param    [in] formatForDstRect
//!           Format for Dst Rect
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VpHal_RndrRectSurfaceAlignment(
    PVPHAL_SURFACE       pSurface,
    MOS_FORMAT           formatForDstRect);

//!
//! \brief    Search for the best match BB according to the render BB arguments
//! \details  Based on the params of the BB, search the BB table and try to get
//!           the best match
//! \param    [in] pBatchBufferTable
//!           Point to the BB table to be searched
//! \param    [in] pInputBbParams
//!           Point to the BB params required for the BB needed
//! \param    [in] iBbSize
//!           The BB size required for the BB needed
//! \param    [out] ppBatchBuffer
//!           Point to the addr of the best matched BB. Point to nullptr if there's no.
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VpHal_RenderGetBestMatchBB(
    PVPHAL_BATCH_BUFFER_TABLE       pBatchBufferTable,
    PVPHAL_BATCH_BUFFER_PARAMS      pInputBbParams,
    int32_t                         iBbSize,
    PMHW_BATCH_BUFFER               *ppBatchBuffer);
#endif // __VPHAL_RENDER_H__
