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

    matrix_ref<ushort, 8, 16> Result_Y = DataBuffer.format<ushort, 96, 16>().select<8, 1, 16, 1>(64, 0);
    matrix_ref<ushort, 8, 16> Result_UV = DataBuffer.format<ushort, 96, 16>().select<8, 1, 16, 1>(72, 0);
    matrix_ref<ushort, 2, 16> Result_Temp_V = DataBuffer.format<ushort, 96, 16>().select<2, 1, 16, 1>(88, 0);
    matrix_ref<ushort, 2, 16> Result_Temp_U = DataBuffer.format<ushort, 96, 16>().select<2, 1, 16, 1>(90, 0);

    SurfaceIndex Dst_Surface_Y(MDF_FC_OUTPUT_BTI_START);
    SurfaceIndex Dst_Surface_UV(MDF_FC_OUTPUT_BTI_START + MDF_FC_UV_PLANE_BTI_OFFSET);

#pragma unroll
    // Write Y plane
    for (uchar i = 0; i < 2; i++)
    {
        Result_Y.select<1, 1, 8, 1>(4 * i, 0) = DataBuffer.format<ushort, 96, 16>().select<1, 1, 8, 1>(16 * i + 2, 0);
        Result_Y.select<1, 1, 8, 1>(4 * i, 8) = DataBuffer.format<ushort, 96, 16>().select<1, 1, 8, 1>(16 * i + 10, 0);

        Result_Y.select<1, 1, 8, 1>(4 * i + 1, 0) = DataBuffer.format<ushort, 96, 16>().select<1, 1, 8, 1>(16 * i + 2, 8);
        Result_Y.select<1, 1, 8, 1>(4 * i + 1, 8) = DataBuffer.format<ushort, 96, 16>().select<1, 1, 8, 1>(16 * i + 10, 8);

        Result_Y.select<1, 1, 8, 1>(4 * i + 2, 0) = DataBuffer.format<ushort, 96, 16>().select<1, 1, 8, 1>(16 * i + 3, 0);
        Result_Y.select<1, 1, 8, 1>(4 * i + 2, 8) = DataBuffer.format<ushort, 96, 16>().select<1, 1, 8, 1>(16 * i + 11, 0);

        Result_Y.select<1, 1, 8, 1>(4 * i + 3, 0) = DataBuffer.format<ushort, 96, 16>().select<1, 1, 8, 1>(16 * i + 3, 8);
        Result_Y.select<1, 1, 8, 1>(4 * i + 3, 8) = DataBuffer.format<ushort, 96, 16>().select<1, 1, 8, 1>(16 * i + 11, 8);
    }

    write(Dst_Surface_Y, 2 * DstX, DstY, Result_Y);

#pragma unroll
    // Write Y plane
    for (uchar i = 0; i < 2; i++)
    {
        Result_Y.select<1, 1, 8, 1>(4 * i, 0) = DataBuffer.format<ushort, 96, 16>().select<1, 1, 8, 1>(16 * i + 2 + 32, 0);
        Result_Y.select<1, 1, 8, 1>(4 * i, 8) = DataBuffer.format<ushort, 96, 16>().select<1, 1, 8, 1>(16 * i + 10 + 32, 0);

        Result_Y.select<1, 1, 8, 1>(4 * i + 1, 0) = DataBuffer.format<ushort, 96, 16>().select<1, 1, 8, 1>(16 * i + 2 + 32, 8);
        Result_Y.select<1, 1, 8, 1>(4 * i + 1, 8) = DataBuffer.format<ushort, 96, 16>().select<1, 1, 8, 1>(16 * i + 10 + 32, 8);

        Result_Y.select<1, 1, 8, 1>(4 * i + 2, 0) = DataBuffer.format<ushort, 96, 16>().select<1, 1, 8, 1>(16 * i + 3 + 32, 0);
        Result_Y.select<1, 1, 8, 1>(4 * i + 2, 8) = DataBuffer.format<ushort, 96, 16>().select<1, 1, 8, 1>(16 * i + 11 + 32, 0);

        Result_Y.select<1, 1, 8, 1>(4 * i + 3, 0) = DataBuffer.format<ushort, 96, 16>().select<1, 1, 8, 1>(16 * i + 3 + 32, 8);
        Result_Y.select<1, 1, 8, 1>(4 * i + 3, 8) = DataBuffer.format<ushort, 96, 16>().select<1, 1, 8, 1>(16 * i + 11 + 32, 8);
    }

    write(Dst_Surface_Y, 2 * DstX, DstY + 8, Result_Y);

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

            Result_UV.select<2, 1, 8, 2>(2 * i, 0) = Result_Temp_U.select<2, 1, 8, 1>(0, 0);
            Result_UV.select<2, 1, 8, 2>(2 * i, 1) = Result_Temp_V.select<2, 1, 8, 1>(0, 0);
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

            Result_UV.select<2, 1, 8, 2>(2 * i, 0) = Result_Temp_U.select<2, 1, 8, 1>(0, 0);
            Result_UV.select<2, 1, 8, 2>(2 * i, 1) = Result_Temp_V.select<2, 1, 8, 1>(0, 0);
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

            Result_UV.select<2, 1, 8, 2>(2 * i, 0) = Result_Temp_U.select<2, 1, 8, 1>(0, 8);
            Result_UV.select<2, 1, 8, 2>(2 * i, 1) = Result_Temp_V.select<2, 1, 8, 1>(0, 8);
            break;
        case CHROMA_SUBSAMPLING_TOP_LEFT:
            Result_UV.select<2, 1, 4, 2>(2 * i, 1) = DataBuffer.format<ushort, 96, 16>().select<2, 1, 4, 2>(16 * i, 0);
            Result_UV.select<2, 1, 4, 2>(2 * i, 9) = DataBuffer.format<ushort, 96, 16>().select<2, 1, 4, 2>(16 * i + 8, 0);

            Result_UV.select<2, 1, 4, 2>(2 * i, 0) = DataBuffer.format<ushort, 96, 16>().select<2, 1, 4, 2>(16 * i + 4, 0);
            Result_UV.select<2, 1, 4, 2>(2 * i, 8) = DataBuffer.format<ushort, 96, 16>().select<2, 1, 4, 2>(16 * i + 12, 0);
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

            Result_UV.select<2, 1, 8, 2>(2 * i, 0) = Result_Temp_U.select<2, 1, 8, 2>(0, 0);
            Result_UV.select<2, 1, 8, 2>(2 * i, 1) = Result_Temp_V.select<2, 1, 8, 2>(0, 0);
            break;
        case CHROMA_SUBSAMPLING_BOTTOM_LEFT:
            Result_UV.select<2, 1, 4, 2>(2 * i, 1) = DataBuffer.format<ushort, 96, 16>().select<2, 1, 4, 2>(16 * i, 9);
            Result_UV.select<2, 1, 4, 2>(2 * i, 9) = DataBuffer.format<ushort, 96, 16>().select<2, 1, 4, 2>(16 * i + 8, 9);

            Result_UV.select<2, 1, 4, 2>(2 * i, 0) = DataBuffer.format<ushort, 96, 16>().select<2, 1, 4, 2>(16 * i + 4, 9);
            Result_UV.select<2, 1, 4, 2>(2 * i, 8) = DataBuffer.format<ushort, 96, 16>().select<2, 1, 4, 2>(16 * i + 12, 9);
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

            Result_UV.select<2, 1, 8, 2>(2 * i, 0) = Result_Temp_U.select<2, 1, 8, 2>(0, 0);
            Result_UV.select<2, 1, 8, 2>(2 * i, 1) = Result_Temp_V.select<2, 1, 8, 2>(0, 0);
            break;
        }
    }

    write(Dst_Surface_UV, 2 * DstX, DstY >> 1, Result_UV);
}