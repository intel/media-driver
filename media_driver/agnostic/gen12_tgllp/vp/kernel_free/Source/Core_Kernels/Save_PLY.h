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

    matrix_ref<uchar, 16, 16> Result_Y = DataBuffer.format<uchar, 192, 16>().select<16, 1, 16, 1>(128, 0);

    SurfaceIndex Dst_Surface(MDF_FC_OUTPUT_BTI_START);

#pragma unroll
    // Write Y plane
    for (uchar i = 0; i < 4; i++)
    {
        Result_Y.select<1, 1, 8, 1>(4 * i, 0) = DataBuffer.format<uchar, 96, 32>().select<1, 1, 8, 2>(16 * i + 2, 1);
        Result_Y.select<1, 1, 8, 1>(4 * i, 8) = DataBuffer.format<uchar, 96, 32>().select<1, 1, 8, 2>(16 * i + 10, 1);
        Result_Y.select<1, 1, 8, 1>(4 * i + 1, 0) = DataBuffer.format<uchar, 96, 32>().select<1, 1, 8, 2>(16 * i + 2, 17);
        Result_Y.select<1, 1, 8, 1>(4 * i + 1, 8) = DataBuffer.format<uchar, 96, 32>().select<1, 1, 8, 2>(16 * i + 10, 17);
        Result_Y.select<1, 1, 8, 1>(4 * i + 2, 0) = DataBuffer.format<uchar, 96, 32>().select<1, 1, 8, 2>(16 * i + 3, 1);
        Result_Y.select<1, 1, 8, 1>(4 * i + 2, 8) = DataBuffer.format<uchar, 96, 32>().select<1, 1, 8, 2>(16 * i + 11, 1);
        Result_Y.select<1, 1, 8, 1>(4 * i + 3, 0) = DataBuffer.format<uchar, 96, 32>().select<1, 1, 8, 2>(16 * i + 3, 17);
        Result_Y.select<1, 1, 8, 1>(4 * i + 3, 8) = DataBuffer.format<uchar, 96, 32>().select<1, 1, 8, 2>(16 * i + 11, 17);
    }

    write(Dst_Surface, DstX, DstY, Result_Y);
}