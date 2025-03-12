# Copyright (c) 2019-2024, Intel Corporation
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

media_include_subdirectory(kernel_args)

set(TMP_SOURCES_
    ${CMAKE_CURRENT_LIST_DIR}/vp_csc_filter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/vp_filter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/vp_rot_mir_filter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/vp_scaling_filter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/vp_dn_filter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/vp_ste_filter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/vp_tcc_filter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/vp_procamp_filter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/vp_hdr_filter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/vp_hdr_render_filter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/vp_di_filter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/vp_fc_filter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/vp_cgc_filter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/vp_ocl_fc_filter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/vp_fc_wrap_filter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/vp_ai_filter.cpp
)

set(TMP_HEADERS_
    ${CMAKE_CURRENT_LIST_DIR}/vp_csc_filter.h
    ${CMAKE_CURRENT_LIST_DIR}/vp_filter.h
    ${CMAKE_CURRENT_LIST_DIR}/vp_rot_mir_filter.h
    ${CMAKE_CURRENT_LIST_DIR}/vp_scaling_filter.h
    ${CMAKE_CURRENT_LIST_DIR}/vp_dn_filter.h
    ${CMAKE_CURRENT_LIST_DIR}/vp_ste_filter.h
    ${CMAKE_CURRENT_LIST_DIR}/vp_tcc_filter.h
    ${CMAKE_CURRENT_LIST_DIR}/vp_procamp_filter.h
    ${CMAKE_CURRENT_LIST_DIR}/vp_hdr_filter.h
    ${CMAKE_CURRENT_LIST_DIR}/vp_hdr_render_filter.h
    ${CMAKE_CURRENT_LIST_DIR}/vp_di_filter.h
    ${CMAKE_CURRENT_LIST_DIR}/vp_fc_filter.h
    ${CMAKE_CURRENT_LIST_DIR}/vp_cgc_filter.h
    ${CMAKE_CURRENT_LIST_DIR}/vp_ocl_fc_filter.h
    ${CMAKE_CURRENT_LIST_DIR}/vp_fc_wrap_filter.h
    ${CMAKE_CURRENT_LIST_DIR}/vp_ai_filter.h
)

set(SOFTLET_VP_SOURCES_
    ${SOFTLET_VP_SOURCES_}
    ${TMP_SOURCES_}
)

set(SOFTLET_VP_HEADERS_
    ${SOFTLET_VP_HEADERS_}
    ${TMP_HEADERS_}
)

source_group( VpHalNext\\Shared FILES ${TMP_SOURCES_} ${TMP_HEADERS_})
set(TMP_SOURCES_ "")
set(TMP_HEADERS_ "")
set (SOFTLET_VP_PRIVATE_INCLUDE_DIRS_
    ${SOFTLET_VP_PRIVATE_INCLUDE_DIRS_}
    ${CMAKE_CURRENT_LIST_DIR}
)