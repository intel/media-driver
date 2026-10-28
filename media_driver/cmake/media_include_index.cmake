# Copyright (c) 2026, Florian Schmaus
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

# Include index
#
# Every media_srcs.cmake adds its own directory to the include path and the
# sources include headers by file name only. The driver targets therefore get
# several hundred -I flags with absolute paths. gcc passes all options to its
# sub-processes in the single environment variable COLLECT_GCC_OPTIONS, which
# the kernel limits to 128 KiB (MAX_ARG_STRLEN), so the build fails once the
# source tree lives in a long enough path.
#
# media_apply_include_index() replaces the in-tree include directories of each
# target by one generated directory. It contains one forwarding file per
# relative path reachable through the original include directories, pointing to
# the file that the original search order would have found first. The real
# header is opened by its own path, so nested #include "..." lookups still
# start in the real header's directory. Header resolution is unchanged.

if(${PLATFORM} STREQUAL "linux" AND NOT CMAKE_WDDM_LINUX)
    set(_media_include_index_default ON)
else()
    set(_media_include_index_default OFF)
endif()
option(MEDIA_USE_INCLUDE_INDEX
    "Replace the in-tree include directories of the driver targets by one generated include index directory"
    ${_media_include_index_default})

bs_set_if_undefined(MEDIA_INCLUDE_INDEX_ROOTS "${BS_DIR_MEDIA}")

# Upper bound for the bytes the include flags of a target may take on the
# compiler command line.
set(MEDIA_INCLUDE_FLAGS_LIMIT 32768)
set(MEDIA_INCLUDE_FLAGS_WARN_LIMIT 98304)

# File types that are included through the include path. googletest's
# gtest-all.cc includes its .cc files that way.
bs_set_if_undefined(MEDIA_INCLUDE_INDEX_PATTERNS "*.h;*.hh;*.hpp;*.hxx;*.inl;*.inc;*.ipp;*.tcc;*.cc")

# Write content to file only if it differs, so a re-configure does not
# touch unchanged forwarding headers and trigger a rebuild.
function(_media_include_index_write file content)
    if(EXISTS "${file}")
        file(READ "${file}" _old)
        if(_old STREQUAL content)
            return()
        endif()
    endif()
    file(WRITE "${file}" "${content}")
endfunction()

# Return the normalized path of dir if it can be indexed, empty otherwise.
# Only existing, absolute directories below MEDIA_INCLUDE_INDEX_ROOTS and
# outside of the build directory are indexed. The build directory is left
# alone because it may receive generated headers after configuration.
function(_media_include_index_normalize dir binary_dir out)
    set(${out} "" PARENT_SCOPE)
    if(dir MATCHES "\\$<" OR NOT IS_ABSOLUTE "${dir}" OR NOT IS_DIRECTORY "${dir}")
        return()
    endif()
    get_filename_component(_norm "${dir}" ABSOLUTE)
    string(FIND "${_norm}/" "${binary_dir}/" _pos)
    if(_pos EQUAL 0)
        return()
    endif()
    foreach(_root ${MEDIA_INCLUDE_INDEX_ROOTS})
        get_filename_component(_root "${_root}" ABSOLUTE)
        string(FIND "${_norm}/" "${_root}/" _pos)
        if(_pos EQUAL 0)
            set(${out} "${_norm}" PARENT_SCOPE)
            return()
        endif()
    endforeach()
endfunction()

# Generate the forwarding headers in index_dir for the given include
# directories (in search order) and remove stale ones.
function(_media_include_index_generate index_dir binary_dir)
    set(_configure_depends)
    if(NOT CMAKE_VERSION VERSION_LESS 3.12)
        set(_configure_depends CONFIGURE_DEPENDS)
    endif()

    foreach(_dir ${ARGN})
        get_filename_component(_norm "${_dir}" ABSOLUTE)
        set(_globs)
        foreach(_pattern ${MEDIA_INCLUDE_INDEX_PATTERNS})
            list(APPEND _globs "${_norm}/${_pattern}")
        endforeach()
        # Recurse to cover subpath includes such as "gtest/internal/gtest-port.h".
        file(GLOB_RECURSE _headers LIST_DIRECTORIES false RELATIVE "${_norm}" ${_configure_depends} ${_globs})
        foreach(_rel ${_headers})
            string(FIND "${_norm}/${_rel}" "${binary_dir}/" _pos)
            if(_pos EQUAL 0 OR DEFINED "_mii_${_rel}")
                continue()
            endif()
            set("_mii_${_rel}" 1)
            # Keep the include directory as given, so the header is opened by
            # the same path as without the index.
            _media_include_index_write("${index_dir}/${_rel}" "#include \"${_dir}/${_rel}\"\n")
        endforeach()
    endforeach()

    file(GLOB_RECURSE _existing LIST_DIRECTORIES false RELATIVE "${index_dir}" "${index_dir}/*")
    foreach(_rel ${_existing})
        if(NOT DEFINED "_mii_${_rel}")
            file(REMOVE "${index_dir}/${_rel}")
        endif()
    endforeach()
endfunction()

function(_media_check_include_flags target mode limit)
    get_target_property(_dirs ${target} INCLUDE_DIRECTORIES)
    if(NOT _dirs)
        return()
    endif()
    # CMake emits each include directory only once.
    list(REMOVE_DUPLICATES _dirs)
    set(_bytes 0)
    foreach(_dir ${_dirs})
        string(LENGTH "${_dir}" _len)
        math(EXPR _bytes "${_bytes} + ${_len} + 3")
    endforeach()
    if(_bytes GREATER limit)
        list(LENGTH _dirs _count)
        if(MEDIA_USE_INCLUDE_INDEX)
            set(_hint "Include directories outside of MEDIA_INCLUDE_INDEX_ROOTS are not indexed.")
        else()
            set(_hint "Enable MEDIA_USE_INCLUDE_INDEX to shorten the compiler command lines.")
        endif()
        message(${mode} "-- media -- ${target}: ${_count} include directories take ${_bytes} bytes on the compiler "
            "command line (limit ${limit}). gcc fails with \"Argument list too long\" once its options exceed 128 KiB. "
            "${_hint}")
    endif()
endfunction()

# Return the compiled targets defined in dir and its subdirectories.
function(_media_include_index_targets dir out)
    set(_targets)
    get_property(_all DIRECTORY "${dir}" PROPERTY BUILDSYSTEM_TARGETS)
    foreach(_target ${_all})
        get_target_property(_type ${_target} TYPE)
        if(_type MATCHES "^(EXECUTABLE|STATIC_LIBRARY|SHARED_LIBRARY|MODULE_LIBRARY|OBJECT_LIBRARY)$")
            list(APPEND _targets ${_target})
        endif()
    endforeach()
    get_property(_subdirs DIRECTORY "${dir}" PROPERTY SUBDIRECTORIES)
    foreach(_subdir ${_subdirs})
        _media_include_index_targets("${_subdir}" _sub)
        list(APPEND _targets ${_sub})
    endforeach()
    set(${out} ${_targets} PARENT_SCOPE)
endfunction()

function(_media_include_index_apply target binary_dir)
    get_target_property(_dirs ${target} INCLUDE_DIRECTORIES)
    if(NOT _dirs)
        return()
    endif()

    # The index takes the place of the first indexed directory. Everything
    # else keeps its position relative to it.
    set(_head)
    set(_indexed)
    set(_tail)
    foreach(_dir ${_dirs})
        _media_include_index_normalize("${_dir}" "${binary_dir}" _norm)
        if(_norm)
            list(APPEND _indexed "${_dir}")
        elseif(_indexed)
            list(APPEND _tail "${_dir}")
        else()
            list(APPEND _head "${_dir}")
        endif()
    endforeach()

    if(_indexed)
        list(REMOVE_DUPLICATES _indexed)
        # Targets with the same include directories share one index.
        string(SHA1 _key "${_indexed}")
        string(SUBSTRING "${_key}" 0 16 _key)
        set(_index_dir "${CMAKE_BINARY_DIR}/media_include_index/${_key}")
        get_property(_generated GLOBAL PROPERTY MEDIA_INCLUDE_INDEX_${_key} SET)
        if(NOT _generated)
            _media_include_index_generate("${_index_dir}" "${binary_dir}" ${_indexed})
            set_property(GLOBAL PROPERTY MEDIA_INCLUDE_INDEX_${_key} 1)
        endif()
        set_property(TARGET ${target} PROPERTY INCLUDE_DIRECTORIES ${_head} "${_index_dir}" ${_tail})
    endif()
endfunction()

# media_apply_include_index()
#
# Applies the index to all targets defined in the current directory and its
# subdirectories. Call it once, after the last of these targets has received
# its include directories.
function(media_apply_include_index)
    # BUILDSYSTEM_TARGETS and SUBDIRECTORIES require CMake 3.7.
    if(CMAKE_VERSION VERSION_LESS 3.7)
        return()
    endif()

    _media_include_index_targets("${CMAKE_CURRENT_SOURCE_DIR}" _targets)

    if(NOT MEDIA_USE_INCLUDE_INDEX)
        foreach(_target ${_targets})
            _media_check_include_flags(${_target} WARNING ${MEDIA_INCLUDE_FLAGS_WARN_LIMIT})
        endforeach()
        return()
    endif()

    get_filename_component(_binary_dir "${CMAKE_BINARY_DIR}" ABSOLUTE)
    foreach(_target ${_targets})
        _media_include_index_apply(${_target} "${_binary_dir}")
        _media_check_include_flags(${_target} FATAL_ERROR ${MEDIA_INCLUDE_FLAGS_LIMIT})
    endforeach()
endfunction()
