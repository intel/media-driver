# Copyright (c) 2018, Intel Corporation
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
cmake_minimum_required(VERSION 3.1)

add_compile_options(-std=c++11)

include(${MEDIA_DRIVER_CMAKE}/linux/media_feature_flags_linux.cmake)
include(${MEDIA_DRIVER_CMAKE}/linux/media_gen_flags_linux.cmake)

if(NOT "${LIBVA_INSTALL_PATH}" STREQUAL "")
    include_directories(BEFORE ${LIBVA_INSTALL_PATH})
elseif(DEFINED ENV{LIBVA_INSTALL_PATH} AND NOT "$ENV{LIBVA_INSTALL_PATH}" STREQUAL "")
    include_directories(BEFORE $ENV{LIBVA_INSTALL_PATH})
else()
    include(FindPkgConfig)
    pkg_check_modules(LIBVA REQUIRED libva>=1.0.0)
    if(LIBVA_FOUND)
        include_directories(BEFORE ${LIBVA_INCLUDE_DIRS})
    endif()
endif()

ADD_SUBDIRECTORY(libdrm_mock)
ADD_SUBDIRECTORY(ult_app)

enable_testing()  
ADD_TEST(NAME test_devult COMMAND devult ${UMD_PATH})   
SET_TESTS_PROPERTIES(test_devult  
    PROPERTIES PASS_REGULAR_EXPRESSION "PASS")  
SET_TESTS_PROPERTIES(test_devult  
    PROPERTIES FAIL_REGULAR_EXPRESSION "FAIL")  
