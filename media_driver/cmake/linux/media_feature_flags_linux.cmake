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

# Full-open-source means only using open source EU kernels and fixed function hardware
# which is controlled by Full_Open_Source_Support flag.
# The opposite is full-feature which permits some features only have binaries.
# The default build is full-feature build so set Full_Open_Source_Support as "no".
bs_set_if_undefined(Full_Open_Source_Support "no")

if(${Full_Open_Source_Support} STREQUAL "yes")
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

# Use_CP_stub is just a flag to control whether real_cp or stub_cp
# source files should be compiled for a <OS>.
# so no definitions passed to source codes inside for now.
bs_set_if_undefined(Use_CP_Stub "yes")


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

if(${Full_Open_Source_Support} STREQUAL "yes")
    add_definitions(-D_FULL_OPEN_SOURCE)
endif()

include(${MEDIA_DRIVER_CMAKE}/ext/linux/media_feature_flags_linux_ext.cmake OPTIONAL)
