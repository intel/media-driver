# Copyright (c) 2024, Intel Corporation

# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
# OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
# ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
# OTHER DEALINGS IN THE SOFTWARE.

# This folder is only for render engine related files!
# Do not add other files here.

if(XE_HPG OR XE_LPG)
    media_include_subdirectory(Xe_HPG_Base)
    media_include_subdirectory(Xe_HPG)
endif()

if(XE2_HPG)
    media_include_subdirectory(Xe2_HPG)
endif()

set(MEDIA_BIN_HEADERS_
    ${MEDIA_BIN_HEADERS_}
    ${CMAKE_CURRENT_LIST_DIR}/media_bin_register_xe_hpg.h
)

set(MEDIA_BIN_INCLUDE_DIR
    ${MEDIA_BIN_INCLUDE_DIR}
    ${CMAKE_CURRENT_LIST_DIR}
)
