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
//! \file     mos_oca_defs.h
//! \brief    Common structure for OCA
//!

#ifndef __MOS_OCA_DEFS_H__
#define __MOS_OCA_DEFS_H__

typedef uint64_t MOS_OCA_BUFFER_HANDLE;
#define OCA_HEAP_INVALID_OFFSET ((uint32_t)-1)

#define MOS_OCA_INVALID_BUFFER_HANDLE -1

#define MAX_NUM_OF_OCA_BUF_CONTEXT   32

#define OCA_LOG_SECTION_SIZE_MAX        0x3000
#define OCA_LOG_SECTION_MAGIC_NUMBER    (0x5F691B7E574ACE30)

#define MOS_OCA_MAX_STRING_LEN          (1024)          //!< The max string len for MosOcaStateHeapLog::TraceMessage.
#define OCA_MAX_RESOURCE_INFO_COUNT_MAX 60

typedef enum _MOS_OCA_LOG_TYPE
{
    MOS_OCA_LOG_TYPE_INVALID = 0,
    MOS_OCA_LOG_TYPE_STRING,
    MOS_OCA_LOG_TYPE_VP_KERNEL_INFO,
    MOS_OCA_LOG_TYPE_VPHAL_PARAM,
    MOS_OCA_LOG_TYPE_CP_PARAM,
    MOS_OCA_LOG_TYPE_RESOURCE_INFO,
    MOS_OCA_LOG_TYPE_FENCE_INFO,
    MOS_OCA_LOG_TYPE_CODECHAL_PARAM,
    MOS_OCA_LOG_TYPE_EXEC_LIST_INFO,
    MOS_OCA_LOG_TYPE_VP_USER_FEATURE_CONTROL_INFO,
    MOS_OCA_LOG_TYPE_CP_IOMSG,
    MOS_OCA_LOG_TYPE_COUNT
}MOS_OCA_LOG_TYPE;

typedef struct _MOS_OCA_LOG_HEADER
{
    uint32_t    type;             //!< Oca log type. Refer to MOS_OCA_LOG_TYPE.
    uint32_t    headerSize;       //!< The size for extented message header.
    uint32_t    dataSize;         //!< The size of data block without message header.
}MOS_OCA_LOG_HEADER, *PMOS_OCA_LOG_HEADER;

typedef struct _MOS_OCA_LOG_HEADER_VPHAL_PARAM
{
    MOS_OCA_LOG_HEADER header;
    // Followed by VPHAL_OCA_RENDER_PARAM
} MOS_OCA_LOG_HEADER_VPHAL_PARAM, *PMOS_OCA_LOG_HEADER_VPHAL_PARAM;

typedef struct _MOS_OCA_LOG_HEADER_CODECHAL_PARAM
{
    MOS_OCA_LOG_HEADER header;
    uint32_t           codec;
    // Followed by CODECHAL_OCA_DECODE_HEADER
} MOS_OCA_LOG_HEADER_CODECHAL_PARAM, *PMOS_OCA_LOG_HEADER_CODECHAL_PARAM;

typedef struct _MOS_OCA_LOG_HEADER_RESOURCE_INFO
{
    MOS_OCA_LOG_HEADER header;
    uint32_t           resCount;         // Resource count dumped.
    uint32_t           resCountSkipped;  // Resource count skiped to be dumped as total count exceeding MOS_OCA_MAX_RESOURCE_INFO_COUNT.
    // Followed by MOS_OCA_RESOURCE_INFO lists.
} MOS_OCA_LOG_HEADER_RESOURCE_INFO, *PMOS_OCA_LOG_HEADER_RESOURCE_INFO;

typedef struct _MOS_OCA_LOG_HEADER_VP_KERNEL_INFO
{
    MOS_OCA_LOG_HEADER header;
    int                vpKernelID;
    int                fcKernelCount;
    // Followed by fc kernel list.
} MOS_OCA_LOG_HEADER_VP_KERNEL_INFO, *PMOS_OCA_LOG_HEADER_VP_KERNEL_INFO;

typedef struct _MOS_OCA_LOG_USER_FEATURE_CONTROL_INFO
{
    bool               PFonVpOutput = 0;
    bool               hwsEnabled   = 0;
} MOS_OCA_LOG_USER_FEATURE_CONTROL_INFO, *PMOS_OCA_LOG_USER_FEATURE_CONTROL_INFO;
#endif // #ifndef __MOS_OCA_DEFS_H__
