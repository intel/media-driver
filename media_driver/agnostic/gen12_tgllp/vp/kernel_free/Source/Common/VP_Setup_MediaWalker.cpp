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

_GENX_MAIN_ _CM_ENTRY_ void VP_Setup_MediaWalker(
    CURBE_INPUT_OUTPUT,
    GLOBAL_BUFFER_OUTPUT)
{
    // Mask calculation for Layer0
    ushort Mask_Temp;
    vector<ushort, 16> Temp(Inc_ushort);

    // Shuffle Mask to sampler writeback style
    matrix<uchar, 1, 8> ShuffleTemp;

    DstX = get_thread_origin_x() * 16;
    DstY = get_thread_origin_y() * 16;
    Layer_Index = 0xff;

    {
        // X coordinate mask
        Mask_Temp = cm_add<ushort>(Top_Left(0), -DstX, SAT);
        Mask_Temp = cm_min<ushort>(Mask_Temp, MDF_FC_BLOCK_WIDTH);

        Top_Left(0) = 0xffff << Mask_Temp;

        Mask_Temp = cm_add<ushort>(DstX + MDF_FC_BLOCK_WIDTH - 1, -Bottom_Right(0), SAT);
        Mask_Temp = cm_min<ushort>(Mask_Temp, MDF_FC_BLOCK_WIDTH);

        Bottom_Right(0) = 0xffff >> Mask_Temp;

        // Y coordinate
        TempMask0 = Top_Left(0) & Bottom_Right(0);
        Temp = DstY + Temp;
        TempMask0.merge(TempMask0, 0, Temp >= Top_Left(1));
        TempMask0.merge(TempMask0, 0, Temp <= Bottom_Right(1));

#pragma unroll
        for (uchar i = 0; i < 4; i++)
        {
            ShuffleTemp.select<1, 1, 8, 1>(0, 0) = TempMask0.format<uchar, 1, 32>().select<1, 1, 8, 1>(0, 8 * i);
            TempMask0.format<uchar, 1, 32>().select<1, 1, 4, 1>(0, 8 * i) = ShuffleTemp.select<1, 1, 4, 2>(0, 0);
            TempMask0.format<uchar, 1, 32>().select<1, 1, 4, 1>(0, 8 * i + 4) = ShuffleTemp.select<1, 1, 4, 2>(0, 1);
        }
    }
}