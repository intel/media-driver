# Copyright (c) 2020, Intel Corporation
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

if("${HEVC_Encode_VDEnc_Supported}" STREQUAL "yes")
media_include_subdirectory(roi)
set(TMP_SOURCES_
    ${CMAKE_CURRENT_LIST_DIR}/encode_hevc_header_packer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/encode_hevc_vdenc_feature_manager.cpp
    ${CMAKE_CURRENT_LIST_DIR}/encode_hevc_cqp.cpp
    ${CMAKE_CURRENT_LIST_DIR}/encode_hevc_vdenc_const_settings.cpp
    ${CMAKE_CURRENT_LIST_DIR}/encode_hevc_brc.cpp
    ${CMAKE_CURRENT_LIST_DIR}/encode_hevc_basic_feature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/encode_hevc_vdenc_weighted_prediction.cpp
    ${CMAKE_CURRENT_LIST_DIR}/encode_hevc_tile.cpp
    ${CMAKE_CURRENT_LIST_DIR}/encode_hevc_dss.cpp
    ${CMAKE_CURRENT_LIST_DIR}/encode_hevc_vdenc_scc.cpp
    ${CMAKE_CURRENT_LIST_DIR}/encode_vdenc_lpla_analysis.cpp
    ${CMAKE_CURRENT_LIST_DIR}/encode_hevc_vdenc_lpla_enc.cpp
    ${CMAKE_CURRENT_LIST_DIR}/encode_hevc_basic_feature_422.cpp
    ${CMAKE_CURRENT_LIST_DIR}/encode_hevc_vdenc_fullenc.cpp
    ${CMAKE_CURRENT_LIST_DIR}/encode_hevc_vdenc_preenc.cpp
    ${CMAKE_CURRENT_LIST_DIR}/encode_hevc_aqm.cpp
    ${CMAKE_CURRENT_LIST_DIR}/encode_vdenc_hevc_fastpass.cpp
)

set(TMP_HEADERS_
    ${CMAKE_CURRENT_LIST_DIR}/encode_hevc_header_packer.h
    ${CMAKE_CURRENT_LIST_DIR}/encode_hevc_vdenc_feature_manager.h
    ${CMAKE_CURRENT_LIST_DIR}/encode_hevc_cqp.h
    ${CMAKE_CURRENT_LIST_DIR}/encode_hevc_vdenc_const_settings.h
    ${CMAKE_CURRENT_LIST_DIR}/encode_hevc_brc.h
    ${CMAKE_CURRENT_LIST_DIR}/encode_hevc_basic_feature.h
    ${CMAKE_CURRENT_LIST_DIR}/encode_hevc_vdenc_weighted_prediction.h
    ${CMAKE_CURRENT_LIST_DIR}/encode_hevc_tile.h
    ${CMAKE_CURRENT_LIST_DIR}/encode_hevc_dss.h
    ${CMAKE_CURRENT_LIST_DIR}/encode_hevc_vdenc_scc.h
    ${CMAKE_CURRENT_LIST_DIR}/encode_vdenc_lpla_analysis.h
    ${CMAKE_CURRENT_LIST_DIR}/encode_hevc_vdenc_lpla_enc.h
    ${CMAKE_CURRENT_LIST_DIR}/encode_hevc_basic_feature_422.h
    ${CMAKE_CURRENT_LIST_DIR}/encode_hevc_vdenc_fullenc.h
    ${CMAKE_CURRENT_LIST_DIR}/encode_hevc_vdenc_preenc.h
    ${CMAKE_CURRENT_LIST_DIR}/encode_hevc_aqm.h
    ${CMAKE_CURRENT_LIST_DIR}/encode_vdenc_hevc_fastpass.h
)

set(SOFTLET_ENCODE_HEVC_HEADERS_
    ${SOFTLET_ENCODE_HEVC_HEADERS_}
    ${TMP_HEADERS_}
)

set(SOFTLET_ENCODE_HEVC_SOURCES_
    ${SOFTLET_ENCODE_HEVC_SOURCES_}
    ${TMP_SOURCES_}
)

source_group( CodecHalNext\\Shared\\Encode FILES ${TMP_SOURCES_} ${TMP_HEADERS_} )

set(TMP_SOURCES_ "")
set(TMP_HEADERS_ "")

endif()

set(SOFTLET_ENCODE_HEVC_PRIVATE_INCLUDE_DIRS_
    ${SOFTLET_ENCODE_HEVC_PRIVATE_INCLUDE_DIRS_}
    ${CMAKE_CURRENT_LIST_DIR}
)
