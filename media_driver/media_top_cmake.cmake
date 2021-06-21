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

project( media )

find_package(PkgConfig)
find_package(X11)

bs_set_if_undefined(LIB_NAME iHD_drv_video)

option (MEDIA_RUN_TEST_SUITE "run google test module after install" ON) 
include(${MEDIA_DRIVER_CMAKE}/media_gen_flags.cmake)
include(${MEDIA_DRIVER_CMAKE}/media_feature_flags.cmake)


if(NOT DEFINED SKIP_GMM_CHECK)
    # checking dependencies
    pkg_check_modules(LIBGMM igdgmm)

    if(LIBGMM_FOUND)
        include_directories(BEFORE ${LIBGMM_INCLUDE_DIRS})
        # link_directories() should appear before add_library and the like
        # otherwise it will not take effect
        link_directories(${LIBGMM_LIBRARY_DIRS})
    endif()
endif(NOT DEFINED SKIP_GMM_CHECK)

message("-- media -- PLATFORM = ${PLATFORM}")
message("-- media -- ARCH = ${ARCH}")
message("-- media -- CMAKE_CURRENT_LIST_DIR = ${CMAKE_CURRENT_LIST_DIR}")
message("-- media -- INCLUDED_LIBS = ${INCLUDED_LIBS}")
message("-- media -- LIB_NAME = ${LIB_NAME}")
message("-- media -- OUTPUT_NAME = ${OUTPUT_NAME}")
message("-- media -- BUILD_TYPE/UFO_BUILD_TYPE/CMAKE_BUILD_TYPE = ${BUILD_TYPE}/${UFO_BUILD_TYPE}/${CMAKE_BUILD_TYPE}")
message("-- media -- LIBVA_INSTALL_PATH = ${LIBVA_INSTALL_PATH}")
message("-- media -- MEDIA_VERSION = ${MEDIA_VERSION}")
if(X11_FOUND)
message("-- media -- X11 Found")
endif()

set(LIB_NAME_OBJ    "${LIB_NAME}_OBJ")
set(LIB_NAME_STATIC "${LIB_NAME}_STATIC")
set(SOURCES_ "")

# add source
media_include_subdirectory(agnostic)
media_include_subdirectory(linux)
media_include_subdirectory(media_driver_next)
media_include_subdirectory(media_interface)
media_include_subdirectory(../media_common/agnostic)
media_include_subdirectory(../media_common/linux)
media_include_subdirectory(../media_softlet/agnostic)
media_include_subdirectory(../media_softlet/linux)

include(${MEDIA_EXT}/media_srcs_ext.cmake OPTIONAL)
include(${MEDIA_SOFTLET_EXT}/media_srcs_ext.cmake OPTIONAL)

include(${MEDIA_DRIVER_CMAKE}/media_include_paths.cmake)

include(${MEDIA_DRIVER_CMAKE}/media_compile_flags.cmake)

#
# set platform specific defines
#
bs_set_defines()

set_source_files_properties(${SOURCES_} PROPERTIES LANGUAGE "CXX")
set_source_files_properties(${SOURCES_SSE2} PROPERTIES LANGUAGE "CXX")
set_source_files_properties(${SOURCES_SSE4} PROPERTIES LANGUAGE "CXX")

add_library(${LIB_NAME}_SSE2 OBJECT ${SOURCES_SSE2})
target_compile_options(${LIB_NAME}_SSE2 PRIVATE -msse2)

add_library(${LIB_NAME}_SSE4 OBJECT ${SOURCES_SSE4})
target_compile_options(${LIB_NAME}_SSE4 PRIVATE -msse4.1)

add_library(${LIB_NAME_OBJ} OBJECT ${SOURCES_})
set_property(TARGET ${LIB_NAME_OBJ} PROPERTY POSITION_INDEPENDENT_CODE 1)

add_library(${LIB_NAME} SHARED
    $<TARGET_OBJECTS:${LIB_NAME_OBJ}>
    $<TARGET_OBJECTS:${LIB_NAME}_SSE2>
    $<TARGET_OBJECTS:${LIB_NAME}_SSE4>)

add_library(${LIB_NAME_STATIC} STATIC 
    $<TARGET_OBJECTS:${LIB_NAME_OBJ}>
    $<TARGET_OBJECTS:${LIB_NAME}_SSE2>
    $<TARGET_OBJECTS:${LIB_NAME}_SSE4>)

set_target_properties(${LIB_NAME_STATIC} PROPERTIES OUTPUT_NAME ${LIB_NAME})

option(MEDIA_BUILD_FATAL_WARNINGS "Turn compiler warnings into fatal errors" ON)
if(MEDIA_BUILD_FATAL_WARNINGS)
    set_target_properties(${LIB_NAME_OBJ} PROPERTIES COMPILE_FLAGS "-Werror")
endif()

set_target_properties(${LIB_NAME} PROPERTIES LINK_FLAGS "-Wl,--no-as-needed -Wl,--gc-sections -z relro -z now -fstack-protector -fPIC")
set_target_properties(${LIB_NAME}        PROPERTIES PREFIX "")
set_target_properties(${LIB_NAME_STATIC} PROPERTIES PREFIX "")

MediaAddCommonTargetDefines(${LIB_NAME_OBJ})

bs_ufo_link_libraries_noBsymbolic(
    ${LIB_NAME}
    "${INCLUDED_LIBS}"
    "${PKG_PCIACCESS_LIBRARIES} m pthread ${CMAKE_DL_LIBS}"
)

if (NOT DEFINED INCLUDED_LIBS OR "${INCLUDED_LIBS}" STREQUAL "")
    # dep libs (gmmlib for now) can be passed through INCLUDED_LIBS, but if not, we need try to setup dep through including dep projects
    if(NOT LIBGMM_FOUND)
        # If we failed to setup dependency from gmmlib via pkg-config we will try to
        # add gmmlib as a target from sources. We need to do this here, after
        # add_library() for iHD driver since gmmlib needs this information.
        if (NOT TARGET igfx_gmmumd_dll)
            add_subdirectory("${BS_DIR_GMMLIB}" "${CMAKE_BINARY_DIR}/gmmlib")
        endif()
        if (NOT TARGET igfx_gmmumd_dll)
            message(FATAL_ERROR "gmm library not found on the system")
        endif()
        set(LIBGMM_CFLAGS_OTHER -DGMM_LIB_DLL)
        set(LIBGMM_LIBRARIES igfx_gmmumd_dll)
    endif()

    target_compile_options( ${LIB_NAME} PUBLIC ${LIBGMM_CFLAGS_OTHER})
    target_link_libraries ( ${LIB_NAME} ${LIBGMM_LIBRARIES})

    include(${MEDIA_EXT_CMAKE}/ext/media_feature_include_ext.cmake OPTIONAL)

endif(NOT DEFINED INCLUDED_LIBS OR "${INCLUDED_LIBS}" STREQUAL "")

# post target attributes
bs_set_post_target()

if(MEDIA_RUN_TEST_SUITE AND ENABLE_KERNELS AND ENABLE_NONFREE_KERNELS)
    add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/linux/ult)
    include(${MEDIA_EXT}/media_driver_next/ult/ult_top_cmake.cmake OPTIONAL)
endif()
