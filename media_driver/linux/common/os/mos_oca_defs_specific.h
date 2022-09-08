/*
* Copyright (c) 2022, Intel Corporation
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
//! \file     mos_oca_defs_specific.h
//! \brief    Linux specfic OCA struct
//!

#ifndef __MOS_OCA_DEFS_SPECIFIC_H__
#define __MOS_OCA_DEFS_SPECIFIC_H__

#include "mos_oca_defs.h"
#include <cstdint>

typedef struct _MOS_OCA_BUFFER_CONFIG
{
    uint32_t            maxResInfoCount = 0;                                                                //!< Max resource info count.
}MOS_OCA_BUFFER_CONFIG;

/****************************************************************************************************/
/*                                      OCA LOG HEADERS                                             */
/****************************************************************************************************/
typedef struct _MOS_OCA_RESOURCE_INFO
{
    uint64_t                    gfxAddress;
    uint64_t                    sizeAllocation;
    uint64_t                    sizeSurface;
    uint64_t                    sizeSurfacePhy;
    uint64_t                    sizeMainSurface;
    uint64_t                    allocationHandle;
    uint32_t                    hwCmdType;
    uint32_t                    locationInCmd;
    uint32_t                    offsetInRes;
    uint32_t                    pitch;
    uint32_t                    width;
    uint32_t                    height;
    uint32_t                    format;
    uint32_t                    gmmFormat;
    uint32_t                    gmmTileMode;
    uint32_t                    gmmClient;
    uint32_t                    gmmResUsageType;
    uint32_t                    mmcMode;
    uint32_t                    mmcHint;
    uint64_t                    auxYOffset;
    uint64_t                    auxUVOffset;
    uint64_t                    auxCCSOffset;
    uint64_t                    auxCCOffset;
    struct {
        uint32_t                isLocalOnly         : 1;
        uint32_t                isNonLocalOnly      : 1;
        uint32_t                isNotLockable       : 1;
        uint32_t                isShared            : 1;
        uint32_t                isCameraCapture     : 1;
        uint32_t                isRenderTarget      : 1;
    } flags;
}MOS_OCA_RESOURCE_INFO, *PMOS_OCA_RESOURCE_INFO;

struct MOS_OCA_BUF_CONTEXT
{
    bool                                inUse                 = false;
    bool                                is1stLevelBBStarted   = false;
    struct
    {
        uint64_t                        *base                 = nullptr;
        uint32_t                        offset                = 0;
        struct {
            uint32_t                    maxResInfoCount       = 0;
            MOS_OCA_RESOURCE_INFO       *resInfoList          = nullptr;
            uint32_t                    resCount              = 0;
            uint32_t                    resCountSkipped       = 0;
        } resInfo;   
    } logSection;
           
};

struct OCA_LOG_SECTION_HEADER
{
    uint64_t magicNum = 0;
    uint64_t rtlogPatchAddr = 0;
};

#pragma pack(push)
#pragma pack(8)
struct MOS_OCA_EXEC_LIST_INFO
{
    int             handle;
    uint64_t        size;
    uint64_t        offset64;
    int             flags;
    int             mem_region;
    bool            is_batch;
};
#pragma pack(pop)

typedef struct _MOS_OCA_LOG_HEADER_EXEC_LIST_INFO
{
    MOS_OCA_LOG_HEADER header;
    uint32_t           count;            // exec bo count dumped.
    uint32_t           reserved;         // reserved
} MOS_OCA_LOG_HEADER_EXEC_LIST_INFO, *PMOS_OCA_LOG_HEADER_EXEC_LIST_INFO;

#endif // #ifndef __MOS_OCA_DEFS_SPECIFIC_H__
