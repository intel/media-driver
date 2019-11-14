# Copyright (c) 2017, Intel Corporation
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
    ${CMAKE_CURRENT_LIST_DIR}/mos_os.c
    ${CMAKE_CURRENT_LIST_DIR}/mos_util_debug.c
    ${CMAKE_CURRENT_LIST_DIR}/mos_util_user_interface.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mos_utilities.c
    ${CMAKE_CURRENT_LIST_DIR}/mos_gpucontext.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mos_gpucontextmgr.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mos_cmdbufmgr.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mos_commandbuffer.cpp
)

set(TMP_HEADERS_
    ${CMAKE_CURRENT_LIST_DIR}/media_fourcc.h
    ${CMAKE_CURRENT_LIST_DIR}/mos_context.h
    ${CMAKE_CURRENT_LIST_DIR}/mos_defs.h
    ${CMAKE_CURRENT_LIST_DIR}/mos_graphicsresource.h
    ${CMAKE_CURRENT_LIST_DIR}/mos_os.h
    ${CMAKE_CURRENT_LIST_DIR}/mos_os_hw.h
    ${CMAKE_CURRENT_LIST_DIR}/mos_os_trace_event.h
    ${CMAKE_CURRENT_LIST_DIR}/mos_resource_defs.h
    ${CMAKE_CURRENT_LIST_DIR}/mos_solo_generic.h
    ${CMAKE_CURRENT_LIST_DIR}/mos_util_debug.h
    ${CMAKE_CURRENT_LIST_DIR}/mos_util_user_feature_keys.h
    ${CMAKE_CURRENT_LIST_DIR}/mos_util_user_interface.h
    ${CMAKE_CURRENT_LIST_DIR}/mos_utilities.h
    ${CMAKE_CURRENT_LIST_DIR}/mos_utilities_common.h
    ${CMAKE_CURRENT_LIST_DIR}/mos_gpucontext.h
    ${CMAKE_CURRENT_LIST_DIR}/mos_gpucontextmgr.h
    ${CMAKE_CURRENT_LIST_DIR}/mos_cmdbufmgr.h
    ${CMAKE_CURRENT_LIST_DIR}/mos_commandbuffer.h
    ${CMAKE_CURRENT_LIST_DIR}/mos_oca_interface.h
    ${CMAKE_CURRENT_LIST_DIR}/mos_interface.h
)

set(SOURCES_
    ${SOURCES_}
    ${TMP_SOURCES_}
 )

set(HEADERS_
    ${HEADERS_}
    ${TMP_HEADERS_}
)

source_group( "MOS" FILES ${TMP_SOURCES_} ${TMP_HEADERS_} )


if(${Media_Scalability_Supported} STREQUAL "yes")

set(TMP_2_SOURCES_
    ${CMAKE_CURRENT_LIST_DIR}/mos_os_virtualengine.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mos_os_virtualengine_scalability.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mos_os_virtualengine_singlepipe.cpp
)

set(TMP_2_HEADERS_
    ${CMAKE_CURRENT_LIST_DIR}/mos_os_virtualengine.h
    ${CMAKE_CURRENT_LIST_DIR}/mos_os_virtualengine_scalability.h
    ${CMAKE_CURRENT_LIST_DIR}/mos_os_virtualengine_singlepipe.h
)

set(SOURCES_
    ${SOURCES_}
    ${TMP_2_SOURCES_}
 )

set(HEADERS_
    ${HEADERS_}
    ${TMP_2_HEADERS_}
)

source_group( "MOS" FILES ${TMP_2_SOURCES_} ${TMP_2_HEADERS_} )

endif()


media_add_curr_to_include_path()
