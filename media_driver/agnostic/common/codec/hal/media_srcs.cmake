# Copyright (c) 2017-2022, Intel Corporation
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

# common
set(TMP_1_SOURCES_
    ${CMAKE_CURRENT_LIST_DIR}/codechal_hw.cpp
    ${CMAKE_CURRENT_LIST_DIR}/codechal_utilities.cpp
    ${CMAKE_CURRENT_LIST_DIR}/codechal_mmc.cpp
    ${CMAKE_CURRENT_LIST_DIR}/codechal_allocator.cpp
    ${CMAKE_CURRENT_LIST_DIR}/codechal_debug_misc.cpp
)

set(TMP_1_HEADERS_
    ${CMAKE_CURRENT_LIST_DIR}/BRCIF.h 
    ${CMAKE_CURRENT_LIST_DIR}/codechal_hw.h
    ${CMAKE_CURRENT_LIST_DIR}/codechal_utilities.h
    ${CMAKE_CURRENT_LIST_DIR}/codechal_mmc.h
    ${CMAKE_CURRENT_LIST_DIR}/codechal_allocator.h
)

if(${MMC_Supported} STREQUAL "yes")
    set(TMP_1_SOURCES_
        ${TMP_1_SOURCES_}
        ${CMAKE_CURRENT_LIST_DIR}/codechal_memdecomp.cpp
    )
    set(TMP_1_HEADERS_
        ${TMP_1_HEADERS_}
        ${CMAKE_CURRENT_LIST_DIR}/codechal_memdecomp.h
    )
endif()

set(TMP_1_HEADERS_
        ${TMP_1_HEADERS_}
)

#decode
set(TMP_2_SOURCES_
    ${CMAKE_CURRENT_LIST_DIR}/codechal_decode_nv12top010.cpp
    ${CMAKE_CURRENT_LIST_DIR}/codechal_decoder.cpp
    ${CMAKE_CURRENT_LIST_DIR}/codechal_decode_histogram.cpp
    ${CMAKE_CURRENT_LIST_DIR}/codechal_decode_histogram_vebox.cpp
    ${CMAKE_CURRENT_LIST_DIR}/codechal_decode_singlepipe_virtualengine.cpp
    ${CMAKE_CURRENT_LIST_DIR}/codechal_decode_scalability.cpp
)

set(TMP_2_HEADERS_
    ${CMAKE_CURRENT_LIST_DIR}/codechal_decode_nv12top010.h
    ${CMAKE_CURRENT_LIST_DIR}/codechal_decoder.h
    ${CMAKE_CURRENT_LIST_DIR}/codechal_decode_histogram.h
    ${CMAKE_CURRENT_LIST_DIR}/codechal_decode_histogram_vebox.h
    ${CMAKE_CURRENT_LIST_DIR}/codechal_decode_singlepipe_virtualengine.h
    ${CMAKE_CURRENT_LIST_DIR}/codechal_decode_scalability.h
    ${CMAKE_CURRENT_LIST_DIR}/codechal_secure_decode_interface.h
)
if(${Decode_Processing_Supported} STREQUAL "yes")
    set(TMP_2_SOURCES_
        ${TMP_2_SOURCES_}
        ${CMAKE_CURRENT_LIST_DIR}/codechal_decode_downsampling.cpp
        ${CMAKE_CURRENT_LIST_DIR}/codechal_decode_sfc.cpp
    )
    set(TMP_2_HEADERS_
        ${TMP_2_HEADERS_}
        ${CMAKE_CURRENT_LIST_DIR}/codechal_decode_downsampling.h
        ${CMAKE_CURRENT_LIST_DIR}/codechal_decode_sfc.h
    )
endif()

if(${AVC_Decode_Supported} STREQUAL "yes")
    set(TMP_2_SOURCES_
        ${TMP_2_SOURCES_}
        ${CMAKE_CURRENT_LIST_DIR}/codechal_decode_avc.cpp
    )
    set(TMP_2_HEADERS_
        ${TMP_2_HEADERS_}
        ${CMAKE_CURRENT_LIST_DIR}/codechal_decode_avc.h
    )
    if(${Decode_Processing_Supported} STREQUAL "yes")
        set(TMP_2_SOURCES_
            ${TMP_2_SOURCES_}
            ${CMAKE_CURRENT_LIST_DIR}/codechal_decode_sfc_avc.cpp
        )
        set(TMP_2_HEADERS_
            ${TMP_2_HEADERS_}
            ${CMAKE_CURRENT_LIST_DIR}/codechal_decode_sfc_avc.h
        )
    endif()

    if(${MMC_Supported} STREQUAL "yes")
        set(TMP_2_SOURCES_
            ${TMP_2_SOURCES_}
            ${CMAKE_CURRENT_LIST_DIR}/codechal_mmc_decode_avc.cpp
        )
        set(TMP_2_HEADERS_
            ${TMP_2_HEADERS_}
            ${CMAKE_CURRENT_LIST_DIR}/codechal_mmc_decode_avc.h
        )
    endif()
endif()

if(${HEVC_Decode_Supported} STREQUAL "yes")
    set(TMP_2_SOURCES_
        ${TMP_2_SOURCES_}
        ${CMAKE_CURRENT_LIST_DIR}/codechal_decode_hevc.cpp
    )
    set(TMP_2_HEADERS_
        ${TMP_2_HEADERS_}
        ${CMAKE_CURRENT_LIST_DIR}/codechal_decode_hevc.h
    )
    if(${Decode_Processing_Supported} STREQUAL "yes")
        set(TMP_2_SOURCES_
            ${TMP_2_SOURCES_}
            ${CMAKE_CURRENT_LIST_DIR}/codechal_decode_sfc_hevc.cpp
        )
        set(TMP_2_HEADERS_
            ${TMP_2_HEADERS_}
            ${CMAKE_CURRENT_LIST_DIR}/codechal_decode_sfc_hevc.h
        )
    endif()

    if(${MMC_Supported} STREQUAL "yes")
        set(TMP_2_SOURCES_
            ${TMP_2_SOURCES_}
            ${CMAKE_CURRENT_LIST_DIR}/codechal_mmc_decode_hevc.cpp
        )
        set(TMP_2_HEADERS_
            ${TMP_2_HEADERS_}
            ${CMAKE_CURRENT_LIST_DIR}/codechal_mmc_decode_hevc.h
        )
    endif()
endif()

if(${JPEG_Decode_Supported} STREQUAL "yes")
    set(TMP_2_SOURCES_
        ${TMP_2_SOURCES_}
        ${CMAKE_CURRENT_LIST_DIR}/codechal_decode_jpeg.cpp
        ${CMAKE_CURRENT_LIST_DIR}/codechal_mmc_decode_jpeg.cpp
    )
    set(TMP_2_HEADERS_
        ${TMP_2_HEADERS_}
        ${CMAKE_CURRENT_LIST_DIR}/codechal_decode_jpeg.h
        ${CMAKE_CURRENT_LIST_DIR}/codechal_mmc_decode_jpeg.h
    )
    if(${Decode_Processing_Supported} STREQUAL "yes")
        set(TMP_2_SOURCES_
            ${TMP_2_SOURCES_}
            ${CMAKE_CURRENT_LIST_DIR}/codechal_decode_sfc_jpeg.cpp
        )
        set(TMP_2_HEADERS_
            ${TMP_2_HEADERS_}
            ${CMAKE_CURRENT_LIST_DIR}/codechal_decode_sfc_jpeg.h
        )
    endif()

    if(${MMC_Supported} STREQUAL "yes")
        set(TMP_2_SOURCES_
            ${TMP_2_SOURCES_}
            ${CMAKE_CURRENT_LIST_DIR}/codechal_mmc_decode_jpeg.cpp
        )
        set(TMP_2_HEADERS_
            ${TMP_2_HEADERS_}
            ${CMAKE_CURRENT_LIST_DIR}/codechal_mmc_decode_jpeg.h
        )
    endif()
endif()


if(${MPEG2_Decode_Supported} STREQUAL "yes")
    set(TMP_2_SOURCES_
        ${TMP_2_SOURCES_}
        ${CMAKE_CURRENT_LIST_DIR}/codechal_decode_mpeg2.cpp
    )
    set(TMP_2_HEADERS_
        ${TMP_2_HEADERS_}
        ${CMAKE_CURRENT_LIST_DIR}/codechal_decode_mpeg2.h
    )

    if(${MMC_Supported} STREQUAL "yes")
        set(TMP_2_SOURCES_
            ${TMP_2_SOURCES_}
            ${CMAKE_CURRENT_LIST_DIR}/codechal_mmc_decode_mpeg2.cpp
        )
        set(TMP_2_HEADERS_
            ${TMP_2_HEADERS_}
            ${CMAKE_CURRENT_LIST_DIR}/codechal_mmc_decode_mpeg2.h
        )
    endif()
endif()

if(${VC1_Decode_Supported} STREQUAL "yes")
    set(TMP_2_SOURCES_
        ${TMP_2_SOURCES_}
        ${CMAKE_CURRENT_LIST_DIR}/codechal_decode_vc1.cpp
    )
    set(TMP_2_HEADERS_
        ${TMP_2_HEADERS_}
        ${CMAKE_CURRENT_LIST_DIR}/codechal_decode_vc1.h
    )

    if(${MMC_Supported} STREQUAL "yes")
        set(TMP_2_SOURCES_
            ${TMP_2_SOURCES_}
            ${CMAKE_CURRENT_LIST_DIR}/codechal_mmc_decode_vc1.cpp
        )
        set(TMP_2_HEADERS_
            ${TMP_2_HEADERS_}
            ${CMAKE_CURRENT_LIST_DIR}/codechal_mmc_decode_vc1.h
        )
    endif()
endif()

if(${VP8_Decode_Supported} STREQUAL "yes")
    set(TMP_2_SOURCES_
        ${TMP_2_SOURCES_}
        ${CMAKE_CURRENT_LIST_DIR}/codechal_decode_vp8.cpp
    )
    set(TMP_2_HEADERS_
        ${TMP_2_HEADERS_}
        ${CMAKE_CURRENT_LIST_DIR}/codechal_decode_vp8.h
    )

    if(${MMC_Supported} STREQUAL "yes")
        set(TMP_2_SOURCES_
            ${TMP_2_SOURCES_}
            ${CMAKE_CURRENT_LIST_DIR}/codechal_mmc_decode_vp8.cpp
        )
        set(TMP_2_HEADERS_
            ${TMP_2_HEADERS_}
            ${CMAKE_CURRENT_LIST_DIR}/codechal_mmc_decode_vp8.h
        )
    endif()
endif()

if(${VP9_Decode_Supported} STREQUAL "yes")
    set(TMP_2_SOURCES_
        ${TMP_2_SOURCES_}
        ${CMAKE_CURRENT_LIST_DIR}/codechal_decode_vp9.cpp
    )
    set(TMP_2_HEADERS_
        ${TMP_2_HEADERS_}
        ${CMAKE_CURRENT_LIST_DIR}/codechal_decode_vp9.h
    )

    if(${MMC_Supported} STREQUAL "yes")
        set(TMP_2_SOURCES_
            ${TMP_2_SOURCES_}
            ${CMAKE_CURRENT_LIST_DIR}/codechal_mmc_decode_vp9.cpp
        )
        set(TMP_2_HEADERS_
            ${TMP_2_HEADERS_}
            ${CMAKE_CURRENT_LIST_DIR}/codechal_mmc_decode_vp9.h
        )
    endif()
endif()


# encode
if(${Common_Encode_Supported} STREQUAL "yes")
    set(TMP_3_SOURCES_
        ${CMAKE_CURRENT_LIST_DIR}/codechal_debug_encode_par.cpp
        ${CMAKE_CURRENT_LIST_DIR}/codechal_encode_csc_ds.cpp
        ${CMAKE_CURRENT_LIST_DIR}/codechal_encode_sfc_base.cpp
        ${CMAKE_CURRENT_LIST_DIR}/codechal_kernel_base.cpp
        ${CMAKE_CURRENT_LIST_DIR}/codechal_kernel_hme.cpp
        ${CMAKE_CURRENT_LIST_DIR}/codechal_kernel_intra_dist.cpp
        ${CMAKE_CURRENT_LIST_DIR}/codechal_encode_wp.cpp
        ${CMAKE_CURRENT_LIST_DIR}/codechal_encoder_base.cpp
        ${CMAKE_CURRENT_LIST_DIR}/codechal_encode_tracked_buffer.cpp
        ${CMAKE_CURRENT_LIST_DIR}/codechal_huc_cmd_initializer.cpp
        ${CMAKE_CURRENT_LIST_DIR}/codechal_encode_allocator.cpp
        ${CMAKE_CURRENT_LIST_DIR}/codechal_encode_singlepipe_virtualengine.cpp
        ${CMAKE_CURRENT_LIST_DIR}/codechal_encode_scalability.cpp
        ${CMAKE_CURRENT_LIST_DIR}/codechal_encode_sw_scoreboard.cpp
    )

    set(TMP_3_HEADERS_
        ${CMAKE_CURRENT_LIST_DIR}/codechal_debug_encode_par.h
        ${CMAKE_CURRENT_LIST_DIR}/codechal_encode_csc_ds.h
        ${CMAKE_CURRENT_LIST_DIR}/codechal_encode_sfc_base.h
        ${CMAKE_CURRENT_LIST_DIR}/codechal_encode_sfc.h
        ${CMAKE_CURRENT_LIST_DIR}/codechal_kernel_base.h
        ${CMAKE_CURRENT_LIST_DIR}/codechal_kernel_hme.h
        ${CMAKE_CURRENT_LIST_DIR}/codechal_kernel_intra_dist.h
        ${CMAKE_CURRENT_LIST_DIR}/codechal_encode_wp.h
        ${CMAKE_CURRENT_LIST_DIR}/codechal_encoder_base.h
        ${CMAKE_CURRENT_LIST_DIR}/codechal_encoder_unsupported.h
        ${CMAKE_CURRENT_LIST_DIR}/codechal_encode_tracked_buffer.h
        ${CMAKE_CURRENT_LIST_DIR}/codechal_huc_cmd_initializer.h
        ${CMAKE_CURRENT_LIST_DIR}/codechal_encode_allocator.h
        ${CMAKE_CURRENT_LIST_DIR}/codechal_encode_singlepipe_virtualengine.h
        ${CMAKE_CURRENT_LIST_DIR}/codechal_encode_scalability.h
        ${CMAKE_CURRENT_LIST_DIR}/codechal_encode_sw_scoreboard.h
    )
endif()

if ("${AVC_Encode_VME_Supported}" STREQUAL "yes" OR "${AVC_Encode_VDEnc_Supported}" STREQUAL "yes")
    set (TMP_3_SOURCES_
        ${TMP_3_SOURCES_}
        ${CMAKE_CURRENT_LIST_DIR}/codechal_encode_avc_base.cpp
    )
    set (TMP_3_HEADERS_
        ${TMP_3_HEADERS_}
        ${CMAKE_CURRENT_LIST_DIR}/codechal_encode_avc_base.h
    )

    if ("${AVC_Encode_VME_Supported}" STREQUAL "yes")
        set (TMP_3_SOURCES_
            ${TMP_3_SOURCES_}
            ${CMAKE_CURRENT_LIST_DIR}/codechal_encode_avc.cpp
        )
        set (TMP_3_HEADERS_
            ${TMP_3_HEADERS_}
            ${CMAKE_CURRENT_LIST_DIR}/codechal_encode_avc.h
        )
    endif ()

    if ("${AVC_Encode_VDEnc_Supported}" STREQUAL "yes")
        set (TMP_3_SOURCES_
            ${TMP_3_SOURCES_}
            ${CMAKE_CURRENT_LIST_DIR}/codechal_vdenc_avc.cpp
        )
        set (TMP_3_HEADERS_
            ${TMP_3_HEADERS_}
            ${CMAKE_CURRENT_LIST_DIR}/codechal_vdenc_avc.h
        )
    endif ()

    if (${MMC_Supported} STREQUAL "yes")
        set (TMP_3_SOURCES_
            ${TMP_3_SOURCES_}
            ${CMAKE_CURRENT_LIST_DIR}/codechal_mmc_encode_avc.cpp
        )
        set (TMP_3_HEADERS_
            ${TMP_3_HEADERS_}
            ${CMAKE_CURRENT_LIST_DIR}/codechal_mmc_encode_avc.h
        )
    endif ()
endif ()

if ("${HEVC_Encode_VME_Supported}" STREQUAL "yes" OR "${HEVC_Encode_VDEnc_Supported}" STREQUAL "yes")
    set (TMP_3_SOURCES_
        ${TMP_3_SOURCES_}
        ${CMAKE_CURRENT_LIST_DIR}/codechal_encode_hevc_base.cpp
        ${CMAKE_CURRENT_LIST_DIR}/codechal_encode_tracked_buffer_hevc.cpp
    )
    set (TMP_3_HEADERS_
        ${TMP_3_HEADERS_}
        ${CMAKE_CURRENT_LIST_DIR}/codechal_encode_hevc_base.h
        ${CMAKE_CURRENT_LIST_DIR}/codechal_encode_tracked_buffer_hevc.h
    )

    if ("${HEVC_Encode_VME_Supported}" STREQUAL "yes")
        set (TMP_3_SOURCES_
            ${TMP_3_SOURCES_}
            ${CMAKE_CURRENT_LIST_DIR}/codechal_encode_hevc.cpp
        )
        set (TMP_3_HEADERS_
            ${TMP_3_HEADERS_}
            ${CMAKE_CURRENT_LIST_DIR}/codechal_encode_hevc.h
        )
    endif ()

    if ("${HEVC_Encode_VDEnc_Supported}" STREQUAL "yes")
        set (TMP_3_SOURCES_
            ${TMP_3_SOURCES_}
            ${CMAKE_CURRENT_LIST_DIR}/codechal_vdenc_hevc.cpp
        )
        set (TMP_3_HEADERS_
            ${TMP_3_HEADERS_}
            ${CMAKE_CURRENT_LIST_DIR}/codechal_vdenc_hevc.h
        )
    endif ()

    if (${MMC_Supported} STREQUAL "yes")
        set (TMP_3_SOURCES_
            ${TMP_3_SOURCES_}
            ${CMAKE_CURRENT_LIST_DIR}/codechal_mmc_encode_hevc.cpp
        )
        set (TMP_3_HEADERS_
            ${TMP_3_HEADERS_}
            ${CMAKE_CURRENT_LIST_DIR}/codechal_mmc_encode_hevc.h
        )
    endif ()
endif ()

if(${VP9_Encode_VDEnc_Supported} STREQUAL "yes")
    set(TMP_3_SOURCES_
        ${TMP_3_SOURCES_}
        ${CMAKE_CURRENT_LIST_DIR}/codechal_vdenc_vp9_base.cpp
        ${CMAKE_CURRENT_LIST_DIR}/codechal_mmc_encode_vp9.cpp
    )
    set(TMP_3_HEADERS_
        ${TMP_3_HEADERS_}
        ${CMAKE_CURRENT_LIST_DIR}/codechal_vdenc_vp9_base.h
        ${CMAKE_CURRENT_LIST_DIR}/codechal_mmc_encode_vp9.h
    )
endif()

if(${VP8_Encode_Supported} STREQUAL "yes")
    set(TMP_3_SOURCES_
        ${TMP_3_SOURCES_}
        ${CMAKE_CURRENT_LIST_DIR}/codechal_encode_vp8.cpp
    )
    set(TMP_3_HEADERS_
        ${TMP_3_HEADERS_}
        ${CMAKE_CURRENT_LIST_DIR}/codechal_encode_vp8.h
    )

    if(${MMC_Supported} STREQUAL "yes")
        set(TMP_3_SOURCES_
            ${TMP_3_SOURCES_}
            ${CMAKE_CURRENT_LIST_DIR}/codechal_mmc_encode_vp8.cpp
        )
        set(TMP_3_HEADERS_
            ${TMP_3_HEADERS_}
            ${CMAKE_CURRENT_LIST_DIR}/codechal_mmc_encode_vp8.h
        )
    endif()
endif()


if(${JPEG_Encode_Supported} STREQUAL "yes")
    set(TMP_3_SOURCES_
        ${TMP_3_SOURCES_}
        ${CMAKE_CURRENT_LIST_DIR}/codechal_encode_jpeg.cpp
    )
    set(TMP_3_HEADERS_
        ${TMP_3_HEADERS_}
        ${CMAKE_CURRENT_LIST_DIR}/codechal_encode_jpeg.h
    )
endif()

if(${MPEG2_Encode_VME_Supported} STREQUAL "yes")
    set(TMP_3_SOURCES_
        ${TMP_3_SOURCES_}
        ${CMAKE_CURRENT_LIST_DIR}/codechal_encode_mpeg2.cpp
    )
    set(TMP_3_HEADERS_
        ${TMP_3_HEADERS_}
        ${CMAKE_CURRENT_LIST_DIR}/codechal_encode_mpeg2.h
    )

    if(${MMC_Supported} STREQUAL "yes")
        set(TMP_3_SOURCES_
            ${TMP_3_SOURCES_}
            ${CMAKE_CURRENT_LIST_DIR}/codechal_mmc_encode_mpeg2.cpp
        )
        set(TMP_3_HEADERS_
            ${TMP_3_HEADERS_}
            ${CMAKE_CURRENT_LIST_DIR}/codechal_mmc_encode_mpeg2.h
        )
    endif()
endif()


set(SOURCES_
    ${SOURCES_}
    ${TMP_1_SOURCES_}
    ${TMP_2_SOURCES_}
    ${TMP_3_SOURCES_}
)

set(HEADERS_
    ${HEADERS_}
    ${TMP_1_HEADERS_}
    ${TMP_2_HEADERS_}
    ${TMP_3_HEADERS_}
)

set(CODEC_SOURCES_
    ${CODEC_SOURCES_}
    ${TMP_1_SOURCES_}
    ${TMP_2_SOURCES_}
    ${TMP_3_SOURCES_}
)

set(CODEC_HEADERS_
    ${CODEC_HEADERS_}
    ${TMP_1_HEADERS_}
    ${TMP_2_HEADERS_}
    ${TMP_3_HEADERS_}
)

source_group( CodecHal\\Common FILES ${TMP_1_SOURCES_} ${TMP_1_HEADERS_} )
source_group( CodecHal\\Decode FILES ${TMP_2_SOURCES_} ${TMP_2_HEADERS_} )
source_group( CodecHal\\Encode FILES ${TMP_3_SOURCES_} ${TMP_3_HEADERS_} )
set(TMP_1_SOURCES_ "")
set(TMP_1_HEADERS_ "")
set(TMP_2_SOURCES_ "")
set(TMP_2_HEADERS_ "")
set(TMP_3_SOURCES_ "")
set(TMP_3_HEADERS_ "")

media_add_curr_to_include_path()
