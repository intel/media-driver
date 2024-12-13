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

media_include_subdirectory(hal_ddi_share)

set(TMP_SOURCES_
    ${CMAKE_CURRENT_LIST_DIR}/vp_dumper.cpp
    ${CMAKE_CURRENT_LIST_DIR}/vp_debug.cpp
    ${CMAKE_CURRENT_LIST_DIR}/vp_debug_interface.cpp
    ${CMAKE_CURRENT_LIST_DIR}/vp_debug_config_manager.cpp
    ${CMAKE_CURRENT_LIST_DIR}/vp_utils.cpp
    ${CMAKE_CURRENT_LIST_DIR}/vp_user_feature_control.cpp
    ${CMAKE_CURRENT_LIST_DIR}/vp_visa.cpp
)

set(TMP_HEADERS_
    ${CMAKE_CURRENT_LIST_DIR}/vp_dumper.h
    ${CMAKE_CURRENT_LIST_DIR}/vp_utils.h
    ${CMAKE_CURRENT_LIST_DIR}/vp_debug.h
    ${CMAKE_CURRENT_LIST_DIR}/vp_debug_interface.h
    ${CMAKE_CURRENT_LIST_DIR}/vp_debug_config_manager.h
    ${CMAKE_CURRENT_LIST_DIR}/vp_user_feature_control.h
    ${CMAKE_CURRENT_LIST_DIR}/vp_oca_defs.h
    ${CMAKE_CURRENT_LIST_DIR}/vp_visa.h
)

set(SOFTLET_VP_SOURCES_
    ${SOFTLET_VP_SOURCES_}
    ${TMP_SOURCES_}
)

set(SOFTLET_VP_HEADERS_
    ${SOFTLET_VP_HEADERS_}
    ${TMP_HEADERS_}
)

source_group( VpHalNext\\Shared FILES ${TMP_SOURCES_} ${TMP_HEADERS_})
set(TMP_SOURCES_ "")
set(TMP_HEADERS_ "")
set (SOFTLET_VP_PRIVATE_INCLUDE_DIRS_
    ${SOFTLET_VP_PRIVATE_INCLUDE_DIRS_}
    ${CMAKE_CURRENT_LIST_DIR}
)
media_add_curr_to_include_path()