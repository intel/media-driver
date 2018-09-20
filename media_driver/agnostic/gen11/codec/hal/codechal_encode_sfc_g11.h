/*
* Copyright (c) 2018, Intel Corporation
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

#include "codechal_encode_sfc.h"
#define CODECHAL_SFC_VEBOX_MAX_SLICES_G11                              4
#define CODECHAL_SFC_VEBOX_RGB_HISTOGRAM_SIZE_G11                     (CODECHAL_SFC_VEBOX_RGB_HISTOGRAM_SIZE_PER_SLICE * \
                                                                   CODECHAL_SFC_NUM_RGB_CHANNEL                    * \
                                                                   CODECHAL_SFC_VEBOX_MAX_SLICES_G11)
#define CODECHAL_SFC_VEBOX_STATISTICS_SIZE_G11                          (32 * 8)
#define CODECHAL_SFC_VEBOX_RGB_ACE_HISTOGRAM_SIZE_RESERVED_G11          (3072 * 4)
class CodecHalEncodeSfcG11 : public CodecHalEncodeSfc
{
public:
    CodecHalEncodeSfcG11() {};
    ~CodecHalEncodeSfcG11() {};
    
    virtual MOS_STATUS SetVeboxDiIecpParams(
        PMHW_VEBOX_DI_IECP_CMD_PARAMS         params)override;
};
#endif  // __CODECHAL_ENCODE_SFC_G11_H__
