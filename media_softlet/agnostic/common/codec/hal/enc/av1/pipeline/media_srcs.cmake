# Copyright (c) 2022, Intel Corporation
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

if ("${AV1_Encode_VDEnc_Supported}" STREQUAL "yes")
set(TMP_SOURCES_
    ${CMAKE_CURRENT_LIST_DIR}/encode_av1_pipeline.cpp
    ${CMAKE_CURRENT_LIST_DIR}/encode_av1_vdenc_pipeline.cpp
    ${CMAKE_CURRENT_LIST_DIR}/encode_av1_reference_frames.cpp
    ${CMAKE_CURRENT_LIST_DIR}/encode_av1_user_setting.cpp
)

set(TMP_HEADERS_
    ${CMAKE_CURRENT_LIST_DIR}/encode_av1_pipeline.h
    ${CMAKE_CURRENT_LIST_DIR}/encode_av1_vdenc_pipeline.h
    ${CMAKE_CURRENT_LIST_DIR}/encode_av1_reference_frames.h
)

set(SOFTLET_ENCODE_AV1_HEADERS_
    ${SOFTLET_ENCODE_AV1_HEADERS_}
    ${TMP_HEADERS_}
)

set(SOFTLET_ENCODE_AV1_SOURCES_
    ${SOFTLET_ENCODE_AV1_SOURCES_}
    ${TMP_SOURCES_}
)

source_group( CodecHalNext\\Shared\\Encode FILES ${TMP_SOURCES_} ${TMP_HEADERS_} )

set(TMP_SOURCES_ "")
set(TMP_HEADERS_ "")

endif()

set(SOFTLET_ENCODE_AV1_PRIVATE_INCLUDE_DIRS_
    ${SOFTLET_ENCODE_AV1_PRIVATE_INCLUDE_DIRS_}
    ${CMAKE_CURRENT_LIST_DIR}
)

media_add_curr_to_include_path()