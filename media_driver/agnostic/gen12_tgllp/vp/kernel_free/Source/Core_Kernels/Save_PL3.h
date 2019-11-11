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

    matrix_ref<uchar, 16, 16> Result_Y = DataBuffer.format<uchar, 192, 16>().select<16, 1, 16, 1>(128, 0);

    //matrix <uint, 3, 8> mesg1 = DataBuffer.format<uint, 96, 8>().select<3, 1, 8, 1>(64, 0);
    //matrix <uint, 3, 8> mesg2 = DataBuffer.format<uint, 96, 8>().select<3, 1, 8, 1>(68, 0);

    //mesg1.select<1, 1, 8, 1>(0, 0) = cm_get_r0<uint>();
    //mesg2.select<1, 1, 8, 1>(0, 0) = cm_get_r0<uint>();

    //matrix_ref<uint, 2, 8> Result_U = mesg1.select<2, 1, 8, 1>(1, 0);
    //matrix_ref<uint, 2, 8> Result_V = mesg2.select<2, 1, 8, 1>(1, 0);

    matrix_ref<uchar, 2, 32> Result_U = DataBuffer.format<uchar, 96, 32>().select<2, 1, 32, 1>(72, 0);
    matrix_ref<uchar, 2, 32> Result_V = DataBuffer.format<uchar, 96, 32>().select<2, 1, 32, 1>(76, 0);
    matrix_ref<ushort, 2, 16> Result_Temp_V = DataBuffer.format<ushort, 96, 16>().select<2, 1, 16, 1>(88, 0);
    matrix_ref<ushort, 2, 16> Result_Temp_U = DataBuffer.format<ushort, 96, 16>().select<2, 1, 16, 1>(90, 0);

    SurfaceIndex Dst_Surface_Y(MDF_FC_OUTPUT_BTI_START);
    SurfaceIndex Dst_Surface_U(MDF_FC_OUTPUT_BTI_START + MDF_FC_U_PLANE_BTI_OFFSET);
    SurfaceIndex Dst_Surface_V(MDF_FC_OUTPUT_BTI_START + MDF_FC_V_PLANE_BTI_OFFSET);

#pragma unroll
    // Rounding Y plane
    for (uchar i = 0; i < 4; i++)
    {
        DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(16 * i + 2, 0) = cm_add<ushort>(DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(16 * i + 2, 0), 0x80, SAT);
        DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(16 * i + 3, 0) = cm_add<ushort>(DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(16 * i + 3, 0), 0x80, SAT);
        DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(16 * i + 10, 0) = cm_add<ushort>(DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(16 * i + 10, 0), 0x80, SAT);
        DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(16 * i + 11, 0) = cm_add<ushort>(DataBuffer.format<ushort, 96, 16>().select<1, 1, 16, 1>(16 * i + 11, 0), 0x80, SAT);
    }

#pragma unroll
    // Write Y plane
    for (uchar i = 0; i < 4; i++)
    {
        Result_Y.select<1, 1, 8, 1>(4 * i, 0) = DataBuffer.format<uchar, 96, 32>().select<1, 1, 8, 2>(16 * i + 2, 1);
        Result_Y.select<1, 1, 8, 1>(4 * i, 8) = DataBuffer.format<uchar, 96, 32>().select<1, 1, 8, 2>(16 * i + 10, 1);
        Result_Y.select<1, 1, 8, 1>(4 * i + 1, 0) = DataBuffer.format<uchar, 96, 32>().select<1, 1, 8, 2>(16 * i + 2, 17);
        Result_Y.select<1, 1, 8, 1>(4 * i + 1, 8) = DataBuffer.format<uchar, 96, 32>().select<1, 1, 8, 2>(16 * i + 10, 17);
        Result_Y.select<1, 1, 8, 1>(4 * i + 2, 0) = DataBuffer.format<uchar, 96, 32>().select<1, 1, 8, 2>(16 * i + 3, 1);
        Result_Y.select<1, 1, 8, 1>(4 * i + 2, 8) = DataBuffer.format<uchar, 96, 32>().select<1, 1, 8, 2>(16 * i + 11, 1);
        Result_Y.select<1, 1, 8, 1>(4 * i + 3, 0) = DataBuffer.format<uchar, 96, 32>().select<1, 1, 8, 2>(16 * i + 3, 17);
        Result_Y.select<1, 1, 8, 1>(4 * i + 3, 8) = DataBuffer.format<uchar, 96, 32>().select<1, 1, 8, 2>(16 * i + 11, 17);
    }

    write(Dst_Surface_Y, DstX, DstY, Result_Y);

#pragma unroll
    for (uchar i = 0; i < 4; i++)
    {
        // Write UV plane
        switch (RotationChromaSitingFlag & 0x07000000)
        {
        case CHROMA_SUBSAMPLING_TOP_CENTER:
            Result_Temp_V.select<1, 1, 16, 1>(0, 0).format<ushort, 2, 8>().select<1, 1, 4, 1>(0, 0) = cm_avg<ushort>(DataBuffer.format<ushort, 96, 16>().select<1, 1, 4, 2>(16 * i, 0), DataBuffer.format<ushort, 96, 16>().select<1, 1, 4, 2>(16 * i, 1));
            Result_Temp_V.select<1, 1, 16, 1>(0, 0).format<ushort, 2, 8>().select<1, 1, 4, 1>(0, 4) = cm_avg<ushort>(DataBuffer.format<ushort, 96, 16>().select<1, 1, 4, 2>(16 * i + 8, 0), DataBuffer.format<ushort, 96, 16>().select<1, 1, 4, 2>(16 * i + 8, 1));
            Result_Temp_V.select<1, 1, 16, 1>(1, 0).format<ushort, 2, 8>().select<1, 1, 4, 1>(0, 0) = cm_avg<ushort>(DataBuffer.format<ushort, 96, 16>().select<1, 1, 4, 2>(16 * i + 1, 0), DataBuffer.format<ushort, 96, 16>().select<1, 1, 4, 2>(16 * i + 1, 1));
            Result_Temp_V.select<1, 1, 16, 1>(1, 0).format<ushort, 2, 8>().select<1, 1, 4, 1>(0, 4) = cm_avg<ushort>(DataBuffer.format<ushort, 96, 16>().select<1, 1, 4, 2>(16 * i + 9, 0), DataBuffer.format<ushort, 96, 16>().select<1, 1, 4, 2>(16 * i + 9, 1));

            Result_Temp_U.select<1, 1, 16, 1>(0, 0).format<ushort, 2, 8>().select<1, 1, 4, 1>(0, 0) = cm_avg<ushort>(DataBuffer.format<ushort, 96, 16>().select<1, 1, 4, 2>(16 * i + 4, 0), DataBuffer.format<ushort, 96, 16>().select<1, 1, 4, 2>(16 * i + 4, 1));
            Result_Temp_U.select<1, 1, 16, 1>(0, 0).format<ushort, 2, 8>().select<1, 1, 4, 1>(0, 4) = cm_avg<ushort>(DataBuffer.format<ushort, 96, 16>().select<1, 1, 4, 2>(16 * i + 12, 0), DataBuffer.format<ushort, 96, 16>().select<1, 1, 4, 2>(16 * i + 12, 1));
            Result_Temp_U.select<1, 1, 16, 1>(1, 0).format<ushort, 2, 8>().select<1, 1, 4, 1>(0, 0) = cm_avg<ushort>(DataBuffer.format<ushort, 96, 16>().select<1, 1, 4, 2>(16 * i + 5, 0), DataBuffer.format<ushort, 96, 16>().select<1, 1, 4, 2>(16 * i + 5, 1));
            Result_Temp_U.select<1, 1, 16, 1>(1, 0).format<ushort, 2, 8>().select<1, 1, 4, 1>(0, 4) = cm_avg<ushort>(DataBuffer.format<ushort, 96, 16>().select<1, 1, 4, 2>(16 * i + 13, 0), DataBuffer.format<ushort, 96, 16>().select<1, 1, 4, 2>(16 * i + 13, 1));

            Result_Temp_U = cm_add<ushort>(Result_Temp_U, 0x80, SAT);
            Result_Temp_V = cm_add<ushort>(Result_Temp_V, 0x80, SAT);

            Result_U.format<uchar, 8, 8>().select<2, 1, 8, 1>(2 * i, 0) = Result_Temp_U.format<uchar, 4, 16>().select<2, 2, 8, 2>(0, 1);
            Result_V.format<uchar, 8, 8>().select<2, 1, 8, 1>(2 * i, 0) = Result_Temp_V.format<uchar, 4, 16>().select<2, 2, 8, 2>(0, 1);
            break;
        case CHROMA_SUBSAMPLING_CENTER_CENTER:
            Result_Temp_V.select<1, 1, 16, 1>(0, 0).format<ushort, 2, 8>().select<1, 1, 4, 1>(0, 0) = cm_avg<ushort>(DataBuffer.format<ushort, 96, 16>().select<1, 1, 4, 2>(16 * i, 0), DataBuffer.format<ushort, 96, 16>().select<1, 1, 4, 2>(16 * i, 1));
            Result_Temp_V.select<1, 1, 16, 1>(0, 0).format<ushort, 2, 8>().select<1, 1, 4, 1>(0, 4) = cm_avg<ushort>(DataBuffer.format<ushort, 96, 16>().select<1, 1, 4, 2>(16 * i + 8, 0), DataBuffer.format<ushort, 96, 16>().select<1, 1, 4, 2>(16 * i + 8, 1));
            Result_Temp_V.select<1, 1, 16, 1>(0, 0).format<ushort, 2, 8>().select<1, 1, 4, 1>(1, 0) = cm_avg<ushort>(DataBuffer.format<ushort, 96, 16>().select<1, 1, 4, 2>(16 * i, 8), DataBuffer.format<ushort, 96, 16>().select<1, 1, 4, 2>(16 * i, 9));
            Result_Temp_V.select<1, 1, 16, 1>(0, 0).format<ushort, 2, 8>().select<1, 1, 4, 1>(1, 4) = cm_avg<ushort>(DataBuffer.format<ushort, 96, 16>().select<1, 1, 4, 2>(16 * i + 8, 8), DataBuffer.format<ushort, 96, 16>().select<1, 1, 4, 2>(16 * i + 8, 9));
            Result_Temp_V.select<1, 1, 16, 1>(1, 0).format<ushort, 2, 8>().select<1, 1, 4, 1>(0, 0) = cm_avg<ushort>(DataBuffer.format<ushort, 96, 16>().select<1, 1, 4, 2>(16 * i + 1, 0), DataBuffer.format<ushort, 96, 16>().select<1, 1, 4, 2>(16 * i + 1, 1));
            Result_Temp_V.select<1, 1, 16, 1>(1, 0).format<ushort, 2, 8>().select<1, 1, 4, 1>(0, 4) = cm_avg<ushort>(DataBuffer.format<ushort, 96, 16>().select<1, 1, 4, 2>(16 * i + 9, 0), DataBuffer.format<ushort, 96, 16>().select<1, 1, 4, 2>(16 * i + 9, 1));
            Result_Temp_V.select<1, 1, 16, 1>(1, 0).format<ushort, 2, 8>().select<1, 1, 4, 1>(1, 0) = cm_avg<ushort>(DataBuffer.format<ushort, 96, 16>().select<1, 1, 4, 2>(16 * i + 1, 8), DataBuffer.format<ushort, 96, 16>().select<1, 1, 4, 2>(16 * i + 1, 9));
            Result_Temp_V.select<1, 1, 16, 1>(1, 0).format<ushort, 2, 8>().select<1, 1, 4, 1>(1, 4) = cm_avg<ushort>(DataBuffer.format<ushort, 96, 16>().select<1, 1, 4, 2>(16 * i + 9, 8), DataBuffer.format<ushort, 96, 16>().select<1, 1, 4, 2>(16 * i + 9, 9));

            Result_Temp_U.select<1, 1, 16, 1>(0, 0).format<ushort, 2, 8>().select<1, 1, 4, 1>(0, 0) = cm_avg<ushort>(DataBuffer.format<ushort, 96, 16>().select<1, 1, 4, 2>(16 * i + 4, 0), DataBuffer.format<ushort, 96, 16>().select<1, 1, 4, 2>(16 * i + 4, 1));
            Result_Temp_U.select<1, 1, 16, 1>(0, 0).format<ushort, 2, 8>().select<1, 1, 4, 1>(0, 4) = cm_avg<ushort>(DataBuffer.format<ushort, 96, 16>().select<1, 1, 4, 2>(16 * i + 12, 0), DataBuffer.format<ushort, 96, 16>().select<1, 1, 4, 2>(16 * i + 12, 1));
            Result_Temp_U.select<1, 1, 16, 1>(0, 0).format<ushort, 2, 8>().select<1, 1, 4, 1>(1, 0) = cm_avg<ushort>(DataBuffer.format<ushort, 96, 16>().select<1, 1, 4, 2>(16 * i + 4, 8), DataBuffer.format<ushort, 96, 16>().select<1, 1, 4, 2>(16 * i + 4, 9));
            Result_Temp_U.select<1, 1, 16, 1>(0, 0).format<ushort, 2, 8>().select<1, 1, 4, 1>(1, 4) = cm_avg<ushort>(DataBuffer.format<ushort, 96, 16>().select<1, 1, 4, 2>(16 * i + 12, 8), DataBuffer.format<ushort, 96, 16>().select<1, 1, 4, 2>(16 * i + 12, 9));
            Result_Temp_U.select<1, 1, 16, 1>(1, 0).format<ushort, 2, 8>().select<1, 1, 4, 1>(0, 0) = cm_avg<ushort>(DataBuffer.format<ushort, 96, 16>().select<1, 1, 4, 2>(16 * i + 5, 0), DataBuffer.format<ushort, 96, 16>().select<1, 1, 4, 2>(16 * i + 5, 1));
            Result_Temp_U.select<1, 1, 16, 1>(1, 0).format<ushort, 2, 8>().select<1, 1, 4, 1>(0, 4) = cm_avg<ushort>(DataBuffer.format<ushort, 96, 16>().select<1, 1, 4, 2>(16 * i + 13, 0), DataBuffer.format<ushort, 96, 16>().select<1, 1, 4, 2>(16 * i + 13, 1));
            Result_Temp_U.select<1, 1, 16, 1>(1, 0).format<ushort, 2, 8>().select<1, 1, 4, 1>(1, 0) = cm_avg<ushort>(DataBuffer.format<ushort, 96, 16>().select<1, 1, 4, 2>(16 * i + 5, 8), DataBuffer.format<ushort, 96, 16>().select<1, 1, 4, 2>(16 * i + 5, 9));
            Result_Temp_U.select<1, 1, 16, 1>(1, 0).format<ushort, 2, 8>().select<1, 1, 4, 1>(1, 4) = cm_avg<ushort>(DataBuffer.format<ushort, 96, 16>().select<1, 1, 4, 2>(16 * i + 13, 8), DataBuffer.format<ushort, 96, 16>().select<1, 1, 4, 2>(16 * i + 13, 9));

            Result_Temp_V.format<ushort, 4, 8>().select<2, 2, 8, 1>(0, 0) = cm_avg<ushort>(Result_Temp_V.format<ushort, 4, 8>().select<2, 2, 8, 1>(0, 0), Result_Temp_V.format<ushort, 4, 8>().select<2, 2, 8, 1>(1, 0));
            Result_Temp_U.format<ushort, 4, 8>().select<2, 2, 8, 1>(0, 0) = cm_avg<ushort>(Result_Temp_U.format<ushort, 4, 8>().select<2, 2, 8, 1>(0, 0), Result_Temp_U.format<ushort, 4, 8>().select<2, 2, 8, 1>(1, 0));

            Result_Temp_U = cm_add<ushort>(Result_Temp_U, 0x80, SAT);
            Result_Temp_V = cm_add<ushort>(Result_Temp_V, 0x80, SAT);

            Result_U.format<uchar, 8, 8>().select<2, 1, 8, 1>(2 * i, 0) = Result_Temp_U.format<uchar, 4, 16>().select<2, 2, 8, 2>(0, 1);
            Result_V.format<uchar, 8, 8>().select<2, 1, 8, 1>(2 * i, 0) = Result_Temp_V.format<uchar, 4, 16>().select<2, 2, 8, 2>(0, 1);

            break;
        case CHROMA_SUBSAMPLING_BOTTOM_CENTER:
            Result_Temp_V.select<1, 1, 16, 1>(0, 0).format<ushort, 2, 8>().select<1, 1, 4, 1>(1, 0) = cm_avg<ushort>(DataBuffer.format<ushort, 96, 16>().select<1, 1, 4, 2>(16 * i, 8), DataBuffer.format<ushort, 96, 16>().select<1, 1, 4, 2>(16 * i, 9));
            Result_Temp_V.select<1, 1, 16, 1>(0, 0).format<ushort, 2, 8>().select<1, 1, 4, 1>(1, 4) = cm_avg<ushort>(DataBuffer.format<ushort, 96, 16>().select<1, 1, 4, 2>(16 * i + 8, 8), DataBuffer.format<ushort, 96, 16>().select<1, 1, 4, 2>(16 * i + 8, 9));
            Result_Temp_V.select<1, 1, 16, 1>(1, 0).format<ushort, 2, 8>().select<1, 1, 4, 1>(1, 0) = cm_avg<ushort>(DataBuffer.format<ushort, 96, 16>().select<1, 1, 4, 2>(16 * i + 1, 8), DataBuffer.format<ushort, 96, 16>().select<1, 1, 4, 2>(16 * i + 1, 9));
            Result_Temp_V.select<1, 1, 16, 1>(1, 0).format<ushort, 2, 8>().select<1, 1, 4, 1>(1, 4) = cm_avg<ushort>(DataBuffer.format<ushort, 96, 16>().select<1, 1, 4, 2>(16 * i + 9, 8), DataBuffer.format<ushort, 96, 16>().select<1, 1, 4, 2>(16 * i + 9, 9));

            Result_Temp_U.select<1, 1, 16, 1>(0, 0).format<ushort, 2, 8>().select<1, 1, 4, 1>(1, 0) = cm_avg<ushort>(DataBuffer.format<ushort, 96, 16>().select<1, 1, 4, 2>(16 * i + 4, 8), DataBuffer.format<ushort, 96, 16>().select<1, 1, 4, 2>(16 * i + 4, 9));
            Result_Temp_U.select<1, 1, 16, 1>(0, 0).format<ushort, 2, 8>().select<1, 1, 4, 1>(1, 4) = cm_avg<ushort>(DataBuffer.format<ushort, 96, 16>().select<1, 1, 4, 2>(16 * i + 12, 8), DataBuffer.format<ushort, 96, 16>().select<1, 1, 4, 2>(16 * i + 12, 9));
            Result_Temp_U.select<1, 1, 16, 1>(1, 0).format<ushort, 2, 8>().select<1, 1, 4, 1>(1, 0) = cm_avg<ushort>(DataBuffer.format<ushort, 96, 16>().select<1, 1, 4, 2>(16 * i + 5, 8), DataBuffer.format<ushort, 96, 16>().select<1, 1, 4, 2>(16 * i + 5, 9));
            Result_Temp_U.select<1, 1, 16, 1>(1, 0).format<ushort, 2, 8>().select<1, 1, 4, 1>(1, 4) = cm_avg<ushort>(DataBuffer.format<ushort, 96, 16>().select<1, 1, 4, 2>(16 * i + 13, 8), DataBuffer.format<ushort, 96, 16>().select<1, 1, 4, 2>(16 * i + 13, 9));

            Result_Temp_U = cm_add<ushort>(Result_Temp_U, 0x80, SAT);
            Result_Temp_V = cm_add<ushort>(Result_Temp_V, 0x80, SAT);

            Result_U.format<uchar, 8, 8>().select<2, 1, 8, 1>(2 * i, 0) = Result_Temp_U.format<uchar, 4, 16>().select<2, 2, 8, 2>(1, 1);
            Result_V.format<uchar, 8, 8>().select<2, 1, 8, 1>(2 * i, 0) = Result_Temp_V.format<uchar, 4, 16>().select<2, 2, 8, 2>(1, 1);
            break;
        case CHROMA_SUBSAMPLING_TOP_LEFT:
            DataBuffer.format<ushort, 96, 16>().select<2, 1, 16, 1>(16 * i, 0) = cm_add<ushort>(DataBuffer.format<ushort, 96, 16>().select<2, 1, 16, 1>(16 * i, 0), 0x80, SAT);
            DataBuffer.format<ushort, 96, 16>().select<2, 1, 16, 1>(16 * i + 8, 0) = cm_add<ushort>(DataBuffer.format<ushort, 96, 16>().select<2, 1, 16, 1>(16 * i + 8, 0), 0x80, SAT);

            DataBuffer.format<ushort, 96, 16>().select<2, 1, 16, 1>(16 * i + 4, 0) = cm_add<ushort>(DataBuffer.format<ushort, 96, 16>().select<2, 1, 16, 1>(16 * i + 4, 0), 0x80, SAT);
            DataBuffer.format<ushort, 96, 16>().select<2, 1, 16, 1>(16 * i + 12, 0) = cm_add<ushort>(DataBuffer.format<ushort, 96, 16>().select<2, 1, 16, 1>(16 * i + 12, 0), 0x80, SAT);

            Result_V.format<uchar, 8, 8>().select<2, 1, 4, 1>(2 * i, 0) = DataBuffer.format<uchar, 96, 32>().select<2, 1, 4, 4>(16 * i, 1);
            Result_V.format<uchar, 8, 8>().select<2, 1, 4, 1>(2 * i, 4) = DataBuffer.format<uchar, 96, 32>().select<2, 1, 4, 4>(16 * i + 8, 1);

            Result_U.format<uchar, 8, 8>().select<2, 1, 4, 1>(2 * i, 0) = DataBuffer.format<uchar, 96, 32>().select<2, 1, 4, 4>(16 * i + 4, 1);
            Result_U.format<uchar, 8, 8>().select<2, 1, 4, 1>(2 * i, 4) = DataBuffer.format<uchar, 96, 32>().select<2, 1, 4, 4>(16 * i + 12, 1);
            break;
        case CHROMA_SUBSAMPLING_CENTER_LEFT:
            Result_Temp_V.select<1, 1, 8, 1>(0, 0) = cm_avg<ushort>(DataBuffer.format<ushort, 96, 16>().select<1, 1, 8, 1>(16 * i, 0), DataBuffer.format<ushort, 96, 16>().select<1, 1, 8, 1>(16 * i, 8));
            Result_Temp_V.select<1, 1, 8, 1>(0, 8) = cm_avg<ushort>(DataBuffer.format<ushort, 96, 16>().select<1, 1, 8, 1>(16 * i + 8, 0), DataBuffer.format<ushort, 96, 16>().select<1, 1, 8, 1>(16 * i + 8, 8));
            Result_Temp_V.select<1, 1, 8, 1>(1, 0) = cm_avg<ushort>(DataBuffer.format<ushort, 96, 16>().select<1, 1, 8, 1>(16 * i + 1, 0), DataBuffer.format<ushort, 96, 16>().select<1, 1, 8, 1>(16 * i + 1, 8));
            Result_Temp_V.select<1, 1, 8, 1>(1, 8) = cm_avg<ushort>(DataBuffer.format<ushort, 96, 16>().select<1, 1, 8, 1>(16 * i + 9, 0), DataBuffer.format<ushort, 96, 16>().select<1, 1, 8, 1>(16 * i + 9, 8));

            Result_Temp_U.select<1, 1, 8, 1>(0, 0) = cm_avg<ushort>(DataBuffer.format<ushort, 96, 16>().select<1, 1, 8, 1>(16 * i + 4, 0), DataBuffer.format<ushort, 96, 16>().select<1, 1, 8, 1>(16 * i + 4, 8));
            Result_Temp_U.select<1, 1, 8, 1>(0, 8) = cm_avg<ushort>(DataBuffer.format<ushort, 96, 16>().select<1, 1, 8, 1>(16 * i + 12, 0), DataBuffer.format<ushort, 96, 16>().select<1, 1, 8, 1>(16 * i + 12, 8));
            Result_Temp_U.select<1, 1, 8, 1>(1, 0) = cm_avg<ushort>(DataBuffer.format<ushort, 96, 16>().select<1, 1, 8, 1>(16 * i + 5, 0), DataBuffer.format<ushort, 96, 16>().select<1, 1, 8, 1>(16 * i + 5, 8));
            Result_Temp_U.select<1, 1, 8, 1>(1, 8) = cm_avg<ushort>(DataBuffer.format<ushort, 96, 16>().select<1, 1, 8, 1>(16 * i + 13, 0), DataBuffer.format<ushort, 96, 16>().select<1, 1, 8, 1>(16 * i + 13, 8));

            Result_Temp_U = cm_add<ushort>(Result_Temp_U, 0x80, SAT);
            Result_Temp_V = cm_add<ushort>(Result_Temp_V, 0x80, SAT);

            Result_U.format<uchar, 8, 8>().select<2, 1, 8, 1>(2 * i, 0) = Result_Temp_U.format<uchar, 2, 32>().select<2, 1, 8, 4>(0, 1);
            Result_V.format<uchar, 8, 8>().select<2, 1, 8, 1>(2 * i, 0) = Result_Temp_V.format<uchar, 2, 32>().select<2, 1, 8, 4>(0, 1);

            break;
        case CHROMA_SUBSAMPLING_BOTTOM_LEFT:
            DataBuffer.format<ushort, 96, 16>().select<2, 1, 16, 1>(16 * i, 0) = cm_add<ushort>(DataBuffer.format<ushort, 96, 16>().select<2, 1, 16, 1>(16 * i, 0), 0x80, SAT);
            DataBuffer.format<ushort, 96, 16>().select<2, 1, 16, 1>(16 * i + 8, 0) = cm_add<ushort>(DataBuffer.format<ushort, 96, 16>().select<2, 1, 16, 1>(16 * i + 8, 0), 0x80, SAT);

            DataBuffer.format<ushort, 96, 16>().select<2, 1, 16, 1>(16 * i + 4, 0) = cm_add<ushort>(DataBuffer.format<ushort, 96, 16>().select<2, 1, 16, 1>(16 * i + 4, 0), 0x80, SAT);
            DataBuffer.format<ushort, 96, 16>().select<2, 1, 16, 1>(16 * i + 12, 0) = cm_add<ushort>(DataBuffer.format<ushort, 96, 16>().select<2, 1, 16, 1>(16 * i + 12, 0), 0x80, SAT);

            Result_V.format<uchar, 8, 8>().select<2, 1, 4, 1>(2 * i, 0) = DataBuffer.format<uchar, 96, 32>().select<2, 1, 4, 4>(16 * i, 17);
            Result_V.format<uchar, 8, 8>().select<2, 1, 4, 1>(2 * i, 4) = DataBuffer.format<uchar, 96, 32>().select<2, 1, 4, 4>(16 * i + 8, 17);

            Result_U.format<uchar, 8, 8>().select<2, 1, 4, 1>(2 * i, 0) = DataBuffer.format<uchar, 96, 32>().select<2, 1, 4, 4>(16 * i + 4, 17);
            Result_U.format<uchar, 8, 8>().select<2, 1, 4, 1>(2 * i, 4) = DataBuffer.format<uchar, 96, 32>().select<2, 1, 4, 4>(16 * i + 12, 17);
            break;
        default:
            Result_Temp_V.select<1, 1, 8, 1>(0, 0) = cm_avg<ushort>(DataBuffer.format<ushort, 96, 16>().select<1, 1, 8, 2>(16 * i, 0), DataBuffer.format<ushort, 96, 16>().select<1, 1, 8, 2>(16 * i, 1));
            Result_Temp_V.select<1, 1, 8, 1>(0, 8) = cm_avg<ushort>(DataBuffer.format<ushort, 96, 16>().select<1, 1, 8, 2>(16 * i + 8, 0), DataBuffer.format<ushort, 96, 16>().select<1, 1, 8, 2>(16 * i + 8, 1));
            Result_Temp_V.select<1, 1, 8, 1>(1, 0) = cm_avg<ushort>(DataBuffer.format<ushort, 96, 16>().select<1, 1, 8, 2>(16 * i + 1, 0), DataBuffer.format<ushort, 96, 16>().select<1, 1, 8, 2>(16 * i + 1, 1));
            Result_Temp_V.select<1, 1, 8, 1>(1, 8) = cm_avg<ushort>(DataBuffer.format<ushort, 96, 16>().select<1, 1, 8, 2>(16 * i + 9, 0), DataBuffer.format<ushort, 96, 16>().select<1, 1, 8, 2>(16 * i + 9, 1));

            Result_Temp_U.select<1, 1, 8, 1>(0, 0) = cm_avg<ushort>(DataBuffer.format<ushort, 96, 16>().select<1, 1, 8, 2>(16 * i + 4, 0), DataBuffer.format<ushort, 96, 16>().select<1, 1, 8, 2>(16 * i + 4, 1));
            Result_Temp_U.select<1, 1, 8, 1>(0, 8) = cm_avg<ushort>(DataBuffer.format<ushort, 96, 16>().select<1, 1, 8, 2>(16 * i + 12, 0), DataBuffer.format<ushort, 96, 16>().select<1, 1, 8, 2>(16 * i + 12, 1));
            Result_Temp_U.select<1, 1, 8, 1>(1, 0) = cm_avg<ushort>(DataBuffer.format<ushort, 96, 16>().select<1, 1, 8, 2>(16 * i + 5, 0), DataBuffer.format<ushort, 96, 16>().select<1, 1, 8, 2>(16 * i + 5, 1));
            Result_Temp_U.select<1, 1, 8, 1>(1, 8) = cm_avg<ushort>(DataBuffer.format<ushort, 96, 16>().select<1, 1, 8, 2>(16 * i + 13, 0), DataBuffer.format<ushort, 96, 16>().select<1, 1, 8, 2>(16 * i + 13, 1));

            Result_Temp_U = cm_add<ushort>(Result_Temp_U, 0x80, SAT);
            Result_Temp_V = cm_add<ushort>(Result_Temp_V, 0x80, SAT);

            Result_U.format<uchar, 8, 8>().select<2, 1, 8, 1>(2 * i, 0) = Result_Temp_U.format<uchar, 2, 32>().select<2, 1, 8, 4>(0, 1);
            Result_V.format<uchar, 8, 8>().select<2, 1, 8, 1>(2 * i, 0) = Result_Temp_V.format<uchar, 2, 32>().select<2, 1, 8, 4>(0, 1);
            break;
        }
    }

    write(Dst_Surface_U, DstX >> 1, DstY >> 1, Result_U.format<uchar, 8, 8>());
    write(Dst_Surface_V, DstX >> 1, DstY >> 1, Result_V.format<uchar, 8, 8>());
}