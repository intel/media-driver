/*
* Copyright (c) 2021-2022, Intel Corporation
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
//! \file     ddi_vp_functions.h
//! \brief    ddi vp functions head file
//!

#ifndef __DDI_VP_FUNCTIONS_H__
#define __DDI_VP_FUNCTIONS_H__

#include "ddi_media_functions.h"
#include "media_libva_common_next.h"
#include "vp_common.h"
#include "vp_base.h"

// Maximum primary surface number in VP
#define VP_MAX_PRIMARY_SURFS                1

// surface flag : 1 secure;  0 clear
#if (VA_MAJOR_VERSION < 1)
#define VPHAL_SURFACE_ENCRYPTION_FLAG       0x80000000
#else
#define VPHAL_SURFACE_ENCRYPTION_FLAG       0x00000001
#endif

#define DDI_VP_MAX_NUM_FILTERS              VAProcFilterCount     /* Some filters in va_private.h */

#if (_DEBUG || _RELEASE_INTERNAL)
typedef struct _DDI_VP_DUMP_PARAM
{
    VAProcPipelineParameterBuffer             *pPipelineParamBuffers[VPHAL_MAX_SOURCES] = {};
    MOS_FORMAT                                SrcFormat[VPHAL_MAX_SOURCES]              = {};
    MOS_FORMAT                                TargetFormat[VPHAL_MAX_TARGETS]           = {};
} DDI_VP_DUMP_PARAM, *PDDI_VP_DUMP_PARAM;
#endif //(_DEBUG || _RELEASE_INTERNAL)

typedef struct _DDI_VP_FRAMEID_TRACER
{
    MOS_LINUX_BO                       *pLastSrcSurfBo;
    MOS_LINUX_BO                       *pLastBwdSurfBo;

    uint32_t                                   uiLastSrcSurfFrameID;
    uint32_t                                   uiLastBwdSurfFrameID;

    uint32_t                                   uiFrameIndex;

    uint32_t                                   uiLastSampleType;
} DDI_VP_FRAMEID_TRACER;

class DdiCpInterface;
class DdiCpInterfaceNext;
//core structure for VP DDI
typedef struct DDI_VP_CONTEXT
{
    // VPHAL internal structure
    MOS_CONTEXT                               MosDrvCtx           = {};
    VpBase                                    *pVpHal             = nullptr;
    VPHAL_RENDER_PARAMS                       *pVpHalRenderParams = nullptr;

    DdiCpInterface                            *pCpDdiInterface    = nullptr;
    DdiCpInterfaceNext                        *pCpDdiInterfaceNext = nullptr;

    // target surface id
    VASurfaceID                               TargetSurfID        = 0;

    // Primary surface number
    int32_t                                   iPriSurfs           = 0;

    DDI_VP_FRAMEID_TRACER                     FrameIDTracer       = {};

#if (_DEBUG || _RELEASE_INTERNAL)
    DDI_VP_DUMP_PARAM                         *pCurVpDumpDDIParam = nullptr;
    DDI_VP_DUMP_PARAM                         *pPreVpDumpDDIParam = nullptr;
    FILE                                      *fpDumpFile         = nullptr;
#endif //(_DEBUG || _RELEASE_INTERNAL)

} DDI_VP_CONTEXT, *PDDI_VP_CONTEXT;

typedef struct _DDI_VP_STATE
{
    bool      bProcampEnable     = false;
    bool      bDeinterlaceEnable = false;
    bool      bDenoiseEnable     = false;
    bool      bIEFEnable         = false;
} DDI_VP_STATE;

class DdiVpFunctions :public DdiMediaFunctions
{
public:

    virtual ~DdiVpFunctions() override{};
    //!
    //! \brief  Create context
    //!
    //! \param  [in] ctx
    //!         Pointer to VA driver context
    //! \param  [in] configId
    //!         VA config id
    //! \param  [in] pictureWidth
    //!         Picture width
    //! \param  [in] pictureHeight
    //!         Picture height
    //! \param  [out] flag
    //!         Create flag
    //! \param  [in] renderTargets
    //!         VA render traget
    //! \param  [in] renderTargetsNum
    //!         Number of render targets
    //! \param  [out] ctxID
    //!         VA created context
    //!
    //! \return VAStatus
    //!     VA_STATUS_SUCCESS if success, else fail reason
    //!
    virtual VAStatus CreateContext (
        VADriverContextP  ctx,
        VAConfigID        configId,
        int32_t           pictureWidth,
        int32_t           pictureHeight,
        int32_t           flag,
        VASurfaceID       *renderTargets,
        int32_t           renderTargetsNum,
        VAContextID       *ctxID
    ) override;

    //!
    //! \brief  Destroy context
    //!
    //! \param  [in] ctx
    //!         Pointer to VA driver context
    //! \param  [in] ctxID
    //!         VA context to destroy
    //!
    //! \return VAStatus
    //!     VA_STATUS_SUCCESS if success, else fail reason
    //!
    virtual VAStatus DestroyContext (
        VADriverContextP  ctx,
        VAContextID       ctxID
    ) override;

    //!
    //! \brief  Create buffer
    //!
    //! \param  [in] ctx
    //!         Pointer to VA driver context
    //! \param  [in] context
    //!         VA context id
    //! \param  [in] type
    //!         VA buffer type
    //! \param  [in] size
    //!         Buffer size
    //! \param  [out] elementsNum
    //!         Number of elements
    //! \param  [in] data
    //!         Buffer data
    //! \param  [out] bufId
    //!         VA buffer id
    //!
    //! \return VAStatus
    //!     VA_STATUS_SUCCESS if success, else fail reason
    //!
    virtual VAStatus CreateBuffer (
        VADriverContextP  ctx,
        VAContextID       context,
        VABufferType      type,
        uint32_t          size,
        uint32_t          elementsNum,
        void              *data,
        VABufferID        *bufId
    ) override;

    //! \param  [in] mediaCtx
    //!         Pointer to media context
    //! \param  [in] bufId
    //!         VA buffer ID
    //! \param  [out] buf
    //!         Pointer to buffer
    //! \param  [in] flag
    //!         Flag
    //!
    //! \return VAStatus
    //!     VA_STATUS_SUCCESS if success, else fail reason
    //!
    virtual VAStatus MapBufferInternal(
        PDDI_MEDIA_CONTEXT  mediaCtx,
        VABufferID          bufId,
        void                **buf,
        uint32_t            flag
    ) override;

    //! \brief  Unmap buffer
    //!
    //! \param  [in] mediaCtx
    //!     Pointer to media context
    //! \param  [in] bufId
    //!     VA buffer ID
    //!
    //! \return VAStatus
    //!     VA_STATUS_SUCCESS if success, else fail reason
    //!
    virtual VAStatus UnmapBuffer (
        PDDI_MEDIA_CONTEXT mediaCtx,
        VABufferID         bufId
    )override;

    //!
    //! \brief  Destroy buffer
    //!
    //! \param  [in] mediaCtx
    //!     Pointer to media context
    //! \param  [in] bufId
    //!     VA buffer ID
    //!
    //! \return     VAStatus
    //!     VA_STATUS_SUCCESS if success, else fail reason
    //!
    virtual VAStatus DestroyBuffer(
        DDI_MEDIA_CONTEXT  *mediaCtx,
        VABufferID         bufId
    )override;

    //!
    //! \brief  Get ready to decode a picture to a target surface
    //!
    //! \param  [in] ctx
    //!         Pointer to VA driver context
    //! \param  [in] context
    //!         VA context id
    //! \param  [in] renderTarget
    //!         VA render target surface
    //!
    //! \return VAStatus
    //!     VA_STATUS_SUCCESS if success, else fail reason
    //!
    virtual VAStatus BeginPicture (
        VADriverContextP  ctx,
        VAContextID       context,
        VASurfaceID       renderTarget
    ) override;

    //!
    //! \brief  Send decode buffers to the server
    //! \details    Buffers are automatically destroyed afterwards
    //! \param  [in] ctx
    //!         Pointer to VA driver context
    //! \param  [in] context
    //!         VA buffer id
    //! \param  [in] buffer
    //!         Pointer to VA buffer id
    //! \param  [in] buffersNum
    //!         number of buffers
    //!
    //! \return VAStatus
    //!     VA_STATUS_SUCCESS if success, else fail reason
    //!
    virtual VAStatus RenderPicture (
        VADriverContextP  ctx,
        VAContextID       context,
        VABufferID        *buffers,
        int32_t           buffersNum
    ) override;

    //!
    //! \brief  Make the end of rendering for a picture
    //! \details    The server should start processing all pending operations for this
    //!             surface. This call is non-blocking. The client can start another
    //!             Begin/Render/End sequence on a different render target
    //! \param  [in] ctx
    //!         Pointer to VA driver context
    //! \param  [in] context
    //!         VA buffer id
    //!
    //! \return VAStatus
    //!     VA_STATUS_SUCCESS if success, else fail reason
    //!
    virtual VAStatus EndPicture (
        VADriverContextP  ctx,
        VAContextID       context
    ) override;

    //!
    //! \brief  Create a configuration for the encode/decode/vp pipeline
    //! \details    it passes in the attribute list that specifies the attributes it cares
    //!             about, with the rest taking default values.
    //!
    //! \param  [in] ctx
    //!         Pointer to VA driver context
    //! \param  [in] profile
    //!         VA profile of configuration
    //! \param  [in] entrypoint
    //!         VA entrypoint of configuration
    //! \param  [out] attribList
    //!         VA attrib list
    //! \param  [out] attribsNum
    //!         Number of attribs
    //! \param  [out] configId
    //!         VA config id
    //!
    //! \return VAStatus
    //!     VA_STATUS_SUCCESS if success, else fail reason
    //!
    virtual VAStatus CreateConfig (
        VADriverContextP  ctx,
        VAProfile         profile,
        VAEntrypoint      entrypoint,
        VAConfigAttrib    *attribList,
        int32_t           attribsNum,
        VAConfigID        *configId
    ) override;

    //!
    //! \brief  Query video proc filters
    //!
    //! \param  [in] ctx
    //!         Pointer to VA driver context
    //! \param  [in] context
    //!         VA context ID
    //! \param  [in] filters
    //!         VA proc filter type
    //! \param  [in] filtersNum
    //!         Number of filters
    //!
    //! \return VAStatus
    //!     VA_STATUS_SUCCESS if success, else fail reason
    //!
    virtual VAStatus QueryVideoProcFilters (
        VADriverContextP  ctx,
        VAContextID       context,
        VAProcFilterType  *filters,
        uint32_t          *filtersNum
    ) override;

    //!
    //! \brief  Query video processing filter capabilities.
    //!         The real implementation is in media_libva_vp.c, since it needs to use some definitions in vphal.h.
    //!
    //! \param  [in] ctx
    //!         Pointer to VA driver context
    //! \param  [in] context
    //!         VA context ID
    //! \param  [in] type
    //!         VA proc filter type
    //! \param  [inout] filterCaps
    //!         FIlter caps
    //! \param  [inout] filterCapsNum
    //!         Number of filter caps
    //!
    //! \return VAStatus
    //!     VA_STATUS_SUCCESS if success, else fail reason
    //!
    virtual VAStatus QueryVideoProcFilterCaps (
        VADriverContextP  ctx,
        VAContextID       context,
        VAProcFilterType  type,
        void              *filterCaps,
        uint32_t          *filterCapsNum
    ) override;

    //!
    //! \brief  Query video proc pipeline caps
    //!
    //! \param  [in] ctx
    //!         Pointer to VA driver context
    //! \param  [in] context
    //!         VA context ID
    //! \param  [in] filters
    //!         VA buffer ID
    //! \param  [in] filtersNum
    //!         Number of filters
    //! \param  [in] pipelineCaps
    //!         VA proc pipeline caps
    //!
    //! \return VAStatus
    //!     VA_STATUS_SUCCESS if success, else fail reason
    //!
    virtual VAStatus QueryVideoProcPipelineCaps (
        VADriverContextP    ctx,
        VAContextID         context,
        VABufferID          *filters,
        uint32_t            filtersNum,
        VAProcPipelineCaps  *pipelineCaps
    ) override;

    virtual VAStatus StatusCheck(
        PDDI_MEDIA_CONTEXT mediaCtx,
        DDI_MEDIA_SURFACE  *surface,
        VASurfaceID        surfaceId
    ) override;

    virtual VAStatus ProcessPipeline(
        VADriverContextP    vaDrvCtx,
        VAContextID         ctxID,
        VASurfaceID         srcSurface,
        VARectangle         *srcRect,
        VASurfaceID         dstSurface,
        VARectangle         *dstRect
    ) override;

    virtual VAStatus PutSurface(
        VADriverContextP ctx,
        VASurfaceID      surface,
        void             *draw,
        int16_t          srcx,
        int16_t          srcy,
        uint16_t         srcw,
        uint16_t         srch,
        int16_t          destx,
        int16_t          desty,
        uint16_t         destw,
        uint16_t         desth,
        VARectangle      *cliprects,
        uint32_t         numberCliprects,
        uint32_t         flags
    ) override;

    //!
    //! \brief Extract VAProcPipelineParameterBuffer params and set the appropriate VPHAL params
    //!
    //! \param [in]  vaDrvCtx
    //!        VA Driver context
    //! \param [in]  vpCtx
    //!        VP context
    //! \param [in]  pipelineParam
    //!        Pipeline params from application (VAProcPipelineParameterBuffer)
    //!
    //! \returns VA_STATUS_SUCCESS if call succeeds
    //!
    VAStatus DdiSetProcPipelineParams(
        VADriverContextP               vaDrvCtx,
        PDDI_VP_CONTEXT                vpCtx,
        VAProcPipelineParameterBuffer  *pipelineParam);

private:
    //!
    //! \brief  Helper function for VpAllocateDrvCtxExt to Allocate PDDI_VP_CONTEXT
    //!
    //! \param  [in] vaDrvCtx
    //!         Pointer to VA driver context
    //! \param  [in] vpCtx
    //!          VP context
    //!
    //! \return VAStatus
    //! VA_STATUS_SUCCESS if succeeds, else fail reason
    VAStatus DdiInitCtx(
        VADriverContextP vaDrvCtx,
        PDDI_VP_CONTEXT  vpCtx);

    //!
    //! \brief  Free VPHAL Driver render params resources
    //!
    //! \param  [in] vpCtx
    //!          VP context
    //! \param  [in] vpHalRenderParams
    //!         vphal render params
    //! \return
    //!
    void FreeVpHalRenderParams(
        PDDI_VP_CONTEXT      vpCtx,
        PVPHAL_RENDER_PARAMS vpHalRenderParams);

    //!
    //! \brief  Initialize VPHAL State and VPHAL settings per VP context
    //!
    //! \param  [in] vpCtx
    //!          VP context
    //!
    //! \return VAStatus
    //! VA_STATUS_SUCCESS if succeeds, else fail reason
    VAStatus DdiInitVpHal(PDDI_VP_CONTEXT vpCtx);

    //!
    //! \brief  Destroy VPHAL driver context
    //!
    //! \param  [in] vpCtx
    //!          VP context
    //!
    //! \return VAStatus
    //! VA_STATUS_SUCCESS if succeeds, else fail reason
    VAStatus DdiDestroyVpHal(PDDI_VP_CONTEXT vpCtx);

    //!
    //! \brief  Destroy VPHAL driver render params
    //!
    //! \param  [in] vpCtx
    //!          VP context
    //!
    //! \return VAStatus
    //! VA_STATUS_SUCCESS if succeeds, else fail reason
    VAStatus DdiDestroyRenderParams(PDDI_VP_CONTEXT vpCtx);

    //!
    //! \brief  Destroy VPHAL driver source params
    //!
    //! \param  [in] vpCtx
    //!          VP context
    //!
    //! \return VAStatus
    //! VA_STATUS_SUCCESS if succeeds, else fail reason
    VAStatus DdiDestroySrcParams(PDDI_VP_CONTEXT vpCtx);

    //!
    //! \brief  Destroy VPHAL driver reference params
    //!
    //! \param  [in] vpCtx
    //!          VP context
    //!
    //! \return VAStatus
    //! VA_STATUS_SUCCESS if succeeds, else fail reason
    VAStatus DdiDestroyVpHalSurface(PVPHAL_SURFACE surf);

    //!
    //! \brief  Destroy VPHAL driver target params
    //!
    //! \param  [in] vpCtx
    //!          VP context
    //!
    //! \return VAStatus
    //! VA_STATUS_SUCCESS if succeeds, else fail reason
    VAStatus DdiDestroyTargetParams(PDDI_VP_CONTEXT vpCtx);

    //!
    //! \brief  Query video processing Noise reduction filter caps.
    //!
    //! \param  [in] queryFlag
    //!         QUERY_CAPS_ATTRIBUTE: search caps attribute
    //! \param  [in] queryCapsNum
    //!         The filter caps number queried by app layer
    //! \param  [in] existCapsNum
    //!         The actual number of filters in vp module
    //! \param  [inout] filterCaps
    //!         FIlter caps
    //!
    //! \return VAStatus
    //!     VA_STATUS_SUCCESS if success, else fail reason
    //!
    VAStatus QueryNoiseReductionCapsAttrib(
        uint32_t queryFlag,
        uint32_t queryCapsNum,
        uint32_t existCapsNum,
        void     *filterCaps);

    //!
    //! \brief  Query video processing Deinterlacing filter caps.
    //!
    //! \param  [in] queryFlag
    //!         QUERY_CAPS_ATTRIBUTE: search caps attribute
    //! \param  [in] queryCapsNum
    //!         The filter caps number queried by app layer
    //! \param  [in] existCapsNum
    //!         The actual number of filters in vp module
    //! \param  [inout] filterCaps
    //!         FIlter caps
    //!
    //! \return VAStatus
    //!     VA_STATUS_SUCCESS if success, else fail reason
    //!
    VAStatus QueryDeinterlacingCapsAttrib(
        uint32_t queryFlag,
        uint32_t queryCapsNum,
        uint32_t existCapsNum,
        void     *filterCaps);

    //!
    //! \brief  Query video processing Sharpening filter caps.
    //!
    //! \param  [in] queryFlag
    //!         QUERY_CAPS_ATTRIBUTE: search caps attribute
    //! \param  [in] queryCapsNum
    //!         The filter caps number queried by app layer
    //! \param  [in] existCapsNum
    //!         The actual number of filters in vp module
    //! \param  [inout] filterCaps
    //!         FIlter caps
    //!
    //! \return VAStatus
    //!     VA_STATUS_SUCCESS if success, else fail reason
    //!
    VAStatus QuerySharpeningCapsAttrib(
        uint32_t queryFlag,
        uint32_t queryCapsNum,
        uint32_t existCapsNum,
        void     *filterCaps);

    //!
    //! \brief  Query video processing Color balance filter caps.
    //!
    //! \param  [in] queryFlag
    //!         QUERY_CAPS_ATTRIBUTE: search caps attribute
    //! \param  [in] queryCapsNum
    //!         The filter caps number queried by app layer
    //! \param  [in] existCapsNum
    //!         The actual number of filters in vp module
    //! \param  [inout] filterCaps
    //!         FIlter caps
    //!
    //! \return VAStatus
    //!     VA_STATUS_SUCCESS if success, else fail reason
    //!
    VAStatus QueryColorBalanceCapsAttrib(
        uint32_t queryFlag,
        uint32_t queryCapsNum,
        uint32_t existCapsNum,
        void     *filterCaps);

    //!
    //! \brief  Query video processing SkinToneEnhancement filter caps.
    //!
    //! \param  [in] queryFlag
    //!         QUERY_CAPS_ATTRIBUTE: search caps attribute
    //! \param  [in] queryCapsNum
    //!         The filter caps number queried by app layer
    //! \param  [in] existCapsNum
    //!         The actual number of filters in vp module
    //! \param  [inout] filterCaps
    //!         FIlter caps
    //!
    //! \return VAStatus
    //!     VA_STATUS_SUCCESS if success, else fail reason
    //!
    VAStatus QuerySkinToneEnhancementCapsAttrib(
        uint32_t queryFlag,
        uint32_t queryCapsNum,
        uint32_t existCapsNum,
        void     *filterCaps);

    //!
    //! \brief  Query video processing TotalColorCorrection filter caps.
    //!
    //! \param  [in] queryFlag
    //!         QUERY_CAPS_ATTRIBUTE: search caps attribute
    //! \param  [in] queryCapsNum
    //!         The filter caps number queried by app layer
    //! \param  [in] existCapsNum
    //!         The actual number of filters in vp module
    //! \param  [inout] filterCaps
    //!         FIlter caps
    //!
    //! \return VAStatus
    //!     VA_STATUS_SUCCESS if success, else fail reason
    //!
    VAStatus QueryTotalColorCorrectionCapsAttrib(
        uint32_t queryFlag,
        uint32_t queryCapsNum,
        uint32_t existCapsNum,
        void     *filterCaps);

    //!
    //! \brief  Query video processing HDR filter caps.
    //!
    //! \param  [in] queryFlag
    //!         QUERY_CAPS_ATTRIBUTE: search caps attribute
    //! \param  [in] queryCapsNum
    //!         The filter caps number queried by app layer
    //! \param  [in] existCapsNum
    //!         The actual number of filters in vp module
    //! \param  [inout] filterCaps
    //!         FIlter caps
    //!
    //! \return VAStatus
    //!     VA_STATUS_SUCCESS if success, else fail reason
    //!
    VAStatus QueryHDRToneMappingCapsAttrib(
        uint32_t            queryFlag,
        uint32_t            queryCapsNum,
        uint32_t            existCapsNum,
        void                *filterCaps);

    //!
    //! \brief  Query video processing 3Dlut filter caps.
    //!
    //! \param  [in] queryFlag
    //!         QUERY_CAPS_ATTRIBUTE: search caps attribute
    //! \param  [in] queryCapsNum
    //!         The filter caps number queried by app layer
    //! \param  [in] existCapsNum
    //!         The actual number of filters in vp module
    //! \param  [inout] filterCaps
    //!         FIlter caps
    //!
    //! \return VAStatus
    //!     VA_STATUS_SUCCESS if success, else fail reason
    //!
    VAStatus QueryLut3DCapsAttrib(
        uint32_t            queryFlag,
        uint32_t            queryCapsNum,
        uint32_t            existCapsNum,
        void                *filterCaps);

    //! \brief Get the render parameters from Va Driver Context.
    //!
    //! \params [in] vpCtx
    //!         VP context
    //!
    //! \returns Pointer of render parameters
    //!
    PVPHAL_RENDER_PARAMS VpGetRenderParams(PDDI_VP_CONTEXT vpCtx);

    //! \brief  Get resource informaton from target surface, set OS Resource for VPHAL
    //!
    //! \params [in] pVpCtx
    //!         VP context
    //! \params [in] boRt
    //!         media surface
    //! \params [in] targetIndex
    //!         index of target surface in VpHal RenderParams.
    //!
    //! \returns: Result of operation
    //!
    VAStatus VpSetOsResource(PDDI_VP_CONTEXT vpCtx, PDDI_MEDIA_SURFACE boRt, uint32_t targetIndex);

    //! \brief  Set Render Params for VPHAL
    //!
    //! \params [in] mediaSurf
    //!         media surface
    //! \params [inout] vpHalRenderParams
    //!         PVPHAL_RENDER_PARAMS.
    //! \params [in] renderTarget
    //!         surface ID.
    //!
    void VpSetRenderParams(
        PDDI_MEDIA_SURFACE    mediaSurf,
        PVPHAL_RENDER_PARAMS  vpHalRenderParams,
        VASurfaceID           renderTarget);

    //! \brief judge whether the pitch size match 16aligned usrptr path require or not
    //!
    //! \params [in] pitch
    //!         surface pitch size.
    //! \params [in] format
    //!         surface foramt
    //! \returns true if matched
    //! for YV12 format, if pitch aligned with 128, go legacy path; if aligned with 16/32/64, go 16usrptr path
    //! for other formats, legcy path for aligned with 64, 16usrpt path for aligned with 16/32
    //!
    bool VpIs16UsrPtrPitch(uint32_t pitch, DDI_MEDIA_FORMAT format);

    //!
    //! \brief   dump feature mode parameters for Android
    //!
    //! \param   [in] vpCtx
    //!          vp context
    //! \return  VAStatus
    //!          return VA_STATUS_SUCCESS if params is dumped to file.
    //!
    VAStatus VpReportFeatureMode(PDDI_VP_CONTEXT vpCtx);

    //!
    //! \brief  initialize configuration values for Android
    //!
    //! \param  [in] configValues
    //!          vp config values
    //!
    void VpConfigValuesInit(PVP_CONFIG  configValues);

    //!
    //! \brief    Report mode of different features
    //!
    //! \param    [in] vpHalState
    //!           VPHAL state pointer
    //! \param    [inout] configValues
    //!           Porinter to configuration report value structure,
    //!           feature modes will be store in this structure.
    //! \return   void
    //!
    void VpHalDdiReportFeatureMode(
        VpBase                 *vpHalState,
        PVP_CONFIG             configValues);

    //!
    //! \brief   dump config values for Android
    //!
    //! \param   [in] config
    //!          vp config values
    //!
    void VpFeatureReport(PVP_CONFIG config, PDDI_VP_CONTEXT vpCtx);

    //!
    //! \purpose judge whether the PipelineParam buffer is for target or not
    //!
    //! \param  [in] vaDrvCtx
    //!         VA Driver context
    //! \param  [in] vpCtx
    //!          VP context
    //! \param  [in] pipelineParam
    //!         Pipeline parameters from application (VAProcPipelineParameterBuffer)
    //!
    //! \returns VA_STATUS_SUCCESS if call succeeds
    //!
    bool VpIsRenderTarget(
        VADriverContextP              vaDrvCtx,
        PDDI_VP_CONTEXT               vpCtx,
        VAProcPipelineParameterBuffer *pipelineParam);

    VAStatus DdiSetGpuPriority(
        PDDI_VP_CONTEXT     vpCtx,
        int32_t             priority);

    //!
    //! \brief Extract VAProcPipelineParameterBuffer params for target surface and set the appropriate VPHAL params
    //!
    //! \param  [in] vaDrvCtx
    //!         VA Driver context
    //! \param  [in] vpCtx
    //!         VP context
    //! \param  [in] pipelineParam
    //!         Pipeline parameters from application (VAProcPipelineParameterBuffer)
    //!
    //! \returns VA_STATUS_SUCCESS if call succeeds
    //!
    VAStatus VpSetRenderTargetParams(
        VADriverContextP               vaDrvCtx,
        PDDI_VP_CONTEXT                vpCtx,
        VAProcPipelineParameterBuffer  *pipelineParam);

    //!
    //! \brief Set Color Standard Explictly.
    //!
    //! \param  [in]  vpHalSurf
    //!         src/target surface
    //! \param  [in]  colorStandard
    //!         VA color standard VAProcColorStandardType
    //! \param  [in]  colorProperties
    //!         input/output surface color properties
    //! \returns appropriate VA_STATUS_SUCCESS if call succeeds
    //!
    VAStatus VpSetColorStandardExplictly(
        PVPHAL_SURFACE          vpHalSurf,
        VAProcColorStandardType colorStandard,
        VAProcColorProperties   colorProperties);

    //!
    //! \brief // Set colorspace by default to avoid application don't set ColorStandard
    //!
    //! \param  [in]  vpHalSurf
    //!         src/target surface
    //!
    void VpSetColorSpaceByDefault(PVPHAL_SURFACE  vpHalSurf);

    //!
    //! \brief Set Color Space according to ColorStandard.
    //!
    //! \param  [in]  vpHalSurf
    //!         src/target surface
    //! \param  [in]  colorStandard
    //!         VA color standard VAProcColorStandardType
    //! \param  [in]  colorProperties
    //!         input/output surface color properties
    //! \returns
    //!
    void VpSetColorSpaceByColorStandard(
        PVPHAL_SURFACE          vpHalSurf,
        VAProcColorStandardType colorStandard,
        VAProcColorProperties   colorProperties,
        uint8_t                 colorRange);

    //!
    //! \brief Set Color Space according to ColorStandard.
    //!
    //! \param  [in]  vpCtx
    //!         vp context
    //! \param  [in]  srcSurf
    //!         vphal surface
    //! \returns
    //!
    void SetFrameID(PDDI_VP_CONTEXT vpCtx, PVPHAL_SURFACE srcSurf);

    //!
    //! \brief update sample type
    //!
    //! \param  [in]  srcSurf
    //!         vphal surface
    //! \param  [in]  flags
    //!         Deinterlacing flags
    //! \returns
    //!
    void UpdateSampleType(
        PVPHAL_SURFACE srcSurf,
        uint32_t       flags);

    //!
    //! \brief  Set src surface rect
    //!
    //! \param  [inout] surfaceRegion
    //!         pointed to src rect
    //! \param  [in] vpHalTgtSurf
    //!         pointed to vphal surface
    //! \param  [in] mediaSrcSurf
    //!         pointed to media surface
    //!
    void SetSrcRect(
        const VARectangle  *surfaceRegion,
        PVPHAL_SURFACE     vpHalTgtSurf,
        PDDI_MEDIA_SURFACE mediaSrcSurf);

    //!
    //! \brief  Set dest surface rect
    //!
    //! \param  [inout] surfaceRegion
    //!         pointed to dest rect
    //! \param  [in] vpHalTgtSurf
    //!         pointed to vphal surface
    //! \param  [in] mediaSrcSurf
    //!         pointed to media surface
    //!
    void SetDestRect(
        const VARectangle  *surfaceRegion,
        PVPHAL_SURFACE     vpHalTgtSurf,
        PDDI_MEDIA_SURFACE mediaSrcSurf);

    //!
    //! \brief   Check whether there is only Procamp with adjusting Brightness
    //! \params  [in]  vpHalSrcSurf
    //!          pointed to vphal surfeca
    //!
    //! \returns true if call succeeds
    //!
    bool IsProcmpEnable(PVPHAL_SURFACE vpHalSrcSurf);

    //!
    //! \brief  Map Chroma Sitting flags to appropriate VPHAL chroma sitting params
    //!
    //! \param  [in] vpHalSurf
    //!         pointed to vphal surface
    //! \param  [in] chromasitingState
    //!         chromasiting State
    //! \returns VA_STATUS_SUCCESS if call succeeds
    //!
    VAStatus VpUpdateProcChromaSittingState(
        PVPHAL_SURFACE vpHalSurf,
        uint8_t        chromasitingState);

    //!
    //! \brief  Set src surface rect
    //!
    //! \param  [inout] surfaceRegion
    //!         pointed to src rect
    //! \param  [in] vpHalTgtSurf
    //!         pointed to vphal surface
    //! \param  [in] mediaSrcSurf
    //!         pointed to media surface
    //!
    void SetSrcSurfaceRect(
        const VARectangle  *surfRegion,
        PVPHAL_SURFACE     vpHalSrcSurf,
        PDDI_MEDIA_SURFACE mediaSrcSurf);

    //!
    //! \brief  Set dest surface rect
    //!
    //! \param  [inout] outputRegion
    //!         pointed to dest rect
    //! \param  [in] vpHalTgtSurf
    //!         pointed to src vphal surface
    //! \param  [in] mediaSrcSurf
    //!         pointed to tgt vphal surface
    //!
    VAStatus SetDestSurfaceRect(
        const VARectangle *outputRegion,
        PVPHAL_SURFACE    vpHalSrcSurf,
        PVPHAL_SURFACE    vpHalTgtSurf);

    //! \brief Convert VAProcColorStandardType to VPHAL_CSPACE
    //!
    //! \param [in]  vpHalSurf
    //!        src/target surface
    //! \param [in]  colorStandard
    //!        VA color standard VAProcColorStandardType
    //! \param [in]  flag
    //!        input/output surface flag for full/reduced color range
    //! \returns appropriate VPHAL_CSPACE if call succeeds
    //!
#if (VA_MAJOR_VERSION < 1)
    VAStatus DdiGetColorSpace(
        PVPHAL_SURFACE          vpHalSurf,
        VAProcColorStandardType colorStandard,
        uint32_t                flag);
#else
    VAStatus DdiGetColorSpace(
        PVPHAL_SURFACE          vpHalSurf,
        VAProcColorStandardType colorStandard,
        VAProcColorProperties   colorProperties);
#endif

    //!
    //! \brief  Set background color for pipiline params
    //!
    //! \param  [in]  vpHalRenderParams
    //!         pointer to  PVPHAL_RENDER_PARAMS
    //! \param  [in]  outBackGroundcolor
    //!         output background color
    //!
    VAStatus SetBackgroundColorfill(
        PVPHAL_RENDER_PARAMS vpHalRenderParams,
        uint32_t             outBackGroundcolor);

    //!
    //! \brief  Set up split screen demo mode
    //! \param  [in] splitDemoPosDdi
    //!         split demo position setting from DDI layer
    //! \param  [in] splitDemoParaDdi
    //!         split demo parameters setting from DDI layer
    //! \param  [inout] splitScreenDemoModeParams
    //!         pointer to struct for split-screen demo mode parameters
    //! \param  [inout] disableDemoMode
    //!         return whether demo mode will be disable or not
    //! \param  [in] osInterface
    //!         pointer to MOS INTERFACE for OS interaction
    //! \return MOS_STATUS
    //!
    MOS_STATUS VpHalDdiSetupSplitScreenDemoMode(
        uint32_t                             splitDemoPosDdi,
        uint32_t                             splitDemoParaDdi,
        PVPHAL_SPLIT_SCREEN_DEMO_MODE_PARAMS *splitScreenDemoModeParams,
        bool                                 *disableDemoMode,
        PMOS_INTERFACE                       osInterface);

    //!
    //! \brief Update the future reference frames for VPHAL input surface
    //! \param  [in]  vpCtx
    //!         VP context
    //! \param  [in]  vaDrvCtx
    //!         VA Driver context
    //! \param  [in]  vpHalSrcSurf
    //!         VpHal source surface
    //! \param  [in]  pipelineParam
    //!         Pipeline parameter from application (VAProcPipelineParameterBuffer)
    //!
    //! \returns VA_STATUS_SUCCESS if call succeeds
    //!
    VAStatus DdiUpdateProcPipelineFutureReferenceFrames(
        PDDI_VP_CONTEXT                vpCtx,
        VADriverContextP               vaDrvCtx,
        PVPHAL_SURFACE                 vpHalSrcSurf,
        VAProcPipelineParameterBuffer  *pipelineParam);

    //!
    //! \brief Update the past reference frames for VPHAL input surface
    //!
    //! \param [in]  vpCtx
    //!        VP context
    //! \param [in]  vaDrvCtx
    //!        VA Driver context
    //! \param [in]  vpHalSrcSurf
    //!        VpHal source surface
    //! \param [in]  pipelineParam
    //!        Pipeline parameter from application (VAProcPipelineParameterBuffer)
    //! \returns VA_STATUS_SUCCESS if call succeeds
    //!
    VAStatus DdiUpdateProcPipelinePastReferenceFrames(
        PDDI_VP_CONTEXT               vpCtx,
        VADriverContextP              vaDrvCtx,
        PVPHAL_SURFACE                vpHalSrcSurf,
        VAProcPipelineParameterBuffer *pipelineParam);

    //!
    //! \brief Setup the appropriate filter params for VPHAL input surface based on Filter type
    //! \param  [in] vaDrvCtx
    //!         Driver context
    //! \param  [in]  vpCtx
    //!         VP context
    //! \param  [in]  surfIndex
    //!         surfIndex to the input surface array
    //! \param  [in]  filterType
    //!         Filter type
    //! \param  [in]  data
    //!         Buffer data
    //! \param  [in]  elementNum
    //!         number of elements in the buffer(FilterParameter)
    //! \param  [in]  vpStateFlags
    //!         filter enable status
    //!
    //! \returns VA_STATUS_SUCCESS if call succeeds
    //!
    VAStatus DdiUpdateFilterParamBuffer(
        VADriverContextP vaDrvCtx,
        PDDI_VP_CONTEXT  vpCtx,
        uint32_t         surfIndex,
        int32_t          filterType,
        void             *data,
        uint32_t         elementNum,
        DDI_VP_STATE     *vpStateFlags);

    //!
    //! \brief Set DI filter params for input VPHAL surface
    //!
    //!  \param  [in]  vpCtx
    //!          VP context
    //!  \param  [in]  surfIndex
    //!          surfIndex to the input surface array
    //!  \param  [in]  diParamBuff
    //!          Pointer to DI param buffer data
    //!
    //! \returns VA_STATUS_SUCCESS if call succeeds
    //!
    VAStatus DdiSetProcFilterDinterlaceParams(
        PDDI_VP_CONTEXT                            vpCtx,
        uint32_t                                   surfIndex,
        VAProcFilterParameterBufferDeinterlacing   *diParamBuff);

    //!
    //! \brief Set DN filter params for VPHAL input surface
    //!
    //! \param [in]  vpCtx
    //!        VP context
    //! \param [in]  surfIndex
    //!        surfIndex to the input surface array
    //! \param [in]  dnParamBuff
    //!        Pointer to DN param buffer data
    //!
    //! \returns VA_STATUS_SUCCESS if call succeeds
    //!
    VAStatus DdiSetProcFilterDenoiseParams(
        PDDI_VP_CONTEXT                vpCtx,
        uint32_t                       surfIndex,
        VAProcFilterParameterBuffer    *dnParamBuff);

    //!
    //! \brief Set DN filter params for VPHAL input surface
    //!
    //! \param [in]  vpCtx
    //!        VP context
    //! \param [in]  surfIndex
    //!        surfIndex to the input surface array
    //! \param [in]  hvsDnParamBuff
    //!        Pointer to HVS DN param buffer data
    //!
    //! \returns VA_STATUS_SUCCESS if call succeeds
    //!
    VAStatus DdiSetProcFilterHVSDenoiseParams(
        PDDI_VP_CONTEXT                              vpCtx,
        uint32_t                                     surfIndex,
        VAProcFilterParameterBufferHVSNoiseReduction *hvsDnParamBuff);

    //!
    //! \brief Set DN filter params for VPHAL input surface
    //!
    //! \param [in]  hvsDnParamBuff
    //!        Pointer to HVS DN param buffer data
    //! \param [in]  denoiseParams
    //!        Pointer to PVPHAL_DENOISE_PARAMS
    //!
    //! \returns VA_STATUS_SUCCESS if call succeeds
    //!
    void SetHVSDnParams(
        VAProcFilterParameterBufferHVSNoiseReduction *hvsDnParamBuff,
        PVPHAL_DENOISE_PARAMS                        denoiseParams);

    //!
    //! \brief    Init IEF Params to their default value
    //! \param    [out] iefParams
    //!           The IEF Params struct to be initialized
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    MOS_STATUS VpHalDdiInitIEFParams(
        PVPHAL_IEF_PARAMS       iefParams);

    //!
    //! \brief Set Sharpness (Image Enhancement Filter, IEF) filter params for VPHAL input surface
    //!
    //! \param [in]  vpCtx
    //!        VP context
    //! \param [in]  surfIndex
    //!        surfIndex to the input surface array
    //! \param [in]  sharpParamBuff
    //!        Pointer to Sharpness param buffer data
    //!
    //! \returns VA_STATUS_SUCCESS if call succeeds
    //!
    VAStatus DdiSetProcFilterSharpnessParams(
        PDDI_VP_CONTEXT             vpCtx,
        uint32_t                    surfIndex,
        VAProcFilterParameterBuffer *sharpParamBuff);

    //!
    //! \brief Set Color Balance (procamp) filter params for VPHAL input surface
    //!
    //! \param [in]  vpCtx
    //!        VP context
    //! \param [in]  surfIndex
    //!        surfIndex to the input surface array
    //! \param [in]  colorBalanceParamBuff
    //!        Pointer to Colorbalance param buffer data
    //! \param [in]  elementNum
    //!        number of elements in the Colorbalance param buffer data
    //!
    //! \returns VA_STATUS_SUCCESS if call succeeds
    //!
    VAStatus DdiSetProcFilterColorBalanceParams(
        PDDI_VP_CONTEXT                         vpCtx,
        uint32_t                                surfIndex,
        VAProcFilterParameterBufferColorBalance *colorBalanceParamBuff,
        uint32_t                                elementNum);

    //!
    //! \brief Set Color Balance (procamp) filter params for VPHAL input surface
    //!
    //! \param [in]  src
    //!        VPHAL input surface
    //! \param [in]  colorBalanceParamBuff
    //!        Pointer to Colorbalance param buffer data
    //! \param [in]  procamp
    //!        procamp flag
    //!
    //! \returns VA_STATUS_SUCCESS if call succeeds
    //!
    VAStatus SetColorBalanceParams(
        VAProcFilterParameterBufferColorBalance *colorBalanceParamBuff,
        uint32_t                                index,
        PVPHAL_SURFACE                          src,
        bool                                    procamp);

    //!
    //! \brief Set Skin Tone Enhancement (STE) filter params for VPHAL input surface
    //!
    //! \param [in]  vpCtx
    //!        VP context
    //! \param [in]  surfIndex
    //!        surfIndex to the input surface array
    //! \param [in]  steParamBuff
    //!        Pointer to Skin Tone Enhancement param buffer data
    //!
    //! \returns VA_STATUS_SUCCESS if call succeeds
    //!
    VAStatus DdiSetProcFilterSkinToneEnhancementParams(
        PDDI_VP_CONTEXT              vpCtx,
        uint32_t                     surfIndex,
        VAProcFilterParameterBuffer  *steParamBuff);

    //!
    //! \brief Total Color Correction (TCC) filter params for VPHAL input surface
    //!
    //! \param [in]  vpCtx
    //!        VP context
    //! \param [in]  surfIndex
    //!        surfIndex to the input surface array
    //! \param [in]  tccParamBuff
    //!        Pointer to Total Color Correction param buffer data
    //! \param [in]  elementNum
    //!        number of elements in the Total Color Correction param buffer data
    //!
    //! \returns VA_STATUS_SUCCESS if call succeeds
    //!
    VAStatus DdiSetProcFilterTotalColorCorrectionParams(
        PDDI_VP_CONTEXT                                 vpCtx,
        uint32_t                                        surfIndex,
        VAProcFilterParameterBufferTotalColorCorrection *tccParamBuff,
        uint32_t                                        elementNum);

    //!
    //! \brief High Dynamic Range (HDR) Tone Mapping filter params for VPHAL input surface
    //!
    //! \param [in]  vpCtx
    //!        VP context
    //! \param [in]  surfIndex
    //!        uSurfIndex to the input surface array
    //! \param [in]  hdrParamBuff
    //!         Pointer to High Dynamic Range Tone Mapping param buffer data
    //! \param [in]  elementNum
    //!        number of elements in the High Dynamic Range Tone Mapping param buffer data
    //!
    //! \returns VA_STATUS_SUCCESS if call succeeds
    //!
    VAStatus DdiSetProcFilterHdrTmParams(
        PDDI_VP_CONTEXT                           vpCtx,
        uint32_t                                  surfIndex,
        VAProcFilterParameterBufferHDRToneMapping *hdrTmParamBuff);

    //!
    //! \brief Set the appropriate HDR params according to colour standard, HDR metadata.
    //!
    //! \param [in]  vpHalSurf
    //!        VPHAL Surface
    //! \param [in]  hdrMetadata
    //!        HDR metadata
    //!
    //! \returns VA_STATUS_SUCCESS if call succeeds
    //!
    VAStatus VpUpdateProcHdrState(
        const PVPHAL_SURFACE vpHalSurf,
        const VAHdrMetaData  *hdrMetadata);

#if VA_CHECK_VERSION(1, 12, 0)
    //!
    //! \brief Three Three-Dimensional Look Up Table(3DLUT) filter params for VPHAL input surface
    //! \param
    //! \param [in]  vaDrvCtx
    //!        Driver context
    //! \param [in]  vpCtx
    //!        VP context
    //! \param [in]  surfIndex
    //!        surfIndex to the input surface array
    //! \param [in]  lut3DParamBuff
    //!        Pointer to 3DLUT param buffer data
    //!
    //! \returns VA_STATUS_SUCCESS if call succeeds
    //!
    VAStatus DdiSetProcFilter3DLutParams(
        VADriverContextP                 vaDrvCtx,
        PDDI_VP_CONTEXT                  vpCtx,
        uint32_t                         surfIndex,
        VAProcFilterParameterBuffer3DLUT *lut3DParamBuff);
#endif

    //!
    //! \brief clear filter params which is disabled for VPHAL input surface 
    //!
    //! \param [in]  vpCtx
    //!         VP context
    //! \param [in]  surfIndex
    //!        uSurfIndex to the input surface array
    //! \param [in]  vpStateFlags
    //!        filter enable status
    //!
    //! \returns VA_STATUS_SUCCESS if call succeeds
    //!
    VAStatus DdiClearFilterParamBuffer(
            PDDI_VP_CONTEXT     vpCtx,
            uint32_t            surfIndex,
            DDI_VP_STATE        vpStateFlags);

    //!
    //! \brief Set Interpolation Method according to the flag
    //!
    //! \param [in]  surface
    //!        VA Surface
    //! \param [in]  nterpolationflags
    //!        Interpolation Flag
    //!
    //! \returns VA_STATUS_SUCCESS if call succeeds
    //!
    VAStatus VpSetInterpolationParams(
        PVPHAL_SURFACE   surface,
        uint32_t         nterpolationflags);

    //!
    //! \brief Update VPHAL weave DI params
    //!
    //! \param [in]  vpHalSrcSurf
    //! \param [in]  filterFlags
    //!
    //! \returns VA_STATUS_SUCCESS if call succeeds
    //!
    void VpUpdateWeaveDI(PVPHAL_SURFACE vpHalSrcSurf, uint32_t filterFlags);

    //!
    //! \brief Map VA Rotation flags to appropriate VPHAL Rotation params
    //!
    //! \param [in]  vpHalSrcSurf
    //! \param [in]  rotationState
    //!
    //! \returns VA_STATUS_SUCCESS if call succeeds
    //!
    VAStatus VpUpdateProcRotateState(PVPHAL_SURFACE vpHalSrcSurf, uint32_t rotationState);

    //!
    //! \brief Map VA Mirroring flags to appropriate VPHAL Mirroring params
    //!
    //! \param [in]  pHalSrcSurf
    //! \param [in]  mirrorState
    //!
    //! \returns VA_STATUS_SUCCESS if call succeeds
    //!
    VAStatus VpUpdateProcMirrorState(PVPHAL_SURFACE vpHalSrcSurf, uint32_t mirrorState);

    //!
    //! \brief Set alpha blending params for VPHAL input surface
    //!
    //! \param [in]  vpCtx
    //!        VP context
    //! \param [in]  surfIndex
    //!        surfIndex to the input surface array
    //! \param [in]  pipelineParam
    //!        Pipeline paramseter from application (VAProcPipelineParameterBuffer)
    //!
    //! \returns VA_STATUS_SUCCESS if call succeeds
    //!
    VAStatus DdiSetProcPipelineBlendingParams(
        PDDI_VP_CONTEXT                        vpCtx,
        uint32_t                               surfIndex,
        VAProcPipelineParameterBuffer          *pipelineParam);

    //!
    //! \brief Set alpha blending types
    //!
    //! \param [in]  preMultAlpha
    //!        flag
    //! \param [in]  globalAlpha
    //!        flag
    //! \param [in]  blendingParams
    //!        Pointer to Blending Params
    //!
    //! \returns VA_STATUS_SUCCESS if call succeeds
    //!
    void SetBlendingTypes(
        bool                   preMultAlpha,
        bool                   globalAlpha,
        PVPHAL_BLENDING_PARAMS blendingParams,
        float                  globalalpha);

    //!
    //! \def IS_COLOR_SPACE_BT2020_YUV(_a)
    //! Check if the color space is BT2020 YUV
    //!
    #define IS_COLOR_SPACE_BT2020_YUV(_a) (_a == CSpace_BT2020 || \
                                        _a == CSpace_BT2020_FullRange)

    //!
    //! \def IS_COLOR_SPACE_BT2020_RGB(_a)
    //! Check if the color space is BT2020 RGB
    //!
    #define IS_COLOR_SPACE_BT2020_RGB(_a) (_a == CSpace_BT2020_RGB || \
                                        _a == CSpace_BT2020_stRGB)

    //!
    //! \def IS_COLOR_SPACE_BT2020(_a)
    //! Check if the color space is BT2020
    //!
    #define IS_COLOR_SPACE_BT2020(_a) (IS_COLOR_SPACE_BT2020_YUV(_a) || \
                                    IS_COLOR_SPACE_BT2020_RGB(_a))

    //!
    //! \brief Get ready to process a picture to a target surface
    //!
    //! \param [in]  vaDrvCtx
    //!        VA Driver Context
    //! \param [in]  vpCtx
    //!        VP context
    //! \param [in]  vaSurfID
    //!        target surface ID
    //!
    //! \returns VA_STATUS_SUCCESS if call succeeds
    //!
    VAStatus DdiBeginPictureInt(
            VADriverContextP    vaDrvCtx,
            PDDI_VP_CONTEXT     vpCtx,
            VASurfaceID         vaSurfID);

    //!
    //! \brief Check if the format contains alpha channel
    //!
    //! \param [in]  surface
    //!        VpHal Surface
    //!
    //! \returns true if the format of surface contains alpha channel
    //!
    bool hasAlphaInSurface(PVPHAL_SURFACE surface);

    //!
    //! \brief Extract Render Target params from VAProcPipelineParameterBuffer and set the appropriate VPHAL params for RT
    //!
    //! \param [in]  vaDrvCtx
    //!        VA Driver context
    //! \param [in]  vpCtx
    //!        VP context
    //! \param [in]  pipelineParam
    //!        VAProcPipelineParameterBuffer Pipeline paramseter from application
    //!
    //! \returns VA_STATUS_SUCCESS if call succeeds
    //!
    VAStatus DdiUpdateVphalTargetSurfColorSpace(
        VADriverContextP              vaDrvCtx,
        PDDI_VP_CONTEXT               vpCtx,
        VAProcPipelineParameterBuffer *pipelineParam,
        uint32_t                      targetIndex);

    //!
    //! \brief set vphal surface when Surface sample type is VA_TOP_FIELD_FIRST
    //!
    //! \param [in]  surfaceFlag
    //!        Surface sample type
    //! \param [in]  vpHalSrcSurf
    //!        Pointer to PVPHAL_SURFACE
    //!
    VAStatus SetSurfaceParamsTopFieldFirst(uint32_t surfaceFlag, PVPHAL_SURFACE vpHalSrcSurf, PVPHAL_SURFACE vpHalTgtSurf);

    //!
    //! \brief set vphal surface when Surface sample type is VA_BOTTOM_FIELD_FIRST
    //!
    //! \param [in]  surfaceFlag
    //!        Surface sample type
    //! \param [in]  vpHalSrcSurf
    //!        Pointer to PVPHAL_SURFACE
    //!
    VAStatus SetSurfaceParamsBottomFieldFirst(uint32_t surfaceFlag, PVPHAL_SURFACE vpHalSrcSurf, PVPHAL_SURFACE vpHalTgtSurf);

    //!
    //! \brief set vphal surface when Surface sample type is VA_TOP_FIELD
    //!
    //! \param [in]  surfaceFlag
    //!        Surface sample type
    //! \param [in]  vpHalSrcSurf
    //!        Pointer to PVPHAL_SURFACE
    //!
    VAStatus SetSurfaceParamsTopField(uint32_t surfaceFlag, PVPHAL_SURFACE vpHalSrcSurf, PVPHAL_SURFACE vpHalTgtSurf);

    //!
    //! \brief set vphal surface when Surface sample type is VA_BOTTOM_FIELD
    //!
    //! \param [in]  surfaceFlag
    //!        Surface sample type
    //! \param [in]  vpHalSrcSurf
    //!        Pointer to PVPHAL_SURFACE
    //!
    VAStatus SetSurfaceParamsBottomField(uint32_t surfaceFlag, PVPHAL_SURFACE vpHalSrcSurf, PVPHAL_SURFACE vpHalTgtSurf);

    //!
    //! \brief set vphal surface when Surface sample type is VA_BOTTOM_FIELD
    //!
    //! \param [in]  filterFlags
    //!        filter flag
    //! \param [in]  vpHalSrcSurf
    //!        Pointer to PVPHAL_SURFACE
    //!
    void SetLegacyInterlaceScalingParams(PVPHAL_SURFACE vpHalSrcSurf, uint32_t filterFlags);

#if defined(X11_FOUND)
    //!
    //! \brief  Rectangle initialization
    //!
    //! \param  [in] rect
    //!         Rectangle
    //! \param  [in] destx
    //!         Destination X
    //! \param  [in] desty
    //!         Destination Y
    //! \param  [in] destw
    //!          Destination W
    //! \param  [in] desth
    //!         Destination H
    //!
    void RectInit(
        RECT            *rect,
        int16_t          destx,
        int16_t          desty,
        uint16_t         destw,
        uint16_t         desth);

    //!
    //! \param  ctx
    //!     Pointer to VA driver context
    //! \param  surface
    //!     VA surface ID
    //! \param  draw
    //!     Drawable of window system
    //! \param  srcx
    //!     Source X of the region
    //! \param  srcy
    //!     Source Y of the region
    //! \param  srcw
    //!     Source W of the region
    //! \param  srch
    //!     Source H of the region
    //! \param  destx
    //!     Destination X
    //! \param  desty
    //!     Destination Y
    //! \param  destw
    //!     Destination W
    //! \param  desth
    //!     Destination H
    //! \param  cliprects
    //!     Client-supplied clip list
    //! \param  numberCliprects
    //!     Number of clip rects in the clip list
    //! \param  flags
    //!     De-interlacing flags
    //!
    VAStatus PutSurfaceLinuxHW(
        VADriverContextP ctx,
        VASurfaceID      surface,
        void             *draw,
        int16_t          srcx,
        int16_t          srcy,
        uint16_t         srcw,
        uint16_t         srch,
        int16_t          destx,
        int16_t          desty,
        uint16_t         destw,
        uint16_t         desth,
        VARectangle      *cliprects,
        uint32_t         numberCliprects,
        uint32_t         flags);
#endif // defined(X11_FOUND)

protected:
    static const VAProcFilterCapColorBalance m_vpColorBalCap[];
    static const VAProcFilterType            m_vpSupportedFilters[DDI_VP_MAX_NUM_FILTERS];

MEDIA_CLASS_DEFINE_END(DdiVpFunctions)
};

#endif //__DDI_VP_FUNCTIONS_H__
