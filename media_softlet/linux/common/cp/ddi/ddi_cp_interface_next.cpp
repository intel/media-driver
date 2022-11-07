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
//! \file     ddi_cp_interface_next.cpp
//! \brief    The class implementation of DdiCpInterfaceNext
//!

#include "ddi_cp_interface_next.h"
#include "media_libva_util_next.h"
#include "media_libva_decoder.h"
#include "media_libva_vp.h"
#include "cp_interfaces.h"

static void DdiCpInterfaceNextStubMessage()
{
    DDI_CP_NORMALMESSAGE("This function is stubbed as CP is not enabled.");
}

DdiCpInterfaceNext *CreateDdiCp()
{
    DdiCpInterfaceNext *ddiCp = nullptr;
    ddiCp = DdiCpFactory::Create(DdiCpCreateType::CreateDdiCpInterface);
    if(ddiCp == nullptr)
    {
        DDI_CP_ASSERTMESSAGE("Create Cp instance failed.");
    }
    
    return ddiCp;
}

void DdiCpInterfaceNext::SetCpParams(
    uint32_t         encryptionType,
    CodechalSetting *setting)
{
    DdiCpInterfaceNextStubMessage();
}

VAStatus DdiCpInterfaceNext::RenderCencPicture(
    VADriverContextP  vaDrvctx,
    VAContextID       contextId,
    DDI_MEDIA_BUFFER *buf,
    void             *data)
{
    DdiCpInterfaceNextStubMessage();
    return VA_STATUS_ERROR_UNSUPPORTED_BUFFERTYPE;
}

VAStatus DdiCpInterfaceNext::CreateBuffer(
    VABufferType             type,
    DDI_MEDIA_BUFFER*        buffer,
    uint32_t                 size,
    uint32_t                 num_elements
)
{
    DdiCpInterfaceNextStubMessage();
    return VA_STATUS_ERROR_UNSUPPORTED_BUFFERTYPE;
}

VAStatus DdiCpInterfaceNext::InitHdcp2Buffer(
    DDI_CODEC_COM_BUFFER_MGR* bufMgr)
{
    DdiCpInterfaceNextStubMessage();
    return VA_STATUS_SUCCESS;
}

VAStatus DdiCpInterfaceNext::StatusReportForHdcp2Buffer(
    DDI_CODEC_COM_BUFFER_MGR *bufMgr,
    void                     *encodeStatusReport)
{
    DdiCpInterfaceNextStubMessage();
    return VA_STATUS_SUCCESS;
}

void DdiCpInterfaceNext::FreeHdcp2Buffer(
    DDI_CODEC_COM_BUFFER_MGR* bufMgr)
{
    DdiCpInterfaceNextStubMessage();
    return ;
}

MOS_STATUS DdiCpInterfaceNext::StoreCounterToStatusReport(
    encode::DDI_ENCODE_STATUS_REPORT_INFO* info)
{
    DdiCpInterfaceNextStubMessage();
    return MOS_STATUS_SUCCESS;
}

void DdiCpInterfaceNext::SetInputResourceEncryption(
    PMOS_INTERFACE osInterface,
    PMOS_RESOURCE  resource)
{
    DdiCpInterfaceNextStubMessage();
}

VAStatus DdiCpInterfaceNext::ParseCpParamsForEncode()
{
    DdiCpInterfaceNextStubMessage();
    return VA_STATUS_SUCCESS;
}

void DdiCpInterfaceNext::SetCpFlags(int32_t flag)
{
    DdiCpInterfaceNextStubMessage();
}

bool DdiCpInterfaceNext::IsHdcp2Enabled()
{
    DdiCpInterfaceNextStubMessage();
    return false;
}

VAStatus DdiCpInterfaceNext::CreateCencDecode(
    CodechalDebugInterface      *debugInterface,
    PMOS_CONTEXT                osContext,
    CodechalSetting *           settings)
{
    DdiCpInterfaceNextStubMessage();
    return VA_STATUS_SUCCESS;
}

VAStatus DdiCpInterfaceNext::SetDecodeParams(
    DDI_DECODE_CONTEXT *ddiDecodeContext,
    CodechalSetting *setting)
{
    DdiCpInterfaceNextStubMessage();
    return VA_STATUS_SUCCESS;
}

bool DdiCpInterfaceNext::IsCencProcessing()
{
    DdiCpInterfaceNextStubMessage();
    return false;
}

VAStatus DdiCpInterfaceNext::EndPicture(
    VADriverContextP    ctx,
    VAContextID         context
)
{
    DdiCpInterfaceNextStubMessage();
    return VA_STATUS_SUCCESS;
}

VAStatus DdiCpInterfaceNext::IsAttachedSessionAlive()
{
    DdiCpInterfaceNextStubMessage();
    return VA_STATUS_SUCCESS;
}
