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

#include "cm_test.h"

class BufferUPTest: public CmTest
{
public:
    static const uint32_t SIZE = 4096;

    BufferUPTest(): m_buffer(nullptr), m_sys_mem(nullptr) {}

    ~BufferUPTest() { Release(); }

    int32_t CreateDestroy(uint32_t size)
    {
        uint32_t real_size = size? size: SIZE;
        m_sys_mem = AllocateAlignedMemory(real_size, 0x1000);

        int32_t result = m_mockDevice->CreateBufferUP(size, m_sys_mem, m_buffer);
        if (CM_SUCCESS != result)
        {
            Release();
            return result;
        }

        SurfaceIndex *surface_index = nullptr;
        result = m_buffer->GetIndex(surface_index);
        EXPECT_EQ(CM_SUCCESS, result);
        EXPECT_GT(surface_index->get_data(), static_cast<uint32_t>(0));
        result = m_mockDevice->DestroyBufferUP(m_buffer);
        Release();
        return result;
    }//===============

    int32_t CreateDestroy(void *sys_mem)
    {
        int32_t result = m_mockDevice->CreateBufferUP(SIZE, sys_mem, m_buffer);
        if (CM_SUCCESS != result)
        {
            return result;
        }
        return m_mockDevice->DestroyBufferUP(m_buffer);
    }//===============================================

    bool Release()
    {
        if (nullptr == m_sys_mem)
        {
            return true;
        }
        FreeAlignedMemory(m_sys_mem);
        m_sys_mem = nullptr;
        return true;
    }//=============

private:
    CMRT_UMD::CmBufferUP *m_buffer;
    void *m_sys_mem;
};//================

TEST_F(BufferUPTest, MultipleSizes)
{
    RunEach<int32_t>(CM_SUCCESS,
                     [this]() { return CreateDestroy(SIZE); });

    RunEach<int32_t>(CM_INVALID_WIDTH,
                     [this]() { return CreateDestroy(SIZE + 1); });

    RunEach<int32_t>(CM_SUCCESS,
                     [this]() { return CreateDestroy(4); });  // 4-byte is the minimum size.

    RunEach<int32_t>(CM_SUCCESS,
                     [this]() { return CreateDestroy(16); });

    RunEach<int32_t>(CM_SUCCESS,
                     [this]() { return CreateDestroy(512*1024*1024); });  // The maximum size.

    uint32_t zero_size = 0;
    RunEach<int32_t>(CM_INVALID_WIDTH,
                     [this, zero_size]() { return CreateDestroy(zero_size); });

    return;
}//========

TEST_F(BufferUPTest, InvalidSystemMemory)
{
    uint8_t *sys_mem = nullptr;
    RunEach<int32_t>(CM_INVALID_ARG_VALUE,
                     [this, sys_mem]() { return CreateDestroy(sys_mem); });

    sys_mem = new uint8_t[SIZE];
    uint8_t *sys_mem_for_surface = sys_mem;
    if ((reinterpret_cast<uintptr_t>(sys_mem) & (0x1000 - 1)) == 0)  // 4k-aligned.
    {
        ++sys_mem_for_surface;
    }
    auto CreateWithNonalignedPointer
        = [this, sys_mem_for_surface]()
          { return CreateDestroy(sys_mem_for_surface); };
    RunEach<int32_t>(CM_INVALID_ARG_VALUE, CreateWithNonalignedPointer);
    delete[] sys_mem;
    return;
}//========
