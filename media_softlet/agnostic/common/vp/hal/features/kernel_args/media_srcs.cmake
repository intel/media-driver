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

set(TMP_SOURCES_

)

set(TMP_HEADERS_
    ${CMAKE_CURRENT_LIST_DIR}/igvp3dlut_args.h
    ${CMAKE_CURRENT_LIST_DIR}/igvpfc_common_args.h
    ${CMAKE_CURRENT_LIST_DIR}/igvpfc_fp_args.h
    ${CMAKE_CURRENT_LIST_DIR}/igvpfc_420PL3_input_args.h
    ${CMAKE_CURRENT_LIST_DIR}/igvpfc_420PL3_output_args.h
    ${CMAKE_CURRENT_LIST_DIR}/igvpfc_444PL3_input_args.h
    ${CMAKE_CURRENT_LIST_DIR}/igvpfc_444PL3_output_args.h
    ${CMAKE_CURRENT_LIST_DIR}/igvpfc_422HV_input_args.h
)

set(SOFTLET_VP_SOURCES_
    ${SOFTLET_VP_SOURCES_}
    ${TMP_SOURCES_}
)

set(SOFTLET_VP_HEADERS_
    ${SOFTLET_VP_HEADERS_}
    ${TMP_HEADERS_}
)

source_group( VpHalNext\\Shared\\KernelArgs FILES ${TMP_SOURCES_} ${TMP_HEADERS_})
set(TMP_SOURCES_ "")
set(TMP_HEADERS_ "")
set (SOFTLET_VP_PRIVATE_INCLUDE_DIRS_
    ${SOFTLET_VP_PRIVATE_INCLUDE_DIRS_}
    ${CMAKE_CURRENT_LIST_DIR}
)