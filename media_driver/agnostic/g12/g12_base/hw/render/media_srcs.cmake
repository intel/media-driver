# Copyright (c) 2021, Intel Corporation

# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
# OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
# ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
# OTHER DEALINGS IN THE SOFTWARE.

set(TMP_RENDER_SOURCES_
    ${CMAKE_CURRENT_LIST_DIR}/mhw_render_g12_X.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mhw_render_hwcmd_g12_X.cpp
)

set(TMP_RENDER_HEADERS_
    ${CMAKE_CURRENT_LIST_DIR}/mhw_render_g12_X.h
    ${CMAKE_CURRENT_LIST_DIR}/mhw_render_hwcmd_g12_X.h
)


set(COMMON_SOURCES_
    ${COMMON_SOURCES_}
    ${TMP_RENDER_SOURCES_}
)

set(COMMON_HEADERS_
    ${COMMON_HEADERS_}
    ${TMP_RENDER_HEADERS_}
)

source_group("MHW\\Render Engine" FILES ${TMP_RENDER_SOURCES_} ${TMP_RENDER_HEADERS_})

set(COMMON_PRIVATE_INCLUDE_DIRS_
    ${COMMON_PRIVATE_INCLUDE_DIRS_}
    ${CMAKE_CURRENT_LIST_DIR}
)