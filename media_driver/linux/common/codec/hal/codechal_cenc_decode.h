/*
* Copyright (c) 2013-2018, Intel Corporation
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
//! \file      codechal_cenc_decode.h 
//! \brief     Stub file for CodecHal CENC Decode 
//!
#ifndef __CODECHAL_CENC_DECODE_H__
#define __CODECHAL_CENC_DECODE_H__

#include "codechal.h"
#include "codechal_decoder.h"
#include "mos_defs.h"

//!
//! \class  CodechalCencDecode
//! \brief  Codechal cenc decode
//!
class CodechalCencDecode
{

public:

    //!
    //! \brief  Destructor
    //!
    ~CodechalCencDecode(){};

    //!
    //! \brief  Create Cenc Decode
    //! \return MOS_STATUS
    //!
    static MOS_STATUS Create(
        CODECHAL_STANDARD           standard,
        CodechalCencDecode          **cencDecoder)
    {
        *cencDecoder = nullptr;
        return MOS_STATUS_SUCCESS;
    };

    //!
    //! \brief  Initilize
    //! \return MOS_STATUS_UNIMPLEMENTED
    //!
    MOS_STATUS Initialize(
        CodechalDebugInterface      *debugInterface,
        PMOS_CONTEXT                 osContext,
        CodechalSetting              *settings)
    {
        return MOS_STATUS_UNIMPLEMENTED;
    };

    //!
    //! \brief  Execute Bitstream Cenc Decode
    //! \return MOS_STATUS_UNIMPLEMENTED
    //!
    MOS_STATUS Execute(
        void                        *params)
    {
        return MOS_STATUS_UNIMPLEMENTED;
    };

    //!
    //! \brief  Get Status Report
    //! \return MOS_STATUS_UNIMPLEMENTED
    //!
    MOS_STATUS GetStatusReport(
        CodechalDebugInterface       *debugInterface,
        void                         *status,
        uint16_t                     numStatus)
    {
        return MOS_STATUS_UNIMPLEMENTED;
    };

    //!
    //! \brief  Set Params For Decode
    //! \return MOS_STATUS_UNIMPLEMENTED
    //!
    MOS_STATUS SetParamsForDecode(
        void                        *state,
        CodechalHwInterface         *hwInterface,
        CodechalDebugInterface      *debugInterface,
        CodechalDecodeParams        *decodeParams)
    {
        return MOS_STATUS_UNIMPLEMENTED;
    };

    //!
    //! \brief  Set Batch Buffer For Decode
    //! \return MOS_STATUS_UNIMPLEMENTED
    //!
    MOS_STATUS SetBatchBufferForDecode(
        CodechalHwInterface          *hwInterface,
        CodechalDebugInterface       *debugInterface,
        uint8_t                      sliceBatchBufferIdx,
        PMOS_COMMAND_BUFFER          cmdBuffer)
    {
        return MOS_STATUS_UNIMPLEMENTED;
    };

    //!
    //! \brief  Check Status Report Num
    //! \return MOS_STATUS_UNIMPLEMENTED
    //!
    MOS_STATUS CheckStatusReportNum(
        void                         *state,
        PMOS_COMMAND_BUFFER          cmdBuffer,
        MHW_VDBOX_NODE_IND           vdboxIndex)
    {
        return MOS_STATUS_UNIMPLEMENTED;
    };

    PMOS_INTERFACE                  osInterface = nullptr;

protected:

    //!
    //! \brief  Constructor
    //!
    CodechalCencDecode(){};
};

#endif // __CODECHAL_CENC_DECODE_H__
