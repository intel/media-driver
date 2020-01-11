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

set(MEDIA_COMPILER_FLAGS_COMMON
    # Global setting for warnings
    -Wall
    -Winit-self
    -Wpointer-arith
    -Wno-unused
    -Wno-unknown-pragmas
    -Wno-comments
    -Wno-sign-compare
    -Wno-attributes
    -Wno-narrowing
    -Wno-overflow
    -Wno-parentheses
    -Wno-delete-incomplete
    -Werror=address
    -Werror=format-security
    -Werror=non-virtual-dtor
    -Werror=return-type

    # General optimization options
    -finline-functions
    -funswitch-loops
    -fno-short-enums
    -Wa,--noexecstack
    -fno-strict-aliasing

    # Other common flags
    -fmessage-length=0
    -fvisibility=hidden
    -fdata-sections
    -ffunction-sections
    -Wl,--gc-sections

    # Enable c++14 features
    -std=c++14
    # -m32 or -m64
    -m${ARCH}

    # Global defines
    -DLINUX=1
    -DLINUX
    -DNO_RTTI
    -DNO_EXCEPTION_HANDLING
    -DINTEL_NOT_PUBLIC
    -g
)

if(MEDIA_BUILD_HARDENING)
    set(MEDIA_COMPILER_FLAGS_COMMON
        ${MEDIA_COMPILER_FLAGS_COMMON}
        -fstack-protector)
endif()

if(${UFO_MARCH} STREQUAL "slm")
    set(MEDIA_COMPILER_FLAGS_COMMON
        ${MEDIA_COMPILER_FLAGS_COMMON}
        -maes
        # Optimizing driver for Intel
        -mtune=atom
    )
endif()

if(${ARCH} STREQUAL "64")
    set(MEDIA_COMPILER_FLAGS_COMMON
        ${MEDIA_COMPILER_FLAGS_COMMON}
        -D_AMD64_
        -D__CT__
    )
endif()

if(NOT ${PLATFORM} STREQUAL "android")
    set(MEDIA_COMPILER_FLAGS_COMMON
        ${MEDIA_COMPILER_FLAGS_COMMON}
            -fno-tree-pre
        -fPIC
            -Wl,--no-as-needed
        )
    endif()

set(MEDIA_COMPILER_CXX_FLAGS_COMMON
    # for cpp
    -Wreorder
    -Wsign-promo
    -Wnon-virtual-dtor
    -Wno-invalid-offsetof
    -fvisibility-inlines-hidden
    -fno-use-cxa-atexit
    -frtti
    -fexceptions
    -fpermissive
    -fcheck-new
)

if(NOT ${PLATFORM} STREQUAL "android")
    set(MEDIA_COMPILER_CXX_FLAGS_COMMON
        ${MEDIA_COMPILER_CXX_FLAGS_COMMON}
        -std=c++1y
    )
endif()

set(MEDIA_COMPILER_FLAGS_RELEASE "")

if(${UFO_VARIANT} STREQUAL "default")
    set(MEDIA_COMPILER_FLAGS_RELEASE
        ${MEDIA_COMPILER_FLAGS_RELEASE}
        -O2
        -fno-omit-frame-pointer
    )
    if(MEDIA_BUILD_HARDENING)
        set(MEDIA_COMPILER_FLAGS_RELEASE
            ${MEDIA_COMPILER_FLAGS_RELEASE}
            -D_FORTIFY_SOURCE=2)
    endif()
endif()

if(NOT ${PLATFORM} STREQUAL "android")
    if(${UFO_VARIANT} STREQUAL "default")
            set(MEDIA_COMPILER_FLAGS_RELEASE
                ${MEDIA_COMPILER_FLAGS_RELEASE}
                -finline-limit=100
            )
    elseif(${UFO_VARIANT} STREQUAL "nano")
        set(MEDIA_COMPILER_FLAGS_RELEASE
            ${MEDIA_COMPILER_FLAGS_RELEASE}
            -Os
            -fomit-frame-pointer
            -fno-asynchronous-unwind-tables
            -flto
            -Wl,-flto
        )
    endif()
endif()

set(MEDIA_COMPILER_FLAGS_RELEASEINTERNAL
    -O0
    -fno-omit-frame-pointer
)

if(NOT ${PLATFORM} STREQUAL "android")
        set(MEDIA_COMPILER_FLAGS_RELEASEINTERNAL
            ${MEDIA_COMPILER_FLAGS_RELEASEINTERNAL}
            -finline-limit=100
        )
    endif()

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    list(APPEND MEDIA_COMPILER_FLAGS_COMMON
         -Wno-deprecated
         -Wno-missing-braces
         -Wno-overloaded-virtual
         -Wbitfield-constant-conversion
         -Wno-extern-c-compat
         -Wno-inconsistent-missing-override
         # for verison under clang-17
         -Wno-ignored-optimization-argument
         -gdwarf-4
        )
    list(REMOVE_ITEM MEDIA_COMPILER_FLAGS_COMMON
         -funswitch-loops
         -Wl,--gc-sections
         -Wl,--no-as-needed
         -fno-tree-pre
        )
    list(REMOVE_ITEM MEDIA_COMPILER_FLAGS_RELEASE
         -finline-limit=100
        )
    list(REMOVE_ITEM MEDIA_COMPILER_FLAGS_RELEASEINTERNAL
         -finline-limit=100
        )
endif()

set(MEDIA_COMPILER_FLAGS_DEBUG
    -O0
    -DINSTR_GTUNE_EXT
)

if(X11_FOUND AND LIBVAX11_FOUND)
    add_definitions(-DX11_FOUND)
endif()

include(${MEDIA_EXT_CMAKE}/ext/linux/media_compile_flags_linux_ext.cmake OPTIONAL)

if(${PLATFORM} STREQUAL "linux")
    #set predefined compiler flags set
    add_compile_options("${MEDIA_COMPILER_FLAGS_COMMON}")
    add_compile_options("$<$<CONFIG:Debug>:${MEDIA_COMPILER_FLAGS_DEBUG}>")
    add_compile_options("$<$<CONFIG:Release>:${MEDIA_COMPILER_FLAGS_RELEASE}>")
    add_compile_options("$<$<CONFIG:ReleaseInternal>:${MEDIA_COMPILER_FLAGS_RELEASEINTERNAL}>")

    foreach (flag ${MEDIA_COMPILER_CXX_FLAGS_COMMON})
        SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${flag}")
    endforeach()
endif()

if (NOT DEFINED GIT_COMMIT)
    execute_process(
        COMMAND git rev-parse --short HEAD
        OUTPUT_VARIABLE GIT_COMMIT
        OUTPUT_STRIP_TRAILING_WHITESPACE
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )
endif()

add_definitions(-DMEDIA_VERSION="${MEDIA_VERSION}")
add_definitions(-DMEDIA_VERSION_DETAILS="${GIT_COMMIT}")
