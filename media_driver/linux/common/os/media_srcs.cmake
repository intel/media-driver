# Copyright (c) 2017-2018, Intel Corporation
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

media_include_subdirectory(i915)


set(TMP_SOURCES_
    ${CMAKE_CURRENT_LIST_DIR}/hwinfo_linux.c
    ${CMAKE_CURRENT_LIST_DIR}/mos_context_specific.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mos_graphicsresource_specific.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mos_os_specific.c
    ${CMAKE_CURRENT_LIST_DIR}/mos_util_debug_specific.c
    ${CMAKE_CURRENT_LIST_DIR}/mos_util_devult_specific.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mos_utilities_specific.c
    ${CMAKE_CURRENT_LIST_DIR}/mos_commandbuffer_specific.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mos_gpucontext_specific.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mos_auxtable_mgr.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mos_interface.cpp
)

set(TMP_HEADERS_
    ${CMAKE_CURRENT_LIST_DIR}/hwinfo_linux.h
    ${CMAKE_CURRENT_LIST_DIR}/mos_context_specific.h
    ${CMAKE_CURRENT_LIST_DIR}/media_skuwa_specific.h
    ${CMAKE_CURRENT_LIST_DIR}/mos_graphicsresource_specific.h
    ${CMAKE_CURRENT_LIST_DIR}/mos_os_specific.h
    ${CMAKE_CURRENT_LIST_DIR}/mos_util_user_feature_keys_specific.h
    ${CMAKE_CURRENT_LIST_DIR}/mos_util_devult_specific.h
    ${CMAKE_CURRENT_LIST_DIR}/mos_utilities_specific.h
    ${CMAKE_CURRENT_LIST_DIR}/mos_commandbuffer_specific.h
    ${CMAKE_CURRENT_LIST_DIR}/mos_gpucontext_specific.h
    ${CMAKE_CURRENT_LIST_DIR}/mos_auxtable_mgr.h
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
    ${CMAKE_CURRENT_LIST_DIR}/mos_os_virtualengine_specific.h
)

endif()

set(SOURCES_
    ${SOURCES_}
    ${TMP_SOURCES_}
 )

set(HEADERS_
    ${HEADERS_}
    ${TMP_HEADERS_}
)


media_add_curr_to_include_path()
