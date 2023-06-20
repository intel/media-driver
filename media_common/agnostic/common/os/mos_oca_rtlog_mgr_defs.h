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
//! \file     mos_oca_rtlog_mgr_defs.h
//! \brief    Linux specific OCA struct
//!

#ifndef __MOS_OCA_RTLOG_MGR_DEFS_H__
#define __MOS_OCA_RTLOG_MGR_DEFS_H__

#include <cstdint>

enum MOS_OCA_RTLOG_COMPONENT_TPYE
{
    MOS_OCA_RTLOG_COMPONENT_DECODE = 0,
    MOS_OCA_RTLOG_COMPONENT_VP,
    MOS_OCA_RTLOG_COMPONENT_ENCODE,
    MOS_OCA_RTLOG_COMPONENT_COMMON,
    MOS_OCA_RTLOG_COMPONENT_MAX
};

#define MOS_OCA_RTLOG_MAGIC_NUM 0x494D5445
#define MAX_OCA_RT_SUB_SIZE 0x100
#define MAX_OCA_RT_COMMON_SUB_SIZE 0x3D00
#define MAX_OCA_RT_SIZE (MAX_OCA_RT_SUB_SIZE * (MOS_OCA_RTLOG_COMPONENT_MAX-1) + MAX_OCA_RT_COMMON_SUB_SIZE)
#define MAX_OCA_RT_POOL_SIZE (MAX_OCA_RT_SIZE + MOS_PAGE_SIZE)
#define MOS_OCA_RTLOG_MAX_PARAM_COUNT 1
// sizeof(int32_t)+sizeof(int64_t) is the size of MT_PARAM
#define MOS_OCA_RTLOG_ENTRY_SIZE (MOS_OCA_RTLOG_MAX_PARAM_COUNT*(sizeof(int32_t)+sizeof(int64_t))+sizeof(MOS_OCA_RTLOG_HEADER))


struct MOS_OCA_RTLOG_HEAP
{
    void                   *ocaHeapCpuVa = nullptr;
    uint64_t                ocaHeapGpuVa  = 0;
    uint32_t                offset        = 0;
    uint32_t                size          = 0;
};

struct MOS_OCA_RTLOG_HEADER
{
    uint64_t  globalId   = 0;
    uint32_t  id         = 0;
    uint32_t  paramCount = 0;
};

struct MOS_OCA_RTLOG_SECTION_HEADER
{
    uint32_t magicNum = 0;
    MOS_OCA_RTLOG_COMPONENT_TPYE componentType = MOS_OCA_RTLOG_COMPONENT_MAX;
    uint64_t  freq = 0;
};

#endif // #ifndef __MOS_OCA_RTLOG_MGR_DEFS_H__