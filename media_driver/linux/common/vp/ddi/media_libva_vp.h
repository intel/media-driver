/*
* Copyright (c) 2009-2017, Intel Corporation
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
//! \file     media_libva_vp.h
//! \brief    Header for Video Acceleration (LibVA) VP extensions for VPG drivers
//!

#ifndef _MEDIA_LIBVA_VP_H_
#define _MEDIA_LIBVA_VP_H_

#include "media_libva_common.h"
#include "media_libva_cp_interface.h"
#include <va/va.h>
#include <va/va_vpp.h>
#include <va/va_backend_vpp.h>
#include "vphal.h"
#include "mos_os.h"

// Maximum primary surface number in VP
#define VP_MAX_PRIMARY_SURFS     1

//For Gen8, only support max 16k-32
#define VP_MAX_PIC_WIDTH_Gen8    16352
#define VP_MAX_PIC_HEIGHT_Gen8   16352
//For Gen9+ platform, supprot 16k
#define VP_MAX_PIC_WIDTH         16384
#define VP_MAX_PIC_HEIGHT        16384

#define VP_MIN_PIC_WIDTH         16
#define VP_MIN_PIC_HEIGHT        16

// surface flag : 1 secure;  0 clear
#if (VA_MAJOR_VERSION < 1)
#define VPHAL_SURFACE_ENCRYPTION_FLAG 0x80000000
#else
#define VPHAL_SURFACE_ENCRYPTION_FLAG 0x00000001
#endif
#define NUM_SURFS 1

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

//core structure for VP DDI
typedef struct DDI_VP_CONTEXT
{
    // VPHAL internal structure
    MOS_CONTEXT                               MosDrvCtx           = {};
    VpBase                                    *pVpHal             = nullptr;
    VPHAL_RENDER_PARAMS                       *pVpHalRenderParams = nullptr;

    DdiCpInterface                            *pCpDdiInterface    = nullptr;

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

// public APIs

VAStatus DdiVp_CreateContext(
    VADriverContextP    pVaDrvCtx,
    VAConfigID          vaConfigID,
    int32_t             iWidth,
    int32_t             iHeight,
    int32_t             iFlag,
    VASurfaceID        *vaSurfIDs,
    int32_t             iNumSurfs,
    VAContextID        *pVaCtxID
);

VAStatus DdiVp_DestroyContext(
    VADriverContextP    pVaDrvCtx,
    VAContextID         vaCtxID
);

VAStatus DdiVp_CreateBuffer(
    VADriverContextP           ctx,
    void                       *pDecCtx,
    VABufferType               type,
    uint32_t                   size,
    uint32_t                   num_elements,
    void                       *pData,
    VABufferID                 *pBufId
);

/*
 * Create a configuration for the VP pipeline
 * it passes in the attribute list that specifies the attributes it cares
 * about, with the rest taking default values.
 */
VAStatus DdiVp_CreateConfig(
    VAProfile               profile,
    VAEntrypoint            entrypoint,
    VAConfigAttrib          *attrib_list,
    int32_t                 num_attribs,
    VAConfigID              *config_id
);

VAStatus DdiVp_BeginPicture(
        VADriverContextP    pVaDrvCtx,
        VAContextID         vaCtxID,
        VASurfaceID         vaSurfID
);

VAStatus DdiVp_EndPicture(
        VADriverContextP    pVaDrvCtx,
        VAContextID         vaCtxID
);

VAStatus DdiVp_RenderPicture(
    VADriverContextP    pVaDrvCtx,
    VAContextID         vpCtxID,
    VABufferID*         buffers,
    int32_t             num_buffers
);

VAStatus DdiVp_VideoProcessPipeline(
    VADriverContextP    pVaDrvCtx,
    VAContextID         vpCtxID,
    VASurfaceID         srcSurface,
    VARectangle         *srcRect,
    VASurfaceID         dstSurface,
    VARectangle         *dstRect
);

VAStatus DdiVp_QueryVideoProcFilterCaps(
    VADriverContextP    ctx,
    VAContextID         context,
    int32_t             type,
    void                *filter_caps,
    uint32_t            *num_filter_caps
);

VAStatus DdiVp_SetProcPipelineParams(
    VADriverContextP                pVaDrvCtx,
    PDDI_VP_CONTEXT                 pVpCtx,
    VAProcPipelineParameterBuffer*  pPipelineParam
);

PVPHAL_RENDER_PARAMS VpGetRenderParams(PDDI_VP_CONTEXT pVpCtx);

PDDI_VP_CONTEXT DdiVp_GetVpContextFromContextID(VADriverContextP ctx, VAContextID vaCtxID);

VAStatus DdiVp_SetGpuPriority(
    PDDI_VP_CONTEXT     pVpCtx,
    int32_t             priority
);
#endif //_MEDIA_LIBVA_VP_H_

