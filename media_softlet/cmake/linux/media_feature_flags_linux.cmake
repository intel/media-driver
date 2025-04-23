# Copyright (c) 2017, Intel Corporation
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
# OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
# ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
# OTHER DEALINGS IN THE SOFTWARE.

# global flag for encode AVC_VME/HEVC_VME/MPEG2/VP8
bs_set_if_undefined(Encode_VME_Supported "yes")
# global flag for encode AVC_VDENC/HEVC_VDENC/VP9_VDENC/JPEG
bs_set_if_undefined(Encode_VDEnc_Supported "yes")

# Some features can't be supported if shaders (kernels) are not
# available. So, we switch such features off explicitly. That's
# possible either if user requested a build entirely without
# shaders or a build with free only shaders. The list of switched
# off features correspnds to the free kernels case, but we can
# reuse the full list for enable kernels as well.
if(NOT ENABLE_KERNELS OR NOT ENABLE_NONFREE_KERNELS)
    # full-open-source
    bs_set_if_undefined(AVC_Encode_VME_Supported "no")
    bs_set_if_undefined(HEVC_Encode_VME_Supported "no")
    bs_set_if_undefined(MPEG2_Encode_VME_Supported "no")
    bs_set_if_undefined(CMRT_HEVC_ENC_FEI_Supported "no")
    bs_set_if_undefined(MMC_Supported "no")
    bs_set_if_undefined(VC1_Decode_Supported "no")
    bs_set_if_undefined(Decode_Processing_Supported "no")
    bs_set_if_undefined(Kernel_Auto_Denoise_Supported "no")
    bs_set_if_undefined(VP8_Encode_Supported "no")
else()
    # full-feature
    bs_set_if_undefined(AVC_Encode_VME_Supported "${Encode_VME_Supported}")
    bs_set_if_undefined(HEVC_Encode_VME_Supported "${Encode_VME_Supported}")
    bs_set_if_undefined(MPEG2_Encode_VME_Supported "${Encode_VME_Supported}")
    bs_set_if_undefined(CMRT_HEVC_ENC_FEI_Supported "yes")
    bs_set_if_undefined(MMC_Supported "yes")
    bs_set_if_undefined(VC1_Decode_Supported "yes")
    bs_set_if_undefined(Decode_Processing_Supported "yes")
    bs_set_if_undefined(Kernel_Auto_Denoise_Supported "yes")
    bs_set_if_undefined(VP8_Encode_Supported "${Encode_VME_Supported}")
endif()

# features are always able to open
bs_set_if_undefined(VVC_Decode_Supported "yes")
bs_set_if_undefined(AV1_Decode_Supported "yes")
bs_set_if_undefined(AVC_Decode_Supported "yes")
bs_set_if_undefined(HEVC_Decode_Supported "yes")
bs_set_if_undefined(JPEG_Decode_Supported "yes")
bs_set_if_undefined(MPEG2_Decode_Supported "yes")
bs_set_if_undefined(VP8_Decode_Supported "yes")
bs_set_if_undefined(VP9_Decode_Supported "yes")
bs_set_if_undefined(VP_SFC_Supported "yes")
bs_set_if_undefined(Common_Encode_Supported "yes")
bs_set_if_undefined(Media_Scalability_Supported "yes")

# features controlled by global flag Encode_VDEnc_Supported
bs_set_if_undefined(AVC_Encode_VDEnc_Supported "${Encode_VDEnc_Supported}")
bs_set_if_undefined(HEVC_Encode_VDEnc_Supported "${Encode_VDEnc_Supported}")
bs_set_if_undefined(VP9_Encode_VDEnc_Supported "${Encode_VDEnc_Supported}")
bs_set_if_undefined(JPEG_Encode_Supported "${Encode_VDEnc_Supported}")

if(${Common_Encode_Supported} STREQUAL "yes")
    add_definitions(-D_COMMON_ENCODE_SUPPORTED)
endif()

if(${AVC_Encode_VME_Supported} STREQUAL "yes")
    add_definitions(-D_AVC_ENCODE_VME_SUPPORTED)
endif()

if(${AVC_Encode_VDEnc_Supported} STREQUAL "yes")
    add_definitions(-D_AVC_ENCODE_VDENC_SUPPORTED)
endif()

if(${AVC_Decode_Supported} STREQUAL "yes")
    add_definitions(-D_AVC_DECODE_SUPPORTED)
endif()

if (${HEVC_Encode_VME_Supported} STREQUAL "yes")
    add_definitions (-D_HEVC_ENCODE_VME_SUPPORTED)
endif()

if (${HEVC_Encode_VDEnc_Supported} STREQUAL "yes")
    add_definitions(-D_HEVC_ENCODE_VDENC_SUPPORTED)
endif ()

if(${HEVC_Decode_Supported} STREQUAL "yes")
    add_definitions(-D_HEVC_DECODE_SUPPORTED)
endif()

if(${JPEG_Encode_Supported} STREQUAL "yes")
    add_definitions(-D_JPEG_ENCODE_SUPPORTED)
endif()

if(${JPEG_Decode_Supported} STREQUAL "yes")
    add_definitions(-D_JPEG_DECODE_SUPPORTED)
endif()

if(${MPEG2_Encode_VME_Supported} STREQUAL "yes")
    add_definitions(-D_MPEG2_ENCODE_VME_SUPPORTED)
endif()

if(${MPEG2_Decode_Supported} STREQUAL "yes")
    add_definitions(-D_MPEG2_DECODE_SUPPORTED)
endif()

if(${VC1_Decode_Supported} STREQUAL "yes")
    add_definitions(-D_VC1_DECODE_SUPPORTED)
endif()

if(${VP8_Decode_Supported} STREQUAL "yes")
    add_definitions(-D_VP8_DECODE_SUPPORTED)
endif()

if(${VP8_Encode_Supported} STREQUAL "yes")
    add_definitions(-D_VP8_ENCODE_SUPPORTED)
endif()

if(${VP9_Encode_VDEnc_Supported} STREQUAL "yes")
    add_definitions(-D_VP9_ENCODE_VDENC_SUPPORTED)
endif()

if(${VP9_Decode_Supported} STREQUAL "yes")
    add_definitions(-D_VP9_DECODE_SUPPORTED)
endif()

if(${AV1_Decode_Supported} STREQUAL "yes")
    add_definitions(-D_AV1_DECODE_SUPPORTED)
endif()

if(${VVC_Decode_Supported} STREQUAL "yes")
    add_definitions(-D_VVC_DECODE_SUPPORTED)
endif()

if(${CMRT_HEVC_ENC_FEI_Supported} STREQUAL "yes")
    add_definitions(-DHEVC_FEI_ENABLE_CMRT)
endif()

if(${Decode_Processing_Supported} STREQUAL "yes")
    add_definitions(-D_DECODE_PROCESSING_SUPPORTED)
endif()

if(${MMC_Supported} STREQUAL "yes")
    add_definitions(-D_MMC_SUPPORTED)
endif()

if(${Kernel_Auto_Denoise_Supported} STREQUAL "yes")
    add_definitions(-DVEBOX_AUTO_DENOISE_SUPPORTED=1)
else()
    add_definitions(-DVEBOX_AUTO_DENOISE_SUPPORTED=0)
endif()

if(${VP_SFC_Supported} STREQUAL "yes")
    add_definitions(-D__VPHAL_SFC_SUPPORTED=1)
else()
    add_definitions(-D__VPHAL_SFC_SUPPORTED=0)
endif()

bs_set_if_undefined(CLASS_TRACE 0)
add_definitions(-DCLASS_TRACE=${CLASS_TRACE})

if(ENABLE_KERNELS)
    add_definitions(-DENABLE_KERNELS)
endif()

if(NOT ENABLE_NONFREE_KERNELS)
    add_definitions(-D_FULL_OPEN_SOURCE)
endif()

include(${MEDIA_SOFTLET_EXT_CMAKE}/linux/media_feature_flags_linux_ext.cmake OPTIONAL)
