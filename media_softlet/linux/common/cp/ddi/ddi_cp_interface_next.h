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
#include "cp_factory.h"
#include "mos_os.h"
#include "media_libva_util_next.h"

struct CodechalDecodeParams;
class CodechalSetting;
class DdiCpInterfaceNext;

namespace encode
{
typedef struct _DDI_ENCODE_STATUS_REPORT_INFO *PDDI_ENCODE_STATUS_REPORT_INFO;
}

namespace decode
{
struct DDI_DECODE_CONTEXT;
}

//core structure for CP DDI
typedef struct _DDI_CP_CONTEXT_NEXT
{
    PDDI_MEDIA_CONTEXT              pMediaCtx;
    DdiCpInterfaceNext             *pCpDdiInterface;
    std::shared_ptr<void>           pDrvPrivate;
    std::multimap<uint32_t, void *> mapAttaching;
    PERF_DATA                       perfData;
} DDI_CP_CONTEXT_NEXT, *PDDI_CP_CONTEXT_NEXT;

static __inline PDDI_CP_CONTEXT_NEXT GetCpContextNextFromPVOID(void *cpCtx)
{
    return (PDDI_CP_CONTEXT_NEXT)cpCtx;
}

#define DDI_CP_STUB_MESSAGE DDI_CP_NORMALMESSAGE("This function is stubbed as CP is not enabled.")
class DdiCpInterfaceNext
{
public:
    DdiCpInterfaceNext(MOS_CONTEXT& mosCtx) {}

    virtual ~DdiCpInterfaceNext() {}

    virtual void SetCpParams(uint32_t encryptionType, CodechalSetting *setting) 
    {
        DDI_CP_STUB_MESSAGE;
    }

    virtual VAStatus RenderCencPicture(
        VADriverContextP      vaDrvctx,
        VAContextID           contextId,
        DDI_MEDIA_BUFFER      *buf,
        void                  *data)
    {
        DDI_CP_STUB_MESSAGE;
        return VA_STATUS_ERROR_UNSUPPORTED_BUFFERTYPE;
    }

    virtual VAStatus CreateBuffer(
        VABufferType             type,
        DDI_MEDIA_BUFFER*        buffer,
        uint32_t                 size,
        uint32_t                 num_elements)
    {
        DDI_CP_STUB_MESSAGE;
        return VA_STATUS_ERROR_UNSUPPORTED_BUFFERTYPE;
    }

    virtual VAStatus InitHdcp2Buffer(DDI_CODEC_COM_BUFFER_MGR* bufMgr)
    {
        DDI_CP_STUB_MESSAGE;
        return VA_STATUS_SUCCESS;
    }

    virtual VAStatus StatusReportForHdcp2Buffer(
        DDI_CODEC_COM_BUFFER_MGR *bufMgr,
        void                     *encodeStatusReport)
    {
        DDI_CP_STUB_MESSAGE;
        return VA_STATUS_SUCCESS;
    }

    virtual void FreeHdcp2Buffer(DDI_CODEC_COM_BUFFER_MGR *bufMgr)
    {
        DDI_CP_STUB_MESSAGE;
        return;
    }

    virtual MOS_STATUS StoreCounterToStatusReport(
        encode::PDDI_ENCODE_STATUS_REPORT_INFO info)
    {
        DDI_CP_STUB_MESSAGE;
        return MOS_STATUS_SUCCESS;
    }

    virtual VAStatus ParseCpParamsForEncode()
    {
        DDI_CP_STUB_MESSAGE;
        return VA_STATUS_SUCCESS;
    }

    virtual void SetCpFlags(int32_t flag)
    {
        DDI_CP_STUB_MESSAGE;
    }

    virtual bool IsHdcp2Enabled()
    {
        DDI_CP_STUB_MESSAGE;
        return false;
    }

    virtual void SetInputResourceEncryption(
        PMOS_INTERFACE osInterface,
        PMOS_RESOURCE  resource)
    {
        DDI_CP_STUB_MESSAGE;
    }

    virtual VAStatus CreateCencDecode(
        CodechalDebugInterface *debugInterface,
        PMOS_CONTEXT            osContext,
        CodechalSetting        *settings)
    {
        DDI_CP_STUB_MESSAGE;
        return VA_STATUS_SUCCESS;
    }

    virtual VAStatus SetDecodeParams(
        decode::DDI_DECODE_CONTEXT *ddiDecodeContext,
        CodechalSetting    *setting)
    {
        DDI_CP_STUB_MESSAGE;
        return VA_STATUS_SUCCESS;
    }

    virtual bool IsCencProcessing()
    {
        DDI_CP_STUB_MESSAGE;
        return false;
    }

    virtual VAStatus EndPicture(
        VADriverContextP ctx,
        VAContextID      context)
    {
        DDI_CP_STUB_MESSAGE;
        return VA_STATUS_SUCCESS;
    }

    virtual VAStatus IsAttachedSessionAlive()
    {
        DDI_CP_STUB_MESSAGE;
        return VA_STATUS_SUCCESS;
    }

MEDIA_CLASS_DEFINE_END(DdiCpInterfaceNext)
};

enum DdiCpCreateType
{
    CreateDdiCpInterface = 0,
    CreateDdiCpImp       = 1
};

typedef CpFactory<DdiCpInterfaceNext, MOS_CONTEXT&> DdiCpFactory;

DdiCpInterfaceNext* CreateDdiCpNext(MOS_CONTEXT* mosCtx);

#endif
