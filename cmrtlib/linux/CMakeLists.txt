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

cmake_minimum_required(VERSION 2.8)
include (${CMAKE_CURRENT_LIST_DIR}/cmrt_utils.cmake)

# Distinguish between 32 and 64 bits
# The string that is set is used to modify the target names of some of the libraries generated
if(CMAKE_SIZEOF_VOID_P EQUAL 4)
  set(TARGET_MODIFIER "32")
  set(PB_PATH_MODIFIER "x86")
else()
  set(TARGET_MODIFIER "64")
  set(PB_PATH_MODIFIER "x64")
endif()

set(EXECUTABLE_OUTPUT_PATH ${MDF_OPTION__OUTPUT_DIR})
set(LIBRARY_OUTPUT_PATH    ${MDF_OPTION__OUTPUT_DIR})

# Set up compile options that will be used for the Linux build
if(CMAKE_COMPILER_IS_GNUCC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-variable -Wno-write-strings -Wno-deprecated -DVPHAL -DLINUX -msse4.1 -std=gnu++0x -fPIC -fpermissive -fstack-protector-all")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fno-strict-aliasing -D_FORTIFY_SOURCE=2")
    set(CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG -D__DEBUG -O0")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-unused-variable -Wno-write-strings -Wno-deprecated -DVPHAL -DLINUX -msse4.1 -std=gnu++0x -fPIC -fpermissive -fstack-protector-all")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -fno-strict-aliasing -D_FORTIFY_SOURCE=2")
    set(CMAKE_C_FLAGS_DEBUG   "${CMAKE_C_FLAGS_DEBUG} -D_DEBUG -D__DEBUG -O0")

    set(GCC_SECURE_LINK_FLAGS "-z relro -z now")
    set(CMAKE_SKIP_RPATH ON)
endif(CMAKE_COMPILER_IS_GNUCC)

#setup_configurations must be called before project()
#setup_configurations()

#include paths
include_directories(${CMAKE_CURRENT_LIST_DIR}/../agnostic/share)
include_directories(${CMAKE_CURRENT_LIST_DIR}/../agnostic/hardware)
include_directories(${CMAKE_CURRENT_LIST_DIR}/../linux/share)
include_directories(${CMAKE_CURRENT_LIST_DIR}/../linux/hardware)

if(NOT "${LIBVA_INSTALL_PATH}" STREQUAL "")
    include_directories(BEFORE ${LIBVA_INSTALL_PATH})
    link_directories(BEFORE ${LIBVA_LIBRARY_PATH})
else()
    include(FindPkgConfig)
    if(NOT LIBVA_FOUND)
        pkg_check_modules(LIBVA REQUIRED libva>=1.0.0)
    endif()
    if(LIBVA_FOUND)
        include_directories(BEFORE ${LIBVA_INCLUDE_DIRS})
        link_directories(${LIBVA_LIBRARY_DIRS})
    endif()
endif()

# Set up the source files
set(CMRT_SOURCES "")
set(CMRT_DEFINES "")
cmrt_include_directory(${CMAKE_CURRENT_LIST_DIR}/../agnostic)
cmrt_include_directory(${CMAKE_CURRENT_LIST_DIR}/../linux)

setup_library(igfxcmrt "${CMRT_SOURCES}" TRUE "igfxcmrt${TARGET_MODIFIER}")

set(CMRT_DEFINES ${CMRT_DEFINES} VPHAL ISTDLIB_UMD CM_RT_EXPORTS)

# Extra defines for 64 bit builds
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
  set(CMRT_DEFINES ${CMRT_DEFINES} __CT__)
endif()
set_target_properties( igfxcmrt PROPERTIES 
  FOLDER CM_RT 
  COMPILE_DEFINITIONS "${CMRT_DEFINES}" 
  OUTPUT_NAME "igfxcmrt${TARGET_MODIFIER}"
  )
set_target_properties( igfxcmrt PROPERTIES PREFIX "")
target_link_libraries( igfxcmrt dl va rt ${GCC_SECURE_LINK_FLAGS})

