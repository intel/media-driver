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
//! \file     media_ddi_decode_mpeg2.h
//! \brief    Defines DdiDecodeMPEG2 class for MPEG2 decode
//!

#ifndef __MEDIA_DDI_DECODER_MPEG2_H__
#define __MEDIA_DDI_DECODER_MPEG2_H__

#include <va/va.h>
#include "media_ddi_decode_base.h"

//!
//! \class  DdiDecodeMPEG2
//! \brief  Ddi decode MPEG2
//!
class DdiDecodeMPEG2 : public DdiMediaDecode
{
public:
    //!
    //! \brief Constructor
    //!
    DdiDecodeMPEG2(DDI_DECODE_CONFIG_ATTR *ddiDecodeAttr) : DdiMediaDecode(ddiDecodeAttr){};

    //!
    //! \brief Destructor
    //!
    virtual ~DdiDecodeMPEG2(){};

    // inherited virtual functions
    virtual void DestroyContext(
        VADriverContextP ctx) override;

    virtual VAStatus RenderPicture(
        VADriverContextP ctx,
        VAContextID      context,
        VABufferID       *buffers,
        int32_t          num_buffers) override;

    virtual VAStatus SetDecodeParams() override;

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
    //! \brief   ParaSliceParam for MPEG2
    //! \details parse the sliceParam info required by MPEG2 decoding for
    //!          each slice
    //!
    //! \param   [in] *mediaCtx
    //!          DDI_MEDIA_CONTEXT
    //! \param   [in] *slcParam
    //!          VASliceParameterBufferMPEG2
    //! \param   [in] numSlices
    //!             uint32_t
    //!
    //! \return  VA_STATUS_SUCCESS is returned if it is parsed successfully.
    //!          else fail reason
    VAStatus ParseSliceParams(
        DDI_MEDIA_CONTEXT            *mediaCtx,
        VASliceParameterBufferMPEG2  *slcParam,
        uint32_t                      numSlices);

    //!
    //! \brief   ParseIQMatrixParam for MPEG2
    //! \details parse the QMatrix info required by MPEG2 decoding
    //!
    //! \param   [in] *mediaCtx
    //!          DDI_MEDIA_CONTEXT
    //! \param   [in] *qMatrix
    //!          VAIQMatrixBufferMPEG2
    //!
    //! \return  VA_STATUS_SUCCESS is returned if it is parsed successfully.
    //!          else fail reason
    VAStatus ParseIQMatrix(
        DDI_MEDIA_CONTEXT     *mediaCtx,
        VAIQMatrixBufferMPEG2 *matrix);

    //! \brief   ParsePicParam for MPEG2
    //! \details parse the PicParam info required by MPEG2 decoding
    //!
    //! \param   [in] *mediaCtx
    //!          DDI_MEDIA_CONTEXT
    //! \param   [in] *qMatrix
    //!          VAIQMatrixBufferH264
    //!
    //! \return  VA_STATUS_SUCCESS is returned if it is parsed successfully.
    //!          else fail reason
    VAStatus ParsePicParams(
        DDI_MEDIA_CONTEXT             *mediaCtx,
        VAPictureParameterBufferMPEG2 *picParam);

    //! \brief   Alloc SliceParam content for MPEG2
    //! \details Alloc/resize SlicePram content for MPEG2 decoding
    //!
    //! \param   [in] numSlices
    //!          uint32_t the required number of slices
    //!
    //! \return  VA_STATUS_SUCCESS is returned if it is parsed successfully.
    //!          else fail reason
    VAStatus AllocSliceParamContext(
        uint32_t numSlices);

    //!
    //! \brief   Parse and Refine the number of MBs for MPEG2
    //! \details parse and Refine the number of MBs for each slices
    //!          This helps to fix the issue passed from upper middleware.
    //!
    //! \param   [in] numSlices
    //!             int32_t
    void ParseNumMbsForSlice(int32_t numSlices);

    //! \brief   Init Resource buffer for MPEG2
    //! \details Initialize and allocate the Resource buffer for MPEG2
    //!
    //! \return  VA_STATUS_SUCCESS is returned if it is parsed successfully.
    //!          else fail reason
    VAStatus InitResourceBuffer();

    //! \brief   Free Resource buffer for MPEG2
    //!
    void FreeResourceBuffer();

};

#endif /* _MEDIA_DDI_DECODE_MPEG2_H */
