# Copyright (c) 2018-2024, Intel Corporation
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

set(TMP_SFC_HEADERS_
    ${CMAKE_CURRENT_LIST_DIR}/mhw_sfc_hwcmd_xe2_hpm_next.h
    ${CMAKE_CURRENT_LIST_DIR}/mhw_sfc_xe2_hpm_next_impl.h
)

set(TMP_SFC_SOURCES_
    ${CMAKE_CURRENT_LIST_DIR}/mhw_sfc_hwcmd_xe2_hpm_next.cpp
)

set(SOFTLET_MHW_SFC_HEADERS_
    ${SOFTLET_MHW_SFC_HEADERS_}
    ${TMP_SFC_HEADERS_}
)

set(SOFTLET_MHW_SFC_SOURCES_
    ${SOFTLET_MHW_SFC_SOURCES_}
    ${TMP_SFC_SOURCES_}
)

set(TMP_VEBOX_HEADERS
    ${CMAKE_CURRENT_LIST_DIR}/mhw_vebox_hwcmd_xe2_hpm_next.h
    ${CMAKE_CURRENT_LIST_DIR}/mhw_vebox_xe2_hpm_next_impl.h
)

set(TMP_VEBOX_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/mhw_vebox_hwcmd_xe2_hpm.cpp
)

set(TMP_BLT_HEADERS
    ${CMAKE_CURRENT_LIST_DIR}/mhw_blt_hwcmd_xe2_hpm.h
    ${CMAKE_CURRENT_LIST_DIR}/mhw_blt_xe2_hpm_impl.h
)

set(TMP_BLT_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/mhw_blt_hwcmd_xe2_hpm.cpp
)
set(SOFTLET_COMMON_HEADERS_
    ${SOFTLET_COMMON_HEADERS_}
    ${TMP_SFC_HEADERS_}
    ${TMP_VEBOX_HEADERS}
    ${TMP_BLT_HEADERS}
)

set(SOFTLET_MHW_VEBOX_SOURCES_
    ${SOFTLET_MHW_VEBOX_SOURCES_}
    ${TMP_VEBOX_SOURCES}
    ${TMP_BLT_SOURCES}
)

source_group("MHW\\SFC" FILES ${TMP_SFC_SOURCES_} ${TMP_SFC_HEADERS_})
source_group("MHW\\vebox" FILES ${TMP_VEBOX_SOURCES} ${TMP_VEBOX_HEADERS})
source_group("MHW\\BLT" FILES ${TMP_BLT_SOURCES} ${TMP_BLT_HEADERS})


set(TMP_SFC_SOURCES_ "")
set(TMP_SFC_HEADERS_ "")
set(TMP_VEBOX_SOURCES "")
set(TMP_VEBOX_HEADERS "")
set(TMP_BLT_SOURCES "")
set(TMP_BLT_HEADERS "")

set(SOFTLET_MHW_SFC_PRIVATE_INCLUDE_DIRS_
    ${SOFTLET_MHW_SFC_PRIVATE_INCLUDE_DIRS_}
    ${CMAKE_CURRENT_LIST_DIR}
)
set(SOFTLET_MHW_VEBOX_PRIVATE_INCLUDE_DIRS_
    ${SOFTLET_MHW_VEBOX_PRIVATE_INCLUDE_DIRS_}
    ${CMAKE_CURRENT_LIST_DIR}
)
set(SOFTLET_MHW_BLT_PRIVATE_INCLUDE_DIRS_
    ${SOFTLET_MHW_BLT_PRIVATE_INCLUDE_DIRS_}
    ${CMAKE_CURRENT_LIST_DIR}
)

