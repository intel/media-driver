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


set(EXECUTABLE_OUTPUT_PATH ${MDF_OPTION__OUTPUT_DIR})
set(LIBRARY_OUTPUT_PATH    ${MDF_OPTION__OUTPUT_DIR})

if (LATEST_CPP_NEEDED)
    set(CPP_STANDARD_OPTION -std=c++17)
else()
    set(CPP_STANDARD_OPTION -std=c++11)
endif()

# Set up compile options that will be used for the Linux build
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CPP_STANDARD_OPTION} -fPIC -fpermissive")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fno-strict-aliasing")
set(CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG -D__DEBUG -O0")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CPP_STANDARD_OPTION} -fPIC -fpermissive")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -fno-strict-aliasing")
set(CMAKE_C_FLAGS_DEBUG   "${CMAKE_C_FLAGS_DEBUG} -D_DEBUG -D__DEBUG -O0")
if(MEDIA_BUILD_HARDENING)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS} -fstack-protector-all")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -D_FORTIFY_SOURCE=2")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CMAKE_C_FLAGS} -fstack-protector-all")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -D_FORTIFY_SOURCE=2")
endif()
if(MEDIA_BUILD_FATAL_WARNINGS)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS} -Werror")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CMAKE_C_FLAGS} -Werror")
endif()

set(GCC_SECURE_LINK_FLAGS "-z relro -z now")
set(CMAKE_SKIP_RPATH ON)

#include paths
include_directories(${CMAKE_CURRENT_LIST_DIR}/../proprietary/share/)
include_directories(${CMAKE_CURRENT_LIST_DIR}/../agnostic/share)
include_directories(${CMAKE_CURRENT_LIST_DIR}/../agnostic/hardware)
include_directories(${CMAKE_CURRENT_LIST_DIR}/../linux/share)
include_directories(${CMAKE_CURRENT_LIST_DIR}/../linux/hardware)

if(NOT "${LIBVA_INSTALL_PATH}" STREQUAL "")
    include_directories(BEFORE ${LIBVA_INSTALL_PATH})
    link_directories(BEFORE ${LIBVA_LIBRARY_PATH})
elseif(DEFINED ENV{LIBVA_INSTALL_PATH} AND NOT "$ENV{LIBVA_INSTALL_PATH}" STREQUAL "")
    include_directories(BEFORE $ENV{LIBVA_INSTALL_PATH})
    link_directories(BEFORE ${LIBVA_LIBRARY_PATH})
else()
    include(FindPkgConfig)
    if(NOT LIBVA_FOUND)
        pkg_check_modules(LIBVA REQUIRED libva>=1.8.0)
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

add_library(igfxcmrt SHARED "${CMRT_SOURCES}")

set(CMRT_DEFINES ${CMRT_DEFINES} VPHAL ISTDLIB_UMD CM_RT_EXPORTS)

# Extra defines for 64 bit builds
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
  set(CMRT_DEFINES ${CMRT_DEFINES} __CT__)
endif()
set_target_properties( igfxcmrt PROPERTIES 
  FOLDER CM_RT 
  COMPILE_DEFINITIONS "${CMRT_DEFINES}" 
  )

# set the version
get_cmrt_versions(CMRT_VERSION_MAJOR CMRT_VERSION_MINOR CMRT_VERSION_PATCH)
set(CMRT_VERSION ${CMRT_VERSION_MAJOR}.${CMRT_VERSION_MINOR}.${CMRT_VERSION_PATCH})

set_target_properties(igfxcmrt PROPERTIES VERSION ${CMRT_VERSION})
set_target_properties(igfxcmrt PROPERTIES SOVERSION ${CMRT_VERSION_MAJOR})
target_link_libraries( igfxcmrt ${CMAKE_DL_LIBS} va rt ${GCC_SECURE_LINK_FLAGS})

include(GNUInstallDirs)

configure_file(${CMAKE_CURRENT_LIST_DIR}/igfxcmrt.pc.in ${CMAKE_CURRENT_BINARY_DIR}/igfxcmrt.pc @ONLY)

install(TARGETS igfxcmrt LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT media NAMELINK_SKIP)
install(TARGETS igfxcmrt LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT media NAMELINK_ONLY)

set(CMRT_HEADERS
    ${CMRT_HEADERS}
    ${CMAKE_CURRENT_LIST_DIR}/../agnostic/share/cm_rt.h
    ${CMAKE_CURRENT_LIST_DIR}/../agnostic/share/cm_rt_g8.h
    ${CMAKE_CURRENT_LIST_DIR}/../agnostic/share/cm_rt_g9.h
    ${CMAKE_CURRENT_LIST_DIR}/../agnostic/share/cm_rt_g10.h
    ${CMAKE_CURRENT_LIST_DIR}/../agnostic/share/cm_rt_g11.h
    ${CMAKE_CURRENT_LIST_DIR}/../agnostic/share/cm_rt_g12_tgl.h
    ${CMAKE_CURRENT_LIST_DIR}/../agnostic/share/cm_rt_g12_dg1.h
    ${CMAKE_CURRENT_LIST_DIR}/../agnostic/share/cm_hw_vebox_cmd_g10.h
    ${CMAKE_CURRENT_LIST_DIR}/share/cm_rt_def_os.h
    ${CMAKE_CURRENT_LIST_DIR}/share/cm_rt_api_os.h
    )

file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/cm_rt_extension.h "//empty file for future implementation")

install(FILES ${CMRT_HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/igfxcmrt COMPONENT media)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/cm_rt_extension.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/igfxcmrt COMPONENT media)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/igfxcmrt.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig COMPONENT media)
