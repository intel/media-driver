/*
* Copyright (c) 2016-2021, Intel Corporation
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
//! \file       renderhal_g12_0.h
//! \brief      header file of Gen12 hardware functions
//! \details    Gen12 hardware functions declare
//!
#ifndef __RENDERHAL_G12_0_H__
#define __RENDERHAL_G12_0_H__

#include "renderhal_g12_base.h"
#include "media_class_trace.h"

class XRenderHal_Interface_G12_0 : public XRenderHal_Interface_G12_Base
{
public:
    XRenderHal_Interface_G12_0() : XRenderHal_Interface_G12_Base()
    {

    }

    virtual ~XRenderHal_Interface_G12_0() {}
MEDIA_CLASS_DEFINE_END(XRenderHal_Interface_G12_0)
};

#endif // __RENDERHAL_G12_0_H__
