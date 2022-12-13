# Copyright (c) 2021-2022, Intel Corporation

# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
# OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
# ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
# OTHER DEALINGS IN THE SOFTWARE.

function(gen_vpkernel_from_cm)
    
    set(genx "xe")
    set(kind "hpg")
    set(name "vp")
    set(platform "Xe_HPG_Base")

    set(krn ig${name}krn_${genx}_${kind})
    set(krnpatch ig${name}krn_${genx}_${kind}_cmfcpatch)

    set(krn_dir ${CMAKE_SOURCE_DIR}/media_softlet/agnostic/Xe_R/${platform}/vp/kernel_free)
    set(out_dir ${CMAKE_SOURCE_DIR}/media_softlet/agnostic/Xe_R/${platform}/vp/kernel_free/cache_kernel)


    set(link_file ${krn_dir}/component_release/LinkFile.txt)
    set(kernel_dir ${out_dir}/kernel)
    set(patch_dir ${out_dir}/fcpatch)
    set(kernel_hex_dir ${kernel_dir}/hex)
    set(patch_hex_dir ${patch_dir}/hex)
    set(krn_header ${CMAKE_SOURCE_DIR}/media_common/agnostic/common/vp/kernel/${name}krnheader.h)

    message("krn: " ${krn})
    message("krnpatch: " ${krnpatch})
    message("krn_dir: " ${krn_dir})
    message("out_dir: " ${out_dir})
    message("link_file: " ${link_file})
    message("kernel_dir: " ${kernel_dir})
    message("patch_dir: " ${patch_dir})
    message("kernel_hex_dir: " ${kernel_hex_dir})
    message("patch_hex_dir: " ${patch_hex_dir})
    message("krn_header: " ${krn_header})

    add_custom_command(
        OUTPUT ${out_dir} ${kernel_dir} ${patch_dir} ${kernel_hex_dir} ${patch_hex_dir}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${out_dir}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${kernel_dir}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${patch_dir}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${kernel_hex_dir}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${patch_hex_dir}
        COMMENT "Creating VP cmfcpatch kernels output directory")

    # Compiling all the sources in the kernel source directory.
    file(GLOB_RECURSE srcs ${krn_dir}/Source/*.cpp)

    set(objsname "")
    set(cm_genx ${kind})

    foreach(src ${srcs})
        get_filename_component(obj ${src} NAME_WE) # there are other outputs from cmc command, but we use only .dat and .fcpatch

        string(FIND ${obj} "Dscale" retVal)
        if(NOT retVal STREQUAL "-1")
            string(REPLACE "Dscale" "DScale" obj ${obj})
        endif()
        
        if(obj STREQUAL "EOT" OR obj STREQUAL "Secure_Block_Copy" OR obj STREQUAL "ByteCopy_MediaWalker") # "EOT" and "Secure_Block_Copy" don't generate the related .fcpatch file.
            add_custom_command(
                TARGET 
                OUTPUT ${out_dir}/${obj}_0.dat
                PRE_BUILD
                DEPENDS ${src} ${out_dir}
                WORKING_DIRECTORY ${out_dir}
                COMMAND ${CMC}
                    -c -D BUILD_DSCALE_BUF4_BUF5_ROTATION_KERNEL 
                    -I ${krn_dir}/Source
                    -I ${krn_dir}/Source/Core_Kernels
                    -I ${krn_dir}/Source/Common 
                    -I ${krn_dir}/Source/Components 
                    -Qxcm -march="dg2" -Qxcm_jit_option=-stepping -Qxcm_jit_option=B 
                    -Qxcm_jit_option="-nocompaction -output -binary"
                    -mCM_emit_common_isa 
                    -mCM_no_input_reorder 
                    -mCM_unique_labels=MDF_FC 
                    -mCM_printregusage 
                    -mCM_disable_jmpi
                    -mCM_old_asm_name
                    #-mdump_asm
                    ${src}
                    && python ${krn_dir}/Source/rename.py ${out_dir} ${obj}_0.dat
                COMMENT "Compiling ${src}... to ${out_dir}/${obj}_0.dat\
                    Checking if renaming is needed..."
            )
        else()
            add_custom_command(
                TARGET 
                OUTPUT ${out_dir}/${obj}_0.dat ${out_dir}/${obj}.fcpatch
                PRE_BUILD
                DEPENDS ${src} ${out_dir}
                WORKING_DIRECTORY ${out_dir}
                COMMAND ${CMC}
                    -c -D BUILD_DSCALE_BUF4_BUF5_ROTATION_KERNEL 
                    -I ${krn_dir}/Source
                    -I ${krn_dir}/Source/Core_Kernels
                    -I ${krn_dir}/Source/Common 
                    -I ${krn_dir}/Source/Components 
                    -Qxcm -march="dg2" -Qxcm_jit_option=-stepping -Qxcm_jit_option=B 
                    -Qxcm_jit_option="-nocompaction -output -binary"
                    -mCM_emit_common_isa 
                    -mCM_no_input_reorder 
                    -mCM_unique_labels=MDF_FC 
                    -mCM_printregusage 
                    -mCM_disable_jmpi
                    -mCM_old_asm_name
                    #-mdump_asm
                    ${src}
                    && python ${krn_dir}/Source/rename.py ${out_dir} ${obj}_0.dat ${obj}.fcpatch
                COMMENT "Compiling ${src}... to ${out_dir}/${obj}_0.dat and ${out_dir}/${obj}.fcpatch\
                    Checking if renaming is needed..."
            )
        endif()

        list(APPEND objsname
            ${obj}
        )
    endforeach()

    # Rename Dscale to DScale
    #add_custom_target(
    #   rename ALL
    #    DEPENDS ${krn_dir}/rename.c
    #)
    #add_custom_command(
    #    OUTPUT ${krn_dir}/rename.c
    #    DEPENDS ${objsname}
    #    COMMAND python ${krn_dir}/Source/dscale_rename.py ${out_dir} ${out_dir}
    #    COMMENT "Renaming Ds to DS..."
    #)

    #Generate the .hex files from the .dat files by using KrnToHex.
    set(hexs "")

    foreach(objname ${objsname})
        add_custom_command(
        OUTPUT ${kernel_dir}/${objname}.krn
        DEPENDS ${out_dir}/${objname}_0.dat ${kernel_dir}
        WORKING_DIRECTORY ${out_dir}
        COMMAND ${CMAKE_COMMAND} -E copy ${out_dir}/${objname}_0.dat ${kernel_dir}/${objname}.krn
        COMMENT "Copying ${out_dir}/${objname}_0.dat to ${kernel_dir}/${objname}.krn..."
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
        COMMENT "Generating ${kernel_dir}/${objname}.hex of cmfc kernel from ${kernel_dir}/${objname}.krn files using KrnToHex...\
            Copying ${kernel_dir}/${objname}.hex to ${kernel_hex_dir}/${objname}.hex...\
            Removing ${kernel_dir}/${objname}.hex...")
        set(hexs ${hexs} ${kernel_hex_dir}/${objname}.hex)
    endforeach()

    # Remove "EOT", "Secure_Block_Copy" and.
    list(REMOVE_ITEM objsname 
        "EOT" 
        "Secure_Block_Copy"
        "ByteCopy_MediaWalker"
    )

    ## Generate the .hex files from the .fcpatch files by using KrnToHex.
    foreach(objname ${objsname})
        add_custom_command(
        OUTPUT ${patch_dir}/${objname}.krn
        DEPENDS ${out_dir}/${objname}.fcpatch ${patch_dir}
        WORKING_DIRECTORY ${out_dir}
        COMMAND ${CMAKE_COMMAND} -E copy ${out_dir}/${objname}.fcpatch ${patch_dir}/${objname}.krn
        COMMENT "Copying ${out_dir}/${objname}.fcpatch to ${patch_dir}/${objname}.krn..."
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
        COMMENT "Generating hex files of cmfc kernel from krn files using KrnToHex...\
            Removing old hex files...\
            Copying hex files to hex folders...")
        set(fcpatch_hexs ${fcpatch_hexs} ${patch_hex_dir}/${objname}.hex)
    endforeach()

    # Generating the .bin files for cmfc kernel and patch respectively.

    add_custom_command(
        OUTPUT ${kernel_hex_dir}/${krn}.bin ${krn_header}
        DEPENDS GenDmyHex GenKrnBin ${hexs} ${link_file}   #Generate the dummy hexs from the pre-built header
        WORKING_DIRECTORY ${kernel_hex_dir}
        COMMAND GenDmyHex ${kernel_hex_dir} ${krn_header}
        COMMAND ${CMAKE_COMMAND} -E copy ${link_file} ${kernel_hex_dir}
        COMMAND GenKrnBin ${kernel_hex_dir} ${name} ${genx} ${kind}
        COMMAND ${CMAKE_COMMAND} -E copy ${krn_header} ${krn_header}
        COMMENT "Generating ${krn_header} in ${kernel_hex_dir} using GenDmyHex...\
            Copying ${link_file} to ${kernel_hex_dir}\
            Generating ${name} ${genx} ${kind} in ${kernel_hex_dir}...\
            Copying ${krn_header} to ${krn_header}..."
        )

    add_custom_command(
        OUTPUT ${patch_hex_dir}/${krnpatch}.bin
        DEPENDS GenKrnBin ${fcpatch_hexs} ${link_file}
        WORKING_DIRECTORY ${patch_hex_dir}
        COMMAND ${CMAKE_COMMAND} -E copy ${link_file} ${patch_hex_dir}
        COMMAND GenKrnBin ${patch_hex_dir} ${name} ${genx} ${kind}
        COMMAND ${CMAKE_COMMAND} -E copy ${patch_hex_dir}/${krn}.bin ${patch_hex_dir}/${krnpatch}.bin        ######
        COMMENT "Copying ${link_file} to ${patch_hex_dir}...\
            GenKrnBin ${patch_hex_dir} ${name} ${genx} ${kind}"
    )

    # Generating kernel source files for cmfc kernel and patch.

    message("Kernel file ${krn_dir}/${krn}.h will be generated")
    add_custom_command(
        OUTPUT ${krn_dir}/${krn}.c ${krn_dir}/${krn}.h
        DEPENDS KernelBinToSource ${kernel_hex_dir}/${krn}.bin
        COMMAND KernelBinToSource -i ${kernel_hex_dir}/${krn}.bin -o ${krn_dir}
        COMMENT "Generating kernel file...\
            KernelBinToSource -i ${kernel_hex_dir}/${krn}.bin -o ${krn_dir}"
    )

    message("FC patch file ${krn_dir}/cmfcpatch/${krnpatch}.h will be generated")
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

media_include_subdirectory(cmfcpatch)


set(TMP_SOURCES_
    ${CMAKE_CURRENT_LIST_DIR}/igvpkrn_xe_hpg.c
)


set(TMP_HEADERS_
    ${CMAKE_CURRENT_LIST_DIR}/igvpkrn_xe_hpg.h
)

set(SOFTLET_VP_SOURCES_
    ${SOFTLET_VP_SOURCES_}
    ${TMP_SOURCES_}
)

set(SOFTLET_VP_HEADERS_
    ${SOFTLET_VP_HEADERS_}
    ${TMP_HEADERS_}
)

source_group( "Kernel\\VpKernel" FILES ${TMP_SOURCES_} ${TMP_HEADERS_} )
set(TMP_SOURCES_ "")
set(TMP_HEADERS_ "")

set(SOFTLET_VP_PRIVATE_INCLUDE_DIRS_
    ${SOFTLET_VP_PRIVATE_INCLUDE_DIRS_}
    ${CMAKE_CURRENT_LIST_DIR}
)