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
#ifndef DDI_REGISTER_COMPONENTS_SPECIFIC_H
#define DDI_REGISTER_COMPONENTS_SPECIFIC_H

#include "media_factory.h"
#include "ddi_encode_base_specific.h"
#include "media_capstable_specific.h" //Include for component info Comparison
#include "ddi_decode_base_specific.h"

typedef MediaFactory<ComponentInfo, encode::DdiEncodeBase> DdiEncodeFactory;
typedef MediaFactory<ComponentInfo, decode::DdiDecodeBase> DdiDecodeFactory;

#if _MEDIA_RESERVED
#include "ddi_register_components_specific_reserved.h"
#endif

#if defined (_HEVC_ENCODE_VDENC_SUPPORTED)
#include "ddi_encode_hevc_specific.h"

static bool RegisteredHevcMain =
    DdiEncodeFactory::
        Register<encode::DdiEncodeHevc>(ComponentInfo {VAProfileHEVCMain, VAEntrypointEncSlice});
static bool RegisteredHevcMain10 =
    DdiEncodeFactory::
        Register<encode::DdiEncodeHevc>(ComponentInfo{VAProfileHEVCMain10, VAEntrypointEncSlice});
static bool RegisteredHevcMain422_10 =
    DdiEncodeFactory::
        Register<encode::DdiEncodeHevc>(ComponentInfo{VAProfileHEVCMain422_10, VAEntrypointEncSlice});
static bool RegisteredHevcMain444 =
    DdiEncodeFactory::
        Register<encode::DdiEncodeHevc>(ComponentInfo{VAProfileHEVCMain444, VAEntrypointEncSlice});
static bool RegisteredHevcMain444_10 =
    DdiEncodeFactory::
        Register<encode::DdiEncodeHevc>(ComponentInfo{VAProfileHEVCMain444_10, VAEntrypointEncSlice});
static bool RegisteredHevcSCCMain =
    DdiEncodeFactory::
        Register<encode::DdiEncodeHevc>(ComponentInfo{VAProfileHEVCSccMain, VAEntrypointEncSlice});
static bool RegisteredHevcSCCMain10 =
    DdiEncodeFactory::
        Register<encode::DdiEncodeHevc>(ComponentInfo{VAProfileHEVCSccMain10, VAEntrypointEncSlice});
static bool RegisteredHevcSCCMain444 =
    DdiEncodeFactory::
        Register<encode::DdiEncodeHevc>(ComponentInfo{VAProfileHEVCSccMain444, VAEntrypointEncSlice});
static bool RegisteredHevcSCCMain444_10 =
    DdiEncodeFactory::
        Register<encode::DdiEncodeHevc>(ComponentInfo{VAProfileHEVCSccMain444_10, VAEntrypointEncSlice});
#endif // _HEVC_ENCODE_VDENC_SUPPORTED

#if defined (_AV1_ENCODE_VDENC_SUPPORTED)
#include "ddi_encode_av1_specific.h"
static bool RegisteredAv1Profile0 =
    DdiEncodeFactory::
        Register<encode::DdiEncodeAV1>(ComponentInfo {VAProfileAV1Profile0, VAEntrypointEncSlice});
#endif // _AV1_ENCODE_VDENC_SUPPORTED

#if defined (_VP9_ENCODE_VDENC_SUPPORTED)
#include "ddi_encode_vp9_specific.h"
static bool RegisteredVp9Profile0 =
    DdiEncodeFactory::
        Register<encode::DdiEncodeVp9>(ComponentInfo {VAProfileVP9Profile0, VAEntrypointEncSlice});
static bool RegisteredVp9Profile1 =
    DdiEncodeFactory::
        Register<encode::DdiEncodeVp9>(ComponentInfo {VAProfileVP9Profile1, VAEntrypointEncSlice});
static bool RegisteredVp9Profile2 =
    DdiEncodeFactory::
        Register<encode::DdiEncodeVp9>(ComponentInfo {VAProfileVP9Profile2, VAEntrypointEncSlice});
static bool RegisteredVp9Profile3 =
    DdiEncodeFactory::
        Register<encode::DdiEncodeVp9>(ComponentInfo {VAProfileVP9Profile3, VAEntrypointEncSlice});
#endif // _VP9_ENCODE_VDENC_SUPPORTED

#if defined (_AVC_ENCODE_VDENC_SUPPORTED)
#include "ddi_encode_avc_specific.h"

static bool RegisteredH264Main =
    DdiEncodeFactory::
        Register<encode::DdiEncodeAvc>(ComponentInfo {VAProfileH264Main, VAEntrypointEncSlice});
static bool RegisteredH264High =
    DdiEncodeFactory::
        Register<encode::DdiEncodeAvc>(ComponentInfo{VAProfileH264High, VAEntrypointEncSlice});
static bool RegisteredH264ConstrainedBaseline =
    DdiEncodeFactory::
        Register<encode::DdiEncodeAvc>(ComponentInfo{VAProfileH264ConstrainedBaseline, VAEntrypointEncSlice});
#endif // _AVC_ENCODE_VDENC_SUPPORTED

#if defined (_JPEG_ENCODE_SUPPORTED)
#include "ddi_encode_jpeg_specific.h"

static bool RegisteredJPEGBaseline =
    DdiEncodeFactory::
        Register<encode::DdiEncodeJpeg>(ComponentInfo {VAProfileJPEGBaseline, VAEntrypointEncPicture});
#endif // _JPEG_ENCODE_SUPPORTED

#if defined (_HEVC_DECODE_SUPPORTED)
#include "ddi_decode_hevc_specific.h"

static bool RegisteredHEVCMainVLD =
    DdiDecodeFactory::
        Register<decode::DdiDecodeHevc>(ComponentInfo {VAProfileHEVCMain, VAEntrypointVLD});
static bool RegisteredHEVCMain10VLD =
    DdiDecodeFactory::
        Register<decode::DdiDecodeHevc>(ComponentInfo {VAProfileHEVCMain10, VAEntrypointVLD});
static bool RegisteredHEVCMain12VLD =
    DdiDecodeFactory::
        Register<decode::DdiDecodeHevc>(ComponentInfo {VAProfileHEVCMain12, VAEntrypointVLD});
static bool RegisteredHEVCMain422_10VLD =
    DdiDecodeFactory::
        Register<decode::DdiDecodeHevc>(ComponentInfo {VAProfileHEVCMain422_10, VAEntrypointVLD});
static bool RegisteredHEVCMain422_12VLD =
    DdiDecodeFactory::
        Register<decode::DdiDecodeHevc>(ComponentInfo {VAProfileHEVCMain422_12, VAEntrypointVLD});
static bool RegisteredHEVCMain444VLD =
    DdiDecodeFactory::
        Register<decode::DdiDecodeHevc>(ComponentInfo {VAProfileHEVCMain444, VAEntrypointVLD});
static bool RegisteredHEVCMain444_10VLD =
    DdiDecodeFactory::
        Register<decode::DdiDecodeHevc>(ComponentInfo {VAProfileHEVCMain444_10, VAEntrypointVLD});
static bool RegisteredHEVCMain444_12VLD =
    DdiDecodeFactory::
        Register<decode::DdiDecodeHevc>(ComponentInfo {VAProfileHEVCMain444_12, VAEntrypointVLD});
static bool RegisteredHEVCMainSCCVLD =
    DdiDecodeFactory::
        Register<decode::DdiDecodeHevc>(ComponentInfo {VAProfileHEVCSccMain, VAEntrypointVLD});
static bool RegisteredHEVCMainSCC_10VLD =
    DdiDecodeFactory::
        Register<decode::DdiDecodeHevc>(ComponentInfo {VAProfileHEVCSccMain10, VAEntrypointVLD});
static bool RegisteredHEVCMainSCC444VLD =
    DdiDecodeFactory::
        Register<decode::DdiDecodeHevc>(ComponentInfo {VAProfileHEVCSccMain444, VAEntrypointVLD});
static bool RegisteredHEVCMainSCC444_10VLD =
    DdiDecodeFactory::
        Register<decode::DdiDecodeHevc>(ComponentInfo {VAProfileHEVCSccMain444_10, VAEntrypointVLD});
#endif // _HEVC_DECODE_SUPPORTED

#if defined (_AVC_DECODE_SUPPORTED)
#include "ddi_decode_avc_specific.h"

static bool RegisteredH264MainVLD =
    DdiDecodeFactory::
        Register<decode::DdiDecodeAvc>(ComponentInfo {VAProfileH264Main, VAEntrypointVLD});
static bool RegisteredH264HighVLD =
    DdiDecodeFactory::
        Register<decode::DdiDecodeAvc>(ComponentInfo{VAProfileH264High, VAEntrypointVLD});
static bool RegisteredH264ConstrainedBaselineVLD =
    DdiDecodeFactory::
        Register<decode::DdiDecodeAvc>(ComponentInfo{VAProfileH264ConstrainedBaseline, VAEntrypointVLD});
#endif // _AVC_DECODE_SUPPORTED

#if defined (_AV1_DECODE_SUPPORTED)
#include "ddi_decode_av1_specific.h"

static bool RegisteredAv1Profile0VLD =
    DdiDecodeFactory::
        Register<decode::DdiDecodeAv1>(ComponentInfo {VAProfileAV1Profile0, VAEntrypointVLD});
#endif // _AV1_DECODE_SUPPORTED

#if defined (_VVC_DECODE_SUPPORTED)
#include "ddi_decode_vvc_specific.h"

static bool RegisteredVAProfileVVCMain10 =
    DdiDecodeFactory::
        Register<decode::DdiDecodeVvc>(ComponentInfo {(VAProfile)VAProfileVVCMain10, VAEntrypointVLD});
#endif

#if defined (_JPEG_DECODE_SUPPORTED)
#include "ddi_decode_jpeg_specific.h"

static bool RegisteredJpegBaselineVLD =
    DdiDecodeFactory::
        Register<decode::DdiDecodeJpeg>(ComponentInfo {VAProfileJPEGBaseline, VAEntrypointVLD});
#endif // _JPEG_DECODE_SUPPORTED

#if defined (_MPEG2_DECODE_SUPPORTED)
#include "ddi_decode_mpeg2_specific.h"

static bool RegisteredMPEG2SimpleVLD =
    DdiDecodeFactory::
        Register<decode::DdiDecodeMpeg2>(ComponentInfo {VAProfileMPEG2Simple, VAEntrypointVLD});
static bool RegisteredMPEG2MainVLD =
    DdiDecodeFactory::
        Register<decode::DdiDecodeMpeg2>(ComponentInfo {VAProfileMPEG2Main, VAEntrypointVLD});
#endif // _MPEG2_DECODE_SUPPORTED

#if defined (_VP8_DECODE_SUPPORTED)
#include "ddi_decode_vp8_specific.h"

static bool RegisteredVP8Version0_3VLD =
    DdiDecodeFactory::
        Register<decode::DdiDecodeVp8>(ComponentInfo {VAProfileVP8Version0_3, VAEntrypointVLD});
#endif // _VP8_DECODE_SUPPORTED

#if defined (_VP9_DECODE_SUPPORTED)
#include "ddi_decode_vp9_specific.h"

static bool RegisteredVP9Profile0VLD =
    DdiDecodeFactory::
        Register<decode::DdiDecodeVp9>(ComponentInfo {VAProfileVP9Profile0, VAEntrypointVLD});
static bool RegisteredVP9Profile1VLD =
    DdiDecodeFactory::
        Register<decode::DdiDecodeVp9>(ComponentInfo {VAProfileVP9Profile1, VAEntrypointVLD});
static bool RegisteredVP9Profile2VLD =
    DdiDecodeFactory::
        Register<decode::DdiDecodeVp9>(ComponentInfo {VAProfileVP9Profile2, VAEntrypointVLD});
static bool RegisteredVP9Profile3VLD =
    DdiDecodeFactory::
        Register<decode::DdiDecodeVp9>(ComponentInfo {VAProfileVP9Profile3, VAEntrypointVLD});
#endif // _VP9_DECODE_SUPPORTED

#endif // DDI_REGISTER_COMPONENTS_SPECIFIC_H
