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

media_include_subdirectory(cmfcpatch)


set(TMP_SOURCES_
    ${CMAKE_CURRENT_LIST_DIR}/igvpkrn_xe2_hpg.c
    ${CMAKE_CURRENT_LIST_DIR}/igvpkrn_l0_xe2_hpg.c
    ${CMAKE_CURRENT_LIST_DIR}/igvpfc_common_xe2.cpp
    ${CMAKE_CURRENT_LIST_DIR}/igvpfc_fp_xe2.cpp
    ${CMAKE_CURRENT_LIST_DIR}/igvpfc_420PL3_input_xe2.cpp
    ${CMAKE_CURRENT_LIST_DIR}/igvpfc_420PL3_output_xe2.cpp
    ${CMAKE_CURRENT_LIST_DIR}/igvpfc_444PL3_input_xe2.cpp
    ${CMAKE_CURRENT_LIST_DIR}/igvpfc_444PL3_output_xe2.cpp
)


set(TMP_HEADERS_
    ${CMAKE_CURRENT_LIST_DIR}/igvpkrn_xe2_hpg.h
    ${CMAKE_CURRENT_LIST_DIR}/igvpkrn_l0_xe2_hpg.h
    ${CMAKE_CURRENT_LIST_DIR}/igvpfc_common_xe2.h
    ${CMAKE_CURRENT_LIST_DIR}/igvpfc_fp_xe2.h
    ${CMAKE_CURRENT_LIST_DIR}/igvpfc_420PL3_input_xe2.h
    ${CMAKE_CURRENT_LIST_DIR}/igvpfc_420PL3_output_xe2.h
    ${CMAKE_CURRENT_LIST_DIR}/igvpfc_444PL3_input_xe2.h
    ${CMAKE_CURRENT_LIST_DIR}/igvpfc_444PL3_output_xe2.h
)

set(SOFTLET_VP_SOURCES_
    ${SOFTLET_VP_SOURCES_}
    ${TMP_SOURCES_}
)

set(SOFTLET_VP_HEADERS_
    ${SOFTLET_VP_HEADERS_}
    ${TMP_HEADERS_}
)

set(MEDIA_BIN_SOURCES_
    ${MEDIA_BIN_SOURCES_}
    ${TMP_SOURCES_}
)

set(MEDIA_BIN_HEADERS_
    ${MEDIA_BIN_HEADERS_}
    ${TMP_HEADERS_}
)

source_group( "Kernel\\VpKernel" FILES ${TMP_SOURCES_} ${TMP_HEADERS_} )
set(TMP_SOURCES_ "")
set(TMP_HEADERS_ "")

set(SOFTLET_VP_PRIVATE_INCLUDE_DIRS_
    ${SOFTLET_VP_PRIVATE_INCLUDE_DIRS_}
    ${CMAKE_CURRENT_LIST_DIR}
)

set(MEDIA_BIN_INCLUDE_DIR
    ${MEDIA_BIN_INCLUDE_DIR}
    ${CMAKE_CURRENT_LIST_DIR}
)