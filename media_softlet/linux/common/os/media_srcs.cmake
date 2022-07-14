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

media_include_subdirectory(osservice)

if(NOT CMAKE_WDDM_LINUX)
set(TMP_SOURCES_
    ${CMAKE_CURRENT_LIST_DIR}/mos_context_specific_next.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mos_graphicsresource_specific_next.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mos_commandbuffer_specific_next.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mos_gpucontext_specific_next.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mos_gpucontext_specific_next_ext.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mos_os_specific_next.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mos_decompression_base.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mos_mediacopy_base.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mos_user_setting_specific.cpp
)

set(TMP_HEADERS_
    ${CMAKE_CURRENT_LIST_DIR}/mos_context_specific_next.h
    ${CMAKE_CURRENT_LIST_DIR}/mos_graphicsresource_specific_next.h
    ${CMAKE_CURRENT_LIST_DIR}/mos_util_devult_specific_next.h
    ${CMAKE_CURRENT_LIST_DIR}/mos_commandbuffer_specific_next.h
    ${CMAKE_CURRENT_LIST_DIR}/mos_gpucontext_specific_next.h
    ${CMAKE_CURRENT_LIST_DIR}/mos_decompression.h
    ${CMAKE_CURRENT_LIST_DIR}/mos_decompression_base.h
    ${CMAKE_CURRENT_LIST_DIR}/media_skuwa_specific.h
)

if(${Media_Scalability_Supported} STREQUAL "yes")
set(TMP_SOURCES_
    ${TMP_SOURCES_}
    ${CMAKE_CURRENT_LIST_DIR}/mos_os_virtualengine_scalability_specific_next.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mos_os_virtualengine_singlepipe_specific_next.cpp
)

set(TMP_HEADERS_
    ${TMP_HEADERS_}
    ${CMAKE_CURRENT_LIST_DIR}/mos_os_virtualengine_scalability_specific_next.h
    ${CMAKE_CURRENT_LIST_DIR}/mos_os_virtualengine_singlepipe_specific_next.h
)
endif() #if(${Media_Scalability_Supported} STREQUAL "yes")

set(MOS_COMMON_SOURCES_
    ${MOS_COMMON_SOURCES_}
    ${TMP_SOURCES_}
 )

set(MOS_COMMON_HEADERS_
    ${MOS_COMMON_HEADERS_}
    ${TMP_HEADERS_}
)

set(MOS_PUBLIC_INCLUDE_DIRS_
    ${MOS_PUBLIC_INCLUDE_DIRS_}
    ${CMAKE_CURRENT_LIST_DIR}
)
source_group( "mos_softlet" FILES ${TMP_SOURCES_} ${TMP_HEADERS_} )

endif() # CMAKE_WDDM_LINUX
