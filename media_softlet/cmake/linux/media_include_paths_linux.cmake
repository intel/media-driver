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

include_directories(${BS_DIR_SKUWA}/linux)

if(NOT "${LIBVA_INSTALL_PATH}" STREQUAL "")
    include_directories(BEFORE ${LIBVA_INSTALL_PATH})
elseif(DEFINED ENV{LIBVA_INSTALL_PATH} AND NOT "$ENV{LIBVA_INSTALL_PATH}" STREQUAL "")
    include_directories(BEFORE $ENV{LIBVA_INSTALL_PATH})
else()
    include(FindPkgConfig)
    pkg_check_modules(LIBVA REQUIRED libva>=1.8.0)
    if(LIBVA_FOUND)
        include_directories(BEFORE ${LIBVA_INCLUDE_DIRS})
        if("${LIBVA_DRIVERS_PATH}" STREQUAL "")
            pkg_get_variable(LIBVA_DRIVERS_PATH libva driverdir)
            set(LIBVA_DRIVERS_PATH ${LIBVA_DRIVERS_PATH} PARENT_SCOPE)
        endif()
    endif()
endif()

include(${MEDIA_SOFTLET_EXT_CMAKE}/linux/media_include_paths_linux_ext.cmake OPTIONAL)
