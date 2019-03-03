# *****************************************************************************
# Copyright (c) 2017 Intel Corporation
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
# *****************************************************************************


if(NOT DEFINED _bs_include_dir_names)
set(_bs_include_dir_names TRUE)


include(${BUILD_SYS_INC}/bs_base_utils.cmake)


if(NOT DEFINED GFX_DEVELOPMENT_DIR)
    if(DEFINED ENV{GFX_DEVELOPMENT_DIR})
        set(GFX_DEVELOPMENT_DIR "$ENV{GFX_DEVELOPMENT_DIR}")
    else()
        message(FATAL_ERROR "Required variable not defined: GFX_DEVELOPMENT_DIR")
    endif()
endif(NOT DEFINED GFX_DEVELOPMENT_DIR)


# base dir vars
#
# These will be used later to help with separation of components.
#
bs_set_if_undefined(BS_DIR_SOURCE          "${CMAKE_SOURCE_DIR}/..")

bs_set_if_undefined(BS_DIR_COMMON          "${BS_DIR_SOURCE}/gmmlib/Source/Common")
bs_set_if_undefined(BS_DIR_GMMLIB          "${BS_DIR_SOURCE}/gmmlib/Source/GmmLib")
bs_set_if_undefined(BS_DIR_INC             "${BS_DIR_SOURCE}/gmmlib/Source/inc")
bs_set_if_undefined(BS_DIR_INSTALL         "${BS_DIR_SOURCE}/install")
bs_set_if_undefined(BS_DIR_MEDIA           "${CMAKE_SOURCE_DIR}")

endif(NOT DEFINED _bs_include_dir_names)
