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

set(TMP_SOURCES_
    ${CMAKE_CURRENT_LIST_DIR}/vp_feature_manager.cpp
    ${CMAKE_CURRENT_LIST_DIR}/policy.cpp
    ${CMAKE_CURRENT_LIST_DIR}/vp_obj_factories.cpp
    ${CMAKE_CURRENT_LIST_DIR}/hw_filter_pipe.cpp
    ${CMAKE_CURRENT_LIST_DIR}/hw_filter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/sw_filter_pipe.cpp
    ${CMAKE_CURRENT_LIST_DIR}/sw_filter.cpp
)

set(TMP_HEADERS_
    ${CMAKE_CURRENT_LIST_DIR}/vp_feature_manager.h
    ${CMAKE_CURRENT_LIST_DIR}/policy.h
    ${CMAKE_CURRENT_LIST_DIR}/vp_obj_factories.h
    ${CMAKE_CURRENT_LIST_DIR}/hw_filter_pipe.h
    ${CMAKE_CURRENT_LIST_DIR}/hw_filter.h
    ${CMAKE_CURRENT_LIST_DIR}/sw_filter_pipe.h
    ${CMAKE_CURRENT_LIST_DIR}/sw_filter.h
    ${CMAKE_CURRENT_LIST_DIR}/vp_feature_caps.h
)

set(SOURCES_NEW
    ${SOURCES_NEW}
    ${TMP_SOURCES_}
)

set(HEADERS_NEW
    ${HEADERS_NEW}
    ${TMP_HEADERS_}
)

source_group( VpHalNext\\Shared\\FeatureManager FILES ${TMP_SOURCES_} ${TMP_HEADERS_})

media_add_curr_to_include_path()