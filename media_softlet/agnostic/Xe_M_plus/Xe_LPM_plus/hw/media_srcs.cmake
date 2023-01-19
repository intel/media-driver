# Copyright (c) 2022, Intel Corporation
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
    ${CMAKE_CURRENT_LIST_DIR}/mhw_mmio_xe_lpm_plus.h
)

set(SOFTLET_MHW_MI_HEADERS_
    ${SOFTLET_MHW_MI_HEADERS_}
    ${TMP_HEADERS_}
)

set(TMP_1_HEADERS_
    ${CMAKE_CURRENT_LIST_DIR}/mhw_vebox_hwcmd_xe_lpm_plus_next.h
)

set(TMP_1_SOURCES_
    ${CMAKE_CURRENT_LIST_DIR}/mhw_vebox_hwcmd_xe_lpm_plus_next.cpp
)

set(SOFTLET_MHW_VEBOX_HEADERS_
    ${SOFTLET_MHW_VEBOX_HEADERS_}
    ${TMP_1_HEADERS_}
)

set(SOFTLET_MHW_VEBOX_SOURCES_
    ${SOFTLET_MHW_VEBOX_SOURCES_}
    ${TMP_1_SOURCES_}
)

set(TMP_2_HEADERS_
    ${CMAKE_CURRENT_LIST_DIR}/mhw_sfc_hwcmd_xe_lpm_plus_next.h
)

set(TMP_2_SOURCES_
    ${CMAKE_CURRENT_LIST_DIR}/mhw_sfc_hwcmd_xe_lpm_plus_next.cpp
)

set(SOFTLET_MHW_SFC_HEADERS_
    ${SOFTLET_MHW_SFC_HEADERS_}
    ${TMP_2_HEADERS_}
)

set(SOFTLET_MHW_SFC_SOURCES_
    ${SOFTLET_MHW_SFC_SOURCES_}
    ${TMP_2_SOURCES_}
)

set(TMP_3_HEADERS_
    ${CMAKE_CURRENT_LIST_DIR}/mhw_blt_hwcmd_xe_lpm_plus_next.h
)

set(TMP_3_SOURCES_
    ${CMAKE_CURRENT_LIST_DIR}/mhw_blt_hwcmd_xe_lpm_plus_next.cpp
)

set(SOFTLET_MHW_BLT_HEADERS_
    ${SOFTLET_MHW_BLT_HEADERS_}
    ${TMP_3_HEADERS_}
)

set(SOFTLET_MHW_BLT_HEADERS_
    ${SOFTLET_MHW_BLT_HEADERS_}
    ${TMP_3_SOURCES_}
)

source_group("MHW\\Common MI" FILES ${TMP_HEADERS_})
source_group("MHW\\VEBOX" FILES ${TMP_1_SOURCES_} ${TMP_1_HEADERS_})
source_group("MHW\\SFC" FILES ${TMP_2_SOURCES_} ${TMP_2_HEADERS_})
source_group("MHW\\BLT" FILES ${TMP_3_SOURCES_} ${TMP_3_HEADERS_})

set(TMP_HEADERS_ "")
set(TMP_1_HEADERS_ "")
set(TMP_1_SOURCES_ "")
set(TMP_2_HEADERS_ "")
set(TMP_2_SOURCES_ "")
set(TMP_3_HEADERS_ "")
set(TMP_3_SOURCES_ "")

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