# Copyright (c) 2019-2024, Intel Corporation
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
if(XE_LPM_PLUS_SUPPORT)
    media_include_subdirectory(xe_lpm_plus_r0)
    media_include_subdirectory(xe_lpm_plus)
endif()

if(MTL OR ARL)
    media_include_subdirectory(Xe_M_plus)
endif()

if(XE2_LPM_SUPPORT)
media_include_subdirectory(xe2_lpm)
media_include_subdirectory(xe2_lpm_r0)
endif()

if(XE2_HPM_SUPPORT)
media_include_subdirectory(xe2_hpm)
media_include_subdirectory(xe2_hpm_r0)
endif()

if(XE3_LPM_SUPPORT)
media_include_subdirectory(xe3_lpm)
media_include_subdirectory(xe3_lpm_r0)
endif()