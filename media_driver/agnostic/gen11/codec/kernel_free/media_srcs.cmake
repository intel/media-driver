# Copyright (c) 2019, Intel Corporation
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

if (BUILD_KERNELS)

    function(gen_cm_kernel_from_source name platform)

        set(cm_krn_dir ${CMAKE_SOURCE_DIR}/media_driver/agnostic/${platform}/codec/kernel_free)
        set(cm_krn ig${name}krn_g11)

        # Generating source from the .krn file
        add_custom_command(
            OUTPUT ${cm_krn_dir}/${cm_krn}.c ${cm_krn_dir}/${cm_krn}.h
            DEPENDS KernelBinToSource
            COMMAND cp -r ${CMAKE_CURRENT_LIST_DIR}/Source .
            COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_LIST_DIR}/build.py .
            COMMAND ${PYTHON} ARGS build.py
            COMMAND ${CMAKE_COMMAND} -E rename commonkernel.krn ${cm_krn}.krn
            COMMAND KernelBinToSource ARGS -i ${cm_krn}.krn -o . -v ${cm_krn} -index 14 -t 17
            COMMAND ${CMAKE_COMMAND} -E copy ${cm_krn}.c ${cm_krn}.h ${CMAKE_CURRENT_LIST_DIR}/
            COMMENT "Generate source file from krn")
    endfunction()

    if(GEN11_ICLLP)

        gen_cm_kernel_from_source(codec gen11)

    endif()
endif()

set(TMP_SOURCES_
    ${CMAKE_CURRENT_LIST_DIR}/igcodeckrn_g11.c
)

set(TMP_HEADERS_
    ${CMAKE_CURRENT_LIST_DIR}/igcodeckrn_g11.h
)

set(SOURCES_
    ${SOURCES_}
    ${TMP_SOURCES_}
)

set(HEADERS_
    ${HEADERS_}
    ${TMP_HEADERS_}
)

source_group("Kernel\\CodecKernel" FILES ${TMP_SOURCES_} ${TMP_HEADERS_})


media_add_curr_to_include_path()
