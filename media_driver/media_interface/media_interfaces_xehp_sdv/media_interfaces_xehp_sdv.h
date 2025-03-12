/*===================== begin_copyright_notice ==================================

# Copyright (c) 2021, Intel Corporation

# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
# OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
# ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
# OTHER DEALINGS IN THE SOFTWARE.

======================= end_copyright_notice ==================================*/
//!
//! \file     media_interfaces_xehp_sdv.h
//! \brief    All interfaces used for XeHP that require factory creation
//!

#ifndef __MEDIA_INTERFACES_XE_XPM_H__
#define __MEDIA_INTERFACES_XE_XPM_H__

#include "media_interfaces_mhw.h"
#include "media_interfaces_codechal.h"
#include "media_interfaces_mmd.h"
#include "media_interfaces_mcpy.h"
#include "media_interfaces_cmhal.h"
#include "media_interfaces_vphal.h"
#include "media_interfaces_renderhal.h"
#include "media_interfaces_nv12top010.h"
#include "media_interfaces_decode_histogram.h"
#include "media_interfaces_hwinfo_device.h"

#include "mhw_cp_interface.h"
#if IGFX_GEN12_SUPPORTED
#include "mhw_mi_g12_X.h"
#endif
#include "mhw_render_xe_hp.h"
#include "mhw_sfc_xe_xpm.h"
#include "mhw_state_heap_xe_xpm.h"
#include "mhw_vebox_xe_xpm.h"
#include "mhw_vdbox_mfx_xe_xpm.h"
#include "mhw_vdbox_hcp_xe_xpm.h"
#include "mhw_vdbox_avp_xe_xpm.h"
#include "mhw_vdbox_huc_g12_X.h"
#include "mhw_vdbox_avp_g12_X.h"
#ifdef IGFX_XEHP_SDV_ENABLE_NON_UPSTREAM
#include "mhw_vdbox_vdenc_xe_xpm.h"
#include "mhw_vdbox_vdenc_xe_xpm_ext.h"
#else
#include "mhw_vdbox_vdenc_g12_X.h"
#endif
#include "vphal_render_vebox_memdecomp_xe_xpm.h"
#include "media_copy_xe_xpm_base.h"
#include "mhw_blt_xe_hp_base.h"

#include "codechal_hw_xe_xpm.h"

#ifdef _AVC_DECODE_SUPPORTED
#include "decode_avc_pipeline_adapter_m12.h"
#endif

#ifdef _HEVC_DECODE_SUPPORTED
#include "decode_hevc_pipeline_adapter_m12.h"
#endif

#ifdef _JPEG_DECODE_SUPPORTED
#include "decode_jpeg_pipeline_adapter_m12.h"
#endif

#ifdef _MPEG2_DECODE_SUPPORTED
#include "decode_mpeg2_pipeline_adapter_m12.h"
#endif

#ifdef _VC1_DECODE_SUPPORTED
#include "codechal_decode_vc1_xe_xpm.h"
#endif

#ifdef _VP8_DECODE_SUPPORTED
#include "codechal_decode_vp8_g12.h"
#endif

#ifdef _VP9_DECODE_SUPPORTED
#include "decode_vp9_pipeline_adapter_m12.h"
#endif

#ifdef _AV1_DECODE_SUPPORTED
#include "decode_av1_pipeline_adapter_g12.h"
#endif

#ifdef IGFX_XEHP_SDV_ENABLE_NON_UPSTREAM
#ifdef _JPEG_ENCODE_SUPPORTED
#include "codechal_encode_jpeg_g12.h"
#endif

#include "codechal_encode_csc_ds_mdf_xe_xpm.h"

#ifdef _MPEG2_ENCODE_VME_SUPPORTED
#include "codechal_encode_mpeg2_g12.h"
#include "codechal_encode_mpeg2_mdf_xe_xpm.h"
#endif

#ifdef _HEVC_ENCODE_VME_SUPPORTED
#include "codechal_encode_hevc_g12.h"
#include "codechal_encode_hevc_mbenc_xe_xpm.h"
#include "codechal_encode_csc_ds_mdf_g12.h"
#endif
#ifdef _HEVC_ENCODE_VDENC_SUPPORTED
#include "codechal_vdenc_hevc_xe_xpm.h"
#ifdef _APOGEIOS_SUPPORTED
#include "decode_hevc_pipeline_adapter_m12.h"
#endif
#endif

#ifdef _AVC_ENCODE_VME_SUPPORTED
#include "codechal_encode_avc_g12.h"
#include "codechal_encode_avc_mdf_xe_xpm.h"
#endif
#ifdef _AVC_ENCODE_VDENC_SUPPORTED
#include "codechal_vdenc_avc_xe_xpm.h"
#endif


#ifdef _VP9_ENCODE_VDENC_SUPPORTED
#include "codechal_vdenc_vp9_g12.h"
#include "codechal_vdenc_vp9_xe_hpm.h"
#ifdef _APOGEIOS_SUPPORTED
#include "encode_vp9_vdenc_pipeline_adapter_xe_xpm.h"
#endif
#endif


#ifdef IGFX_XEHP_SDV_ENABLE_NON_UPSTREAM
#include "cm_hal_xe_xpm.h"
#endif
#endif
#include "vphal_xe_xpm.h"
#include "renderhal_xe_hp.h"

#include "codechal_decode_histogram.h"
#include "codechal_decode_histogram_g12.h"

#include "decode_scalability_singlepipe.h"
#include "decode_scalability_multipipe.h"
class MhwInterfacesXehp_Sdv : public MhwInterfaces
{
public:
    MhwVdboxAvpInterface *m_avpInterface = nullptr;

    using Mi        = MhwMiInterfaceG12;
    using Cp        = MhwCpInterface;
    using Render    = MhwRenderInterfaceXe_Hp;
    using Sfc       = MhwSfcInterfaceXe_Xpm;
    using StateHeap = MHW_STATE_HEAP_INTERFACE_XE_XPM;
    using Mfx       = MhwVdboxMfxInterfaceXe_Xpm;
    using Hcp       = MhwVdboxHcpInterfaceXe_Xpm;
    using Avp       = MhwVdboxAvpInterfaceXe_Xpm;
    using Huc       = MhwVdboxHucInterfaceG12;
#ifdef IGFX_XEHP_SDV_ENABLE_NON_UPSTREAM
    using Vdenc     = MhwVdboxVdencInterfaceG12ExtXehp;
#else
    using Vdenc     = MhwVdboxVdencInterfaceG12X;
#endif
    using Vebox     = MhwVeboxInterfaceXe_Xpm;
    using Blt       = MhwBltInterfaceXe_Hp_Base;

    virtual MOS_STATUS Initialize(
        CreateParams params,
        PMOS_INTERFACE osInterface);

};

class MmdDeviceXe_Xpm : public MmdDevice
{
public:
    using Mmd  = MediaVeboxDecompStateXe_Xpm;

    MOS_STATUS Initialize(
        PMOS_INTERFACE osInterface,
        MhwInterfaces *mhwInterfaces);

    MhwInterfaces* CreateMhwInterface(
        PMOS_INTERFACE osInterface);
};

class McpyDeviceXe_Xpm : public McpyDevice
{
public:
    using Mcpy  = MediaCopyStateXe_Xpm_Base;

    MOS_STATUS Initialize(
        PMOS_INTERFACE osInterface);
protected:
    MhwInterfaces* CreateMhwInterface(
        PMOS_INTERFACE osInterface);
};

class Nv12ToP010DeviceXe_Xpm : public Nv12ToP010Device
{
public:

    MOS_STATUS Initialize(
        PMOS_INTERFACE osInterface);
};

class CodechalDecodeInterfacesXe_Xpm
{
public:
#ifdef _AVC_DECODE_SUPPORTED
    using Avc = DecodeAvcPipelineAdapterM12;
#endif
#ifdef _HEVC_DECODE_SUPPORTED
    using Hevc = DecodeHevcPipelineAdapterM12;
#endif
#ifdef _JPEG_DECODE_SUPPORTED
    using Jpeg = DecodeJpegPipelineAdapterM12;
#endif
#ifdef _MPEG2_DECODE_SUPPORTED
    using Mpeg2 = DecodeMpeg2PipelineAdapterM12;
#endif
#ifdef _VC1_DECODE_SUPPORTED
    using Vc1 = CodechalDecodeVc1Xe_Xpm;
#endif
#ifdef _VP8_DECODE_SUPPORTED
    using Vp8 = CodechalDecodeVp8G12;
#endif
#ifdef _VP9_DECODE_SUPPORTED
    using Vp9 = DecodeVp9PipelineAdapterG12;
#endif
#ifdef _AV1_DECODE_SUPPORTED
    using Av1 = DecodeAv1PipelineAdapterG12;
#endif
};

#ifdef IGFX_XEHP_SDV_ENABLE_NON_UPSTREAM
class CodechalEncodeInterfacesXe_Xpm
{
public:
    using CscDs = CodechalEncodeCscDsMdfXe_Xpm;
    using CscDsMdf = CodechalEncodeCscDsMdfXe_Xpm;
#ifdef _VP9_ENCODE_VDENC_SUPPORTED
    using Vp9 = CodechalVdencVp9StateXe_Xpm;
#endif
#ifdef _JPEG_ENCODE_SUPPORTED
    using Jpeg = CodechalEncodeJpegStateG12;
#endif
#ifdef _MPEG2_ENCODE_VME_SUPPORTED
    using Mpeg2 = CodechalEncodeMpeg2EncMdfG12;
#endif
#ifdef _HEVC_ENCODE_VME_SUPPORTED
    using HevcEnc = CodechalEncHevcStateG12;
    using HevcMbenc = CodecHalHevcMbencXe_Xpm;
#endif
#ifdef _HEVC_ENCODE_VDENC_SUPPORTED
    using HevcVdenc = CodechalVdencHevcStateXe_Xpm;
#endif
#ifdef _AVC_ENCODE_VME_SUPPORTED
    using AvcEnc = CodechalEncodeAvcEncMdfG12;
#endif
#ifdef _AVC_ENCODE_VDENC_SUPPORTED
    using AvcVdenc = CodechalVdencAvcStateXe_Xpm;
#endif
};
#endif
class CodechalInterfacesXe_Xpm : public CodechalDevice
{
public:
    using Decode = CodechalDecodeInterfacesXe_Xpm;
#ifdef IGFX_XEHP_SDV_ENABLE_NON_UPSTREAM
    using Encode = CodechalEncodeInterfacesXe_Xpm;
#endif
    using Hw = CodechalHwInterfaceXe_Xpm;

    MOS_STATUS Initialize(
        void *standardInfo,
        void *settings,
        MhwInterfaces *mhwInterfaces,
        PMOS_INTERFACE osInterface) override;
};

#ifdef IGFX_XEHP_SDV_ENABLE_NON_UPSTREAM
class CMHalInterfacesXe_Xpm : public CMHalDevice
{
protected:
    using CMHal = CmHalXe_Xpm;
    MOS_STATUS Initialize(
        CM_HAL_STATE *pCmState);
};
#endif

class VphalInterfacesXe_Xpm : public VphalDevice
{
public:
    using VphalState = VphalStateXe_Xpm;

    MOS_STATUS Initialize(
        PMOS_INTERFACE  osInterface,
        bool            bInitVphalState,
        MOS_STATUS      *eStatus,
        bool            clearViewMode = false);

    MOS_STATUS CreateVpPlatformInterface(
        PMOS_INTERFACE           osInterface,
        MOS_STATUS *             eStatus);
};

class RenderHalInterfacesXe_Xpm : public RenderHalDevice
{
protected:
    using XRenderHal = XRenderHal_Interface_Xe_Hp;
    MOS_STATUS Initialize();
};

class DecodeHistogramDeviceXe_Xpm : public DecodeHistogramDevice
{
public:
    using DecodeHistogramG12 = CodechalDecodeHistogramG12;

    MOS_STATUS Initialize(
        CodechalHwInterface *hwInterface,
        PMOS_INTERFACE osInterface);
};

class MediaInterfacesHwInfoDeviceXe_Xpm : public MediaInterfacesHwInfoDevice
{
public:
    virtual MOS_STATUS Initialize(PLATFORM platform) override;
};


#endif // __MEDIA_INTERFACES_XE_XPM_H__
