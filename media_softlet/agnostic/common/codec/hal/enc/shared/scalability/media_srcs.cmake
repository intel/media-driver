# Copyright (c) 2018, Intel Corporation
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

if(${Common_Encode_Supported} STREQUAL "yes")
set(TMP_SOURCES_
    ${CMAKE_CURRENT_LIST_DIR}/encode_scalability_multipipe.cpp
    ${CMAKE_CURRENT_LIST_DIR}/encode_scalability_option.cpp
    ${CMAKE_CURRENT_LIST_DIR}/encode_scalability_singlepipe.cpp
)

set(TMP_HEADERS_
    ${CMAKE_CURRENT_LIST_DIR}/encode_scalability_defs.h
    ${CMAKE_CURRENT_LIST_DIR}/encode_scalability_multipipe.h
    ${CMAKE_CURRENT_LIST_DIR}/encode_scalability_option.h
    ${CMAKE_CURRENT_LIST_DIR}/encode_scalability_singlepipe.h
)

set(SOFTLET_ENCODE_COMMON_HEADERS_
    ${SOFTLET_ENCODE_COMMON_HEADERS_}
    ${TMP_HEADERS_}
)

set(SOFTLET_ENCODE_COMMON_SOURCES_
    ${SOFTLET_ENCODE_COMMON_SOURCES_}
    ${TMP_SOURCES_}
)

source_group( CodecHalNext\\Shared\\Encode FILES ${TMP_SOURCES_} ${TMP_HEADERS_} )

set(TMP_SOURCES_ "")
set(TMP_HEADERS_ "")

endif()

set(SOFTLET_ENCODE_COMMON_PRIVATE_INCLUDE_DIRS_
    ${SOFTLET_ENCODE_COMMON_PRIVATE_INCLUDE_DIRS_}
    ${CMAKE_CURRENT_LIST_DIR}
)