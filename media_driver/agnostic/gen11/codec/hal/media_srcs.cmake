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

# common
set(TMP_1_SOURCES_
    ${CMAKE_CURRENT_LIST_DIR}/codechal_hw_g11_X.cpp
)

set(TMP_1_HEADERS_
    ${CMAKE_CURRENT_LIST_DIR}/codechal_hw_g11_X.h
)


#decode
set(TMP_2_SOURCES_ 
    ${CMAKE_CURRENT_LIST_DIR}/codechal_decode_histogram_vebox_g11.cpp
)

set(TMP_2_HEADERS_ 
    ${CMAKE_CURRENT_LIST_DIR}/codechal_decode_histogram_vebox_g11.h
)

if(${AVC_Decode_Supported} STREQUAL "yes")
    set(TMP_2_SOURCES_
        ${TMP_2_SOURCES_}
        ${CMAKE_CURRENT_LIST_DIR}/codechal_decode_avc_g11.cpp
    )
    set(TMP_2_HEADERS_
        ${TMP_2_HEADERS_}
        ${CMAKE_CURRENT_LIST_DIR}/codechal_decode_avc_g11.h
    )
endif()

if(${HEVC_Decode_Supported} STREQUAL "yes")
    set(TMP_2_SOURCES_
        ${TMP_2_SOURCES_}
        ${CMAKE_CURRENT_LIST_DIR}/codechal_decode_hevc_g11.cpp
    )
    set(TMP_2_HEADERS_
        ${TMP_2_HEADERS_}
        ${CMAKE_CURRENT_LIST_DIR}/codechal_decode_hevc_g11.h
    )
endif()

if(${JPEG_Decode_Supported} STREQUAL "yes")
    set(TMP_2_SOURCES_
        ${TMP_2_SOURCES_}
        ${CMAKE_CURRENT_LIST_DIR}/codechal_decode_jpeg_g11.cpp
    )
    set(TMP_2_HEADERS_
        ${TMP_2_HEADERS_}
        ${CMAKE_CURRENT_LIST_DIR}/codechal_decode_jpeg_g11.h
    )
endif()


if(${MPEG2_Decode_Supported} STREQUAL "yes")
    set(TMP_2_SOURCES_
        ${TMP_2_SOURCES_}
        ${CMAKE_CURRENT_LIST_DIR}/codechal_decode_mpeg2_g11.cpp
   )
   set(TMP_2_HEADERS_
        ${TMP_2_HEADERS_}
        ${CMAKE_CURRENT_LIST_DIR}/codechal_decode_mpeg2_g11.h
   )
endif()

if(${VC1_Decode_Supported} STREQUAL "yes")
    set(TMP_2_SOURCES_
        ${TMP_2_SOURCES_}
        ${CMAKE_CURRENT_LIST_DIR}/codechal_decode_vc1_g11.cpp
    )
    set(TMP_2_HEADERS_
        ${TMP_2_HEADERS_}
        ${CMAKE_CURRENT_LIST_DIR}/codechal_decode_vc1_g11.h
    )
endif()

if(${VP8_Decode_Supported} STREQUAL "yes")
    set(TMP_2_SOURCES_
        ${TMP_2_SOURCES_}
        ${CMAKE_CURRENT_LIST_DIR}/codechal_decode_vp8_g11.cpp
    )
    set(TMP_2_HEADERS_
        ${TMP_2_HEADERS_}
        ${CMAKE_CURRENT_LIST_DIR}/codechal_decode_vp8_g11.h
    )
endif()

if(${VP9_Decode_Supported} STREQUAL "yes")
    set(TMP_2_SOURCES_
        ${TMP_2_SOURCES_}
        ${CMAKE_CURRENT_LIST_DIR}/codechal_decode_vp9_g11.cpp
    )
    set(TMP_2_HEADERS_
        ${TMP_2_HEADERS_}
        ${CMAKE_CURRENT_LIST_DIR}/codechal_decode_vp9_g11.h
    )
endif()


# encode
if(${Common_Encode_Supported} STREQUAL "yes")
    set(TMP_3_SOURCES_
        ${CMAKE_CURRENT_LIST_DIR}/codechal_encode_csc_ds_g11.cpp
        ${CMAKE_CURRENT_LIST_DIR}/codechal_kernel_hme_g11.cpp
        ${CMAKE_CURRENT_LIST_DIR}/codechal_encode_wp_g11.cpp
        ${CMAKE_CURRENT_LIST_DIR}/codechal_debug_encode_par_g11.cpp
        ${CMAKE_CURRENT_LIST_DIR}/codechal_encode_sw_scoreboard_g11.cpp
        ${CMAKE_CURRENT_LIST_DIR}/codechal_huc_cmd_initializer_g11.cpp
        ${CMAKE_CURRENT_LIST_DIR}/codechal_encode_sfc_g11.cpp
    )

    set(TMP_3_HEADERS_
        ${CMAKE_CURRENT_LIST_DIR}/codechal_encode_csc_ds_g11.h
        ${CMAKE_CURRENT_LIST_DIR}/codechal_kernel_hme_g11.h
        ${CMAKE_CURRENT_LIST_DIR}/codechal_encode_wp_g11.h
        ${CMAKE_CURRENT_LIST_DIR}/codechal_kernel_header_g11.h
        ${CMAKE_CURRENT_LIST_DIR}/codechal_debug_encode_par_g11.h
        ${CMAKE_CURRENT_LIST_DIR}/codechal_encode_sw_scoreboard_g11.h
        ${CMAKE_CURRENT_LIST_DIR}/codechal_huc_cmd_initializer_g11.h
        ${CMAKE_CURRENT_LIST_DIR}/codechal_encode_sfc_g11.h
    )
endif()

if ("${AVC_Encode_VME_Supported}" STREQUAL "yes" OR "${AVC_Encode_VDEnc_Supported}" STREQUAL "yes")
    if ("${AVC_Encode_VME_Supported}" STREQUAL "yes")
        set (TMP_3_SOURCES_
            ${TMP_3_SOURCES_}
            ${CMAKE_CURRENT_LIST_DIR}/codechal_encode_avc_g11.cpp
        )
        set (TMP_3_HEADERS_
            ${TMP_3_HEADERS_}
            ${CMAKE_CURRENT_LIST_DIR}/codechal_encode_avc_g11.h
        )
    endif ()

    if ("${AVC_Encode_VDEnc_Supported}" STREQUAL "yes")
        set (TMP_3_SOURCES_
            ${TMP_3_SOURCES_}
            ${CMAKE_CURRENT_LIST_DIR}/codechal_vdenc_avc_g11.cpp
        )
        set (TMP_3_HEADERS_
            ${TMP_3_HEADERS_}
            ${CMAKE_CURRENT_LIST_DIR}/codechal_vdenc_avc_g11.h
        )
    endif ()
endif ()

if ("${HEVC_Encode_VME_Supported}" STREQUAL "yes" OR "${HEVC_Encode_VDEnc_Supported}" STREQUAL "yes")
    set (TMP_3_SOURCES_
        ${TMP_3_SOURCES_}
        ${CMAKE_CURRENT_LIST_DIR}/codechal_encode_hevc_table_g11.cpp
    )

    if ("${HEVC_Encode_VME_Supported}" STREQUAL "yes")
        set (TMP_3_SOURCES_
            ${TMP_3_SOURCES_}
            ${CMAKE_CURRENT_LIST_DIR}/codechal_encode_hevc_g11.cpp
        )
        set (TMP_3_HEADERS_
            ${TMP_3_HEADERS_}
            ${CMAKE_CURRENT_LIST_DIR}/codechal_encode_hevc_g11.h
        )
    endif ()

    if ("${HEVC_Encode_VDEnc_Supported}" STREQUAL "yes")
        set (TMP_3_SOURCES_
            ${TMP_3_SOURCES_}
            ${CMAKE_CURRENT_LIST_DIR}/codechal_vdenc_hevc_g11.cpp
        )
        set (TMP_3_HEADERS_
            ${TMP_3_HEADERS_}
            ${CMAKE_CURRENT_LIST_DIR}/codechal_vdenc_hevc_g11.h
        )
    endif ()
endif ()

if(${VP9_Encode_VDEnc_Supported} STREQUAL "yes")
    set(TMP_3_SOURCES_
        ${TMP_3_SOURCES_}
        ${CMAKE_CURRENT_LIST_DIR}/codechal_vdenc_vp9_g11.cpp
    )
    set(TMP_3_HEADERS_
        ${TMP_3_HEADERS_}
        ${CMAKE_CURRENT_LIST_DIR}/codechal_vdenc_vp9_g11.h
    )
endif()

if(${MPEG2_Encode_VME_Supported} STREQUAL "yes")
    set(TMP_3_SOURCES_
        ${TMP_3_SOURCES_}
        ${CMAKE_CURRENT_LIST_DIR}/codechal_encode_mpeg2_g11.cpp
    )
    set(TMP_3_HEADERS_
        ${TMP_3_HEADERS_}
        ${CMAKE_CURRENT_LIST_DIR}/codechal_encode_mpeg2_g11.h
    )
endif()

if(${JPEG_Encode_Supported} STREQUAL "yes")
    set(TMP_3_SOURCES_
        ${TMP_3_SOURCES_}
        ${CMAKE_CURRENT_LIST_DIR}/codechal_encode_jpeg_g11.cpp
    )
    set(TMP_3_HEADERS_
        ${TMP_3_HEADERS_}
        ${CMAKE_CURRENT_LIST_DIR}/codechal_encode_jpeg_g11.h
    )
endif()

if(${VP8_Encode_Supported} STREQUAL "yes")
    set(TMP_3_SOURCES_
        ${TMP_3_SOURCES_}
        ${CMAKE_CURRENT_LIST_DIR}/codechal_encode_vp8_g11.cpp
    )
    set(TMP_3_HEADERS_
        ${TMP_3_HEADERS_}
        ${CMAKE_CURRENT_LIST_DIR}/codechal_encode_vp8_g11.h
    )
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

source_group( CodecHal\\Common FILES ${TMP_1_SOURCES_} ${TMP_1_HEADERS_} )
source_group( CodecHal\\Decode FILES ${TMP_2_SOURCES_} ${TMP_2_HEADERS_} )
source_group( CodecHal\\Encode FILES ${TMP_3_SOURCES_} ${TMP_3_HEADERS_} )


media_add_curr_to_include_path()
