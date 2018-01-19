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

class Surface3DTest: public CmTest
{
public:
    static const uint32_t WIDTH = 16;
    static const uint32_t HEIGHT = 16;
    static const uint32_t DEPTH = 16;

    Surface3DTest(): m_surface(nullptr) {}

    ~Surface3DTest() {}

    int32_t CreateDestroy(CM_SURFACE_FORMAT format,
                          uint32_t width,
                          uint32_t height,
                          uint32_t depth)
    {
        CMRT_UMD::MockDevice mock_device(&m_driverLoader);
        int32_t result = mock_device->CreateSurface3D(width, height, depth,
                                                      format, m_surface);
        if (result != CM_SUCCESS)
        {
            return result;
        }
        SurfaceIndex *surface_index = nullptr;
        result = m_surface->GetIndex(surface_index);
        EXPECT_EQ(CM_SUCCESS, result);
        EXPECT_GT(surface_index->get_data(), 0);
        return mock_device->DestroySurface(m_surface);
    }//===============================================

    int32_t ReadWrite()
    {
        uint8_t to_surface[4*WIDTH*HEIGHT*DEPTH] = {1};
        uint8_t from_surface[4*WIDTH*HEIGHT*DEPTH] = {2};
        for (uint32_t i = 0; i < WIDTH*HEIGHT*DEPTH; ++i)
        {
            to_surface[4*i] = to_surface[4*i + 1] = to_surface[4*i + 2]
                = to_surface[4*i + 3] = i%255;
        }

        CMRT_UMD::MockDevice mock_device(&m_driverLoader);
        int32_t result = mock_device->CreateSurface3D(
            WIDTH, HEIGHT, DEPTH, CM_SURFACE_FORMAT_A8R8G8B8, m_surface);
        EXPECT_EQ(CM_SUCCESS, result);

        result = m_surface->WriteSurface(to_surface, nullptr);
        EXPECT_EQ(CM_SUCCESS, result);

        result = m_surface->ReadSurface(from_surface, nullptr);
        EXPECT_EQ(CM_SUCCESS, result);

        result = memcmp(to_surface, from_surface, sizeof(to_surface));
        EXPECT_EQ(0, result);

        return mock_device->DestroySurface(m_surface);
    }//===============================================

    int32_t Initialize()
    {
        CMRT_UMD::MockDevice mock_device(&m_driverLoader);
        int32_t result
                = mock_device->CreateSurface3D(WIDTH, HEIGHT, DEPTH,
                                               CM_SURFACE_FORMAT_A8R8G8B8,
                                               m_surface);
        EXPECT_EQ(CM_SUCCESS, result);

        result = m_surface->InitSurface(0x42424242, nullptr);
        EXPECT_EQ(CM_SUCCESS, result);

        uint8_t data[4*WIDTH*HEIGHT*DEPTH] = {0};
        result = m_surface->ReadSurface(data, nullptr);
        EXPECT_EQ(CM_SUCCESS, result);
        EXPECT_EQ(0x42, data[0]);
        EXPECT_EQ(0x42, data[4*WIDTH*HEIGHT*DEPTH - 1]);
        return mock_device->DestroySurface(m_surface);
    }//===============================================

private:
    CMRT_UMD::CmSurface3D *m_surface;
};//=================================

TEST_F(Surface3DTest, MultipleSizes)
{
    RunEach(CM_SUCCESS,
            [this]() { return CreateDestroy(CM_SURFACE_FORMAT_A8R8G8B8,
                                            WIDTH, HEIGHT, DEPTH); });

    RunEach(CM_SUCCESS,
            [this]() { return CreateDestroy(CM_SURFACE_FORMAT_A8R8G8B8,
                                            CM_MIN_SURF_WIDTH,
                                            CM_MIN_SURF_HEIGHT,
                                            CM_MIN_SURF_DEPTH); });

    RunEach(CM_SUCCESS,
            [this]() { return CreateDestroy(CM_SURFACE_FORMAT_A8R8G8B8,
                                            CM_MAX_3D_SURF_WIDTH, HEIGHT,
                                            DEPTH); });

    RunEach(CM_SUCCESS,
            [this]() { return CreateDestroy(CM_SURFACE_FORMAT_A8R8G8B8,
                                            WIDTH, CM_MAX_3D_SURF_HEIGHT,
                                            DEPTH); });

    RunEach(CM_SUCCESS,
            [this]() { return CreateDestroy(CM_SURFACE_FORMAT_A8R8G8B8,
                                            WIDTH, HEIGHT,
                                            CM_MAX_3D_SURF_DEPTH); });

    RunEach(CM_INVALID_WIDTH,
            [this]() { return CreateDestroy(CM_SURFACE_FORMAT_A8R8G8B8, 0,
                                            HEIGHT, DEPTH); });

    RunEach(CM_INVALID_HEIGHT,
            [this]() { return CreateDestroy(CM_SURFACE_FORMAT_A8R8G8B8, WIDTH,
                                            0, DEPTH); });

    RunEach(CM_INVALID_DEPTH,
            [this]() { return CreateDestroy(CM_SURFACE_FORMAT_A8R8G8B8, WIDTH,
                                            HEIGHT, 0); });

    RunEach(CM_INVALID_DEPTH,
            [this]() { return CreateDestroy(CM_SURFACE_FORMAT_A8R8G8B8, WIDTH,
                                            HEIGHT,
                                            CM_MAX_3D_SURF_DEPTH + 1); });

    // Tests size in odd numbers.
    RunEach(CM_SUCCESS,
            [this]() { return CreateDestroy(CM_SURFACE_FORMAT_A8R8G8B8,
                                            WIDTH + 1, HEIGHT + 3,
                                            DEPTH + 5); });

    return;
}//========

TEST_F(Surface3DTest, Abgr16Format)
{
    RunEach(CM_SUCCESS,
            [this]() { return CreateDestroy(CM_SURFACE_FORMAT_A16B16G16R16,
                                            WIDTH, HEIGHT, DEPTH); });

    RunEach(CM_SUCCESS,
            [this]() { return CreateDestroy(CM_SURFACE_FORMAT_A16B16G16R16,
                                            WIDTH + 1, HEIGHT + 3,
                                            DEPTH + 5); });
    return;
}//========

TEST_F(Surface3DTest, ReadWrite)
{
    RunEach(CM_SUCCESS,
            [this]() { return ReadWrite(); });
    return;
}//========

TEST_F(Surface3DTest, Initialization)
{
    RunEach(CM_SUCCESS,
            [this]() { return Initialize(); });
    return;
}//========
