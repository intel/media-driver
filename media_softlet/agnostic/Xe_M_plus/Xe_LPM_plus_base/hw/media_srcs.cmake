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

set(TMP_MI_HEADERS_
    ${CMAKE_CURRENT_LIST_DIR}/mhw_mi_hwcmd_xe_lpm_plus_base_next.h
    ${CMAKE_CURRENT_LIST_DIR}/mhw_mi_xe_lpm_plus_base_next_impl.h
)

set(SOFTLET_MHW_MI_HEADERS_
    ${SOFTLET_MHW_MI_HEADERS_}
    ${TMP_MI_HEADERS_}
)

set(TMP_VEBOX_HEADERS_
    ${CMAKE_CURRENT_LIST_DIR}/mhw_vebox_xe_lpm_plus_base_next_impl.h
)

set(SOFTLET_MHW_VEBOX_HEADERS_
    ${SOFTLET_MHW_VEBOX_HEADERS_}
    ${TMP_VEBOX_HEADERS_}
)

set(TMP_SFC_HEADERS_
    ${CMAKE_CURRENT_LIST_DIR}/mhw_sfc_xe_lpm_plus_base_next_impl.h
)

set(SOFTLET_MHW_SFC_HEADERS_
    ${SOFTLET_MHW_SFC_HEADERS_}
    ${TMP_SFC_HEADERS_}
)

set(TMP_BLT_HEADERS_
    ${CMAKE_CURRENT_LIST_DIR}/mhw_blt_xe_lpm_plus_base_next_impl.h
)

set(SOFTLET_MHW_BLT_HEADERS_
    ${SOFTLET_MHW_BLT_HEADERS_}
    ${TMP_BLT_HEADERS_}
)

source_group("MHW\\Common MI" FILES ${TMP_MI_HEADERS_})
source_group("MHW\\VEBOX" FILES ${TMP_VEBOX_HEADERS_})
source_group("MHW\\SFC" FILES ${TMP_SFC_HEADERS_})
source_group("MHW\\BLT" FILES ${TMP_BLT_HEADERS_})
set(TMP_MI_HEADERS_ "")
set(TMP_VEBOX_HEADERS_ "")
set(TMP_SFC_HEADERS_ "")
set(TMP_BLT_HEADERS_ "")

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
#media_add_curr_to_include_path()