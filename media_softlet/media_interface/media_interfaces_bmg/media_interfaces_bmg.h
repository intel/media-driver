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
//! \file     media_interfaces_bmg.h
//! \brief    All interfaces used for BMG that require factory creation
//!

#ifndef __MEDIA_INTERFACES_XE2_HPM_H__
#define __MEDIA_INTERFACES_XE2_HPM_H__

#include "media_interfaces_mhw_next.h"
#include "media_interfaces_codechal_next.h"
#include "media_interfaces_mcpy_next.h"
#include "media_interfaces_vphal.h"
#include "media_interfaces_renderhal.h"
#include "media_interfaces_hwinfo_device.h"

#include "mhw_cp_interface.h"
#include "mhw_state_heap_xe2_hpg.h"
#include "mhw_vdbox_mfx_impl_xe2_hpm.h"
#include "mhw_vdbox_avp_impl_xe2_hpm.h"
#include "mhw_vdbox_huc_impl_xe2_hpm.h"
#include "mhw_vdbox_hcp_impl_xe2_hpm.h"

#include "mhw_vdbox_vdenc_impl_xe2_hpm.h"

#include "codec_hw_xe_lpm_plus_base.h"

#ifdef _AVC_DECODE_SUPPORTED
#include "decode_avc_pipeline_adapter_xe2_hpm.h"
#endif

#ifdef _HEVC_DECODE_SUPPORTED
#include "decode_hevc_pipeline_adapter_xe2_hpm.h"
#endif

#ifdef _JPEG_DECODE_SUPPORTED
#include "decode_jpeg_pipeline_adapter_xe2_hpm.h"
#endif

#ifdef _MPEG2_DECODE_SUPPORTED
#include "decode_mpeg2_pipeline_adapter_xe2_hpm.h"
#endif

#ifdef _VP9_DECODE_SUPPORTED
#include "decode_vp9_pipeline_adapter_xe2_hpm.h"
#endif

#ifdef _AV1_DECODE_SUPPORTED
#include "decode_av1_pipeline_adapter_xe2_hpm.h"
#endif

#ifdef _VP8_DECODE_SUPPORTED
#include "decode_vp8_pipeline_adapter_xe2_hpm.h"
#endif

#ifdef _JPEG_ENCODE_SUPPORTED
#include "encode_jpeg_pipeline_adapter.h"
#endif

#ifdef _HEVC_ENCODE_VDENC_SUPPORTED
#include "encode_hevc_vdenc_pipeline_adapter_xe2_hpm.h"
#endif

#ifdef _AVC_ENCODE_VDENC_SUPPORTED
#include "encode_avc_vdenc_pipeline_adapter_xe2_hpm.h"
#endif

#ifdef _AV1_ENCODE_VDENC_SUPPORTED
#include "encode_av1_vdenc_pipeline_adapter_xe2_hpm.h"
#endif

#ifdef _MEDIA_RESERVED
#ifdef _VP9_ENCODE_VDENC_SUPPORTED
#include "encode_vp9_vdenc_pipeline_adapter_xe2_hpm.h"
#endif
#endif

#include "renderhal_xe2_hpg_next.h"
#include "decode_scalability_singlepipe_next.h"
#include "decode_scalability_multipipe_next.h"

#include "media_copy_xe2_hpm_base.h"

class MhwInterfacesBmg_Next : public MhwInterfacesNext
{
public:

    //TODO, Remove legacy mhw sub interfaces
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

MEDIA_CLASS_DEFINE_END(MhwInterfacesBmg_Next)
};

class CodechalDecodeInterfacesXe2_Hpm
{
public:
#ifdef _AVC_DECODE_SUPPORTED
    using Avc = DecodeAvcPipelineAdapterXe2_Hpm;
#endif
#ifdef _HEVC_DECODE_SUPPORTED
    using Hevc = DecodeHevcPipelineAdapterXe2_Hpm;
#endif
#ifdef _JPEG_DECODE_SUPPORTED
    using Jpeg = DecodeJpegPipelineAdapterXe2_Hpm;
#endif
#ifdef _MPEG2_DECODE_SUPPORTED
    using Mpeg2 = DecodeMpeg2PipelineAdapterXe2_Hpm;
#endif
#ifdef _VP9_DECODE_SUPPORTED
    using Vp9 = DecodeVp9PipelineAdapterXe2_Hpm;
#endif
#ifdef _AV1_DECODE_SUPPORTED
    using Av1 = DecodeAv1PipelineAdapterXe2_Hpm;
#endif
#ifdef _VP8_DECODE_SUPPORTED
    using Vp8 = DecodeVp8PipelineAdapterXe2_Hpm;
#endif
    MEDIA_CLASS_DEFINE_END(CodechalDecodeInterfacesXe2_Hpm)
};

class CodechalInterfacesXe2_Hpm : public CodechalDeviceNext
{
public:
    using Decode = CodechalDecodeInterfacesXe2_Hpm;
    using Hw     = CodechalHwInterfaceXe_Lpm_Plus_Base;

    MOS_STATUS Initialize(
        void *         standardInfo,
        void *         settings,
        MhwInterfacesNext *mhwInterfaces,
        PMOS_INTERFACE osInterface) override;

MEDIA_CLASS_DEFINE_END(CodechalInterfacesXe2_Hpm)
};

/*
#define DG2_L3_CONFIG_COUNT     6
// 4KB per Way for DG2, two Way per section
static const L3ConfigRegisterValues DG2_L3_PLANES[DG2_L3_CONFIG_COUNT] =
{                                    //  Rest  R/W  RO   UTC  CB  Sum (in KB)
    {0x00000200, 0, 0, 0},           //  512   0    0    0    0   512
    {0xC0000000, 0x40000000, 0, 0},  //  384   0    0    128  0   512
    {0xF0000000, 0x00000080, 0, 0},  //  480   0    0    0    32  512
    {0x80000000, 0x80000000, 0, 0},  //  256   0    0    256  0   512
    {0x40000000, 0x00000080, 0, 0},  //  0     128  352  0    32  512
    {0x80000000, 0x70000080, 0, 0},  //  256   0    0    224  32  512
};
*/

class VphalInterfacesXe2_Hpm : public VphalDevice
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

MEDIA_CLASS_DEFINE_END(VphalInterfacesXe2_Hpm)
};

class RenderHalInterfacesXe2_Hpg : public RenderHalDevice
{
protected:
    using XRenderHal = XRenderHal_Interface_Xe2_Hpg_Next;
    MOS_STATUS Initialize();

MEDIA_CLASS_DEFINE_END(RenderHalInterfacesXe2_Hpg)
};

class MediaInterfacesHwInfoDeviceXe2_Hpm : public MediaInterfacesHwInfoDevice
{
public:
    virtual MOS_STATUS Initialize(PLATFORM platform) override;

MEDIA_CLASS_DEFINE_END(MediaInterfacesHwInfoDeviceXe2_Hpm)
};

using McpyDeviceXe2_Hpm = McpyDeviceNextImpl<MediaCopyStateXe2_Hpm_Base>;

#endif  // __MEDIA_INTERFACES_XE2_HPM_H__
