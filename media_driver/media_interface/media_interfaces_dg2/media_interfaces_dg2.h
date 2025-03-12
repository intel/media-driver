/*===================== begin_copyright_notice ==================================

# Copyright (c) 2022, Intel Corporation

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
//! \file     media_interfaces_dg2.h
//! \brief    All interfaces used for DG2 that require factory creation
//!

#ifndef __MEDIA_INTERFACES_XE_HPM_H__
#define __MEDIA_INTERFACES_XE_HPM_H__

#include "media_interfaces_mhw.h"
#include "media_interfaces_mhw_next.h"
#include "media_interfaces_codechal.h"
#include "media_interfaces_codechal_next.h"
#include "media_interfaces_mmd.h"
#include "media_interfaces_mcpy.h"
#include "media_interfaces_cmhal.h"
#include "media_interfaces_vphal.h"
#include "media_interfaces_renderhal.h"
#include "media_interfaces_nv12top010.h"
#include "media_interfaces_decode_histogram.h"
#include "media_interfaces_hwinfo_device.h"

#include "mhw_cp_interface.h"
#include "mhw_mi_xe_xpm_base.h"
#include "mhw_render_xe_hpg.h"
#include "mhw_sfc_xe_xpm.h"
#include "mhw_state_heap_xe_xpm.h"
#include "mhw_vebox_xe_hpm.h"
#include "mhw_vdbox_mfx_xe_xpm.h"
#include "mhw_vdbox_hcp_xe_hpm.h"
#include "mhw_vdbox_avp_xe_hpm.h"
#include "mhw_vdbox_huc_xe_hpm.h"
#include "mhw_vdbox_avp_g12_X.h"
#ifdef IGFX_VDENC_INTERFACE_EXT_SUPPORT
#include "mhw_vdbox_vdenc_xe_hpm_ext.h"
#else
#include "mhw_vdbox_vdenc_xe_hpm.h"
#endif
#include "vphal_render_vebox_memdecomp_xe_xpm.h"
#include "media_copy_xe_hpm.h"
#include "mhw_blt_xe_hp_base.h"

#include "codechal_hw_xe_hpm.h"
#include "codechal_hw_next_xe_hpm.h"

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
#include "codechal_decode_vc1_xe_hpm.h"
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

#ifdef _HEVC_ENCODE_VDENC_SUPPORTED
#include "encode_hevc_vdenc_pipeline_adapter_xe_hpm.h"
#endif

#ifdef _AV1_ENCODE_VDENC_SUPPORTED
#include "encode_av1_vdenc_pipeline_adapter_xe_hpm.h"
#endif

#ifdef _AVC_ENCODE_VDENC_SUPPORTED
#include "codechal_vdenc_avc_xe_hpm.h"
#endif

#ifdef _VP9_ENCODE_VDENC_SUPPORTED
#include "codechal_vdenc_vp9_g12.h"
#include "codechal_vdenc_vp9_xe_hpm.h"
#endif

#ifdef _JPEG_ENCODE_SUPPORTED
#include "codechal_encode_jpeg_g12.h"
#endif

#ifdef _MEDIA_RESERVED
#ifdef _AVC_ENCODE_VDENC_SUPPORTED
#include "codechal_vdenc_avc_xe_hpm_ext.h"
#endif

#ifdef _VP9_ENCODE_VDENC_SUPPORTED
#ifdef _APOGEIOS_SUPPORTED
#include "encode_vp9_vdenc_pipeline_adapter_xe_hpm.h"
#endif
#endif


#include "cm_hal_xe_xpm.h"

#endif // _MEDIA_RESERVED

#include "vphal_xe_xpm.h"
#include "vphal_xe_hpm.h"
#include "renderhal_xe_hpg.h"

#include "codechal_decode_histogram.h"
#include "codechal_decode_histogram_g12.h"
#include "decode_scalability_singlepipe.h"
#include "decode_scalability_multipipe.h"

class MhwInterfacesDg2_Next : public MhwInterfacesNext
{
public:
    // Need to remove legacy mhw sub interfaces
    using Mi        = MhwMiInterfaceXe_Xpm_Base;
    using Cp        = MhwCpInterface;
    using Render    = MhwRenderInterfaceXe_Hpg;
    using Sfc       = MhwSfcInterfaceXe_Xpm;
    using StateHeap = MHW_STATE_HEAP_INTERFACE_XE_XPM;
    using Mfx       = MhwVdboxMfxInterfaceXe_Xpm;
    using Hcp       = MhwVdboxHcpInterfaceXe_Hpm;
    using Huc       = MhwVdboxHucInterfaceXe_Hpm;
#ifdef IGFX_VDENC_INTERFACE_EXT_SUPPORT
    using Vdenc     = MhwVdboxVdencInterfaceXe_HpmExt;
#else
    using Vdenc     = MhwVdboxVdencInterfaceXe_Hpm;
#endif
    using Vebox     = MhwVeboxInterfaceXe_Hpm;
    using Blt       = MhwBltInterfaceXe_Hp_Base;

    virtual MOS_STATUS Initialize(
        CreateParams params,
        PMOS_INTERFACE osInterface);

    //!
    //! \brief    Destroys all created MHW interfaces
    //! \details  If the HAL creation fails, this is used for cleanup
    //!
    virtual void Destroy();

    std::shared_ptr<MhwMiInterface> m_miInterface = nullptr;

    MhwRenderInterface *m_renderInterface = nullptr;
};

class MhwInterfacesDg2 : public MhwInterfaces
{
public:
    MhwVdboxAvpInterface *m_avpInterface = nullptr;

    using Mi        = MhwMiInterfaceXe_Xpm_Base;
    using Cp        = MhwCpInterface;
    using Render    = MhwRenderInterfaceXe_Hpg;
    using Sfc       = MhwSfcInterfaceXe_Xpm;
    using StateHeap = MHW_STATE_HEAP_INTERFACE_XE_XPM;
    using Mfx       = MhwVdboxMfxInterfaceXe_Xpm;
    using Hcp       = MhwVdboxHcpInterfaceXe_Hpm;
    using Avp       = MhwVdboxAvpInterfaceXe_Hpm;
    using Huc       = MhwVdboxHucInterfaceXe_Hpm;
#ifdef IGFX_VDENC_INTERFACE_EXT_SUPPORT
    using Vdenc     = MhwVdboxVdencInterfaceXe_HpmExt;
#else
    using Vdenc     = MhwVdboxVdencInterfaceXe_Hpm;
#endif
    using Vebox     = MhwVeboxInterfaceXe_Hpm;
    using Blt       = MhwBltInterfaceXe_Hp_Base;

    virtual MOS_STATUS Initialize(
        CreateParams params,
        PMOS_INTERFACE osInterface);

};

class MmdDeviceXe_Hpm : public MmdDevice
{
public:
    using Mmd  = MediaVeboxDecompStateXe_Xpm;

    MOS_STATUS Initialize(
        PMOS_INTERFACE osInterface,
        MhwInterfaces *mhwInterfaces);

    MhwInterfaces* CreateMhwInterface(
        PMOS_INTERFACE osInterface);
};

class McpyDeviceXe_Hpm : public McpyDevice
{
public:
    using Mcpy  = MediaCopyState_Xe_Hpm;

    MOS_STATUS Initialize(
        PMOS_INTERFACE osInterface);
protected:
    MhwInterfaces* CreateMhwInterface(
        PMOS_INTERFACE osInterface);
};

class Nv12ToP010DeviceXe_Hpm : public Nv12ToP010Device
{
public:

    MOS_STATUS Initialize(
        PMOS_INTERFACE osInterface);
};

class CodechalDecodeInterfacesXe_Hpm
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
    using Vc1 = CodechalDecodeVc1Xe_Hpm;
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

class CodechalEncodeInterfacesXe_Hpm
{
public:
#ifdef _MEDIA_RESERVED
#ifdef _VP9_ENCODE_VDENC_SUPPORTED
    using Vp9 = CodechalVdencVp9StateXe_Xpm;
#endif
#ifdef _AVC_ENCODE_VDENC_SUPPORTED
    using AvcVdenc = CodechalVdencAvcStateXe_HpmExt;
#endif
#else
#ifdef _VP9_ENCODE_VDENC_SUPPORTED
    using Vp9 = CodechalVdencVp9StateXe_Xpm;
#endif
#ifdef _AVC_ENCODE_VDENC_SUPPORTED
    using AvcVdenc = CodechalVdencAvcStateXe_Hpm;
#endif
#endif

#ifdef _JPEG_ENCODE_SUPPORTED
    using Jpeg = CodechalEncodeJpegStateG12;
#endif
#ifdef _HEVC_ENCODE_VDENC_SUPPORTED
    using HevcVdenc = EncodeHevcVdencPipelineAdapterXe_Hpm;
#endif
#ifdef _AV1_ENCODE_VDENC_SUPPORTED
    using Av1Vdenc = EncodeAv1VdencPipelineAdapterXe_Hpm;
#endif
};

class CodechalInterfacesXe_Hpm : public CodechalDevice
{
public:
    using Decode = CodechalDecodeInterfacesXe_Hpm;
    using Encode = CodechalEncodeInterfacesXe_Hpm;
    using Hw = CodechalHwInterfaceXe_Hpm;

    virtual MOS_STATUS Initialize(
        void *standardInfo,
        void *settings,
        MhwInterfaces *mhwInterfaces,
        PMOS_INTERFACE osInterface) override;

protected:
    virtual MOS_STATUS CreateCodecHalInterface(MhwInterfaces  *mhwInterfaces,
                                       CodechalHwInterface    *&pHwInterface,
                                       CodechalDebugInterface *&pDebugInterface,
                                       PMOS_INTERFACE         osInterface,
                                       CODECHAL_FUNCTION      CodecFunction,
                                       bool                   disableScalability);

    virtual MOS_STATUS CreateCodecHalInterface(MhwInterfaces *mhwInterfaces,
        MhwInterfacesNext                                   *&pMhwInterfacesNext,
        CodechalHwInterfaceNext                                 *&pHwInterface,
        CodechalDebugInterface                              *&pDebugInterface,
        PMOS_INTERFACE                                        osInterface,
        CODECHAL_FUNCTION                                     CodecFunction,
        bool                                                  disableScalability);
};
class CodechalInterfacesNextXe_Hpm : public CodechalDeviceNext
{
public:
    using Decode = CodechalDecodeInterfacesXe_Hpm;
    using Encode = CodechalEncodeInterfacesXe_Hpm;
    using Hw     = CodechalHwInterfaceNextXe_Hpm;

    virtual MOS_STATUS Initialize(
        void          *standardInfo,
        void          *settings,
        MhwInterfacesNext *mhwInterfaces,
        PMOS_INTERFACE osInterface) override;
};

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

#ifdef _MEDIA_RESERVED
class CMHalInterfacesXe_Hpm : public CMHalDevice
{
protected:
    using CMHal = CmHalXe_Xpm;
    MOS_STATUS Initialize(
        CM_HAL_STATE *pCmState);
};
#endif

class VphalInterfacesXe_Hpm : public VphalDevice
{
public:
    using VphalState = VphalStateXe_Hpm;

    MOS_STATUS Initialize(
        PMOS_INTERFACE  osInterface,
        bool            bInitVphalState,
        MOS_STATUS      *eStatus,
        bool            clearViewMode = false);
        
    MOS_STATUS CreateVpPlatformInterface(
        PMOS_INTERFACE           osInterface,
        MOS_STATUS *             eStatus);

private:
    void InitPlatformKernelBinary(
        vp::VpPlatformInterface  *&vpPlatformInterface);
};

class RenderHalInterfacesXe_Hpg : public RenderHalDevice
{
protected:
    using XRenderHal = XRenderHal_Interface_Xe_Hpg;
    MOS_STATUS Initialize();
};

class DecodeHistogramDeviceXe_Hpm : public DecodeHistogramDevice
{
public:
    using DecodeHistogramG12 = CodechalDecodeHistogramG12;

    MOS_STATUS Initialize(
        CodechalHwInterface *hwInterface,
        PMOS_INTERFACE osInterface);
};

class MediaInterfacesHwInfoDeviceDg2 : public MediaInterfacesHwInfoDevice
{
public:
    virtual MOS_STATUS RefreshRevId(PLATFORM &platform, MEDIA_WA_TABLE *waTable) override;
    virtual MOS_STATUS Initialize(PLATFORM platform) override;
};

#endif // __MEDIA_INTERFACES_XE_HPM_H__
