/*
* Copyright (c) 2017, Intel Corporation
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
//! \file     media_skuwa_specific.h
//! \brief    Defines the media_specific skuwa for Linux
//!
#ifndef __MEDIA_SKUWA_H__
#define __MEDIA_SKUWA_H__

#include <stdint.h>

#include "linux_media_skuwa.h"
#include "linux_shadow_skuwa.h"

using MEDIA_FEATURE_TABLE = MediaFeatureTable;
using MEDIA_WA_TABLE      = MediaWaTable;
using MEDIA_SYSTEM_INFO   = MEDIA_GT_SYSTEM_INFO;
using MEDIA_ENGINE_INFO   = MEDIA_GT_SYSTEM_INFO;

/* the below is for GMM */
using GMM_SKU_FEATURE_TABLE   = SHADOW_MEDIA_FEATURE_TABLE;
using GMM_WA_TABLE            = SHADOW_MEDIA_WA_TABLE;
using GMM_GT_SYSTEM_INFO      = MEDIA_GT_SYSTEM_INFO;
using GMM_ADAPTER_BDF         = MEDIA_ADAPTER_BDF;

#endif // __MEDIA_SKUWA_H__
