# Copyright (c) 2020-2022, Intel Corporation
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

media_include_subdirectory(vdbox)

set(TMP_HEADERS_
    ${CMAKE_CURRENT_LIST_DIR}/mhw_hwcmd.h
    ${CMAKE_CURRENT_LIST_DIR}/mhw_cmdpar.h
    ${CMAKE_CURRENT_LIST_DIR}/mhw_itf.h
    ${CMAKE_CURRENT_LIST_DIR}/mhw_impl.h
    ${CMAKE_CURRENT_LIST_DIR}/mhw_hwcmd_process_cmdfields.h
    ${CMAKE_CURRENT_LIST_DIR}/mhw_utilities_next.h
)

set(SOFTLET_MHW_COMMON_HEADERS_
    ${SOFTLET_MHW_COMMON_HEADERS_}
    ${TMP_HEADERS_}
)

set(TMP_RENDER_HEADERS_
    ${CMAKE_CURRENT_LIST_DIR}/mhw_render_cmdpar.h
    ${CMAKE_CURRENT_LIST_DIR}/mhw_render_impl.h
    ${CMAKE_CURRENT_LIST_DIR}/mhw_render_itf.h
)

set(SOFTLET_MHW_RENDER_HEADERS_
    ${SOFTLET_MHW_RENDER_HEADERS_}
    ${TMP_RENDER_HEADERS_}
)

set(TMP_MI_HEADERS_
    ${CMAKE_CURRENT_LIST_DIR}/mhw_mi_cmdpar.h
    ${CMAKE_CURRENT_LIST_DIR}/mhw_mi_impl.h
    ${CMAKE_CURRENT_LIST_DIR}/mhw_mi_itf.h
    ${CMAKE_CURRENT_LIST_DIR}/mhw_mmio_common.h
)

set(SOFTLET_MHW_MI_HEADERS_
    ${SOFTLET_MHW_MI_HEADERS_}
    ${TMP_MI_HEADERS_}
)

set(TMP_VEBOX_HEADERS_
    ${CMAKE_CURRENT_LIST_DIR}/mhw_vebox_cmdpar.h
    ${CMAKE_CURRENT_LIST_DIR}/mhw_vebox_impl.h
    ${CMAKE_CURRENT_LIST_DIR}/mhw_vebox_itf.h
)

set(SOFTLET_MHW_VEBOX_HEADERS_
    ${SOFTLET_MHW_VEBOX_HEADERS_}
    ${TMP_VEBOX_HEADERS_}
)

set(TMP_SFC_HEADERS_
    ${CMAKE_CURRENT_LIST_DIR}/mhw_sfc_cmdpar.h
    ${CMAKE_CURRENT_LIST_DIR}/mhw_sfc_impl.h
    ${CMAKE_CURRENT_LIST_DIR}/mhw_sfc_itf.h
)

set(SOFTLET_MHW_SFC_HEADERS_
    ${SOFTLET_MHW_SFC_HEADERS_}
    ${TMP_SFC_HEADERS_}
)

set(TMP_BLT_HEADERS_
    ${CMAKE_CURRENT_LIST_DIR}/mhw_blt_cmdpar.h
    ${CMAKE_CURRENT_LIST_DIR}/mhw_blt_impl.h
    ${CMAKE_CURRENT_LIST_DIR}/mhw_blt_itf.h
)

set(TMP_BLT_SOURCES_
    ${CMAKE_CURRENT_LIST_DIR}/mhw_blt.cpp
)

set(SOFTLET_MHW_BLT_HEADERS_
    ${SOFTLET_MHW_BLT_HEADERS_}
    ${TMP_BLT_HEADERS_}
)

set(SOFTLET_MHW_BLT_SOURCES_
    ${SOFTLET_MHW_BLT_SOURCES_}
    ${TMP_BLT_SOURCES_}
)

set(TMP_SOURCES_
    ${CMAKE_CURRENT_LIST_DIR}/mhw_state_heap.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mhw_block_manager.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mhw_memory_pool.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mhw_blt.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mhw_utilities_next.cpp  
)

set(SOFTLET_MHW_COMMON_SOURCES_
    ${SOFTLET_MHW_COMMON_SOURCES_}
    ${TMP_SOURCES_}
)

source_group( "MHW" FILES ${TMP_SOURCES_} ${TMP_HEADERS_}
    ${TMP_RENDER_HEADERS_} ${TMP_MI_HEADERS_}
    ${TMP_VEBOX_HEADERS_} ${TMP_SFC_HEADERS_}
    ${TMP_BLT_HEADERS_} ${TMP_BLT_SOURCES_}
)
set(TMP_HEADERS_ "")
set(TMP_SOURCES_ "")
set(TMP_RENDER_HEADERS_ "")
set(TMP_MI_HEADERS_ "")
set(TMP_VEBOX_HEADERS_ "")
set(TMP_SFC_HEADERS_ "")
set(TMP_BLT_HEADERS_ "")
set(TMP_BLT_SOURCES_ "")

set(SOFTLET_MHW_RENDER_PRIVATE_INCLUDE_DIRS_
    ${SOFTLET_MHW_RENDER_PRIVATE_INCLUDE_DIRS_}
    ${CMAKE_CURRENT_LIST_DIR}
)

set(SOFTLET_MHW_MI_PRIVATE_INCLUDE_DIRS_
    ${SOFTLET_MHW_MI_PRIVATE_INCLUDE_DIRS_}
    ${CMAKE_CURRENT_LIST_DIR}
)

set(SOFTLET_MHW_VEBOX_PRIVATE_INCLUDE_DIRS_
    ${SOFTLET_MHW_VEBOX_PRIVATE_INCLUDE_DIRS_}
    ${CMAKE_CURRENT_LIST_DIR}
)

set(SOFTLET_MHW_SFC_PRIVATE_INCLUDE_DIRS_
    ${SOFTLET_MHW_SFC_PRIVATE_INCLUDE_DIRS_}
    ${CMAKE_CURRENT_LIST_DIR}
)

set(SOFTLET_MHW_BLT_PRIVATE_INCLUDE_DIRS_
    ${SOFTLET_MHW_BLT_PRIVATE_INCLUDE_DIRS_}
    ${CMAKE_CURRENT_LIST_DIR}
)

set(SOFTLET_MHW_COMMON_PRIVATE_INCLUDE_DIRS_
    ${SOFTLET_MHW_COMMON_PRIVATE_INCLUDE_DIRS_}
    ${CMAKE_CURRENT_LIST_DIR}
)