# Copyright (c) 2020-2021, Intel Corporation
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

set(TMP_HEADERS_
    ${CMAKE_CURRENT_LIST_DIR}/codechal_setting.h
    ${CMAKE_CURRENT_LIST_DIR}/codechal_common.h
    ${CMAKE_CURRENT_LIST_DIR}/codechal_debug.h
    ${CMAKE_CURRENT_LIST_DIR}/codechal_debug_config_manager.h
)

set(TMP_1_SOURCES_
    ${CMAKE_CURRENT_LIST_DIR}/codechal_common.cpp
    ${CMAKE_CURRENT_LIST_DIR}/codechal_debug.cpp
    ${CMAKE_CURRENT_LIST_DIR}/codechal_debug_config_manager.cpp
)

if(NOT "${Media_Reserved}" STREQUAL "yes")
    set(TMP_1_SOURCES_
            ${TMP_1_SOURCES_}
            ${CMAKE_CURRENT_LIST_DIR}/codechal_setting.cpp
       )
endif()

set(SOURCES_
    ${SOURCES_}
    ${TMP_1_SOURCES_}
)

set(HEADERS_
    ${HEADERS_}
    ${TMP_HEADERS_}
)

set(COMMON_HEADERS_
    ${COMMON_HEADERS_}
    ${TMP_HEADERS_}
)

set(COMMON_SOURCES_
    ${COMMON_SOURCES_}
    ${TMP_1_SOURCES_}
)

source_group( "CodecHal\\Common" FILES ${TMP_1_SOURCES_} ${TMP_HEADERS_} )

media_add_curr_to_include_path()

media_include_subdirectory(dec)
media_include_subdirectory(enc)
media_include_subdirectory(shared)
