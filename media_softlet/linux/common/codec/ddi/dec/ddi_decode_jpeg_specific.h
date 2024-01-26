/*
* Copyright (c) 2022-2023, Intel Corporation
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
//! \file      ddi_decode_jpeg_specific.h 
//! \brief     Defines DdiDecodeJpeg class for JPEG decode
//!

#ifndef __DDI_DECODE_JPEG_SPECIFIC_H__
#define __DDI_DECODE_JPEG_SPECIFIC_H__

#include "ddi_decode_base_specific.h"

namespace decode
{

//!
//! \class DdiDecodeJpeg
//! \brief This class defines the member fields, functions etc used by JPEG decoder.
//!
class DdiDecodeJpeg : public DdiDecodeBase
{
public:
    //!
    //! \brief Constructor
    //!
    DdiDecodeJpeg() : DdiDecodeBase() {};

    //!
    //! \brief Destructor
    //!
    virtual ~DdiDecodeJpeg(){};

    // inherited virtual functions
    virtual VAStatus BeginPicture(
        VADriverContextP ctx,
        VAContextID      context,
        VASurfaceID      renderTarget) override;

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

    virtual void ContextInit(
        int32_t picWidth,
        int32_t picHeight) override;

    virtual VAStatus CodecHalInit(
        DDI_MEDIA_CONTEXT *mediaCtx,
        void              *ptr) override;

    virtual VAStatus AllocSliceControlBuffer(
        DDI_MEDIA_BUFFER *buf) override;

    virtual VAStatus AllocBsBuffer(
        DDI_CODEC_COM_BUFFER_MGR *bufMgr,
        DDI_MEDIA_BUFFER         *buf) override;

    virtual uint8_t* GetPicParamBuf(
        DDI_CODEC_COM_BUFFER_MGR *bufMgr) override;

private:
    //!
    //! \brief   ParaSliceParam for Jpeg
    //! \details parse the sliceParam info required by JPEG decoding for
    //!          each slice
    //!
    //! \param   [in] *mediaCtx
    //!          DDI_MEDIA_CONTEXT
    //! \param   [in] *slcParam
    //!          VASliceParameterBufferJPEGBaseline
    //! \param   [in] numSlices
    //!             uint32_t
    //!
    //! \return  VA_STATUS_SUCCESS is returned if it is parsed successfully.
    //!          else fail reason
    VAStatus ParseSliceParams(
        DDI_MEDIA_CONTEXT                  *mediaCtx,
        VASliceParameterBufferJPEGBaseline *slcParam,
        uint32_t                           numSlices);

    //!
    //! \brief   ParaQMatrixParam for JPEG
    //! \details parse the IQMatrix info required by JPEG decoding
    //!
    //! \param   [in] *mediaCtx
    //!          DDI_MEDIA_CONTEXT
    //! \param   [in] *matrix
    //!          VAIQMatrixBufferJPEGBaseline
    //!
    //! \param   [in] numSlices
    //!          int32_t
    //!
    //! \return  VA_STATUS_SUCCESS is returned if it is parsed successfully.
    //!          else fail reason
    VAStatus ParseIQMatrix(
        DDI_MEDIA_CONTEXT            *mediaCtx,
        VAIQMatrixBufferJPEGBaseline *matrix);

    //! \brief   ParsePicParam for JPEG
    //! \details parse the PicParam info required by JPEG decoding
    //!
    //! \param   [in] *mediaCtx
    //!          DDI_MEDIA_CONTEXT
    //! \param   [in] picParam
    //!          VAPictureParameterBufferJPEGBaseline
    //!
    //! \param   [in] numSlices
    //!          int32_t
    //! \return  VA_STATUS_SUCCESS is returned if it is parsed successfully.
    //!          else fail reason
    VAStatus ParsePicParams(
        DDI_MEDIA_CONTEXT                    *mediaCtx,
        VAPictureParameterBufferJPEGBaseline *picParam);

    //! \brief   Alloc SliceParam content for JPEG
    //! \details Alloc/resize SlicePram content for JPEG decoding
    //!
    //! \param   [in] numSlices
    //!          uint32_t the required number of slices
    //!
    //! \return  VA_STATUS_SUCCESS is returned if it is parsed successfully.
    //!          else fail reason
    VAStatus AllocSliceParamContext(
        uint32_t numSlices);

    //! \brief   Init resource buffer for JPEG
    //! \details Initialize and allocate the resource buffer for JPEG
    //!
    //! \return  VA_STATUS_SUCCESS is returned if it is parsed successfully.
    //!          else fail reason
    VAStatus InitResourceBuffer();

    //! \brief   Free Resource buffer for JPEG
    //!
    void FreeResourceBuffer();

    //! \brief   ParseHuffmanTbl for JPEG
    //! \details parse the Huffman table info required by JPEG decoding
    //!
    //! \param   [in] *mediaCtx
    //!          DDI_MEDIA_CONTEXT
    //! \param   [in] huffmanTbl
    //!          VAHuffmanTableBufferJPEGBaseline
    //!
    //! \return  VA_STATUS_SUCCESS is returned if it is parsed successfully.
    //!          else fail reason
    VAStatus ParseHuffmanTbl(
        DDI_MEDIA_CONTEXT                *mediaCtx,
        VAHuffmanTableBufferJPEGBaseline *huffmanTbl);

    //! \brief   Set input buffer as rendered for JPEG decoding
    //! \details Mark the input data buffer as rendered. This will be used in later decoding.
    //!
    //! \param   [in] bufferID
    //!          VABufferID
    //!
    //! \return  VA_STATUS_SUCCESS is returned if it is parsed successfully.
    //!          else fail reason
    VAStatus SetBufferRendered(VABufferID bufferID);

    //! \brief   Check the output format for JPEG decoding
    //! \details Check whether the output format matches the chromat format.
    //!
    //! \param   [in] format
    //!          MOS_FORMAT
    //!
    //! \return  return true if pass the format check.
    //!          else false
    bool CheckFormat(MOS_FORMAT format);

    void FreeResource();

    //! \brief  the internal JPEG bit-stream buffer
    DDI_MEDIA_BUFFER *m_jpegBitstreamBuf = nullptr;

    //! \brief the total num of JPEG scans
    int32_t m_numScans = 0;

    MEDIA_CLASS_DEFINE_END(decode__DdiDecodeJpeg)
};
} // namespace decode
#endif // __DDI_DECODE_JPEG_SPECIFIC_H__
