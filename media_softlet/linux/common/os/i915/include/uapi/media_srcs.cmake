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
)

set(SOFTLET_MOS_COMMON_HEADERS_
    ${SOFTLET_MOS_COMMON_HEADERS_}
    ${TMP_HEADERS_}
)

#use 'BEFORE' as below to make this folder higher priority in include path. This could avoid libdrm headers in system path are used unproperly.
set (SOFTLET_MOS_PREPEND_INCLUDE_DIRS_
    ${CMAKE_CURRENT_LIST_DIR}
    ${SOFTLET_MOS_PREPEND_INCLUDE_DIRS_}
)

