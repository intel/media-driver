# Copyright (c) 2017-2024, Intel Corporation
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
    "GEN12" OFF)

cmake_dependent_option(DG2
    "Enabled DG2 support" ON
    "Xe_M" OFF)

# Using render IP name for kernel binary
cmake_dependent_option(XE_HPG
    "Enabled XE_HPG support" ON
    "DG2" OFF)

cmake_dependent_option(XEHP_SDV
    "Enabled Xehp_sdv support" ON
    "Xe_M" OFF)

cmake_dependent_option(PVC
    "Enabled PVC support" ON
    "Xe_M;ENABLE_PRODUCTION_KMD" OFF)

option(MTL "Enable MTL support" ON)

option(ARL "Enable ARL support" ON)

if(MTL OR ARL)
    option(XE_LPM_PLUS_SUPPORT "Enable XE_LPM_PLUS support" ON)
    option(XE_LPG "Enable XE_LPG support" ON)
endif()

option(BMG "Enable BMG support" ON)

if(MTL OR BMG)
    option(Xe_M_plus "Enable Xe_M_plus support" ON)
endif()

option(LNL "Enable LNL support" ON)

if(LNL)
    option(XE2_LPM_SUPPORT "Enable XE2_LPM support" ON)
endif()

if(LNL OR BMG)
    option(XE2_HPG "Enable XE2_HPG support" ON)
endif()

if(LNL)
    option(Xe2_M_plus "Enable Xe2_M_plus support" ON)
endif()

if(BMG)
    option(XE2_HPM_SUPPORT "Enable XE2_HPM support" ON)
endif()

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
endif()

if(PVC)
    add_definitions(-DIGFX_PVC_SUPPORTED)
    add_definitions(-DIGFX_PVC_CMFCPATCH_SUPPORTED)
endif()

if(XEHP_SDV)
    add_definitions(-DIGFX_XEHP_SDV_SUPPORTED)
    add_definitions(-DIGFX_XEHP_SDV_CMFCPATCH_SUPPORTED)
endif()

if(XE_HPG)
    add_definitions(-DIGFX_XE_HPG_SUPPORTED)
    add_definitions(-DIGFX_XE_HPG_CMFCPATCH_SUPPORTED)
endif()

if(MTL)
    add_definitions(-DIGFX_MTL_SUPPORTED)
endif()

if(ARL)
    add_definitions(-DIGFX_ARL_SUPPORTED)
endif()

if(XE2_HPG)
    add_definitions(-DIGFX_XE2_HPG_SUPPORTED)
    add_definitions(-DIGFX_XE2_HPG_CMFCPATCH_SUPPORTED)
endif()

if(LNL)
    add_definitions(-DIGFX_LNL_SUPPORTED)
endif()

include(${MEDIA_EXT_CMAKE}/ext/linux/media_gen_flags_linux_ext.cmake OPTIONAL)
