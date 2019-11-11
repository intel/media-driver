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
#ifdef BUFFER_0
    Buffer_Index = 0;
    vector <ushort, 4> CalculationMaskTemp = (TempMask0.select<1, 1, 4, 1>(0, 0) != 0);
#endif
#ifdef BUFFER_1
    Buffer_Index = 1;
    vector <ushort, 4> CalculationMaskTemp = (TempMask0.select<1, 1, 4, 1>(0, 4) != 0);
#endif
#ifdef BUFFER_2
    Buffer_Index = 2;
    vector <ushort, 4> CalculationMaskTemp = (TempMask0.select<1, 1, 4, 1>(0, 8) != 0);
#endif
#ifdef BUFFER_3
    Buffer_Index = 3;
    vector <ushort, 4> CalculationMaskTemp = (TempMask0.select<1, 1, 4, 1>(0, 12) != 0);
#endif
#ifdef BUFFER_4
    Buffer_Index = 4;
    uchar sec_half_offset = (Layer_Index >> 7) << 3;
    vector <ushort, 4> CalculationMaskTemp = (TempMask.select<4, 1>(0 + sec_half_offset) != 0);
#endif
#ifdef BUFFER_5
    Buffer_Index = 5;
    uchar sec_half_offset = (Layer_Index >> 7) << 3;
    vector <ushort, 4> CalculationMaskTemp = (TempMask.select<4, 1>(4 + sec_half_offset) != 0);
#endif

    CalculationMask = cm_pack_mask(CalculationMaskTemp);
}