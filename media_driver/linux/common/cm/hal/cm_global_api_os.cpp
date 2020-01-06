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
//! \file  cm_global_api_os.cpp
//! \brief Contains implementations of gloabl CM APIs which are Linux-dependent.
//!

#include "cm_device_rt.h"

using CMRT_UMD::CmDevice;
using CMRT_UMD::CmDeviceRT;
//!
//! \brief    Creates a CmDevice from a MOS context.
//! \details  If an existing CmDevice has already associated to the MOS context,
//!           the existing CmDevice will be returned. Otherwise, a new CmDevice
//!           instance will be created and associatied with that MOS context.
//! \param    mosContext
//!           [in] pointer to MOS conetext.
//! \param    device
//!           [in,out] reference to the pointer to the CmDevice.
//! \param    devCreateOption
//!           [in] option to customize CmDevice.
//! \retval   CM_SUCCESS if the CmDevice is successfully created.
//! \retval   CM_NULL_POINTER if mosContext is null.
//! \retval   CM_FAILURE otherwise.
//!
CM_RT_API int32_t CreateCmDevice(MOS_CONTEXT *mosContext,
                                 CmDevice* &device,
                                 uint32_t devCreateOption)
{
    if (mosContext == nullptr)
    {
        return CM_NULL_POINTER;
    }
    CmDeviceRT* deviceRT = nullptr;

    int32_t ret = CmDeviceRT::Create(mosContext, deviceRT, devCreateOption);
    if(ret == CM_SUCCESS)
    {
        device = deviceRT;
    }

    return ret;
}

//!
//! \brief    Destroys the CmDevice.
//! \details  This function also destroys surfaces, kernels, programs, samplers,
//!           threadspaces, tasks and the queues that were created using this
//!           device instance but haven't explicitly been destroyed by calling
//!           respective destroy functions. 
//! \param    device
//!           [in] reference to the pointer to the CmDevice.
//! \retval   CM_SUCCESS if CmDevice is successfully destroyed.
//! \retval   CM_FAILURE otherwise.
//!
CM_RT_API int32_t DestroyCmDevice(CmDevice* & device)
{
    if (device == nullptr)
    {
        return CM_SUCCESS;
    }

    CmDeviceRT* deviceRT = static_cast<CmDeviceRT*>(device);
    int32_t ret = CmDeviceRT::Destroy(deviceRT);
    if (ret != CM_SUCCESS)
    {
        return ret;
    }

    device = nullptr;

    return CM_SUCCESS;
}
