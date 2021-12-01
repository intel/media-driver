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


if(ENABLE_REQUIRED_GEN_CODE OR GEN8_BDW)
    media_include_subdirectory(media_interfaces_m8_bdw)
endif()

if(ENABLE_REQUIRED_GEN_CODE OR GEN9_BXT)
    media_include_subdirectory(media_interfaces_m9_bxt)
endif()

if(ENABLE_REQUIRED_GEN_CODE OR GEN9_SKL)
    media_include_subdirectory(media_interfaces_m9_skl)
endif()

if(ENABLE_REQUIRED_GEN_CODE OR GEN9_CFL)
    media_include_subdirectory(media_interfaces_m9_cfl)
endif()

if(ENABLE_REQUIRED_GEN_CODE OR GEN9_GLK)
    media_include_subdirectory(media_interfaces_m9_glk)
endif()

if(ENABLE_REQUIRED_GEN_CODE OR GEN9_KBL)
    media_include_subdirectory(media_interfaces_m9_kbl)
endif()

if(ENABLE_REQUIRED_GEN_CODE OR GEN10_CNL)
    media_include_subdirectory(media_interfaces_m10_cnl)
endif()

if(ENABLE_REQUIRED_GEN_CODE OR GEN11_ICLLP)
    media_include_subdirectory(media_interfaces_m11_icllp)
endif()

if(ENABLE_REQUIRED_GEN_CODE OR GEN11_JSL)
    media_include_subdirectory(media_interfaces_m11_jsl_ehl)
endif()

if(ENABLE_REQUIRED_GEN_CODE OR GEN12_TGLLP)
    media_include_subdirectory(media_interfaces_m12_tgllp)
endif()

if(ENABLE_REQUIRED_GEN_CODE OR GEN12_DG1)
    media_include_subdirectory(media_interfaces_m12_dg1)
endif()

if(ENABLE_REQUIRED_GEN_CODE OR GEN12_RKL)
    media_include_subdirectory(media_interfaces_m12_rkl)
endif()

if(ENABLE_REQUIRED_GEN_CODE OR GEN12_ADLS)
    media_include_subdirectory(media_interfaces_m12_adls)
endif()

if(ENABLE_REQUIRED_GEN_CODE OR GEN12_ADLP)
    media_include_subdirectory(media_interfaces_m12_adlp)
endif()

if(GEN12_ADLN)
    media_include_subdirectory(media_interfaces_m12_adln)
endif()

if(XEHP_SDV)
    media_include_subdirectory(media_interfaces_xehp_sdv)
endif()

if(DG2)
    media_include_subdirectory(media_interfaces_dg2)
endif()

if(PVC)
    media_include_subdirectory(media_interfaces_pvc)
endif()


include(${MEDIA_EXT}/media_interface/media_srcs.cmake OPTIONAL)
