/*
* Copyright (c) 2024, Intel Corporation
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
//! \file     nvlMediaKernels.h
//! \brief    Header file of the Xe3P_LPM_Base Huc kernel binary.
//!
#ifndef __NVL_MEDIA_KERNELS_H__
#define __NVL_MEDIA_KERNELS_H__
#include <stdint.h>
#include "media_bin_mgr.h"

#define DECLARE_HUC_KERNEL_BIN(hucKernelShortName, platform)                                   \
    DECLARE_SHARED_ARRAY_SIZE_UINT32(__MediaKernels_##hucKernelShortName##_##platform##_size); \
    DECLARE_SHARED_ARRAY_UINT32(__MediaKernels_##hucKernelShortName##_##platform);

#define DECLARE_HUC_KERNEL_MFT(platform)                                           \
    DECLARE_SHARED_ARRAY_SIZE_UINT32(__MediaKernels_manifest##_##platform##_size); \
    DECLARE_SHARED_ARRAY_UINT32(__MediaKernels_manifest##_##platform);

// The huc kernel short name should exactly match the name on Huc binary raw,
// for example __MediaKernels_s2l_nvl on nvlMediaKernel_s2l.h
DECLARE_HUC_KERNEL_BIN(s2l,            nvl);
DECLARE_HUC_KERNEL_BIN(drm,            nvl);
DECLARE_HUC_KERNEL_BIN(copykrn,        nvl);
DECLARE_HUC_KERNEL_BIN(avcbrc_init,    nvl);
DECLARE_HUC_KERNEL_BIN(avcbrc_update,  nvl);
DECLARE_HUC_KERNEL_BIN(vp9dec,         nvl);
DECLARE_HUC_KERNEL_BIN(vp9hpu,         nvl);
DECLARE_HUC_KERNEL_BIN(hevcbrc_init,   nvl);
DECLARE_HUC_KERNEL_BIN(hevcbrc_update, nvl);
DECLARE_HUC_KERNEL_BIN(vp9brc_init,    nvl);
DECLARE_HUC_KERNEL_BIN(vp9brc_update,  nvl);
DECLARE_HUC_KERNEL_BIN(pakint,         nvl);
DECLARE_HUC_KERNEL_BIN(lookahead,      nvl);
DECLARE_HUC_KERNEL_BIN(av1ba,          nvl);
DECLARE_HUC_KERNEL_BIN(av1brc_init,    nvl);
DECLARE_HUC_KERNEL_BIN(av1brc_update,  nvl);
DECLARE_HUC_KERNEL_BIN(vvcs2l,         nvl);
DECLARE_HUC_KERNEL_BIN(avcbrc_pxp_init,  nvl);
DECLARE_HUC_KERNEL_BIN(avcbrc_pxp_update,nvl);
DECLARE_HUC_KERNEL_BIN(av1slbb_update,  nvl);
DECLARE_HUC_KERNEL_BIN(avcslbb_update,  nvl);
DECLARE_HUC_KERNEL_BIN(hevcslbb_update, nvl);

DECLARE_HUC_KERNEL_MFT(nvl);

#if defined(MEDIA_BIN_SUPPORT)
#define __MediaKernels_s2l_nvl_NAME            "MediaKernels_s2l_1360"
#define __MediaKernels_drm_nvl_NAME            "MediaKernels_drm_1360"
#define __MediaKernels_copykrn_nvl_NAME        "MediaKernels_copykrn_1360"
#define __MediaKernels_avcbrc_init_nvl_NAME    "MediaKernels_avcbrc_init_1360"
#define __MediaKernels_avcbrc_update_nvl_NAME  "MediaKernels_avcbrc_update_1360"
#define __MediaKernels_vp9dec_nvl_NAME         "MediaKernels_vp9dec_1360"
#define __MediaKernels_vp9hpu_nvl_NAME         "MediaKernels_vp9hpu_1360"
#define __MediaKernels_hevcbrc_init_nvl_NAME   "MediaKernels_hevcbrc_init_1360"
#define __MediaKernels_hevcbrc_update_nvl_NAME "MediaKernels_hevcbrc_update_1360"
#define __MediaKernels_vp9brc_init_nvl_NAME    "MediaKernels_vp9brc_init_1360"
#define __MediaKernels_vp9brc_update_nvl_NAME  "MediaKernels_vp9brc_update_1360"
#define __MediaKernels_pakint_nvl_NAME         "MediaKernels_pakint_1360"
#define __MediaKernels_lookahead_nvl_NAME      "MediaKernels_lookahead_1360"
#define __MediaKernels_av1ba_nvl_NAME          "MediaKernels_av1ba_1360"
#define __MediaKernels_av1brc_init_nvl_NAME    "MediaKernels_av1brc_init_1360"
#define __MediaKernels_av1brc_update_nvl_NAME  "MediaKernels_av1brc_update_1360"
#define __MediaKernels_vvcs2l_nvl_NAME         "MediaKernels_vvcs2l_1360"
#define __MediaKernels_avcbrc_pxp_init_nvl_NAME   "MediaKernels_avcbrc_pxp_init_1360"
#define __MediaKernels_avcbrc_pxp_update_nvl_NAME "MediaKernels_avcbrc_pxp_update_1360"
#define __MediaKernels_av1slbb_update_nvl_NAME   "MediaKernels_av1slbb_update_1360"
#define __MediaKernels_avcslbb_update_nvl_NAME   "MediaKernels_avcslbb_update_1360"
#define __MediaKernels_hevcslbb_update_nvl_NAME  "MediaKernels_hevcslbb_update_1360"

#define __MediaKernels_manifest_nvl_NAME       "MediaKernels_manifest_1360"

#if defined(MEDIA_BIN_DLL)
#define REGISTER_HUC_KERNEL_BIN(hucKernelShortName, platform)                     \
    static bool  register__MediaKernels_##hucKernelShortName##_##platform =       \
        RegisterMediaBin(__MediaKernels_##hucKernelShortName##_##platform##_NAME, \
                         __MediaKernels_##hucKernelShortName##_##platform##_size, \
                         __MediaKernels_##hucKernelShortName##_##platform);

#define REGISTER_HUC_KERNEL_MFT(platform)                             \
    static bool  register__MediaKernels_manifest##_##platform =       \
        RegisterMediaBin(__MediaKernels_manifest##_##platform##_NAME, \
                         __MediaKernels_manifest##_##platform##_size, \
                         __MediaKernels_manifest##_##platform);

REGISTER_HUC_KERNEL_BIN(s2l,            nvl);
REGISTER_HUC_KERNEL_BIN(drm,            nvl);
REGISTER_HUC_KERNEL_BIN(copykrn,        nvl);
REGISTER_HUC_KERNEL_BIN(avcbrc_init,    nvl);
REGISTER_HUC_KERNEL_BIN(avcbrc_update,  nvl);
REGISTER_HUC_KERNEL_BIN(vp9dec,         nvl);
REGISTER_HUC_KERNEL_BIN(vp9hpu,         nvl);
REGISTER_HUC_KERNEL_BIN(hevcbrc_init,   nvl);
REGISTER_HUC_KERNEL_BIN(hevcbrc_update, nvl);
REGISTER_HUC_KERNEL_BIN(vp9brc_init,    nvl);
REGISTER_HUC_KERNEL_BIN(vp9brc_update,  nvl);
REGISTER_HUC_KERNEL_BIN(pakint,         nvl);
REGISTER_HUC_KERNEL_BIN(lookahead,      nvl);
REGISTER_HUC_KERNEL_BIN(av1ba,          nvl);
REGISTER_HUC_KERNEL_BIN(av1brc_init,    nvl);
REGISTER_HUC_KERNEL_BIN(av1brc_update,  nvl);
REGISTER_HUC_KERNEL_BIN(vvcs2l,         nvl);
REGISTER_HUC_KERNEL_BIN(avcbrc_pxp_init,   nvl);
REGISTER_HUC_KERNEL_BIN(avcbrc_pxp_update, nvl);
REGISTER_HUC_KERNEL_BIN(av1slbb_update,  nvl);
REGISTER_HUC_KERNEL_BIN(avcslbb_update,  nvl);
REGISTER_HUC_KERNEL_BIN(hevcslbb_update, nvl);

REGISTER_HUC_KERNEL_MFT(nvl);
#endif  // defined(MEDIA_BIN_DLL)

#endif  // defined(MEDIA_BIN_SUPPORT)

#endif  // __NVL_MEDIA_KERNELS_H__

