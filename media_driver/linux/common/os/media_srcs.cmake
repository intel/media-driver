# Copyright (c) 2017-2022, Intel Corporation
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

set(TMP_HEADERS_ "")
set(TMP_SOURCES_ "")

if(NOT CMAKE_WDDM_LINUX)
set(TMP_SOURCES_
    ${CMAKE_CURRENT_LIST_DIR}/mos_context_specific.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mos_graphicsresource_specific.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mos_os_specific.c
    ${CMAKE_CURRENT_LIST_DIR}/mos_util_devult_specific.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mos_commandbuffer_specific.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mos_gpucontext_specific.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mos_gpucontext_specific_ext.cpp
    ${CMAKE_CURRENT_LIST_DIR}/memory_policy_manager_specific.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mos_decompression.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mos_mediacopy.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mos_utilities_specific_usersetting.cpp
)

set(TMP_HEADERS_
    ${CMAKE_CURRENT_LIST_DIR}/mos_context_specific.h
    ${CMAKE_CURRENT_LIST_DIR}/mos_graphicsresource_specific.h
    ${CMAKE_CURRENT_LIST_DIR}/mos_util_devult_specific.h
    ${CMAKE_CURRENT_LIST_DIR}/mos_commandbuffer_specific.h
    ${CMAKE_CURRENT_LIST_DIR}/mos_gpucontext_specific.h
)

if(${Media_Scalability_Supported} STREQUAL "yes")

set(TMP_SOURCES_
    ${TMP_SOURCES_}
    ${CMAKE_CURRENT_LIST_DIR}/mos_os_virtualengine_scalability_specific.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mos_os_virtualengine_singlepipe_specific.cpp
)

set(TMP_HEADERS_
    ${TMP_HEADERS_}
    ${CMAKE_CURRENT_LIST_DIR}/mos_os_virtualengine_scalability_specific.h
    ${CMAKE_CURRENT_LIST_DIR}/mos_os_virtualengine_singlepipe_specific.h
)

endif()

set(MOS_COMMON_SOURCES_
    ${MOS_COMMON_SOURCES_}
    ${TMP_SOURCES_}
 )

set(MOS_COMMON_HEADERS_
    ${MOS_COMMON_HEADERS_}
    ${TMP_HEADERS_}
)

set (MOS_PUBLIC_INCLUDE_DIRS_
    ${MOS_PUBLIC_INCLUDE_DIRS_}
    ${CMAKE_CURRENT_LIST_DIR}
)
endif() #NOT CMAKE_WDDM_LINUX
