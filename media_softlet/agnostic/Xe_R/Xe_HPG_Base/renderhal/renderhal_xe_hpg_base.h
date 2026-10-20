/*===================== begin_copyright_notice ==================================

# Copyright (c) 2021-2023, Intel Corporation

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
//! \file       renderhal_xe_hpg_base.h
//! \brief      header file of xe_hpg hardware functions
//! \details    Xe_HPM hardware functions declare
//!
#ifndef __RENDERHAL_XE_HPG_BASE_H__
#define __RENDERHAL_XE_HPG_BASE_H__

#include "renderhal_platform_interface_next.h"
#include "mhw_render_hwcmd_xe_hpg.h"
#include "mhw_state_heap_xe_hpg.h"

// MHW_VFE_PARAMS_NEXT, the L3 CNTLREG value, the nano second per tick constant and
// g_cLookup_RotationMode_Next now live in renderhal_platform_interface_next.h,
// shared with the other platforms deriving from XRenderHal_Platform_Interface_Next.

class XRenderHal_Interface_Xe_Hpg_Base : public XRenderHal_Platform_Interface_Next
{
public:
    //!
    //! \brief    Setup Surface State
    //! \details  Setup Surface States for Xe_HPM
    //! \param    PRENDERHAL_INTERFACE pRenderHal
    //!           [in] Pointer to Hardware Interface Structure
    //! \param    PRENDERHAL_SURFACE pRenderHalSurface
    //!           [in] Pointer to Render Hal Surface
    //! \param    PRENDERHAL_SURFACE_STATE_PARAMS pParams
    //!           [in] Pointer to Surface State Params
    //! \param    int32_t *piNumEntries
    //!           [out] Pointer to Number of Surface State Entries (Num Planes)
    //! \param    PRENDERHAL_SURFACE_STATE_ENTRY * ppSurfaceEntries
    //!           [out] Array of Surface State Entries
    //! \param    PRENDERHAL_OFFSET_OVERRIDE pOffsetOverride
    //!           [in] If not nullptr, provides adjustments to Y, UV plane offsets,
    //!           used for kernel WA in a few cases. nullptr is the most common usage.
    //! \return   MOS_STATUS
    //!
    MOS_STATUS SetupSurfaceState(
        PRENDERHAL_INTERFACE            pRenderHal,
        PRENDERHAL_SURFACE              pRenderHalSurface,
        PRENDERHAL_SURFACE_STATE_PARAMS pParams,
        int32_t *                       piNumEntries,
        PRENDERHAL_SURFACE_STATE_ENTRY *ppSurfaceEntries,
        PRENDERHAL_OFFSET_OVERRIDE      pOffsetOverride) override;

    //!
    //! \brief    Initialize the State Heap Settings per platform
    //! \param    PRENDERHAL_INTERFACE    pRenderHal
    //!           [out] Pointer to PRENDERHAL_INTERFACE
    //! \return   void
    //!
    void InitStateHeapSettings(
        PRENDERHAL_INTERFACE pRenderHal) override;

    //! \brief      Get the size of Render Surface State Command
    //! \return     size_t
    //!             the size of render surface state command
    virtual size_t GetSurfaceStateCmdSize() override;

    //! \brief      Get the address of the ith Palette Data
    //! \param      [in] i
    //!             Index of the palette data
    //! \return     void *
    //!             address of the ith palette data table
    virtual void *GetPaletteDataAddress(int i) override { return &m_paletteData[i]; }

    //! \brief      Get the size of Binding Table State Command
    //! \return     size_t
    //!             the size of binding table state command
    virtual size_t GetBTStateCmdSize() override { return mhw_state_heap_xe_hpg::BINDING_TABLE_STATE_CMD::byteSize; }

    virtual uint32_t GetGrfSize() override
    {
        return 32;
    }

    MHW_SETPAR_DECL_HDR(CFE_STATE);

protected:
    XRenderHal_Interface_Xe_Hpg_Base();
    virtual ~XRenderHal_Interface_Xe_Hpg_Base() {}

    mhw::render::xe_hpg::Cmd::PALETTE_ENTRY_CMD
        m_paletteData[RENDERHAL_PALETTE_MAX][RENDERHAL_PALETTE_ENTRIES_MAX];

MEDIA_CLASS_DEFINE_END(XRenderHal_Interface_Xe_Hpg_Base)
};

#endif  // __RENDERHAL_XE_HPG_BASE_H__
