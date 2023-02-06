# Copyright (c) 2021-2022, Intel Corporation
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
    ${CMAKE_CURRENT_LIST_DIR}/mhw_utilities.h
    ${CMAKE_CURRENT_LIST_DIR}/mhw_block_manager.h
    ${CMAKE_CURRENT_LIST_DIR}/mhw_memory_pool.h
)

set(SOFTLET_MHW_COMMON_HEADERS_
    ${SOFTLET_MHW_COMMON_HEADERS_}
    ${TMP_HEADERS_}
)

set(TMP_RENDER_HEADERS_
    ${CMAKE_CURRENT_LIST_DIR}/mhw_render.h
)

set(TMP_STATE_HEAP_HEADERS_
    ${CMAKE_CURRENT_LIST_DIR}/mhw_state_heap.h
    ${CMAKE_CURRENT_LIST_DIR}/mhw_state_heap_generic.h
)

set(SOFTLET_MHW_RENDER_HEADERS_
    ${SOFTLET_MHW_RENDER_HEADERS_}
    ${TMP_RENDER_HEADERS_}
    ${TMP_STATE_HEAP_HEADERS_}
)

set(TMP_BLT_HEADERS_
    ${CMAKE_CURRENT_LIST_DIR}/mhw_blt.h
)

set(SOFTLET_MHW_BLT_HEADERS_
    ${SOFTLET_MHW_BLT_HEADERS_}
    ${TMP_BLT_HEADERS_}
)

set(TMP_MI_HEADERS_
    ${CMAKE_CURRENT_LIST_DIR}/mhw_mi.h
    ${CMAKE_CURRENT_LIST_DIR}/mhw_mmio.h
)

set(SOFTLET_MHW_MI_HEADERS_
    ${SOFTLET_MHW_MI_HEADERS_}
    ${TMP_MI_HEADERS_}
)

set(TMP_SFC_HEADERS_
    ${CMAKE_CURRENT_LIST_DIR}/mhw_sfc.h
)

set(SOFTLET_MHW_SFC_HEADERS_
    ${SOFTLET_MHW_SFC_HEADERS_}
    ${TMP_SFC_HEADERS_}
)

set(TMP_VEBOX_HEADERS_
    ${CMAKE_CURRENT_LIST_DIR}/mhw_vebox.h
)

set(SOFTLET_MHW_VEBOX_HEADERS_
    ${SOFTLET_MHW_VEBOX_HEADERS_}
    ${TMP_VEBOX_HEADERS_}
)

source_group("MHW" FILES ${TMP_HEADERS_})
source_group("MHW\\Render Engine" FILES ${TMP_RENDER_HEADERS_})
source_group("MHW\\State Heap" FILES ${TMP_STATE_HEAP_HEADERS_})
source_group("MHW\\BLT" FILES ${TMP_BLT_HEADERS_})
source_group("MHW\\Common MI" FILES ${TMP_MI_HEADERS_})
source_group("MHW\\SFC" FILES ${TMP_SFC_HEADERS_})
source_group("MHW\\VEBOX" FILES ${TMP_VEBOX_HEADERS_})
set(SOFTLET_MHW_COMMON_PRIVATE_INCLUDE_DIRS_
    ${SOFTLET_MHW_COMMON_PRIVATE_INCLUDE_DIRS_}
    ${CMAKE_CURRENT_LIST_DIR}
)

set(SOFTLET_MHW_RENDER_PRIVATE_INCLUDE_DIRS_
    ${SOFTLET_MHW_RENDER_PRIVATE_INCLUDE_DIRS_}
    ${CMAKE_CURRENT_LIST_DIR}
)

set(SOFTLET_MHW_BLT_PRIVATE_INCLUDE_DIRS_
    ${SOFTLET_MHW_BLT_PRIVATE_INCLUDE_DIRS_}
    ${CMAKE_CURRENT_LIST_DIR}
)

set(SOFTLET_MHW_MI_PRIVATE_INCLUDE_DIRS_
    ${SOFTLET_MHW_MI_PRIVATE_INCLUDE_DIRS_}
    ${CMAKE_CURRENT_LIST_DIR}
)

set(SOFTLET_MHW_SFC_PRIVATE_INCLUDE_DIRS_
    ${SOFTLET_MHW_SFC_PRIVATE_INCLUDE_DIRS_}
    ${CMAKE_CURRENT_LIST_DIR}
)

set(SOFTLET_MHW_VEBOX_PRIVATE_INCLUDE_DIRS_
    ${SOFTLET_MHW_VEBOX_PRIVATE_INCLUDE_DIRS_}
    ${CMAKE_CURRENT_LIST_DIR}
)