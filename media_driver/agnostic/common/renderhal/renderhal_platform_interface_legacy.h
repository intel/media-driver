/*
* Copyright (c) 2021, Intel Corporation
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
//! \file      renderhal_platform_interface_legacy.h 
//! \brief     abstract the platfrom specific APIs into one class 
//!
//!
//! \file     renderhal.h
//! \brief    Render Engine Interfaces shared across platforms
//! \details  Platform Independent Hardware Interfaces
//!
#ifndef __RENDERHAL_PLATFORM_INTERFACE_LEGACY_H__
#define __RENDERHAL_PLATFORM_INTERFACE_LEGACY_H__

#include "mos_os.h"
#include "renderhal.h"
#include "renderhal_platform_interface.h"

class XRenderHal_Platform_Interface_Legacy : public XRenderHal_Platform_Interface
{
public:
    XRenderHal_Platform_Interface_Legacy() {}
    virtual ~XRenderHal_Platform_Interface_Legacy() {}
};

#endif // __RENDERHAL_PLATFORM_INTERFACE_LEGACY_H__
