/*
* Copyright (c) 2015-2020, Intel Corporation
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
//! \file     media_libva_cp_interface.cpp
//! \brief    The class implementation of DdiCpInterface
//!

#include "media_libva_util.h"
#include "media_libva_decoder.h"
#include "media_libva_encoder.h"
#include "media_libva_cp_interface.h"
#include "media_libva_vp.h"
#include "cp_interfaces.h"
#include <typeinfo>

static void DdiStubMessage()
{
    MOS_NORMALMESSAGE(
        MOS_COMPONENT_CP,
        MOS_CP_SUBCOMP_DDI,
        "This function is stubbed as CP is not enabled.");
}

DdiCpInterface* Create_DdiCpInterface(MOS_CONTEXT& mosCtx)
{
    DdiCpInterface* pInterface = nullptr;
    CpInterfaces *cp_interface = CpInterfacesFactory::Create(CP_INTERFACE);
    if (cp_interface)
    {
        pInterface = cp_interface->Create_DdiCpInterface(mosCtx);
        MOS_Delete(cp_interface);
    }

    if (nullptr == pInterface) DdiStubMessage();

    return nullptr == pInterface ? MOS_New(DdiCpInterface, mosCtx) : pInterface;
}

void Delete_DdiCpInterface(DdiCpInterface* pInterface)
{
    CpInterfaces *cp_interface = CpInterfacesFactory::Create(CP_INTERFACE);
    if (pInterface != nullptr && cp_interface != nullptr)
    {
        cp_interface->Delete_DdiCpInterface(pInterface);
        pInterface = nullptr;
    }
    MOS_Delete(pInterface);
    MOS_Delete(cp_interface);
}

void DdiCpInterface::SetCpParams(uint32_t encryptionType, CodechalSetting *setting)
{
    DdiStubMessage();
}

VAStatus DdiCpInterface::RenderCencPicture(
    VADriverContextP      vaDrvctx,
    VAContextID           contextId,
    DDI_MEDIA_BUFFER      *buf,
    void                  *data)
{
    DdiStubMessage();
    return VA_STATUS_ERROR_UNSUPPORTED_BUFFERTYPE;
}

VAStatus DdiCpInterface::CreateBuffer(
    VABufferType             type,
    DDI_MEDIA_BUFFER*        buffer,
    uint32_t                 size,
    uint32_t                 num_elements
)
{
    DdiStubMessage();
    return VA_STATUS_ERROR_UNSUPPORTED_BUFFERTYPE;
}

VAStatus DdiCpInterface::InitHdcp2Buffer(DDI_CODEC_COM_BUFFER_MGR* bufMgr)
{
    DdiStubMessage();
    return VA_STATUS_SUCCESS;
}

VAStatus DdiCpInterface::StatusReportForHdcp2Buffer(
    DDI_CODEC_COM_BUFFER_MGR*       bufMgr,
    void*               encodeStatusReport)
{
    DdiStubMessage();
    return VA_STATUS_SUCCESS;
}

void DdiCpInterface::FreeHdcp2Buffer(DDI_CODEC_COM_BUFFER_MGR* bufMgr)
{
    DdiStubMessage();
    return ;
}

MOS_STATUS DdiCpInterface::StoreCounterToStatusReport(
    PDDI_ENCODE_STATUS_REPORT_INFO info)
{
    DdiStubMessage();
    return MOS_STATUS_SUCCESS;
}

void DdiCpInterface::SetInputResourceEncryption(PMOS_INTERFACE osInterface, PMOS_RESOURCE resource)
{
    DdiStubMessage();
}

VAStatus DdiCpInterface::ParseCpParamsForEncode()
{
    DdiStubMessage();
    return VA_STATUS_SUCCESS;
}

void DdiCpInterface::SetCpFlags(int32_t flag)
{
    DdiStubMessage();
}

bool DdiCpInterface::IsHdcp2Enabled()
{
    DdiStubMessage();
    return false;
}

VAStatus DdiCpInterface::CreateCencDecode(
    CodechalDebugInterface      *debugInterface,
    PMOS_CONTEXT                osContext,
    CodechalSetting *           settings)
{
    DdiStubMessage();
    return VA_STATUS_SUCCESS;
}

VAStatus DdiCpInterface::SetDecodeParams(
    DDI_DECODE_CONTEXT *ddiDecodeContext,
    CodechalSetting *setting)
{
    DdiStubMessage();
    return VA_STATUS_SUCCESS;
}

bool DdiCpInterface::IsCencProcessing()
{
    DdiStubMessage();
    return false;
}

VAStatus DdiCpInterface::EndPicture(
    VADriverContextP    ctx,
    VAContextID         context
)
{
    DdiStubMessage();
    return VA_STATUS_SUCCESS;
}

VAStatus DdiCpInterface::IsAttachedSessionAlive()
{
    DdiStubMessage();
    return VA_STATUS_SUCCESS;
}
