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
//! \file     ddi_encode_jpeg_specific.h
//! \brief    Defines class for DDI media jpeg encode
//!

#ifndef __DDI_ENCODER_JPEG_SPECIFIC_H__
#define __DDI_ENCODER_JPEG_SPECIFIC_H__

#include "ddi_encode_base_specific.h"

namespace encode
{

static constexpr int32_t MaxNumQuantTableIndex = 3;
static constexpr int32_t QuantMatrixSize       = 64;
static constexpr int32_t MaxNumHuffTables      = 2;

static const uint32_t defaultLumaQuant[64] =  //!< Default Quantization Matrix for luma component
{                                             //!< of JPEG Encode in zig zag scan order (from JPEG Spec, Table K.1)
    16, 11, 12, 14, 12, 10, 16, 14,
    13, 14, 18, 17, 16, 19, 24, 40,
    26, 24, 22, 22, 24, 49, 35, 37,
    29, 40, 58, 51, 61, 60, 57, 51,
    56, 55, 64, 72, 92, 78, 64, 68,
    87, 69, 55, 56, 80, 109, 81, 87,
    95, 98, 103, 104, 103, 62, 77, 113,
    121, 112, 100, 120, 92, 101, 103, 99
};

static const uint32_t defaultChromaQuant[64] =  //!< Default Quantization Matrix for chroma component
{                                               //!< of JPEG Encode in zig zag scan order (from JPEG Spec, Table K.2)
    17, 18, 18, 24, 21, 24, 47, 26,
    26, 47, 99, 66, 56, 66, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99
};

//!
//! \enum   DDI_ENCODE_JPEG_INPUTSURFACEFORMATS
//! \brief  Ddi encode JPEG input surface formats
//!
enum DDI_ENCODE_JPEG_INPUTSURFACEFORMATS  //!< Jpeg input surface formats.
{
    DDI_ENCODE_JPEG_INPUTFORMAT_RESERVED = 0,
    DDI_ENCODE_JPEG_INPUTFORMAT_NV12     = 1,
    DDI_ENCODE_JPEG_INPUTFORMAT_UYVY     = 2,
    DDI_ENCODE_JPEG_INPUTFORMAT_YUY2     = 3,
    DDI_ENCODE_JPEG_INPUTFORMAT_Y8       = 4,
    DDI_ENCODE_JPEG_INPUTFORMAT_RGB      = 5
};

//!
//! \class  DdiEncodeJpeg
//! \brief  Ddi encode JPEG
//!
class DdiEncodeJpeg : public encode::DdiEncodeBase
{
public:
    //!
    //! \brief    Constructor
    //!
    DdiEncodeJpeg(){};

    //!
    //! \brief    Destructor
    //!
    virtual ~DdiEncodeJpeg();

    //!
    //! \brief    Initialize Encode Context and CodecHal Setting for Jpeg
    //!
    //! \param    [out] codecHalSettings
    //!           Pointer to CodechalSetting *
    //!
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if success, else fail reason
    //!
    VAStatus ContextInitialize(
        CodechalSetting *codecHalSettings) override;

    //!
    //! \brief    Parse buffer to the server.
    //!
    //! \param    [in] ctx
    //!           Pointer to VADriverContextP
    //! \param    [in] context
    //!           VA context ID
    //! \param    [in] buffers
    //!           Pointer to VABufferID
    //! \param    [in] numBuffers
    //!           Number of buffers
    //!
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if success, else fail reason
    //!
    VAStatus RenderPicture(
        VADriverContextP ctx,
        VAContextID      context,
        VABufferID       *buffers,
        int32_t          numBuffers) override;

protected:
    //!
    //! \brief    Reset Encode Context At Frame Level
    //!
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if success, else fail reason
    //!
    VAStatus ResetAtFrameLevel() override;

    //!
    //! \brief    Encode in CodecHal for Jpeg
    //!
    //! \param    [in] numSlices
    //!           Number of slice data structures
    //!
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if success, else fail reason
    //!
    VAStatus EncodeInCodecHal(
        uint32_t numSlices) override;

    //!
    //! \brief    Parse Picture Parameter buffer to Encode Context
    //!
    //! \param    [in] mediaCtx
    //!           Pointer to DDI_MEDIA_CONTEXT
    //! \param    [in] ptr
    //!           Pointer to Picture Parameter buffer
    //!
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if success, else fail reason
    //!
    VAStatus ParsePicParams(
        DDI_MEDIA_CONTEXT *mediaCtx,
        void              *ptr) override;

    uint32_t getSliceParameterBufferSize() override;

    uint32_t getPictureParameterBufferSize() override;

    uint32_t getQMatrixBufferSize() override;

    //!
    //! \brief    Parse QMatrix buffer to Encode Context
    //!
    //! \param    [in] ptr
    //!           Pointer to QMatrix buffer
    //!
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if success, else fail reason
    //!
    VAStatus Qmatrix(
        void *ptr);

    //!
    //! \brief    Parse Slice Parameter buffer to Encode Context
    //!
    //! \param    [in] mediaCtx
    //!           Pointer to DDI_MEDIA_CONTEXT
    //! \param    [in] ptr
    //!           Pointer to Slice Parameter buffer
    //! \param    [in] numSlices
    //!           Number of slice
    //!
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if success, else fail reason
    //!
    VAStatus ParseSlcParams(
        DDI_MEDIA_CONTEXT *mediaCtx,
        void              *ptr,
        uint32_t          numSlices);

    //!
    //! \brief    Parse Huffman Parameter buffer to Encode Context
    //!
    //! \param    [in] ptr
    //!           Pointer to Huffman Parameter buffer
    //!
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if success, else fail reason
    //!
    VAStatus ParseHuffmanParams(void *ptr);

    //!
    //! \brief    Parse Application Data buffer to Encode Context
    //!
    //! \param    [in] ptr
    //!           Pointer to Application Data buffer
    //! \param    [in] size
    //!           Size of Application Data buffer
    //!
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if success, else fail reason
    //!
    VAStatus ParseAppData(
        void    *ptr,
        int32_t size);

    //!
    //! \brief    Return the CODECHAL_FUNCTION type for give profile and entrypoint
    //!
    //! \param    [in] profile
    //!           Specify the VAProfile
    //!
    //! \param    [in] entrypoint
    //!           Specify the VAEntrypoint
    //!
    //! \return   Codehal function
    //!
    CODECHAL_FUNCTION GetEncodeCodecFunction(VAProfile profile, VAEntrypoint entrypoint, bool bVDEnc) override;
    //!
    //! \brief    Return internal encode mode for given profile and entrypoint
    //!
    //! \param    [in] profile
    //!           Specify the VAProfile
    //!
    //! \param    [in] entrypoint
    //!           Specify the VAEntrypoint
    //!
    //! \return   Codehal mode
    //!
    CODECHAL_MODE GetEncodeCodecMode(VAProfile profile, VAEntrypoint entrypoint) override;

private:
    //!
    //! \brief    Parse QMatrix buffer to Encode Context,
    //!           if quant table is not supplied by application
    //!
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if success, else fail reason
    //!
    VAStatus DefaultQmatrix();
    //!
    //! \brief    Application send whole header and qmatrix
    //!           must be extracted from it
    //!
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if success, else fail reason
    //!
    VAStatus QmatrixFromHeader();
    //!
    //! \brief    Convert Media Format To Input Surface Format
    //!
    //! \param    [in] format
    //!           Media format
    //!
    //! \return   uint32_t
    //!           Input surface format
    //!
    uint32_t ConvertMediaFormatToInputSurfaceFormat(DDI_MEDIA_FORMAT format);

    CodecEncodeJpegHuffmanDataArray    *m_huffmanTable = nullptr;    //!< Huffman table.
    void                               *m_appData      = nullptr;    //!< Application data.
    bool                               m_quantSupplied = false;      //!< whether Quant table is supplied by the app for JPEG encoder.
    uint32_t                           m_appDataTotalSize   = 0;          //!< Total size of application data.
    uint32_t                           m_appDataSize   = 0;          //!< Size of application data.
    bool                               m_appDataWholeHeader = false; //!< whether the app data include whole headers , such as SOI, DQT ...

MEDIA_CLASS_DEFINE_END(encode__DdiEncodeJpeg)
};

}
#endif /* __DDI_ENCODER_JPEG_SPECIFIC_H__ */
