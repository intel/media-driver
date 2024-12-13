# Copyright (c) 2021-2024, Intel Corporation
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

if(${AV1_Decode_Supported} STREQUAL "yes")
set(SOFTLET_DECODE_AV1_SOURCES_
    ${SOFTLET_DECODE_AV1_SOURCES_}
    ${CMAKE_CURRENT_LIST_DIR}/decode_av1_packet_xe2_lpm_base.cpp
    ${CMAKE_CURRENT_LIST_DIR}/decode_av1_picture_packet_xe2_lpm_base.cpp
    ${CMAKE_CURRENT_LIST_DIR}/decode_av1_tile_packet_xe2_lpm_base.cpp
    ${CMAKE_CURRENT_LIST_DIR}/decode_av1_downsampling_packet_xe2_lpm_base.cpp
)

set(SOFTLET_DECODE_AV1_HEADERS_
    ${SOFTLET_DECODE_AV1_HEADERS_}
    ${CMAKE_CURRENT_LIST_DIR}/decode_av1_packet_xe2_lpm_base.h
    ${CMAKE_CURRENT_LIST_DIR}/decode_av1_picture_packet_xe2_lpm_base.h
    ${CMAKE_CURRENT_LIST_DIR}/decode_av1_tile_packet_xe2_lpm_base.h
    ${CMAKE_CURRENT_LIST_DIR}/decode_av1_downsampling_packet_xe2_lpm_base.h
)


source_group( CodecHalNext\\Xe2_LPM_base\\Decode FILES ${SOFTLET_DECODE_AV1_SOURCES_} ${SOFTLET_DECODE_AV1_HEADERS_})

media_add_curr_to_include_path()

endif()


set(SOFTLET_DECODE_AV1_PRIVATE_INCLUDE_DIRS_
    ${SOFTLET_DECODE_AV1_PRIVATE_INCLUDE_DIRS_}
    ${CMAKE_CURRENT_LIST_DIR}
)
