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
    uchar Buffer_Index1 = (Buffer_Index & 0x0f) << 4;
    uchar Buffer_Index2 = (Buffer_Index >> 4) << 4;

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

    {
        matrix<uint, 1, 16> Temp;
        {
            // R1/G1/B1/A1
            matrix<ushort, 1, 16> Alpha = DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index2 + 6, 0);

            {
                Temp = (((DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index1 + Channel_Offset_RV_0, 0) * (cm_add<ushort>(0xFF00, - Alpha, SAT)))) + ((DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index2 + Channel_Offset_RV_0, 0) * 0xFFFF))) >> 16;
                DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index1 + Channel_Offset_RV_0, 0).merge(Temp.format<ushort, 1, 32>().select<1, 1, 16, 2>(0, 1), DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index1 + Channel_Offset_RV_0, 0), TempMask[CalculationMask]);
            }

            {
                Temp = (((DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index1 + Channel_Offset_GY_0, 0) * (cm_add<ushort>(0xFF00, - Alpha, SAT)))) + ((DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index2 + Channel_Offset_GY_0, 0) * 0xFFFF))) >> 16;
                DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index1 + Channel_Offset_GY_0, 0).merge(Temp.format<ushort, 1, 32>().select<1, 1, 16, 2>(0, 1), DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index1 + Channel_Offset_GY_0, 0), TempMask[CalculationMask]);
            }

            {
                Temp = (((DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index1 + Channel_Offset_BU_0, 0) * (cm_add<ushort>(0xFF00, - Alpha, SAT)))) + ((DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index2 + Channel_Offset_BU_0, 0) * 0xFFFF))) >> 16;
                DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index1 + Channel_Offset_BU_0, 0).merge(Temp.format<ushort, 1, 32>().select<1, 1, 16, 2>(0, 1), DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index1 + Channel_Offset_BU_0, 0), TempMask[CalculationMask]);
            }
        }

        {
            // R2/G2/B2/A2
            matrix<ushort, 1, 16> Alpha = DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index2 + 7, 0);

            {
                Temp = (((DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index1 + Channel_Offset_RV_1, 0) * (cm_add<ushort>(0xFF00, - Alpha, SAT)))) + ((DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index2 + Channel_Offset_RV_1, 0) * 0xFFFF))) >> 16;
                DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index1 + Channel_Offset_RV_1, 0).merge(Temp.format<ushort, 1, 32>().select<1, 1, 16, 2>(0, 1), DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index1 + Channel_Offset_RV_1, 0), TempMask[CalculationMask + 1]);
            }

            {
                Temp = (((DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index1 + Channel_Offset_GY_1, 0) * (cm_add<ushort>(0xFF00, - Alpha, SAT)))) + ((DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index2 + Channel_Offset_GY_1, 0) * 0xFFFF))) >> 16;
                DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index1 + Channel_Offset_GY_1, 0).merge(Temp.format<ushort, 1, 32>().select<1, 1, 16, 2>(0, 1), DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index1 + Channel_Offset_GY_1, 0), TempMask[CalculationMask + 1]);
            }

            {
                Temp = (((DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index1 + Channel_Offset_BU_1, 0) * (cm_add<ushort>(0xFF00, - Alpha, SAT)))) + ((DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index2 + Channel_Offset_BU_1, 0) * 0xFFFF))) >> 16;
                DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index1 + Channel_Offset_BU_1, 0).merge(Temp.format<ushort, 1, 32>().select<1, 1, 16, 2>(0, 1), DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index1 + Channel_Offset_BU_1, 0), TempMask[CalculationMask + 1]);
            }
        }

        {
            // R3/G3/B3/A3
            matrix<ushort, 1, 16> Alpha = DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index2 + 14, 0);

            {
                Temp = (((DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index1 + Channel_Offset_RV_2, 0) * (cm_add<ushort>(0xFF00, - Alpha, SAT)))) + ((DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index2 + Channel_Offset_RV_2, 0) * 0xFFFF))) >> 16;
                DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index1 + Channel_Offset_RV_2, 0).merge(Temp.format<ushort, 1, 32>().select<1, 1, 16, 2>(0, 1), DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index1 + Channel_Offset_RV_2, 0), TempMask[CalculationMask + 2]);
            }

            {
                Temp = (((DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index1 + Channel_Offset_GY_2, 0) * (cm_add<ushort>(0xFF00, - Alpha, SAT)))) + ((DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index2 + Channel_Offset_GY_2, 0) * 0xFFFF))) >> 16;
                DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index1 + Channel_Offset_GY_2, 0).merge(Temp.format<ushort, 1, 32>().select<1, 1, 16, 2>(0, 1), DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index1 + Channel_Offset_GY_2, 0), TempMask[CalculationMask + 2]);
            }

            {
                Temp = (((DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index1 + Channel_Offset_BU_2, 0) * (cm_add<ushort>(0xFF00, - Alpha, SAT)))) + ((DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index2 + Channel_Offset_BU_2, 0) * 0xFFFF))) >> 16;
                DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index1 + Channel_Offset_BU_2, 0).merge(Temp.format<ushort, 1, 32>().select<1, 1, 16, 2>(0, 1), DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index1 + Channel_Offset_BU_2, 0), TempMask[CalculationMask + 2]);
            }
        }

        {
            // R4/G4/B4/A4
            matrix<ushort, 1, 16> Alpha = DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index2 + 15, 0);

            {
                Temp = (((DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index1 + Channel_Offset_RV_3, 0) * (cm_add<ushort>(0xFF00, - Alpha, SAT)))) + ((DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index2 + Channel_Offset_RV_3, 0) * 0xFFFF))) >> 16;
                DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index1 + Channel_Offset_RV_3, 0).merge(Temp.format<ushort, 1, 32>().select<1, 1, 16, 2>(0, 1), DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index1 + Channel_Offset_RV_3, 0), TempMask[CalculationMask + 3]);
            }

            {
                Temp = (((DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index1 + Channel_Offset_GY_3, 0) * (cm_add<ushort>(0xFF00, - Alpha, SAT)))) + ((DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index2 + Channel_Offset_GY_3, 0) * 0xFFFF))) >> 16;
                DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index1 + Channel_Offset_GY_3, 0).merge(Temp.format<ushort, 1, 32>().select<1, 1, 16, 2>(0, 1), DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index1 + Channel_Offset_GY_3, 0), TempMask[CalculationMask + 3]);
            }

            {
                Temp = (((DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index1 + Channel_Offset_BU_3, 0) * (cm_add<ushort>(0xFF00, - Alpha, SAT)))) + ((DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index2 + Channel_Offset_BU_3, 0) * 0xFFFF))) >> 16;
                DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index1 + Channel_Offset_BU_3, 0).merge(Temp.format<ushort, 1, 32>().select<1, 1, 16, 2>(0, 1), DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(Buffer_Index1 + Channel_Offset_BU_3, 0), TempMask[CalculationMask + 3]);
            }
        }
    }

}