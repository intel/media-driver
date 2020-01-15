/*
* Copyright (c) 2015-2018, Intel Corporation
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
//! \file     media_libva_cp_interface.h
//! \brief    This file defines the C++ class/interface for DDI interface of content protection
//! \details  Other components will call this interface for content protection operations directly
//!

#ifndef  __MEDIA_LIBVA_CP_INTERFACE_H__
#define  __MEDIA_LIBVA_CP_INTERFACE_H__
#include "media_libva.h"
#include "codechal_encoder_base.h"
#include "mos_os.h"

typedef struct _DDI_ENCODE_STATUS_REPORT_INFO *PDDI_ENCODE_STATUS_REPORT_INFO;
class CodechalSetting;
struct CodechalDecodeParams;

class DdiCpInterface
{
public:
    DdiCpInterface(MOS_CONTEXT& mosCtx) {}

    virtual ~DdiCpInterface() {}

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
        DDI_CODEC_COM_BUFFER_MGR*       bufMgr,
        void*              encodeStatusReport);

    virtual void FreeHdcp2Buffer(DDI_CODEC_COM_BUFFER_MGR* bufMgr);

    virtual MOS_STATUS StoreCounterToStatusReport(
        PDDI_ENCODE_STATUS_REPORT_INFO info);

    virtual VAStatus ParseCpParamsForEncode();

    virtual void SetCpFlags(int32_t flag);

    virtual bool IsHdcp2Enabled();

    virtual void SetInputResourceEncryption(PMOS_INTERFACE osInterface, PMOS_RESOURCE resource);

    virtual VAStatus CreateCencDecode(
        CodechalDebugInterface      *debugInterface,
        PMOS_CONTEXT                osContext,
        CodechalSetting *           settings);

    virtual VAStatus SetDecodeParams(CodechalDecodeParams    *decodeParams);
};

//!
//! \brief    Create DdiCpInterface Object according CPLIB loading status
//!           Must use Delete_DdiCpInterface to delete created Object to avoid ULT Memory Leak errors
//!
//! \param    [in] *pMosCtx
//!           MOS_CONTEXT*
//!
//! \return   Return CP Wrapper Object if CPLIB not loaded
//!
DdiCpInterface* Create_DdiCpInterface(MOS_CONTEXT& mosCtx);

//!
//! \brief    Delete the MhwCpInterface Object according CPLIB loading status
//!
//! \param    [in] *pDdiCpInterface 
//!           DdiCpInterface
//!
void Delete_DdiCpInterface(DdiCpInterface* pDdiCpInterface);
#endif

