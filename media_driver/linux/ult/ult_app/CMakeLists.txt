# Copyright (c) 2018, Intel Corporation
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
cmake_minimum_required(VERSION 3.1)

project(devult)

add_subdirectory(googletest)

set(agnostic_cm_tests ../../../agnostic/ult/cm)

set(INTERNAL_INC_PATH
    ../inc
    ./cm
    ./googletest/include
    ./gpu_cmd
    ${agnostic_cm_tests}
    ../../../linux/common/cp/shared
)
include_directories(${INTERNAL_INC_PATH} ${LIBVA_PATH})
if (NOT "${BS_DIR_GMMLIB}" STREQUAL "")
    include_directories(${BS_DIR_GMMLIB}/inc)
endif ()
if (NOT "${BS_DIR_INC}" STREQUAL "")
   include_directories(${BS_DIR_INC} ${BS_DIR_INC}/common)
endif ()

aux_source_directory(. SOURCES)
aux_source_directory(./cm SOURCES)
aux_source_directory(${agnostic_cm_tests} SOURCES)
if (ENABLE_NONFREE_KERNELS)
    aux_source_directory(./gpu_cmd SOURCES)
    set(SOURCES
        ${SOURCES}
        ../../../agnostic/gen9_bxt/hw/vdbox/mhw_vdbox_mfx_hwcmd_g9_bxt.cpp
        ../../../agnostic/gen9_skl/hw/vdbox/mhw_vdbox_mfx_hwcmd_g9_skl.cpp
    )
else ()
    set(SOURCES
        ${SOURCES}
        ${CMAKE_CURRENT_LIST_DIR}/gpu_cmd/gpu_cmd_factory.cpp
    )
endif ()

add_executable(devult ${SOURCES})
target_link_libraries(devult libgtest ${CMAKE_DL_LIBS})
target_include_directories(devult BEFORE PRIVATE
    ${SOFTLET_MOS_PREPEND_INCLUDE_DIRS_}
    ${MOS_PUBLIC_INCLUDE_DIRS_}     ${SOFTLET_MOS_PUBLIC_INCLUDE_DIRS_}
    ${COMMON_PRIVATE_INCLUDE_DIRS_} ${SOFTLET_COMMON_PRIVATE_INCLUDE_DIRS_}
    ${VP_PRIVATE_INCLUDE_DIRS_}     ${SOFTLET_VP_PRIVATE_INCLUDE_DIRS_}
    ${COMMON_CP_DIRECTORIES_}
    ${SOFTLET_DDI_PUBLIC_INCLUDE_DIRS_} ${SOFTLET_MHW_PRIVATE_INCLUDE_DIRS_}
)

if (DEFINED BYPASS_MEDIA_ULT AND "${BYPASS_MEDIA_ULT}" STREQUAL "yes")
    # must explictly pass along BYPASS_MEDIA_ULT as yes then could bypass the running of media ult
    message("-- media -- BYPASS_MEDIA_ULT = ${BYPASS_MEDIA_ULT}")
else ()
    if (ENABLE_NONFREE_KERNELS)
        add_custom_target(RunULT ALL DEPENDS ${LIB_NAME} devult)

        add_custom_command(
            TARGET RunULT
            POST_BUILD
            COMMAND LD_PRELOAD=${MEDIA_MISC_LIB}:../libdrm_mock/libdrm_mock.so ./devult ../../../${LIB_NAME}.so
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            COMMENT "Running devult...")
        endif ()
endif ()
