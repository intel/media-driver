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

_GENX_MAIN_ void Compute_Lumakey(
    CURBE_INPUT_OUTPUT,
    GLOBAL_BUFFER_INPUT_OUTPUT)
{
    if (Buffer_Index)
    {
        uchar Mask_Index = cm_add<uchar>((Buffer_Index >> 4), -4, SAT);
        Mask_Index = Mask_Index << 2;
        Mask_Index = Mask_Index + ((Layer_Index & 0x80) >> 4); // Shift right Layer_Index 7 bits for sec half flag, left 3 bits for mask offset

        uchar Buffer_Index1 = (Buffer_Index & 0x0f) << 4;
        uchar Buffer_Index2 = (Buffer_Index >> 4) << 4;
        ushort LowLuma = LumakeyLowThreshold << 8;
        ushort HighLuma = LumakeyHighThreshold << 8;
        vector<ushort, 16> Lumakey_mask;
        ushort Temp_packed_mask;
        matrix <ushort, 1, 16> Compare_temp;

        /*
        Buffer layout after shuffle
        _________________________________________________
        |_______Block0__________|_______Block1__________|
        |_______Block2__________|_______Block3__________|
        |_______Block4__________|_______Block5__________|
        |_______Block6__________|_______Block7__________|

        Write back buffer layout correlate to the block number#, each box stands for 1 GRF
        _______________________________________________
        |____R0_________R1_____|____R2_________R3_____|
        |____G0_________G1_____|____G2_________G3_____|
        |____B0_________B1_____|____B2_________B3_____|
        |____A0_________A1_____|____A2_________A3_____|
        |____R4_________R5_____|____R6_________R7_____|
        |____G4_________G5_____|____G6_________G7_____|
        |____B4_________B5_____|____B6_________B7_____|
        |____A4_________A5_____|____A6_________A7_____|
        */
        matrix_ref<ushort, 16, 16> WriteBackBuffer1 = DataBuffer.format<ushort, 96, 16>().select<16, 1, 16, 1>(Buffer_Index1, 0);
        matrix_ref<ushort, 16, 16> WriteBackBuffer2 = DataBuffer.format<ushort, 96, 16>().select<16, 1, 16, 1>(Buffer_Index2, 0);

        Compare_temp = WriteBackBuffer2.select<1, 1, 16, 1>(2, 0) & 0xFF00;

        Lumakey_mask = Compare_temp > HighLuma;
        Lumakey_mask |= Compare_temp < LowLuma;

        Temp_packed_mask = cm_pack_mask(Lumakey_mask);
        TempMask[Mask_Index] = Temp_packed_mask & TempMask[Mask_Index];

        // U/V/A channels bottom half
        Compare_temp = WriteBackBuffer2.select<1, 1, 16, 1>(3, 0) & 0xFF00;

        Lumakey_mask = Compare_temp > HighLuma;
        Lumakey_mask |= Compare_temp < LowLuma;

        Temp_packed_mask = cm_pack_mask(Lumakey_mask);
        TempMask[Mask_Index + 1] = Temp_packed_mask & TempMask[Mask_Index + 1];

        // U/V/A channels bottom half
        Compare_temp = WriteBackBuffer2.select<1, 1, 16, 1>(10, 0) & 0xFF00;

        Lumakey_mask = Compare_temp > HighLuma;
        Lumakey_mask |= Compare_temp < LowLuma;

        Temp_packed_mask = cm_pack_mask(Lumakey_mask);
        TempMask[Mask_Index + 2] = Temp_packed_mask & TempMask[Mask_Index + 2];

        // U/V/A channels bottom half
        Compare_temp = WriteBackBuffer2.select<1, 1, 16, 1>(11, 0) & 0xFF00;

        Lumakey_mask = Compare_temp > HighLuma;
        Lumakey_mask |= Compare_temp < LowLuma;

        Temp_packed_mask = cm_pack_mask(Lumakey_mask);
        TempMask[Mask_Index + 3] = Temp_packed_mask & TempMask[Mask_Index + 3];
    }
}