/*
* Copyright (c) 2009-2021, Intel Corporation
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
//! \file      renderhal_g8.h 
//! \brief 
//!
//!
//! \file       renderhal_g8.h
//! \brief      header file of Gen8 hardware functions
//! \details    Gen8 hardware functions declare
//!
#ifndef __RENDERHAL_G8_H__
#define __RENDERHAL_G8_H__

#include "renderhal_platform_interface_legacy.h"
#include "mhw_render_hwcmd_g8_X.h"
#include "mhw_state_heap_hwcmd_g8_X.h"
#include "mhw_mmio_g8.h"

//!
//! \brief      L3$ registers and configuration Gen8
//!




#define RENDERHAL_SAMPLERS_AVS_G8           3   // 1 layer - One each for Y, U and V

//!
//! \brief      L3 cache CNTLREG value with SLM disabled on BDW GT2 VP/MDF
//!             SLM     URB     DC      RO      I/S     C       T
//!             { 0,    256,    64,     448,    0,      0,      0      }
#define RENDERHAL_L3_CACHE_CNTL_REG_VALUE_G8                    (0x0021C040)

//! \brief      for BDW GT2 with WA for D0 hang
//!              SLM     URB    Rest     DC     RO    I/S     C       T
//!             { 256,   128,   384,     0,     0,    0,     0,      0,     }
#define RENDERHAL_L3_CACHE_CNTL_REG_VALUE_ALLOCATE_SLM_D0_WA_G8 (0x60000021)

//!
//! \brief      L3 cache CNTLREG value with SLM disabled
//!             SLM     URB     Rest   DC      RO      I/S     C       T
//!             { 0,    256,    512,   0,      0,      0,      0,      0    }
//!
#define RENDERHAL_L3_CACHE_CNTL_REG_SLM_DISABLE_ALL_L3_512K_G8       (0x80000040)

//!
//! \brief      L3 cache CNTLREG value with SLM enabled
//!             SLM     URB    Rest   DC       RO     I/S     C     T      Sum
//!             { 192,  128,   0,     256,     128,   0,      0,    0       }
#define RENDERHAL_L3_CACHE_CNTL_REG_SLM_ENABLE_G8                    (0x00808021)

typedef enum _RENDERHAL_MO_TARGET_CACHE_G8
{
    RENDERHAL_MO_TARGET_CACHE_ELLC_G8            = 0x0,
    RENDERHAL_MO_TARGET_CACHE_LLC_G8             = 0x1,
    RENDERHAL_MO_TARGET_CACHE_LLC_ELLC_G8        = 0x2,
    RENDERHAL_MO_TARGET_CACHE_L3_LLC_ELLC_G8     = 0x3
} RENDERHAL_MO_TARGET_CACHE_G8;

typedef enum _RENDERHAL_MO_CACHE_CONTROL_G8
{
    RENDERHAL_MO_CACHE_CONTROL_USE_PTE_G8           = 0x0,
    RENDERHAL_MO_CACHE_CONTROL_UC_G8                = 0x1,
    RENDERHAL_MO_CACHE_CONTROL_WT_G8                = 0x2,
    RENDERHAL_MO_CACHE_CONTROL_WB_G8                = 0x3
} RENDERHAL_MO_CACHE_CONTROL_G8;

class XRenderHal_Interface_g8 : public XRenderHal_Platform_Interface_Legacy
{
public:
    XRenderHal_Interface_g8() {}
    virtual ~XRenderHal_Interface_g8() {}

    //!
    //! \brief    Setup Surface State
    //! \details  Setup Surface States for Gen75/Gen8
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
        int32_t                         *piNumEntries,
        PRENDERHAL_SURFACE_STATE_ENTRY  *ppSurfaceEntries,
        PRENDERHAL_OFFSET_OVERRIDE      pOffsetOverride);

    //!
    //! \brief    Check if Sampler128Elements is supported
    //! \return   true of false
    //!
    virtual inline bool IsSampler128ElementsSupported() { return false; }

    //!
    //! \brief      Checks how per thread scratch space size bits in VFE state are interpreted by HW
    //! \details    For BDW GT1/2/3 A0 steppings, per thread scratch space size in VFE state
    //!             is 11 bits indicating [2k bytes, 2 Mbytes]: 0=2k, 1=4k, 2=8k ... 10=2M
    //!             BDW+ excluding A0 step is 12 bits indicating [1k bytes, 2 Mbytes]: 0=1k, 1=2k, 2=4k, 3=8k ... 11=2M
    //! \param      PRENDERHAL_INTERFACE pRenderHal
    //!             [in]    Pointer to RenderHal interface
    //! \return     true if BDW A0 stepping, false otherwise
    //!
    bool PerThreadScratchSpaceStart2K(
        PRENDERHAL_INTERFACE pRenderHal);

    //!
    //! \brief    Encode SLM Size for Interface Descriptor
    //! \details  Setup SLM size
    //! \param    uint32_t SLMSize
    //!           [in] SLM size in 1K
    //! \return   encoded output
    //!
    uint32_t EncodeSLMSize(uint32_t SLMSize);

    //!
    //! \brief    Set Chroma Direction
    //! \details  Setup Chroma Direction for G8
    //! \param    PRENDERHAL_INTERFACE pRenderHal
    //!           [in]  Pointer to Hardware Interface
    //! \param    PRENDERHAL_SURFACE pRenderHalSurface
    //!           [in]  Pointer to Render Hal Surface
    //! \return   uint8_t
    //!
    uint8_t SetChromaDirection(
        PRENDERHAL_INTERFACE pRenderHal,
        PRENDERHAL_SURFACE   pRenderHalSurface);

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
        PRENDERHAL_INTERFACE    pRenderHal,
        uint64_t                iTicks,
        uint64_t                *piNs);

    //!
    //! \brief    Initialize the State Heap Settings per platform
    //! \param    PRENDERHAL_INTERFACE    pRenderHal
    //!           [out] Pointer to PRENDERHAL_INTERFACE
    //! \return   void
    //!
    void InitStateHeapSettings(
        PRENDERHAL_INTERFACE    pRenderHal);

    //!
    //! \brief    Initialize the default surface type and advanced surface type  per platform
    //! \param    PRENDERHAL_INTERFACE    pRenderHal
    //!           [out] Pointer to PRENDERHAL_INTERFACE
    //! \return   void
    //!
    void InitSurfaceTypes(
        PRENDERHAL_INTERFACE    pRenderHal);

    //!
    //! \brief    Check if YV12 Single Pass is supported
    //! \param    PRENDERHAL_INTERFACE    pRenderHal
    //!           [in]  Pointer to Hardware Interface
    //! \return   true of false
    //!
    inline bool IsEnableYV12SinglePass(
        PRENDERHAL_INTERFACE    pRenderHal)
    {
        MOS_UNUSED(pRenderHal);
        return false;
    }

    //!
    //! \brief     Get the Size of AVS Sampler State
    //! \param    PRENDERHAL_INTERFACE    pRenderHal
    //!           [in]  Pointer to Hardware Interface
    //! \return   size
    //!
    inline uint32_t GetSizeSamplerStateAvs(
        PRENDERHAL_INTERFACE    pRenderHal)
    {
        return pRenderHal->pHwSizes->dwSizeSamplerStateAvs;
    }

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
        void                    **ppSampler);

    //!
    //! \brief      Initialize the DSH Settings
    //! \details    Initialize the structure DynamicHeapSettings in pRenderHal
    //! \param      PRENDERHAL_INTERFACE pRenderHal
    //!             [in]    Pointer to HW interface
    //! \return     void
    //!
    virtual void InitDynamicHeapSettings(
        PRENDERHAL_INTERFACE  pRenderHal);

    //!
    //! \brief      Get the depth bit mask for buffer 
    //! \details    Get the depth bit mask for buffer 
    //! \return     uint32_t
    //!             depth bit mask for buffer 
    //!
    uint32_t GetDepthBitMaskForBuffer() { return MOS_MASKBITS32(21, 26); };

    //!
    //! \brief      Get the depth bit mask for raw buffer 
    //! \details    Get the depth bit mask for raw buffer 
    //! \return     uint32_t
    //!             depth bit mask for raw buffer 
    //!
    uint32_t GetDepthBitMaskForRawBuffer() { return MOS_MASKBITS32(21, 30); };

    //!
    //! \brief      Get the pointer to the MHW_VFE_PARAMS
    //! \return     MHW_VFE_PARAMS*
    //!             pointer to the MHW_VFE_PARAMS
    //!
    virtual MHW_VFE_PARAMS* GetVfeStateParameters() { return &m_vfeStateParams; }

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
        PMOS_COMMAND_BUFFER          pCmdBuffer);

    //!
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

    //! \brief      Get the size of Render Surface State Command
    //! \return     size_t
    //!             the size of render surface state command
    virtual size_t GetSurfaceStateCmdSize();

    //! \brief      Get the address of the ith Palette Data
    //! \param      [in] i
    //!             Index of the palette data
    //! \return     void *
    //!             address of the ith palette data table
    virtual void* GetPaletteDataAddress(int i) {return &m_paletteData[i];}

    //! \brief      Get the size of Binding Table State Command
    //! \return     size_t
    //!             the size of binding table state command
    virtual size_t GetBTStateCmdSize() {return mhw_state_heap_g8_X::BINDING_TABLE_STATE_CMD::byteSize;}

    //! \brief    Check if compute context in use
    //! \param    PRENDERHAL_INTERFACE    pRenderHal
    //!           [in]  Pointer to Hardware Interface
    //! \return   true of false
    virtual bool IsComputeContextInUse(PRENDERHAL_INTERFACE pRenderHal)
    {
        MOS_UNUSED(pRenderHal);
        return false;
    }

protected:
    //!
    //! \brief    Get L3 control register value with SLM enabled
    //! \return   uint32_t
    //!           L3 Control Register value
    //!
    virtual uint32_t GetL3CacheCntlRegWithSLM() = 0;

    MHW_VFE_PARAMS               m_vfeStateParams;
    mhw_render_g8_X::PALETTE_ENTRY_CMD
                                 m_paletteData[RENDERHAL_PALETTE_MAX][RENDERHAL_PALETTE_ENTRIES_MAX];

};

#endif // __RENDERHAL_G8_H__
