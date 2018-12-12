/*
// Copyright (c) 2018 Intel Corporation
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the
// 'Software'), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
// CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
// TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
// SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

const unsigned int IGCDUMMYKRN_SIZE = 0;
const unsigned int IGCDUMMYKRN[] = {
};

//gen9 dummy kernel
extern const unsigned int IGCODECKRN_G9_SIZE = IGCDUMMYKRN_SIZE;
extern const unsigned int* IGCODECKRN_G9;
const unsigned int* IGCODECKRN_G9 =IGCDUMMYKRN;

extern const unsigned int IGVPKRN_G9_SIZE = IGCDUMMYKRN_SIZE;
extern const unsigned int* IGVPKRN_G9;
const unsigned int* IGVPKRN_G9 = IGCDUMMYKRN;

extern const unsigned int IGVP_HVS_DENOISE_G900_SIZE = IGCDUMMYKRN_SIZE;
extern const unsigned int* IGVP_HVS_DENOISE_G900;
const unsigned int* IGVP_HVS_DENOISE_G900 = IGCDUMMYKRN;

//gen9 bxt dummy kernel
extern const unsigned int IGCODECKRN_G9_BXT_SIZE = IGCDUMMYKRN_SIZE;
extern const unsigned int* IGCODECKRN_G9_BXT;
const unsigned int* IGCODECKRN_G9_BXT = IGCDUMMYKRN;

//gen9 kbl dummy kernel
extern const unsigned int IGCODECKRN_G9_KBL_SIZE = IGCDUMMYKRN_SIZE;
extern const unsigned int* IGCODECKRN_G9_KBL;
const unsigned int* IGCODECKRN_G9_KBL = IGCDUMMYKRN;

//gen10 dummy kernel
extern const unsigned int IGCODECKRN_G10_SIZE = IGCDUMMYKRN_SIZE;
extern const unsigned int* IGCODECKRN_G10;
const unsigned int* IGCODECKRN_G10 = IGCDUMMYKRN;

extern const unsigned int IGVPKRN_G10_SIZE = IGCDUMMYKRN_SIZE;
extern const unsigned int* IGVPKRN_G10;
const unsigned int* IGVPKRN_G10 = IGCDUMMYKRN;

//gen11 dummy kernel
extern const unsigned int IGCODECKRN_G11_SIZE = IGCDUMMYKRN_SIZE;
extern const unsigned int* IGCODECKRN_G11;
const unsigned int* IGCODECKRN_G11 = IGCDUMMYKRN;

//gen11 lp dummy kernel
extern const unsigned int IGCODECKRN_G11_ICLLP_SIZE = IGCDUMMYKRN_SIZE;
extern const unsigned int* IGCODECKRN_G11_ICLLP;
const unsigned int* IGCODECKRN_G11_ICLLP = IGCDUMMYKRN;

extern const unsigned int IGVPKRN_G11_ICLLP_SIZE = IGCDUMMYKRN_SIZE;
extern const unsigned int* IGVPKRN_G11_ICLLP;
const unsigned int* IGVPKRN_G11_ICLLP = IGCDUMMYKRN;

extern const unsigned int IGVP3DLUT_GENERATION_G11_ICLLP_SIZE = IGCDUMMYKRN_SIZE;
extern const unsigned int* IGVP3DLUT_GENERATION_G11_ICLLP;
const unsigned int* IGVP3DLUT_GENERATION_G11_ICLLP = IGCDUMMYKRN;
