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
//! \file      media_libva_decoder.h 
//! \brief     libva(and its extension) decoder head file  
//!

#ifndef __MEDIA_LIBVA_DECODER_H__
#define __MEDIA_LIBVA_DECODER_H__

#include "media_libva.h"
#include "media_libva_cp.h"

#define DDI_DECODE_SFC_MAX_WIDTH                    4096
#define DDI_DECODE_SFC_MAX_HEIGHT                   4096
#define DDI_DECODE_SFC_MIN_WIDTH                    128
#define DDI_DECODE_SFC_MIN_HEIGHT                   128
#define DDI_DECODE_SFC_MIN_SCALE_RATIO              (1.0F/8.0F)

typedef enum _DDI_DECODE_MODE
{
    MPEG2 = 1,
    VC1   = 2,
    H264  = 4,
    JPEG  = 8
} DDI_DECODE_MODE;

struct DDI_DECODE_CONFIG_ATTR
{
    VAProfile           profile;
    VAEntrypoint        entrypoint;
    uint32_t            uiDecSliceMode;
    uint32_t            uiEncryptionType;
    uint32_t            uiDecProcessingType;
};

typedef struct DDI_DECODE_CONFIG_ATTR *PDDI_DECODE_CONFIG_ATTR;

class DdiMediaDecode;

struct DDI_DECODE_CONTEXT
{
    // Decoder data related with the specific codec
    // The instance of DdiDecodeXXX. For example: DdiDecodeAvc, DdiDecodeJPEG
    DdiMediaDecode                  *m_ddiDecode;
    // Decoder private data
    DdiCpInterface                  *pCpDdiInterface;

    // Parameters
    CodechalDecodeParams            DecodeParams;
    CODECHAL_STANDARD               Standard;
    uint32_t                        m_groupIndex;

    uint16_t                        wPicWidthInMB;          // Picture Width in MB width count
    uint16_t                        wPicHeightInMB;         // Picture Height in MB height count

    uint16_t                        wMode;                  // Get the info during hand shaking
    uint32_t                        dwWidth;                // Picture Width
    uint32_t                        dwHeight;               // Picture Height
    Codechal                        *pCodecHal;
    bool                            bShortFormatInUse;
    bool                            bStreamOutEnabled;

    // Status Report
    CodechalDecodeStatusReport     *pDecodeStatusReport;
    uint16_t                        wDecStatusReportNum;
    VASurfaceDecodeMBErrors         vaSurfDecErrOutput[2];

    DDI_CODEC_RENDER_TARGET_TABLE   RTtbl;
    DDI_CODEC_COM_BUFFER_MGR        BufMgr;
    PDDI_MEDIA_CONTEXT              pMediaCtx;

    // Add a list to track DPB.
    VASurfaceID                     RecListSurfaceID[CODECHAL_AVC_NUM_UNCOMPRESSED_SURFACE];
    uint32_t                        dwSliceParamBufNum;
    uint32_t                        dwSliceCtrlBufNum;
    uint32_t                        uiDecProcessingType;
};

typedef struct DDI_DECODE_CONTEXT *PDDI_DECODE_CONTEXT;

static __inline PDDI_DECODE_CONTEXT DdiDecode_GetDecContextFromPVOID (void *pDecCtx)
{
    return (PDDI_DECODE_CONTEXT)pDecCtx;
}

PDDI_DECODE_CONTEXT DdiDecode_GetDecContextFromContextID (VADriverContextP ctx, VAContextID vaCtxID);
int32_t DdiDecode_GetBitstreamBufIndexFromBuffer(DDI_CODEC_COM_BUFFER_MGR *pBufMgr, DDI_MEDIA_BUFFER *pBuf);

VAStatus DdiDecode_CreateBuffer(
    VADriverContextP         ctx,
    PDDI_DECODE_CONTEXT      pDecCtx,
    VABufferType             type,
    uint32_t                 size,
    uint32_t                 num_elements,
    void                    *pData,
    VABufferID              *pBufId
);

VAStatus DdiDecode_UnRegisterRTSurfaces(
    VADriverContextP    ctx,
    PDDI_MEDIA_SURFACE surface);

VAStatus DdiDecode_CreateConfig (
    VAProfile           profile,
    VAEntrypoint        entrypoint,
    VAConfigAttrib     *attrib_list,
    int32_t             num_attribs,
    VAConfigID         *config_id
);

VAStatus DdiDecode_BeginPicture (
    VADriverContextP    ctx,
    VAContextID         context,
    VASurfaceID         render_target
);

VAStatus DdiDecode_EndPicture (
    VADriverContextP    ctx,
    VAContextID         context
);

VAStatus DdiDecode_RenderPicture (
    VADriverContextP    ctx,
    VAContextID         context,
    VABufferID         *buffers,
    int32_t             num_buffers
);

VAStatus DdiDecode_CreateContext (
    VADriverContextP    ctx,
    VAConfigID          config_id,
    int32_t             picture_width,
    int32_t             picture_height,
    int32_t             flag,
    VASurfaceID        *render_targets,
    int32_t             num_render_targets,
    VAContextID        *context
);

VAStatus DdiDecode_DestroyContext (
    VADriverContextP    ctx,
    VAContextID         context
);

#endif
