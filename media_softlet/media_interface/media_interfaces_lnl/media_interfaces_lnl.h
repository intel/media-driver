/*===================== begin_copyright_notice ==================================

* Copyright (c) 2024, Intel Corporation
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

======================= end_copyright_notice ==================================*/
//!
//! \file     media_interfaces_lnl.h
//! \brief    All interfaces used for LNL that require factory creation
//!

#ifndef __MEDIA_INTERFACES_XE2_LPM_H__
#define __MEDIA_INTERFACES_XE2_LPM_H__

#include "media_interfaces_mhw_next.h"
#include "media_interfaces_codechal_next.h"
#include "media_interfaces_vphal.h"
#include "media_interfaces_renderhal.h"
#include "media_interfaces_hwinfo_device.h"
#include "media_interfaces_mcpy_next.h"

#include "mhw_cp_interface.h"
#include "mhw_state_heap_xe2_hpg.h"
#include "mhw_vdbox_hcp_impl_xe2_lpm.h"

#include "mhw_vdbox_mfx_impl_xe2_lpm.h"

#include "mhw_vdbox_vvcp_impl_xe2_lpm.h"
#include "mhw_vdbox_avp_impl_xe2_lpm.h"

#include "mhw_vdbox_huc_impl_xe2_lpm.h"

#include "mhw_vdbox_vdenc_impl_xe2_lpm.h"

#ifdef _MEDIA_RESERVED
#include "codec_hw_xe2_lpm_base.h"
#endif

#ifdef _AVC_DECODE_SUPPORTED
#include "decode_avc_pipeline_adapter_xe2_lpm_base.h"
#endif

#ifdef _HEVC_DECODE_SUPPORTED
#include "decode_hevc_pipeline_adapter_xe2_lpm_base.h"
#endif

#ifdef _JPEG_DECODE_SUPPORTED
#include "decode_jpeg_pipeline_adapter_xe2_lpm_base.h"
#endif

#ifdef _MPEG2_DECODE_SUPPORTED
#include "decode_mpeg2_pipeline_adapter_xe2_lpm_base.h"
#endif

#ifdef _VP9_DECODE_SUPPORTED
#include "decode_vp9_pipeline_adapter_xe2_lpm_base.h"
#endif

#ifdef _VP8_DECODE_SUPPORTED
#include "decode_vp8_pipeline_adapter_xe2_lpm_base.h"
#endif

#ifdef _AV1_DECODE_SUPPORTED
#include "decode_av1_pipeline_adapter_xe2_lpm_base.h"
#endif

#ifdef _VVC_DECODE_SUPPORTED
#include "decode_vvc_pipeline_adapter_xe2_lpm.h"
#endif

#ifdef _JPEG_ENCODE_SUPPORTED
#include "encode_jpeg_pipeline_adapter.h"
#endif

#ifdef _AVC_ENCODE_VDENC_SUPPORTED
#include "encode_avc_vdenc_pipeline_adapter_xe2_lpm.h"
#endif

#ifdef _HEVC_ENCODE_VDENC_SUPPORTED
#include "encode_hevc_vdenc_pipeline_adapter_xe2_lpm_base.h"
#endif

#ifdef _AV1_ENCODE_VDENC_SUPPORTED
#include "encode_av1_vdenc_pipeline_adapter_xe2_lpm.h"
#endif

#ifdef _MEDIA_RESERVED
#ifdef _VP9_ENCODE_VDENC_SUPPORTED
#include "encode_vp9_vdenc_pipeline_adapter_xe2_lpm.h"
#endif
#endif

#include "renderhal_xe2_hpg_next.h"
#include "decode_scalability_singlepipe_next.h"
#include "decode_scalability_multipipe_next.h"

#include "media_copy_xe2_lpm.h"

class MhwInterfacesLnl_Next : public MhwInterfacesNext
{
public:

    using Cp        = MhwCpInterface;
    using StateHeap = MHW_STATE_HEAP_INTERFACE_XE2_HPG;

    virtual MOS_STATUS Initialize(
        CreateParams   params,
        PMOS_INTERFACE osInterface);

    //!
    //! \brief    Destroys all created MHW interfaces
    //! \details  If the HAL creation fails, this is used for cleanup
    //!
    virtual void Destroy();
MEDIA_CLASS_DEFINE_END(MhwInterfacesLnl_Next)
};

class CodechalDecodeInterfacesXe2_Lpm
{
public:
#ifdef _AVC_DECODE_SUPPORTED
    using Avc = DecodeAvcPipelineAdapterXe2_Lpm_Base;
#endif
#ifdef _HEVC_DECODE_SUPPORTED
    using Hevc = DecodeHevcPipelineAdapterXe2_Lpm_Base;
#endif
#ifdef _JPEG_DECODE_SUPPORTED
    using Jpeg = DecodeJpegPipelineAdapterXe2_Lpm_Base;
#endif
#ifdef _MPEG2_DECODE_SUPPORTED
    using Mpeg2 = DecodeMpeg2PipelineAdapterXe2_Lpm_Base;
#endif
#ifdef _VP9_DECODE_SUPPORTED
    using Vp9 = DecodeVp9PipelineAdapterXe2_Lpm_Base;
#endif
#ifdef _VP8_DECODE_SUPPORTED
    using Vp8 = DecodeVp8PipelineAdapterXe2_Lpm_Base;
#endif
#ifdef _AV1_DECODE_SUPPORTED
    using Av1 = DecodeAv1PipelineAdapterXe2_Lpm_Base;
#endif
#ifdef _VVC_DECODE_SUPPORTED
    using Vvc = DecodeVvcPipelineAdapterXe2Lpm;
#endif

MEDIA_CLASS_DEFINE_END(CodechalDecodeInterfacesXe2_Lpm)
};

class CodechalInterfacesXe2_Lpm : public CodechalDeviceNext
{
public:
    using Decode = CodechalDecodeInterfacesXe2_Lpm;
    using Hw     = CodechalHwInterfaceXe2_Lpm_Base;

    MOS_STATUS Initialize(
        void *         standardInfo,
        void *         settings,
        MhwInterfacesNext *mhwInterfaces,
        PMOS_INTERFACE osInterface) override;

MEDIA_CLASS_DEFINE_END(CodechalInterfacesXe2_Lpm)
};

class VphalInterfacesXe2_Lpm : public VphalDevice
{
public:
    MOS_STATUS Initialize(
        PMOS_INTERFACE osInterface,
        bool           bInitVphalState,
        MOS_STATUS *   eStatus,
        bool           clearViewMode = false);

    MOS_STATUS CreateVpPlatformInterface(
        PMOS_INTERFACE           osInterface,
        MOS_STATUS *             eStatus);

private:
    void InitPlatformKernelBinary(
        vp::VpPlatformInterface  *&vpPlatformInterface);

MEDIA_CLASS_DEFINE_END(VphalInterfacesXe2_Lpm)
};

class RenderHalInterfacesXe2_Lpc : public RenderHalDevice
{
protected:
    using XRenderHal = XRenderHal_Interface_Xe2_Hpg_Next;
    MOS_STATUS Initialize();

MEDIA_CLASS_DEFINE_END(RenderHalInterfacesXe2_Lpc)
};

class MediaInterfacesHwInfoDeviceXe2_Lpm : public MediaInterfacesHwInfoDevice
{
public:
    virtual MOS_STATUS Initialize(PLATFORM platform) override;

MEDIA_CLASS_DEFINE_END(MediaInterfacesHwInfoDeviceXe2_Lpm)
};

using McpyDeviceXe2_Lpm = McpyDeviceNextImpl<MediaCopyStateXe2_Lpm>;

#endif  // __MEDIA_INTERFACES_XE2_LPM_H__
