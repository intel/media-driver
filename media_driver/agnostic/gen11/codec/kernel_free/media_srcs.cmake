# Copyright (c) 2019-2022, Intel Corporation
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

function(gen_codec_kernel_from_cm)

    set(genx "g11")
    set(kind "")
    set(name "codec")
    set(platform "gen11")

    set(krn ig${name}krn_${genx})
    set(krn_dir ${CMAKE_SOURCE_DIR}/media_driver/agnostic/${platform}/${name}/kernel_free)
    set(out_dir ${CMAKE_SOURCE_DIR}/media_driver/agnostic/${platform}/${name}/kernel_free/cache_kernel)
    set(kernel_dir ${out_dir}/kernel)
    set(krn_header ${CMAKE_SOURCE_DIR}/media_driver/agnostic/common/${name}/kernel/${name}krnheader.h)

    message("krn: " ${krn})
    message("krn_dir: " ${krn_dir})
    message("out_dir: " ${out_dir})
    message("kernel_dir: " ${kernel_dir})
    message("krn_header: " ${krn_header})

    add_custom_command(
        OUTPUT ${out_dir} ${kernel_dir} ${patch_dir} ${kernel_hex_dir} ${patch_hex_dir}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${out_dir}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${kernel_dir}
        COMMENT "Creating ${name} cmfc kernels output directory")

    # Compiling all the sources in the kernel source directory.
    file(GLOB_RECURSE srcs ${krn_dir}/Source/*.cpp)

    set(objsname "")
    set(cm_genx icllp)

    foreach(src ${srcs})
        get_filename_component(obj ${src} NAME_WE) # there are other outputs from cmc command, but we use only .dat and .fcpatch

        add_custom_command(
            OUTPUT ${out_dir}/${obj}_0.dat
            DEPENDS ${src} ${out_dir}
            WORKING_DIRECTORY ${out_dir}
            COMMAND ${CMC}
                -c -Qxcm -Qxcm_jit_target=${cm_genx} 
                -I ${krn_dir}/Source/ 
                -Qxcm_jit_option="-nocompaction -output -binary" 
                -mCM_emit_common_isa 
                -mCM_no_input_reorder 
                #-mCM_unique_labels=MDF_FC 
                -mCM_printregusage
                -mCM_old_asm_name
                ${src}
            COMMENT "Compiling ${src}... to ${out_dir}/${obj}_0.dat\
                Checking if renaming is needed..."
        )

        set(objsname ${objsname} ${out_dir}/${obj}_0.dat)
    endforeach()

    add_custom_command(
        OUTPUT ${krn_dir}/${krn}.c ${krn_dir}/${krn}.h
        DEPENDS KernelBinToSource  ${objsname}
        WORKING_DIRECTORY ${out_dir}
        COMMAND ${PYTHON} ${CMAKE_SOURCE_DIR}//media_driver/agnostic/common/codec/kernel/merge.py -o ${krn}.krn ${objsname}
        COMMAND KernelBinToSource -i ${krn}.krn -o ${krn_dir}
        COMMENT "Generate source file from krn")

endfunction()

if(BUILD_KERNELS)
    gen_codec_kernel_from_cm()
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

set(CODEC_SOURCES_
    ${CODEC_SOURCES_}
    ${TMP_SOURCES_}
)

set(CODEC_HEADERS_
    ${CODEC_HEADERS_}
    ${TMP_HEADERS_}
)

source_group("Kernel\\CodecKernel" FILES ${TMP_SOURCES_} ${TMP_HEADERS_})
set(TMP_SOURCES_ "")
set(TMP_HEADERS_ "")

media_add_curr_to_include_path()
