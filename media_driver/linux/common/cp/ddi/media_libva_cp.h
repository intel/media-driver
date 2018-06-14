/*
* Copyright (c) 2015-2017, Intel Corporation
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
//! \file     media_libva_cp.h
//! \brief    DDI interface for content protection parameters and operations.
//! \details  Other components will call this interface for content protection operations directly
//!

#ifndef  __MEDIA_LIBVA_CP_H__
#define  __MEDIA_LIBVA_CP_H__
#include "media_libva.h"
#include "codechal_encoder_base.h"

typedef struct _DDI_ENCODE_STATUS_REPORT_INFO *PDDI_ENCODE_STATUS_REPORT_INFO;
class CodechalSetting;

class DdiCpInterface
{
public:
    DdiCpInterface(MOS_CONTEXT& mosCtx);

    ~DdiCpInterface();

    void SetCpParams(uint32_t encryptionType, CodechalSetting *setting);

    VAStatus EndPictureCenc(
        VADriverContextP vaDrvCtx,
        VAContextID      contextId);

    VAStatus QueryCencStatus(
        VADriverContextP    ctx,
        VASurfaceStatus     *status);

    VAStatus RenderCencPicture(
        VADriverContextP      vaDrvctx,
        VAContextID           contextId,
        DDI_MEDIA_BUFFER      *buf,
        void                  *data);

    VAStatus CreateBuffer(
        VABufferType             type,
        DDI_MEDIA_BUFFER*        buffer,
        uint32_t                 size,
        uint32_t                 num_elements);

    static bool CheckSupportedBufferForCodec(VABufferType type);

    VAStatus InitHdcp2Buffer(DDI_CODEC_COM_BUFFER_MGR* bufMgr);

    VAStatus StatusReportForHdcp2Buffer(
        DDI_CODEC_COM_BUFFER_MGR*       bufMgr,
        void*              encodeStatusReport);

    void FreeHdcp2Buffer(DDI_CODEC_COM_BUFFER_MGR* bufMgr);

    MOS_STATUS StoreCounterToStatusReport(
        PDDI_ENCODE_STATUS_REPORT_INFO info);

    VAStatus ParseCpParamsForEncode();

    void SetHdcp2Enabled(int32_t flag);

    bool IsHdcp2Enabled();

    void ResetCpContext();

    VAStatus RenderPictureForVp(
        VADriverContextP      vaDrvCtx,
        VAContextID           vpCtxID,
        DDI_MEDIA_BUFFER      *buf,
        void                  *data);

    static bool CheckSupportedBufferForVp(VABufferType type);

    VAStatus CreateCencDecode(
        CodechalDecode              *decoder,
        PMOS_CONTEXT                osContext,
        CodechalSetting *           settings);
};
#endif

