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
    SurfaceIndex Gamma_Lut_Surface(MDF_FC_GAMMA_LUT_BTI);

    read(Gamma_Lut_Surface, 0, 0, GammaC_Lut);

#endif
#ifdef BUFFER_1
    Buffer_Index = 1;
#endif
#ifdef BUFFER_2
    Buffer_Index = 2;
#endif
#ifdef BUFFER_3
    Buffer_Index = 3;
#endif
#ifdef BUFFER_4
    Buffer_Index = 4;
#endif
#ifdef BUFFER_5
    Buffer_Index = 5;
#endif
}