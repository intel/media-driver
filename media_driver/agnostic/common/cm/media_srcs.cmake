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
    ${CMAKE_CURRENT_LIST_DIR}/cm_array.cpp
    ${CMAKE_CURRENT_LIST_DIR}/cm_buffer_rt.cpp
    ${CMAKE_CURRENT_LIST_DIR}/cm_state_buffer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/cm_def.cpp
    ${CMAKE_CURRENT_LIST_DIR}/cm_device_rt_base.cpp
    ${CMAKE_CURRENT_LIST_DIR}/cm_event_rt.cpp
    ${CMAKE_CURRENT_LIST_DIR}/cm_group_space.cpp
    ${CMAKE_CURRENT_LIST_DIR}/cm_hal.cpp
    ${CMAKE_CURRENT_LIST_DIR}/cm_hal_hashtable.cpp
    ${CMAKE_CURRENT_LIST_DIR}/cm_hal_dump.cpp
    ${CMAKE_CURRENT_LIST_DIR}/cm_hal_vebox.cpp
    ${CMAKE_CURRENT_LIST_DIR}/cm_kernel_rt.cpp
    ${CMAKE_CURRENT_LIST_DIR}/cm_kernel_data.cpp
    ${CMAKE_CURRENT_LIST_DIR}/cm_log.cpp
    ${CMAKE_CURRENT_LIST_DIR}/cm_perf.cpp
    ${CMAKE_CURRENT_LIST_DIR}/cm_printf_host.cpp
    ${CMAKE_CURRENT_LIST_DIR}/cm_program.cpp
    ${CMAKE_CURRENT_LIST_DIR}/cm_queue_rt.cpp
    ${CMAKE_CURRENT_LIST_DIR}/cm_sampler_rt.cpp
    ${CMAKE_CURRENT_LIST_DIR}/cm_sampler8x8_state_rt.cpp
    ${CMAKE_CURRENT_LIST_DIR}/cm_surface.cpp
    ${CMAKE_CURRENT_LIST_DIR}/cm_surface_2d_rt_base.cpp
    ${CMAKE_CURRENT_LIST_DIR}/cm_surface_2d_up_rt.cpp
    ${CMAKE_CURRENT_LIST_DIR}/cm_surface_3d_rt.cpp
    ${CMAKE_CURRENT_LIST_DIR}/cm_surface_manager_base.cpp
    ${CMAKE_CURRENT_LIST_DIR}/cm_surface_sampler.cpp
    ${CMAKE_CURRENT_LIST_DIR}/cm_surface_sampler8x8.cpp
    ${CMAKE_CURRENT_LIST_DIR}/cm_surface_vme.cpp
    ${CMAKE_CURRENT_LIST_DIR}/cm_task_rt.cpp
    ${CMAKE_CURRENT_LIST_DIR}/cm_task_internal.cpp
    ${CMAKE_CURRENT_LIST_DIR}/cm_thread_space_rt.cpp
    ${CMAKE_CURRENT_LIST_DIR}/cm_vebox_rt.cpp
    ${CMAKE_CURRENT_LIST_DIR}/cm_vebox_data.cpp
    ${CMAKE_CURRENT_LIST_DIR}/cm_visa.cpp
    ${CMAKE_CURRENT_LIST_DIR}/cm_wrapper.cpp
    ${CMAKE_CURRENT_LIST_DIR}/cm_global_api.cpp
    ${CMAKE_CURRENT_LIST_DIR}/cm_execution_adv.cpp
    ${CMAKE_CURRENT_LIST_DIR}/cm_ish_base.cpp
    ${CMAKE_CURRENT_LIST_DIR}/cm_kernel_ex.cpp
    ${CMAKE_CURRENT_LIST_DIR}/cm_surface_state.cpp
    ${CMAKE_CURRENT_LIST_DIR}/cm_surface_state_manager.cpp
    ${CMAKE_CURRENT_LIST_DIR}/cm_ssh.cpp
    ${CMAKE_CURRENT_LIST_DIR}/cm_scratch_space.cpp
    ${CMAKE_CURRENT_LIST_DIR}/cm_media_state.cpp
    ${CMAKE_CURRENT_LIST_DIR}/cm_dsh.cpp
    ${CMAKE_CURRENT_LIST_DIR}/cm_tracker.cpp
    ${CMAKE_CURRENT_LIST_DIR}/cm_event_ex_base.cpp
    ${CMAKE_CURRENT_LIST_DIR}/cm_command_buffer.cpp
)

set(TMP_HEADERS_
    ${CMAKE_CURRENT_LIST_DIR}/cm_array.h
    ${CMAKE_CURRENT_LIST_DIR}/cm_buffer.h
    ${CMAKE_CURRENT_LIST_DIR}/cm_buffer_rt.h
    ${CMAKE_CURRENT_LIST_DIR}/cm_common.h
    ${CMAKE_CURRENT_LIST_DIR}/cm_debug.h
    ${CMAKE_CURRENT_LIST_DIR}/cm_def.h
    ${CMAKE_CURRENT_LIST_DIR}/cm_event.h
    ${CMAKE_CURRENT_LIST_DIR}/cm_event_rt.h
    ${CMAKE_CURRENT_LIST_DIR}/cm_group_space.h
    ${CMAKE_CURRENT_LIST_DIR}/cm_hal.h
    ${CMAKE_CURRENT_LIST_DIR}/cm_hal_generic.h
    ${CMAKE_CURRENT_LIST_DIR}/cm_hal_hashtable.h
    ${CMAKE_CURRENT_LIST_DIR}/cm_hal_vebox.h
    ${CMAKE_CURRENT_LIST_DIR}/cm_kernel.h
    ${CMAKE_CURRENT_LIST_DIR}/cm_kernel_rt.h
    ${CMAKE_CURRENT_LIST_DIR}/cm_kernel_data.h
    ${CMAKE_CURRENT_LIST_DIR}/cm_log.h
    ${CMAKE_CURRENT_LIST_DIR}/cm_mem.h
    ${CMAKE_CURRENT_LIST_DIR}/cm_mov_inst.h
    ${CMAKE_CURRENT_LIST_DIR}/cm_perf.h
    ${CMAKE_CURRENT_LIST_DIR}/cm_printf_host.h
    ${CMAKE_CURRENT_LIST_DIR}/cm_program.h
    ${CMAKE_CURRENT_LIST_DIR}/cm_queue.h
    ${CMAKE_CURRENT_LIST_DIR}/cm_queue_rt.h
    ${CMAKE_CURRENT_LIST_DIR}/cm_sampler.h
    ${CMAKE_CURRENT_LIST_DIR}/cm_sampler_rt.h
    ${CMAKE_CURRENT_LIST_DIR}/cm_sampler8x8.h
    ${CMAKE_CURRENT_LIST_DIR}/cm_sampler8x8_state_rt.h
    ${CMAKE_CURRENT_LIST_DIR}/cm_state_buffer.h
    ${CMAKE_CURRENT_LIST_DIR}/cm_surface.h
    ${CMAKE_CURRENT_LIST_DIR}/cm_surface_2d_up.h
    ${CMAKE_CURRENT_LIST_DIR}/cm_surface_2d_up_rt.h
    ${CMAKE_CURRENT_LIST_DIR}/cm_surface_3d.h
    ${CMAKE_CURRENT_LIST_DIR}/cm_surface_3d_rt.h
    ${CMAKE_CURRENT_LIST_DIR}/cm_surface_sampler.h
    ${CMAKE_CURRENT_LIST_DIR}/cm_surface_sampler8x8.h
    ${CMAKE_CURRENT_LIST_DIR}/cm_surface_vme.h
    ${CMAKE_CURRENT_LIST_DIR}/cm_task.h
    ${CMAKE_CURRENT_LIST_DIR}/cm_task_rt.h
    ${CMAKE_CURRENT_LIST_DIR}/cm_task_internal.h
    ${CMAKE_CURRENT_LIST_DIR}/cm_thread_space.h
    ${CMAKE_CURRENT_LIST_DIR}/cm_thread_space_rt.h
    ${CMAKE_CURRENT_LIST_DIR}/cm_vebox.h
    ${CMAKE_CURRENT_LIST_DIR}/cm_vebox_rt.h
    ${CMAKE_CURRENT_LIST_DIR}/cm_vebox_data.h
    ${CMAKE_CURRENT_LIST_DIR}/cm_visa.h
    ${CMAKE_CURRENT_LIST_DIR}/cm_wrapper.h
    ${CMAKE_CURRENT_LIST_DIR}/cm_execution_adv.h
    ${CMAKE_CURRENT_LIST_DIR}/cm_rt_umd.h
    ${CMAKE_CURRENT_LIST_DIR}/cm_surface_manager_base.h
    ${CMAKE_CURRENT_LIST_DIR}/cm_surface_2d_rt_base.h
    ${CMAKE_CURRENT_LIST_DIR}/cm_device_rt_base.h
    ${CMAKE_CURRENT_LIST_DIR}/cm_ish_base.h
    ${CMAKE_CURRENT_LIST_DIR}/cm_kernel_ex.h
    ${CMAKE_CURRENT_LIST_DIR}/cm_surface_state.h
    ${CMAKE_CURRENT_LIST_DIR}/cm_surface_state_manager.h
    ${CMAKE_CURRENT_LIST_DIR}/cm_ssh.h
    ${CMAKE_CURRENT_LIST_DIR}/cm_scratch_space.h
    ${CMAKE_CURRENT_LIST_DIR}/cm_media_state.h
    ${CMAKE_CURRENT_LIST_DIR}/cm_dsh.h
    ${CMAKE_CURRENT_LIST_DIR}/cm_tracker.h
    ${CMAKE_CURRENT_LIST_DIR}/cm_event_ex_base.h
    ${CMAKE_CURRENT_LIST_DIR}/cm_command_buffer.h
)

set(SOURCES_
    ${SOURCES_}
    ${TMP_SOURCES_}
)

set(HEADERS_
    ${HEADERS_}
    ${TMP_HEADERS_}
)

source_group( CM FILES ${TMP_SOURCES_} ${TMP_HEADERS_} )


media_add_curr_to_include_path()
