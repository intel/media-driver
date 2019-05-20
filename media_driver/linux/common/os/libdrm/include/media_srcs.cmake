# Copyright (c) 2017, Intel Corporation
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

set(TMP_SOURCES_ "")

set(TMP_HEADERS_
    ${CMAKE_CURRENT_LIST_DIR}/drm.h
    ${CMAKE_CURRENT_LIST_DIR}/drm_fourcc.h
    ${CMAKE_CURRENT_LIST_DIR}/drm_mode.h
    ${CMAKE_CURRENT_LIST_DIR}/drm_sarea.h
    ${CMAKE_CURRENT_LIST_DIR}/i915_drm.h
    ${CMAKE_CURRENT_LIST_DIR}/i915_header.h
    ${CMAKE_CURRENT_LIST_DIR}/intel_aub.h
    ${CMAKE_CURRENT_LIST_DIR}/intel_chipset.h
    ${CMAKE_CURRENT_LIST_DIR}/libdrm_lists.h
    ${CMAKE_CURRENT_LIST_DIR}/libdrm_macros.h
    ${CMAKE_CURRENT_LIST_DIR}/mos_bufmgr.h
    ${CMAKE_CURRENT_LIST_DIR}/mos_bufmgr_priv.h
    ${CMAKE_CURRENT_LIST_DIR}/xf86atomic.h
    ${CMAKE_CURRENT_LIST_DIR}/xf86drm.h
    ${CMAKE_CURRENT_LIST_DIR}/xf86drmHash.h
    ${CMAKE_CURRENT_LIST_DIR}/xf86drmMode.h
    ${CMAKE_CURRENT_LIST_DIR}/xf86drmRandom.h
)

# no source in this folder
#set(SOURCES_
#    ${SOURCES_}
#    ${TMP_SOURCES_}
#)

set(HEADERS_
    ${HEADERS_}
    ${TMP_HEADERS_}
)


# use 'BEFORE' as below to make this folder higher priority in include path. This could avoid libdrm headers in system path are used unproperly.
include_directories(BEFORE ${CMAKE_CURRENT_LIST_DIR})
