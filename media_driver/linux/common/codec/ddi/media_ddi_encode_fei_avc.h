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
//! \file     media_ddi_encode_fei_avc.h
//! \brief    Defines class for DDI media avc fei encode
//!

#ifndef __MEDIA_DDI_ENCODE_FEI_AVC_H__
#define __MEDIA_DDI_ENCODE_FEI_AVC_H__

#include "media_ddi_encode_avc.h"

//!
//! \class  DdiEncodeAvcFei
//! \brief  Ddi encode AVC FEI
//!
class DdiEncodeAvcFei : public DdiEncodeAvc
{
public:
    //!
    //! \brief   Constructor
    //!
    DdiEncodeAvcFei() {}

    //!
    //! \brief   Destructor
    //!
    virtual ~DdiEncodeAvcFei();

    virtual VAStatus ContextInitialize(PCODECHAL_SETTINGS codecHalSettings);

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
    //! \brief    Add preenc output buffer info to status report queue
    //! \details  Add preenc output buffer info to status report queue,
    //!           this function will be invoked at RenderPicture
    //!
    //! \param    [in] preEncBuf
    //!           Pointer to PreEnc mv buffer
    //! \param    [in] typeIdx
    //!           DDI_ENCODE_PRE_ENC_BUFFER_TYPE
    //!
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if successful, else fail reason
    //!
    VAStatus AddToPreEncStatusReportQueue(
        void                           *preEncBuf,
        DDI_ENCODE_PRE_ENC_BUFFER_TYPE typeIdx);

    //!
    //! \brief    Update position
    //! \details  Update position after add preenc buffer to
    //!           status report queue, this funciton will be
    //!           invoked at RenderPicture
    //!
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if successful, else fail reason
    //!
    VAStatus AddToPreEncStatusReportQueueUpdatePos();

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

    //!
    //! \brief    Parse statistics data to preenc parameters
    //! \details  Parse statistics data to preenc parameters
    //!
    //! \param    [in] mediaCtx
    //!           Pointer to DDI_MEDIA_CONTEXT
    //! \param    [in] ptr
    //!           Pointer to buffer VAStatsStatisticsParameter16x16Intel
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if successful, else fail reason
    //!
    VAStatus ParseStatsParams(
        DDI_MEDIA_CONTEXT *mediaCtx,
        void              *ptr);

private:
    //! \brief H.264 Inverse Quantization Matrix Buffer.
    PCODEC_AVC_IQ_MATRIX_PARAMS iqMatrixParams = nullptr;

    //! \brief H.264 Inverse Quantization Weight Scale.
    PCODEC_AVC_ENCODE_IQ_WEIGTHSCALE_LISTS iqWeightScaleLists = nullptr;
};
#endif /* __MEDIA_DDI_ENCODE_FEI_AVC_H__ */
