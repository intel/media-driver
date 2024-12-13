# Copyright (c) 2019, Intel Corporation
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

if ("${VP9_Encode_VDEnc_Supported}" STREQUAL "yes")
set(TMP_SOURCES_
    ${CMAKE_CURRENT_LIST_DIR}/encode_vp9_vdenc_packet.cpp
    ${CMAKE_CURRENT_LIST_DIR}/encode_vp9_huc_brc_init_packet.cpp
    ${CMAKE_CURRENT_LIST_DIR}/encode_vp9_huc_brc_update_packet.cpp
    ${CMAKE_CURRENT_LIST_DIR}/encode_vp9_hpu_packet.cpp
    ${CMAKE_CURRENT_LIST_DIR}/encode_vp9_hpu_super_frame_packet.cpp
    ${CMAKE_CURRENT_LIST_DIR}/encode_vp9_pak_integrate_packet.cpp
)
set(TMP_HEADERS_
    ${CMAKE_CURRENT_LIST_DIR}/encode_vp9_vdenc_packet.h
    ${CMAKE_CURRENT_LIST_DIR}/encode_vp9_huc_brc_init_packet.h
    ${CMAKE_CURRENT_LIST_DIR}/encode_vp9_huc_brc_update_packet.h
    ${CMAKE_CURRENT_LIST_DIR}/encode_vp9_hpu_packet.h
    ${CMAKE_CURRENT_LIST_DIR}/encode_vp9_hpu_super_frame_packet.h
    ${CMAKE_CURRENT_LIST_DIR}/encode_vp9_pak_integrate_packet.h
    ${CMAKE_CURRENT_LIST_DIR}/media_vp9_packet_defs.h
)

set(SOFTLET_ENCODE_VP9_HEADERS_
    ${SOFTLET_ENCODE_VP9_HEADERS_}
    ${TMP_HEADERS_}
)

set(SOFTLET_ENCODE_VP9_SOURCES_
    ${SOFTLET_ENCODE_VP9_SOURCES_}
    ${TMP_SOURCES_}
)

source_group( CodecHalNext\\Shared\\Encode FILES ${TMP_SOURCES_} ${TMP_HEADERS_} )

set(TMP_SOURCES_ "")
set(TMP_HEADERS_ "")

endif()

set(SOFTLET_ENCODE_VP9_PRIVATE_INCLUDE_DIRS_
    ${SOFTLET_ENCODE_VP9_PRIVATE_INCLUDE_DIRS_}
    ${CMAKE_CURRENT_LIST_DIR}
)
media_add_curr_to_include_path()