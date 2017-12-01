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

include(GNUInstallDirs)

add_subdirectory(media_driver)

if (DEFINED BUILD_ALONG_WITH_CMRTLIB)
    add_subdirectory(cmrtlib)
endif()

if (TARGET iHD_drv_video AND TARGET igfxcmrt)

    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/media_driver/cmake/linux/intel-media.sh.in ${CMAKE_CURRENT_BINARY_DIR}/media_driver/cmake/linux/intel-media.sh)

    install (FILES ${CMAKE_CURRENT_BINARY_DIR}/media_driver/iHD_drv_video.so DESTINATION ${CMAKE_INSTALL_LIBDIR}/dri/)
    install (FILES ${CMAKE_CURRENT_BINARY_DIR}/media_driver/cmake/linux/intel-media.sh  DESTINATION ${CMAKE_INSTALL_FULL_SYSCONFDIR}/profile.d/)
    install (FILES ${CMAKE_CURRENT_BINARY_DIR}/cmrtlib/linux/igfxcmrt64.so DESTINATION ${CMAKE_INSTALL_LIBDIR})

    if (IS_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/../libva-install/usr/bin)
        install (DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/../libva-install/usr/bin DESTINATION ${CMAKE_INSTALL_PREFIX}
                 FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
    endif()
    if (IS_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/../libva-install/usr/include)
        install (DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/../libva-install/usr/include DESTINATION ${CMAKE_INSTALL_PREFIX})
    endif()
    if (IS_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/../libva-install/usr/lib)
        install (DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/../libva-install/usr/lib DESTINATION ${CMAKE_INSTALL_PREFIX})
    endif()

    if (IS_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/../libdrm-install/usr/lib)
        install (DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/../libdrm-install/usr/lib DESTINATION ${CMAKE_INSTALL_PREFIX})
    endif()

    set(CPACK_GENERATOR "DEB")
    set(CPACK_PACKAGE_NAME "intel-media")
    set(CPACK_PACKAGE_VENDOR "Intel")
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "intel-media deb package for internal usage only")

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

    include(CPack)

endif()
