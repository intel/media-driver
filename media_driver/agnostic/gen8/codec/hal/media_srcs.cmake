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
    ${CMAKE_CURRENT_LIST_DIR}/codechal_hw_g8_X.cpp
)

set(TMP_1_HEADERS_
    ${CMAKE_CURRENT_LIST_DIR}/codechal_hw_g8_X.h
)


#decode
set(TMP_2_SOURCES_ "")

set(TMP_2_HEADERS_ "")

if(${VC1_Decode_Supported} STREQUAL "yes")
    set(TMP_2_SOURCES_
        ${TMP_2_SOURCES_}
        ${CMAKE_CURRENT_LIST_DIR}/codechal_decode_vc1_g8.cpp
    )
    set(TMP_2_HEADERS_
        ${TMP_2_HEADERS_}
        ${CMAKE_CURRENT_LIST_DIR}/codechal_decode_vc1_g8.h
    )
endif()


# encode
if(${Common_Encode_Supported} STREQUAL "yes")
    set(TMP_3_SOURCES_
        ${CMAKE_CURRENT_LIST_DIR}/codechal_encode_csc_ds_g8.cpp
        ${CMAKE_CURRENT_LIST_DIR}/codechal_kernel_hme_g8.cpp
        ${CMAKE_CURRENT_LIST_DIR}/codechal_encode_wp_g8.cpp
    )

    set(TMP_3_HEADERS_
        ${CMAKE_CURRENT_LIST_DIR}/codechal_encode_csc_ds_g8.h
        ${CMAKE_CURRENT_LIST_DIR}/codechal_kernel_hme_g8.h
        ${CMAKE_CURRENT_LIST_DIR}/codechal_encode_wp_g8.h
    )
endif()

if ("${AVC_Encode_VME_Supported}" STREQUAL "yes" OR "${AVC_Encode_VDEnc_Supported}" STREQUAL "yes")
    if ("${AVC_Encode_VME_Supported}" STREQUAL "yes")
        set (TMP_3_SOURCES_
            ${TMP_3_SOURCES_}
            ${CMAKE_CURRENT_LIST_DIR}/codechal_encode_avc_g8.cpp
            ${CMAKE_CURRENT_LIST_DIR}/codechal_fei_avc_g8.cpp
        )
        set (TMP_3_HEADERS_
            ${TMP_3_HEADERS_}
            ${CMAKE_CURRENT_LIST_DIR}/codechal_encode_avc_g8.h
            ${CMAKE_CURRENT_LIST_DIR}/codechal_fei_avc_g8.h
        )
    endif ()
endif ()

if(${MPEG2_Encode_VME_Supported} STREQUAL "yes")
    set(TMP_3_SOURCES_
        ${TMP_3_SOURCES_}
        ${CMAKE_CURRENT_LIST_DIR}/codechal_encode_mpeg2_g8.cpp
    )
    set(TMP_3_HEADERS_
        ${TMP_3_HEADERS_}
        ${CMAKE_CURRENT_LIST_DIR}/codechal_encode_mpeg2_g8.h
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
