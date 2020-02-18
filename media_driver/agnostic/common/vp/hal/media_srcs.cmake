# Copyright (c) 2019, Intel Corporation
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
    ${CMAKE_CURRENT_LIST_DIR}/vphal.cpp
    ${CMAKE_CURRENT_LIST_DIR}/vphal_common.c
    ${CMAKE_CURRENT_LIST_DIR}/vphal_ddi.c
    ${CMAKE_CURRENT_LIST_DIR}/vphal_debug.c
    ${CMAKE_CURRENT_LIST_DIR}/vphal_render_common.c
    ${CMAKE_CURRENT_LIST_DIR}/vphal_render_composite.cpp
    ${CMAKE_CURRENT_LIST_DIR}/vphal_render_ief.cpp
    ${CMAKE_CURRENT_LIST_DIR}/vphal_render_renderstate.cpp
    ${CMAKE_CURRENT_LIST_DIR}/vphal_render_sfc_base.cpp
    ${CMAKE_CURRENT_LIST_DIR}/vphal_render_vebox_base.cpp
    ${CMAKE_CURRENT_LIST_DIR}/vphal_render_vebox_iecp.cpp
    ${CMAKE_CURRENT_LIST_DIR}/vphal_render_vebox_procamp.cpp
    ${CMAKE_CURRENT_LIST_DIR}/vphal_render_vebox_ste.cpp
    ${CMAKE_CURRENT_LIST_DIR}/vphal_render_vebox_tcc.cpp
    ${CMAKE_CURRENT_LIST_DIR}/vphal_render_vebox_util_base.c
    ${CMAKE_CURRENT_LIST_DIR}/vphal_renderer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/vphal_mdf_wrapper.cpp
    ${CMAKE_CURRENT_LIST_DIR}/vphal_render_16alignment.cpp
    ${CMAKE_CURRENT_LIST_DIR}/vphal_render_fast1ton.cpp
    ${CMAKE_CURRENT_LIST_DIR}/vphal_render_vebox_denoise.cpp
    ${CMAKE_CURRENT_LIST_DIR}/vphal_render_hdr_base.cpp
    ${CMAKE_CURRENT_LIST_DIR}/vphal_render_vebox_memdecomp.cpp
)

set(TMP_HEADERS_
    ${CMAKE_CURRENT_LIST_DIR}/vphal.h
    ${CMAKE_CURRENT_LIST_DIR}/vphal_common.h
    ${CMAKE_CURRENT_LIST_DIR}/vphal_common_tools.h
    ${CMAKE_CURRENT_LIST_DIR}/vphal_ddi.h
    ${CMAKE_CURRENT_LIST_DIR}/vphal_debug.h
    ${CMAKE_CURRENT_LIST_DIR}/vphal_render_common.h
    ${CMAKE_CURRENT_LIST_DIR}/vphal_render_composite.h
    ${CMAKE_CURRENT_LIST_DIR}/vphal_render_ief.h
    ${CMAKE_CURRENT_LIST_DIR}/vphal_render_renderstate.h
    ${CMAKE_CURRENT_LIST_DIR}/vphal_render_sfc_base.h
    ${CMAKE_CURRENT_LIST_DIR}/vphal_render_vebox_base.h
    ${CMAKE_CURRENT_LIST_DIR}/vphal_render_vebox_iecp.h
    ${CMAKE_CURRENT_LIST_DIR}/vphal_render_vebox_procamp.h
    ${CMAKE_CURRENT_LIST_DIR}/vphal_render_vebox_ste.h
    ${CMAKE_CURRENT_LIST_DIR}/vphal_render_vebox_tcc.h
    ${CMAKE_CURRENT_LIST_DIR}/vphal_render_vebox_util_base.h
    ${CMAKE_CURRENT_LIST_DIR}/vphal_renderer.h
    ${CMAKE_CURRENT_LIST_DIR}/vphal_mdf_wrapper.h
    ${CMAKE_CURRENT_LIST_DIR}/vphal_render_16alignment.h
    ${CMAKE_CURRENT_LIST_DIR}/vphal_render_fast1ton.h
    ${CMAKE_CURRENT_LIST_DIR}/vphal_common_hdr.h
    ${CMAKE_CURRENT_LIST_DIR}/vphal_render_vebox_denoise.h
    ${CMAKE_CURRENT_LIST_DIR}/vphal_render_hdr_base.h
    ${CMAKE_CURRENT_LIST_DIR}/vphal_render_vebox_memdecomp.h
)


set(SOURCES_
    ${SOURCES_}
    ${TMP_SOURCES_}
)

set(HEADERS_
    ${HEADERS_}
    ${TMP_HEADERS_}
)

source_group("VpHal\\Common" FILES ${TMP_SOURCES_} ${TMP_HEADERS_})


media_add_curr_to_include_path()
