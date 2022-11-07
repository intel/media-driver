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
//! \file     ddi_cp_interface_next.h
//! \brief    This file defines the C++ class/interface for DDI interface of content protection
//! \details  Other components will call this interface for content protection operations directly
//!

#ifndef  __DDI_CP_INTERFACE_NEXT_H__
#define  __DDI_CP_INTERFACE_NEXT_H__

#include <map>
#include "media_libva.h"
#include "media_factory.h"
#include "mos_os.h"

struct CodechalDecodeParams;
struct DDI_DECODE_CONTEXT;
class CodechalSetting;
class DdiCpInterfaceNext;

namespace encode
{
typedef struct _DDI_ENCODE_STATUS_REPORT_INFO DDI_ENCODE_STATUS_REPORT_INFO;
}

//core structure for CP DDI
typedef struct _DDI_CP_CONTEXT_NEXT
{
    PDDI_MEDIA_CONTEXT              pMediaCtx;
    DdiCpInterfaceNext             *pCpDdiInterface;
    std::shared_ptr<void>           pDrvPrivate;
    std::multimap<uint32_t, void *> mapAttaching;
} DDI_CP_CONTEXT_NEXT, *PDDI_CP_CONTEXT_NEXT;

static __inline PDDI_CP_CONTEXT_NEXT DdiCp_GetCpContextNextFromPVOID(void *cpCtx)
{
    return (PDDI_CP_CONTEXT_NEXT)cpCtx;
}

class DdiCpInterfaceNext
{
public:
    DdiCpInterfaceNext(MOS_CONTEXT& mosCtx) {}

    virtual ~DdiCpInterfaceNext() {}

    virtual void SetCpParams(uint32_t encryptionType, CodechalSetting *setting);

    virtual VAStatus RenderCencPicture(
        VADriverContextP      vaDrvctx,
        VAContextID           contextId,
        DDI_MEDIA_BUFFER      *buf,
        void                  *data);

    virtual VAStatus CreateBuffer(
        VABufferType             type,
        DDI_MEDIA_BUFFER*        buffer,
        uint32_t                 size,
        uint32_t                 num_elements);

    virtual VAStatus InitHdcp2Buffer(DDI_CODEC_COM_BUFFER_MGR* bufMgr);

    virtual VAStatus StatusReportForHdcp2Buffer(
        DDI_CODEC_COM_BUFFER_MGR *bufMgr,
        void                     *encodeStatusReport);

    virtual void FreeHdcp2Buffer(DDI_CODEC_COM_BUFFER_MGR* bufMgr);

    virtual MOS_STATUS StoreCounterToStatusReport(
        encode::DDI_ENCODE_STATUS_REPORT_INFO* info);

    virtual VAStatus ParseCpParamsForEncode();

    virtual void SetCpFlags(int32_t flag);

    virtual bool IsHdcp2Enabled();

    virtual void SetInputResourceEncryption(
        PMOS_INTERFACE osInterface,
        PMOS_RESOURCE  resource);

    virtual VAStatus CreateCencDecode(
        CodechalDebugInterface *debugInterface,
        PMOS_CONTEXT            osContext,
        CodechalSetting        *settings);

    virtual VAStatus SetDecodeParams(
        DDI_DECODE_CONTEXT *ddiDecodeContext,
        CodechalSetting    *setting);

    virtual bool IsCencProcessing();

    virtual VAStatus EndPicture(
        VADriverContextP ctx,
        VAContextID      context);

    virtual VAStatus IsAttachedSessionAlive();

MEDIA_CLASS_DEFINE_END(DdiCpInterfaceNext)
};

enum DdiCpCreateType
{
    CreateDdiCpInterface = 0,
    CreateDdiCpImp       = 1
};

typedef MediaFactory<DdiCpCreateType, DdiCpInterfaceNext> DdiCpFactory;

DdiCpInterfaceNext *CreateDdiCp();

#endif
