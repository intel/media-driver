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

#include "ColorE.h"

_GENX_MAIN_ _CM_CALLABLE_ void ColorE(
    CURBE_INPUT_OUTPUT,
    GLOBAL_BUFFER_INPUT_OUTPUT)
{
    /*
    Buffer layout after shuffle
    _________________________________________________
    |_______Block0__________|_______Block1__________|
    |_______Block2__________|_______Block3__________|
    |_______Block4__________|_______Block5__________|
    |_______Block6__________|_______Block7__________|

    Write back buffer layout correlate to the block number#, each box stands for 1 GRF
    _______________________________________________
    |____R0_________R1_____|____R2_________R3_____|
    |____G0_________G1_____|____G2_________G3_____|
    |____B0_________B1_____|____B2_________B3_____|
    |____A0_________A1_____|____A2_________A3_____|
    |____R4_________R5_____|____R6_________R7_____|
    |____G4_________G5_____|____G6_________G7_____|
    |____B4_________B5_____|____B6_________B7_____|
    |____A4_________A5_____|____A6_________A7_____|
    */
    uint desc_y = nCOLOR_ENHENCEMENT_DSC + MDF_FC_COLORE_LUT_BTI + (MDF_FC_COLORE_nSI_LINEAR << 8);
    matrix_ref<ushort, 16, 16> WriteBackBuffer = DataBuffer.select<4, 1, 64, 1>(Buffer_Index << 2, 0).format<ushort, 16, 16>();

    matrix_ref<uint, 7, 8> msg1 = DataBuffer.format<uint, 96, 8>().select<7, 1, 8, 1>(64, 0);
    matrix_ref<uint, 7, 8> msg2 = DataBuffer.format<uint, 96, 8>().select<7, 1, 8, 1>(71, 0);
    matrix_ref<uint, 7, 8> msg3 = DataBuffer.format<uint, 96, 8>().select<7, 1, 8, 1>(78, 0);
    matrix_ref<uint, 7, 8> msg4 = DataBuffer.format<uint, 96, 8>().select<7, 1, 8, 1>(85, 0);

    msg1.select<1, 1, 8, 1>(0, 0) = cm_get_r0<uint>();
    msg2.select<1, 1, 8, 1>(0, 0) = cm_get_r0<uint>();
    msg3.select<1, 1, 8, 1>(0, 0) = cm_get_r0<uint>();
    msg4.select<1, 1, 8, 1>(0, 0) = cm_get_r0<uint>();

    msg1.select<1, 1, 1, 1>(0, 2) = nSAMPLER_BGR_CHANNELS;
    msg2.select<1, 1, 1, 1>(0, 2) = nSAMPLER_BGR_CHANNELS;
    msg3.select<1, 1, 1, 1>(0, 2) = nSAMPLER_BGR_CHANNELS;
    msg4.select<1, 1, 1, 1>(0, 2) = nSAMPLER_BGR_CHANNELS;

    matrix_ref<float, 6, 8> Coordinates1 = msg1.format<float, 7, 8>().select<6, 1, 8, 1>(1, 0);

    matrix_ref<float, 1, 16> CoordinateB1 = Coordinates1.format<float, 3, 16>().select<1, 1, 16, 1>(0, 0);
    matrix_ref<float, 1, 16> CoordinateG1 = Coordinates1.format<float, 3, 16>().select<1, 1, 16, 1>(1, 0);
    matrix_ref<float, 1, 16> CoordinateR1 = Coordinates1.format<float, 3, 16>().select<1, 1, 16, 1>(2, 0);

    matrix_ref<float, 6, 8> Coordinates2 = msg2.format<float, 7, 8>().select<6, 1, 8, 1>(1, 0);

    matrix_ref<float, 1, 16> CoordinateB2 = Coordinates2.format<float, 3, 16>().select<1, 1, 16, 1>(0, 0);
    matrix_ref<float, 1, 16> CoordinateG2 = Coordinates2.format<float, 3, 16>().select<1, 1, 16, 1>(1, 0);
    matrix_ref<float, 1, 16> CoordinateR2 = Coordinates2.format<float, 3, 16>().select<1, 1, 16, 1>(2, 0);

    matrix_ref<float, 6, 8> Coordinates3 = msg3.format<float, 7, 8>().select<6, 1, 8, 1>(1, 0);

    matrix_ref<float, 1, 16> CoordinateB3 = Coordinates3.format<float, 3, 16>().select<1, 1, 16, 1>(0, 0);
    matrix_ref<float, 1, 16> CoordinateG3 = Coordinates3.format<float, 3, 16>().select<1, 1, 16, 1>(1, 0);
    matrix_ref<float, 1, 16> CoordinateR3 = Coordinates3.format<float, 3, 16>().select<1, 1, 16, 1>(2, 0);

    matrix_ref<float, 6, 8> Coordinates4 = msg4.format<float, 7, 8>().select<6, 1, 8, 1>(1, 0);

    matrix_ref<float, 1, 16> CoordinateB4 = Coordinates4.format<float, 3, 16>().select<1, 1, 16, 1>(0, 0);
    matrix_ref<float, 1, 16> CoordinateG4 = Coordinates4.format<float, 3, 16>().select<1, 1, 16, 1>(1, 0);
    matrix_ref<float, 1, 16> CoordinateR4 = Coordinates4.format<float, 3, 16>().select<1, 1, 16, 1>(2, 0);

    CoordinateR1 = WriteBackBuffer.select<1, 1, 16, 1>(0, 0) * 0.000015318627450980392156862745098039f + 0.0126582278481013f;
    CoordinateG1 = WriteBackBuffer.select<1, 1, 16, 1>(2, 0) * 0.000015318627450980392156862745098039f + 0.0126582278481013f;
    CoordinateB1 = WriteBackBuffer.select<1, 1, 16, 1>(4, 0) * 0.000015318627450980392156862745098039f + 0.0126582278481013f;

    cm_send(Coordinates1,
        msg1,
        nSMPL_ENGINE,
        desc_y,
        0);

    WriteBackBuffer.select<1, 1, 16, 1>(0, 0) = matrix<ushort, 1, 16>(CoordinateB1 * 65280.0f, SAT);
    WriteBackBuffer.select<1, 1, 16, 1>(2, 0) = matrix<ushort, 1, 16>(CoordinateG1 * 65280.0f, SAT);
    WriteBackBuffer.select<1, 1, 16, 1>(4, 0) = matrix<ushort, 1, 16>(CoordinateR1 * 65280.0f, SAT);

    CoordinateR2 = WriteBackBuffer.select<1, 1, 16, 1>(1, 0) * 0.000015318627450980392156862745098039f + 0.0126582278481013f;
    CoordinateG2 = WriteBackBuffer.select<1, 1, 16, 1>(3, 0) * 0.000015318627450980392156862745098039f + 0.0126582278481013f;
    CoordinateB2 = WriteBackBuffer.select<1, 1, 16, 1>(5, 0) * 0.000015318627450980392156862745098039f + 0.0126582278481013f;

    cm_send(Coordinates2,
        msg2,
        nSMPL_ENGINE,
        desc_y,
        0);

    WriteBackBuffer.select<1, 1, 16, 1>(1, 0) = matrix<ushort, 1, 16>(CoordinateB2 * 65280.0f, SAT);
    WriteBackBuffer.select<1, 1, 16, 1>(3, 0) = matrix<ushort, 1, 16>(CoordinateG2 * 65280.0f, SAT);
    WriteBackBuffer.select<1, 1, 16, 1>(5, 0) = matrix<ushort, 1, 16>(CoordinateR2 * 65280.0f, SAT);

    CoordinateR3 = WriteBackBuffer.select<1, 1, 16, 1>(8, 0) * 0.000015318627450980392156862745098039f + 0.0126582278481013f;
    CoordinateG3 = WriteBackBuffer.select<1, 1, 16, 1>(10, 0) * 0.000015318627450980392156862745098039f + 0.0126582278481013f;
    CoordinateB3 = WriteBackBuffer.select<1, 1, 16, 1>(12, 0) * 0.000015318627450980392156862745098039f + 0.0126582278481013f;

    cm_send(Coordinates3,
        msg3,
        nSMPL_ENGINE,
        desc_y,
        0);

    WriteBackBuffer.select<1, 1, 16, 1>(8, 0) = matrix<ushort, 1, 16>(CoordinateB3 * 65280.0f, SAT);
    WriteBackBuffer.select<1, 1, 16, 1>(10, 0) = matrix<ushort, 1, 16>(CoordinateG3 * 65280.0f, SAT);
    WriteBackBuffer.select<1, 1, 16, 1>(12, 0) = matrix<ushort, 1, 16>(CoordinateR3 * 65280.0f, SAT);

    CoordinateR4 = WriteBackBuffer.select<1, 1, 16, 1>(9, 0) * 0.000015318627450980392156862745098039f + 0.0126582278481013f;
    CoordinateG4 = WriteBackBuffer.select<1, 1, 16, 1>(11, 0) * 0.000015318627450980392156862745098039f + 0.0126582278481013f;
    CoordinateB4 = WriteBackBuffer.select<1, 1, 16, 1>(13, 0) * 0.000015318627450980392156862745098039f + 0.0126582278481013f;

    cm_send(Coordinates4,
        msg4,
        nSMPL_ENGINE,
        desc_y,
        0);

    WriteBackBuffer.select<1, 1, 16, 1>(9, 0) = matrix<ushort, 1, 16>(CoordinateB4 * 65280.0f, SAT);
    WriteBackBuffer.select<1, 1, 16, 1>(11, 0) = matrix<ushort, 1, 16>(CoordinateG4 * 65280.0f, SAT);
    WriteBackBuffer.select<1, 1, 16, 1>(13, 0) = matrix<ushort, 1, 16>(CoordinateR4 * 65280.0f, SAT);
}
