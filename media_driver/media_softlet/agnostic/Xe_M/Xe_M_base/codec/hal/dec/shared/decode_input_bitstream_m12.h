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
//! \file     decode_input_bitstream_m12.h
//! \brief    Defines the common interface for decode input bitstream
//! \details  Defines the interface to handle the decode input bitstream in
//!           both single execution call mode and multiple excution call mode.
//!

#ifndef __DECODE_INPUT_BITSTREAM_M12_H__
#define __DECODE_INPUT_BITSTREAM_M12_H__

#include "decode_input_bitstream.h"
#include "decode_jpeg_basic_feature.h"
#include "codechal_hw.h"

namespace decode
{

class DecodeInputBitstreamM12 : public DecodeInputBitstream
{
public:
    //!
    //! \brief  Decode input bitstream constructor
    //!
    DecodeInputBitstreamM12(DecodePipeline *pipeline, MediaTask *task, uint8_t numVdbox, CodechalHwInterface *hwInterface);

    //!
    //! \brief  Decode input bitstream destructor
    //!
    virtual ~DecodeInputBitstreamM12(){};

    //!
    //! \brief  Initialize the bitstream context
    //!
    //! \param  [in] settings
    //!         Reference to the Codechal settings
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Init(CodechalSetting &settings) override;

protected:
    CodechalHwInterface *m_hwInterface = nullptr;

    MEDIA_CLASS_DEFINE_END(decode__DecodeInputBitstreamM12)
};

class DecodeJpegInputBitstreamM12 : public DecodeInputBitstreamM12
{
public:
    //!
    //! \brief  JPEG Decode input bitstream constructor
    //!
    DecodeJpegInputBitstreamM12(DecodePipeline *pipeline, MediaTask *task, uint8_t numVdbox, CodechalHwInterface *hwInterface);

    //!
    //! \brief  Decode input bitstream destructor
    //!
    virtual ~DecodeJpegInputBitstreamM12() {}

    MOS_STATUS Init(CodechalSetting &settings) override;

    MOS_STATUS Append(const CodechalDecodeParams &decodeParams) override;
    bool       IsComplete() override;

private:
    JpegBasicFeature *m_jpegBasicFeature  = nullptr;  //!< Decode basic feature
    bool              m_completeJpegScan  = false;    //to indicate the scan was complete
    bool              m_completeBitStream = false;

    MEDIA_CLASS_DEFINE_END(decode__DecodeJpegInputBitstreamM12)
};

}  // namespace decode
#endif
