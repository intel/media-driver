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
//! \file     nvlMediaKernels.cpp
//! \brief    Implementation of the huc kernel source binary
//!
#include "nvlMediaKernels.h"

#if defined(MEDIA_BIN_SUPPORT) && !defined(MEDIA_BIN_DLL)

#define DEFINE_HUC_KERNEL_BIN(hucKernelShortName, platform)                          \
    unsigned int  __MediaKernels_##hucKernelShortName##_##platform##_size = 0;       \
    unsigned int *__MediaKernels_##hucKernelShortName##_##platform        = nullptr; \
    static bool get##hucKernelShortName = LoadMediaBin(                              \
         __MediaKernels_##hucKernelShortName##_##platform##_NAME,                    \
        &__MediaKernels_##hucKernelShortName##_##platform##_size,                    \
        &__MediaKernels_##hucKernelShortName##_##platform);

#define DEFINE_HUC_KERNEL_MFT(platform)                                  \
    unsigned int  __MediaKernels_manifest##_##platform##_size = 0;       \
    unsigned int *__MediaKernels_manifest##_##platform        = nullptr; \
    static bool getManifest##platform = LoadMediaBin(                    \
         __MediaKernels_manifest##_##platform##_NAME,                    \
        &__MediaKernels_manifest##_##platform##_size,                    \
        &__MediaKernels_manifest##_##platform);

DEFINE_HUC_KERNEL_BIN(s2l,            nvl);
DEFINE_HUC_KERNEL_BIN(drm,            nvl);
DEFINE_HUC_KERNEL_BIN(copykrn,        nvl);
DEFINE_HUC_KERNEL_BIN(avcbrc_init,    nvl);
DEFINE_HUC_KERNEL_BIN(avcbrc_update,  nvl);
DEFINE_HUC_KERNEL_BIN(vp9dec,         nvl);
DEFINE_HUC_KERNEL_BIN(vp9hpu,         nvl);
DEFINE_HUC_KERNEL_BIN(hevcbrc_init,   nvl);
DEFINE_HUC_KERNEL_BIN(hevcbrc_update, nvl);
DEFINE_HUC_KERNEL_BIN(vp9brc_init,    nvl);
DEFINE_HUC_KERNEL_BIN(vp9brc_update,  nvl);
DEFINE_HUC_KERNEL_BIN(pakint,         nvl);
DEFINE_HUC_KERNEL_BIN(lookahead,      nvl);
DEFINE_HUC_KERNEL_BIN(av1ba,          nvl);
DEFINE_HUC_KERNEL_BIN(av1brc_init,    nvl);
DEFINE_HUC_KERNEL_BIN(av1brc_update,  nvl);
DEFINE_HUC_KERNEL_BIN(vvcs2l,         nvl);
DEFINE_HUC_KERNEL_BIN(avcbrc_pxp_init,nvl);
DEFINE_HUC_KERNEL_BIN(avcbrc_pxp_update, nvl);
DEFINE_HUC_KERNEL_BIN(av1slbb_update,  nvl);
DEFINE_HUC_KERNEL_BIN(avcslbb_update,  nvl);
DEFINE_HUC_KERNEL_BIN(hevcslbb_update, nvl);

DEFINE_HUC_KERNEL_MFT(nvl);

#endif  // defined(MEDIA_BIN_SUPPORT) && !defined(MEDIA_BIN_DLL)

#if !defined(MEDIA_BIN_SUPPORT) || defined(MEDIA_BIN_DLL)

#include "nvlMediaKernel_av1ba.h"
#include "nvlMediaKernel_av1brc_init.h"
#include "nvlMediaKernel_av1brc_update.h"
#include "nvlMediaKernel_avcbrc_init.h"
#include "nvlMediaKernel_avcbrc_update.h"
#include "nvlMediaKernel_copykrn.h"
#include "nvlMediaKernel_hevcbrc_init.h"
#include "nvlMediaKernel_s2l.h"
#include "nvlMediaKernel_hevcbrc_update.h"
#include "nvlMediaKernel_lookahead.h"
#include "nvlMediaKernel_pakint.h"
#include "nvlMediaKernel_vp9brc_init.h"
#include "nvlMediaKernel_vp9brc_update.h"
#include "nvlMediaKernel_vp9hpu.h"
#include "nvlMediaKernel_vp9dec.h"
#include "nvlMediaKernel_vvcs2l.h"
#include "nvlMediaKernel_av1slbb_update.h"
#include "nvlMediaKernel_avcslbb_update.h"
#include "nvlMediaKernel_hevcslbb_update.h"
#ifdef _MEDIA_RESERVED
#include "nvlMediaKernel_drm.h"
#include "nvlMediaKernel_avcbrc_pxp_init.h"
#include "nvlMediaKernel_avcbrc_pxp_update.h"
#include "nvlMediaKernel_manifest.h"
#endif

#define DEFINE_HUC_KERNEL_BIN_SIZE(hucKernelShortName, platform)   \
    DEFINE_SHARED_ARRAY_SIZE_UINT32(                               \
        __MediaKernels_##hucKernelShortName##_##platform##_size,   \
        sizeof(__MediaKernels_##hucKernelShortName##_##platform));

#define DEFINE_HUC_KERNEL_MFT_SIZE(platform)           \
    DEFINE_SHARED_ARRAY_SIZE_UINT32(                   \
        __MediaKernels_manifest##_##platform##_size,   \
        sizeof(__MediaKernels_manifest##_##platform));

DEFINE_HUC_KERNEL_BIN_SIZE(s2l,            nvl);
DEFINE_HUC_KERNEL_BIN_SIZE(copykrn,        nvl);
DEFINE_HUC_KERNEL_BIN_SIZE(avcbrc_init,    nvl);
DEFINE_HUC_KERNEL_BIN_SIZE(avcbrc_update,  nvl);
DEFINE_HUC_KERNEL_BIN_SIZE(vp9dec,         nvl);
DEFINE_HUC_KERNEL_BIN_SIZE(vp9hpu,         nvl);
DEFINE_HUC_KERNEL_BIN_SIZE(hevcbrc_init,   nvl);
DEFINE_HUC_KERNEL_BIN_SIZE(hevcbrc_update, nvl);
DEFINE_HUC_KERNEL_BIN_SIZE(vp9brc_init,    nvl);
DEFINE_HUC_KERNEL_BIN_SIZE(vp9brc_update,  nvl);
DEFINE_HUC_KERNEL_BIN_SIZE(pakint,         nvl);
DEFINE_HUC_KERNEL_BIN_SIZE(lookahead,      nvl);
DEFINE_HUC_KERNEL_BIN_SIZE(av1ba,          nvl);
DEFINE_HUC_KERNEL_BIN_SIZE(av1brc_init,    nvl);
DEFINE_HUC_KERNEL_BIN_SIZE(av1brc_update,  nvl);
DEFINE_HUC_KERNEL_BIN_SIZE(vvcs2l,         nvl);
DEFINE_HUC_KERNEL_BIN_SIZE(av1slbb_update,  nvl);
DEFINE_HUC_KERNEL_BIN_SIZE(avcslbb_update,  nvl);
DEFINE_HUC_KERNEL_BIN_SIZE(hevcslbb_update, nvl);
#ifdef _MEDIA_RESERVED
DEFINE_HUC_KERNEL_BIN_SIZE(drm,               nvl);
DEFINE_HUC_KERNEL_BIN_SIZE(avcbrc_pxp_init,   nvl);
DEFINE_HUC_KERNEL_BIN_SIZE(avcbrc_pxp_update, nvl);
DEFINE_HUC_KERNEL_MFT_SIZE(nvl);
#endif

#endif // !defined(MEDIA_BIN_SUPPORT) || defined(MEDIA_BIN_DLL)
