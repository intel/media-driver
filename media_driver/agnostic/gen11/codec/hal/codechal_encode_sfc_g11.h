/*
* Copyright (c) 2018-2020, Intel Corporation
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
//! \file     codechal_encode_sfc_g11.h
//! \brief    Defines the encode interface for CSC via SFC.
//! \details  Downsampling in this case is supported by the SFC fixed function HW unit.
//!

#ifndef __CODECHAL_ENCODE_SFC_G11_H__
#define __CODECHAL_ENCODE_SFC_G11_H__

#include "codechal_encode_sfc_base.h"

class CodecHalEncodeSfcG11 : public CodecHalEncodeSfcBase
{
public:
    CodecHalEncodeSfcG11() {}
    virtual ~CodecHalEncodeSfcG11() {}
    
    virtual MOS_STATUS SetVeboxDiIecpParams(
        PMHW_VEBOX_DI_IECP_CMD_PARAMS         params) override;

protected:
    virtual int32_t GetSfcVeboxStatisticsSize() const override { return 32 * 8; }

    // ACE/LACE/RGBHistogram buffer
    virtual int32_t GetVeboxRgbAceHistogramSizeReserved() const override { return 3072 * 4; }
    virtual int32_t GetVeboxMaxSlicesNum() const override { return 4; }
};
#endif  // __CODECHAL_ENCODE_SFC_G11_H__
