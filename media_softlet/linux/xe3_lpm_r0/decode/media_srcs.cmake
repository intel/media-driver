# Copyright (c) 2024, Intel Corporation
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

if(${AV1_Decode_Supported} STREQUAL "yes")
    media_include_subdirectory(av1)
endif()
if(${AVC_Decode_Supported} STREQUAL "yes")
    media_include_subdirectory(avc)
endif()
if(${JPEG_Decode_Supported} STREQUAL "yes")
    media_include_subdirectory(jpeg)
endif()
if(${HEVC_Decode_Supported} STREQUAL "yes")
    media_include_subdirectory(hevc)
endif()
if(${MPEG2_Decode_Supported} STREQUAL "yes")
    media_include_subdirectory(mpeg2)
endif()
if(${VP8_Decode_Supported} STREQUAL "yes")
    media_include_subdirectory(vp8)
endif()
if(${VP9_Decode_Supported} STREQUAL "yes")
    media_include_subdirectory(vp9)
endif()
if(${VVC_Decode_Supported} STREQUAL "yes")
    media_include_subdirectory(vvc)
endif()