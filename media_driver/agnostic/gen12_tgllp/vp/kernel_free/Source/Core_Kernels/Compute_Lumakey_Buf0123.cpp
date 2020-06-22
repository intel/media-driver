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

_GENX_MAIN_ void Compute_Lumakey_Buf0123(
    CURBE_INPUT_OUTPUT,
    GLOBAL_BUFFER_INPUT_OUTPUT)
{
    ushort LowLuma = LumakeyLowThreshold << 8;
    ushort HighLuma = LumakeyHighThreshold << 8;
    vector<ushort, 16> Lumakey_mask;
    ushort Temp_packed_mask;
    matrix <ushort, 1, 16> Compare_temp;

#pragma unroll
    for (int i = 0; i < 4; i++)
    {
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

        Compare_temp = DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(16 * i + 2, 0) & 0xFF00;

        Lumakey_mask = Compare_temp > HighLuma;
        Lumakey_mask |= Compare_temp < LowLuma;

        Temp_packed_mask = cm_pack_mask(Lumakey_mask);
        TempMask0[0][4 * i] = Temp_packed_mask & TempMask0[0][4 * i];

        Compare_temp = DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(16 * i + 3, 0) & 0xFF00;

        Lumakey_mask = Compare_temp > HighLuma;
        Lumakey_mask |= Compare_temp < LowLuma;

        Temp_packed_mask = cm_pack_mask(Lumakey_mask);
        TempMask0[0][4 * i + 1] = Temp_packed_mask & TempMask0[0][4 * i + 1];

        // U/V/A channels bottom half
        Compare_temp = DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(16 * i + 10, 0) & 0xFF00;

        Lumakey_mask = Compare_temp > HighLuma;
        Lumakey_mask |= Compare_temp < LowLuma;

        Temp_packed_mask = cm_pack_mask(Lumakey_mask);
        TempMask0[0][4 * i + 2] = Temp_packed_mask & TempMask0[0][4 * i + 2];

        Compare_temp = DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(16 * i + 11, 0) & 0xFF00;

        Lumakey_mask = Compare_temp > HighLuma;
        Lumakey_mask |= Compare_temp < LowLuma;

        Temp_packed_mask = cm_pack_mask(Lumakey_mask);
        TempMask0[0][4 * i + 3] = Temp_packed_mask & TempMask0[0][4 * i + 3];
    }
}