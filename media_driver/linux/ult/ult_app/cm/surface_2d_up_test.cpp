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
        if (m_sys_mem)
        {
            free(m_sys_mem);
        }
        return;
    }//========

    int32_t CreateDestroy(uint32_t width, uint32_t height)
    {
        CMRT_UMD::MockDevice mock_device(&m_driverLoader);
        m_sys_mem = memalign(4096, 4*width*height);
        int32_t result = mock_device->CreateSurface2DUP(
            width, height, CM_SURFACE_FORMAT_A8R8G8B8, m_sys_mem, m_surface);
        if (result != CM_SUCCESS)
        {
            return result;
        }
        SurfaceIndex *surface_index = nullptr;
        result = m_surface->GetIndex(surface_index);
        EXPECT_EQ(CM_SUCCESS, result);
        EXPECT_GT(surface_index->get_data(), 0);
        return mock_device->DestroySurface2DUP(m_surface);
    }//===================================================

private:
    CMRT_UMD::CmSurface2DUP *m_surface;
    void *m_sys_mem;
};//================

TEST_F(Surface2DUPTest, MultipleSizes)
{
    RunEach(CM_SUCCESS,
            [this]() { return CreateDestroy(WIDTH, HEIGHT); });

    RunEach(CM_SUCCESS,
            [this]() { return CreateDestroy(CM_MIN_SURF_WIDTH, HEIGHT); });

    RunEach(CM_SUCCESS,
            [this]() { return CreateDestroy(WIDTH, CM_MIN_SURF_HEIGHT); });
    return;
}//========
