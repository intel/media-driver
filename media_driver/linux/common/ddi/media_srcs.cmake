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

set(TMP_SOURCES_
    ${CMAKE_CURRENT_LIST_DIR}/media_ddi_base.cpp
    ${CMAKE_CURRENT_LIST_DIR}/media_libva.cpp
    ${CMAKE_CURRENT_LIST_DIR}/media_libva_caps.cpp
    ${CMAKE_CURRENT_LIST_DIR}/media_libva_common.cpp
    ${CMAKE_CURRENT_LIST_DIR}/media_libva_util.cpp
    ${CMAKE_CURRENT_LIST_DIR}/media_libva_apo_decision.cpp
    ${CMAKE_CURRENT_LIST_DIR}/media_libva_interface.cpp
)

set(TMP_HEADERS_
    ${CMAKE_CURRENT_LIST_DIR}/ddi_media_context.h
    ${CMAKE_CURRENT_LIST_DIR}/va_preopen.h
    ${CMAKE_CURRENT_LIST_DIR}/media_ddi_base.h
    ${CMAKE_CURRENT_LIST_DIR}/media_ddi_factory.h
    ${CMAKE_CURRENT_LIST_DIR}/media_libva.h
    ${CMAKE_CURRENT_LIST_DIR}/media_libva_caps.h
    ${CMAKE_CURRENT_LIST_DIR}/media_libva_caps_factory.h
    ${CMAKE_CURRENT_LIST_DIR}/media_libva_common.h
    ${CMAKE_CURRENT_LIST_DIR}/media_libva_util.h
    ${CMAKE_CURRENT_LIST_DIR}/media_libva_apo_decision.h
    ${CMAKE_CURRENT_LIST_DIR}/media_libva_interface.h
)

if(NOT ${PLATFORM} STREQUAL "android" AND X11_FOUND)
 set(TMP_SOURCES_
    ${TMP_SOURCES_}
    ${CMAKE_CURRENT_LIST_DIR}/media_libva_putsurface_linux.cpp
)

set(TMP_HEADERS_
    ${TMP_HEADERS_}
    ${CMAKE_CURRENT_LIST_DIR}/media_libva_putsurface_linux.h
)
endif()

set(SOURCES_
    ${SOURCES_}
    ${TMP_SOURCES_}
 )

set(HEADERS_
    ${HEADERS_}
    ${TMP_HEADERS_}
)


media_add_curr_to_include_path()
