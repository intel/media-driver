# Copyright (c) 2017-2022, Intel Corporation
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

# encode
set(TMP_3_SOURCES_ "")

set(TMP_3_HEADERS_ "")

if((${HEVC_Encode_VME_Supported} STREQUAL "yes") AND (${CMRT_HEVC_ENC_FEI_Supported} STREQUAL "yes"))
    set(TMP_3_SOURCES_
        ${TMP_3_SOURCES_}
        ${CMAKE_CURRENT_LIST_DIR}/CMRTKernelBase.cpp
        ${CMAKE_CURRENT_LIST_DIR}/CMRTKernel_DownScaling.cpp
        ${CMAKE_CURRENT_LIST_DIR}/CMRTKernel_I_32x32.cpp
        ${CMAKE_CURRENT_LIST_DIR}/CMRTKernel_PB_32x32.cpp
        ${CMAKE_CURRENT_LIST_DIR}/CMRTKernel_I_16x16Sad.cpp
        ${CMAKE_CURRENT_LIST_DIR}/CMRTKernel_I_16x16Mode.cpp
        ${CMAKE_CURRENT_LIST_DIR}/CMRTKernel_I_8x8.cpp
        ${CMAKE_CURRENT_LIST_DIR}/CMRTKernel_I_8x8Mode.cpp
        ${CMAKE_CURRENT_LIST_DIR}/CMRTKernel_PB_8x8MbEnc.cpp
        ${CMAKE_CURRENT_LIST_DIR}/CMRTKernel_PB_8x8Pak.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Hme_Downscale_gen9.cpp
        ${CMAKE_CURRENT_LIST_DIR}/HevcEncFei_I_gen9.cpp
        ${CMAKE_CURRENT_LIST_DIR}/HevcEncFei_PB_gen9.cpp 
    )
    set(TMP_3_HEADERS_
        ${TMP_3_HEADERS_}
        ${CMAKE_CURRENT_LIST_DIR}/CMRTKernelBase.h
        ${CMAKE_CURRENT_LIST_DIR}/CMRTKernel_DownScaling.h
        ${CMAKE_CURRENT_LIST_DIR}/CMRTKernel_I_32x32.h
        ${CMAKE_CURRENT_LIST_DIR}/CMRTKernel_PB_32x32.h
        ${CMAKE_CURRENT_LIST_DIR}/CMRTKernel_I_16x16Sad.h
        ${CMAKE_CURRENT_LIST_DIR}/CMRTKernel_I_16x16Mode.h
        ${CMAKE_CURRENT_LIST_DIR}/CMRTKernel_I_8x8.h
        ${CMAKE_CURRENT_LIST_DIR}/CMRTKernel_I_8x8Mode.h
        ${CMAKE_CURRENT_LIST_DIR}/CMRTKernel_PB_8x8MbEnc.h
        ${CMAKE_CURRENT_LIST_DIR}/CMRTKernel_PB_8x8Pak.h
        ${CMAKE_CURRENT_LIST_DIR}/CMRTKernel_DS_Kernel_def.h
        ${CMAKE_CURRENT_LIST_DIR}/CMRTKernel_I_Kernel_def.h
        ${CMAKE_CURRENT_LIST_DIR}/CMRTKernel_PB_Kernel_def.h
        ${CMAKE_CURRENT_LIST_DIR}/CMRTKernel_header_file.h
        ${CMAKE_CURRENT_LIST_DIR}/Hme_Downscale_gen9.h
        ${CMAKE_CURRENT_LIST_DIR}/HevcEncFei_I_gen9.h
        ${CMAKE_CURRENT_LIST_DIR}/HevcEncFei_PB_gen9.h
    )
endif()

if ("${AVC_Encode_VME_Supported}" STREQUAL "yes")
    set (TMP_3_SOURCES_
        ${TMP_3_SOURCES_}
        ${CMAKE_CURRENT_LIST_DIR}/AvcEncFei_Mfe_gen9.cpp
    )
    set (TMP_3_HEADERS_
        ${TMP_3_HEADERS_}
        ${CMAKE_CURRENT_LIST_DIR}/AvcEncFei_Mfe_gen9.h
    )
endif ()

set(SOURCES_
    ${SOURCES_}
    ${TMP_3_SOURCES_}
)

set(HEADERS_
    ${HEADERS_}
    ${TMP_3_HEADERS_}
)

set(CODEC_SOURCES_
    ${CODEC_SOURCES_}
    ${TMP_3_SOURCES_}
)

set(CODEC_HEADERS_
    ${CODEC_HEADERS_}
    ${TMP_3_HEADERS_}
)

source_group( CodecHal\\Encode FILES ${TMP_3_HEADERS_} )
set(TMP_3_SOURCES_ "")
set(TMP_3_HEADERS_ "")

media_add_curr_to_include_path()
