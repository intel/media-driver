# Copyright (c) 2021, Intel Corporation
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

bs_set_if_undefined(PLATFORM         "linux")
bs_set_if_undefined(ARCH             "64")
bs_set_if_undefined(UFO_MARCH        "corei7")
bs_set_if_undefined(CMAKE_BUILD_TYPE "Release")
# UFO_VARIANT=default  --> default build
# UFO_VARIANT=nano     --> build optimized for size, a loss of performance is allowed
bs_set_if_undefined(UFO_VARIANT      "default")

bs_set_if_undefined(MEDIA_SOFTLET            "${BS_DIR_MEDIA}/media_softlet")
bs_set_if_undefined(MEDIA_SOFTLET_CMAKE      "${MEDIA_SOFTLET}/cmake")
bs_set_if_undefined(MEDIA_SOFTLET_AGNOSTIC   "${MEDIA_SOFTLET}/agnostic")
bs_set_if_undefined(MEDIA_SOFTLET_LINUX      "${MEDIA_SOFTLET}/linux")

bs_set_if_undefined(MEDIA_COMMON            "${BS_DIR_MEDIA}/media_common")

include( ${MEDIA_SOFTLET_EXT_CMAKE}/media_defs_ext.cmake OPTIONAL )
include( ${MEDIA_COMMON_EXT_CMAKE}/media_defs_ext.cmake OPTIONAL )

#need to clean when softlet completed
bs_set_if_undefined(MEDIA_DRIVER_ROOT        "${BS_DIR_MEDIA}/media_driver")
