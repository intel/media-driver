/*
* Copyright (c) 2025, Intel Corporation
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
//! \file     encode_avc_brc_xe3_lpm.h
//! \brief    Defines the common interface for encode avc Xe3_LPM brc
//!
#ifndef __ENCODE_AVC_BRC_XE3_LPM_H__
#define __ENCODE_AVC_BRC_XE3_LPM_H__

#include "encode_avc_brc.h"

namespace encode
{

class AvcEncodeBRCXe3_Lpm : public AvcEncodeBRC
{
public:
    AvcEncodeBRCXe3_Lpm(MediaFeatureManager *featureManager,
        EncodeAllocator                     *allocator,
        CodechalHwInterfaceNext             *hwInterface,
        void                                *constSettings) 
        : AvcEncodeBRC(featureManager, allocator, hwInterface, constSettings) {}

    virtual ~AvcEncodeBRCXe3_Lpm() = default;

    virtual MOS_STATUS SetDmemForInit(void *params) override;

MEDIA_CLASS_DEFINE_END(encode__AvcEncodeBRCXe3_Lpm)
};

}  // namespace encode

#endif  // !__ENCODE_AVC_BRC_XE3_LPM_H__
