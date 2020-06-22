/*
* Copyright (c) 2019, Intel Corporation
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
#include "MDF_FC_common_genx.h"

_GENX_MAIN_ void Prepare_LumaKey_SampleUnorm(
    CURBE_INPUT_OUTPUT,
    GLOBAL_BUFFER_INPUT_OUTPUT)
{
    //            BYTE 3           |            BYTE 2         |          BYTE  1         |       BYTE 0
    //   31 30 23 22 | 29 28 21 20 | 15 14 07 06 | 13 12 05 04 | 27 26 19 18 |25 24 17 16 | 11 10 03 02 | 09 08 01 00  -- Sampler returned format
    //   31 30 29 28 | 23 22 21 20 | 27 26 25 24 | 19 18 17 16 | 15 14 13 12 |07 06 05 04 | 11 10 09 08 | 03 02 01 00  -- Intermediate Format
    //   31 30 29 28 | 27 26 25 24 | 23 22 21 20 | 19 18 17 16 | 15 14 13 12 |11 10 09 08 | 07 06 05 04 | 03 02 01 00  -- Destination format

    uchar Buffer_Index2 = (Buffer_Index >> 4) << 4;
    uchar Mask_Index = cm_add<uchar>((Buffer_Index >> 4), -4, SAT);
    Mask_Index = Mask_Index << 2;
    Mask_Index = Mask_Index + ((Layer_Index & 0x80) >> 4); // Shift right Layer_Index 7 bits for sec half flag, left 3 bits for mask offset

    matrix<uchar, 1, 4> Sampler_Lumakey_Temp_Bit_01;
    matrix<uchar, 1, 4> Sampler_Lumakey_Temp_Bit_23;
    matrix<uchar, 1, 4> Sampler_Lumakey_Temp_Bit_45;
    matrix<uchar, 1, 4> Sampler_Lumakey_Temp_Bit_67;

    // 1st half
    Sampler_Lumakey_Temp_Bit_01.select<1, 1, 2, 2>(0, 0) = DataBuffer.format<uchar, 96, 32>().select<1, 1, 2, 1>(Buffer_Index2 + Channel_Offset_A_0, 0) & 0x03;
    Sampler_Lumakey_Temp_Bit_01.select<1, 1, 2, 2>(0, 1) = (DataBuffer.format<uchar, 96, 32>().select<1, 1, 2, 1>(Buffer_Index2 + Channel_Offset_A_0, 0) & 0x0C) >> 2;

    Sampler_Lumakey_Temp_Bit_23.select<1, 1, 2, 2>(0, 0) = (DataBuffer.format<uchar, 96, 32>().select<1, 1, 2, 1>(Buffer_Index2 + Channel_Offset_A_0, 0) & 0x30) >> 2;
    Sampler_Lumakey_Temp_Bit_23.select<1, 1, 2, 2>(0, 1) = (DataBuffer.format<uchar, 96, 32>().select<1, 1, 2, 1>(Buffer_Index2 + Channel_Offset_A_0, 0) & 0xC0) >> 4;

    Sampler_Lumakey_Temp_Bit_45.select<1, 1, 2, 2>(0, 0) = (DataBuffer.format<uchar, 96, 32>().select<1, 1, 2, 1>(Buffer_Index2 + Channel_Offset_A_0, 2) & 0x03) << 4;
    Sampler_Lumakey_Temp_Bit_45.select<1, 1, 2, 2>(0, 1) = (DataBuffer.format<uchar, 96, 32>().select<1, 1, 2, 1>(Buffer_Index2 + Channel_Offset_A_0, 2) & 0x0C) << 2;

    Sampler_Lumakey_Temp_Bit_67.select<1, 1, 2, 2>(0, 0) = (DataBuffer.format<uchar, 96, 32>().select<1, 1, 2, 1>(Buffer_Index2 + Channel_Offset_A_0, 2) & 0x30) << 2;
    Sampler_Lumakey_Temp_Bit_67.select<1, 1, 2, 2>(0, 1) = DataBuffer.format<uchar, 96, 32>().select<1, 1, 2, 1>(Buffer_Index2 + Channel_Offset_A_0, 2) & 0xC0;

    Sampler_Lumakey_Temp_Bit_01 = Sampler_Lumakey_Temp_Bit_01 | Sampler_Lumakey_Temp_Bit_23;
    Sampler_Lumakey_Temp_Bit_01 = Sampler_Lumakey_Temp_Bit_01 | Sampler_Lumakey_Temp_Bit_45;
    Sampler_Lumakey_Temp_Bit_01 = Sampler_Lumakey_Temp_Bit_01 | Sampler_Lumakey_Temp_Bit_67;

    TempMask[Mask_Index] = TempMask[Mask_Index] & Sampler_Lumakey_Temp_Bit_01.format<ushort, 1, 2>().row(0)[0];
    TempMask[Mask_Index + 1] = TempMask[Mask_Index + 1] & Sampler_Lumakey_Temp_Bit_01.format<ushort, 1, 2>().row(0)[1];

    // 2nd half
    Sampler_Lumakey_Temp_Bit_01.select<1, 1, 2, 2>(0, 0) = DataBuffer.format<uchar, 96, 32>().select<1, 1, 2, 1>(Buffer_Index2 + Channel_Offset_A_0 + 1, 0) & 0x03;
    Sampler_Lumakey_Temp_Bit_01.select<1, 1, 2, 2>(0, 1) = (DataBuffer.format<uchar, 96, 32>().select<1, 1, 2, 1>(Buffer_Index2 + Channel_Offset_A_0 + 1, 0) & 0x0C) >> 2;

    Sampler_Lumakey_Temp_Bit_23.select<1, 1, 2, 2>(0, 0) = (DataBuffer.format<uchar, 96, 32>().select<1, 1, 2, 1>(Buffer_Index2 + Channel_Offset_A_0 + 1, 0) & 0x30) >> 2;
    Sampler_Lumakey_Temp_Bit_23.select<1, 1, 2, 2>(0, 1) = (DataBuffer.format<uchar, 96, 32>().select<1, 1, 2, 1>(Buffer_Index2 + Channel_Offset_A_0 + 1, 0) & 0xC0) >> 4;

    Sampler_Lumakey_Temp_Bit_45.select<1, 1, 2, 2>(0, 0) = (DataBuffer.format<uchar, 96, 32>().select<1, 1, 2, 1>(Buffer_Index2 + Channel_Offset_A_0 + 1, 2) & 0x03) << 4;
    Sampler_Lumakey_Temp_Bit_45.select<1, 1, 2, 2>(0, 1) = (DataBuffer.format<uchar, 96, 32>().select<1, 1, 2, 1>(Buffer_Index2 + Channel_Offset_A_0 + 1, 2) & 0x0C) << 2;

    Sampler_Lumakey_Temp_Bit_67.select<1, 1, 2, 2>(0, 0) = (DataBuffer.format<uchar, 96, 32>().select<1, 1, 2, 1>(Buffer_Index2 + Channel_Offset_A_0 + 1, 2) & 0x30) << 2;
    Sampler_Lumakey_Temp_Bit_67.select<1, 1, 2, 2>(0, 1) = DataBuffer.format<uchar, 96, 32>().select<1, 1, 2, 1>(Buffer_Index2 + Channel_Offset_A_0 + 1, 2) & 0xC0;

    Sampler_Lumakey_Temp_Bit_01 = Sampler_Lumakey_Temp_Bit_01 | Sampler_Lumakey_Temp_Bit_23;
    Sampler_Lumakey_Temp_Bit_01 = Sampler_Lumakey_Temp_Bit_01 | Sampler_Lumakey_Temp_Bit_45;
    Sampler_Lumakey_Temp_Bit_01 = Sampler_Lumakey_Temp_Bit_01 | Sampler_Lumakey_Temp_Bit_67;

    TempMask[Mask_Index + 2] = TempMask[Mask_Index + 2] & Sampler_Lumakey_Temp_Bit_01.format<ushort, 1, 2>().row(0)[0];
    TempMask[Mask_Index + 3] = TempMask[Mask_Index + 3] & Sampler_Lumakey_Temp_Bit_01.format<ushort, 1, 2>().row(0)[1];
}