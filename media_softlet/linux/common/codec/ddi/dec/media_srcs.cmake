# Copyright (c) 2021-2022, Intel Corporation
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

set(TMP_SOURCES_
    ${CMAKE_CURRENT_LIST_DIR}/ddi_decode_functions.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ddi_decode_base_specific.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ddi_decode_trace_specific.cpp
)

set(TMP_HEADERS_
    ${CMAKE_CURRENT_LIST_DIR}/ddi_decode_functions.h
    ${CMAKE_CURRENT_LIST_DIR}/ddi_decode_base_specific.h
    ${CMAKE_CURRENT_LIST_DIR}/ddi_decode_trace_specific.h
)

if(${AVC_Decode_Supported} STREQUAL "yes")
    set(TMP_SOURCES_
        ${TMP_SOURCES_}
        ${CMAKE_CURRENT_LIST_DIR}/ddi_decode_avc_specific.cpp
    )
    set(TMP_HEADERS_
        ${TMP_HEADERS_}
        ${CMAKE_CURRENT_LIST_DIR}/ddi_decode_avc_specific.h
    )
endif()

if(${HEVC_Decode_Supported} STREQUAL "yes")
    set(TMP_SOURCES_
        ${TMP_SOURCES_}
        ${CMAKE_CURRENT_LIST_DIR}/ddi_decode_hevc_specific.cpp
    )
    set(TMP_HEADERS_
        ${TMP_HEADERS_}
        ${CMAKE_CURRENT_LIST_DIR}/ddi_decode_hevc_specific.h
    )
endif()

if(${JPEG_Decode_Supported} STREQUAL "yes")
    set(TMP_SOURCES_
        ${TMP_SOURCES_}
        ${CMAKE_CURRENT_LIST_DIR}/ddi_decode_jpeg_specific.cpp
    )
    set(TMP_HEADERS_
        ${TMP_HEADERS_}
        ${CMAKE_CURRENT_LIST_DIR}/ddi_decode_jpeg_specific.h
    )
endif()

if(${MPEG2_Decode_Supported} STREQUAL "yes")
    set(TMP_SOURCES_
        ${TMP_SOURCES_}
        ${CMAKE_CURRENT_LIST_DIR}/ddi_decode_mpeg2_specific.cpp
    )
    set(TMP_HEADERS_
        ${TMP_HEADERS_}
        ${CMAKE_CURRENT_LIST_DIR}/ddi_decode_mpeg2_specific.h
    )
endif()

if(${VP8_Decode_Supported} STREQUAL "yes")
    set(TMP_SOURCES_
        ${TMP_SOURCES_}
        ${CMAKE_CURRENT_LIST_DIR}/ddi_decode_vp8_specific.cpp
    )
    set(TMP_HEADERS_
        ${TMP_HEADERS_}
        ${CMAKE_CURRENT_LIST_DIR}/ddi_decode_vp8_specific.h
    )
endif()

if(${VP9_Decode_Supported} STREQUAL "yes")
    set(TMP_SOURCES_
        ${TMP_SOURCES_}
        ${CMAKE_CURRENT_LIST_DIR}/ddi_decode_vp9_specific.cpp
    )
    set(TMP_HEADERS_
        ${TMP_HEADERS_}
        ${CMAKE_CURRENT_LIST_DIR}/ddi_decode_vp9_specific.h
    )
endif()

if(${AV1_Decode_Supported} STREQUAL "yes")
    set(TMP_SOURCES_
        ${TMP_SOURCES_}
        ${CMAKE_CURRENT_LIST_DIR}/ddi_decode_av1_specific.cpp
    )
    set(TMP_HEADERS_
        ${TMP_HEADERS_}
        ${CMAKE_CURRENT_LIST_DIR}/ddi_decode_av1_specific.h
    )
endif()

if(${VVC_Decode_Supported} STREQUAL "yes")
    set(TMP_SOURCES_
        ${TMP_SOURCES_}
        ${CMAKE_CURRENT_LIST_DIR}/ddi_decode_vvc_specific.cpp
    )
    set(TMP_HEADERS_
        ${TMP_HEADERS_}
        ${CMAKE_CURRENT_LIST_DIR}/ddi_decode_vvc_specific.h
    )
endif()

set(SOFTLET_DDI_SOURCES_
    ${SOFTLET_DDI_SOURCES_}
    ${TMP_SOURCES_}
 )

set(SOFTLET_DDI_HEADERS_
    ${SOFTLET_DDI_HEADERS_}
    ${TMP_HEADERS_}
)

set(SOFTLET_DDI_PUBLIC_INCLUDE_DIRS_
    ${SOFTLET_DDI_PUBLIC_INCLUDE_DIRS_}
    ${CMAKE_CURRENT_LIST_DIR}
)