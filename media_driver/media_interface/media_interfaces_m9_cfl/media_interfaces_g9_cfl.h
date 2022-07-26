/*
* Copyright (c) 2017-2018, Intel Corporation
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
//! \file     media_interfaces_g9_cfl.h
//! \brief    All interfaces used for CFL that require factory creation
//!

#ifndef __MEDIA_INTERFACES_G9_CFL_H__
#define __MEDIA_INTERFACES_G9_CFL_H__

#include "media_factory.h"
#include "media_interfaces_cmhal.h"
#include "media_interfaces_decode_histogram.h"
#include "media_interfaces_hwinfo_device.h"

#include "cm_hal_g9.h"

#include "codechal_decode_histogram_vebox_g9.h"

class CMHalInterfacesG9Cfl : public CMHalDevice
{
protected:
    using CMHal = CM_HAL_G9_X;
    MOS_STATUS Initialize(
        CM_HAL_STATE *pCmState);
};

class DecodeHistogramDeviceG9Cfl : public DecodeHistogramDevice
{
public:
    using DecodeHistogramVebox = CodechalDecodeHistogramVeboxG9;

    MOS_STATUS Initialize(
        CodechalHwInterface *hwInterface,
        PMOS_INTERFACE osInterface);
};

class MediaInterfacesHwInfoDeviceG9Cfl : public MediaInterfacesHwInfoDevice
{
public:
    MOS_STATUS Initialize(PLATFORM platform);
};

#endif // __MEDIA_INTERFACES_G9_CFL_H__
