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
    matrix<ushort, 1, 16> Temp;
    uchar ConstAlpha = (ColorFill(3));

#pragma unroll
    for (int i = 0; i < 4; i++)
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
        {
            matrix<ushort, 1, 16> Alpha = DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(16 * i + 6, 0);
            Alpha = (Alpha * (cm_add<uchar>(ConstAlpha, 1, SAT))) >> 8;

            // R0/G0/B0/A0
            Temp = (((DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(16 * i + Channel_Offset_RV_0, 0) * (cm_add<ushort>(Alpha, 256, SAT)))) + (((ColorFill(0) << 8) * (cm_add<ushort>(0xFF00, - Alpha, SAT))))) >> 16;
            DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(16 * i + Channel_Offset_RV_0, 0).merge(Temp, (ColorFill(0) << 8), TempMask0[0][4 * i]);
            Temp = (((DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(16 * i + Channel_Offset_GY_0, 0) * (cm_add<ushort>(Alpha, 256, SAT)))) + (((ColorFill(1) << 8) * (cm_add<ushort>(0xFF00, - Alpha, SAT))))) >> 16;
            DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(16 * i + Channel_Offset_GY_0, 0).merge(Temp, (ColorFill(1) << 8), TempMask0[0][4 * i]);
            Temp = (((DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(16 * i + Channel_Offset_BU_0, 0) * (cm_add<ushort>(Alpha, 256, SAT)))) + (((ColorFill(2) << 8) * (cm_add<ushort>(0xFF00, - Alpha, SAT))))) >> 16;
            DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(16 * i + Channel_Offset_BU_0, 0).merge(Temp, (ColorFill(2) << 8), TempMask0[0][4 * i]);
            Temp = (((DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(16 * i + Channel_Offset_A_0, 0) * (cm_add<ushort>(Alpha, 256, SAT)))) + (((ColorFill(3) << 8) * (cm_add<ushort>(0xFF00, - Alpha, SAT))))) >> 16;
            DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(16 * i + Channel_Offset_A_0, 0).merge(Temp, (ColorFill(3) << 8), TempMask0[0][4 * i]);
        }

        {
            matrix<ushort, 1, 16> Alpha = DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(16 * i + 7, 0);
            Alpha = (Alpha * (cm_add<uchar>(ConstAlpha, 1, SAT))) >> 8;

            // R1/G1/B1/A1
            Temp = (((DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(16 * i + Channel_Offset_RV_1, 0) * (cm_add<ushort>(Alpha, 256, SAT)))) + (((ColorFill(0) << 8) * (cm_add<ushort>(0xFF00, - Alpha, SAT))))) >> 16;
            DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(16 * i + Channel_Offset_RV_1, 0).merge(Temp, (ColorFill(0) << 8), TempMask0[0][4 * i + 1]);
            Temp = (((DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(16 * i + Channel_Offset_GY_1, 0) * (cm_add<ushort>(Alpha, 256, SAT)))) + (((ColorFill(1) << 8) * (cm_add<ushort>(0xFF00, - Alpha, SAT))))) >> 16;
            DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(16 * i + Channel_Offset_GY_1, 0).merge(Temp, (ColorFill(1) << 8), TempMask0[0][4 * i + 1]);
            Temp = (((DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(16 * i + Channel_Offset_BU_1, 0) * (cm_add<ushort>(Alpha, 256, SAT)))) + (((ColorFill(2) << 8) * (cm_add<ushort>(0xFF00, - Alpha, SAT))))) >> 16;
            DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(16 * i + Channel_Offset_BU_1, 0).merge(Temp, (ColorFill(2) << 8), TempMask0[0][4 * i + 1]);
            Temp = (((DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(16 * i + Channel_Offset_A_1, 0) * (cm_add<ushort>(Alpha, 256, SAT)))) + (((ColorFill(3) << 8) * (cm_add<ushort>(0xFF00, - Alpha, SAT))))) >> 16;
            DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(16 * i + Channel_Offset_A_1, 0).merge(Temp, (ColorFill(3) << 8), TempMask0[0][4 * i + 1]);
        }

        {
            matrix<ushort, 1, 16> Alpha = DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(16 * i + 14, 0);
            Alpha = (Alpha * (cm_add<uchar>(ConstAlpha, 1, SAT))) >> 8;

            // R2/G2/B2/A2
            Temp = (((DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(16 * i + Channel_Offset_RV_2, 0) * (cm_add<ushort>(Alpha, 256, SAT)))) + (((ColorFill(0) << 8) * (cm_add<ushort>(0xFF00, - Alpha, SAT))))) >> 16;
            DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(16 * i + Channel_Offset_RV_2, 0).merge(Temp, (ColorFill(0) << 8), TempMask0[0][4 * i + 2]);
            Temp = (((DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(16 * i + Channel_Offset_GY_2, 0) * (cm_add<ushort>(Alpha, 256, SAT)))) + (((ColorFill(1) << 8) * (cm_add<ushort>(0xFF00, - Alpha, SAT))))) >> 16;
            DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(16 * i + Channel_Offset_GY_2, 0).merge(Temp, (ColorFill(1) << 8), TempMask0[0][4 * i + 2]);
            Temp = (((DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(16 * i + Channel_Offset_BU_2, 0) * (cm_add<ushort>(Alpha, 256, SAT)))) + (((ColorFill(2) << 8) * (cm_add<ushort>(0xFF00, - Alpha, SAT))))) >> 16;
            DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(16 * i + Channel_Offset_BU_2, 0).merge(Temp, (ColorFill(2) << 8), TempMask0[0][4 * i + 2]);
            Temp = (((DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(16 * i + Channel_Offset_A_2, 0) * (cm_add<ushort>(Alpha, 256, SAT)))) + (((ColorFill(3) << 8) * (cm_add<ushort>(0xFF00, - Alpha, SAT))))) >> 16;
            DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(16 * i + Channel_Offset_A_2, 0).merge(Temp, (ColorFill(3) << 8), TempMask0[0][4 * i + 2]);
        }

        {
            matrix<ushort, 1, 16> Alpha = DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(16 * i + 15, 0);
            Alpha = (Alpha * (cm_add<uchar>(ConstAlpha, 1, SAT))) >> 8;

            // R3/G3/B3/A3
            Temp = (((DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(16 * i + Channel_Offset_RV_3, 0) * (cm_add<ushort>(Alpha, 256, SAT)))) + (((ColorFill(0) << 8) * (cm_add<ushort>(0xFF00, - Alpha, SAT))))) >> 16;
            DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(16 * i + Channel_Offset_RV_3, 0).merge(Temp, (ColorFill(0) << 8), TempMask0[0][4 * i + 3]);
            Temp = (((DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(16 * i + Channel_Offset_GY_3, 0) * (cm_add<ushort>(Alpha, 256, SAT)))) + (((ColorFill(1) << 8) * (cm_add<ushort>(0xFF00, - Alpha, SAT))))) >> 16;
            DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(16 * i + Channel_Offset_GY_3, 0).merge(Temp, (ColorFill(1) << 8), TempMask0[0][4 * i + 3]);
            Temp = (((DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(16 * i + Channel_Offset_BU_3, 0) * (cm_add<ushort>(Alpha, 256, SAT)))) + (((ColorFill(2) << 8) * (cm_add<ushort>(0xFF00, - Alpha, SAT))))) >> 16;
            DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(16 * i + Channel_Offset_BU_3, 0).merge(Temp, (ColorFill(2) << 8), TempMask0[0][4 * i + 3]);
            Temp = (((DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(16 * i + Channel_Offset_A_3, 0) * (cm_add<ushort>(Alpha, 256, SAT)))) + (((ColorFill(3) << 8) * (cm_add<ushort>(0xFF00, - Alpha, SAT))))) >> 16;
            DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(16 * i + Channel_Offset_A_3, 0).merge(Temp, (ColorFill(3) << 8), TempMask0[0][4 * i + 3]);
        }
    }
    TempMask0 = 0xFFFFFFFF;
}