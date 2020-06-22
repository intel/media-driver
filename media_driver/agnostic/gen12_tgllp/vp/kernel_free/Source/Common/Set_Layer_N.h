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
    ushort Mask_Temp;
    vector<ushort, 16> Temp(Inc_ushort);
#ifdef LAYER_0
    Layer_Index = 0;
#endif
#ifdef LAYER_1
    Layer_Index = 1;
    ConstAlphaTemp = ConstantBlendingAlpha(0);
#endif
#ifdef LAYER_2
    Layer_Index = 2;
    ConstAlphaTemp = ConstantBlendingAlpha(1);
#endif
#ifdef LAYER_3
    Layer_Index = 3;
    ConstAlphaTemp = ConstantBlendingAlpha(2);
#endif
#ifdef LAYER_4
    Layer_Index = 4;
    ConstAlphaTemp = ConstantBlendingAlpha(3);
#endif
#ifdef LAYER_5
    Layer_Index = 5;
    ConstAlphaTemp = ConstantBlendingAlpha(4);
#endif
#ifdef LAYER_6
    Layer_Index = 6;
    ConstAlphaTemp = ConstantBlendingAlpha(5);
#endif
#ifdef LAYER_7
    Layer_Index = 7;
    ConstAlphaTemp = ConstantBlendingAlpha(6);
#endif

#if (defined LAYER_1) || (defined LAYER_2) || (defined LAYER_3) || (defined LAYER_4)|| (defined LAYER_5) || (defined LAYER_6) || (defined LAYER_7)
    // X coordinate mask
    Mask_Temp = cm_add<ushort>(Top_Left(Layer_Index << 1), -DstX, SAT);
    Mask_Temp = cm_min<ushort>(Mask_Temp, MDF_FC_BLOCK_WIDTH);

    Top_Left(Layer_Index << 1) = 0xffff << Mask_Temp;

    Mask_Temp = cm_add<ushort>(DstX + MDF_FC_BLOCK_WIDTH - 1, -Bottom_Right(Layer_Index << 1), SAT);
    Mask_Temp = cm_min<ushort>(Mask_Temp, MDF_FC_BLOCK_WIDTH);

    Bottom_Right(Layer_Index << 1) = 0xffff >> Mask_Temp;

    // Y coordinate
    TempMask = Top_Left(Layer_Index << 1) & Bottom_Right(Layer_Index << 1);
    Temp = DstY + Temp;
    TempMask.merge(TempMask, 0, Temp >= Top_Left((Layer_Index << 1) + 1));
    TempMask.merge(TempMask, 0, Temp <= Bottom_Right((Layer_Index << 1) + 1));

    // Shuffle Mask to sampler writeback style
#pragma unroll
    for (uchar i = 0; i < 4; i++)
    {
        Temp.format<uchar>().select<8, 1>(0) = TempMask.format<uchar>().select<8, 1>(8 * i);
        TempMask.format<uchar>().select<4, 1>(8 * i) = Temp.format<uchar>().select<4, 2>(0);
        TempMask.format<uchar>().select<4, 1>(8 * i + 4) = Temp.format<uchar>().select<4, 2>(1);
    }
#endif
}