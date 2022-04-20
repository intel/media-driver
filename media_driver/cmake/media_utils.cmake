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

# Only can include subdirectory which has a media_srcs.cmake
# the effect is like include(${CMAKE_CURRENT_LIST_DIR}/<subd>/media_srcs.cmake)

macro(media_include_subdirectory subd)
    if(EXISTS ${CMAKE_CURRENT_LIST_DIR}/${subd}/media_srcs.cmake)
        if(CMAKE_WDDM_LINUX)
            media_wsl_include_subdirectory(${subd})
        else()
            include(${CMAKE_CURRENT_LIST_DIR}/${subd}/media_srcs.cmake)
        endif()
    else()
        message("-- ${CMAKE_CURRENT_LIST_DIR}/${subd}/media_srcs.cmake doesn't exist, macro(media_include_subdirectory) just does nothing")
    endif()
endmacro()

# add current path to include path
macro(media_add_curr_to_include_path)
    if(${PLATFORM} STREQUAL "linux")
        include_directories(${CMAKE_CURRENT_LIST_DIR})
    else()
        media_add_curr_to_include_path_ext(${CMAKE_CURRENT_LIST_DIR})
    endif()
endmacro()

# MediaSetLinkerFlags: apply linker flags for given configuration
# linkerFlags: linker specific options
# linkerTarget: optional parameter - apply linker flags for specfied target
macro (MediaSetLinkerFlags linkerFlags linkerTarget)
    foreach (opt ${linkerFlags})
        if ("${linkerTarget}" STREQUAL "")
            set (CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${opt}")
            set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${opt}")
        else()
            set (CMAKE_SHARED_LINKER_FLAGS_${linkerTarget} "${CMAKE_SHARED_LINKER_FLAGS_${linkerTarget}} ${opt}")
            set (CMAKE_EXE_LINKER_FLAGS_${linkerTarget} "${CMAKE_EXE_LINKER_FLAGS_${linkerTarget}} ${opt}")
        endif()
    endforeach()
endmacro()

# common defines
macro (MediaAddCommonTargetDefines target)
    if (TARGET ${target})
        set_property(TARGET ${target} APPEND PROPERTY COMPILE_DEFINITIONS
            $<$<CONFIG:Release>:_RELEASE>
            $<$<CONFIG:ReleaseInternal>: _RELEASE_INTERNAL>
            $<$<CONFIG:Debug>: _DEBUG DEBUG>
        )
    endif()
endmacro()

include( ${MEDIA_EXT_CMAKE}/ext/media_utils_ext.cmake OPTIONAL)
