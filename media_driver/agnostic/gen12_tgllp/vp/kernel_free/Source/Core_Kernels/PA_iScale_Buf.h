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
#if (defined BUFFER_0) || (defined BUFFER_1) || (defined BUFFER_2) || (defined BUFFER_3)
vector <ushort, 16> LoadMaskTemp = (TempMask0.select<1, 1, 16, 1>(0, 0) != 0);
#elif (defined BUFFER_4) || (defined BUFFER_5)
uchar sec_half_shift = (Layer_Index >> 7) * 8;
vector <ushort, 16> LoadMaskTemp = (TempMask.select<16, 1>(0) != 0);
#endif

#if defined BUFFER_0
Buffer_Index = 0;
#elif defined BUFFER_1
Buffer_Index = 1;
#elif defined BUFFER_2
Buffer_Index = 2;
#elif defined BUFFER_3
Buffer_Index = 3;
#elif defined BUFFER_4
Buffer_Index = 4;
#elif defined BUFFER_5
Buffer_Index = 5;
#endif

ushort LoadMask = cm_pack_mask(LoadMaskTemp);
CalculationMask = LoadMask == 0 ? 0x00 : 0xFF;

if (CalculationMask != 0)
{
    float StartX;
    float StartY;
    float StartX1;
    float StartY1;
    float DeltaX;
    float DeltaY;
    uchar RotationFlag;

    /*
    AVS Sampler 16x4 write back buffer layout for R/V, G/Y, B/U channel, each box stands for 8x1 ushort write back pixel
    16x4 pixle deviding to 8 8x1 pixel block
    _________________________________________________
    |_______Block0__________|_______Block1__________|
    |_______Block2__________|_______Block3__________|
    |_______Block4__________|_______Block5__________|
    |_______Block6__________|_______Block7__________|

    Write back buffer layout correlate to the block number#, each box stands for 1 GRF
    _______________________________________________
    |____R0_________R2_____|____R4_________R6_____|
    |____G0_________G2_____|____G4_________G6_____|
    |____B0_________B2_____|____B4_________B6_____|
    |____A0_________A2_____|____A4_________A6_____|
    |____R1_________R3_____|____R5_________R7_____|
    |____G1_________G3_____|____G5_________G7_____|
    |____B1_________B3_____|____B5_________B7_____|
    |____A1_________A3_____|____A5_________A7_____|
    */
    matrix <float, 2, 8> mesg;
    mesg.select<1, 1, 8, 1>(0, 0).format<uint>() = cm_get_r0<uint>();
    mesg.select<1, 1, 8, 1>(1, 0).format<uint>() = cm_get_r0<uint>();
    uint desc_y;
    uchar Layer_Index_45;

#ifdef BUFFER_0
#ifdef ROTATE_90
    StartX = Start_X(0);
    StartY = Start_Y(0);
    DeltaX = Delta_X(0);
    DeltaY = Delta_Y(0);

    StartX = StartX + DstY * DeltaX;
    StartY = StartY + (Dst_Width - MDF_FC_BLOCK_WIDTH - DstX) * DeltaY;

    StartY1 = StartY;
    StartX1 = StartX + 4 * DeltaX;
#elif defined ROTATE_180
    StartX = Start_X(0);
    StartY = Start_Y(0);
    DeltaX = Delta_X(0);
    DeltaY = Delta_Y(0);

    StartX += (Dst_Width - MDF_FC_BLOCK_WIDTH - DstX + MDF_FC_SAMPLER_UNORM_WIDTH) * DeltaX;
    StartX1 = StartX - DeltaX * MDF_FC_SAMPLER_UNORM_WIDTH;

    StartY = StartY + (Dst_Height - MDF_FC_BLOCK_HEIGHT - DstY + 8) * DeltaY;
    StartY1 = StartY;
#elif defined ROTATE_270
    StartX = Start_X(0);
    StartY = Start_Y(0);
    DeltaX = Delta_X(0);
    DeltaY = Delta_Y(0);

    StartX = StartX + (Dst_Height - MDF_FC_BLOCK_HEIGHT - DstY + 12) * DeltaX;
    StartY = StartY + DstX * DeltaY;

    StartY1 = StartY;
    StartX1 = StartX - 4 * DeltaX;
#else
    StartX = Start_X(0);
    StartY = Start_Y(0);
    DeltaX = Delta_X(0);
    DeltaY = Delta_Y(0);

    RotationFlag = (uchar)(RotationChromaSitingFlag & 0x07);
    if (RotationFlag == MDF_FC_MIRROR_HORIZONTAL)
    {
        StartX += (Dst_Width - MDF_FC_BLOCK_WIDTH - DstX) * DeltaX;
    }
    else
    {
        StartX += DstX * DeltaX;
    }

    StartX1 = StartX + MDF_FC_SAMPLER_UNORM_WIDTH * DeltaX;

    StartY += DstY * DeltaY;
    StartY1 = StartY;
#endif
#endif

#ifdef BUFFER_1
#ifdef ROTATE_90
    StartX = Start_X(0);
    StartY = Start_Y(0);
    DeltaX = Delta_X(0);
    DeltaY = Delta_Y(0);

    StartX = StartX + DstY * DeltaX;
    StartY = StartY + (Dst_Width - MDF_FC_BLOCK_WIDTH - DstX) * DeltaY;

    StartY1 = StartY;
    StartX1 = StartX + 4 * DeltaX;

#elif defined ROTATE_180
    StartX = Start_X(0);
    StartY = Start_Y(0);
    DeltaX = Delta_X(0);
    DeltaY = Delta_Y(0);

    StartX += (Dst_Width - MDF_FC_BLOCK_WIDTH - DstX + MDF_FC_SAMPLER_UNORM_WIDTH) * DeltaX;
    StartX1 = StartX - DeltaX * MDF_FC_SAMPLER_UNORM_WIDTH;

    StartY = StartY + (Dst_Height - MDF_FC_BLOCK_HEIGHT - DstY + 8) * DeltaY;
    StartY1 = StartY;
#elif defined ROTATE_270
    StartX = Start_X(0);
    StartY = Start_Y(0);
    DeltaX = Delta_X(0);
    DeltaY = Delta_Y(0);

    StartX = StartX + (Dst_Height - MDF_FC_BLOCK_HEIGHT - DstY + 12) * DeltaX;
    StartY = StartY + DstX * DeltaY;

    StartY1 = StartY;
    StartX1 = StartX - 4 * DeltaX;
#else
    StartX = Start_X(0);
    StartY = Start_Y(0);
    DeltaX = Delta_X(0);
    DeltaY = Delta_Y(0);
    RotationFlag = (uchar)(RotationChromaSitingFlag & 0x07);
    if (RotationFlag == MDF_FC_MIRROR_HORIZONTAL)
    {
        StartX += (Dst_Width - MDF_FC_BLOCK_WIDTH - DstX) * DeltaX;
    }
    else
    {
        StartX += DstX * DeltaX;
    }
    //StartX += DstX * DeltaX;
    StartY += DstY * DeltaY;

    StartX1 = StartX + DeltaX * MDF_FC_SAMPLER_UNORM_WIDTH;
    StartY1 = StartY;
#endif
#endif

#ifdef BUFFER_2
#ifdef ROTATE_90
    StartX = Start_X(0);
    StartY = Start_Y(0);
    DeltaX = Delta_X(0);
    DeltaY = Delta_Y(0);

    StartX = StartX + (8 + DstY) * DeltaX;
    StartY = StartY + (Dst_Width - MDF_FC_BLOCK_WIDTH - DstX) * DeltaY;

    StartY1 = StartY;
    StartX1 = StartX + 4 * DeltaX;

#elif defined ROTATE_180
    StartX = Start_X(0);
    StartY = Start_Y(0);
    DeltaX = Delta_X(0);
    DeltaY = Delta_Y(0);

    StartX += (Dst_Width - MDF_FC_BLOCK_WIDTH - DstX + 8) * DeltaX;
    StartX1 = StartX - DeltaX * MDF_FC_SAMPLER_UNORM_WIDTH;

    StartY = StartY + (Dst_Height - MDF_FC_BLOCK_HEIGHT - DstY) * DeltaY;
    StartY1 = StartY;
#elif defined ROTATE_270
    StartX = Start_X(0);
    StartY = Start_Y(0);
    DeltaX = Delta_X(0);
    DeltaY = Delta_Y(0);

    StartX = StartX + (Dst_Height - MDF_FC_BLOCK_HEIGHT - DstY + 4) * DeltaX;
    StartY = StartY + DstX * DeltaY;

    StartY1 = StartY;
    StartX1 = StartX - 4 * DeltaX;
#else
    StartX = Start_X(0);
    StartY = Start_Y(0);
    DeltaX = Delta_X(0);
    DeltaY = Delta_Y(0);
    RotationFlag = (uchar)(RotationChromaSitingFlag & 0x07);
    if (RotationFlag == MDF_FC_MIRROR_HORIZONTAL)
    {
        StartX += (Dst_Width - MDF_FC_BLOCK_WIDTH - DstX) * DeltaX;
    }
    else
    {
        StartX += DstX * DeltaX;
    }
    //StartX += DstX * DeltaX;
    StartY += (DstY + 2 * MDF_FC_SAMPLER_UNORM_HEIGHT)* DeltaY;
    StartX1 = StartX + DeltaX * MDF_FC_SAMPLER_UNORM_WIDTH;
    StartY1 = StartY;
#endif
#endif

#ifdef BUFFER_3
#ifdef ROTATE_90
    StartX = Start_X(0);
    StartY = Start_Y(0);
    DeltaX = Delta_X(0);
    DeltaY = Delta_Y(0);

    StartX = StartX + (8 + DstY) * DeltaX;
    StartY = StartY + (Dst_Width - MDF_FC_BLOCK_WIDTH - DstX) * DeltaY;

    StartY1 = StartY;
    StartX1 = StartX + 4 * DeltaX;

#elif defined ROTATE_180
    StartX = Start_X(0);
    StartY = Start_Y(0);
    DeltaX = Delta_X(0);
    DeltaY = Delta_Y(0);

    StartX += (Dst_Width - MDF_FC_BLOCK_WIDTH - DstX + 8) * DeltaX;
    StartX1 = StartX - DeltaX * MDF_FC_SAMPLER_UNORM_WIDTH;

    StartY = StartY + (Dst_Height - MDF_FC_BLOCK_HEIGHT - DstY) * DeltaY;
    StartY1 = StartY;
#elif defined ROTATE_270
    StartX = Start_X(0);
    StartY = Start_Y(0);
    DeltaX = Delta_X(0);
    DeltaY = Delta_Y(0);

    StartX = StartX + (Dst_Height - MDF_FC_BLOCK_HEIGHT - DstY + 4) * DeltaX;
    StartY = StartY + DstX * DeltaY;

    StartY1 = StartY;
    StartX1 = StartX - 4 * DeltaX;
#else
    StartX = Start_X(0);
    StartY = Start_Y(0);
    DeltaX = Delta_X(0);
    DeltaY = Delta_Y(0);

    RotationFlag = (uchar)(RotationChromaSitingFlag & 0x07);

    if (RotationFlag == MDF_FC_MIRROR_HORIZONTAL)
    {
        StartX += (Dst_Width - MDF_FC_BLOCK_WIDTH - DstX) * DeltaX;
    }
    else
    {
        StartX += DstX * DeltaX;
    }
    //StartX += DstX * DeltaX;
    StartY += (DstY + 2 * MDF_FC_SAMPLER_UNORM_HEIGHT)* DeltaY;
    StartX1 = StartX + DeltaX * MDF_FC_SAMPLER_UNORM_WIDTH;
    StartY1 = StartY;
#endif
#endif

#ifdef BUFFER_0
#define WriteBackBuffer DataBuffer0
    mesg.select<1, 1, 1, 1>(1, 2) = StartX;
    mesg.select<1, 1, 1, 1>(1, 3) = StartY;
    mesg.select<1, 1, 1, 1>(1, 4) = DeltaX;
    mesg.select<1, 1, 1, 1>(1, 5) = 2 * DeltaY;

#if (defined ROTATE_180) || (defined ROTATE_270)
    desc_y = nSMPL_UNORM_444_16BITS_MSGDSC_4CH + (MDF_FC_3D_SAMPLER_SI_Y << 8) + MDF_FC_INPUT_BTI_F2;
#else
    desc_y = nSMPL_UNORM_444_16BITS_MSGDSC_4CH + (MDF_FC_3D_SAMPLER_SI_Y << 8) + MDF_FC_INPUT_BTI_START;
#endif
    cm_send(WriteBackBuffer.format<ushort, 16, 16>().select<8, 1, 16, 1>(0, 0),
        mesg.format<ushort, 2, 16>(),
        nSMPL_ENGINE,
        desc_y,
        0);

    mesg.select<1, 1, 1, 1>(1, 2) = StartX1;
    mesg.select<1, 1, 1, 1>(1, 3) = StartY1;
    mesg.select<1, 1, 1, 1>(1, 4) = DeltaX;
    mesg.select<1, 1, 1, 1>(1, 5) = 2 * DeltaY;

    cm_send(WriteBackBuffer.format<ushort, 16, 16>().select<8, 1, 16, 1>(8, 0),
        mesg.format<ushort, 2, 16>(),
        nSMPL_ENGINE,
        desc_y,
        0);

#endif
#ifdef BUFFER_1
#define WriteBackBuffer DataBuffer1
    mesg.select<1, 1, 1, 1>(1, 2) = StartX;
    mesg.select<1, 1, 1, 1>(1, 3) = StartY;
    mesg.select<1, 1, 1, 1>(1, 4) = DeltaX;
    mesg.select<1, 1, 1, 1>(1, 5) = 2 * DeltaY;

#if (defined ROTATE_180) || (defined ROTATE_270)
    desc_y = nSMPL_UNORM_444_16BITS_MSGDSC_4CH + (MDF_FC_3D_SAMPLER_SI_Y << 8) + MDF_FC_INPUT_BTI_START;
#else
    desc_y = nSMPL_UNORM_444_16BITS_MSGDSC_4CH + (MDF_FC_3D_SAMPLER_SI_Y << 8) + MDF_FC_INPUT_BTI_F2;
#endif
    cm_send(WriteBackBuffer.format<ushort, 16, 16>().select<8, 1, 16, 1>(0, 0),
        mesg.format<ushort, 2, 16>(),
        nSMPL_ENGINE,
        desc_y,
        0);

    mesg.select<1, 1, 1, 1>(1, 2) = StartX1;
    mesg.select<1, 1, 1, 1>(1, 3) = StartY1;
    mesg.select<1, 1, 1, 1>(1, 4) = DeltaX;
    mesg.select<1, 1, 1, 1>(1, 5) = 2 * DeltaY;
    cm_send(WriteBackBuffer.format<ushort, 16, 16>().select<8, 1, 16, 1>(8, 0),
        mesg.format<ushort, 2, 16>(),
        nSMPL_ENGINE,
        desc_y,
        0);

#endif
#ifdef BUFFER_2
#define WriteBackBuffer DataBuffer2
    mesg.select<1, 1, 1, 1>(1, 2) = StartX;
    mesg.select<1, 1, 1, 1>(1, 3) = StartY;
    mesg.select<1, 1, 1, 1>(1, 4) = DeltaX;
    mesg.select<1, 1, 1, 1>(1, 5) = 2 * DeltaY;

#if (defined ROTATE_180) || (defined ROTATE_270)
    desc_y = nSMPL_UNORM_444_16BITS_MSGDSC_4CH + (MDF_FC_3D_SAMPLER_SI_Y << 8) + MDF_FC_INPUT_BTI_F2;
#else
    desc_y = nSMPL_UNORM_444_16BITS_MSGDSC_4CH + (MDF_FC_3D_SAMPLER_SI_Y << 8) + MDF_FC_INPUT_BTI_START;
#endif
    cm_send(WriteBackBuffer.format<ushort, 16, 16>().select<8, 1, 16, 1>(0, 0),
        mesg.format<ushort, 2, 16>(),
        nSMPL_ENGINE,
        desc_y,
        0);

    mesg.select<1, 1, 1, 1>(1, 2) = StartX1;
    mesg.select<1, 1, 1, 1>(1, 3) = StartY1;
    mesg.select<1, 1, 1, 1>(1, 4) = DeltaX;
    mesg.select<1, 1, 1, 1>(1, 5) = 2 * DeltaY;
    cm_send(WriteBackBuffer.format<ushort, 16, 16>().select<8, 1, 16, 1>(8, 0),
        mesg.format<ushort, 2, 16>(),
        nSMPL_ENGINE,
        desc_y,
        0);

#endif
#ifdef BUFFER_3
#define WriteBackBuffer DataBuffer3
    mesg.select<1, 1, 1, 1>(1, 2) = StartX;
    mesg.select<1, 1, 1, 1>(1, 3) = StartY;
    mesg.select<1, 1, 1, 1>(1, 4) = DeltaX;
    mesg.select<1, 1, 1, 1>(1, 5) = 2 * DeltaY;

#if (defined ROTATE_180) || (defined ROTATE_270)
    desc_y = nSMPL_UNORM_444_16BITS_MSGDSC_4CH + (MDF_FC_3D_SAMPLER_SI_Y << 8) + MDF_FC_INPUT_BTI_START;
#else
    desc_y = nSMPL_UNORM_444_16BITS_MSGDSC_4CH + (MDF_FC_3D_SAMPLER_SI_Y << 8) + MDF_FC_INPUT_BTI_F2;
#endif
    cm_send(WriteBackBuffer.format<ushort, 16, 16>().select<8, 1, 16, 1>(0, 0),
        mesg.format<ushort, 2, 16>(),
        nSMPL_ENGINE,
        desc_y,
        0);

    mesg.select<1, 1, 1, 1>(1, 2) = StartX1;
    mesg.select<1, 1, 1, 1>(1, 3) = StartY1;
    mesg.select<1, 1, 1, 1>(1, 4) = DeltaX;
    mesg.select<1, 1, 1, 1>(1, 5) = 2 * DeltaY;
    cm_send(WriteBackBuffer.format<ushort, 16, 16>().select<8, 1, 16, 1>(8, 0),
        mesg.format<ushort, 2, 16>(),
        nSMPL_ENGINE,
        desc_y,
        0);

#endif

    // Shuffle the write back of sampler
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
#undef WriteBackBuffer
}