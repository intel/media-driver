# Copyright (c) 2018-2020, Intel Corporation
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

media_include_subdirectory(roi)

if("${AVC_Encode_VDEnc_Supported}" STREQUAL "yes")
set(TMP_SOURCES_
    ${CMAKE_CURRENT_LIST_DIR}/encode_avc_basic_feature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/encode_avc_brc.cpp
    ${CMAKE_CURRENT_LIST_DIR}/encode_avc_rounding.cpp
    ${CMAKE_CURRENT_LIST_DIR}/encode_avc_trellis.cpp
    ${CMAKE_CURRENT_LIST_DIR}/encode_avc_vdenc_stream_in_feature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/encode_avc_vdenc_const_settings.cpp
    ${CMAKE_CURRENT_LIST_DIR}/encode_avc_header_packer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/encode_avc_vdenc_weighted_prediction.cpp
    ${CMAKE_CURRENT_LIST_DIR}/encode_avc_vdenc_feature_manager.cpp
    ${CMAKE_CURRENT_LIST_DIR}/encode_avc_vdenc_preenc.cpp
    ${CMAKE_CURRENT_LIST_DIR}/encode_avc_vdenc_fullenc.cpp
    ${CMAKE_CURRENT_LIST_DIR}/encode_avc_aqm.cpp
    ${CMAKE_CURRENT_LIST_DIR}/encode_avc_vdenc_fastpass.cpp
)

set(TMP_HEADERS_
    ${CMAKE_CURRENT_LIST_DIR}/encode_avc_basic_feature.h
    ${CMAKE_CURRENT_LIST_DIR}/encode_avc_brc.h
    ${CMAKE_CURRENT_LIST_DIR}/encode_avc_rounding.h
    ${CMAKE_CURRENT_LIST_DIR}/encode_avc_trellis.h
    ${CMAKE_CURRENT_LIST_DIR}/encode_avc_vdenc_stream_in_feature.h
    ${CMAKE_CURRENT_LIST_DIR}/encode_avc_vdenc_const_settings.h
    ${CMAKE_CURRENT_LIST_DIR}/encode_avc_header_packer.h
    ${CMAKE_CURRENT_LIST_DIR}/encode_avc_vdenc_weighted_prediction.h
    ${CMAKE_CURRENT_LIST_DIR}/encode_avc_vdenc_feature_manager.h
    ${CMAKE_CURRENT_LIST_DIR}/media_avc_feature_defs.h
    ${CMAKE_CURRENT_LIST_DIR}/encode_avc_vdenc_preenc.h
    ${CMAKE_CURRENT_LIST_DIR}/encode_avc_vdenc_fullenc.h
    ${CMAKE_CURRENT_LIST_DIR}/encode_avc_aqm.h
    ${CMAKE_CURRENT_LIST_DIR}/encode_avc_vdenc_fastpass.h
)

set(SOFTLET_ENCODE_AVC_HEADERS_
    ${SOFTLET_ENCODE_AVC_HEADERS_}
    ${TMP_HEADERS_}
)

set(SOFTLET_ENCODE_AVC_SOURCES_
    ${SOFTLET_ENCODE_AVC_SOURCES_}
    ${TMP_SOURCES_}
)

source_group( CodecHalNext\\Shared\\Encode FILES ${TMP_SOURCES_} ${TMP_HEADERS_} )

set(TMP_SOURCES_ "")
set(TMP_HEADERS_ "")

endif()

set(SOFTLET_ENCODE_AVC_PRIVATE_INCLUDE_DIRS_
    ${SOFTLET_ENCODE_AVC_PRIVATE_INCLUDE_DIRS_}
    ${CMAKE_CURRENT_LIST_DIR}
)