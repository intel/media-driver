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

project( media )

find_package(PkgConfig)
find_package(X11)

bs_set_if_undefined(LIB_NAME iHD_drv_video)

option (MEDIA_RUN_TEST_SUITE "run google test module after install" ON)
include(${MEDIA_SOFTLET_CMAKE}/media_gen_flags.cmake)
include(${MEDIA_SOFTLET_CMAKE}/media_feature_flags.cmake)

check_include_file_cxx("execinfo.h" HAVE_EXECINFO_H)
if(HAVE_EXECINFO_H)
    target_compile_definitions(${LIB_NAME} PRIVATE HAVE_EXECINFO_H)
endif()

check_library_exists(execinfo backtrace "" HAVE_LIBEXECINFO)
if(HAVE_LIBEXECINFO)
    target_link_libraries (${LIB_NAME} PRIVATE execinfo)
endif()

if(NOT DEFINED SKIP_GMM_CHECK)
    # checking dependencies
    pkg_check_modules(LIBGMM REQUIRED igdgmm>=12.0.0)

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
    pkg_check_modules(LIBVAX11 libva-x11)
    if(LIBVAX11_FOUND)
        message("-- media -- LIBVAX11 Found")
    else()
        set(X11_FOUND FALSE)
    endif()
endif()

set(LIB_NAME_STATIC "${LIB_NAME}_softlet_STATIC")
set(SOFTLET_COMMON_SOURCES_ "")
set(SOFTLET_COMMON_PRIVATE_INCLUDE_DIRS_ "")
set(SOFTLET_VP_SOURCES_ "")       # softlet source group
set(SOFTLET_VP_PRIVATE_INCLUDE_DIRS_ "")
set(CP_COMMON_SHARED_SOURCES_ "") # legacy and softlet shared source group
set(CP_COMMON_NEXT_SOURCES_ "")   # softlet source group
set(SOFTLET_DDI_PUBLIC_INCLUDE_DIRS_ "")
set(SOFTLET_CODEC_SOURCES_ "")
set(SOFTLET_CODEC_PRIVATE_INCLUDE_DIRS_ "")
set(CP_INTERFACE_DIRECTORIES_ "")

######################################################
#MOS LIB
set (SOFTLET_MOS_COMMON_SOURCES_ "")
set (SOFTLET_MOS_COMMON_HEADERS_ "")
set (SOFTLET_MOS_PRIVATE_SOURCES_ "")
set (SOFTLET_MOS_PUBLIC_INCLUDE_DIRS_ "")
set (SOFTLET_MOS_PRIVATE_INCLUDE_DIRS_ "")
set (SOFTLET_MOS_PREPEND_INCLUDE_DIRS_ "")
set (SOFTLET_MOS_EXT_INCLUDE_DIRS_ "")
######################################################
################################################################################
# MHW
################################################################################
set(SOFTLET_MHW_VDBOX_PRIVATE_INCLUDE_DIRS_ "")
set(SOFTLET_MHW_PRIVATE_INCLUDE_DIRS_ "")

set(SOFTLET_MHW_VDBOX_COMMON_SOURCES_ "")
set(SOFTLET_MHW_VDBOX_SOURCES_ "")
set(SOFTLET_MHW_SOURCES_ "")
set(SOFTLET_MHW_VDBOX_HCP_SOURCES_ "")
set(SOFTLET_MHW_VDBOX_HUC_SOURCES_ "")
set(SOFTLET_MHW_VDBOX_VDENC_SOURCES_ "")
set(SOFTLET_MHW_VDBOX_AVP_SOURCES_ "")
set(SOFTLET_MHW_VDBOX_MFX_SOURCES_ "")
set(SOFTLET_MHW_VDBOX_AQM_SOURCES_ "")
set(SOFTLET_MHW_EXT_SOURCES_ "")

# MHW settings
set(SOFTLET_MHW_PRIVATE_INCLUDE_DIRS_
${SOFTLET_MHW_PRIVATE_INCLUDE_DIRS_}
${SOFTLET_MHW_VDBOX_PRIVATE_INCLUDE_DIRS_}
${SOFTLET_MHW_COMMON_PRIVATE_INCLUDE_DIRS_}
${SOFTLET_MHW_BLT_PRIVATE_INCLUDE_DIRS_}
${SOFTLET_MHW_RENDER_PRIVATE_INCLUDE_DIRS_}
${SOFTLET_MHW_SFC_PRIVATE_INCLUDE_DIRS_}
${SOFTLET_MHW_VEBOX_PRIVATE_INCLUDE_DIRS_}
${SOFTLET_MHW_MI_PRIVATE_INCLUDE_DIRS_}
)

set(SOFTLET_MHW_VDBOX_SOURCES_
${SOFTLET_MHW_VDBOX_SOURCES_}
${SOFTLET_MHW_VDBOX_COMMON_SOURCES_}
${SOFTLET_MHW_VDBOX_HCP_SOURCES_}
${SOFTLET_MHW_VDBOX_HUC_SOURCES_}
${SOFTLET_MHW_VDBOX_VDENC_SOURCES_}
${SOFTLET_MHW_VDBOX_AVP_SOURCES_}
${SOFTLET_MHW_VDBOX_MFX_SOURCES_}
${SOFTLET_MHW_VDBOX_AQM_SOURCES_}
${SOFTLET_MHW_EXT_SOURCES_}
)

set(SOFTLET_MHW_SOURCES_
${SOFTLET_MHW_SOURCES_}
${SOFTLET_MHW_VDBOX_SOURCES_}
${SOFTLET_MHW_COMMON_SOURCES_}
${SOFTLET_MHW_BLT_SOURCES_}
${SOFTLET_MHW_RENDER_SOURCES_}
${SOFTLET_MHW_SFC_SOURCES_}
${SOFTLET_MHW_VEBOX_SOURCES_}
${SOFTLET_MHW_MI_SOURCES_}
)

# add source
media_include_subdirectory(../media_common/agnostic)
media_include_subdirectory(../media_common/linux)
media_include_subdirectory(media_interface)

media_include_subdirectory(agnostic)
media_include_subdirectory(linux)

include(${MEDIA_SOFTLET_EXT}/media_srcs_ext.cmake OPTIONAL)
include(${MEDIA_COMMON_EXT}/media_srcs_ext.cmake OPTIONAL)

include(${MEDIA_SOFTLET_CMAKE}/media_include_paths.cmake)

include(${MEDIA_SOFTLET_CMAKE}/media_compile_flags.cmake)

#
# set platform specific defines
#
bs_set_defines()


set_source_files_properties(${SOURCES_} PROPERTIES LANGUAGE "CXX")
set_source_files_properties(${SOFTLET_COMMON_SOURCES_} PROPERTIES LANGUAGE "CXX")
set_source_files_properties(${SOFTLET_MHW_SOURCES_} PROPERTIES LANGUAGE "CXX")
set_source_files_properties(${SOFTLET_VP_SOURCES_} PROPERTIES LANGUAGE "CXX")
set_source_files_properties(${CP_COMMON_SHARED_SOURCES_} PROPERTIES LANGUAGE "CXX")
set_source_files_properties(${CP_COMMON_NEXT_SOURCES_} PROPERTIES LANGUAGE "CXX")
set_source_files_properties(${SOURCES_SSE2} PROPERTIES LANGUAGE "CXX")
set_source_files_properties(${SOURCES_SSE4} PROPERTIES LANGUAGE "CXX")

add_library(${LIB_NAME}_SOFTLET_COMMON OBJECT ${SOFTLET_COMMON_SOURCES_} ${SOFTLET_MHW_SOURCES_})
set_property(TARGET ${LIB_NAME}_SOFTLET_COMMON PROPERTY POSITION_INDEPENDENT_CODE 1)
MediaAddCommonTargetDefines(${LIB_NAME}_SOFTLET_COMMON)
target_include_directories(${LIB_NAME}_SOFTLET_COMMON BEFORE PRIVATE
    ${SOFTLET_MOS_PREPEND_INCLUDE_DIRS_}
    ${SOFTLET_MOS_PUBLIC_INCLUDE_DIRS_}
    ${SOFTLET_COMMON_PRIVATE_INCLUDE_DIRS_}
    ${SOFTLET_MHW_PRIVATE_INCLUDE_DIRS_}
    ${SOFTLET_CODEC_PRIVATE_INCLUDE_DIRS_}
    ${SOFTLET_VP_PRIVATE_INCLUDE_DIRS_}
    ${CP_INTERFACE_DIRECTORIES_}
    ${SOFTLET_DDI_PUBLIC_INCLUDE_DIRS_}
)

add_library(${LIB_NAME}_SOFTLET_VP OBJECT ${SOFTLET_VP_SOURCES_})
set_property(TARGET ${LIB_NAME}_SOFTLET_VP PROPERTY POSITION_INDEPENDENT_CODE 1)
MediaAddCommonTargetDefines(${LIB_NAME}_SOFTLET_VP)
target_include_directories(${LIB_NAME}_SOFTLET_VP BEFORE PRIVATE
    ${SOFTLET_MOS_PREPEND_INCLUDE_DIRS_}
    ${SOFTLET_MOS_PUBLIC_INCLUDE_DIRS_}
    ${SOFTLET_COMMON_PRIVATE_INCLUDE_DIRS_}
    ${SOFTLET_MHW_PRIVATE_INCLUDE_DIRS_}
    ${SOFTLET_VP_PRIVATE_INCLUDE_DIRS_}
    ${SOFTLET_DDI_PUBLIC_INCLUDE_DIRS_}
    ${SOFTLET_CODEC_PRIVATE_INCLUDE_DIRS_}
    ${CP_INTERFACE_DIRECTORIES_}
)

add_library(${LIB_NAME}_SOFTLET_CODEC OBJECT ${SOFTLET_CODEC_SOURCES_})
set_property(TARGET ${LIB_NAME}_SOFTLET_CODEC PROPERTY POSITION_INDEPENDENT_CODE 1)
MediaAddCommonTargetDefines(${LIB_NAME}_SOFTLET_CODEC)
target_include_directories(${LIB_NAME}_SOFTLET_CODEC BEFORE PRIVATE
    ${SOFTLET_MOS_PREPEND_INCLUDE_DIRS_}
    ${SOFTLET_MOS_PUBLIC_INCLUDE_DIRS_}
    ${SOFTLET_COMMON_PRIVATE_INCLUDE_DIRS_}
    ${SOFTLET_MHW_PRIVATE_INCLUDE_DIRS_}
    ${SOFTLET_VP_PRIVATE_INCLUDE_DIRS_}
    ${SOFTLET_DDI_PUBLIC_INCLUDE_DIRS_}
    ${SOFTLET_CODEC_PRIVATE_INCLUDE_DIRS_}
    ${CP_INTERFACE_DIRECTORIES_}
)
############## MOS LIB ########################################
set_source_files_properties(${MOS_COMMON_SOURCES_} PROPERTIES LANGUAGE "CXX")
set_source_files_properties(${SOFTLET_MOS_COMMON_SOURCES_} PROPERTIES LANGUAGE "CXX")

# This is to include drm_device.h in cmrtlib, no cpp file needed.
set (SOFTLET_MOS_EXT_INCLUDE_DIRS_
${SOFTLET_MOS_EXT_INCLUDE_DIRS_}
${BS_DIR_MEDIA}/cmrtlib/linux/hardware
)


#1 softlet mos lib
add_library(${LIB_NAME}_mos_softlet OBJECT ${SOFTLET_MOS_COMMON_SOURCES_} ${SOFTLET_MOS_PRIVATE_SOURCES_})
set_property(TARGET ${LIB_NAME}_mos_softlet PROPERTY POSITION_INDEPENDENT_CODE 1)
MediaAddCommonTargetDefines(${LIB_NAME}_mos_softlet)
target_include_directories(${LIB_NAME}_mos_softlet BEFORE PRIVATE
    ${SOFTLET_MOS_PREPEND_INCLUDE_DIRS_}
    ${SOFTLET_MOS_EXT_INCLUDE_DIRS_}
    ${SOFTLET_MOS_PUBLIC_INCLUDE_DIRS_}
)
############## MOS LIB END ########################################

############## Media Driver Static and Shared Lib ##################
# The shared library cannot succeed until all refactor done. Comment it out.
#add_library(${LIB_NAME}_softlet SHARED
#    $<TARGET_OBJECTS:${LIB_NAME}_mos_softlet>
#    $<TARGET_OBJECTS:${LIB_NAME}_SOFTLET_VP>
#    $<TARGET_OBJECTS:${LIB_NAME}_SOFTLET_COMMON>)

add_library(${LIB_NAME_STATIC} STATIC
    $<TARGET_OBJECTS:${LIB_NAME}_mos_softlet>
    $<TARGET_OBJECTS:${LIB_NAME}_SOFTLET_VP>
    $<TARGET_OBJECTS:${LIB_NAME}_SOFTLET_CODEC>
    $<TARGET_OBJECTS:${LIB_NAME}_SOFTLET_COMMON>)

set_target_properties(${LIB_NAME_STATIC} PROPERTIES OUTPUT_NAME ${LIB_NAME_STATIC})

option(MEDIA_BUILD_FATAL_WARNINGS "Turn compiler warnings into fatal errors" ON)
if(MEDIA_BUILD_FATAL_WARNINGS)
    set_target_properties(${LIB_NAME}_mos_softlet PROPERTIES COMPILE_FLAGS "-Werror")
    set_target_properties(${LIB_NAME}_SOFTLET_VP PROPERTIES COMPILE_FLAGS "-Werror")
    set_target_properties(${LIB_NAME}_SOFTLET_CODEC PROPERTIES COMPILE_FLAGS "-Werror")
    set_target_properties(${LIB_NAME}_SOFTLET_COMMON PROPERTIES COMPILE_FLAGS "-Werror")
endif()

set(MEDIA_LINK_FLAGS "-Wl,--no-as-needed -Wl,--gc-sections -z relro -z now -fPIC")
option(MEDIA_BUILD_HARDENING "Enable hardening (stack-protector, fortify source)" ON)
if(MEDIA_BUILD_HARDENING)
    set(MEDIA_LINK_FLAGS "${MEDIA_LINK_FLAGS} -fstack-protector")
endif()
set_target_properties(${LIB_NAME}_softlet PROPERTIES LINK_FLAGS ${MEDIA_LINK_FLAGS})

set_target_properties(${LIB_NAME}_softlet        PROPERTIES PREFIX "")
set_target_properties(${LIB_NAME_STATIC} PROPERTIES PREFIX "")

bs_ufo_link_libraries_noBsymbolic(
    ${LIB_NAME}_softlet
    "${INCLUDED_LIBS}"
    "${PKG_PCIACCESS_LIBRARIES} m pthread dl"
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

    target_compile_options( ${LIB_NAME}_softlet PUBLIC ${LIBGMM_CFLAGS_OTHER})
    target_link_libraries ( ${LIB_NAME}_softlet ${LIBGMM_LIBRARIES})

    target_compile_definitions(${LIB_NAME}_softlet PUBLIC GMM_LIB_DLL)
    include(${MEDIA_SOFTLET_EXT_CMAKE}/media_feature_include_ext.cmake OPTIONAL)

endif(NOT DEFINED INCLUDED_LIBS OR "${INCLUDED_LIBS}" STREQUAL "")

############## Media Driver Static and Shared Lib ##################

# post target attributes
bs_set_post_target()

if(MEDIA_RUN_TEST_SUITE AND ENABLE_KERNELS AND ENABLE_NONFREE_KERNELS)

endif()
