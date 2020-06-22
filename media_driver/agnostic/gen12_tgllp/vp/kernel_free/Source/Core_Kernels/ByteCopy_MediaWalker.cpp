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
#include "MDF_FC_common_genx.h"

_GENX_MAIN_ void ByteCopy_MediaWalker(
    CURBE_INPUT_OUTPUT,
    GLOBAL_BUFFER_INPUT_OUTPUT)
{
    matrix_ref<uchar, 16, 16> Temp = DataBuffer4.format<uchar, 32, 16>().select<16, 1, 16, 1>(0, 0);

    SurfaceIndex Input(MDF_FC_INPUT_BTI_START);
    SurfaceIndex Output(MDF_FC_OUTPUT_BTI_START);

    DstX = get_thread_origin_x() * 16;
    DstY = get_thread_origin_y() * 16;

    ushort StartXOffset, StartYOffset;
    StartXOffset = Reserved1.format<ushort>()[2];
    StartYOffset = Reserved1.format<ushort>()[3];

    read(Input, DstX + StartXOffset, DstY + StartYOffset, Temp);

    StartXOffset = ColorFill.format<ushort>()[0];
    StartYOffset = ColorFill.format<ushort>()[1];

    write(Output, DstX + StartXOffset, DstY + StartYOffset, Temp);
}