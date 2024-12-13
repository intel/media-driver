# Copyright (c) 2024, Intel Corporation
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

set(TMP_SOURCES_
    ${CMAKE_CURRENT_LIST_DIR}/media_copy_xe2_lpm.cpp
    ${CMAKE_CURRENT_LIST_DIR}/media_vebox_copy_xe2_lpm.cpp
    ${CMAKE_CURRENT_LIST_DIR}/media_render_copy_xe2_lpm.cpp
    ${CMAKE_CURRENT_LIST_DIR}/media_blt_copy_xe2_lpm.cpp
)


set(TMP_HEADERS_
    ${CMAKE_CURRENT_LIST_DIR}/media_copy_xe2_lpm.h
    ${CMAKE_CURRENT_LIST_DIR}/media_vebox_copy_xe2_lpm.h
    ${CMAKE_CURRENT_LIST_DIR}/media_render_copy_xe2_lpm.h
    ${CMAKE_CURRENT_LIST_DIR}/media_blt_copy_xe2_lpm.h
)

set(SOFTLET_COMMON_SOURCES_
    ${SOFTLET_COMMON_SOURCES_}
    ${TMP_SOURCES_}
)

set(SOFTLET_COMMON_HEADERS_
    ${SOFTLET_COMMON_HEADERS_}
    ${TMP_HEADERS_}
)

source_group( "SharedNext\\Xe2_LPM_base\\shared\\MediaCopy" FILES ${TMP_SOURCES_} ${TMP_HEADERS_} )

set(SOFTLET_COMMON_PRIVATE_INCLUDE_DIRS_
    ${SOFTLET_COMMON_PRIVATE_INCLUDE_DIRS_}
    ${CMAKE_CURRENT_LIST_DIR}
)

media_add_curr_to_include_path()
