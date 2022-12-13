/*
* Copyright (c) 2021, Intel Corporation
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
// Color Fill with no input cases
if (Layer_Index == 0xff)
{
    TempMask0 = 0;
    vector<half, 4> hfFillColor;
    hfFillColor.format<uchar>().select<4, 1>(0) = ColorFill;
    hfFillColor.format<uchar>().select<1, 1>(4) = LumakeyLowThreshold;
    hfFillColor.format<uchar>().select<1, 1>(5) = LumakeyHighThreshold;
    hfFillColor.format<uchar>().select<1, 1>(6) = Reserved2;
    hfFillColor.format<uchar>().select<1, 1>(7) = Reserved3;

    #pragma unroll
    for (int i = 0; i < 4; i++)
    {
        // R0/G0/B0/A0
        DataBuffer.format<half, 96, 16>().select<1, 1, 16, 1>(16 * i + Channel_Offset_RV_0, 0).merge(DataBuffer.format<half, 96, 16>().select<1, 1, 16, 1>(16 * i + Channel_Offset_RV_0, 0), (hfFillColor(0)), TempMask0[0][4 * i]);
        DataBuffer.format<half, 96, 16>().select<1, 1, 16, 1>(16 * i + Channel_Offset_GY_0, 0).merge(DataBuffer.format<half, 96, 16>().select<1, 1, 16, 1>(16 * i + Channel_Offset_GY_0, 0), (hfFillColor(1)), TempMask0[0][4 * i]);
        DataBuffer.format<half, 96, 16>().select<1, 1, 16, 1>(16 * i + Channel_Offset_BU_0, 0).merge(DataBuffer.format<half, 96, 16>().select<1, 1, 16, 1>(16 * i + Channel_Offset_BU_0, 0), (hfFillColor(2)), TempMask0[0][4 * i]);
        DataBuffer.format<half, 96, 16>().select<1, 1, 16, 1>(16 * i + Channel_Offset_A_0, 0).merge(DataBuffer.format<half, 96, 16>().select<1, 1, 16, 1>(16 * i + Channel_Offset_A_0, 0), (hfFillColor(3)), TempMask0[0][4 * i]);

        // R1/G1/B1/A1
        DataBuffer.format<half, 96, 16>().select<1, 1, 16, 1>(16 * i + Channel_Offset_RV_1, 0).merge(DataBuffer.format<half, 96, 16>().select<1, 1, 16, 1>(16 * i + Channel_Offset_RV_1, 0), (hfFillColor(0)), TempMask0[0][4 * i + 1]);
        DataBuffer.format<half, 96, 16>().select<1, 1, 16, 1>(16 * i + Channel_Offset_GY_1, 0).merge(DataBuffer.format<half, 96, 16>().select<1, 1, 16, 1>(16 * i + Channel_Offset_GY_1, 0), (hfFillColor(1)), TempMask0[0][4 * i + 1]);
        DataBuffer.format<half, 96, 16>().select<1, 1, 16, 1>(16 * i + Channel_Offset_BU_1, 0).merge(DataBuffer.format<half, 96, 16>().select<1, 1, 16, 1>(16 * i + Channel_Offset_BU_1, 0), (hfFillColor(2)), TempMask0[0][4 * i + 1]);
        DataBuffer.format<half, 96, 16>().select<1, 1, 16, 1>(16 * i + Channel_Offset_A_1, 0).merge(DataBuffer.format<half, 96, 16>().select<1, 1, 16, 1>(16 * i + Channel_Offset_A_1, 0), (hfFillColor(3)), TempMask0[0][4 * i + 1]);

        // R2/G2/B2/A2
        DataBuffer.format<half, 96, 16>().select<1, 1, 16, 1>(16 * i + Channel_Offset_RV_2, 0).merge(DataBuffer.format<half, 96, 16>().select<1, 1, 16, 1>(16 * i + Channel_Offset_RV_2, 0), (hfFillColor(0)), TempMask0[0][4 * i + 2]);
        DataBuffer.format<half, 96, 16>().select<1, 1, 16, 1>(16 * i + Channel_Offset_GY_2, 0).merge(DataBuffer.format<half, 96, 16>().select<1, 1, 16, 1>(16 * i + Channel_Offset_GY_2, 0), (hfFillColor(1)), TempMask0[0][4 * i + 2]);
        DataBuffer.format<half, 96, 16>().select<1, 1, 16, 1>(16 * i + Channel_Offset_BU_2, 0).merge(DataBuffer.format<half, 96, 16>().select<1, 1, 16, 1>(16 * i + Channel_Offset_BU_2, 0), (hfFillColor(2)), TempMask0[0][4 * i + 2]);
        DataBuffer.format<half, 96, 16>().select<1, 1, 16, 1>(16 * i + Channel_Offset_A_2, 0).merge(DataBuffer.format<half, 96, 16>().select<1, 1, 16, 1>(16 * i + Channel_Offset_A_2, 0), (hfFillColor(3)), TempMask0[0][4 * i + 2]);

        // R3/G3/B3/A3
        DataBuffer.format<half, 96, 16>().select<1, 1, 16, 1>(16 * i + Channel_Offset_RV_3, 0).merge(DataBuffer.format<half, 96, 16>().select<1, 1, 16, 1>(16 * i + Channel_Offset_RV_3, 0), (hfFillColor(0)), TempMask0[0][4 * i + 3]);
        DataBuffer.format<half, 96, 16>().select<1, 1, 16, 1>(16 * i + Channel_Offset_GY_3, 0).merge(DataBuffer.format<half, 96, 16>().select<1, 1, 16, 1>(16 * i + Channel_Offset_GY_3, 0), (hfFillColor(1)), TempMask0[0][4 * i + 3]);
        DataBuffer.format<half, 96, 16>().select<1, 1, 16, 1>(16 * i + Channel_Offset_BU_3, 0).merge(DataBuffer.format<half, 96, 16>().select<1, 1, 16, 1>(16 * i + Channel_Offset_BU_3, 0), (hfFillColor(2)), TempMask0[0][4 * i + 3]);
        DataBuffer.format<half, 96, 16>().select<1, 1, 16, 1>(16 * i + Channel_Offset_A_3, 0).merge(DataBuffer.format<half, 96, 16>().select<1, 1, 16, 1>(16 * i + Channel_Offset_A_3, 0), (hfFillColor(3)), TempMask0[0][4 * i + 3]);
    }

    TempMask0 = 0xFFFFFFFF;

}