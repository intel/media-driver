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
//!
//! \file     codechal_encode_vp8.cpp
//! \brief    Defines base class for VP8 dual-pipe encoder.
//!
#include "codechal_encode_vp8.h"
#include "codechal_mmc_encode_vp8.h"
#if USE_CODECHAL_DEBUG_TOOL
#include "codechal_debug.h"
#endif

#define ENCODE_VP8_BRC_HISTORY_BUFFER_SIZE      704

static uint16_t PakQPInputTable[160 * 18] =
{
    0x4000, 0x0004, 0x0010, 0x4000, 0x0004, 0x0010, 0x2000, 0x0008, 0x0010, 0x2000, 0x0008, 0x0010, 0x4000, 0x0004, 0x0010, 0x4000, 0x0004, 0x0010,
    0x4000, 0x0004, 0x0010, 0x4000, 0x0004, 0x0010, 0x2000, 0x0008, 0x0010, 0x2000, 0x0008, 0x0010, 0x4000, 0x0004, 0x0010, 0x4000, 0x0004, 0x0010,
    0x4000, 0x0004, 0x0010, 0x4000, 0x0004, 0x0010, 0x2000, 0x0008, 0x0010, 0x2000, 0x0008, 0x0010, 0x4000, 0x0004, 0x0010, 0x4000, 0x0004, 0x0010,
    0x4000, 0x0004, 0x0010, 0x4000, 0x0004, 0x0010, 0x2000, 0x0008, 0x0010, 0x2000, 0x0008, 0x0010, 0x4000, 0x0004, 0x0010, 0x4000, 0x0004, 0x0010,
    0x4000, 0x0004, 0x0010, 0x4000, 0x0004, 0x0010, 0x2000, 0x0008, 0x0010, 0x2000, 0x0008, 0x0010, 0x4000, 0x0004, 0x0010, 0x4000, 0x0004, 0x0010,
    0x4000, 0x0004, 0x0010, 0x4000, 0x0004, 0x0010, 0x2000, 0x0008, 0x0010, 0x2000, 0x0008, 0x0010, 0x4000, 0x0004, 0x0010, 0x4000, 0x0004, 0x0010,
    0x4000, 0x0004, 0x0010, 0x4000, 0x0004, 0x0010, 0x2000, 0x0008, 0x0010, 0x2000, 0x0008, 0x0010, 0x4000, 0x0004, 0x0010, 0x4000, 0x0004, 0x0010,
    0x4000, 0x0004, 0x0010, 0x4000, 0x0004, 0x0010, 0x2000, 0x0008, 0x0010, 0x2000, 0x0008, 0x0010, 0x4000, 0x0004, 0x0010, 0x4000, 0x0004, 0x0010,
    0x4000, 0x0004, 0x0010, 0x4000, 0x0004, 0x0010, 0x2000, 0x0008, 0x0010, 0x2000, 0x0008, 0x0010, 0x4000, 0x0004, 0x0010, 0x4000, 0x0004, 0x0010,
    0x4000, 0x0004, 0x0010, 0x4000, 0x0004, 0x0010, 0x2000, 0x0008, 0x0010, 0x2000, 0x0008, 0x0010, 0x4000, 0x0004, 0x0010, 0x4000, 0x0004, 0x0010,
    0x4000, 0x0004, 0x0010, 0x4000, 0x0004, 0x0010, 0x2000, 0x0008, 0x0010, 0x2000, 0x0008, 0x0010, 0x4000, 0x0004, 0x0010, 0x4000, 0x0004, 0x0010,
    0x4000, 0x0004, 0x0010, 0x4000, 0x0004, 0x0010, 0x2000, 0x0008, 0x0010, 0x2000, 0x0008, 0x0010, 0x4000, 0x0004, 0x0010, 0x4000, 0x0004, 0x0010,
    0x4000, 0x0004, 0x0010, 0x4000, 0x0004, 0x0010, 0x2000, 0x0008, 0x0010, 0x2000, 0x0008, 0x0010, 0x4000, 0x0004, 0x0010, 0x4000, 0x0004, 0x0010,
    0x4000, 0x0004, 0x0010, 0x4000, 0x0004, 0x0010, 0x2000, 0x0008, 0x0010, 0x2000, 0x0008, 0x0010, 0x4000, 0x0004, 0x0010, 0x4000, 0x0004, 0x0010,
    0x4000, 0x0004, 0x0010, 0x4000, 0x0004, 0x0010, 0x2000, 0x0008, 0x0010, 0x2000, 0x0008, 0x0010, 0x4000, 0x0004, 0x0010, 0x4000, 0x0004, 0x0010,
    0x4000, 0x0004, 0x0010, 0x4000, 0x0004, 0x0010, 0x2000, 0x0008, 0x0010, 0x2000, 0x0008, 0x0010, 0x4000, 0x0004, 0x0010, 0x4000, 0x0004, 0x0010,
    0x4000, 0x0004, 0x0010, 0x4000, 0x0004, 0x0010, 0x2000, 0x0008, 0x0010, 0x2000, 0x0008, 0x0010, 0x4000, 0x0004, 0x0010, 0x4000, 0x0004, 0x0010,
    0x3333, 0x0005, 0x0010, 0x3333, 0x0005, 0x0010, 0x2000, 0x0008, 0x0010, 0x1999, 0x000a, 0x0010, 0x3333, 0x0005, 0x0010, 0x3333, 0x0005, 0x0010,
    0x2aaa, 0x0006, 0x0010, 0x2aaa, 0x0006, 0x0010, 0x1c71, 0x0009, 0x0010, 0x1555, 0x000c, 0x0010, 0x2aaa, 0x0006, 0x0010, 0x2aaa, 0x0006, 0x0010,
    0x2492, 0x0007, 0x0010, 0x2492, 0x0007, 0x0010, 0x1999, 0x000a, 0x0010, 0x1249, 0x000e, 0x0010, 0x2492, 0x0007, 0x0010, 0x2492, 0x0007, 0x0010,
    0x2000, 0x0008, 0x0010, 0x2000, 0x0008, 0x0010, 0x1555, 0x000c, 0x0010, 0x1000, 0x0010, 0x0010, 0x2000, 0x0008, 0x0010, 0x2000, 0x0008, 0x0010,
    0x1c71, 0x0009, 0x0010, 0x1c71, 0x0009, 0x0010, 0x13b1, 0x000d, 0x0010, 0x0e38, 0x0012, 0x0010, 0x1c71, 0x0009, 0x0010, 0x1c71, 0x0009, 0x0010,
    0x1999, 0x000a, 0x0010, 0x1999, 0x000a, 0x0010, 0x1111, 0x000f, 0x0010, 0x0ccc, 0x0014, 0x0010, 0x1999, 0x000a, 0x0010, 0x1999, 0x000a, 0x0010,
    0x1745, 0x000b, 0x0010, 0x1999, 0x000a, 0x0010, 0x0f0f, 0x0011, 0x0010, 0x0ccc, 0x0014, 0x0010, 0x1745, 0x000b, 0x0010, 0x1999, 0x000a, 0x0010,
    0x1555, 0x000c, 0x0010, 0x1745, 0x000b, 0x0010, 0x0e38, 0x0012, 0x0010, 0x0ba2, 0x0016, 0x0010, 0x1555, 0x000c, 0x0010, 0x1745, 0x000b, 0x0010,
    0x13b1, 0x000d, 0x0010, 0x1555, 0x000c, 0x0010, 0x0ccc, 0x0014, 0x0010, 0x0aaa, 0x0018, 0x0010, 0x13b1, 0x000d, 0x0010, 0x1555, 0x000c, 0x0010,
    0x1249, 0x000e, 0x0010, 0x13b1, 0x000d, 0x0010, 0x0c30, 0x0015, 0x0010, 0x09d8, 0x001a, 0x0010, 0x1249, 0x000e, 0x0010, 0x13b1, 0x000d, 0x0010,
    0x1111, 0x000f, 0x0010, 0x1249, 0x000e, 0x0010, 0x0b21, 0x0017, 0x0010, 0x0924, 0x001c, 0x0010, 0x1111, 0x000f, 0x0010, 0x1249, 0x000e, 0x0010,
    0x1000, 0x0010, 0x0010, 0x1111, 0x000f, 0x0010, 0x0aaa, 0x0018, 0x0010, 0x0888, 0x001e, 0x0010, 0x1000, 0x0010, 0x0010, 0x1111, 0x000f, 0x0010,
    0x0f0f, 0x0011, 0x0010, 0x1000, 0x0010, 0x0010, 0x09d8, 0x001a, 0x0010, 0x0800, 0x0020, 0x0010, 0x0f0f, 0x0011, 0x0010, 0x1000, 0x0010, 0x0010,
    0x0e38, 0x0012, 0x0010, 0x0f0f, 0x0011, 0x0010, 0x097b, 0x001b, 0x0010, 0x0787, 0x0022, 0x0010, 0x0e38, 0x0012, 0x0010, 0x0f0f, 0x0011, 0x0010,
    0x0d79, 0x0013, 0x0010, 0x0f0f, 0x0011, 0x0010, 0x08d3, 0x001d, 0x0010, 0x0787, 0x0022, 0x0010, 0x0d79, 0x0013, 0x0010, 0x0f0f, 0x0011, 0x0010,
    0x0ccc, 0x0014, 0x0010, 0x0e38, 0x0012, 0x0010, 0x0842, 0x001f, 0x0010, 0x071c, 0x0024, 0x0010, 0x0ccc, 0x0014, 0x0010, 0x0e38, 0x0012, 0x0010,
    0x0c30, 0x0015, 0x0010, 0x0d79, 0x0013, 0x0010, 0x0800, 0x0020, 0x0010, 0x06bc, 0x0026, 0x0010, 0x0c30, 0x0015, 0x0010, 0x0d79, 0x0013, 0x0010,
    0x0ba2, 0x0016, 0x0010, 0x0ccc, 0x0014, 0x0010, 0x0787, 0x0022, 0x0010, 0x0666, 0x0028, 0x0010, 0x0ba2, 0x0016, 0x0010, 0x0ccc, 0x0014, 0x0010,
    0x0b21, 0x0017, 0x0010, 0x0ccc, 0x0014, 0x0010, 0x0750, 0x0023, 0x0010, 0x0666, 0x0028, 0x0010, 0x0b21, 0x0017, 0x0010, 0x0ccc, 0x0014, 0x0010,
    0x0aaa, 0x0018, 0x0010, 0x0c30, 0x0015, 0x0010, 0x06eb, 0x0025, 0x0010, 0x0618, 0x002a, 0x0010, 0x0aaa, 0x0018, 0x0010, 0x0c30, 0x0015, 0x0010,
    0x0a3d, 0x0019, 0x0010, 0x0c30, 0x0015, 0x0010, 0x06bc, 0x0026, 0x0010, 0x0618, 0x002a, 0x0010, 0x0a3d, 0x0019, 0x0010, 0x0c30, 0x0015, 0x0010,
    0x09d8, 0x001a, 0x0010, 0x0ba2, 0x0016, 0x0010, 0x0666, 0x0028, 0x0010, 0x05d1, 0x002c, 0x0010, 0x09d8, 0x001a, 0x0010, 0x0ba2, 0x0016, 0x0010,
    0x097b, 0x001b, 0x0010, 0x0ba2, 0x0016, 0x0010, 0x063e, 0x0029, 0x0010, 0x05d1, 0x002c, 0x0010, 0x097b, 0x001b, 0x0010, 0x0ba2, 0x0016, 0x0010,
    0x0924, 0x001c, 0x0010, 0x0b21, 0x0017, 0x0010, 0x05f4, 0x002b, 0x0010, 0x0590, 0x002e, 0x0010, 0x0924, 0x001c, 0x0010, 0x0b21, 0x0017, 0x0010,
    0x08d3, 0x001d, 0x0010, 0x0b21, 0x0017, 0x0010, 0x05d1, 0x002c, 0x0010, 0x0590, 0x002e, 0x0010, 0x08d3, 0x001d, 0x0010, 0x0b21, 0x0017, 0x0010,
    0x0888, 0x001e, 0x0010, 0x0aaa, 0x0018, 0x0010, 0x0590, 0x002e, 0x0010, 0x0555, 0x0030, 0x0010, 0x0888, 0x001e, 0x0010, 0x0aaa, 0x0018, 0x0010,
    0x0842, 0x001f, 0x0010, 0x0a3d, 0x0019, 0x0010, 0x0555, 0x0030, 0x0010, 0x051e, 0x0032, 0x0010, 0x0842, 0x001f, 0x0010, 0x0a3d, 0x0019, 0x0010,
    0x0800, 0x0020, 0x0010, 0x0a3d, 0x0019, 0x0010, 0x0539, 0x0031, 0x0010, 0x051e, 0x0032, 0x0010, 0x0800, 0x0020, 0x0010, 0x0a3d, 0x0019, 0x0010,
    0x07c1, 0x0021, 0x0010, 0x09d8, 0x001a, 0x0010, 0x0505, 0x0033, 0x0010, 0x04ec, 0x0034, 0x0010, 0x07c1, 0x0021, 0x0010, 0x09d8, 0x001a, 0x0010,
    0x0787, 0x0022, 0x0010, 0x097b, 0x001b, 0x0010, 0x04ec, 0x0034, 0x0010, 0x04bd, 0x0036, 0x0010, 0x0787, 0x0022, 0x0010, 0x097b, 0x001b, 0x0010,
    0x0750, 0x0023, 0x0010, 0x0924, 0x001c, 0x0010, 0x04bd, 0x0036, 0x0010, 0x0492, 0x0038, 0x0010, 0x0750, 0x0023, 0x0010, 0x0924, 0x001c, 0x0010,
    0x071c, 0x0024, 0x0010, 0x08d3, 0x001d, 0x0010, 0x04a7, 0x0037, 0x0010, 0x0469, 0x003a, 0x0010, 0x071c, 0x0024, 0x0010, 0x08d3, 0x001d, 0x0010,
    0x06eb, 0x0025, 0x0010, 0x0888, 0x001e, 0x0010, 0x047d, 0x0039, 0x0010, 0x0444, 0x003c, 0x0010, 0x06eb, 0x0025, 0x0010, 0x0888, 0x001e, 0x0010,
    0x06bc, 0x0026, 0x0010, 0x0842, 0x001f, 0x0010, 0x0469, 0x003a, 0x0010, 0x0421, 0x003e, 0x0010, 0x06bc, 0x0026, 0x0010, 0x0842, 0x001f, 0x0010,
    0x0690, 0x0027, 0x0010, 0x0800, 0x0020, 0x0010, 0x0444, 0x003c, 0x0010, 0x0400, 0x0040, 0x0010, 0x0690, 0x0027, 0x0010, 0x0800, 0x0020, 0x0010,
    0x0666, 0x0028, 0x0010, 0x07c1, 0x0021, 0x0010, 0x0421, 0x003e, 0x0010, 0x03e0, 0x0042, 0x0010, 0x0666, 0x0028, 0x0010, 0x07c1, 0x0021, 0x0010,
    0x063e, 0x0029, 0x0010, 0x0787, 0x0022, 0x0010, 0x0410, 0x003f, 0x0010, 0x03c3, 0x0044, 0x0010, 0x063e, 0x0029, 0x0010, 0x0787, 0x0022, 0x0010,
    0x0618, 0x002a, 0x0010, 0x0750, 0x0023, 0x0010, 0x03f0, 0x0041, 0x0010, 0x03a8, 0x0046, 0x0010, 0x0618, 0x002a, 0x0010, 0x0750, 0x0023, 0x0010,
    0x05f4, 0x002b, 0x0010, 0x071c, 0x0024, 0x0010, 0x03e0, 0x0042, 0x0010, 0x038e, 0x0048, 0x0010, 0x05f4, 0x002b, 0x0010, 0x071c, 0x0024, 0x0010,
    0x05d1, 0x002c, 0x0010, 0x06eb, 0x0025, 0x0010, 0x03c3, 0x0044, 0x0010, 0x0375, 0x004a, 0x0010, 0x05d1, 0x002c, 0x0010, 0x06eb, 0x0025, 0x0010,
    0x05b0, 0x002d, 0x0010, 0x06eb, 0x0025, 0x0010, 0x03b5, 0x0045, 0x0010, 0x0375, 0x004a, 0x0010, 0x05b0, 0x002d, 0x0010, 0x06eb, 0x0025, 0x0010,
    0x0590, 0x002e, 0x0010, 0x06bc, 0x0026, 0x0010, 0x039b, 0x0047, 0x0010, 0x035e, 0x004c, 0x0010, 0x0590, 0x002e, 0x0010, 0x06bc, 0x0026, 0x0010,
    0x0572, 0x002f, 0x0010, 0x0690, 0x0027, 0x0010, 0x038e, 0x0048, 0x0010, 0x0348, 0x004e, 0x0010, 0x0572, 0x002f, 0x0010, 0x0690, 0x0027, 0x0010,
    0x0555, 0x0030, 0x0010, 0x0666, 0x0028, 0x0010, 0x0375, 0x004a, 0x0010, 0x0333, 0x0050, 0x0010, 0x0555, 0x0030, 0x0010, 0x0666, 0x0028, 0x0010,
    0x0539, 0x0031, 0x0010, 0x063e, 0x0029, 0x0010, 0x0369, 0x004b, 0x0010, 0x031f, 0x0052, 0x0010, 0x0539, 0x0031, 0x0010, 0x063e, 0x0029, 0x0010,
    0x051e, 0x0032, 0x0010, 0x0618, 0x002a, 0x0010, 0x0353, 0x004d, 0x0010, 0x030c, 0x0054, 0x0010, 0x051e, 0x0032, 0x0010, 0x0618, 0x002a, 0x0010,
    0x0505, 0x0033, 0x0010, 0x05f4, 0x002b, 0x0010, 0x033d, 0x004f, 0x0010, 0x02fa, 0x0056, 0x0010, 0x0505, 0x0033, 0x0010, 0x05f4, 0x002b, 0x0010,
    0x04ec, 0x0034, 0x0010, 0x05d1, 0x002c, 0x0010, 0x0333, 0x0050, 0x0010, 0x02e8, 0x0058, 0x0010, 0x04ec, 0x0034, 0x0010, 0x05d1, 0x002c, 0x0010,
    0x04d4, 0x0035, 0x0010, 0x05b0, 0x002d, 0x0010, 0x031f, 0x0052, 0x0010, 0x02d8, 0x005a, 0x0010, 0x04d4, 0x0035, 0x0010, 0x05b0, 0x002d, 0x0010,
    0x04bd, 0x0036, 0x0010, 0x0590, 0x002e, 0x0010, 0x0315, 0x0053, 0x0010, 0x02c8, 0x005c, 0x0010, 0x04bd, 0x0036, 0x0010, 0x0590, 0x002e, 0x0010,
    0x04a7, 0x0037, 0x0010, 0x0590, 0x002e, 0x0010, 0x0303, 0x0055, 0x0010, 0x02c8, 0x005c, 0x0010, 0x04a7, 0x0037, 0x0010, 0x0590, 0x002e, 0x0010,
    0x0492, 0x0038, 0x0010, 0x0572, 0x002f, 0x0010, 0x02fa, 0x0056, 0x0010, 0x02b9, 0x005e, 0x0010, 0x0492, 0x0038, 0x0010, 0x0572, 0x002f, 0x0010,
    0x047d, 0x0039, 0x0010, 0x0555, 0x0030, 0x0010, 0x02e8, 0x0058, 0x0010, 0x02aa, 0x0060, 0x0010, 0x047d, 0x0039, 0x0010, 0x0555, 0x0030, 0x0010,
    0x0469, 0x003a, 0x0010, 0x0539, 0x0031, 0x0010, 0x02e0, 0x0059, 0x0010, 0x029c, 0x0062, 0x0010, 0x0469, 0x003a, 0x0010, 0x0539, 0x0031, 0x0010,
    0x0444, 0x003c, 0x0010, 0x051e, 0x0032, 0x0010, 0x02c0, 0x005d, 0x0010, 0x028f, 0x0064, 0x0010, 0x0444, 0x003c, 0x0010, 0x051e, 0x0032, 0x0010,
    0x0421, 0x003e, 0x0010, 0x0505, 0x0033, 0x0010, 0x02aa, 0x0060, 0x0010, 0x0282, 0x0066, 0x0010, 0x0421, 0x003e, 0x0010, 0x0505, 0x0033, 0x0010,
    0x0400, 0x0040, 0x0010, 0x04ec, 0x0034, 0x0010, 0x0295, 0x0063, 0x0010, 0x0276, 0x0068, 0x0010, 0x0400, 0x0040, 0x0010, 0x04ec, 0x0034, 0x0010,
    0x03e0, 0x0042, 0x0010, 0x04d4, 0x0035, 0x0010, 0x0282, 0x0066, 0x0010, 0x026a, 0x006a, 0x0010, 0x03e0, 0x0042, 0x0010, 0x04d4, 0x0035, 0x0010,
    0x03c3, 0x0044, 0x0010, 0x04bd, 0x0036, 0x0010, 0x0270, 0x0069, 0x0010, 0x025e, 0x006c, 0x0010, 0x03c3, 0x0044, 0x0010, 0x04bd, 0x0036, 0x0010,
    0x03a8, 0x0046, 0x0010, 0x04a7, 0x0037, 0x0010, 0x025e, 0x006c, 0x0010, 0x0253, 0x006e, 0x0010, 0x03a8, 0x0046, 0x0010, 0x04a7, 0x0037, 0x0010,
    0x038e, 0x0048, 0x0010, 0x0492, 0x0038, 0x0010, 0x024e, 0x006f, 0x0010, 0x0249, 0x0070, 0x0010, 0x038e, 0x0048, 0x0010, 0x0492, 0x0038, 0x0010,
    0x0375, 0x004a, 0x0010, 0x047d, 0x0039, 0x0010, 0x023e, 0x0072, 0x0010, 0x023e, 0x0072, 0x0010, 0x0375, 0x004a, 0x0010, 0x047d, 0x0039, 0x0010,
    0x035e, 0x004c, 0x0010, 0x0469, 0x003a, 0x0010, 0x0230, 0x0075, 0x0010, 0x0234, 0x0074, 0x0010, 0x035e, 0x004c, 0x0010, 0x0469, 0x003a, 0x0010,
    0x0348, 0x004e, 0x0010, 0x0456, 0x003b, 0x0010, 0x0222, 0x0078, 0x0010, 0x022b, 0x0076, 0x0010, 0x0348, 0x004e, 0x0010, 0x0456, 0x003b, 0x0010,
    0x0333, 0x0050, 0x0010, 0x0444, 0x003c, 0x0010, 0x0210, 0x007c, 0x0010, 0x0222, 0x0078, 0x0010, 0x0333, 0x0050, 0x0010, 0x0444, 0x003c, 0x0010,
    0x031f, 0x0052, 0x0010, 0x0432, 0x003d, 0x0010, 0x0204, 0x007f, 0x0010, 0x0219, 0x007a, 0x0010, 0x031f, 0x0052, 0x0010, 0x0432, 0x003d, 0x0010,
    0x030c, 0x0054, 0x0010, 0x0421, 0x003e, 0x0010, 0x01f8, 0x0082, 0x0010, 0x0210, 0x007c, 0x0010, 0x030c, 0x0054, 0x0010, 0x0421, 0x003e, 0x0010,
    0x02fa, 0x0056, 0x0010, 0x0410, 0x003f, 0x0010, 0x01ec, 0x0085, 0x0010, 0x0208, 0x007e, 0x0010, 0x02fa, 0x0056, 0x0010, 0x0410, 0x003f, 0x0010,
    0x02e8, 0x0058, 0x0010, 0x0400, 0x0040, 0x0010, 0x01e1, 0x0088, 0x0010, 0x0200, 0x0080, 0x0010, 0x02e8, 0x0058, 0x0010, 0x0400, 0x0040, 0x0010,
    0x02d8, 0x005a, 0x0010, 0x03f0, 0x0041, 0x0010, 0x01d7, 0x008b, 0x0010, 0x01f8, 0x0082, 0x0010, 0x02d8, 0x005a, 0x0010, 0x03f0, 0x0041, 0x0010,
    0x02c8, 0x005c, 0x0010, 0x03e0, 0x0042, 0x0010, 0x01cd, 0x008e, 0x0010, 0x01f0, 0x0084, 0x0010, 0x02c8, 0x005c, 0x0010, 0x03e0, 0x0042, 0x0010,
    0x02b9, 0x005e, 0x0010, 0x03d2, 0x0043, 0x0010, 0x01c3, 0x0091, 0x0010, 0x01e9, 0x0086, 0x0010, 0x02b9, 0x005e, 0x0010, 0x03d2, 0x0043, 0x0010,
    0x02aa, 0x0060, 0x0010, 0x03c3, 0x0044, 0x0010, 0x01ba, 0x0094, 0x0010, 0x01e1, 0x0088, 0x0010, 0x02aa, 0x0060, 0x0010, 0x03c3, 0x0044, 0x0010,
    0x029c, 0x0062, 0x0010, 0x03b5, 0x0045, 0x0010, 0x01b2, 0x0097, 0x0010, 0x01da, 0x008a, 0x0010, 0x029c, 0x0062, 0x0010, 0x03b5, 0x0045, 0x0010,
    0x028f, 0x0064, 0x0010, 0x03a8, 0x0046, 0x0010, 0x01a6, 0x009b, 0x0010, 0x01d4, 0x008c, 0x0010, 0x028f, 0x0064, 0x0010, 0x03a8, 0x0046, 0x0010,
    0x0282, 0x0066, 0x0010, 0x039b, 0x0047, 0x0010, 0x019e, 0x009e, 0x0010, 0x01cd, 0x008e, 0x0010, 0x0282, 0x0066, 0x0010, 0x039b, 0x0047, 0x0010,
    0x0276, 0x0068, 0x0010, 0x038e, 0x0048, 0x0010, 0x0197, 0x00a1, 0x0010, 0x01c7, 0x0090, 0x0010, 0x0276, 0x0068, 0x0010, 0x038e, 0x0048, 0x0010,
    0x026a, 0x006a, 0x0010, 0x0381, 0x0049, 0x0010, 0x018f, 0x00a4, 0x0010, 0x01c0, 0x0092, 0x0010, 0x026a, 0x006a, 0x0010, 0x0381, 0x0049, 0x0010,
    0x025e, 0x006c, 0x0010, 0x0375, 0x004a, 0x0010, 0x0188, 0x00a7, 0x0010, 0x01ba, 0x0094, 0x0010, 0x025e, 0x006c, 0x0010, 0x0375, 0x004a, 0x0010,
    0x0253, 0x006e, 0x0010, 0x0369, 0x004b, 0x0010, 0x0181, 0x00aa, 0x0010, 0x01b4, 0x0096, 0x0010, 0x0253, 0x006e, 0x0010, 0x0369, 0x004b, 0x0010,
    0x0249, 0x0070, 0x0010, 0x035e, 0x004c, 0x0010, 0x017a, 0x00ad, 0x0010, 0x01af, 0x0098, 0x0010, 0x0249, 0x0070, 0x0010, 0x035e, 0x004c, 0x0010,
    0x023e, 0x0072, 0x0010, 0x035e, 0x004c, 0x0010, 0x0174, 0x00b0, 0x0010, 0x01af, 0x0098, 0x0010, 0x023e, 0x0072, 0x0010, 0x035e, 0x004c, 0x0010,
    0x0234, 0x0074, 0x0010, 0x0353, 0x004d, 0x0010, 0x016e, 0x00b3, 0x0010, 0x01a9, 0x009a, 0x0010, 0x0234, 0x0074, 0x0010, 0x0353, 0x004d, 0x0010,
    0x0226, 0x0077, 0x0010, 0x0348, 0x004e, 0x0010, 0x0164, 0x00b8, 0x0010, 0x01a4, 0x009c, 0x0010, 0x0226, 0x0077, 0x0010, 0x0348, 0x004e, 0x0010,
    0x0219, 0x007a, 0x0010, 0x033d, 0x004f, 0x0010, 0x015a, 0x00bd, 0x0010, 0x019e, 0x009e, 0x0010, 0x0219, 0x007a, 0x0010, 0x033d, 0x004f, 0x0010,
    0x020c, 0x007d, 0x0010, 0x0333, 0x0050, 0x0010, 0x0153, 0x00c1, 0x0010, 0x0199, 0x00a0, 0x0010, 0x020c, 0x007d, 0x0010, 0x0333, 0x0050, 0x0010,
    0x0200, 0x0080, 0x0010, 0x0329, 0x0051, 0x0010, 0x014a, 0x00c6, 0x0010, 0x0194, 0x00a2, 0x0010, 0x0200, 0x0080, 0x0010, 0x0329, 0x0051, 0x0010,
    0x01f4, 0x0083, 0x0010, 0x031f, 0x0052, 0x0010, 0x0142, 0x00cb, 0x0010, 0x018f, 0x00a4, 0x0010, 0x01f4, 0x0083, 0x0010, 0x031f, 0x0052, 0x0010,
    0x01e9, 0x0086, 0x0010, 0x0315, 0x0053, 0x0010, 0x013c, 0x00cf, 0x0010, 0x018a, 0x00a6, 0x0010, 0x01e9, 0x0086, 0x0010, 0x0315, 0x0053, 0x0010,
    0x01de, 0x0089, 0x0010, 0x030c, 0x0054, 0x0010, 0x0135, 0x00d4, 0x0010, 0x0186, 0x00a8, 0x0010, 0x01de, 0x0089, 0x0010, 0x030c, 0x0054, 0x0010,
    0x01d4, 0x008c, 0x0010, 0x0303, 0x0055, 0x0010, 0x012e, 0x00d9, 0x0010, 0x0181, 0x00aa, 0x0010, 0x01d4, 0x008c, 0x0010, 0x0303, 0x0055, 0x0010,
    0x01ca, 0x008f, 0x0010, 0x02fa, 0x0056, 0x0010, 0x0128, 0x00dd, 0x0010, 0x017d, 0x00ac, 0x0010, 0x01ca, 0x008f, 0x0010, 0x02fa, 0x0056, 0x0010,
    0x01c0, 0x0092, 0x0010, 0x02f1, 0x0057, 0x0010, 0x0121, 0x00e2, 0x0010, 0x0178, 0x00ae, 0x0010, 0x01c0, 0x0092, 0x0010, 0x02f1, 0x0057, 0x0010,
    0x01b7, 0x0095, 0x0010, 0x02e8, 0x0058, 0x0010, 0x011c, 0x00e6, 0x0010, 0x0174, 0x00b0, 0x0010, 0x01b7, 0x0095, 0x0010, 0x02e8, 0x0058, 0x0010,
    0x01af, 0x0098, 0x0010, 0x02e0, 0x0059, 0x0010, 0x0116, 0x00eb, 0x0010, 0x0170, 0x00b2, 0x0010, 0x01af, 0x0098, 0x0010, 0x02e0, 0x0059, 0x0010,
    0x01a6, 0x009b, 0x0010, 0x02d0, 0x005b, 0x0010, 0x0111, 0x00f0, 0x0010, 0x0168, 0x00b6, 0x0010, 0x01a6, 0x009b, 0x0010, 0x02d0, 0x005b, 0x0010,
    0x019e, 0x009e, 0x0010, 0x02c0, 0x005d, 0x0010, 0x010c, 0x00f4, 0x0010, 0x0160, 0x00ba, 0x0010, 0x019e, 0x009e, 0x0010, 0x02c0, 0x005d, 0x0010,
    0x0197, 0x00a1, 0x0010, 0x02b1, 0x005f, 0x0010, 0x0107, 0x00f9, 0x0010, 0x0158, 0x00be, 0x0010, 0x0197, 0x00a1, 0x0010, 0x02b1, 0x005f, 0x0010,
    0x018f, 0x00a4, 0x0010, 0x02aa, 0x0060, 0x0010, 0x0102, 0x00fe, 0x0010, 0x0155, 0x00c0, 0x0010, 0x018f, 0x00a4, 0x0010, 0x02aa, 0x0060, 0x0010,
    0x0188, 0x00a7, 0x0010, 0x029c, 0x0062, 0x0010, 0x00fe, 0x0102, 0x0010, 0x014e, 0x00c4, 0x0010, 0x0188, 0x00a7, 0x0010, 0x029c, 0x0062, 0x0010,
    0x0181, 0x00aa, 0x0010, 0x028f, 0x0064, 0x0010, 0x00f9, 0x0107, 0x0010, 0x0147, 0x00c8, 0x0010, 0x0181, 0x00aa, 0x0010, 0x028f, 0x0064, 0x0010,
    0x017a, 0x00ad, 0x0010, 0x0288, 0x0065, 0x0010, 0x00f4, 0x010c, 0x0010, 0x0144, 0x00ca, 0x0010, 0x017a, 0x00ad, 0x0010, 0x0288, 0x0065, 0x0010,
    0x0172, 0x00b1, 0x0010, 0x0282, 0x0066, 0x0010, 0x00ef, 0x0112, 0x0010, 0x0141, 0x00cc, 0x0010, 0x0172, 0x00b1, 0x0010, 0x0282, 0x0066, 0x0010,
    0x016a, 0x00b5, 0x0010, 0x0276, 0x0068, 0x0010, 0x00ea, 0x0118, 0x0010, 0x013b, 0x00d0, 0x0010, 0x016a, 0x00b5, 0x0010, 0x0276, 0x0068, 0x0010,
    0x0162, 0x00b9, 0x0010, 0x026a, 0x006a, 0x0010, 0x00e5, 0x011e, 0x0010, 0x0135, 0x00d4, 0x0010, 0x0162, 0x00b9, 0x0010, 0x026a, 0x006a, 0x0010,
    0x015a, 0x00bd, 0x0010, 0x025e, 0x006c, 0x0010, 0x00e0, 0x0124, 0x0010, 0x012f, 0x00d8, 0x0010, 0x015a, 0x00bd, 0x0010, 0x025e, 0x006c, 0x0010,
    0x0153, 0x00c1, 0x0010, 0x0253, 0x006e, 0x0010, 0x00db, 0x012b, 0x0010, 0x0129, 0x00dc, 0x0010, 0x0153, 0x00c1, 0x0010, 0x0253, 0x006e, 0x0010,
    0x014c, 0x00c5, 0x0010, 0x0249, 0x0070, 0x0010, 0x00d6, 0x0131, 0x0010, 0x0124, 0x00e0, 0x0010, 0x014c, 0x00c5, 0x0010, 0x0249, 0x0070, 0x0010,
    0x0146, 0x00c9, 0x0010, 0x023e, 0x0072, 0x0010, 0x00d2, 0x0137, 0x0010, 0x011f, 0x00e4, 0x0010, 0x0146, 0x00c9, 0x0010, 0x023e, 0x0072, 0x0010,
    0x013f, 0x00cd, 0x0010, 0x0234, 0x0074, 0x0010, 0x00ce, 0x013d, 0x0010, 0x011a, 0x00e8, 0x0010, 0x013f, 0x00cd, 0x0010, 0x0234, 0x0074, 0x0010,
    0x0139, 0x00d1, 0x0010, 0x022b, 0x0076, 0x0010, 0x00ca, 0x0143, 0x0010, 0x0115, 0x00ec, 0x0010, 0x0139, 0x00d1, 0x0010, 0x022b, 0x0076, 0x0010,
    0x0133, 0x00d5, 0x0010, 0x0219, 0x007a, 0x0010, 0x00c6, 0x014a, 0x0010, 0x010c, 0x00f4, 0x0010, 0x0133, 0x00d5, 0x0010, 0x0219, 0x007a, 0x0010,
    0x012e, 0x00d9, 0x0010, 0x0210, 0x007c, 0x0010, 0x00c3, 0x0150, 0x0010, 0x0108, 0x00f8, 0x0010, 0x012e, 0x00d9, 0x0010, 0x0210, 0x007c, 0x0010,
    0x0128, 0x00dd, 0x0010, 0x0208, 0x007e, 0x0010, 0x00bf, 0x0156, 0x0010, 0x0104, 0x00fc, 0x0010, 0x0128, 0x00dd, 0x0010, 0x0208, 0x007e, 0x0010,
    0x0123, 0x00e1, 0x0010, 0x0200, 0x0080, 0x0010, 0x00bc, 0x015c, 0x0010, 0x0100, 0x0100, 0x0010, 0x0123, 0x00e1, 0x0010, 0x0200, 0x0080, 0x0010,
    0x011e, 0x00e5, 0x0010, 0x01f8, 0x0082, 0x0010, 0x00b9, 0x0162, 0x0010, 0x00fc, 0x0104, 0x0010, 0x011e, 0x00e5, 0x0010, 0x01f8, 0x0082, 0x0010,
    0x0118, 0x00ea, 0x0010, 0x01f0, 0x0084, 0x0010, 0x00b5, 0x016a, 0x0010, 0x00f8, 0x0108, 0x0010, 0x0118, 0x00ea, 0x0010, 0x01f0, 0x0084, 0x0010,
    0x0112, 0x00ef, 0x0010, 0x01e9, 0x0086, 0x0010, 0x00b1, 0x0172, 0x0010, 0x00f4, 0x010c, 0x0010, 0x0112, 0x00ef, 0x0010, 0x01f0, 0x0084, 0x0010,
    0x010b, 0x00f5, 0x0010, 0x01e1, 0x0088, 0x0010, 0x00ac, 0x017b, 0x0010, 0x00f0, 0x0110, 0x0010, 0x010b, 0x00f5, 0x0010, 0x01f0, 0x0084, 0x0010,
    0x0107, 0x00f9, 0x0010, 0x01da, 0x008a, 0x0010, 0x00aa, 0x0181, 0x0010, 0x00ed, 0x0114, 0x0010, 0x0107, 0x00f9, 0x0010, 0x01f0, 0x0084, 0x0010,
    0x0102, 0x00fe, 0x0010, 0x01d4, 0x008c, 0x0010, 0x00a6, 0x0189, 0x0010, 0x00ea, 0x0118, 0x0010, 0x0102, 0x00fe, 0x0010, 0x01f0, 0x0084, 0x0010,
    0x00fd, 0x0103, 0x0010, 0x01ca, 0x008f, 0x0010, 0x00a3, 0x0191, 0x0010, 0x00e5, 0x011e, 0x0010, 0x00fd, 0x0103, 0x0010, 0x01f0, 0x0084, 0x0010,
    0x00f8, 0x0108, 0x0010, 0x01c3, 0x0091, 0x0010, 0x00a0, 0x0199, 0x0010, 0x00e1, 0x0122, 0x0010, 0x00f8, 0x0108, 0x0010, 0x01f0, 0x0084, 0x0010,
    0x00f3, 0x010d, 0x0010, 0x01ba, 0x0094, 0x0010, 0x009d, 0x01a0, 0x0010, 0x00dd, 0x0128, 0x0010, 0x00f3, 0x010d, 0x0010, 0x01f0, 0x0084, 0x0010,
    0x00ef, 0x0112, 0x0010, 0x01b2, 0x0097, 0x0010, 0x009a, 0x01a8, 0x0010, 0x00d9, 0x012e, 0x0010, 0x00ef, 0x0112, 0x0010, 0x01f0, 0x0084, 0x0010,
    0x00ea, 0x0117, 0x0010, 0x01a9, 0x009a, 0x0010, 0x0097, 0x01b0, 0x0010, 0x00d4, 0x0134, 0x0010, 0x00ea, 0x0117, 0x0010, 0x01f0, 0x0084, 0x0010,
    0x00e6, 0x011c, 0x0010, 0x01a1, 0x009d, 0x0010, 0x0094, 0x01b8, 0x0010, 0x00d0, 0x013a, 0x0010, 0x00e6, 0x011c, 0x0010, 0x01f0, 0x0084, 0x0010,
    0x00e6, 0x011c, 0x0010, 0x01a1, 0x009d, 0x0010, 0x0094, 0x01b8, 0x0010, 0x00d0, 0x013a, 0x0010, 0x00e6, 0x011c, 0x0010, 0x01f0, 0x0084, 0x0010,
    0x00e6, 0x011c, 0x0010, 0x01a1, 0x009d, 0x0010, 0x0094, 0x01b8, 0x0010, 0x00d0, 0x013a, 0x0010, 0x00e6, 0x011c, 0x0010, 0x01f0, 0x0084, 0x0010,
    0x00e6, 0x011c, 0x0010, 0x01a1, 0x009d, 0x0010, 0x0094, 0x01b8, 0x0010, 0x00d0, 0x013a, 0x0010, 0x00e6, 0x011c, 0x0010, 0x01f0, 0x0084, 0x0010,
    0x00e6, 0x011c, 0x0010, 0x01a1, 0x009d, 0x0010, 0x0094, 0x01b8, 0x0010, 0x00d0, 0x013a, 0x0010, 0x00e6, 0x011c, 0x0010, 0x01f0, 0x0084, 0x0010,
    0x00e6, 0x011c, 0x0010, 0x01a1, 0x009d, 0x0010, 0x0094, 0x01b8, 0x0010, 0x00d0, 0x013a, 0x0010, 0x00e6, 0x011c, 0x0010, 0x01f0, 0x0084, 0x0010,
    0x00e6, 0x011c, 0x0010, 0x01a1, 0x009d, 0x0010, 0x0094, 0x01b8, 0x0010, 0x00d0, 0x013a, 0x0010, 0x00e6, 0x011c, 0x0010, 0x01f0, 0x0084, 0x0010,
    0x00e6, 0x011c, 0x0010, 0x01a1, 0x009d, 0x0010, 0x0094, 0x01b8, 0x0010, 0x00d0, 0x013a, 0x0010, 0x00e6, 0x011c, 0x0010, 0x01f0, 0x0084, 0x0010,
    0x00e6, 0x011c, 0x0010, 0x01a1, 0x009d, 0x0010, 0x0094, 0x01b8, 0x0010, 0x00d0, 0x013a, 0x0010, 0x00e6, 0x011c, 0x0010, 0x01f0, 0x0084, 0x0010,
    0x00e6, 0x011c, 0x0010, 0x01a1, 0x009d, 0x0010, 0x0094, 0x01b8, 0x0010, 0x00d0, 0x013a, 0x0010, 0x00e6, 0x011c, 0x0010, 0x01f0, 0x0084, 0x0010,
    0x00e6, 0x011c, 0x0010, 0x01a1, 0x009d, 0x0010, 0x0094, 0x01b8, 0x0010, 0x00d0, 0x013a, 0x0010, 0x00e6, 0x011c, 0x0010, 0x01f0, 0x0084, 0x0010,
    0x00e6, 0x011c, 0x0010, 0x01a1, 0x009d, 0x0010, 0x0094, 0x01b8, 0x0010, 0x00d0, 0x013a, 0x0010, 0x00e6, 0x011c, 0x0010, 0x01f0, 0x0084, 0x0010,
    0x00e6, 0x011c, 0x0010, 0x01a1, 0x009d, 0x0010, 0x0094, 0x01b8, 0x0010, 0x00d0, 0x013a, 0x0010, 0x00e6, 0x011c, 0x0010, 0x01f0, 0x0084, 0x0010,
    0x00e6, 0x011c, 0x0010, 0x01a1, 0x009d, 0x0010, 0x0094, 0x01b8, 0x0010, 0x00d0, 0x013a, 0x0010, 0x00e6, 0x011c, 0x0010, 0x01f0, 0x0084, 0x0010,
    0x00e6, 0x011c, 0x0010, 0x01a1, 0x009d, 0x0010, 0x0094, 0x01b8, 0x0010, 0x00d0, 0x013a, 0x0010, 0x00e6, 0x011c, 0x0010, 0x01f0, 0x0084, 0x0010,
    0x00e6, 0x011c, 0x0010, 0x01a1, 0x009d, 0x0010, 0x0094, 0x01b8, 0x0010, 0x00d0, 0x013a, 0x0010, 0x00e6, 0x011c, 0x0010, 0x01f0, 0x0084, 0x0010,
    0x00e6, 0x011c, 0x0010, 0x01a1, 0x009d, 0x0010, 0x0094, 0x01b8, 0x0010, 0x00d0, 0x013a, 0x0010, 0x00e6, 0x011c, 0x0010, 0x01f0, 0x0084, 0x0010
};

MOS_STATUS CodechalEncodeVp8::InitMmcState()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;
#ifdef _MMC_SUPPORTED
    m_mmcState = MOS_New(CodechalMmcEncodeVp8, m_hwInterface, this);
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_mmcState);
#endif
    return MOS_STATUS_SUCCESS;
}

CodechalEncodeVp8::CodechalEncodeVp8(
    CodechalHwInterface*    hwInterface,
    CodechalDebugInterface* debugInterface,
    PCODECHAL_STANDARD_INFO standardInfo) :
    CodechalEncoderState(hwInterface, debugInterface, standardInfo)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_ASSERT(hwInterface);
    m_hwInterface = hwInterface;
    CODECHAL_ENCODE_ASSERT(m_hwInterface->GetOsInterface());
    m_osInterface = m_hwInterface->GetOsInterface();
    CODECHAL_ENCODE_ASSERT(m_hwInterface->GetMfxInterface());
    m_mfxInterface = m_hwInterface->GetMfxInterface();
    CODECHAL_ENCODE_ASSERT(m_hwInterface->GetHcpInterface());
    m_hcpInterface = m_hwInterface->GetHcpInterface();
    CODECHAL_ENCODE_ASSERT(m_hwInterface->GetHucInterface());
    m_hucInterface = m_hwInterface->GetHucInterface();
    CODECHAL_ENCODE_ASSERT(m_hwInterface->GetVdencInterface());
    m_vdencInterface = m_hwInterface->GetVdencInterface();
    CODECHAL_ENCODE_ASSERT(m_hwInterface->GetMiInterface());
    m_miInterface = m_hwInterface->GetMiInterface();
    CODECHAL_ENCODE_ASSERT(m_hwInterface->GetRenderInterface());
    CODECHAL_ENCODE_ASSERT(m_hwInterface->GetRenderInterface()->m_stateHeapInterface);
    m_stateHeapInterface = m_hwInterface->GetRenderInterface()->m_stateHeapInterface;

    bVMEKernelDump = 1;

    TpuKernelState = MHW_KERNEL_STATE();
    MpuKernelState = MHW_KERNEL_STATE();
    MeKernelState = MHW_KERNEL_STATE();

    for (uint8_t i = 0; i < CODECHAL_ENCODE_VP8_MBENC_IDX_NUM; i++)
    {
        MbEncKernelStates[i] = MHW_KERNEL_STATE();
    }
    for (uint8_t i = 0; i < CODECHAL_ENCODE_VP8_BRC_IDX_NUM; i++)
    {
        BrcKernelStates[i] = MHW_KERNEL_STATE();
    }

    MOS_ZeroMemory(&PicIdx, sizeof(PicIdx));
    MOS_ZeroMemory(&pRefList, sizeof(pRefList));
    MOS_ZeroMemory(&s4xMEMVDataBuffer, sizeof(s4xMEMVDataBuffer));
    MOS_ZeroMemory(&MbEncBindingTable, sizeof(MbEncBindingTable));
    MOS_ZeroMemory(&s4xMEDistortionBuffer, sizeof(s4xMEDistortionBuffer));
    MOS_ZeroMemory(&BrcBuffers, sizeof(BrcBuffers));

    /* VP8 uses a CM based down scale kernel */
    m_useCmScalingKernel = true;
    m_interlacedFieldDisabled = true;
    m_firstField = true;

    /* No field pictures in VP8 */
    m_verticalLineStride = CODECHAL_VLINESTRIDE_FRAME;
    m_verticalLineStrideOffset = CODECHAL_VLINESTRIDEOFFSET_TOP_FIELD;
    
    m_codecGetStatusReportDefined = true;
}

CodechalEncodeVp8::~CodechalEncodeVp8()
{
    FreeResources();
}

MOS_STATUS CodechalEncodeVp8::Initialize(PCODECHAL_SETTINGS codecHalSettings)
{
    MOS_STATUS status = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodechalEncoderState::Initialize(codecHalSettings));

    CODECHAL_ENCODE_CHK_NULL_RETURN(m_osInterface);
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_hwInterface);
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_miInterface);
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_stateHeapInterface);

    uint32_t numMBs = 0;

    // MVOffset should be 4KB aligned
    numMBs = m_picWidthInMb * m_picHeightInMb;    
    m_mvOffset = MOS_ALIGN_CEIL((((uint16_t)numMBs) * 16 * 4), CODECHAL_PAGE_SIZE); //for MB code
    m_mbCodeSize = m_mvOffset + (numMBs * 16 * sizeof(uint32_t));

    // for VP8: the Ds+Copy kernel is by default used to do CSC and copy non-aligned surface
    m_cscDsState->EnableCopy();
    m_cscDsState->EnableColor();

    MOS_USER_FEATURE_VALUE_DATA UserFeatureData;
    MOS_ZeroMemory(&UserFeatureData, sizeof(UserFeatureData));

    if (m_codecFunction != CODECHAL_FUNCTION_PAK)
    {
        MOS_ZeroMemory(&UserFeatureData, sizeof(UserFeatureData));
        MOS_UserFeature_ReadValue_ID(
            nullptr,
            __MEDIA_USER_FEATURE_VALUE_VP8_HW_SCOREBOARD_ENABLE_ID,
            &UserFeatureData);
        m_useHwScoreboard = (UserFeatureData.i32Data) ? true : false;

        // HME enabled by default for VP8
        MOS_ZeroMemory(&UserFeatureData, sizeof(UserFeatureData));
        MOS_UserFeature_ReadValue_ID(
            nullptr,
            __MEDIA_USER_FEATURE_VALUE_VP8_ENCODE_ME_ENABLE_ID,
            &UserFeatureData);

        m_hmeSupported = (UserFeatureData.i32Data) ? true : false;

        MOS_ZeroMemory(&UserFeatureData, sizeof(UserFeatureData));
        MOS_UserFeature_ReadValue_ID(
            nullptr,
            __MEDIA_USER_FEATURE_VALUE_VP8_ENCODE_16xME_ENABLE_ID,
            &UserFeatureData);

        m_16xMeSupported = (UserFeatureData.i32Data) ? true : false;

        // disable superHME when HME is disabled
        if (m_hmeSupported == false)
        {
            m_16xMeSupported = false;
        }

        // Repak enabled by default for VP8
        MOS_ZeroMemory(&UserFeatureData, sizeof(UserFeatureData));
        MOS_UserFeature_ReadValue_ID(
            nullptr,
            __MEDIA_USER_FEATURE_VALUE_VP8_ENCODE_REPAK_ENABLE_ID,
            &UserFeatureData);
        m_bRepakSupported = (UserFeatureData.i32Data) ? true : false;

        // Adaptive RePAK enabled by default
        bAdaptiveRepakSupported = true;
#if (_DEBUG || _RELEASE_INTERNAL)
        MOS_ZeroMemory(&UserFeatureData, sizeof(UserFeatureData));
        MOS_UserFeature_ReadValue_ID(
            nullptr,
            __MEDIA_USER_FEATURE_VALUE_VP8_ENCODE_ADAPTIVE_REPAK_ENABLE_ID,
            &UserFeatureData);
        bAdaptiveRepakSupported = (UserFeatureData.i32Data) ? true : false;
#endif // _DEBUG || _RELEASE_INTERNAL

        MOS_ZeroMemory(&UserFeatureData, sizeof(UserFeatureData));
        MOS_UserFeature_ReadValue_ID(
            nullptr,
            __MEDIA_USER_FEATURE_VALUE_VP8_ENCODE_MULTIPASS_BRC_ENABLE_ID,
            &UserFeatureData);
        m_multipassBrcSupported = (UserFeatureData.i32Data) ? true : false;
    }

    bBrcInit = true;

    MotionEstimationDisableCheck();
    
    if (CodecHalUsesRenderEngine(m_codecFunction, m_standard))
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(InitKernelState());
    }

    if (m_singleTaskPhaseSupported)
    {
        m_maxBtCount = GetMaxBtCount();
    }

    // Picture Level Commands
    m_hwInterface->GetMfxStateCommandsDataSize(
        CODECHAL_ENCODE_MODE_MPEG2,
        &m_pictureStatesSize,
        &m_picturePatchListSize,
        0);

    // Slice Level Commands (cannot be placed in 2nd level batch)
    m_hwInterface->GetMfxPrimitiveCommandsDataSize(
        CODECHAL_ENCODE_MODE_MPEG2,
        &m_sliceStatesSize,
        &m_slicePatchListSize,
        0);     

    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitMmcState());

    return status;
}

uint32_t CodechalEncodeVp8::GetMaxBtCount()
{
    auto btIdxAlignment = m_stateHeapInterface->pStateHeapInterface->GetBtIdxAlignment();
    uint32_t scalingBtCount = MOS_ALIGN_CEIL(
        m_scaling4xKernelStates[0].KernelParams.iBTCount,
        btIdxAlignment);
    uint32_t meBtCount = MOS_ALIGN_CEIL(
        m_meKernelStates[0].KernelParams.iBTCount,
        btIdxAlignment);
    uint32_t mbEncBtCount = MOS_ALIGN_CEIL(
        MbEncKernelStates[0].KernelParams.iBTCount,
        btIdxAlignment);

    uint32_t brcBtCount = 0;
    for (uint32_t i = 0; i < CODECHAL_ENCODE_VP8_BRC_IDX_NUM; i++)
    {
        brcBtCount += MOS_ALIGN_CEIL(
            BrcKernelStates[i].KernelParams.iBTCount,
            btIdxAlignment);
    }

   return MOS_MAX(scalingBtCount + meBtCount, mbEncBtCount + brcBtCount);
}

MOS_STATUS CodechalEncodeVp8::AllocateBuffer(
    PMOS_RESOURCE               buffer,
    uint32_t                    bufSize,
    const char *                name)
{
    MOS_STATUS status = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_NULL_RETURN(buffer);

    MOS_ALLOC_GFXRES_PARAMS allocParams;
    MOS_ZeroMemory(&allocParams, sizeof(MOS_ALLOC_GFXRES_PARAMS));
    allocParams.Type = MOS_GFXRES_BUFFER;
    allocParams.TileType = MOS_TILE_LINEAR;
    allocParams.Format = Format_Buffer;
    allocParams.dwBytes = bufSize;
    allocParams.pBufName = name; 

    status = (MOS_STATUS)m_osInterface->pfnAllocateResource(
        m_osInterface,
        &allocParams,
        buffer);

    if (status != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to allocate %s.", name);
        return status;
    }
    
    CodechalResLock bufLock(m_osInterface, buffer);
    auto data = bufLock.Lock(CodechalResLock::writeOnly);
    CODECHAL_ENCODE_CHK_NULL_RETURN(data);

    MOS_ZeroMemory(data, bufSize);

    return status;
}

MOS_STATUS CodechalEncodeVp8::AllocateBuffer2D(
    PMOS_SURFACE         surface,
    uint32_t             surfWidth,
    uint32_t             surfHeight,
    const char *         name)
{
    MOS_STATUS status = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_NULL_RETURN(surface);

    MOS_ZeroMemory(surface, sizeof(*surface));

    surface->TileType = MOS_TILE_LINEAR;
    surface->bArraySpacing = true;
    surface->Format = Format_Buffer_2D;
    surface->dwWidth = surfWidth;
    surface->dwHeight = surfHeight;
    surface->dwPitch = MOS_ALIGN_CEIL(surface->dwWidth, 64);

    MOS_ALLOC_GFXRES_PARAMS AllocParamsForBuffer2D;
    MOS_ZeroMemory(&AllocParamsForBuffer2D, sizeof(MOS_ALLOC_GFXRES_PARAMS));
    AllocParamsForBuffer2D.Type = MOS_GFXRES_2D;
    AllocParamsForBuffer2D.TileType = surface->TileType;
    AllocParamsForBuffer2D.Format = surface->Format;
    AllocParamsForBuffer2D.dwWidth = surface->dwWidth;
    AllocParamsForBuffer2D.dwHeight = surface->dwHeight;
    AllocParamsForBuffer2D.pBufName = name;

    status = (MOS_STATUS)m_osInterface->pfnAllocateResource(
        m_osInterface,
        &AllocParamsForBuffer2D,
        &surface->OsResource);

    if (status != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to allocate %s.", name);
        return status;
    }

    CodechalResLock bufLock(m_osInterface, &surface->OsResource);
    auto data = bufLock.Lock(CodechalResLock::writeOnly);
    CODECHAL_ENCODE_CHK_NULL_RETURN(data);

    MOS_ZeroMemory(data, surface->dwWidth * surface->dwHeight);

    return status;
}

MOS_STATUS CodechalEncodeVp8::AllocateBatchBuffer(
    PMHW_BATCH_BUFFER            batchBuffer,
    uint32_t                     bufSize,
    const char *                 name)
{
    MOS_STATUS status = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_NULL_RETURN(batchBuffer);
    
    MOS_ZeroMemory(
        batchBuffer,
        sizeof(MHW_BATCH_BUFFER));
    
    batchBuffer->bSecondLevel = true;
    
    status = Mhw_AllocateBb(
        m_osInterface,
        batchBuffer,
        nullptr,
        bufSize);
    
    if (status != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to allocate %s.", name);
        return status;
    }
    
     status = Mhw_LockBb(m_osInterface, batchBuffer);

    if (status != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to lock %s.", name);
        return status;
    }

    MOS_ZeroMemory(batchBuffer->pData, bufSize);

    status = Mhw_UnlockBb(
        m_osInterface,
        batchBuffer,
        false);

    if (status != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to unlock  %s.", name);
        return status;
    }

    return status;
}

MOS_STATUS CodechalEncodeVp8::AllocateResources()
{
    MOS_STATUS                              status = MOS_STATUS_SUCCESS;
    struct CodechalResourcesBrcParams       allocateResBrcParams;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodechalEncoderState::AllocateResources());

     // Allocate Ref Lists
    CodecHalAllocateDataList(
        pRefList,
        CODECHAL_NUM_UNCOMPRESSED_SURFACE_VP8);

    //Reference Frame MB count surface  32 bytes, 
    CODECHAL_ENCODE_CHK_STATUS_RETURN(
        AllocateBuffer(&resRefMbCountSurface, 
                        32, 
                        "Reference Frame MB count surface"));

    // MB Mode Cost Luma Buffer,  10 words
    CODECHAL_ENCODE_CHK_STATUS_RETURN(
        AllocateBuffer2D(&sMBModeCostLumaBuffer, 
                        MOS_ALIGN_CEIL((sizeof(uint16_t) * 10), 64), 
                        1,
                        "MBMode Cost Luma Buffer"));

    // Block Mode Cost Luma Buffer, 10*10*10 words
    CODECHAL_ENCODE_CHK_STATUS_RETURN(
        AllocateBuffer2D(&sBlockModeCostBuffer,
                        MOS_ALIGN_CEIL((sizeof(uint16_t)) * 10 * 10 * 10, 64), 
                        1, 
                        "BlockMode Cost Buffer"));

    // Chroma Recon Buffer, 64 bytes per MB
    CODECHAL_ENCODE_CHK_STATUS_RETURN(
        AllocateBuffer(&sChromaReconBuffer, 
                        64 * m_picWidthInMb * m_picHeightInMb, 
                        "Chroma Recon Buffer"));

    // Per-MB Quant data, 1 Dword per MB
    CODECHAL_ENCODE_CHK_STATUS_RETURN(
        AllocateBuffer2D(&sPerMBQuantDataBuffer, 
                        MOS_ALIGN_CEIL((m_picWidthInMb * 4), 64), 
                        m_picHeightInMb, 
                        "Per MB Quant Data Buffer"));

    ///pred mv data surface
    CODECHAL_ENCODE_CHK_STATUS_RETURN(
        AllocateBuffer(&resPredMvDataSurface, 
                        4 * m_picWidthInMb * m_picHeightInMb * sizeof(uint32_t), 
                        "Per MV data surface"));

    //ModeCostUpdate Surface used by P-kernel and MPU kernel
    CODECHAL_ENCODE_CHK_STATUS_RETURN(
        AllocateBuffer(&resModeCostUpdateSurface, 
                        16 * sizeof(uint32_t), 
                        "Mode Cost Update Surface"));
    
    if (m_encEnabled)
    {
        MOS_ZeroMemory(&allocateResBrcParams, sizeof(struct CodechalResourcesBrcParams));
        allocateResBrcParams.bHWWalker = m_hwWalker;
        allocateResBrcParams.dwDownscaledWidthInMB4x = m_downscaledWidthInMb4x;
        allocateResBrcParams.dwDownscaledHeightInMB4x = m_downscaledHeightInMb4x;
        allocateResBrcParams.dwFrameWidthInMB = m_picWidthInMb;
        allocateResBrcParams.dwFrameHeightInMB = m_picHeightInMb;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(AllocateBrcResources(&allocateResBrcParams));

        // allocate VME Kernel Dump buffer
        if (bVMEKernelDump)
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(
                AllocateBuffer(&resVmeKernelDumpBuffer, 
                                VP8_KERNEL_DUMP_SIZE, 
                                "VME Kernel Dump Buffer"));
        }

        if (m_hmeSupported)
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(
                AllocateBuffer2D(&s4xMEMVDataBuffer,  
                                m_downscaledWidthInMb4x * 32, 
                                m_downscaledHeightInMb4x * 4 * 4, 
                                "4xME MV Data Buffer"));

            CODECHAL_ENCODE_CHK_STATUS_RETURN(
                AllocateBuffer2D(&s4xMEDistortionBuffer,  
                                m_downscaledWidthInMb4x * 8,
                                m_downscaledHeightInMb4x * 4 * 4, 
                                "4xME Distortion Buffer"));
        }

        if (m_16xMeSupported)
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(
                AllocateBuffer2D(&s16xMEMVDataBuffer,  
                                MOS_ALIGN_CEIL(m_downscaledWidthInMb16x * 32, 64), 
                                m_downscaledHeightInMb16x * 4 * CODECHAL_VP8_ME_ME_DATA_SIZE_MULTIPLIER, 
                                "16xME MV Data Buffer"));           
        }

        CODECHAL_ENCODE_CHK_STATUS_RETURN(
            AllocateBuffer(&resHistogram, 
                            HISTOGRAM_SIZE, 
                            "Histogram"));
    }

    if (m_pakEnabled)
    {
        // Intra Row Store Scratch buffer
        // 1 cacheline per MB
        CODECHAL_ENCODE_CHK_STATUS_RETURN(
            AllocateBuffer(&resIntraRowStoreScratchBuffer, 
                            m_picWidthInMb * CODECHAL_CACHELINE_SIZE, 
                            "Intra Row Store Scratch Buffer"));     

        CODECHAL_ENCODE_CHK_STATUS_RETURN(
            AllocateBuffer(&resFrameHeader, 
                            CODECHAL_VP8_FRAME_HEADER_SIZE, 
                            "Frame Header buffer"));        

        CODECHAL_ENCODE_CHK_STATUS_RETURN(
            AllocateBuffer(&MpuTpuBuffers.resModeProbs, 
                            MODE_PROPABILITIES_SIZE, 
                            "Mode Probs buffer"));      

        CODECHAL_ENCODE_CHK_STATUS_RETURN(
            AllocateBuffer(&MpuTpuBuffers.resRefModeProbs, 
                            MODE_PROPABILITIES_SIZE, 
                            "Ref Mode Probs buffer"));      

        CODECHAL_ENCODE_CHK_STATUS_RETURN(
            AllocateBuffer(&MpuTpuBuffers.resCoeffProbs, 
                            COEFFS_PROPABILITIES_SIZE, 
                            "Coeff Probs buffer"));     

        CODECHAL_ENCODE_CHK_STATUS_RETURN(
            AllocateBuffer(&MpuTpuBuffers.resRefCoeffProbs, 
                            COEFFS_PROPABILITIES_SIZE, 
                            "Ref Coeff Probs buffer"));     

        CODECHAL_ENCODE_CHK_STATUS_RETURN(
            AllocateBuffer(&MpuTpuBuffers.resTokenBitsData, 
                            TOKEN_BITS_DATA_SIZE, 
                            "Token bits data"));        

        CODECHAL_ENCODE_CHK_STATUS_RETURN(
            AllocateBuffer(&MpuTpuBuffers.resPictureState, 
                            PICTURE_STATE_SIZE, 
                            "Picture state buffer"));       

        CODECHAL_ENCODE_CHK_STATUS_RETURN(
            AllocateBuffer(&MpuTpuBuffers.resMpuBitstream, 
                            MPU_BITSTREAM_SIZE, 
                            "Mpu bitstream buffer"));       

        CODECHAL_ENCODE_CHK_STATUS_RETURN(
            AllocateBuffer(&MpuTpuBuffers.resTpuBitstream, 
                            TPU_BITSTREAM_SIZE, 
                            "Tpu bitstream buffer"));       

        CODECHAL_ENCODE_CHK_STATUS_RETURN(
            AllocateBuffer(&MpuTpuBuffers.resEntropyCostTable, 
                            ENTROPY_COST_TABLE_SIZE, 
                            "Entropy cost table"));     

        CODECHAL_ENCODE_CHK_STATUS_RETURN(
            AllocateBuffer(&MpuTpuBuffers.resPakTokenStatistics, 
                            TOKEN_STATISTICS_SIZE, 
                            "Pak Token statistics"));   

        CODECHAL_ENCODE_CHK_STATUS_RETURN(
            AllocateBuffer(&MpuTpuBuffers.resPakTokenUpdateFlags, 
                            COEFFS_PROPABILITIES_SIZE, 
                            "Pak Token update flags"));  

        CODECHAL_ENCODE_CHK_STATUS_RETURN(
            AllocateBuffer(&MpuTpuBuffers.resDefaultTokenProbability, 
                            COEFFS_PROPABILITIES_SIZE, 
                            "Default Token Probability"));    


        CODECHAL_ENCODE_CHK_STATUS_RETURN(
            AllocateBuffer(&MpuTpuBuffers.resKeyFrameTokenProbability, 
                            COEFFS_PROPABILITIES_SIZE, 
                            "Key frame token probability"));      

        CODECHAL_ENCODE_CHK_STATUS_RETURN(
        AllocateBuffer(&MpuTpuBuffers.resUpdatedTokenProbability, 
                        COEFFS_PROPABILITIES_SIZE, 
                        "Updated token probability"));    

        CODECHAL_ENCODE_CHK_STATUS_RETURN(
        AllocateBuffer(&MpuTpuBuffers.resHwTokenProbabilityPass2, 
                        COEFFS_PROPABILITIES_SIZE, 
                        "Hw token probability pak Pass 2"));      

        CODECHAL_ENCODE_CHK_STATUS_RETURN(
        AllocateBuffer(&resPakIntermediateBuffer, 
                         (m_frameWidth  * m_frameHeight  * 2) + ((m_frameWidth * m_frameHeight) / 4) + INTERMEDIATE_PARTITION0_SIZE , 
                        "Intermediate buffer"));     

        CODECHAL_ENCODE_CHK_STATUS_RETURN(
        AllocateBuffer(&MpuTpuBuffers.resRepakDecisionSurface, 
                        REPAK_DECISION_BUF_SIZE, 
                        "Tpu Repak Decision buffer"));    

        CODECHAL_ENCODE_CHK_STATUS_RETURN(InitMpuTpuBuffer());
    }

    return status;
}

MOS_STATUS CodechalEncodeVp8::AllocateBrcResources(struct CodechalResourcesBrcParams* params)
{
    uint32_t                        size, width, height, i;
    MOS_ALLOC_GFXRES_PARAMS         allocParamsForBufferLinear;
    MOS_ALLOC_GFXRES_PARAMS         allocParamsForBuffer2D;
    MOS_LOCK_PARAMS                 lockFlagsWriteOnly;
    MOS_STATUS                      status = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    // initiate allocation paramters and lock flags
    MOS_ZeroMemory(&allocParamsForBufferLinear, sizeof(MOS_ALLOC_GFXRES_PARAMS));
    allocParamsForBufferLinear.Type = MOS_GFXRES_BUFFER;
    allocParamsForBufferLinear.TileType = MOS_TILE_LINEAR;
    allocParamsForBufferLinear.Format = Format_Buffer;

    MOS_ZeroMemory(&allocParamsForBuffer2D, sizeof(MOS_ALLOC_GFXRES_PARAMS));
    allocParamsForBuffer2D.Type = MOS_GFXRES_2D;
    allocParamsForBuffer2D.TileType = MOS_TILE_LINEAR;
    allocParamsForBuffer2D.Format = Format_Buffer_2D;

    MOS_ZeroMemory(&lockFlagsWriteOnly, sizeof(MOS_LOCK_PARAMS));
    lockFlagsWriteOnly.WriteOnly = 1;

    // BRC history buffer
    CODECHAL_ENCODE_CHK_STATUS_RETURN(
    AllocateBuffer(&BrcBuffers.resBrcHistoryBuffer, 
                    ENCODE_VP8_BRC_HISTORY_BUFFER_SIZE, 
                    "BRC History Buffer"));   

    // internal segment map is provided from BRC update kernel
    CODECHAL_ENCODE_CHK_STATUS_RETURN(
    AllocateBuffer2D(&sInSegmentMapSurface, 
                    MOS_ALIGN_CEIL(m_picWidthInMb, 4), 
                    m_picHeightInMb, 
                    "BRC Segment Map Surface"));      

    if (bBrcDistortionBufferSupported)
    {
        // BRC Distortion Surface
        CODECHAL_ENCODE_CHK_STATUS_RETURN(
            AllocateBuffer2D(&BrcBuffers.sMeBrcDistortionBuffer, 
                        MOS_ALIGN_CEIL((params->dwDownscaledWidthInMB4x * 8), 64), 
                        2 * MOS_ALIGN_CEIL((params->dwDownscaledHeightInMB4x * 4), 8), 
                        "BRC Distortion Surface Buffer"));    
    }

    // PAK Statistics buffer
    CODECHAL_ENCODE_CHK_STATUS_RETURN(
    AllocateBuffer(&BrcBuffers.resBrcPakStatisticBuffer[0], 
                    m_brcPakStatisticsSize,
                    "BRC PAK Statistics Buffer"));    

    // Encoder CFG State Cmd
    // Use BRC_MAXIMUM_NUM_PASSES here since actual number of passes is 
    // determined later using PicParams.BRCPrecision
    CODECHAL_ENCODE_CHK_STATUS_RETURN(
    AllocateBuffer(&BrcBuffers.resEncoderCfgCommandReadBuffer, 
                    BRC_IMG_STATE_SIZE_PER_PASS * CODECHAL_ENCODE_BRC_MAXIMUM_NUM_PASSES,
                    "Encoder CFG State Read Buffer"));    

    CODECHAL_ENCODE_CHK_STATUS_RETURN(
    AllocateBuffer(&BrcBuffers.resEncoderCfgCommandWriteBuffer, 
                    BRC_IMG_STATE_SIZE_PER_PASS * CODECHAL_ENCODE_BRC_MAXIMUM_NUM_PASSES,
                    "Encoder CFG State Write Buffer"));   

    // Check if the constant data surface is present
    if (bBrcConstantBufferSupported)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(
            AllocateBuffer(&BrcBuffers.resBrcConstantDataBuffer, 
                        BRC_CONSTANTSURFACE_VP8,
                        "BRC Constant Data Buffer"));     
    
        for (i = 0; i < CODECHAL_ENCODE_RECYCLED_BUFFER_NUM; i++)
        {
            //BRC Constant Data Surfaces
            CODECHAL_ENCODE_CHK_STATUS_RETURN(
                AllocateBuffer2D(&BrcBuffers.sBrcConstantDataBuffer[i], 
                            MOS_ALIGN_CEIL(dwBrcConstantSurfaceWidth, 64), 
                            dwBrcConstantSurfaceHeight, 
                            "BRC Constant Data Buffer"));     
        }
    }

    // PAK Statistics Dump buffer that stores stats per pak pass. useful for debugging
    // 12: 10 data buffers + 2 extra buffers for empty spaces (easier to read dumps)
    CODECHAL_ENCODE_CHK_STATUS_RETURN(
    AllocateBuffer(&BrcBuffers.resBrcPakStatsBeforeDumpBuffer, 
                    m_hwInterface->GetMfxInterface()->GetBrcNumPakPasses() * sizeof(uint32_t) * 12,
                    "BRC PAK Statistics Dump Buffer"));   

    // PAK Statistics Dump buffer that stores stats per pak pass. useful for debugging
    CODECHAL_ENCODE_CHK_STATUS_RETURN(
    AllocateBuffer(&BrcBuffers.resBrcPakStatsAfterDumpBuffer, 
                    m_hwInterface->GetMfxInterface()->GetBrcNumPakPasses() * sizeof(uint32_t) * 12,
                    "BRC PAK Statistics Init Dump Buffer"));      

    return status;
}

void CodechalEncodeVp8::FreeResources()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    CodechalEncoderState::FreeResources();

    // Release Ref Lists
    CodecHalFreeDataList(pRefList, CODECHAL_NUM_UNCOMPRESSED_SURFACE_VP8);

    m_osInterface->pfnFreeResource(
        m_osInterface,
        &resRefMbCountSurface);

    m_osInterface->pfnFreeResource(
        m_osInterface,
        &sMBModeCostLumaBuffer.OsResource);

    m_osInterface->pfnFreeResource(
        m_osInterface,
        &sBlockModeCostBuffer.OsResource);

    m_osInterface->pfnFreeResource(
        m_osInterface,
        &sChromaReconBuffer);

    m_osInterface->pfnFreeResource(
        m_osInterface,
        &sPerMBQuantDataBuffer.OsResource);

    m_osInterface->pfnFreeResource(
        m_osInterface,
        &resModeCostUpdateSurface);

    m_osInterface->pfnFreeResource(
        m_osInterface,
        &resPredMvDataSurface);

    FreeBrcResources();

    if (m_encEnabled)
    {
        if (bVMEKernelDump)
        {
            m_osInterface->pfnFreeResource(
                m_osInterface,
                &resVmeKernelDumpBuffer);
        }

        if (m_hmeSupported)
        {
            // 4xME ME MV data buffer
            m_osInterface->pfnFreeResource(
                m_osInterface,
                &s4xMEMVDataBuffer.OsResource);

            // 4xME distortion buffer
            m_osInterface->pfnFreeResource(
                m_osInterface,
                &s4xMEDistortionBuffer.OsResource);
        }

        // 16xME MV data buffer
        if (m_16xMeSupported)
        {
            m_osInterface->pfnFreeResource(
                m_osInterface,
                &s16xMEMVDataBuffer.OsResource);
        }

        m_osInterface->pfnFreeResource(
            m_osInterface,
            &resHistogram);
    }

    if (m_encEnabled)
    {
        m_osInterface->pfnFreeResource(
            m_osInterface,
            &sInSegmentMapSurface.OsResource);
    }

    if (m_pakEnabled)
    {
        m_osInterface->pfnFreeResource(
            m_osInterface,
            &resFrameHeader);

        m_osInterface->pfnFreeResource(
            m_osInterface,
            &MpuTpuBuffers.resModeProbs);

        m_osInterface->pfnFreeResource(
            m_osInterface,
            &MpuTpuBuffers.resRefModeProbs);

        m_osInterface->pfnFreeResource(
            m_osInterface,
            &MpuTpuBuffers.resCoeffProbs);

        m_osInterface->pfnFreeResource(
            m_osInterface,
            &MpuTpuBuffers.resRefCoeffProbs);

        m_osInterface->pfnFreeResource(
            m_osInterface,
            &MpuTpuBuffers.resTokenBitsData);

        m_osInterface->pfnFreeResource(
            m_osInterface,
            &MpuTpuBuffers.resPictureState);

        m_osInterface->pfnFreeResource(
            m_osInterface,
            &MpuTpuBuffers.resMpuBitstream);

        m_osInterface->pfnFreeResource(
            m_osInterface,
            &MpuTpuBuffers.resTpuBitstream);

        m_osInterface->pfnFreeResource(
            m_osInterface,
            &MpuTpuBuffers.resEntropyCostTable);

        m_osInterface->pfnFreeResource(
            m_osInterface,
            &MpuTpuBuffers.resPakTokenStatistics);

        m_osInterface->pfnFreeResource(
            m_osInterface,
            &MpuTpuBuffers.resPakTokenUpdateFlags);

        m_osInterface->pfnFreeResource(
            m_osInterface,
            &MpuTpuBuffers.resDefaultTokenProbability);

        m_osInterface->pfnFreeResource(
            m_osInterface,
            &MpuTpuBuffers.resKeyFrameTokenProbability);

        m_osInterface->pfnFreeResource(
            m_osInterface,
            &MpuTpuBuffers.resUpdatedTokenProbability);

        m_osInterface->pfnFreeResource(
            m_osInterface,
            &MpuTpuBuffers.resHwTokenProbabilityPass2);

        m_osInterface->pfnFreeResource(
            m_osInterface,
            &resPakIntermediateBuffer);

        m_osInterface->pfnFreeResource(
            m_osInterface,
            &resIntraRowStoreScratchBuffer);

        m_osInterface->pfnFreeResource(
            m_osInterface,
            &MpuTpuBuffers.resRepakDecisionSurface);

    }

    return;
}

void CodechalEncodeVp8::FreeBrcResources()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    for (uint32_t i = 0; i < CODECHAL_ENCODE_RECYCLED_BUFFER_NUM; i++)
    {
        m_osInterface->pfnFreeResource(
            m_osInterface,
            &BrcBuffers.sBrcConstantDataBuffer[i].OsResource);
    }

    m_osInterface->pfnFreeResource(
        m_osInterface,
        &BrcBuffers.resBrcPakStatisticBuffer[0]);

    m_osInterface->pfnFreeResource(
        m_osInterface,
        &BrcBuffers.sMeBrcDistortionBuffer.OsResource);

    m_osInterface->pfnFreeResource(
        m_osInterface,
        &BrcBuffers.resBrcHistoryBuffer);

    m_osInterface->pfnFreeResource(
        m_osInterface,
        &BrcBuffers.resBrcConstantDataBuffer);

    m_osInterface->pfnFreeResource(
        m_osInterface,
        &BrcBuffers.resBrcPakStatsBeforeDumpBuffer);

    m_osInterface->pfnFreeResource(
        m_osInterface,
        &BrcBuffers.resBrcPakStatsAfterDumpBuffer);

    m_osInterface->pfnFreeResource(
        m_osInterface,
        &BrcBuffers.resEncoderCfgCommandReadBuffer);

    m_osInterface->pfnFreeResource(
        m_osInterface,
        &BrcBuffers.resEncoderCfgCommandWriteBuffer);

    m_osInterface->pfnFreeResource(
        m_osInterface,
        &BrcBuffers.resPakQPInputTable);

    return;
}

void CodechalEncodeVp8::ResizeBuffer()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    // if resolution changed, free existing DS/CSC/MbCode/MvData resources
    m_trackedBuf->Resize();
    m_cscDsState->Resize();
}


//MOS_STATUS CodechalEncodeVp8::EncodeInitialize()
MOS_STATUS CodechalEncodeVp8::InitializePicture(const EncoderParams& params)
{
    MOS_STATUS status = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    m_bitstreamUpperBound = params.dwBitstreamSize;
    sMbSegmentMapSurface = *(params.psMbSegmentMapSurface);

    pVp8SeqParams = (PCODEC_VP8_ENCODE_SEQUENCE_PARAMS)(params.pSeqParams);
    pVp8PicParams = (PCODEC_VP8_ENCODE_PIC_PARAMS)(params.pPicParams);
    pVp8QuantData = (PCODEC_VP8_ENCODE_QUANT_DATA)(params.pQuantData);

    CODECHAL_ENCODE_CHK_NULL_RETURN(pVp8SeqParams);
    CODECHAL_ENCODE_CHK_NULL_RETURN(pVp8PicParams);
    CODECHAL_ENCODE_CHK_NULL_RETURN(pVp8QuantData);

    // reset for next frame
    if (b16xMeEnabled)
    {
        b16xMeDone = false;
    }
    if (bHmeEnabled)
    {
        bHmeDone = false;
    }

    if (m_newSeq)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(SetSequenceStructs());
    }
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetPictureStructs());

    // Scaling occurs when HME is enabled
    m_scalingEnabled = m_hmeSupported;
    m_useRawForRef = pVp8SeqParams->UseRawReconRef;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(
        SetStatusReportParams(pRefList[m_currReconstructedPic.FrameIdx]));

    CODECHAL_DEBUG_TOOL(
        m_debugInterface->CurrPic = pVp8PicParams->CurrOriginalPic;
        m_debugInterface->dwBufferDumpFrameNum = m_storeData;
        m_debugInterface->wFrameType = m_pictureCodingType;

        if (m_newSeq)
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(
                DumpVp8EncodeSeqParams(pVp8SeqParams));
        }

        CODECHAL_ENCODE_CHK_STATUS_RETURN(
            DumpVp8EncodePicParams(pVp8PicParams));
    )

    return status;
}

MOS_STATUS CodechalEncodeVp8::SetSequenceStructs()
{
    MOS_STATUS status = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    // setup internal parameters
    m_oriFrameWidth = pVp8SeqParams->FrameWidth;
    m_oriFrameHeight = pVp8SeqParams->FrameHeight;
    m_picWidthInMb = (uint16_t)CODECHAL_GET_WIDTH_IN_MACROBLOCKS(m_oriFrameWidth);
    m_picHeightInMb = (uint16_t)CODECHAL_GET_HEIGHT_IN_MACROBLOCKS(m_oriFrameHeight);
    m_frameWidth = m_picWidthInMb * CODECHAL_MACROBLOCK_WIDTH;
    m_frameHeight = m_picHeightInMb * CODECHAL_MACROBLOCK_HEIGHT;

    // HME Scaling WxH
    m_downscaledWidthInMb4x =
        CODECHAL_GET_WIDTH_IN_MACROBLOCKS(m_frameWidth / SCALE_FACTOR_4x);
    m_downscaledHeightInMb4x =
        CODECHAL_GET_HEIGHT_IN_MACROBLOCKS(m_frameHeight / SCALE_FACTOR_4x);
    m_downscaledWidth4x =
        m_downscaledWidthInMb4x * CODECHAL_MACROBLOCK_WIDTH;
    m_downscaledHeight4x =
        m_downscaledHeightInMb4x * CODECHAL_MACROBLOCK_HEIGHT;

    // SuperHME Scaling WxH
    m_downscaledWidthInMb16x =
        CODECHAL_GET_WIDTH_IN_MACROBLOCKS(m_frameWidth / SCALE_FACTOR_16x);
    m_downscaledHeightInMb16x =
        CODECHAL_GET_HEIGHT_IN_MACROBLOCKS(m_frameHeight / SCALE_FACTOR_16x);
    m_downscaledWidth16x =
        m_downscaledWidthInMb16x * CODECHAL_MACROBLOCK_WIDTH;
    m_downscaledHeight16x =
        m_downscaledHeightInMb16x * CODECHAL_MACROBLOCK_HEIGHT;

    MotionEstimationDisableCheck();

    m_kernelMode = CodecHal_TargetUsageToMode_VP8[pVp8SeqParams->TargetUsage & 0x7];

    // Disable 16xMe in performance mode
    if (m_16xMeSupported && (m_kernelMode == encodePerformanceMode))
    {
        m_16xMeSupported = false;
    }

    // All Rate control parameters currently not required for BDW and hence
    // not being read from Sequence params
    m_numPasses = 0;
    usMinPakPasses = 1;

    bRefCtrlOptimizationDone = false;

    if (m_firstFrame)
    {
        m_oriFrameHeight = pVp8SeqParams->FrameHeight;
        m_oriFrameWidth = pVp8SeqParams->FrameWidth;
    }

    // check if there is a dynamic resolution change
    if ((m_oriFrameHeight && (m_oriFrameHeight != pVp8SeqParams->FrameHeight)) ||
        (m_oriFrameWidth && (m_oriFrameWidth != pVp8SeqParams->FrameWidth)))
    {
        m_resolutionChanged = true;
        m_oriFrameHeight = pVp8SeqParams->FrameHeight;
        m_oriFrameWidth = pVp8SeqParams->FrameWidth;
    }
    else
    {
        m_resolutionChanged = false;
    }

    // if GOP structure is I-frame only, we use 3 non-ref slots for tracked buffer
    m_gopIsIdrFrameOnly = (pVp8SeqParams->GopPicSize == 1);

    return status;
}

MOS_STATUS CodechalEncodeVp8::SetPictureStructs()
{
    uint8_t         currRefIdx, numRef;
    uint16_t        brcPrecision;
    uint32_t        averageQp = 0;
    MOS_STATUS      status = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    if (Mos_ResourceIsNull(&m_reconSurface.OsResource) && (m_codecFunction != CODECHAL_FUNCTION_ENC))
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }

    // Sync initialize
    if ((m_firstFrame) ||
        (m_codecFunction == CODECHAL_FUNCTION_ENC))
    {
        m_waitForPak = false;
    }
    else
    {
        m_waitForPak= true;
    }

    if (m_codecFunction != CODECHAL_FUNCTION_ENC)
    {
        m_signalEnc = true;
    }
    else
    {
        m_signalEnc = false;
    }

    currRefIdx = pVp8PicParams->CurrReconstructedPic.FrameIdx;

    if (pVp8PicParams->segmentation_enabled)
    {
        for (uint8_t i = 0; i < CODECHAL_VP8_MAX_SEGMENTS; i++)
        {
            averageQp += CodecHal_Clip3(0, CODECHAL_VP8_QP_MAX, pVp8QuantData->QIndex[i] + pVp8QuantData->QIndexDelta[0]);
        }
        averageQp = averageQp / 4;
    }
    else
    {
        averageQp = CodecHal_Clip3(0, CODECHAL_VP8_QP_MAX, pVp8QuantData->QIndex[0] + pVp8QuantData->QIndexDelta[0]);
    }

    m_pictureCodingType = pVp8PicParams->frame_type ? P_TYPE : I_TYPE;

    if (m_pictureCodingType == I_TYPE)
    {
        dwAverageKeyFrameQp = averageQp;
        dwPFramePositionInGOP = 0;
    }
    else
    {
        dwAveragePFrameQp = averageQp;
        dwPFramePositionInGOP = (m_storeData - 1) % pVp8SeqParams->GopPicSize;
    }

    numRef = 0;
    if (!CodecHal_PictureIsInvalid(m_currOriginalPic))
    {
        if (!CodecHal_PictureIsInvalid(pVp8PicParams->LastRefPic))
        {
            pRefList[currRefIdx]->RefList[numRef++] = pVp8PicParams->LastRefPic;
        }

        if (!CodecHal_PictureIsInvalid(pVp8PicParams->GoldenRefPic))
        {
            pRefList[currRefIdx]->RefList[numRef++] = pVp8PicParams->GoldenRefPic;
        }

        if (!CodecHal_PictureIsInvalid(pVp8PicParams->AltRefPic))
        {
            pRefList[currRefIdx]->RefList[numRef++] = pVp8PicParams->AltRefPic;
        }
    }
    pRefList[currRefIdx]->ucNumRef = numRef;
    pRefList[currRefIdx]->bUsedAsRef = true; /* VP8 has no non reference pictures */
    m_currRefList = pRefList[currRefIdx];

    if (m_codecFunction == CODECHAL_FUNCTION_ENC_PAK)
    {
        // the actual MbCode/MvData surface to be allocated later
        m_trackedBuf->SetAllocationFlag(true);
    }
    else
    {
        CODECHAL_ENCODE_CHK_NULL_RETURN(m_encodeParams.presMbCodeSurface);
        m_resMbCodeSurface = *m_encodeParams.presMbCodeSurface;
        pRefList[currRefIdx]->resRefMbCodeBuffer = m_resMbCodeSurface;
    }

    pRefList[currRefIdx]->sRefReconBuffer = m_reconSurface;
    pRefList[currRefIdx]->sRefRawBuffer = m_rawSurface;
    pRefList[currRefIdx]->RefPic = pVp8PicParams->CurrOriginalPic;
    pRefList[currRefIdx]->resBitstreamBuffer = m_resBitstreamBuffer;

    m_currOriginalPic = pVp8PicParams->CurrOriginalPic;
    m_currReconstructedPic = pVp8PicParams->CurrReconstructedPic;
    m_frameFieldHeight = m_frameHeight;
    m_frameFieldHeightInMb = m_picHeightInMb;
    m_downscaledFrameFieldHeightInMb4x = m_downscaledHeightInMb4x;
    m_downscaledFrameFieldHeightInMb16x = m_downscaledHeightInMb16x;
    m_currEncBbSet = MB_ENC_Frame_BB;
    m_waitForPak = false;

    m_statusReportFeedbackNumber = pVp8PicParams->StatusReportFeedbackNumber;

    bHmeEnabled = m_hmeSupported && m_pictureCodingType != I_TYPE && (pVp8PicParams->ref_frame_ctrl != 0);
    b16xMeEnabled = m_16xMeSupported && m_pictureCodingType != I_TYPE;
    bMbEncIFrameDistEnabled =
        bBrcDistortionBufferSupported &&
        (m_pictureCodingType == I_TYPE);

    bBrcEnabled = (pVp8SeqParams->RateControlMethod == RATECONTROL_CBR || pVp8SeqParams->RateControlMethod == RATECONTROL_VBR);

    // Multi-Pass BRC
    // brcPrecision = 2, GetNumBrcPakPasses(2) = BRC_DEFAULT_NUM_PASSES = 4
    // brcPrecision = 1, GetNumBrcPakPasses(1) = BRC_MINIMUM_NUM_PASSES = 2
    // CQP: dwBrcNumPakPasses = 1
    // should be based on BRC precision but currenlty VP8 Encode DDI does not have anything for precision.
    brcPrecision = 1;
    m_hwInterface->GetMfxInterface()->SetBrcNumPakPasses(
        (bBrcEnabled && m_multipassBrcSupported) ? GetNumBrcPakPasses(brcPrecision) : 1);

    // Init Distortion buffer flag
    bInitBrcDistortionBuffer = false;

    if (bBrcEnabled)
    {
        if (m_pictureCodingType == I_TYPE)
        {
            bInitBrcDistortionBuffer = true;
        }
        else
        {
            //For P frame following I frame
            if (dwPFramePositionInGOP == 1)
            {
                bInitBrcDistortionBuffer = true;
            }
        }
    }


    //decide number of pak passes
    m_numPasses = 0;
    usMinPakPasses = 1;
    m_waitForPak = false;
    usRepakPassIterVal = 0;

    // RePak = 0: m_numPasses = 0, usMinPakPasses = 1
    // RePak = 1: m_numPasses = 1, usMinPakPasses = 2
    switch (m_kernelMode)
    {
    case encodeQualityMode:
        m_numPasses = 1;
        usMinPakPasses = 2;

        if (dwPFramePositionInGOP >= 1 && m_bRepakSupported)
        {
            m_waitForPak = true;
        }
        break;
    case encodeNormalMode:
        // with adaptive RePAK, RePAK is enabled by default both with I & P frames
        m_numPasses = 1;
        usMinPakPasses = 2;

        // needs to wait for PAK execution except 1st frame
        if (m_bRepakSupported && m_storeData > 1)
        {
            m_waitForPak = true;
        }
        break;
    case encodePerformanceMode:
        m_numPasses = 0;
        usMinPakPasses = 1;
        break;
    default:
        m_numPasses = 0;
        usMinPakPasses = 1;
    }

    //this will always override above decision on numpasses and minpakpasse, bRepakSupported is controlled through reg key
    if (!m_bRepakSupported)
    {
        m_numPasses = 0;
        usMinPakPasses = 1;
    }

    // Keep the code below for MultiPass BRC to be enabled in future
    if (bBrcEnabled)
    {
        if (m_multipassBrcSupported)   // Multi-Pass BRC
        {
            m_numPasses += GetNumBrcPakPasses(brcPrecision) - 1;
        }
    }

    if (m_bRepakSupported && usMinPakPasses > 1)  // Repak enabled by default for VP8
    {
        // Single-Pass BRC: ucNumPasses = 0 or 1

        // Multi-Pass BRC:  ucNumPasses = 3, Repak will always be 5th pass no matter how many PAK passes are executed
        // example) even if 2 passes are executed, there are PAK commands for 4 passes and cmds for remaining 2 passes are skipped
        usRepakPassIterVal = m_numPasses;
    }

    return status;
}

MOS_STATUS CodechalEncodeVp8::ExecuteKernelFunctions()
{
    MOS_SYNC_PARAMS                   syncParams;
    CodechalEncodeCscDs::KernelParams cscScalingKernelParams;
    bool                              isEncPhase1NotRun;
    MOS_STATUS                        status = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpYUVSurface(
            m_rawSurfaceToEnc,
            CodechalDbgAttr::attrEncodeRawInputSurface,
            "SrcSurf"))
    );

    MOS_ZeroMemory(&syncParams, sizeof(MOS_SYNC_PARAMS));
    // Wait on PAK, if its the P frame after I frame only
    if ((m_waitForPak) && !Mos_ResourceIsNull(&m_resSyncObjectVideoContextInUse))
    {
        syncParams = g_cInitSyncParams;
        syncParams.GpuContext = m_renderContext;
        syncParams.presSyncResource = &m_resSyncObjectVideoContextInUse;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnEngineWait(m_osInterface, &syncParams));
    }

    // BRC init/reset needs to be called before HME since it will reset the Brc Distortion surface
    if (bBrcEnabled)
    {
        bBrcReset = pVp8SeqParams->ResetBRC;
        if (bBrcInit || bBrcReset)
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(BrcInitResetKernel());
        }
    }

    // Csc, Downscaling, and/or 10-bit to 8-bit conversion
    MOS_ZeroMemory(&cscScalingKernelParams, sizeof(cscScalingKernelParams));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_cscDsState->KernelFunctions(&cscScalingKernelParams));

    if (bHmeEnabled)
    {
        if (b16xMeEnabled)
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(MeKernel());
        }

        CODECHAL_ENCODE_CHK_STATUS_RETURN(MeKernel());

        CODECHAL_DEBUG_TOOL(
            if (bHmeEnabled)
            {
                CODECHAL_ME_OUTPUT_PARAMS meOutputParams;
                MOS_ZeroMemory(&meOutputParams, sizeof(CODECHAL_ME_OUTPUT_PARAMS));
                meOutputParams.psMeMvBuffer = &s4xMEMVDataBuffer;
                meOutputParams.psMeBrcDistortionBuffer = bBrcEnabled ? &BrcBuffers.sMeBrcDistortionBuffer : nullptr;
                meOutputParams.psMeDistortionBuffer = &s4xMEDistortionBuffer;
                meOutputParams.b16xMeInUse = false;
                
                CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
                    &meOutputParams.psMeMvBuffer->OsResource,
                    CodechalDbgAttr::attrOutput,
                    "MvData",
                    meOutputParams.psMeMvBuffer->dwHeight * meOutputParams.psMeMvBuffer->dwPitch,
                    CodecHal_PictureIsBottomField(m_currOriginalPic) ? MOS_ALIGN_CEIL((m_downscaledWidthInMb4x * 32), 64) * (m_downscaledFrameFieldHeightInMb4x * 4) : 0,
                    CODECHAL_MEDIA_STATE_4X_ME));
                    
                if(bBrcEnabled)
                {
                    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
                        &meOutputParams.psMeBrcDistortionBuffer->OsResource,
                        CodechalDbgAttr::attrOutput,
                        "BrcDist",
                        meOutputParams.psMeBrcDistortionBuffer->dwHeight * meOutputParams.psMeBrcDistortionBuffer->dwPitch,
                        CodecHal_PictureIsBottomField(m_currOriginalPic) ? MOS_ALIGN_CEIL((m_downscaledWidthInMb4x * 8), 64) * MOS_ALIGN_CEIL((m_downscaledFrameFieldHeightInMb4x * 4), 8) : 0,
                        CODECHAL_MEDIA_STATE_4X_ME));
                }

                CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
                    &meOutputParams.psMeDistortionBuffer->OsResource,
                    CodechalDbgAttr::attrOutput,
                    "MeDist",
                    meOutputParams.psMeDistortionBuffer->dwHeight * meOutputParams.psMeDistortionBuffer->dwPitch,
                    CodecHal_PictureIsBottomField(m_currOriginalPic) ? MOS_ALIGN_CEIL((m_downscaledWidthInMb4x * 8), 64) * MOS_ALIGN_CEIL((m_downscaledFrameFieldHeightInMb4x * 4 * 10), 8) : 0,
                    CODECHAL_MEDIA_STATE_4X_ME));
                    
                if (b16xMeEnabled)
                {
                    meOutputParams.psMeMvBuffer = &s16xMEMVDataBuffer;
                    meOutputParams.psMeBrcDistortionBuffer = nullptr;
                    meOutputParams.psMeDistortionBuffer = nullptr;
                    meOutputParams.b16xMeInUse = true;

                    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
                        &meOutputParams.psMeMvBuffer->OsResource,
                        CodechalDbgAttr::attrOutput,
                        "MvData",
                        meOutputParams.psMeMvBuffer->dwHeight * meOutputParams.psMeMvBuffer->dwPitch,
                        CodecHal_PictureIsBottomField(m_currOriginalPic) ? MOS_ALIGN_CEIL((m_downscaledWidthInMb16x * 32), 64) * (m_downscaledFrameFieldHeightInMb16x * 4) : 0,
                        CODECHAL_MEDIA_STATE_16X_ME));
                }
            }
        )
    }

    // Call Idistortion and BRCUpdate kernels
    if (bBrcEnabled)
    {
        // Intra Distortion kernel
        if (bMbEncIFrameDistEnabled)
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(MbEncKernel(false, false, true));
        }

        CODECHAL_ENCODE_CHK_STATUS_RETURN(BrcUpdateKernel());
        m_osInterface->pfnResetPerfBufferID(m_osInterface);
    }


    // Reset after BRC Init has been processed
    bBrcInit = false;

    // skip Phase 1 for I frame with performance mode
    if ((m_pictureCodingType == I_TYPE) && (m_kernelMode == encodePerformanceMode))
    {
        isEncPhase1NotRun = true;
    }
    else
    {
        // Phase 1 MbEnc Kernel
        CODECHAL_ENCODE_CHK_STATUS_RETURN(MbEncKernel(false, false, false));
        isEncPhase1NotRun = false;
    }

    /* Call Phase 2 of I frame ENC kernel to process the Chroma component */
    if (m_pictureCodingType == I_TYPE)
    {
        // Phase 2 MbEnc Kernel
        CODECHAL_ENCODE_CHK_STATUS_RETURN(MbEncKernel(isEncPhase1NotRun, true, false));
    }

    /* Call MPU kernel */

    CODECHAL_ENCODE_CHK_STATUS_RETURN(MpuKernel());

    // send signal from render to video after MPU kernel execution
    if (!Mos_ResourceIsNull(&m_resSyncObjectRenderContextInUse))
    {
        syncParams = g_cInitSyncParams;
        syncParams.GpuContext = m_renderContext;
        syncParams.presSyncResource = &m_resSyncObjectRenderContextInUse;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnEngineSignal(m_osInterface, &syncParams));
    }

    if (bBrcEnabled)
    {
        bMbEncCurbeSetInBrcUpdate = false;
        bMbPakCurbeSetInBrcUpdate = false;
    }

    m_frameNum += 1;

    return status;
}

MOS_STATUS CodechalEncodeVp8::BrcInitResetKernel()
{
    PMHW_STATE_HEAP_INTERFACE stateHeapInterface;
    CODECHAL_MEDIA_STATE_TYPE encFunctionType;
    PMHW_KERNEL_STATE kernelState;
    MHW_INTERFACE_DESCRIPTOR_PARAMS idParams;
    struct CodechalVp8BrcInitResetCurbeParams    brcInitResetCurbeParams;
    MOS_COMMAND_BUFFER cmdBuffer;
    SendKernelCmdsParams sendKernelCmdsParams;
    struct CodechalVp8BrcInitResetSurfaceParams  brcInitResetSurfaceParams;
    MHW_MEDIA_OBJECT_PARAMS mediaObjectParams;
    PerfTagSetting perfTag;
    MOS_STATUS status = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(m_hwInterface->GetMiInterface());
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_hwInterface->GetRenderInterface());

    stateHeapInterface = m_hwInterface->GetRenderInterface()->m_stateHeapInterface;
    CODECHAL_ENCODE_CHK_NULL_RETURN(stateHeapInterface);

    perfTag.Value = 0;
    perfTag.Mode = (uint16_t)m_mode & CODECHAL_ENCODE_MODE_BIT_MASK;
    perfTag.CallType = CODECHAL_ENCODE_PERFTAG_CALL_BRC_INIT_RESET;
    perfTag.PictureCodingType = m_pictureCodingType;
    m_osInterface->pfnSetPerfTag(m_osInterface, perfTag.Value);

    encFunctionType = CODECHAL_MEDIA_STATE_BRC_INIT_RESET;

    kernelState = bBrcInit ?
        &BrcKernelStates[CODECHAL_ENCODE_VP8_BRC_IDX_INIT] :
        &BrcKernelStates[CODECHAL_ENCODE_VP8_BRC_IDX_RESET];

    CODECHAL_ENCODE_CHK_STATUS_RETURN(stateHeapInterface->pfnRequestSshSpaceForCmdBuf(
        stateHeapInterface,
        kernelState->KernelParams.iBTCount));
    m_vmeStatesSize =
        m_hwInterface->GetKernelLoadCommandSize(kernelState->KernelParams.iBTCount);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(VerifySpaceAvailable());

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->AssignDshAndSshSpace(
        stateHeapInterface,
        kernelState,
        false,
        0,
        false,
        m_storeData));

    MOS_ZeroMemory(&idParams, sizeof(idParams));
    idParams.pKernelState = kernelState;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(stateHeapInterface->pfnSetInterfaceDescriptor(
        stateHeapInterface,
        1,
        &idParams));

    MOS_ZeroMemory(&brcInitResetCurbeParams, sizeof(brcInitResetCurbeParams));
    brcInitResetCurbeParams.CurrPic = m_currOriginalPic;
    brcInitResetCurbeParams.pPicParams = pVp8PicParams;
    brcInitResetCurbeParams.pSeqParams = pVp8SeqParams;
    brcInitResetCurbeParams.dwFrameWidth = m_frameWidth;
    brcInitResetCurbeParams.dwFrameHeight = m_frameHeight;
    brcInitResetCurbeParams.dwAVBRAccuracy = usAVBRAccuracy;
    brcInitResetCurbeParams.dwAVBRConvergence = usAVBRConvergence;
    brcInitResetCurbeParams.bInitBrc = bBrcInit;
    brcInitResetCurbeParams.bMbBrcEnabled = bMbBrcEnabled;
    brcInitResetCurbeParams.pdBrcInitCurrentTargetBufFullInBits =
        &dBrcInitCurrentTargetBufFullInBits;
    brcInitResetCurbeParams.pdwBrcInitResetBufSizeInBits =
        &dwBrcInitResetBufSizeInBits;
    brcInitResetCurbeParams.pdBrcInitResetInputBitsPerFrame =
        &dBrcInitResetInputBitsPerFrame;
    brcInitResetCurbeParams.dwFramerate = uiFrameRate;
    brcInitResetCurbeParams.pKernelState = kernelState;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetBrcInitResetCurbe(&brcInitResetCurbeParams));

    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpKernelRegion(
            encFunctionType,
            MHW_DSH_TYPE,
            kernelState));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpCurbe(
            encFunctionType,
            kernelState));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpKernelRegion(
            encFunctionType,
            MHW_ISH_TYPE,
            kernelState));
    )

    MOS_ZeroMemory(&cmdBuffer, sizeof(cmdBuffer));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnGetCommandBuffer(m_osInterface, &cmdBuffer, 0));

    sendKernelCmdsParams = SendKernelCmdsParams();
    sendKernelCmdsParams.EncFunctionType = encFunctionType;
    sendKernelCmdsParams.bBrcResetRequested = bBrcReset ? true : false;
    sendKernelCmdsParams.pKernelState = kernelState;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(SendGenericKernelCmds(&cmdBuffer, &sendKernelCmdsParams));
    
    // Add binding table
    CODECHAL_ENCODE_CHK_STATUS_RETURN(stateHeapInterface->pfnSetBindingTable(
        stateHeapInterface,
        kernelState));

    //Add surface states
    MOS_ZeroMemory(&brcInitResetSurfaceParams, sizeof(brcInitResetSurfaceParams));
    brcInitResetSurfaceParams.presBrcHistoryBuffer =
        &BrcBuffers.resBrcHistoryBuffer;
    brcInitResetSurfaceParams.psMeBrcDistortionBuffer =
        &BrcBuffers.sMeBrcDistortionBuffer;
    brcInitResetSurfaceParams.dwMeBrcDistortionBottomFieldOffset =
        BrcBuffers.dwMeBrcDistortionBottomFieldOffset;
    brcInitResetSurfaceParams.dwDownscaledWidthInMb4x = m_downscaledWidthInMb4x;
    brcInitResetSurfaceParams.dwDownscaledFrameHeightInMb4x =
        m_downscaledHeightInMb4x;
    brcInitResetSurfaceParams.pKernelState = kernelState;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SendBrcInitResetSurfaces(
        &cmdBuffer,
        &brcInitResetSurfaceParams));

    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpKernelRegion(
            encFunctionType,
            MHW_SSH_TYPE,
            kernelState));
    )

    MOS_ZeroMemory(&mediaObjectParams, sizeof(mediaObjectParams));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->GetRenderInterface()->AddMediaObject(
        &cmdBuffer,
        nullptr,
        &mediaObjectParams));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(EndStatusReport(&cmdBuffer, encFunctionType));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(stateHeapInterface->pfnSubmitBlocks(
        stateHeapInterface,
        kernelState));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(stateHeapInterface->pfnUpdateGlobalCmdBufId(
        stateHeapInterface));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->GetMiInterface()->AddMiBatchBufferEnd(&cmdBuffer, nullptr));

    CODECHAL_DEBUG_TOOL(CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpCmdBuffer(
        &cmdBuffer,
        encFunctionType,
        nullptr)));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(
        m_hwInterface->UpdateSSEuForCmdBuffer(&cmdBuffer, m_singleTaskPhaseSupported, m_lastTaskInPhase));

    m_osInterface->pfnReturnCommandBuffer(m_osInterface, &cmdBuffer, 0);

    m_osInterface->pfnSubmitCommandBuffer(m_osInterface, &cmdBuffer, m_renderContextUsesNullHw);

    m_lastTaskInPhase = false;

    return status;
}

MOS_STATUS CodechalEncodeVp8::BrcUpdateKernel()
{
    PMHW_STATE_HEAP_INTERFACE                       stateHeapInterface;
    PMHW_KERNEL_STATE                               kernelState, mbEncKernelState, mbEncChromaKernelState;
    MHW_INTERFACE_DESCRIPTOR_PARAMS                 idParams;
    struct CodechalVp8MbencCurbeParams              mbEncCurbeParams;
    struct CodechalVp8MbpakCurbeParams              mbPakCurbeParams;
    struct CodechalVp8MpuCurbeParams                mpuCurbeParams;
    struct CodechalVp8TpuCurbeParams                tpuCurbeParams;
    struct CodechalVp8BrcUpdateCurbeParams          brcUpdateCurbeParams;
    struct CodechalVp8InitBrcConstantBufferParams   initBrcConstantBufferParams;
    PMOS_RESOURCE                                   resBrcImageStatesReadBuffer;
    struct CodechalVp8BrcUpdateSurfaceParams        brcUpdateSurfaceParams;
    struct CodechalBindingTableVp8BrcUpdate         bindingTable;
    CODECHAL_MEDIA_STATE_TYPE                       encFunctionType, mbEncFunctionType, mbPakFunctionType;
    SendKernelCmdsParams                            sendKernelCmdsParams;
    MHW_MEDIA_OBJECT_PARAMS                         mediaObjectParams;
    MOS_COMMAND_BUFFER                              cmdBuffer;
    PerfTagSetting                                  perfTag;
    MHW_VDBOX_VP8_ENCODER_CFG_PARAMS                encoderCfgParams;
    uint32_t                                        refFrameFlag, finalRefFrameFlag;
    MOS_LOCK_PARAMS                                 lockFlags;
    MOS_STATUS                                      status = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(m_hwInterface->GetMiInterface());
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_hwInterface->GetRenderInterface());

    stateHeapInterface = m_hwInterface->GetRenderInterface()->m_stateHeapInterface;
    CODECHAL_ENCODE_CHK_NULL_RETURN(stateHeapInterface);

    MOS_ZeroMemory(&perfTag, sizeof(PerfTagSetting));
    perfTag.Value = 0;
    perfTag.Mode = (uint16_t)m_mode & CODECHAL_ENCODE_MODE_BIT_MASK;
    perfTag.CallType = CODECHAL_ENCODE_PERFTAG_CALL_BRC_UPDATE;
    perfTag.PictureCodingType = m_pictureCodingType;
    m_osInterface->pfnSetPerfTag(m_osInterface, perfTag.Value);

    kernelState = &BrcKernelStates[CODECHAL_ENCODE_VP8_BRC_IDX_UPDATE];
    mbEncKernelState = nullptr;
    mbEncChromaKernelState = nullptr;
    if (m_pictureCodingType == I_TYPE)
    {
        mbEncKernelState = &MbEncKernelStates[CODECHAL_ENCODE_VP8_MBENC_IDX_I_LUMA];
        mbEncChromaKernelState = &MbEncKernelStates[CODECHAL_ENCODE_VP8_MBENC_IDX_I_CHROMA];
    }
    else
    {
        mbEncKernelState = &MbEncKernelStates[CODECHAL_ENCODE_VP8_MBENC_IDX_P];
        mbEncChromaKernelState = nullptr;
    }
    BrcBuffers.pMbEncKernelStateInUse = mbEncKernelState;

    encFunctionType = CODECHAL_MEDIA_STATE_BRC_UPDATE;
    mbPakFunctionType = CODECHAL_MEDIA_STATE_HYBRID_PAK_P1;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(stateHeapInterface->pfnRequestSshSpaceForCmdBuf(
        stateHeapInterface,
        kernelState->KernelParams.iBTCount));
    m_vmeStatesSize =
        m_hwInterface->GetKernelLoadCommandSize(kernelState->KernelParams.iBTCount);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(VerifySpaceAvailable());

    // Request space for MbEnc DSH, no SSH space requested as single task phase not supported for VP8
    // I frames require extra DSH space for the chroma ID
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->AssignDshAndSshSpace(
        stateHeapInterface,
        mbEncKernelState,
        false,
        mbEncChromaKernelState ? dwMbEncIFrameDshSize : 0,
        true,
        m_storeData));

    MOS_ZeroMemory(&idParams, sizeof(idParams));
    idParams.pKernelState = mbEncKernelState;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(stateHeapInterface->pfnSetInterfaceDescriptor(
        stateHeapInterface,
        1,
        &idParams));

    if (mbEncChromaKernelState)
    {
        mbEncChromaKernelState->m_dshRegion = mbEncKernelState->m_dshRegion;
        MOS_ZeroMemory(&idParams, sizeof(idParams));
        idParams.pKernelState = mbEncChromaKernelState;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(stateHeapInterface->pfnSetInterfaceDescriptor(
            stateHeapInterface,
            1,
            &idParams));
    }

    MOS_ZeroMemory(&mbEncCurbeParams, sizeof(struct CodechalVp8MbencCurbeParams));
    mbEncCurbeParams.pPicParams = pVp8PicParams;
    mbEncCurbeParams.pSeqParams = pVp8SeqParams;
    mbEncCurbeParams.pVp8QuantData = pVp8QuantData;
    mbEncCurbeParams.pVp8SliceParams = pVp8SliceParams;
    mbEncCurbeParams.ppRefList = &(pRefList[0]);
    mbEncCurbeParams.wPicWidthInMb = m_picWidthInMb;
    mbEncCurbeParams.wFieldFrameHeightInMb = m_frameFieldHeightInMb;
    mbEncCurbeParams.ucKernelMode = m_kernelMode;
    mbEncCurbeParams.bHmeEnabled = bHmeEnabled;
    mbEncCurbeParams.bVmeKernelDump = bVMEKernelDump;
    mbEncCurbeParams.wPictureCodingType = m_pictureCodingType;
    mbEncCurbeParams.bMbEncIFrameDistInUse = false;
    mbEncCurbeParams.bBrcEnabled = bBrcEnabled;
    mbEncCurbeParams.pCurrOriginalPic = &m_currOriginalPic;
    mbEncCurbeParams.pLastRefPic = &pVp8PicParams->LastRefPic;
    mbEncCurbeParams.pGoldenRefPic = &pVp8PicParams->GoldenRefPic;
    mbEncCurbeParams.pAlternateRefPic = &pVp8PicParams->AltRefPic;
    mbEncCurbeParams.bBrcEnabled = true;
    mbEncCurbeParams.pKernelState = mbEncKernelState;

    //driver and kernel optimization when multiref is supported
    refFrameFlag = 0;
    if (m_pictureCodingType == P_TYPE)
    {
        refFrameFlag = 0x07;
        if (pVp8PicParams->LastRefPic.FrameIdx == pVp8PicParams->GoldenRefPic.FrameIdx)
        {
            refFrameFlag &= ~VP8_GOLDEN_REF_FLAG;
        }
        if (pVp8PicParams->LastRefPic.FrameIdx == pVp8PicParams->AltRefPic.FrameIdx)
        {
            refFrameFlag &= ~VP8_ALT_REF_FLAG;
        }
        if (pVp8PicParams->GoldenRefPic.FrameIdx == pVp8PicParams->AltRefPic.FrameIdx)
        {
            refFrameFlag &= ~VP8_ALT_REF_FLAG;
        }
    }
    else
    {
        refFrameFlag = 1;
    }

    finalRefFrameFlag = 0;
    switch (pVp8PicParams->ref_frame_ctrl)
    {
    case 0:
        finalRefFrameFlag = 0;
        break;
    case 1:
        finalRefFrameFlag = 1; //Last Ref only
        break;
    case 2:
        finalRefFrameFlag = 2; //Gold Ref only
        break;
    case 4:
        finalRefFrameFlag = 4; //Alt Ref only
        break;
    default:
        finalRefFrameFlag = refFrameFlag;
    }

    pVp8PicParams->ref_frame_ctrl = finalRefFrameFlag;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetMbEncCurbe(&mbEncCurbeParams));

    bMbEncCurbeSetInBrcUpdate = true;

    // Request space for MPU DSH, no SSH space requested as single task phase not supported for VP8
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->AssignDshAndSshSpace(
        stateHeapInterface,
        &MpuKernelState,
        false,
        0,
        true,
        m_storeData));

    MOS_ZeroMemory(&idParams, sizeof(idParams));
    idParams.pKernelState = &MpuKernelState;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(stateHeapInterface->pfnSetInterfaceDescriptor(
        stateHeapInterface,
        1,
        &idParams));

    // Setup MPU Curbe
    MOS_ZeroMemory(&mpuCurbeParams, sizeof(struct CodechalVp8MpuCurbeParams));
    mpuCurbeParams.pPicParams = pVp8PicParams;
    mpuCurbeParams.pSeqParams = pVp8SeqParams;
    mpuCurbeParams.pVp8QuantData = pVp8QuantData;
    mpuCurbeParams.ucKernelMode = m_kernelMode;
    mpuCurbeParams.bVmeKernelDump = bVMEKernelDump;
    mpuCurbeParams.wPictureCodingType = m_pictureCodingType;
    mpuCurbeParams.EncFunctionType = encFunctionType;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetMpuCurbe(&mpuCurbeParams));

    bMpuCurbeSetInBrcUpdate = true;

    // Request space for TPU DSH, no SSH space requested as single task phase not supported for VP8
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->AssignDshAndSshSpace(
        stateHeapInterface,
        &TpuKernelState,
        false,
        0,
        true,
        m_storeData));

    MOS_ZeroMemory(&idParams, sizeof(idParams));
    idParams.pKernelState = &TpuKernelState;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(stateHeapInterface->pfnSetInterfaceDescriptor(
        stateHeapInterface,
        1,
        &idParams));

    // Setup TPU Curbe for BRC
    MOS_ZeroMemory(&tpuCurbeParams, sizeof(struct CodechalVp8TpuCurbeParams));
    tpuCurbeParams.pPicParams = pVp8PicParams;
    tpuCurbeParams.pSeqParams = pVp8SeqParams;
    tpuCurbeParams.pVp8QuantData = pVp8QuantData;
    tpuCurbeParams.ucKernelMode = m_kernelMode;
    tpuCurbeParams.bVmeKernelDump = bVMEKernelDump;
    tpuCurbeParams.wPictureCodingType = m_pictureCodingType;
    tpuCurbeParams.EncFunctionType = encFunctionType;
    tpuCurbeParams.wPicWidthInMb = m_picWidthInMb;
    tpuCurbeParams.wFieldFrameHeightInMb = m_frameFieldHeightInMb;
    tpuCurbeParams.bRebinarizationFrameHdr = usRepakPassIterVal > 0 ? true : false;
    // Adaptive RePak can be enabled only when RePak is enabled
    tpuCurbeParams.bAdaptiveRePak = usRepakPassIterVal > 0 ? bAdaptiveRepakSupported : 0;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetTpuCurbe(&tpuCurbeParams));

    bTpuCurbeSetInBrcUpdate = true;

    // Request space for BRC DSH and SSH
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->AssignDshAndSshSpace(
        stateHeapInterface,
        kernelState,
        false,
        0,
        false,
        m_storeData));

    MOS_ZeroMemory(&idParams, sizeof(idParams));
    idParams.pKernelState = kernelState;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(stateHeapInterface->pfnSetInterfaceDescriptor(
        stateHeapInterface,
        1,
        &idParams));

    // Setup BRC Update Curbe
    MOS_ZeroMemory(&brcUpdateCurbeParams, sizeof(struct CodechalVp8BrcUpdateCurbeParams));
    brcUpdateCurbeParams.CurrPic = m_currOriginalPic;
    brcUpdateCurbeParams.pPicParams = pVp8PicParams;
    brcUpdateCurbeParams.pSeqParams = pVp8SeqParams;
    brcUpdateCurbeParams.pSliceParams = pVp8SliceParams;
    brcUpdateCurbeParams.pVp8QuantData = pVp8QuantData;
    brcUpdateCurbeParams.dwAVBRAccuracy = usAVBRAccuracy;
    brcUpdateCurbeParams.dwAVBRConvergence = usAVBRConvergence;
    brcUpdateCurbeParams.wPictureCodingType = m_pictureCodingType;
    brcUpdateCurbeParams.dwFrameWidthInMB = m_picWidthInMb;
    brcUpdateCurbeParams.dwFrameHeightInMB = m_picHeightInMb;
    brcUpdateCurbeParams.bHmeEnabled = bHmeEnabled;
    brcUpdateCurbeParams.bInitBrc = bBrcInit;
    brcUpdateCurbeParams.bUsedAsRef = pRefList[m_currReconstructedPic.FrameIdx]->bUsedAsRef;
    brcUpdateCurbeParams.ucKernelMode = m_kernelMode;
    brcUpdateCurbeParams.dwVp8BrcNumPakPasses = m_hwInterface->GetMfxInterface()->GetBrcNumPakPasses();
    brcUpdateCurbeParams.dwHeaderBytesInserted = m_headerBytesInserted;
    brcUpdateCurbeParams.wFrameNumber = m_frameNum;
    brcUpdateCurbeParams.dwBrcInitResetBufSizeInBits = dwBrcInitResetBufSizeInBits;
    brcUpdateCurbeParams.dBrcInitResetInputBitsPerFrame = dBrcInitResetInputBitsPerFrame;
    brcUpdateCurbeParams.pdBrcInitCurrentTargetBufFullInBits = &dBrcInitCurrentTargetBufFullInBits;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetBrcUpdateCurbe(
        &brcUpdateCurbeParams));

    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpKernelRegion(
            encFunctionType,
            MHW_DSH_TYPE,
            kernelState));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpCurbe(
            encFunctionType,
            kernelState));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpKernelRegion(
            encFunctionType,
            MHW_ISH_TYPE,
            kernelState));
    )

    MOS_ZeroMemory(&cmdBuffer, sizeof(MOS_COMMAND_BUFFER));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnGetCommandBuffer(m_osInterface, &cmdBuffer, 0));

    sendKernelCmdsParams = SendKernelCmdsParams();
    sendKernelCmdsParams.EncFunctionType = encFunctionType;
    sendKernelCmdsParams.pKernelState = kernelState;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(SendGenericKernelCmds(&cmdBuffer, &sendKernelCmdsParams));

    // Check if the constant data surface is present
    if (bBrcConstantBufferSupported)
    {
        MOS_ZeroMemory(&initBrcConstantBufferParams, sizeof(initBrcConstantBufferParams));
        initBrcConstantBufferParams.pOsInterface = m_osInterface;
        initBrcConstantBufferParams.pVp8PicIdx = &PicIdx[0];
        initBrcConstantBufferParams.resBrcConstantDataBuffer =
            BrcBuffers.resBrcConstantDataBuffer;
        initBrcConstantBufferParams.pPicParams = pVp8PicParams;
        initBrcConstantBufferParams.wPictureCodingType = m_pictureCodingType;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(InitBrcConstantBuffer(&initBrcConstantBufferParams));
    }

    //Set MFX_VP8_ENCODER_CFG command
    MOS_ZeroMemory(&encoderCfgParams, sizeof(encoderCfgParams));
    encoderCfgParams.bFirstPass = !(m_currPass);
    encoderCfgParams.bBRCEnabled = bBrcEnabled ? true : false;
    encoderCfgParams.dwCfgBufferSize = HEADER_METADATA_SIZE;
    encoderCfgParams.pEncodeVP8SeqParams = pVp8SeqParams;
    encoderCfgParams.pEncodeVP8PicParams = pVp8PicParams;
    encoderCfgParams.pEncodeVP8QuantData = pVp8QuantData;

    for (int i = 0; i < CODECHAL_ENCODE_BRC_MAXIMUM_NUM_PASSES; i++)
    {
        encoderCfgParams.dwCfgCmdOffset = i * HEADER_METADATA_SIZE;
        encoderCfgParams.bFirstPass = !i;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->GetMfxInterface()->InitMfxVp8EncoderCfgCmd(&BrcBuffers.resEncoderCfgCommandWriteBuffer, &encoderCfgParams));
    }

    bMfxEncoderConfigCommandInitialized = true;

    // Add binding table
    CODECHAL_ENCODE_CHK_STATUS_RETURN(stateHeapInterface->pfnSetBindingTable(
        stateHeapInterface,
        kernelState));

    if (pVp8PicParams->segmentation_enabled)
    {
        CodecHalGetResourceInfo(m_osInterface, &sMbSegmentMapSurface);
    }

    //Add surface states
    MOS_ZeroMemory(&brcUpdateSurfaceParams, sizeof(struct CodechalVp8BrcUpdateSurfaceParams));
    brcUpdateSurfaceParams.pMbEncKernelState = mbEncKernelState;
    brcUpdateSurfaceParams.presBrcHistoryBuffer = &BrcBuffers.resBrcHistoryBuffer;
    brcUpdateSurfaceParams.presBrcPakStatisticBuffer = &BrcBuffers.resBrcPakStatisticBuffer[0];
    brcUpdateSurfaceParams.presVp8PakQPInputTable = &BrcBuffers.resPakQPInputTable;
    brcUpdateSurfaceParams.presVp8EncoderCfgCommandReadBuffer = &BrcBuffers.resEncoderCfgCommandReadBuffer;
    brcUpdateSurfaceParams.presVp8EncoderCfgCommandWriteBuffer = &BrcBuffers.resEncoderCfgCommandWriteBuffer;
    brcUpdateSurfaceParams.wPictureCodingType = m_pictureCodingType;
    brcUpdateSurfaceParams.ps4xMeDistortionBuffer = &s4xMEDistortionBuffer;
    brcUpdateSurfaceParams.psMeBrcDistortionBuffer = &BrcBuffers.sMeBrcDistortionBuffer;
    brcUpdateSurfaceParams.presVp8BrcConstantDataBuffer = &BrcBuffers.resBrcConstantDataBuffer;
    brcUpdateSurfaceParams.dwDownscaledWidthInMb4x = m_downscaledWidthInMb4x;
    brcUpdateSurfaceParams.dwDownscaledFrameFieldHeightInMb4x = m_downscaledFrameFieldHeightInMb4x;
    brcUpdateSurfaceParams.bMbBrcEnabled = bMbBrcEnabled;
    brcUpdateSurfaceParams.dwBrcPakStatisticsSize = m_brcPakStatisticsSize;
    // Brc update kernel will always update internal segment map (sInSegmentMapSurface) regardless of MBRC = 0 or 1
    // However, MbEnc kernel will ignore internal segment map when MBRC = 0
    brcUpdateSurfaceParams.psSegmentationMap = &sInSegmentMapSurface;
    brcUpdateSurfaceParams.presMbCodeBuffer = &m_resMbCodeSurface;
    brcUpdateSurfaceParams.pKernelState = kernelState;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SendBrcUpdateSurfaces(
        &cmdBuffer,
        &brcUpdateSurfaceParams));

    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpKernelRegion(
            encFunctionType,
            MHW_SSH_TYPE,
            kernelState));
    )

    MOS_ZeroMemory(&mediaObjectParams, sizeof(mediaObjectParams));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->GetRenderInterface()->AddMediaObject(
        &cmdBuffer,
        nullptr,
        &mediaObjectParams));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(EndStatusReport(&cmdBuffer, encFunctionType));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(stateHeapInterface->pfnSubmitBlocks(
        stateHeapInterface,
        kernelState));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(stateHeapInterface->pfnUpdateGlobalCmdBufId(
        stateHeapInterface));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->GetMiInterface()->AddMiBatchBufferEnd(&cmdBuffer, nullptr));

    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpCmdBuffer(
            &cmdBuffer,
            encFunctionType,
            nullptr));
    )

    CODECHAL_ENCODE_CHK_STATUS_RETURN(
        m_hwInterface->UpdateSSEuForCmdBuffer(&cmdBuffer, m_singleTaskPhaseSupported, m_lastTaskInPhase));

    m_osInterface->pfnReturnCommandBuffer(m_osInterface, &cmdBuffer, 0);

    m_osInterface->pfnSubmitCommandBuffer(m_osInterface, &cmdBuffer, m_renderContextUsesNullHw);

    m_lastTaskInPhase = false;

    return status;
}

MOS_STATUS CodechalEncodeVp8::MbEncKernel(bool isEncPhase1NotRun, bool isEncPhase2, bool mbEncIFrameDistInUse)
{
    PMHW_STATE_HEAP_INTERFACE                       stateHeapInterface;
    PMHW_KERNEL_STATE                               kernelState, chromaKernelState;
    MHW_INTERFACE_DESCRIPTOR_PARAMS                 idParams;
    struct CodechalVp8MbencCurbeParams              mbEncCurbeParams;
    struct CodechalVp8MbencSurfaceParams            mbEncSurfaceParams;
    struct CodechalBindingTableVp8Mbenc             bindingTable;
    struct CodechalVp8InitMbencConstantBufferParams initMBEncConstantBufferParams;
    CODECHAL_MEDIA_STATE_TYPE                       encFunctionType;
    SendKernelCmdsParams                            sendKernelCmdsParams;
    MHW_WALKER_PARAMS                               walkerParams;
    CODECHAL_WALKER_CODEC_PARAMS                    walkerCodecParams;
    MOS_COMMAND_BUFFER                              cmdBuffer;
    uint8_t*                                        data;
    PerfTagSetting                                  perfTag;
    bool                                            use45DegreePattern;
    uint8_t                                         index, currScaledIdx;
    MOS_LOCK_PARAMS                                 lockFlags;
    uint32_t                                        resolutionX, resolutionY;
    uint32_t                                        refFrameFlag, finalRefFrameFlag;
    MOS_STATUS                                      status = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(m_hwInterface->GetMiInterface());
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_hwInterface->GetRenderInterface());
    CODECHAL_ENCODE_CHK_NULL_RETURN(pRefList);
    CODECHAL_ENCODE_CHK_NULL_RETURN(pVp8PicParams);

    stateHeapInterface = m_hwInterface->GetRenderInterface()->m_stateHeapInterface;
    CODECHAL_ENCODE_CHK_NULL_RETURN(stateHeapInterface);
    use45DegreePattern = false;
    chromaKernelState = nullptr;
    currScaledIdx = pRefList[m_currReconstructedPic.FrameIdx]->ucScalingIdx;

    perfTag.Value = 0;
    perfTag.Mode = (uint16_t)m_mode & CODECHAL_ENCODE_MODE_BIT_MASK;
    perfTag.CallType = CODECHAL_ENCODE_PERFTAG_CALL_MBENC_PHASE1_KERNEL;

    if (isEncPhase2 == true)
    {
        perfTag.CallType = CODECHAL_ENCODE_PERFTAG_CALL_MBENC_PHASE2_KERNEL;
    }

    if (mbEncIFrameDistInUse == true)
    {
        perfTag.CallType = CODECHAL_ENCODE_PERFTAG_CALL_INTRA_DIST;
    }

    perfTag.PictureCodingType = m_pictureCodingType;
    m_osInterface->pfnSetPerfTag(m_osInterface, perfTag.Value);

    if (m_pictureCodingType == I_TYPE)
    {
        kernelState = &MbEncKernelStates[CODECHAL_ENCODE_VP8_MBENC_IDX_I_LUMA];
        chromaKernelState = &MbEncKernelStates[CODECHAL_ENCODE_VP8_MBENC_IDX_I_CHROMA];
    }
    else
    {
        kernelState = &MbEncKernelStates[CODECHAL_ENCODE_VP8_MBENC_IDX_P];
    }

    if (mbEncIFrameDistInUse)
    {
        encFunctionType = CODECHAL_MEDIA_STATE_ENC_I_FRAME_DIST;
        kernelState = &BrcKernelStates[CODECHAL_ENCODE_VP8_BRC_IDX_IFRAMEDIST];
        chromaKernelState = nullptr;
    }
    else if (m_kernelMode == encodeNormalMode)
    {
        encFunctionType = CODECHAL_MEDIA_STATE_ENC_NORMAL;
    }
    else if (m_kernelMode == encodePerformanceMode)
    {
        encFunctionType = CODECHAL_MEDIA_STATE_ENC_PERFORMANCE;
    }
    else
    {
        encFunctionType = CODECHAL_MEDIA_STATE_ENC_QUALITY;
    }

    if (isEncPhase2 &&
        !mbEncIFrameDistInUse) // MB ENC I Chroma
    {
        encFunctionType = CODECHAL_MEDIA_STATE_ENC_I_FRAME_CHROMA;
        kernelState = &MbEncKernelStates[CODECHAL_ENCODE_VP8_MBENC_IDX_I_CHROMA];
    }

    bindingTable = MbEncBindingTable;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(stateHeapInterface->pfnRequestSshSpaceForCmdBuf(
        stateHeapInterface,
        kernelState->KernelParams.iBTCount));
    m_vmeStatesSize =
        m_hwInterface->GetKernelLoadCommandSize(kernelState->KernelParams.iBTCount);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(VerifySpaceAvailable());


    /* Both Phase 1 & 2 can use the same memory area for Curbe data, as they accept identical Curbe data.
    Same could have been done for the Surface states also but the SSH is part of the command buffer and right now we are using the different command buffer
    for Phase 1 & 2, so the setup of surface states in each command buffer is necessary */
    if ((isEncPhase2 == false) || (isEncPhase2 == true && isEncPhase1NotRun == true))    // Phase 1  OR  Phase 2 (when Phase 1 not executed) // MB ENC I2
    {
        if (bMbEncCurbeSetInBrcUpdate && !mbEncIFrameDistInUse)
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->AssignDshAndSshSpace(
                stateHeapInterface,
                kernelState,
                true,
                0,
                false,
                m_storeData));
        }
        else
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->AssignDshAndSshSpace(
                stateHeapInterface,
                kernelState,
                false,
                chromaKernelState ? dwMbEncIFrameDshSize : 0,
                false,
                m_storeData));

            MOS_ZeroMemory(&idParams, sizeof(idParams));
            idParams.pKernelState = kernelState;
            CODECHAL_ENCODE_CHK_STATUS_RETURN(stateHeapInterface->pfnSetInterfaceDescriptor(
                stateHeapInterface,
                1,
                &idParams));

            if (chromaKernelState)
            {
                chromaKernelState->m_dshRegion = kernelState->m_dshRegion;
                MOS_ZeroMemory(&idParams, sizeof(idParams));
                idParams.pKernelState = chromaKernelState;
                CODECHAL_ENCODE_CHK_STATUS_RETURN(stateHeapInterface->pfnSetInterfaceDescriptor(
                    stateHeapInterface,
                    1,
                    &idParams));
            }

            // Setup VP8 Curbe
            MOS_ZeroMemory(&mbEncCurbeParams, sizeof(struct CodechalVp8MbencCurbeParams));
            mbEncCurbeParams.pPicParams = pVp8PicParams;
            mbEncCurbeParams.pSeqParams = pVp8SeqParams;
            mbEncCurbeParams.pVp8SliceParams = pVp8SliceParams;
            mbEncCurbeParams.pVp8QuantData = pVp8QuantData;
            mbEncCurbeParams.ppRefList = &(pRefList[0]);
            mbEncCurbeParams.wPicWidthInMb = m_picWidthInMb;
            mbEncCurbeParams.wFieldFrameHeightInMb = m_frameFieldHeightInMb;
            mbEncCurbeParams.ucKernelMode = m_kernelMode;
            mbEncCurbeParams.bHmeEnabled = bHmeEnabled;
            mbEncCurbeParams.bVmeKernelDump = bVMEKernelDump;
            mbEncCurbeParams.wPictureCodingType = m_pictureCodingType;
            mbEncCurbeParams.bMbEncIFrameDistInUse = mbEncIFrameDistInUse;
            mbEncCurbeParams.pCurrOriginalPic = &m_currOriginalPic;
            mbEncCurbeParams.pLastRefPic = &pVp8PicParams->LastRefPic;
            mbEncCurbeParams.pGoldenRefPic = &pVp8PicParams->GoldenRefPic;
            mbEncCurbeParams.pAlternateRefPic = &pVp8PicParams->AltRefPic;
            mbEncCurbeParams.pKernelState = kernelState;

            if (!bRefCtrlOptimizationDone)
            {
                // Driver and kernel optimization when multiref is supported. 
                // With this optimization ref_frame_ctrl value is modified to 1 for first P frame in the GOP
                // and to 3 for the second P frame in the GOP when ref_frame_ctrl value sent by app is
                // 3, 5, 6 or 7.
                if (m_pictureCodingType == P_TYPE)
                {
                    refFrameFlag = 0x07;
                    if (pVp8PicParams->LastRefPic.FrameIdx == pVp8PicParams->GoldenRefPic.FrameIdx)
                    {
                        refFrameFlag &= ~VP8_GOLDEN_REF_FLAG;
                    }
                    if (pVp8PicParams->LastRefPic.FrameIdx == pVp8PicParams->AltRefPic.FrameIdx)
                    {
                        refFrameFlag &= ~VP8_ALT_REF_FLAG;
                    }
                    if (pVp8PicParams->GoldenRefPic.FrameIdx == pVp8PicParams->AltRefPic.FrameIdx)
                    {
                        refFrameFlag &= ~VP8_ALT_REF_FLAG;
                    }
                }
                else
                {
                    refFrameFlag = 1;
                }

                switch (pVp8PicParams->ref_frame_ctrl)
                {
                case 0:
                    finalRefFrameFlag = 0;
                    break;
                case 1:
                    finalRefFrameFlag = 1; //Last Ref only
                    break;
                case 2:
                    finalRefFrameFlag = 2; //Gold Ref only
                    break;
                case 4:
                    finalRefFrameFlag = 4; //Alt Ref only
                    break;
                default:
                    finalRefFrameFlag = refFrameFlag;
                }

                pVp8PicParams->ref_frame_ctrl = finalRefFrameFlag;
            }

            CODECHAL_ENCODE_CHK_STATUS_RETURN(SetMbEncCurbe(&mbEncCurbeParams));

            CODECHAL_DEBUG_TOOL(
                CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpKernelRegion(
                    encFunctionType,
                    MHW_DSH_TYPE,
                    kernelState));
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpCurbe(
                encFunctionType,
                kernelState));
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpKernelRegion(
                encFunctionType,
                MHW_ISH_TYPE,
                kernelState));
            )
        }


        if (m_pictureCodingType == P_TYPE)
        {
            if (!CodecHal_PictureIsInvalid(pVp8PicParams->LastRefPic))
            {
                index = pVp8PicParams->LastRefPic.FrameIdx;
                pRefList[index]->sRefBuffer = pVp8SeqParams->UseRawReconRef ?
                    pRefList[index]->sRefRawBuffer : pRefList[index]->sRefReconBuffer;
                CodecHalGetResourceInfo(m_osInterface, &pRefList[index]->sRefBuffer);
            }

            if (!CodecHal_PictureIsInvalid(pVp8PicParams->GoldenRefPic))
            {
                index = pVp8PicParams->GoldenRefPic.FrameIdx;
                pRefList[index]->sRefBuffer = pRefList[index]->sRefReconBuffer; /* Always Recon buffer for Golden ref pic */
                CodecHalGetResourceInfo(m_osInterface, &pRefList[index]->sRefBuffer);
            }

            if (!CodecHal_PictureIsInvalid(pVp8PicParams->AltRefPic))
            {
                index = pVp8PicParams->AltRefPic.FrameIdx;
                pRefList[index]->sRefBuffer = pRefList[index]->sRefReconBuffer; /* Always Recon buffer for Alternate ref pic */
                CodecHalGetResourceInfo(m_osInterface, &pRefList[index]->sRefBuffer);
            }
        }
        else if (m_pictureCodingType == I_TYPE)
        {
            initMBEncConstantBufferParams.pOsInterface = m_osInterface;
            initMBEncConstantBufferParams.sMBModeCostLumaBuffer = sMBModeCostLumaBuffer;
            initMBEncConstantBufferParams.sBlockModeCostBuffer = sBlockModeCostBuffer;
            initMBEncConstantBufferParams.presHistogram = &resHistogram;
            CODECHAL_ENCODE_CHK_STATUS_RETURN(InitMBEncConstantBuffer(&initMBEncConstantBufferParams));
        }
        if (bInitBrcDistortionBuffer && mbEncIFrameDistInUse)
        {
            InitBrcDistortionBuffer();
        }
    }

    if (pVp8PicParams->segmentation_enabled)
    {
        CodecHalGetResourceInfo(m_osInterface, &sMbSegmentMapSurface);
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnGetCommandBuffer(m_osInterface, &cmdBuffer, 0));

    sendKernelCmdsParams = SendKernelCmdsParams();
    sendKernelCmdsParams.EncFunctionType = encFunctionType;
    sendKernelCmdsParams.pKernelState = kernelState;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SendGenericKernelCmds(&cmdBuffer, &sendKernelCmdsParams));

    // Add binding table
    CODECHAL_ENCODE_CHK_STATUS_RETURN(stateHeapInterface->pfnSetBindingTable(
        stateHeapInterface,
        kernelState));

    if (CodecHalUsesVideoEngine(m_codecFunction) &&
        ((isEncPhase2 == false) || (isEncPhase2 == true && isEncPhase1NotRun == true)))   // Phase 1  OR  Phase 2 (when Phase 1 not executed)
    {
        /* zero histogram surface only once */
        MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
        lockFlags.WriteOnly = 1;
        data = (uint8_t*)m_osInterface->pfnLockResource(
            m_osInterface,
            &resHistogram,
            &lockFlags);
        CODECHAL_ENCODE_CHK_NULL_RETURN(data);

        MOS_ZeroMemory(data, HISTOGRAM_SIZE);

        m_osInterface->pfnUnlockResource(
            m_osInterface,
            &resHistogram);
    }

    //Add surface states
    MOS_ZeroMemory(&mbEncSurfaceParams, sizeof(struct CodechalVp8MbencSurfaceParams));
    mbEncSurfaceParams.MediaStateType = encFunctionType;
    mbEncSurfaceParams.ppRefList = &pRefList[0];
    mbEncSurfaceParams.wPictureCodingType = m_pictureCodingType;
    mbEncSurfaceParams.pCurrReconstructedPic = &m_currReconstructedPic;
    mbEncSurfaceParams.psCurrPicSurface = m_rawSurfaceToEnc;

    if (m_pictureCodingType == P_TYPE)
    {
        mbEncSurfaceParams.pLastRefPic = &pVp8PicParams->LastRefPic;
        mbEncSurfaceParams.pGoldenRefPic = &pVp8PicParams->GoldenRefPic;
        mbEncSurfaceParams.pAlternateRefPic = &pVp8PicParams->AltRefPic;
        mbEncSurfaceParams.ps4xMeMvDataBuffer = &s4xMEMVDataBuffer;
        mbEncSurfaceParams.ps4xMeDistortionBuffer = &s4xMEDistortionBuffer;
        mbEncSurfaceParams.presRefMbCountSurface = &resRefMbCountSurface;
        mbEncSurfaceParams.dwMvOffset = (uint32_t)m_mvOffset;
        mbEncSurfaceParams.presModeCostUpdateSurface = &resModeCostUpdateSurface;
    }
    else
    {
        mbEncSurfaceParams.psMBModeCostLumaBuffer = &sMBModeCostLumaBuffer;
        mbEncSurfaceParams.psBlockModeCostBuffer = &sBlockModeCostBuffer;
    }

    mbEncSurfaceParams.dwFrameWidthInMb = (uint32_t)m_picWidthInMb;
    mbEncSurfaceParams.dwFrameFieldHeightInMb = (uint32_t)m_frameFieldHeightInMb;
    mbEncSurfaceParams.dwOriFrameWidth = m_oriFrameWidth;
    mbEncSurfaceParams.dwOriFrameHeight = m_oriFrameHeight;
    mbEncSurfaceParams.dwVerticalLineStride = m_verticalLineStride;
    mbEncSurfaceParams.dwVerticalLineStrideOffset = m_verticalLineStrideOffset;
    mbEncSurfaceParams.dwHistogramSize = HISTOGRAM_SIZE;
    mbEncSurfaceParams.bHmeEnabled = bHmeEnabled;
    mbEncSurfaceParams.bVMEKernelDump = bVMEKernelDump;
    mbEncSurfaceParams.bSegmentationEnabled = pVp8PicParams->segmentation_enabled;
    mbEncSurfaceParams.presPerMB_MBCodeOpData = &m_resMbCodeSurface;
    mbEncSurfaceParams.psPerMBQuantDataBuffer = &sPerMBQuantDataBuffer;
    mbEncSurfaceParams.presVmeKernelDumpBuffer = &resVmeKernelDumpBuffer;
    // 0: Default, decided internally based on target usage.
    // 1: MB BRC enabled.
    // 2: MB BRC disabled.
    // MBRC = 1: internal segment map (sInSegmentMapSurface) is provided from BRC update kernel
    // MBRC = 0, 2: external segment map (sMbSegmentMapSurface) is provided from the app, ignore internal segment map
    mbEncSurfaceParams.psSegmentationMap = (pVp8SeqParams->MBBRC == 1) ? &sInSegmentMapSurface : &sMbSegmentMapSurface;
    mbEncSurfaceParams.presHistogram = &resHistogram;
    mbEncSurfaceParams.psInterPredictionDistortionSurface = &s4xMEDistortionBuffer;
    mbEncSurfaceParams.presPerMVDataSurface = &resPredMvDataSurface;
    mbEncSurfaceParams.bMbEncIFrameDistInUse = mbEncIFrameDistInUse;
    mbEncSurfaceParams.pVp8SliceParams = pVp8SliceParams;
    mbEncSurfaceParams.pTrackedBuffer = &m_trackedBuffer[0];
    mbEncSurfaceParams.psMeBrcDistortionBuffer = mbEncIFrameDistInUse ?
        &BrcBuffers.sMeBrcDistortionBuffer :
        &s4xMEDistortionBuffer;
    mbEncSurfaceParams.uiRefCtrl = pVp8PicParams->ref_frame_ctrl;
    mbEncSurfaceParams.pMbEncBindingTable = &bindingTable;
    mbEncSurfaceParams.pKernelState = kernelState;

    mbEncSurfaceParams.psChromaReconBuffer = &sChromaReconBuffer;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(SendMbEncSurfaces(&cmdBuffer, &mbEncSurfaceParams));

    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpKernelRegion(
            encFunctionType,
            MHW_SSH_TYPE,
            kernelState));
    )

    resolutionX = mbEncIFrameDistInUse ?
        m_downscaledWidthInMb4x : (uint32_t)m_picWidthInMb;
    resolutionY = mbEncIFrameDistInUse ?
        m_downscaledHeightInMb4x : (uint32_t)m_frameFieldHeightInMb;

    MOS_ZeroMemory(&walkerCodecParams, sizeof(walkerCodecParams));
    walkerCodecParams.WalkerMode = m_walkerMode;
    walkerCodecParams.bUseScoreboard = m_useHwScoreboard;
    walkerCodecParams.dwResolutionX = resolutionX;
    walkerCodecParams.dwResolutionY = resolutionY;
    walkerCodecParams.wPictureCodingType = m_pictureCodingType;

    if ((m_pictureCodingType == I_TYPE) && (isEncPhase2 == false))
    {
        walkerCodecParams.bNoDependency = true;
    }
    else
    {
        walkerCodecParams.WalkerDegree = CODECHAL_45_DEGREE;
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalInitMediaObjectWalkerParams(
        m_hwInterface,
        &walkerParams,
        &walkerCodecParams));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->GetRenderInterface()->AddMediaObjectWalkerCmd(
        &cmdBuffer,
        &walkerParams));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(EndStatusReport(&cmdBuffer, encFunctionType));


    if( mbEncIFrameDistInUse ||
        !isEncPhase2 ||
        (isEncPhase2 && isEncPhase1NotRun))
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(stateHeapInterface->pfnSubmitBlocks(
            stateHeapInterface,
            kernelState));
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(stateHeapInterface->pfnUpdateGlobalCmdBufId(
        stateHeapInterface));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->GetMiInterface()->AddMiBatchBufferEnd(&cmdBuffer, nullptr));

    CODECHAL_DEBUG_TOOL(CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpCmdBuffer(
        &cmdBuffer,
        encFunctionType,
        nullptr)));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(
        m_hwInterface->UpdateSSEuForCmdBuffer(&cmdBuffer, m_singleTaskPhaseSupported, m_lastTaskInPhase));

    m_osInterface->pfnReturnCommandBuffer(m_osInterface, &cmdBuffer, 0);

    CODECHAL_ENCODE_CHK_STATUS_RETURN(
        m_osInterface->pfnSubmitCommandBuffer(m_osInterface, &cmdBuffer, m_renderContextUsesNullHw));

    return status;
}

MOS_STATUS CodechalEncodeVp8::MeKernel()
{
    PMHW_STATE_HEAP_INTERFACE               stateHeapInterface;
    PMHW_KERNEL_STATE                       kernelState;
    MHW_INTERFACE_DESCRIPTOR_PARAMS         idParams;
    PerfTagSetting                          perfTag;
    struct CodechalVp8MeCurbeParams         meParams;
    struct CodechalVp8MeSurfaceParams       meSurfaceParams;
    struct CodechalBindingTableVp8Me        bindingTable;
    CODECHAL_MEDIA_STATE_TYPE               encFunctionType;
    SendKernelCmdsParams                    sendKernelCmdsParams;
    MHW_WALKER_PARAMS                       walkerParams;
    CODECHAL_WALKER_CODEC_PARAMS            walkerCodecParams;
    MOS_COMMAND_BUFFER                      cmdBuffer;
    bool                                    use16xMe;
    uint32_t                                resolutionX, resolutionY;
    uint8_t                                 refScaledIdx, goldScaledIdx, altrefScaledIdx;
    uint32_t                                refFrameFlag, finalRefFrameFlag;
    MOS_STATUS                              status = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(m_hwInterface->GetMiInterface());
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_hwInterface->GetRenderInterface());

    stateHeapInterface = m_hwInterface->GetRenderInterface()->m_stateHeapInterface;
    CODECHAL_ENCODE_CHK_NULL_RETURN(stateHeapInterface);
    use16xMe = b16xMeEnabled && !b16xMeDone;

    perfTag.Value = 0;
    perfTag.Mode = (uint16_t)m_mode >> CODECHAL_ENCODE_MODE_BIT_OFFSET;
    perfTag.CallType = CODECHAL_ENCODE_PERFTAG_CALL_ME_KERNEL;
    perfTag.PictureCodingType = m_pictureCodingType;
    m_osInterface->pfnSetPerfTag(m_osInterface, perfTag.Value);

    encFunctionType = (use16xMe) ? CODECHAL_MEDIA_STATE_16X_ME : CODECHAL_MEDIA_STATE_4X_ME;
    kernelState = &MeKernelState;
    bindingTable = MeBindingTable;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(stateHeapInterface->pfnRequestSshSpaceForCmdBuf(
        stateHeapInterface,
        kernelState->KernelParams.iBTCount));
    m_vmeStatesSize =
        m_hwInterface->GetKernelLoadCommandSize(kernelState->KernelParams.iBTCount);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(VerifySpaceAvailable());

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->AssignDshAndSshSpace(
        stateHeapInterface,
        kernelState,
        false,
        0,
        false,
        m_storeData));

    MOS_ZeroMemory(&idParams, sizeof(idParams));
    idParams.pKernelState = kernelState;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(stateHeapInterface->pfnSetInterfaceDescriptor(
        stateHeapInterface,
        1,
        &idParams));

    // Driver and kernel optimization when multiref is supported.  
    // With this optimization ref_frame_ctrl value is modified to 1 for first P frame in the GOP
    // and to 3 for the second P frame in the GOP when ref_frame_ctrl value sent by app is either
    // 3, 5, 6 or 7.
    if (m_pictureCodingType == P_TYPE)
    {
        refFrameFlag = 0x07;
        if (pVp8PicParams->LastRefPic.FrameIdx == pVp8PicParams->GoldenRefPic.FrameIdx)
        {
            refFrameFlag &= ~VP8_GOLDEN_REF_FLAG;
        }
        if (pVp8PicParams->LastRefPic.FrameIdx == pVp8PicParams->AltRefPic.FrameIdx)
        {
            refFrameFlag &= ~VP8_ALT_REF_FLAG;
        }
        if (pVp8PicParams->GoldenRefPic.FrameIdx == pVp8PicParams->AltRefPic.FrameIdx)
        {
            refFrameFlag &= ~VP8_ALT_REF_FLAG;
        }
    }
    else
    {
        refFrameFlag = 1;
    }

    switch (pVp8PicParams->ref_frame_ctrl)
    {
    case 0:
        finalRefFrameFlag = 0;
        break;
    case 1:
        finalRefFrameFlag = 1; //Last Ref only
        break;
    case 2:
        finalRefFrameFlag = 2; //Gold Ref only
        break;
    case 4:
        finalRefFrameFlag = 4; //Alt Ref only
        break;
    default:
        finalRefFrameFlag = refFrameFlag;
    }

    pVp8PicParams->ref_frame_ctrl = finalRefFrameFlag;
    bRefCtrlOptimizationDone = true;

    // Setup VP8 Curbe
    meParams.pPicParams = pVp8PicParams;
    meParams.pSeqParams = pVp8SeqParams;
    meParams.dwFrameWidth = m_frameWidth;
    meParams.dwFrameFieldHeight = m_frameFieldHeight;
    meParams.wPictureCodingType = m_pictureCodingType;
    meParams.b16xME = use16xMe;
    meParams.b16xMeEnabled = b16xMeEnabled;
    meParams.ucKernelMode = m_kernelMode;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetMeCurbe(&meParams));

    refScaledIdx = pRefList[pVp8PicParams->LastRefPic.FrameIdx]->ucScalingIdx;
    goldScaledIdx = pRefList[pVp8PicParams->GoldenRefPic.FrameIdx]->ucScalingIdx;
    altrefScaledIdx = pRefList[pVp8PicParams->AltRefPic.FrameIdx]->ucScalingIdx;

    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpKernelRegion(
            encFunctionType,
            MHW_DSH_TYPE,
            kernelState));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpCurbe(
        encFunctionType,
        kernelState));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpKernelRegion(
        encFunctionType,
        MHW_ISH_TYPE,
        kernelState));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpYUVSurface(
        &(m_trackedBuffer[refScaledIdx].sScaled4xSurface),
        CodechalDbgAttr::attrReferenceSurfaces,
        "MeRef4xScaledSurf"))

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpYUVSurface(
            &(m_trackedBuffer[goldScaledIdx].sScaled4xSurface),
            CodechalDbgAttr::attrReferenceSurfaces,
            "GoldenRef4xScaledSurf"))

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpYUVSurface(
            &(m_trackedBuffer[altrefScaledIdx].sScaled4xSurface),
            CodechalDbgAttr::attrReferenceSurfaces,
            "AltRef4xScaledSurf"))
        )

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnGetCommandBuffer(m_osInterface, &cmdBuffer, 0));

    sendKernelCmdsParams = SendKernelCmdsParams();
    sendKernelCmdsParams.EncFunctionType = encFunctionType;
    sendKernelCmdsParams.pKernelState = kernelState;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SendGenericKernelCmds(&cmdBuffer, &sendKernelCmdsParams));

    // Add binding table
    CODECHAL_ENCODE_CHK_STATUS_RETURN(stateHeapInterface->pfnSetBindingTable(
        stateHeapInterface,
        kernelState));

    if (bInitBrcDistortionBuffer && !use16xMe)
    {
        InitBrcDistortionBuffer();
    }

    //Add surface states
    MOS_ZeroMemory(&meSurfaceParams, sizeof(struct CodechalVp8MeSurfaceParams));
    meSurfaceParams.ppRefList = &pRefList[0];
    meSurfaceParams.pLastRefPic = &pVp8PicParams->LastRefPic;
    meSurfaceParams.pGoldenRefPic = &pVp8PicParams->GoldenRefPic;
    meSurfaceParams.pAlternateRefPic = &pVp8PicParams->AltRefPic;
    meSurfaceParams.pCurrReconstructedPic = &m_currReconstructedPic;
    meSurfaceParams.ps4xMeMvDataBuffer = &s4xMEMVDataBuffer;
    meSurfaceParams.ps16xMeMvDataBuffer = &s16xMEMVDataBuffer;
    meSurfaceParams.pTrackedBuffer = &m_trackedBuffer[0];
    meSurfaceParams.psMeDistortionBuffer = &s4xMEDistortionBuffer;
    // Me kernel is only for P frame, no need to worry about I frame
    meSurfaceParams.psMeBrcDistortionBuffer = bBrcEnabled ?
        &BrcBuffers.sMeBrcDistortionBuffer :
        &s4xMEDistortionBuffer;
    meSurfaceParams.dwDownscaledWidthInMb =
        (use16xMe) ? m_downscaledWidthInMb16x : m_downscaledWidthInMb4x;
    meSurfaceParams.dwDownscaledHeightInMb =
        (use16xMe) ? m_downscaledHeightInMb16x : m_downscaledHeightInMb4x;
    meSurfaceParams.dwVerticalLineStride = m_verticalLineStride;
    meSurfaceParams.dwVerticalLineStrideOffset = m_verticalLineStrideOffset;
    meSurfaceParams.b16xMeInUse = use16xMe;
    meSurfaceParams.b16xMeEnabled = b16xMeEnabled;
    meSurfaceParams.RefCtrl = pVp8PicParams->ref_frame_ctrl;
    meSurfaceParams.pMeBindingTable = &bindingTable;
    meSurfaceParams.pKernelState = kernelState;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SendMeSurfaces(&cmdBuffer, &meSurfaceParams));

    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpKernelRegion(
            encFunctionType,
            MHW_SSH_TYPE,
            kernelState));
    )

    resolutionX = (use16xMe) ? m_downscaledWidthInMb16x : m_downscaledWidthInMb4x;
    resolutionY = (use16xMe) ? m_downscaledFrameFieldHeightInMb16x : m_downscaledFrameFieldHeightInMb4x;

    MOS_ZeroMemory(&walkerCodecParams, sizeof(walkerCodecParams));
    walkerCodecParams.WalkerMode = m_walkerMode;
    walkerCodecParams.dwResolutionX = resolutionX;
    walkerCodecParams.dwResolutionY = resolutionY;
    walkerCodecParams.bNoDependency = true;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalInitMediaObjectWalkerParams(
        m_hwInterface,
        &walkerParams,
        &walkerCodecParams));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->GetRenderInterface()->AddMediaObjectWalkerCmd(
        &cmdBuffer,
        &walkerParams));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(EndStatusReport(&cmdBuffer, encFunctionType));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(stateHeapInterface->pfnSubmitBlocks(
        stateHeapInterface,
        kernelState));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(stateHeapInterface->pfnUpdateGlobalCmdBufId(
        stateHeapInterface));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->GetMiInterface()->AddMiBatchBufferEnd(&cmdBuffer, nullptr));

    CODECHAL_DEBUG_TOOL(CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpCmdBuffer(
        &cmdBuffer,
        encFunctionType,
        nullptr)));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->UpdateSSEuForCmdBuffer(&cmdBuffer, m_singleTaskPhaseSupported, m_lastTaskInPhase));

    m_osInterface->pfnReturnCommandBuffer(m_osInterface, &cmdBuffer, 0);

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnSubmitCommandBuffer(m_osInterface, &cmdBuffer, m_renderContextUsesNullHw));

    if (use16xMe)
    {
        b16xMeDone = true;
    }

    return status;
}

MOS_STATUS CodechalEncodeVp8::MpuKernel()
{
    PMHW_STATE_HEAP_INTERFACE                       stateHeapInterface;
    PMHW_KERNEL_STATE                               kernelState;
    MHW_INTERFACE_DESCRIPTOR_PARAMS                 idParams;
    struct CodechalVp8MpuCurbeParams                mpuCurbeParams;
    struct CodechalVp8MpuSurfaceParams              mpuSurfaceParams;
    struct CodechalBindingTableVp8Mpu               bindingTable;
    SendKernelCmdsParams                            sendKernelCmdsParams;
    MHW_WALKER_PARAMS                               walkerParams;
    CODECHAL_WALKER_CODEC_PARAMS                    walkerCodecParams;
    MOS_COMMAND_BUFFER                              cmdBuffer;
    CODECHAL_MEDIA_STATE_TYPE                       encFunctionType;
    MHW_VDBOX_VP8_ENCODER_CFG_PARAMS                encoderCfgParams;
    struct CodechalVp8UpdateMpuTpuBufferParams      updateMpuTpuBuffersParams;
    MHW_MEDIA_OBJECT_PARAMS                         mediaObjectParams;
    MOS_STATUS                                      status = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(m_hwInterface->GetMiInterface());
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_hwInterface->GetRenderInterface());
    CODECHAL_ENCODE_CHK_NULL_RETURN(pRefList);

    stateHeapInterface = m_hwInterface->GetRenderInterface()->m_stateHeapInterface;
    CODECHAL_ENCODE_CHK_NULL_RETURN(stateHeapInterface);

    encFunctionType = CODECHAL_MEDIA_STATE_MPU_FHB;
    kernelState = &MpuKernelState;
    bindingTable = MpuBindingTable;

    //Update all the data for the key frames
    if (m_pictureCodingType == I_TYPE)
    {
        MOS_ZeroMemory(&updateMpuTpuBuffersParams, sizeof(updateMpuTpuBuffersParams));
        updateMpuTpuBuffersParams.pOsInterface = m_osInterface;
        updateMpuTpuBuffersParams.dwCoeffProbsSize = COEFFS_PROPABILITIES_SIZE;
        updateMpuTpuBuffersParams.presCurrFrameTokenProbability = &MpuTpuBuffers.resCoeffProbs;
        updateMpuTpuBuffersParams.presHwTokenProbabilityPass1 = &MpuTpuBuffers.resRefCoeffProbs;
        updateMpuTpuBuffersParams.presKeyFrameTokenProbability = &MpuTpuBuffers.resKeyFrameTokenProbability;
        updateMpuTpuBuffersParams.presHwTokenProbabilityPass2 = &MpuTpuBuffers.resHwTokenProbabilityPass2;
        updateMpuTpuBuffersParams.presRepakDecisionSurface = &MpuTpuBuffers.resRepakDecisionSurface;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(KeyFrameUpdateMpuTpuBuffer(&updateMpuTpuBuffersParams));
    }

    if (!bMfxEncoderConfigCommandInitialized)
    {
        //Set MFX_VP8_ENCODER_CFG
        MOS_ZeroMemory(&encoderCfgParams, sizeof(encoderCfgParams));
        encoderCfgParams.bFirstPass = !(m_currPass);
        encoderCfgParams.bBRCEnabled = bBrcEnabled ? true : false;
        encoderCfgParams.dwCfgCmdOffset = HEADER_METADATA_OFFSET;
        encoderCfgParams.dwCfgBufferSize = PICTURE_STATE_SIZE;
        encoderCfgParams.pEncodeVP8SeqParams = pVp8SeqParams;
        encoderCfgParams.pEncodeVP8PicParams = pVp8PicParams;
        encoderCfgParams.pEncodeVP8QuantData = pVp8QuantData;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->GetMfxInterface()->InitMfxVp8EncoderCfgCmd(&MpuTpuBuffers.resPictureState, &encoderCfgParams));
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(stateHeapInterface->pfnRequestSshSpaceForCmdBuf(
        stateHeapInterface,
        kernelState->KernelParams.iBTCount));
    m_vmeStatesSize =
        m_hwInterface->GetKernelLoadCommandSize(kernelState->KernelParams.iBTCount);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(VerifySpaceAvailable());

    if (bMpuCurbeSetInBrcUpdate)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->AssignDshAndSshSpace(
            stateHeapInterface,
            kernelState,
            true,
            0,
            false,
            m_storeData));
    }
    else
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->AssignDshAndSshSpace(
            stateHeapInterface,
            kernelState,
            false,
            0,
            false,
            m_storeData));

        MOS_ZeroMemory(&idParams, sizeof(idParams));
        idParams.pKernelState = kernelState;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(stateHeapInterface->pfnSetInterfaceDescriptor(
            stateHeapInterface,
            1,
            &idParams));

        // Setup VP8 Curbe
        MOS_ZeroMemory(&mpuCurbeParams, sizeof(mpuCurbeParams));
        mpuCurbeParams.pPicParams = pVp8PicParams;
        mpuCurbeParams.pSeqParams = pVp8SeqParams;
        mpuCurbeParams.pVp8QuantData = pVp8QuantData;
        mpuCurbeParams.ucKernelMode = m_kernelMode;
        mpuCurbeParams.bVmeKernelDump = bVMEKernelDump;
        mpuCurbeParams.wPictureCodingType = m_pictureCodingType;
        mpuCurbeParams.EncFunctionType = encFunctionType;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(SetMpuCurbe(&mpuCurbeParams));
    }

    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpKernelRegion(
            encFunctionType,
            MHW_DSH_TYPE,
            kernelState));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpCurbe(
            encFunctionType,
            kernelState));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpKernelRegion(
            encFunctionType,
            MHW_ISH_TYPE,
            kernelState));
    )

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnGetCommandBuffer(m_osInterface, &cmdBuffer, 0));

    sendKernelCmdsParams = SendKernelCmdsParams();
    sendKernelCmdsParams.EncFunctionType = encFunctionType;
    sendKernelCmdsParams.pKernelState = kernelState;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SendGenericKernelCmds(&cmdBuffer, &sendKernelCmdsParams));

    // Add binding table
    CODECHAL_ENCODE_CHK_STATUS_RETURN(stateHeapInterface->pfnSetBindingTable(
        stateHeapInterface,
        kernelState));

    //Add surface states
    MOS_ZeroMemory(&mpuSurfaceParams, sizeof(mpuSurfaceParams));
    mpuSurfaceParams.MediaStateType = encFunctionType;
    mpuSurfaceParams.dwHistogramSize = HISTOGRAM_SIZE;
    mpuSurfaceParams.dwModeProbabilitySize = MODE_PROPABILITIES_SIZE;
    mpuSurfaceParams.dwTokenProbabilitySize = COEFFS_PROPABILITIES_SIZE;
    mpuSurfaceParams.dwFrameHeaderSize = CODECHAL_VP8_FRAME_HEADER_SIZE;
    mpuSurfaceParams.dwHeaderMetadataSize = HEADER_METADATA_SIZE;
    mpuSurfaceParams.dwPictureStateSize = PICTURE_STATE_SIZE;
    mpuSurfaceParams.dwMpuBitstreamSize = MPU_BITSTREAM_SIZE;
    mpuSurfaceParams.dwTpuBitstreamSize = TPU_BITSTREAM_SIZE;
    mpuSurfaceParams.dwEntropyCostTableSize = ENTROPY_COST_TABLE_SIZE;
    mpuSurfaceParams.dwHeaderMetaDataOffset = bBrcEnabled ? 0 : HEADER_METADATA_OFFSET;
    mpuSurfaceParams.dwTokenBitsDataSize = TOKEN_BITS_DATA_SIZE;
    mpuSurfaceParams.dwKernelDumpSize = VP8_KERNEL_DUMP_SIZE;
    mpuSurfaceParams.bVMEKernelDump = bVMEKernelDump;
    mpuSurfaceParams.presHistogram = &resHistogram;
    mpuSurfaceParams.presRefModeProbability = &MpuTpuBuffers.resRefModeProbs;
    mpuSurfaceParams.presModeProbability = &MpuTpuBuffers.resModeProbs;
    mpuSurfaceParams.presRefTokenProbability = &MpuTpuBuffers.resRefCoeffProbs;
    mpuSurfaceParams.presTokenProbability = &MpuTpuBuffers.resCoeffProbs;
    mpuSurfaceParams.presFrameHeader = &resFrameHeader;
    mpuSurfaceParams.presHeaderMetadata = bBrcEnabled ?
        &BrcBuffers.resEncoderCfgCommandWriteBuffer :
        &MpuTpuBuffers.resPictureState;
    mpuSurfaceParams.presPictureState = &MpuTpuBuffers.resPictureState;
    mpuSurfaceParams.presMpuBitstream = &MpuTpuBuffers.resMpuBitstream;
    mpuSurfaceParams.presTpuBitstream = &MpuTpuBuffers.resTpuBitstream;
    mpuSurfaceParams.presVmeKernelDumpBuffer = &resVmeKernelDumpBuffer;
    mpuSurfaceParams.presEntropyCostTable = &MpuTpuBuffers.resEntropyCostTable;
    mpuSurfaceParams.presTokenBitsData = &MpuTpuBuffers.resTokenBitsData;
    mpuSurfaceParams.presModeCostUpdateBuffer = &resModeCostUpdateSurface;
    mpuSurfaceParams.pKernelState = kernelState;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(SendMpuSurfaces(&cmdBuffer, &mpuSurfaceParams));

    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpKernelRegion(
            encFunctionType,
            MHW_SSH_TYPE,
            kernelState));
    )

    MOS_ZeroMemory(&mediaObjectParams, sizeof(mediaObjectParams));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->GetRenderInterface()->AddMediaObject(
        &cmdBuffer,
        nullptr,
        &mediaObjectParams));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(EndStatusReport( &cmdBuffer, encFunctionType));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(stateHeapInterface->pfnSubmitBlocks(
        stateHeapInterface,
        kernelState));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(stateHeapInterface->pfnUpdateGlobalCmdBufId(
        stateHeapInterface));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(
        m_hwInterface->GetMiInterface()->AddMiBatchBufferEnd(&cmdBuffer, nullptr));

    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpCmdBuffer(
        &cmdBuffer,
        encFunctionType,
        nullptr)));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(
        m_hwInterface->UpdateSSEuForCmdBuffer(&cmdBuffer, m_singleTaskPhaseSupported, m_lastTaskInPhase));

    m_osInterface->pfnReturnCommandBuffer(m_osInterface, &cmdBuffer, 0);

    CODECHAL_ENCODE_CHK_STATUS_RETURN(
        m_osInterface->pfnSubmitCommandBuffer(m_osInterface, &cmdBuffer, m_renderContextUsesNullHw));

    return status;
}

MOS_STATUS CodechalEncodeVp8::TpuKernel()
{
    PMHW_STATE_HEAP_INTERFACE                       stateHeapInterface;
    PMHW_KERNEL_STATE                               kernelState;
    MHW_INTERFACE_DESCRIPTOR_PARAMS                 idParams;
    struct CodechalVp8TpuCurbeParams                tpuCurbeParams;
    struct CodechalVp8TpuSurfaceParams              tpuSurfaceParams;
    struct CodechalBindingTableVp8Tpu               bindingTable;
    SendKernelCmdsParams                            sendKernelCmdsParams;
    MHW_WALKER_PARAMS                               walkerParams;
    CODECHAL_WALKER_CODEC_PARAMS                    walkerCodecParams;
    MOS_COMMAND_BUFFER                              cmdBuffer;
    CODECHAL_MEDIA_STATE_TYPE                       encFunctionType;
    MHW_MEDIA_OBJECT_PARAMS                         mediaObjectParams;
    MOS_STATUS                                      status = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(m_hwInterface->GetMiInterface());
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_hwInterface->GetRenderInterface());
    CODECHAL_ENCODE_CHK_NULL_RETURN(pRefList);

    stateHeapInterface = m_hwInterface->GetRenderInterface()->m_stateHeapInterface;
    CODECHAL_ENCODE_CHK_NULL_RETURN(stateHeapInterface);

    encFunctionType = CODECHAL_MEDIA_STATE_TPU_FHB;
    kernelState = &TpuKernelState;
    bindingTable = TpuBindingTable;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(stateHeapInterface->pfnRequestSshSpaceForCmdBuf(
        stateHeapInterface,
        kernelState->KernelParams.iBTCount));
    m_vmeStatesSize =
        m_hwInterface->GetKernelLoadCommandSize(kernelState->KernelParams.iBTCount);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(VerifySpaceAvailable());

    if (bTpuCurbeSetInBrcUpdate)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->AssignDshAndSshSpace(
            stateHeapInterface,
            kernelState,
            true,
            0,
            false,
            m_storeData));
    }
    else
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->AssignDshAndSshSpace(
            stateHeapInterface,
            kernelState,
            false,
            0,
            false,
            m_storeData));

        MOS_ZeroMemory(&idParams, sizeof(idParams));
        idParams.pKernelState = kernelState;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(stateHeapInterface->pfnSetInterfaceDescriptor(
            stateHeapInterface,
            1,
            &idParams));

        // Setup VP8 Curbe
        MOS_ZeroMemory(&tpuCurbeParams, sizeof(struct CodechalVp8TpuCurbeParams));
        tpuCurbeParams.pPicParams = pVp8PicParams;
        tpuCurbeParams.pSeqParams = pVp8SeqParams;
        tpuCurbeParams.pVp8QuantData = pVp8QuantData;
        tpuCurbeParams.ucKernelMode = m_kernelMode;
        tpuCurbeParams.bVmeKernelDump = bVMEKernelDump;
        tpuCurbeParams.wPictureCodingType = m_pictureCodingType;
        tpuCurbeParams.EncFunctionType = encFunctionType;
        tpuCurbeParams.wPicWidthInMb = m_picWidthInMb;
        tpuCurbeParams.wFieldFrameHeightInMb = m_frameFieldHeightInMb;
        tpuCurbeParams.bRebinarizationFrameHdr = usRepakPassIterVal > 0 ? true : false;
        // Adaptive RePak can be enabled only when RePak is enabled
        tpuCurbeParams.bAdaptiveRePak = usRepakPassIterVal > 0 ? bAdaptiveRepakSupported : 0;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(SetTpuCurbe(&tpuCurbeParams));
    }

    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpKernelRegion(
            encFunctionType,
            MHW_DSH_TYPE,
            kernelState));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpCurbe(
            encFunctionType,
            kernelState));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpKernelRegion(
            encFunctionType,
            MHW_ISH_TYPE,
            kernelState));
    )

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnGetCommandBuffer(m_osInterface, &cmdBuffer, 0));

    sendKernelCmdsParams = SendKernelCmdsParams();
    sendKernelCmdsParams.EncFunctionType = encFunctionType;
    sendKernelCmdsParams.pKernelState = kernelState;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SendGenericKernelCmds(&cmdBuffer, &sendKernelCmdsParams));

    // Add binding table
    CODECHAL_ENCODE_CHK_STATUS_RETURN(stateHeapInterface->pfnSetBindingTable(
        stateHeapInterface,
        kernelState));

    //Add surface states
    MOS_ZeroMemory(&tpuSurfaceParams, sizeof(tpuSurfaceParams));
    tpuSurfaceParams.MediaStateType = encFunctionType;
    tpuSurfaceParams.dwPakTokenStatsSize = TOKEN_STATISTICS_SIZE;
    tpuSurfaceParams.dwTokenProbabilitySize = COEFFS_PROPABILITIES_SIZE;
    tpuSurfaceParams.dwEntropyCostTableSize = ENTROPY_COST_TABLE_SIZE;
    tpuSurfaceParams.dwFrameHeaderSize = CODECHAL_VP8_FRAME_HEADER_SIZE;
    tpuSurfaceParams.dwPictureStateSize = PICTURE_STATE_SIZE;
    tpuSurfaceParams.dwMpuCurbeSize = TOKEN_BITS_DATA_SIZE;
    tpuSurfaceParams.dwHeaderMetadataSize = HEADER_METADATA_SIZE;
    tpuSurfaceParams.dwHeaderMetaDataOffset = bBrcEnabled ? 0 : HEADER_METADATA_OFFSET;
    tpuSurfaceParams.dwKernelDumpSize = VP8_KERNEL_DUMP_SIZE;
    tpuSurfaceParams.dwRepakDecision = REPAK_DECISION_BUF_SIZE;
    tpuSurfaceParams.bVMEKernelDump = bVMEKernelDump;
    tpuSurfaceParams.presPakTokenStatistics = &MpuTpuBuffers.resPakTokenStatistics;
    tpuSurfaceParams.presPakTokenUpdateFlags = &MpuTpuBuffers.resPakTokenUpdateFlags;
    tpuSurfaceParams.presEntropyCostTable = &MpuTpuBuffers.resEntropyCostTable;
    tpuSurfaceParams.presFrameHeader = &resFrameHeader;
    tpuSurfaceParams.presDefaultTokenProbability = &MpuTpuBuffers.resDefaultTokenProbability;
    tpuSurfaceParams.presPictureState = &MpuTpuBuffers.resPictureState;
    tpuSurfaceParams.presMpuCurbeData = &MpuTpuBuffers.resTokenBitsData;
    tpuSurfaceParams.presHeaderMetadata = bBrcEnabled ?
        &BrcBuffers.resEncoderCfgCommandWriteBuffer :
        &MpuTpuBuffers.resPictureState;
    tpuSurfaceParams.presCurrFrameTokenProbability = &MpuTpuBuffers.resCoeffProbs;
    tpuSurfaceParams.presHwTokenProbabilityPass1 = &MpuTpuBuffers.resRefCoeffProbs;
    tpuSurfaceParams.presKeyFrameTokenProbability = &MpuTpuBuffers.resKeyFrameTokenProbability;
    tpuSurfaceParams.presUpdatedFrameTokenProbability = &MpuTpuBuffers.resUpdatedTokenProbability;
    tpuSurfaceParams.presHwTokenProbabilityPass2 = &MpuTpuBuffers.resHwTokenProbabilityPass2;
    tpuSurfaceParams.presVmeKernelDumpBuffer = &resVmeKernelDumpBuffer;
    tpuSurfaceParams.presRepakDecisionSurface = &MpuTpuBuffers.resRepakDecisionSurface;
    tpuSurfaceParams.pKernelState = kernelState;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(SendTpuSurfaces(&cmdBuffer, &tpuSurfaceParams));

    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpKernelRegion(
            encFunctionType,
            MHW_SSH_TYPE,
            kernelState));
    )

    MOS_ZeroMemory(&mediaObjectParams, sizeof(mediaObjectParams));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->GetRenderInterface()->AddMediaObject(
        &cmdBuffer,
        nullptr,
        &mediaObjectParams));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(EndStatusReport( &cmdBuffer, encFunctionType));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(stateHeapInterface->pfnSubmitBlocks(
        stateHeapInterface,
        kernelState));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(stateHeapInterface->pfnUpdateGlobalCmdBufId(
        stateHeapInterface));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->GetMiInterface()->AddMiBatchBufferEnd(&cmdBuffer, nullptr));

    CODECHAL_DEBUG_TOOL(CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpCmdBuffer(
        &cmdBuffer,
        encFunctionType,
        nullptr)));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(
        m_hwInterface->UpdateSSEuForCmdBuffer(&cmdBuffer, m_singleTaskPhaseSupported, m_lastTaskInPhase));

    m_osInterface->pfnReturnCommandBuffer(m_osInterface, &cmdBuffer, 0);

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnSubmitCommandBuffer(m_osInterface, &cmdBuffer, m_renderContextUsesNullHw));

    return status;
}

MOS_STATUS CodechalEncodeVp8::ExecutePictureLevel()
{
    MhwMiInterface                                  *commonMiInterface;
    MOS_COMMAND_BUFFER                              cmdBuffer;
    MHW_VDBOX_PIPE_MODE_SELECT_PARAMS               pipeModeSelectParams;
    MHW_VDBOX_SURFACE_PARAMS                        surfaceParams;
    MHW_VDBOX_PIPE_BUF_ADDR_PARAMS                  pipeBufAddrParams;
    MHW_VDBOX_IND_OBJ_BASE_ADDR_PARAMS              indObjBaseAddrParams;
    MHW_VDBOX_VP8_BSP_BUF_BASE_ADDR_PARAMS          vp8BspBufBaseAddrParams;
    MHW_MI_CONDITIONAL_BATCH_BUFFER_END_PARAMS      miConditionalBatchBufferEndParams;
    MHW_BATCH_BUFFER                                batchBuffer;
    PerfTagSetting                                  perfTag;
    uint8_t                                         index, picIdx;
    int32_t                                         i;
    MOS_STATUS                                      status = MOS_STATUS_SUCCESS;
    bool                                            deblockingEnable, suppressReconPic;
    bool                                            newCommandBufferStarted = false;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(m_hwInterface->GetMiInterface());
    commonMiInterface = m_hwInterface->GetMiInterface();

    perfTag.Value = 0;
    perfTag.Mode = (uint16_t)m_mode & CODECHAL_ENCODE_MODE_BIT_MASK;
    perfTag.CallType = CODECHAL_ENCODE_PERFTAG_CALL_PAK_ENGINE;
    perfTag.PictureCodingType = m_pictureCodingType;
    m_osInterface->pfnSetPerfTag(m_osInterface, perfTag.Value);

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnGetCommandBuffer(m_osInterface, &cmdBuffer, 0));

    // set MFX_PIPE_MODE_SELECT values
    MOS_ZeroMemory(&pipeModeSelectParams, sizeof(pipeModeSelectParams));
    pipeModeSelectParams.Mode = m_mode;
    pipeModeSelectParams.bStreamOutEnabled = false;
    deblockingEnable = ((pVp8PicParams->version == 0) || (pVp8PicParams->version == 1)) ? 1 : 0;
    suppressReconPic =
        ((!pRefList[m_currReconstructedPic.FrameIdx]->bUsedAsRef) && m_suppressReconPicSupported);
    pipeModeSelectParams.bPreDeblockOutEnable = !deblockingEnable && !suppressReconPic;
    pipeModeSelectParams.bPostDeblockOutEnable = deblockingEnable && !suppressReconPic;

    // set MFX_PIPE_BUF_ADDR_STATE values
    MOS_ZeroMemory(&pipeBufAddrParams, sizeof(pipeBufAddrParams));
    pipeBufAddrParams.Mode = m_mode;
    pipeBufAddrParams.psPreDeblockSurface = &m_reconSurface;
    pipeBufAddrParams.psPostDeblockSurface = &m_reconSurface;
    pipeBufAddrParams.psRawSurface = m_rawSurfaceToPak;
    pipeBufAddrParams.presStreamOutBuffer = &m_resStreamOutBuffer[m_currRecycledBufIdx];
    pipeBufAddrParams.presMfdDeblockingFilterRowStoreScratchBuffer = &m_resDeblockingFilterRowStoreScratchBuffer;
    pipeBufAddrParams.presMfdIntraRowStoreScratchBuffer = &resIntraRowStoreScratchBuffer;

    CODECHAL_ENCODE_CHK_NULL_RETURN(m_mmcState);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_mmcState->SetPipeBufAddr(&pipeBufAddrParams));

    // Setting invalid entries to nullptr
    for (i = 0; i < CODEC_MAX_NUM_REF_FRAME; i++)
    {
        pipeBufAddrParams.presReferences[i] = nullptr;
    }

    if (m_pictureCodingType == P_TYPE)
    {
        if (!CodecHal_PictureIsInvalid(pVp8PicParams->LastRefPic))
        {
            index = pVp8PicParams->LastRefPic.FrameIdx;
            pipeBufAddrParams.presReferences[0] = &(pRefList[index]->sRefBuffer.OsResource);
        }

        if (!CodecHal_PictureIsInvalid(pVp8PicParams->GoldenRefPic))
        {
            index = pVp8PicParams->GoldenRefPic.FrameIdx;
            pipeBufAddrParams.presReferences[1] = &(pRefList[index]->sRefBuffer.OsResource);
        }

        if (!CodecHal_PictureIsInvalid(pVp8PicParams->AltRefPic))
        {
            index = pVp8PicParams->AltRefPic.FrameIdx;
            pipeBufAddrParams.presReferences[2] = &(pRefList[index]->sRefBuffer.OsResource);
        }
    }
    for (i = 0; i < CODEC_MAX_NUM_REF_FRAME_NON_AVC; i++)
    {
        if (PicIdx[i].bValid)
        {
            picIdx = PicIdx[i].ucPicIdx;
            CodecHalGetResourceInfo(
                m_osInterface,
                &(pRefList[picIdx]->sRefReconBuffer));
            pipeBufAddrParams.presReferences[i] = &(pRefList[picIdx]->sRefReconBuffer.OsResource);
        }
    }

    // set MFX_SURFACE_STATE values
    MOS_ZeroMemory(&surfaceParams, sizeof(surfaceParams));
    surfaceParams.Mode = m_mode;

    // set MFX_IND_OBJ_BASE_ADDR_STATE values
    MOS_ZeroMemory(&indObjBaseAddrParams, sizeof(indObjBaseAddrParams));
    indObjBaseAddrParams.Mode = CODECHAL_ENCODE_MODE_VP8;
    indObjBaseAddrParams.presMvObjectBuffer = &m_resMbCodeSurface;
    indObjBaseAddrParams.dwMvObjectOffset = m_mvOffset + m_mvBottomFieldOffset;
    indObjBaseAddrParams.dwMvObjectSize = m_mbCodeSize - m_mvOffset;
    indObjBaseAddrParams.presPakBaseObjectBuffer = &m_resBitstreamBuffer;
    indObjBaseAddrParams.presDataBuffer = &m_resBitstreamBuffer;
    indObjBaseAddrParams.dwPakBaseObjectSize = m_bitstreamUpperBound;
    indObjBaseAddrParams.dwDataSize = m_bitstreamUpperBound;

    //set MFX_VP8_BSP_BUF_BASE_ADDR_STATE values
    MOS_ZeroMemory(&vp8BspBufBaseAddrParams, sizeof(vp8BspBufBaseAddrParams));
    vp8BspBufBaseAddrParams.presFrameHeaderBuffer = &resFrameHeader;
    vp8BspBufBaseAddrParams.presPakIntermediateBuffer = &resPakIntermediateBuffer;
    vp8BspBufBaseAddrParams.presPakFinalFrameBuffer = &m_resBitstreamBuffer;
    vp8BspBufBaseAddrParams.presTokenStatisticsBuffer = &MpuTpuBuffers.resPakTokenStatistics;
    vp8BspBufBaseAddrParams.presBsdMpcRowStoreScratchBuffer = &m_resMPCRowStoreScratchBuffer;
    vp8BspBufBaseAddrParams.dwPakIntermediatePartition0Size = ((m_frameWidth * m_frameHeight) / 4) + INTERMEDIATE_PARTITION0_SIZE;
    vp8BspBufBaseAddrParams.dwPakIntermediateTokenSize = (m_frameWidth * m_frameHeight * 2);
    vp8BspBufBaseAddrParams.dwPartitions = 1 << (pVp8PicParams->CodedCoeffTokenPartition);
    vp8BspBufBaseAddrParams.presCoeffProbsBuffer = &MpuTpuBuffers.resCoeffProbs;

    if (usRepakPassIterVal > 0 &&
        m_pictureCodingType == I_TYPE &&
        usRepakPassIterVal == m_currPass)
    {
        // When repak enabled, for I frames during repak pass assign key frame token prob. required by kernel
        vp8BspBufBaseAddrParams.presCoeffProbsBuffer = &MpuTpuBuffers.resKeyFrameTokenProbability;
    }

    // Send command buffer header at the beginning 
    if (!bBrcEnabled)
    {
        newCommandBufferStarted = true;
    }
    else
    {
        if (m_currPass == 0)
        {
            newCommandBufferStarted = true;
        }
        else if (m_currPass == usRepakPassIterVal && usRepakPassIterVal > 0)
        {
            newCommandBufferStarted = true;
        }
    }

    if (newCommandBufferStarted)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(SendPrologWithFrameTracking(&cmdBuffer, false));
    }

    // BRC 2-4th Pass (excludes RePAK)
    if (bBrcEnabled &&
        m_currPass > 0 &&
        (m_currPass < usRepakPassIterVal || usRepakPassIterVal == 0))
    {
        // Insert conditional batch buffer end
        MOS_ZeroMemory(
            &miConditionalBatchBufferEndParams,
            sizeof(MHW_MI_CONDITIONAL_BATCH_BUFFER_END_PARAMS));

        miConditionalBatchBufferEndParams.presSemaphoreBuffer =
            &m_encodeStatusBuf.resStatusBuffer;
        miConditionalBatchBufferEndParams.dwOffset =
            (m_encodeStatusBuf.wCurrIndex * m_encodeStatusBuf.dwReportSize) +
            m_encodeStatusBuf.dwImageStatusMaskOffset +
            (sizeof(uint32_t) * 2);
        CODECHAL_ENCODE_CHK_STATUS_RETURN(commonMiInterface->AddMiConditionalBatchBufferEndCmd(
            &cmdBuffer,
            &miConditionalBatchBufferEndParams));
    }

    // RePAK needed for both CQP and BRC
    if ((usRepakPassIterVal > 0) && (m_currPass == usRepakPassIterVal))
    {
        // Execute RePAK based on decision from TPU kernel
        // by using conditional batch buffer end command
        MOS_ZeroMemory(
            &miConditionalBatchBufferEndParams,
            sizeof(MHW_MI_CONDITIONAL_BATCH_BUFFER_END_PARAMS));

        miConditionalBatchBufferEndParams.presSemaphoreBuffer =
            &MpuTpuBuffers.resRepakDecisionSurface;
        miConditionalBatchBufferEndParams.dwOffset = 0;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(commonMiInterface->AddMiConditionalBatchBufferEndCmd(
            &cmdBuffer,
            &miConditionalBatchBufferEndParams));
    }

    if (!m_currPass && m_osInterface->bTagResourceSync)
    {
        // This is a short term WA to solve the sync tag issue: the sync tag write for PAK is inserted at the end of 2nd pass PAK BB 
        // which may be skipped in multi-pass PAK enabled case. The idea here is to insert the previous frame's tag at the beginning 
        // of the BB and keep the current frame's tag at the end of the BB. There will be a delay for tag update but it should be fine
        // as long as Dec/VP/Enc won't depend on this PAK so soon.
        MHW_MI_STORE_DATA_PARAMS                        Params;
        MOS_RESOURCE                                    resGlobalGpuContextSyncTagBuffer;
        uint32_t                                           dwValue;

        CODECHAL_HW_CHK_STATUS_RETURN(m_osInterface->pfnGetGpuStatusBufferResource(
            m_osInterface,
            &resGlobalGpuContextSyncTagBuffer));

        dwValue = m_osInterface->pfnGetGpuStatusTag(m_osInterface, m_osInterface->CurrentGpuContextOrdinal);
        Params.pOsResource = &resGlobalGpuContextSyncTagBuffer;
        Params.dwResourceOffset = m_osInterface->pfnGetGpuStatusTagOffset(m_osInterface, m_osInterface->CurrentGpuContextOrdinal);
        Params.dwValue = (dwValue > 0) ? (dwValue - 1) : 0;
        CODECHAL_HW_CHK_STATUS_RETURN(m_hwInterface->GetMiInterface()->AddMiStoreDataImmCmd(&cmdBuffer, &Params));
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(StartStatusReport(&cmdBuffer, CODECHAL_NUM_MEDIA_STATES));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->GetMfxInterface()->AddMfxPipeModeSelectCmd(&cmdBuffer, &pipeModeSelectParams));

    // Ref surface
    surfaceParams.ucSurfaceStateId = CODECHAL_MFX_REF_SURFACE_ID;
    surfaceParams.psSurface = &m_reconSurface;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->GetMfxInterface()->AddMfxSurfaceCmd(&cmdBuffer, &surfaceParams));

    // Src surface
    surfaceParams.ucSurfaceStateId = CODECHAL_MFX_SRC_SURFACE_ID;
    surfaceParams.psSurface = m_rawSurfaceToPak;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->GetMfxInterface()->AddMfxSurfaceCmd(&cmdBuffer, &surfaceParams));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->GetMfxInterface()->AddMfxPipeBufAddrCmd(&cmdBuffer, &pipeBufAddrParams));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->GetMfxInterface()->AddMfxIndObjBaseAddrCmd(&cmdBuffer, &indObjBaseAddrParams));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->GetMfxInterface()->AddMfxVp8BspBufBaseAddrCmd(&cmdBuffer, &vp8BspBufBaseAddrParams));

    // MMIO register reads from PAK engine, different from PAK statistics buffer used for BrcUpdate kernel input
    // save Pak stats into dump buffer BEFORE every PAK pass execution for debugging
    CODECHAL_ENCODE_CHK_STATUS_RETURN(
        SetPakStatsDebugBuffer(
            &cmdBuffer,
            &BrcBuffers.resBrcPakStatsBeforeDumpBuffer,
            m_currPass * 48));

    // Insert Batch Buffer Start command to MFX_VP8_PIC_STATE and MFX_VP8_ENCODER_CFG and BB end

    // Insert Batch Buffer Start command to Pic State command
    MOS_ZeroMemory(&batchBuffer, sizeof(batchBuffer));
    batchBuffer.OsResource = MpuTpuBuffers.resPictureState;
    batchBuffer.bSecondLevel = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(commonMiInterface->AddMiBatchBufferStartCmd(
        &cmdBuffer,
        &batchBuffer));

    if (bBrcEnabled)
    {
        // Insert Batch Buffer Start command to MFX_VP8_ENCODER_CFG   Config States
        MOS_ZeroMemory(&batchBuffer, sizeof(batchBuffer));
        batchBuffer.OsResource = BrcBuffers.resEncoderCfgCommandWriteBuffer;
        if (usRepakPassIterVal == 0)
        {
            //No repak in this case
            batchBuffer.dwOffset = m_currPass * BRC_IMG_STATE_SIZE_PER_PASS;
        }
        else
        {
            batchBuffer.dwOffset = usRepakPassIterVal == m_currPass ?
                0 : m_currPass * BRC_IMG_STATE_SIZE_PER_PASS;
        }
        batchBuffer.bSecondLevel = true;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(commonMiInterface->AddMiBatchBufferStartCmd(
            &cmdBuffer,
            &batchBuffer));
    }

    // Insert Batch Buffer Start command to send VP8_PAK_OBJ data for MBs in this slice
    MOS_ZeroMemory(&batchBuffer, sizeof(batchBuffer));
    batchBuffer.OsResource = m_resMbCodeSurface;
    batchBuffer.bSecondLevel = true;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(commonMiInterface->AddMiBatchBufferStartCmd(
        &cmdBuffer,
        &batchBuffer));

    m_osInterface->pfnReturnCommandBuffer(m_osInterface, &cmdBuffer, 0);

    return status;
}

MOS_STATUS CodechalEncodeVp8::SetPakStatsDebugBuffer(PMOS_COMMAND_BUFFER cmdBuffer, PMOS_RESOURCE pResource, uint32_t dwBaseOffset)
{
    MhwMiInterface                      *commonMiInterface;
    MHW_MI_STORE_REGISTER_MEM_PARAMS    miStoreRegMemParams;
    MHW_MI_FLUSH_DW_PARAMS              flushDwParams;
    MHW_MI_STORE_DATA_PARAMS            storeDataParams;
    MmioRegistersMfx                    *mmioRegisters;
    MOS_STATUS status = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(cmdBuffer);
    CODECHAL_ENCODE_CHK_NULL_RETURN(pResource);
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_hwInterface->GetMiInterface());

    commonMiInterface = m_hwInterface->GetMiInterface();

    CODECHAL_ENCODE_CHK_COND_RETURN((m_vdboxIndex > m_hwInterface->GetMfxInterface()->GetMaxVdboxIndex()), "ERROR - vdbox index exceed the maximum");
    mmioRegisters = m_hwInterface->GetMfxInterface()->GetMmioRegisters(m_vdboxIndex);

    // this is just for dump purpose after each PAK pass
    // doesn't have to do with order in CodecHalVp8_EncodeReadPakStatistics
    // this order can be changed based on driver need
    MOS_ZeroMemory(&miStoreRegMemParams, sizeof(miStoreRegMemParams));

    miStoreRegMemParams.presStoreBuffer = pResource;
    miStoreRegMemParams.dwOffset = dwBaseOffset + sizeof(uint32_t) * 0;
    miStoreRegMemParams.dwRegister = mmioRegisters->mfcVP8ImageStatusMaskRegOffset;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(commonMiInterface->AddMiStoreRegisterMemCmd(cmdBuffer, &miStoreRegMemParams));

    miStoreRegMemParams.presStoreBuffer = pResource;
    miStoreRegMemParams.dwOffset = dwBaseOffset + sizeof(uint32_t) * 1;
    miStoreRegMemParams.dwRegister = mmioRegisters->mfcVP8ImageStatusCtrlRegOffset;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(commonMiInterface->AddMiStoreRegisterMemCmd(cmdBuffer, &miStoreRegMemParams));

    miStoreRegMemParams.presStoreBuffer = pResource;
    miStoreRegMemParams.dwOffset = dwBaseOffset + sizeof(uint32_t) * 2;
    miStoreRegMemParams.dwRegister = mmioRegisters->mfcVP8BitstreamBytecountFrameRegOffset;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(commonMiInterface->AddMiStoreRegisterMemCmd(cmdBuffer, &miStoreRegMemParams));

    // Driver needs to read MFX_VP8_PIC_STATUS.DW4 / MFX_VP8_PIC_STATUS.DW5 (DeltaQindex / DeltaLoopFilter reads). 
    // This would trigger HW to register DeltaQindex / DeltaLoopFilter to be used to update CumulativeDeltaQindex / CumulativeDeltaLoopFilter.

    miStoreRegMemParams.presStoreBuffer = pResource;
    miStoreRegMemParams.dwOffset = dwBaseOffset + sizeof(uint32_t) * 5;
    miStoreRegMemParams.dwRegister = mmioRegisters->mfxVP8BrcCumulativeDQIndex01RegOffset;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(commonMiInterface->AddMiStoreRegisterMemCmd(cmdBuffer, &miStoreRegMemParams));

    miStoreRegMemParams.presStoreBuffer = pResource;
    miStoreRegMemParams.dwOffset = dwBaseOffset + sizeof(uint32_t) * 6;
    miStoreRegMemParams.dwRegister = mmioRegisters->mfxVP8BrcCumulativeDQIndex23RegOffset;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(commonMiInterface->AddMiStoreRegisterMemCmd(cmdBuffer, &miStoreRegMemParams));

    miStoreRegMemParams.presStoreBuffer = pResource;
    miStoreRegMemParams.dwOffset = dwBaseOffset + sizeof(uint32_t) * 7;
    miStoreRegMemParams.dwRegister = mmioRegisters->mfxVP8BrcCumulativeDLoopFilter01RegOffset;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(commonMiInterface->AddMiStoreRegisterMemCmd(cmdBuffer, &miStoreRegMemParams));

    miStoreRegMemParams.presStoreBuffer = pResource;
    miStoreRegMemParams.dwOffset = dwBaseOffset + sizeof(uint32_t) * 8;
    miStoreRegMemParams.dwRegister = mmioRegisters->mfxVP8BrcCumulativeDLoopFilter23RegOffset;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(commonMiInterface->AddMiStoreRegisterMemCmd(cmdBuffer, &miStoreRegMemParams));

    miStoreRegMemParams.presStoreBuffer = pResource;
    miStoreRegMemParams.dwOffset = dwBaseOffset + sizeof(uint32_t) * 9;
    miStoreRegMemParams.dwRegister = mmioRegisters->mfxVP8BrcConvergenceStatusRegOffset;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(commonMiInterface->AddMiStoreRegisterMemCmd(cmdBuffer, &miStoreRegMemParams));

    return status;
}

MOS_STATUS CodechalEncodeVp8::AddBBEndToPicStateCmd()
{
    MOS_LOCK_PARAMS                         lockFlagsWriteOnly;
    uint32_t                                *data;
    MOS_STATUS                              status = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    //ADD BB End command to PIC_STATE_CMD
    MOS_ZeroMemory(&lockFlagsWriteOnly, sizeof(MOS_LOCK_PARAMS));
    lockFlagsWriteOnly.WriteOnly = 1;
    data = (uint32_t *)m_osInterface->pfnLockResource(
        m_osInterface,
        &(MpuTpuBuffers.resPictureState),
        &lockFlagsWriteOnly);

    if (data == nullptr)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to Lock VP8 Pic State CMD.");
        return MOS_STATUS_UNKNOWN;
    }
    data += 38;
    *data = 0x05000000; // BATCH Buffer End command. 
    m_osInterface->pfnUnlockResource(m_osInterface, &MpuTpuBuffers.resPictureState);

    return status;
}

MOS_STATUS CodechalEncodeVp8::EncodeSliceLevelBrc(PMOS_COMMAND_BUFFER cmdBuffer)
{
    MOS_SYNC_PARAMS                             syncParams;
    EncodeReadBrcPakStatsParams                 readBrcPakStatsParams;
    uint32_t                                    dwOffset;
    uint32_t                                    *data;
    MOS_LOCK_PARAMS                             lockFlagsWriteOnly;
    MOS_STATUS                                  status = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    dwOffset =
        (m_encodeStatusBuf.wCurrIndex * m_encodeStatusBuf.dwReportSize) +
        m_encodeStatusBuf.dwNumPassesOffset +   // Num passes offset
        sizeof(uint32_t) * 2;                               // pEncodeStatus is offset by 2 DWs in the resource

                                                            // Read PaK statistics buffer here for next pass's use
    readBrcPakStatsParams.pHwInterface = m_hwInterface;
    readBrcPakStatsParams.presBrcPakStatisticBuffer = &BrcBuffers.resBrcPakStatisticBuffer[0];
    readBrcPakStatsParams.VideoContext = m_videoContext;
    readBrcPakStatsParams.ucPass = m_currPass;
    readBrcPakStatsParams.presStatusBuffer = &m_encodeStatusBuf.resStatusBuffer;
    readBrcPakStatsParams.dwStatusBufNumPassesOffset = dwOffset;

    // MMIO register reads from PAK engine, different from PAK statistics buffer used for BrcUpdate kernel input
    // save Pak stats into dump buffer "AFTER" every pass for debugging
    CODECHAL_ENCODE_CHK_STATUS_RETURN(
        SetPakStatsDebugBuffer(
            cmdBuffer,
            &BrcBuffers.resBrcPakStatsAfterDumpBuffer,
            m_currPass * 48));

    // LOAD_REG_MEM & STORE_REG_MEM in BRC flow diagram
    // LOAD_REG_MEM is only needed for 2nd-4th PAK pass
    CODECHAL_ENCODE_CHK_STATUS_RETURN(ReadBrcPakStatistics(
        cmdBuffer,
        &readBrcPakStatsParams));

    MOS_ZeroMemory(&lockFlagsWriteOnly, sizeof(MOS_LOCK_PARAMS));
    lockFlagsWriteOnly.WriteOnly = 1;
    data = (uint32_t *)m_osInterface->pfnLockResource(
        m_osInterface,
        &(MpuTpuBuffers.resPictureState),
        &lockFlagsWriteOnly);

    if (data == nullptr)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to Lock Pic State Read Buffer.");
        return MOS_STATUS_UNKNOWN;
    }
    data += 38;
    *data = 0x05000000; // BATCH Buffer End command. Remove me once kernel is fixed.
    m_osInterface->pfnUnlockResource(m_osInterface, &MpuTpuBuffers.resPictureState);

    // Write MFC MMIO register values to Tpu curbe (including cumulative QP & cumulative LF)
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetPakStatsInTpuCurbe(cmdBuffer));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(EndStatusReport( cmdBuffer, CODECHAL_NUM_MEDIA_STATES));

    // 4th BRC PAK pass go inside if statement
    if (((usRepakPassIterVal > 0 && (m_currPass == m_numPasses - 1))) ||     // Repak enabled,  last PAK pass before Repak
        (usRepakPassIterVal == 0 && m_currPass == m_numPasses))               // Repak disabled, last PAK pass 
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->GetMiInterface()->AddMiBatchBufferEnd(cmdBuffer, nullptr));

        // last PAK pass except RePak
        std::string pakPassName = "PAK_PASS" + std::to_string(static_cast<uint32_t>(m_currPass));
        CODECHAL_DEBUG_TOOL(CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpCmdBuffer(
            cmdBuffer,
            CODECHAL_NUM_MEDIA_STATES,
            pakPassName.data())));

        m_osInterface->pfnReturnCommandBuffer(m_osInterface, cmdBuffer, 0);

        // MFX should wait for RenderContext aka MPU kernel to finish on the render engine
        if (!Mos_ResourceIsNull(&m_resSyncObjectRenderContextInUse))
        {
            syncParams = g_cInitSyncParams;
            syncParams.GpuContext = m_videoContext;
            syncParams.presSyncResource = &m_resSyncObjectRenderContextInUse;
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnEngineWait(m_osInterface, &syncParams));
        }

        // Submit cmdBuffer at last BRC pass or at in the repak pass if repak is enabled.
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnSubmitCommandBuffer(m_osInterface, cmdBuffer, m_videoContextUsesNullHw));

        // send signal from video, last PAK pass except RePAK
        if (m_signalEnc && !Mos_ResourceIsNull(&m_resSyncObjectVideoContextInUse))
        {
            // signal semaphore
            syncParams = g_cInitSyncParams;
            syncParams.GpuContext = m_videoContext;
            syncParams.presSyncResource = &m_resSyncObjectVideoContextInUse;

            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnEngineSignal(m_osInterface, &syncParams));
        }
    }
    else  // remaining PAK passes 
    {
        std::string pakPassName = "PAK_PASS" + std::to_string(static_cast<uint32_t>(m_currPass));
        CODECHAL_DEBUG_TOOL(CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpCmdBuffer(
            cmdBuffer,
            CODECHAL_NUM_MEDIA_STATES,
            pakPassName.data())));

        m_osInterface->pfnReturnCommandBuffer(m_osInterface, cmdBuffer, 0);
    }

    return status;
}

MOS_STATUS CodechalEncodeVp8::ExecuteSliceLevel()
{
    MOS_COMMAND_BUFFER                          cmdBuffer;
    MOS_SYNC_PARAMS                             syncParams;
    EncodeReadBrcPakStatsParams                 readBrcPakStatsParams;
    uint32_t                                    *data;
    MOS_LOCK_PARAMS                             lockFlagsWriteOnly;
    MOS_STATUS                                  status = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(m_hwInterface->GetMiInterface());

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnGetCommandBuffer(m_osInterface, &cmdBuffer, 0));

    // read image status
    CODECHAL_ENCODE_CHK_STATUS_RETURN(ReadMfcStatus(&cmdBuffer));

    if (m_hwInterface->GetMfxInterface()->GetBrcNumPakPasses() == CODECHAL_ENCODE_BRC_SINGLE_PASS)  // Multi-Pass BRC: disabled by default
    {
        // BRC PAK statistics different for each pass
        if (bBrcEnabled)
        {
            // Read PaK statistics buffer here for next frame's use
            readBrcPakStatsParams.pHwInterface = m_hwInterface;
            readBrcPakStatsParams.presBrcPakStatisticBuffer = &BrcBuffers.resBrcPakStatisticBuffer[0];
            readBrcPakStatsParams.VideoContext = m_videoContext;

            CODECHAL_ENCODE_CHK_STATUS_RETURN(ReadBrcPakStatistics(
                &cmdBuffer,
                &readBrcPakStatsParams));

            MOS_ZeroMemory(&lockFlagsWriteOnly, sizeof(MOS_LOCK_PARAMS));
            lockFlagsWriteOnly.WriteOnly = 1;
            data = (uint32_t *)m_osInterface->pfnLockResource(
                m_osInterface,
                &(MpuTpuBuffers.resPictureState),
                &lockFlagsWriteOnly);

            if (data == nullptr)
            {
                CODECHAL_ENCODE_ASSERTMESSAGE("Failed to Lock BRC Encoder CFG Read Buffer.");
                return MOS_STATUS_UNKNOWN;
            }
            data += 38;
            *data = 0x05000000; // BATCH Buffer End command. Remove me once kernel is fixed.
            m_osInterface->pfnUnlockResource(m_osInterface, &MpuTpuBuffers.resPictureState);

        }

        CODECHAL_ENCODE_CHK_STATUS_RETURN(EndStatusReport( &cmdBuffer, CODECHAL_NUM_MEDIA_STATES));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->GetMiInterface()->AddMiBatchBufferEnd(&cmdBuffer, nullptr));

        std::string Pak_pass = "PAK_PASS" + std::to_string(static_cast<uint32_t>(m_currPass));
        CODECHAL_DEBUG_TOOL(CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpCmdBuffer(
            &cmdBuffer,
            CODECHAL_NUM_MEDIA_STATES,
            Pak_pass.data())));
        m_osInterface->pfnReturnCommandBuffer(m_osInterface, &cmdBuffer, 0);

        if (bBrcEnabled)
        {
            // RePAK waits for TPU kernel execution
            if ((m_currPass == 0 || m_currPass == usRepakPassIterVal) &&
                !Mos_ResourceIsNull(&m_resSyncObjectRenderContextInUse))
            {
                syncParams = g_cInitSyncParams;
                syncParams.GpuContext = m_videoContext;
                syncParams.presSyncResource = &m_resSyncObjectRenderContextInUse;

                CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnEngineWait(m_osInterface, &syncParams));
            }
        }
        else
        {
            // MFX should wait for RenderContext
            if (!Mos_ResourceIsNull(&m_resSyncObjectRenderContextInUse))
            {
                syncParams = g_cInitSyncParams;
                syncParams.GpuContext = m_videoContext;
                syncParams.presSyncResource = &m_resSyncObjectRenderContextInUse;

                CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnEngineWait(m_osInterface, &syncParams));
            }
        }

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnSubmitCommandBuffer(m_osInterface, &cmdBuffer, m_videoContextUsesNullHw));

        if (bBrcEnabled)
        {
            if (m_currPass == (m_numPasses - 1) ||
                m_currPass == (m_numPasses) &&
                m_signalEnc &&
                !Mos_ResourceIsNull(&m_resSyncObjectVideoContextInUse)
                )
            {

                // signal semaphore
                syncParams = g_cInitSyncParams;
                syncParams.GpuContext = m_videoContext;
                syncParams.presSyncResource = &m_resSyncObjectVideoContextInUse;

                CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnEngineSignal(m_osInterface, &syncParams));
            }
        }
        else // CQP
        {
            //Signal TPU on first pak pass and MBEnc in second pak pass if next frame is P
            // PAK signals end of execution
            // I frame only case also needs to signal RePAK end (ucCurrPass = 1 && GopSize = 1)
            // since next frame render engine will always wait
            if (m_signalEnc &&
                !Mos_ResourceIsNull(&m_resSyncObjectVideoContextInUse) &&
                ((m_currPass == 0) || (pVp8SeqParams->GopPicSize >= 1)))
            {
                // signal semaphore
                syncParams = g_cInitSyncParams;
                syncParams.GpuContext = m_videoContext;
                syncParams.presSyncResource = &m_resSyncObjectVideoContextInUse;

                CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnEngineSignal(m_osInterface, &syncParams));
            }
        }
    }
    else    // Multi-Pass BRC: enabled
    {
        // PAK Passes except RePak
        if ((bBrcEnabled) &&
            ((m_currPass < m_numPasses && usRepakPassIterVal > 0) ||        // Repak enabled
            (m_currPass <= m_numPasses && usRepakPassIterVal == 0))     // Repak disabled
            )
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(EncodeSliceLevelBrc(&cmdBuffer));
        }
        else  // CQP PAK pass, RePak pass for CQP & BRC
        {
            // RePak pass for BRC
            if (bBrcEnabled)
            {
                uint32_t   dwOffset;

                dwOffset =
                    (m_encodeStatusBuf.wCurrIndex * m_encodeStatusBuf.dwReportSize) +
                    m_encodeStatusBuf.dwNumPassesOffset +   // Num passes offset
                    sizeof(uint32_t) * 2;                   // pEncodeStatus is offset by 2 DWs in the resource

                // Read PaK statistics buffer here for next pass's use
                readBrcPakStatsParams.pHwInterface = m_hwInterface;
                readBrcPakStatsParams.presBrcPakStatisticBuffer = &BrcBuffers.resBrcPakStatisticBuffer[0];
                readBrcPakStatsParams.VideoContext = m_videoContext;

                readBrcPakStatsParams.ucPass = m_currPass;
                readBrcPakStatsParams.presStatusBuffer = &m_encodeStatusBuf.resStatusBuffer;
                readBrcPakStatsParams.dwStatusBufNumPassesOffset = dwOffset;

                // LOAD_REG_MEM & STORE_REG_MEM in BRC flow diagram
                CODECHAL_ENCODE_CHK_STATUS_RETURN(ReadBrcPakStatistics(
                    &cmdBuffer,
                    &readBrcPakStatsParams));
            }

            //TPU kernel call, CQP Pak pass and Repak pass below
            MOS_ZeroMemory(&lockFlagsWriteOnly, sizeof(MOS_LOCK_PARAMS));
            lockFlagsWriteOnly.WriteOnly = 1;
            data = (uint32_t *)m_osInterface->pfnLockResource(
                m_osInterface,
                &(MpuTpuBuffers.resPictureState),
                &lockFlagsWriteOnly);

            if (data == nullptr)
            {
                CODECHAL_ENCODE_ASSERTMESSAGE("Failed to Lock BRC Picture State Write Buffer.");
                return MOS_STATUS_UNKNOWN;
            }
            data += 38;
            *data = 0x05000000; // BATCH Buffer End command. Remove me once kernel is fixed. 
            m_osInterface->pfnUnlockResource(m_osInterface, &MpuTpuBuffers.resPictureState);

            CODECHAL_ENCODE_CHK_STATUS_RETURN(EndStatusReport( &cmdBuffer, CODECHAL_NUM_MEDIA_STATES));

            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->GetMiInterface()->AddMiBatchBufferEnd(&cmdBuffer, nullptr));

            std::string pakPassName = "PAK_PASS" + std::to_string(static_cast<uint32_t>(m_currPass));
            CODECHAL_DEBUG_TOOL(CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpCmdBuffer(
                &cmdBuffer,
                CODECHAL_NUM_MEDIA_STATES,
                pakPassName.data())));
            m_osInterface->pfnReturnCommandBuffer(m_osInterface, &cmdBuffer, 0);

            // MFX should wait for CurrPass 0 and CurrPass 1
            if (!Mos_ResourceIsNull(&m_resSyncObjectRenderContextInUse))
            {
                syncParams = g_cInitSyncParams;
                syncParams.GpuContext = m_videoContext;
                syncParams.presSyncResource = &m_resSyncObjectRenderContextInUse;

                CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnEngineWait(m_osInterface, &syncParams));
            }

            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnSubmitCommandBuffer(m_osInterface, &cmdBuffer, m_videoContextUsesNullHw));

            // send signal from video, RePAK
            //Signal after every PAK
            if (m_signalEnc &&
                !Mos_ResourceIsNull(&m_resSyncObjectVideoContextInUse))
            {
                // signal semaphore
                syncParams = g_cInitSyncParams;
                syncParams.GpuContext = m_videoContext;
                syncParams.presSyncResource = &m_resSyncObjectVideoContextInUse;

                CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnEngineSignal(m_osInterface, &syncParams));
            }
        }
    }

    CODECHAL_DEBUG_TOOL(
        if (!bMmcUserFeatureUpdated)
        {
            CODECHAL_UPDATE_ENCODE_MMC_USER_FEATURE(m_reconSurface);
            bMmcUserFeatureUpdated = true;
        }
    )

        // Reset parameters for next PAK execution
        if (m_currPass == m_numPasses)
        {
            m_newPpsHeader = 0;
            m_newSeqHeader = 0;
        }

    //call the TPU kernel only once
    if (((m_currPass == m_numPasses - 1) && usRepakPassIterVal > 0) ||   // RePAK enabled:   last PAK pass before RePAK
        (m_currPass == m_numPasses && usRepakPassIterVal == 0))          // RePAK disabled:  last PAK pass
    {

        //call the TPU kernel
        m_osInterface->pfnSetGpuContext(m_osInterface, m_renderContext);
        m_osInterface->pfnResetOsStates(m_osInterface);

        if (!Mos_ResourceIsNull(&m_resSyncObjectVideoContextInUse))
        {
            // TPU kernel waits for PAK execution
            syncParams = g_cInitSyncParams;
            syncParams.GpuContext = m_renderContext;
            syncParams.presSyncResource = &m_resSyncObjectVideoContextInUse;

            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnEngineWait(m_osInterface, &syncParams));
        }

        CODECHAL_ENCODE_CHK_STATUS_RETURN(TpuKernel());


        // if repak is enabled, then need to signal after TPU kernel execution
        // m_numPasses >= 1 even when RePak is disabled due to multi-pass BRC
        // fix needed since P frame RePAK can be disabled sometimes
        if (!Mos_ResourceIsNull(&m_resSyncObjectRenderContextInUse))
        {

            // TPU kernel signals RePAK after execution
            syncParams = g_cInitSyncParams;
            syncParams.GpuContext = m_renderContext;
            syncParams.presSyncResource = &m_resSyncObjectRenderContextInUse;

            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnEngineSignal(m_osInterface, &syncParams));
        }

        m_osInterface->pfnSetGpuContext(m_osInterface, m_videoContext);
        m_osInterface->pfnResetOsStates(m_osInterface);
    }

    return status;
}

MOS_STATUS CodechalEncodeVp8::ReadImageStatus(PMOS_COMMAND_BUFFER cmdBuffer)
{
    MhwMiInterface                      *commonMiInterface;
    EncodeStatusBuffer                  *encodeStatusBuf;
    MHW_MI_STORE_REGISTER_MEM_PARAMS    miStoreRegMemParams;
    MHW_MI_FLUSH_DW_PARAMS              flushDwParams;
    uint32_t                            baseOffset;
    MmioRegistersMfx                    *mmioRegisters;
    MOS_STATUS                          status = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(m_hwInterface->GetMiInterface());
    CODECHAL_ENCODE_CHK_NULL_RETURN(cmdBuffer);

    commonMiInterface = m_hwInterface->GetMiInterface();

    CODECHAL_ENCODE_CHK_COND_RETURN((m_vdboxIndex > m_hwInterface->GetMfxInterface()->GetMaxVdboxIndex()), "ERROR - vdbox index exceed the maximum");
    mmioRegisters = m_hwInterface->GetMfxInterface()->GetMmioRegisters(m_vdboxIndex);

    commonMiInterface = m_hwInterface->GetMiInterface();
    encodeStatusBuf = &m_encodeStatusBuf;

    baseOffset =
        (encodeStatusBuf->wCurrIndex * m_encodeStatusBuf.dwReportSize) +
        sizeof(uint32_t) * 2;  // pEncodeStatus is offset by 2 DWs in the resource

    miStoreRegMemParams.presStoreBuffer = &encodeStatusBuf->resStatusBuffer;
    miStoreRegMemParams.dwOffset = baseOffset + encodeStatusBuf->dwImageStatusMaskOffset;
    miStoreRegMemParams.dwRegister = mmioRegisters->mfcVP8ImageStatusMaskRegOffset;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(commonMiInterface->AddMiStoreRegisterMemCmd(cmdBuffer, &miStoreRegMemParams));

    miStoreRegMemParams.presStoreBuffer = &encodeStatusBuf->resStatusBuffer;
    miStoreRegMemParams.dwOffset = baseOffset + encodeStatusBuf->dwImageStatusCtrlOffset;
    miStoreRegMemParams.dwRegister = mmioRegisters->mfcVP8ImageStatusCtrlRegOffset;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(commonMiInterface->AddMiStoreRegisterMemCmd(cmdBuffer, &miStoreRegMemParams));

    MOS_ZeroMemory(&flushDwParams, sizeof(flushDwParams));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(commonMiInterface->AddMiFlushDwCmd(cmdBuffer, &flushDwParams));

    return status;
}

MOS_STATUS CodechalEncodeVp8::ReadMfcStatus(PMOS_COMMAND_BUFFER cmdBuffer)
{
    MhwMiInterface                      *commonMiInterface;
    EncodeStatusBuffer                  *encodeStatusBuf;
    MHW_MI_STORE_REGISTER_MEM_PARAMS    miStoreRegMemParams;
    MHW_MI_FLUSH_DW_PARAMS              flushDwParams;
    MmioRegistersMfx                    *mmioRegisters;
    uint32_t                            baseOffset;
    MOS_STATUS                          status = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(m_hwInterface->GetMiInterface());
    CODECHAL_ENCODE_CHK_NULL_RETURN(cmdBuffer);

    commonMiInterface = m_hwInterface->GetMiInterface();

    CODECHAL_ENCODE_CHK_COND_RETURN((m_vdboxIndex > m_hwInterface->GetMfxInterface()->GetMaxVdboxIndex()), "ERROR - vdbox index exceed the maximum");
    mmioRegisters = m_hwInterface->GetMfxInterface()->GetMmioRegisters(m_vdboxIndex);

    encodeStatusBuf = &m_encodeStatusBuf;

    baseOffset =
        (encodeStatusBuf->wCurrIndex * m_encodeStatusBuf.dwReportSize) +
        sizeof(uint32_t) * 2;  // pEncodeStatus is offset by 2 DWs in the resource

    MOS_ZeroMemory(&flushDwParams, sizeof(flushDwParams));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(commonMiInterface->AddMiFlushDwCmd(cmdBuffer, &flushDwParams));

    miStoreRegMemParams.presStoreBuffer = &encodeStatusBuf->resStatusBuffer;
    miStoreRegMemParams.dwOffset = baseOffset + encodeStatusBuf->dwBSByteCountOffset;
    miStoreRegMemParams.dwRegister = mmioRegisters->mfcVP8BitstreamBytecountFrameRegOffset;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(commonMiInterface->AddMiStoreRegisterMemCmd(cmdBuffer, &miStoreRegMemParams));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(ReadImageStatus(cmdBuffer))

        return status;
}

MOS_STATUS CodechalEncodeVp8::ReadBrcPakStatistics(
    PMOS_COMMAND_BUFFER cmdBuffer,
    EncodeReadBrcPakStatsParams* params)
{
    MhwMiInterface                      *commonMiInterface;
    MHW_MI_STORE_REGISTER_MEM_PARAMS    miStoreRegMemParams;
    MHW_MI_FLUSH_DW_PARAMS              flushDwParams;
    MHW_MI_STORE_DATA_PARAMS            storeDataParams;
    MmioRegistersMfx                    *mmioRegisters;
    MOS_STATUS                          status = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(cmdBuffer);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pHwInterface->GetMiInterface());
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->presBrcPakStatisticBuffer);

    commonMiInterface = params->pHwInterface->GetMiInterface();

    CODECHAL_ENCODE_CHK_COND_RETURN((m_vdboxIndex > m_hwInterface->GetMfxInterface()->GetMaxVdboxIndex()), "ERROR - vdbox index exceed the maximum");
    mmioRegisters = m_hwInterface->GetMfxInterface()->GetMmioRegisters(m_vdboxIndex);

    // MI_FLUSH
    MOS_ZeroMemory(&flushDwParams, sizeof(flushDwParams));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(commonMiInterface->AddMiFlushDwCmd(cmdBuffer, &flushDwParams));

    // PAK statistics buffer DW0, DW2, DW4 are used by BrcUpdate kernel exeuction in the next frame

    // PAK stats needs to be updated from PAK pass number from (N)th PAK pass  N: # of PAK pass executed
    if (params->ucPass < m_hwInterface->GetMfxInterface()->GetBrcNumPakPasses())
    {
        // MI_STORE_DATA_IMM
        MOS_ZeroMemory(&storeDataParams, sizeof(storeDataParams));
        storeDataParams.pOsResource = params->presBrcPakStatisticBuffer;
        storeDataParams.dwResourceOffset = sizeof(uint32_t) * 2;
        storeDataParams.dwValue = ((uint32_t)params->ucPass + 1) << 8;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(commonMiInterface->AddMiStoreDataImmCmd(cmdBuffer, &storeDataParams));
    }

    // MI_STORE_REGISTER_MEM - read from MMIO memory & write to buffer
    MOS_ZeroMemory(&miStoreRegMemParams, sizeof(miStoreRegMemParams));
    miStoreRegMemParams.presStoreBuffer = params->presBrcPakStatisticBuffer;
    miStoreRegMemParams.dwOffset = 0;
    miStoreRegMemParams.dwRegister = mmioRegisters->mfcVP8BitstreamBytecountFrameRegOffset;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(commonMiInterface->AddMiStoreRegisterMemCmd(cmdBuffer, &miStoreRegMemParams));

    // Read following registers as this will trigger HW to register these values
    // and update cumulative Qp and LF registers for next pass
    // PAK statistics buffer (DW5-6 & DW9-13) is used to write these values
    // assuming BrcUpdate kernel doesn't need to use this later
    // Driver just need buffer to write these values, but driver doesn't need to use them later
    MOS_ZeroMemory(&miStoreRegMemParams, sizeof(miStoreRegMemParams));
    miStoreRegMemParams.presStoreBuffer = params->presBrcPakStatisticBuffer;
    miStoreRegMemParams.dwOffset = sizeof(uint32_t) * 5;
    miStoreRegMemParams.dwRegister = mmioRegisters->mfxVP8BrcDQIndexRegOffset;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(commonMiInterface->AddMiStoreRegisterMemCmd(cmdBuffer, &miStoreRegMemParams));

    MOS_ZeroMemory(&miStoreRegMemParams, sizeof(miStoreRegMemParams));
    miStoreRegMemParams.presStoreBuffer = params->presBrcPakStatisticBuffer;
    miStoreRegMemParams.dwOffset = sizeof(uint32_t) * 6;
    miStoreRegMemParams.dwRegister = mmioRegisters->mfxVP8BrcDLoopFilterRegOffset;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(commonMiInterface->AddMiStoreRegisterMemCmd(cmdBuffer, &miStoreRegMemParams));

    // PAK stats needs to be updated from QP/LF delta values from (N-1)th PAK pass  N: # of PAK pass executed
    if (params->ucPass == 0)   // need to update if # of maximum PAK pass > 2 in the future
    {
        // MFX_VP8_BRC_CumulativeDQindex01 MMIO register read needs to come after 
        // MFX_VP8_BRC_DQindex & MFX_VP8_BRC_DLoopFilter MMIO register reads
        // otherwise, cumulative QP value will not be read correctly
        MOS_ZeroMemory(&miStoreRegMemParams, sizeof(miStoreRegMemParams));
        miStoreRegMemParams.presStoreBuffer = params->presBrcPakStatisticBuffer;
        miStoreRegMemParams.dwOffset = sizeof(uint32_t) * 4;
        miStoreRegMemParams.dwRegister = mmioRegisters->mfxVP8BrcCumulativeDQIndex01RegOffset;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(commonMiInterface->AddMiStoreRegisterMemCmd(cmdBuffer, &miStoreRegMemParams));
    }

    // DW7-8 skipped on purpose since BrcUpdate Kernel started to use DW8 for Golden Ref Suggestion
    // dwMfxVP8BrcCumulativeDQIndex01RegOffset: already read with DW4 above
    MOS_ZeroMemory(&miStoreRegMemParams, sizeof(miStoreRegMemParams));
    miStoreRegMemParams.presStoreBuffer = params->presBrcPakStatisticBuffer;
    miStoreRegMemParams.dwOffset = sizeof(uint32_t) * 9;
    miStoreRegMemParams.dwRegister = mmioRegisters->mfxVP8BrcCumulativeDQIndex01RegOffset;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(commonMiInterface->AddMiStoreRegisterMemCmd(cmdBuffer, &miStoreRegMemParams));

    MOS_ZeroMemory(&miStoreRegMemParams, sizeof(miStoreRegMemParams));
    miStoreRegMemParams.presStoreBuffer = params->presBrcPakStatisticBuffer;
    miStoreRegMemParams.dwOffset = sizeof(uint32_t) * 10;
    miStoreRegMemParams.dwRegister = mmioRegisters->mfxVP8BrcCumulativeDQIndex23RegOffset;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(commonMiInterface->AddMiStoreRegisterMemCmd(cmdBuffer, &miStoreRegMemParams));

    MOS_ZeroMemory(&miStoreRegMemParams, sizeof(miStoreRegMemParams));
    miStoreRegMemParams.presStoreBuffer = params->presBrcPakStatisticBuffer;
    miStoreRegMemParams.dwOffset = sizeof(uint32_t) * 11;
    miStoreRegMemParams.dwRegister = mmioRegisters->mfxVP8BrcCumulativeDLoopFilter01RegOffset;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(commonMiInterface->AddMiStoreRegisterMemCmd(cmdBuffer, &miStoreRegMemParams));

    MOS_ZeroMemory(&miStoreRegMemParams, sizeof(miStoreRegMemParams));
    miStoreRegMemParams.presStoreBuffer = params->presBrcPakStatisticBuffer;
    miStoreRegMemParams.dwOffset = sizeof(uint32_t) * 12;
    miStoreRegMemParams.dwRegister = mmioRegisters->mfxVP8BrcCumulativeDLoopFilter23RegOffset;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(commonMiInterface->AddMiStoreRegisterMemCmd(cmdBuffer, &miStoreRegMemParams));

    MOS_ZeroMemory(&miStoreRegMemParams, sizeof(miStoreRegMemParams));
    miStoreRegMemParams.presStoreBuffer = params->presBrcPakStatisticBuffer;
    miStoreRegMemParams.dwOffset = sizeof(uint32_t) * 13;
    miStoreRegMemParams.dwRegister = mmioRegisters->mfxVP8BrcConvergenceStatusRegOffset;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(commonMiInterface->AddMiStoreRegisterMemCmd(cmdBuffer, &miStoreRegMemParams));

    return status;
}

MOS_STATUS CodechalEncodeVp8::GetStatusReport(
        EncodeStatus       *pEncodeStatus,
        EncodeStatusReport *pEncodeStatusReport)
{
    uint32_t                   longTermIndication;
    uint32_t                   qindex;
    PMOS_RESOURCE              pakBuffer;
    uint8_t                    *data;
    MOS_LOCK_PARAMS            lockFlagsReadOnly;
    MOS_STATUS                 status = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(pEncodeStatus);
    CODECHAL_ENCODE_CHK_NULL_RETURN(pEncodeStatusReport);

    pakBuffer = &BrcBuffers.resBrcPakStatisticBuffer[0];

    MOS_ZeroMemory(&lockFlagsReadOnly, sizeof(MOS_LOCK_PARAMS));
    lockFlagsReadOnly.ReadOnly = 1;

    data = (uint8_t*)m_osInterface->pfnLockResource(m_osInterface, pakBuffer, &lockFlagsReadOnly);

    CODECHAL_ENCODE_CHK_NULL_RETURN(data);

    longTermIndication = *(data + sizeof(uint32_t) * 8); // reading longterm reference suggestion

    qindex = *(data + sizeof(uint32_t) * 4); // reading qindex
    qindex = qindex & 0x7f; // masking for 7 bits

    m_osInterface->pfnUnlockResource(m_osInterface, pakBuffer);

    pEncodeStatusReport->LongTermIndication = (uint8_t)longTermIndication;

    pEncodeStatusReport->CodecStatus = CODECHAL_STATUS_SUCCESSFUL;
    pEncodeStatusReport->bitstreamSize = pEncodeStatus->dwMFCBitstreamByteCountPerFrame + pEncodeStatus->dwHeaderBytesInserted;
    pEncodeStatusReport->AverageQp = (uint8_t)qindex;
    // This loop filter value needs to revisit if the app needs to get a right value and do something
    pEncodeStatusReport->loopFilterLevel = pEncodeStatus->LoopFilterLevel;

    return status;
}

MOS_STATUS CodechalEncodeVp8::SendBrcUpdateSurfaces(
    PMOS_COMMAND_BUFFER cmdBuffer, 
    struct CodechalVp8BrcUpdateSurfaceParams* params)
{
    PMHW_STATE_HEAP_INTERFACE                   stateHeapInterface;
    PMHW_KERNEL_STATE                           kernelState;
    CODECHAL_SURFACE_CODEC_PARAMS               surfaceCodecParams;
    struct CodechalBindingTableVp8BrcUpdate*    vp8BrcUpdateBindingTable;
    uint32_t                                    size;
    MOS_RESOURCE                                *dsh = nullptr;
    MOS_STATUS                                  status = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(m_hwInterface->GetRenderInterface());
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_hwInterface->GetRenderInterface());
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_hwInterface->GetRenderInterface()->m_stateHeapInterface);
    CODECHAL_ENCODE_CHK_NULL_RETURN(cmdBuffer);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pMbEncKernelState);


    vp8BrcUpdateBindingTable = &BrcUpdateBindingTable;
    stateHeapInterface = m_hwInterface->GetRenderInterface()->m_stateHeapInterface;
    kernelState = params->pKernelState;

    // 0. BRC history buffer
    size = ENCODE_VP8_BRC_HISTORY_BUFFER_SIZE;
    MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    surfaceCodecParams.presBuffer = params->presBrcHistoryBuffer;
    surfaceCodecParams.dwBindingTableOffset = vp8BrcUpdateBindingTable->dwBrcHistoryBuffer;
    surfaceCodecParams.dwSize = size;
    surfaceCodecParams.bIsWritable = true;
    surfaceCodecParams.bRenderTarget = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // 1. PAK Statistics buffer
    MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    surfaceCodecParams.presBuffer = params->presBrcPakStatisticBuffer;
    surfaceCodecParams.dwSize = params->dwBrcPakStatisticsSize;
    surfaceCodecParams.dwBindingTableOffset = vp8BrcUpdateBindingTable->dwBrcPakStatisticsOutputBuffer;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // 2. Encoder CFG command surface - read only
    size = BRC_IMG_STATE_SIZE_PER_PASS * 7;
    MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    surfaceCodecParams.presBuffer = params->presVp8EncoderCfgCommandWriteBuffer;
    surfaceCodecParams.dwSize = size;
    surfaceCodecParams.dwBindingTableOffset = vp8BrcUpdateBindingTable->dwBrcEncoderCfgReadBuffer;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // 3. Encoder CFG command surface - write only
    MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    surfaceCodecParams.presBuffer = params->presVp8EncoderCfgCommandWriteBuffer;
    surfaceCodecParams.dwSize = size;
    surfaceCodecParams.dwBindingTableOffset = vp8BrcUpdateBindingTable->dwBrcEncoderCfgWriteBuffer;
    surfaceCodecParams.bIsWritable = true;
    surfaceCodecParams.bRenderTarget = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // 4. BRC ENC CURBE Buffer - read only
    size = MOS_ALIGN_CEIL(
        params->pMbEncKernelState->KernelParams.iCurbeLength,
        stateHeapInterface->pStateHeapInterface->GetCurbeAlignment());
    MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    CODECHAL_ENCODE_CHK_NULL_RETURN(dsh = params->pMbEncKernelState->m_dshRegion.GetResource());
    surfaceCodecParams.presBuffer = dsh;
    surfaceCodecParams.dwOffset =
        params->pMbEncKernelState->m_dshRegion.GetOffset() +
        params->pMbEncKernelState->dwCurbeOffset;
    surfaceCodecParams.dwSize = size;
    surfaceCodecParams.dwBindingTableOffset = vp8BrcUpdateBindingTable->dwBrcMbEncCurbeReadBuffer;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // 5. BRC ENC CURBE Buffer - write only
    MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    CODECHAL_ENCODE_CHK_NULL_RETURN(dsh = params->pMbEncKernelState->m_dshRegion.GetResource());
    surfaceCodecParams.presBuffer = dsh;
    surfaceCodecParams.dwOffset =
        params->pMbEncKernelState->m_dshRegion.GetOffset() +
        params->pMbEncKernelState->dwCurbeOffset;
    surfaceCodecParams.dwSize = size;
    surfaceCodecParams.dwBindingTableOffset = vp8BrcUpdateBindingTable->dwBrcMbEncCurbeWriteData;
    surfaceCodecParams.bRenderTarget = true;
    surfaceCodecParams.bIsWritable = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // 6. VP8_ME BRC Distortion data buffer - input/output
    MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    surfaceCodecParams.bIs2DSurface = true;
    surfaceCodecParams.bMediaBlockRW = true;
    surfaceCodecParams.psSurface = (params->wPictureCodingType == I_TYPE) ?
        params->psMeBrcDistortionBuffer : params->ps4xMeDistortionBuffer;
    surfaceCodecParams.dwBindingTableOffset = vp8BrcUpdateBindingTable->dwBrcDistortionBuffer;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // 7. BRC Constant Data Surface
    size = BRC_CONSTANTSURFACE_VP8;
    MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    surfaceCodecParams.bRenderTarget = true;
    surfaceCodecParams.presBuffer = params->presVp8BrcConstantDataBuffer;
    surfaceCodecParams.dwBindingTableOffset = vp8BrcUpdateBindingTable->dwBrcConstantData;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // 8. Segmap surface - Enable this when Segmentation is enabled
    MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    surfaceCodecParams.bIs2DSurface = true;
    surfaceCodecParams.psSurface = params->psSegmentationMap;
    surfaceCodecParams.dwBindingTableOffset = vp8BrcUpdateBindingTable->dwVp8BrcSegmentationMap;
    surfaceCodecParams.bRenderTarget = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // 9. BRC Mpu CURBE Buffer - read only
    size = MOS_ALIGN_CEIL(
        MpuKernelState.KernelParams.iCurbeLength,
        stateHeapInterface->pStateHeapInterface->GetCurbeAlignment());
    MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    CODECHAL_ENCODE_CHK_NULL_RETURN(dsh = MpuKernelState.m_dshRegion.GetResource());
    surfaceCodecParams.presBuffer = dsh;
    surfaceCodecParams.dwOffset =
        MpuKernelState.m_dshRegion.GetOffset() +
        MpuKernelState.dwCurbeOffset;
    surfaceCodecParams.dwSize = size;
    surfaceCodecParams.dwBindingTableOffset = vp8BrcUpdateBindingTable->dwBrcMpuCurbeReadBuffer;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // 10. BRC Mpu CURBE Buffer - write only
    MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    memset(&dsh, 0, sizeof(dsh));
    CODECHAL_ENCODE_CHK_NULL_RETURN(dsh = MpuKernelState.m_dshRegion.GetResource());
    surfaceCodecParams.presBuffer = dsh;
    surfaceCodecParams.dwOffset =
        MpuKernelState.m_dshRegion.GetOffset() +
        MpuKernelState.dwCurbeOffset;
    surfaceCodecParams.dwSize = size;
    surfaceCodecParams.dwBindingTableOffset = vp8BrcUpdateBindingTable->dwBrcMpuCurbeWriteData;
    surfaceCodecParams.bRenderTarget = true;
    surfaceCodecParams.bIsWritable = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // 11. BRC Tpu CURBE Buffer - read only
    size = MOS_ALIGN_CEIL(
        TpuKernelState.KernelParams.iCurbeLength,
        stateHeapInterface->pStateHeapInterface->GetCurbeAlignment());
    MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    memset(&dsh, 0, sizeof(dsh));
    CODECHAL_ENCODE_CHK_NULL_RETURN(dsh = TpuKernelState.m_dshRegion.GetResource());
    surfaceCodecParams.presBuffer = dsh;
    surfaceCodecParams.dwOffset =
        TpuKernelState.m_dshRegion.GetOffset() +
        TpuKernelState.dwCurbeOffset;
    surfaceCodecParams.dwSize = size;
    surfaceCodecParams.dwBindingTableOffset = vp8BrcUpdateBindingTable->dwBrcTpuCurbeReadBuffer;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // 12. BRC Tpu CURBE Buffer - write only
    MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    memset(&dsh, 0, sizeof(dsh));
    CODECHAL_ENCODE_CHK_NULL_RETURN(dsh = TpuKernelState.m_dshRegion.GetResource());
    surfaceCodecParams.presBuffer = dsh;
    surfaceCodecParams.dwOffset =
        TpuKernelState.m_dshRegion.GetOffset() +
        TpuKernelState.dwCurbeOffset;
    surfaceCodecParams.dwSize = size;
    surfaceCodecParams.dwBindingTableOffset = vp8BrcUpdateBindingTable->dwBrcTpuCurbeWriteData;
    surfaceCodecParams.bRenderTarget = true;
    surfaceCodecParams.bIsWritable = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    return status;
}

MOS_STATUS CodechalEncodeVp8::SendMeSurfaces(
    PMOS_COMMAND_BUFFER cmdBuffer, 
    struct CodechalVp8MeSurfaceParams* params)
{
    PMOS_SURFACE                            scaledSurface, meMvDataBuffer;
    uint32_t                                width, height;
    uint8_t                                 lastRefPicIdx, goldenRefPicIdx, alternateRefPicIdx, scaledIdx;
    PMHW_KERNEL_STATE                       kernelState;
    CODECHAL_SURFACE_CODEC_PARAMS           surfaceCodecParams;
    struct CodechalBindingTableVp8Me*       vp8MeBindingTable;
    CODECHAL_MEDIA_STATE_TYPE               encMediaStateType;
    MOS_STATUS                              status = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(cmdBuffer);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pCurrReconstructedPic);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->ps4xMeMvDataBuffer);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->ps16xMeMvDataBuffer);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pTrackedBuffer);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pMeBindingTable);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pKernelState);

    kernelState = params->pKernelState;

    encMediaStateType = params->b16xMeInUse ? CODECHAL_MEDIA_STATE_16X_ME : CODECHAL_MEDIA_STATE_4X_ME;
    vp8MeBindingTable = params->pMeBindingTable;

    lastRefPicIdx = CODECHAL_ENCODE_VP8_INVALID_PIC_ID;
    goldenRefPicIdx = CODECHAL_ENCODE_VP8_INVALID_PIC_ID;
    alternateRefPicIdx = CODECHAL_ENCODE_VP8_INVALID_PIC_ID;
    scaledIdx = params->ppRefList[params->pCurrReconstructedPic->FrameIdx]->ucScalingIdx;

    if (!CodecHal_PictureIsInvalid(*params->pLastRefPic))
    {
        lastRefPicIdx = params->pLastRefPic->FrameIdx;
    }

    if (!CodecHal_PictureIsInvalid(*params->pGoldenRefPic))
    {
        goldenRefPicIdx = params->pGoldenRefPic->FrameIdx;
    }

    if (!CodecHal_PictureIsInvalid(*params->pAlternateRefPic))
    {
        alternateRefPicIdx = params->pAlternateRefPic->FrameIdx;
    }

    if (params->b16xMeInUse)
    {
        scaledSurface = &params->pTrackedBuffer[scaledIdx].sScaled16xSurface;
        meMvDataBuffer = params->ps16xMeMvDataBuffer;
    }
    else
    {
        scaledSurface = &params->pTrackedBuffer[scaledIdx].sScaled4xSurface;
        meMvDataBuffer = params->ps4xMeMvDataBuffer;
    }

    width = params->dwDownscaledWidthInMb * 32;
    width = MOS_ALIGN_CEIL(width, 64);

    height = params->dwDownscaledHeightInMb * 4 * CODECHAL_VP8_ME_ME_DATA_SIZE_MULTIPLIER;

    // Force the values
    meMvDataBuffer->dwWidth = width;
    meMvDataBuffer->dwHeight = height;
    meMvDataBuffer->dwPitch = width;

    MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    surfaceCodecParams.bIs2DSurface = true;
    surfaceCodecParams.bMediaBlockRW = true;
    surfaceCodecParams.psSurface = meMvDataBuffer;
    surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_MV_DATA_ENCODE].Value;
    surfaceCodecParams.dwBindingTableOffset = vp8MeBindingTable->dwVp8MEMVDataSurface;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // Insert 16xMe Mv Buffer when 16xMe is enabled
    if (params->b16xMeEnabled)
    {
        MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        surfaceCodecParams.bIs2DSurface = true;
        surfaceCodecParams.bMediaBlockRW = true;
        surfaceCodecParams.psSurface = params->ps16xMeMvDataBuffer;
        surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_MV_DATA_ENCODE].Value;
        surfaceCodecParams.dwBindingTableOffset = vp8MeBindingTable->dwVp816xMEMVDataSurface;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));
    }

    // Insert Distortion buffers only for 4xMe case
    if (!params->b16xMeInUse)
    {
        MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        surfaceCodecParams.bIs2DSurface = true;
        surfaceCodecParams.bMediaBlockRW = true;
        surfaceCodecParams.psSurface = params->psMeDistortionBuffer;
        surfaceCodecParams.dwBindingTableOffset = vp8MeBindingTable->dwVp8MeDist;
        surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_ME_DISTORTION_ENCODE].Value;
        surfaceCodecParams.bIsWritable = true;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));

        MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        surfaceCodecParams.bIs2DSurface = true;
        surfaceCodecParams.bMediaBlockRW = true;
        surfaceCodecParams.psSurface = params->psMeBrcDistortionBuffer; // Using the Distortion buffer only, not yet allocated the BRC specifc distortion buffer
        surfaceCodecParams.dwBindingTableOffset = vp8MeBindingTable->dwVp8MeBrcDist;
        surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_ME_DISTORTION_ENCODE].Value;
        surfaceCodecParams.bIsWritable = true;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));
    }

    // Current Picture - VME Inter Prediction Surface 
    MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    surfaceCodecParams.bUseAdvState = true;
    surfaceCodecParams.psSurface = scaledSurface;
    surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_CURR_ENCODE].Value;
    surfaceCodecParams.dwBindingTableOffset = vp8MeBindingTable->dwVp8MeCurrPic;
    surfaceCodecParams.ucVDirection = g_cMhw_VDirection[MHW_FRAME];
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // Refctrl uses 3 bits- bit zero is Last ref, bit 1 is golden ref and bit 2 is alt ref 

    //Last - set this ref surface only for appropriate RefCtrl value
    scaledIdx = params->ppRefList[lastRefPicIdx]->ucScalingIdx;
    MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    surfaceCodecParams.bUseAdvState = true;
    surfaceCodecParams.psSurface = params->b16xMeInUse ?
        (&params->pTrackedBuffer[scaledIdx].sScaled16xSurface) :
        (&params->pTrackedBuffer[scaledIdx].sScaled4xSurface);
    surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_CURR_ENCODE].Value;
    surfaceCodecParams.ucVDirection = g_cMhw_VDirection[MHW_FRAME];

    switch (params->RefCtrl)
    {
    case 1:
    case 3:
    case 5:
    case 7:
        surfaceCodecParams.dwBindingTableOffset = vp8MeBindingTable->dwVp8MeRef1Pic;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));
        break;
    }

    //Golden- set this ref surface only for appropriate RefCtrl value
    scaledIdx = params->ppRefList[goldenRefPicIdx]->ucScalingIdx;
    MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    surfaceCodecParams.bUseAdvState = true;
    surfaceCodecParams.psSurface = params->b16xMeInUse ?
        (&params->pTrackedBuffer[scaledIdx].sScaled16xSurface) :
        (&params->pTrackedBuffer[scaledIdx].sScaled4xSurface);
    surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_CURR_ENCODE].Value;
    surfaceCodecParams.ucVDirection = g_cMhw_VDirection[MHW_FRAME];

    switch (params->RefCtrl)
    {
    case 2:
    case 6:
        surfaceCodecParams.dwBindingTableOffset = vp8MeBindingTable->dwVp8MeRef1Pic;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));
        break;
    case 3:
    case 7:
        surfaceCodecParams.dwBindingTableOffset = vp8MeBindingTable->dwVp8MeRef2Pic;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));
        break;
    }

    //Alt Ref- set this ref surface only for appropriate RefCtrl value
    scaledIdx = params->ppRefList[alternateRefPicIdx]->ucScalingIdx;
    MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    surfaceCodecParams.bUseAdvState = true;
    surfaceCodecParams.psSurface = params->b16xMeInUse ?
        (&params->pTrackedBuffer[scaledIdx].sScaled16xSurface) :
        (&params->pTrackedBuffer[scaledIdx].sScaled4xSurface);
    surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_CURR_ENCODE].Value;
    surfaceCodecParams.ucVDirection = g_cMhw_VDirection[MHW_FRAME];

    switch (params->RefCtrl)
    {
    case 4:
        surfaceCodecParams.dwBindingTableOffset = vp8MeBindingTable->dwVp8MeRef1Pic;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));
        break;
    case 5:
    case 6:
        surfaceCodecParams.dwBindingTableOffset = vp8MeBindingTable->dwVp8MeRef2Pic;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));
        break;
    case 7:
        surfaceCodecParams.dwBindingTableOffset = vp8MeBindingTable->dwVp8MeRef3Pic;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));
        break;
    }

    return status;
}

MOS_STATUS CodechalEncodeVp8::SendTpuSurfaces(
    PMOS_COMMAND_BUFFER cmdBuffer, 
    struct CodechalVp8TpuSurfaceParams* params)
{
    uint32_t                                size;
    PMHW_KERNEL_STATE                       kernelState;
    CODECHAL_SURFACE_CODEC_PARAMS           surfaceCodecParams;
    struct CodechalBindingTableVp8Tpu*      vp8TpuBindingTable;
    MOS_STATUS                              status = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(cmdBuffer);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pKernelState);

    vp8TpuBindingTable = &TpuBindingTable;

    kernelState = params->pKernelState;

    // Pak token statistics
    size = params->dwPakTokenStatsSize;
    MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    surfaceCodecParams.dwSize = size;
    surfaceCodecParams.presBuffer = params->presPakTokenStatistics;
    surfaceCodecParams.dwBindingTableOffset = vp8TpuBindingTable->dwVp8TpuPakTokenStatistics;
    surfaceCodecParams.bRenderTarget = true;
    surfaceCodecParams.bRawSurface = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // Pak token Update flags
    size = params->dwTokenProbabilitySize;
    MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    surfaceCodecParams.dwSize = size;
    surfaceCodecParams.presBuffer = params->presPakTokenUpdateFlags;
    surfaceCodecParams.dwBindingTableOffset = vp8TpuBindingTable->dwVp8TpuTokenUpdateFlags;
    surfaceCodecParams.bRenderTarget = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // Entropy cost
    size = params->dwEntropyCostTableSize;
    MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    surfaceCodecParams.dwSize = size;
    surfaceCodecParams.presBuffer = params->presEntropyCostTable;
    surfaceCodecParams.dwBindingTableOffset = vp8TpuBindingTable->dwVp8TpuEntropyCost;
    surfaceCodecParams.bRenderTarget = true;
    surfaceCodecParams.bRawSurface = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // Frame header 
    size = params->dwFrameHeaderSize;
    MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    surfaceCodecParams.dwSize = size;
    surfaceCodecParams.presBuffer = params->presFrameHeader;
    surfaceCodecParams.dwBindingTableOffset = vp8TpuBindingTable->dwVp8TpuFrameHeaderBitstream;
    surfaceCodecParams.bRenderTarget = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // Default token token probability
    size = params->dwTokenProbabilitySize;
    MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    surfaceCodecParams.dwSize = size;
    surfaceCodecParams.presBuffer = params->presDefaultTokenProbability;
    surfaceCodecParams.dwBindingTableOffset = vp8TpuBindingTable->dwVp8TpuDefaultTokenProbability;
    surfaceCodecParams.bRenderTarget = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // Picture state surface
    size = params->dwPictureStateSize;
    MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    surfaceCodecParams.dwSize = size;
    surfaceCodecParams.presBuffer = params->presPictureState;
    surfaceCodecParams.dwBindingTableOffset = vp8TpuBindingTable->dwVp8TpuPictureState;
    surfaceCodecParams.bRenderTarget = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // MPU Curbe info from TPU
    size = params->dwMpuCurbeSize;
    MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    surfaceCodecParams.dwSize = size;
    surfaceCodecParams.presBuffer = params->presMpuCurbeData;
    surfaceCodecParams.dwBindingTableOffset = vp8TpuBindingTable->dwVp8TpuMpuCurbeData;
    surfaceCodecParams.bRenderTarget = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // Encoder CFG command surface
    size = params->dwHeaderMetadataSize;
    MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    surfaceCodecParams.dwSize = size;
    surfaceCodecParams.dwOffset = params->dwHeaderMetaDataOffset;
    surfaceCodecParams.presBuffer = params->presHeaderMetadata;
    surfaceCodecParams.dwBindingTableOffset = vp8TpuBindingTable->dwVp8TpuHeaderMetaData;
    surfaceCodecParams.bRenderTarget = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // Current frame token probability
    size = params->dwTokenProbabilitySize;
    MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    surfaceCodecParams.dwSize = size;
    surfaceCodecParams.presBuffer = params->presCurrFrameTokenProbability;
    surfaceCodecParams.dwBindingTableOffset = vp8TpuBindingTable->dwVp8TpuTokenProbability;
    surfaceCodecParams.bRenderTarget = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // Hardware token probability pass 1
    size = params->dwTokenProbabilitySize;
    MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    surfaceCodecParams.dwSize = size;
    surfaceCodecParams.presBuffer = params->presHwTokenProbabilityPass1;
    surfaceCodecParams.dwBindingTableOffset = vp8TpuBindingTable->dwVp8TpuPakHardwareTokenProbabilityPass1;
    surfaceCodecParams.bRenderTarget = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // key frame token probability
    size = params->dwTokenProbabilitySize;
    MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    surfaceCodecParams.dwSize = size;
    surfaceCodecParams.presBuffer = params->presKeyFrameTokenProbability;
    surfaceCodecParams.dwBindingTableOffset = vp8TpuBindingTable->dwVp8TpuKeyFrameTokenProbability;
    surfaceCodecParams.bRenderTarget = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // update token probability
    size = params->dwTokenProbabilitySize;
    MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    surfaceCodecParams.dwSize = size;
    surfaceCodecParams.presBuffer = params->presUpdatedFrameTokenProbability;
    surfaceCodecParams.dwBindingTableOffset = vp8TpuBindingTable->dwVp8TpuUpdatedTokenProbability;
    surfaceCodecParams.bRenderTarget = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // Hardware token probability pass 2
    size = params->dwTokenProbabilitySize;
    MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    surfaceCodecParams.dwSize = size;
    surfaceCodecParams.presBuffer = params->presHwTokenProbabilityPass2;
    surfaceCodecParams.dwBindingTableOffset = vp8TpuBindingTable->dwVp8TpuPakHardwareTokenProbabilityPass2;
    surfaceCodecParams.bRenderTarget = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    if (params->bVMEKernelDump)
    {
        size = params->dwKernelDumpSize;
        MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        surfaceCodecParams.presBuffer = params->presVmeKernelDumpBuffer;
        surfaceCodecParams.dwSize = size;
        surfaceCodecParams.dwBindingTableOffset = vp8TpuBindingTable->dwVp8TpuKernelDebugDump;
        surfaceCodecParams.bRenderTarget = true;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));
    }

    size = params->dwRepakDecision;
    MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    surfaceCodecParams.dwSize = size;
    surfaceCodecParams.presBuffer = params->presRepakDecisionSurface;
    surfaceCodecParams.dwBindingTableOffset = vp8TpuBindingTable->dwVp8TpuRepakDecision;
    surfaceCodecParams.bRenderTarget = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    return status;
}

MOS_STATUS CodechalEncodeVp8::SendMbEncSurfaces(
    PMOS_COMMAND_BUFFER cmdBuffer, 
    struct CodechalVp8MbencSurfaceParams* params)
{
    uint32_t                                size;
    uint8_t                                 lastRefPicIdx, goldenRefPicIdx, alternateRefPicIdx;
    PMHW_KERNEL_STATE                       kernelState;
    CODECHAL_SURFACE_CODEC_PARAMS           surfaceCodecParams;
    struct CodechalBindingTableVp8Mbenc*    vp8MbEncBindingTable;
    MOS_STATUS                              status = MOS_STATUS_SUCCESS;
    uint8_t                                 vdirection, scaledIdx;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(cmdBuffer);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pCurrReconstructedPic);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->psCurrPicSurface);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pMbEncBindingTable);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pKernelState);

    lastRefPicIdx = CODECHAL_ENCODE_VP8_INVALID_PIC_ID;
    goldenRefPicIdx = CODECHAL_ENCODE_VP8_INVALID_PIC_ID;
    alternateRefPicIdx = CODECHAL_ENCODE_VP8_INVALID_PIC_ID;

    vp8MbEncBindingTable = params->pMbEncBindingTable;

    m_osInterface = m_hwInterface->GetOsInterface();
    kernelState = params->pKernelState;

    vdirection = CODECHAL_VDIRECTION_FRAME;

    // Per-MB output data buffer
    size = params->dwFrameWidthInMb * params->dwFrameFieldHeightInMb * 16;
    MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    surfaceCodecParams.dwSize = size;
    surfaceCodecParams.bMediaBlockRW = true;
    surfaceCodecParams.presBuffer = params->presPerMB_MBCodeOpData;
    surfaceCodecParams.dwBindingTableOffset = vp8MbEncBindingTable->dwVp8MBEncMBOut;
    surfaceCodecParams.bRenderTarget = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // Current Picture Y
    CodecHalGetResourceInfo(m_osInterface, params->psCurrPicSurface);

    MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    surfaceCodecParams.bIs2DSurface = true;
    surfaceCodecParams.bMediaBlockRW = true;
    surfaceCodecParams.bUseUVPlane = true;
    surfaceCodecParams.psSurface = params->psCurrPicSurface;
    surfaceCodecParams.dwOffset = params->dwCurrPicSurfaceOffset;
    surfaceCodecParams.dwCacheabilityControl =
        m_hwInterface->ComposeSurfaceCacheabilityControl(
            MOS_CODEC_RESOURCE_USAGE_SURFACE_CURR_ENCODE,
            (codechalL3 | codechalLLC));
    surfaceCodecParams.dwBindingTableOffset = vp8MbEncBindingTable->dwVp8MBEncCurrY;
    surfaceCodecParams.dwUVBindingTableOffset = vp8MbEncBindingTable->dwVp8MBEncCurrUV;
    surfaceCodecParams.dwVerticalLineStride = params->dwVerticalLineStride;
    surfaceCodecParams.dwVerticalLineStrideOffset = params->dwVerticalLineStrideOffset;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // Current Picture - VME Prediction Surface 
    MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    surfaceCodecParams.bUseAdvState = true;
    surfaceCodecParams.psSurface = params->psCurrPicSurface;
    surfaceCodecParams.dwOffset = params->dwCurrPicSurfaceOffset;
    surfaceCodecParams.dwCacheabilityControl =
        m_hwInterface->ComposeSurfaceCacheabilityControl(
            MOS_CODEC_RESOURCE_USAGE_SURFACE_CURR_ENCODE,
            (codechalL3 | codechalLLC));
    surfaceCodecParams.dwBindingTableOffset = vp8MbEncBindingTable->dwVp8MBEncVME;
    surfaceCodecParams.ucVDirection = vdirection;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    if (params->wPictureCodingType == I_TYPE)
    {
        // MB Mode Cost Luma buffer
        MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        surfaceCodecParams.bIs2DSurface = true;
        surfaceCodecParams.psSurface = params->psMBModeCostLumaBuffer;
        surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_VP8_L3_LLC_ENCODE].Value;
        surfaceCodecParams.dwBindingTableOffset = vp8MbEncBindingTable->dwVp8MBEncMBModeCostLuma;
        surfaceCodecParams.bRenderTarget = true;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));

        // Block Mode Cost buffer
        MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        surfaceCodecParams.bIs2DSurface = true;
        surfaceCodecParams.psSurface = params->psBlockModeCostBuffer;
        surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_VP8_L3_LLC_ENCODE].Value;
        surfaceCodecParams.dwBindingTableOffset = vp8MbEncBindingTable->dwVp8MBEncBlockModeCost;
        surfaceCodecParams.bRenderTarget = true;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));

        // Chroma Recon Buffer 
        size = 64 * params->dwFrameWidthInMb * params->dwFrameFieldHeightInMb;
        MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        surfaceCodecParams.dwSize = size;
        surfaceCodecParams.presBuffer = params->psChromaReconBuffer;
        surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_VP8_L3_LLC_ENCODE].Value;
        surfaceCodecParams.dwBindingTableOffset = vp8MbEncBindingTable->dwVp8MBEncChromaRecon;
        surfaceCodecParams.bRenderTarget = true;
        surfaceCodecParams.bIsWritable = true;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));

        // Histogram
        size = params->dwHistogramSize;
        MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        surfaceCodecParams.dwSize = size;
        //surfaceCodecParams.bMediaBlockRW              = true;
        surfaceCodecParams.presBuffer = params->presHistogram;
        surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_VP8_HISTOGRAM_ENCODE].Value;
        surfaceCodecParams.dwBindingTableOffset = vp8MbEncBindingTable->dwVp8MBEncHistogram;
        surfaceCodecParams.bRenderTarget = true;
        surfaceCodecParams.bRawSurface = true;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));

        if (params->bSegmentationEnabled)
        {
            // Segmentation map
            MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
            surfaceCodecParams.bIs2DSurface = true;
            surfaceCodecParams.psSurface = params->psSegmentationMap;
            surfaceCodecParams.dwBindingTableOffset = vp8MbEncBindingTable->dwVp8MBEncSegmentationMap;
            surfaceCodecParams.bRenderTarget = true;
            CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
                m_hwInterface,
                cmdBuffer,
                &surfaceCodecParams,
                kernelState));
        }

        // BRC distortion data buffer for I frame
        if (params->bMbEncIFrameDistInUse)
        {
            // Distortion Surface (output)
            MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
            surfaceCodecParams.bIs2DSurface = true;
            surfaceCodecParams.bMediaBlockRW = true;
            surfaceCodecParams.psSurface = params->psMeBrcDistortionBuffer;
            surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_VP8_L3_LLC_ENCODE].Value;
            surfaceCodecParams.dwBindingTableOffset = vp8MbEncBindingTable->dwVp8MBEncBRCDist;
            surfaceCodecParams.bIsWritable = true;
            surfaceCodecParams.bRenderTarget = true;
            CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
                m_hwInterface,
                cmdBuffer,
                &surfaceCodecParams,
                kernelState));

            // Curr Y downscaled (input)
            scaledIdx = params->ppRefList[params->pCurrReconstructedPic->FrameIdx]->ucScalingIdx;
            MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
            surfaceCodecParams.bIs2DSurface = true;
            surfaceCodecParams.psSurface = &params->pTrackedBuffer[scaledIdx].sScaled4xSurface;
            surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_CURR_ENCODE].Value;
            surfaceCodecParams.dwBindingTableOffset = vp8MbEncBindingTable->dwVp8MbEncCurrYDownscaled;
            surfaceCodecParams.ucVDirection = g_cMhw_VDirection[MHW_FRAME];
            CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
                m_hwInterface,
                cmdBuffer,
                &surfaceCodecParams,
                kernelState));

            // VME Coarse Intra downscaled (input)
            scaledIdx = params->ppRefList[params->pCurrReconstructedPic->FrameIdx]->ucScalingIdx;
            MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
            surfaceCodecParams.bUseAdvState = true;
            surfaceCodecParams.psSurface = &params->pTrackedBuffer[scaledIdx].sScaled4xSurface;
            surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_CURR_ENCODE].Value;
            surfaceCodecParams.dwBindingTableOffset = vp8MbEncBindingTable->dwVp8MBEncVMECoarseIntra;
            surfaceCodecParams.ucVDirection = g_cMhw_VDirection[MHW_FRAME];
            CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
                m_hwInterface,
                cmdBuffer,
                &surfaceCodecParams,
                kernelState));
        }
    }
    else
    {
        // P_TYPE
        // MV data buffer
        size = CODECHAL_GET_WIDTH_IN_MACROBLOCKS(params->dwOriFrameWidth) * CODECHAL_GET_HEIGHT_IN_MACROBLOCKS(params->dwOriFrameHeight) * 64 /*64 * NumMBInFrame*/;
        MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        surfaceCodecParams.dwSize = size;
        surfaceCodecParams.presBuffer = params->presPerMB_MBCodeOpData;
        surfaceCodecParams.bMediaBlockRW = true;
        surfaceCodecParams.dwOffset = params->dwMvOffset;
        surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_MV_DATA_ENCODE].Value;
        surfaceCodecParams.dwBindingTableOffset = vp8MbEncBindingTable->dwVp8MBEncIndMVData;
        surfaceCodecParams.bRenderTarget = true;
        surfaceCodecParams.bIsWritable = true;
        surfaceCodecParams.bRawSurface = true;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));

        // VP8_ME MV data buffer
        if (params->bHmeEnabled)
        {
            CODECHAL_ENCODE_CHK_NULL_RETURN(params->ps4xMeMvDataBuffer);

            MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
            surfaceCodecParams.bIs2DSurface = true;
            surfaceCodecParams.bMediaBlockRW = true;
            surfaceCodecParams.psSurface = params->ps4xMeMvDataBuffer;
            surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_MV_DATA_ENCODE].Value;
            surfaceCodecParams.dwBindingTableOffset = vp8MbEncBindingTable->dwVp8MBEncMVDataFromME;
            CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
                m_hwInterface,
                cmdBuffer,
                &surfaceCodecParams,
                kernelState));
        }

        //Reference Frame MB Count
        size = sizeof(uint32_t) * 8;
        MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        surfaceCodecParams.dwSize = size;
        surfaceCodecParams.bMediaBlockRW = true;
        surfaceCodecParams.presBuffer = params->presRefMbCountSurface;
        surfaceCodecParams.dwBindingTableOffset = vp8MbEncBindingTable->dwVp8MBEncRefMBCount;
        surfaceCodecParams.bRenderTarget = true;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));

        // VME Inter Prediction Surface 
        MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        surfaceCodecParams.bUseAdvState = true;
        surfaceCodecParams.psSurface = params->psCurrPicSurface;
        surfaceCodecParams.dwOffset = params->dwCurrPicSurfaceOffset;
        surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_CURR_ENCODE].Value;
        surfaceCodecParams.dwBindingTableOffset = vp8MbEncBindingTable->dwVp8MBEncVMEInterPred;
        surfaceCodecParams.ucVDirection = vdirection;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));

        if (!CodecHal_PictureIsInvalid(*params->pLastRefPic))
        {
            lastRefPicIdx = params->pLastRefPic->FrameIdx;
        }

        if (!CodecHal_PictureIsInvalid(*params->pGoldenRefPic))
        {
            goldenRefPicIdx = params->pGoldenRefPic->FrameIdx;
        }

        if (!CodecHal_PictureIsInvalid(*params->pAlternateRefPic))
        {
            alternateRefPicIdx = params->pAlternateRefPic->FrameIdx;
        }

        // Last reference
        if (lastRefPicIdx != CODECHAL_ENCODE_VP8_INVALID_PIC_ID)
        {
            // Picture Y VME
            MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
            surfaceCodecParams.bUseAdvState = true;
            surfaceCodecParams.psSurface = &params->ppRefList[lastRefPicIdx]->sRefBuffer;
            surfaceCodecParams.ucVDirection = vdirection;
            surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_CURR_ENCODE].Value;

            switch (params->uiRefCtrl)
            {
            case 1:
            case 3:
            case 5:
            case 7:
                surfaceCodecParams.dwBindingTableOffset = vp8MbEncBindingTable->dwVp8MBEncRef1Pic;
                CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
                    m_hwInterface,
                    cmdBuffer,
                    &surfaceCodecParams,
                    kernelState));
                break;
            }

        }

        // Golden reference
        if (goldenRefPicIdx != CODECHAL_ENCODE_VP8_INVALID_PIC_ID)
        {
            // Picture Y VME
            MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
            surfaceCodecParams.bUseAdvState = true;
            surfaceCodecParams.psSurface = &params->ppRefList[goldenRefPicIdx]->sRefBuffer;
            surfaceCodecParams.ucVDirection = vdirection;
            surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_CURR_ENCODE].Value;

            switch (params->uiRefCtrl)
            {
            case 2:
            case 6:
                surfaceCodecParams.dwBindingTableOffset = vp8MbEncBindingTable->dwVp8MBEncRef1Pic;
                CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
                    m_hwInterface,
                    cmdBuffer,
                    &surfaceCodecParams,
                    kernelState));
                break;
            case 3:
            case 7:
                surfaceCodecParams.dwBindingTableOffset = vp8MbEncBindingTable->dwVp8MBEncRef2Pic;
                CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
                    m_hwInterface,
                    cmdBuffer,
                    &surfaceCodecParams,
                    kernelState));
                break;
            }

        }

        // Alternate reference
        if (alternateRefPicIdx != CODECHAL_ENCODE_VP8_INVALID_PIC_ID)
        {
            // Picture Y VME
            MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
            surfaceCodecParams.bUseAdvState = true;
            surfaceCodecParams.psSurface = &params->ppRefList[alternateRefPicIdx]->sRefBuffer;
            surfaceCodecParams.ucVDirection = vdirection;
            surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_CURR_ENCODE].Value;

            switch (params->uiRefCtrl)
            {
            case 4:
                surfaceCodecParams.dwBindingTableOffset = vp8MbEncBindingTable->dwVp8MBEncRef1Pic;
                CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
                    m_hwInterface,
                    cmdBuffer,
                    &surfaceCodecParams,
                    kernelState));
                break;
            case 5:
            case 6:
                surfaceCodecParams.dwBindingTableOffset = vp8MbEncBindingTable->dwVp8MBEncRef2Pic;
                CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
                    m_hwInterface,
                    cmdBuffer,
                    &surfaceCodecParams,
                    kernelState));
                break;
            case 7:
                surfaceCodecParams.dwBindingTableOffset = vp8MbEncBindingTable->dwVp8MBEncRef3Pic;
                CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
                    m_hwInterface,
                    cmdBuffer,
                    &surfaceCodecParams,
                    kernelState));
                break;
            }
        }

        //Per MB Quant Data Surface
        MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        surfaceCodecParams.bIs2DSurface = true;
        surfaceCodecParams.bMediaBlockRW = true;
        surfaceCodecParams.psSurface = params->psPerMBQuantDataBuffer;
        surfaceCodecParams.dwBindingTableOffset = vp8MbEncBindingTable->dwVp8MBEncPerMBQuantDataP;
        surfaceCodecParams.bRenderTarget = true;
        surfaceCodecParams.bIsWritable = true;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));

        if (params->bSegmentationEnabled)
        {
            //Per Segmentation Map
            MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
            surfaceCodecParams.bIs2DSurface = true;
            surfaceCodecParams.psSurface = params->psSegmentationMap;
            surfaceCodecParams.dwBindingTableOffset = vp8MbEncBindingTable->dwVp8MBEncSegmentationMap;
            surfaceCodecParams.bRenderTarget = true;
            CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
                m_hwInterface,
                cmdBuffer,
                &surfaceCodecParams,
                kernelState));
        }

        //Inter Prediction Distortion Surface
        MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        surfaceCodecParams.bIs2DSurface = true;
        surfaceCodecParams.psSurface = params->psInterPredictionDistortionSurface;
        surfaceCodecParams.dwBindingTableOffset = vp8MbEncBindingTable->dwVp8InterPredDistortion;
        surfaceCodecParams.bRenderTarget = true;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));

        //Per MV Data Surface
        size = params->dwFrameWidthInMb * params->dwFrameFieldHeightInMb * 16;
        MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        surfaceCodecParams.dwSize = size;
        surfaceCodecParams.bMediaBlockRW = true;
        surfaceCodecParams.presBuffer = params->presPerMVDataSurface;
        surfaceCodecParams.dwBindingTableOffset = vp8MbEncBindingTable->dwVp8PerMVDataSurface;
        surfaceCodecParams.bRenderTarget = true;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));

        //MB/MV Counter Surface, initialize to zero
        size = params->dwHistogramSize;
        MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        surfaceCodecParams.dwSize = size;
        surfaceCodecParams.presBuffer = params->presHistogram;
        surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_VP8_HISTOGRAM_ENCODE].Value;
        surfaceCodecParams.dwBindingTableOffset = vp8MbEncBindingTable->dwVp8MBEncHistogram;
        surfaceCodecParams.bRenderTarget = true;
        surfaceCodecParams.bRawSurface = true;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));

        //Mode Cost Update Surface, 64 bytes
        size = CODECHAL_VP8_MODE_COST_SURFACE_SIZE;
        MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        surfaceCodecParams.dwSize = size;
        surfaceCodecParams.presBuffer = params->presModeCostUpdateSurface;
        surfaceCodecParams.dwBindingTableOffset = vp8MbEncBindingTable->dwVp8MBModeCostUpdateSurface;
        surfaceCodecParams.bRenderTarget = true;
        surfaceCodecParams.bRawSurface = true;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));

    }
    // Kernel Debug Dump surface
    if (params->bVMEKernelDump)
    {
        size = params->dwFrameWidthInMb * params->dwFrameFieldHeightInMb * 32;
        MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        surfaceCodecParams.presBuffer = params->presVmeKernelDumpBuffer;
        surfaceCodecParams.dwSize = size;
        surfaceCodecParams.dwOffset = params->dwMvOffset;
        surfaceCodecParams.dwBindingTableOffset = (params->wPictureCodingType == I_TYPE) ?
            vp8MbEncBindingTable->dwVp8MBEncVMEDebugStreamoutI : vp8MbEncBindingTable->dwVp8MBEncVMEDebugStreamoutP;
        surfaceCodecParams.bRenderTarget = true;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));
    }

    return status;
}

MOS_STATUS CodechalEncodeVp8::SetPakStatsInTpuCurbe(PMOS_COMMAND_BUFFER cmdBuffer)
{
    MhwMiInterface                      *commonMiInterface;
    PMHW_STATE_HEAP_INTERFACE           stateHeapInterface;
    MHW_MI_STORE_REGISTER_MEM_PARAMS    miStoreRegMemParams;
    MHW_MI_FLUSH_DW_PARAMS              flushDwParams;
    uint32_t                            baseOffset;
    PMOS_RESOURCE                       presTpuCurbeBuffer;
    MHW_MI_STORE_DATA_PARAMS            storeDataParams;
    MmioRegistersMfx                    *mmioRegisters;
    MOS_RESOURCE                        *dsh = nullptr;
    MOS_STATUS                          status = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(cmdBuffer);
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_hwInterface);
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_hwInterface->GetMiInterface());
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_hwInterface->GetRenderInterface()->m_stateHeapInterface);

    commonMiInterface = m_hwInterface->GetMiInterface();
    stateHeapInterface = m_hwInterface->GetRenderInterface()->m_stateHeapInterface;

    CODECHAL_ENCODE_CHK_COND_RETURN((m_vdboxIndex > m_hwInterface->GetMfxInterface()->GetMaxVdboxIndex()), "ERROR - vdbox index exceed the maximum");
    mmioRegisters = m_hwInterface->GetMfxInterface()->GetMmioRegisters(m_vdboxIndex);

    CODECHAL_ENCODE_CHK_NULL_RETURN(dsh = TpuKernelState.m_dshRegion.GetResource());
    presTpuCurbeBuffer = dsh;
    CODECHAL_ENCODE_CHK_NULL_RETURN(presTpuCurbeBuffer);

    baseOffset =
        TpuKernelState.m_dshRegion.GetOffset() +
        TpuKernelState.dwCurbeOffset;

    MOS_ZeroMemory(&storeDataParams, sizeof(storeDataParams));
    storeDataParams.pOsResource = presTpuCurbeBuffer;
    storeDataParams.dwResourceOffset = baseOffset + sizeof(uint32_t) * 6;
    storeDataParams.dwValue = ((uint32_t)m_currPass + 1) << 8;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(commonMiInterface->AddMiStoreDataImmCmd(cmdBuffer, &storeDataParams));

    // TPU curbe needs to be updated from QP/LF delta values from (N-1)th PAK pass  N: # of PAK pass executed
    if (m_currPass == 0)  // need to update if # of maximum PAK pass > 2 in the future
    {
        MOS_ZeroMemory(&miStoreRegMemParams, sizeof(miStoreRegMemParams));

        miStoreRegMemParams.presStoreBuffer = presTpuCurbeBuffer;
        miStoreRegMemParams.dwOffset = baseOffset + sizeof(uint32_t) * 8;
        miStoreRegMemParams.dwRegister = mmioRegisters->mfxVP8BrcCumulativeDQIndex01RegOffset;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(commonMiInterface->AddMiStoreRegisterMemCmd(cmdBuffer, &miStoreRegMemParams));

        miStoreRegMemParams.presStoreBuffer = presTpuCurbeBuffer;
        miStoreRegMemParams.dwOffset = baseOffset + sizeof(uint32_t) * 9;
        miStoreRegMemParams.dwRegister = mmioRegisters->mfxVP8BrcCumulativeDQIndex23RegOffset;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(commonMiInterface->AddMiStoreRegisterMemCmd(cmdBuffer, &miStoreRegMemParams));

        miStoreRegMemParams.presStoreBuffer = presTpuCurbeBuffer;
        miStoreRegMemParams.dwOffset = baseOffset + sizeof(uint32_t) * 10;
        miStoreRegMemParams.dwRegister = mmioRegisters->mfxVP8BrcCumulativeDLoopFilter01RegOffset;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(commonMiInterface->AddMiStoreRegisterMemCmd(cmdBuffer, &miStoreRegMemParams));

        miStoreRegMemParams.presStoreBuffer = presTpuCurbeBuffer;
        miStoreRegMemParams.dwOffset = baseOffset + sizeof(uint32_t) * 11;
        miStoreRegMemParams.dwRegister = mmioRegisters->mfxVP8BrcCumulativeDLoopFilter23RegOffset;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(commonMiInterface->AddMiStoreRegisterMemCmd(cmdBuffer, &miStoreRegMemParams));
    }

    return status;
}

#if USE_CODECHAL_DEBUG_TOOL
MOS_STATUS CodechalEncodeVp8::DumpVp8EncodePicParams(
    PCODEC_VP8_ENCODE_PIC_PARAMS picParams)
{
    CODECHAL_DEBUG_FUNCTION_ENTER;
    if (!m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrPicParams))
    {
        return MOS_STATUS_SUCCESS;
    }

    CODECHAL_DEBUG_CHK_NULL(picParams);

    std::ostringstream oss;
    oss.setf(std::ios::showbase | std::ios::uppercase);

    oss << "# DDI Parameters:" << std::endl;
    oss << "CurrOriginalPic = " << std::dec << +picParams->CurrOriginalPic.FrameIdx << std::endl;
    oss << "CurrReconstructedPic = " << std::dec << +picParams->CurrReconstructedPic.FrameIdx << std::endl;
    oss << "LastRefPic = " << std::dec << +picParams->LastRefPic.FrameIdx << std::endl;
    oss << "GoldenRefPic = " << std::dec << +picParams->GoldenRefPic.FrameIdx << std::endl;
    oss << "AltRefPic = " << std::dec << +picParams->AltRefPic.FrameIdx << std::endl;
    oss << "uPicFlags = " << std::dec << +picParams->uPicFlags << std::endl;
    oss << "frame_type: " << std::hex << +picParams->frame_type << std::endl;
    oss << "version: " << std::hex << +picParams->version << std::endl;
    oss << "show_frame: " << std::hex << +picParams->show_frame << std::endl;
    oss << "color_space: " << std::hex << +picParams->color_space << std::endl;
    oss << "clamping_type: " << std::hex << +picParams->clamping_type << std::endl;
    oss << "segmentation_enabled: " << std::hex << +picParams->segmentation_enabled << std::endl;
    oss << "update_mb_segmentation_map: " << std::hex << +picParams->update_mb_segmentation_map << std::endl;
    oss << "update_segment_feature_data: " << std::hex << +picParams->update_segment_feature_data << std::endl;
    oss << "filter_type: " << std::hex << +picParams->filter_type << std::endl;
    oss << "loop_filter_adj_enable: " << std::hex << +picParams->loop_filter_adj_enable << std::endl;
    oss << "CodedCoeffTokenPartition: " << std::hex << +picParams->CodedCoeffTokenPartition << std::endl;
    oss << "refresh_golden_frame: " << std::hex << +picParams->refresh_golden_frame << std::endl;
    oss << "refresh_alternate_frame: " << std::hex << +picParams->refresh_alternate_frame << std::endl;
    oss << "copy_buffer_to_golden: " << std::hex << +picParams->copy_buffer_to_golden << std::endl;
    oss << "copy_buffer_to_alternate: " << std::hex << +picParams->copy_buffer_to_alternate << std::endl;
    oss << "sign_bias_golden: " << std::hex << +picParams->sign_bias_golden << std::endl;
    oss << "sign_bias_alternate: " << std::hex << +picParams->sign_bias_alternate << std::endl;
    oss << "refresh_entropy_probs: " << std::hex << +picParams->refresh_entropy_probs << std::endl;
    oss << "refresh_last: " << std::hex << +picParams->refresh_last << std::endl;
    oss << "mb_no_coeff_skip: " << std::hex << +picParams->mb_no_coeff_skip << std::endl;
    oss << "forced_lf_adjustment: " << std::hex << +picParams->forced_lf_adjustment << std::endl;
    oss << "ref_frame_ctrl: " << std::hex << +picParams->ref_frame_ctrl << std::endl;

    for (uint8_t i = 0; i < 4; ++i)
    {
        oss << "loop_filter_level[" << +i << "]: " << std::hex << +picParams->loop_filter_level[i] << std::endl;
    }

    for (uint8_t i = 0; i < 4; ++i)
    {
        oss << "ref_lf_delta[" << +i << "]: " << std::hex << +picParams->ref_lf_delta[i] << std::endl;
    }

    for (uint8_t i = 0; i < 4; ++i)
    {
        oss << "mode_lf_delta[" << +i << "]: " << std::hex << +picParams->mode_lf_delta[i] << std::endl;
    }

    oss << "sharpness_level: " << std::hex << +picParams->sharpness_level << std::endl;
    oss << "StatusReportFeedbackNumber: " << std::hex << +picParams->StatusReportFeedbackNumber << std::endl;
    oss << "ClampQindexHigh: " << std::hex << +picParams->ClampQindexHigh << std::endl;
    oss << "ClampQindexLow: " << std::hex << +picParams->ClampQindexLow << std::endl;
    oss << "temporal_id: " << std::hex << +picParams->temporal_id << std::endl;

    const char * fileName = m_debugInterface->CreateFileName(
        "_ENC",
        CodechalDbgBufferType::bufPicParams,
        CodechalDbgExtType::txt);
    std::ofstream ofs(fileName, std::ios::out);
    ofs << oss.str();
    ofs.close();

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalEncodeVp8::DumpVp8EncodeSeqParams(
    PCODEC_VP8_ENCODE_SEQUENCE_PARAMS seqParams)
{
    CODECHAL_DEBUG_FUNCTION_ENTER;

    if (!m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrSeqParams))
    {
        return MOS_STATUS_SUCCESS;
    }

    CODECHAL_DEBUG_CHK_NULL(seqParams);

    std::ostringstream oss;
    oss.setf(std::ios::showbase | std::ios::uppercase);

    oss << "# DDI Parameters:" << std::endl;
    oss << "FrameWidth = " << std::dec << +seqParams->FrameWidth << std::endl;
    oss << "FrameWidthScale = " << std::dec << +seqParams->FrameWidthScale << std::endl;
    oss << "FrameHeight = " << std::dec << +seqParams->FrameHeight << std::endl;
    oss << "FrameHeightScale = " << std::dec << +seqParams->FrameHeightScale << std::endl;
    oss << "GopPicSize = " << std::dec << +seqParams->GopPicSize << std::endl;
    oss << "TargetUsage = " << std::dec << +seqParams->TargetUsage << std::endl;
    oss << "RateControlMethod = " << std::dec << +seqParams->RateControlMethod << std::endl;
    oss << "TargetBitRate = " << std::dec << +seqParams->TargetBitRate[0] << std::endl;
    oss << "MaxBitRate = " << std::dec << +seqParams->MaxBitRate << std::endl;
    oss << "MinBitRate = " << std::dec << +seqParams->MinBitRate << std::endl;
    oss << "InitVBVBufferFullnessInBit = " << +seqParams->InitVBVBufferFullnessInBit << std::endl;
    oss << "VBVBufferSizeInBit = " << std::dec << +seqParams->VBVBufferSizeInBit << std::endl;
    // begining of union/struct
    oss << "# bResetBRC = " << std::dec << +seqParams->ResetBRC << std::endl;
    oss << "# bNoFrameHeaderInsertion = " << std::dec << +seqParams->NoFrameHeaderInsertion << std::endl;
    // Next 5 fields not currently implemented.  nullptr output
    oss << "# UseRawReconRef = " << std::dec << +seqParams->UseRawReconRef << std::endl;
    oss << "# MBBRC = " << std::dec << +seqParams->MBBRC << std::endl;
    oss << "# bReserved = " << std::dec << +seqParams->bReserved << std::endl;
    oss << "# sFlags = " << std::dec << +seqParams->sFlags << std::endl;
    // end of union/struct
    oss << "UserMaxFrameSize = " << std::dec << +seqParams->UserMaxFrameSize << std::endl;
    oss << "FramesPer100Sec = " << std::dec << +seqParams->FramesPer100Sec[0] << std::endl;

    const char * fileName = m_debugInterface->CreateFileName(
        "_DDIEnc",
        CodechalDbgBufferType::bufSeqParams,
        CodechalDbgExtType::txt);

    std::ofstream ofs(fileName, std::ios::out);
    ofs << oss.str();
    ofs.close();

    if (m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrDriverUltDump))
    {
        if (!m_debugInterface->m_ddiFileName.empty())
        {
            std::ofstream ofs(m_debugInterface->m_ddiFileName, std::ios::app);
            ofs << "SeqParamFile"
                << " = \"" << m_debugInterface->sFileName << "\"" << std::endl;
            ofs.close();
        }
    }

    return MOS_STATUS_SUCCESS;
}
#endif
