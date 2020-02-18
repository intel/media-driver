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

set(TMP_1_SOURCES_
    ${CMAKE_CURRENT_LIST_DIR}/vphal_g9.cpp
)

set(TMP_1_HEADERS_
    ${CMAKE_CURRENT_LIST_DIR}/vphal_g9.h
)

set(TMP_2_SOURCES_
    ${CMAKE_CURRENT_LIST_DIR}/vphal_render_composite_g9.cpp
    ${CMAKE_CURRENT_LIST_DIR}/vphal_render_sfc_g9_base.cpp
    ${CMAKE_CURRENT_LIST_DIR}/vphal_render_vebox_g9_base.cpp
    ${CMAKE_CURRENT_LIST_DIR}/vphal_renderer_g9.cpp
    ${CMAKE_CURRENT_LIST_DIR}/vphal_render_hdr_g9_base.cpp
)

set(TMP_2_HEADERS_
    ${CMAKE_CURRENT_LIST_DIR}/vphal_render_composite_g9.h
    ${CMAKE_CURRENT_LIST_DIR}/vphal_render_sfc_g9_base.h
    ${CMAKE_CURRENT_LIST_DIR}/vphal_render_vebox_g9_base.h
    ${CMAKE_CURRENT_LIST_DIR}/vphal_renderer_g9.h
    ${CMAKE_CURRENT_LIST_DIR}/vphal_render_hdr_g9_base.h
)

set(SOURCES_
    ${SOURCES_}
    ${TMP_1_SOURCES_}
    ${TMP_2_SOURCES_}
 )

set(HEADERS_
    ${HEADERS_}
    ${TMP_1_HEADERS_}
    ${TMP_2_HEADERS_}
)

source_group( "VpHal\\Common" FILES ${TMP_1_SOURCES_} ${TMP_1_HEADERS_} )
source_group( "VpHal\\Render" FILES ${TMP_2_SOURCES_} ${TMP_2_HEADERS_} )


media_add_curr_to_include_path()
