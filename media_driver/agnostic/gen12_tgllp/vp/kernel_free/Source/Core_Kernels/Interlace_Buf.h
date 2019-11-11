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
    uchar RotationFlag = (uchar)(RotationChromaSitingFlag & 0x07);

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

    // ==== Pre-Interlaced =====
    // Buffer 0 (Field 1):
    // ----------------------------------------------
    // | Line 1 Left Half F1 | Line 1 Right Half F1 |
    // | Line 2 Left Half F1 | Line 2 Right Half F1 |
    // | Line 3 Left Half F1 | Line 3 Right Half F1 |
    // | Line 4 Left Half F1 | Line 4 Right Half F1 |
    // ---------------------------------------------- 
    // Buffer 1 (Field 2):
    // ----------------------------------------------
    // | Line 1 Left Half F2 | Line 1 Right Half F2 |
    // | Line 2 Left Half F2 | Line 2 Right Half F2 |
    // | Line 3 Left Half F2 | Line 3 Right Half F2 |
    // | Line 4 Left Half F2 | Line 4 Right Half F2 |
    // ---------------------------------------------- 

    // ==== Post-Interlaced =====
    // Buffer 0:
    // ----------------------------------------------
    // | Line 1 Left Half F1 | Line 1 Right Half F1 |
    // | Line 1 Left Half F2 | Line 1 Right Half F2 |
    // | Line 2 Left Half F1 | Line 2 Right Half F1 |
    // | Line 2 Left Half F2 | Line 2 Right Half F2 |
    // ---------------------------------------------- 
    // Buffer 1:
    // ----------------------------------------------
    // | Line 3 Left Half F1 | Line 3 Right Half F1 |
    // | Line 3 Left Half F2 | Line 3 Right Half F2 |
    // | Line 4 Left Half F1 | Line 4 Right Half F1 |
    // | Line 4 Left Half F2 | Line 4 Right Half F2 |
    // ---------------------------------------------- 

#ifdef BUFFER_0
#define WriteBackBuffer_F1 DataBuffer0
#define WriteBackBuffer_F2 DataBuffer1
#endif

#ifdef BUFFER_2
#define WriteBackBuffer_F1 DataBuffer2
#define WriteBackBuffer_F2 DataBuffer3
#endif

    if (RotationFlag == MDF_FC_ROTATION_90 || RotationFlag == MDF_FC_ROTATION_270)
    {
        matrix<ushort, 1, 8> temp;
        matrix_ref<ushort, 2, 16> temp1 = DataBuffer4.format<ushort, 16, 16>().select<2, 1, 16, 1>(0, 0);
        matrix_ref<ushort, 1, 16> temp2 = DataBuffer4.format<ushort, 16, 16>().select<1, 1, 16, 1>(2, 0);

#ifdef OUTPUT_PA
#pragma unroll
        for (short j = 0; j < 4; j++)
        {
            // Store temp data
            temp1 = WriteBackBuffer_F1.format<ushort, 16, 16>().select<2, 1, 16, 1>(2 * j + 8, 0);
            temp2.select<1, 1, 4, 1>(0, 0) = WriteBackBuffer_F1.format<ushort, 16, 16>().select<1, 1, 4, 1>(2 * j, 4);
            temp2.select<1, 1, 4, 1>(0, 4) = WriteBackBuffer_F1.format<ushort, 16, 16>().select<1, 1, 4, 1>(2 * j, 12);
            temp2.select<1, 1, 4, 1>(0, 8) = WriteBackBuffer_F1.format<ushort, 16, 16>().select<1, 1, 4, 1>(2 * j + 1, 4);
            temp2.select<1, 1, 4, 1>(0, 12) = WriteBackBuffer_F1.format<ushort, 16, 16>().select<1, 1, 4, 1>(2 * j + 1, 12);

            // Interlace Top field
            temp.select<1, 1, 4, 2>(0, 0) = WriteBackBuffer_F2.format<ushort, 16, 16>().select<1, 1, 4, 1>(2 * j, 0);
            temp.select<1, 1, 4, 2>(0, 1) = WriteBackBuffer_F1.format<ushort, 16, 16>().select<1, 1, 4, 1>(2 * j, 0);
            WriteBackBuffer_F1.format<ushort, 16, 16>().select<1, 1, 8, 1>(2 * j, 0) = temp;

            temp.select<1, 1, 4, 2>(0, 0) = WriteBackBuffer_F2.format<ushort, 16, 16>().select<1, 1, 4, 1>(2 * j, 8);
            temp.select<1, 1, 4, 2>(0, 1) = WriteBackBuffer_F1.format<ushort, 16, 16>().select<1, 1, 4, 1>(2 * j, 8);
            WriteBackBuffer_F1.format<ushort, 16, 16>().select<1, 1, 8, 1>(2 * j, 8) = temp;

            temp.select<1, 1, 4, 2>(0, 0) = WriteBackBuffer_F2.format<ushort, 16, 16>().select<1, 1, 4, 1>(2 * j + 1, 0);
            temp.select<1, 1, 4, 2>(0, 1) = WriteBackBuffer_F1.format<ushort, 16, 16>().select<1, 1, 4, 1>(2 * j + 1, 0);
            WriteBackBuffer_F1.format<ushort, 16, 16>().select<1, 1, 8, 1>(2 * j + 1, 0) = temp;

            temp.select<1, 1, 4, 2>(0, 0) = WriteBackBuffer_F2.format<ushort, 16, 16>().select<1, 1, 4, 1>(2 * j + 1, 8);
            temp.select<1, 1, 4, 2>(0, 1) = WriteBackBuffer_F1.format<ushort, 16, 16>().select<1, 1, 4, 1>(2 * j + 1, 8);
            WriteBackBuffer_F1.format<ushort, 16, 16>().select<1, 1, 8, 1>(2 * j + 1, 8) = temp;


            temp.select<1, 1, 4, 2>(0, 0) = WriteBackBuffer_F2.format<ushort, 16, 16>().select<1, 1, 4, 1>(2 * j, 4);
            temp.select<1, 1, 4, 2>(0, 1) = temp2.select<1, 1, 4, 1>(0, 0);
            WriteBackBuffer_F1.format<ushort, 16, 16>().select<1, 1, 8, 1>(2 * j + 8, 0) = temp;

            temp.select<1, 1, 4, 2>(0, 0) = WriteBackBuffer_F2.format<ushort, 16, 16>().select<1, 1, 4, 1>(2 * j, 12);
            temp.select<1, 1, 4, 2>(0, 1) = temp2.select<1, 1, 4, 1>(0, 4);
            WriteBackBuffer_F1.format<ushort, 16, 16>().select<1, 1, 8, 1>(2 * j + 8, 8) = temp;

            temp.select<1, 1, 4, 2>(0, 0) = WriteBackBuffer_F2.format<ushort, 16, 16>().select<1, 1, 4, 1>(2 * j + 1, 4);
            temp.select<1, 1, 4, 2>(0, 1) = temp2.select<1, 1, 4, 1>(0, 8);
            WriteBackBuffer_F1.format<ushort, 16, 16>().select<1, 1, 8, 1>(2 * j + 9, 0) = temp;

            temp.select<1, 1, 4, 2>(0, 0) = WriteBackBuffer_F2.format<ushort, 16, 16>().select<1, 1, 4, 1>(2 * j + 1, 12);
            temp.select<1, 1, 4, 2>(0, 1) = temp2.select<1, 1, 4, 1>(0, 12);
            WriteBackBuffer_F1.format<ushort, 16, 16>().select<1, 1, 8, 1>(2 * j + 9, 8) = temp;

            // Interlace Bottom field
            temp.select<1, 1, 4, 2>(0, 0) = WriteBackBuffer_F2.format<ushort, 16, 16>().select<1, 1, 4, 1>(2 * j + 8, 0);
            temp.select<1, 1, 4, 2>(0, 1) = temp1.select<1, 1, 4, 1>(0, 0);
            WriteBackBuffer_F2.format<ushort, 16, 16>().select<1, 1, 8, 1>(2 * j, 0) = temp;

            temp.select<1, 1, 4, 2>(0, 0) = WriteBackBuffer_F2.format<ushort, 16, 16>().select<1, 1, 4, 1>(2 * j + 8, 8);
            temp.select<1, 1, 4, 2>(0, 1) = temp1.select<1, 1, 4, 1>(0, 8);
            WriteBackBuffer_F2.format<ushort, 16, 16>().select<1, 1, 8, 1>(2 * j, 8) = temp;



            temp.select<1, 1, 4, 2>(0, 0) = WriteBackBuffer_F2.format<ushort, 16, 16>().select<1, 1, 4, 1>(2 * j + 9, 0);
            temp.select<1, 1, 4, 2>(0, 1) = temp1.select<1, 1, 4, 1>(1, 0);
            WriteBackBuffer_F2.format<ushort, 16, 16>().select<1, 1, 8, 1>(2 * j + 1, 0) = temp;

            temp.select<1, 1, 4, 2>(0, 0) = WriteBackBuffer_F2.format<ushort, 16, 16>().select<1, 1, 4, 1>(2 * j + 9, 8);
            temp.select<1, 1, 4, 2>(0, 1) = temp1.select<1, 1, 4, 1>(1, 8);
            WriteBackBuffer_F2.format<ushort, 16, 16>().select<1, 1, 8, 1>(2 * j + 1, 8) = temp;



            temp.select<1, 1, 4, 2>(0, 0) = WriteBackBuffer_F2.format<ushort, 16, 16>().select<1, 1, 4, 1>(2 * j + 8, 4);
            temp.select<1, 1, 4, 2>(0, 1) = temp1.select<1, 1, 4, 1>(0, 4);
            WriteBackBuffer_F2.format<ushort, 16, 16>().select<1, 1, 8, 1>(2 * j + 8, 0) = temp;

            temp.select<1, 1, 4, 2>(0, 0) = WriteBackBuffer_F2.format<ushort, 16, 16>().select<1, 1, 4, 1>(2 * j + 8, 12);
            temp.select<1, 1, 4, 2>(0, 1) = temp1.select<1, 1, 4, 1>(0, 12);
            WriteBackBuffer_F2.format<ushort, 16, 16>().select<1, 1, 8, 1>(2 * j + 8, 8) = temp;



            temp.select<1, 1, 4, 2>(0, 0) = WriteBackBuffer_F2.format<ushort, 16, 16>().select<1, 1, 4, 1>(2 * j + 9, 4);
            temp.select<1, 1, 4, 2>(0, 1) = temp1.select<1, 1, 4, 1>(1, 4);
            WriteBackBuffer_F2.format<ushort, 16, 16>().select<1, 1, 8, 1>(2 * j + 9, 0) = temp;

            temp.select<1, 1, 4, 2>(0, 0) = WriteBackBuffer_F2.format<ushort, 16, 16>().select<1, 1, 4, 1>(2 * j + 9, 12);
            temp.select<1, 1, 4, 2>(0, 1) = temp1.select<1, 1, 4, 1>(1, 12);
            WriteBackBuffer_F2.format<ushort, 16, 16>().select<1, 1, 8, 1>(2 * j + 9, 8) = temp;
        }
#endif
#ifdef OUTPUT_420
#pragma unroll
        for (short j = 0; j < 3; j++)
        {
            //temp1.format<ushort, 4, 8>().select<2, 1, 8, 1>(0, 0) = WriteBackBuffer_F1.format<ushort, 16, 16>().select<2, 1, 8, 1>(2 * j, 8);
            //temp1.format<ushort, 4, 8>().select<2, 1, 8, 1>(2, 0) = WriteBackBuffer_F1.format<ushort, 16, 16>().select<2, 1, 8, 1>(2 * j + 8, 8);

            /*
            // Reorder lines inside field #1
            temp = WriteBackBuffer_F1.format<ushort, 16, 16>().select<1, 1, 8, 1>(2 * j, 8);
            WriteBackBuffer_F1.format<ushort, 16, 16>().select<1, 1, 8, 1>(2 * j, 8) = WriteBackBuffer_F1.format<ushort, 16, 16>().select<1, 1, 8, 1>(2 * j + 1, 0);
            WriteBackBuffer_F1.format<ushort, 16, 16>().select<1, 1, 8, 1>(2 * j + 1, 0) = WriteBackBuffer_F1.format<ushort, 16, 16>().select<1, 1, 8, 1>(2 * j + 8, 0);
            WriteBackBuffer_F1.format<ushort, 16, 16>().select<1, 1, 8, 1>(2 * j + 8, 0) = temp;

            temp = WriteBackBuffer_F1.format<ushort, 16, 16>().select<1, 1, 8, 1>(2 * j + 1, 8);
            WriteBackBuffer_F1.format<ushort, 16, 16>().select<1, 1, 8, 1>(2 * j + 1, 8) = WriteBackBuffer_F1.format<ushort, 16, 16>().select<1, 1, 8, 1>(2 * j + 9, 0);
            WriteBackBuffer_F1.format<ushort, 16, 16>().select<1, 1, 8, 1>(2 * j + 9, 0) = WriteBackBuffer_F1.format<ushort, 16, 16>().select<1, 1, 8, 1>(2 * j + 8, 8);
            WriteBackBuffer_F1.format<ushort, 16, 16>().select<1, 1, 8, 1>(2 * j + 8, 8) = temp;

            // Reorder lines inside field #2
            temp = WriteBackBuffer_F2.format<ushort, 16, 16>().select<1, 1, 8, 1>(2 * j, 8);
            WriteBackBuffer_F2.format<ushort, 16, 16>().select<1, 1, 8, 1>(2 * j, 8) = WriteBackBuffer_F2.format<ushort, 16, 16>().select<1, 1, 8, 1>(2 * j + 1, 0);
            WriteBackBuffer_F2.format<ushort, 16, 16>().select<1, 1, 8, 1>(2 * j + 1, 0) = WriteBackBuffer_F2.format<ushort, 16, 16>().select<1, 1, 8, 1>(2 * j + 8, 0);
            WriteBackBuffer_F2.format<ushort, 16, 16>().select<1, 1, 8, 1>(2 * j + 8, 0) = temp;

            temp = WriteBackBuffer_F2.format<ushort, 16, 16>().select<1, 1, 8, 1>(2 * j + 1, 8);
            WriteBackBuffer_F2.format<ushort, 16, 16>().select<1, 1, 8, 1>(2 * j + 1, 8) = WriteBackBuffer_F2.format<ushort, 16, 16>().select<1, 1, 8, 1>(2 * j + 9, 0);
            WriteBackBuffer_F2.format<ushort, 16, 16>().select<1, 1, 8, 1>(2 * j + 9, 0) = WriteBackBuffer_F2.format<ushort, 16, 16>().select<1, 1, 8, 1>(2 * j + 8, 8);
            WriteBackBuffer_F2.format<ushort, 16, 16>().select<1, 1, 8, 1>(2 * j + 8, 8) = temp;
            */

            // Store temp data
            temp1 = WriteBackBuffer_F1.format<ushort, 16, 16>().select<2, 1, 16, 1>(2 * j + 8, 0);
            temp2.select<1, 1, 4, 1>(0, 0) = WriteBackBuffer_F1.format<ushort, 16, 16>().select<1, 1, 4, 1>(2 * j, 4);
            temp2.select<1, 1, 4, 1>(0, 4) = WriteBackBuffer_F1.format<ushort, 16, 16>().select<1, 1, 4, 1>(2 * j, 12);
            temp2.select<1, 1, 4, 1>(0, 8) = WriteBackBuffer_F1.format<ushort, 16, 16>().select<1, 1, 4, 1>(2 * j + 1, 4);
            temp2.select<1, 1, 4, 1>(0, 12) = WriteBackBuffer_F1.format<ushort, 16, 16>().select<1, 1, 4, 1>(2 * j + 1, 12);

            // Interlace Top field
            temp.select<1, 1, 4, 2>(0, 0) = WriteBackBuffer_F2.format<ushort, 16, 16>().select<1, 1, 4, 1>(2 * j, 0);
            temp.select<1, 1, 4, 2>(0, 1) = WriteBackBuffer_F1.format<ushort, 16, 16>().select<1, 1, 4, 1>(2 * j, 0);
            WriteBackBuffer_F1.format<ushort, 16, 16>().select<1, 1, 8, 1>(2 * j, 0) = temp;

            temp.select<1, 1, 4, 2>(0, 0) = WriteBackBuffer_F2.format<ushort, 16, 16>().select<1, 1, 4, 1>(2 * j, 8);
            temp.select<1, 1, 4, 2>(0, 1) = WriteBackBuffer_F1.format<ushort, 16, 16>().select<1, 1, 4, 1>(2 * j, 8);
            WriteBackBuffer_F1.format<ushort, 16, 16>().select<1, 1, 8, 1>(2 * j, 8) = temp;

            temp.select<1, 1, 4, 2>(0, 0) = WriteBackBuffer_F2.format<ushort, 16, 16>().select<1, 1, 4, 1>(2 * j + 1, 0);
            temp.select<1, 1, 4, 2>(0, 1) = WriteBackBuffer_F1.format<ushort, 16, 16>().select<1, 1, 4, 1>(2 * j + 1, 0);
            WriteBackBuffer_F1.format<ushort, 16, 16>().select<1, 1, 8, 1>(2 * j + 1, 0) = temp;

            temp.select<1, 1, 4, 2>(0, 0) = WriteBackBuffer_F2.format<ushort, 16, 16>().select<1, 1, 4, 1>(2 * j + 1, 8);
            temp.select<1, 1, 4, 2>(0, 1) = WriteBackBuffer_F1.format<ushort, 16, 16>().select<1, 1, 4, 1>(2 * j + 1, 8);
            WriteBackBuffer_F1.format<ushort, 16, 16>().select<1, 1, 8, 1>(2 * j + 1, 8) = temp;


            temp.select<1, 1, 4, 2>(0, 0) = WriteBackBuffer_F2.format<ushort, 16, 16>().select<1, 1, 4, 1>(2 * j, 4);
            temp.select<1, 1, 4, 2>(0, 1) = temp2.select<1, 1, 4, 1>(0, 0);
            WriteBackBuffer_F1.format<ushort, 16, 16>().select<1, 1, 8, 1>(2 * j + 8, 0) = temp;

            temp.select<1, 1, 4, 2>(0, 0) = WriteBackBuffer_F2.format<ushort, 16, 16>().select<1, 1, 4, 1>(2 * j, 12);
            temp.select<1, 1, 4, 2>(0, 1) = temp2.select<1, 1, 4, 1>(0, 4);
            WriteBackBuffer_F1.format<ushort, 16, 16>().select<1, 1, 8, 1>(2 * j + 8, 8) = temp;

            temp.select<1, 1, 4, 2>(0, 0) = WriteBackBuffer_F2.format<ushort, 16, 16>().select<1, 1, 4, 1>(2 * j + 1, 4);
            temp.select<1, 1, 4, 2>(0, 1) = temp2.select<1, 1, 4, 1>(0, 8);
            WriteBackBuffer_F1.format<ushort, 16, 16>().select<1, 1, 8, 1>(2 * j + 9, 0) = temp;

            temp.select<1, 1, 4, 2>(0, 0) = WriteBackBuffer_F2.format<ushort, 16, 16>().select<1, 1, 4, 1>(2 * j + 1, 12);
            temp.select<1, 1, 4, 2>(0, 1) = temp2.select<1, 1, 4, 1>(0, 12);
            WriteBackBuffer_F1.format<ushort, 16, 16>().select<1, 1, 8, 1>(2 * j + 9, 8) = temp;

            // Interlace Bottom field
            temp.select<1, 1, 4, 2>(0, 0) = WriteBackBuffer_F2.format<ushort, 16, 16>().select<1, 1, 4, 1>(2 * j + 8, 0);
            temp.select<1, 1, 4, 2>(0, 1) = temp1.select<1, 1, 4, 1>(0, 0);
            WriteBackBuffer_F2.format<ushort, 16, 16>().select<1, 1, 8, 1>(2 * j, 0) = temp;

            temp.select<1, 1, 4, 2>(0, 0) = WriteBackBuffer_F2.format<ushort, 16, 16>().select<1, 1, 4, 1>(2 * j + 8, 8);
            temp.select<1, 1, 4, 2>(0, 1) = temp1.select<1, 1, 4, 1>(0, 8);
            WriteBackBuffer_F2.format<ushort, 16, 16>().select<1, 1, 8, 1>(2 * j, 8) = temp;



            temp.select<1, 1, 4, 2>(0, 0) = WriteBackBuffer_F2.format<ushort, 16, 16>().select<1, 1, 4, 1>(2 * j + 9, 0);
            temp.select<1, 1, 4, 2>(0, 1) = temp1.select<1, 1, 4, 1>(1, 0);
            WriteBackBuffer_F2.format<ushort, 16, 16>().select<1, 1, 8, 1>(2 * j + 1, 0) = temp;

            temp.select<1, 1, 4, 2>(0, 0) = WriteBackBuffer_F2.format<ushort, 16, 16>().select<1, 1, 4, 1>(2 * j + 9, 8);
            temp.select<1, 1, 4, 2>(0, 1) = temp1.select<1, 1, 4, 1>(1, 8);
            WriteBackBuffer_F2.format<ushort, 16, 16>().select<1, 1, 8, 1>(2 * j + 1, 8) = temp;



            temp.select<1, 1, 4, 2>(0, 0) = WriteBackBuffer_F2.format<ushort, 16, 16>().select<1, 1, 4, 1>(2 * j + 8, 4);
            temp.select<1, 1, 4, 2>(0, 1) = temp1.select<1, 1, 4, 1>(0, 4);
            WriteBackBuffer_F2.format<ushort, 16, 16>().select<1, 1, 8, 1>(2 * j + 8, 0) = temp;

            temp.select<1, 1, 4, 2>(0, 0) = WriteBackBuffer_F2.format<ushort, 16, 16>().select<1, 1, 4, 1>(2 * j + 8, 12);
            temp.select<1, 1, 4, 2>(0, 1) = temp1.select<1, 1, 4, 1>(0, 12);
            WriteBackBuffer_F2.format<ushort, 16, 16>().select<1, 1, 8, 1>(2 * j + 8, 8) = temp;



            temp.select<1, 1, 4, 2>(0, 0) = WriteBackBuffer_F2.format<ushort, 16, 16>().select<1, 1, 4, 1>(2 * j + 9, 4);
            temp.select<1, 1, 4, 2>(0, 1) = temp1.select<1, 1, 4, 1>(1, 4);
            WriteBackBuffer_F2.format<ushort, 16, 16>().select<1, 1, 8, 1>(2 * j + 9, 0) = temp;

            temp.select<1, 1, 4, 2>(0, 0) = WriteBackBuffer_F2.format<ushort, 16, 16>().select<1, 1, 4, 1>(2 * j + 9, 12);
            temp.select<1, 1, 4, 2>(0, 1) = temp1.select<1, 1, 4, 1>(1, 12);
            WriteBackBuffer_F2.format<ushort, 16, 16>().select<1, 1, 8, 1>(2 * j + 9, 8) = temp;
    }
#endif
    }
    else
    {
        matrix<ushort, 1, 8> temp;
        matrix<ushort, 1, 16> temp1;

#ifdef OUTPUT_PA
#pragma unroll
        for (short j = 0; j < 4; j++)
        {
            // RGBA channel left half
            temp = WriteBackBuffer_F1.format<ushort, 16, 16>().select<1, 1, 8, 1>(2 * j, 8);
            WriteBackBuffer_F1.format<ushort, 16, 16>().select<1, 1, 8, 1>(2 * j, 8) = WriteBackBuffer_F2.format<ushort, 16, 16>().select<1, 1, 8, 1>(2 * j, 0);
            WriteBackBuffer_F2.format<ushort, 16, 16>().select<1, 1, 8, 1>(2 * j, 0) = WriteBackBuffer_F1.format<ushort, 16, 16>().select<1, 1, 8, 1>(2 * j + 1, 0);
            WriteBackBuffer_F1.format<ushort, 16, 16>().select<1, 1, 8, 1>(2 * j + 1, 0) = temp;

            temp = WriteBackBuffer_F1.format<ushort, 16, 16>().select<1, 1, 8, 1>(1 + 2 * j, 8);
            WriteBackBuffer_F1.format<ushort, 16, 16>().select<1, 1, 8, 1>(1 + 2 * j, 8) = WriteBackBuffer_F2.format<ushort, 16, 16>().select<1, 1, 8, 1>(2 * j, 8);
            WriteBackBuffer_F2.format<ushort, 16, 16>().select<1, 1, 8, 1>(2 * j, 8) = WriteBackBuffer_F2.format<ushort, 16, 16>().select<1, 1, 8, 1>(2 * j + 1, 0);
            WriteBackBuffer_F2.format<ushort, 16, 16>().select<1, 1, 8, 1>(2 * j + 1, 0) = temp;

            // RGBA channel right  half
            temp = WriteBackBuffer_F1.format<ushort, 16, 16>().select<1, 1, 8, 1>(2 * j + 8, 8);
            WriteBackBuffer_F1.format<ushort, 16, 16>().select<1, 1, 8, 1>(2 * j + 8, 8) = WriteBackBuffer_F2.format<ushort, 16, 16>().select<1, 1, 8, 1>(2 * j + 8, 0);
            WriteBackBuffer_F2.format<ushort, 16, 16>().select<1, 1, 8, 1>(2 * j + 8, 0) = WriteBackBuffer_F1.format<ushort, 16, 16>().select<1, 1, 8, 1>(2 * j + 9, 0);
            WriteBackBuffer_F1.format<ushort, 16, 16>().select<1, 1, 8, 1>(2 * j + 9, 0) = temp;

            temp = WriteBackBuffer_F1.format<ushort, 16, 16>().select<1, 1, 8, 1>(9 + 2 * j, 8);
            WriteBackBuffer_F1.format<ushort, 16, 16>().select<1, 1, 8, 1>(9 + 2 * j, 8) = WriteBackBuffer_F2.format<ushort, 16, 16>().select<1, 1, 8, 1>(2 * j + 8, 8);
            WriteBackBuffer_F2.format<ushort, 16, 16>().select<1, 1, 8, 1>(2 * j + 8, 8) = WriteBackBuffer_F2.format<ushort, 16, 16>().select<1, 1, 8, 1>(2 * j + 9, 0);
            WriteBackBuffer_F2.format<ushort, 16, 16>().select<1, 1, 8, 1>(2 * j + 9, 0) = temp;
        }
#endif
#ifdef OUTPUT_420
        // Y Channel
        {
            temp = WriteBackBuffer_F1.format<ushort, 16, 16>().select<1, 1, 8, 1>(2, 8);
            WriteBackBuffer_F1.format<ushort, 16, 16>().select<1, 1, 8, 1>(2, 8) = WriteBackBuffer_F2.format<ushort, 16, 16>().select<1, 1, 8, 1>(2, 0);
            WriteBackBuffer_F2.format<ushort, 16, 16>().select<1, 1, 8, 1>(2, 0) = WriteBackBuffer_F1.format<ushort, 16, 16>().select<1, 1, 8, 1>(2 + 1, 0);
            WriteBackBuffer_F1.format<ushort, 16, 16>().select<1, 1, 8, 1>(2 + 1, 0) = temp;

            temp = WriteBackBuffer_F1.format<ushort, 16, 16>().select<1, 1, 8, 1>(1 + 2, 8);
            WriteBackBuffer_F1.format<ushort, 16, 16>().select<1, 1, 8, 1>(1 + 2, 8) = WriteBackBuffer_F2.format<ushort, 16, 16>().select<1, 1, 8, 1>(2, 8);
            WriteBackBuffer_F2.format<ushort, 16, 16>().select<1, 1, 8, 1>(2, 8) = WriteBackBuffer_F2.format<ushort, 16, 16>().select<1, 1, 8, 1>(2 + 1, 0);
            WriteBackBuffer_F2.format<ushort, 16, 16>().select<1, 1, 8, 1>(2 + 1, 0) = temp;

            // RGBA channel right  half
            temp = WriteBackBuffer_F1.format<ushort, 16, 16>().select<1, 1, 8, 1>(2 + 8, 8);
            WriteBackBuffer_F1.format<ushort, 16, 16>().select<1, 1, 8, 1>(2 + 8, 8) = WriteBackBuffer_F2.format<ushort, 16, 16>().select<1, 1, 8, 1>(2 + 8, 0);
            WriteBackBuffer_F2.format<ushort, 16, 16>().select<1, 1, 8, 1>(2 + 8, 0) = WriteBackBuffer_F1.format<ushort, 16, 16>().select<1, 1, 8, 1>(2 + 9, 0);
            WriteBackBuffer_F1.format<ushort, 16, 16>().select<1, 1, 8, 1>(2 + 9, 0) = temp;

            temp = WriteBackBuffer_F1.format<ushort, 16, 16>().select<1, 1, 8, 1>(9 + 2, 8);
            WriteBackBuffer_F1.format<ushort, 16, 16>().select<1, 1, 8, 1>(9 + 2, 8) = WriteBackBuffer_F2.format<ushort, 16, 16>().select<1, 1, 8, 1>(2 + 8, 8);
            WriteBackBuffer_F2.format<ushort, 16, 16>().select<1, 1, 8, 1>(2 + 8, 8) = WriteBackBuffer_F2.format<ushort, 16, 16>().select<1, 1, 8, 1>(2 + 9, 0);
            WriteBackBuffer_F2.format<ushort, 16, 16>().select<1, 1, 8, 1>(2 + 9, 0) = temp;
        }

        // U Channel
        {
            temp1 = WriteBackBuffer_F1.format<ushort, 16, 16>().select<1, 1, 16, 1>(5, 0);
            WriteBackBuffer_F1.format<ushort, 16, 16>().select<1, 1, 16, 1>(5, 0) = WriteBackBuffer_F2.format<ushort, 16, 16>().select<1, 1, 16, 1>(4, 0);
            WriteBackBuffer_F2.format<ushort, 16, 16>().select<1, 1, 16, 1>(4, 0) = temp1;

            temp1 = WriteBackBuffer_F1.format<ushort, 16, 16>().select<1, 1, 16, 1>(5 + 8, 0);
            WriteBackBuffer_F1.format<ushort, 16, 16>().select<1, 1, 16, 1>(5 + 8, 0) = WriteBackBuffer_F2.format<ushort, 16, 16>().select<1, 1, 16, 1>(4 + 8, 0);
            WriteBackBuffer_F2.format<ushort, 16, 16>().select<1, 1, 16, 1>(4 + 8, 0) = temp1;
        }

        // V Channel
        {
            temp1 = WriteBackBuffer_F1.format<ushort, 16, 16>().select<1, 1, 16, 1>(1, 0);
            WriteBackBuffer_F1.format<ushort, 16, 16>().select<1, 1, 16, 1>(1, 0) = WriteBackBuffer_F2.format<ushort, 16, 16>().select<1, 1, 16, 1>(0, 0);
            WriteBackBuffer_F2.format<ushort, 16, 16>().select<1, 1, 16, 1>(0, 0) = temp1;

            temp1 = WriteBackBuffer_F1.format<ushort, 16, 16>().select<1, 1, 16, 1>(1 + 8, 0);
            WriteBackBuffer_F1.format<ushort, 16, 16>().select<1, 1, 16, 1>(1 + 8, 0) = WriteBackBuffer_F2.format<ushort, 16, 16>().select<1, 1, 16, 1>(0 + 8, 0);
            WriteBackBuffer_F2.format<ushort, 16, 16>().select<1, 1, 16, 1>(0 + 8, 0) = temp1;
        }
#endif
    }
#undef WriteBackBuffer_F1
#undef WriteBackBuffer_F2
}