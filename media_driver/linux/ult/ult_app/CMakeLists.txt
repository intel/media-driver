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
cmake_minimum_required(VERSION 3.1)

project(drvult)

add_subdirectory(googletest)

set(INC_PATH ./googletest/include ../inc ../../../linux/common/ddi)
include_directories(${INC_PATH} ${LIBVA_PATH})
include_directories(cm
  ../../../../../GmmLib/inc
  ../../../../../inc/common
  ../../../agnostic/common/cm
  ../../../agnostic/common/os
  ../../../agnostic/common/vp/hal
  ../../../agnostic/common/hw
  ../../../agnostic/common/renderhal
  ../../../agnostic/common/codec/shared
  ../../../agnostic/common/heap_manager
  ../../../linux/common/cm
  ../../../linux/common/os/libdrm/include
  ../../../linux/common/os
  ../../../linux/common/cp/os
  ../../../linux/common/cp/hw)

if (NOT "${BS_DIR_GMMLIB}" STREQUAL "")
    include_directories(${BS_DIR_GMMLIB}/inc)
endif()

if (NOT "${BS_DIR_INC}" STREQUAL "")
   include_directories(${BS_DIR_INC} ${BS_DIR_INC}/common)
endif()

aux_source_directory(. SOURCES)
aux_source_directory(cm SOURCES)
add_compile_options(-std=c++11)
set(CMAKE_C_COMPILER "g++")

SET(CMAKE_C_FLAGS "-g -O0")
SET(CMAKE_CXX_FLAGS "-g -O0")

add_executable(drvult ${SOURCES})

LINK_DIRECTORIES(/usr/lib/x86_64-linux-gnu/)
TARGET_LINK_LIBRARIES(drvult libgtest libdl.so)


if (DEFINED BUILD_TYPE AND "${BUILD_TYPE}" STREQUAL "debug")
	set(BYPASS_MEDIA_ULT "yes")
endif ()

if (DEFINED BYPASS_MEDIA_ULT AND "${BYPASS_MEDIA_ULT}" STREQUAL "yes")
    # must explictly pass along BYPASS_MEDIA_ULT as yes then could bypass the running of media ult
    message("-- media -- BYPASS_MEDIA_ULT = ${BYPASS_MEDIA_ULT}")
else ()

if (NOT "${Full_Open_Source_Support}" STREQUAL "yes")
add_custom_target(RunULT ALL DEPENDS iHD_drv_video drvult)

if(${Use_CP_Stub} STREQUAL "no")
add_custom_command(
    TARGET RunULT
    COMMENT "copying coreu"
    COMMAND cp ${CMAKE_BINARY_DIR}/coreu/libcoreuclient/libcoreuclient.so ${CMAKE_BINARY_DIR}/
    COMMAND cp ${CMAKE_BINARY_DIR}/coreu/coreu_interface/libcoreuinterface.so ${CMAKE_BINARY_DIR}/
)
endif()

add_custom_command(
    TARGET RunULT
    POST_BUILD
    COMMAND LD_PRELOAD=../libdrm_mock/libdrm_mock.so ./drvult ../../../iHD_drv_video.so
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Running drvult..."
)
endif ()

endif ()
