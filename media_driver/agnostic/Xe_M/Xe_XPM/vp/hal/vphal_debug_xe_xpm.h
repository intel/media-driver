/*===================== begin_copyright_notice ==================================

# Copyright (c) 2020-2021, Intel Corporation

# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
# OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
# ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
# OTHER DEALINGS IN THE SOFTWARE.

======================= end_copyright_notice ==================================*/
//!
//! \file     vphal_debug_xe_xpm.h
//! \brief    Definition of extention structures and functions for debugging VPHAL
//! \details  This file contains the extension definition of structures and functions for
//!           surface dumper, hw state dumper, perf counter dumper, and render
//!           parameter dumper
//!
#ifndef __VPHAL_DEBUG_XE_XPM_H__
#define __VPHAL_DEBUG_XE_XPM_H__

#include "vphal_debug.h"
#include "vphal.h"
#include "media_blt_copy_xe_xpm_base.h"

#if (_DEBUG || _RELEASE_INTERNAL)
//==<Surface dump>==============================================================
#define VPHAL_DBG_SURF_DUMP_CREATE_XE_XPM()                                  \
    m_surfaceDumper = MOS_New(VphalSurfaceDumperXe_Xpm, m_pOsInterface);      \
    if (m_surfaceDumper)                                                        \
        m_surfaceDumper->GetSurfaceDumpSpec();

//!
//! Class VphalState
//! \brief VPHAL class definition
//!
class VphalSurfaceDumperXe_Xpm : public VphalSurfaceDumper
{
public:
    // allocate and VpHalDbg_GetSurfaceDumpSpec
    VphalSurfaceDumperXe_Xpm(PMOS_INTERFACE pOsInterface);

    // VpHalDbg_FreeSurfaceSpec
    virtual ~VphalSurfaceDumperXe_Xpm();

    // Get BltState
    BltStateXe_Xpm *GetBltState();

    virtual MOS_STATUS DumpSurfaceToFile(
        PMOS_INTERFACE              pOsInterface,
        PVPHAL_SURFACE              pSurface,
        const char*                 psPathPrefix,
        uint64_t                    iCounter,
        bool                        bLockSurface,
        bool                        bNoDecompWhenLock,
        uint8_t*                    pData);

#if !EMUL
    virtual MOS_STATUS DumpCompressedSurface(
        PMOS_INTERFACE pOsInterface,
        PVPHAL_SURFACE pSurface,
        const char *   psPathPrefix,
        uint64_t       iCounter,
        bool           bLockSurface);
#endif

protected:

    BltStateXe_Xpm                *pBltState;
};
#else
#define VPHAL_DBG_SURF_DUMP_CREATE_XE_XPM()
#endif
#endif // __VPHAL_DEBUG_XE_XPM_H__
