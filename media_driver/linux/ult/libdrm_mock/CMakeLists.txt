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
cmake_minimum_required(VERSION 2.8)

project(libdrm_mock)

include_directories(../../common/os/i915/include ../inc)
aux_source_directory(. SOURCES)

if (MEDIA_EXT)
    list(REMOVE_ITEM SOURCES ./mos_bufmgr_mock.c)
    list(REMOVE_ITEM SOURCES ./libdrm_mock/xf86drm_mock.c)
    set(SOURCES
        ${MEDIA_EXT_ULT}/linux/mock/libdrm_mock/mos_bufmgr_mock.c
        ${MEDIA_EXT_ULT}/linux/mock/libdrm_mock/xf86drm_mock.c
)
endif ()

set_source_files_properties(${SOURCES} PROPERTIES LANGUAGE "CXX")
add_library(drm_mock SHARED ${SOURCES})

target_link_libraries(drm_mock pciaccess)
set_target_properties(drm_mock PROPERTIES VERSION 2.4.0 SOVERSION 2)
