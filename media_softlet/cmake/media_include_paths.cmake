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

# external dependency
# Common path we need to include for now.
include_directories(${BS_DIR_INC})
include_directories(${BS_DIR_INC}/common)
include_directories(${BS_DIR_INC}/platform/iAlm)
include_directories(${BS_DIR_INC}/umKmInc)

# external components' header path which media depends on now
include_directories(${BS_DIR_SKUWA})
include_directories(${BS_DIR_GMMLIB}/inc)
include_directories(${BS_DIR_SOURCE}/huc/inc)

if(${PLATFORM} STREQUAL "linux")
    include(${MEDIA_SOFTLET_CMAKE}/linux/media_include_paths_linux.cmake)
else()
    include(${MEDIA_SOFTLET_EXT_CMAKE}/media_include_paths_ext.cmake OPTIONAL)
endif()
