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

bs_set_if_undefined(GEN8_Supported "yes")
bs_set_if_undefined(GEN8_BDW_Supported "yes")
bs_set_if_undefined(GEN9_Supported "yes")
bs_set_if_undefined(GEN9_BXT_Supported "yes")
bs_set_if_undefined(GEN9_SKL_Supported "yes")
bs_set_if_undefined(GEN9_CFL_Supported "yes")
bs_set_if_undefined(GEN9_GLK_Supported "yes")
bs_set_if_undefined(GEN9_KBL_Supported "yes")
bs_set_if_undefined(GEN10_Supported "yes")
bs_set_if_undefined(GEN10_CNL_Supported "yes")


if(${GEN8_Supported} STREQUAL "yes")
    add_definitions(-DIGFX_GEN8_SUPPORTED)
endif()

if(${GEN8_BDW_Supported} STREQUAL "yes")
    add_definitions(-DIGFX_GEN8_BDW_SUPPORTED)
endif()

if(${GEN9_Supported} STREQUAL "yes")
    add_definitions(-DIGFX_GEN9_SUPPORTED)
endif()

if(${GEN9_BXT_Supported} STREQUAL "yes")
    add_definitions(-DIGFX_GEN9_BXT_SUPPORTED)
endif()

if(${GEN9_SKL_Supported} STREQUAL "yes")
    add_definitions(-DIGFX_GEN9_SKL_SUPPORTED)
endif()

if(${GEN9_CFL_Supported} STREQUAL "yes")
    add_definitions(-DIGFX_GEN9_CFL_SUPPORTED)
endif()

if(${GEN9_GLK_Supported} STREQUAL "yes")
    add_definitions(-DIGFX_GEN9_GLK_SUPPORTED)
endif()

if(${GEN9_KBL_Supported} STREQUAL "yes")
    add_definitions(-DIGFX_GEN9_KBL_SUPPORTED)
endif()

if(${GEN10_Supported} STREQUAL "yes")
    add_definitions(-DIGFX_GEN10_SUPPORTED)
endif()

if(${GEN10_CNL_Supported} STREQUAL "yes")
    add_definitions(-DIGFX_GEN10_CNL_SUPPORTED)
endif()


include(${MEDIA_DRIVER_CMAKE}/ext/linux/media_gen_flags_linux_ext.cmake OPTIONAL)
