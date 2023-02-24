/*
* Copyright (c) 2019-2020, Intel Corporation
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
//! \file     media_interfaces_g11_jsl_ehl.h
//! \brief    All interfaces used for JSL and EHL that require factory creation
//!

#ifndef __MEDIA_INTERFACES_G11_JSL_EHL_H__
#define __MEDIA_INTERFACES_G11_JSL_EHL_H__

#include "media_interfaces_mhw.h"
#include "media_interfaces_codechal.h"
#include "media_interfaces_mmd.h"
#include "media_interfaces_cmhal.h"
#include "media_interfaces_vphal.h"
#include "media_interfaces_renderhal.h"
#include "media_interfaces_nv12top010.h"
#include "media_interfaces_decode_histogram.h"

#include "mhw_cp_interface.h"
#include "mhw_mi_g11_X.h"
#include "mhw_render_g11_X.h"
#include "mhw_sfc_g11_X.h"
#include "mhw_state_heap_g11.h"
#include "mhw_vebox_g11_X.h"
#include "mhw_vdbox_mfx_g11_X.h"
#include "mhw_vdbox_hcp_g11_X.h"
#include "mhw_vdbox_huc_g11_X.h"
#include "mhw_vdbox_vdenc_g11_X.h"

#include "codechal_hw_g11_X.h"

#ifdef _AVC_DECODE_SUPPORTED
#include "codechal_decode_avc_g11.h"
#endif

#include "codechal_decode_downsampling_g11_icllp.h"

#ifdef _HEVC_DECODE_SUPPORTED
#include "codechal_decode_hevc_g11.h"
#endif

#ifdef _JPEG_DECODE_SUPPORTED
#include "codechal_decode_jpeg_g11.h"
#endif

#ifdef _MPEG2_DECODE_SUPPORTED
#include "codechal_decode_mpeg2_g11.h"
#endif

#ifdef _VC1_DECODE_SUPPORTED
#include "codechal_decode_vc1_g11.h"
#endif

#ifdef _VP8_DECODE_SUPPORTED
#include "codechal_decode_vp8_g11.h"
#endif

#ifdef _VP9_DECODE_SUPPORTED
#include "codechal_decode_vp9_g11.h"
#endif

#include "codechal_encode_sfc.h"
#include "codechal_encode_csc_ds_g11.h"

#ifdef _AVC_ENCODE_VDENC_SUPPORTED
#include "codechal_vdenc_avc_g11_jsl_ehl.h"
#endif

#ifdef _HEVC_ENCODE_VDENC_SUPPORTED
#include "codechal_vdenc_hevc_g11_jsl_ehl.h"
#endif

#ifdef _VP9_ENCODE_VDENC_SUPPORTED
#include "codechal_vdenc_vp9_g11_jsl_ehl.h"
#endif

#ifdef _JPEG_ENCODE_SUPPORTED
#include "codechal_encode_jpeg_g11_jsl_ehl.h"
#endif

#include "cm_hal_g11.h"
#include "mos_util_user_interface.h"
#include "vphal_g11_jsl_ehl.h"
#include "renderhal_g11_jsl_ehl.h"


class MhwInterfacesG11JslEhl : public MhwInterfaces
{
public:
    using Mi = MhwMiInterfaceG11;
    using Cp = MhwCpInterface;
    using Render = MhwRenderInterfaceG11;
    using Sfc = MhwSfcInterfaceG11;
    using StateHeap = MHW_STATE_HEAP_INTERFACE_G11_X;
    using Mfx = MhwVdboxMfxInterfaceG11;
    using Hcp = MhwVdboxHcpInterfaceG11;
    using Huc = MhwVdboxHucInterfaceG11;
    using Vdenc = MhwVdboxVdencInterfaceG11Icl;
    using Vebox = MhwVeboxInterfaceG11;

    MOS_STATUS Initialize(
        CreateParams params,
        PMOS_INTERFACE osInterface);
};

class Nv12ToP010DeviceG11JslEhl : public Nv12ToP010Device
{
public:

    MOS_STATUS Initialize(
        PMOS_INTERFACE osInterface);
};

class CodechalDecodeInterfacesG11JslEhl
{
public:
#ifdef _AVC_DECODE_SUPPORTED
    using Avc = CodechalDecodeAvcG11;
#endif
#ifdef _HEVC_DECODE_SUPPORTED
    using Hevc = CodechalDecodeHevcG11;
#endif
#ifdef _JPEG_DECODE_SUPPORTED
    using Jpeg = CodechalDecodeJpegG11;
#endif
#ifdef _MPEG2_DECODE_SUPPORTED
    using Mpeg2 = CodechalDecodeMpeg2G11;
#endif
#ifdef _VC1_DECODE_SUPPORTED
    using Vc1 = CodechalDecodeVc1G11;
#endif
#ifdef _VP8_DECODE_SUPPORTED
    using Vp8 = CodechalDecodeVp8G11;
#endif
#ifdef _VP9_DECODE_SUPPORTED
    using Vp9 = CodechalDecodeVp9G11;
#endif
#ifdef _DECODE_PROCESSING_SUPPORTED
    using FieldScaling = FieldScalingInterfaceG11IclLp;
#endif
};

class CodechalEncodeInterfacesG11JslEhl
{
public:
#ifdef _AVC_ENCODE_VDENC_SUPPORTED
    using AvcVdenc = CodechalVdencAvcStateG11JslEhl;
#endif
    using CscDs = CodechalEncodeCscDsG11;
#ifdef _VP9_ENCODE_VDENC_SUPPORTED
    using Vp9 = CodechalVdencVp9StateG11JslEhl;
#endif
#ifdef _JPEG_ENCODE_SUPPORTED
    using Jpeg = CodechalEncodeJpegStateG11JslEhl;
#endif
#ifdef _HEVC_ENCODE_VDENC_SUPPORTED
    using HevcVdenc = CodechalVdencHevcStateG11JslEhl;
#endif
};

class CodechalInterfacesG11JslEhl : public CodechalDevice
{
public:
    using Decode = CodechalDecodeInterfacesG11JslEhl;
    using Hw = CodechalHwInterfaceG11;
    using Encode = CodechalEncodeInterfacesG11JslEhl;

    MOS_STATUS Initialize(
        void *standardInfo,
        void *settings,
        MhwInterfaces *mhwInterfaces,
        PMOS_INTERFACE osInterface) override;
};

class CMHalInterfacesG11JslEhl : public CMHalDevice
{
protected:
    using CMHal = CM_HAL_G11_X;
    MOS_STATUS Initialize(
        CM_HAL_STATE *pCmState);
};

class VphalInterfacesG11JslEhl : public VphalDevice
{
public:
    using VphalState = VphalStateG11JslEhl;

    MOS_STATUS Initialize(
        PMOS_INTERFACE  osInterface,
        bool            bInitVphalState,
        MOS_STATUS      *eStatus,
        bool            clearViewMode = false);
};

class RenderHalInterfacesG11JslEhl : public RenderHalDevice
{
protected:
    using XRenderHal = XRenderHal_Interface_g11_JslEhl;
    MOS_STATUS Initialize();
};

class DecodeHistogramDeviceG11JslEhl : public DecodeHistogramDevice
{
public:
    using DecodeHistogramVebox = CodechalDecodeHistogramVeboxG11;

    MOS_STATUS Initialize(
        CodechalHwInterface *hwInterface,
        PMOS_INTERFACE osInterface);
};

#endif // __MEDIA_INTERFACES_G11_JSL_EHL_H__
