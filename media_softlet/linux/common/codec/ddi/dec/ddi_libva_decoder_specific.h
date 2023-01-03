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
//! \file      ddi_libva_decoder_specific.h
//! \brief     libva(and its extension) decoder head file
//!

#ifndef __DDI_LIBVA_DECODER_SPECIFIC_H__
#define __DDI_LIBVA_DECODER_SPECIFIC_H__

#include "media_libva.h"
#include "ddi_cp_interface_next.h"
#include "media_libva_cp_interface.h"
#include "ddi_vp_functions.h"

namespace decode
{

#define DDI_DECODE_SFC_MAX_WIDTH       4096
#define DDI_DECODE_SFC_MAX_HEIGHT      4096
#define DDI_DECODE_SFC_MIN_WIDTH       128
#define DDI_DECODE_SFC_MIN_HEIGHT      128
#define DDI_DECODE_HCP_SFC_MAX_WIDTH   (16*1024)
#define DDI_DECODE_HCP_SFC_MAX_HEIGHT  (16*1024)
//!
//! \struct DDI_DECODE_CONFIG_ATTR_NEXT
//! \brief  Ddi decode configuration attribute
//!
struct DDI_DECODE_CONFIG_ATTR_NEXT
{
    VAProfile     profile;
    VAEntrypoint  entrypoint;
    uint32_t      uiDecSliceMode;
    uint32_t      uiEncryptionType;
    uint32_t      uiDecProcessingType;
};

typedef struct DDI_DECODE_CONFIG_ATTR_NEXT *PDDI_DECODE_CONFIG_ATTR_NEXT;

class DdiDecodeBase;

//!
//! \struct DDI_DECODE_CONTEXT
//! \brief  Ddi decode context
//!
struct DDI_DECODE_CONTEXT
{
    // Decoder data related with the specific codec
    // The instance of DdiDecodeXXX. For example: DdiDecodeAvc, DdiDecodeJpeg
    DdiDecodeBase                 *m_ddiDecodeNext;
    // Decoder private data
    DdiCpInterface                *pCpDdiInterface;
    DdiCpInterfaceNext            *pCpDdiInterfaceNext;
    DdiVpFunctions                *pVpDdiInterface;
    // Parameters
    CodechalDecodeParams          DecodeParams;
    uint16_t                      wMode; // Get the info during hand shaking
    Codechal                      *pCodecHal;
    bool                          bShortFormatInUse;
    bool                          bDecodeModeReported;
    VASurfaceDecodeMBErrors       vaSurfDecErrOutput[2];
    DDI_CODEC_RENDER_TARGET_TABLE RTtbl;
    DDI_CODEC_COM_BUFFER_MGR      BufMgr;
    PDDI_MEDIA_CONTEXT            pMediaCtx;
    // Add a list to track DPB.
    VASurfaceID                   RecListSurfaceID[CODEC_AVC_NUM_UNCOMPRESSED_SURFACE];
    uint32_t                      dwSliceParamBufNum;
    uint32_t                      dwSliceCtrlBufNum;
    uint32_t                      uiDecProcessingType;
};

typedef struct DDI_DECODE_CONTEXT *PDDI_DECODE_CONTEXT;

static __inline PDDI_DECODE_CONTEXT GetDecContextFromPVOID (void *decCtx)
{
    return (PDDI_DECODE_CONTEXT)decCtx;
}
} // namespace decode
#endif // __DDI_LIBVA_DECODER_SPECIFIC_H__
