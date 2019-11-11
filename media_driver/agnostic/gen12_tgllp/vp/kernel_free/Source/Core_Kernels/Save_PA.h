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
    matrix_ref<uint, 4, 8> TempResult4x8_Top    = Result.select<4, 1, 8, 1>(0, 0);
    matrix_ref<uint, 4, 8> TempResult4x8_Bottom = Result.select<4, 1, 8, 1>(4, 0);
#pragma unroll
    for (uchar i = 0; i < 2; i++, DstY += 8)
    {
        // Rounding
        DataBuffer.format<ushort, 96, 16>().select<6, 1, 16, 1>(32 * i, 0) = cm_add<ushort>(DataBuffer.format<ushort, 96, 16>().select<6, 1, 16, 1>(32 * i, 0), 0x80, SAT);
        DataBuffer.format<ushort, 96, 16>().select<6, 1, 16, 1>(32 * i + 8, 0) = cm_add<ushort>(DataBuffer.format<ushort, 96, 16>().select<6, 1, 16, 1>(32 * i + 8, 0), 0x80, SAT);
        DataBuffer.format<ushort, 96, 16>().select<6, 1, 16, 1>(32 * i + 16, 0) = cm_add<ushort>(DataBuffer.format<ushort, 96, 16>().select<6, 1, 16, 1>(32 * i + 16, 0), 0x80, SAT);
        DataBuffer.format<ushort, 96, 16>().select<6, 1, 16, 1>(32 * i + 24, 0) = cm_add<ushort>(DataBuffer.format<ushort, 96, 16>().select<6, 1, 16, 1>(32 * i + 24, 0), 0x80, SAT);

        // First 8x16
        {
            // Y channel
            TempResult4x8_Top.format<uchar, 4, 32>().select<1, 1, 8, 2>(0, DestinationPackedYOffset) = DataBuffer.format<uchar, 96, 32>().select<1, 1, 8, 2>(2 + 32 * i, 1);
            TempResult4x8_Top.format<uchar, 4, 32>().select<1, 1, 8, 2>(0, 16 + DestinationPackedYOffset) = DataBuffer.format<uchar, 96, 32>().select<1, 1, 8, 2>(10 + 32 * i, 1);
            TempResult4x8_Top.format<uchar, 4, 32>().select<1, 1, 8, 2>(1, DestinationPackedYOffset) = DataBuffer.format<uchar, 96, 32>().select<1, 1, 8, 2>(2 + 32 * i, 17);
            TempResult4x8_Top.format<uchar, 4, 32>().select<1, 1, 8, 2>(1, 16 + DestinationPackedYOffset) = DataBuffer.format<uchar, 96, 32>().select<1, 1, 8, 2>(10 + 32 * i, 17);
            TempResult4x8_Top.format<uchar, 4, 32>().select<1, 1, 8, 2>(2, DestinationPackedYOffset) = DataBuffer.format<uchar, 96, 32>().select<1, 1, 8, 2>(3 + 32 * i, 1);
            TempResult4x8_Top.format<uchar, 4, 32>().select<1, 1, 8, 2>(2, 16 + DestinationPackedYOffset) = DataBuffer.format<uchar, 96, 32>().select<1, 1, 8, 2>(11 + 32 * i, 1);
            TempResult4x8_Top.format<uchar, 4, 32>().select<1, 1, 8, 2>(3, DestinationPackedYOffset) = DataBuffer.format<uchar, 96, 32>().select<1, 1, 8, 2>(3 + 32 * i, 17);
            TempResult4x8_Top.format<uchar, 4, 32>().select<1, 1, 8, 2>(3, 16 + DestinationPackedYOffset) = DataBuffer.format<uchar, 96, 32>().select<1, 1, 8, 2>(11 + 32 * i, 17);

            TempResult4x8_Bottom.format<uchar, 4, 32>().select<1, 1, 8, 2>(0, DestinationPackedYOffset) = DataBuffer.format<uchar, 96, 32>().select<1, 1, 8, 2>(18 + 32 * i, 1);
            TempResult4x8_Bottom.format<uchar, 4, 32>().select<1, 1, 8, 2>(0, 16 + DestinationPackedYOffset) = DataBuffer.format<uchar, 96, 32>().select<1, 1, 8, 2>(26 + 32 * i, 1);
            TempResult4x8_Bottom.format<uchar, 4, 32>().select<1, 1, 8, 2>(1, DestinationPackedYOffset) = DataBuffer.format<uchar, 96, 32>().select<1, 1, 8, 2>(18 + 32 * i, 17);
            TempResult4x8_Bottom.format<uchar, 4, 32>().select<1, 1, 8, 2>(1, 16 + DestinationPackedYOffset) = DataBuffer.format<uchar, 96, 32>().select<1, 1, 8, 2>(26 + 32 * i, 17);
            TempResult4x8_Bottom.format<uchar, 4, 32>().select<1, 1, 8, 2>(2, DestinationPackedYOffset) = DataBuffer.format<uchar, 96, 32>().select<1, 1, 8, 2>(19 + 32 * i, 1);
            TempResult4x8_Bottom.format<uchar, 4, 32>().select<1, 1, 8, 2>(2, 16 + DestinationPackedYOffset) = DataBuffer.format<uchar, 96, 32>().select<1, 1, 8, 2>(27 + 32 * i, 1);
            TempResult4x8_Bottom.format<uchar, 4, 32>().select<1, 1, 8, 2>(3, DestinationPackedYOffset) = DataBuffer.format<uchar, 96, 32>().select<1, 1, 8, 2>(19 + 32 * i, 17);
            TempResult4x8_Bottom.format<uchar, 4, 32>().select<1, 1, 8, 2>(3, 16 + DestinationPackedYOffset) = DataBuffer.format<uchar, 96, 32>().select<1, 1, 8, 2>(27 + 32 * i, 17);

            // UV channel
            // V channel
            TempResult4x8_Top.format<uchar, 4, 32>().select<1, 1, 4, 4>(0, DestinationPackedVOffset) = DataBuffer.format<uchar, 96, 32>().select<1, 1, 4, 4>(32 * i, 1);
            TempResult4x8_Top.format<uchar, 4, 32>().select<1, 1, 4, 4>(0, 16 + DestinationPackedVOffset) = DataBuffer.format<uchar, 96, 32>().select<1, 1, 4, 4>(32 * i + 8, 1);
            TempResult4x8_Top.format<uchar, 4, 32>().select<1, 1, 4, 4>(1, DestinationPackedVOffset) = DataBuffer.format<uchar, 96, 32>().select<1, 1, 4, 4>(32 * i, 17);
            TempResult4x8_Top.format<uchar, 4, 32>().select<1, 1, 4, 4>(1, 16 + DestinationPackedVOffset) = DataBuffer.format<uchar, 96, 32>().select<1, 1, 4, 4>(32 * i + 8, 17);
            TempResult4x8_Top.format<uchar, 4, 32>().select<1, 1, 4, 4>(2, DestinationPackedVOffset) = DataBuffer.format<uchar, 96, 32>().select<1, 1, 4, 4>(32 * i + 1, 1);
            TempResult4x8_Top.format<uchar, 4, 32>().select<1, 1, 4, 4>(2, 16 + DestinationPackedVOffset) = DataBuffer.format<uchar, 96, 32>().select<1, 1, 4, 4>(32 * i + 9, 1);
            TempResult4x8_Top.format<uchar, 4, 32>().select<1, 1, 4, 4>(3, DestinationPackedVOffset) = DataBuffer.format<uchar, 96, 32>().select<1, 1, 4, 4>(32 * i + 1, 17);
            TempResult4x8_Top.format<uchar, 4, 32>().select<1, 1, 4, 4>(3, 16 + DestinationPackedVOffset) = DataBuffer.format<uchar, 96, 32>().select<1, 1, 4, 4>(32 * i + 9, 17);

            TempResult4x8_Bottom.format<uchar, 4, 32>().select<1, 1, 4, 4>(0, DestinationPackedVOffset) = DataBuffer.format<uchar, 96, 32>().select<1, 1, 4, 4>(32 * i + 16, 1);
            TempResult4x8_Bottom.format<uchar, 4, 32>().select<1, 1, 4, 4>(0, 16 + DestinationPackedVOffset) = DataBuffer.format<uchar, 96, 32>().select<1, 1, 4, 4>(32 * i + 24, 1);
            TempResult4x8_Bottom.format<uchar, 4, 32>().select<1, 1, 4, 4>(1, DestinationPackedVOffset) = DataBuffer.format<uchar, 96, 32>().select<1, 1, 4, 4>(32 * i + 16, 17);
            TempResult4x8_Bottom.format<uchar, 4, 32>().select<1, 1, 4, 4>(1, 16 + DestinationPackedVOffset) = DataBuffer.format<uchar, 96, 32>().select<1, 1, 4, 4>(32 * i + 24, 17);
            TempResult4x8_Bottom.format<uchar, 4, 32>().select<1, 1, 4, 4>(2, DestinationPackedVOffset) = DataBuffer.format<uchar, 96, 32>().select<1, 1, 4, 4>(32 * i + 17, 1);
            TempResult4x8_Bottom.format<uchar, 4, 32>().select<1, 1, 4, 4>(2, 16 + DestinationPackedVOffset) = DataBuffer.format<uchar, 96, 32>().select<1, 1, 4, 4>(32 * i + 25, 1);
            TempResult4x8_Bottom.format<uchar, 4, 32>().select<1, 1, 4, 4>(3, DestinationPackedVOffset) = DataBuffer.format<uchar, 96, 32>().select<1, 1, 4, 4>(32 * i + 17, 17);
            TempResult4x8_Bottom.format<uchar, 4, 32>().select<1, 1, 4, 4>(3, 16 + DestinationPackedVOffset) = DataBuffer.format<uchar, 96, 32>().select<1, 1, 4, 4>(32 * i + 25, 17);

            // U channel
            TempResult4x8_Top.format<uchar, 4, 32>().select<1, 1, 4, 4>(0, DestinationPackedUOffset) = DataBuffer.format<uchar, 96, 32>().select<1, 1, 4, 4>(32 * i + 4, 1);
            TempResult4x8_Top.format<uchar, 4, 32>().select<1, 1, 4, 4>(0, 16 + DestinationPackedUOffset) = DataBuffer.format<uchar, 96, 32>().select<1, 1, 4, 4>(32 * i + 12, 1);
            TempResult4x8_Top.format<uchar, 4, 32>().select<1, 1, 4, 4>(1, DestinationPackedUOffset) = DataBuffer.format<uchar, 96, 32>().select<1, 1, 4, 4>(32 * i + 4, 17);
            TempResult4x8_Top.format<uchar, 4, 32>().select<1, 1, 4, 4>(1, 16 + DestinationPackedUOffset) = DataBuffer.format<uchar, 96, 32>().select<1, 1, 4, 4>(32 * i + 12, 17);
            TempResult4x8_Top.format<uchar, 4, 32>().select<1, 1, 4, 4>(2, DestinationPackedUOffset) = DataBuffer.format<uchar, 96, 32>().select<1, 1, 4, 4>(32 * i + 5, 1);
            TempResult4x8_Top.format<uchar, 4, 32>().select<1, 1, 4, 4>(2, 16 + DestinationPackedUOffset) = DataBuffer.format<uchar, 96, 32>().select<1, 1, 4, 4>(32 * i + 13, 1);
            TempResult4x8_Top.format<uchar, 4, 32>().select<1, 1, 4, 4>(3, DestinationPackedUOffset) = DataBuffer.format<uchar, 96, 32>().select<1, 1, 4, 4>(32 * i + 5, 17);
            TempResult4x8_Top.format<uchar, 4, 32>().select<1, 1, 4, 4>(3, 16 + DestinationPackedUOffset) = DataBuffer.format<uchar, 96, 32>().select<1, 1, 4, 4>(32 * i + 13, 17);

            TempResult4x8_Bottom.format<uchar, 4, 32>().select<1, 1, 4, 4>(0, DestinationPackedUOffset) = DataBuffer.format<uchar, 96, 32>().select<1, 1, 4, 4>(32 * i + 20, 1);
            TempResult4x8_Bottom.format<uchar, 4, 32>().select<1, 1, 4, 4>(0, 16 + DestinationPackedUOffset) = DataBuffer.format<uchar, 96, 32>().select<1, 1, 4, 4>(32 * i + 28, 1);
            TempResult4x8_Bottom.format<uchar, 4, 32>().select<1, 1, 4, 4>(1, DestinationPackedUOffset) = DataBuffer.format<uchar, 96, 32>().select<1, 1, 4, 4>(32 * i + 20, 17);
            TempResult4x8_Bottom.format<uchar, 4, 32>().select<1, 1, 4, 4>(1, 16 + DestinationPackedUOffset) = DataBuffer.format<uchar, 96, 32>().select<1, 1, 4, 4>(32 * i + 28, 17);
            TempResult4x8_Bottom.format<uchar, 4, 32>().select<1, 1, 4, 4>(2, DestinationPackedUOffset) = DataBuffer.format<uchar, 96, 32>().select<1, 1, 4, 4>(32 * i + 21, 1);
            TempResult4x8_Bottom.format<uchar, 4, 32>().select<1, 1, 4, 4>(2, 16 + DestinationPackedUOffset) = DataBuffer.format<uchar, 96, 32>().select<1, 1, 4, 4>(32 * i + 29, 1);
            TempResult4x8_Bottom.format<uchar, 4, 32>().select<1, 1, 4, 4>(3, DestinationPackedUOffset) = DataBuffer.format<uchar, 96, 32>().select<1, 1, 4, 4>(32 * i + 21, 17);
            TempResult4x8_Bottom.format<uchar, 4, 32>().select<1, 1, 4, 4>(3, 16 + DestinationPackedUOffset) = DataBuffer.format<uchar, 96, 32>().select<1, 1, 4, 4>(32 * i + 29, 17);

            write(Dst_Surface, DstX * 2, DstY, Result);
        }
    }
}