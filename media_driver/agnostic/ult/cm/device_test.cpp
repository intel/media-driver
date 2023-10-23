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

struct CM_PLATFORM_INFO
{
    uint32_t value0;
    uint32_t value1;
    uint32_t value2;
    uint32_t value3;
    uint32_t value4;
};

class DeviceTest: public CmTest
{
public:
    DeviceTest() {}
    ~DeviceTest() {}

    int32_t Destroy(CmDevice *device)
    { return m_mockDevice.ReleaseNewDevice(device); }

    int32_t CreateWithOptions(uint32_t additional_options)
    {
        CmDevice *new_device = m_mockDevice.CreateNewDevice(additional_options);
        if (nullptr == new_device)
        {
            return CM_FAILURE;
        }
        return m_mockDevice.ReleaseNewDevice(new_device);
    }//==================================================

    int32_t CheckIsStreamLeaked()
    {
        int32_t  initStreams, releaseStreams;
        initStreams = m_mockDevice.CheckAvaliableStream();
        CmDevice *new_device      = m_mockDevice.CreateNewDevice();
        if (nullptr == new_device)
        {
            return CM_FAILURE;
        }
        int32_t  result = m_mockDevice.ReleaseNewDevice(new_device);
        releaseStreams =m_mockDevice.CheckAvaliableStream();
        EXPECT_EQ(initStreams, releaseStreams);
        return result;
    }  //==================================================

    template<typename T>
    int32_t GetCaps(CM_DEVICE_CAP_NAME cap_name, T *cap_value)
    {
        uint32_t value_size = static_cast<uint32_t>(sizeof(T));
        return m_mockDevice->GetCaps(cap_name, value_size, cap_value);
    }

    template<typename T>
    int32_t SetCaps(CM_DEVICE_CAP_NAME cap_name, T *cap_value)
    {
        uint32_t value_size = static_cast<uint32_t>(sizeof(T));
        return m_mockDevice->SetCaps(cap_name, value_size, cap_value);
    }
};

TEST_F(DeviceTest, Destroy)
{
    RunEach<int32_t>(CM_SUCCESS,
                     [this]() { return Destroy(nullptr); });
    return;
}//========

TEST_F(DeviceTest, NoScratchSpace)
{
    uint32_t option = CM_DEVICE_CREATE_OPTION_SCRATCH_SPACE_DISABLE;
    RunEach<int32_t>(CM_SUCCESS,
                     [this, option]() { return CreateWithOptions(option); });
    return;
}//========

TEST_F(DeviceTest, CheckStreamLeak)
{
    RunEach<int32_t>(CM_SUCCESS,
        [this]() { return CheckIsStreamLeaked(); });
    return;
}  //========

TEST_F(DeviceTest, QueryGpuPlatform)
{
    auto QueryGpuPlatform = [this]() {
        uint32_t gpu_platform = 0;
        GetCaps(CAP_GPU_PLATFORM, &gpu_platform);
        return gpu_platform != PLATFORM_INTEL_UNKNOWN;
    };
    RunEach<bool>(true, QueryGpuPlatform);
    return;
}

TEST_F(DeviceTest, QueryThreadCount)
{
    auto QueryThreadCount = [this]() {
        uint32_t per_walker_count = 0, per_group_count = 0;
        GetCaps(CAP_USER_DEFINED_THREAD_COUNT_PER_MEDIA_WALKER,
                &per_walker_count);
        GetCaps(CAP_USER_DEFINED_THREAD_COUNT_PER_THREAD_GROUP,
                &per_group_count);
        return per_walker_count > 0 && per_group_count > 0;
    };
    RunEach<bool>(true, QueryThreadCount);
    return;
}

TEST_F(DeviceTest, QueryFrequency)
{
    auto QueryFrequency = [this]() {
        uint32_t min_frequency = 0, max_frequency = 0, current_frequency = 0;
        GetCaps(CAP_MIN_FREQUENCY, &min_frequency);
        GetCaps(CAP_MAX_FREQUENCY, &max_frequency);
        GetCaps(CAP_GPU_CURRENT_FREQUENCY, &current_frequency);
        return current_frequency >= min_frequency
        && max_frequency >= current_frequency;
    };
    RunEach<bool>(true, QueryFrequency);
    return;
}

TEST_F(DeviceTest, QueryPlatformInformation)
{
    auto QueryPlatformInformation = [this]() {
        CM_PLATFORM_INFO platform_info;
        GetCaps(CAP_PLATFORM_INFO, &platform_info);
        return platform_info.value0 > 0 && platform_info.value1 > 0
        && platform_info.value2 > 0 && platform_info.value3 > 0;
    };
    RunEach<bool>(true, QueryPlatformInformation);
    return;
}

TEST_F(DeviceTest, SetThreadCount)
{
    auto SetThreadCount = [this]() {
        uint32_t thread_count = 0;
        GetCaps(CAP_HW_THREAD_COUNT, &thread_count);
        thread_count /= 2;
        return SetCaps(CAP_HW_THREAD_COUNT, &thread_count);
    };
    RunEach<int32_t>(CM_SUCCESS, SetThreadCount);
    return;
}

TEST_F(DeviceTest, GetInvalidCap)
{
    auto GetInvalidCap = [this]() {
        CM_DEVICE_CAP_NAME cap_name = (CM_DEVICE_CAP_NAME)-99;
        uint32_t value = 0;
        return GetCaps(cap_name, &value);
    };
    RunEach<int32_t>(CM_FAILURE, GetInvalidCap);
    return;
}
