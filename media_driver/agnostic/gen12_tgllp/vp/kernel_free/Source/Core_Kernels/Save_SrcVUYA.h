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
    matrix_ref<ushort, 4, 16> TempResult4x8_Top = Result.format<ushort, 8, 16>().select<4, 1, 16, 1>(0, 0);
    matrix_ref<ushort, 4, 16> TempResult4x8_Bottom = Result.format<ushort, 8, 16>().select<4, 1, 16, 1>(4, 0);

#pragma unroll
    for (uchar i = 0; i < 2; i++, DstY += 8)
    {
        // First 8x8
        {
            // R/G/B channel 1st half
            matrix_ref<ushort, 1, 8> TempR0 = DataBuffer.select<1, 1, 8, 1>(8 * i, 0);
            matrix_ref<ushort, 1, 8> TempR2 = DataBuffer.select<1, 1, 8, 1>(8 * i, 8);
            matrix_ref<ushort, 1, 8> TempR4 = DataBuffer.select<1, 1, 8, 1>(8 * i, 16);
            matrix_ref<ushort, 1, 8> TempR6 = DataBuffer.select<1, 1, 8, 1>(8 * i, 24);

            matrix_ref<ushort, 1, 8> TempG0 = DataBuffer.select<1, 1, 8, 1>(8 * i, 32);
            matrix_ref<ushort, 1, 8> TempG2 = DataBuffer.select<1, 1, 8, 1>(8 * i, 40);
            matrix_ref<ushort, 1, 8> TempG4 = DataBuffer.select<1, 1, 8, 1>(8 * i, 48);
            matrix_ref<ushort, 1, 8> TempG6 = DataBuffer.select<1, 1, 8, 1>(8 * i, 56);

            matrix_ref<ushort, 1, 8> TempB0 = DataBuffer.select<1, 1, 8, 1>(8 * i + 1, 0);
            matrix_ref<ushort, 1, 8> TempB2 = DataBuffer.select<1, 1, 8, 1>(8 * i + 1, 8);
            matrix_ref<ushort, 1, 8> TempB4 = DataBuffer.select<1, 1, 8, 1>(8 * i + 1, 16);
            matrix_ref<ushort, 1, 8> TempB6 = DataBuffer.select<1, 1, 8, 1>(8 * i + 1, 24);

            matrix_ref<ushort, 1, 8> TempA0 = DataBuffer.select<1, 1, 8, 1>(8 * i + 1, 32);
            matrix_ref<ushort, 1, 8> TempA2 = DataBuffer.select<1, 1, 8, 1>(8 * i + 1, 40);
            matrix_ref<ushort, 1, 8> TempA4 = DataBuffer.select<1, 1, 8, 1>(8 * i + 1, 48);
            matrix_ref<ushort, 1, 8> TempA6 = DataBuffer.select<1, 1, 8, 1>(8 * i + 1, 56);

            TempB0 = TempB0 & 0xFF00;
            TempA0 = TempA0 & 0xFF00;

            TempResult4x8_Top.format<ushort, 4, 16>().select<1, 1, 8, 2>(0, 0) = (TempR0 >> 8) + (TempB0);
            TempResult4x8_Top.format<ushort, 4, 16>().select<1, 1, 8, 2>(0, 1) = (TempG0 >> 8) + (TempA0);

            TempB2 = TempB2 & 0xFF00;
            TempA2 = TempA2 & 0xFF00;

            TempResult4x8_Top.format<ushort, 4, 16>().select<1, 1, 8, 2>(1, 0) = (TempR2 >> 8) + (TempB2);
            TempResult4x8_Top.format<ushort, 4, 16>().select<1, 1, 8, 2>(1, 1) = (TempG2 >> 8) + (TempA2);

            TempB4 = TempB4 & 0xFF00;
            TempA4 = TempA4 & 0xFF00;

            TempResult4x8_Top.format<ushort, 4, 16>().select<1, 1, 8, 2>(2, 0) = (TempR4 >> 8) + (TempB4);
            TempResult4x8_Top.format<ushort, 4, 16>().select<1, 1, 8, 2>(2, 1) = (TempG4 >> 8) + (TempA4);

            TempB6 = TempB6 & 0xFF00;
            TempA6 = TempA6 & 0xFF00;

            TempResult4x8_Top.format<ushort, 4, 16>().select<1, 1, 8, 2>(3, 0) = (TempR6 >> 8) + (TempB6);
            TempResult4x8_Top.format<ushort, 4, 16>().select<1, 1, 8, 2>(3, 1) = (TempG6 >> 8) + (TempA6);

            // R/G/B channel 2nd half
            matrix_ref<ushort, 1, 8> TempR8 = DataBuffer.select<1, 1, 8, 1>(8 * i + 4, 0);
            matrix_ref<ushort, 1, 8> TempR10 = DataBuffer.select<1, 1, 8, 1>(8 * i + 4, 8);
            matrix_ref<ushort, 1, 8> TempR12 = DataBuffer.select<1, 1, 8, 1>(8 * i + 4, 16);
            matrix_ref<ushort, 1, 8> TempR14 = DataBuffer.select<1, 1, 8, 1>(8 * i + 4, 24);

            matrix_ref<ushort, 1, 8> TempG8 = DataBuffer.select<1, 1, 8, 1>(8 * i + 4, 32);
            matrix_ref<ushort, 1, 8> TempG10 = DataBuffer.select<1, 1, 8, 1>(8 * i + 4, 40);
            matrix_ref<ushort, 1, 8> TempG12 = DataBuffer.select<1, 1, 8, 1>(8 * i + 4, 48);
            matrix_ref<ushort, 1, 8> TempG14 = DataBuffer.select<1, 1, 8, 1>(8 * i + 4, 56);

            matrix_ref<ushort, 1, 8> TempB8 = DataBuffer.select<1, 1, 8, 1>(8 * i + 5, 0);
            matrix_ref<ushort, 1, 8> TempB10 = DataBuffer.select<1, 1, 8, 1>(8 * i + 5, 8);
            matrix_ref<ushort, 1, 8> TempB12 = DataBuffer.select<1, 1, 8, 1>(8 * i + 5, 16);
            matrix_ref<ushort, 1, 8> TempB14 = DataBuffer.select<1, 1, 8, 1>(8 * i + 5, 24);

            matrix_ref<ushort, 1, 8> TempA8 = DataBuffer.select<1, 1, 8, 1>(8 * i + 5, 32);
            matrix_ref<ushort, 1, 8> TempA10 = DataBuffer.select<1, 1, 8, 1>(8 * i + 5, 40);
            matrix_ref<ushort, 1, 8> TempA12 = DataBuffer.select<1, 1, 8, 1>(8 * i + 5, 48);
            matrix_ref<ushort, 1, 8> TempA14 = DataBuffer.select<1, 1, 8, 1>(8 * i + 5, 56);

            TempB8 = TempB8 & 0xFF00;
            TempA8 = TempA8 & 0xFF00;

            TempResult4x8_Bottom.format<ushort, 4, 16>().select<1, 1, 8, 2>(0, 0) = (TempR8 >> 8) + (TempB8);
            TempResult4x8_Bottom.format<ushort, 4, 16>().select<1, 1, 8, 2>(0, 1) = (TempG8 >> 8) + (TempA8);

            TempB10 = TempB10 & 0xFF00;
            TempA10 = TempA10 & 0xFF00;

            TempResult4x8_Bottom.format<ushort, 4, 16>().select<1, 1, 8, 2>(1, 0) = (TempR10 >> 8) + (TempB10);
            TempResult4x8_Bottom.format<ushort, 4, 16>().select<1, 1, 8, 2>(1, 1) = (TempG10 >> 8) + (TempA10);

            TempB12 = TempB12 & 0xFF00;
            TempA12 = TempA12 & 0xFF00;

            TempResult4x8_Bottom.format<ushort, 4, 16>().select<1, 1, 8, 2>(2, 0) = (TempR12 >> 8) + (TempB12);
            TempResult4x8_Bottom.format<ushort, 4, 16>().select<1, 1, 8, 2>(2, 1) = (TempG12 >> 8) + (TempA12);

            TempB14 = TempB14 & 0xFF00;
            TempA14 = TempA14 & 0xFF00;

            TempResult4x8_Bottom.format<ushort, 4, 16>().select<1, 1, 8, 2>(3, 0) = (TempR14 >> 8) + (TempB14);
            TempResult4x8_Bottom.format<ushort, 4, 16>().select<1, 1, 8, 2>(3, 1) = (TempG14 >> 8) + (TempA14);

            write(Dst_Surface, DstX * 4, DstY, Result);
        }

        // Second 8x8
        {
            // R/G/B channel 1st half
            matrix_ref<ushort, 1, 8> TempR0 = DataBuffer.select<1, 1, 8, 1>(8 * i + 2, 0);
            matrix_ref<ushort, 1, 8> TempR2 = DataBuffer.select<1, 1, 8, 1>(8 * i + 2, 8);
            matrix_ref<ushort, 1, 8> TempR4 = DataBuffer.select<1, 1, 8, 1>(8 * i + 2, 16);
            matrix_ref<ushort, 1, 8> TempR6 = DataBuffer.select<1, 1, 8, 1>(8 * i + 2, 24);

            matrix_ref<ushort, 1, 8> TempG0 = DataBuffer.select<1, 1, 8, 1>(8 * i + 2, 32);
            matrix_ref<ushort, 1, 8> TempG2 = DataBuffer.select<1, 1, 8, 1>(8 * i + 2, 40);
            matrix_ref<ushort, 1, 8> TempG4 = DataBuffer.select<1, 1, 8, 1>(8 * i + 2, 48);
            matrix_ref<ushort, 1, 8> TempG6 = DataBuffer.select<1, 1, 8, 1>(8 * i + 2, 56);

            matrix_ref<ushort, 1, 8> TempB0 = DataBuffer.select<1, 1, 8, 1>(8 * i + 3, 0);
            matrix_ref<ushort, 1, 8> TempB2 = DataBuffer.select<1, 1, 8, 1>(8 * i + 3, 8);
            matrix_ref<ushort, 1, 8> TempB4 = DataBuffer.select<1, 1, 8, 1>(8 * i + 3, 16);
            matrix_ref<ushort, 1, 8> TempB6 = DataBuffer.select<1, 1, 8, 1>(8 * i + 3, 24);

            matrix_ref<ushort, 1, 8> TempA0 = DataBuffer.select<1, 1, 8, 1>(8 * i + 3, 32);
            matrix_ref<ushort, 1, 8> TempA2 = DataBuffer.select<1, 1, 8, 1>(8 * i + 3, 40);
            matrix_ref<ushort, 1, 8> TempA4 = DataBuffer.select<1, 1, 8, 1>(8 * i + 3, 48);
            matrix_ref<ushort, 1, 8> TempA6 = DataBuffer.select<1, 1, 8, 1>(8 * i + 3, 56);

            TempB0 = TempB0 & 0xFF00;
            TempA0 = TempA0 & 0xFF00;

            TempResult4x8_Top.format<ushort, 4, 16>().select<1, 1, 8, 2>(0, 0) = (TempR0 >> 8) + (TempB0);
            TempResult4x8_Top.format<ushort, 4, 16>().select<1, 1, 8, 2>(0, 1) = (TempG0 >> 8) + (TempA0);

            TempB2 = TempB2 & 0xFF00;
            TempA2 = TempA2 & 0xFF00;

            TempResult4x8_Top.format<ushort, 4, 16>().select<1, 1, 8, 2>(1, 0) = (TempR2 >> 8) + (TempB2);
            TempResult4x8_Top.format<ushort, 4, 16>().select<1, 1, 8, 2>(1, 1) = (TempG2 >> 8) + (TempA2);

            TempB4 = TempB4 & 0xFF00;
            TempA4 = TempA4 & 0xFF00;

            TempResult4x8_Top.format<ushort, 4, 16>().select<1, 1, 8, 2>(2, 0) = (TempR4 >> 8) + (TempB4);
            TempResult4x8_Top.format<ushort, 4, 16>().select<1, 1, 8, 2>(2, 1) = (TempG4 >> 8) + (TempA4);

            TempB6 = TempB6 & 0xFF00;
            TempA6 = TempA6 & 0xFF00;

            TempResult4x8_Top.format<ushort, 4, 16>().select<1, 1, 8, 2>(3, 0) = (TempR6 >> 8) + (TempB6);
            TempResult4x8_Top.format<ushort, 4, 16>().select<1, 1, 8, 2>(3, 1) = (TempG6 >> 8) + (TempA6);

            // R/G/B channel 2nd half
            matrix_ref<ushort, 1, 8> TempR8 = DataBuffer.select<1, 1, 8, 1>(8 * i + 6, 0);
            matrix_ref<ushort, 1, 8> TempR10 = DataBuffer.select<1, 1, 8, 1>(8 * i + 6, 8);
            matrix_ref<ushort, 1, 8> TempR12 = DataBuffer.select<1, 1, 8, 1>(8 * i + 6, 16);
            matrix_ref<ushort, 1, 8> TempR14 = DataBuffer.select<1, 1, 8, 1>(8 * i + 6, 24);

            matrix_ref<ushort, 1, 8> TempG8 = DataBuffer.select<1, 1, 8, 1>(8 * i + 6, 32);
            matrix_ref<ushort, 1, 8> TempG10 = DataBuffer.select<1, 1, 8, 1>(8 * i + 6, 40);
            matrix_ref<ushort, 1, 8> TempG12 = DataBuffer.select<1, 1, 8, 1>(8 * i + 6, 48);
            matrix_ref<ushort, 1, 8> TempG14 = DataBuffer.select<1, 1, 8, 1>(8 * i + 6, 56);

            matrix_ref<ushort, 1, 8> TempB8 = DataBuffer.select<1, 1, 8, 1>(8 * i + 7, 0);
            matrix_ref<ushort, 1, 8> TempB10 = DataBuffer.select<1, 1, 8, 1>(8 * i + 7, 8);
            matrix_ref<ushort, 1, 8> TempB12 = DataBuffer.select<1, 1, 8, 1>(8 * i + 7, 16);
            matrix_ref<ushort, 1, 8> TempB14 = DataBuffer.select<1, 1, 8, 1>(8 * i + 7, 24);

            matrix_ref<ushort, 1, 8> TempA8 = DataBuffer.select<1, 1, 8, 1>(8 * i + 7, 32);
            matrix_ref<ushort, 1, 8> TempA10 = DataBuffer.select<1, 1, 8, 1>(8 * i + 7, 40);
            matrix_ref<ushort, 1, 8> TempA12 = DataBuffer.select<1, 1, 8, 1>(8 * i + 7, 48);
            matrix_ref<ushort, 1, 8> TempA14 = DataBuffer.select<1, 1, 8, 1>(8 * i + 7, 56);

            TempB8 = TempB8 & 0xFF00;
            TempA8 = TempA8 & 0xFF00;

            TempResult4x8_Bottom.format<ushort, 4, 16>().select<1, 1, 8, 2>(0, 0) = (TempR8 >> 8) + (TempB8);
            TempResult4x8_Bottom.format<ushort, 4, 16>().select<1, 1, 8, 2>(0, 1) = (TempG8 >> 8) + (TempA8);

            TempB10 = TempB10 & 0xFF00;
            TempA10 = TempA10 & 0xFF00;

            TempResult4x8_Bottom.format<ushort, 4, 16>().select<1, 1, 8, 2>(1, 0) = (TempR10 >> 8) + (TempB10);
            TempResult4x8_Bottom.format<ushort, 4, 16>().select<1, 1, 8, 2>(1, 1) = (TempG10 >> 8) + (TempA10);

            TempB12 = TempB12 & 0xFF00;
            TempA12 = TempA12 & 0xFF00;

            TempResult4x8_Bottom.format<ushort, 4, 16>().select<1, 1, 8, 2>(2, 0) = (TempR12 >> 8) + (TempB12);
            TempResult4x8_Bottom.format<ushort, 4, 16>().select<1, 1, 8, 2>(2, 1) = (TempG12 >> 8) + (TempA12);

            TempB14 = TempB14 & 0xFF00;
            TempA14 = TempA14 & 0xFF00;

            TempResult4x8_Bottom.format<ushort, 4, 16>().select<1, 1, 8, 2>(3, 0) = (TempR14 >> 8) + (TempB14);
            TempResult4x8_Bottom.format<ushort, 4, 16>().select<1, 1, 8, 2>(3, 1) = (TempG14 >> 8) + (TempA14);

            write(Dst_Surface, DstX * 4 + 32, DstY, Result);
        }
    }
}