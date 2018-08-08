# *****************************************************************************
# Copyright (c) 2017 Intel Corporation
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
# *****************************************************************************

# utility functions for cmake


if(NOT DEFINED _bs_include_base_utils)

set(_bs_include_base_utils TRUE)

# bs_set_if_undefined - If not defined, assign value from env or arg
macro(bs_set_if_undefined VAR_NAME VAR_VALUE)
    if(DEFINED ${VAR_NAME} AND NOT ${VAR_NAME} STREQUAL "")
        # Already defined - no action to take
    elseif(DEFINED ENV{VAR_NAME} AND NOT "$ENV{VAR_NAME}" STREQUAL "")
        set(${VAR_NAME} "$ENV{VAR_NAME}")
    else()
        set(${VAR_NAME} "${VAR_VALUE}")
    endif()
endmacro()


# function bs_check_build_type - Enforce build type variable consistency
#
# Ensure that deprecated variable UFO_BUILD_TYPE has the same value as
# BUILD_TYPE.
#
# Ensure that the value of CMAKE_BUILD_TYPE is consistent with the value
# of BUILD_TYPE.
#
# QuickBuild passes in BUILD_TYPE with per-word Caps.
# The Linux makefiles force the value to lower-case.
# For consistency, the same thing is done here.
#
function(bs_check_build_type)
    if(NOT DEFINED BUILD_TYPE OR "${BUILD_TYPE}" STREQUAL "")
        if(DEFINED UFO_BUILD_TYPE AND NOT "${UFO_BUILD_TYPE}" STREQUAL "")
            set(BUILD_TYPE "${UFO_BUILD_TYPE}")
        else()
            message("*BUILD_TYPE not defined, default to: release")
            set(BUILD_TYPE "release")
        endif()
    endif()
    string(TOLOWER "${BUILD_TYPE}" BUILD_TYPE)

    if ("${BUILD_TYPE}" STREQUAL "release")
        set(_val_cmake_build_type "Release")
    elseif ("${BUILD_TYPE}" STREQUAL "release-internal")
        set(_val_cmake_build_type "ReleaseInternal")
    elseif ("${BUILD_TYPE}" STREQUAL "debug")
        set(_val_cmake_build_type "Debug")
    else()
        set(_val_cmake_build_type "${BUILD_TYPE}")
    endif()

    if(DEFINED CMAKE_BUILD_TYPE AND NOT CMAKE_BUILD_TYPE STREQUAL "")
        if(NOT "${CMAKE_BUILD_TYPE}" STREQUAL "${_val_cmake_build_type}")
            # Would consider FATAL_ERROR, except for pre-existing condition
            # in Linux legacy build system:
            # In invocation of Source/instrumentation/instr.cmake :
            #   Inconsistent: BUILD_TYPE="release-internal" vs CMAKE_BUILD_TYPE="Release"
            message(WARNING "Inconsistent: BUILD_TYPE=\"${BUILD_TYPE}\" vs CMAKE_BUILD_TYPE=\"${CMAKE_BUILD_TYPE}\"")
        endif()
    else()
        set(CMAKE_BUILD_TYPE "${_val_cmake_build_type}")
    endif()

    set(BUILD_TYPE "${BUILD_TYPE}" PARENT_SCOPE)
    set(UFO_BUILD_TYPE "${BUILD_TYPE}" PARENT_SCOPE)
    set(CMAKE_BUILD_TYPE "${CMAKE_BUILD_TYPE}" PARENT_SCOPE)

endfunction(bs_check_build_type)


endif(NOT DEFINED _bs_include_base_utils)
