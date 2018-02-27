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

project(devult)

set(INC_PATH ./include ./)
include_directories(${INC_PATH})

aux_source_directory(./src GTEST_SOURCES)

set(SOURCES ./src/gtest-all.cc ./src/gtest_main.cc)

add_definitions(-DCMAKE_BUILD_TYPE=Debug)

SET(CMAKE_C_FLAGS "-g -O0")
SET(CMAKE_CXX_FLAGS "-g -O0")



ADD_LIBRARY(libgtest ${SOURCES})

LINK_DIRECTORIES(/usr/lib/x86_64-linux-gnu/)

find_package(Threads)
if (CMAKE_USE_PTHREADS_INIT)
    target_link_libraries(libgtest ${CMAKE_THREAD_LIBS_INIT})
endif()
