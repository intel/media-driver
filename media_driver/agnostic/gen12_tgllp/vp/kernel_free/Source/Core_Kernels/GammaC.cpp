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

#include "GammaC.h"

const uchar R[16] = {
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0
};

_GENX_MAIN_ _CM_CALLABLE_ void GammaC(
    CURBE_INPUT_OUTPUT,
    GLOBAL_BUFFER_INPUT_OUTPUT)
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
    matrix_ref<ushort, 16, 16> WriteBackBuffer = DataBuffer.select<4, 1, 64, 1>(Buffer_Index << 2, 0).format<ushort, 16, 16>();
    matrix<uchar, 1, 256> GammaC_Lut_Linear = GammaC_Lut.format<uchar, 1, 256>();
    vector<uchar, 16> row(R);
    matrix<ushort, 2, 16> Temp;

#pragma unroll
    for (uchar i = 0; i < 3; i++)
    {
        Temp = WriteBackBuffer.select<2, 1, 16, 1>(2 * i, 0);
        vector<uchar, 16> LutCoordinate;
        vector<uchar, 16> LutCoordinate1;

        Temp = cm_add<ushort>(Temp, 0x80, SAT);

        LutCoordinate = Temp.row(0).format<uchar>().select<16, 2>(1);
        LutCoordinate1 = Temp.row(1).format<uchar>().select<16, 2>(1);

        WriteBackBuffer.select<1, 1, 16, 1>(2 * i, 0)     = GammaC_Lut_Linear.iselect(row, LutCoordinate);
        WriteBackBuffer.select<1, 1, 16, 1>(2 * i + 1, 0) = GammaC_Lut_Linear.iselect(row, LutCoordinate1);

        WriteBackBuffer.select<2, 1, 16, 1>(2 * i, 0) = cm_shl<ushort>(WriteBackBuffer.select<2, 1, 16, 1>(2 * i, 0), 8, SAT);
    }

#pragma unroll
    for (uchar i = 0; i < 3; i++)
    {
        Temp = WriteBackBuffer.select<2, 1, 16, 1>(2 * i + 8, 0);
        vector<uchar, 16> LutCoordinate;
        vector<uchar, 16> LutCoordinate1;

        Temp = cm_add<ushort>(Temp, 0x80, SAT);

        LutCoordinate = Temp.row(0).format<uchar>().select<16, 2>(1);
        LutCoordinate1 = Temp.row(1).format<uchar>().select<16, 2>(1);

        WriteBackBuffer.select<1, 1, 16, 1>(2 * i + 8, 0) = GammaC_Lut_Linear.iselect(row, LutCoordinate);
        WriteBackBuffer.select<1, 1, 16, 1>(2 * i + 9, 0) = GammaC_Lut_Linear.iselect(row, LutCoordinate1);

        WriteBackBuffer.select<2, 1, 16, 1>(2 * i + 8, 0) = cm_shl<ushort>(WriteBackBuffer.select<2, 1, 16, 1>(2 * i + 8, 0), 8, SAT);
    }
}
