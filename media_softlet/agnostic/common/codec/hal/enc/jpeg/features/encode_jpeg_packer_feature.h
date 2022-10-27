/*
* Copyright (c) 2021, Intel Corporation
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
//! \file     encode_jpeg_packer_feature.h
//! \brief    Defines packing logic for jpeg encode
//!
#ifndef __ENCODE_JPEG_PACKER_FEATURE_H__
#define __ENCODE_JPEG_PACKER_FEATURE_H__

#include "media_feature.h"
#include "encode_jpeg_basic_feature.h"

namespace encode
{

//!
//! \struct EncodeJpegQuantHeader
//! \brief Define JPEG Quant Header structure
//!
struct EncodeJpegQuantHeader
{
    uint16_t m_dqt;                           //!< Define Quantization Marker
    uint16_t m_lq;                            //!< Quantization table definition length
    uint8_t  m_tablePrecisionAndDestination;  //!< 4 bits of element precision and 4 bits of destination selector
    uint8_t  m_qk[JPEG_NUM_QUANTMATRIX];      //!< Quantization table elements
};

#pragma pack(push, 1)
//!
//! \struct CodechalEncodeJpegFrameHeader
//! \brief Define JPEG Frame Header structure
//!
struct EncodeJpegFrameHeader
{
    uint16_t m_sof;  //!< Start of frame marker
    uint16_t m_lf;   //!< Frame header length
    uint8_t  m_p;    //!< Precision
    uint16_t m_y;    //!< max number of lines in image
    uint16_t m_x;    //!< max number of samples per line in image
    uint8_t  m_nf;   //!< Number of image components in the frame

    struct
    {
        uint8_t m_ci;                     //!< Component identifier
        uint8_t m_samplingFactori;        //!< 4 MSBs are the horizontal and 4 LSBs are vertical sampling factors
        uint8_t m_tqi;                    //!< Quantization table selector (0-3)
    } m_codechalJpegFrameComponent[256];  //!< JPEG frame component array
};

//!
//! \struct CodechalJpegHuffmanHeader
//! \brief Define JPEG Huffman Header structure
//!
struct EncodeJpegHuffmanHeader
{
    uint16_t m_dht;                                  //!< Define Huffman Table Marker
    uint16_t m_lh;                                   //!< Huffman table definition length
    uint8_t  m_tableClassAndDestn;                   //!< 4 bits of Huffman table class (0 = DC, 1 = AC) and 4 bits of destination identifier
    uint8_t  m_li[JPEG_NUM_HUFF_TABLE_AC_BITS];      //!< List BITS of Huffman table
    uint8_t  m_vij[JPEG_NUM_HUFF_TABLE_AC_HUFFVAL];  //!< Value associated with each huffman code
};

//!
//! \struct CodechalEncodeJpegRestartHeader
//! \brief Define JPEG Restart Header structure
//!
struct EncodeJpegRestartHeader
{
    uint16_t m_dri;  //!< Define restart interval marker
    uint16_t m_lr;   //!< Restart interval segment length
    uint16_t m_ri;   //!< Restart interval
};

//!
//! \struct CodechalEncodeJpegScanComponent
//! \brief JPEG Scan Component structure
//!
struct EncodeJpegScanComponent
{
    uint8_t m_csj;   //!< Scan component selector
    uint8_t m_tdaj;  //!< 4 bits of DC huffman table destination selector and 4 bits of AC huffman table selector
};

//!
//! \struct CodechalEncodeJpegScanHeader
//! \brief JPEG Scan Header structure
//!
struct EncodeJpegScanHeader
{
    uint16_t                m_sos;  //!< Start of scan marker
    uint16_t                m_ls;   //!< Scan header length
    uint8_t                 m_ns;   //!< Number of image components in scan
    EncodeJpegScanComponent m_scanComponent[jpegNumComponent];
    uint8_t                 m_ss;   //!< start of spectral selection
    uint8_t                 m_se;   //!< end of spectral selection
    uint8_t                 m_ahl;  //!< successive approximation bit position high and low
};
#pragma pack(pop)


class JpegPackerFeature : public MediaFeature
{
public:

    //!
    //! \brief  AvcEncodeHeaderPacker constructor
    //!
    JpegPackerFeature(MediaFeatureManager *featureManager,
        EncodeAllocator *                  allocator,
        CodechalHwInterfaceNext *              hwInterface,
        void *                             constSettings);

    //!
    //! \brief  AvcEncodeHeaderPacker deconstructor
    //!
    virtual ~JpegPackerFeature() {}

    //!
    //! \brief    Pack SOI
    //!
    //! \param    [out] buffer
    //!           Bitstream buffer
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS PackSOI(BSBuffer *buffer);

    //!
    //! \brief    Pack Application Data
    //!
    //! \param    [out] buffer
    //!           Bitstream buffer
    //! \param    [in] appDataChunk
    //!           Application Data Chunk
    //! \param    [in] size
    //!           Application Data Size
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS PackApplicationData(
        BSBuffer *buffer,
        uint8_t * appDataChunk,
        uint32_t  size);

    //!
    //! \brief    Pack Quant Table
    //!
    //! \param    [out] buffer
    //!           Bitstream buffer
    //! \param    [in] componentType
    //!           The Component Type
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS PackQuantTable(
        BSBuffer *          buffer,
        CodecJpegComponents componentType);

    //!
    //! \brief    Pack Frame Header
    //!
    //! \param    [out] buffer
    //!           Bitstream buffer
    //! \param    [in] useSingleDefaultQuantTable
    //!           The flag of using single default Quant Table
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS PackFrameHeader(
        BSBuffer *buffer,
        bool      useSingleDefaultQuantTable);

    //!
    //! \brief    Pack Huffman Table
    //!
    //! \param    [out] buffer
    //!           Bitstream buffer
    //! \param    [in] tableIndex
    //!           The Huffman Table Index
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS PackHuffmanTable(
        BSBuffer *buffer,
        uint32_t  tableIndex);

    //!
    //! \brief    Pack Restart Interval
    //!
    //! \param    [out] buffer
    //!           Bitstream buffer
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS PackRestartInterval(
        BSBuffer *buffer);

    //!
    //! \brief    Pack Scan Header
    //!
    //! \param    [out] buffer
    //!           Bitstream buffer
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS PackScanHeader(
        BSBuffer *buffer);

protected:

    static const uint32_t m_jpegEncodeSoi = 0xFFD8;  //!< JPEG Encode Header Markers SOI
    static const uint32_t m_jpegEncodeSos = 0xFFDA;  //!< JPEG Encode Header Markers SOS

MEDIA_CLASS_DEFINE_END(encode__JpegPackerFeature)
};

}  // namespace encode

#endif  // !__ENCODE_JPEG_PACKER_FEATURE_H__
