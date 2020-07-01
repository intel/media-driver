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
//! \file     media_interfaces_g8_bdw.h
//! \brief    All interfaces used for BDW that require factory creation
//!

#ifndef __MEDIA_INTERFACES_G8_BDW_H__
#define __MEDIA_INTERFACES_G8_BDW_H__

#include "media_interfaces_mhw.h"
#include "media_interfaces_codechal.h"
#include "media_interfaces_mmd.h"
#include "media_interfaces_cmhal.h"
#include "media_interfaces_mosutil.h"
#include "media_interfaces_vphal.h"
#include "media_interfaces_renderhal.h"

#include "mhw_cp_interface.h"
#include "mhw_mi_g8_X.h"
#include "mhw_render_g8_X.h"
#include "mhw_sfc_g9_X.h"
#include "mhw_state_heap_g8.h"
#include "mhw_vebox_g8_X.h"
#include "mhw_vdbox_mfx_g8_bdw.h"

#include "codechal_hw_g8_bdw.h"

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
#include "codechal_decode_vc1_g8.h"
#endif

#ifdef _VP8_DECODE_SUPPORTED
#include "codechal_decode_vp8.h"
#endif

#ifdef _VP9_DECODE_SUPPORTED
#include "codechal_decode_vp9.h"
#endif

#ifdef _JPEG_ENCODE_SUPPORTED
#include "codechal_encode_jpeg.h"
#endif

#ifdef _MPEG2_ENCODE_VME_SUPPORTED
#include "codechal_encode_mpeg2_g8.h"
#endif
#include "codechal_encode_csc_ds_g8.h"
#ifdef _AVC_ENCODE_VME_SUPPORTED
#include "codechal_encode_avc_g8.h"
#include "codechal_fei_avc_g8.h"
#endif
#include "cm_hal_g8.h"
#include "vphal_g8.h"

#include "renderhal_g8_bdw.h"

class MhwInterfacesG8Bdw : public MhwInterfaces
{
public:
    using Mi = MhwMiInterfaceG8;
    using Cp = MhwCpInterface;
    using Render = MhwRenderInterfaceG8;
    using StateHeap = MHW_STATE_HEAP_INTERFACE_G8_X;
    using Mfx = MhwVdboxMfxInterfaceG8Bdw;
    using Vebox = MhwVeboxInterfaceG8;

    MOS_STATUS Initialize(
        CreateParams params,
        PMOS_INTERFACE osInterface);
};

class CodechalDecodeInterfacesG8Bdw
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
    using Vc1 = CodechalDecodeVc1G8;
#endif
#ifdef _VP8_DECODE_SUPPORTED
    using Vp8 = CodechalDecodeVp8;
#endif
#ifdef _VP9_DECODE_SUPPORTED
    using Vp9 = CodechalDecodeVp9;
#endif
};

class CodechalEncodeInterfacesG8Bdw
{
public:
#ifdef _JPEG_ENCODE_SUPPORTED
    using Jpeg = CodechalEncodeJpegState;
#endif
#ifdef _MPEG2_ENCODE_VME_SUPPORTED
    using Mpeg2 = CodechalEncodeMpeg2G8;
#endif
    using CscDs = CodechalEncodeCscDsG8;
#ifdef _AVC_ENCODE_VME_SUPPORTED
    using AvcEnc = CodechalEncodeAvcEncG8;
    using AvcFei = CodechalEncodeAvcEncFeiG8;
#endif
};

class CodechalInterfacesG8Bdw : public CodechalDevice
{
public:
    using Decode = CodechalDecodeInterfacesG8Bdw;
    using Encode = CodechalEncodeInterfacesG8Bdw;
    using Hw = CodechalHwInterfaceG8Bdw;

    MOS_STATUS Initialize(
        void *standardInfo,
        void *settings,
        MhwInterfaces *mhwInterfaces,
        PMOS_INTERFACE osInterface) override;
};

class CMHalInterfacesG8Bdw : public CMHalDevice
{
protected:
    using CMHal = CM_HAL_G8_X;
    MOS_STATUS Initialize(
        CM_HAL_STATE *pCmState);
};

class MosUtilDeviceG8Bdw : public MosUtilDevice
{
public:
    using MosUtil = MediaUserSettingsMgr;

    MOS_STATUS Initialize();
};

class VphalInterfacesG8Bdw : public VphalDevice
{
public:
    using VphalState = VphalStateG8;

    MOS_STATUS Initialize(
        PMOS_INTERFACE  osInterface,
        PMOS_CONTEXT    osDriverContext,
        MOS_STATUS      *eStatus);
};

class RenderHalInterfacesG8Bdw : public RenderHalDevice
{
protected:
    using XRenderHal = XRenderHal_Interface_g8_bdw;
    MOS_STATUS Initialize();
};

#endif // __MEDIA_INTERFACES_G8_BDW_H__
