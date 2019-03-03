# Copyright (c) 2018, Intel Corporation
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
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
# OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
# ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
# OTHER DEALINGS IN THE SOFTWARE.


if(NOT DEFINED _os_release_info)
set(_os_release_info TRUE)


# os_release_info.cmake - Function to dump OS name and version

# This file has no dependencies on other files (e.g., functions or definitions)
# of the local cmake environment.

# Set cmake policies for at least this level:
cmake_minimum_required(VERSION 2.8.12)


# Function get_os_release_info - Determine and return OS name and version
#
# Args:
# 1.  the name of a variable to receive os_name
# 2.  the name of a variable to receive os_version
#
# Return values: (Quotation marks are always stripped).
# Upon failure, return values are null strings.
#
# Examples:
#   os_name           os_version
#   --------------    -------
#   clear-linux-os    21180          (Changes twice daily)
#   ubuntu            12.04  16.04  17.10  18.04
#   fedora            27
#   centos            6.9  7.4.1708
#
# Potential sources are tried (in order of preference) until a
# suitable one is found.

# Implementation documentation:
#
# The potential sources, in order, are as follows.
# - /etc/centos-release
#       Centos 7 also has /etc/os-release.  File /etc/os-release is less
#       precise about the Centos version (e.g., "7" instead of "7.4.1708").
#       For that reason, this file is checked first.
#       Examples:
#       CentOS release 6.9 (Final)
#       CentOS Linux release 7.4.1708 (Core)
# - /usr/lib/os-release
#       Present for Clear Linux, modern Fedora, and Ubuntu since some time
#       between 14.04 and 16.04.  The ID and VERSION_ID values are used.
#       Examples:
#       ID=clear-linux-os      VERSION_ID=21180
#       ID=fedora              VERSION_ID=27
#       ID=ubuntu              VERSION_ID="14.04"
#       ID=ubuntu              VERSION_ID="16.04"
#       ID="ubuntu"            VERSION_ID="17.10"
# - /etc/os-release - Same form as (sometimes a link to) /usr/lib/os-release
#       ID="Ubuntu"            VERSION_ID="12.04"
#       ID="Ubuntu"            VERSION_ID="14.04"
#           with a symbolic link: /etc/os-release -> ../usr/lib/os-release
#       ID="CentOS Linux"      VERSION_ID="7"    Also: ID_LIKE="rhel fedora"
# - /etc/lsb-release
#       For Centos, not too meaningful.
#       Other "OS"s are more reasonable:
#       DISTRIB_ID=Ubuntu      DISTRIB_RELEASE=12.04
#       DISTRIB_ID=Ubuntu      DISTRIB_RELEASE=14.04
#       DISTRIB_ID=Ubuntu      DISTRIB_RELEASE=17.10


function(get_os_release_info _vn_id _vn_version_id)
    set(_var_id "")
    set(_var_version_id "")

    if("${_var_id}" STREQUAL "")
        set(file_path "/etc/centos-release")
        if(EXISTS "${file_path}")
            # Example: CentOS release 6.9 (Final)
            file(STRINGS "${file_path}" file_list LIMIT_COUNT 1)
            list(GET file_list 0 file_line)

            # Remove all parenthesized items.
            string(REGEX REPLACE "\\([^)]+\\)" "" file_line "${file_line}")

            # Extract start and end, discard optional "version" or "release"
            string(REGEX MATCH "^([A-Za-z0-9_]+)( +(version|release))? +(.*)$" _dummy "${file_line}")
            #                    1              2  3                    4

            set(_var_id "${CMAKE_MATCH_1}")
            set(_var_version_id "${CMAKE_MATCH_4}")
        endif()
    endif()

    if("${_var_id}" STREQUAL "")
        if(EXISTS "/usr/lib/os-release")
            set(file_path "/usr/lib/os-release")
        elseif(EXISTS "/etc/os-release")
            set(file_path "/etc/os-release")
        else()
            set(file_path "")
        endif()

        if(NOT "${file_path}" STREQUAL "")
            file(STRINGS "${file_path}" data_list REGEX "^(ID|VERSION_ID)=")

            # Look for lines like "ID="..." and VERSION_ID="..."
            foreach(_var ${data_list})
                if("${_var}" MATCHES "^(ID)=(.*)$")
                    set(_var_id "${CMAKE_MATCH_2}")
                elseif("${_var}" MATCHES "^(VERSION_ID)=(.*)$")
                    set(_var_version_id "${CMAKE_MATCH_2}")
                endif()
            endforeach()
        endif()
    endif()

    if("${_var_id}" STREQUAL "")
        set(file_path "/etc/lsb-release")
        if(EXISTS "${file_path}")
            file(STRINGS "${file_path}" data_list REGEX "^(DISTRIB_ID|DISTRIB_RELEASE)=")

            # Look for lines like "DISTRIB_ID="..." and DISTRIB_RELEASE="..."
            foreach(_var ${data_list})
                if("${_var}" MATCHES "^(DISTRIB_ID)=(.*)$")
                    set(_var_id "${CMAKE_MATCH_2}")
                elseif("${_var}" MATCHES "^(DISTRIB_RELEASE)=(.*)$")
                    set(_var_version_id "${CMAKE_MATCH_2}")
                endif()
            endforeach()
        endif()
    endif()

    string(TOLOWER "${_var_id}" "_var_id")

    string(STRIP "${_var_id}" _var_id)
    string(STRIP "${_var_version_id}" _var_version_id)

    # Remove any enclosing quotation marks
    string(REGEX REPLACE "^\"(.*)\"$" "\\1" _var_id "${_var_id}")
    string(REGEX REPLACE "^\"(.*)\"$" "\\1" _var_version_id "${_var_version_id}")

    if(NOT "${_vn_id}" STREQUAL "")
        set(${_vn_id} "${_var_id}" PARENT_SCOPE)
    endif()

    if(NOT "${_vn_version_id}" STREQUAL "")
        set(${_vn_version_id} "${_var_version_id}" PARENT_SCOPE)
    endif()

endfunction()


endif(NOT DEFINED _os_release_info)
