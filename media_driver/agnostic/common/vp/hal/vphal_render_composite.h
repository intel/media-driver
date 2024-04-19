/*
* Copyright (c) 2016-2021, Intel Corporation
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
//! \file     vphal_render_composite.h
//! \brief    Common interface and structure used in Composite
//! \details  Common interface and structure used in Composite
//!
#ifndef __VPHAL_RENDER_COMPOSITE_H__
#define __VPHAL_RENDER_COMPOSITE_H__

#include "vphal.h"
#include "vphal_render_renderstate.h"
#include "vphal_render_common.h"
#include "mhw_render_legacy.h"

//!
//! \brief Compositing buffers
//!
#define VPHAL_COMP_BUFFERS_DEFAULT  4
#define VPHAL_COMP_BUFFERS_MAX      32

#define VPHAL_MAX_PROCAMP           2

#define VPHAL_SURFACE_INDEX_INVALID -1
#define VPHAL_COMP_MAX_PALETTES     2
#define VPHAL_COMP_MAX_LUMA_KEY     1
#define VPHAL_COMP_MAX_AVS          1
#define VPHAL_COMP_MAX_PROCAMP      1
#define VPHAL_COMP_SAMPLER_NEAREST  1
#define VPHAL_COMP_SAMPLER_BILINEAR 2
#define VPHAL_COMP_SAMPLER_LUMAKEY  4
#define VPHAL_COMP_MAX_SAMPLER      (VPHAL_COMP_SAMPLER_NEAREST | VPHAL_COMP_SAMPLER_BILINEAR | VPHAL_COMP_SAMPLER_LUMAKEY)

#define VPHAL_SAMPLER_Y                 1
#define VPHAL_SAMPLER_U                 2
#define VPHAL_SAMPLER_V                 3
#define VPHAL_COMP_COMPUTE_WALKER_THREAD_SPACE_WIDTH    1
#define VPHAL_COMP_COMPUTE_WALKER_THREAD_SPACE_HEIGHT   1
#define VPHAL_COMP_COMPUTE_WALKER_THREAD_SPACE_DEPTH    1

// GRF 8 for unified kernel inline data (NLAS is enabled)
struct MEDIA_OBJECT_NLAS_INLINE_DATA
{
    // DWORD 0 - GRF R6.0
    union
    {
        struct
        {
            float   HorizontalFrameOriginLayer0;
        };

        uint32_t       Value;
    } DW00;

    // DWORD 1 - GRF R6.1
    union
    {
        struct
        {
            float   HorizontalFrameOriginLayer1;
        };

        uint32_t       Value;
    } DW01;

    // DWORD 2 - GRF R6.2
    union
    {
        struct
        {
            float   HorizontalFrameOriginLayer2;
        };

        uint32_t       Value;
    } DW02;

    // DWORD 03 - GRF R6.3
    union
    {
        struct
        {
            float   HorizontalFrameOriginLayer3;
        };

        uint32_t       Value;
    } DW03;

    // DWORD 04 - GRF R6.4
    union
    {
        struct
        {
            float   HorizontalFrameOriginLayer4;
        };

        uint32_t       Value;
    } DW04;

    // DWORD 05 - GRF R6.5
    union
    {
        struct
        {
            float   HorizontalFrameOriginLayer5;
        };

        uint32_t       Value;
    } DW05;

    // DWORD 06 - GRF R6.6
    union
    {
        struct
        {
            float   HorizontalFrameOriginLayer6;
        };

        uint32_t       Value;
    } DW06;

    // DWORD 07 - GRF R6.7
    union
    {
        struct
        {
            float   HorizontalFrameOriginLayer7;
        };

        uint32_t       Value;
    } DW07;
};

//!
//! \brief Structure to VPHAL Composite Media Object Inline Data
//!
struct VPHAL_COMPOSITE_MO_INLINE_DATA
{
    // Do not change the two members' order because of low level memcpy
    MEDIA_OBJECT_NLAS_INLINE_DATA       NLASInline;
    MEDIA_OBJECT_KA2_INLINE_DATA        KA2Inline;
};

//!
//! \brief Structure to VPHAL Composite Parameters
//!
typedef struct _VPHAL_COMPOSITE_PARAMS
{
    // Pointer to target and source surfaces
    uint32_t                uSourceCount;                       //!< Number of sources
    PVPHAL_SURFACE          pSource[VPHAL_COMP_MAX_LAYERS];
    uint32_t                uTargetCount;                       //!< Number of targets
    VPHAL_SURFACE           Target[VPHAL_MAX_TARGETS];          //!< Render targets
    // Needed by CP during MHW VP integration, due to pTokenState->pResourceInfo
    RENDERHAL_SURFACE       RenderHalSurfaceSrc[VPHAL_COMP_MAX_LAYERS];
    RENDERHAL_SURFACE       RenderHalSurfaceSrcField[VPHAL_COMP_MAX_LAYERS];
    RENDERHAL_SURFACE       RenderHalSurfaceTarget[VPHAL_MAX_TARGETS];

    bool                    bSkipBlocks;          //!< Skip empty blocks
    PRECT                   pConstriction;        //!< Constricted output
    PVPHAL_COLORFILL_PARAMS pColorFillParams;     //!< ColorFill - BG only
    PVPHAL_ALPHA_PARAMS     pCompAlpha;           //!< Alpha for composited surface
    bool                    bAlphaCalculateEnable;
    bool                    bForceSkipColorFill;  //!< Force skip colorfill even the first layer is translucent

    // Resource counters
    int32_t                 nLayers;
    int32_t                 nPalettes;
    int32_t                 nAVS;
    int32_t                 nProcamp;
    int32_t                 nLumaKeys;
    int32_t                 nSampler;
    VPHAL_ROTATION          Rotation;           //!< Layer 0 rotation info
} VPHAL_COMPOSITE_PARAMS, *PVPHAL_COMPOSITE_PARAMS;

//!
//! \brief Structure to VPHAL Composite Rendering data
//!
typedef struct _VPHAL_RENDERING_DATA_COMPOSITE
{
    // Sources
    int32_t                             iLayers;
    PVPHAL_SURFACE                      pLayers[VPHAL_COMP_MAX_LAYERS];
    PVPHAL_SURFACE                      pTarget[VPHAL_MAX_TARGETS];
    PVPHAL_COLORFILL_PARAMS             pColorFill;
    PVPHAL_ALPHA_PARAMS                 pCompAlpha;

    // Geometry
    int32_t                             iBlocksX;
    int32_t                             iBlocksY;
    int32_t                             iBindingTable;
    int32_t                             iMediaID;
    int32_t                             iCurbeOffset;
    int32_t                             iCurbeLength;
    RECT                                rcOutput;

    // Constriction parameters
    PRECT                               pConstriction;
    int32_t                             ConstrictionOriginX;
    int32_t                             ConstrictionOriginY;
    float                               fConstrictionStepX;
    float                               fConstrictionStepY;

    // HDC Direct Write flag
    bool                                bHdcDwEnable;

    // Alpha Calculate flag
    bool                                bAlphaCalculateEnable;

    // CM FC flag
    bool                                bCmFcEnable;

    // States
    PRENDERHAL_MEDIA_STATE              pMediaState;
    MHW_SAMPLER_STATE_PARAM             SamplerStateParams[MHW_RENDER_ENGINE_SAMPLERS_MAX];
    int32_t                             iCmdInlineSize;
    int32_t                             iNLASInlineSize;
    MEDIA_OBJECT_KA2_STATIC_DATA        Static;
    MEDIA_WALKER_KA2_STATIC_DATA        WalkerStatic;
    MEDIA_OBJECT_KA2_INLINE_DATA        Inline;
    PMHW_AVS_PARAMS                     pAvsParams;
    MEDIA_DP_FC_STATIC_DATA             DPFCStatic;
    // Batch Buffer rendering arguments
    VPHAL_BB_COMP_ARGS                  BbArgs;

    // Kernel Information
    Kdll_CacheEntry                     *pKernelEntry;
    Kdll_Procamp                        *pProcamp;

    // Extension data
    void*                               pExtensionData;

} VPHAL_RENDERING_DATA_COMPOSITE, *PVPHAL_RENDERING_DATA_COMPOSITE;

//!
//! \brief Structure to VPHAL 16x16 Block COMPOSITE Mask
//!
typedef struct _VPHAL_16X16BLOCK_COMPOSITE_MASK
{
    uint32_t       HorizontalBlockCompositeMask    : 16;
    uint32_t       VerticalBlockCompositeMask      : 16;
} VPHAL_16X16BLOCK_COMPOSITE_MASK, *PVPHAL_16X16BLOCK_COMPOSITE_MASK;

//!
//! \brief Class to VPHAL Composite render
//!
class CompositeState : public RenderState
{
public:
    //!
    //! \brief    Composite Constructor
    //! \details  Construct Composite render and allocate member data structure
    //! \param    [in] pOsInterface
    //!           Pointer to MOS interface structure
    //! \param    [in] pRenderHal
    //!           Pointer to RenderHal interface structure
    //! \param    [in] pPerfData
    //!           Pointer to performance data structure
    //! \param    [in] compositeCacheCntl
    //!           Composite Cache Control Data
    //! \param    [out] peStatus
    //!           Pointer to MOS status
    //!
    CompositeState(
        PMOS_INTERFACE                      pOsInterface,
        PRENDERHAL_INTERFACE                pRenderHal,
        PVPHAL_RNDR_PERF_DATA               pPerfData,
        const VPHAL_COMPOSITE_CACHE_CNTL    &compositeCacheCntl,
        MOS_STATUS                          *peStatus);

    //!
    //! \brief    Composite render Destructor
    //! \details  Destroy Composite render and release all related RenderState resources
    //!
    virtual ~CompositeState();

    //!
    //! \brief    Initialize Composite render
    //! \param    [in] pSettings
    //!           Pointer to VPHAL Settings
    //! \param    [in] pKernelDllState
    //!           Pointer to KernelDLL State
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful
    //!
    virtual MOS_STATUS Initialize(
        const VphalSettings    *pSettings,
        Kdll_State             *pKernelDllState);

    //!
    //! \brief    Composite Destroy function
    //! \details  Destroy resource allocated by Composite
    //!
    virtual void Destroy();

    //!
    //! \brief    Composite render Rendering
    //! \details  VPHal Composite render entry
    //! \param    [in] pcRenderParams
    //!           Pointer to Render parameters
    //! \param    [in,out] pRenderPassData
    //!           Pointer to Render data
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    MOS_STATUS Render(
        PCVPHAL_RENDER_PARAMS  pcRenderParams,
        RenderpassData         *pRenderPassData);

    //!
    //! \brief    Judge if Composite render is needed
    //! \details  Check Render parameter/data if Composite render needed
    //! \param    [in] pcRenderParams
    //!           Pointer to Render parameters
    //! \param    [in,out] pRenderPassData
    //!           Pointer to Render data
    //! \return   bool
    //!           true if meeded. Else false
    //!
    bool IsNeeded(
        PCVPHAL_RENDER_PARAMS  pcRenderParams,
        RenderpassData         *pRenderPassData);

    void PrintCurbeData(MEDIA_OBJECT_KA2_STATIC_DATA *pWalkerStatic);

    void PrintWalkerParas(PMHW_GPGPU_WALKER_PARAMS pWalkerParams);

    void PrintSamplerParams(PMHW_SAMPLER_STATE_PARAM pSamplerParams);

    //!
    //! \brief    set Report data
    //! \details  set Report data for this render
    //! \param    [in] pSource 
    //!           pointer to the surface
    //!
    virtual void SetReporting(PVPHAL_SURFACE pSource);

    //!
    //! \brief    copy Report data
    //! \details  copy Report data from this render
    //! \param    [out] pReporting 
    //!           pointer to the Report data to copy data to
    //!
    virtual void CopyReporting(VphalFeatureReport* pReporting);

    //!
    //! \brief    Judge if Composite render support multiple stream rendering
    //! \details  Judge if Composite render support multiple stream rendering
    //! \return   bool
    //!           true if supported. Else false
    //!
    bool IsMultipleStreamSupported();

    //!
    //! \brief    Search for the best match BB according to the Composition BB arguments
    //! \param    [in] pBatchBufferTable
    //!           Pointer to the BB table to be searched
    //! \param    [in] pInputBbParams
    //!           Pointer to the BB params required for the best match
    //! \param    [in] iBbSize
    //!           the BB size required for the best match
    //! \param    [out] ppBatchBuffer
    //!           Pointer to the addr of the best matched BB, pointer to nullptr if there's
    //!           no available matched BB
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    static MOS_STATUS GetBestMatchBB(
        PVPHAL_BATCH_BUFFER_TABLE     pBatchBufferTable,
        PVPHAL_BATCH_BUFFER_PARAMS    pInputBbParams,
        int32_t                       iBbSize,
        PMHW_BATCH_BUFFER             *ppBatchBuffer);

    //!
    //! \brief    Load Palette Data
    //! \details  Load Palette Data according to color space and CSC matrix.
    //! \param    [in] pInPalette
    //!           Pointer to Input Palette structure
    //! \param    [in] srcCspace
    //!           Source color space
    //! \param    [in] dstCspace
    //!           Destination color space
    //! \param    [in] piCscMatrix
    //!           Pointer to CSC matrix to use in fixed point format
    //! \param    [in] iNumEntries
    //!           Number of Palette entries to be filled
    //! \param    [in,out] pPaletteData
    //!           Pointer to Output Palette Address
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS, otherwise MOS_STATUS_UNIMPLEMENTED if Destination Colorspace not supported,
    //!            or MOS_STATUS_INVALID_PARAMETER/MOS_STATUS_NULL_POINTER
    //!
    MOS_STATUS LoadPaletteData(
        PVPHAL_PALETTE          pInPalette,
        VPHAL_CSPACE            srcCspace,
        VPHAL_CSPACE            dstCspace,
        int32_t*                piCscMatrix,
        int32_t                 iNumEntries,
        void*                   pPaletteData);

protected:
    //!
    //! \brief    Set Sampler Avs 8x8 Table
    //! \param    [in] pRenderHal
    //!           Pointer to RenderHal Interface Structure
    //! \param    [in] pSamplerStateParams
    //!           Pointer to Sampler State Params
    //! \param    [in,out] pAvsParams
    //!           Pointer to AVS Params
    //! \param    [in] SrcFormat
    //!           Source Format
    //! \param    [in] fScaleX
    //!           Horizontal Scale Factor
    //! \param    [in] fScaleY
    //!           Vertical Scale Factor
    //! \param    [in] dwChromaSiting
    //!           Chroma Siting
    //! \return   MOS_STATUS
    //!
    virtual MOS_STATUS SetSamplerAvsTableParam(
        PRENDERHAL_INTERFACE            pRenderHal,
        PMHW_SAMPLER_STATE_PARAM        pSamplerStateParams,
        PMHW_AVS_PARAMS                 pAvsParams,
        MOS_FORMAT                      SrcFormat,
        float                           fScaleX,
        float                           fScaleY,
        uint32_t                        dwChromaSiting);

    //!
    //! \brief    Get Plane Offset override parameter for Kernel WA
    //! \details  Get Y/UV Plane Offset override parameters for Kernel WA
    //! \param    pRenderHalSurface
    //!           [in,out] Pointer to Render Hal Surface
    //! \param    pParams
    //!           [in] Pointer to Surface State Params
    //! \param    pOverride
    //!           [out] Pointer to override param that provides adjustments to
    //!                 Y, UV plane offsets, used for kernel WA in a few cases.
    //! \return   RENDERHAL_OFFSET_OVERRIDE
    //!           return pointer to RENDERHAL_OFFSET_OVERRIDE if need, otherwise return nullptr.
    //!
    virtual PRENDERHAL_OFFSET_OVERRIDE GetPlaneOffsetOverrideParam(
        PRENDERHAL_SURFACE              pRenderHalSurface,
        PRENDERHAL_SURFACE_STATE_PARAMS pParams,
        PRENDERHAL_OFFSET_OVERRIDE      pOverride)
    {
        return nullptr;
    }

    //!
    //! \brief    Get Thread Count for VFE state parameter
    //! \details  Get Thread Count for VFE state parameter
    //! \param    pRenderingData
    //!           [in] Pointer to Composite state
    //! \param    pTarget
    //!           [in] Pointer to target surface
    //! \return   INT
    //!           return the thread count
    //!
    virtual int32_t GetThreadCountForVfeState(
        PVPHAL_RENDERING_DATA_COMPOSITE     pRenderingData,
        PVPHAL_SURFACE                      pTarget);

    //!
    //! \brief    Calculate Composite parameter and render data
    //! \param    [in,out] pCompParams
    //!           Pointer to Composite parameters.
    //! \param    [in,out] pSource
    //!           Pointer to surface.
    //! \param    [in,out] pRenderingData
    //!           Pointer to Composite RenderData.
    //! \param    [out] pbColorfill
    //!           Pointer to color fill flag.
    //! \return   void
    //!
    virtual void CalculateRenderData(
        PVPHAL_COMPOSITE_PARAMS         pCompParams,
        PVPHAL_SURFACE                  pSurface,
        PVPHAL_RENDERING_DATA_COMPOSITE pRenderingData,
        bool*                           pbColorfill);

    //!
    //! \brief    Adjust Params Based On Fc Limit
    //! \param    [in,out] PCVPHAL_RENDER_PARAMS
    //!           Pointer to pcRenderParam parameters.
    //! \return   bool
    //!
    bool AdjustParamsBasedOnFcLimit(
        PCVPHAL_RENDER_PARAMS pcRenderParam);

    //!
    //! \brief    Set Sampler AVS parameters
    //! \param    [in] pRenderingData
    //!           pointer to render data
    //! \param    [in] pSource
    //!           pointer to source surface
    //! \param    [in] pSurfaceEntry
    //!           pointer to source state entry
    //! \param    [out] pSamplerStateParams
    //!           pointer to Sampler state params
    //! \param    [in] fScaleX
    //!           width scaling ratio
    //! \param    [in] fScaleY
    //!           height scaling ratio
    //! \return   MOS_STATUS
    //!
    virtual MOS_STATUS SetSamplerAvsParams(
        PVPHAL_RENDERING_DATA_COMPOSITE pRenderingData,
        PVPHAL_SURFACE                  pSource,
        PRENDERHAL_SURFACE_STATE_ENTRY  pSurfaceEntry,
        PMHW_SAMPLER_STATE_PARAM        pSamplerStateParams,
        float                           fScaleX,
        float                           fScaleY);

    //!
    //! \brief    set inline data
    //! \param    [in] pBbArgs
    //!           Pointer to Composite BB argument
    //! \param    [in] pRenderingData
    //!           Pointer to Composite render data
    //! \param    [in] pStatic
    //!           Pointer to static data
    //! \param    [in,out] pInline
    //!           Pointer to inline data
    //! \param    [in,out] pInlineNLAS
    //!           Pointer to NLAS inline data
    //! \param    [in] x
    //!           horizontal origin
    //! \param    [out] fSrcX
    //!           horizontal origin of layers
    //! \return   void
    //!
    virtual void ModifyInlineData(
        PVPHAL_BB_COMP_ARGS                 pBbArgs,
        PVPHAL_RENDERING_DATA_COMPOSITE     pRenderingData,
        MEDIA_OBJECT_KA2_STATIC_DATA        *pStatic,
        MEDIA_OBJECT_KA2_INLINE_DATA        *pInline,
        MEDIA_OBJECT_NLAS_INLINE_DATA       *pInlineNLAS,
        int32_t                             x,
        float                               *fSrcX) {;}

    //!
    //! \brief    Calculate crop factor
    //! \param    [in] iLayer
    //!           layer index
    //! \param    [in] pRenderingData
    //!           pointer to render data
    //! \param    [out] pfCropX
    //!           crop factor
    //! \param    [out] pfCropY
    //!           crop factor
    //! \return   MOS_STATUS
    //!
    virtual MOS_STATUS CalculateCropParams(
        int32_t                         iLayer,
        PVPHAL_RENDERING_DATA_COMPOSITE pRenderingData,
        float*                          pfCropX,
        float*                          pfCropY);

    //!
    //! \brief    Calculate and set inline data size
    //! \param    [in] pRenderingData
    //!           pointer to render data
    //! \param    [out] pStatic
    //!           pointer to static data
    //! \return   void
    //!
    virtual int32_t CalculateInlineDataSize(
        PVPHAL_RENDERING_DATA_COMPOSITE pRenderingData,
        MEDIA_OBJECT_KA2_STATIC_DATA    *pStatic);

    //!
    //! \brief    Calculate Media Object size
    //! \param    [in] pRenderingData
    //!           Pointer to Rendering Data
    //! \return   int32_t
    //!           Return the size of Media Object
    //!
    virtual int32_t CalculateMediaObjectSize(
        PVPHAL_RENDERING_DATA_COMPOSITE     pRenderingData);

    //!
    //! \brief    Modify MediaWalker Static Data
    //! \param    [in] pRenderingData
    //!           Pointer to Rendering Data
    //! \return   void
    //!
    virtual void ModifyMediaWalkerStaticData(
        PVPHAL_RENDERING_DATA_COMPOSITE pRenderingData);

    //!
    //! \brief    Initialize Composite Rendering data
    //! \details  Initialize Composite Rendering data, set output area, number of blocks,
    //!           Sources, constriction parameters, rendering states, etc.
    //! \param    [in] pCompParams
    //!           Pointer to Composite parameters
    //! \param    [out] pRenderingData
    //!           Pointer to Composite Rendering data
    //! \return   MOS_STATUS
    //!
    virtual MOS_STATUS RenderInit(
        PVPHAL_COMPOSITE_PARAMS         pCompParams,
        PVPHAL_RENDERING_DATA_COMPOSITE pRenderingData);

    //!
    //! \brief    Release Composite Rendering data
    //! \param    [in] pRenderingData
    //!           Pointer to Composite Rendering data
    //! \return   MOS_STATUS
    //!
    virtual void CleanRenderingData(
        PVPHAL_RENDERING_DATA_COMPOSITE pRenderingData);

    //!
    //! \brief    Fill in Gen specific static data
    //! \details  Fill in Gen specific static data
    //! \param    pRenderingData
    //!           [in] Pointer to REnder Data
    //! \param    pTarget
    //!           [in] Pointer to Target Surface
    //! \param    pStatic
    //!           [in,out] Pointer to Static Data
    //! \return   void
    //!
    virtual void SubmitStatesFillGenSpecificStaticData(
        PVPHAL_RENDERING_DATA_COMPOSITE     pRenderingData,
        PVPHAL_SURFACE                      pTarget,
        MEDIA_OBJECT_KA2_STATIC_DATA        *pStatic) = 0;

    //!
    //! \brief    Get Output Surface Chroma sitting position for kernel
    //! \details  Get Output Surface Chroma sitting position for kernel
    //! \param    pTarget
    //!           [in] Pointer to Target Surface
    //! \return   DWORD
    //!           Return chroma sitting position
    //!
    uint32_t GetOutputChromaSitting(
        PVPHAL_SURFACE                      pTarget);

    //!
    //! \brief    Set Surface Compressed Parameters
    //! \details  Set Surface Compressed Parameters, and compression mode
    //! \param    [in,out] pSource
    //!           Pointer to Source Surface
    //! \param    [in] isRenderTarget
    //!           Render Target or not
    //! \return   void
    //!
    virtual void SetSurfaceCompressionParams(
        PVPHAL_SURFACE                  pSource,
        bool                            isRenderTarget);

    //!
    //! \brief    Check NV12 luma key sampler solution is needed or not
    //! \details  This func is needed for Gen9 platforms
    //! \param    pSrc
    //!           [in] Pointer to Source Surface
    //! \param    pRenderHal
    //!           [in] Pointer to render hal
    //! \return   bool
    //!           Return TRUE if needed, otherwise FALSE
    //!
    virtual bool IsNV12SamplerLumakeyNeeded(PVPHAL_SURFACE pSrc, PRENDERHAL_INTERFACE pRenderHal)
    {
        return false;
    }

    //!
    //! \brief    Check whether parameters for composition valid or not.
    //! \param    [in] CompositeParams
    //!           Parameters for composition
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS IsCompositeParamsValid(
        const VPHAL_COMPOSITE_PARAMS& CompositeParams);

    //!
    //! \brief    Get Sampler Index associated with a surface state for composite
    //! \param    [in] pSurface
    //!           point to input Surface
    //! \param    [in] pEntry
    //!           Pointer to Surface state
    //! \param    [out] pSamplerIndex
    //!           Pointer to Sampler Index
    //! \param    [out] pSamplerType
    //!           Pointer to Sampler Type
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise MOS_STATUS_UNKNOWN
    //!
    virtual MOS_STATUS GetSamplerIndex(
        PVPHAL_SURFACE                      pSurface,
        PRENDERHAL_SURFACE_STATE_ENTRY      pEntry,
        int32_t*                            pSamplerIndex,
        PMHW_SAMPLER_TYPE                   pSamplerType);

    //!
    //! \brief    Update SamplerStateParams associated with a surface state for composite
    //! \param    [in] pSamplerStateParams
    //!           Pointer to SamplerStateParams
    //! \param    [in] pEntry
    //!           Pointer to Surface state
    //! \param    [in] pRenderData
    //!           Pointer to RenderData
    //! \param    [in] uLayerNum
    //!           Layer total number
    //! \param    [in] SamplerFilterMode
    //!           SamplerFilterMode to be set
    //! \param    [out] pSamplerIndex
    //!           Pointer to Sampler Index
    //! \param    [out] pSurface
    //!           point to Surface
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise MOS_STATUS_UNKNOWN
    //!
    virtual MOS_STATUS SetSamplerFilterMode(
        PMHW_SAMPLER_STATE_PARAM       &pSamplerStateParams,
        PRENDERHAL_SURFACE_STATE_ENTRY  pEntry,
        PVPHAL_RENDERING_DATA_COMPOSITE pRenderData,
        uint32_t                        uLayerNum,
        MHW_SAMPLER_FILTER_MODE         SamplerFilterMode,
        int32_t                        *pSamplerIndex,
        PVPHAL_SURFACE                  pSource);

    //!
    //! \brief    Check whether the 3Dsampler use for Y plane
    //! \param    [in] SamplerID
    //!           sampler ID
    //! \return   bool
    //!           Return true if the 3Dsampler use for Y plane, otherwise fase
    //!
    virtual bool IsSamplerIDForY(
        int32_t                            SamplerID);

    //! \brief    set Sampler status
    //! \param    [in] pSurface
    //!           point to input Surface
    //! \param    [in] Layer
    //!           composition layer
    //! \param    [in] pStatic
    //!           Pointer to static data
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise MOS_STATUS_UNKNOWN
    //!
    virtual MOS_STATUS Set3DSamplerStatus(
        PVPHAL_SURFACE                 pSurface,
        uint8_t                        Layer,
        MEDIA_OBJECT_KA2_STATIC_DATA   *pStatic)
    {
        // Just need the sub class to implement if it is needed;
        return MOS_STATUS_SUCCESS;
    }

    //! \brief    Update Inline Data status
    //! \param    [in] pSurface
    //!           point to input Surface
    //! \param    [in] pStatic
    //!           Pointer to static data
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise MOS_STATUS_UNKNOWN
    //!
    virtual MOS_STATUS UpdateInlineDataStatus(
        PVPHAL_SURFACE                 pSurface,
        MEDIA_OBJECT_KA2_STATIC_DATA   *pStatic)
    {
        // Just need the sub class to implement if it is needed;
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS IntermediateAllocation(PVPHAL_SURFACE &pIntermediate,
        PMOS_INTERFACE                               pOsInterface,
        uint32_t                                     dwTempWidth,
        uint32_t                                     dwTempHeight,
        PVPHAL_SURFACE                               pTarget);

    //!
    //! \brief    Prepare phases for composite and allocate intermediate buffer for rendering
    //! \param    [in] pcRenderParams
    //!           Pointer to Render parameters
    //! \param    [in] ppSources
    //!           Pointer to the address of Source Surfaces
    //! \param    [in] iSources
    //!           Count of Source Surfaces
    //! \return   bool
    //!           Return true if multiple phases, otherwise false
    //!
    virtual bool PreparePhases(
        PCVPHAL_RENDER_PARAMS       pcRenderParams,
        PVPHAL_SURFACE              *ppSources,
        int32_t                     iSources);

    //!
    //! \brief    Reset composite rendering parameters for the current phase
    //! \param    [in,out] pComposite
    //!           Pointer to Composite parameters
    //! \return   void
    //!
    void ResetCompParams(
        PVPHAL_COMPOSITE_PARAMS     pComposite);

    //!
    //! \brief    Adds a source layer for composite
    //! \param    [in,out] pComposite
    //!           Pointer to Composite parameters
    //! \param    [in] pSource
    //!           Pointer to Source Surface
    //! \return   bool
    //!           Return TURE if source may be processed in the same phase, otherwise false
    //!
    bool AddCompLayer(
        PVPHAL_COMPOSITE_PARAMS     pComposite,
        PVPHAL_SURFACE              pSource,
        bool                        bDisableAvsSampler);

    //!
    //! \brief    Adds render target layer for composite
    //! \param    [in,out] pComposite
    //!           Pointer to Composite parameters
    //! \param    [in] pTarget
    //!           Pointer to target surface
    //! \return   bool
    //!           Return TURE if target may be processed in the same phase, otherwise false
    //!
    bool AddCompTarget(
        PVPHAL_COMPOSITE_PARAMS     pComposite,
        PVPHAL_SURFACE              pTarget);

    //!
    //! \brief    set sclaing Ratio
    //! \details  set sclaing Ratio for kernels which need to use different kernel to process scaling.
    //!           parameters
    //! \param    [in,out] Kdll_Scalingratio
    //!           Pointer to scaling ratio
    //! \return   void
    virtual void SetFilterScalingRatio(
        Kdll_Scalingratio*    ScalingRatio) {}

    //!
    //! \brief    Render Compute Walker Buffer
    //! \details  Render Compute Walker Buffer, fill Walker static data fields and set walker
    //!           cmd params
    //! \param    [in] pBatchBuffer
    //!           Pointer to BatchBuffer
    //! \param    [in] pRenderingData
    //!           Pointer to Rendering Data
    //! \param    [in] pWalkerParams
    //!           Pointer to Walker parameters
    //! \return   bool
    //!           Return true if successful, otherwise false
    //!
    virtual bool RenderBufferComputeWalker(
        PMHW_BATCH_BUFFER               pBatchBuffer,
        PVPHAL_RENDERING_DATA_COMPOSITE pRenderingData,
        PMHW_GPGPU_WALKER_PARAMS        pWalkerParams);

    //!
    //! \brief    Submit Composite states
    //! \details  Submit Composite states, including load CSC matrix, set NLAS Inline data,
    //!           set background color, load Palettes, set output format, load kernel, load
    //!           curbe data, set sampler state, set VFE State params, and etc
    //! \param    [in] pRenderingData
    //!           Pointer to Composite state
    //! \return   bool
    //!           Return TURE if successful, otherwise false
    //!
    virtual bool SubmitStates(
        PVPHAL_RENDERING_DATA_COMPOSITE     pRenderingData);

    virtual bool IsDisableAVSSampler(
        int32_t         iSources,
        bool            isTargetY);

    //!
    //! \brief    Decompress the Surface
    //! \details  Decompress the interlaced Surface which is in the RC compression mode
    //! \param    [in,out] pSource
    //!           Pointer to Source Surface
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS DecompressInterlacedSurf(PVPHAL_SURFACE pSource);

private:
    //!
    //! \brief    Prepare phases for composite and determine intermediate colorspace
    //! \param    [in] pcRenderParams
    //!           Pointer to Render parameters
    //! \param    [in] ppSources
    //!           Pointer to the address of Source Surfaces
    //! \param    [in] iSources
    //!           Count of Source Surfaces
    //! \return   VPHAL_CSPACE
    //!           Return intermediate colorspace
    //!
    VPHAL_CSPACE PrepareCSC(
        PCVPHAL_RENDER_PARAMS   pcRenderParams,
        PVPHAL_SURFACE          *ppSources,
        int32_t                 iSources);

    //!
    //! \brief    Composite multiple phase rendering
    //! \details  Composite render with multiple phases. In some cases we cannot process composition just in one phase
    //!           for example, if the input streams count is 9 (1 primary + 8 substreams), we need to postpone the
    //!           9th stream to next second phase due to the input count limitation of current composition kernel.
    //! \param    [in] pcRenderParams
    //!           Pointer to VPHAL_RENDER_PARAMS
    //! \param    [in] ppSources
    //!           Pointer to PVPHAL_SURFACE, array of input surfaces
    //! \param    [in] iSources
    //!           constant int iSource indicating the size of ppSources
    //! \param    [in] pOutput
    //!           Pointer to VPHAL_SURFACE, output surface for the overall composition process
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    MOS_STATUS RenderMultiPhase(
        PCVPHAL_RENDER_PARAMS   pcRenderParams,
        PVPHAL_SURFACE          *ppSources,
        const int32_t           iSources,
        PVPHAL_SURFACE          pOutput);

    //!
    //! \brief    Perform multiple layer composite operation in one phase
    //! \details  Perform multiple layer composite operation in one phase(scaling, blending,
    //!           lumakey, CSC)
    //! \param    [in,out] pCompParams
    //!           Pointer to Composite parameters
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    MOS_STATUS RenderPhase(
        PVPHAL_COMPOSITE_PARAMS pCompParams);

    //!
    //! \brief    Set Composite Scaling mode
    //! \param    [in,out] pSource
    //!           Pointer to Source Surface
    //! \param    [in] uSourceCount
    //!           Count of Source Surfaces
    //! \return   void
    //!
    void SetScalingMode(
        PVPHAL_SURFACE          pSource,
        uint32_t                uSourceCount);

    //!
    //! \brief    Judge whether Bob Di should be enabled
    //! \details  Judge whether Bob Di should be enabled according to the parameter
    //!           of pDeinterlaceParams and the height of the input surface
    //! \param    [in] pSrc
    //!           Pointer to Source Surface
    //! \return   bool
    //!           Return true if Bob DI should be enabled, otherwise false
    //!
    virtual bool IsBobDiEnabled(PVPHAL_SURFACE pSrc);

    //!
    //! \brief    Set Composite Layer
    //! \details  Set Composite Layer, including setup surface state and binding table, setup
    //!           lumakey parameters, setup samplers, setup alpha blending parameters, adjust
    //!           geometry for BOB DI, normalize source co-ordinates, set curbe and inline
    //!           data, and etc
    //! \param    [in] pRenderingData
    //!           Pointer to Composite Rendering data
    //! \param    [in] pSource
    //!           Pointer to Source Surface
    //! \param    [in] iLayerIdInCompParams
    //!           Layer id in pCompParams for pSource
    //! \param    [in,out] pCompParams
    //!           Pointer to Composite parameters
    //! \return   int32_t
    //!           Return 1 if set layer successful, otherwise -1
    //!
    int32_t SetLayer(
        PVPHAL_RENDERING_DATA_COMPOSITE pRenderingData,
        PVPHAL_SURFACE                  pSource,
        int                             iLayerIdInCompParams,
        PVPHAL_COMPOSITE_PARAMS         pCompParams);

    //!
    //! \brief    Set Composite Render Target Layer
    //! \details  Set Composite Render Target Layer, setup surface state and binding table
    //! \param    [in] pRenderingData
    //!           Pointer to Composite Rendering data
    //! \param    [in] pCompParams
    //!           Pointer to Composite parameters
    //! \return   int32_t
    //!           Return number of Surface State entries if successful, otherwise -1
    //!
    int32_t SetLayerRT(
        PVPHAL_RENDERING_DATA_COMPOSITE pRenderingData,
        PVPHAL_COMPOSITE_PARAMS         pCompParams);

    //!
    //! \brief    Build filter description for dynamic linking
    //! \details  Build filter description(render method, current layer, layer format, layer
    //!           rotation, layer colorspace, sampling mode, scaling mode, luma key, blending,
    //!           colorfill, procamp, CSC) for dynamic linking
    //!           parameters
    //! \param    [in] pCompParams
    //!           Pointer to Composite parameters
    //! \param    [out] pFilter
    //!           Pointer to first filter entry
    //! \param    [out] piFilterSize
    //!           Pointer to filter size
    //! \return   bool
    //!           Return true if successful, otherwise false
    //!
    bool BuildFilter(
        PVPHAL_COMPOSITE_PARAMS         pCompParams,
        PKdll_FilterEntry               pFilter,
        int32_t*                        piFilterSize);

    //!
    //! \brief    Render Composite BatchBuffer
    //! \details  Render Composite BatchBuffer, fill Walker static data fields and set walker
    //!           cmd params
    //! \param    [in] pBatchBuffer
    //!           Pointer to BatchBuffer
    //! \param    [in] pRenderingData
    //!           Pointer to Rendering Data
    //! \param    [in] pWalkerParams
    //!           Pointer to Walker parameters
    //! \return   bool
    //!           Return true if successful, otherwise false
    //!
    bool RenderBufferMediaWalker(
        PMHW_BATCH_BUFFER               pBatchBuffer,
        PVPHAL_RENDERING_DATA_COMPOSITE pRenderingData,
        PMHW_WALKER_PARAMS              pWalkerParams);

    //!
    //! \brief    Judge whether  media walker pattern  will be vertical or not
    //! \details  if input layer is one , and input is linear format and rotation 90
    //!           or 270 is needed then the media walker pattern should be vertical
    //! \param    [in] pRenderingData
    //!           Pointer to Rendering Data
    //! \return   bool
    //!           Return true if vertical media pattern used, otherwise false
    //!
    bool MediaWalkerVertical(
        PVPHAL_RENDERING_DATA_COMPOSITE pRenderingData);

    //!
    //! \brief    Set Surface Parameters
    //! \details  Set Surface Parameters, set flags for RT, set surface type based on scaling
    //!           mode, set interlacing flags, etc.
    //! \param    [in,out] pSource
    //!           Pointer to Source Surface
    //! \param    [out] pSurfaceParams
    //!           Pointer to Surface Parameters
    //! \return   void
    //!
    void SetSurfaceParams(
        PVPHAL_SURFACE                  pSource,
        PRENDERHAL_SURFACE_STATE_PARAMS pSurfaceParams);

    //!
    //! \brief    Allocate Composite BatchBuffer
    //! \details  Allocate Composite BatchBuffer, search from existing BBs for a match. If
    //!           none, allocate new BB
    //! \param    [in] pRenderingData
    //!           Pointer to Rendering Data
    //! \param    [out] ppBatchBuffer
    //!           Pointer to the addr of the available BB. Pointer to nullptr if there's no
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    MOS_STATUS AllocateBuffer(
        PVPHAL_RENDERING_DATA_COMPOSITE     pRenderingData,
        PMHW_BATCH_BUFFER                   *ppBatchBuffer);

    //!
    //! \brief    Render Composite BatchBuffer
    //! \details  Render Composite BatchBuffer, setup Media Object header and inline data
    //! \param    [in] pBatchBuffer
    //!           Pointer to BatchBuffer
    //! \param    [in] pRenderingData
    //!           Pointer to Rendering Data
    //! \return   bool
    //!           Return true if successful, otherwise false
    //!
    bool RenderBuffer(
        PMHW_BATCH_BUFFER               pBatchBuffer,
        PVPHAL_RENDERING_DATA_COMPOSITE pRenderingData);

    //!
    //! \brief    Judge whether 8-tap adaptive filter for all channels should be enabled
    //! \details  Judge whether 8-tap adaptive filter for all channels should be enabled according to the input parameter
    //! \param    [in] pSrc
    //!           Pointer to Source Surface
    //! \param    [in] fScaleX
    //!           width scaling ratio
    //! \param    [in] fScaleY
    //!           height scaling ratio
    //! \return   bool
    //!           Return true 8-tap adaptive filter for all channels should be enabled, otherwise false
    //!
    bool Is8TapAdaptiveEnabled(
        PVPHAL_SURFACE          pSrc,
        float                   fScaleX,
        float                   fScaleY);

    //!
    //! \brief    Initialize Colorfill parameters
    //! \details  Initialize Colorfill parameters
    //! \return   void
    //!
    void InitColorFillParams();

    //!
    //! \brief    Check if sample unorm being used for source surface.
    //! \param    [in] pCompParams
    //!           Pointer to Composite parameters
    //! \param    pSrc
    //!           [in] Pointer to Source Surface
    //! \return   bool
    //!           Return TRUE if use sample unorm, otherwise FALSE
    //!
    bool IsUsingSampleUnorm(
        PVPHAL_COMPOSITE_PARAMS         pCompParams,
        PVPHAL_SURFACE                  pSrc);

    //!
    //! \brief    Check if sampler lumakey being supported or not for source surface.
    //! \param    pSrc
    //!           [in] Pointer to Source Surface
    //! \return   bool
    //!           Return TRUE if support, otherwise FALSE
    //!
    bool IsSamplerLumakeySupported(PVPHAL_SURFACE pSrc);

    //!
    //! \brief    Get intermediate surface output
    //! \param    pOutput
    //!           [in] Pointer to Intermediate Output Surface
    //! \return   PVPHAL_SURFACE
    //!           Return the chose output
    //!
    virtual MOS_STATUS GetIntermediateOutput(PVPHAL_SURFACE &output);

    virtual PVPHAL_SURFACE GetIntermediateSurface();
    virtual PVPHAL_SURFACE GetIntermediate1Surface();
    virtual PVPHAL_SURFACE GetIntermediate2Surface();
    // Procamp
    int32_t                         m_iMaxProcampEntries;
    int32_t                         m_iProcampVersion;
    Kdll_Procamp                    m_Procamp[VPHAL_MAX_PROCAMP];

    // Cache attributes
    VPHAL_COMPOSITE_CACHE_CNTL      m_SurfMemObjCtl;

    bool                            m_bNullHwRenderComp;      //!< Null rendering flag for Composite function

    bool                            m_b8TapAdaptiveEnable;    //!< 8 tap adaptive filter enable flag, read from user feature key

    Kdll_FilterDesc                 m_SearchFilter;
    Kdll_SearchState                m_KernelSearch;
    int32_t                         m_ThreadCountPrimary;

    // CMFC CSC Coefficient surface
    VPHAL_SURFACE                   m_CmfcCoeff;
    RENDERHAL_SURFACE               m_RenderHalCmfcCoeff;

    // Batch buffers
    int32_t                         m_iBatchBufferCount;
    MHW_BATCH_BUFFER                m_BatchBuffer[VPHAL_COMP_BUFFERS_MAX];
    VPHAL_BATCH_BUFFER_PARAMS       m_BufferParam[VPHAL_COMP_BUFFERS_MAX];

    // Multiple phase support
    int32_t                         m_iCallID;

    bool                            m_bLastPhase;                 //!< Flag for indicating the last Comp render phase

protected:

     // Background Color fill parameters
    struct
    {
        VPHAL_COLOR_SAMPLE_8        m_csSrc;
        VPHAL_COLOR_SAMPLE_8        m_csDst;
        VPHAL_CSPACE                m_CSpaceSrc;
        VPHAL_CSPACE                m_CSpaceDst;
    };

    // Feature flags
    float                           m_fSamplerLinearBiasX;        //!< Linear sampler bias X
    float                           m_fSamplerLinearBiasY;        //!< Linear sampler bias Y
    bool                            m_bFtrMediaWalker;            //!< Media Object Walker enabled
    bool                            m_bFtrComputeWalker;          //!< Compute Walker enabled
    bool                            m_bFtrCSCCoeffPatchMode;      //!< Set CSC Coeff using patch mode
    bool                            m_bSamplerSupportRotation;    //!< Use sampler for Rotation
    bool                            m_bChromaUpSampling;          //!< Chroma Up Sampling needed
    bool                            m_bChromaDownSampling;        //!< Chroma Down Sampling needed
    bool                            m_bFallbackIefPatch;          //!< Fall back IEF path from AVS to SFC
    bool                            m_bKernelSupportDualOutput;   //!< Kernel support Dual Output
    bool                            m_bKernelSupportHdcDW;        //!< Kernel support HDC direct write
    bool                            m_bApplyTwoLayersCompOptimize;//!< Apply 2 layers composition optimization
    bool                            m_need3DSampler;              //!< If AVS Sampler not avaliable on specific platform, then we need 3D sampler instead
    bool                            m_bEnableSamplerLumakey;      //!< Enable/Disable sampler lumakey feature.
    bool                            m_bYV12iAvsScaling;           //!< Interlace AVS scaling support YV12 input format

    // AVS table
    MHW_AVS_PARAMS                  m_AvsParameters;
    MHW_SAMPLER_AVS_TABLE_PARAM     m_mhwSamplerAvsTableParam;    //!< params for AVS scaling 8x8 table
    bool                            m_bAvsTableCoeffExtraEnabled; //!< Sampler AVS table param, bIsCoeffExtraEnabled
    bool                            m_bAvsTableBalancedFilter;    //!< Sampler AVS table param, bBalancedFilter

    static const int                AVS_CACHE_SIZE = 4;           //!< AVS coefficients cache size
    AvsCoeffsCache<AVS_CACHE_SIZE>  m_AvsCoeffsCache;             //!< AVS coefficients calculation is expensive, add cache to mitigate
    VPHAL_SURFACE                   m_IntermediateSurface  = {};  //!< Intermediate surface (multiple phase / constriction support)
    VPHAL_SURFACE                   m_IntermediateSurface1 = {};  //!< Intermediate surface (multiple phase / constriction support)
    VPHAL_SURFACE                   *m_Intermediate  = nullptr;   //!< Intermediate surface (multiple phase / constriction support)
    VPHAL_SURFACE                   *m_Intermediate1 = nullptr;   //!< Intermediate surface (multiple phase / constriction support)
    VPHAL_SURFACE                   *m_Intermediate2 = nullptr;   //!< Rotation output intermediate surface
    VPHAL_SURFACE                   m_IntermediateSurface2 = {};  //!< Rotation output intermediate surface
    
    VPHAL_SURFACE                   m_AuxiliarySyncSurface = {};  //!< This Auxiliary surface is used to sync engine workload

    Kdll_State                      *m_pKernelDllState = nullptr; //!< Compositing Kernel DLL/Search state
    RENDERHAL_KERNEL_PARAM          m_KernelParams = {0};

    float                           m_fScaleX = 1.0f;
    float                           m_fScaleY = 1.0f;

    bool                            m_FusedEuDispatch = false;
};

typedef CompositeState * PCComposite;

#endif // __VPHAL_RENDER_COMPOSITE_H__
