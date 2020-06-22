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
#ifndef _MDF_FC_COMMON_GENX_H_
#define _MDF_FC_COMMON_GENX_H_

#include <cm/cm.h>
#include <cm/cmtl.h>
#include "MDF_FC_common.h"

#define CURBE \
    PrimaryCscCoeff, \
    ConstantBlendingAlpha, \
    PointerToInlineParameters, \
    Dst_Width, \
    Dst_Height, \
    WAFlag, \
    RotationChromaSitingFlag, \
    Reserved1, \
    ColorFill, \
    LumakeyLowThreshold, \
    LumakeyHighThreshold, \
    Reserved2, \
    Reserved3, \
    DestinationPackedYOffset, \
    DestinationPackedUOffset, \
    DestinationPackedVOffset, \
    DestinationRGBFormat, \
    Delta_X, \
    Delta_Y, \
    Start_Y, \
    Start_X, \
    Top_Left, \
    Bottom_Right, \
    TempMask0

#define CURBE_OUTPUT \
    _CM_OUTPUT_ vector<short, 12> PrimaryCscCoeff, \
    _CM_OUTPUT_ vector<uchar, 7> ConstantBlendingAlpha, \
    _CM_OUTPUT_ uchar PointerToInlineParameters, \
    _CM_OUTPUT_ ushort Dst_Width, \
    _CM_OUTPUT_ ushort Dst_Height, \
    _CM_OUTPUT_ uint WAFlag, \
    _CM_OUTPUT_ uint RotationChromaSitingFlag, \
    _CM_OUTPUT_ vector<uint, 2> Reserved1, \
    _CM_OUTPUT_ vector<uchar, 4> ColorFill, \
    _CM_OUTPUT_ uchar LumakeyLowThreshold, \
    _CM_OUTPUT_ uchar LumakeyHighThreshold, \
    _CM_OUTPUT_ uchar Reserved2, \
    _CM_OUTPUT_ uchar Reserved3, \
    _CM_OUTPUT_ uchar DestinationPackedYOffset, \
    _CM_OUTPUT_ uchar DestinationPackedUOffset, \
    _CM_OUTPUT_ uchar DestinationPackedVOffset, \
    _CM_OUTPUT_ uchar DestinationRGBFormat, \
    _CM_OUTPUT_ vector<float, MDF_FC_MAX_INPUT_LAYER_NUM> Delta_X, \
    _CM_OUTPUT_ vector<float, MDF_FC_MAX_INPUT_LAYER_NUM> Delta_Y, \
    _CM_OUTPUT_ vector<float, MDF_FC_MAX_INPUT_LAYER_NUM> Start_Y, \
    _CM_OUTPUT_ vector<float, MDF_FC_MAX_INPUT_LAYER_NUM> Start_X, \
    _CM_OUTPUT_ vector<ushort, MDF_FC_MAX_INPUT_LAYER_NUM * 2> Top_Left, \
    _CM_OUTPUT_ vector<ushort, MDF_FC_MAX_INPUT_LAYER_NUM * 2> Bottom_Right, \
    _CM_OUTPUT_ matrix<ushort, 1, 16> TempMask0


#define CURBE_INPUT_OUTPUT \
    _CM_INPUT_OUTPUT_ vector<short, 12> PrimaryCscCoeff, \
    _CM_INPUT_OUTPUT_ vector<uchar, 7> ConstantBlendingAlpha, \
    _CM_INPUT_OUTPUT_ uchar PointerToInlineParameters, \
    _CM_INPUT_OUTPUT_ ushort Dst_Width, \
    _CM_INPUT_OUTPUT_ ushort Dst_Height, \
    _CM_INPUT_OUTPUT_ uint WAFlag, \
    _CM_INPUT_OUTPUT_ uint RotationChromaSitingFlag, \
    _CM_INPUT_OUTPUT_ vector<uint, 2> Reserved1, \
    _CM_INPUT_OUTPUT_ vector<uchar, 4> ColorFill, \
    _CM_INPUT_OUTPUT_ uchar LumakeyLowThreshold, \
    _CM_INPUT_OUTPUT_ uchar LumakeyHighThreshold, \
    _CM_INPUT_OUTPUT_ uchar Reserved2, \
    _CM_INPUT_OUTPUT_ uchar Reserved3, \
    _CM_INPUT_OUTPUT_ uchar DestinationPackedYOffset, \
    _CM_INPUT_OUTPUT_ uchar DestinationPackedUOffset, \
    _CM_INPUT_OUTPUT_ uchar DestinationPackedVOffset, \
    _CM_INPUT_OUTPUT_ uchar DestinationRGBFormat, \
    _CM_INPUT_OUTPUT_ vector<float, MDF_FC_MAX_INPUT_LAYER_NUM> Delta_X, \
    _CM_INPUT_OUTPUT_ vector<float, MDF_FC_MAX_INPUT_LAYER_NUM> Delta_Y, \
    _CM_INPUT_OUTPUT_ vector<float, MDF_FC_MAX_INPUT_LAYER_NUM> Start_Y, \
    _CM_INPUT_OUTPUT_ vector<float, MDF_FC_MAX_INPUT_LAYER_NUM> Start_X, \
    _CM_INPUT_OUTPUT_ vector<ushort, MDF_FC_MAX_INPUT_LAYER_NUM * 2> Top_Left, \
    _CM_INPUT_OUTPUT_ vector<ushort, MDF_FC_MAX_INPUT_LAYER_NUM * 2> Bottom_Right, \
    _CM_INPUT_OUTPUT_ matrix<ushort, 1, 16> TempMask0

#define CURBE_NONE_GLOBAL_TAG \
    vector<short, 12> PrimaryCscCoeff, \
    vector<uchar, 7> ConstantBlendingAlpha, \
    uchar PointerToInlineParameters, \
    ushort Dst_Width, \
    ushort Dst_Height, \
    uint WAFlag, \
    uint RotationChromaSitingFlag, \
    vector<uint, 2> Reserved1, \
    vector<uchar, 4> ColorFill, \
    uchar LumakeyLowThreshold, \
    uchar LumakeyHighThreshold, \
    uchar Reserved2, \
    uchar Reserved3, \
    uchar DestinationPackedYOffset, \
    uchar DestinationPackedUOffset, \
    uchar DestinationPackedVOffset, \
    uchar DestinationRGBFormat, \
    vector<float, MDF_FC_MAX_INPUT_LAYER_NUM> Delta_X, \
    vector<float, MDF_FC_MAX_INPUT_LAYER_NUM> Delta_Y, \
    vector<float, MDF_FC_MAX_INPUT_LAYER_NUM> Start_Y, \
    vector<float, MDF_FC_MAX_INPUT_LAYER_NUM> Start_X, \
    vector<ushort, MDF_FC_MAX_INPUT_LAYER_NUM * 2> Top_Left, \
    vector<ushort, MDF_FC_MAX_INPUT_LAYER_NUM * 2> Bottom_Right, \
    matrix<ushort, 1, 16> TempMask0

#define GLOBAL_BUFFER \
    DataBuffer, \
    TempMask, \
    CscCoeff, \
    DstX, \
    DstY, \
    Buffer_Index, \
    Layer_Index, \
    CalculationMask, \
    ConstAlphaTemp

#define GLOBAL_BUFFER_INPUT_OUTPUT \
    _CM_INPUT_OUTPUT_ matrix <ushort, 24, 64> DataBuffer, \
    _CM_INPUT_OUTPUT_ vector <ushort, 16> TempMask, \
    _CM_INPUT_OUTPUT_ matrix <short, 1, 12> CscCoeff, \
    _CM_INPUT_OUTPUT_ ushort DstX, \
    _CM_INPUT_OUTPUT_ ushort DstY, \
    _CM_INPUT_OUTPUT_ uchar  Buffer_Index, \
    _CM_INPUT_OUTPUT_ uchar  Layer_Index, \
    _CM_INPUT_OUTPUT_ uchar  CalculationMask, \
    _CM_INPUT_OUTPUT_ uchar  ConstAlphaTemp

#define GLOBAL_BUFFER_OUTPUT \
    _CM_OUTPUT_ matrix <ushort, 24, 64> DataBuffer, \
    _CM_OUTPUT_ vector <ushort, 16> TempMask, \
    _CM_OUTPUT_ matrix <short, 1, 12> CscCoeff, \
    _CM_OUTPUT_ ushort DstX, \
    _CM_OUTPUT_ ushort DstY, \
    _CM_OUTPUT_ uchar  Buffer_Index, \
    _CM_OUTPUT_ uchar  Layer_Index, \
    _CM_OUTPUT_ uchar  CalculationMask, \
    _CM_OUTPUT_ uchar  ConstAlphaTemp

#define GLOBAL_BUFFER_NONE_GLOBAL_TAG \
    matrix <ushort, 24, 64> DataBuffer, \
    vector <ushort, 16> TempMask, \
    matrix <short, 1, 12> CscCoeff, \
    ushort DstX, \
    ushort DstY, \
    uchar  Buffer_Index, \
    uchar  Layer_Index, \
    uchar  CalculationMask, \
    uchar  ConstAlphaTemp

#define DataBuffer0 (DataBuffer.select<4, 1, 64, 1>(0, 0))
#define DataBuffer1 (DataBuffer.select<4, 1, 64, 1>(4, 0))
#define DataBuffer2 (DataBuffer.select<4, 1, 64, 1>(8, 0))
#define DataBuffer3 (DataBuffer.select<4, 1, 64, 1>(12, 0))
#define DataBuffer4 (DataBuffer.select<4, 1, 64, 1>(16, 0))
#define DataBuffer5 (DataBuffer.select<4, 1, 64, 1>(20, 0))

#define GammaC_Lut DataBuffer4.format<uchar, 16, 32>().select<8, 1, 32, 1>(0, 0)
#define layer_num TempMask0.select<1, 1, 1, 1>(0, 0)
#define dst_info TempMask0.select<1, 1, 1, 1>(0, 1)

#define DataBuffer16x1_R0 (DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(0, 0))
#define DataBuffer16x1_R1 (DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(1, 0))
#define DataBuffer16x1_R2 (DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(8, 0))
#define DataBuffer16x1_R3 (DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(9, 0))
#define DataBuffer16x1_R4 (DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(16, 0))
#define DataBuffer16x1_R5 (DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(17, 0))
#define DataBuffer16x1_R6 (DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(24, 0))
#define DataBuffer16x1_R7 (DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(25, 0))
#define DataBuffer16x1_R8 (DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(32, 0))
#define DataBuffer16x1_R9 (DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(33, 0))
#define DataBuffer16x1_R10 (DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(40, 0))
#define DataBuffer16x1_R11 (DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(41, 0))
#define DataBuffer16x1_R12 (DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(48, 0))
#define DataBuffer16x1_R13 (DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(49, 0))
#define DataBuffer16x1_R14 (DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(56, 0))
#define DataBuffer16x1_R15 (DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(57, 0))

#define DataBuffer16x1_G0 (DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(0 + 2, 0))
#define DataBuffer16x1_G1 (DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(1 + 2, 0))
#define DataBuffer16x1_G2 (DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(8 + 2, 0))
#define DataBuffer16x1_G3 (DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(9 + 2, 0))
#define DataBuffer16x1_G4 (DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(16 + 2, 0))
#define DataBuffer16x1_G5 (DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(17 + 2, 0))
#define DataBuffer16x1_G6 (DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(24 + 2, 0))
#define DataBuffer16x1_G7 (DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(25 + 2, 0))
#define DataBuffer16x1_G8 (DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(32 + 2, 0))
#define DataBuffer16x1_G9 (DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(33 + 2, 0))
#define DataBuffer16x1_G10 (DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(40 + 2, 0))
#define DataBuffer16x1_G11 (DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(41 + 2, 0))
#define DataBuffer16x1_G12 (DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(48 + 2, 0))
#define DataBuffer16x1_G13 (DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(49 + 2, 0))
#define DataBuffer16x1_G14 (DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(56 + 2, 0))
#define DataBuffer16x1_G15 (DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(57 + 2, 0))

#define DataBuffer16x1_B0 (DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(0 + 4, 0))
#define DataBuffer16x1_B1 (DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(1 + 4, 0))
#define DataBuffer16x1_B2 (DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(8 + 4, 0))
#define DataBuffer16x1_B3 (DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(9 + 4, 0))
#define DataBuffer16x1_B4 (DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(16 + 4, 0))
#define DataBuffer16x1_B5 (DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(17 + 4, 0))
#define DataBuffer16x1_B6 (DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(24 + 4, 0))
#define DataBuffer16x1_B7 (DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(25 + 4, 0))
#define DataBuffer16x1_B8 (DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(32 + 4, 0))
#define DataBuffer16x1_B9 (DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(33 + 4, 0))
#define DataBuffer16x1_B10 (DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(40 + 4, 0))
#define DataBuffer16x1_B11 (DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(41 + 4, 0))
#define DataBuffer16x1_B12 (DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(48 + 4, 0))
#define DataBuffer16x1_B13 (DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(49 + 4, 0))
#define DataBuffer16x1_B14 (DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(56 + 4, 0))
#define DataBuffer16x1_B15 (DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(57 + 4, 0))

#define DataBuffer16x1_A0 (DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(0 + 6, 0))
#define DataBuffer16x1_A1 (DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(1 + 6, 0))
#define DataBuffer16x1_A2 (DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(8 + 6, 0))
#define DataBuffer16x1_A3 (DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(9 + 6, 0))
#define DataBuffer16x1_A4 (DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(16 + 6, 0))
#define DataBuffer16x1_A5 (DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(17 + 6, 0))
#define DataBuffer16x1_A6 (DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(24 + 6, 0))
#define DataBuffer16x1_A7 (DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(25 + 6, 0))
#define DataBuffer16x1_A8 (DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(32 + 6, 0))
#define DataBuffer16x1_A9 (DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(33 + 6, 0))
#define DataBuffer16x1_A10 (DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(40 + 6, 0))
#define DataBuffer16x1_A11 (DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(41 + 6, 0))
#define DataBuffer16x1_A12 (DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(48 + 6, 0))
#define DataBuffer16x1_A13 (DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(49 + 6, 0))
#define DataBuffer16x1_A14 (DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(56 + 6, 0))
#define DataBuffer16x1_A15 (DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(57 + 6, 0))

// Extended Message Descriptors
#define nSMPL_ENGINE        0x2
#define nMESSAGE_GATEWAY    0x3
#define nDATAPORT_DC2       0x4  // DAP Sampler Cache
#define nDATAPORT_RC        0x5  // DAP Render Cache
#define nDATAPORT_DC3       0x9  // DAP Constant Cache
#define nDATAPORT_DC0       0xA  // DAP Data Cache
#define nDATAPORT_DC1       0xC  // DAP Data Cache Data Port1
#define nURB                0x6

// Block Width and Height Size defines
#define nBLOCK_WIDTH_2   0x00000001      // Block Width  2
#define nBLOCK_WIDTH_4   0x00000003      // Block Width  4
#define nBLOCK_WIDTH_5   0x00000004      // Block Width  5
#define nBLOCK_WIDTH_8   0x00000007      // Block Width  8
#define nBLOCK_WIDTH_9   0x00000008      // Block Width  9
#define nBLOCK_WIDTH_10  0x00000009      // Block Width  10
#define nBLOCK_WIDTH_12  0x0000000B      // Block Width  12
#define nBLOCK_WIDTH_16  0x0000000F      // Block Width  16
#define nBLOCK_WIDTH_18  0x00000011      // Block Width  18
#define nBLOCK_WIDTH_20  0x00000013      // Block Width  20
#define nBLOCK_WIDTH_32  0x0000001F      // Block Width  32
#define nBLOCK_WIDTH_64  0x0000003F      // Block Width  64
#define nBLOCK_HEIGHT_1  0x00000000      // Block Height 1
#define nBLOCK_HEIGHT_2  0x00010000      // Block Height 2
#define nBLOCK_HEIGHT_3  0x00020000      // Block Height 3
#define nBLOCK_HEIGHT_4  0x00030000      // Block Height 4
#define nBLOCK_HEIGHT_5  0x00040000      // Block Height 5
#define nBLOCK_HEIGHT_6  0x00050000      // Block Height 6
#define nBLOCK_HEIGHT_8  0x00070000      // Block Height 8
#define nBLOCK_HEIGHT_9  0x00080000      // Block Height 9
#define nBLOCK_HEIGHT_16 0x000F0000      // Block Height 16
#define nBLOCK_HEIGHT_20 0x00130000      // Block Height 20

//MESSAGE DESCRIPTOR: SAMPLE_UNORM
#define nSMPL_UNORM_444_16BITS_MSGDSC_1CH   0x042EC000
#define nSMPL_UNORM_444_16BITS_MSGDSC_2CH   0x044EC000
#define nSMPL_UNORM_444_16BITS_MSGDSC_3CH   0x046EC000
#define nSMPL_UNORM_444_16BITS_MSGDSC_4CH   0x048EC000

//MESSAGE DESCRIPTOR: SAMPLE_UNORM+LUMAKEY
#define nSMPL_UNORM_LK_444_16BITS_MSGDSC_1CH    0x043EA000
#define nSMPL_UNORM_LK_444_16BITS_MSGDSC_2CH    0x045EA000
#define nSMPL_UNORM_LK_444_16BITS_MSGDSC_3CH    0x047EA000
#define nSMPL_UNORM_LK_444_16BITS_MSGDSC_4CH    0x049EA000

//MESSAGE DESCRIPTOR: SAMPLE_UNORM SIMD16
#define nSIMD16_0X_034X_MSG_DSC_4CH         0x0A8C0000 //Sample SIMD16
#define nSIMD16_0X_034X_MSG_DSC_1CH         0x0A2C0000 //Sample SIMD16
#define nSIMD16_0X_034X_MSG_DSC_2CH         0x0A4C0000 //Sample SIMD16
#define nSIMD16_0X_034X_MSG_DSC_1CH_LK      0x0A3CC000 //Sample SIMD16 + Lumakey
#define nSIMD16_0X_034X_MSG_DSC_2CH_LK      0x0A5CC000 //Sample SIMD16 + Lumakey
#define nSIMD16_0X_034X_MSG_DSC_4CH_LK      0x0A7CC000 //Sample SIMD16 + Lumakey

//Channel Enable for SAMPLE_UNORM Messages
#define nSMPL_ALL_CHANNELS_444_16       0x00000000
#define nSMPL_ALL_CHANNELS_422_16       0x00040000
#define nSMPL_ALL_CHANNELS_444_8        0x000A0000
#define nSMPL_ALL_CHANNELS_422_8        0x000C0000
#define nSMPL_RED_CHANNEL_444_16        0x0000E000
#define nSMPL_RED_CHANNEL_422_16        0x0004E000
#define nSMPL_RED_CHANNEL_444_8         0x000AE000
#define nSMPL_RED_CHANNEL_422_8         0x000CE000
#define nSMPL_GREEN_CHANNEL_444_16      0x0000D000
#define nSMPL_GREEN_CHANNEL_422_16      0x0004D000
#define nSMPL_GREEN_CHANNEL_444_8       0x000AD000
#define nSMPL_GREEN_CHANNEL_422_8       0x000CD000
#define nSMPL_RED_GREEN_CHANNEL_444_16  0x0000C000
#define nSMPL_RED_GREEN_CHANNEL_422_16  0x0004C000
#define nSMPL_RED_GREEN_CHANNEL_444_8   0x000AC000
#define nSMPL_RED_GREEN_CHANNEL_422_8   0x000CC000
#define nSMPL_RED_BLUE_CHANNEL_444_16   0x0000A000
#define nSMPL_RED_BLUE_CHANNEL_422_16   0x0004A000
#define nSMPL_RED_BLUE_CHANNEL_444_8    0x000AA000
#define nSMPL_RED_BLUE_CHANNEL_422_8    0x000CA000
#define nSMPL_UNORM_ALPHA_GREEN_CHANNEL_444_16  0x00005000

// DP read
#define nDPMW_MSGDSC                    0x020A8000
#define nDPMR_MSGDSC                    0x02090000  // Data Port Media Block Read Message Descriptor

// Message Length defines
#define nMSGLEN_1                       0x02000000 // Message Length of 1 GRF for Send
#define nMSGLEN_2                       0x04000000 // Message Length of 2 GRF for Send
#define nMSGLEN_4                       0x08000000 // Message Length of 4 GRF for Send
#define nMSGLEN_8                       0x10000000 // Message Length of 8 GRF for Send

// Response Length defines
#define nRESLEN_1                       0x00100000 // Message Response Length of 1  GRF from Send
#define nRESLEN_2                       0x00200000 // Message Response Length of 2  GRF from Send
#define nRESLEN_3                       0x00300000 // Message Response Length of 3  GRF from Send
#define nRESLEN_4                       0x00400000 // Message Response Length of 4  GRF from Send
#define nRESLEN_5                       0x00500000 // Message Response Length of 5  GRF from Send
#define nRESLEN_6                       0x00600000 // Message Response Length of 5  GRF from Send
#define nRESLEN_8                       0x00800000 // Message Response Length of 8  GRF from Send
#define nRESLEN_9                       0x00900000 // Message Response Length of 9  GRF from Send
#define nRESLEN_10                      0x00A00000 // Message Response Length of 10 GRF from Send
#define nRESLEN_11                      0x00B00000 // Message Response Length of 11 GRF from Send
#define nRESLEN_12                      0x00C00000 // Message Response Length of 12 GRF from Send
#define nRESLEN_16                      0x01000000 // Message Response Length of 16 GRF from Send

// Channel offset definition
#define Channel_Offset_RV_0             0
#define Channel_Offset_RV_1             1
#define Channel_Offset_RV_2             8
#define Channel_Offset_RV_3             9

#define Channel_Offset_GY_0             2
#define Channel_Offset_GY_1             3
#define Channel_Offset_GY_2             10
#define Channel_Offset_GY_3             11

#define Channel_Offset_BU_0             4
#define Channel_Offset_BU_1             5
#define Channel_Offset_BU_2             12
#define Channel_Offset_BU_3             13

#define Channel_Offset_A_0              6
#define Channel_Offset_A_1              7
#define Channel_Offset_A_2              14
#define Channel_Offset_A_3              15

const float Inc[16] = { 0, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f };
const float Dec[16] = { -1.0f, -2.0f, -3.0f, -4.0f, -5.0f, -6.0f, -7.0f, -8.0f, -9.0f, -10.0f, -11.0f, -12.0f, -13.0f, -14.0f, -15.0f, -16.0f };

const ushort Inc_ushort[16] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};

const short Row_Offset[4] = { 0, 0, 1, 1 };
const short Row_offset1[4] = { 8, 8, 9, 9 };
const short Colomn_Offset[4] = { 0, 8, 0, 8 };

const ushort Dither_RB0[8] = {
    0, 4, 1, 5,
    0, 4, 1, 5,
};

const ushort Dither_RB1[8] = {
    6, 2, 7, 3,
    6, 2, 7, 3,
};

const ushort Dither_RB2[8] = {
    1, 5, 0, 4,
    1, 5, 0, 4,
};

const ushort Dither_RB3[8] = {
    7, 3, 6, 2,
    7, 3, 6, 2,
};

const ushort Dither_G0[8] = {
    2, 7, 3, 6,
    2, 7, 3, 6,
};

const ushort Dither_G1[8] = {
    5, 0, 4, 1,
    5, 0, 4, 1,
};

const ushort Dither_G2[8] = {
    3, 6, 2, 7,
    3, 6, 2, 7,
};

const ushort Dither_G3[8] = {
    4, 1, 5, 0,
    4, 1, 5, 0,
};

/*typedef struct _CURBE
{
    vector<short, 12> PrimaryCscCoeff;
    vector<uchar, 7> ConstantBlendingAlpha;
    uchar PointerToInlineParameters;
    ushort Dst_Width;
    ushort Dst_Height;
    uint WAFlag;
    uint RotationChromaSitingFlag;
    vector<uint, 2> Reserved1;
    vector<uchar, 4> ColorFill;
    uchar LumakeyLowThreshold;
    uchar LumakeyHighThreshold;
    uchar Reserved2;
    uchar Reserved3;
    uchar DestinationPackedYOffset;
    uchar DestinationPackedUOffset;
    uchar DestinationPackedVOffset;
    uchar DestinationRGBFormat;
    vector<float, MDF_FC_MAX_INPUT_LAYER_NUM> Delta_X;
    vector<float, MDF_FC_MAX_INPUT_LAYER_NUM> Delta_Y;
    vector<float, MDF_FC_MAX_INPUT_LAYER_NUM> Start_Y;
    vector<float, MDF_FC_MAX_INPUT_LAYER_NUM> Start_X;
    vector<ushort, MDF_FC_MAX_INPUT_LAYER_NUM * 2> Top_Left;
    vector<ushort, MDF_FC_MAX_INPUT_LAYER_NUM * 2> Bottom_Right;
    ushort layer_num;
    ushort dst_info;
} CURBE;*/

/*_GENX_ extern void inline MDF_FC_AVS_load( matrix_ref <ushort, 16, 16> WriteBackBuffer,
                                           uchar Layer_Index,
                                           uchar Sampler_Index,
                                           float StartX,
                                           float StartY,
                                           float DeltaX,
                                           float DeltaY,
                                           ushort DstY,
                                           bool IefBypass);*/

#endif