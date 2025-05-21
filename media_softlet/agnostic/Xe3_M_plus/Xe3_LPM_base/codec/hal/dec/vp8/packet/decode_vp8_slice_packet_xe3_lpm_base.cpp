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
//! \file     decode_vp8_slice_packet_xe3_lpm_base.cpp
//! \brief    Defines the interface for vp8 decode slice packet for Xe3_LPM
//!
#include "decode_vp8_slice_packet_xe3_lpm_base.h"

namespace decode
{

Vp8DecodeSlcPktXe3_Lpm_Base::~Vp8DecodeSlcPktXe3_Lpm_Base()
{
}

MOS_STATUS Vp8DecodeSlcPktXe3_Lpm_Base::Execute(MOS_COMMAND_BUFFER& cmdBuffer)
{
    DECODE_FUNC_CALL();

    DECODE_CHK_STATUS(AddMiFlushDwCmd(cmdBuffer));
    SETPAR_AND_ADDCMD(MFD_VP8_BSD_OBJECT, m_mfxItf, &cmdBuffer);

    return MOS_STATUS_SUCCESS;
}

}
