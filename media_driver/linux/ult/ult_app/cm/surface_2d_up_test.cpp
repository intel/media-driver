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
#include <malloc.h>

class Surface2DUPTest: public CmTest
{
public:
    static const uint32_t WIDTH = 64;
    static const uint32_t HEIGHT = 64;

    Surface2DUPTest(): m_surface(nullptr), m_sys_mem(nullptr) {}

    ~Surface2DUPTest()
    {
        Release();
        return;
    }//========

    int32_t CreateDestroy(CM_SURFACE_FORMAT format,
                          uint32_t width,
                          uint32_t height)
    {
        CMRT_UMD::MockDevice mock_device(&m_driverLoader);
        uint32_t pitch = 0, alloc_size = 0;
        if (CM_SUCCESS == mock_device->GetSurface2DInfo(width, height, format,
                                                        pitch, alloc_size))
        {
            m_sys_mem = memalign(0x1000, alloc_size);
        }
        else  // In case width or height is invalid. This pointer will not be referenced anyway.
        {
            m_sys_mem = memalign(0x1000, 4*WIDTH*HEIGHT);
        }
        int32_t result = mock_device->CreateSurface2DUP(width, height, format,
                                                        m_sys_mem, m_surface);
        if (CM_SUCCESS != result)
        {
            Release();
            return result;
        }
        SurfaceIndex *surface_index = nullptr;
        result = m_surface->GetIndex(surface_index);
        EXPECT_EQ(CM_SUCCESS, result);
        EXPECT_GT(surface_index->get_data(), 0);
        result = mock_device->DestroySurface2DUP(m_surface);
        Release();
        return result;
    }//===============

    int32_t CreateDestroy(void *sys_mem)
    {
        CMRT_UMD::MockDevice mock_device(&m_driverLoader);
        int32_t result = mock_device->CreateSurface2DUP(
            WIDTH, HEIGHT, CM_SURFACE_FORMAT_A8R8G8B8, sys_mem, m_surface);
        if (CM_SUCCESS != result)
        {
            return result;
        }
        return mock_device->DestroySurface2DUP(m_surface);
    }//===================================================

    bool Release()
    {
        if (nullptr == m_sys_mem)
        {
            return false;
        }
        free(m_sys_mem);
        m_sys_mem = nullptr;
        return true;
    }//=============

private:
    CMRT_UMD::CmSurface2DUP *m_surface;
    void *m_sys_mem;
};//================

TEST_F(Surface2DUPTest, MultipleSizes)
{
    RunEach(CM_SUCCESS,
            [this]() { return CreateDestroy(CM_SURFACE_FORMAT_A8R8G8B8,
                                            WIDTH, HEIGHT); });

    RunEach(CM_SUCCESS,
            [this]() { return CreateDestroy(CM_SURFACE_FORMAT_A8R8G8B8,
                                            WIDTH + 1, HEIGHT + 1); });

    RunEach(CM_SUCCESS,
            [this]() { return CreateDestroy(CM_SURFACE_FORMAT_A8R8G8B8,
                                            CM_MIN_SURF_WIDTH, HEIGHT); });

    RunEach(CM_INVALID_WIDTH,
            [this]() { return CreateDestroy(CM_SURFACE_FORMAT_A8R8G8B8,
                                            0, HEIGHT); });

    RunEach(CM_SUCCESS,
            [this]() { return CreateDestroy(CM_SURFACE_FORMAT_A8R8G8B8,
                                            WIDTH, CM_MIN_SURF_HEIGHT); });

    RunEach(CM_INVALID_HEIGHT,
            [this]() { return CreateDestroy(CM_SURFACE_FORMAT_A8R8G8B8,
                                            WIDTH, 0); });

    RunEach(CM_SUCCESS,
            [this]() { return CreateDestroy(CM_SURFACE_FORMAT_A8R8G8B8,
                                            CM_MAX_2D_SURF_WIDTH, HEIGHT); });

    RunEach(CM_INVALID_WIDTH,
            [this]() { return CreateDestroy(CM_SURFACE_FORMAT_A8R8G8B8,
                                            CM_MAX_2D_SURF_WIDTH + 1,
                                            HEIGHT); });

    RunEach(CM_SUCCESS,
            [this]() { return CreateDestroy(CM_SURFACE_FORMAT_A8R8G8B8,
                                            WIDTH, CM_MAX_2D_SURF_HEIGHT); });

    RunEach(CM_INVALID_HEIGHT,
            [this]() { return CreateDestroy(CM_SURFACE_FORMAT_A8R8G8B8,
                                            WIDTH,
                                            CM_MAX_2D_SURF_HEIGHT + 1); });
    return;
}//========

TEST_F(Surface2DUPTest, InvalidSystemMemory)
{
    uint8_t *sys_mem = nullptr;
    RunEach(CM_INVALID_ARG_VALUE,
            [this, sys_mem]() { return CreateDestroy(sys_mem); });

    sys_mem = new uint8_t[4*WIDTH*HEIGHT];
    uint8_t *sys_mem_for_surface = sys_mem;
    if ((reinterpret_cast<uintptr_t>(sys_mem) & (0x1000 - 1)) == 0)  // 4k-aligned.
    {
        ++sys_mem_for_surface;
    }
    auto CreateWithNonalignedPointer
        = [this, sys_mem_for_surface]()
        { return CreateDestroy(sys_mem_for_surface); };
    RunEach(CM_INVALID_ARG_VALUE, CreateWithNonalignedPointer);
    delete[] sys_mem;
    return;
}//========

TEST_F(Surface2DUPTest, PlanarFormat)
{
    RunEach(CM_SUCCESS, [this]() { return CreateDestroy(CM_SURFACE_FORMAT_NV12,
                                                        WIDTH, HEIGHT); });
    RunEach(CM_INVALID_WIDTH,
            [this]() { return CreateDestroy(CM_SURFACE_FORMAT_NV12,
                                            WIDTH + 1, HEIGHT); });

    RunEach(CM_SUCCESS, [this]() { return CreateDestroy(CM_SURFACE_FORMAT_YUY2,
                                                        WIDTH, HEIGHT); });
    RunEach(CM_INVALID_WIDTH,
            [this]() { return CreateDestroy(CM_SURFACE_FORMAT_YUY2,
                                            WIDTH + 1, HEIGHT); });

    return;
}//========
