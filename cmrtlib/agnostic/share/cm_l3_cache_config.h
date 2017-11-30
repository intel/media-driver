/*
* Copyright (c) 2017, Intel Corporation
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

#ifndef CMRTLIB_AGNOSTIC_SHARE_CM_L3_CACHE_CONFIG_H_
#define CMRTLIB_AGNOSTIC_SHARE_CM_L3_CACHE_CONFIG_H_

struct L3ConfigRegisterValues
{
    L3ConfigRegisterValues(): config_register0(0),
                              config_register1(0),
                              config_register2(0),
                              config_register3(0) {}

    L3ConfigRegisterValues(uint32_t registerValue0,
                           uint32_t registerValue1,
                           uint32_t registerValue2,
                           uint32_t registerValue3):
        config_register0(registerValue0),
        config_register1(registerValue1),
        config_register2(registerValue2),
        config_register3(registerValue3) {}

    uint32_t config_register0;
    uint32_t config_register1;
    uint32_t config_register2;
    uint32_t config_register3;
};

typedef enum _L3_SUGGEST_CONFIG
{
   CM_L3_PLANE_DEFAULT = 0,
   CM_L3_PLANE_1,
   CM_L3_PLANE_2,
   CM_L3_PLANE_3,
   CM_L3_PLANE_4,
   CM_L3_PLANE_5,
   CM_L3_PLANE_6,
   CM_L3_PLANE_7,
   CM_L3_PLANE_8,
} L3_SUGGEST_CONFIG;

#endif  // #ifndef CMRTLIB_AGNOSTIC_SHARE_CM_L3_CACHE_CONFIG_H_
