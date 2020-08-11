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
#ifndef __LINUX_MEDIA_SYS_INFO_H__
#define __LINUX_MEDIA_SYS_INFO_H__

#include <stdint.h>
#include <stdbool.h>
#include "linux_shadow_skuwa.h"

#ifndef ENABLE_KERNELS
#define SET_STATUS_BY_FULL_OPEN_SOURCE(sts_not_full_open_source, sts_if_full_open_source)   0
#elif defined(_FULL_OPEN_SOURCE)
#define SET_STATUS_BY_FULL_OPEN_SOURCE(sts_not_full_open_source, sts_if_full_open_source)   (sts_if_full_open_source)
#else
#define SET_STATUS_BY_FULL_OPEN_SOURCE(sts_not_full_open_source, sts_if_full_open_source)   (sts_not_full_open_source)
#endif

struct LinuxDriverInfo
{
    uint32_t euCount;
    uint32_t subSliceCount;
    uint32_t sliceCount; //Derived from subsliceCount
    uint32_t devId;
    uint32_t devRev;  //Revision ID of Graphics device
    uint32_t hasBsd   : 1;
    uint32_t hasBsd2  : 1;
    uint32_t hasVebox : 1;
    uint32_t hasBltRing : 1;
    uint32_t hasHuc   : 1;
    uint32_t hasPpgtt : 1;
    uint32_t hasPreemption : 1;
};

struct LinuxCodecInfo
{
    uint32_t avcDecoding    : 1;
    uint32_t mpeg2Decoding  : 1;
    uint32_t vp8Decoding    : 1;
    uint32_t vc1Decoding    : 1;
    uint32_t jpegDecoding   : 1;
    uint32_t avcEncoding    : 1;
    uint32_t mpeg2Encoding  : 1;
    uint32_t hevcDecoding   : 1;
    uint32_t hevcEncoding   : 1;
    uint32_t jpegEncoding   : 1;
    uint32_t avcVdenc       : 1;
    uint32_t vp9Decoding    : 1;
    uint32_t hevc10Decoding : 1;
    uint32_t vp9b10Decoding : 1;
    uint32_t hevc10Encoding : 1;
    uint32_t hevc12Encoding : 1;
    uint32_t vp8Encoding    : 1;
    uint32_t hevcVdenc      : 1;
    uint32_t vp9Vdenc       : 1;
    uint32_t adv0Decoding   : 1;
    uint32_t adv1Decoding   : 1;
    uint32_t vp9Encoding    : 1;
};

struct GfxDeviceInfo
{
    uint32_t  platformType;
    uint32_t  productFamily;
    uint32_t  displayFamily;
    uint32_t  renderFamily;
    uint32_t  eGTType;
    uint32_t  L3CacheSizeInKb;
    uint32_t  L3BankCount;
    uint32_t  EUCount;
    uint32_t  SliceCount;
    uint32_t  SubSliceCount;
    uint32_t  MaxEuPerSubSlice;
    uint32_t  isLCIA : 1;
    uint32_t  hasLLC  : 1;
    uint32_t  hasERAM : 1;
    bool (*InitMediaSysInfo)(struct GfxDeviceInfo *, MEDIA_GT_SYSTEM_INFO *);
    bool (*InitShadowSku)(struct GfxDeviceInfo *, SHADOW_MEDIA_FEATURE_TABLE *, struct LinuxDriverInfo *);
    bool (*InitShadowWa)(struct GfxDeviceInfo *, SHADOW_MEDIA_WA_TABLE *, struct LinuxDriverInfo *);
};

class MediaFeatureTable;
class MediaWaTable;

struct LinuxDeviceInit
{
    uint32_t productFamily;
    bool (*InitMediaFeature)(struct GfxDeviceInfo *, MediaFeatureTable *, struct LinuxDriverInfo *);
    bool (*InitMediaWa)(struct GfxDeviceInfo *, MediaWaTable *, struct LinuxDriverInfo *);
};

#define MEDIA_EXT_FLAG  0x10000000

#endif //__LINUX_MEDIA_SYS_INFO_H__
