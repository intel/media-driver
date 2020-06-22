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
    matrix_ref<ushort, 4, 16> DitherTempRB = DataBuffer4.format<ushort, 16, 16>().select<4, 1, 16, 1>(0, 0);
    matrix_ref<ushort, 4, 16> DitherTempG  = DataBuffer4.format<ushort, 16, 16>().select<4, 1, 16, 1>(4, 0);

    //dithering algorithm
    //const uint8_t gDitherMatrix_3Bit_4X4[4][4] = {
    //    {  0,  4,  1,  5 },
    //    {  6,  2,  7,  3 },
    //    {  1,  5,  0,  4 },
    //    {  7,  3,  6,  2 }
    //};
    //R2 = (R1 + gDitherMatrix_3Bit_4X4[(x+0) & 3][(y+0) & 3]  )
    //G2 = (G1 + gDitherMatrix_3Bit_4X4[(x+1) & 3][(y+1) & 3]/2)
    //B2 = (B1 + gDitherMatrix_3Bit_4X4[(x+2) & 3][(y+2) & 3]  )

    // -----  Dithering Buffer0  ------
#define TempR_Top DataBuffer0.format<ushort, 16, 16>().select<2, 1, 16, 1>(0, 0)
#define TempG_Top DataBuffer0.format<ushort, 16, 16>().select<2, 1, 16, 1>(2, 0)
#define TempB_Top DataBuffer0.format<ushort, 16, 16>().select<2, 1, 16, 1>(4, 0)
#define TempR_Bot DataBuffer0.format<ushort, 16, 16>().select<2, 1, 16, 1>(8, 0)
#define TempG_Bot DataBuffer0.format<ushort, 16, 16>().select<2, 1, 16, 1>(10, 0)
#define TempB_Bot DataBuffer0.format<ushort, 16, 16>().select<2, 1, 16, 1>(12, 0)

    // R
    // mov(8) uwBUFFER_5(0, 0)<1>  0x51405140 : v //first 16pixel row
    // mov(8) uwBUFFER_5(0, 8)<1>  0x37263726 : v //second 16pixel row
    // mov(8) uwBUFFER_5(1, 0)<1>  0x40514051 : v //third 16pixel row
    // mov(8) uwBUFFER_5(1, 8)<1>  0x26372637 : v //fourth 16pixel row

    {
        vector<ushort, 8> TempDither(Dither_RB0);
        DitherTempRB.select<1, 1, 8, 1>(0, 0) = TempDither;
    }

    {
        vector<ushort, 8> TempDither(Dither_RB1);
        DitherTempRB.select<1, 1, 8, 1>(0, 8) = TempDither;
    }

    {
        vector<ushort, 8> TempDither(Dither_RB2);
        DitherTempRB.select<1, 1, 8, 1>(1, 0) = TempDither;
    }

    {
        vector<ushort, 8> TempDither(Dither_RB3);
        DitherTempRB.select<1, 1, 8, 1>(1, 8) = TempDither;
    }

    DitherTempRB.select<2, 1, 16, 1>(2, 0) = DitherTempRB.select<2, 1, 16, 1>(0, 0);

    DitherTempRB = DitherTempRB << 8;
    TempR_Top = cm_add<ushort>(TempR_Top, DitherTempRB.select<2, 1, 16, 1>(0, 0), SAT);
    TempR_Bot = cm_add<ushort>(TempR_Bot, DitherTempRB.select<2, 1, 16, 1>(2, 0), SAT);

    // G
    // mov(8) uwBUFFER_5(4, 0)<1>  0x63726372 : v //first 16pixel row
    // mov(8) uwBUFFER_5(4, 8)<1>  0x14051405 : v //second 16pixel row
    // mov(8) uwBUFFER_5(5, 0)<1>  0x72637263 : v //third 16pixel row
    // mov(8) uwBUFFER_5(5, 8)<1>  0x05140514 : v //fourth 16pixel row

    {
        vector<ushort, 8> TempDither(Dither_G0);
        DitherTempG.select<1, 1, 8, 1>(0, 0) = TempDither;
    }

    {
        vector<ushort, 8> TempDither(Dither_G1);
        DitherTempG.select<1, 1, 8, 1>(0, 8) = TempDither;
    }

    {
        vector<ushort, 8> TempDither(Dither_G2);
        DitherTempG.select<1, 1, 8, 1>(1, 0) = TempDither;
    }

    {
        vector<ushort, 8> TempDither(Dither_G3);
        DitherTempG.select<1, 1, 8, 1>(1, 8) = TempDither;
    }

    DitherTempG.select<2, 1, 16, 1>(2, 0) = DitherTempG.select<2, 1, 16, 1>(0, 0);

    DitherTempG = DitherTempG << 7;
    TempG_Top = cm_add<ushort>(TempG_Top, DitherTempG.select<2, 1, 16, 1>(0, 0), SAT);
    TempG_Bot = cm_add<ushort>(TempG_Bot, DitherTempG.select<2, 1, 16, 1>(2, 0), SAT);

    // B
    TempB_Top = cm_add<ushort>(TempB_Top, DitherTempRB.select<2, 1, 16, 1>(0, 0), SAT);
    TempB_Bot = cm_add<ushort>(TempB_Bot, DitherTempRB.select<2, 1, 16, 1>(2, 0), SAT);

#undef TempR_Top
#undef TempG_Top
#undef TempB_Top
#undef TempR_Bot
#undef TempG_Bot
#undef TempB_Bot

    // -----  Dithering Buffer1  ------
#define TempR_Top DataBuffer1.format<ushort, 16, 16>().select<2, 1, 16, 1>(0, 0)
#define TempG_Top DataBuffer1.format<ushort, 16, 16>().select<2, 1, 16, 1>(2, 0)
#define TempB_Top DataBuffer1.format<ushort, 16, 16>().select<2, 1, 16, 1>(4, 0)
#define TempR_Bot DataBuffer1.format<ushort, 16, 16>().select<2, 1, 16, 1>(8, 0)
#define TempG_Bot DataBuffer1.format<ushort, 16, 16>().select<2, 1, 16, 1>(10, 0)
#define TempB_Bot DataBuffer1.format<ushort, 16, 16>().select<2, 1, 16, 1>(12, 0)

    // R
    TempR_Top = cm_add<ushort>(TempR_Top, DitherTempRB.select<2, 1, 16, 1>(0, 0), SAT);
    TempR_Bot = cm_add<ushort>(TempR_Bot, DitherTempRB.select<2, 1, 16, 1>(2, 0), SAT);

    // G
    TempG_Top = cm_add<ushort>(TempG_Top, DitherTempG.select<2, 1, 16, 1>(0, 0), SAT);
    TempG_Bot = cm_add<ushort>(TempG_Bot, DitherTempG.select<2, 1, 16, 1>(2, 0), SAT);

    // B
    TempB_Top = cm_add<ushort>(TempB_Top, DitherTempRB.select<2, 1, 16, 1>(0, 0), SAT);
    TempB_Bot = cm_add<ushort>(TempB_Bot, DitherTempRB.select<2, 1, 16, 1>(2, 0), SAT);

#undef TempR_Top
#undef TempG_Top
#undef TempB_Top
#undef TempR_Bot
#undef TempG_Bot
#undef TempB_Bot

    // -----  Dithering Buffer2  ------
#define TempR_Top DataBuffer2.format<ushort, 16, 16>().select<2, 1, 16, 1>(0, 0)
#define TempG_Top DataBuffer2.format<ushort, 16, 16>().select<2, 1, 16, 1>(2, 0)
#define TempB_Top DataBuffer2.format<ushort, 16, 16>().select<2, 1, 16, 1>(4, 0)
#define TempR_Bot DataBuffer2.format<ushort, 16, 16>().select<2, 1, 16, 1>(8, 0)
#define TempG_Bot DataBuffer2.format<ushort, 16, 16>().select<2, 1, 16, 1>(10, 0)
#define TempB_Bot DataBuffer2.format<ushort, 16, 16>().select<2, 1, 16, 1>(12, 0)

    // R
    TempR_Top = cm_add<ushort>(TempR_Top, DitherTempRB.select<2, 1, 16, 1>(0, 0), SAT);
    TempR_Bot = cm_add<ushort>(TempR_Bot, DitherTempRB.select<2, 1, 16, 1>(2, 0), SAT);

    // G
    TempG_Top = cm_add<ushort>(TempG_Top, DitherTempG.select<2, 1, 16, 1>(0, 0), SAT);
    TempG_Bot = cm_add<ushort>(TempG_Bot, DitherTempG.select<2, 1, 16, 1>(2, 0), SAT);

    // B
    TempB_Top = cm_add<ushort>(TempB_Top, DitherTempRB.select<2, 1, 16, 1>(0, 0), SAT);
    TempB_Bot = cm_add<ushort>(TempB_Bot, DitherTempRB.select<2, 1, 16, 1>(2, 0), SAT);

#undef TempR_Top
#undef TempG_Top
#undef TempB_Top
#undef TempR_Bot
#undef TempG_Bot
#undef TempB_Bot

    // -----  Dithering Buffer3  ------
#define TempR_Top DataBuffer3.format<ushort, 16, 16>().select<2, 1, 16, 1>(0, 0)
#define TempG_Top DataBuffer3.format<ushort, 16, 16>().select<2, 1, 16, 1>(2, 0)
#define TempB_Top DataBuffer3.format<ushort, 16, 16>().select<2, 1, 16, 1>(4, 0)
#define TempR_Bot DataBuffer3.format<ushort, 16, 16>().select<2, 1, 16, 1>(8, 0)
#define TempG_Bot DataBuffer3.format<ushort, 16, 16>().select<2, 1, 16, 1>(10, 0)
#define TempB_Bot DataBuffer3.format<ushort, 16, 16>().select<2, 1, 16, 1>(12, 0)

    // R
    TempR_Top = cm_add<ushort>(TempR_Top, DitherTempRB.select<2, 1, 16, 1>(0, 0), SAT);
    TempR_Bot = cm_add<ushort>(TempR_Bot, DitherTempRB.select<2, 1, 16, 1>(2, 0), SAT);

    // G
    TempG_Top = cm_add<ushort>(TempG_Top, DitherTempG.select<2, 1, 16, 1>(0, 0), SAT);
    TempG_Bot = cm_add<ushort>(TempG_Bot, DitherTempG.select<2, 1, 16, 1>(2, 0), SAT);

    // B
    TempB_Top = cm_add<ushort>(TempB_Top, DitherTempRB.select<2, 1, 16, 1>(0, 0), SAT);
    TempB_Bot = cm_add<ushort>(TempB_Bot, DitherTempRB.select<2, 1, 16, 1>(2, 0), SAT);

#undef TempR_Top
#undef TempG_Top
#undef TempB_Top
#undef TempR_Bot
#undef TempG_Bot
#undef TempB_Bot

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