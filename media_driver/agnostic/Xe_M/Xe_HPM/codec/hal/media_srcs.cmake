# Copyright (c) 2019-2022, Intel Corporation

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

if ("${AVC_Encode_VDEnc_Supported}" STREQUAL "yes")
    set (TMP_ENC_SOURCES_
        ${TMP_ENC_SOURCES_}
        ${CMAKE_CURRENT_LIST_DIR}/codechal_vdenc_avc_xe_hpm.cpp
    )
    set (TMP_ENC_HEADERS_
        ${TMP_ENC_HEADERS_}
        ${CMAKE_CURRENT_LIST_DIR}/codechal_vdenc_avc_xe_hpm.h
    )
endif ()

    set (TMP_ENC_SOURCES_
        ${TMP_ENC_SOURCES_}
        ${CMAKE_CURRENT_LIST_DIR}/codechal_hw_xe_hpm.cpp
    )
    set (TMP_ENC_HEADERS_
        ${TMP_ENC_HEADERS_}
        ${CMAKE_CURRENT_LIST_DIR}/codechal_hw_xe_hpm.h
    )

# mmc
if("${MMC_Supported}" STREQUAL "yes")
    set(TMP_ENC_SOURCES_
        ${TMP_ENC_SOURCES_}
        ${CMAKE_CURRENT_LIST_DIR}/codechal_mmc_encode_avc_xe_hpm.cpp
    )
    set(TMP_ENC_HEADERS_
        ${TMP_ENC_HEADERS_}
        ${CMAKE_CURRENT_LIST_DIR}/codechal_mmc_encode_avc_xe_hpm.h
    )
endif()

if ("${VP9_Encode_VDEnc_Supported}" STREQUAL "yes")
    set (TMP_ENC_SOURCES_
        ${TMP_ENC_SOURCES_}
        ${CMAKE_CURRENT_LIST_DIR}/codechal_vdenc_vp9_xe_hpm.cpp
    )
    set (TMP_ENC_HEADERS_
        ${TMP_ENC_HEADERS_}
        ${CMAKE_CURRENT_LIST_DIR}/codechal_vdenc_vp9_xe_hpm.h
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

set(CODEC_SOURCES_
    ${CODEC_SOURCES_}
    ${TMP_ENC_SOURCES_}
    ${TMP_DEC_SOURCES_}
)

set(CODEC_HEADERS_
    ${CODEC_HEADERS_}
    ${TMP_ENC_HEADERS_}
    ${TMP_DEC_HEADERS_}
)

source_group( "CodecHal\\Encode" FILES ${TMP_ENC_SOURCES_} ${TMP_ENC_HEADERS_} )
source_group( "CodecHal\\Decode" FILES ${TMP_DEC_SOURCES_} ${TMP_DEC_HEADERS_} )
set(TMP_ENC_SOURCES_ "")
set(TMP_ENC_HEADERS_ "")
set(TMP_DEC_SOURCES_ "")
set(TMP_DEC_HEADERS_ "")
media_add_curr_to_include_path()
