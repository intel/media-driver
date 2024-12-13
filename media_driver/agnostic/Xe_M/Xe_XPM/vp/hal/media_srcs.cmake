# Copyright (c) 2020-2022, Intel Corporation

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

set(TMP_1_SOURCES_
    ${CMAKE_CURRENT_LIST_DIR}/vphal_xe_xpm.cpp
    ${CMAKE_CURRENT_LIST_DIR}/vphal_debug_xe_xpm.cpp
)

set(TMP_1_HEADERS_
    ${CMAKE_CURRENT_LIST_DIR}/vphal_xe_xpm.h
    ${CMAKE_CURRENT_LIST_DIR}/vphal_debug_xe_xpm.h
)

set(TMP_2_SOURCES_
    ${CMAKE_CURRENT_LIST_DIR}/vphal_render_sfc_xe_xpm.cpp
    ${CMAKE_CURRENT_LIST_DIR}/vphal_render_vebox_xe_xpm.cpp
    ${CMAKE_CURRENT_LIST_DIR}/vphal_renderer_xe_xpm.cpp
    ${CMAKE_CURRENT_LIST_DIR}/vphal_render_composite_xe_xpm.cpp
    ${CMAKE_CURRENT_LIST_DIR}/vphal_render_vebox_xe_xpm_denoise.cpp
    ${CMAKE_CURRENT_LIST_DIR}/vphal_render_vebox_memdecomp_xe_xpm.cpp
)

set(TMP_2_HEADERS_
    ${CMAKE_CURRENT_LIST_DIR}/vphal_render_sfc_xe_xpm.h
    ${CMAKE_CURRENT_LIST_DIR}/vphal_render_vebox_xe_xpm.h
    ${CMAKE_CURRENT_LIST_DIR}/vphal_renderer_xe_xpm.h
    ${CMAKE_CURRENT_LIST_DIR}/vphal_render_composite_xe_xpm.h
    ${CMAKE_CURRENT_LIST_DIR}/vphal_render_vebox_xe_xpm_denoise.h
    ${CMAKE_CURRENT_LIST_DIR}/vphal_render_vebox_memdecomp_xe_xpm.h
)


set(VP_SOURCES_
    ${VP_SOURCES_}
    ${TMP_1_SOURCES_}
    ${TMP_2_SOURCES_}
)

set(VP_HEADERS_
    ${VP_HEADERS_}
    ${TMP_1_HEADERS_}
    ${TMP_2_HEADERS_}
)

source_group( "VpHal\\Common" FILES ${TMP_1_SOURCES_} ${TMP_1_HEADERS_} )
source_group( "VpHal\\Render" FILES ${TMP_2_SOURCES_} ${TMP_2_HEADERS_} )
set(TMP_1_SOURCES_ "")
set(TMP_1_HEADERS_ "")
set(TMP_2_SOURCES_ "")
set(TMP_2_HEADERS_ "")

set(VP_PRIVATE_INCLUDE_DIRS_
    ${VP_PRIVATE_INCLUDE_DIRS_}
    ${CMAKE_CURRENT_LIST_DIR}
)

media_add_curr_to_include_path()