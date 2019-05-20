/*
* Copyright (c) 2009-2018, Intel Corporation
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
//! \file      renderhal_platform_interface.h 
//! \brief     abstract the platfrom specific APIs into one class 
//!
//!
//! \file     renderhal.h
//! \brief    Render Engine Interfaces shared across platforms
//! \details  Platform Independent Hardware Interfaces
//!
#ifndef __RENDERHAL_PLATFORM_INTERFACE_H__
#define __RENDERHAL_PLATFORM_INTERFACE_H__

#include "mos_os.h"
#include "renderhal.h"

class XRenderHal_Platform_Interface
{
public:
    XRenderHal_Platform_Interface() {}
    virtual ~XRenderHal_Platform_Interface() {}

    //!
    //! \brief    Setup Surface State
    //! \details  Setup Surface States
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
    //!           used for kernel in a few cases. nullptr is the most common usage.
    //! \return   MOS_STATUS
    //!
    virtual MOS_STATUS SetupSurfaceState(
        PRENDERHAL_INTERFACE            pRenderHal,
        PRENDERHAL_SURFACE              pRenderHalSurface,
        PRENDERHAL_SURFACE_STATE_PARAMS pParams,
        int32_t                         *piNumEntries,
        PRENDERHAL_SURFACE_STATE_ENTRY  *ppSurfaceEntries,
        PRENDERHAL_OFFSET_OVERRIDE      pOffsetOverride) = 0;

    //!
    //! \brief    Check if Sampler128Elements is supported
    //! \return   true of false
    //!
    virtual bool IsSampler128ElementsSupported() = 0;

    //!
    //! \brief      Checks how per thread scratch space size bits in VFE state are interpreted by HW
    //! \details    For BDW GT1/2/3 A0 steppings, per thread scratch space size in VFE state
    //!             is 11 bits indicating [2k bytes, 2 Mbytes]: 0=2k, 1=4k, 2=8k ... 10=2M
    //!             BDW+ excluding A0 step is 12 bits indicating [1k bytes, 2 Mbytes]: 0=1k, 1=2k, 2=4k, 3=8k ... 11=2M
    //! \param      PRENDERHAL_INTERFACE pRenderHal
    //!             [in]    Pointer to RenderHal interface
    //! \return     true if BDW A0 stepping, false otherwise
    //!
    virtual bool PerThreadScratchSpaceStart2K(
        PRENDERHAL_INTERFACE pRenderHal) = 0;

    //!
    //! \brief      Checks how per thread scratch space size bits in VFE state are interpreted by HW
    //! \details    On some new platforms, per thread scratch space size can be 2^n (n >= 6) bytes.
    //!             If this is supported, total scratch space size can be reduced.
    //! \return     64-byte base size is supported on specific platforms, so false is returned in
    //!             base class implementation.
    //!
    virtual bool PerThreadScratchSpaceStart64Byte(
        RENDERHAL_INTERFACE *renderHal) { return false; }

    //!
    //! \brief    Encode SLM Size for Interface Descriptor
    //! \details  Setup SLM size
    //! \param    uint32_t SLMSize
    //!           [in] SLM size in 1K
    //! \return   encoded output
    //!
    virtual uint32_t EncodeSLMSize(uint32_t SLMSize) = 0;

    //!
    //! \brief    Set Chroma Direction
    //! \details  Setup Chroma Direction
    //! \param    PRENDERHAL_INTERFACE pRenderHal
    //!           [in]  Pointer to Hardware Interface
    //! \param    PRENDERHAL_SURFACE pRenderHalSurface
    //!           [in]  Pointer to Render Hal Surface
    //! \return   uint8_t
    //!
    virtual uint8_t SetChromaDirection(
        PRENDERHAL_INTERFACE pRenderHal,
        PRENDERHAL_SURFACE   pRenderHalSurface) = 0;

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
    virtual void ConvertToNanoSeconds(
        PRENDERHAL_INTERFACE    pRenderHal,
        uint64_t                iTicks,
        uint64_t                *piNs) = 0;

    //!
    //! \brief    Initialize the State Heap Settings per platform
    //! \param    PRENDERHAL_INTERFACE    pRenderHal
    //!           [out] Pointer to PRENDERHAL_INTERFACE
    //! \return   void
    //!
    virtual void InitStateHeapSettings(
        PRENDERHAL_INTERFACE    pRenderHal) = 0;

    //!
    //! \brief    Initialize the default surface type and advanced surface type  per platform
    //! \param    PRENDERHAL_INTERFACE    pRenderHal
    //!           [out] Pointer to PRENDERHAL_INTERFACE
    //! \return   void
    //!
    virtual void InitSurfaceTypes(
        PRENDERHAL_INTERFACE    pRenderHal) = 0;

    //!
    //! \brief    Check if YV12 Single Pass is supported
    //! \param    PRENDERHAL_INTERFACE    pRenderHal
    //!           [in]  Pointer to Hardware Interface
    //! \return   true of false
    //!
    virtual bool IsEnableYV12SinglePass(
        PRENDERHAL_INTERFACE    pRenderHal) = 0;

    //!
    //! \brief     Get the Size of AVS Sampler State
    //! \param    PRENDERHAL_INTERFACE    pRenderHal
    //!           [in]  Pointer to Hardware Interface
    //! \return   size
    //!
    virtual uint32_t GetSizeSamplerStateAvs(
        PRENDERHAL_INTERFACE    pRenderHal) = 0;

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
        PRENDERHAL_L3_CACHE_SETTINGS pCacheSettings) = 0;

    //!
    //! \brief    Get offset and/or pointer to sampler state
    //! \details  Get offset and/or pointer to sampler state in General State Heap
    //! \param    PRENDERHAL_INTERFACE pRenderHal
    //!           [in] Pointer to RenderHal Interface
    //! \param    int32_t iMediaID
    //!           [in] Media ID associated with sampler
    //! \param    int32_t iSamplerID
    //!           [in] Sampler ID
    //! \param    uint32_t *pdwSamplerOffset
    //!           [out] optional; offset of sampler state from GSH base
    //! \param    void  **ppSampler
    //!           [out] optional; pointer to sampler state in GSH
    //! \return   MOS_STATUS
    //!
    virtual MOS_STATUS GetSamplerOffsetAndPtr_DSH(
        PRENDERHAL_INTERFACE     pRenderHal,
        int32_t                  iMediaID,
        int32_t                  iSamplerID,
        PMHW_SAMPLER_STATE_PARAM pSamplerParams,
        uint32_t                 *pdwSamplerOffset,
        void                    **ppSampler) = 0;

    //!
    //! \brief      Initialize the DSH Settings
    //! \details    Initialize the structure DynamicHeapSettings in pRenderHal
    //! \param      PRENDERHAL_INTERFACE pRenderHal
    //!             [in]    Pointer to HW interface
    //! \return     void
    //!
    virtual void InitDynamicHeapSettings(
        PRENDERHAL_INTERFACE  pRenderHal) = 0;

    //!
    //! \brief      Get the depth bit mask for buffer 
    //! \details    Get the depth bit mask for buffer 
    //! \return     uint32_t
    //!             depth bit mask for buffer 
    //!
    virtual uint32_t GetDepthBitMaskForBuffer() { return MOS_MASKBITS32(21, 29); };

    //!
    //! \brief      Get the depth bit mask for raw buffer 
    //! \details    Get the depth bit mask for raw buffer 
    //! \return     uint32_t
    //!             depth bit mask for raw buffer 
    //!
    virtual uint32_t GetDepthBitMaskForRawBuffer() { return MOS_MASKBITS32(21, 29);};

    //!
    //! \brief      Get the pointer to the MHW_VFE_PARAMS
    //! \return     MHW_VFE_PARAMS*
    //!             pointer to the MHW_VFE_PARAMS
    //!
    virtual MHW_VFE_PARAMS* GetVfeStateParameters() = 0;

    //!
    //! \brief    Set Power Option Status
    //! \param    [in] pRenderHal
    //!           Pointer to Hardware Interface
    //! \param    [in,out] pCmdBuffer
    //!           Pointer to Command Buffer
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetPowerOptionStatus(
        PRENDERHAL_INTERFACE         pRenderHal,
        PMOS_COMMAND_BUFFER          pCmdBuffer) = 0;

    //!
    //! \brief    Set Composite Prolog CMD
    //! \param    [in] pRenderHal
    //!           Pointer to Hardware Interface
    //! \param    [in,out] pCmdBuffer
    //!           Pointer to Command Buffer
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetCompositePrologCmd(
        PRENDERHAL_INTERFACE         pRenderHal,
        PMOS_COMMAND_BUFFER          pCmdBuffer)
    {
        return MOS_STATUS_SUCCESS;
    }

    //!
    //! \brief    Get Render Hal MMC Enable/Disable Flag
    //! \param    [in] pRenderHal
    //!           Pointer to Hardware Interface
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS IsRenderHalMMCEnabled(
        PRENDERHAL_INTERFACE         pRenderHal)
    {
        MOS_UNUSED(pRenderHal);

        return MOS_STATUS_SUCCESS;
    }

    //!
    //! \brief    Check if Override is needed or not
    //! \param    [in] pRenderHal
    //!           Pointer to Hardware Interface
    //! \param    [in,out] pCmdBuffer
    //!           Pointer to Command Buffer
    //! \param    [in] pGenericPrologParam
    //!           Pointer to MHW generic prolog parameters
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS IsOvrdNeeded(
        PRENDERHAL_INTERFACE              pRenderHal,
        PMOS_COMMAND_BUFFER               pCmdBuffer,
        PRENDERHAL_GENERIC_PROLOG_PARAMS  pGenericPrologParams) {return MOS_STATUS_SUCCESS;};

    //!
    //! \brief    Check if Chromasiting is enabled
    //! \param    [in] pRenderHal
    //!           Pointer to Hardware Interface
    //! \param    [in] pParams
    //!           Pointer to Surface State Params
    //! \return   bool
    //!           true or false
    //!
    virtual bool IsChromasitingEnabled(
        PRENDERHAL_INTERFACE            pRenderHal,
        PRENDERHAL_SURFACE_STATE_PARAMS pParams) {return false;};

    //!
    //! \brief    Get PlaneDefinition for Format_Y216
    //! \param    [in] isRenderTarget
    //!           The flag to indicate if the surface is rendertarget or not
    //! \param    [in] pRenderHal
    //!           Pointer to Hardware Interface
    //! \param    [in,out] PlaneDefinition
    //!           Pointer to PlaneDefinition
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS GetPlaneDefForFormatY216(
        bool                       isRenderTarget,
        PRENDERHAL_INTERFACE       pRenderHal,
        RENDERHAL_PLANE_DEFINITION &PlaneDefinition)
    {
        PlaneDefinition = isRenderTarget ? RENDERHAL_PLANES_Y210_RT : (pRenderHal->bIsAVS ? RENDERHAL_PLANES_Y210_ADV : RENDERHAL_PLANES_Y210);
        return MOS_STATUS_SUCCESS;
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
        PRENDERHAL_INTERFACE            pRenderHal,
        PRENDERHAL_L3_CACHE_SETTINGS    pCacheSettings,
        bool                            bEnableSLM) = 0;

    //! \brief      Get the size of Render Surface State Command
    //! \return     size_t
    //!             the size of render surface state command
    virtual size_t GetSurfaceStateCmdSize() = 0;

    //! \brief      Get the address of the ith Palette Data
    //! \param      [in] i
    //!             Index of the palette data
    //! \return     void *
    //!             address of the ith palette data table
    virtual void* GetPaletteDataAddress(int i) = 0;

    //! \brief      Get the size of Binding Table State Command
    //! \return     size_t
    //!             the size of binding table state command
    virtual size_t GetBTStateCmdSize() = 0;

    //! \brief    Check if compute context in use
    //! \param    PRENDERHAL_INTERFACE    pRenderHal
    //!           [in]  Pointer to RenderHal Interface
    //! \return   true or false
    virtual bool IsComputeContextInUse(
        PRENDERHAL_INTERFACE    pRenderHal) = 0;

    //! \brief    Send Compute Walker
    //! \details  Send Compute Walker
    //! \param    PRENDERHAL_INTERFACE pRenderHal
    //!           [in] Pointer to RenderHal Interface Structure
    //! \param    PMOS_COMMAND_BUFFER pCmdBuffer
    //!           [in] Pointer to Command Buffer
    //! \param    PRENDERHAL_GPGPU_WALKER_PARAMS pGpGpuWalkerParams
    //!           [in]    Pointer to GPGPU walker parameters
    //! \return   MOS_STATUS
    virtual MOS_STATUS SendComputeWalker(
        PRENDERHAL_INTERFACE        pRenderHal,
        PMOS_COMMAND_BUFFER         pCmdBuffer,
        PMHW_GPGPU_WALKER_PARAMS    pGpGpuWalkerParams)
    {
        return MOS_STATUS_SUCCESS;
    };

    //! \brief    Send To 3DState Binding Table Pool Alloc
    //! \details  Send To 3DState Binding Table Pool Alloc
    //! \param    PRENDERHAL_INTERFACE pRenderHal
    //!           [in] Pointer to RenderHal Interface Structure
    //! \param    PMOS_COMMAND_BUFFER pCmdBuffer
    //!           [in] Pointer to Command Buffer
    //! \return   MOS_STATUS
    virtual MOS_STATUS SendTo3DStateBindingTablePoolAlloc(
        PRENDERHAL_INTERFACE        pRenderHal,
        PMOS_COMMAND_BUFFER         pCmdBuffer)
    {
        return MOS_STATUS_SUCCESS;
    };

    //! \brief    Allocates scratch space buffer.
    //! \details  On some new pltforms, a single scratch space buffer may be allocated and used for
    //!           all threads.
    //! \return   Single scratch space buffer is supported on specific platforms, so
    //!           MOS_STATUS_UNIMPLEMENTED is returned in base class implementation.
    virtual MOS_STATUS AllocateScratchSpaceBuffer(
        uint32_t perThreadScratchSpace,
        RENDERHAL_INTERFACE *renderHal) { return MOS_STATUS_UNIMPLEMENTED; }

    //! \brief    Frees scratch space buffer.
    //! \details  On some new pltforms, a single scratch space buffer may be allocated and used for
    //!           all threads.
    //! \return   Single scratch space buffer is supported on specific platforms, so
    //!           MOS_STATUS_UNIMPLEMENTED is returned in base class implementation.
    virtual MOS_STATUS FreeScratchSpaceBuffer(
        RENDERHAL_INTERFACE *renderHal) { return MOS_STATUS_UNIMPLEMENTED; }
};

#endif // __RENDERHAL_PLATFORM_INTERFACE_H__
