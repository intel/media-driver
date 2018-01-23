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

# Idempotent include implementation
set_property( GLOBAL PROPERTY USE_FOLDERS ON)
set(BUILD_FUNCTIONS_INC ON)

# Functions to add custom build configurations: release, release internal, debug
# Assumption is that there is only one of these per target (otherwise the custom target breaks)
function(custom_build_add configName copyConfigName)
  string(TOUPPER "${configName}" _configName)
  string(TOUPPER "${copyConfigName}" _copyConfigName)

  set("CMAKE_CXX_FLAGS_${_configName}"           "${CMAKE_CXX_FLAGS_${_copyConfigName}}"           CACHE STRING "C++ compilder flags used during \"${configName}\" builds.")
  set("CMAKE_C_FLAGS_${_configName}"             "${CMAKE_C_FLAGS_${_copyConfigName}}"             CACHE STRING "Flags used during \"${configName}\" builds.")
  set("CMAKE_EXE_LINKER_FLAGS_${_configName}"    "${CMAKE_EXE_LINKER_FLAGS_${_copyConfigName}}"    CACHE STRING "Linker flags used during \"${configName}\" builds for executables.")
  set("CMAKE_MODULE_LINKER_FLAGS_${_configName}" "${CMAKE_MODULE_LINKER_FLAGS_${_copyConfigName}}" CACHE STRING "Linker flags used during \"${configName}\" builds for modules.")
  set("CMAKE_SHARED_LINKER_FLAGS_${_configName}" "${CMAKE_SHARED_LINKER_FLAGS_${_copyConfigName}}" CACHE STRING "Linker flags used during \"${configName}\" builds for shared libraries.")
  set("CMAKE_STATIC_LINKER_FLAGS_${_configName}" "${CMAKE_STATIC_LINKER_FLAGS_${_copyConfigName}}" CACHE STRING "Linker flags used during \"${configName}\" builds for static libraries.")
  mark_as_advanced(
      "CMAKE_CXX_FLAGS_${_configName}"
      "CMAKE_C_FLAGS_${_configName}"
      "CMAKE_EXE_LINKER_FLAGS_${_configName}"
      "CMAKE_MODULE_LINKER_FLAGS_${_configName}"
      "CMAKE_SHARED_LINKER_FLAGS_${_configName}"
      "CMAKE_STATIC_LINKER_FLAGS_${_configName}"
    )
  
endfunction()

function( setup_configurations )
    set(CMAKE_CONFIGURATION_TYPES
        "Debug"
        "Release Internal"
        "Release"
      )
    custom_build_add("Release Internal"       "Release")
    set(CMAKE_CONFIGURATION_TYPES ${CMAKE_CONFIGURATION_TYPES} CACHE STRING "Available build configurations." FORCE)
endfunction( setup_configurations )

# If you update this function - make sure you also update the copy of this for the IGC build that is
# hard-coded into the CM_jitter CMakeLists.txt 
function ( setup_library target src_list shared actual_name)
  if ( ${shared} )
    add_library(${target} SHARED ${src_list})
  else ( ${shared} )
    add_library(${target} ${src_list})
  endif ( ${shared} )
endfunction( setup_library )

# Only can include subdirectory which has a cmrt_srcs.cmake
# the effect is like include(${CMAKE_CURRENT_LIST_DIR}/<subd>/cmrt_srcs.cmake)
macro(cmrt_include_subdirectory subd)
    if(EXISTS ${CMAKE_CURRENT_LIST_DIR}/${subd}/cmrt_srcs.cmake)
        include(${CMAKE_CURRENT_LIST_DIR}/${subd}/cmrt_srcs.cmake)
    else()
        message("-- ${CMAKE_CURRENT_LIST_DIR}/${subd}/cmrt_srcs.cmake doesn't exist, macro(cmrt_include_subdirectory) just does nothing")
    endif()
endmacro()

# Only can include subdirectory which has a cmrt_srcs.cmake
# the effect is like include(${CMAKE_CURRENT_LIST_DIR}/<subd>/cmrt_srcs.cmake)
macro(cmrt_include_directory dir)
    if(EXISTS ${dir}/cmrt_srcs.cmake)
        include(${dir}/cmrt_srcs.cmake)
    else()
        message("-- ${dir}/cmrt_srcs.cmake doesn't exist, macro(cmrt_include_subdirectory) just does nothing")
    endif()
endmacro()
