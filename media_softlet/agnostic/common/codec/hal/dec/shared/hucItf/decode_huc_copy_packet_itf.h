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
//! \file     decode_huc_copy_packet_itf.h
//! \brief    Defines the implementation of huc copy packet
//!

#ifndef __CODECHAL_HUC_COPY_PACKET_ITF_H__
#define __CODECHAL_HUC_COPY_PACKET_ITF_H__

#include "decode_utils.h"
#include "mos_os_specific.h"
namespace decode
{
class HucCopyPktItf
{
public:
    struct HucCopyParams
    {
        PMOS_RESOURCE srcBuffer;
        uint32_t      srcOffset;
        PMOS_RESOURCE destBuffer;
        uint32_t      destOffset;
        uint32_t      copyLength;
    };

    HucCopyPktItf()
    {
    }

    virtual ~HucCopyPktItf() {}

    virtual MOS_STATUS PushCopyParams(HucCopyParams &copyParams) = 0;

MEDIA_CLASS_DEFINE_END(decode__HucCopyPktItf)
};

}  // namespace decode
#endif