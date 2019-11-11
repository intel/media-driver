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

    matrix_ref<uint, 8, 8> Result = DataBuffer.format<uint, 96, 8>().select<8, 1, 8, 1>(64, 0);

    SurfaceIndex Dst_Surface(MDF_FC_OUTPUT_BTI_START);
    matrix_ref<uint, 4, 8> TempResult4x8_Top = Result.select<4, 1, 8, 1>(0, 0);
    matrix_ref<uint, 4, 8> TempResult4x8_Bottom = Result.select<4, 1, 8, 1>(4, 0);

#pragma unroll
    for (uchar i = 0; i < 2; i++, DstY += 8)
    {
        // Left 8x16
        {
            // Y channel
            TempResult4x8_Top.format<ushort, 4, 16>().select<1, 1, 8, 2>(0, 0) = DataBuffer.format<ushort, 96, 16>().select<1, 1, 8, 1>(2 + 32 * i, 0);
            TempResult4x8_Top.format<ushort, 4, 16>().select<1, 1, 8, 2>(1, 0) = DataBuffer.format<ushort, 96, 16>().select<1, 1, 8, 1>(2 + 32 * i, 8);
            TempResult4x8_Top.format<ushort, 4, 16>().select<1, 1, 8, 2>(2, 0) = DataBuffer.format<ushort, 96, 16>().select<1, 1, 8, 1>(3 + 32 * i, 0);
            TempResult4x8_Top.format<ushort, 4, 16>().select<1, 1, 8, 2>(3, 0) = DataBuffer.format<ushort, 96, 16>().select<1, 1, 8, 1>(3 + 32 * i, 8);
            TempResult4x8_Bottom.format<ushort, 4, 16>().select<1, 1, 8, 2>(0, 0) = DataBuffer.format<ushort, 96, 16>().select<1, 1, 8, 1>(18 + 32 * i, 0);
            TempResult4x8_Bottom.format<ushort, 4, 16>().select<1, 1, 8, 2>(1, 0) = DataBuffer.format<ushort, 96, 16>().select<1, 1, 8, 1>(18 + 32 * i, 8);
            TempResult4x8_Bottom.format<ushort, 4, 16>().select<1, 1, 8, 2>(2, 0) = DataBuffer.format<ushort, 96, 16>().select<1, 1, 8, 1>(19 + 32 * i, 0);
            TempResult4x8_Bottom.format<ushort, 4, 16>().select<1, 1, 8, 2>(3, 0) = DataBuffer.format<ushort, 96, 16>().select<1, 1, 8, 1>(19 + 32 * i, 8);


            // UV channel
            // V channel
            TempResult4x8_Top.format<ushort, 4, 16>().select<1, 1, 4, 4>(0, 3) = DataBuffer.format<ushort, 96, 16>().select<1, 1, 4, 2>(32 * i, 0);
            TempResult4x8_Top.format<ushort, 4, 16>().select<1, 1, 4, 4>(1, 3) = DataBuffer.format<ushort, 96, 16>().select<1, 1, 4, 2>(32 * i, 8);
            TempResult4x8_Top.format<ushort, 4, 16>().select<1, 1, 4, 4>(2, 3) = DataBuffer.format<ushort, 96, 16>().select<1, 1, 4, 2>(32 * i + 1, 0);
            TempResult4x8_Top.format<ushort, 4, 16>().select<1, 1, 4, 4>(3, 3) = DataBuffer.format<ushort, 96, 16>().select<1, 1, 4, 2>(32 * i + 1, 8);
            TempResult4x8_Bottom.format<ushort, 4, 16>().select<1, 1, 4, 4>(0, 3) = DataBuffer.format<ushort, 96, 16>().select<1, 1, 4, 2>(32 * i + 16, 0);
            TempResult4x8_Bottom.format<ushort, 4, 16>().select<1, 1, 4, 4>(1, 3) = DataBuffer.format<ushort, 96, 16>().select<1, 1, 4, 2>(32 * i + 16, 8);
            TempResult4x8_Bottom.format<ushort, 4, 16>().select<1, 1, 4, 4>(2, 3) = DataBuffer.format<ushort, 96, 16>().select<1, 1, 4, 2>(32 * i + 17, 0);
            TempResult4x8_Bottom.format<ushort, 4, 16>().select<1, 1, 4, 4>(3, 3) = DataBuffer.format<ushort, 96, 16>().select<1, 1, 4, 2>(32 * i + 17, 8);

            // U channel
            TempResult4x8_Top.format<ushort, 4, 16>().select<1, 1, 4, 4>(0, 1) = DataBuffer.format<ushort, 96, 16>().select<1, 1, 4, 2>(32 * i + 4, 0);
            TempResult4x8_Top.format<ushort, 4, 16>().select<1, 1, 4, 4>(1, 1) = DataBuffer.format<ushort, 96, 16>().select<1, 1, 4, 2>(32 * i + 4, 8);
            TempResult4x8_Top.format<ushort, 4, 16>().select<1, 1, 4, 4>(2, 1) = DataBuffer.format<ushort, 96, 16>().select<1, 1, 4, 2>(32 * i + 5, 0);
            TempResult4x8_Top.format<ushort, 4, 16>().select<1, 1, 4, 4>(3, 1) = DataBuffer.format<ushort, 96, 16>().select<1, 1, 4, 2>(32 * i + 5, 8);
            TempResult4x8_Bottom.format<ushort, 4, 16>().select<1, 1, 4, 4>(0, 1) = DataBuffer.format<ushort, 96, 16>().select<1, 1, 4, 2>(32 * i + 20, 0);
            TempResult4x8_Bottom.format<ushort, 4, 16>().select<1, 1, 4, 4>(1, 1) = DataBuffer.format<ushort, 96, 16>().select<1, 1, 4, 2>(32 * i + 20, 8);
            TempResult4x8_Bottom.format<ushort, 4, 16>().select<1, 1, 4, 4>(2, 1) = DataBuffer.format<ushort, 96, 16>().select<1, 1, 4, 2>(32 * i + 21, 0);
            TempResult4x8_Bottom.format<ushort, 4, 16>().select<1, 1, 4, 4>(3, 1) = DataBuffer.format<ushort, 96, 16>().select<1, 1, 4, 2>(32 * i + 21, 8);

            write(Dst_Surface, 4 * DstX, DstY, Result);
        }

        // Right 8x16
        {
            // Y channel
            TempResult4x8_Top.format<ushort, 4, 16>().select<1, 1, 8, 2>(0, 0) = DataBuffer.format<ushort, 96, 16>().select<1, 1, 8, 1>(10 + 32 * i, 0);
            TempResult4x8_Top.format<ushort, 4, 16>().select<1, 1, 8, 2>(1, 0) = DataBuffer.format<ushort, 96, 16>().select<1, 1, 8, 1>(10 + 32 * i, 8);
            TempResult4x8_Top.format<ushort, 4, 16>().select<1, 1, 8, 2>(2, 0) = DataBuffer.format<ushort, 96, 16>().select<1, 1, 8, 1>(11 + 32 * i, 0);
            TempResult4x8_Top.format<ushort, 4, 16>().select<1, 1, 8, 2>(3, 0) = DataBuffer.format<ushort, 96, 16>().select<1, 1, 8, 1>(11 + 32 * i, 8);
            TempResult4x8_Bottom.format<ushort, 4, 16>().select<1, 1, 8, 2>(0, 0) = DataBuffer.format<ushort, 96, 16>().select<1, 1, 8, 1>(26 + 32 * i, 0);
            TempResult4x8_Bottom.format<ushort, 4, 16>().select<1, 1, 8, 2>(1, 0) = DataBuffer.format<ushort, 96, 16>().select<1, 1, 8, 1>(26 + 32 * i, 8);
            TempResult4x8_Bottom.format<ushort, 4, 16>().select<1, 1, 8, 2>(2, 0) = DataBuffer.format<ushort, 96, 16>().select<1, 1, 8, 1>(27 + 32 * i, 0);
            TempResult4x8_Bottom.format<ushort, 4, 16>().select<1, 1, 8, 2>(3, 0) = DataBuffer.format<ushort, 96, 16>().select<1, 1, 8, 1>(27 + 32 * i, 8);


            // UV channel
            // V channel
            TempResult4x8_Top.format<ushort, 4, 16>().select<1, 1, 4, 4>(0, 3) = DataBuffer.format<ushort, 96, 16>().select<1, 1, 4, 2>(32 * i + 8, 0);
            TempResult4x8_Top.format<ushort, 4, 16>().select<1, 1, 4, 4>(1, 3) = DataBuffer.format<ushort, 96, 16>().select<1, 1, 4, 2>(32 * i + 8, 8);
            TempResult4x8_Top.format<ushort, 4, 16>().select<1, 1, 4, 4>(2, 3) = DataBuffer.format<ushort, 96, 16>().select<1, 1, 4, 2>(32 * i + 9, 0);
            TempResult4x8_Top.format<ushort, 4, 16>().select<1, 1, 4, 4>(3, 3) = DataBuffer.format<ushort, 96, 16>().select<1, 1, 4, 2>(32 * i + 9, 8);
            TempResult4x8_Bottom.format<ushort, 4, 16>().select<1, 1, 4, 4>(0, 3) = DataBuffer.format<ushort, 96, 16>().select<1, 1, 4, 2>(32 * i + 24, 0);
            TempResult4x8_Bottom.format<ushort, 4, 16>().select<1, 1, 4, 4>(1, 3) = DataBuffer.format<ushort, 96, 16>().select<1, 1, 4, 2>(32 * i + 24, 8);
            TempResult4x8_Bottom.format<ushort, 4, 16>().select<1, 1, 4, 4>(2, 3) = DataBuffer.format<ushort, 96, 16>().select<1, 1, 4, 2>(32 * i + 25, 0);
            TempResult4x8_Bottom.format<ushort, 4, 16>().select<1, 1, 4, 4>(3, 3) = DataBuffer.format<ushort, 96, 16>().select<1, 1, 4, 2>(32 * i + 25, 8);

            // U channel
            TempResult4x8_Top.format<ushort, 4, 16>().select<1, 1, 4, 4>(0, 1) = DataBuffer.format<ushort, 96, 16>().select<1, 1, 4, 2>(32 * i + 12, 0);
            TempResult4x8_Top.format<ushort, 4, 16>().select<1, 1, 4, 4>(1, 1) = DataBuffer.format<ushort, 96, 16>().select<1, 1, 4, 2>(32 * i + 12, 8);
            TempResult4x8_Top.format<ushort, 4, 16>().select<1, 1, 4, 4>(2, 1) = DataBuffer.format<ushort, 96, 16>().select<1, 1, 4, 2>(32 * i + 13, 0);
            TempResult4x8_Top.format<ushort, 4, 16>().select<1, 1, 4, 4>(3, 1) = DataBuffer.format<ushort, 96, 16>().select<1, 1, 4, 2>(32 * i + 13, 8);
            TempResult4x8_Bottom.format<ushort, 4, 16>().select<1, 1, 4, 4>(0, 1) = DataBuffer.format<ushort, 96, 16>().select<1, 1, 4, 2>(32 * i + 28, 0);
            TempResult4x8_Bottom.format<ushort, 4, 16>().select<1, 1, 4, 4>(1, 1) = DataBuffer.format<ushort, 96, 16>().select<1, 1, 4, 2>(32 * i + 28, 8);
            TempResult4x8_Bottom.format<ushort, 4, 16>().select<1, 1, 4, 4>(2, 1) = DataBuffer.format<ushort, 96, 16>().select<1, 1, 4, 2>(32 * i + 29, 0);
            TempResult4x8_Bottom.format<ushort, 4, 16>().select<1, 1, 4, 4>(3, 1) = DataBuffer.format<ushort, 96, 16>().select<1, 1, 4, 2>(32 * i + 29, 8);

            write(Dst_Surface, 4 * DstX + 32, DstY, Result);
        }
    }
}