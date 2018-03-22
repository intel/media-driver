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

class Surface2DTest: public CmTest
{
public:
    static const uint32_t WIDTH = 64;
    static const uint32_t HEIGHT = 64;

    Surface2DTest(): m_surface(nullptr) {}

    ~Surface2DTest() {}

    int32_t CreateDestroy(CM_SURFACE_FORMAT format,
                          uint32_t width,
                          uint32_t height)
    {
        int32_t result = m_mockDevice->CreateSurface2D(
            width, height, format, m_surface);
        if (result != CM_SUCCESS)
        {
            return result;
        }
        SurfaceIndex *surface_index = nullptr;
        result = m_surface->GetIndex(surface_index);
        EXPECT_EQ(CM_SUCCESS, result);
        EXPECT_GT(surface_index->get_data(), static_cast<uint32_t>(0));
        return m_mockDevice->DestroySurface(m_surface);
    }//================================================

    int32_t DestroyNull() { return m_mockDevice->DestroySurface(m_surface); }
    //=======================================================================

    int32_t ReadWrite(uint32_t width,
                      uint32_t height,
                      uint8_t *to_surface,
                      uint8_t *from_surface)
    {
        int32_t result = m_mockDevice->CreateSurface2D(
            width, height, CM_SURFACE_FORMAT_A8R8G8B8, m_surface);
        EXPECT_EQ(CM_SUCCESS, result);

        result = m_surface->WriteSurface(to_surface, nullptr);
        if (CM_SUCCESS != result)
        {
            m_mockDevice->DestroySurface(m_surface);
            return result;
        }

        result = m_surface->ReadSurface(from_surface, nullptr);
        if (CM_SUCCESS != result)
        {
            m_mockDevice->DestroySurface(m_surface);
            return result;
        }

        result = memcmp(to_surface, from_surface, 4*width*height);
        EXPECT_EQ(0, result);

        return m_mockDevice->DestroySurface(m_surface);
    }//================================================

    int32_t ReadWriteWithStride()
    {
        uint8_t to_surface[2*WIDTH*HEIGHT] = {1};
        uint8_t from_surface[2*WIDTH*HEIGHT] = {2};
        for (uint32_t i = 0; i < 2*WIDTH*HEIGHT; ++i)
        {
            to_surface[i] = i%255;
        }
        to_surface[WIDTH] = 41;
        from_surface[WIDTH] = 42;

        int32_t result = m_mockDevice->CreateSurface2D(
            WIDTH, HEIGHT, CM_SURFACE_FORMAT_A8, m_surface);
        EXPECT_EQ(CM_SUCCESS, result);

        result = m_surface->WriteSurfaceStride(to_surface, nullptr, 2*WIDTH);
        EXPECT_EQ(CM_SUCCESS, result);

        result = m_surface->ReadSurfaceStride(from_surface, nullptr, 2*WIDTH);
        EXPECT_EQ(CM_SUCCESS, result);

        result = 0;
        for (uint32_t row = 0; row < HEIGHT; ++row)
        {
            uint32_t offset = 2*WIDTH*row;
            result += memcmp(to_surface + offset, from_surface + offset, WIDTH);
        }
        EXPECT_EQ(0, result);
        EXPECT_EQ(from_surface[WIDTH], 42);

        return m_mockDevice->DestroySurface(m_surface);
    }//================================================

    int32_t Initialize()
    {
        int32_t result = m_mockDevice->CreateSurface2D(
            WIDTH, HEIGHT, CM_SURFACE_FORMAT_A8R8G8B8, m_surface);
        EXPECT_EQ(CM_SUCCESS, result);

        result = m_surface->InitSurface(0x42424242, nullptr);
        EXPECT_EQ(CM_SUCCESS, result);

        uint8_t data[4*WIDTH*HEIGHT] = {0};
        result = m_surface->ReadSurface(data, nullptr);
        EXPECT_EQ(CM_SUCCESS, result);
        EXPECT_EQ(0x42, data[0]);
        EXPECT_EQ(0x42, data[4*WIDTH*HEIGHT - 1]);
        return m_mockDevice->DestroySurface(m_surface);
    }//================================================

    uint32_t GetAllocationSize(uint32_t width,
                               uint32_t height,
                               CM_SURFACE_FORMAT format)
    {
        uint32_t pitch = 0, physical_size = 0;
        int32_t result = m_mockDevice->GetSurface2DInfo(width, height, format,
                                                       pitch, physical_size);
        EXPECT_GE(pitch, width);
        return physical_size;
    }//======================

protected:
    CMRT_UMD::CmSurface2D *m_surface;
};//=================================

TEST_F(Surface2DTest, MultipleSizes)
{
    RunEach<int32_t>(CM_SUCCESS,
                     [this]() { return CreateDestroy(CM_SURFACE_FORMAT_A8R8G8B8,
                                                     WIDTH, HEIGHT); });

    RunEach<int32_t>(
        CM_SUCCESS,
        [this]() { return CreateDestroy(CM_SURFACE_FORMAT_A8R8G8B8,
                                        CM_MAX_2D_SURF_WIDTH, HEIGHT); });

    RunEach<int32_t>(
        CM_SUCCESS,
        [this]() { return CreateDestroy(CM_SURFACE_FORMAT_A8R8G8B8,
                                        WIDTH, CM_MAX_2D_SURF_HEIGHT); });

    RunEach<int32_t>(
        CM_SUCCESS,
        [this]() { return CreateDestroy(CM_SURFACE_FORMAT_A8R8G8B8,
                                        CM_MIN_SURF_WIDTH, HEIGHT); });

    RunEach<int32_t>(
        CM_SUCCESS,
        [this]() { return CreateDestroy(CM_SURFACE_FORMAT_A8R8G8B8,
                                        WIDTH, CM_MIN_SURF_HEIGHT); });

    RunEach<int32_t>(CM_SUCCESS,
                     [this]() { return CreateDestroy(CM_SURFACE_FORMAT_A8R8G8B8,
                                                     2048, 2048); });

    RunEach<int32_t>(CM_SUCCESS,
                     [this]() { return CreateDestroy(CM_SURFACE_FORMAT_A8R8G8B8,
                                                     2048, 8192); });

    RunEach<int32_t>(CM_SUCCESS,
                     [this]() { return CreateDestroy(CM_SURFACE_FORMAT_A8R8G8B8,
                                                     8192, 2048); });

    RunEach<int32_t>(CM_INVALID_WIDTH,
                     [this]() { return CreateDestroy(CM_SURFACE_FORMAT_A8R8G8B8,
                                                     0, HEIGHT); });

    RunEach<int32_t>(CM_INVALID_WIDTH,
                     [this]() { return CreateDestroy(CM_SURFACE_FORMAT_A8R8G8B8,
                                                     CM_MAX_2D_SURF_WIDTH + 1,
                                                     HEIGHT); });

    RunEach<int32_t>(CM_INVALID_HEIGHT,
                     [this]() { return CreateDestroy(CM_SURFACE_FORMAT_A8R8G8B8,
                                                     WIDTH, 0); });

    RunEach<int32_t>(
        CM_INVALID_HEIGHT,
        [this]() { return CreateDestroy(CM_SURFACE_FORMAT_A8R8G8B8,
                                        WIDTH,
                                        CM_MAX_2D_SURF_HEIGHT + 1); });

    RunEach<int32_t>(CM_SUCCESS,
                     [this]() { return CreateDestroy(CM_SURFACE_FORMAT_A8R8G8B8,
                                                     CM_MAX_2D_SURF_WIDTH,
                                                     CM_MAX_2D_SURF_HEIGHT); });
    return;
}//========

TEST_F(Surface2DTest, PlanarFormats)
{
    RunEach<int32_t>(CM_SUCCESS,
                     [this]() { return CreateDestroy(CM_SURFACE_FORMAT_YV12,
                                                     WIDTH, HEIGHT); });

    RunEach<int32_t>(CM_SUCCESS,
                     [this]() { return CreateDestroy(CM_SURFACE_FORMAT_444P,
                                                     WIDTH, HEIGHT); });

    RunEach<int32_t>(CM_SUCCESS,
                     [this]() { return CreateDestroy(CM_SURFACE_FORMAT_422H,
                                                     WIDTH, HEIGHT); });

    RunEach<int32_t>(CM_SUCCESS,
                     [this]() { return CreateDestroy(CM_SURFACE_FORMAT_422V,
                                                     WIDTH, HEIGHT); });

    RunEach<int32_t>(CM_SUCCESS,
                     [this]() { return CreateDestroy(CM_SURFACE_FORMAT_IMC3,
                                                     WIDTH, HEIGHT); });

    RunEach<int32_t>(CM_SUCCESS,
                     [this]() { return CreateDestroy(CM_SURFACE_FORMAT_411P,
                                                     WIDTH, HEIGHT); });

    return;
}//========

TEST_F(Surface2DTest, DestroyNull)
{
    auto DestroyNullPointer = [this]() { return CM_SUCCESS == DestroyNull(); };
    RunEach(false, DestroyNullPointer);
    return;
}//========

TEST_F(Surface2DTest, ReadWrite)
{
    uint8_t src[4*WIDTH*HEIGHT] = {1};
    uint8_t dst[4*WIDTH*HEIGHT] = {2};
    for (uint32_t i = 0; i < WIDTH*HEIGHT; ++i)
    {
        src[4*i] = src[4*i + 1] = src[4*i + 2] = src[4*i + 3] = i%255;
    }
    uint8_t *to_surface = src;
    uint8_t *from_surface = dst;

    auto NormalReadWrite
        = [this, to_surface, from_surface]()
          { return ReadWrite(WIDTH, HEIGHT, to_surface, from_surface); };
    RunEach<int32_t>(CM_SUCCESS, NormalReadWrite);

    auto ReadToNull
        = [this, to_surface, from_surface]()
          { return ReadWrite(WIDTH, HEIGHT, to_surface, nullptr); };
    RunEach<int32_t>(CM_INVALID_ARG_VALUE, ReadToNull);

    return;
}//========

TEST_F(Surface2DTest, ReadWriteWithStride)
{
    RunEach<int32_t>(CM_SUCCESS,
                     [this]() { return ReadWriteWithStride(); });
    return;
}//========

TEST_F(Surface2DTest, Initialization)
{
    RunEach<int32_t>(CM_SUCCESS,
                     [this]() { return Initialize(); });
    return;
}//========

TEST_F(Surface2DTest, GetInfo)
{
    auto argb8_size0 =
        [this]() { return GetAllocationSize(WIDTH, HEIGHT,
                                            CM_SURFACE_FORMAT_A8R8G8B8)
                   == 4*WIDTH*HEIGHT; };
    RunEach(true, argb8_size0);

    uint32_t width = 15;
    auto argb8_size1 =
        [this, width]() { return GetAllocationSize(width, HEIGHT,
                                                   CM_SURFACE_FORMAT_A8R8G8B8)
                          > 4*width*HEIGHT; };
    RunEach(true, argb8_size1);

    return;
}//========
