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
//! \file     vphal_debug_xe_xpm.c
//! \details  This file contains the Implementation of extention functions for
//!           surface dumper, hw state dumper, perf counter dumper, and render
//!           parameter dumper
//!
#include "vphal_debug_xe_xpm.h"
#include "vp_hal_ddi_utils.h"

#if (_DEBUG || _RELEASE_INTERNAL)

//==<Dump Surface>==============================================================

VphalSurfaceDumperXe_Xpm::VphalSurfaceDumperXe_Xpm(PMOS_INTERFACE pOsInterface):
    VphalSurfaceDumper(pOsInterface)
{
#if !EMUL
    pBltState = MOS_New(BltStateXe_Xpm, m_osInterface);
    if (pBltState && pBltState->m_mhwInterfaces == nullptr)
    {
        VPHAL_DEBUG_ASSERTMESSAGE("MhwInterfaces create factory failed!");
        MOS_Delete(pBltState);
        pBltState = nullptr;
    }
#endif
}

VphalSurfaceDumperXe_Xpm::~VphalSurfaceDumperXe_Xpm()
{
#if !EMUL
    if (pBltState)
    {
        MOS_Delete(pBltState);
        pBltState = nullptr;
    }
#endif
}

BltStateXe_Xpm *VphalSurfaceDumperXe_Xpm::GetBltState()
{
    return pBltState;
}

MOS_STATUS VphalSurfaceDumperXe_Xpm::DumpSurfaceToFile(
    PMOS_INTERFACE          pOsInterface,
    PVPHAL_SURFACE          pSurface,
    const char*             psPathPrefix,
    uint64_t                iCounter,
    bool                    bLockSurface,
    bool                    bNoDecompWhenLock,
    uint8_t*                pData)
{
#if !EMUL
    if (MEDIA_IS_SKU(m_osInterface->pfnGetSkuTable(m_osInterface), FtrFlatPhysCCS) && m_dumpSpec.enableAuxDump)
    {
        return DumpCompressedSurface(
            m_osInterface,
            pSurface,
            m_dumpPrefix,
            iCounter,
            true);
    }
    else
#endif
    {
        return VphalSurfaceDumper::DumpSurfaceToFile(
            m_osInterface,
            pSurface,
            m_dumpPrefix,
            iCounter,
            true,
            bNoDecompWhenLock,
            pData);
    }
}

#if !EMUL
MOS_STATUS VphalSurfaceDumperXe_Xpm::DumpCompressedSurface(
    PMOS_INTERFACE pOsInterface,
    PVPHAL_SURFACE pSurface,
    const char *   psPathPrefix,
    uint64_t       iCounter,
    bool           bLockSurface)
{
    MOS_STATUS eStatus;
    char       sPath[MAX_PATH], sOsPath[MAX_PATH];

    VPHAL_DEBUG_ASSERT(pSurface);
    VPHAL_DEBUG_ASSERT(pOsInterface);
    VPHAL_DEBUG_ASSERT(psPathPrefix);

    eStatus = MOS_STATUS_SUCCESS;
    MOS_ZeroMemory(sPath, MAX_PATH);
    MOS_ZeroMemory(sOsPath, MAX_PATH);

    MOS_SURFACE surface = VpHalDDIUtils::ConvertVphalSurfaceToMosSurface(pSurface);
    VPHAL_DEBUG_CHK_STATUS(pBltState->LockSurface(&surface));

    // dump main surface
    MOS_SecureStringPrint(
        sPath,
        MAX_PATH,
        sizeof(sPath),
        "%s_f[%03lld]_w[%d]_h[%d]_p[%d].%s",
        psPathPrefix,
        iCounter,
        pSurface->dwWidth,
        pSurface->dwHeight,
        pSurface->dwPitch,
        VphalDumperTool::GetFormatStr(pSurface->Format));
    VphalDumperTool::GetOsFilePath(sPath, sOsPath);
    VPHAL_DEBUG_CHK_NULL(pBltState->GetMainSurfaceData())
    VPHAL_DEBUG_CHK_STATUS(MosUtilities::MosWriteFileFromPtr(sOsPath, pBltState->GetMainSurfaceData(), pBltState->GetMainSurfaceSize()));

    MOS_ZeroMemory(sPath, MAX_PATH);
    MOS_ZeroMemory(sOsPath, MAX_PATH);
    // dump aux
    MOS_SecureStringPrint(
        sPath,
        MAX_PATH,
        sizeof(sPath),
        "%s_f[%03lld]_w[%d]_h[%d]_p[%d].aux",
        psPathPrefix,
        iCounter,
        pBltState->GetAuxSize(),
        1,
        pBltState->GetAuxSize());
    VphalDumperTool::GetOsFilePath(sPath, sOsPath);
    VPHAL_DEBUG_CHK_NULL(pBltState->GetAuxData());
    VPHAL_DEBUG_CHK_STATUS(MosUtilities::MosWriteFileFromPtr(sOsPath, pBltState->GetAuxData(), pBltState->GetAuxSize()));

finish:
    pBltState->UnLockSurface();
    return eStatus;
}
#endif
#endif // (_DEBUG || _RELEASE_INTERNAL)
