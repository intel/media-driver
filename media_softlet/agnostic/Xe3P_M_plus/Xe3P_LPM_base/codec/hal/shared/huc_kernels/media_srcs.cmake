# Copyright (c) 2026, Intel Corporation
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

set(TMP_HUC_SOURCES_
    ${CMAKE_CURRENT_LIST_DIR}/nvlMediaKernels.cpp
)

set(TMP_HUC_HEADERS_
    ${CMAKE_CURRENT_LIST_DIR}/nvlMediaKernels.h
)

set(TMP_HUC_BIN_HEADERS_
    ${CMAKE_CURRENT_LIST_DIR}/nvlMediaKernel_av1ba.h
    ${CMAKE_CURRENT_LIST_DIR}/nvlMediaKernel_av1brc_init.h
    ${CMAKE_CURRENT_LIST_DIR}/nvlMediaKernel_av1brc_update.h
    ${CMAKE_CURRENT_LIST_DIR}/nvlMediaKernel_av1slbb_update.h
    ${CMAKE_CURRENT_LIST_DIR}/nvlMediaKernel_avcslbb_update.h
    ${CMAKE_CURRENT_LIST_DIR}/nvlMediaKernel_hevcslbb_update.h
    ${CMAKE_CURRENT_LIST_DIR}/nvlMediaKernel_avcbrc_init.h
    ${CMAKE_CURRENT_LIST_DIR}/nvlMediaKernel_avcbrc_update.h
    ${CMAKE_CURRENT_LIST_DIR}/nvlMediaKernel_copykrn.h
    ${CMAKE_CURRENT_LIST_DIR}/nvlMediaKernel_hevcbrc_init.h
    ${CMAKE_CURRENT_LIST_DIR}/nvlMediaKernel_s2l.h
    ${CMAKE_CURRENT_LIST_DIR}/nvlMediaKernel_hevcbrc_update.h
    ${CMAKE_CURRENT_LIST_DIR}/nvlMediaKernel_lookahead.h
    ${CMAKE_CURRENT_LIST_DIR}/nvlMediaKernel_pakint.h
    ${CMAKE_CURRENT_LIST_DIR}/nvlMediaKernel_vp9brc_init.h
    ${CMAKE_CURRENT_LIST_DIR}/nvlMediaKernel_vp9brc_update.h
    ${CMAKE_CURRENT_LIST_DIR}/nvlMediaKernel_vp9dec.h
    ${CMAKE_CURRENT_LIST_DIR}/nvlMediaKernel_vp9hpu.h
    ${CMAKE_CURRENT_LIST_DIR}/nvlMediaKernel_vvcs2l.h
)

set(SOFTLET_CODEC_COMMON_SOURCES_
    ${SOFTLET_CODEC_COMMON_SOURCES_}
    ${TMP_HUC_SOURCES_}
)

set(SOFTLET_CODEC_COMMON_HEADERS_
    ${SOFTLET_CODEC_COMMON_HEADERS_}
    ${TMP_HUC_HEADERS_}
)

set(MEDIA_BIN_SOURCES_
    ${MEDIA_BIN_SOURCES_}
    ${TMP_HUC_SOURCES_}
)

set(MEDIA_BIN_HEADERS_
    ${MEDIA_BIN_HEADERS_}
    ${TMP_HUC_HEADERS_}
    ${TMP_HUC_BIN_HEADERS_}
)

source_group( "Kernel\\CodecKernel" FILES ${TMP_HUC_SOURCES_} ${TMP_HUC_HEADERS_} ${TMP_HUC_BIN_HEADERS_} )
set(TMP_HUC_SOURCES_ "")
set(TMP_HUC_HEADERS_ "")
set(TMP_HUC_BIN_HEADERS_ "")

set(SOFTLET_CODEC_COMMON_PRIVATE_INCLUDE_DIRS_
    ${SOFTLET_CODEC_COMMON_PRIVATE_INCLUDE_DIRS_}
    ${CMAKE_CURRENT_LIST_DIR}
)

set(MEDIA_BIN_INCLUDE_DIR
    ${MEDIA_BIN_INCLUDE_DIR}
    ${CMAKE_CURRENT_LIST_DIR}
)