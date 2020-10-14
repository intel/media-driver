/*
* Copyright (c) 2020, Intel Corporation
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
//! \file     codechal_encode_jpeg_g11_jsl_ehl.cpp
//! \brief    Defines state class for JPEG encoder.
//!

#include "codechal_encode_jpeg_g11_jsl_ehl.h"

MOS_STATUS CodechalEncodeJpegStateG11JslEhl::GetStatusReport(
    EncodeStatus* encodeStatus,
    EncodeStatusReport* encodeStatusReport)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    PMHW_VDBOX_IMAGE_STATUS_CONTROL imgStatusCtrl = &encodeStatus->ImageStatusCtrl;

    // The huffman tables sent by application were incorrect (used only for JPEG encoding)
    if (imgStatusCtrl->MissingHuffmanCode == 1)
    {
        // WA for JPEGe on JSL-EHL:
        // Sometimes bit "MissingHuffmanCode" could be sporadic set to 1
        // from parallel execution process (ex: h264 encoding),
        // what introduce "false possitive" issue reporting on JPEG encoding.
        // Disabling this checking for JPEGe, only keep the report in logs

        CODECHAL_ENCODE_ASSERTMESSAGE("Error: JPEG standard encoding: missing huffman code - skip for JSL-EHL");
    }

    eStatus = GetStatusReportCommon(encodeStatus, encodeStatusReport);

    return eStatus;
}
