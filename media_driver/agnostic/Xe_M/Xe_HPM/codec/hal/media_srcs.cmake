#=========================== begin_copyright_notice ============================
#
# INTEL CONFIDENTIAL
#
# Copyright (C) 2020-2022 Intel Corporation
#
# This software and the related documents are Intel copyrighted materials,
# and your use of them is governed by the express license under which they were
# provided to you ("License"). Unless the License provides otherwise,
# you may not use, modify, copy, publish, distribute, disclose or transmit this
# software or the related documents without Intel's prior written permission.
#
# This software and the related documents are provided as is, with no express or
# implied warranties, other than those that are expressly stated in the License.
#
#============================ end_copyright_notice =============================


set (TMP_DEC_SOURCES_
    ${TMP_DEC_SOURCES_}
    ${CMAKE_CURRENT_LIST_DIR}/codechal_decode_vc1_xe_hpm.cpp
    ${CMAKE_CURRENT_LIST_DIR}/codechal_kernel_olp_mdf_xe_hpm.cpp
)
set (TMP_DEC_HEADERS_
    ${TMP_DEC_HEADERS_}
    ${CMAKE_CURRENT_LIST_DIR}/codechal_decode_vc1_xe_hpm.h
    ${CMAKE_CURRENT_LIST_DIR}/codechal_kernel_olp_mdf_xe_hpm.h
)

if ("${HEVC_Encode_VDEnc_Supported}" STREQUAL "yes")
    set (TMP_ENC_SOURCES_
        ${TMP_ENC_SOURCES_}
        ${CMAKE_CURRENT_LIST_DIR}/codechal_hw_xe_hpm.cpp
    )
    set (TMP_ENC_HEADERS_
        ${TMP_ENC_HEADERS_}
        ${CMAKE_CURRENT_LIST_DIR}/codechal_hw_xe_hpm.h
    )
endif ()

if ("${VP9_Encode_VDEnc_Supported}" STREQUAL "yes")
    set (TMP_ENC_HEADERS_
        ${TMP_ENC_HEADERS_}
        ${CMAKE_CURRENT_LIST_DIR}/codechal_vdenc_vp9_xe_hpm.h
    )
    set (TMP_ENC_SOURCES_
        ${TMP_ENC_SOURCES_}
        ${CMAKE_CURRENT_LIST_DIR}/codechal_vdenc_vp9_xe_hpm.cpp
    )
endif ()


set(SOURCES_
    ${SOURCES_}
    ${TMP_ENC_SOURCES_}
    ${TMP_DEC_SOURCES_}
)

set(HEADERS_
    ${HEADERS_}
    ${TMP_ENC_HEADERS_}
    ${TMP_DEC_HEADERS_}
) 

set(COMMON_SOURCES_
    ${COMMON_SOURCES_}
    ${TMP_ENC_SOURCES_}
    ${TMP_DEC_SOURCES_}
)

set(COMMON_HEADERS_
    ${COMMON_HEADERS_}
    ${TMP_ENC_HEADERS_}
    ${TMP_DEC_HEADERS_}
)

source_group( "CodecHal\\Encode" FILES ${TMP_ENC_SOURCES_} ${TMP_ENC_HEADERS_} )
source_group( "CodecHal\\Decode" FILES ${TMP_DEC_SOURCES_} ${TMP_DEC_HEADERS_} )

media_add_curr_to_include_path()
