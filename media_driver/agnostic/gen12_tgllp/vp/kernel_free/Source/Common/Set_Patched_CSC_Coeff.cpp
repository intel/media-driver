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

_GENX_MAIN_ void Set_Patched_CSC_Coeff(
    CURBE_INPUT_OUTPUT,
    GLOBAL_BUFFER_INPUT_OUTPUT)
{
    SurfaceIndex Csc_Coeff_Surface(MDF_FC_CSC_COEFF_BTI);
    read(Csc_Coeff_Surface, 0, Reserved2, CscCoeff);
    Reserved2++;
    /*
    CscCoeff[0] = 0xfffe00e5;
    CscCoeff[1] = 0xdeadbeef;
    CscCoeff[2] = 0xee0f9653;
    CscCoeff[3] = 0x95278032;
    CscCoeff[4] = 0x283c2208;
    CscCoeff[5] = 0x30135057;
    */

}