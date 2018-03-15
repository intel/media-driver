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
    this->vaCmExtSendReqMsg = driver_loader->vaCmExtSendReqMsg;
    if (nullptr == vaDestroySurfaces)
    {
        vaDestroySurfaces = driver_loader->vtable.vaDestroySurfaces;
    }
    m_vaDisplay.pDriverContext = &driver_loader->ctx;

    CreateDeviceParam creation_param;
    creation_param.release_surface_func = ReleaseVaSurface;
    creation_param.create_option |= additinal_options;
    uint32_t function_id = 0x1000;  // CM_FN_CREATECMDEVICE;
    SendRequestMessage(&creation_param, function_id);

    m_cmDevice = reinterpret_cast<CmDevice*>(creation_param.device_in_umd);
    return CM_SUCCESS == creation_param.return_value;
}//==================================================

bool MockDevice::Release()
{
    if (nullptr == m_cmDevice)
    {
        return true;
    }
    DestroyDeviceParam destroy_param;
    destroy_param.device_in_umd = m_cmDevice;
    uint32_t function_id = 0x1001;  // CM_FN_DESTROYCMDEVICE;
    SendRequestMessage(&destroy_param, function_id);
    vaDestroySurfaces = nullptr;
    m_cmDevice = nullptr;
    return CM_SUCCESS == destroy_param.return_value;
}//=================================================
}  // namespace
