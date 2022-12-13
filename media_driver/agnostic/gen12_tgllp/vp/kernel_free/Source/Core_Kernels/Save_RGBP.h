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

    matrix_ref<uchar, 16, 16> Result_R = DataBuffer.format<uchar, 192, 16>().select<16, 1, 16, 1>(128, 0);
    matrix_ref<uchar, 16, 16> Result_G = DataBuffer.format<uchar, 192, 16>().select<16, 1, 16, 1>(144, 0);
    matrix_ref<uchar, 16, 16> Result_B = DataBuffer.format<uchar, 192, 16>().select<16, 1, 16, 1>(160, 0);

    SurfaceIndex Dst_Surface_G(MDF_FC_OUTPUT_BTI_START);
    SurfaceIndex Dst_Surface_B(MDF_FC_OUTPUT_BTI_START + MDF_FC_U_PLANE_BTI_OFFSET);
    SurfaceIndex Dst_Surface_R(MDF_FC_OUTPUT_BTI_START + MDF_FC_V_PLANE_BTI_OFFSET);

#pragma unroll
    // Rounding G plane
    for (uchar i = 0; i < 4; i++)
    {
        DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(16 * i + 2, 0) = cm_add<ushort>(DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(16 * i + 2, 0), 0x80, SAT);
        DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(16 * i + 3, 0) = cm_add<ushort>(DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(16 * i + 3, 0), 0x80, SAT);
        DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(16 * i + 10, 0) = cm_add<ushort>(DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(16 * i + 10, 0), 0x80, SAT);
        DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(16 * i + 11, 0) = cm_add<ushort>(DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(16 * i + 11, 0), 0x80, SAT);
    }

#pragma unroll
    // Write G plane
    for (uchar i = 0; i < 4; i++)
    {
        Result_G.select<1, 1, 8, 1>(4 * i, 0) = DataBuffer.format<uchar, 96, 32>().select<1, 1, 8, 2>(16 * i + 2, 1);
        Result_G.select<1, 1, 8, 1>(4 * i, 8) = DataBuffer.format<uchar, 96, 32>().select<1, 1, 8, 2>(16 * i + 10, 1);
        Result_G.select<1, 1, 8, 1>(4 * i + 1, 0) = DataBuffer.format<uchar, 96, 32>().select<1, 1, 8, 2>(16 * i + 2, 17);
        Result_G.select<1, 1, 8, 1>(4 * i + 1, 8) = DataBuffer.format<uchar, 96, 32>().select<1, 1, 8, 2>(16 * i + 10, 17);
        Result_G.select<1, 1, 8, 1>(4 * i + 2, 0) = DataBuffer.format<uchar, 96, 32>().select<1, 1, 8, 2>(16 * i + 3, 1);
        Result_G.select<1, 1, 8, 1>(4 * i + 2, 8) = DataBuffer.format<uchar, 96, 32>().select<1, 1, 8, 2>(16 * i + 11, 1);
        Result_G.select<1, 1, 8, 1>(4 * i + 3, 0) = DataBuffer.format<uchar, 96, 32>().select<1, 1, 8, 2>(16 * i + 3, 17);
        Result_G.select<1, 1, 8, 1>(4 * i + 3, 8) = DataBuffer.format<uchar, 96, 32>().select<1, 1, 8, 2>(16 * i + 11, 17);
    }

#pragma unroll
    // Rounding B plane
    for (uchar i = 0; i < 4; i++)
    {
        DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(16 * i + 4, 0) = cm_add<ushort>(DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(16 * i + 4, 0), 0x80, SAT);
        DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(16 * i + 5, 0) = cm_add<ushort>(DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(16 * i + 5, 0), 0x80, SAT);
        DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(16 * i + 12, 0) = cm_add<ushort>(DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(16 * i + 12, 0), 0x80, SAT);
        DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(16 * i + 13, 0) = cm_add<ushort>(DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(16 * i + 13, 0), 0x80, SAT);
    }

#pragma unroll
    // Write B plane
    for (uchar i = 0; i < 4; i++)
    {
        Result_B.select<1, 1, 8, 1>(4 * i, 0) = DataBuffer.format<uchar, 96, 32>().select<1, 1, 8, 2>(16 * i + 4, 1);
        Result_B.select<1, 1, 8, 1>(4 * i, 8) = DataBuffer.format<uchar, 96, 32>().select<1, 1, 8, 2>(16 * i + 12, 1);
        Result_B.select<1, 1, 8, 1>(4 * i + 1, 0) = DataBuffer.format<uchar, 96, 32>().select<1, 1, 8, 2>(16 * i + 4, 17);
        Result_B.select<1, 1, 8, 1>(4 * i + 1, 8) = DataBuffer.format<uchar, 96, 32>().select<1, 1, 8, 2>(16 * i + 12, 17);
        Result_B.select<1, 1, 8, 1>(4 * i + 2, 0) = DataBuffer.format<uchar, 96, 32>().select<1, 1, 8, 2>(16 * i + 5, 1);
        Result_B.select<1, 1, 8, 1>(4 * i + 2, 8) = DataBuffer.format<uchar, 96, 32>().select<1, 1, 8, 2>(16 * i + 13, 1);
        Result_B.select<1, 1, 8, 1>(4 * i + 3, 0) = DataBuffer.format<uchar, 96, 32>().select<1, 1, 8, 2>(16 * i + 5, 17);
        Result_B.select<1, 1, 8, 1>(4 * i + 3, 8) = DataBuffer.format<uchar, 96, 32>().select<1, 1, 8, 2>(16 * i + 13, 17);
    }

#pragma unroll
    // Rounding R plane
    for (uchar i = 0; i < 4; i++)
    {
        DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(16 * i + 0, 0) = cm_add<ushort>(DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(16 * i + 0, 0), 0x80, SAT);
        DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(16 * i + 1, 0) = cm_add<ushort>(DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(16 * i + 1, 0), 0x80, SAT);
        DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(16 * i + 8, 0) = cm_add<ushort>(DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(16 * i + 8, 0), 0x80, SAT);
        DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(16 * i + 9, 0) = cm_add<ushort>(DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(16 * i + 9, 0), 0x80, SAT);
    }

#pragma unroll
    // Write R plane
    for (uchar i = 0; i < 4; i++)
    {
        Result_R.select<1, 1, 8, 1>(4 * i, 0) = DataBuffer.format<uchar, 96, 32>().select<1, 1, 8, 2>(16 * i + 0, 1);
        Result_R.select<1, 1, 8, 1>(4 * i, 8) = DataBuffer.format<uchar, 96, 32>().select<1, 1, 8, 2>(16 * i + 8, 1);
        Result_R.select<1, 1, 8, 1>(4 * i + 1, 0) = DataBuffer.format<uchar, 96, 32>().select<1, 1, 8, 2>(16 * i + 0, 17);
        Result_R.select<1, 1, 8, 1>(4 * i + 1, 8) = DataBuffer.format<uchar, 96, 32>().select<1, 1, 8, 2>(16 * i + 8, 17);
        Result_R.select<1, 1, 8, 1>(4 * i + 2, 0) = DataBuffer.format<uchar, 96, 32>().select<1, 1, 8, 2>(16 * i + 1, 1);
        Result_R.select<1, 1, 8, 1>(4 * i + 2, 8) = DataBuffer.format<uchar, 96, 32>().select<1, 1, 8, 2>(16 * i + 9, 1);
        Result_R.select<1, 1, 8, 1>(4 * i + 3, 0) = DataBuffer.format<uchar, 96, 32>().select<1, 1, 8, 2>(16 * i + 1, 17);
        Result_R.select<1, 1, 8, 1>(4 * i + 3, 8) = DataBuffer.format<uchar, 96, 32>().select<1, 1, 8, 2>(16 * i + 9, 17);
    }

    write(Dst_Surface_G, DstX, DstY, Result_G);
    write(Dst_Surface_B, DstX, DstY, Result_B);
    write(Dst_Surface_R, DstX, DstY, Result_R);
}