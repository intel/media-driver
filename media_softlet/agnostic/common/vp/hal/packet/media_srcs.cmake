# Copyright (c) 2019-2022, Intel Corporation
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
    ${CMAKE_CURRENT_LIST_DIR}/vp_cmd_packet.cpp
    ${CMAKE_CURRENT_LIST_DIR}/vp_packet_pipe.cpp
    ${CMAKE_CURRENT_LIST_DIR}/vp_render_ief.cpp
    ${CMAKE_CURRENT_LIST_DIR}/vp_render_sfc_base.cpp
    ${CMAKE_CURRENT_LIST_DIR}/vp_vebox_cmd_packet.cpp
    ${CMAKE_CURRENT_LIST_DIR}/vp_vebox_cmd_packet_base.cpp
    ${CMAKE_CURRENT_LIST_DIR}/vp_render_kernel_obj.cpp
    ${CMAKE_CURRENT_LIST_DIR}/vp_render_cmd_packet.cpp
    ${CMAKE_CURRENT_LIST_DIR}/vp_kernel_config.cpp
    ${CMAKE_CURRENT_LIST_DIR}/vp_render_fc_kernel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/vp_render_vebox_hdr_3dlut_kernel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/vp_render_vebox_hvs_kernel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/vp_render_hdr_kernel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/vp_render_vebox_hdr_3dlut_l0_kernel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/vp_frametracker.cpp
    ${CMAKE_CURRENT_LIST_DIR}/vp_render_ocl_fc_kernel.cpp
)

set(TMP_HEADERS_
    ${CMAKE_CURRENT_LIST_DIR}/vp_cmd_packet.h
    ${CMAKE_CURRENT_LIST_DIR}/vp_packet_pipe.h
    ${CMAKE_CURRENT_LIST_DIR}/vp_render_ief.h
    ${CMAKE_CURRENT_LIST_DIR}/vp_render_sfc_base.h
    ${CMAKE_CURRENT_LIST_DIR}/vp_vebox_cmd_packet.h
    ${CMAKE_CURRENT_LIST_DIR}/vp_vebox_cmd_packet_base.h
    ${CMAKE_CURRENT_LIST_DIR}/vp_sfc_common.h
    ${CMAKE_CURRENT_LIST_DIR}/vp_vebox_common.h
    ${CMAKE_CURRENT_LIST_DIR}/vp_render_common.h
    ${CMAKE_CURRENT_LIST_DIR}/vp_packet_shared_context.h
    ${CMAKE_CURRENT_LIST_DIR}/vp_render_kernel_obj.h
    ${CMAKE_CURRENT_LIST_DIR}/vp_render_cmd_packet.h
    ${CMAKE_CURRENT_LIST_DIR}/vp_kernel_config.h
    ${CMAKE_CURRENT_LIST_DIR}/vp_render_fc_kernel.h
    ${CMAKE_CURRENT_LIST_DIR}/vp_render_fc_types.h
    ${CMAKE_CURRENT_LIST_DIR}/vp_render_vebox_hdr_3dlut_kernel.h
    ${CMAKE_CURRENT_LIST_DIR}/vp_render_vebox_hvs_kernel.h
    ${CMAKE_CURRENT_LIST_DIR}/vp_render_hdr_kernel.h
    ${CMAKE_CURRENT_LIST_DIR}/vp_render_vebox_hdr_3dlut_l0_kernel.h
    ${CMAKE_CURRENT_LIST_DIR}/vp_frametracker.h
    ${CMAKE_CURRENT_LIST_DIR}/vp_render_ocl_fc_kernel.h
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
