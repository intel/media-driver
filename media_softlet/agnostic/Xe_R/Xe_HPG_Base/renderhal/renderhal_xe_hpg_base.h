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

extern const uint32_t g_cLookup_RotationMode_hpg_base[8];

struct MHW_VFE_PARAMS_XE_HPG : MHW_VFE_PARAMS
{
    bool  bFusedEuDispatch = 0;
    uint32_t numOfWalkers = 0;
    bool  enableSingleSliceDispatchCcsMode = 0;

    // Surface state offset of scratch space buffer.
    uint32_t scratchStateOffset = 0;
};

//! \brief      for Xe_HPM VP and MDF
//!              SLM     URB     DC      RO      Rest/L3 Client Pool
//!               0    96(fixed) 0       0       320 (KB chunks based on GT2)
#define RENDERHAL_L3_CACHE_CONFIG_CNTLREG_VALUE_XE_HPG_BASE_RENDERHAL (0x00000200)

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
    //! \brief    Convert To Nano Seconds
    //! \details  Convert to Nano Seconds
    //! \param    PRENDERHAL_INTERFACE pRenderHal
    //!           [in] Pointer to Hardware Interface Structure
    //! \param    uint64_t iTicks
    //!           [in] Ticks
    //! \param    uint64_t *piNs
    //!           [in] Nano Seconds
    //! \return   void
    //!
    void ConvertToNanoSeconds(
        PRENDERHAL_INTERFACE pRenderHal,
        uint64_t             iTicks,
        uint64_t *           piNs) override;

    //!
    //! \brief    Initialize the State Heap Settings per platform
    //! \param    PRENDERHAL_INTERFACE    pRenderHal
    //!           [out] Pointer to PRENDERHAL_INTERFACE
    //! \return   void
    //!
    void InitStateHeapSettings(
        PRENDERHAL_INTERFACE pRenderHal) override;

    //!
    //! \brief    Enables L3 cacheing flag and sets related registers/values
    //! \param    PRENDERHAL_INTERFACE    pRenderHal
    //!           [in]  Pointer to Hardware Interface
    //! \param    pCacheSettings
    //!           [in] L3 Cache Configurations
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS EnableL3Caching(
        PRENDERHAL_INTERFACE         pRenderHal,
        PRENDERHAL_L3_CACHE_SETTINGS pCacheSettings) override;

    //!
    //! \brief      Get the pointer to the MHW_VFE_PARAMS
    //! \return     MHW_VFE_PARAMS*
    //!             pointer to the MHW_VFE_PARAMS
    //!
    virtual MHW_VFE_PARAMS *GetVfeStateParameters() override { return &m_vfeStateParams; }

    //!
    //! \brief      enable/disable the fusedEUDispatch flag in the VFE_PARAMS
    //! \return     no return value
    //!
    virtual void SetFusedEUDispatch(bool enable) override;

    //!
    //! \brief      set the number of walkers in the VFE_PARAMS
    //! \return     MOS_STATUS_SUCCESS
    //!
    MOS_STATUS SetNumOfWalkers(uint32_t numOfWalkers);

    //!
    //! \brief      enable/disable the single slice dispatch flag in the VFE_PARAMS
    //! \return     no return value
    //!
    void SetSingleSliceDispatchCcsMode(bool enable)
    {
        m_vfeStateParams.enableSingleSliceDispatchCcsMode = enable;
    };

    //! \brief      Set L3 cache override config parameters
    //! \param      [in] pRenderHal
    //!             Pointer to RenderHal Interface Structure
    //! \param      [in,out] pCacheSettings
    //!             Pointer to pCacheSettings
    //! \param      [in] bEnableSLM
    //!             Flag to enable SLM
    //! \return     MOS_STATUS
    //!             MOS_STATUS_SUCCESS if success. Error code otherwise
    //!
    virtual MOS_STATUS SetCacheOverrideParams(
        PRENDERHAL_INTERFACE         pRenderHal,
        PRENDERHAL_L3_CACHE_SETTINGS pCacheSettings,
        bool                         bEnableSLM) override;

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

    //! \brief    Sets states of scratch space buffer.
    //! \param    Pointer to RENDERHAL_INTERFACE renderHal
    //!           [in] Pointer to render HAL Interface.
    //! \param    indexOfBindingTable
    //!           [in] Index of the binding table in use.
    //! \return   MOS_STATUS
    MOS_STATUS SetScratchSpaceBufferState(
        RENDERHAL_INTERFACE *renderHal,
        uint32_t             indexOfBindingTable);

    MHW_SETPAR_DECL_HDR(CFE_STATE);

protected:
    XRenderHal_Interface_Xe_Hpg_Base();
    virtual ~XRenderHal_Interface_Xe_Hpg_Base() {}

    MHW_VFE_PARAMS_XE_HPG m_vfeStateParams;

    mhw::render::xe_hpg::Cmd::PALETTE_ENTRY_CMD
        m_paletteData[RENDERHAL_PALETTE_MAX][RENDERHAL_PALETTE_ENTRIES_MAX];

MEDIA_CLASS_DEFINE_END(XRenderHal_Interface_Xe_Hpg_Base)
};

#endif  // __RENDERHAL_XE_HPG_BASE_H__
