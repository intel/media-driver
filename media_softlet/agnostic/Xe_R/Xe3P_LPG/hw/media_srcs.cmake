# Copyright (c) 2023, Intel Corporation

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
    ${CMAKE_CURRENT_LIST_DIR}/mhw_render_hwcmd_xe3p_lpg.cpp
)

set(TMP_RENDER_HEADERS_
    ${CMAKE_CURRENT_LIST_DIR}/mhw_render_hwcmd_xe3p_lpg.h
    ${CMAKE_CURRENT_LIST_DIR}/mhw_render_xe3p_lpg_impl.h
)

set(TMP_STATE_HEAP_SOURCES_
    ${CMAKE_CURRENT_LIST_DIR}/mhw_state_heap_xe3p_lpg.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mhw_state_heap_hwcmd_xe3p_lpg.cpp
)

set(TMP_STATE_HEAP_HEADERS_
    ${CMAKE_CURRENT_LIST_DIR}/mhw_state_heap_xe3p_lpg.h
    ${CMAKE_CURRENT_LIST_DIR}/mhw_state_heap_hwcmd_xe3p_lpg.h
)

set(SOFTLET_MHW_RENDER_SOURCES_
    ${SOFTLET_MHW_RENDER_SOURCES_}
    ${TMP_RENDER_SOURCES_}
    ${TMP_STATE_HEAP_SOURCES_}
)

set(SOFTLET_MHW_RENDER_HEADERS_
    ${SOFTLET_MHW_RENDER_HEADERS_}
    ${TMP_RENDER_HEADERS_}
    ${TMP_STATE_HEAP_HEADERS_}
)

source_group("MHW\\Render Engine" FILES ${TMP_RENDER_SOURCES_} ${TMP_RENDER_HEADERS_})
source_group("MHW\\State Heap" FILES ${TMP_STATE_HEAP_SOURCES_} ${TMP_STATE_HEAP_HEADERS_})
set(TMP_RENDER_SOURCES_ "")
set(TMP_RENDER_HEADERS_ "")
set(TMP_STATE_HEAP_SOURCES_ "")
set(TMP_STATE_HEAP_HEADERS_ "")
set(SOFTLET_MHW_RENDER_PRIVATE_INCLUDE_DIRS_
    ${SOFTLET_MHW_RENDER_PRIVATE_INCLUDE_DIRS_}
    ${CMAKE_CURRENT_LIST_DIR}
)

set(TMP_SOFTLET_CODE_GEN_RENDER_MHW_SOURCES_
    ${CMAKE_CURRENT_LIST_DIR}/mhw_render_hwcmd_xe3p_lpg.cpp
)

set(TMP_SOFTLET_CODE_GEN_STATE_HEAP_MHW_SOURCES_
    ${CMAKE_CURRENT_LIST_DIR}/mhw_state_heap_hwcmd_xe3p_lpg.cpp
)

set(TMP_SOFTLET_CODE_GEN_RENDER_MHW_HEADERS_
    ${CMAKE_CURRENT_LIST_DIR}/mhw_render_hwcmd_xe3p_lpg.h
)

set(TMP_SOFTLET_CODE_GEN_STATE_HEAP_MHW_HEADERS_
    ${CMAKE_CURRENT_LIST_DIR}/mhw_state_heap_hwcmd_xe3p_lpg.h
)

set(SOFTLET_CODE_GEN_MHW_HEADERS_
    ${SOFTLET_CODE_GEN_MHW_HEADERS_}
    ${TMP_SOFTLET_CODE_GEN_RENDER_MHW_HEADERS_}
    ${TMP_SOFTLET_CODE_GEN_STATE_HEAP_MHW_HEADERS_}
)

set(SOFTLET_CODE_GEN_MHW_SOURCES_
    ${SOFTLET_CODE_GEN_MHW_SOURCES_}
    ${TMP_SOFTLET_CODE_GEN_RENDER_MHW_SOURCES_}
    ${TMP_SOFTLET_CODE_GEN_STATE_HEAP_MHW_SOURCES_}
)

source_group( "MHW CMD\\Xe3P_LPG\\Render Engine" FILES 
    ${TMP_SOFTLET_CODE_GEN_RENDER_MHW_HEADERS_}
    ${TMP_SOFTLET_CODE_GEN_RENDER_MHW_SOURCES_}
)

source_group( "MHW CMD\\Xe3P_LPG\\State Heap" FILES 
    ${TMP_SOFTLET_CODE_GEN_STATE_HEAP_MHW_HEADERS_}
    ${TMP_SOFTLET_CODE_GEN_STATE_HEAP_MHW_SOURCES_}
)

set(TMP_SOFTLET_CODE_GEN_RENDER_MHW_SOURCES_ "")
set(TMP_SOFTLET_CODE_GEN_STATE_HEAP_MHW_SOURCES_ "")
set(TMP_SOFTLET_CODE_GEN_RENDER_MHW_HEADERS_ "")
set(TMP_SOFTLET_CODE_GEN_STATE_HEAP_MHW_HEADERS_ "")

set(SOFTLET_CODE_GEN_MHW_PRIVATE_INCLUDE_DIRS_
    ${SOFTLET_CODE_GEN_MHW_PRIVATE_INCLUDE_DIRS_}
    ${CMAKE_CURRENT_LIST_DIR}
)