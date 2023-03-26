# Copyright (c) 2018-2022, Intel Corporation
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

set(TMP_SOURCES_ "")
set(TMP_HEADERS_ "")
media_include_subdirectory(pipeline)
media_include_subdirectory(packet)
media_include_subdirectory(features)
media_include_subdirectory(task)
media_include_subdirectory(scalability)
media_include_subdirectory(mediacontext)
media_include_subdirectory(statusreport)
media_include_subdirectory(mmc)
media_include_subdirectory(bufferMgr)
media_include_subdirectory(mediacopy)
media_include_subdirectory(media_sfc_interface)
media_include_subdirectory(classtrace)
media_include_subdirectory(profiler)

set(TMP_SOURCES_
    ${TMP_SOURCES_}
    ${CMAKE_CURRENT_LIST_DIR}/media_debug_config_manager.cpp
    ${CMAKE_CURRENT_LIST_DIR}/media_debug_interface.cpp
    ${CMAKE_CURRENT_LIST_DIR}/media_render_common.cpp
    ${CMAKE_CURRENT_LIST_DIR}/null_hardware_next.cpp
    ${CMAKE_CURRENT_LIST_DIR}/media_debug_dumper.cpp
    ${CMAKE_CURRENT_LIST_DIR}/media_debug_fast_dump.cpp
    ${CMAKE_CURRENT_LIST_DIR}/oca_rtlog_section_mgr.cpp
)

set(TMP_HEADERS_
    ${TMP_HEADERS_}
    ${CMAKE_CURRENT_LIST_DIR}/media_utils.h
    ${CMAKE_CURRENT_LIST_DIR}/media_common_defs.h
    ${CMAKE_CURRENT_LIST_DIR}/media_capstable.h
    ${CMAKE_CURRENT_LIST_DIR}/media_debug_config_manager.h
    ${CMAKE_CURRENT_LIST_DIR}/media_debug_interface.h
    ${CMAKE_CURRENT_LIST_DIR}/media_debug_utils.h
    ${CMAKE_CURRENT_LIST_DIR}/media_render_common.h
    ${CMAKE_CURRENT_LIST_DIR}/mediamemdecomp.h
    ${CMAKE_CURRENT_LIST_DIR}/media_debug_dumper.h
    ${CMAKE_CURRENT_LIST_DIR}/media_debug_serializer.h
    ${CMAKE_CURRENT_LIST_DIR}/media_debug_fast_dump.h
    ${CMAKE_CURRENT_LIST_DIR}/media_debug_fast_dump_imp.hpp
    ${CMAKE_CURRENT_LIST_DIR}/oca_rtlog_section_mgr.h
)


set(SOFTLET_COMMON_SOURCES_
    ${SOFTLET_COMMON_SOURCES_}
    ${TMP_SOURCES_}
)

set(SOFTLET_COMMON_HEADERS_
    ${SOFTLET_COMMON_HEADERS_}
    ${TMP_HEADERS_}
)

source_group(SharedNext\\shared FILES ${TMP_HEADERS_} ${TMP_SOURCES_})

set(SOFTLET_COMMON_PRIVATE_INCLUDE_DIRS_
    ${SOFTLET_COMMON_PRIVATE_INCLUDE_DIRS_}
    ${CMAKE_CURRENT_LIST_DIR}
)