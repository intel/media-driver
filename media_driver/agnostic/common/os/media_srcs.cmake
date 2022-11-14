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

set(TMP_SOURCES_
    ${CMAKE_CURRENT_LIST_DIR}/mos_context.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mos_graphicsresource.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mos_util_user_interface.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mos_utilities.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mos_gpucontext.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mos_gpucontextmgr.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mos_cmdbufmgr.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mos_commandbuffer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mos_utilities_usersetting.cpp
)

set(TMP_HEADERS_
    ${CMAKE_CURRENT_LIST_DIR}/mos_context.h
    ${CMAKE_CURRENT_LIST_DIR}/mos_graphicsresource.h
    ${CMAKE_CURRENT_LIST_DIR}/mos_gpucontext.h
    ${CMAKE_CURRENT_LIST_DIR}/mos_gpucontextmgr.h
    ${CMAKE_CURRENT_LIST_DIR}/mos_cmdbufmgr.h
    ${CMAKE_CURRENT_LIST_DIR}/mos_commandbuffer.h
    ${CMAKE_CURRENT_LIST_DIR}/mos_util_user_interface.h
)

if(${Media_Scalability_Supported} STREQUAL "yes")

set(TMP_SOURCES_
    ${TMP_SOURCES_}
    ${CMAKE_CURRENT_LIST_DIR}/mos_os_virtualengine.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mos_os_virtualengine_scalability.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mos_os_virtualengine_singlepipe.cpp
)

set(TMP_HEADERS_
    ${TMP_HEADERS_}
    ${CMAKE_CURRENT_LIST_DIR}/mos_os_virtualengine.h
    ${CMAKE_CURRENT_LIST_DIR}/mos_os_virtualengine_scalability.h
    ${CMAKE_CURRENT_LIST_DIR}/mos_os_virtualengine_singlepipe.h
)
endif()

source_group( "MOS" FILES ${TMP_SOURCES_} ${TMP_HEADERS_} )

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

media_add_curr_to_include_path()