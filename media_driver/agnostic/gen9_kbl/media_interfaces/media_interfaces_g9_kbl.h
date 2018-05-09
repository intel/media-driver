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
//! \file     media_interfaces_g9_kbl.h
//! \brief    All interfaces used for KBL that require factory creation
//!

#ifndef __MEDIA_INTERFACES_G9_KBL_H__
#define __MEDIA_INTERFACES_G9_KBL_H__

#include "media_interfaces_mhw.h"
#include "media_interfaces_codechal.h"
#include "media_interfaces_mmd.h"
#include "media_interfaces_cmhal.h"
#include "media_interfaces_mosutil.h"
#include "media_interfaces_vphal.h"
#include "media_interfaces_renderhal.h"
#include "media_interfaces_nv12top010.h"
#include "media_interfaces_decode_histogram.h"

#include "mhw_cp.h"
#include "mhw_mi_g9_X.h"
#include "mhw_render_g9_X.h"
#include "mhw_sfc_g9_X.h"
#include "mhw_state_heap_g9.h"
#include "mhw_vebox_g9_X.h"
#include "mhw_vdbox_mfx_g9_kbl.h"
#include "mhw_vdbox_hcp_g9_kbl.h"
#include "mhw_vdbox_huc_g9_bxt.h"
#include "mhw_vdbox_vdenc_g9_kbl.h"

#include "codechal_hw_g9_kbl.h"
#ifdef _AVC_DECODE_SUPPORTED
#include "codechal_decode_avc.h"
#endif

#include "codechal_decode_downsampling_g9.h"

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
#include "codechal_decode_vc1_g9.h"
#endif

#ifdef _VP8_DECODE_SUPPORTED
#include "codechal_decode_vp8.h"
#endif

#ifdef _VP9_DECODE_SUPPORTED
#include "codechal_decode_vp9.h"
#endif
#include "codechal_decode_nv12top010_g9.h"

#ifdef _JPEG_ENCODE_SUPPORTED
#include "codechal_encode_jpeg.h"
#endif

#ifdef _MPEG2_ENCODE_SUPPORTED
#include "codechal_encode_mpeg2_g9.h"
#endif

#ifdef _HEVC_ENCODE_SUPPORTED
#include "codechal_encode_hevc_g9_kbl.h"
#endif

#ifdef _AVC_ENCODE_SUPPORTED
#include "codechal_encode_avc_g9_kbl.h"
#include "codechal_vdenc_avc_g9_kbl.h"
#endif
#include "codechal_encode_csc_ds_g9.h"

#include "codechal_memdecomp_g9.h"

#include "cm_hal_g9.h"
#include "vphal_g9.h"

#include "renderhal_g9.h"

#include "codechal_decode_histogram_vebox_g9.h"

class MhwInterfacesG9Kbl : public MhwInterfaces
{
public:
    using Mi = MhwMiInterfaceG9;
    using Cp = MhwCpInterface;
    using Render = MhwRenderInterfaceG9;
    using Sfc = MhwSfcInterfaceG9;
    using StateHeap = MHW_STATE_HEAP_INTERFACE_G9_X;
    using Mfx = MhwVdboxMfxInterfaceG9Kbl;
    using Hcp = MhwVdboxHcpInterfaceG9Kbl;
    using Huc = MhwVdboxHucInterfaceG9Bxt;
    using Vdenc = MhwVdboxVdencInterfaceG9Kbl;
    using Vebox = MhwVeboxInterfaceG9;

    MOS_STATUS Initialize(
        CreateParams params,
        PMOS_INTERFACE osInterface);
};

class MmdDeviceG9Kbl : public MmdDevice
{
public:
    using Mmd  = MediaMemDecompStateG9;

    MOS_STATUS Initialize(
        PMOS_INTERFACE osInterface,
        MhwInterfaces *mhwInterfaces);
};

class Nv12ToP010DeviceG9Kbl : public Nv12ToP010Device
{
public:
    using Nv12ToP010  = CodechalDecodeNV12ToP010G9;

    MOS_STATUS Initialize(
        PMOS_INTERFACE osInterface);
};

class CodechalDecodeInterfacesG9Kbl
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
    using Vc1 = CodechalDecodeVc1G9;
#endif
#ifdef _VP8_DECODE_SUPPORTED
    using Vp8 = CodechalDecodeVp8;
#endif
#ifdef _VP9_DECODE_SUPPORTED
    using Vp9 = CodechalDecodeVp9;
#endif
    using Nv12ToP010 = CodechalDecodeNV12ToP010;
#ifdef _DECODE_PROCESSING_SUPPORTED
    using FieldScaling = FieldScalingInterfaceG9;
#endif
};

class CodechalEncodeInterfacesG9Kbl
{
public:
#ifdef _JPEG_ENCODE_SUPPORTED
    using Jpeg = CodechalEncodeJpegState;
#endif
#ifdef _MPEG2_ENCODE_SUPPORTED
    using Mpeg2 = CodechalEncodeMpeg2G9;
#endif

    using CscDs = CodechalEncodeCscDsG9;

#ifdef _HEVC_ENCODE_SUPPORTED
    using HevcEnc = CodechalEncHevcStateG9Kbl;
#endif
#ifdef _AVC_ENCODE_SUPPORTED
    using AvcEnc   = CodechalEncodeAvcEncG9Kbl;
    using AvcVdenc = CodechalVdencAvcStateG9Kbl;
#endif
};

class CodechalInterfacesG9Kbl : public CodechalDevice
{
public:
    using Decode = CodechalDecodeInterfacesG9Kbl;
    using Encode = CodechalEncodeInterfacesG9Kbl; 
    using Hw = CodechalHwInterfaceG9Kbl;

    MOS_STATUS Initialize(
        void *standardInfo,
        void *settings,
        MhwInterfaces *mhwInterfaces,
        PMOS_INTERFACE osInterface) override;

    CodechalHwInterface *CreateCodechalHwInterface(
        CODECHAL_FUNCTION CodecFunction,
        MhwInterfaces *mhwInterfaces,
        PMOS_INTERFACE osInterface) override;
};

class CMHalInterfacesG9Kbl : public CMHalDevice
{
protected:
    using CMHal = CM_HAL_G9_X;
    MOS_STATUS Initialize(
        CM_HAL_STATE *pCmState);
};

class MosUtilDeviceG9Kbl : public MosUtilDevice
{
public:
    using MosUtil = MosUtilUserInterface;

    MOS_STATUS Initialize();
};

class VphalInterfacesG9Kbl : public VphalDevice
{
public:
    using VphalState = VphalStateG9;

    MOS_STATUS Initialize(
        PMOS_INTERFACE  osInterface,
        PMOS_CONTEXT    osDriverContext,
        MOS_STATUS      *eStatus);
};

class RenderHalInterfacesG9Kbl : public RenderHalDevice
{
protected:
    using XRenderHal = XRenderHal_Interface_g9;
    MOS_STATUS Initialize();
};

class DecodeHistogramDeviceG9Kbl : public DecodeHistogramDevice
{
public:
    using DecodeHistogramVebox = CodechalDecodeHistogramVeboxG9;

    MOS_STATUS Initialize(
        CodechalHwInterface *hwInterface,
        PMOS_INTERFACE osInterface);
};

#endif // __MEDIA_INTERFACES_G9_KBL_H__
