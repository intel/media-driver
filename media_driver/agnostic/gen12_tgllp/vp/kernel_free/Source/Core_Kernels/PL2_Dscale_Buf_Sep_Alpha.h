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
CalculationMask = cm_pack_mask(LoadMaskTemp) & 0x000F;
#elif defined BUFFER_1
Buffer_Index = 1;
CalculationMask = (cm_pack_mask(LoadMaskTemp) >> 4) & 0x000F;
#elif defined BUFFER_2
Buffer_Index = 2;
CalculationMask = (cm_pack_mask(LoadMaskTemp) >> 8) & 0x000F;
#elif defined BUFFER_3
Buffer_Index = 3;
CalculationMask = (cm_pack_mask(LoadMaskTemp) >> 12) & 0x000F;
#elif defined BUFFER_4
CalculationMask = (cm_pack_mask(LoadMaskTemp) >> sec_half_shift) & 0x000F;
Buffer_Index = 4;
#elif defined BUFFER_5
CalculationMask = (cm_pack_mask(LoadMaskTemp) >> (4 + sec_half_shift)) & 0x000F;
Buffer_Index = 5;
#endif

if (CalculationMask != 0)
{
    float StartX;
    float StartY;
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
    matrix <float, 5, 8> mesg;
    matrix <float, 2, 16> temp_rg;

    mesg.select<1, 1, 8, 1>(0, 0).format<uint>() = cm_get_r0<uint>();
    uint desc_y;
    uchar Layer_Index_45;

#ifdef BUFFER_0
#define INIT_INCREMENT 0.0f
#elif defined BUFFER_1
#ifdef ROTATE_90
#define INIT_INCREMENT 4.0f
#elif defined ROTATE_180
#define INIT_INCREMENT -4.0f
#elif defined ROTATE_270
#define INIT_INCREMENT -4.0f
#else
#define INIT_INCREMENT 4.0f
#endif
#elif defined BUFFER_2
#ifdef ROTATE_90
#define INIT_INCREMENT 8.0f
#elif defined ROTATE_180
#define INIT_INCREMENT -8.0f
#elif defined ROTATE_270
#define INIT_INCREMENT -8.0f
#else
#define INIT_INCREMENT 8.0f
#endif
#elif defined BUFFER_3
#ifdef ROTATE_90
#define INIT_INCREMENT 12.0f
#elif defined ROTATE_180
#define INIT_INCREMENT -12.0f
#elif defined ROTATE_270
#define INIT_INCREMENT -12.0f
#else
#define INIT_INCREMENT 12.0f
#endif
#elif defined BUFFER_4
#define INIT_INCREMENT 0.0f
#elif defined BUFFER_5
#ifdef ROTATE_90
#define INIT_INCREMENT 4.0f
#elif defined ROTATE_180
#define INIT_INCREMENT -4.0f
#elif defined ROTATE_270
#define INIT_INCREMENT -4.0f
#else
#define INIT_INCREMENT 4.0f
#endif
#endif

#if (defined BUFFER_0) || (defined BUFFER_1) || (defined BUFFER_2) || (defined BUFFER_3)
#ifdef ROTATE_90
    StartX = Start_X(0);
    StartY = Start_Y(0);
    DeltaX = Delta_X(0);
    DeltaY = Delta_Y(0);
    RotationFlag = (uchar)(RotationChromaSitingFlag & 0x07);

    StartX = StartX + DstY * DeltaX;

    if (RotationFlag == MDF_FC_ROTATE_90_MIRROR_HORIZONTAL)
    {
        StartY = StartY + (DstX + MDF_FC_BLOCK_WIDTH) * DeltaY;
    }
    else
    {
        StartY = StartY + (Dst_Width - DstX) * DeltaY;
    }

    vector <float, 16> IncrementX(INIT_INCREMENT);
    vector <float, 16> IncrementY(Dec);
#elif defined ROTATE_180
    StartX = Start_X(0);
    StartY = Start_Y(0);
    DeltaX = Delta_X(0);
    DeltaY = Delta_Y(0);

    RotationFlag = (uchar)(RotationChromaSitingFlag & 0x07);
    if (RotationFlag == MDF_FC_MIRROR_VERTICAL)
    {
        StartX += (DstX + MDF_FC_BLOCK_WIDTH) * DeltaX;
    }
    else
    {
        StartX += (Dst_Width - DstX) * DeltaX;
    }

    StartY = StartY + ((Dst_Height - DstY - 1)) * DeltaY;

    vector <float, 16> IncrementX(Dec);
    vector <float, 16> IncrementY(INIT_INCREMENT);
#elif defined ROTATE_270
    StartX = Start_X(0);
    StartY = Start_Y(0);
    DeltaX = Delta_X(0);
    DeltaY = Delta_Y(0);
    RotationFlag = (uchar)(RotationChromaSitingFlag & 0x07);

    StartX = StartX + (Dst_Height - DstY - 1) * DeltaX;

    if (RotationFlag == MDF_FC_ROTATE_90_MIRROR_VERTICAL)
    {
        StartY = StartY + (Dst_Width - MDF_FC_BLOCK_WIDTH - DstX) * DeltaY;
    }
    else
    {
        StartY = StartY + DstX * DeltaY;
    }

    vector <float, 16> IncrementX(INIT_INCREMENT);
    vector <float, 16> IncrementY(Inc);
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

    StartY += DstY * DeltaY;

    vector <float, 16> IncrementX(Inc);
    vector <float, 16> IncrementY(INIT_INCREMENT);
#endif
#endif

#if (defined BUFFER_4) || (defined BUFFER_5)
#ifdef ROTATE_90
    Layer_Index_45 = Layer_Index & 0x7f;
    StartX = Start_X(Layer_Index_45);
    StartY = Start_Y(Layer_Index_45);
    DeltaX = Delta_X(Layer_Index_45);
    DeltaY = Delta_Y(Layer_Index_45);

    StartX = StartX + (DstY + 8 * (Layer_Index >> 7)) * DeltaX;
    RotationFlag = (uchar)((RotationChromaSitingFlag >> (3 * Layer_Index_45)) & 0x07);

    if (RotationFlag == MDF_FC_ROTATE_90_MIRROR_HORIZONTAL)
    {
        StartY = StartY + (DstX + MDF_FC_BLOCK_WIDTH) * DeltaY;
    }
    else
    {
        StartY = StartY + (Dst_Width - DstX) * DeltaY;
    }

    vector <float, 16> IncrementX(INIT_INCREMENT);
    vector <float, 16> IncrementY(Dec);
#elif defined ROTATE_180
    Layer_Index_45 = Layer_Index & 0x7f;
    StartX = Start_X(Layer_Index_45);
    StartY = Start_Y(Layer_Index_45);
    DeltaX = Delta_X(Layer_Index_45);
    DeltaY = Delta_Y(Layer_Index_45);

    RotationFlag = (uchar)((RotationChromaSitingFlag >> (3 * Layer_Index_45)) & 0x07);
    if (RotationFlag == MDF_FC_MIRROR_VERTICAL)
    {
        StartX += (DstX + MDF_FC_BLOCK_WIDTH) * DeltaX;
    }
    else
    {
        StartX += (Dst_Width - DstX) * DeltaX;
    }

    StartY = StartY + (Dst_Height - 1 - DstY - 8 * (Layer_Index >> 7)) * DeltaY;

    vector <float, 16> IncrementX(Dec);
    vector <float, 16> IncrementY(INIT_INCREMENT);
#elif defined ROTATE_270
    Layer_Index_45 = Layer_Index & 0x7f;

    StartX = Start_X(Layer_Index_45);
    StartY = Start_Y(Layer_Index_45);
    DeltaX = Delta_X(Layer_Index_45);
    DeltaY = Delta_Y(Layer_Index_45);

    RotationFlag = (uchar)((RotationChromaSitingFlag >> (3 * Layer_Index_45)) & 0x07);

    StartX = StartX + (Dst_Height - 1 - DstY - 8 * (Layer_Index >> 7)) * DeltaX;
    if (RotationFlag == MDF_FC_ROTATE_90_MIRROR_VERTICAL)
    {
        StartY = StartY + (Dst_Width - MDF_FC_BLOCK_WIDTH - DstX) * DeltaY;
    }
    else
    {
        StartY = StartY + DstX * DeltaY;
    }

    vector <float, 16> IncrementX(INIT_INCREMENT);
    vector <float, 16> IncrementY(Inc);
#else
    Layer_Index_45 = Layer_Index & 0x7f;
    StartX = Start_X(Layer_Index_45);
    StartY = Start_Y(Layer_Index_45);
    DeltaX = Delta_X(Layer_Index_45);
    DeltaY = Delta_Y(Layer_Index_45);
    RotationFlag = (uchar)((RotationChromaSitingFlag >> (3 * Layer_Index_45)) & 0x07);
    if (RotationFlag == MDF_FC_MIRROR_HORIZONTAL)
    {
        StartX += (Dst_Width - MDF_FC_BLOCK_WIDTH - DstX) * DeltaX;
    }
    else
    {
        StartX += DstX * DeltaX;
    }

    StartY = StartY + (DstY + (Layer_Index >> 7) * 8) * DeltaY;

    vector <float, 16> IncrementX(Inc);
    vector <float, 16> IncrementY(INIT_INCREMENT);
#endif
#endif

#ifdef BUFFER_0
#define WriteBackBuffer DataBuffer0
#endif
#ifdef BUFFER_1
#define WriteBackBuffer DataBuffer1
#endif
#ifdef BUFFER_2
#define WriteBackBuffer DataBuffer2
#endif
#ifdef BUFFER_3
#define WriteBackBuffer DataBuffer3
#endif
#ifdef BUFFER_4
#define WriteBackBuffer DataBuffer4
#endif
#ifdef BUFFER_5
#define WriteBackBuffer DataBuffer5
#endif

    mesg.format<uint, 5, 8>().select<1, 1, 8, 1>(0, 0) = cm_get_r0<uint>();

#pragma unroll
    for (short j = 0; j < 4; j++)
    {
        // Y channel
        mesg.format<uint, 5, 8>().select<1, 1, 1, 1>(0, 2) = nSMPL_RED_CHANNEL_444_16;

        mesg.select<1, 1, 8, 1>(1, 0) = StartX + IncrementX.select<8, 1>(0) * DeltaX;
        mesg.select<1, 1, 8, 1>(2, 0) = StartX + IncrementX.select<8, 1>(8) * DeltaX;

        mesg.select<1, 1, 8, 1>(3, 0) = StartY + IncrementY.select<8, 1>(0) * DeltaY;
        mesg.select<1, 1, 8, 1>(4, 0) = StartY + IncrementY.select<8, 1>(8) * DeltaY;

#if (defined BUFFER_0) || (defined BUFFER_1) || (defined BUFFER_2) || (defined BUFFER_3)
        desc_y = nSIMD16_0X_034X_MSG_DSC_1CH + (MDF_FC_3D_SAMPLER_SI_Y << 8) + MDF_FC_INPUT_BTI_START;
#endif
#if (defined BUFFER_4) || (defined BUFFER_5)
        desc_y = nSIMD16_0X_034X_MSG_DSC_1CH + (MDF_FC_3D_SAMPLER_SI_Y << 8) + MDF_FC_INPUT_BTI_START + MDF_FC_INPUT_BTI_PER_LAYER * Layer_Index_45;
#endif

        cm_send(
            temp_rg.select<1, 1, 16, 1>(0, 0),
            mesg.format<ushort, 5, 16>(),
            nSMPL_ENGINE,
            desc_y,
            0);

        vector<short, 4> row_offset(Row_Offset);
        vector<short, 4> row_offset1(Row_offset1);
        vector<short, 4> colonm_offset(Colomn_Offset);

        // Y
        WriteBackBuffer.format<ushort, 16, 16>().select<1, 1, 8, 1>(row_offset[j] + 2, Colomn_Offset[j]) = matrix<ushort, 1, 8>(temp_rg.select<1, 1, 8, 1>(0, 0) * MDF_FC_NORMALIZE_FACTOR, SAT);
        WriteBackBuffer.format<ushort, 16, 16>().select<1, 1, 8, 1>(row_offset1[j] + 2, Colomn_Offset[j]) = matrix<ushort, 1, 8>(temp_rg.select<1, 1, 8, 1>(0, 8) * MDF_FC_NORMALIZE_FACTOR, SAT);

        // UV channel
        mesg.format<uint, 5, 8>().select<1, 1, 1, 1>(0, 2) = nSMPL_RED_GREEN_CHANNEL_444_16;

#if (defined BUFFER_0) || (defined BUFFER_1) || (defined BUFFER_2) || (defined BUFFER_3)
        desc_y = nSIMD16_0X_034X_MSG_DSC_2CH + (MDF_FC_3D_SAMPLER_SI_U << 8) + MDF_FC_INPUT_BTI_START + MDF_FC_UV_PLANE_BTI_OFFSET;
#endif
#if (defined BUFFER_4) || (defined BUFFER_5)
        desc_y = nSIMD16_0X_034X_MSG_DSC_2CH + (MDF_FC_3D_SAMPLER_SI_U << 8) + MDF_FC_INPUT_BTI_START + MDF_FC_INPUT_BTI_PER_LAYER * Layer_Index_45 + MDF_FC_UV_PLANE_BTI_OFFSET;
#endif
        cm_send(temp_rg,
            mesg.format<ushort, 5, 16>(),
            nSMPL_ENGINE,
            desc_y,
            0);

        // R
        WriteBackBuffer.format<ushort, 16, 16>().select<1, 1, 8, 1>(row_offset[j], Colomn_Offset[j]) = matrix<ushort, 1, 8>(temp_rg.select<1, 1, 8, 1>(1, 0) * MDF_FC_NORMALIZE_FACTOR, SAT);
        WriteBackBuffer.format<ushort, 16, 16>().select<1, 1, 8, 1>(row_offset1[j], Colomn_Offset[j]) = matrix<ushort, 1, 8>(temp_rg.select<1, 1, 8, 1>(1, 8) * MDF_FC_NORMALIZE_FACTOR, SAT);
        // B
        WriteBackBuffer.format<ushort, 16, 16>().select<1, 1, 8, 1>(row_offset[j] + 4, Colomn_Offset[j]) = matrix<ushort, 1, 8>(temp_rg.select<1, 1, 8, 1>(0, 0) * MDF_FC_NORMALIZE_FACTOR, SAT);
        WriteBackBuffer.format<ushort, 16, 16>().select<1, 1, 8, 1>(row_offset1[j] + 4, Colomn_Offset[j]) = matrix<ushort, 1, 8>(temp_rg.select<1, 1, 8, 1>(0, 8) * MDF_FC_NORMALIZE_FACTOR, SAT);

#ifdef ROTATE_90
        IncrementX = IncrementX + 1.0f;
#elif defined ROTATE_180
        IncrementY = IncrementY - 1.0f;
#elif defined ROTATE_270
        IncrementX = IncrementX - 1.0f;
#else
        IncrementY = IncrementY + 1.0f;
#endif
    }

    WriteBackBuffer.format<ushort, 16, 16>().select<2, 1, 16, 1>(6, 0) = 0xffff;
    WriteBackBuffer.format<ushort, 16, 16>().select<2, 1, 16, 1>(14, 0) = 0xffff;
#undef WriteBackBuffer
}
