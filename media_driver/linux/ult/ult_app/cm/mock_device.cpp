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

//!
//! \file      mock_device.cpp
//! \brief     MockDevice member functions.
//!

#include "mock_device.h"

namespace CMRT_UMD
{
vaDestroySurfacesFunc MockDevice::vaDestroySurfaces = nullptr;

int32_t ReleaseVaSurface(void *va_display, void *surface)
{
    VADisplayContext *display = reinterpret_cast<VADisplayContext*>(va_display);
    VASurfaceID *surface_id = reinterpret_cast<VASurfaceID*>(surface);
    return MockDevice::vaDestroySurfaces(display->pDriverContext, surface_id,
                                         1);
}//=========================================

template<class InputData>
int32_t MockDevice::SendRequestMessage(InputData *input, uint32_t function_id)
{
    uint32_t va_module_id = 2;  // VAExtModuleCMRT.
    uint32_t input_size = sizeof(InputData);
    int32_t output = 0;
    uint32_t output_size = sizeof(output_size);
    return this->vaCmExtSendReqMsg(&m_vaDisplay, &va_module_id,
                                   &function_id, input,
                                   &input_size, nullptr, &output,
                                   &output_size);
}//==============================================

bool MockDevice::Create(DriverDllLoader *driver_loader,
                        uint32_t additinal_options)
{
    this->vaCmExtSendReqMsg = driver_loader->GetDriverSymbols().vaCmExtSendReqMsg;
    if (nullptr == vaDestroySurfaces)
    {
        vaDestroySurfaces = driver_loader->m_vtable.vaDestroySurfaces;
    }
    m_vaDisplay.pDriverContext = &driver_loader->m_ctx;
    m_cmDevice = CreateNewDevice(additinal_options);
    return nullptr != m_cmDevice;
}//==============================

bool MockDevice::Release()
{
    if (nullptr == m_cmDevice)
    {
        return true;
    }
    int32_t result = ReleaseNewDevice(m_cmDevice);
    if (CM_SUCCESS == result)
    {
        vaDestroySurfaces = nullptr;
        m_cmDevice = nullptr;
        vaCmExtSendReqMsg = nullptr;
        m_vaDisplay.pDriverContext = nullptr;
        return true;
    }
    return false;
}//==============

CmDevice* MockDevice::CreateNewDevice(uint32_t additional_options)
{
    CreateDeviceParam create_param;
    create_param.release_surface_func = ReleaseVaSurface;
    create_param.create_option |= additional_options;
    uint32_t function_id = 0x1000;  // CM_FN_CREATECMDEVICE;
    SendRequestMessage(&create_param, function_id);
    CmDevice *new_device = reinterpret_cast<CmDevice*>(
        create_param.device_in_umd);
    if (CM_SUCCESS != create_param.return_value || nullptr == new_device)
    {
        assert(0);
        printf("Failed to create a CM device!\n");
    }
    return new_device;
}//===================

int32_t MockDevice::ReleaseNewDevice(CmDevice *device)
{
    DestroyDeviceParam destroy_param;
    destroy_param.device_in_umd = device;
    uint32_t function_id = 0x1001;  // CM_FN_DESTROYCMDEVICE;
    SendRequestMessage(&destroy_param, function_id);
    return destroy_param.return_value;
}//===================================

int32_t MockDevice::CheckAvaliableStream()
{
    return 0;
}
}  // namespace
