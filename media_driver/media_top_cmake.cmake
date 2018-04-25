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

bs_set_if_undefined(LIB_NAME iHD_drv_video)

option (MEDIA_RUN_TEST_SUITE "run google test module after install" ON) 
include(${MEDIA_DRIVER_CMAKE}/media_gen_flags.cmake)
include(${MEDIA_DRIVER_CMAKE}/media_feature_flags.cmake)

message("-- media -- PLATFORM = ${PLATFORM}")
message("-- media -- ARCH = ${ARCH}")
message("-- media -- CMAKE_CURRENT_LIST_DIR = ${CMAKE_CURRENT_LIST_DIR}")
message("-- media -- INCLUDED_LIBS = ${INCLUDED_LIBS}")
message("-- media -- LIB_NAME = ${LIB_NAME}")
message("-- media -- OUTPUT_NAME = ${OUTPUT_NAME}")
message("-- media -- BUILD_TYPE/UFO_BUILD_TYPE/CMAKE_BUILD_TYPE = ${BUILD_TYPE}/${UFO_BUILD_TYPE}/${CMAKE_BUILD_TYPE}")
message("-- media -- LIBVA_INSTALL_PATH = ${LIBVA_INSTALL_PATH}")
message("-- media -- BUILD_ALONG_WITH_CMRTLIB = ${BUILD_ALONG_WITH_CMRTLIB}")
Message("-- media -- MEDIA_VERSION = ${MEDIA_VERSION}")

set(SOURCES_ "")

# add source
media_include_subdirectory(agnostic)
media_include_subdirectory(linux)
include(${CMAKE_CURRENT_LIST_DIR}/media_srcs_ext.cmake OPTIONAL)

include(${MEDIA_DRIVER_CMAKE}/media_include_paths.cmake)

include(${MEDIA_DRIVER_CMAKE}/media_compile_flags.cmake)

#
# set platform specific defines
#
bs_set_defines()

set_source_files_properties(${SOURCES_} PROPERTIES LANGUAGE "CXX")

add_library( ${LIB_NAME} SHARED ${SOURCES_})

if(MEDIA_RUN_TEST_SUITE)
	add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/linux/ult)
endif(MEDIA_RUN_TEST_SUITE)

option(MEDIA_BUILD_FATAL_WARNINGS "Turn compiler warnings into fatal errors" ON)
if(MEDIA_BUILD_FATAL_WARNINGS)
    set_target_properties(${LIB_NAME} PROPERTIES COMPILE_FLAGS "-Werror")
endif()

set_target_properties(${LIB_NAME} PROPERTIES LINK_FLAGS "-Wl,--no-as-needed -Wl,--gc-sections -z relro -z now -fstack-protector -fPIC")
set_target_properties(${LIB_NAME} PROPERTIES PREFIX "")

MediaAddCommonTargetDefines(${LIB_NAME})

bs_ufo_link_libraries_noBsymbolic(
    ${LIB_NAME}
    "${INCLUDED_LIBS}"
    "pciaccess m pthread dl rt"
)

if (NOT DEFINED INCLUDED_LIBS OR "${INCLUDED_LIBS}" STREQUAL "")
    # dep libs (gmmlib for now) can be passed through INCLUDED_LIBS, but if not, we need try to setup dep through including dep projects
    if (NOT TARGET gmm_umd)
        add_subdirectory("${BS_DIR_GMMLIB}" "${CMAKE_BINARY_DIR}/gmmlib")
    endif()

    if (TARGET gmm_umd)
        target_link_libraries ( ${LIB_NAME}
            gmm_umd
        )
    endif()

include(${MEDIA_DRIVER_CMAKE}/ext/media_feature_include_ext.cmake OPTIONAL)

endif(NOT DEFINED INCLUDED_LIBS OR "${INCLUDED_LIBS}" STREQUAL "")

# post target attributes
bs_set_post_target()
