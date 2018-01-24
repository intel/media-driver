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

#ifndef MEDIADRIVER_LINUX_CODECHAL_ULT_ULTAPP_CMTEST_H_
#define MEDIADRIVER_LINUX_CODECHAL_ULT_ULTAPP_CMTEST_H_

#include "gtest/gtest.h"

#include "mock_device.h"

class CmTest: public testing::Test
{
public:
    CmTest(): m_currentPlatform(igfx_MAX) {}
    
    template<typename T, class Function>
    bool RunEach(T expected_return, Function func)
    {
        bool result = true;

        const vector<Platform_t> &platforms = m_driverLoader.GetPlatforms();
        int platform_count = static_cast<int>(platforms.size());

        for (int i = 0; i < platform_count; ++i)
        {
            int va_status = m_driverLoader.InitDriver(platforms[i]);
            EXPECT_EQ(VA_STATUS_SUCCESS, va_status);
            m_currentPlatform = platforms[i];

            int32_t function_return = func();
            EXPECT_EQ(function_return, expected_return);
            result &= (function_return == expected_return);

            va_status = m_driverLoader.CloseDriver();
            EXPECT_EQ(VA_STATUS_SUCCESS, va_status);
        }
        return result;
    }

protected:
    DriverDllLoader m_driverLoader;

    Platform_t m_currentPlatform;
};

#endif  // #ifndef MEDIADRIVER_LINUX_CODECHAL_ULT_ULTAPP_CMTEST_H_
