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

#encode

if("${HEVC_Encode_VDEnc_Supported}" STREQUAL "yes")
set(TMP_SOURCES_
    ${CMAKE_CURRENT_LIST_DIR}/encode_hevc_vdenc_roi_arb.cpp
    ${CMAKE_CURRENT_LIST_DIR}/encode_hevc_vdenc_roi_overlap.cpp
    ${CMAKE_CURRENT_LIST_DIR}/encode_hevc_vdenc_roi.cpp
    ${CMAKE_CURRENT_LIST_DIR}/encode_hevc_vdenc_roi_strategy.cpp
    ${CMAKE_CURRENT_LIST_DIR}/encode_hevc_vdenc_roi_huc_forceqp.cpp
    ${CMAKE_CURRENT_LIST_DIR}/encode_hevc_vdenc_roi_native.cpp
    ${CMAKE_CURRENT_LIST_DIR}/encode_hevc_vdenc_roi_dirty.cpp
    ${CMAKE_CURRENT_LIST_DIR}/encode_hevc_vdenc_roi_forceqp.cpp
    ${CMAKE_CURRENT_LIST_DIR}/encode_hevc_vdenc_roi_qpmap.cpp
    ${CMAKE_CURRENT_LIST_DIR}/encode_hevc_vdenc_roi_forcedeltaqp.cpp
)

set(TMP_HEADERS_
    ${CMAKE_CURRENT_LIST_DIR}/encode_hevc_vdenc_roi_arb.h
    ${CMAKE_CURRENT_LIST_DIR}/encode_hevc_vdenc_roi_overlap.h
    ${CMAKE_CURRENT_LIST_DIR}/encode_hevc_vdenc_roi.h
    ${CMAKE_CURRENT_LIST_DIR}/encode_hevc_vdenc_roi_strategy.h
    ${CMAKE_CURRENT_LIST_DIR}/encode_hevc_vdenc_roi_huc_forceqp.h
    ${CMAKE_CURRENT_LIST_DIR}/encode_hevc_vdenc_roi_native.h
    ${CMAKE_CURRENT_LIST_DIR}/encode_hevc_vdenc_roi_dirty.h
    ${CMAKE_CURRENT_LIST_DIR}/encode_hevc_vdenc_roi_forceqp.h
    ${CMAKE_CURRENT_LIST_DIR}/encode_hevc_vdenc_roi_qpmap.h
    ${CMAKE_CURRENT_LIST_DIR}/encode_hevc_vdenc_roi_forcedeltaqp.h
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
