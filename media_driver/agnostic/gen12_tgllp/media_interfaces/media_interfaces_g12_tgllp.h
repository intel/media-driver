/*
* Copyright (c) 2017-2019, Intel Corporation
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
//! \file     media_interfaces_g12_tgllp.h
//! \brief    All interfaces used for TGLLP that require factory creation
//!

#ifndef __MEDIA_INTERFACES_G12_TGLLP_H__
#define __MEDIA_INTERFACES_G12_TGLLP_H__

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
#include "mhw_mi_g12_X.h"
#include "mhw_render_g12_X.h"
#include "mhw_sfc_g12_X.h"
#include "mhw_state_heap_g12.h"
#include "mhw_vebox_g12_X.h"
#include "mhw_vdbox_mfx_g12_X.h"
#include "mhw_vdbox_hcp_g12_X.h"
#include "mhw_vdbox_huc_g12_X.h"
#include "mhw_vdbox_vdenc_g12_X.h"

#ifdef IGFX_VDENC_INTERFACE_EXT_SUPPORT
#include "mhw_vdbox_vdenc_g12_X_ext.h"
#endif

#include "vphal_render_vebox_memdecomp_g12.h"

#include "codechal_hw_g12_X.h"
#ifdef _AVC_DECODE_SUPPORTED
#include "codechal_decode_avc_g12.h"
#endif
#include "codechal_decode_downsampling_g12.h"

#ifdef _HEVC_DECODE_SUPPORTED
#include "codechal_decode_hevc_g12.h"
#endif

#ifdef _JPEG_DECODE_SUPPORTED
#include "codechal_decode_jpeg_g12.h"
#endif

#ifdef _MPEG2_DECODE_SUPPORTED
#include "codechal_decode_mpeg2_g12.h"
#endif

#ifdef _VC1_DECODE_SUPPORTED
#include "codechal_decode_vc1_g12.h"
#endif

#ifdef _VP8_DECODE_SUPPORTED
#include "codechal_decode_vp8_g12.h"
#endif

#ifdef _VP9_DECODE_SUPPORTED
#include "codechal_decode_vp9_g12.h"
#endif

#ifdef _JPEG_ENCODE_SUPPORTED
#include "codechal_encode_jpeg_g12.h"
#endif

#include "codechal_encode_csc_ds_g12.h"
#include "codechal_encode_csc_ds_mdf_g12.h"

#ifdef _VP9_ENCODE_VDENC_SUPPORTED
#include "codechal_vdenc_vp9_g12.h"
#endif

#ifdef _MPEG2_ENCODE_VME_SUPPORTED
#include "codechal_encode_mpeg2_g12.h"
#endif

#ifdef _HEVC_ENCODE_VME_SUPPORTED
#include "codechal_encode_hevc_g12.h"
#include "codechal_encode_hevc_mbenc_g12.h"
#endif
#ifdef _HEVC_ENCODE_VDENC_SUPPORTED
#include "codechal_vdenc_hevc_g12.h"
#ifdef _APOGEIOS_SUPPORTED
#include "encode_hevc_vdenc_pipeline_adapter_g12.h"
#include "decode_hevc_pipeline_adapter_g12.h"
#endif
#endif

#ifdef _AVC_ENCODE_VME_SUPPORTED
#include "codechal_encode_avc_g12.h"
#endif
#ifdef _AVC_ENCODE_VDENC_SUPPORTED
#include "codechal_vdenc_avc_g12.h"
#endif

#include "cm_hal_g12.h"
#include "vphal_g12_tgllp.h"
#include "renderhal_g12.h"
#include "mos_util_user_interface_g12.h"

#include "codechal_decode_histogram_g12.h"

#if 0 
#include "vp_pipeline_adapter_g12.h"
#endif


class MhwInterfacesG12Tgllp : public MhwInterfaces
{
public:

    using Mi = MhwMiInterfaceG12;
    using Cp = MhwCpInterface;
    using Render = MhwRenderInterfaceG12;
    using Sfc = MhwSfcInterfaceG12;
    using StateHeap = MHW_STATE_HEAP_INTERFACE_G12_X; // to be updated when headers regenerated
    using Mfx = MhwVdboxMfxInterfaceG12;
    using Hcp = MhwVdboxHcpInterfaceG12;
    using Huc = MhwVdboxHucInterfaceG12;
#ifdef IGFX_VDENC_INTERFACE_EXT_SUPPORT
    using Vdenc = MhwVdboxVdencInterfaceG12ExtX;
#else
    using Vdenc = MhwVdboxVdencInterfaceG12X;
#endif
    using Vebox = MhwVeboxInterfaceG12;

    MOS_STATUS Initialize(
        CreateParams params,
        PMOS_INTERFACE osInterface);
};

class MmdDeviceG12Tgllp : public MmdDevice
{
public:
    using Mmd  = MediaVeboxDecompStateG12;

    MOS_STATUS Initialize(
        PMOS_INTERFACE osInterface,
        MhwInterfaces *mhwInterfaces);

    MhwInterfaces* CreateMhwInterface(
        PMOS_INTERFACE osInterface);
};

class Nv12ToP010DeviceG12Tgllp : public Nv12ToP010Device
{
public:

    MOS_STATUS Initialize(
        PMOS_INTERFACE osInterface);
};

class CodechalDecodeInterfacesG12Tgllp
{
public:
#ifdef _AVC_DECODE_SUPPORTED
    using Avc = CodechalDecodeAvcG12;
#endif
#ifdef _HEVC_DECODE_SUPPORTED
    using Hevc = CodechalDecodeHevcG12;
#endif
#ifdef _JPEG_DECODE_SUPPORTED
    using Jpeg = CodechalDecodeJpegG12;
#endif
#ifdef _MPEG2_DECODE_SUPPORTED
    using Mpeg2 = CodechalDecodeMpeg2G12;
#endif
#ifdef _VC1_DECODE_SUPPORTED
    using Vc1 = CodechalDecodeVc1G12;
#endif
#ifdef _VP8_DECODE_SUPPORTED
    using Vp8 = CodechalDecodeVp8G12;
#endif
#ifdef _VP9_DECODE_SUPPORTED
    using Vp9 = CodechalDecodeVp9G12;
#endif
#ifdef _DECODE_PROCESSING_SUPPORTED
    using FieldScaling = FieldScalingInterfaceG12;
#endif
};

class CodechalEncodeInterfacesG12Tgllp
{
public:
    using CscDs = CodechalEncodeCscDsG12;
    using CscDsMdf = CodechalEncodeCscDsMdfG12;
#ifdef _VP9_ENCODE_VDENC_SUPPORTED
    using Vp9 = CodechalVdencVp9StateG12;
#endif
#ifdef _JPEG_ENCODE_SUPPORTED
    using Jpeg = CodechalEncodeJpegStateG12;
#endif
#ifdef _MPEG2_ENCODE_VME_SUPPORTED
    using Mpeg2 = CodechalEncodeMpeg2G12;
#endif
#ifdef _HEVC_ENCODE_VME_SUPPORTED
    using HevcEnc = CodechalEncHevcStateG12;
    using HevcMbenc = CodecHalHevcMbencG12;
#endif
#ifdef _HEVC_ENCODE_VDENC_SUPPORTED
    using HevcVdenc = CodechalVdencHevcStateG12;
#endif
#ifdef _AVC_ENCODE_VME_SUPPORTED
    using AvcEnc = CodechalEncodeAvcEncG12;
#endif
#ifdef _AVC_ENCODE_VDENC_SUPPORTED
    using AvcVdenc = CodechalVdencAvcStateG12;
#endif
};


class CodechalInterfacesG12Tgllp : public CodechalDevice
{
public:
    using Decode = CodechalDecodeInterfacesG12Tgllp;
    using Encode =  CodechalEncodeInterfacesG12Tgllp;
    using Hw = CodechalHwInterfaceG12;

    MOS_STATUS Initialize(
        void *standardInfo,
        void *settings,
        MhwInterfaces *mhwInterfaces,
        PMOS_INTERFACE osInterface) override;
};

class CMHalInterfacesG12Tgllp : public CMHalDevice
{
protected:
    using CMHal = CM_HAL_G12_X;
    MOS_STATUS Initialize(
        CM_HAL_STATE *pCmState);
};

class MosUtilDeviceG12Tgllp : public MosUtilDevice
{
public:
    using MosUtil = MosUtilUserInterface_g12;

    MOS_STATUS Initialize();
};

class VphalInterfacesG12Tgllp : public VphalDevice
{
public:
    using VphalState = VphalStateG12Tgllp;

    MOS_STATUS Initialize(
        PMOS_INTERFACE  osInterface,
        PMOS_CONTEXT    osDriverContext,
        MOS_STATUS      *eStatus);
};

class RenderHalInterfacesG12Tgllp : public RenderHalDevice
{
protected:
    using XRenderHal = XRenderHal_Interface_g12;
    MOS_STATUS Initialize();
};

class DecodeHistogramDeviceG12Tgllp : public DecodeHistogramDevice
{
public:
    using DecodeHistogramG12 = CodechalDecodeHistogramG12;

    MOS_STATUS Initialize(
        CodechalHwInterface *hwInterface,
        PMOS_INTERFACE osInterface);
};

#endif // __MEDIA_INTERFACES_G12_TGLLP_H__
