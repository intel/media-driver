/*
* Copyright (c) 2020-2022, Intel Corporation
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
//! \file     encode_vp9_vdenc_packet_xe_lpm_plus.cpp
//! \brief    Defines the interface for vp9 encode vdenc packet
//!

#include "encode_vp9_vdenc_packet_xe_lpm_plus.h"
#include "mhw_vdbox_vdenc_impl_xe_lpm_plus.h"

namespace encode
{
// Xe_LPM_plus specific and/or overwrite vdenc packet function implement here

MOS_STATUS Vp9VdencPktXe_Lpm_Plus::Init()
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_STATUS_RETURN(Vp9VdencPktXe_Lpm_Plus_Base::Init());

    return MOS_STATUS_SUCCESS;
}

}  // namespace encode
