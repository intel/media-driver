# Copyright (c) 2017-2018, Intel Corporation
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

# shared
set(TMP_2_HEADERS_
    ${CMAKE_CURRENT_LIST_DIR}/codec_def_common.h
    ${CMAKE_CURRENT_LIST_DIR}/codec_def_common_avc.h
    ${CMAKE_CURRENT_LIST_DIR}/codec_def_common_encode.h
    ${CMAKE_CURRENT_LIST_DIR}/codec_def_common_hevc.h
    ${CMAKE_CURRENT_LIST_DIR}/codec_def_common_jpeg.h
    ${CMAKE_CURRENT_LIST_DIR}/codec_def_common_mpeg2.h
    ${CMAKE_CURRENT_LIST_DIR}/codec_def_common_vp9.h
    ${CMAKE_CURRENT_LIST_DIR}/codec_def_vp8_probs.h
    ${CMAKE_CURRENT_LIST_DIR}/codec_def_vp9_probs.h
    ${CMAKE_CURRENT_LIST_DIR}/codec_def_decode_avc.h
    ${CMAKE_CURRENT_LIST_DIR}/codec_def_decode_hevc.h
    ${CMAKE_CURRENT_LIST_DIR}/codec_def_decode_jpeg.h
    ${CMAKE_CURRENT_LIST_DIR}/codec_def_decode_mpeg2.h
    ${CMAKE_CURRENT_LIST_DIR}/codec_def_decode_vc1.h
    ${CMAKE_CURRENT_LIST_DIR}/codec_def_decode_vp8.h
    ${CMAKE_CURRENT_LIST_DIR}/codec_def_decode_vp9.h
    ${CMAKE_CURRENT_LIST_DIR}/codec_def_decode.h
    ${CMAKE_CURRENT_LIST_DIR}/codec_def_encode_avc.h
    ${CMAKE_CURRENT_LIST_DIR}/codec_def_encode_hevc.h
    ${CMAKE_CURRENT_LIST_DIR}/codec_def_encode_jpeg.h
    ${CMAKE_CURRENT_LIST_DIR}/codec_def_encode_mpeg2.h
    ${CMAKE_CURRENT_LIST_DIR}/codec_def_encode_vp9.h
    ${CMAKE_CURRENT_LIST_DIR}/codec_def_encode_vp8.h
    ${CMAKE_CURRENT_LIST_DIR}/codec_def_encode.h
    ${CMAKE_CURRENT_LIST_DIR}/codec_def_cenc_decode.h
)

set(HEADERS_
    ${HEADERS_}
    ${TMP_2_HEADERS_}
)

source_group( "Codec\\Shared" FILES ${TMP_2_HEADERS_} )

media_add_curr_to_include_path()
