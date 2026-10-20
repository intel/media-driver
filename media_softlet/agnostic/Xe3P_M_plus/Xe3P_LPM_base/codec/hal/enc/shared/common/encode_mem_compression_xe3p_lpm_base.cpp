/*
* Copyright (c) 2023, Intel Corporation
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
//! \file     encode_mem_compression_xe3p_lpm_base.cpp
//! \brief    Defines the common interface for encode mmc.
//! \details  The mmc is to handle mmc operations,
//! including compression and decompressin of encode
//!

#include "encode_mem_compression_xe3p_lpm_base.h"
#include "encode_utils.h"

EncodeMemCompXe3P_Lpm_Base::EncodeMemCompXe3P_Lpm_Base(
    CodechalHwInterfaceNext *hwInterface):
    EncodeMemComp(hwInterface), CodecMmcAuxTableXe3P_Lpm_Base()
{

}

MOS_STATUS EncodeMemCompXe3P_Lpm_Base::SendPrologCmd(
    PMOS_COMMAND_BUFFER    cmdBuffer,
    bool    bRcsIsUsed)
{
    MOS_STATUS status = MOS_STATUS_SUCCESS;

    ENCODE_CHK_NULL_RETURN(m_miItf);
    ENCODE_CHK_STATUS_RETURN(CodecMmcAuxTableXe3P_Lpm_Base::LoadAuxTableMmio(m_osInterface, *m_miItf, cmdBuffer, bRcsIsUsed));

    return MOS_STATUS_SUCCESS;
}
