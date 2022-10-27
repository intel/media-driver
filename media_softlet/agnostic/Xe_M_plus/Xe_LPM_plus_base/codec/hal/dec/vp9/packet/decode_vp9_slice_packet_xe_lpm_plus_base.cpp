/*
* Copyright (c) 2021-2022, Intel Corporation
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
//! \file     decode_vp9_slice_packet_xe_lpm_plus_base.cpp
//! \brief    Defines the interface for vp9 decode slice packet for Xe_LPM_plus+
//!
#include "decode_vp9_slice_packet_xe_lpm_plus_base.h"

namespace decode
{

Vp9DecodeSlcPktXe_Lpm_Plus_Base::~Vp9DecodeSlcPktXe_Lpm_Plus_Base()
{
}

MOS_STATUS Vp9DecodeSlcPktXe_Lpm_Plus_Base::Execute(MOS_COMMAND_BUFFER &cmdBuffer, uint32_t sliceIdx, uint32_t subTileIdx)
{
 
    // Todo: no slice level command for Huc based DRM and scalability decode BE phases, need add scalability
    // BE check here when enable scalability
    DECODE_CHK_STATUS(AddHcpCpState(cmdBuffer, sliceIdx, subTileIdx));
    SETPAR_AND_ADDCMD(HCP_BSD_OBJECT, m_hcpItf, &cmdBuffer);
    return MOS_STATUS_SUCCESS;
}

}
