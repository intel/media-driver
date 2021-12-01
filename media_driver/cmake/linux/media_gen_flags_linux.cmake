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

# TODO: Remove this option when all Gen options can successfully disable
#       driver code.
# Status:
#   GEN8:  TODO
#   GEN9:  TODO
#   GEN10: TODO (Remove)
#   GEN11: TODO
#   GEN12: TODO
option(ENABLE_REQUIRED_GEN_CODE "Make per-Gen options disable only media-kernels (not driver code) if driver code is known to be required for a successful build"
    ON)

option(GEN8 "Enable Gen8 support" ON)
cmake_dependent_option(GEN8_BDW
    "Enabled BDW support (Gen8)" ON
    "GEN8" OFF)

option(GEN9 "Enable Gen9 support" ON)
cmake_dependent_option(GEN9_BXT
    "Enabled BXT support (Gen9)" ON
    "GEN9" OFF)
cmake_dependent_option(GEN9_CFL
    "Enabled CFL support (Gen9)" ON
    "GEN9" OFF)
cmake_dependent_option(GEN9_GLK
    "Enabled GLK support (Gen9)" ON
    "GEN9" OFF)
cmake_dependent_option(GEN9_KBL
    "Enabled KBL support (Gen9)" ON
    "GEN9" OFF)
cmake_dependent_option(GEN9_SKL
    "Enabled SKL support (Gen9)" ON
    "GEN9" OFF)
cmake_dependent_option(GEN9_CML
    "Enabled CML support (Gen9)" ON
    "GEN9" OFF)
cmake_dependent_option(GEN9_CMPV
    "Enabled CMPV support (Gen9)" ON
    "GEN9" OFF)

option(GEN10 "Enable Gen10 support" ON)
cmake_dependent_option(GEN10_CNL
    "Enabled CNL support (Gen10)" ON
    "GEN10" OFF)

option(GEN11 "Enable Gen11 support" ON)
cmake_dependent_option(GEN11_ICLLP
    "Enabled ICLLP support (Gen11)" ON
    "GEN11" OFF)

cmake_dependent_option(GEN11_JSL
    "Enabled JSL support (Gen11)" ON
    "GEN11" OFF)

option(GEN12 "Enable Gen12 support" ON)
cmake_dependent_option(GEN12_TGLLP
    "Enabled TGLLP support (Gen12)" ON
    "GEN12" OFF)

cmake_dependent_option(GEN12_DG1
    "Enabled DG1 support (Gen12)" ON
    "GEN12" OFF)

cmake_dependent_option(GEN12_RKL
    "Enabled RKL support (Gen12)" ON
    "GEN12_TGLLP" OFF)

cmake_dependent_option(GEN12_ADLS
    "Enabled ADLS support (Gen12)" ON
    "GEN12_TGLLP" OFF)

cmake_dependent_option(GEN12_ADLP
    "Enabled ADLP support (Gen12)" ON
    "GEN12_TGLLP" OFF)

cmake_dependent_option(GEN12_ADLN
    "Enabled ADLN support (Gen12)" ON
    "GEN12_TGLLP" OFF)

cmake_dependent_option(Xe_M
    "Enabled support for Xehp_sdv+ platforms" ON
    "ENABLE_PRODUCTION_KMD" OFF)

cmake_dependent_option(DG2
    "Enabled DG2 support" ON
    "Xe_M;ENABLE_PRODUCTION_KMD" OFF)

cmake_dependent_option(XEHP_SDV
    "Enabled Xehp_sdv support" ON
    "Xe_M;ENABLE_PRODUCTION_KMD" OFF)

cmake_dependent_option(PVC
    "Enabled PVC support" ON
    "Xe_M;ENABLE_PRODUCTION_KMD" OFF)

if(GEN8)
    add_definitions(-DIGFX_GEN8_SUPPORTED)
endif()

if(GEN8_BDW)
    add_definitions(-DIGFX_GEN8_BDW_SUPPORTED)
endif()

if(GEN9)
    add_definitions(-DIGFX_GEN9_SUPPORTED)
endif()

if(GEN9_BXT)
    add_definitions(-DIGFX_GEN9_BXT_SUPPORTED)
endif()

if(GEN9_SKL)
    add_definitions(-DIGFX_GEN9_SKL_SUPPORTED)
endif()

if(GEN9_CFL)
    add_definitions(-DIGFX_GEN9_CFL_SUPPORTED)
endif()

if(GEN9_GLK)
    add_definitions(-DIGFX_GEN9_GLK_SUPPORTED)
endif()

if(GEN9_KBL)
    add_definitions(-DIGFX_GEN9_KBL_SUPPORTED)
endif()

if(GEN9_CML)
    add_definitions(-DIGFX_GEN9_CML_SUPPORTED)
endif()

if(GEN9_CMPV)
    add_definitions(-DIGFX_GEN9_CMPV_SUPPORTED)
endif()

if(GEN10)
    add_definitions(-DIGFX_GEN10_SUPPORTED)
endif()

if(GEN10_CNL)
    add_definitions(-DIGFX_GEN10_CNL_SUPPORTED)
endif()

if(GEN11)
    add_definitions(-DIGFX_GEN11_SUPPORTED)
endif()

if(GEN11_ICLLP)
    add_definitions(-DIGFX_GEN11_ICLLP_SUPPORTED)
endif()

if(GEN11_JSL)
    add_definitions(-DIGFX_GEN11_JSL_SUPPORTED)
endif()

if(GEN12)
    add_definitions(-DIGFX_GEN12_SUPPORTED)
endif()

if(GEN12_TGLLP)
    add_definitions(-DIGFX_GEN12_TGLLP_SUPPORTED)
    add_definitions(-DIGFX_GEN12_TGLLP_SWSB_SUPPORTED)
    add_definitions(-DIGFX_GEN12_TGLLP_CMFC_SUPPORTED)
    add_definitions(-DIGFX_GEN12_TGLLP_CMFCPATCH_SUPPORTED)
endif()

if(GEN12_DG1)
    add_definitions(-DIGFX_GEN12_DG1_SUPPORTED)
endif()

if(GEN12_RKL)
    add_definitions(-DIGFX_GEN12_RKL_SUPPORTED)
endif()

if(GEN12_ADLS)
    add_definitions(-DIGFX_GEN12_ADLS_SUPPORTED)
endif()

if(GEN12_ADLP)
    add_definitions(-DIGFX_GEN12_ADLP_SUPPORTED)
endif()

if(GEN12_ADLN)
    add_definitions(-DIGFX_GEN12_ADLN_SUPPORTED)
endif()

if(DG2)
    add_definitions(-DIGFX_DG2_SUPPORTED)
    add_definitions(-DIGFX_DG2_CMFCPATCH_SUPPORTED)
endif()

if(PVC)
    add_definitions(-DIGFX_PVC_SUPPORTED)
    add_definitions(-DIGFX_PVC_CMFCPATCH_SUPPORTED)
endif()

if(XEHP_SDV)
    add_definitions(-DIGFX_XEHP_SDV_SUPPORTED)
    add_definitions(-DIGFX_XEHP_SDV_CMFCPATCH_SUPPORTED)
endif()

include(${MEDIA_EXT_CMAKE}/ext/linux/media_gen_flags_linux_ext.cmake OPTIONAL)
