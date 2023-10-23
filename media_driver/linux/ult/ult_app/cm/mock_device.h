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
//! \file      mock_device.h
//! \brief     Encapsulation of CmDevice for mock run.
//!

#ifndef MEDIADRIVER_LINUX_CODECHAL_ULT_ULTAPP_CMMOCKDEVICE_H_
#define MEDIADRIVER_LINUX_CODECHAL_ULT_ULTAPP_CMMOCKDEVICE_H_

#include "va/va.h"
#include "cm_rt_umd.h"
#include "../driver_loader.h"

namespace CMRT_UMD
{
typedef int32_t (*ReleaseSurfaceCallback)(void *va_display,
                                          void *surface);

typedef VAStatus (*vaDestroySurfacesFunc)(VADriverContextP ctx,
                                          VASurfaceID *surface_list,
                                          int num_surfaces);

int32_t ReleaseVaSurface(void *va_display, void *surface);

struct CreateDeviceParam
{
    CreateDeviceParam(): create_option(CM_DEVICE_CONFIG_MOCK_RUNTIME_ENABLE),
                         release_surface_func(nullptr),
                         device_in_umd(nullptr),
                         version(0),
                         enable_driver_store(0),
                         return_value(0) {}

    uint32_t create_option;
    ReleaseSurfaceCallback release_surface_func;
    void *device_in_umd;
    uint32_t version;
    uint32_t enable_driver_store;
    int32_t return_value;
};//=====================

struct DestroyDeviceParam
{
    DestroyDeviceParam(): device_in_umd(nullptr),
                          return_value(0) {}

    void *device_in_umd;
    int32_t return_value;
};//======================

//! Encapsulation of CmDevice created from mock device context.
//!   Creation is put into contructor and destroy is put into destructor to avoid
//! duplications in upcoming cases.
class MockDevice
{
public:
    static vaDestroySurfacesFunc vaDestroySurfaces;

    MockDevice(): vaCmExtSendReqMsg(nullptr),
                  m_cmDevice(nullptr)
    {
        MOS_ZeroMemory(&vaCmExtSendReqMsg, sizeof(vaCmExtSendReqMsg));
        MOS_ZeroMemory(&m_vaDisplay, sizeof(m_vaDisplay));
    }

    ~MockDevice() { Release(); }

    bool Create(DriverDllLoader *driver_loader, uint32_t additinal_options);

    bool Create(DriverDllLoader *driver_loader)
    { return Create(driver_loader, 0); }

    bool Release();

    CmDevice* operator->() { return m_cmDevice; }

    CmDevice* CreateNewDevice(uint32_t additional_options);

    CmDevice* CreateNewDevice() { return CreateNewDevice(0); }

    int32_t ReleaseNewDevice(CmDevice *device);

    int32_t CheckAvaliableStream();

private:
    template<class InputData>
    int32_t SendRequestMessage(InputData *input, uint32_t function_id);
    
    VADisplayContext m_vaDisplay;
    CmExtSendReqMsgFunc vaCmExtSendReqMsg;
    CmDevice *m_cmDevice;
};
};  // namespace

#endif  // #ifndef MEDIADRIVER_LINUX_CODECHAL_ULT_ULTAPP_CMMOCKDEVICE_H_
