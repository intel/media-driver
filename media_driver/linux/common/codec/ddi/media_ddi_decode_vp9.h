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
//! \brief    Defines DdiDecodeVP9 class for VP9 decode
//!

#ifndef __MEDIA_DDI_DECODER_VP9_H__
#define __MEDIA_DDI_DECODER_VP9_H__

#include <va/va.h>
#include "media_ddi_decode_base.h"

//!
//! \class  DdiDecodeVP9
//! \brief  Ddi decode VP9
//!
class DdiDecodeVP9 : public DdiMediaDecode
{
public:
    //!
    //! \brief Constructor
    //!
    DdiDecodeVP9(DDI_DECODE_CONFIG_ATTR *ddiDecodeAttr) : DdiMediaDecode(ddiDecodeAttr){m_withDpb = false;};

    //!
    //! \brief Destructor
    //!
    virtual ~DdiDecodeVP9(){};

    // inherited virtual functions
    virtual void DestroyContext(
        VADriverContextP ctx) override;

    virtual VAStatus RenderPicture(
        VADriverContextP ctx,
        VAContextID      context,
        VABufferID       *buffers,
        int32_t          numBuffers) override;

    virtual VAStatus InitDecodeParams(
        VADriverContextP ctx,
        VAContextID      context) override;

    virtual VAStatus SetDecodeParams() override;
    
    virtual MOS_FORMAT GetFormat() override;

    /*virtual VAStatus EndPicture(
        VADriverContextP ctx,
        VAContextID      context) override;*/

    virtual void ContextInit(
        int32_t picWidth,
        int32_t picHeight) override;

    virtual VAStatus CodecHalInit(
        DDI_MEDIA_CONTEXT *mediaCtx,
        void              *ptr) override;

    virtual VAStatus AllocSliceControlBuffer(
        DDI_MEDIA_BUFFER       *buf) override;

    virtual uint8_t* GetPicParamBuf(
        DDI_CODEC_COM_BUFFER_MGR     *bufMgr) override;

private:
    //!
    //! \brief   ParseSliceParam for VP9
    //! \details parse the sliceParam info required by VP9 decoding for
    //!          each slice
    //!
    //! \param   [in] *mediaCtx
    //!          DDI_MEDIA_CONTEXT
    //! \param   [in] *slcParam
    //!          VASliceParameterBufferVP9
    //!
    //! \return  VA_STATUS_SUCCESS is returned if it is parsed successfully.
    //!          else fail reason
    VAStatus ParseSliceParams(
        DDI_MEDIA_CONTEXT         *mediaCtx,
        VASliceParameterBufferVP9 *slcParam);

    //! \brief   ParsePicParam for VP9
    //! \details parse the PicParam info required by VP9 decoding
    //!
    //! \param   [in] *mediaCtx
    //!          DDI_MEDIA_CONTEXT
    //! \param   [in] *qMatrix
    //!          VAIQMatrixBufferH264
    //!
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if success, else fail reason
    //! \return  VA_STATUS_SUCCESS is returned if it is parsed successfully.
    //!          else fail reason
    VAStatus ParsePicParams(
        DDI_MEDIA_CONTEXT              *mediaCtx,
        VADecPictureParameterBufferVP9 *picParam);

    //! \brief   Init Resource buffer for VP9
    //! \details Initialize and allocate the Resource buffer for VP9
    //!
    //! \return  VA_STATUS_SUCCESS is returned if it is parsed successfully.
    //!          else fail reason
    VAStatus InitResourceBuffer();

    //! \brief   Free Resource buffer for VP9
    //!
    void FreeResourceBuffer();

    //! \brief   the flag of slice data. It indicates whether slc data is passed
    bool slcFlag = false;
};

#endif /* _MEDIA_DDI_DECODE_VP9_H */
