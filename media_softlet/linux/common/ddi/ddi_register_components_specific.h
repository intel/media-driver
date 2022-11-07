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

typedef MediaFactory<ComponentInfo, encode::DdiEncodeBase> DdiEncodeFactory;

#if defined (_HEVC_ENCODE_VDENC_SUPPORTED)
#include "ddi_encode_hevc_specific.h"

static bool RegisteredHevcMain =
    DdiEncodeFactory::
        Register<encode::DdiEncodeHevc>(ComponentInfo {VAProfileHEVCMain, VAEntrypointEncSliceLP});
static bool RegisteredHevcMain10 =
    DdiEncodeFactory::
        Register<encode::DdiEncodeHevc>(ComponentInfo{VAProfileHEVCMain10, VAEntrypointEncSliceLP});
static bool RegisteredHevcMain444 =
    DdiEncodeFactory::
        Register<encode::DdiEncodeHevc>(ComponentInfo{VAProfileHEVCMain444, VAEntrypointEncSliceLP});
static bool RegisteredHevcMain444_10 =
    DdiEncodeFactory::
        Register<encode::DdiEncodeHevc>(ComponentInfo{VAProfileHEVCMain444_10, VAEntrypointEncSliceLP});
static bool RegisteredHevcSCCMain =
    DdiEncodeFactory::
        Register<encode::DdiEncodeHevc>(ComponentInfo{VAProfileHEVCSccMain, VAEntrypointEncSliceLP});
static bool RegisteredHevcSCCMain10 =
    DdiEncodeFactory::
        Register<encode::DdiEncodeHevc>(ComponentInfo{VAProfileHEVCSccMain10, VAEntrypointEncSliceLP});
static bool RegisteredHevcSCCMain444 =
    DdiEncodeFactory::
        Register<encode::DdiEncodeHevc>(ComponentInfo{VAProfileHEVCSccMain444, VAEntrypointEncSliceLP});
static bool RegisteredHevcSCCMain444_10 =
    DdiEncodeFactory::
        Register<encode::DdiEncodeHevc>(ComponentInfo{VAProfileHEVCSccMain444_10, VAEntrypointEncSliceLP});
#endif // _HEVC_ENCODE_VDENC_SUPPORTED

#if defined (_AV1_ENCODE_VDENC_SUPPORTED)
#include "ddi_encode_av1_specific.h"
static bool RegisteredAv1Profile0 =
    DdiEncodeFactory::
        Register<encode::DdiEncodeAV1>(ComponentInfo {VAProfileAV1Profile0, VAEntrypointEncSliceLP});
#endif // _AV1_ENCODE_VDENC_SUPPORTED

#if defined (_VP9_ENCODE_VDENC_SUPPORTED)
#include "ddi_encode_vp9_specific.h"
static bool RegisteredVp9Profile0 =
    DdiEncodeFactory::
        Register<encode::DdiEncodeVp9>(ComponentInfo {VAProfileVP9Profile0, VAEntrypointEncSliceLP});
static bool RegisteredVp9Profile1 =
    DdiEncodeFactory::
        Register<encode::DdiEncodeVp9>(ComponentInfo {VAProfileVP9Profile1, VAEntrypointEncSliceLP});
static bool RegisteredVp9Profile2 =
    DdiEncodeFactory::
        Register<encode::DdiEncodeVp9>(ComponentInfo {VAProfileVP9Profile2, VAEntrypointEncSliceLP});
static bool RegisteredVp9Profile3 =
    DdiEncodeFactory::
        Register<encode::DdiEncodeVp9>(ComponentInfo {VAProfileVP9Profile3, VAEntrypointEncSliceLP});
#endif // _VP9_ENCODE_VDENC_SUPPORTED

#if defined (_AVC_ENCODE_VDENC_SUPPORTED)
#include "ddi_encode_avc_specific.h"

static bool RegisteredH264Main =
    DdiEncodeFactory::
        Register<encode::DdiEncodeAvc>(ComponentInfo {VAProfileH264Main, VAEntrypointEncSliceLP});
static bool RegisteredH264High =
    DdiEncodeFactory::
        Register<encode::DdiEncodeAvc>(ComponentInfo{VAProfileH264High, VAEntrypointEncSliceLP});
static bool RegisteredH264ConstrainedBaseline =
    DdiEncodeFactory::
        Register<encode::DdiEncodeAvc>(ComponentInfo{VAProfileH264ConstrainedBaseline, VAEntrypointEncSliceLP});
#endif // _AVC_ENCODE_VDENC_SUPPORTED

#if defined (_JPEG_ENCODE_SUPPORTED)
#include "ddi_encode_jpeg_specific.h"

static bool RegisteredJPEGBaseline =
    DdiEncodeFactory::
        Register<encode::DdiEncodeJpeg>(ComponentInfo {VAProfileJPEGBaseline, VAEntrypointEncPicture});
#endif // _JPEG_ENCODE_SUPPORTED

#endif // DDI_REGISTER_COMPONENTS_SPECIFIC_H
