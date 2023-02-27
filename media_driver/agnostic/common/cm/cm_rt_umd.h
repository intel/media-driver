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
//! \file  cm_rt_umd.h
//! \brief Contains, indirectly, declarations of all CMRT API in UMD. Other
//!        UMD components only need to include this file to call CMRT APIs.
//!

#ifndef MEDIADRIVER_AGNOSTIC_COMMON_CM_CMRT_H_
#define MEDIADRIVER_AGNOSTIC_COMMON_CM_CMRT_H_

#include "cm_buffer.h"
#include "cm_device.h"
#include "cm_event.h"
#include "cm_kernel.h"
#include "cm_queue.h"
#include "cm_sampler.h"
#include "cm_sampler8x8.h"
#include "cm_surface_2d.h"
#include "cm_surface_2d_up.h"
#include "cm_surface_3d.h"
#include "cm_task.h"
#include "cm_thread_space.h"
#include "cm_vebox.h"
#include "cm_type.h"

using namespace CMRT_UMD;

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
//! \param    priority
//!           [in] priority for queue.
//! \retval   CM_SUCCESS if the CmDevice is successfully created.
//! \retval   CM_NULL_POINTER if pMosContext is null.
//! \retval   CM_FAILURE otherwise.
//!
CM_RT_API int32_t CreateCmDevice(MOS_CONTEXT *mosContext,
                                 CmDevice *&  device,
                                 uint32_t devCreateOption,
                                 uint8_t  priority = CM_DEVICE_CREATE_PRIORITY_DEFAULT);

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
CM_RT_API int32_t DestroyCmDevice(CMRT_UMD::CmDevice* &device);

//!
//! \brief      Returns the corresponding CM_RETURN_CODE error string.
//! \param      [in] errCode
//!             CM error code.
//! \return     Corresponding error string if valid Code. \n
//!             "Internal Error" if invalid.
//!
CM_RT_API const char* GetCmErrorString(int errCode);

#endif  // #ifndef MEDIADRIVER_AGNOSTIC_COMMON_CM_CMRT_H_
