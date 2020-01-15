/*
* Copyright (c) 2009-2017, Intel Corporation
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
//! \file        hwinfo_linux.h 
//! \brief 
//!
#ifndef __HWINFO_LINUX_H__
#define __HWINFO_LINUX_H__

#include "media_libva_common.h"
#include "linux_shadow_skuwa.h"

//------------------------------------------------------------------------------
//| Definitions specific to Linux
//------------------------------------------------------------------------------
#define IS_BROXTON(device_id)    ( device_id == IBXT_A_DEVICE_F0_ID    || \
        device_id == IBXT_C_DEVICE_F0_ID    || \
        device_id == IBXT_X_DEVICE_F0_ID    || \
        device_id == IBXT_GT_3x6_DEVICE_ID  || \
        device_id == IBXT_PRO_3x6_DEVICE_ID || \
        device_id == IBXT_P_3x6_DEVICE_ID   || \
        device_id == IBXT_P_12EU_3x6_DEVICE_ID )
#define IS_ATOMSOC(device_id)       ( IS_BROXTON(device_id) )

extern MOS_STATUS HWInfo_GetGfxProductFamily(int32_t fd, PRODUCT_FAMILY &eProductFamily);

extern MOS_STATUS HWInfo_GetGfxInfo(int32_t    fd,
                          PLATFORM             *gfxPlatform,
                          MEDIA_FEATURE_TABLE  *skuTable,
                          MEDIA_WA_TABLE       *waTable,
                          MEDIA_SYSTEM_INFO    *gtSystemInfo);

struct LinuxDriverInfo;
extern MOS_STATUS HWInfoGetLinuxDrvInfo(int  fd, struct LinuxDriverInfo *drvInfo);

extern MOS_STATUS HWInfo_GetGmmInfo(int               fd,
                          SHADOW_MEDIA_FEATURE_TABLE  *shadowSkuTable,
                          SHADOW_MEDIA_WA_TABLE       *shadowWaTable,
                          MEDIA_SYSTEM_INFO           *systemInfo);

#endif

