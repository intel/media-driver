# Copyright (c) 2019, Intel Corporation
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
    ${CMAKE_CURRENT_LIST_DIR}/cm_device_rt.cpp
    ${CMAKE_CURRENT_LIST_DIR}/cm_event_rt_os.cpp
    ${CMAKE_CURRENT_LIST_DIR}/cm_queue_rt_os.cpp
    ${CMAKE_CURRENT_LIST_DIR}/cm_ftrace.cpp
    ${CMAKE_CURRENT_LIST_DIR}/cm_hal_os.cpp
    ${CMAKE_CURRENT_LIST_DIR}/cm_surface_2d_rt.cpp
    ${CMAKE_CURRENT_LIST_DIR}/cm_surface_manager.cpp
    ${CMAKE_CURRENT_LIST_DIR}/cm_task_internal_os.cpp
    ${CMAKE_CURRENT_LIST_DIR}/cm_task_rt_os.cpp
    ${CMAKE_CURRENT_LIST_DIR}/cm_wrapper_os.cpp
    ${CMAKE_CURRENT_LIST_DIR}/cm_global_api_os.cpp
    ${CMAKE_CURRENT_LIST_DIR}/cm_debug_os.cpp
    ${CMAKE_CURRENT_LIST_DIR}/cm_ish.cpp
    ${CMAKE_CURRENT_LIST_DIR}/cm_surface_state_manager_os.cpp
    ${CMAKE_CURRENT_LIST_DIR}/cm_event_ex.cpp
    ${CMAKE_CURRENT_LIST_DIR}/cm_command_buffer_os.cpp
)

set(TMP_HEADERS_
    ${CMAKE_CURRENT_LIST_DIR}/cm_csync.h
    ${CMAKE_CURRENT_LIST_DIR}/cm_def_os.h
    ${CMAKE_CURRENT_LIST_DIR}/cm_device.h
    ${CMAKE_CURRENT_LIST_DIR}/cm_device_rt.h
    ${CMAKE_CURRENT_LIST_DIR}/cm_ftrace.h
    ${CMAKE_CURRENT_LIST_DIR}/cm_innerdef_os.h
    ${CMAKE_CURRENT_LIST_DIR}/cm_mem_os.h
    ${CMAKE_CURRENT_LIST_DIR}/cm_surface_2d.h
    ${CMAKE_CURRENT_LIST_DIR}/cm_surface_2d_rt.h
    ${CMAKE_CURRENT_LIST_DIR}/cm_surface_manager.h
    ${CMAKE_CURRENT_LIST_DIR}/cm_wrapper_os.h
    ${CMAKE_CURRENT_LIST_DIR}/cm_ish.h
    ${CMAKE_CURRENT_LIST_DIR}/cm_event_ex.h
)

set(SOURCES_
    ${SOURCES_}
    ${TMP_SOURCES_}
 )

set(HEADERS_
    ${HEADERS_}
    ${TMP_HEADERS_}
)

media_add_curr_to_include_path()
