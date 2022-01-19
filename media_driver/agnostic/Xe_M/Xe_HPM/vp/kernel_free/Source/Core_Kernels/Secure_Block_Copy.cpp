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

#include "MDF_FC_common_genx.h"

_GENX_MAIN_ void Secure_Block_Copy()
{
    SurfaceIndex Input_Surface(MDF_FC_SECURITY_COPY_INPUT_BTI);
    SurfaceIndex Output_Surface(MDF_FC_SECURITY_COPY_OUTPUT_BTI);

    matrix<uint, 9, 8> Mesg1;
    matrix<uint, 9, 8> Mesg2;

    matrix_ref<uchar, 8, 32> Temp1 = Mesg1.format<uchar, 9, 32>().select<8, 1, 32, 1>(1, 0);
    matrix_ref<uchar, 8, 32> Temp2 = Mesg2.format<uchar, 9, 32>().select<8, 1, 32, 1>(1, 0);

    ushort Buffer_Offset = get_thread_origin_y() * MDF_FC_SECURITY_COPY_BLOCK_HEIGHT;

#pragma unroll
    for (uint i = 0; i < MDF_FC_SECURITY_COPY_BLOCK_HEIGHT; i+= 8)
    {
        read(Input_Surface, 0, i + Buffer_Offset, Temp1);
        write(Output_Surface, 0, i + Buffer_Offset, Temp1);
        read(Input_Surface, 32, i + Buffer_Offset, Temp2);
        write(Output_Surface,32, i + Buffer_Offset, Temp2);
    }
}