/*
* Copyright (c) 2017, Intel Corporation
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
//! \file     media_ddi_encode_fei_hevc.h
//! \brief    Defines class for DDI media hevc fei encode
//!

#ifndef __MEDIA_DDI_ENCODE_FEI_HEVC_H__
#define __MEDIA_DDI_ENCODE_FEI_HEVC_H__

#include "media_ddi_encode_hevc.h"

//!
//! \class  DdiEncodeHevcFei
//! \brief  Ddi encode HEVC FEI
//!
class DdiEncodeHevcFei : public DdiEncodeHevc
{
public:
    //!
    //! \brief   Constructor
    //!
    DdiEncodeHevcFei() {}

    //!
    //! \brief   Destructor
    //!
    virtual ~DdiEncodeHevcFei();

    virtual VAStatus ContextInitialize(CodechalSetting * codecHalSettings);

    virtual VAStatus EncodeInCodecHal(uint32_t numSlices);

    virtual VAStatus ResetAtFrameLevel();

    virtual VAStatus RenderPicture(
        VADriverContextP ctx,
        VAContextID      context,
        VABufferID       *buffers,
        int32_t          numBuffers);

    //!
    //! \brief    Add ENC output buffer information
    //! \details  Add ENC output buffer information to
    //!           status report queue. This function will
    //!           be invoked at RenderPicture
    //!
    //! \param    [in] encBuf
    //!           Pointer to ENC mv output buffer
    //! \param    [in] typeIdx
    //!           DDI_ENCODE_FEI_ENC_BUFFER_TYPE
    //!
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if successful, else fail reason
    //!
    VAStatus AddToEncStatusReportQueue(
        void                           *encBuf,
        DDI_ENCODE_FEI_ENC_BUFFER_TYPE typeIdx);

    //!
    //! \brief    Update position
    //! \details  Update position after add ENC output buffer
    //!           to status report queue. This function will be
    //!           invoked at RenderPicture
    //!
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if successful, else fail reason
    //!
    VAStatus AddToEncStatusReportQueueUpdatePos();

    //!
    //! \brief    Parse FEI picture parameters
    //! \details  Parse FEI picture parameters
    //!
    //! \param    [in] data
    //!           Pointer to buffer VAEncMiscParameterFEIFrameControlH264Intel
    //!
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if successful, else fail reason
    //!
    VAStatus ParseMiscParamFeiPic(void *data);

    virtual VAStatus ParseMiscParams(void *ptr);

};
#endif /* __MEDIA_DDI_ENCODE_FEI_HEVC_H__ */
