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
{
    uchar Buffer_Index1 = (Buffer_Index & 0x0f) << 4;
    uchar Buffer_Index2 = (Buffer_Index >> 4) << 4;
    uchar maskShift = (RotationChromaSitingFlag >> 27) & 0x7;
    vector <ushort, 16> maskOffset(Dec_ushort);
    maskOffset.select<8, 1>(8) = maskOffset.select<8, 1>(8) - 8;
    maskOffset = maskOffset + maskShift;
    maskOffset = maskOffset & 0x7;
    maskOffset = maskOffset + 8;
    maskOffset = 1 << maskOffset;
    matrix <ushort, 1, 16> xorTemp;

    {
        {
            // R1/G1/B1/A1
            DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index2 + Channel_Offset_GY_0, 0) = DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index2 + Channel_Offset_GY_0, 0) & maskOffset;
            DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index2 + Channel_Offset_GY_0, 0).merge(0xFF00, 0, (DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index2 + Channel_Offset_GY_0, 0) != 0));
            DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index2 + Channel_Offset_BU_0, 0) = DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index2 + Channel_Offset_BU_0, 0) & maskOffset;
            DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index2 + Channel_Offset_BU_0, 0).merge(0xFF00, 0, (DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index2 + Channel_Offset_BU_0, 0) != 0));

            xorTemp = DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index1 + Channel_Offset_RV_0, 0) & DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index2 + Channel_Offset_GY_0, 0);
            xorTemp = xorTemp ^ DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index2 + Channel_Offset_BU_0, 0);
            DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index1 + Channel_Offset_RV_0, 0).merge(xorTemp, DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index1 + Channel_Offset_RV_0, 0), TempMask[CalculationMask]);

            xorTemp = DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index1 + Channel_Offset_GY_0, 0) & DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index2 + Channel_Offset_GY_0, 0);
            xorTemp = xorTemp ^ DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index2 + Channel_Offset_BU_0, 0);
            DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index1 + Channel_Offset_GY_0, 0).merge(xorTemp, DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index1 + Channel_Offset_GY_0, 0), TempMask[CalculationMask]);

            xorTemp = DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index1 + Channel_Offset_BU_0, 0) & DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index2 + Channel_Offset_GY_0, 0);
            xorTemp = xorTemp ^ DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index2 + Channel_Offset_BU_0, 0);
            DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index1 + Channel_Offset_BU_0, 0).merge(xorTemp, DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index1 + Channel_Offset_BU_0, 0), TempMask[CalculationMask]);

            xorTemp = DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index1 + Channel_Offset_A_0, 0) & DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index2 + Channel_Offset_GY_0, 0);
            xorTemp = xorTemp ^ DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index2 + Channel_Offset_BU_0, 0);
            DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index1 + Channel_Offset_A_0, 0).merge(xorTemp, DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index1 + Channel_Offset_A_0, 0), TempMask[CalculationMask]);
        }

        {
            DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index2 + Channel_Offset_GY_1, 0) = DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index2 + Channel_Offset_GY_1, 0) & maskOffset;
            DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index2 + Channel_Offset_GY_1, 0).merge(0xFF00, 0, (DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index2 + Channel_Offset_GY_1, 0) != 0));
            DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index2 + Channel_Offset_BU_1, 0) = DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index2 + Channel_Offset_BU_1, 0) & maskOffset;
            DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index2 + Channel_Offset_BU_1, 0).merge(0xFF00, 0, (DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index2 + Channel_Offset_BU_1, 0) != 0));

            xorTemp = DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index1 + Channel_Offset_RV_1, 0) & DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index2 + Channel_Offset_GY_1, 0);
            xorTemp = xorTemp ^ DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index2 + Channel_Offset_BU_1, 0);
            DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index1 + Channel_Offset_RV_1, 0).merge(xorTemp, DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index1 + Channel_Offset_RV_1, 0), TempMask[CalculationMask + 1]);

            xorTemp = DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index1 + Channel_Offset_GY_1, 0) & DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index2 + Channel_Offset_GY_1, 0);
            xorTemp = xorTemp ^ DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index2 + Channel_Offset_BU_1, 0);
            DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index1 + Channel_Offset_GY_1, 0).merge(xorTemp, DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index1 + Channel_Offset_GY_1, 0), TempMask[CalculationMask + 1]);

            xorTemp = DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index1 + Channel_Offset_BU_1, 0) & DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index2 + Channel_Offset_GY_1, 0);
            xorTemp = xorTemp ^ DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index2 + Channel_Offset_BU_1, 0);
            DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index1 + Channel_Offset_BU_1, 0).merge(xorTemp, DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index1 + Channel_Offset_BU_1, 0), TempMask[CalculationMask + 1]);

            xorTemp = DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index1 + Channel_Offset_A_1, 0) & DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index2 + Channel_Offset_GY_1, 0);
            xorTemp = xorTemp ^ DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index2 + Channel_Offset_BU_1, 0);
            DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index1 + Channel_Offset_A_1, 0).merge(xorTemp, DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index1 + Channel_Offset_A_1, 0), TempMask[CalculationMask + 1]);

            // R2/G2/B2/A2
            //DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index1 + Channel_Offset_RV_1, 0).merge(DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index2 + Channel_Offset_RV_1, 0), DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index1 + Channel_Offset_RV_1, 0), TempMask[CalculationMask + 1]);
            //DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index1 + Channel_Offset_GY_1, 0).merge(DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index2 + Channel_Offset_GY_1, 0), DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index1 + Channel_Offset_GY_1, 0), TempMask[CalculationMask + 1]);
            //DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index1 + Channel_Offset_BU_1, 0).merge(DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index2 + Channel_Offset_BU_1, 0), DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index1 + Channel_Offset_BU_1, 0), TempMask[CalculationMask + 1]);
            //DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index1 + Channel_Offset_A_1, 0).merge(DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index2 + Channel_Offset_A_1, 0), DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index1 + Channel_Offset_A_1, 0), TempMask[CalculationMask + 1]);
        }

        {
            DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index2 + Channel_Offset_GY_2, 0) = DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index2 + Channel_Offset_GY_2, 0) & maskOffset;
            DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index2 + Channel_Offset_GY_2, 0).merge(0xFF00, 0, (DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index2 + Channel_Offset_GY_2, 0) != 0));
            DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index2 + Channel_Offset_BU_2, 0) = DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index2 + Channel_Offset_BU_2, 0) & maskOffset;
            DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index2 + Channel_Offset_BU_2, 0).merge(0xFF00, 0, (DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index2 + Channel_Offset_BU_2, 0) != 0));

            xorTemp = DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index1 + Channel_Offset_RV_2, 0) & DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index2 + Channel_Offset_GY_2, 0);
            xorTemp = xorTemp ^ DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index2 + Channel_Offset_BU_2, 0);
            DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index1 + Channel_Offset_RV_2, 0).merge(xorTemp, DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index1 + Channel_Offset_RV_2, 0), TempMask[CalculationMask + 2]);

            xorTemp = DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index1 + Channel_Offset_GY_2, 0) & DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index2 + Channel_Offset_GY_2, 0);
            xorTemp = xorTemp ^ DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index2 + Channel_Offset_BU_2, 0);
            DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index1 + Channel_Offset_GY_2, 0).merge(xorTemp, DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index1 + Channel_Offset_GY_2, 0), TempMask[CalculationMask + 2]);

            xorTemp = DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index1 + Channel_Offset_BU_2, 0) & DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index2 + Channel_Offset_GY_2, 0);
            xorTemp = xorTemp ^ DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index2 + Channel_Offset_BU_2, 0);
            DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index1 + Channel_Offset_BU_2, 0).merge(xorTemp, DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index1 + Channel_Offset_BU_2, 0), TempMask[CalculationMask + 2]);

            xorTemp = DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index1 + Channel_Offset_A_2, 0) & DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index2 + Channel_Offset_GY_2, 0);
            xorTemp = xorTemp ^ DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index2 + Channel_Offset_BU_2, 0);
            DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index1 + Channel_Offset_A_2, 0).merge(xorTemp, DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index1 + Channel_Offset_A_2, 0), TempMask[CalculationMask + 2]);

            // R3/G3/B3/A3
            //DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index1 + Channel_Offset_RV_2, 0).merge(DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index2 + Channel_Offset_RV_2, 0), DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index1 + Channel_Offset_RV_2, 0), TempMask[CalculationMask + 2]);
            //DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index1 + Channel_Offset_GY_2, 0).merge(DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index2 + Channel_Offset_GY_2, 0), DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index1 + Channel_Offset_GY_2, 0), TempMask[CalculationMask + 2]);
            //DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index1 + Channel_Offset_BU_2, 0).merge(DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index2 + Channel_Offset_BU_2, 0), DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index1 + Channel_Offset_BU_2, 0), TempMask[CalculationMask + 2]);
            //DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index1 + Channel_Offset_A_2, 0).merge(DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index2 + Channel_Offset_A_2, 0), DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index1 + Channel_Offset_A_2, 0), TempMask[CalculationMask + 2]);
        }

        {
            DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index2 + Channel_Offset_GY_3, 0) = DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index2 + Channel_Offset_GY_3, 0) & maskOffset;
            DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index2 + Channel_Offset_GY_3, 0).merge(0xFF00, 0, (DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index2 + Channel_Offset_GY_3, 0) != 0));
            DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index2 + Channel_Offset_BU_3, 0) = DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index2 + Channel_Offset_BU_3, 0) & maskOffset;
            DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index2 + Channel_Offset_BU_3, 0).merge(0xFF00, 0, (DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index2 + Channel_Offset_BU_3, 0) != 0));

            xorTemp = DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index1 + Channel_Offset_RV_3, 0) & DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index2 + Channel_Offset_GY_3, 0);
            xorTemp = xorTemp ^ DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index2 + Channel_Offset_BU_3, 0);
            DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index1 + Channel_Offset_RV_3, 0).merge(xorTemp, DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index1 + Channel_Offset_RV_3, 0), TempMask[CalculationMask + 3]);

            xorTemp = DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index1 + Channel_Offset_GY_3, 0) & DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index2 + Channel_Offset_GY_3, 0);
            xorTemp = xorTemp ^ DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index2 + Channel_Offset_BU_3, 0);
            DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index1 + Channel_Offset_GY_3, 0).merge(xorTemp, DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index1 + Channel_Offset_GY_3, 0), TempMask[CalculationMask + 3]);

            xorTemp = DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index1 + Channel_Offset_BU_3, 0) & DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index2 + Channel_Offset_GY_3, 0);
            xorTemp = xorTemp ^ DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index2 + Channel_Offset_BU_3, 0);
            DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index1 + Channel_Offset_BU_3, 0).merge(xorTemp, DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index1 + Channel_Offset_BU_3, 0), TempMask[CalculationMask + 3]);

            xorTemp = DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index1 + Channel_Offset_A_3, 0) & DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index2 + Channel_Offset_GY_3, 0);
            xorTemp = xorTemp ^ DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index2 + Channel_Offset_BU_3, 0);
            DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index1 + Channel_Offset_A_3, 0).merge(xorTemp, DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index1 + Channel_Offset_A_3, 0), TempMask[CalculationMask + 3]);

            // R4/G4/B4/A4
            //DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index1 + Channel_Offset_RV_3, 0).merge(DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index2 + Channel_Offset_RV_3, 0), DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index1 + Channel_Offset_RV_3, 0), TempMask[CalculationMask + 3]);
            //DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index1 + Channel_Offset_GY_3, 0).merge(DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index2 + Channel_Offset_GY_3, 0), DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index1 + Channel_Offset_GY_3, 0), TempMask[CalculationMask + 3]);
            //DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index1 + Channel_Offset_BU_3, 0).merge(DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index2 + Channel_Offset_BU_3, 0), DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index1 + Channel_Offset_BU_3, 0), TempMask[CalculationMask + 3]);
            //DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index1 + Channel_Offset_A_3, 0).merge(DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index2 + Channel_Offset_A_3, 0), DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index1 + Channel_Offset_A_3, 0), TempMask[CalculationMask + 3]);
        }
    }
}