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


if(NOT DEFINED BS_USE_OSDM_BUILD_SYSTEM)
    if(DEFINED ENV{BS_USE_OSDM_BUILD_SYSTEM})
        set(BS_USE_OSDM_BUILD_SYSTEM "$ENV{BS_USE_OSDM_BUILD_SYSTEM}")
    else()
        set(BS_USE_OSDM_BUILD_SYSTEM TRUE)
    endif()
endif()


include(${BUILD_SYS_INC}/bs_base_utils.cmake)

bs_check_build_type()

if(DEFINED UFO_ANDROID_VERSION)
    add_definition(-DANDROID_VERSION=${UFO_ANDROID_VERSION})
elseif(DEFINED ENV{UFO_ANDROID_VERSION})
    add_definition(-DANDROID_VERSION=$ENV{UFO_ANDROID_VERSION})
endif()

# Ensure definitions for some variables that should be set by the overall build system.
bs_set_if_undefined(PLATFORM "linux")
bs_set_if_undefined(ARCH "64")
bs_set_if_undefined(GFXGEN "9")
bs_set_if_undefined(CXXFLAGS "-D_GLIBCXX_USE_CXX11_ABI=0")

# Other useful definitions
bs_set_if_undefined(CMAKE_EXPORT_COMPILE_COMMANDS "ON")
bs_set_if_undefined(CMAKE_VERBOSE_MAKEFILE "1")

## bs_set_if_undefined(UFO_CMAKE_DEBUG "ON")
## bs_set_if_undefined(BUILD_VERSION_STRING "9999")


include(${BUILD_SYS_INC}/bs_dir_names.cmake)

# Needed for bs_add_some_common_includes
include(${BUILD_SYS_INC}/utils.cmake)

if(BS_USE_OSDM_BUILD_SYSTEM)

include(${BUILD_SYS_INC}/stdvars.cmake)

include(${BUILD_SYS_DIR}/build_sys.cmake)

bs_set_base_flags()

endif(BS_USE_OSDM_BUILD_SYSTEM)
