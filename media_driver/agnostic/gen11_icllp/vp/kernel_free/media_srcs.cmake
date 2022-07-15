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

# iga64 -i <src> -o <dst> -p <genx> -a

# Function generate kernel for the specified platform. It assumes
# that kernel sources are located in a certain location (see ${krn_dir})
# calculated in a function body. And it assumes that output files should
# be stored in a certain locations (see ${krn_dir}/${krn}.h/c and ${common_header}).
# Notes:
#   - <platform> = gen<X>_<kind>, for example, gen11_icllp
#   - As of now this function works for the single platform only (gen11_icllp) while it
#  actully should process all the platforms at once. This single processing
#  is needed to generate shared ${common_header} with the unified kernels definition.
function(gen_kernel_from_asm)

    set(platform "gen11_icllp")
    set(name "vp")
    set(genx "g11")
    set(kind "icllp")

    set(krn_dir ${CMAKE_SOURCE_DIR}/media_driver/agnostic/${platform}/vp/kernel_free)
    set(link_file ${krn_dir}/component_release/LinkFile.txt)
    set(krn ig${name}krn_${genx}_${kind})
    set(header ${CMAKE_CURRENT_LIST_DIR}/${name}krnheader.h)
    set(common_header ${CMAKE_SOURCE_DIR}/media_common/agnostic/common/vp/kernel/${name}krnheader.h)
    set(cache_dir ${CMAKE_SOURCE_DIR}/media_driver/agnostic/${platform}/vp/kernel_free/cache_kernel)
    set(cache_hex_dir ${cache_dir}/hex)

    message("krn: " ${krn})
    message("krn_dir: " ${krn_dir})
    message("cache_dir: " ${cache_dir})
    message("link_file: " ${link_file})
    message("header: " ${header})
    message("common_header: " ${common_header})

    add_custom_command(
            OUTPUT ${cache_dir} ${cache_hex_dir}
            COMMAND ${CMAKE_COMMAND} -E make_directory ${cache_dir}
            COMMAND ${CMAKE_COMMAND} -E make_directory ${cache_hex_dir}
            COMMENT "Creating VP cmfc kernels output directory")

    # Compiling all the sources in the kernel source directory
    file(GLOB srcs ${krn_dir}/Source/*.asm)
    set(hexs "")
    foreach(src ${srcs})

        get_filename_component(obj ${src} NAME_WE)
        add_custom_command(
            OUTPUT ${obj}.hex
            DEPENDS KrnToHex_IGA ${src} ${cache_dir} ${cache_hex_dir}
            WORKING_DIRECTORY ${cache_hex_dir}
            COMMAND ${IGA} ${src} -p=11 -o ${obj}.krn
            COMMAND KrnToHex_IGA ${obj}.krn
            COMMAND ${CMAKE_COMMAND} -E remove ${obj}.krn
            COMMENT "Compiling ${src} ...\
                ${obj}.krn generated..."
        )

        list(APPEND hexs ${obj}.hex)

    endforeach()

    # Generating common .bin file containing all the kernels and its header
    # file shared across all platforms (${name}krnheader.h)
    add_custom_command(
        OUTPUT ${krn}.bin ${common_header}
        DEPENDS GenKrnBin ${hexs} ${link_file}
        COMMAND ${CMAKE_COMMAND} -E copy ${link_file} ${cache_hex_dir}
        COMMAND GenKrnBin ${cache_hex_dir} ${name} ${genx} icllp
        COMMAND ${CMAKE_COMMAND} -E copy ${cache_hex_dir}/${krn}.h ${header}
        #COMMAND ${CMAKE_COMMAND} -E copy ${header} ${common_header}
        COMMENT "Copying link file ${link_file} to folder ${cache_hex_dir}...\
            Generating bin files from hexes in ${cache_hex_dir}...\
            Copying header file ${krn}.h to ${common_header}..."
    )

    # Generating kernel from the .bin file
    message("Kernel file ${krn_dir}/${krn}.c will be generated")
    add_custom_command(
        OUTPUT ${krn_dir}/${krn}.c ${krn_dir}/${krn}.h
        DEPENDS KernelBinToSource ${krn}.bin ${common_header}
        WORKING_DIRECTORY ${cache_hex_dir}
        COMMAND KernelBinToSource -i ${krn}.bin -o ${krn_dir})
endfunction()

if(BUILD_KERNELS)
    gen_kernel_from_asm()
endif()

set(TMP_SOURCES_
    ${CMAKE_CURRENT_LIST_DIR}/igvpkrn_g11_icllp.c
)

set(TMP_HEADERS_
    ${CMAKE_CURRENT_LIST_DIR}/igvpkrn_g11_icllp.h
)


set(VP_SOURCES_
    ${VP_SOURCES_}
    ${TMP_SOURCES_}
)

set(VP_HEADERS_
    ${VP_HEADERS_}
    ${TMP_HEADERS_}
)

source_group( "Kernel\\VpKernel" FILES ${TMP_SOURCES_} ${TMP_HEADERS_} )
set(TMP_SOURCES_ "")
set(TMP_HEADERS_ "")

set(VP_PRIVATE_INCLUDE_DIRS_
    ${VP_PRIVATE_INCLUDE_DIRS_}
    ${CMAKE_CURRENT_LIST_DIR}
)