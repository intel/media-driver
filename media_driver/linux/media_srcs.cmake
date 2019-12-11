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

if(GEN8)
    media_include_subdirectory(gen8)
endif()

if(GEN9)
    media_include_subdirectory(gen9)
endif()

if(GEN9_CML)
    media_include_subdirectory(gen9_cml)
endif()

if(GEN9_CMPV)
    media_include_subdirectory(gen9_cmpv)
endif()

if(GEN9_BXT)
    media_include_subdirectory(gen9_bxt)
endif()

if(GEN9_SKL)
    media_include_subdirectory(gen9_skl)
endif()

if(GEN9_SKL)
    media_include_subdirectory(gen9_kbl)
endif()

if(GEN9_GLK)
    media_include_subdirectory(gen9_glk)
endif()

if(GEN9_CFL)
    media_include_subdirectory(gen9_cfl)
endif()

if(GEN10)
    media_include_subdirectory(gen10)
endif()

if(GEN10_CNL)
    media_include_subdirectory(gen10_cnl)
endif()

if(GEN11)
    media_include_subdirectory(gen11)
endif()

if(GEN12)
    media_include_subdirectory(gen12)
endif()

include(${MEDIA_EXT}/linux/media_srcs_ext.cmake OPTIONAL)
