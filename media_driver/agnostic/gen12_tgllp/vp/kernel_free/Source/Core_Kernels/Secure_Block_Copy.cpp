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

_GENX_MAIN_ void Secure_Block_Copy(ushort Block_Height, ushort Buffer_Offset)
{
    SurfaceIndex Input_Surface(MDF_FC_SECURITY_COPY_INPUT_BTI);
    SurfaceIndex Output_Surface(MDF_FC_SECURITY_COPY_OUTPUT_BTI);

    matrix<uint, 9, 8> Mesg1;
    matrix<uint, 9, 8> Mesg2;

    uint desc_y1, desc_y2;
    uint desc_y3, desc_y4;

    Mesg1.select<1, 1, 8, 1>(0, 0) = cm_get_r0();
    Mesg2.select<1, 1, 8, 1>(0, 0) = cm_get_r0();

    matrix_ref<uchar, 8, 32> Temp1 = Mesg1.format<uchar, 9, 32>().select<8, 1, 32, 1>(1, 0);
    matrix_ref<uchar, 8, 32> Temp2 = Mesg2.format<uchar, 9, 32>().select<8, 1, 32, 1>(1, 0);
    uint Read_8_rows;
    Read_8_rows = Block_Height & 0xFFF8;

    uint RemainingRowOffset = Buffer_Offset + Read_8_rows;
    uint RemainingRows = Block_Height & 0x7;

    for (uint i = 0; i < Read_8_rows; i+= 8)
    {
        read(Input_Surface, 0, i + Buffer_Offset, Temp1);
        write(Output_Surface, 0, i + Buffer_Offset, Temp1);
        read(Input_Surface, 32, i + Buffer_Offset, Temp2);
        write(Output_Surface,32, i + Buffer_Offset, Temp2);
    }

    if (RemainingRows)
    {
        desc_y1 = nDPMR_MSGDSC + (RemainingRows << 20) + MDF_FC_SECURITY_COPY_INPUT_BTI;
        desc_y2 = nDPMW_MSGDSC + (RemainingRows << 25) + MDF_FC_SECURITY_COPY_OUTPUT_BTI;

        desc_y3 = nDPMR_MSGDSC + (RemainingRows << 20) + MDF_FC_SECURITY_COPY_INPUT_BTI;
        desc_y4 = nDPMW_MSGDSC + (RemainingRows << 25) + MDF_FC_SECURITY_COPY_OUTPUT_BTI;

        Mesg1.select<1, 1, 1, 1>(0, 0) = 0;
        Mesg1.select<1, 1, 1, 1>(0, 1) = RemainingRowOffset;
        Mesg1.select<1, 1, 1, 1>(0, 2) = nBLOCK_WIDTH_32 + ((RemainingRows - 1) << 16);

        Mesg2.select<1, 1, 1, 1>(0, 0) = 32;
        Mesg2.select<1, 1, 1, 1>(0, 1) = RemainingRowOffset;
        Mesg2.select<1, 1, 1, 1>(0, 2) = nBLOCK_WIDTH_32 + ((RemainingRows - 1) << 16);

        // left half read/write
        cm_send(Temp1,
            Mesg1,
            nDATAPORT_DC1,
            desc_y1,
            0);

        cm_send(NULL,
            Mesg1,
            nDATAPORT_DC1,
            desc_y2,
            0);

        // right half read/write
        cm_send(Temp2,
            Mesg2,
            nDATAPORT_DC1,
            desc_y3,
            0);

        cm_send(NULL,
            Mesg2,
            nDATAPORT_DC1,
            desc_y4,
            0);

    }
}