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
//! \file     media_interfaces_g9_glk.h
//! \brief    All interfaces used for GLK that require factory creation
//!

#ifndef __MEDIA_INTERFACES_G9_GLK_H__
#define __MEDIA_INTERFACES_G9_GLK_H__
#include "media_factory.h"
#include "media_interfaces_cmhal.h"
#include "media_interfaces_vphal.h"
#include "media_interfaces_decode_histogram.h"

#ifdef _HEVC_ENCODE_VME_SUPPORTED
#include "codechal_encode_hevc_g9_glk.h"
#endif

#ifdef _AVC_ENCODE_VME_SUPPORTED
#include "codechal_encode_avc_g9_kbl.h"
#endif
#ifdef _AVC_ENCODE_VDENC_SUPPORTED
#include "codechal_vdenc_avc_g9_kbl.h"
#endif
#ifdef _VP8_ENCODE_SUPPORTED
#include "codechal_encode_vp8_g9.h"
#endif
#include "codechal_decode_nv12top010_g9_glk.h"
#include "cm_hal_g9.h"
#include "vphal_g9_glk.h"

#include "codechal_decode_histogram_vebox_g9.h"

class CMHalInterfacesG9Glk : public CMHalDevice
{
protected:
    using CMHal = CM_HAL_G9_X;
    MOS_STATUS Initialize(
        CM_HAL_STATE *pCmState);
};

class VphalInterfacesG9Glk : public VphalDevice
{
public:
    using VphalState = VphalStateG9Glk;

    MOS_STATUS Initialize(
        PMOS_INTERFACE  osInterface,
        bool            bInitVphalState,
        MOS_STATUS      *eStatus,
        bool            clearViewMode = false);
};

class CodechalEncodeInterfacesG9Glk
{
public:
#ifdef _JPEG_ENCODE_SUPPORTED
    using Jpeg = CodechalEncodeJpegState;
#endif
#ifdef _MPEG2_ENCODE_VME_SUPPORTED
    using Mpeg2 = CodechalEncodeMpeg2G9;
#endif
    using CscDs = CodechalEncodeCscDsG9;
#ifdef _HEVC_ENCODE_VME_SUPPORTED
    using HevcEnc = CodechalEncHevcStateG9Glk;
#endif
#ifdef _AVC_ENCODE_VME_SUPPORTED
    using AvcEnc = CodechalEncodeAvcEncG9Kbl;
#endif
#ifdef _AVC_ENCODE_VDENC_SUPPORTED
    using AvcVdenc = CodechalVdencAvcStateG9Kbl;
#endif
#ifdef _VP8_ENCODE_SUPPORTED
    using Vp8 = CodechalEncodeVp8G9;
#endif
};

class CodechalInterfacesG9Glk : public CodechalDevice
{
public:
    using Decode = CodechalDecodeInterfacesG9Kbl;
    using Encode = CodechalEncodeInterfacesG9Glk;
    using Hw = CodechalHwInterfaceG9Kbl;

    MOS_STATUS Initialize(
        void *standardInfo,
        void *settings,
        MhwInterfaces *mhwInterfaces,
        PMOS_INTERFACE osInterface) override;
};

class Nv12ToP010DeviceG9Glk : public Nv12ToP010Device
{
public:
    using Nv12ToP010  = CodechalDecodeNv12ToP010G9Glk;

    MOS_STATUS Initialize(
            PMOS_INTERFACE osInterface);
};

class DecodeHistogramDeviceG9Glk : public DecodeHistogramDevice
{
public:
    using DecodeHistogramVebox = CodechalDecodeHistogramVeboxG9;

    MOS_STATUS Initialize(
        CodechalHwInterface *hwInterface,
        PMOS_INTERFACE osInterface);
};

#endif // __MEDIA_INTERFACES_G9_GLK_H__
