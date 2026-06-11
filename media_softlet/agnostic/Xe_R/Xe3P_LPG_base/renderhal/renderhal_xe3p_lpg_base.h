/*===================== begin_copyright_notice ==================================

# Copyright (c) 2023, Intel Corporation

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
//! \file       renderhal_xe3p_lpg_base.h
//! \brief      header file of xe3p_lpg_base hardware functions
//! \details    Xe3P_LPG_BASE hardware functions declare
//!
#ifndef __RENDERHAL_XE3P_LPG_BASE_H__
#define __RENDERHAL_XE3P_LPG_BASE_H__

#include "renderhal_platform_interface_next.h"

struct MHW_VFE_PARAMS_XE3P_LPG_BASE : MHW_VFE_PARAMS
{
    bool  bFusedEuDispatch                  = 0;
    uint32_t numOfWalkers                   = 0;
    bool  enableSingleSliceDispatchCcsMode  = 0;
    uint32_t scratchStateOffset             = 0;            //!< Surface state offset of scratch space buffer.
};

class XRenderHal_Interface_Xe3P_Lpg_Base : public XRenderHal_Platform_Interface_Next
{
public:
    //!
    //! \brief    Setup Surface State
    //! \details  Setup Surface States for Xe3P_LPG_Base
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
    //! \brief    Send To 3DState Binding Table Pool Alloc
    //! \details  Send To 3DState Binding Table Pool Alloc
    //! \param    PRENDERHAL_INTERFACE pRenderHal
    //!           [in] Pointer to RenderHal Interface Structure
    //! \param    PMOS_COMMAND_BUFFER pCmdBuffer
    //!           [in] Pointer to Command Buffer
    //! \return   MOS_STATUS
    //!
    MOS_STATUS SendTo3DStateBindingTablePoolAlloc(
        PRENDERHAL_INTERFACE pRenderHal,
        PMOS_COMMAND_BUFFER  pCmdBuffer) override;

    virtual MOS_STATUS SendStateComputeMode(
        PRENDERHAL_INTERFACE pRenderHal,
        PMOS_COMMAND_BUFFER  pCmdBuffer) override;

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
    MOS_STATUS EnableL3Caching(
        PRENDERHAL_INTERFACE         pRenderHal,
        PRENDERHAL_L3_CACHE_SETTINGS pCacheSettings) override;

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
    MOS_STATUS SetCacheOverrideParams(
        PRENDERHAL_INTERFACE         pRenderHal,
        PRENDERHAL_L3_CACHE_SETTINGS pCacheSettings,
        bool                         bEnableSLM) override;

    //!
    //! \brief      Get the pointer to the MHW_VFE_PARAMS
    //! \return     MHW_VFE_PARAMS*
    //!             pointer to the MHW_VFE_PARAMS
    //!
    MHW_VFE_PARAMS *GetVfeStateParameters() override { return &m_vfeStateParams; }

    //!
    //! \brief      enable/disable the fusedEUDispatch flag in the VFE_PARAMS
    //! \return     no return value
    //!
    void SetFusedEUDispatch(bool enable) override;

    //!
    //! \brief    Sets states of scratch space buffer.
    //! \param    [in] renderHal
    //!           Pointer to Hardware Interface
    //! \param    [in] indexOfBindingTable
    //!           Index of the binding table in use
    //! \return   MOS_STATUS
    //!
    MOS_STATUS SetScratchSpaceBufferState(
        RENDERHAL_INTERFACE     *renderHal,
        uint32_t                indexOfBindingTable);

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
    }

    bool IsL8FormatSupported() override
    {
        return false;
    }

    bool IsBindlessHeapInUse(
        PRENDERHAL_INTERFACE pRenderHal) override
    {
        return true;
    }

    virtual uint32_t GetGrfSize() override
    {
        return 64;
    }

    MHW_SETPAR_DECL_HDR(COMPUTE_WALKER);
    MHW_SETPAR_DECL_HDR(CFE_STATE);

protected:
    XRenderHal_Interface_Xe3P_Lpg_Base() {}
    virtual ~XRenderHal_Interface_Xe3P_Lpg_Base() {}

    MHW_VFE_PARAMS_XE3P_LPG_BASE m_vfeStateParams;

MEDIA_CLASS_DEFINE_END(XRenderHal_Interface_Xe3P_Lpg_Base)
};

#endif  // __RENDERHAL_XE3P_LPG_BASE_H__
