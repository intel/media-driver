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
//! \file     media_interfaces_pvc.h
//! \brief    All interfaces used for pvc that require factory creation
//!

#ifndef __MEDIA_INTERFACES_PVC_H__
#define __MEDIA_INTERFACES_PVC_H__

#include "media_interfaces_mhw.h"
#include "media_interfaces_mhw_next.h"
#include "media_interfaces_codechal.h"
#include "media_interfaces_mmd.h"
#include "media_interfaces_mcpy.h"
#include "media_interfaces_cmhal.h"
#include "media_interfaces_vphal.h"
#include "media_interfaces_renderhal.h"
#include "media_interfaces_nv12top010.h"
#include "media_interfaces_decode_histogram.h"

#include "mhw_cp_interface.h"
#include "mhw_mi_xe_xpm_base.h"
#include "mhw_render_xe_hpc.h"
#include "mhw_render_xe_hpg.h"
#include "mhw_sfc_xe_xpm.h"
#include "mhw_state_heap_xe_xpm.h"
#include "mhw_vebox_xe_xpm.h"
#include "mhw_vdbox_mfx_xe_xpm.h"
#include "mhw_vdbox_avp_xe_xpm.h"
#include "mhw_vdbox_hcp_xe_xpm_plus.h"
#include "mhw_vdbox_huc_xe_xpm_plus.h"
#include "mhw_vdbox_avp_g12_X.h"
#ifdef IGFX_PVC_ENABLE_NON_UPSTREAM
#include "mhw_vdbox_vdenc_xe_xpm_plus.h"
#include "mhw_vdbox_vdenc_xe_xpm_plus_ext.h"
#else
#include "mhw_vdbox_vdenc_g12_X.h"
#endif
#include "renderhal_memdecomp.h"
#include "mhw_blt_xe_hpc.h"
#include "vphal_xe_xpm_plus.h"
#include "media_copy_xe_xpm_plus.h"
#include "codechal_hw_xe_xpm_plus.h"

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

#ifdef IGFX_PVC_ENABLE_NON_UPSTREAM
#ifdef _JPEG_ENCODE_SUPPORTED
#include "codechal_encode_jpeg_g12.h"
#endif

#include "codechal_encode_csc_ds_xe_xpm.h"
#include "codechal_encode_csc_ds_mdf_xe_xpm.h"

#ifdef _HEVC_ENCODE_VME_SUPPORTED
#include "codechal_encode_hevc_g12.h"
#include "codechal_encode_hevc_mbenc_xe_xpm.h"
#include "codechal_encode_csc_ds_mdf_g12.h"
#endif
#ifdef _HEVC_ENCODE_VDENC_SUPPORTED
#include "codechal_vdenc_hevc_xe_xpm.h"
#ifdef _APOGEIOS_SUPPORTED
#include "encode_hevc_vdenc_pipeline_adapter_xe_xpm_plus.h"
#endif
#endif

#ifdef _AVC_ENCODE_VDENC_SUPPORTED
#include "codechal_vdenc_avc_xe_xpm_plus.h"
#endif


#ifdef _VP9_ENCODE_VDENC_SUPPORTED
#include "codechal_vdenc_vp9_g12.h"
#include "codechal_vdenc_vp9_xe_hpm.h"
#endif

#include "codechal_encoder_unsupported.h"
#endif
#ifdef IGFX_PVC_ENABLE_NON_UPSTREAM
#include "cm_hal_xe_xpm.h"
#endif
#include "vphal_xe_xpm.h"
#include "renderhal_xe_hpc.h"

#include "codechal_decode_histogram.h"
#include "decode_scalability_singlepipe.h"
#include "decode_scalability_multipipe.h"

class MhwInterfacesPvc : public MhwInterfaces
{
public:
    MhwVdboxAvpInterface *m_avpInterface = nullptr;

    using Mi        = MhwMiInterfaceXe_Xpm_Base;
    using Cp        = MhwCpInterface;
    using Render    = MhwRenderInterfaceXe_Hpc;
    using Sfc       = MhwSfcInterfaceXe_Xpm;
    using StateHeap = MHW_STATE_HEAP_INTERFACE_XE_XPM;
    using Mfx       = MhwVdboxMfxInterfaceXe_Xpm;
    using Hcp       = MhwVdboxHcpInterfaceXe_Xpm_PLUS;
    using Avp       = MhwVdboxAvpInterfaceXe_Xpm;
    using Huc       = MhwVdboxHucInterfaceXe_Xpm_Plus;
#ifdef IGFX_PVC_ENABLE_NON_UPSTREAM
    using Vdenc     = MhwVdboxVdencInterfaceG12ExtPvc;
#else
    using Vdenc     = MhwVdboxVdencInterfaceG12X;
#endif
    using Vebox     = MhwVeboxInterfaceXe_Xpm;
    using Blt       = MhwBltInterfaceXe_Hpc;

    MOS_STATUS Initialize(
        CreateParams params,
        PMOS_INTERFACE osInterface);

};

class MhwInterfacesPvc_Next : public MhwInterfacesNext
{
public:
    using Mi        = MhwMiInterfaceXe_Xpm_Base;
    using Cp        = MhwCpInterface;
    using Render    = MhwRenderInterfaceXe_Hpc;
    using Sfc       = MhwSfcInterfaceXe_Xpm;
    using StateHeap = MHW_STATE_HEAP_INTERFACE_XE_XPM;
    using Mfx       = MhwVdboxMfxInterfaceXe_Xpm;
    using Hcp       = MhwVdboxHcpInterfaceXe_Xpm_PLUS;
    using Huc       = MhwVdboxHucInterfaceXe_Xpm_Plus;
#ifdef IGFX_PVC_ENABLE_NON_UPSTREAM
    using Vdenc     = MhwVdboxVdencInterfaceG12ExtPvc;
#else
    using Vdenc     = MhwVdboxVdencInterfaceG12X;
#endif
    using Vebox     = MhwVeboxInterfaceXe_Xpm;
    using Blt       = MhwBltInterfaceXe_Hpc;

    MOS_STATUS Initialize(
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

class MmdDeviceXe_Xpm_Plus : public MmdDevice
{
public:
    using Mmd = MediaRenderDecompState;
    MOS_STATUS Initialize(
        PMOS_INTERFACE osInterface,
        MhwInterfaces *mhwInterfaces);

    MhwInterfaces* CreateMhwInterface(
        PMOS_INTERFACE osInterface);
};

class McpyDeviceXe_Xpm_Plus : public McpyDevice
{
public:
    using Mcpy = MediaCopyStateXe_Xpm_Plus;
    MOS_STATUS Initialize(
        PMOS_INTERFACE osInterface);
protected:
    MhwInterfaces* CreateMhwInterface(
        PMOS_INTERFACE osInterface);
};

class Nv12ToP010DeviceXe_Xpm_Plus : public Nv12ToP010Device
{
public:

    MOS_STATUS Initialize(
        PMOS_INTERFACE osInterface);
};

class CodechalDecodeInterfacesXe_Xpm_Plus
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

#ifdef IGFX_PVC_ENABLE_NON_UPSTREAM
class CodechalEncodeInterfacesXe_Xpm_Plus
{
public:
    using CscDs = CodechalEncodeCscDsXe_Xpm;
    using CscDsMdf = CodechalEncodeCscDsMdfXe_Xpm;
#ifdef _VP9_ENCODE_VDENC_SUPPORTED
    using Vp9 = CodechalVdencVp9StateXe_Xpm;
#endif
#ifdef _JPEG_ENCODE_SUPPORTED
    using Jpeg = CodechalEncodeJpegStateG12;
#endif
#ifdef _HEVC_ENCODE_VME_SUPPORTED
    using HevcEnc = CodechalEncHevcStateG12;
    using HevcMbenc = CodecHalHevcMbencXe_Xpm;
#endif
#ifdef _HEVC_ENCODE_VDENC_SUPPORTED
    using HevcVdenc = CodechalVdencHevcStateXe_Xpm;
#endif
#ifdef _AVC_ENCODE_VDENC_SUPPORTED
    using AvcVdenc = CodechalVdencAvcStateXe_Xpm_Plus;
#endif
};
#endif

class CodechalInterfacesXe_Xpm_Plus : public CodechalDevice
{
public:
    using Decode = CodechalDecodeInterfacesXe_Xpm_Plus;
#ifdef IGFX_PVC_ENABLE_NON_UPSTREAM
    using Encode = CodechalEncodeInterfacesXe_Xpm_Plus;
#endif
    using Hw = CodechalHwInterfaceXe_Xpm_Plus;

    MOS_STATUS Initialize(
        void *standardInfo,
        void *settings,
        MhwInterfaces *mhwInterfaces,
        PMOS_INTERFACE osInterface) override;

private:
    MOS_STATUS CreateCodecHalInterface(MhwInterfaces          *mhwInterfaces,
                                       CodechalHwInterface    *&pHwInterface,
                                       CodechalDebugInterface *&pDebugInterface,
                                       PMOS_INTERFACE         osInterface,
                                       CODECHAL_FUNCTION      CodecFunction,
                                       bool                   disableScalability);

    MOS_STATUS CreateCodecHalInterface(MhwInterfaces *mhwInterfaces,
        MhwInterfacesNext                           *&pMhwInterfacesNext,
        CodechalHwInterfaceNext                     *&pHwInterface,
        CodechalDebugInterface                      *&pDebugInterface,
        PMOS_INTERFACE                              osInterface,
        CODECHAL_FUNCTION                           CodecFunction,
        bool                                        disableScalability);
};

#define PVC_L3_CONFIG_COUNT     6
// 4KB per Way for Pvc, two Way per section
static const L3ConfigRegisterValues PVC_L3_PLANES[PVC_L3_CONFIG_COUNT] =
{                                    //  Rest  R/W  RO   UTC  CB  Sum (in KB)
    {0x00000200, 0, 0, 0},           //  512   0    0    0    0   512
    {0xC0000000, 0x40000000, 0, 0},  //  384   0    0    128  0   512
    {0xF0000000, 0x00000080, 0, 0},  //  480   0    0    0    32  512
    {0x80000000, 0x80000000, 0, 0},  //  256   0    0    256  0   512
    {0x40000000, 0x00000080, 0, 0},  //  0     128  352  0    32  512
    {0x80000000, 0x70000080, 0, 0},  //  256   0    0    224  32  512
};

#ifdef IGFX_PVC_ENABLE_NON_UPSTREAM
class CMHalInterfacesXe_Xpm_Plus : public CMHalDevice
{
protected:
    using CMHal = CmHalXe_Xpm;
    MOS_STATUS Initialize(
        CM_HAL_STATE *pCmState);
};
#endif

class VphalInterfacesXe_Xpm_Plus : public VphalDevice
{
public:
    using VphalState = VphalStateXe_Xpm_Plus;

    MOS_STATUS Initialize(
        PMOS_INTERFACE  osInterface,
        bool            bInitVphalState,
        MOS_STATUS      *eStatus,
        bool            clearViewMode = false);
};

class RenderHalInterfacesXe_Hpc : public RenderHalDevice
{
protected:
    using XRenderHal = XRenderHal_Interface_Xe_Hpc;
    MOS_STATUS Initialize();
};

class DecodeHistogramDeviceXe_Xpm_Plus : public DecodeHistogramDevice
{
public:
    using DecodeHistogramG12 = CodechalDecodeHistogram;

    MOS_STATUS Initialize(
        CodechalHwInterface *hwInterface,
        PMOS_INTERFACE osInterface);
};

#endif // __MEDIA_INTERFACES_PVC_H__
