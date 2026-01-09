/*===================== begin_copyright_notice ==================================
Copyright (c) 2026, Intel Corporation

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
//! \file     media_interfaces_nvls.h
//! \brief    All interfaces used for NVL_Xe3G that require factory creation
//!

#ifndef __MEDIA_INTERFACES_NVL_XE3G_H__
#define __MEDIA_INTERFACES_NVL_XE3G_H__

#include "media_interfaces_mhw_next.h"
#include "media_interfaces_codechal_next.h"
#include "media_interfaces_vphal.h"
#include "media_interfaces_renderhal.h"
#include "media_interfaces_hwinfo_device.h"
#include "media_interfaces_mcpy_next.h"
#include "media_interfaces_huc_kernel_source.h"

#include "mhw_cp_interface.h"
#include "mhw_state_heap_xe2_hpg.h"
#include "renderhal_xe2_hpg_next.h"
#include "media_copy_xe3p_lpm_base.h"
#include "decode_scalability_singlepipe_next.h"
#include "decode_scalability_multipipe_next.h"

#ifdef _AVC_DECODE_SUPPORTED
#include "decode_avc_pipeline_adapter_xe3p_lpm_base.h"
#endif

#ifdef _HEVC_DECODE_SUPPORTED
#include "decode_hevc_pipeline_adapter_xe3p_lpm_base.h"
#endif

#ifdef _JPEG_DECODE_SUPPORTED
#include "decode_jpeg_pipeline_adapter_xe3p_lpm_base.h"
#endif

#ifdef _MPEG2_DECODE_SUPPORTED
#include "decode_mpeg2_pipeline_adapter_xe3p_lpm_base.h"
#endif

#ifdef _VP9_DECODE_SUPPORTED
#include "decode_vp9_pipeline_adapter_xe3p_lpm_base.h"
#endif

#ifdef _VP8_DECODE_SUPPORTED
#include "decode_vp8_pipeline_adapter_xe3p_lpm_base.h"
#endif

#ifdef _AV1_DECODE_SUPPORTED
#include "decode_av1_pipeline_adapter_xe3p_lpm_base.h"
#endif

#ifdef _VVC_DECODE_SUPPORTED
#include "decode_vvc_pipeline_adapter_xe3p_lpm_base.h"
#endif

#ifdef _MEDIA_RESERVED
#ifdef _JPEG_ENCODE_SUPPORTED
#include "encode_jpeg_pipeline_adapter_xe3p_lpm_base.h"
#endif

#ifdef _HEVC_ENCODE_VDENC_SUPPORTED
#include "encode_hevc_vdenc_pipeline_adapter_xe3p_lpm_base.h"
#endif

#ifdef _AV1_ENCODE_VDENC_SUPPORTED
#include "encode_av1_vdenc_pipeline_adapter_xe3p_lpm.h"
#endif

#ifdef _AVC_ENCODE_VDENC_SUPPORTED
#include "encode_avc_vdenc_pipeline_adapter_xe3p_lpm.h"
#endif

#ifdef _VP9_ENCODE_VDENC_SUPPORTED
#include "encode_vp9_vdenc_pipeline_adapter_xe3p_lpm_base.h"
#endif
#endif
class MhwInterfacesNvl_Xe3G : virtual public MhwInterfacesNext
{
public:
    using Cp        = MhwCpInterface;
    using StateHeap = MHW_STATE_HEAP_INTERFACE_XE2_HPG;

    virtual MOS_STATUS Initialize(
        CreateParams   params,
        PMOS_INTERFACE osInterface);

protected:
    virtual void InitializeRenderComponent(
        CreateParams   params,
        PMOS_INTERFACE osInterface);

MEDIA_CLASS_DEFINE_END(MhwInterfacesNvl_Xe3G)
};

class CodechalDecodeInterfacesXe3P_Lpm
{
public:
#ifdef _AVC_DECODE_SUPPORTED
    using Avc = DecodeAvcPipelineAdapterXe3P_Lpm_Base;
#endif
#ifdef _HEVC_DECODE_SUPPORTED
    using Hevc = DecodeHevcPipelineAdapterXe3P_Lpm_Base;
#endif
#ifdef _JPEG_DECODE_SUPPORTED
    using Jpeg = DecodeJpegPipelineAdapterXe3P_Lpm_Base;
#endif
#ifdef _MPEG2_DECODE_SUPPORTED
    using Mpeg2 = DecodeMpeg2PipelineAdapterXe3P_Lpm_Base;
#endif
#ifdef _VP9_DECODE_SUPPORTED
    using Vp9 = DecodeVp9PipelineAdapterXe3P_Lpm_Base;
#endif
#ifdef _VP8_DECODE_SUPPORTED
    using Vp8 = DecodeVp8PipelineAdapterXe3P_Lpm_Base;
#endif
#ifdef _AV1_DECODE_SUPPORTED
    using Av1 = DecodeAv1PipelineAdapterXe3P_Lpm_Base;
#endif
#ifdef _VVC_DECODE_SUPPORTED
    using Vvc = DecodeVvcPipelineAdapterXe3P_Lpm_Base;
#endif

    MEDIA_CLASS_DEFINE_END(CodechalDecodeInterfacesXe3P_Lpm)
};

class CodechalInterfacesXe3P_Lpm : public CodechalDeviceNext
{
public:
    using Decode = CodechalDecodeInterfacesXe3P_Lpm;
    using Hw     = CodechalHwInterfaceXe3P_Lpm_Base;

    MOS_STATUS Initialize(
        void *         standardInfo,
        void *         settings,
        MhwInterfacesNext *mhwInterfaces,
        PMOS_INTERFACE osInterface) override;

    MEDIA_CLASS_DEFINE_END(CodechalInterfacesXe3P_Lpm)
};

class VphalInterfacesNvl_Xe3G : public VphalDevice
{
public:
    MOS_STATUS Initialize(
        PMOS_INTERFACE osInterface,
        bool           bInitVphalState,
        MOS_STATUS *   eStatus,
        bool           clearViewMode = false) override;

    MOS_STATUS CreateVpPlatformInterface(
        PMOS_INTERFACE osInterface,
        MOS_STATUS *   eStatus) override;

protected:
    virtual void InitPlatformKernelBinary(
        vp::VpPlatformInterface *&vpPlatformInterface);

    MEDIA_CLASS_DEFINE_END(VphalInterfacesNvl_Xe3G)
};

class RenderHalInterfacesNvl_Xe3G : public RenderHalDevice
{
protected:
    using XRenderHal = XRenderHal_Interface_Xe2_Hpg_Next;
    MOS_STATUS Initialize();

    MEDIA_CLASS_DEFINE_END(RenderHalInterfacesNvl_Xe3G)
};

class MediaInterfacesHwInfoDeviceXe3P_Lpm : public MediaInterfacesHwInfoDevice
{
public:
    virtual MOS_STATUS Initialize(PLATFORM platform) override;

    MEDIA_CLASS_DEFINE_END(MediaInterfacesHwInfoDeviceXe3P_Lpm)
};

class McpyDeviceXe3P_Lpm_Base: public McpyDeviceNextImpl<MediaCopyStateXe3P_Lpm_Base>
{
protected:
    virtual MhwInterfacesNext *CreateMhwInterface(
        PMOS_INTERFACE osInterface)
    {
            MhwInterfacesNext::CreateParams params;
            MOS_ZeroMemory(&params, sizeof(params));
            params.Flags.m_vebox     = true;
            params.Flags.m_blt       = true;

            MhwInterfacesNext *mhw = MhwInterfacesNext::CreateFactory(params, osInterface);
        
            return mhw;
    }
    MEDIA_CLASS_DEFINE_END(McpyDeviceXe3P_Lpm_Base)
};

class HucKernelSourceDeviceXe3P_Lpm : public HucKernelSourceDevice
{
public:
    MOS_STATUS Initialize() override;

    MEDIA_CLASS_DEFINE_END(HucKernelSourceDeviceXe3P_Lpm)
};
#endif  // __MEDIA_INTERFACES_NVL_XE3G_H__
