# Copyright (c) 2020-2022, Intel Corporation

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

media_include_subdirectory(vdbox)

set(TMP_UTILITIES_HEADERS_
    ${CMAKE_CURRENT_LIST_DIR}/mhw_utilities_xe_xpm.h
)

set(TMP_SFC_SOURCES_
    ${CMAKE_CURRENT_LIST_DIR}/mhw_sfc_xe_xpm.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mhw_sfc_hwcmd_xe_xpm.cpp
)

set(TMP_SFC_HEADERS_
    ${CMAKE_CURRENT_LIST_DIR}/mhw_sfc_xe_xpm.h
    ${CMAKE_CURRENT_LIST_DIR}/mhw_sfc_hwcmd_xe_xpm.h
)

set(TMP_STATE_HEAP_SOURCES_
    ${CMAKE_CURRENT_LIST_DIR}/mhw_state_heap_xe_xpm.c
    ${CMAKE_CURRENT_LIST_DIR}/mhw_state_heap_hwcmd_xe_xpm.cpp
)

set(TMP_STATE_HEAP_HEADERS_
    ${CMAKE_CURRENT_LIST_DIR}/mhw_state_heap_xe_xpm.h
    ${CMAKE_CURRENT_LIST_DIR}/mhw_state_heap_hwcmd_xe_xpm.h
)

set(TMP_VEBOX_SOURCES_
    ${CMAKE_CURRENT_LIST_DIR}/mhw_vebox_xe_xpm.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mhw_vebox_hwcmd_xe_xpm.cpp
)

set(TMP_VEBOX_HEADERS_
    ${CMAKE_CURRENT_LIST_DIR}/mhw_vebox_xe_xpm.h
    ${CMAKE_CURRENT_LIST_DIR}/mhw_vebox_hwcmd_xe_xpm.h
)

set(TMP_MI_SOURCES_
    ${CMAKE_CURRENT_LIST_DIR}/mhw_mi_hwcmd_xe_xpm.cpp
)

set(TMP_MI_HEADERS_
    ${CMAKE_CURRENT_LIST_DIR}/mhw_mi_hwcmd_xe_xpm.h
)


set(COMMON_SOURCES_
    ${COMMON_SOURCES_}
    ${TMP_SFC_SOURCES_}
    ${TMP_STATE_HEAP_SOURCES_}
    ${TMP_VEBOX_SOURCES_}
    ${TMP_MI_SOURCES_}
)

set(COMMON_HEADERS_
    ${COMMON_HEADERS_}
    ${TMP_UTILITIES_HEADERS_}
    ${TMP_SFC_HEADERS_}
    ${TMP_STATE_HEAP_HEADERS_}
    ${TMP_VEBOX_HEADERS_}
    ${TMP_MI_HEADERS_}
)

source_group("MHW" FILES ${TMP_UTILITIES_HEADERS_})
source_group("MHW\\SFC" FILES ${TMP_SFC_SOURCES_} ${TMP_SFC_HEADERS_})
source_group("MHW\\State Heap" FILES ${TMP_STATE_HEAP_SOURCES_} ${TMP_STATE_HEAP_HEADERS_})
source_group("MHW\\VEBOX" FILES ${TMP_VEBOX_SOURCES_} ${TMP_VEBOX_HEADERS_})
source_group("MHW\\Common MI" FILES ${TMP_MI_SOURCES_} ${TMP_MI_HEADERS_})

set(COMMON_PRIVATE_INCLUDE_DIRS_
    ${COMMON_PRIVATE_INCLUDE_DIRS_}
    ${CMAKE_CURRENT_LIST_DIR}
)