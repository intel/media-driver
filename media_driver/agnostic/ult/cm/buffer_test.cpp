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

class BufferTest: public CmTest
{
public:
    static const uint32_t ELEMENT_COUNT = 64;
    static const uint32_t SIZE = ELEMENT_COUNT*sizeof(uint32_t);

    BufferTest(): m_buffer(nullptr) {}

    ~BufferTest() {}

    int32_t CreateDestroy(uint32_t size)
    {
        int32_t result = m_mockDevice->CreateBuffer(size, m_buffer);
        if (result != CM_SUCCESS)
        {
            return result;
        }
        SurfaceIndex *surface_index = nullptr;
        result = m_buffer->GetIndex(surface_index);
        EXPECT_EQ(CM_SUCCESS, result);
        EXPECT_GT(surface_index->get_data(), static_cast<uint32_t>(0));
        return m_mockDevice->DestroySurface(m_buffer);
    }//===============================================

    int32_t ReadWrite()
    {
        uint32_t to_buffer[ELEMENT_COUNT] = {1};
        uint32_t from_buffer[ELEMENT_COUNT] = {2};
        for (uint8_t i = 0; i < ELEMENT_COUNT; ++i)
        {
            to_buffer[i] = i;
        }
        int32_t result = m_mockDevice->CreateBuffer(SIZE, m_buffer);
        EXPECT_EQ(CM_SUCCESS, result);

        result = m_buffer->WriteSurface(reinterpret_cast<uint8_t*>(to_buffer),
                                        nullptr);
        EXPECT_EQ(CM_SUCCESS, result);

        result = m_buffer->ReadSurface(reinterpret_cast<uint8_t*>(from_buffer),
                                       nullptr);
        EXPECT_EQ(CM_SUCCESS, result);

        result = memcmp(to_buffer, from_buffer, sizeof(to_buffer));
        EXPECT_EQ(0, result);

        return m_mockDevice->DestroySurface(m_buffer);
    }//===============================================

    int32_t Initialize()
    {
        int32_t result = m_mockDevice->CreateBuffer(SIZE, m_buffer);
        EXPECT_EQ(CM_SUCCESS, result);

        result = m_buffer->InitSurface(0x42434445, nullptr);
        EXPECT_EQ(CM_SUCCESS, result);

        uint32_t data[ELEMENT_COUNT] = {0};
        result = m_buffer->ReadSurface(reinterpret_cast<uint8_t*>(data),
                                       nullptr);
        EXPECT_EQ(CM_SUCCESS, result);
        EXPECT_EQ(0x42434445, data[0]);
        EXPECT_EQ(0x42434445, data[ELEMENT_COUNT - 1]);
        return m_mockDevice->DestroySurface(m_buffer);
    }//===============================================

protected:
    CMRT_UMD::CmBuffer *m_buffer;
};//=============================

TEST_F(BufferTest, MultipleSizes)
{
    RunEach<int32_t>(CM_SUCCESS,
                     [this]() { return CreateDestroy(SIZE); });

    RunEach<int32_t>(CM_INVALID_WIDTH,
                     [this]() { return CreateDestroy(0); });

    RunEach<int32_t>(CM_SUCCESS,
                     [this]() { return CreateDestroy(1); });

    RunEach<int32_t>(CM_SUCCESS,
                     [this]() { return CreateDestroy(16*1024*1024); });

    RunEach<int32_t>(CM_SUCCESS,
                     [this]() { return CreateDestroy(64*1024*1024); });

    uint32_t large_size = 0x40000001;  // 1-byte larger than maximum size.
    RunEach<int32_t>(
        CM_INVALID_WIDTH,
        [this, large_size]() { return CreateDestroy(large_size); });
    
    return;
}//========

TEST_F(BufferTest, ReadWrite)
{
    RunEach<int32_t>(CM_SUCCESS,
                     [this]() { return ReadWrite(); });
    return;
}//========

TEST_F(BufferTest, Initialization)
{
    RunEach<int32_t>(CM_SUCCESS,
                     [this]() { return Initialize(); });
    return;
}//========
