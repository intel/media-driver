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
//! \file     media_ddi_decode_vp8.h
//! \brief    Defines DdiDecodeVP8 class for VP8 decode
//!

#ifndef __MEDIA_DDI_DECODER_VP8_H__
#define __MEDIA_DDI_DECODER_VP8_H__

#include <va/va.h>
#include "media_ddi_decode_base.h"

struct _DDI_MEDIA_BUFFER;

//!
//! \class  DdiDecodeVP8
//! \brief  Ddi decode VP8
//!
class DdiDecodeVP8 : public DdiMediaDecode {
public:
    //!
    //! \brief Constructor
    //!
    DdiDecodeVP8(DDI_DECODE_CONFIG_ATTR *ddiDecodeAttr) : DdiMediaDecode(ddiDecodeAttr) { };

    //!
    //! \brief Destructor
    //!
    virtual ~DdiDecodeVP8(){};

    // inherited virtual function
    virtual void DestroyContext (
        VADriverContextP    ctx) override;

    virtual VAStatus RenderPicture (
        VADriverContextP    ctx,
        VAContextID         context,
        VABufferID          *buffers,
        int32_t             numBuffers) override;

    virtual VAStatus SetDecodeParams() override;

    virtual void ContextInit(
        int32_t picWidth,
        int32_t picHeight) override;

    virtual VAStatus CodecHalInit(
        DDI_MEDIA_CONTEXT  *mediaCtx,
        void               *ptr) override;

    virtual VAStatus AllocSliceControlBuffer(
        DDI_MEDIA_BUFFER       *buf) override;

    virtual uint8_t* GetPicParamBuf(
        DDI_CODEC_COM_BUFFER_MGR     *bufMgr) override;

private:
    //!
    //! \brief   ParseIQMatrixParam for VP8
    //! \details parse the QMatrix info required by VP8 decoding
    //!
    //! \param   [in] *mediaCtx
    //!          DDI_MEDIA_CONTEXT
    //! \param   [in] *qMatrix
    //!          VAIQMatrixBufferHEVC
    //!
    //! \return  VA_STATUS_SUCCESS is returned if it is parsed successfully.
    //!          else fail reason
    VAStatus ParseIQMatrix(
        DDI_MEDIA_CONTEXT   *mediaCtx,
        VAIQMatrixBufferVP8 *matrix);

    //!
    //! \brief   ParseSliceParam for VP8
    //! \details parse the sliceParam info required by VP8 decoding for
    //!          each slice
    //!
    //! \param   [in] *mediaCtx
    //!          DDI_MEDIA_CONTEXT
    //! \param   [in] *slcParam
    //!          VASliceParameterBufferVP8
    //!
    //! \return  VA_STATUS_SUCCESS is returned if it is parsed successfully.
    //!          else fail reason
    VAStatus ParseSliceParams(
        DDI_MEDIA_CONTEXT         *mediaCtx,
        VASliceParameterBufferVP8 *slcParam);

    //! \brief   ParsePicParam for VP8
    //! \details parse the PicParam info required by VP8 decoding
    //!
    //! \param   [in] *mediaCtx
    //!          DDI_MEDIA_CONTEXT
    //! \param   [in] *qMatrix
    //!          VAPictureParameterBufferVP8
    //!
    //! \return  VA_STATUS_SUCCESS is returned if it is parsed successfully.
    //!          else fail reason
    VAStatus ParsePicParams(
        DDI_MEDIA_CONTEXT           *mediaCtx,
        VAPictureParameterBufferVP8 *picParam);

    //! \brief   ParseProbabilityData for VP8
    //! \details parse the VP8 Prob table required by VP8 decoding
    //!
    //! \param   [in] *vp8ProbDataBuffer
    //!          struct _DDI_MEDIA_BUFFER
    //! \param   [in] *probBuffer
    //!          VAProbabilityDataBufferVP8
    //!
    //! \return  VA_STATUS_SUCCESS is returned if it is parsed successfully.
    //!          else fail reason
    VAStatus ParseProbabilityData(
        struct _DDI_MEDIA_BUFFER   *vp8ProbDataBuff,
        VAProbabilityDataBufferVP8 *probInputBuf);

    //! \brief   Init Resource buffer for VP8
    //! \details Initialize and allocate the Resource buffer for VP8
    //!
    //! \return  VA_STATUS_SUCCESS is returned if it is parsed successfully.
    //!          else fail reason
    VAStatus InitResourceBuffer(DDI_MEDIA_CONTEXT *mediaCtx);

    //! \brief   Free Resource buffer for VP8
    //!
    void FreeResourceBuffer();

    MOS_RESOURCE m_resNoneRegLastRefFrame;
    MOS_RESOURCE m_resNoneRegGoldenRefFrame;
    MOS_RESOURCE m_resNoneRegAltRefFrame;
};

#endif /* _MEDIA_DDI_DECODE_VP8_H */
