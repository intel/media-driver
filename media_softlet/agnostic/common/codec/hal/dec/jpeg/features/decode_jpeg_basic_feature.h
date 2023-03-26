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
//! \file     decode_jpeg_basic_feature.h
//! \brief    Defines the common interface for decode jpeg basic feature
//!
#ifndef __DECODE_JPEG_BASIC_FEATURE_H__
#define __DECODE_JPEG_BASIC_FEATURE_H__

#include "decode_basic_feature.h"
#include "codec_def_common_jpeg.h"

namespace decode {

#define CODECHAL_DECODE_JPEG_BLOCK_ALIGN_SIZE 8
#define CODECHAL_DECODE_JPEG_BLOCK_ALIGN_SIZE_X2 16
#define CODECHAL_DECODE_JPEG_BLOCK_ALIGN_SIZE_X4 32
#define CODECHAL_DECODE_JPEG_ERR_FRAME_WIDTH 32
#define CODECHAL_DECODE_JPEG_ERR_FRAME_HEIGHT 32

#define MAX_NUM_HUFF_TABLE_INDEX 2

class JpegBasicFeature : public DecodeBasicFeature
{
public:
    //!
    //! \brief  JpegBasicFeature constructor
    //!
    JpegBasicFeature(DecodeAllocator *allocator, void *hwInterface, PMOS_INTERFACE osInterface) : 
        DecodeBasicFeature(allocator, hwInterface, osInterface)
    {
        MOS_ZeroMemory(&m_jpegHuffmanTable, sizeof(m_jpegHuffmanTable));
        if (osInterface != nullptr)
        {
            m_osInterface = osInterface;
        }
    };

    //!
    //! \brief  JpegBasicFeature deconstructor
    //!
    virtual ~JpegBasicFeature();

    //!
    //! \brief  Initialize Jpeg basic feature CodechalSetting
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Init(void *setting) override;

    //!
    //! \brief  Update Jpeg decodeParams
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Update(void *params) override;

    // App must use 420_OPAQUE as DecodeRT for other JPEG output formats except NV12 and YUY2 due to runtime
    // restriction, so the real JPEG format is passed to driver in PPS data. The code here is just to get the real output format.
    // On SKL+, app would use AYUV (instead of 420_OPAQUE) as DecodeRT for direct YUV to ARGB8888 conversion; in such case,
    // real output format (ARGB8888) should also be from JPEG PPS; MSDK would handle the details of treating AYUV as ARGB.
    virtual void GetRenderTargetFormat(PMOS_FORMAT format);

    CodecDecodeJpegPicParams           *m_jpegPicParams          = nullptr;          //!< Pointer to Jpeg picture parameter
 
  //  uint32_t                            m_dataSize;                  //!< Data size of the bitstream
  //  uint32_t                            m_dataOffset;                //!< Data offset of the bitstream
    CodecDecodeJpegScanParameter *      m_jpegScanParams = nullptr;  //!< Scan parameter for JPEG
    CodecJpegQuantMatrix *              m_jpegQMatrix    = nullptr;  //!< QMatrix for JPEG
    PCODECHAL_DECODE_JPEG_HUFFMAN_TABLE m_jpegHuffmanTable;          //!< Huffman table for JPEG


protected:
    MOS_STATUS         SetPictureStructs();
    MOS_STATUS         CheckSupportedFormat(PMOS_FORMAT format);
    virtual MOS_STATUS SetRequiredBitstreamSize(uint32_t requiredSize) override;
   
    PMOS_INTERFACE        m_osInterface  = nullptr;

MEDIA_CLASS_DEFINE_END(decode__JpegBasicFeature)
};

}//decode

#endif // !__DECODE_JPEG_BASIC_FEATURE_H__
