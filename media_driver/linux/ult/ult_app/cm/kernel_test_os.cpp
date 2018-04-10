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
//! \file     kernel_test_os.cpp
//! \brief    Implementation of Linux-dependent part of class KernelTest.
//! \details
//!

#include "kernel_test.h"

IsaData* KernelTest::FindIsaData(IsaArray *isa_array)
{
    uint32_t index = static_cast<uint32_t>(m_currentPlatform);
    return &((*isa_array)[index]);
}

//*-----------------------------
//| Reset the dafault ISA array.
//*------------------------------------
bool KernelTest::ResetDefaultIsaArray()
{
    m_isaArray.clear();
    m_isaArray.resize(static_cast<size_t>(igfx_MAX));
    return true;
}

//*--------------------------------------------------------
//| Sets pointers to ISA binaries in the dafault ISA array.
//*------------------------------------------
bool KernelTest::SetDefaultIsaArrayBinaries()
{
    m_isaArray[0] = IsaData(SKYLAKE_DONOTHING_ISA, 0, nullptr);
    m_isaArray[1] = IsaData(BROXTON_DONOTHING_ISA, 0, nullptr);
    m_isaArray[2] = IsaData(BROADWELL_DONOTHING_ISA, 0, nullptr);
    m_isaArray[3] = IsaData(CANNONLAKE_DONOTHING_ISA, 0, nullptr);
    return true;
}

//*----------------------------------------------------
//| Sets sizs of ISA binaries in the dafault ISA array.
//*---------------------------------------
bool KernelTest::SetDefaultIsaArraySizes()
{
    size_t code_sizes[] = {sizeof(SKYLAKE_DONOTHING_ISA),
                           sizeof(BROXTON_DONOTHING_ISA),
                           sizeof(BROADWELL_DONOTHING_ISA),
                           sizeof(CANNONLAKE_DONOTHING_ISA)};
    for (size_t i = 0; i < m_isaArray.size(); ++i)
    {
        m_isaArray[i].size = code_sizes[i];
    }
    return true;
}
