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

class DeviceTest: public CmTest
{
public:
    DeviceTest() {}
    ~DeviceTest() {}

    int32_t Destroy(CmDevice *device)
    {
        CMRT_UMD::DestroyDeviceParam destroy_param;
        destroy_param.device_in_umd = device;
        uint32_t function_id = 0x1001;
        m_mockDevice.SendRequestMessage(&destroy_param, function_id);
        return destroy_param.return_value;
    }//===================================

    int32_t CreateWithOptions(uint32_t options)
    {
        CMRT_UMD::CreateDeviceParam create_param;
        create_param.release_surface_func = ReleaseVaSurface;
        create_param.create_option |= options;
        uint32_t function_id = 0x1000;
        m_mockDevice.SendRequestMessage(&create_param, function_id);

        CMRT_UMD::DestroyDeviceParam destroy_param;
        destroy_param.device_in_umd = create_param.device_in_umd;
        function_id = 0x1001;
        m_mockDevice.SendRequestMessage(&destroy_param, function_id);
        return create_param.return_value;
    }//==================================
};

TEST_F(DeviceTest, Destroy)
{
    RunEach<int32_t>(CM_NULL_POINTER,
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
