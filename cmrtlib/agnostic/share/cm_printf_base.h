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
#pragma once

#define PRINT_HEADER_SIZE 32

#define CM_PRINT_OBJECT_TYPE_ENTRY_INDEX 0
#define CM_PRINT_DATA___TYPE_ENTRY_INDEX 1
#define CM_PRINT_LOWER32BITS_ENTRY_INDEX 6
#define CM_PRINT_UPPER32BITS_ENTRY_INDEX 7

#define CM_PRINT_DATA_TYPE_CHAR   0
#define CM_PRINT_DATA_TYPE_UCHAR  1
#define CM_PRINT_DATA_TYPE_FLOAT  2
#define CM_PRINT_DATA_TYPE_INT    3
#define CM_PRINT_DATA_TYPE_UINT   4
#define CM_PRINT_DATA_TYPE_SHORT  5
#define CM_PRINT_DATA_TYPE_USHORT 6
#define CM_PRINT_DATA_TYPE_QWORD  7
#define CM_PRINT_DATA_TYPE_UQWORD 8
#define CM_PRINT_DATA_TYPE_DOUBLE 9

#define CM_PRINT_OBJECT_TYPE_UNKNOWN 0
#define CM_PRINT_OBJECT_TYPE_MATRIX  1
#define CM_PRINT_OBJECT_TYPE_VECTOR  2
#define CM_PRINT_OBJECT_TYPE_SCALAR  3
#define CM_PRINT_OBJECT_TYPE_STRING  4
#define CM_PRINT_OBJECT_TYPE_FORMAT  5

/// If you want to change the static
/// buffer change these two macros.
#define CM_PRINTF_STATIC_BUFFER_ID 1
#define CM_PRINT_BUFFER CM_STATIC_BUFFER_1

