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
        // First 8x16
        {
            // R/G/B/A channels
            TempResult4x8_Top.format<ushort, 4, 16>().select<1, 1, 8, 1>(0, 0) = (DataBuffer.format<ushort, 96, 16>().select<1, 1, 8, 1>(4 + 32 * i, 0) >> 11) + ((DataBuffer.format<ushort, 96, 16>().select<1, 1, 8, 1>(2 + 32 * i, 0) >> 10) << 5) + (DataBuffer.format<ushort, 96, 16>().select<1, 1, 8, 1>(32 * i, 0) & 0xF800);
            TempResult4x8_Top.format<ushort, 4, 16>().select<1, 1, 8, 1>(0, 8) = (DataBuffer.format<ushort, 96, 16>().select<1, 1, 8, 1>(12+ 32 * i, 0) >> 11) + ((DataBuffer.format<ushort, 96, 16>().select<1, 1, 8, 1>(10 + 32 * i, 0) >> 10) << 5) + (DataBuffer.format<ushort, 96, 16>().select<1, 1, 8, 1>(8 + 32 * i, 0) & 0xF800);
            TempResult4x8_Top.format<ushort, 4, 16>().select<1, 1, 8, 1>(1, 0) = (DataBuffer.format<ushort, 96, 16>().select<1, 1, 8, 1>(4 + 32 * i, 8) >> 11) + ((DataBuffer.format<ushort, 96, 16>().select<1, 1, 8, 1>(2 + 32 * i, 8) >> 10) << 5) + (DataBuffer.format<ushort, 96, 16>().select<1, 1, 8, 1>(32 * i, 8) & 0xF800);
            TempResult4x8_Top.format<ushort, 4, 16>().select<1, 1, 8, 1>(1, 8) = (DataBuffer.format<ushort, 96, 16>().select<1, 1, 8, 1>(12 + 32 * i, 8) >> 11) + ((DataBuffer.format<ushort, 96, 16>().select<1, 1, 8, 1>(10 + 32 * i, 8) >> 10) << 5) + (DataBuffer.format<ushort, 96, 16>().select<1, 1, 8, 1>(8 + 32 * i, 8) & 0xF800);

            TempResult4x8_Top.format<ushort, 4, 16>().select<1, 1, 8, 1>(2, 0) = (DataBuffer.format<ushort, 96, 16>().select<1, 1, 8, 1>(5 + 32 * i, 0) >> 11) + ((DataBuffer.format<ushort, 96, 16>().select<1, 1, 8, 1>(3 + 32 * i, 0) >> 10) << 5) + (DataBuffer.format<ushort, 96, 16>().select<1, 1, 8, 1>(1 + 32 * i, 0) & 0xF800);
            TempResult4x8_Top.format<ushort, 4, 16>().select<1, 1, 8, 1>(2, 8) = (DataBuffer.format<ushort, 96, 16>().select<1, 1, 8, 1>(13 + 32 * i, 0) >> 11) + ((DataBuffer.format<ushort, 96, 16>().select<1, 1, 8, 1>(11 + 32 * i, 0) >> 10) << 5) + (DataBuffer.format<ushort, 96, 16>().select<1, 1, 8, 1>(9 + 32 * i, 0) & 0xF800);
            TempResult4x8_Top.format<ushort, 4, 16>().select<1, 1, 8, 1>(3, 0) = (DataBuffer.format<ushort, 96, 16>().select<1, 1, 8, 1>(5 + 32 * i, 8) >> 11) + ((DataBuffer.format<ushort, 96, 16>().select<1, 1, 8, 1>(3 + 32 * i, 8) >> 10) << 5) + (DataBuffer.format<ushort, 96, 16>().select<1, 1, 8, 1>(1 + 32 * i, 8) & 0xF800);
            TempResult4x8_Top.format<ushort, 4, 16>().select<1, 1, 8, 1>(3, 8) = (DataBuffer.format<ushort, 96, 16>().select<1, 1, 8, 1>(13 + 32 * i, 8) >> 11) + ((DataBuffer.format<ushort, 96, 16>().select<1, 1, 8, 1>(11 + 32 * i, 8) >> 10) << 5) + (DataBuffer.format<ushort, 96, 16>().select<1, 1, 8, 1>(9 + 32 * i, 8) & 0xF800);


            TempResult4x8_Bottom.format<ushort, 4, 16>().select<1, 1, 8, 1>(0, 0) = (DataBuffer.format<ushort, 96, 16>().select<1, 1, 8, 1>(4 + 16 + 32 * i, 0) >> 11) + ((DataBuffer.format<ushort, 96, 16>().select<1, 1, 8, 1>(2 + 16 + 32 * i, 0) >> 10) << 5) + (DataBuffer.format<ushort, 96, 16>().select<1, 1, 8, 1>(16 + 32 * i, 0) & 0xF800);
            TempResult4x8_Bottom.format<ushort, 4, 16>().select<1, 1, 8, 1>(0, 8) = (DataBuffer.format<ushort, 96, 16>().select<1, 1, 8, 1>(12 + 16 + 32 * i, 0) >> 11) + ((DataBuffer.format<ushort, 96, 16>().select<1, 1, 8, 1>(10 + 16 + 32 * i, 0) >> 10) << 5) + (DataBuffer.format<ushort, 96, 16>().select<1, 1, 8, 1>(8 + 16 + 32 * i, 0) & 0xF800);
            TempResult4x8_Bottom.format<ushort, 4, 16>().select<1, 1, 8, 1>(1, 0) = (DataBuffer.format<ushort, 96, 16>().select<1, 1, 8, 1>(4 + 16 + 32 * i, 8) >> 11) + ((DataBuffer.format<ushort, 96, 16>().select<1, 1, 8, 1>(2 + 16 + 32 * i, 8) >> 10) << 5) + (DataBuffer.format<ushort, 96, 16>().select<1, 1, 8, 1>(16 + 32 * i, 8) & 0xF800);
            TempResult4x8_Bottom.format<ushort, 4, 16>().select<1, 1, 8, 1>(1, 8) = (DataBuffer.format<ushort, 96, 16>().select<1, 1, 8, 1>(12 + 16 + 32 * i, 8) >> 11) + ((DataBuffer.format<ushort, 96, 16>().select<1, 1, 8, 1>(10 + 16 + 32 * i, 8) >> 10) << 5) + (DataBuffer.format<ushort, 96, 16>().select<1, 1, 8, 1>(8 + 16 + 32 * i, 8) & 0xF800);

            TempResult4x8_Bottom.format<ushort, 4, 16>().select<1, 1, 8, 1>(2, 0) = (DataBuffer.format<ushort, 96, 16>().select<1, 1, 8, 1>(5 + 16 + 32 * i, 0) >> 11) + ((DataBuffer.format<ushort, 96, 16>().select<1, 1, 8, 1>(3 + 16 + 32 * i, 0) >> 10) << 5) + (DataBuffer.format<ushort, 96, 16>().select<1, 1, 8, 1>(1 + 16 + 32 * i, 0) & 0xF800);
            TempResult4x8_Bottom.format<ushort, 4, 16>().select<1, 1, 8, 1>(2, 8) = (DataBuffer.format<ushort, 96, 16>().select<1, 1, 8, 1>(13 + 16 + 32 * i, 0) >> 11) + ((DataBuffer.format<ushort, 96, 16>().select<1, 1, 8, 1>(11 + 16 + 32 * i, 0) >> 10) << 5) + (DataBuffer.format<ushort, 96, 16>().select<1, 1, 8, 1>(9 + 16 + 32 * i, 0) & 0xF800);
            TempResult4x8_Bottom.format<ushort, 4, 16>().select<1, 1, 8, 1>(3, 0) = (DataBuffer.format<ushort, 96, 16>().select<1, 1, 8, 1>(5 + 16 + 32 * i, 8) >> 11) + ((DataBuffer.format<ushort, 96, 16>().select<1, 1, 8, 1>(3 + 16 + 32 * i, 8) >> 10) << 5) + (DataBuffer.format<ushort, 96, 16>().select<1, 1, 8, 1>(1 + 16 + 32 * i, 8) & 0xF800);
            TempResult4x8_Bottom.format<ushort, 4, 16>().select<1, 1, 8, 1>(3, 8) = (DataBuffer.format<ushort, 96, 16>().select<1, 1, 8, 1>(13 + 16 + 32 * i, 8) >> 11) + ((DataBuffer.format<ushort, 96, 16>().select<1, 1, 8, 1>(11 + 16 + 32 * i, 8) >> 10) << 5) + (DataBuffer.format<ushort, 96, 16>().select<1, 1, 8, 1>(9 + 16 + 32 * i, 8) & 0xF800);

            write(Dst_Surface, DstX * 2, DstY, Result);
        }
    }
}