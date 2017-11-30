# *****************************************************************************
# Copyright (c) 2017 Intel Corporation
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
# *****************************************************************************

# utility functions for cmake

if(NOT DEFINED _bs_include_utils)
set(_bs_include_utils TRUE)

include(${BUILD_SYS_INC}/bs_base_utils.cmake)


# macro to setup output name to OUTPUT_NAME
# and turn on position independent code. This
# was pulled in to cover the separate hooks
# that each component was using to a standard macro
# for Linux builds only.  Can be extended to
# cover other OS targets without the need to update
# individual component lists.
macro(bs_set_post_target)
    if (${PLATFORM} STREQUAL linux)
        set_property(TARGET ${LIB_NAME} PROPERTY OUTPUT_NAME ${OUTPUT_NAME})
        set_property(TARGET ${LIB_NAME} PROPERTY POSITION_INDEPENDENT_CODE ON)
    endif(${PLATFORM} STREQUAL linux)
endmacro()

# macro to setup standard defines This
# was pulled in to cover the separate v2c hooks
# that each component was using to a standard macro
# Can be extended to cover more OS targets without the need to update
# individual component lists.
macro(bs_set_defines)
    if (${PLATFORM} STREQUAL "ghs")
        # LANGUAGE "CXX" alone is not enough for GHS compiler
        # as both CC and CXX execs point to the same binary
        add_definitions(-dotciscxx)
    endif (${PLATFORM} STREQUAL "ghs")

    if(NOT ${PLATFORM} STREQUAL "qnx")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
    endif()

    if (${PLATFORM} STREQUAL linux)
        add_definitions(-D__STDC_LIMIT_MACROS)
        add_definitions(-D__STDC_CONSTANT_MACROS)
    endif (${PLATFORM} STREQUAL linux)
endmacro()

# macro to properly setup the target_link_libraries
# for both INTERNAL_LIBS and EXTERNAL_LIBS.
macro(bs_ufo_link_libraries TARGET_NAME INTERNAL_LIBS EXTERNAL_LIBS)

    string(REPLACE " " ";" INTERNAL_LIBS_LIST "${INTERNAL_LIBS}")
    string(REPLACE " " ";" EXTERNAL_LIBS_LIST "${EXTERNAL_LIBS}")

    set(CMAKE_EXE_LINKER_FLAGS_RELEASE )

    target_link_libraries(
        ${TARGET_NAME}
        -Wl,-Bsymbolic
        -Wl,--no-undefined
    )

    foreach(lib ${INTERNAL_LIBS_LIST})
        # work around an undef (DeleteGHAL3DRegInfo) when linking with external projects
        if(NOT lib MATCHES "glsl_fe_link")
            target_link_libraries(${TARGET_NAME} -Wl,--whole-archive ${lib})
        endif()
    endforeach()

    target_link_libraries(
        ${TARGET_NAME}
        -Wl,--no-whole-archive
    )

    foreach(lib ${EXTERNAL_LIBS_LIST})
        target_link_libraries(${TARGET_NAME} ${lib})
    endforeach()

endmacro()
macro(bs_ufo_link_libraries_noBsymbolic TARGET_NAME INTERNAL_LIBS EXTERNAL_LIBS)

    string(REPLACE " " ";" INTERNAL_LIBS_LIST "${INTERNAL_LIBS}")
    string(REPLACE " " ";" EXTERNAL_LIBS_LIST "${EXTERNAL_LIBS}")

    set(CMAKE_EXE_LINKER_FLAGS_RELEASE )

    target_link_libraries(
        ${TARGET_NAME}
        -Wl,--no-undefined
    )

    foreach(lib ${INTERNAL_LIBS_LIST})
        # work around an undef (DeleteGHAL3DRegInfo) when linking with external projects
        if(NOT lib MATCHES "glsl_fe_link")
            target_link_libraries(${TARGET_NAME} -Wl,--whole-archive ${lib})
        endif()
    endforeach()

    target_link_libraries(
        ${TARGET_NAME}
        -Wl,--no-whole-archive
    )

    foreach(lib ${EXTERNAL_LIBS_LIST})
        target_link_libraries(${TARGET_NAME} ${lib})
    endforeach()

endmacro()
# macro to setup forced exceptions for Linux
# builds only.  Should be extended for other
# targets that require forced exceptions.
macro(bs_set_force_exceptions)
    if (${PLATFORM} STREQUAL "linux")
        string(REPLACE "no-exceptions" "exceptions" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
        string(REPLACE "no-exceptions" "exceptions" CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}")
        string(REPLACE "no-exceptions" "exceptions" CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE}")
        string(REPLACE "no-exceptions" "exceptions" CMAKE_C_FLAGS_RELEASEINTERNAL "${CMAKE_C_FLAGS_RELEASEINTERNAL}")

        string(REPLACE "no-exceptions" "exceptions" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
        string(REPLACE "no-exceptions" "exceptions" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
        string(REPLACE "no-exceptions" "exceptions" CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
        string(REPLACE "no-exceptions" "exceptions" CMAKE_CXX_FLAGS_RELEASEINTERNAL "${CMAKE_CXX_FLAGS_RELEASEINTERNAL}")

        string(REPLACE "-DNO_EXCEPTION_HANDLING" "" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
        string(REPLACE "-DNO_EXCEPTION_HANDLING" "" CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}")
        string(REPLACE "-DNO_EXCEPTION_HANDLING" "" CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE}")
        string(REPLACE "-DNO_EXCEPTION_HANDLING" "" CMAKE_C_FLAGS_RELEASEINERNAL "${CMAKE_C_FLAGS_RELEASEINERNAL}")

        string(REPLACE "-DNO_EXCEPTION_HANDLING" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
        string(REPLACE "-DNO_EXCEPTION_HANDLING" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
        string(REPLACE "-DNO_EXCEPTION_HANDLING" "" CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
        string(REPLACE "-DNO_EXCEPTION_HANDLING" "" CMAKE_CXX_FLAGS_RELEASEINTERNAL "${CMAKE_CXX_FLAGS_RELEASEINTERNAL}")
    endif()
endmacro()

# function to force an error if a variable if it is not
# defined
function(bs_fatal_if_undefined)
    string(REPLACE " " ";" _var_names "${ARGV}")
    set(_undef_seen FALSE)
    foreach(_varnam ${_var_names})
        if(NOT DEFINED ${_varnam})
            message(SEND_ERROR "Required variable not defined: ${_varnam}")
            set(_undef_seen TRUE)
        endif()
    endforeach()
    if(_undef_seen)
        message(FATAL_ERROR "Stopping due to undefined variables")
    endif(_undef_seen)
endfunction(bs_fatal_if_undefined)

# macro to return the llvm directory path
# for host and target builds.
macro(bs_get_llvm_dir)
    set(LLVM_DIR ${DUMP_DIR}/igc/llvm/${LLVM_INT_DIR})
    set(CODEGEN_DIR ${DUMP_DIR}/codegen)
endmacro()

# macro to add common includes used by multiple components.
macro(bs_add_some_common_includes)
    bs_fatal_if_undefined(PLATFORM)
    if (${PLATFORM} STREQUAL linux)
        bs_get_llvm_dir()
        bs_fatal_if_undefined(CODEGEN_DIR LLVM_DIR GFX_DEVELOPMENT_DIR)
        include_directories(${CODEGEN_DIR})
        include_directories(${LLVM_DIR}/include)
        include_directories(${GFX_DEVELOPMENT_DIR}/Source/OpenGL/source/os/linux/oskl)

        if(NOT "${LIBDRM_SRC}" STREQUAL "")
            message("using LIBDRM_SRC=${LIBDRM_SRC}")
            include_directories(${LIBDRM_SRC})
            include_directories(${LIBDRM_SRC}/include/drm)
            include_directories(${LIBDRM_SRC}/intel)
        else()
            include_directories(${BS_DIR_OPENGL}/source/os/linux/oskl/drm_intel)
            include_directories(${BS_DIR_INSTRUMENTATION}/driver/linux/drm_intel)
        endif()
        set(DRM_LIB_PATH drm)
        set(DRM_INTEL_LIB_PATH drm_intel)
    endif()
endmacro()

# macro to allow setting a list of extra compile
# definitions.
macro(bs_set_extra_target_properties targ propValues)
    string(REPLACE " " ";" PROP_VALUES "${ARGV}")
    if(TARGET "${targ}")
        foreach(prop ${PROP_VALUES})
            if (${prop} STREQUAL ${targ})
                continue()
            endif()
            set_property(TARGET "${targ}" APPEND PROPERTY COMPILE_DEFINITIONS
                ${prop}
            )
        endforeach()
    endif(TARGET "${targ}")
endmacro()

endif(NOT DEFINED _bs_include_utils)
