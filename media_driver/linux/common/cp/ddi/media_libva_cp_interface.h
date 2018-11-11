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
#include "cplib.h"
#include "mos_os.h"

typedef struct _DDI_ENCODE_STATUS_REPORT_INFO *PDDI_ENCODE_STATUS_REPORT_INFO;
class CodechalSetting;
struct CodechalDecodeParams;

class DdiCpInterface
{
public:
    DdiCpInterface(MOS_CONTEXT& mosCtx) {}

    virtual ~DdiCpInterface() {}

    //! \brief    set HostEncryptMode in EncryptionParams
    //! \param    [in] encryptionType
    //!           input type from outside
    //!
    //! \param    [out]setting 
    //!           CodechalSetting
    //!
    virtual void SetCpParams(uint32_t encryptionType, CodechalSetting *setting);

    //! \brief    End picture process 
    //! \param    [in] vaDrvCtx
    //!           
    //! \param    [in] contextId
    //!           input va context id to get decode ctx
    //! \return   VAStatus
    //!           return VA_STATUS_SUCCESS if succeed, otherwise failed
    virtual VAStatus EndPictureCenc(
        VADriverContextP vaDrvCtx,
        VAContextID      contextId);

    //! \brief    query status
    //! \param    [in] ctx
    //!           input driver ctx to get decode ctx
    //! \param    [in] context
    //!           input ctx id to get decode ctx
    //! \param    [in] bufferdata
    //!           input buffer, queried status address
    //! \param    [in] data_size
    //!           status data size
    //! \return   VAStatus
    //!           return VA_STATUS_SUCCESS if succeed, otherwise failed
    virtual VAStatus QueryCencStatus(
        VADriverContextP    ctx,
        VASurfaceStatus     *status);

    //! \brief    render picture process 
    //! \param    [in] vaDrvCtx
    //!           
    //! \param    [in] contextId
    //!           input va context id to get decode ctx
    //! \param    [in] buf
    //!           input media buffer
    //! \param    [in] data
    //!           input data contain parameters
    //! \return   VA_STATUS
    //!           return VA_STATUS_SUCCESS if succeed, otherwise failed
    virtual VAStatus RenderCencPicture(
        VADriverContextP      vaDrvctx,
        VAContextID           contextId,
        DDI_MEDIA_BUFFER      *buf,
        void                  *data);

    //! \brief    create buffer for CP related buffer types
    //! \param    [in] type
    //!           buffer type to be created 
    //! \param    [in] buffer
    //!           pointer to general buffer
    //! \param    [in] size
    //!           size of general buffer size
    //! \param    [in] num_elements
    //!           number of general buffer element
    //! \return   VA_STATUS
    //!           return VA_STATUS_SUCCESS if succeed, otherwise failed
    virtual VAStatus CreateBuffer(
        VABufferType             type,
        DDI_MEDIA_BUFFER*        buffer,
        uint32_t                 size,
        uint32_t                 num_elements);

    //! \brief    check input buffer type is supported or not for codec
    //! \param    [in] type
    //!           buffer typer
    //! \return   bool
    //!           return true if supported, otherwise false
    static bool CheckSupportedBufferForCodec(VABufferType type);

    //! \brief    Allocate and initialize HDCP2 buffer for encode status report
    //! \param    [in] bufMgr
    //!           input buffer manager to add hdcp2 buffer.
    //! \return   VAStatus
    //!           return VA_STATUS_SUCCESS if succeed, otherwise failed
    virtual VAStatus InitHdcp2Buffer(DDI_CODEC_COM_BUFFER_MGR* bufMgr);

    //! \brief    fill the HDCP buffer based on status report
    //! \param    [in] bufMgr
    //!           input BufMgr to fill in.
    //! \param    [in] info
    //!           input info to get status report info.
    //! \return   VAStatus
    //!           return VA_STATUS_SUCCESS if succeed, otherwise failed
    virtual VAStatus StatusReportForHdcp2Buffer(
        DDI_CODEC_COM_BUFFER_MGR*       bufMgr,
        void*              encodeStatusReport);

    //! \brief    Allocate and initialize HDCP2 buffer for encode status report
    //! \param    [in] bufMgr
    //!           status report buffer manager to be added.
    //! \return   void
    virtual void FreeHdcp2Buffer(DDI_CODEC_COM_BUFFER_MGR* bufMgr);

    //! \brief    store counter in status report so App can get correct values by the order in which they are added
    //! \param    [in] info
    //!           input DDI_ENCODE_STATUS_REPORT_INFO to be writed
    //! \return   MOS_STATUS
    //!           return MOS_STATUS_SUCCESS if succeed, otherwise failed
    virtual MOS_STATUS StoreCounterToStatusReport(
        PDDI_ENCODE_STATUS_REPORT_INFO info);

    //! \brief    parse EncryptionParams in decoder
    //! \return   VA_STATUS
    //!           return VA_STATUS_SUCCESS if succeed, otherwise failed
    virtual VAStatus ParseCpParamsForEncode();

    virtual void SetHdcp2Enabled(int32_t flag);

    //! \brief    Is Hdcp2Enabled for encoder
    //! \return   [in] bool
    //!           true if hdcp2 enabled, otherwise false
    virtual bool IsHdcp2Enabled();

    //! \brief    Set cp tag for an input resource
    //! \param    [in] osInterface
    //!           pointer of mos interface
    //! \param    [in] resource
    //!           pointer of the os resource
    //! \return
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

