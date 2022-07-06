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
//! \file       renderhal_xe_hp_base.h
//! \brief      header file of Xe_XPM+ hardware functions
//! \details    Xe_XPM+ hardware functions declare
//!
#ifndef __RENDERHAL_XE_HP_BASE_H__
#define __RENDERHAL_XE_HP_BASE_H__

#include <stdint.h>
#include "media_class_trace.h"
#include "mhw_render.h"
#include "mos_defs.h"
#include "mos_os_specific.h"
#include "renderhal.h"
#include "renderhal_g12_base.h"

//! \brief      for XeXPM+ VP and MDF
//!              SLM     URB     DC      RO      Rest/L3 Client Pool
//!               0    96(fixed) 0       0       320 (KB chunks based on GT2)
#define RENDERHAL_L3_CACHE_CONFIG_CNTLREG_VALUE_G12HP_RENDERHAL (0x00000200)

class XRenderHal_Interface_Xe_Hp_Base : public XRenderHal_Interface_G12_Base
{
public:
    XRenderHal_Interface_Xe_Hp_Base() : XRenderHal_Interface_G12_Base()
    {

    }
    
    virtual ~XRenderHal_Interface_Xe_Hp_Base() {}

    //!
    //! \brief    Setup Surface State
    //! \details  Setup Surface States for Xe_XPM+
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
    virtual MOS_STATUS SetupSurfaceState(
        PRENDERHAL_INTERFACE            pRenderHal,
        PRENDERHAL_SURFACE              pRenderHalSurface,
        PRENDERHAL_SURFACE_STATE_PARAMS pParams,
        int32_t                         *piNumEntries,
        PRENDERHAL_SURFACE_STATE_ENTRY  *ppSurfaceEntries,
        PRENDERHAL_OFFSET_OVERRIDE      pOffsetOverride);

    //!
    //! \brief      Checks how per thread scratch space size bits in VFE state are interpreted by HW.
    //! \details    On XeHP, per thread scratch space size can be 2^n (n >= 6) bytes.
    //! \param      PRENDERHAL_INTERFACE pRenderHal
    //!             [in]    Pointer to RenderHal interface
    //! \return     true on XeHP.
    //!
    virtual bool PerThreadScratchSpaceStart64Byte(
        RENDERHAL_INTERFACE *renderHal)
    {
        return true;
    }

    //! \brief    Check if compute context in use
    //! \param    PRENDERHAL_INTERFACE    pRenderHal
    //!           [in]  Pointer to Hardware Interface
    //! \return   true of false
    virtual bool IsComputeContextInUse(PRENDERHAL_INTERFACE pRenderHal)
    {
        return true;
    }

    //!
    //! \brief    Get Render Engine MMC Enable/Disable Flag
    //! \param    [in] pRenderHal
    //!           Pointer to Hardware Interface
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS IsRenderHalMMCEnabled(
        PRENDERHAL_INTERFACE         pRenderHal);

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
        PRENDERHAL_L3_CACHE_SETTINGS pCacheSettings);

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
        PRENDERHAL_INTERFACE            pRenderHal,
        PRENDERHAL_L3_CACHE_SETTINGS    pCacheSettings,
        bool                            bEnableSLM);
    //! \brief    Allocates scratch space buffer.
    //! \details  A single scratch space buffer is allocated and used for all threads since XeHP.
    virtual MOS_STATUS AllocateScratchSpaceBuffer(
        uint32_t perThreadScratchSpace,
        RENDERHAL_INTERFACE *renderHal);

    //! \brief    Send Compute Walker
    //! \details  Send Compute Walker
    //! \param    PRENDERHAL_INTERFACE pRenderHal
    //!           [in] Pointer to Hardware Interface Structure
    //! \param    PMOS_COMMAND_BUFFER pCmdBuffer
    //!           [in] Pointer to Command Buffer
    //! \param    PRENDERHAL_GPGPU_WALKER_PARAMS pGpGpuWalkerParams
    //!           [in]    Pointer to GPGPU walker parameters
    //! \return   MOS_STATUS
    virtual MOS_STATUS SendComputeWalker(
        PRENDERHAL_INTERFACE        pRenderHal,
        PMOS_COMMAND_BUFFER         pCmdBuffer,
        PMHW_GPGPU_WALKER_PARAMS    pGpGpuWalkerParams);

    //! \brief    Send To 3DState Binding Table Pool Alloc
    //! \details  Send To 3DState Binding Table Pool Alloc
    //! \param    PRENDERHAL_INTERFACE pRenderHal
    //!           [in] Pointer to RenderHal Interface Structure
    //! \param    PMOS_COMMAND_BUFFER pCmdBuffer
    //!           [in] Pointer to Command Buffer
    //! \return   MOS_STATUS
    virtual MOS_STATUS SendTo3DStateBindingTablePoolAlloc(
        PRENDERHAL_INTERFACE        pRenderHal,
        PMOS_COMMAND_BUFFER         pCmdBuffer);
MEDIA_CLASS_DEFINE_END(XRenderHal_Interface_Xe_Hp_Base)
};

#endif // __RENDERHAL_XE_HP_BASE_H__
