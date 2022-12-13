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

#check whether need some flags to guard the including of below folders

function(gen_vpkernel_from_cm)

    set(genx "g12")
    set(kind "tgllp")
    set(name "vp")
    set(platform "gen12_tgllp")

    set(krn ig${name}krn_${genx}_${kind}_cmfc)
    set(krnpatch ig${name}krn_${genx}_${kind}_cmfcpatch)
    set(krn_dir ${CMAKE_SOURCE_DIR}/media_driver/agnostic/${platform}/vp/kernel_free)
    set(link_file ${krn_dir}/component_release/LinkFile.txt)
    set(cache_dir ${CMAKE_SOURCE_DIR}/media_driver/agnostic/${platform}/vp/kernel_free/cache_kernel)
    set(kernel_dir ${cache_dir}/kernel)
    set(patch_dir ${cache_dir}/fcpatch)
    set(kernel_hex_dir ${kernel_dir}/hex)
    set(patch_hex_dir ${patch_dir}/hex)
    set(common_header ${CMAKE_SOURCE_DIR}/media_common/agnostic/common/vp/kernel/${name}krnheader.h)
    set(header ${CMAKE_CURRENT_LIST_DIR}/${name}krnheader.h)

    message("krn: " ${krn})
    message("krnpatch: " ${krnpatch})
    message("krn_dir: " ${krn_dir})
    message("cache_dir: " ${cache_dir})
    message("link_file: " ${link_file})
    message("kernel_dir: " ${kernel_dir})
    message("patch_dir: " ${patch_dir})
    message("kernel_hex_dir: " ${kernel_hex_dir})
    message("patch_hex_dir: " ${patch_hex_dir})
    message("common_header: " ${common_header})
    message("header: " ${header})

    add_custom_command(
        OUTPUT ${cache_dir} ${kernel_dir} ${patch_dir} ${kernel_hex_dir} ${patch_hex_dir}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${cache_dir}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${kernel_dir}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${patch_dir}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${kernel_hex_dir}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${patch_hex_dir}
        COMMENT "Creating VP cmfc kernels output directory")

    # Compiling all the sources in the kernel source directory.
    file(GLOB_RECURSE srcs ${krn_dir}/Source/*.cpp)

    set(objsname "")
    set(cm_genx tgllp)

    foreach(src ${srcs})
        get_filename_component(obj ${src} NAME_WE) # there are other outputs from cmc command, but we use only .dat and .fcpatch

        string(FIND ${obj} "Dscale" retVal)
        if(NOT retVal STREQUAL "-1")
            string(REPLACE "Dscale" "DScale" obj ${obj})
        endif()

        if(obj STREQUAL "EOT" OR obj STREQUAL "Secure_Block_Copy") # "EOT" and "Secure_Block_Copy" don't generate the related .fcpatch file.
            add_custom_command(
                OUTPUT ${cache_dir}/${obj}_0.dat
                DEPENDS ${src} ${cache_dir}
                WORKING_DIRECTORY ${cache_dir}
                COMMAND ${CMC}
                    -c -Qxcm -Qxcm_jit_target=${cm_genx} 
                    -I ${krn_dir}/Source/ 
                    -I ${krn_dir}/Source/Common 
                    -I ${krn_dir}/Source/Components
                    -I ${krn_dir}/Source/Core_Kernels 
                    -Qxcm_jit_option="-nocompaction -output -binary" 
                    -mCM_emit_common_isa 
                    -mCM_no_input_reorder 
                    -mCM_unique_labels=MDF_FC 
                    -mCM_printregusage
                    -mCM_old_asm_name
                    ${src}
                    && python ${krn_dir}/Source/rename.py ${cache_dir} ${obj}_0.dat
                COMMENT "Compiling ${src}... to ${cache_dir}/${obj}_0.dat\
                    Checking if renaming is needed..."
            )
        else()
            add_custom_command(
                OUTPUT ${cache_dir}/${obj}_0.dat ${cache_dir}/${obj}.fcpatch
                DEPENDS ${src} ${cache_dir}
                WORKING_DIRECTORY ${cache_dir}
                COMMAND ${CMC}
                    -c -Qxcm -Qxcm_jit_target=${cm_genx} 
                    -I ${krn_dir}/Source/ 
                    -I ${krn_dir}/Source/Common 
                    -I ${krn_dir}/Source/Components
                    -I ${krn_dir}/Source/Core_Kernels 
                    -Qxcm_jit_option="-nocompaction -output -binary" 
                    -mCM_emit_common_isa 
                    -mCM_no_input_reorder 
                    -mCM_unique_labels=MDF_FC 
                    -mCM_printregusage
                    -mCM_old_asm_name
                    ${src}
                    && python ${krn_dir}/Source/rename.py ${cache_dir} ${obj}_0.dat ${obj}.fcpatch
                COMMENT "Compiling ${src}... to ${cache_dir}/${obj}_0.dat\
                    Checking if renaming is needed..."
            )
        endif()
        list(APPEND objsname
            ${obj}
        )
    endforeach()

    #Generate the .hex files from the .dat files by using KrnToHex.
    set(hexs "")

    foreach(objname ${objsname})
        add_custom_command(
        OUTPUT ${kernel_dir}/${objname}.krn
        DEPENDS ${cache_dir}/${objname}_0.dat ${kernel_dir}
        WORKING_DIRECTORY ${cache_dir}
        COMMAND ${CMAKE_COMMAND} -E copy ${cache_dir}/${objname}_0.dat ${kernel_dir}/${objname}.krn
        )
    endforeach()

    foreach(objname ${objsname})
        add_custom_command(
            OUTPUT ${kernel_hex_dir}/${objname}.hex
            DEPENDS KrnToHex ${kernel_dir}/${objname}.krn ${kernel_hex_dir}
            WORKING_DIRECTORY ${kernel_dir}
            COMMAND KrnToHex ${kernel_dir}/${objname}.krn
            COMMAND ${CMAKE_COMMAND} -E copy ${kernel_dir}/${objname}.hex ${kernel_hex_dir}/${objname}.hex
            COMMAND ${CMAKE_COMMAND} -E remove ${kernel_dir}/${objname}.hex
            COMMENT "Generate the hex files of cmfc kernel"
        )
        set(hexs ${hexs} ${kernel_hex_dir}/${objname}.hex)
    endforeach()

    ##Generate the .hex files from the .fcpatch files by using KrnToHex.

    list(REMOVE_ITEM objsname "EOT" "Secure_Block_Copy") # Remove "EOT" and "Secure_Block_Copy".

    foreach(objname ${objsname})
        add_custom_command(
            OUTPUT ${patch_dir}/${objname}.krn
            DEPENDS ${cache_dir}/${objname}.fcpatch ${patch_dir}
            WORKING_DIRECTORY ${cache_dir}
            COMMAND ${CMAKE_COMMAND} -E copy ${cache_dir}/${objname}.fcpatch ${patch_dir}/${objname}.krn
    )
    endforeach()

    set(fcpatch_hexs "")
    foreach(objname ${objsname})
        add_custom_command(
            OUTPUT ${patch_hex_dir}/${objname}.hex
            DEPENDS KrnToHex ${patch_dir}/${objname}.krn ${patch_hex_dir}
            WORKING_DIRECTORY ${patch_dir}
            COMMAND KrnToHex ${patch_dir}/${objname}.krn
            COMMAND ${CMAKE_COMMAND} -E copy ${patch_dir}/${objname}.hex ${patch_hex_dir}/${objname}.hex
            COMMAND ${CMAKE_COMMAND} -E remove ${patch_dir}/${objname}.hex
            COMMENT "Generate the hex files of cmfc patch"
        )
        set(fcpatch_hexs ${fcpatch_hexs} ${patch_hex_dir}/${objname}.hex)
    endforeach()

    add_custom_command(
        OUTPUT  ${common_header}
        DEPENDS GenDmyHex ${kernel_hex_dir}   #Generate the dummy hexs from the pre-built header
        COMMAND GenDmyHex ${kernel_hex_dir} ${common_header}
        COMMENT "Generating ${common_header} in ${kernel_hex_dir} using GenDmyHex..."
    )

    # Generating the .bin files for cmfc kernel and patch respectively.

    add_custom_command(
        OUTPUT ${kernel_hex_dir}/${krn}.bin
        DEPENDS GenDmyHex GenKrnBin ${hexs} ${link_file}   #Generate the dummy hexs from the pre-built header
        WORKING_DIRECTORY ${kernel_hex_dir}
        COMMAND ${CMAKE_COMMAND} -E copy ${link_file} ${kernel_hex_dir}
        COMMAND GenKrnBin ${kernel_hex_dir} ${name} ${genx} tgllp_cmfc
        COMMAND ${CMAKE_COMMAND} -E copy ${kernel_hex_dir}/${krn}.h ${header}
        #COMMAND ${CMAKE_COMMAND} -E copy ${header} ${common_header}
        COMMENT "Copying ${link_file} to ${kernel_hex_dir}\
            Generating ${name} ${genx} ${kind} in ${kernel_hex_dir}...\
            Copying ${kernel_hex_dir}/${krn}.h to ${common_header}..."
    )

    add_custom_command(
        OUTPUT ${patch_hex_dir}/${krnpatch}.bin
        DEPENDS GenKrnBin ${fcpatch_hexs} ${link_file}
        WORKING_DIRECTORY ${patch_hex_dir}
        COMMAND ${CMAKE_COMMAND} -E copy ${link_file} ${patch_hex_dir}
        COMMAND GenKrnBin ${patch_hex_dir} ${name} ${genx} tgllp_cmfcpatch
        COMMENT "Copying ${link_file} to ${patch_hex_dir}...\
            GenKrnBin ${patch_hex_dir} ${name} ${genx} tgllp_cmfcpatch"
    )

    # Generating kernel source files for cmfc kernel and patch.

    message("Kernel file ${krn_dir}/cmfc/${krn}.c will be generated")
    add_custom_command(
        OUTPUT ${krn_dir}/cmfc/${krn}.c ${krn_dir}/cmfc/${krn}.h
        DEPENDS KernelBinToSource ${kernel_hex_dir}/${krn}.bin
        COMMAND KernelBinToSource -i ${kernel_hex_dir}/${krn}.bin -o ${krn_dir}/cmfc
        COMMENT "Generating kernel file...\
            KernelBinToSource -i ${kernel_hex_dir}/${krn}.bin -o ${krn_dir}/cmfc"
    )

    message("FC patch file ${krn_dir}/cmfcpatch/${krnpatch}.c will be generated")
    add_custom_command(
        OUTPUT ${krn_dir}/cmfcpatch/${krnpatch}.c ${krn_dir}/cmfcpatch/${krnpatch}.h
        DEPENDS KernelBinToSource ${patch_hex_dir}/${krnpatch}.bin
        COMMAND KernelBinToSource -i ${patch_hex_dir}/${krnpatch}.bin -o ${krn_dir}/cmfcpatch
        COMMENT "Generating FC patch file...\
            KernelBinToSource -i ${patch_hex_dir}/${krnpatch}.bin -o ${krn_dir}/cmfcpatch"    
    )

endfunction()


if(BUILD_KERNELS)
    gen_vpkernel_from_cm()
endif()

media_include_subdirectory(cmfc)
media_include_subdirectory(cmfcpatch)

source_group( "Kernel\\VpKernel" FILES ${TMP_SOURCES_} ${TMP_HEADERS_} )

media_add_curr_to_include_path()
