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

set(TMP_SOFTLET_MHW_VDBOX_HUC_SOURCES_
    ${CMAKE_CURRENT_LIST_DIR}/mhw_vdbox_huc_hwcmd_xe_lpm_plus.cpp
)

set(TMP_SOFTLET_MHW_VDBOX_AVP_SOURCES_
    ${CMAKE_CURRENT_LIST_DIR}/mhw_vdbox_avp_hwcmd_xe_lpm_plus.cpp
)

set(TMP_SOFTLET_MHW_VDBOX_MFX_SOURCES_
    ${CMAKE_CURRENT_LIST_DIR}/mhw_vdbox_mfx_hwcmd_xe_lpm_plus.cpp
)


set(SOFTLET_MHW_VDBOX_HUC_SOURCES_
    ${SOFTLET_MHW_VDBOX_HUC_SOURCES_}
    ${TMP_SOFTLET_MHW_VDBOX_HUC_SOURCES_}
)

set(SOFTLET_MHW_VDBOX_AVP_SOURCES_
    ${SOFTLET_MHW_VDBOX_AVP_SOURCES_}
    ${TMP_SOFTLET_MHW_VDBOX_AVP_SOURCES_}
)

set(SOFTLET_MHW_VDBOX_MFX_SOURCES_
    ${SOFTLET_MHW_VDBOX_MFX_SOURCES_}
    ${TMP_SOFTLET_MHW_VDBOX_MFX_SOURCES_}
)


set(TMP_SOFTLET_MHW_VDBOX_HCP_HEADERS_
    ${CMAKE_CURRENT_LIST_DIR}/mhw_vdbox_hcp_impl_xe_lpm_plus.h
    ${CMAKE_CURRENT_LIST_DIR}/mhw_vdbox_hcp_hwcmd_xe_lpm_plus.h
)

set(TMP_SOFTLET_MHW_VDBOX_HUC_HEADERS_
    ${CMAKE_CURRENT_LIST_DIR}/mhw_vdbox_huc_hwcmd_xe_lpm_plus.h
    ${CMAKE_CURRENT_LIST_DIR}/mhw_vdbox_huc_impl_xe_lpm_plus.h
)

set(TMP_SOFTLET_MHW_VDBOX_VDENC_HEADERS_
    ${CMAKE_CURRENT_LIST_DIR}/mhw_vdbox_vdenc_hwcmd_xe_lpm_plus.h
    ${CMAKE_CURRENT_LIST_DIR}/mhw_vdbox_vdenc_impl_xe_lpm_plus.h
)

set(TMP_SOFTLET_MHW_VDBOX_AVP_HEADERS_
    ${CMAKE_CURRENT_LIST_DIR}/mhw_vdbox_avp_impl_xe_lpm_plus.h
    ${CMAKE_CURRENT_LIST_DIR}/mhw_vdbox_avp_hwcmd_xe_lpm_plus.h
)

set(TMP_SOFTLET_MHW_VDBOX_MFX_HEADERS_
    ${CMAKE_CURRENT_LIST_DIR}/mhw_vdbox_mfx_hwcmd_xe_lpm_plus.h
    ${CMAKE_CURRENT_LIST_DIR}/mhw_vdbox_mfx_impl_xe_lpm_plus.h
)


set(SOFTLET_MHW_VDBOX_HCP_HEADERS_
    ${SOFTLET_MHW_VDBOX_HCP_HEADERS_}
    ${TMP_SOFTLET_MHW_VDBOX_HCP_HEADERS_}
)

set(SOFTLET_MHW_VDBOX_HUC_HEADERS_
    ${SOFTLET_MHW_VDBOX_HUC_HEADERS_}
    ${TMP_SOFTLET_MHW_VDBOX_HUC_HEADERS_}
)

set(SOFTLET_MHW_VDBOX_VDENC_HEADERS_
    ${SOFTLET_MHW_VDBOX_VDENC_HEADERS_}
    ${TMP_SOFTLET_MHW_VDBOX_VDENC_HEADERS_}
)

set(SOFTLET_MHW_VDBOX_AVP_HEADERS_
    ${SOFTLET_MHW_VDBOX_AVP_HEADERS_}
    ${TMP_SOFTLET_MHW_VDBOX_AVP_HEADERS_}
)

set(SOFTLET_MHW_VDBOX_MFX_HEADERS_
    ${SOFTLET_MHW_VDBOX_MFX_HEADERS_}
    ${TMP_SOFTLET_MHW_VDBOX_MFX_HEADERS_}
)

source_group( "MHW\\Vdbox" FILES ${TMP_SOFTLET_MHW_VDBOX_HUC_SOURCES_} 
${TMP_SOFTLET_MHW_VDBOX_AVP_SOURCES_}
${TMP_SOFTLET_MHW_VDBOX_MFX_SOURCES_}
${TMP_SOFTLET_MHW_VDBOX_HCP_HEADERS_}
${TMP_SOFTLET_MHW_VDBOX_HUC_HEADERS_}
${TMP_SOFTLET_MHW_VDBOX_VDENC_HEADERS_}
${TMP_SOFTLET_MHW_VDBOX_AVP_HEADERS_}
${TMP_SOFTLET_MHW_VDBOX_MFX_HEADERS_})

set(TMP_SOFTLET_MHW_VDBOX_HUC_SOURCES_ "")
set(TMP_SOFTLET_MHW_VDBOX_AVP_SOURCES_ "")
set(TMP_SOFTLET_MHW_VDBOX_MFX_SOURCES_ "")

set(TMP_SOFTLET_MHW_VDBOX_HCP_HEADERS_ "")
set(TMP_SOFTLET_MHW_VDBOX_HUC_HEADERS_ "")
set(TMP_SOFTLET_MHW_VDBOX_VDENC_HEADERS_ "")
set(TMP_SOFTLET_MHW_VDBOX_AVP_HEADERS_ "")
set(TMP_SOFTLET_MHW_VDBOX_MFX_HEADERS_ "")

set(SOFTLET_MHW_VDBOX_PRIVATE_INCLUDE_DIRS_
    ${SOFTLET_MHW_VDBOX_PRIVATE_INCLUDE_DIRS_}
    ${CMAKE_CURRENT_LIST_DIR}
)
media_add_curr_to_include_path()