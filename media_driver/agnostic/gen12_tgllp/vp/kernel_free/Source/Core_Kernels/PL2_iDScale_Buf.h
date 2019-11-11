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
    float DeltaX;
    float DeltaY;
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

#if (defined BUFFER_0) || (defined BUFFER_1)
#define INIT_INCREMENT 0.0f
#elif (defined BUFFER_2) || (defined BUFFER_3)
#define INIT_INCREMENT 4.0f
#endif

#if (defined BUFFER_0) || (defined BUFFER_1) || (defined BUFFER_2) || (defined BUFFER_3)
    StartX = Start_X(0);
    StartY = Start_Y(0);
    DeltaX = Delta_X(0);
    DeltaY = Delta_Y(0);

    StartX += DstX * DeltaX;
    StartY += DstY * DeltaY;

    vector <float, 16> IncrementX(Inc);
    vector <float, 16> IncrementY(INIT_INCREMENT);
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

    mesg.format<uint, 5, 8>().select<1, 1, 8, 1>(0, 0) = cm_get_r0<uint>();
    mesg.format<uint, 5, 8>().select<1, 1, 1, 1>(0, 2) = 0x0;

#pragma unroll
    for (short j = 0; j < 4; j++)
    {
        // Y channel
        mesg.format<uint, 5, 8>().select<1, 1, 1, 1>(0, 2) = nSMPL_RED_CHANNEL_444_16;

        mesg.select<1, 1, 8, 1>(1, 0) = StartX + IncrementX.select<8, 1>(0) * DeltaX;
        mesg.select<1, 1, 8, 1>(2, 0) = StartX + IncrementX.select<8, 1>(8) * DeltaX;

        mesg.select<1, 1, 8, 1>(3, 0) = StartY + IncrementY.select<8, 1>(0) * 2 * DeltaY;
        mesg.select<1, 1, 8, 1>(4, 0) = StartY + IncrementY.select<8, 1>(8) * 2 * DeltaY;

#if (defined BUFFER_0) || (defined BUFFER_2)
        desc_y = nSIMD16_0X_034X_MSG_DSC_1CH + (MDF_FC_3D_SAMPLER_SI_Y << 8) + MDF_FC_INPUT_BTI_START;
#elif (defined BUFFER_1) || (defined BUFFER_3)
        desc_y = nSIMD16_0X_034X_MSG_DSC_1CH + (MDF_FC_3D_SAMPLER_SI_Y << 8) + MDF_FC_INPUT_BTI_F2;
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

#if (defined BUFFER_0) || (defined BUFFER_2)
        desc_y = nSIMD16_0X_034X_MSG_DSC_2CH + (MDF_FC_3D_SAMPLER_SI_U << 8) + MDF_FC_INPUT_BTI_START + MDF_FC_UV_PLANE_BTI_OFFSET;
#elif (defined BUFFER_1) || (defined BUFFER_3)
        desc_y = nSIMD16_0X_034X_MSG_DSC_2CH + (MDF_FC_3D_SAMPLER_SI_U << 8) + MDF_FC_INPUT_BTI_F2 + MDF_FC_UV_PLANE_BTI_OFFSET;
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

        IncrementY = IncrementY + 1.0f;
    }

    WriteBackBuffer.format<ushort, 16, 16>().select<2, 1, 16, 1>(6, 0) = 0xffff;
    WriteBackBuffer.format<ushort, 16, 16>().select<2, 1, 16, 1>(14, 0) = 0xffff;

#undef WriteBackBuffer
}