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

media_include_subdirectory(common)

if(${GEN8_Supported} STREQUAL "yes")
    media_include_subdirectory(gen8)
endif()

if(${GEN8_BDW_Supported} STREQUAL "yes")
    media_include_subdirectory(gen8_bdw)
endif()

if(${GEN9_Supported} STREQUAL "yes")
    media_include_subdirectory(gen9)
endif()

if(${GEN9_BXT_Supported} STREQUAL "yes")
    media_include_subdirectory(gen9_bxt)
endif()

if(${GEN9_SKL_Supported} STREQUAL "yes")
    media_include_subdirectory(gen9_skl)
endif()

if(${GEN10_Supported} STREQUAL "yes")
    media_include_subdirectory(gen10)
endif()

if(${GEN10_CNL_Supported} STREQUAL "yes")
    media_include_subdirectory(gen10_cnl)
endif()

include(${CMAKE_CURRENT_LIST_DIR}/media_srcs_ext.cmake OPTIONAL)
