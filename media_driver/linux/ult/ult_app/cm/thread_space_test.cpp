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

class ThreadSpaceTest: public CmTest
{
public:
    static const uint32_t WIDTH = 64;
    static const uint32_t HEIGHT = 64;

    ThreadSpaceTest(): m_thread_space(nullptr) {}
  
    ~ThreadSpaceTest() {}

    int32_t CreateDestroy(uint32_t width, uint32_t height)
    {
        CMRT_UMD::MockDevice mock_device(&m_driverLoader);
        int32_t result = mock_device->CreateThreadSpace(width, height,
                                                        m_thread_space);
        if (result != CM_SUCCESS)
        {
            return result;
        }
        return mock_device->DestroyThreadSpace(m_thread_space);
    }//========================================================

    int32_t SelectDependencyPattern(CM_DEPENDENCY_PATTERN pattern)
    {
        CMRT_UMD::MockDevice mock_device(&m_driverLoader);
        int32_t result = mock_device->CreateThreadSpace(WIDTH, HEIGHT,
                                                        m_thread_space);
        EXPECT_EQ(CM_SUCCESS, result);
        result = m_thread_space->SelectThreadDependencyPattern(pattern);
        int32_t destroy_result
            = mock_device->DestroyThreadSpace(m_thread_space);
        EXPECT_EQ(CM_SUCCESS, destroy_result);
        return result;
    }//===============

    int32_t SelectMediaWalkingPattern(CM_WALKING_PATTERN pattern)
    {
        CMRT_UMD::MockDevice mock_device(&m_driverLoader);
        int32_t result = mock_device->CreateThreadSpace(WIDTH, HEIGHT,
                                                        m_thread_space);
        EXPECT_EQ(CM_SUCCESS, result);
        result = m_thread_space->SelectMediaWalkingPattern(pattern);
        int32_t destroy_result
            = mock_device->DestroyThreadSpace(m_thread_space);
        EXPECT_EQ(CM_SUCCESS, destroy_result);
        return result;
    }//===============

private:
    CMRT_UMD::CmThreadSpace *m_thread_space;
};//========================================

TEST_F(ThreadSpaceTest, MultipleSizes)
{
    RunEach(CM_SUCCESS,
            [this]() { return CreateDestroy(WIDTH, HEIGHT); });

    uint32_t max_width = CM_MAX_THREADSPACE_WIDTH_SKLUP_FOR_MW;
    uint32_t max_height = CM_MAX_THREADSPACE_HEIGHT_SKLUP_FOR_MW;
    RunEach(CM_SUCCESS,
            [this, max_width, max_height]()
                { return CreateDestroy(max_width, max_height);});
    return;
}//========

TEST_F(ThreadSpaceTest, DependencyPattern)
{
    CM_DEPENDENCY_PATTERN pattern = static_cast<CM_DEPENDENCY_PATTERN>(39);
    RunEach(CM_FAILURE,
            [this, pattern]() { return SelectDependencyPattern(pattern); });

    RunEach(CM_SUCCESS,
            [this]() { return SelectDependencyPattern(CM_NONE_DEPENDENCY); });
    return;
}//========

TEST_F(ThreadSpaceTest, MediaWalkingPattern)
{
    CM_WALKING_PATTERN pattern = static_cast<CM_WALKING_PATTERN>(39);
    RunEach(CM_INVALID_MEDIA_WALKING_PATTERN,
            [this, pattern]() { return SelectMediaWalkingPattern(pattern); });

    RunEach(CM_SUCCESS,
            [this]() { return SelectMediaWalkingPattern(CM_WALK_DEFAULT); });
    return;
}//========
