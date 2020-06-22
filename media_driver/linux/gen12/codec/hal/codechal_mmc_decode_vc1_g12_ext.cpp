/*
* Copyright (c) 2017-2019, Intel Corporation
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
//! \file     codechal_mmc_decode_vc1_g12_ext.cpp
//! \brief    Implements VC1 Gen12 media memory compression extra interface
//!

#include "codechal_mmc_decode_vc1_g12_ext.h"

CodechalMmcDecodeVc1G12Ext::CodechalMmcDecodeVc1G12Ext(
    CodechalHwInterface* hwInterface,
    CodecHalMmcState* mmcState) :
    m_hwInterface(hwInterface),
    m_mmcState(mmcState)
{
    CODECHAL_DECODE_FUNCTION_ENTER;
}

MOS_STATUS CodechalMmcDecodeVc1G12Ext::CopyAuxSurfForSkip(
    PMOS_COMMAND_BUFFER cmdBuffer,
    PMOS_RESOURCE       srcResource,
    PMOS_RESOURCE       destResource)
{
    CODECHAL_DECODE_FUNCTION_ENTER;
    
    return MOS_STATUS_SUCCESS;
}