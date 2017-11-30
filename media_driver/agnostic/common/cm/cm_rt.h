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
//! \file  cm_rt.h
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
#include "cm_wrapper.h"

#endif  // #ifndef MEDIADRIVER_AGNOSTIC_COMMON_CM_CMRT_H_
