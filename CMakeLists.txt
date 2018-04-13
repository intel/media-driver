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

cmake_minimum_required(VERSION 3.5)


include(os_release_info.cmake)

get_os_release_info(os_name os_version)

if("${os_name}" STREQUAL "clear-linux-os")
    # clear-linux-os distribution avoids /etc for distribution defaults.
    # Set this variable explicitly before including GNUInstallDirs.
    set(CMAKE_INSTALL_SYSCONFDIR "usr/share/defaults/etc")
endif()

include(GNUInstallDirs)

add_subdirectory(cmrtlib)

add_subdirectory(media_driver)

if("${LIBVA_DRIVERS_PATH}" STREQUAL "")
    set(LIBVA_DRIVERS_PATH "${CMAKE_INSTALL_FULL_LIBDIR}/dri/")
endif()
message(STATUS "LIBVA_DRIVERS_PATH = ${LIBVA_DRIVERS_PATH}")

install (FILES ${CMAKE_CURRENT_BINARY_DIR}/media_driver/iHD_drv_video.so DESTINATION ${LIBVA_DRIVERS_PATH} COMPONENT media)
install (FILES ${CMAKE_CURRENT_BINARY_DIR}/cmrtlib/linux/igfxcmrt64.so DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT media)

option (INSTALL_DRIVER_SYSCONF "Install driver system configuration file" ON)
if (INSTALL_DRIVER_SYSCONF)
    configure_file (
        ${CMAKE_CURRENT_SOURCE_DIR}/media_driver/cmake/linux/intel-media.sh.in
        ${CMAKE_CURRENT_BINARY_DIR}/media_driver/cmake/linux/intel-media.sh)
    install (FILES ${CMAKE_CURRENT_BINARY_DIR}/media_driver/cmake/linux/intel-media.sh
        DESTINATION ${CMAKE_INSTALL_FULL_SYSCONFDIR}/profile.d/ COMPONENT media)
endif()

include(${CMAKE_CURRENT_LIST_DIR}/pack_ext.cmake OPTIONAL)

set(CPACK_GENERATOR "DEB")
set(CPACK_PACKAGE_NAME "intel-media")
set(CPACK_PACKAGE_VENDOR "Intel")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "intel-media deb package for internal usage only")
set(CPACK_RPM_PACKAGE_ARCHITECTURE "x86_64")

if (DEFINED MEDIA_VERSION)
    string(REPLACE "." ";" MEDIA_VERSION_LIST ${MEDIA_VERSION})
    list(GET MEDIA_VERSION_LIST 0 MEDIA_VERSION_MAJOR)
    list(GET MEDIA_VERSION_LIST 1 MEDIA_VERSION_MINOR)
    list(GET MEDIA_VERSION_LIST 2 MEDIA_VERSION_PATCH)
    set(CPACK_PACKAGE_VERSION_MAJOR "${MEDIA_VERSION_MAJOR}")
    set(CPACK_PACKAGE_VERSION_MINOR "${MEDIA_VERSION_MINOR}")
    set(CPACK_PACKAGE_VERSION_PATCH "${MEDIA_VERSION_PATCH}")
endif()

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
set(PACKAGE_VERSION "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")
string(TOLOWER "${BUILD_TYPE}.${CPACK_RPM_PACKAGE_ARCHITECTURE}" PACKAGE_TYPE)

set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${PACKAGE_VERSION}-${DISTRO_INFO}-${PACKAGE_TYPE}")

include(CPack)
