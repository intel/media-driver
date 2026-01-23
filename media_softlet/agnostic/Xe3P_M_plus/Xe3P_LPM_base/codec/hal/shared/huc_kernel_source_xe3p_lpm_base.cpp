/*
* Copyright (c) 2024-2026, Intel Corporation
*
* Permission is hereby granted, free of charge, to any person obtaining a
* copy of this software and associated documentation files (the "Software"),
* to deal in the Software without restriction, including without limitation
* the rights to use, copy, modify, merge, publish, distribute, sublicense,
* and/or sell copies of the Software, and to permit persons to whom the
* Software is furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included
* in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
* OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
* OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
* ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
* OTHER DEALINGS IN THE SOFTWARE.
*/
//!
//! \file     huc_kernel_source_xe3p_lpm_base.cpp
//! \brief    Implementation of the huc kernel source management
//!
#include "huc_kernel_source_xe3p_lpm_base.h"

#include "nvlMediaKernels.h"

#ifndef MEDIA_BIN_SUPPORT
HUC_KERNEL_BIN_LOCAL(__MediaKernels_s2l_nvl)
HUC_KERNEL_BIN_LOCAL(__MediaKernels_copykrn_nvl)
HUC_KERNEL_BIN_LOCAL(__MediaKernels_avcbrc_init_nvl)
HUC_KERNEL_BIN_LOCAL(__MediaKernels_avcbrc_update_nvl)
HUC_KERNEL_BIN_LOCAL(__MediaKernels_vp9dec_nvl)
HUC_KERNEL_BIN_LOCAL(__MediaKernels_vp9hpu_nvl)
HUC_KERNEL_BIN_LOCAL(__MediaKernels_hevcbrc_init_nvl)
HUC_KERNEL_BIN_LOCAL(__MediaKernels_hevcbrc_update_nvl)
HUC_KERNEL_BIN_LOCAL(__MediaKernels_vp9brc_init_nvl)
HUC_KERNEL_BIN_LOCAL(__MediaKernels_vp9brc_update_nvl)
HUC_KERNEL_BIN_LOCAL(__MediaKernels_pakint_nvl)
HUC_KERNEL_BIN_LOCAL(__MediaKernels_lookahead_nvl)
HUC_KERNEL_BIN_LOCAL(__MediaKernels_av1ba_nvl)
HUC_KERNEL_BIN_LOCAL(__MediaKernels_av1brc_init_nvl)
HUC_KERNEL_BIN_LOCAL(__MediaKernels_av1brc_update_nvl)
HUC_KERNEL_BIN_LOCAL(__MediaKernels_vvcs2l_nvl)
#ifdef _MEDIA_RESERVED
HUC_KERNEL_BIN_LOCAL(__MediaKernels_drm_nvl)
HUC_KERNEL_BIN_LOCAL(__MediaKernels_avcbrc_pxp_init_nvl)
HUC_KERNEL_BIN_LOCAL(__MediaKernels_avcbrc_pxp_update_nvl)
#endif
HUC_KERNEL_BIN_LOCAL(__MediaKernels_av1slbb_update_nvl)
HUC_KERNEL_BIN_LOCAL(__MediaKernels_avcslbb_update_nvl)
HUC_KERNEL_BIN_LOCAL(__MediaKernels_hevcslbb_update_nvl)
#endif

const HucKernelSource::BinaryTable HucKernelSourceXe3P_Lpm_Base::m_binTable =
{
    {HucKernelSource::hevcS2lKernelId,           HUC_KERNEL_BIN_ELEMENT(__MediaKernels_s2l_nvl)},
#ifdef _MEDIA_RESERVED
    {HucKernelSource::drmKernelId,               HUC_KERNEL_BIN_ELEMENT(__MediaKernels_drm_nvl)},
#endif
    {HucKernelSource::copyKernelId,              HUC_KERNEL_BIN_ELEMENT(__MediaKernels_copykrn_nvl)},
    {HucKernelSource::vdencBrcInitKernelId,      HUC_KERNEL_BIN_ELEMENT(__MediaKernels_avcbrc_init_nvl)},
    {HucKernelSource::vdencBrcUpdateKernelId,    HUC_KERNEL_BIN_ELEMENT(__MediaKernels_avcbrc_update_nvl)},
    {HucKernelSource::vp9ProbUpdateKernelId,     HUC_KERNEL_BIN_ELEMENT(__MediaKernels_vp9dec_nvl)},
    {HucKernelSource::vp9EncKernelId,            HUC_KERNEL_BIN_ELEMENT(__MediaKernels_vp9hpu_nvl)},
    {HucKernelSource::hevcBrcInitKernelId,       HUC_KERNEL_BIN_ELEMENT(__MediaKernels_hevcbrc_init_nvl)},
    {HucKernelSource::hevcBrcUpdateKernelId,     HUC_KERNEL_BIN_ELEMENT(__MediaKernels_hevcbrc_update_nvl)},
    {HucKernelSource::hevcBrcLowdelayKernelId,   HucKernelSource::m_invalidKernelBin},
    {HucKernelSource::vp9VdencBrcInitKernelId,   HUC_KERNEL_BIN_ELEMENT(__MediaKernels_vp9brc_init_nvl)},
    {HucKernelSource::vp9VdencBrcUpdateKernelId, HUC_KERNEL_BIN_ELEMENT(__MediaKernels_vp9brc_update_nvl)},
    {HucKernelSource::vp9VdencProbKernelId,      HucKernelSource::m_invalidKernelBin},
    {HucKernelSource::pakIntegrationKernelId,    HUC_KERNEL_BIN_ELEMENT(__MediaKernels_pakint_nvl)},
    {HucKernelSource::hevcLaAnalysisKernelId,    HUC_KERNEL_BIN_ELEMENT(__MediaKernels_lookahead_nvl)},
    {HucKernelSource::backAnnonationKernelId,    HUC_KERNEL_BIN_ELEMENT(__MediaKernels_av1ba_nvl)},
    {HucKernelSource::av1BrcInitKernelId,        HUC_KERNEL_BIN_ELEMENT(__MediaKernels_av1brc_init_nvl)},
    {HucKernelSource::av1BrcUpdateKernelId,      HUC_KERNEL_BIN_ELEMENT(__MediaKernels_av1brc_update_nvl)},
    {HucKernelSource::vvcS2lKernelId,            HUC_KERNEL_BIN_ELEMENT(__MediaKernels_vvcs2l_nvl)},
#ifdef _MEDIA_RESERVED
    {HucKernelSource::avcPxpBrcInitKernelId,     HUC_KERNEL_BIN_ELEMENT(__MediaKernels_avcbrc_pxp_init_nvl)},
    {HucKernelSource::avcPxpBrcUpdateKernelId,   HUC_KERNEL_BIN_ELEMENT(__MediaKernels_avcbrc_pxp_update_nvl)},
#endif
    {HucKernelSource::av1SlbbUpdateKernelId,     HUC_KERNEL_BIN_ELEMENT(__MediaKernels_av1slbb_update_nvl)},
    {HucKernelSource::avcSlbbUpdateKernelId,     HUC_KERNEL_BIN_ELEMENT(__MediaKernels_avcslbb_update_nvl)},
    {HucKernelSource::hevcSlbbUpdateKernelId,    HUC_KERNEL_BIN_ELEMENT(__MediaKernels_hevcslbb_update_nvl)},
};

// Hash index table (kernel id, hash index)
const HucKernelSource::HashIdxTable HucKernelSourceXe3P_Lpm_Base::m_hashIdxTable =
{
    {HucKernelSource::hevcS2lKernelId,           1},
    {HucKernelSource::drmKernelId,               2},
    {HucKernelSource::copyKernelId,              3},
    {HucKernelSource::avcPxpBrcInitKernelId,     4},
    {HucKernelSource::avcPxpBrcUpdateKernelId,   5},
    {HucKernelSource::vp9ProbUpdateKernelId,     6},
    {HucKernelSource::vdencBrcInitKernelId,      HucKernelSource::m_invalidHashIndex},
    {HucKernelSource::vdencBrcUpdateKernelId,    HucKernelSource::m_invalidHashIndex},
    {HucKernelSource::vp9EncKernelId,            HucKernelSource::m_invalidHashIndex},
    {HucKernelSource::hevcBrcInitKernelId,       HucKernelSource::m_invalidHashIndex},
    {HucKernelSource::hevcBrcUpdateKernelId,     HucKernelSource::m_invalidHashIndex},
    {HucKernelSource::hevcBrcLowdelayKernelId,   HucKernelSource::m_invalidHashIndex},
    {HucKernelSource::vp9VdencBrcInitKernelId,   HucKernelSource::m_invalidHashIndex},
    {HucKernelSource::vp9VdencBrcUpdateKernelId, HucKernelSource::m_invalidHashIndex},
    {HucKernelSource::vp9VdencProbKernelId,      HucKernelSource::m_invalidHashIndex},
    {HucKernelSource::cmdInitializerKernelId,    HucKernelSource::m_invalidHashIndex},
    {HucKernelSource::pakIntegrationKernelId,    HucKernelSource::m_invalidHashIndex},
    {HucKernelSource::hevcLaAnalysisKernelId,    HucKernelSource::m_invalidHashIndex},
    {HucKernelSource::backAnnonationKernelId,    HucKernelSource::m_invalidHashIndex},
    {HucKernelSource::av1BrcInitKernelId,        HucKernelSource::m_invalidHashIndex},
    {HucKernelSource::av1BrcUpdateKernelId,      HucKernelSource::m_invalidHashIndex},
    {HucKernelSource::vvcS2lKernelId,            7},
    {HucKernelSource::av1SlbbUpdateKernelId,     HucKernelSource::m_invalidHashIndex},
    {HucKernelSource::avcSlbbUpdateKernelId,     HucKernelSource::m_invalidHashIndex},
    {HucKernelSource::hevcSlbbUpdateKernelId,    HucKernelSource::m_invalidHashIndex},
};

MOS_STATUS HucKernelSourceXe3P_Lpm_Base::GetManifest(HucManifest &manifest)
{
#ifdef _MEDIA_RESERVED
    manifest.m_data = reinterpret_cast<const uint8_t *>(__MediaKernels_manifest_nvl);
    manifest.m_size = __MediaKernels_manifest_nvl_size;
#endif
    return MOS_STATUS_SUCCESS;
}

HucKernelSourceXe3P_Lpm_Base &HucKernelSourceXe3P_Lpm_Base::GetInstance()
{
    static HucKernelSourceXe3P_Lpm_Base instance;
    return instance;
}
