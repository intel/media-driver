# Copyright (c) 2017, Intel Corporation
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
    # Here we define build steps to generate c-array binary shaders (kernels)
    # and their header files. If you don't use BUILD_KERNELS option these
    # kernels will just be used from pre-built form. If you will regenerate
    # kernels you may notice the difference from the per-built kernels in git-diff.

    # iga64 -i <src> -o <dst> -p <genx> -a
    function(iga_compile src dst genx)
        get_filename_component(tgt ${src} NAME)
        add_custom_command(
          OUTPUT ${dst}
          DEPENDS KrnToHex_IGA ${src}
          COMMAND ${IGA} -p ${genx} -a ${src} -o ${tgt}.krn
          COMMAND KrnToHex_IGA ${tgt}.krn
          COMMAND ${CMAKE_COMMAND} -E remove ${tgt}.krn
          COMMAND ${CMAKE_COMMAND} -E rename ${tgt}.hex ${dst})
    endfunction()

    function(platform_to_genx platform genx kind)
        if(platform STREQUAL "gen11_icllp")
            set(genx "11" PARENT_SCOPE)
            set(kind "icllp" PARENT_SCOPE)
        endif()
    endfunction()

    # Function generate kernel for the specified platform. It assumes
    # that kernel sources are located in a certain location (see ${krn_dir})
    # calculated in a function body. And it assumes that output files should
    # be stored in a certain locations (see ${krn_dir}/${krn}.h/c and ${krn_header}).
    # Notes:
    #   - <platform> = gen<X>_<kind>, for example, gen11_icllp
    #   - As of now this function works for the single platform only (gen11_icllp) while it
    #  actully should process all the platforms at once. This single processing
    #  is needed to generate shared ${krn_header} with the unified kernels definition.
    function(gen_kernel_from_asm name platform)
        platform_to_genx(${platform} genx kind)

        set(krn_dir ${CMAKE_SOURCE_DIR}/media_driver/agnostic/${platform}/vp/kernel_free)
        set(link_file ${krn_dir}/component_release/LinkFile.txt)
        set(krn ig${name}krn_g${genx}_${kind})
        set(krn_header ${CMAKE_CURRENT_LIST_DIR}/common/vp/kernel/${name}krnheader.h)

        # Compiling all the sources in the kernel source directory
        file(GLOB srcs ${krn_dir}/Source/*.asm)
        set(hexs "")
        foreach(src ${srcs})
            get_filename_component(tgt ${src} NAME_WE)
            iga_compile(${src} ${tgt}.hex ${genx})
            list(APPEND hexs ${tgt}.hex)
        endforeach()

        # Generating common .bin file containing all the kernels and its header
        # file shared across all platforms (${name}krnheader.h)
        add_custom_command(
            OUTPUT ${krn}.bin ${krn_header}
            DEPENDS GenKrnBin ${hexs} ${link_file}
            COMMAND ${CMAKE_COMMAND} -E copy ${link_file} ./
            COMMAND GenKrnBin . ${name}
            COMMAND ${CMAKE_COMMAND} -E copy ${krn}.h ${krn_header})

        # Generating kernel from the .bin file
        add_custom_command(
            OUTPUT ${krn_dir}/${krn}.c ${krn_dir}/${krn}.h
            DEPENDS KernelBinToSource ${krn}.bin
            COMMAND KernelBinToSource -i ${krn}.bin -o ${krn_dir}/)
    endfunction()

    if(GEN11_ICLLP)
        gen_kernel_from_asm(vp gen11_icllp)
    endif()
endif()

media_include_subdirectory(common)

if(GEN8)
    media_include_subdirectory(gen8)
endif()

if(GEN8_BDW)
    media_include_subdirectory(gen8_bdw)
endif()

if(GEN9)
    media_include_subdirectory(gen9)
endif()

if(GEN9_BXT)
    media_include_subdirectory(gen9_bxt)
endif()

if(GEN9_SKL)
    media_include_subdirectory(gen9_skl)
endif()

if(GEN9_CFL)
    media_include_subdirectory(gen9_cfl)
endif()

if(GEN9_GLK)
    media_include_subdirectory(gen9_glk)
endif()

if(GEN9_KBL)
    media_include_subdirectory(gen9_kbl)
endif()

if(GEN10)
    media_include_subdirectory(gen10)
endif()

if(GEN10_CNL)
    media_include_subdirectory(gen10_cnl)
endif()

if(GEN11)
    media_include_subdirectory(gen11)
endif()

if(GEN11_ICLLP)
    media_include_subdirectory(gen11_icllp)
endif()

include(${MEDIA_EXT}/agnostic/media_srcs_ext.cmake OPTIONAL)
