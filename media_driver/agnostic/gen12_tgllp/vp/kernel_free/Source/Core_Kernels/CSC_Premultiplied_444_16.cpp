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

#include "CSC_Premultiplied_444_16.h"

_GENX_MAIN_ _CM_CALLABLE_ void CSC_Premultiplied_444_16(
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

    uchar CSCShift = (uchar)((WAFlag & 0xe0) >> 5);
    CSCShift = CSCShift + 7;

    matrix_ref<ushort, 4, 64> WriteBackBuffer = DataBuffer.select<4, 1, 64, 1>(Buffer_Index << 2, 0);

#define tempA WriteBackBuffer.format<ushort, 16, 16>().select<1, 1, 16, 1>(6, 0)
    matrix<ushort, 1, 16> tempR = WriteBackBuffer.format<ushort, 16, 16>().select<1, 1, 16, 1>(0, 0);
    matrix<ushort, 1, 16> tempG = WriteBackBuffer.format<ushort, 16, 16>().select<1, 1, 16, 1>(2, 0);

    WriteBackBuffer.format<ushort, 16, 16>().select<1, 1, 16, 1>(0, 0) = cm_shr<ushort>(tempR * CscCoeff[0][2] + tempG * CscCoeff[0][0] + WriteBackBuffer.format<ushort, 16, 16>().select<1, 1, 16, 1>(4, 0) * CscCoeff[0][1] + CscCoeff[0][3] * tempA, CSCShift, SAT);
    WriteBackBuffer.format<ushort, 16, 16>().select<1, 1, 16, 1>(2, 0) = cm_shr<ushort>(tempR * CscCoeff[0][6] + tempG * CscCoeff[0][4] + WriteBackBuffer.format<ushort, 16, 16>().select<1, 1, 16, 1>(4, 0) * CscCoeff[0][5] + CscCoeff[0][7] * tempA, CSCShift, SAT);
    WriteBackBuffer.format<ushort, 16, 16>().select<1, 1, 16, 1>(4, 0) = cm_shr<ushort>(WriteBackBuffer.format<ushort, 16, 16>().select<1, 1, 16, 1>(4, 0) * CscCoeff[0][9] + tempR * CscCoeff[0][10] + tempG * CscCoeff[0][8] + CscCoeff[0][11] * tempA, CSCShift, SAT);
#undef tempA

#define tempA WriteBackBuffer.format<ushort, 16, 16>().select<1, 1, 16, 1>(7, 0)
    tempR = WriteBackBuffer.format<ushort, 16, 16>().select<1, 1, 16, 1>(1, 0);
    tempG = WriteBackBuffer.format<ushort, 16, 16>().select<1, 1, 16, 1>(3, 0);

    WriteBackBuffer.format<ushort, 16, 16>().select<1, 1, 16, 1>(1, 0) = cm_shr<ushort>(tempR * CscCoeff[0][2] + tempG * CscCoeff[0][0] + WriteBackBuffer.format<ushort, 16, 16>().select<1, 1, 16, 1>(5, 0)  * CscCoeff[0][1] + CscCoeff[0][3] * tempA, CSCShift, SAT);
    WriteBackBuffer.format<ushort, 16, 16>().select<1, 1, 16, 1>(3, 0) = cm_shr<ushort>(tempR * CscCoeff[0][6] + tempG * CscCoeff[0][4] + WriteBackBuffer.format<ushort, 16, 16>().select<1, 1, 16, 1>(5, 0)  * CscCoeff[0][5] + CscCoeff[0][7] * tempA, CSCShift, SAT);
    WriteBackBuffer.format<ushort, 16, 16>().select<1, 1, 16, 1>(5, 0) = cm_shr<ushort>(WriteBackBuffer.format<ushort, 16, 16>().select<1, 1, 16, 1>(5, 0) * CscCoeff[0][9] + tempR * CscCoeff[0][10] + tempG * CscCoeff[0][8] + CscCoeff[0][11] * tempA, CSCShift, SAT);
#undef tempA

#define tempA WriteBackBuffer.format<ushort, 16, 16>().select<1, 1, 16, 1>(14, 0)
    tempR = WriteBackBuffer.format<ushort, 16, 16>().select<1, 1, 16, 1>(8, 0);
    tempG = WriteBackBuffer.format<ushort, 16, 16>().select<1, 1, 16, 1>(10, 0);

    WriteBackBuffer.format<ushort, 16, 16>().select<1, 1, 16, 1>(8, 0) = cm_shr<ushort>(tempR * CscCoeff[0][2] + tempG * CscCoeff[0][0] + WriteBackBuffer.format<ushort, 16, 16>().select<1, 1, 16, 1>(12, 0) * CscCoeff[0][1] + CscCoeff[0][3] * tempA, CSCShift, SAT);
    WriteBackBuffer.format<ushort, 16, 16>().select<1, 1, 16, 1>(10, 0) = cm_shr<ushort>(tempR * CscCoeff[0][6] + tempG * CscCoeff[0][4] + WriteBackBuffer.format<ushort, 16, 16>().select<1, 1, 16, 1>(12, 0) * CscCoeff[0][5] + CscCoeff[0][7] * tempA, CSCShift, SAT);
    WriteBackBuffer.format<ushort, 16, 16>().select<1, 1, 16, 1>(12, 0) = cm_shr<ushort>(WriteBackBuffer.format<ushort, 16, 16>().select<1, 1, 16, 1>(12, 0) * CscCoeff[0][9] + tempR * CscCoeff[0][10] + tempG * CscCoeff[0][8] + CscCoeff[0][11] * tempA, CSCShift, SAT);
#undef tempA

#define tempA WriteBackBuffer.format<ushort, 16, 16>().select<1, 1, 16, 1>(15, 0)
    tempR = WriteBackBuffer.format<ushort, 16, 16>().select<1, 1, 16, 1>(9, 0);
    tempG = WriteBackBuffer.format<ushort, 16, 16>().select<1, 1, 16, 1>(11, 0);

    WriteBackBuffer.format<ushort, 16, 16>().select<1, 1, 16, 1>(9, 0) = cm_shr<ushort>(tempR * CscCoeff[0][2] + tempG * CscCoeff[0][0] + WriteBackBuffer.format<ushort, 16, 16>().select<1, 1, 16, 1>(13, 0) * CscCoeff[0][1] + CscCoeff[0][3] * tempA, CSCShift, SAT);
    WriteBackBuffer.format<ushort, 16, 16>().select<1, 1, 16, 1>(11, 0) = cm_shr<ushort>(tempR * CscCoeff[0][6] + tempG * CscCoeff[0][4] + WriteBackBuffer.format<ushort, 16, 16>().select<1, 1, 16, 1>(13, 0) * CscCoeff[0][5] + CscCoeff[0][7] * tempA, CSCShift, SAT);
    WriteBackBuffer.format<ushort, 16, 16>().select<1, 1, 16, 1>(13, 0) = cm_shr<ushort>(WriteBackBuffer.format<ushort, 16, 16>().select<1, 1, 16, 1>(13, 0) * CscCoeff[0][9] + tempR * CscCoeff[0][10] + tempG * CscCoeff[0][8] + CscCoeff[0][11] * tempA, CSCShift, SAT);
#undef tempA
}
