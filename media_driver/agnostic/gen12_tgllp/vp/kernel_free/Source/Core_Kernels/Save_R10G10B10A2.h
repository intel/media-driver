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

    matrix_ref<uint, 9, 8> Msg = DataBuffer.format<uint, 96, 8>().select<9, 1, 8, 1>(64, 0);
    matrix_ref<uint, 8, 8> Result = Msg.select<8, 1, 8, 1>(1, 0);
    uint descriptor;

    Msg.select<1, 1, 8, 1>(0, 0) = cm_get_r0<uint>();
    Msg.select<1, 1, 1, 1>(0, 2) = nBLOCK_WIDTH_32 + nBLOCK_HEIGHT_8;
    descriptor = MDF_FC_OUTPUT_BTI_START + nDPMW_MSGDSC + nMSGLEN_8;
    uchar ChannelSwap = (WAFlag >> 16) & 0x01;

    matrix_ref<uint, 4, 8> TempResult4x8_Top = Result.select<4, 1, 8, 1>(0, 0);
    matrix_ref<uint, 4, 8> TempResult4x8_Bottom = Result.select<4, 1, 8, 1>(4, 0);
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

            // Rounding
            TempR0 = cm_add<ushort>(TempR0, 0x20, SAT);
            TempR0 = TempR0 & 0xFFC0;
            TempR2 = cm_add<ushort>(TempR2, 0x20, SAT);
            TempR2 = TempR2 & 0xFFC0;
            TempR4 = cm_add<ushort>(TempR4, 0x20, SAT);
            TempR4 = TempR4 & 0xFFC0;
            TempR6 = cm_add<ushort>(TempR6, 0x20, SAT);
            TempR6 = TempR6 & 0xFFC0;

            TempG0 = cm_add<ushort>(TempG0, 0x20, SAT);
            TempG0 = TempG0 & 0xFFC0;
            TempG2 = cm_add<ushort>(TempG2, 0x20, SAT);
            TempG2 = TempG2 & 0xFFC0;
            TempG4 = cm_add<ushort>(TempG4, 0x20, SAT);
            TempG4 = TempG4 & 0xFFC0;
            TempG6 = cm_add<ushort>(TempG6, 0x20, SAT);
            TempG6 = TempG6 & 0xFFC0;

            TempB0 = cm_add<ushort>(TempB0, 0x20, SAT);
            TempB0 = TempB0 & 0xFFC0;
            TempB2 = cm_add<ushort>(TempB2, 0x20, SAT);
            TempB2 = TempB2 & 0xFFC0;
            TempB4 = cm_add<ushort>(TempB4, 0x20, SAT);
            TempB4 = TempB4 & 0xFFC0;
            TempB6 = cm_add<ushort>(TempB6, 0x20, SAT);
            TempB6 = TempB6 & 0xFFC0;

            TempA0 = cm_add<ushort>(TempA0, 0x2000, SAT);
            TempA0 = TempA0 & 0xC000;
            TempA2 = cm_add<ushort>(TempA2, 0x2000, SAT);
            TempA2 = TempA2 & 0xC000;
            TempA4 = cm_add<ushort>(TempA4, 0x2000, SAT);
            TempA4 = TempA4 & 0xC000;
            TempA6 = cm_add<ushort>(TempA6, 0x2000, SAT);
            TempA6 = TempA6 & 0xC000;

            if (ChannelSwap)
            {
                TempResult4x8_Top.select<1, 1, 8, 1>(0, 0) = TempB0 >> 6;
                TempResult4x8_Top.select<1, 1, 8, 1>(0, 0) = TempResult4x8_Top.select<1, 1, 8, 1>(0, 0) + TempG0 * 0x10;
                TempResult4x8_Top.select<1, 1, 8, 1>(0, 0) = TempResult4x8_Top.select<1, 1, 8, 1>(0, 0) + TempR0 * 0x4000;
                TempResult4x8_Top.select<1, 1, 8, 1>(0, 0) = TempResult4x8_Top.select<1, 1, 8, 1>(0, 0) + TempA0 * 0x10000;

                TempResult4x8_Top.select<1, 1, 8, 1>(1, 0) = TempB2 >> 6;
                TempResult4x8_Top.select<1, 1, 8, 1>(1, 0) = TempResult4x8_Top.select<1, 1, 8, 1>(1, 0) + TempG2 * 0x10;
                TempResult4x8_Top.select<1, 1, 8, 1>(1, 0) = TempResult4x8_Top.select<1, 1, 8, 1>(1, 0) + TempR2 * 0x4000;
                TempResult4x8_Top.select<1, 1, 8, 1>(1, 0) = TempResult4x8_Top.select<1, 1, 8, 1>(1, 0) + TempA2 * 0x10000;

                TempResult4x8_Top.select<1, 1, 8, 1>(2, 0) = TempB4 >> 6;
                TempResult4x8_Top.select<1, 1, 8, 1>(2, 0) = TempResult4x8_Top.select<1, 1, 8, 1>(2, 0) + TempG4 * 0x10;
                TempResult4x8_Top.select<1, 1, 8, 1>(2, 0) = TempResult4x8_Top.select<1, 1, 8, 1>(2, 0) + TempR4 * 0x4000;
                TempResult4x8_Top.select<1, 1, 8, 1>(2, 0) = TempResult4x8_Top.select<1, 1, 8, 1>(2, 0) + TempA4 * 0x10000;

                TempResult4x8_Top.select<1, 1, 8, 1>(3, 0) = TempB6 >> 6;
                TempResult4x8_Top.select<1, 1, 8, 1>(3, 0) = TempResult4x8_Top.select<1, 1, 8, 1>(3, 0) + TempG6 * 0x10;
                TempResult4x8_Top.select<1, 1, 8, 1>(3, 0) = TempResult4x8_Top.select<1, 1, 8, 1>(3, 0) + TempR6 * 0x4000;
                TempResult4x8_Top.select<1, 1, 8, 1>(3, 0) = TempResult4x8_Top.select<1, 1, 8, 1>(3, 0) + TempA6 * 0x10000;
            }
            else
            {
                TempResult4x8_Top.select<1, 1, 8, 1>(0, 0) = TempR0 >> 6;
                TempResult4x8_Top.select<1, 1, 8, 1>(0, 0) = TempResult4x8_Top.select<1, 1, 8, 1>(0, 0) + TempG0 * 0x10;
                TempResult4x8_Top.select<1, 1, 8, 1>(0, 0) = TempResult4x8_Top.select<1, 1, 8, 1>(0, 0) + TempB0 * 0x4000;
                TempResult4x8_Top.select<1, 1, 8, 1>(0, 0) = TempResult4x8_Top.select<1, 1, 8, 1>(0, 0) + TempA0 * 0x10000;

                TempResult4x8_Top.select<1, 1, 8, 1>(1, 0) = TempR2 >> 6;
                TempResult4x8_Top.select<1, 1, 8, 1>(1, 0) = TempResult4x8_Top.select<1, 1, 8, 1>(1, 0) + TempG2 * 0x10;
                TempResult4x8_Top.select<1, 1, 8, 1>(1, 0) = TempResult4x8_Top.select<1, 1, 8, 1>(1, 0) + TempB2 * 0x4000;
                TempResult4x8_Top.select<1, 1, 8, 1>(1, 0) = TempResult4x8_Top.select<1, 1, 8, 1>(1, 0) + TempA2 * 0x10000;

                TempResult4x8_Top.select<1, 1, 8, 1>(2, 0) = TempR4 >> 6;
                TempResult4x8_Top.select<1, 1, 8, 1>(2, 0) = TempResult4x8_Top.select<1, 1, 8, 1>(2, 0) + TempG4 * 0x10;
                TempResult4x8_Top.select<1, 1, 8, 1>(2, 0) = TempResult4x8_Top.select<1, 1, 8, 1>(2, 0) + TempB4 * 0x4000;
                TempResult4x8_Top.select<1, 1, 8, 1>(2, 0) = TempResult4x8_Top.select<1, 1, 8, 1>(2, 0) + TempA4 * 0x10000;

                TempResult4x8_Top.select<1, 1, 8, 1>(3, 0) = TempR6 >> 6;
                TempResult4x8_Top.select<1, 1, 8, 1>(3, 0) = TempResult4x8_Top.select<1, 1, 8, 1>(3, 0) + TempG6 * 0x10;
                TempResult4x8_Top.select<1, 1, 8, 1>(3, 0) = TempResult4x8_Top.select<1, 1, 8, 1>(3, 0) + TempB6 * 0x4000;
                TempResult4x8_Top.select<1, 1, 8, 1>(3, 0) = TempResult4x8_Top.select<1, 1, 8, 1>(3, 0) + TempA6 * 0x10000;
            }

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

            // Rounding
            TempR8 = cm_add<ushort>(TempR8, 0x20, SAT);
            TempR8 = TempR8 & 0xFFC0;
            TempR10 = cm_add<ushort>(TempR10, 0x20, SAT);
            TempR10 = TempR10 & 0xFFC0;
            TempR12 = cm_add<ushort>(TempR12, 0x20, SAT);
            TempR12 = TempR12 & 0xFFC0;
            TempR14 = cm_add<ushort>(TempR14, 0x20, SAT);
            TempR14 = TempR14 & 0xFFC0;

            TempG8 = cm_add<ushort>(TempG8, 0x20, SAT);
            TempG8 = TempG8 & 0xFFC0;
            TempG10 = cm_add<ushort>(TempG10, 0x20, SAT);
            TempG10 = TempG10 & 0xFFC0;
            TempG12 = cm_add<ushort>(TempG12, 0x20, SAT);
            TempG12 = TempG12 & 0xFFC0;
            TempG14 = cm_add<ushort>(TempG14, 0x20, SAT);
            TempG14 = TempG14 & 0xFFC0;

            TempB8 = cm_add<ushort>(TempB8, 0x20, SAT);
            TempB8 = TempB8 & 0xFFC0;
            TempB10 = cm_add<ushort>(TempB10, 0x20, SAT);
            TempB10 = TempB10 & 0xFFC0;
            TempB12 = cm_add<ushort>(TempB12, 0x20, SAT);
            TempB12 = TempB12 & 0xFFC0;
            TempB14 = cm_add<ushort>(TempB14, 0x20, SAT);
            TempB14 = TempB14 & 0xFFC0;

            TempA8 = cm_add<ushort>(TempA8, 0x2000, SAT);
            TempA8 = TempA8 & 0xC000;
            TempA10 = cm_add<ushort>(TempA10, 0x2000, SAT);
            TempA10 = TempA10 & 0xC000;
            TempA12 = cm_add<ushort>(TempA12, 0x2000, SAT);
            TempA12 = TempA12 & 0xC000;
            TempA14 = cm_add<ushort>(TempA14, 0x2000, SAT);
            TempA14 = TempA14 & 0xC000;

            if (ChannelSwap)
            {
                TempResult4x8_Bottom.select<1, 1, 8, 1>(0, 0) = TempB8 >> 6;
                TempResult4x8_Bottom.select<1, 1, 8, 1>(0, 0) = TempResult4x8_Bottom.select<1, 1, 8, 1>(0, 0) + TempG8 * 0x10;
                TempResult4x8_Bottom.select<1, 1, 8, 1>(0, 0) = TempResult4x8_Bottom.select<1, 1, 8, 1>(0, 0) + TempR8 * 0x4000;
                TempResult4x8_Bottom.select<1, 1, 8, 1>(0, 0) = TempResult4x8_Bottom.select<1, 1, 8, 1>(0, 0) + TempA8 * 0x10000;

                TempResult4x8_Bottom.select<1, 1, 8, 1>(1, 0) = TempB10 >> 6;
                TempResult4x8_Bottom.select<1, 1, 8, 1>(1, 0) = TempResult4x8_Bottom.select<1, 1, 8, 1>(1, 0) + TempG10 * 0x10;
                TempResult4x8_Bottom.select<1, 1, 8, 1>(1, 0) = TempResult4x8_Bottom.select<1, 1, 8, 1>(1, 0) + TempR10 * 0x4000;
                TempResult4x8_Bottom.select<1, 1, 8, 1>(1, 0) = TempResult4x8_Bottom.select<1, 1, 8, 1>(1, 0) + TempA10 * 0x10000;

                TempResult4x8_Bottom.select<1, 1, 8, 1>(2, 0) = TempB12 >> 6;
                TempResult4x8_Bottom.select<1, 1, 8, 1>(2, 0) = TempResult4x8_Bottom.select<1, 1, 8, 1>(2, 0) + TempG12 * 0x10;
                TempResult4x8_Bottom.select<1, 1, 8, 1>(2, 0) = TempResult4x8_Bottom.select<1, 1, 8, 1>(2, 0) + TempR12 * 0x4000;
                TempResult4x8_Bottom.select<1, 1, 8, 1>(2, 0) = TempResult4x8_Bottom.select<1, 1, 8, 1>(2, 0) + TempA12 * 0x10000;

                TempResult4x8_Bottom.select<1, 1, 8, 1>(3, 0) = TempB14 >> 6;
                TempResult4x8_Bottom.select<1, 1, 8, 1>(3, 0) = TempResult4x8_Bottom.select<1, 1, 8, 1>(3, 0) + TempG14 * 0x10;
                TempResult4x8_Bottom.select<1, 1, 8, 1>(3, 0) = TempResult4x8_Bottom.select<1, 1, 8, 1>(3, 0) + TempR14 * 0x4000;
                TempResult4x8_Bottom.select<1, 1, 8, 1>(3, 0) = TempResult4x8_Bottom.select<1, 1, 8, 1>(3, 0) + TempA14 * 0x10000;
            }
            else
            {
                TempResult4x8_Bottom.select<1, 1, 8, 1>(0, 0) = TempR8 >> 6;
                TempResult4x8_Bottom.select<1, 1, 8, 1>(0, 0) = TempResult4x8_Bottom.select<1, 1, 8, 1>(0, 0) + TempG8 * 0x10;
                TempResult4x8_Bottom.select<1, 1, 8, 1>(0, 0) = TempResult4x8_Bottom.select<1, 1, 8, 1>(0, 0) + TempB8 * 0x4000;
                TempResult4x8_Bottom.select<1, 1, 8, 1>(0, 0) = TempResult4x8_Bottom.select<1, 1, 8, 1>(0, 0) + TempA8 * 0x10000;

                TempResult4x8_Bottom.select<1, 1, 8, 1>(1, 0) = TempR10 >> 6;
                TempResult4x8_Bottom.select<1, 1, 8, 1>(1, 0) = TempResult4x8_Bottom.select<1, 1, 8, 1>(1, 0) + TempG10 * 0x10;
                TempResult4x8_Bottom.select<1, 1, 8, 1>(1, 0) = TempResult4x8_Bottom.select<1, 1, 8, 1>(1, 0) + TempB10 * 0x4000;
                TempResult4x8_Bottom.select<1, 1, 8, 1>(1, 0) = TempResult4x8_Bottom.select<1, 1, 8, 1>(1, 0) + TempA10 * 0x10000;

                TempResult4x8_Bottom.select<1, 1, 8, 1>(2, 0) = TempR12 >> 6;
                TempResult4x8_Bottom.select<1, 1, 8, 1>(2, 0) = TempResult4x8_Bottom.select<1, 1, 8, 1>(2, 0) + TempG12 * 0x10;
                TempResult4x8_Bottom.select<1, 1, 8, 1>(2, 0) = TempResult4x8_Bottom.select<1, 1, 8, 1>(2, 0) + TempB12 * 0x4000;
                TempResult4x8_Bottom.select<1, 1, 8, 1>(2, 0) = TempResult4x8_Bottom.select<1, 1, 8, 1>(2, 0) + TempA12 * 0x10000;

                TempResult4x8_Bottom.select<1, 1, 8, 1>(3, 0) = TempR14 >> 6;
                TempResult4x8_Bottom.select<1, 1, 8, 1>(3, 0) = TempResult4x8_Bottom.select<1, 1, 8, 1>(3, 0) + TempG14 * 0x10;
                TempResult4x8_Bottom.select<1, 1, 8, 1>(3, 0) = TempResult4x8_Bottom.select<1, 1, 8, 1>(3, 0) + TempB14 * 0x4000;
                TempResult4x8_Bottom.select<1, 1, 8, 1>(3, 0) = TempResult4x8_Bottom.select<1, 1, 8, 1>(3, 0) + TempA14 * 0x10000;
            }

            Msg.select<1, 1, 1, 1>(0, 0) = DstX * 4;
            Msg.select<1, 1, 1, 1>(0, 1) = DstY;

            cm_send(NULL,
                Msg,
                nDATAPORT_DC1,
                descriptor,
                0);
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

            // Rounding
            TempR0 = cm_add<ushort>(TempR0, 0x20, SAT);
            TempR0 = TempR0 & 0xFFC0;
            TempR2 = cm_add<ushort>(TempR2, 0x20, SAT);
            TempR2 = TempR2 & 0xFFC0;
            TempR4 = cm_add<ushort>(TempR4, 0x20, SAT);
            TempR4 = TempR4 & 0xFFC0;
            TempR6 = cm_add<ushort>(TempR6, 0x20, SAT);
            TempR6 = TempR6 & 0xFFC0;

            TempG0 = cm_add<ushort>(TempG0, 0x20, SAT);
            TempG0 = TempG0 & 0xFFC0;
            TempG2 = cm_add<ushort>(TempG2, 0x20, SAT);
            TempG2 = TempG2 & 0xFFC0;
            TempG4 = cm_add<ushort>(TempG4, 0x20, SAT);
            TempG4 = TempG4 & 0xFFC0;
            TempG6 = cm_add<ushort>(TempG6, 0x20, SAT);
            TempG6 = TempG6 & 0xFFC0;

            TempB0 = cm_add<ushort>(TempB0, 0x20, SAT);
            TempB0 = TempB0 & 0xFFC0;
            TempB2 = cm_add<ushort>(TempB2, 0x20, SAT);
            TempB2 = TempB2 & 0xFFC0;
            TempB4 = cm_add<ushort>(TempB4, 0x20, SAT);
            TempB4 = TempB4 & 0xFFC0;
            TempB6 = cm_add<ushort>(TempB6, 0x20, SAT);
            TempB6 = TempB6 & 0xFFC0;

            TempA0 = cm_add<ushort>(TempA0, 0x2000, SAT);
            TempA0 = TempA0 & 0xC000;
            TempA2 = cm_add<ushort>(TempA2, 0x2000, SAT);
            TempA2 = TempA2 & 0xC000;
            TempA4 = cm_add<ushort>(TempA4, 0x2000, SAT);
            TempA4 = TempA4 & 0xC000;
            TempA6 = cm_add<ushort>(TempA6, 0x2000, SAT);
            TempA6 = TempA6 & 0xC000;

            if (ChannelSwap)
            {
                TempResult4x8_Top.select<1, 1, 8, 1>(0, 0) = TempB0 >> 6;
                TempResult4x8_Top.select<1, 1, 8, 1>(0, 0) = TempResult4x8_Top.select<1, 1, 8, 1>(0, 0) + TempG0 * 0x10;
                TempResult4x8_Top.select<1, 1, 8, 1>(0, 0) = TempResult4x8_Top.select<1, 1, 8, 1>(0, 0) + TempR0 * 0x4000;
                TempResult4x8_Top.select<1, 1, 8, 1>(0, 0) = TempResult4x8_Top.select<1, 1, 8, 1>(0, 0) + TempA0 * 0x10000;

                TempResult4x8_Top.select<1, 1, 8, 1>(1, 0) = TempB2 >> 6;
                TempResult4x8_Top.select<1, 1, 8, 1>(1, 0) = TempResult4x8_Top.select<1, 1, 8, 1>(1, 0) + TempG2 * 0x10;
                TempResult4x8_Top.select<1, 1, 8, 1>(1, 0) = TempResult4x8_Top.select<1, 1, 8, 1>(1, 0) + TempR2 * 0x4000;
                TempResult4x8_Top.select<1, 1, 8, 1>(1, 0) = TempResult4x8_Top.select<1, 1, 8, 1>(1, 0) + TempA2 * 0x10000;

                TempResult4x8_Top.select<1, 1, 8, 1>(2, 0) = TempB4 >> 6;
                TempResult4x8_Top.select<1, 1, 8, 1>(2, 0) = TempResult4x8_Top.select<1, 1, 8, 1>(2, 0) + TempG4 * 0x10;
                TempResult4x8_Top.select<1, 1, 8, 1>(2, 0) = TempResult4x8_Top.select<1, 1, 8, 1>(2, 0) + TempR4 * 0x4000;
                TempResult4x8_Top.select<1, 1, 8, 1>(2, 0) = TempResult4x8_Top.select<1, 1, 8, 1>(2, 0) + TempA4 * 0x10000;

                TempResult4x8_Top.select<1, 1, 8, 1>(3, 0) = TempB6 >> 6;
                TempResult4x8_Top.select<1, 1, 8, 1>(3, 0) = TempResult4x8_Top.select<1, 1, 8, 1>(3, 0) + TempG6 * 0x10;
                TempResult4x8_Top.select<1, 1, 8, 1>(3, 0) = TempResult4x8_Top.select<1, 1, 8, 1>(3, 0) + TempR6 * 0x4000;
                TempResult4x8_Top.select<1, 1, 8, 1>(3, 0) = TempResult4x8_Top.select<1, 1, 8, 1>(3, 0) + TempA6 * 0x10000;
            }
            else
            {
                TempResult4x8_Top.select<1, 1, 8, 1>(0, 0) = TempR0 >> 6;
                TempResult4x8_Top.select<1, 1, 8, 1>(0, 0) = TempResult4x8_Top.select<1, 1, 8, 1>(0, 0) + TempG0 * 0x10;
                TempResult4x8_Top.select<1, 1, 8, 1>(0, 0) = TempResult4x8_Top.select<1, 1, 8, 1>(0, 0) + TempB0 * 0x4000;
                TempResult4x8_Top.select<1, 1, 8, 1>(0, 0) = TempResult4x8_Top.select<1, 1, 8, 1>(0, 0) + TempA0 * 0x10000;

                TempResult4x8_Top.select<1, 1, 8, 1>(1, 0) = TempR2 >> 6;
                TempResult4x8_Top.select<1, 1, 8, 1>(1, 0) = TempResult4x8_Top.select<1, 1, 8, 1>(1, 0) + TempG2 * 0x10;
                TempResult4x8_Top.select<1, 1, 8, 1>(1, 0) = TempResult4x8_Top.select<1, 1, 8, 1>(1, 0) + TempB2 * 0x4000;
                TempResult4x8_Top.select<1, 1, 8, 1>(1, 0) = TempResult4x8_Top.select<1, 1, 8, 1>(1, 0) + TempA2 * 0x10000;

                TempResult4x8_Top.select<1, 1, 8, 1>(2, 0) = TempR4 >> 6;
                TempResult4x8_Top.select<1, 1, 8, 1>(2, 0) = TempResult4x8_Top.select<1, 1, 8, 1>(2, 0) + TempG4 * 0x10;
                TempResult4x8_Top.select<1, 1, 8, 1>(2, 0) = TempResult4x8_Top.select<1, 1, 8, 1>(2, 0) + TempB4 * 0x4000;
                TempResult4x8_Top.select<1, 1, 8, 1>(2, 0) = TempResult4x8_Top.select<1, 1, 8, 1>(2, 0) + TempA4 * 0x10000;

                TempResult4x8_Top.select<1, 1, 8, 1>(3, 0) = TempR6 >> 6;
                TempResult4x8_Top.select<1, 1, 8, 1>(3, 0) = TempResult4x8_Top.select<1, 1, 8, 1>(3, 0) + TempG6 * 0x10;
                TempResult4x8_Top.select<1, 1, 8, 1>(3, 0) = TempResult4x8_Top.select<1, 1, 8, 1>(3, 0) + TempB6 * 0x4000;
                TempResult4x8_Top.select<1, 1, 8, 1>(3, 0) = TempResult4x8_Top.select<1, 1, 8, 1>(3, 0) + TempA6 * 0x10000;
            }

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

            // Rounding
            TempR8 = cm_add<ushort>(TempR8, 0x20, SAT);
            TempR8 = TempR8 & 0xFFC0;
            TempR10 = cm_add<ushort>(TempR10, 0x20, SAT);
            TempR10 = TempR10 & 0xFFC0;
            TempR12 = cm_add<ushort>(TempR12, 0x20, SAT);
            TempR12 = TempR12 & 0xFFC0;
            TempR14 = cm_add<ushort>(TempR14, 0x20, SAT);
            TempR14 = TempR14 & 0xFFC0;

            TempG8 = cm_add<ushort>(TempG8, 0x20, SAT);
            TempG8 = TempG8 & 0xFFC0;
            TempG10 = cm_add<ushort>(TempG10, 0x20, SAT);
            TempG10 = TempG10 & 0xFFC0;
            TempG12 = cm_add<ushort>(TempG12, 0x20, SAT);
            TempG12 = TempG12 & 0xFFC0;
            TempG14 = cm_add<ushort>(TempG14, 0x20, SAT);
            TempG14 = TempG14 & 0xFFC0;

            TempB8 = cm_add<ushort>(TempB8, 0x20, SAT);
            TempB8 = TempB8 & 0xFFC0;
            TempB10 = cm_add<ushort>(TempB10, 0x20, SAT);
            TempB10 = TempB10 & 0xFFC0;
            TempB12 = cm_add<ushort>(TempB12, 0x20, SAT);
            TempB12 = TempB12 & 0xFFC0;
            TempB14 = cm_add<ushort>(TempB14, 0x20, SAT);
            TempB14 = TempB14 & 0xFFC0;

            TempA8 = cm_add<ushort>(TempA8, 0x2000, SAT);
            TempA8 = TempA8 & 0xC000;
            TempA10 = cm_add<ushort>(TempA10, 0x2000, SAT);
            TempA10 = TempA10 & 0xC000;
            TempA12 = cm_add<ushort>(TempA12, 0x2000, SAT);
            TempA12 = TempA12 & 0xC000;
            TempA14 = cm_add<ushort>(TempA14, 0x2000, SAT);
            TempA14 = TempA14 & 0xC000;

            if (ChannelSwap)
            {
                TempResult4x8_Bottom.select<1, 1, 8, 1>(0, 0) = TempB8 >> 6;
                TempResult4x8_Bottom.select<1, 1, 8, 1>(0, 0) = TempResult4x8_Bottom.select<1, 1, 8, 1>(0, 0) + TempG8 * 0x10;
                TempResult4x8_Bottom.select<1, 1, 8, 1>(0, 0) = TempResult4x8_Bottom.select<1, 1, 8, 1>(0, 0) + TempR8 * 0x4000;
                TempResult4x8_Bottom.select<1, 1, 8, 1>(0, 0) = TempResult4x8_Bottom.select<1, 1, 8, 1>(0, 0) + TempA8 * 0x10000;

                TempResult4x8_Bottom.select<1, 1, 8, 1>(1, 0) = TempB10 >> 6;
                TempResult4x8_Bottom.select<1, 1, 8, 1>(1, 0) = TempResult4x8_Bottom.select<1, 1, 8, 1>(1, 0) + TempG10 * 0x10;
                TempResult4x8_Bottom.select<1, 1, 8, 1>(1, 0) = TempResult4x8_Bottom.select<1, 1, 8, 1>(1, 0) + TempR10 * 0x4000;
                TempResult4x8_Bottom.select<1, 1, 8, 1>(1, 0) = TempResult4x8_Bottom.select<1, 1, 8, 1>(1, 0) + TempA10 * 0x10000;

                TempResult4x8_Bottom.select<1, 1, 8, 1>(2, 0) = TempB12 >> 6;
                TempResult4x8_Bottom.select<1, 1, 8, 1>(2, 0) = TempResult4x8_Bottom.select<1, 1, 8, 1>(2, 0) + TempG12 * 0x10;
                TempResult4x8_Bottom.select<1, 1, 8, 1>(2, 0) = TempResult4x8_Bottom.select<1, 1, 8, 1>(2, 0) + TempR12 * 0x4000;
                TempResult4x8_Bottom.select<1, 1, 8, 1>(2, 0) = TempResult4x8_Bottom.select<1, 1, 8, 1>(2, 0) + TempA12 * 0x10000;

                TempResult4x8_Bottom.select<1, 1, 8, 1>(3, 0) = TempB14 >> 6;
                TempResult4x8_Bottom.select<1, 1, 8, 1>(3, 0) = TempResult4x8_Bottom.select<1, 1, 8, 1>(3, 0) + TempG14 * 0x10;
                TempResult4x8_Bottom.select<1, 1, 8, 1>(3, 0) = TempResult4x8_Bottom.select<1, 1, 8, 1>(3, 0) + TempR14 * 0x4000;
                TempResult4x8_Bottom.select<1, 1, 8, 1>(3, 0) = TempResult4x8_Bottom.select<1, 1, 8, 1>(3, 0) + TempA14 * 0x10000;
            }
            else
            {
                TempResult4x8_Bottom.select<1, 1, 8, 1>(0, 0) = TempR8 >> 6;
                TempResult4x8_Bottom.select<1, 1, 8, 1>(0, 0) = TempResult4x8_Bottom.select<1, 1, 8, 1>(0, 0) + TempG8 * 0x10;
                TempResult4x8_Bottom.select<1, 1, 8, 1>(0, 0) = TempResult4x8_Bottom.select<1, 1, 8, 1>(0, 0) + TempB8 * 0x4000;
                TempResult4x8_Bottom.select<1, 1, 8, 1>(0, 0) = TempResult4x8_Bottom.select<1, 1, 8, 1>(0, 0) + TempA8 * 0x10000;

                TempResult4x8_Bottom.select<1, 1, 8, 1>(1, 0) = TempR10 >> 6;
                TempResult4x8_Bottom.select<1, 1, 8, 1>(1, 0) = TempResult4x8_Bottom.select<1, 1, 8, 1>(1, 0) + TempG10 * 0x10;
                TempResult4x8_Bottom.select<1, 1, 8, 1>(1, 0) = TempResult4x8_Bottom.select<1, 1, 8, 1>(1, 0) + TempB10 * 0x4000;
                TempResult4x8_Bottom.select<1, 1, 8, 1>(1, 0) = TempResult4x8_Bottom.select<1, 1, 8, 1>(1, 0) + TempA10 * 0x10000;

                TempResult4x8_Bottom.select<1, 1, 8, 1>(2, 0) = TempR12 >> 6;
                TempResult4x8_Bottom.select<1, 1, 8, 1>(2, 0) = TempResult4x8_Bottom.select<1, 1, 8, 1>(2, 0) + TempG12 * 0x10;
                TempResult4x8_Bottom.select<1, 1, 8, 1>(2, 0) = TempResult4x8_Bottom.select<1, 1, 8, 1>(2, 0) + TempB12 * 0x4000;
                TempResult4x8_Bottom.select<1, 1, 8, 1>(2, 0) = TempResult4x8_Bottom.select<1, 1, 8, 1>(2, 0) + TempA12 * 0x10000;

                TempResult4x8_Bottom.select<1, 1, 8, 1>(3, 0) = TempR14 >> 6;
                TempResult4x8_Bottom.select<1, 1, 8, 1>(3, 0) = TempResult4x8_Bottom.select<1, 1, 8, 1>(3, 0) + TempG14 * 0x10;
                TempResult4x8_Bottom.select<1, 1, 8, 1>(3, 0) = TempResult4x8_Bottom.select<1, 1, 8, 1>(3, 0) + TempB14 * 0x4000;
                TempResult4x8_Bottom.select<1, 1, 8, 1>(3, 0) = TempResult4x8_Bottom.select<1, 1, 8, 1>(3, 0) + TempA14 * 0x10000;
            }

            Msg.select<1, 1, 1, 1>(0, 0) = DstX * 4 + 32;
            Msg.select<1, 1, 1, 1>(0, 1) = DstY;

            cm_send(NULL,
                Msg,
                nDATAPORT_DC1,
                descriptor,
                0);
        }
    }
    //}
}