/*
* Copyright (c) 2019, Intel Corporation
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
* FITNESS FOR A PARTICULAR PURPOSNextE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
* OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
* ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
* OTHER DEALINGS IN THE SOFTWARE.
*/
//!
//! \file     mos_os_next.h
//! \brief    Common interface and structure used in MOSOSNext
//!

#ifndef __MOS_OS_NEXT_H__
#define __MOS_OS_NEXT_H__

#include "mos_os.h"

#if (_RELEASE_INTERNAL || _DEBUG)
#if defined(CM_DIRECT_GUC_SUPPORT)
#include "work_queue_mngr.h"
#include "KmGucClientInterface.h"
#endif
#endif

//!
//! \brief OSNextspecific includes and definitions
//!

#if MOS_COMMAND_RESINFO_DUMP_SUPPORTED
//!
//! \brief Class to Dump GPU Command Info
//!
class GpuCmdResInfoDumpNext
{
public:

    static const GpuCmdResInfoDumpNext *GetInstance();
    GpuCmdResInfoDumpNext();

    void Dump(PMOS_INTERFACE pOstInterface) const;

    void StoreCmdResPtr(PMOS_INTERFACE pOstInterface, const void *pRes) const;

    void ClearCmdResPtrs(PMOS_INTERFACE pOstInterface) const;

private:

    struct GpuCmdResInfo;

    void Dump(const void *pRes, std::ofstream &outputFile) const;

    const std::vector<const void *> &GetCmdResPtrs(PMOS_INTERFACE pOstInterface) const;

    const char *GetResType(MOS_GFXRES_TYPE resType) const;

    const char *GetTileType(MOS_TILE_TYPE tileType) const;

private:

    static std::shared_ptr<GpuCmdResInfoDumpNext> m_instance;
    mutable uint32_t         m_cnt         = 0;
    bool                     m_dumpEnabled = false;
    std::string              m_path;
};
#endif // MOS_COMMAND_RESINFO_DUMP_SUPPORTED
#endif  // __MOS_OSNext_NEXT_H__
