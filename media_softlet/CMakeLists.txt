# Copyright (c) 2021, Intel Corporation
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


cmake_minimum_required(VERSION 3.5)

if(NOT DEFINED BS_USE_OSDM_BUILD_SYSTEM)
    if(DEFINED ENV{BS_USE_OSDM_BUILD_SYSTEM})
        set(BS_USE_OSDM_BUILD_SYSTEM "$ENV{BS_USE_OSDM_BUILD_SYSTEM}")
    else()
        set(BS_USE_OSDM_BUILD_SYSTEM FALSE)
    endif()
endif()

# begin -- label bldsys file prologue
# WARNING: The "project" statement triggers reading of CMAKE_TOOLCHAIN_FILE
#   and so must precede the inclusion below of bs_init.cmake .
function(bs_find_build_system gfx_dev_dir build_sys_dir build_sys_inc)
    # If we are not building as a standalone project
    if(DEFINED GFX_DEVELOPMENT_DIR)
        set(_bs_gfx_development_dir "${GFX_DEVELOPMENT_DIR}")
    elseif(DEFINED ENV{GFX_DEVELOPMENT_DIR})
        set(_bs_gfx_development_dir "$ENV{GFX_DEVELOPMENT_DIR}")
    else()
        get_filename_component(_bs_cur_cmake_dir "${CMAKE_CURRENT_LIST_FILE}" PATH)
        get_filename_component(_bs_parent_dir "${_bs_cur_cmake_dir}" DIRECTORY)
        set(_bs_gfx_dir_found false)
        while(NOT _bs_gfx_dir_found)
            set(_bs_bldsys_dir "${_bs_parent_dir}/Tools/bldsys")
            if(EXISTS ${_bs_bldsys_dir})
                set(_bs_gfx_development_dir ${_bs_parent_dir})
                set(_bs_gfx_dir_found true)
                break()
            endif()
            get_filename_component(_bs_parent_dir "${_bs_parent_dir}" DIRECTORY)
            if(${_bs_parent_dir} STREQUAL "/")
                break()
            endif()
        endwhile(NOT _bs_gfx_dir_found)
        if (NOT _bs_gfx_development_dir)
            message(FATAL_ERROR "GFX_DEVELOPMENT_DIR not found (${_bs_gfx_development_dir}) - exiting!")
            exit(1)
        endif()
    endif()
    set(${gfx_dev_dir} "${_bs_gfx_development_dir}" PARENT_SCOPE)
    set(${build_sys_dir} "${_bs_gfx_development_dir}/Tools/bldsys" PARENT_SCOPE)
    set(${build_sys_inc} "${_bs_gfx_development_dir}/Tools/bldsys/include" PARENT_SCOPE)
endfunction(bs_find_build_system)

bs_find_build_system(GFX_DEVELOPMENT_DIR BUILD_SYS_DIR BUILD_SYS_INC)

include(${BUILD_SYS_DIR}/bs_init.cmake)
# end -- label bldsys file prologue

if(NOT BUILD_KERNELS)
    # We regenerate shaders (kernels) and store them in-source.
    set(CMAKE_DISABLE_SOURCE_CHANGES ON)
endif()
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)

if ("${BUILD_TYPE}" STREQUAL "release")
    set(CMAKE_BUILD_TYPE "Release")
elseif ("${BUILD_TYPE}" STREQUAL "release-internal")
    set(CMAKE_BUILD_TYPE "ReleaseInternal")
elseif ("${BUILD_TYPE}" STREQUAL "debug")
    set(CMAKE_BUILD_TYPE "Debug")
endif()

include(media_pre_top_cmake.cmake OPTIONAL)

include(${BS_DIR_MEDIA}/media_softlet/cmake/media_defs.cmake)

include(${MEDIA_SOFTLET_CMAKE}/media_utils.cmake)

find_file(EXT_CONF NAMES media_top_cmake_ext.cmake PATHS ${CMAKE_CURRENT_LIST_DIR})

if(EXT_CONF)
    include(${CMAKE_CURRENT_LIST_DIR}/media_top_cmake_ext.cmake)
else()
    include(${CMAKE_CURRENT_LIST_DIR}/media_top_cmake.cmake)
endif()
