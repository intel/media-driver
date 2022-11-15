# Copyright (c) 2020-2022, Intel Corporation
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

set(TMP_MMC_SOURCES_
    ${CMAKE_CURRENT_LIST_DIR}/codec_mem_compression_xe_lpm_plus_base_next.cpp
)

set(TMP_MMC_HEADERS_
    ${CMAKE_CURRENT_LIST_DIR}/codec_mem_compression_xe_lpm_plus_base_next.h
)

set(TMP_HW_SOURCES_
    ${CMAKE_CURRENT_LIST_DIR}/codec_hw_xe_lpm_plus_base.cpp
)

set(TMP_HW_HEADERS_
    ${CMAKE_CURRENT_LIST_DIR}/codec_hw_xe_lpm_plus_base.h
)

set(SOFTLET_CODEC_COMMON_SOURCES_
    ${SOFTLET_CODEC_COMMON_SOURCES_}
    ${TMP_MMC_SOURCES_}
    ${TMP_HW_SOURCES_}
)

set(SOFTLET_CODEC_COMMON_HEADERS_
    ${SOFTLET_CODEC_COMMON_HEADERS_}
    ${TMP_MMC_HEADERS_}
    ${TMP_HW_HEADERS_}
)


source_group(CodecHalNext\\Xe_M_plus\\shared\\mmc FILES ${TMP_MMC_SOURCES_} ${TMP_MMC_HEADERS_})
source_group(CodecHalNext\\Xe_M_plus\\shared\\hw FILES ${TMP_HW_SOURCES_} ${TMP_HW_HEADERS_})
set(TMP_MMC_SOURCES_ "")
set(TMP_MMC_HEADERS_ "")
set(TMP_HW_SOURCES_ "")
set(TMP_HW_HEADERS_ "")
set(SOFTLET_CODEC_COMMON_PRIVATE_INCLUDE_DIRS_
    ${SOFTLET_CODEC_COMMON_PRIVATE_INCLUDE_DIRS_}
    ${CMAKE_CURRENT_LIST_DIR}
)