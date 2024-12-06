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

cmake_minimum_required(VERSION 3.5)
project(IntelMediaDriver)
set(MEDIA_VERSION "25.1.0${MEDIA_VERSION_EXTRA}" CACHE STRING "" FORCE)






include(os_release_info.cmake)

get_os_release_info(os_name os_version)

if(NOT DEFINED CMAKE_INSTALL_PREFIX OR CMAKE_INSTALL_PREFIX STREQUAL "")
    set(CMAKE_INSTALL_PREFIX "/usr/local" CACHE PATH "..." FORCE)
endif()
message(STATUS "CMAKE_INSTALL_PREFIX = ${CMAKE_INSTALL_PREFIX}")

if("${os_name}" STREQUAL "clear-linux-os")
    # clear-linux-os distribution avoids /etc for distribution defaults.
    # Set this variable explicitly before including GNUInstallDirs.
    set(CMAKE_INSTALL_SYSCONFDIR "usr/share/defaults/etc")
endif()

include(CMakeDependentOption)

option (ENABLE_KERNELS "Build driver with shaders (kernels) support" ON)
# ENABLE_NONFREE_KERNELS option permits to enable closed source EU shaders (kernels)
# additionally to open source EU shaders and fixed function hardware.
cmake_dependent_option( ENABLE_NONFREE_KERNELS
    "Use closed source pre-built  binary shaders (kernels)" ON
    "ENABLE_KERNELS" OFF)
cmake_dependent_option( BUILD_KERNELS
    "Rebuild shaders (kernels) from sources" OFF
    "ENABLE_KERNELS;NOT ENABLE_NONFREE_KERNELS" OFF)
option (BUILD_CMRTLIB "Build and Install cmrtlib together with media driver" ON)

option (ENABLE_PRODUCTION_KMD "Enable Production KMD header files" OFF)

include(GNUInstallDirs)

if (BUILD_CMRTLIB AND NOT CMAKE_WDDM_LINUX)
    add_subdirectory(cmrtlib)
endif()

if (BUILD_KERNELS)
    find_program(IGA iga64)
    if (NOT IGA)
        message(FATAL_ERROR "Failed to find IGA")
    endif()

    find_program(CMC cmc)
    if (NOT CMC)
        message(FATAL_ERROR "Failed to find CMC")
    endif()

    find_program(PYTHON python)
    if (NOT PYTHON)
        message(FATAL_ERROR "Failed to find PYTHON")
    endif()

    add_subdirectory(Tools/MediaDriverTools)
endif()

add_subdirectory(media_driver)

if("${LIBVA_DRIVERS_PATH}" STREQUAL "")
    set(LIBVA_DRIVERS_PATH "${CMAKE_INSTALL_FULL_LIBDIR}/dri/")
endif()
message(STATUS "LIBVA_DRIVERS_PATH = ${LIBVA_DRIVERS_PATH}")

install (PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/media_driver/iHD_drv_video.so DESTINATION ${LIBVA_DRIVERS_PATH} COMPONENT media)

option (INSTALL_DRIVER_SYSCONF "Install driver system configuration file" OFF)
if (INSTALL_DRIVER_SYSCONF)
    configure_file (
        ${CMAKE_CURRENT_SOURCE_DIR}/media_driver/cmake/linux/intel-media.sh.in
        ${CMAKE_CURRENT_BINARY_DIR}/media_driver/cmake/linux/intel-media.sh)
    install (FILES ${CMAKE_CURRENT_BINARY_DIR}/media_driver/cmake/linux/intel-media.sh
        DESTINATION ${CMAKE_INSTALL_FULL_SYSCONFDIR}/profile.d/ COMPONENT media)
endif()

include(${CMAKE_CURRENT_LIST_DIR}/pack_ext.cmake OPTIONAL)

if (NOT MEDIA_PACKING)
    set(CPACK_GENERATOR "DEB" "RPM")
else()
    set(CPACK_GENERATOR ${MEDIA_PACKAGING})
endif()
set(CPACK_PACKAGE_NAME "intel-media")
set(CPACK_PACKAGE_VENDOR "Intel")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "intel-media Open Source Driver")
set(CPACK_RPM_PACKAGE_ARCHITECTURE "x86_64")
set(CPACK_RPM_PACKAGE_GROUP "System Environment/Libraries")
set(CPACK_RPM_PACKAGE_LICENSE "Intel")

set(CPACK_PACKAGE_VERSION ${MEDIA_VERSION})

set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Intel")

# Create Distro OS initial based on 1st-2nd letter of string
# or label "generic" for unsupported/unknown distros for CPACK_PACKAGE_FILE_NAME
if("${os_name}" STREQUAL "ubuntu")
    set(DISTRO_INFO "u${os_version}")
elseif("${os_name}" STREQUAL "fedora")
    set(DISTRO_INFO "f${os_version}")
elseif("${os_name}" STREQUAL "clear-linux-os")
    set(DISTRO_INFO "cl${os_version}")
elseif("${os_name}" STREQUAL "centos")
    set(DISTRO_INFO "ce${os_version}")
else()
    set(DISTRO_INFO "generic")
endif()

# Format Package Version and Build Type Strings for CPACK_PACKAGE_FILE_NAME
string(TOLOWER "${BUILD_TYPE}.${CPACK_RPM_PACKAGE_ARCHITECTURE}" PACKAGE_TYPE)

set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${DISTRO_INFO}-${PACKAGE_TYPE}")

include(CPack)

