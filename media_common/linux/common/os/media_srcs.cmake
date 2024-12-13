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

if(NOT CMAKE_WDDM_LINUX)
set(TMP_HEADERS_
    ${CMAKE_CURRENT_LIST_DIR}/mos_defs_specific.h
    ${CMAKE_CURRENT_LIST_DIR}/mos_util_user_feature_keys_specific.h
    ${CMAKE_CURRENT_LIST_DIR}/mos_os_specific.h
)

if(${Media_Scalability_Supported} STREQUAL "yes")
set(TMP_HEADERS_
    ${TMP_HEADERS_}
    ${CMAKE_CURRENT_LIST_DIR}/mos_os_virtualengine_specific.h
)
endif() #if(${Media_Scalability_Supported} STREQUAL "yes")

source_group( "MOS" FILES ${TMP_SOURCES_} ${TMP_HEADERS_} )

set(SOFTLET_MOS_COMMON_HEADERS_
    ${SOFTLET_MOS_COMMON_HEADERS_}
    ${TMP_HEADERS_}
)
set(SOFTLET_MOS_PUBLIC_INCLUDE_DIRS_
    ${SOFTLET_MOS_PUBLIC_INCLUDE_DIRS_}
    ${CMAKE_CURRENT_LIST_DIR}
)

endif() # CMAKE_WDDM_LINUX

media_add_curr_to_include_path()