/*
* Copyright (c) 2007-2017, Intel Corporation
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
//! \file      cm_surface_2d_codec_wrapper.cpp
//! \brief     Contains class CmSurface2DCodecWrapper definitions.
//!

#include "cm_surface_2d_codec_wrapper.h"
#include "cm_device_rt.h"
#include "cm_surface_2d_rt.h"

int32_t CreateCmSurface2D(CmDevice *cmDevice,
                          uint32_t width,
                          uint32_t height,
                          CM_SURFACE_FORMAT format,
                          CmSurface2DCodecWrapper **cmSurface)
{
    return cmDevice->CreateSurface2D(width, height, format, *cmSurface);
}

int32_t CreateCmSurface2D(CmDevice *cmDevice,
                          MOS_RESOURCE *mosResource,
                          CmSurface2DCodecWrapper **cmSurface)
{
    return cmDevice->CreateSurface2D(mosResource, *cmSurface);
}

int32_t DestroyCmSurface2D(CmDevice *cmDevice,
                           CmSurface2DCodecWrapper **cmSurface)
{
    return cmDevice->DestroySurface(*cmSurface);
}

int32_t UpdateCmSurface2D(CmDevice *cmDevice,
                          MOS_RESOURCE *mosResource,
                          CmSurface2DCodecWrapper **cmSurface)
{
    return cmDevice->UpdateSurface2D(mosResource, *cmSurface,
                                     MOS_CM_RESOURCE_USAGE_SurfaceState);
}
