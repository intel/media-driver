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
//! \file     decode_mem_compression_g12.cpp
//! \brief    Defines the common interface for decode mmc.
//! \details  The mmc is to handle mmc operations,
//! including compression and decompressin of decode
//!

#include "decode_mem_compression_g12.h"
#include "codechal_hw.h"
#include "mhw_mi_g12_X.h"
#include "decode_utils.h"

DecodeMemCompG12::DecodeMemCompG12(
    CodechalHwInterface *hwInterface):
    DecodeMemComp(*hwInterface, hwInterface->GetOsInterface()), CodecMmcAuxTableG12()
{
    m_mhwMiInterface = hwInterface->GetMiInterface();
}

MOS_STATUS DecodeMemCompG12::SendPrologCmd(
    PMOS_COMMAND_BUFFER    cmdBuffer,
    bool    bRcsIsUsed)
{
    MOS_STATUS status = MOS_STATUS_SUCCESS;

    DECODE_CHK_STATUS(CodecMmcAuxTableG12::LoadAuxTableMmio(m_osInterface, m_mhwMiInterface, cmdBuffer, bRcsIsUsed));

    return MOS_STATUS_SUCCESS;
}
