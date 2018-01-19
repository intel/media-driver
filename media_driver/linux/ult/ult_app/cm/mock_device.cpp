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

bool MockDevice::Create(DriverDllLoader *driver_loader,
                        uint32_t additinal_options)
{
    if (nullptr == vaDestroySurfaces)
    {
        vaDestroySurfaces = driver_loader->vtable.vaDestroySurfaces;
    }

    m_va_display.pDriverContext = &driver_loader->ctx;

    CreateDeviceParam creation_param;
    creation_param.release_surface_func = ReleaseVaSurface;
    creation_param.create_option |= additinal_options;

    uint32_t va_module_id = 2;  // VAExtModuleCMRT.
    uint32_t function_id = 0x1000;  // CM_FN_CREATECMDEVICE;
    uint32_t input_data_size = sizeof(creation_param);
    void *output_data = nullptr;
    uint32_t output_data_size = sizeof(output_data);

    int32_t result = vaCmExtSendReqMsg(&m_va_display, &va_module_id,
                                       &function_id, &creation_param,
                                       &input_data_size, 0, output_data,
                                       &output_data_size);
    m_cmDevice = reinterpret_cast<CmDevice*>(creation_param.device_in_umd);
    return result? false: true;
}//============================

bool MockDevice::Release()
{
    DestroyDeviceParam destroy_param;
    destroy_param.device_in_umd = m_cmDevice;

    uint32_t va_module_id = 2;
    uint32_t function_id = 0x1001;  // CM_FN_DESTROYCMDEVICE;
    uint32_t input_data_size = sizeof(destroy_param);
    void *output_data = m_cmDevice;
    uint32_t output_data_size = sizeof(output_data);

    int32_t result = vaCmExtSendReqMsg(&m_va_display, &va_module_id,
                                       &function_id, &destroy_param,
                                       &input_data_size, 0, output_data,
                                       &output_data_size);
    return result? false: true;
}//============================
}  // namespace
