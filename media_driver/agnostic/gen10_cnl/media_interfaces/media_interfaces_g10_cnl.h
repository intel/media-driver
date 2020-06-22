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
//! \file     media_interfaces_g10_cnl.h
//! \brief    All interfaces used for CNL that require factory creation
//!

#ifndef __MEDIA_INTERFACES_G10_CNL_H__
#define __MEDIA_INTERFACES_G10_CNL_H__

#include "media_interfaces_mhw.h"
#include "media_interfaces_codechal.h"
#include "media_interfaces_mmd.h"
#include "media_interfaces_cmhal.h"
#include "media_interfaces_mosutil.h"
#include "media_interfaces_vphal.h"
#include "media_interfaces_renderhal.h"
#include "media_interfaces_nv12top010.h"
#include "media_interfaces_decode_histogram.h"

#include "mhw_cp_interface.h"
#include "mhw_mi_g10_X.h"
#include "mhw_render_g10_X.h"
#include "mhw_sfc_g10_X.h"
#include "mhw_state_heap_g10.h"
#include "mhw_vebox_g10_X.h"
#include "mhw_vdbox_mfx_g10_X.h"
#include "mhw_vdbox_hcp_g10_X.h"
#include "mhw_vdbox_huc_g10_X.h"
#include "mhw_vdbox_vdenc_g10_X.h"

#include "codechal_hw_g10_X.h"

#ifdef _AVC_DECODE_SUPPORTED
#include "codechal_decode_avc.h"
#endif

#include "codechal_decode_downsampling_g10.h"

#ifdef _HEVC_DECODE_SUPPORTED
#include "codechal_decode_hevc.h"
#endif

#ifdef _JPEG_DECODE_SUPPORTED
#include "codechal_decode_jpeg.h"
#endif

#ifdef _MPEG2_DECODE_SUPPORTED
#include "codechal_decode_mpeg2.h"
#endif

#ifdef _VC1_DECODE_SUPPORTED
#include "codechal_decode_vc1_g10.h"
#endif

#ifdef _VP8_DECODE_SUPPORTED
#include "codechal_decode_vp8.h"
#endif

#ifdef _VP9_DECODE_SUPPORTED
#include "codechal_decode_vp9.h"
#endif

#include "codechal_decode_nv12top010_g10.h"

#include "codechal_memdecomp_g10.h"
#ifdef _AVC_ENCODE_VME_SUPPORTED
#include "codechal_encode_avc_g10.h"
#endif
#ifdef _AVC_ENCODE_VDENC_SUPPORTED
#include "codechal_vdenc_avc_g10.h"
#endif
#include "codechal_encode_sfc.h"
#include "codechal_encode_csc_ds_g10.h"

#ifdef _VP9_ENCODE_VDENC_SUPPORTED
#include "codechal_vdenc_vp9_g10.h"
#endif

#ifdef _JPEG_ENCODE_SUPPORTED
#include "codechal_encode_jpeg.h"
#endif

#ifdef _HEVC_ENCODE_VME_SUPPORTED
#include "codechal_encode_hevc_g10.h"
#endif
#ifdef _HEVC_ENCODE_VDENC_SUPPORTED
#include "codechal_vdenc_hevc_g10.h"
#endif

#ifdef _MPEG2_ENCODE_VME_SUPPORTED
#include "codechal_encode_mpeg2_g10.h"
#endif

#ifdef _VP8_ENCODE_SUPPORTED
#include "codechal_encode_vp8_g10.h"
#endif

#include "cm_hal_g10.h"
#include "mos_utilities.h"
#include "vphal_g10.h"

#include "renderhal_g10.h"

#include "codechal_decode_histogram_vebox_g10.h"

class MhwInterfacesG10Cnl : public MhwInterfaces
{
public:
    using Mi = MhwMiInterfaceG10;
    using Cp = MhwCpInterface;
    using Render = MhwRenderInterfaceG10;
    using Sfc = MhwSfcInterfaceG10;
    using StateHeap = MHW_STATE_HEAP_INTERFACE_G10_X;
    using Mfx = MhwVdboxMfxInterfaceG10;
    using Hcp = MhwVdboxHcpInterfaceG10;
    using Huc = MhwVdboxHucInterfaceG10;
    using Vdenc = MhwVdboxVdencInterfaceG10;
    using Vebox = MhwVeboxInterfaceG10;

    MOS_STATUS Initialize(
        CreateParams params,
        PMOS_INTERFACE osInterface);
};

class MmdDeviceG10Cnl : public MmdDevice
{
public:
    using Mmd  = MediaMemDecompStateG10;

    MOS_STATUS Initialize(
        PMOS_INTERFACE osInterface,
        MhwInterfaces *mhwInterfaces);
};

class Nv12ToP010DeviceG10Cnl : public Nv12ToP010Device
{
public:
    using Nv12ToP010  = CodechalDecodeNV12ToP010G10;

    MOS_STATUS Initialize(
        PMOS_INTERFACE osInterface);
};

class CodechalDecodeInterfacesG10Cnl
{
public:
#ifdef _AVC_DECODE_SUPPORTED
    using Avc = CodechalDecodeAvc;
#endif
#ifdef _HEVC_DECODE_SUPPORTED
    using Hevc = CodechalDecodeHevc;
#endif
#ifdef _JPEG_DECODE_SUPPORTED
    using Jpeg = CodechalDecodeJpeg;
#endif
#ifdef _MPEG2_DECODE_SUPPORTED
    using Mpeg2 = CodechalDecodeMpeg2;
#endif
#ifdef _VC1_DECODE_SUPPORTED
    using Vc1 = CodechalDecodeVc1G10;
#endif
#ifdef _VP8_DECODE_SUPPORTED
    using Vp8 = CodechalDecodeVp8;
#endif
#ifdef _VP9_DECODE_SUPPORTED
    using Vp9 = CodechalDecodeVp9;
#endif
    using Nv12ToP010 = CodechalDecodeNV12ToP010G10;
#ifdef _DECODE_PROCESSING_SUPPORTED
    using FieldScaling = FieldScalingInterfaceG10;
#endif
};

class CodechalEncodeInterfacesG10Cnl
{
public:
#ifdef _AVC_ENCODE_VME_SUPPORTED
    using AvcEnc = CodechalEncodeAvcEncG10;
#endif
#ifdef _AVC_ENCODE_VDENC_SUPPORTED
    using AvcVdenc = CodechalVdencAvcStateG10;
#endif
    using CscDs = CodechalEncodeCscDsG10;
#ifdef _JPEG_ENCODE_SUPPORTED
    using Jpeg = CodechalEncodeJpegState;
#endif
#ifdef _VP9_ENCODE_VDENC_SUPPORTED
    using Vp9 = CodechalVdencVp9StateG10;
#endif
#ifdef _MPEG2_ENCODE_VME_SUPPORTED
    using Mpeg2 = CodechalEncodeMpeg2G10;
#endif
#ifdef _HEVC_ENCODE_VME_SUPPORTED
    using HevcEnc = CodechalEncHevcStateG10;
#endif
#ifdef _HEVC_ENCODE_VDENC_SUPPORTED
    using HevcVdenc = CodechalVdencHevcStateG10;
#endif
#ifdef _VP8_ENCODE_SUPPORTED
    using Vp8 = CodechalEncodeVp8G10;
#endif
};

class CodechalInterfacesG10Cnl : public CodechalDevice
{
public:
    using Decode = CodechalDecodeInterfacesG10Cnl;
    using Hw = CodechalHwInterfaceG10;
    using Encode = CodechalEncodeInterfacesG10Cnl;

    MOS_STATUS Initialize(
        void *standardInfo,
        void *settings,
        MhwInterfaces *mhwInterfaces,
        PMOS_INTERFACE osInterface) override;
};

class CMHalInterfacesG10Cnl : public CMHalDevice
{
protected:
    using CMHal = CM_HAL_G10_X;
    MOS_STATUS Initialize(
        CM_HAL_STATE *pCmState);
};

class MosUtilDeviceG10Cnl : public MosUtilDevice
{
public:
    using MosUtil = MediaUserSettingsMgr;

    MOS_STATUS Initialize();
};

class VphalInterfacesG10Cnl : public VphalDevice
{
public:
    using VphalState = VphalStateG10;

    MOS_STATUS Initialize(
        PMOS_INTERFACE  osInterface,
        PMOS_CONTEXT    osDriverContext,
        MOS_STATUS      *eStatus);
};

class RenderHalInterfacesG10Cnl : public RenderHalDevice
{
protected:
    using XRenderHal = XRenderHal_Interface_g10;
    MOS_STATUS Initialize();
};

class DecodeHistogramDeviceG10Cnl : public DecodeHistogramDevice
{
public:
    using DecodeHistogramVebox = CodechalDecodeHistogramVeboxG10;

    MOS_STATUS Initialize(
        CodechalHwInterface *hwInterface,
        PMOS_INTERFACE osInterface);
};

#endif // __MEDIA_INTERFACES_G10_CNL_H__
