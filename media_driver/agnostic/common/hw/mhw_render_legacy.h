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
//! \file     mhw_render_legacy.h
//! \brief    MHW interface for constructing commands for the render engine
//! \details  Impelements the functionalities common across all platforms for MHW_RENDER
//!

#ifndef __MHW_RENDER_LEGACY_H__
#define __MHW_RENDER_LEGACY_H__

#include "mhw_render.h"
#include "mhw_utilities.h"
#include "mhw_mi.h"
#include "mhw_state_heap_legacy.h"

class MhwRenderInterface
{
public:
    PMHW_STATE_HEAP_INTERFACE m_stateHeapInterface = nullptr;

    virtual ~MhwRenderInterface()
    {
        if (m_stateHeapInterface)
        {
            m_stateHeapInterface->pfnDestroy(m_stateHeapInterface);
        }
    }

    //!
    //! \brief    Allocates the MHW render interface internal parameters
    //! \details  Internal MHW function to allocate all parameters needed for the
    //!           render interface including the state heap interface
    //! \param    MHW_STATE_HEAP_SETTINGS stateHeapSettings
    //!           [in] Setting used to initialize the state heap interface
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS AllocateHeaps(
        MHW_STATE_HEAP_SETTINGS         stateHeapSettings);

    //!
    //! \brief    Adds PIPELINE_SELECT to the command buffer
    //! \param    cmdBuffer
    //!           [in] Command buffer to which commands are added
    //! \param    gpGpuPipe
    //!           [in] false: MEDIA pipe; true:  GPGPU pipe
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddPipelineSelectCmd (
        PMOS_COMMAND_BUFFER             cmdBuffer,
        bool                            gpGpuPipe) = 0;

    //!
    //! \brief    Adds STATE_BASE_ADDRESS to the command buffer
    //! \param    cmdBuffer
    //!           [in] Command buffer to which HW command is added
    //! \param    params
    //!           [in] Params structure used to populate the HW command
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddStateBaseAddrCmd(
        PMOS_COMMAND_BUFFER                 cmdBuffer,
        PMHW_STATE_BASE_ADDR_PARAMS         params) = 0;

    //!
    //! \brief    Adds MEDIA_VFE_STATE to the command buffer
    //! \param    cmdBuffer
    //!           [in] Command buffer to which HW command is added
    //! \param    params
    //!           [in] Params structure used to populate the HW command
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddMediaVfeCmd (
        PMOS_COMMAND_BUFFER             cmdBuffer,
        PMHW_VFE_PARAMS                 params) = 0;

    //!
    //! \brief    Adds CFE_STATE to the command buffer
    //! \param    cmdBuffer
    //!           [in] Command buffer to which HW command is added
    //! \param    params
    //!           [in] Params structure used to populate the HW command
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddCfeStateCmd(
        PMOS_COMMAND_BUFFER             cmdBuffer,
        PMHW_VFE_PARAMS                 params)
    {
        MOS_UNUSED(cmdBuffer);
        MOS_UNUSED(params);

        // CFE_STATE will replace the MEDIA_VFE_STATE on some platform; Just keep the
        // platform which really uses CFE to implement it on inheriting class .
        MHW_ASSERTMESSAGE("Don't support it on this platform");
        return MOS_STATUS_SUCCESS;
    }

    //!
    //! \brief    Adds MEDIA_CURBE_LOAD to the command buffer
    //! \param    cmdBuffer
    //!           [in] Command buffer to which HW command is added
    //! \param    params
    //!           [in] Params structure used to populate the HW command
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddMediaCurbeLoadCmd(
        PMOS_COMMAND_BUFFER             cmdBuffer,
        PMHW_CURBE_LOAD_PARAMS          params) = 0;

    //!
    //! \brief    Adds MEDIA_INTERFACE_DESCRIPTOR_LOAD to the command buffer
    //! \param    cmdBuffer
    //!           [in] Command buffer to which HW command is added
    //! \param    params
    //!           [in] Params structure used to populate the HW command
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddMediaIDLoadCmd(
        PMOS_COMMAND_BUFFER             cmdBuffer,
        PMHW_ID_LOAD_PARAMS             params) = 0;

    //!
    //! \brief    Adds MEDIA_OBJECT to the buffer provided
    //! \details  MEDIA_OBJCET is added to either the command buffer or
    //!           batch buffer (whichever is valid)
    //! \param    cmdBuffer
    //!           [in] If valid, command buffer to which HW command is added
    //! \param    batchBuffer
    //!           [in] If valid, Batch buffer to which HW command is added
    //! \param    params
    //!           [in] Params structure used to populate the HW command
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddMediaObject(
        PMOS_COMMAND_BUFFER             cmdBuffer,
        PMHW_BATCH_BUFFER               batchBuffer,
        PMHW_MEDIA_OBJECT_PARAMS        params) = 0;

    //!
    //! \brief    Adds MEDIA_OBJECT_WALKER to the command buffer
    //! \param    cmdBuffer
    //!           [in] Command buffer to which HW command is added
    //! \param    params
    //!           [in] Params structure used to populate the HW command
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddMediaObjectWalkerCmd(
        PMOS_COMMAND_BUFFER             cmdBuffer,
        PMHW_WALKER_PARAMS              params) = 0;

    //!
    //! \brief    Adds GPGPU_WALKER to the command buffer
    //! \param    cmdBuffer
    //!           [in] Command buffer to which HW command is added
    //! \param    params
    //!           [in] Params structure used to populate the HW command
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddGpGpuWalkerStateCmd(
        PMOS_COMMAND_BUFFER             cmdBuffer,
        PMHW_GPGPU_WALKER_PARAMS        params) = 0;

    //!
    //! \brief    Adds 3DSTATE_CHROMA_KEY to the command buffer
    //! \param    cmdBuffer
    //!           [in] Command buffer to which HW command is added
    //! \param    params
    //!           [in] Params structure used to populate the HW command
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddChromaKeyCmd(
        PMOS_COMMAND_BUFFER             cmdBuffer,
        PMHW_CHROMAKEY_PARAMS           params) = 0;

    //!
    //! \brief    Adds 3DSTATE_SAMPLER_PALETTE_LOADX to the command buffer
    //! \param    cmdBuffer
    //!           [in] Command buffer to which HW command is added
    //! \param    params
    //!           [in] Params structure used to populate the HW command
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddPaletteLoadCmd(
        PMOS_COMMAND_BUFFER             cmdBuffer,
        PMHW_PALETTE_PARAMS             params) = 0;

    //!
    //! \brief    Adds STATE_SIP to the command buffer
    //! \param    cmdBuffer
    //!           [in] Command buffer to which HW command is added
    //! \param    params
    //!           [in] Params structure used to populate the HW command
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddSipStateCmd(
        PMOS_COMMAND_BUFFER             cmdBuffer,
        PMHW_SIP_STATE_PARAMS           params) = 0;

    //!
    //! \brief    Adds GPGPU_CSR_BASE_ADDRESS to the command buffer
    //! \param    cmdBuffer
    //!           [in] Command buffer to which HW command is added
    //! \param    csrResource
    //!           [in] Resource to be used for GPGPU CSR
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddGpgpuCsrBaseAddrCmd(
        PMOS_COMMAND_BUFFER             cmdBuffer,
        PMOS_RESOURCE                   csrResource) = 0;

    //!
    //! \brief    get the size of hw command
    //! \details  Internal function to get the size of MEDIA_OBJECT_CMD
    //! \return   commandSize
    //!           The command size
    //!
    virtual uint32_t GetMediaObjectCmdSize() = 0;

    //!
    //! \brief    Enables L3 cacheing flag and sets related registers/values
    //! \param    cacheSettings
    //!           [in] L3 Cache Configurations, if a null pointer is passed 
    //!           in, it will use default settings.
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS EnableL3Caching(
        PMHW_RENDER_ENGINE_L3_CACHE_SETTINGS    cacheSettings) = 0;

    //!
    //! \brief    Setup L3 cache configuration for kernel workload
    //! \details  Enable L3 cacheing in kernel workload by configuring the
    //!           appropriate MMIO registers.
    //! \param    cmdBuffer
    //!           [in] Command buffer to which HW commands is added
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetL3Cache(
        PMOS_COMMAND_BUFFER             cmdBuffer) = 0;

    //!
    //! \brief    Enables preemption for media workloads on render engine
    //! \details  Sets the MMIO register for preemption so that HW can preempt
    //!           the submitted workload if required
    //! \param    cmdBuffer
    //!           [in] Command buffer to which HW commands is added
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS EnablePreemption(
        PMOS_COMMAND_BUFFER             cmdBuffer);

    //!
    //! \brief    Accessor for m_l3CacheConfig
    //! \return   L3 cache configuration information
    //!
    virtual MHW_RENDER_ENGINE_L3_CACHE_CONFIG* GetL3CacheConfig() = 0;

    //!
    //! \brief    Accessor for m_hwCaps, temporarily returns pointer to m_hwCaps until clients move to using not a pointer
    //! \return   Pointer to HW capabilities
    //!
    MHW_RENDER_ENGINE_CAPS *GetHwCaps() { return &m_hwCaps; }

    //!
    //! \brief    Accessor for m_preemptionEnabled
    //! \return   true if preemption is enabled, false otherwise
    //!
    bool IsPreemptionEnabled() { return m_preemptionEnabled; }

    //!
    //! \brief    Setter for os interface, used in MFE scenario
    //! \return   void
    //!
    void SetOsInterface(PMOS_INTERFACE osInterface) { m_osInterface = osInterface;}

    //!
    //! \brief    Get mmio registers address
    //! \details  Get mmio registers address
    //! \return   [out] PMHW_MI_MMIOREGISTERS*
    //!           mmio registers got.
    //!
    virtual PMHW_MI_MMIOREGISTERS GetMmioRegisters() = 0;

    //!
    //! \brief    Get AVS sampler state Inc unit
    //! \details  Get AVS sampler state Inc unit
    //! \return   [out] uint32_t
    //!           AVS sampler unit.
    virtual uint32_t GetSamplerStateAVSIncUnit() = 0;

    //!
    //! \brief    Get Conv sampler state Inc unit
    //! \details  Get Conv sampler state Inc unit
    //! \return   [out] uint32_t
    //!           Conv sampler unit.
    virtual uint32_t GetSamplerStateConvIncUnit() = 0;

    //!
    //! \brief    Get the sampler height and width align unit
    //! \details  NV12 format needs the width and height to be a multiple of some unit
    //! \param    [in] bool
    //!           true if AVS sampler, false otherwise
    //! \param    [in, out] uint32_t
    //!           weight align unit
    //! \param    [in, out] uint32_t
    //!           height align unit
    virtual void GetSamplerResolutionAlignUnit(bool isAVSSampler, uint32_t &widthAlignUnit, uint32_t &heightAlignUnit) = 0;

    //!
    //! \brief    Get new render interface, temporal solution before switching from
    //!           old interface to new one
    //!
    //! \return   pointer to new render interface
    //!
    virtual std::shared_ptr<mhw::render::Itf> GetNewRenderInterface() { return nullptr; }

protected:
    //!
    //! \brief    Initializes the Render interface
    //! \details  Internal MHW function to initialize all function pointers and some parameters
    //!           Assumes that the caller has checked pointer validity and whether or not an
    //!           addressing method has been selected in the OS interface (bUsesGfxAddress or
    //!           bUsesPatchList).
    //! \param    [in] miInterface
    //!           MI interface, must be valid
    //! \param    [in] osInterface
    //!           OS interface, must be valid
    //! \param    [in] gtSystemInfo
    //!           System information, must be valid
    //! \param    [in] newStateHeapManagerRequested
    //!           A new state heap manager was implemented for MDF, will be adapted for codec & VP,
    //!           migrated to C++, rolled into the existing state heap interface and removed.
    //!           Ultimately this parameter will no longer be necessary as the state heap interface
    //!           will be unified.
    //!
    MhwRenderInterface(
        MhwMiInterface          *miInterface,
        PMOS_INTERFACE          osInterface,
        MEDIA_SYSTEM_INFO       *gtSystemInfo,
        uint8_t                 newStateHeapManagerRequested)
    {
        MHW_FUNCTION_ENTER;

        if (miInterface == nullptr ||
            osInterface == nullptr ||
            gtSystemInfo == nullptr)
        {
            MHW_ASSERTMESSAGE("Invalid input pointers provided");
            return;
        }

        if (!osInterface->bUsesGfxAddress && !osInterface->bUsesPatchList)
        {
            MHW_ASSERTMESSAGE("No valid addressing mode indicated");
            return;
        }

        m_osInterface = osInterface;
        m_miInterface = miInterface;
        m_stateHeapInterface = nullptr;

        memset(&m_hwCaps, 0, sizeof(m_hwCaps));

        if (m_osInterface->bUsesGfxAddress)
        {
            AddResourceToCmd = Mhw_AddResourceToCmd_GfxAddress;
        }
        else // if (m_osInterface->bUsesPatchList)
        {
            AddResourceToCmd = Mhw_AddResourceToCmd_PatchList;
        }

        InitPlatformCaps(gtSystemInfo);

        InitPreemption();

        if (Mhw_StateHeapInterface_InitInterface_Legacy(
            &m_stateHeapInterface,
            m_osInterface,
            newStateHeapManagerRequested) != MOS_STATUS_SUCCESS)
        {
            MHW_ASSERTMESSAGE("State heap initialization failed!");
            return;
        }
    }

    PMOS_INTERFACE      m_osInterface = nullptr;
    MEDIA_FEATURE_TABLE *m_skuTable = nullptr;
    MhwMiInterface      *m_miInterface = nullptr;

    MHW_RENDER_ENGINE_L3_CACHE_CONFIG   m_l3CacheConfig = {};
    MHW_RENDER_ENGINE_CAPS              m_hwCaps = {};

    bool        m_preemptionEnabled = false;
    uint32_t    m_preemptionCntlRegisterOffset = 0;
    uint32_t    m_preemptionCntlRegisterValue = 0;

    uint32_t    m_l3CacheCntlRegisterOffset = M_L3_CACHE_CNTL_REG_OFFSET;
    uint32_t    m_l3CacheCntlRegisterValueDefault = M_L3_CACHE_CNTL_REG_VALUE_DEFAULT;
    std::shared_ptr <mhw::render::Itf> m_renderItfNew = nullptr;

    //!
    //! \brief    Adds a resource to the command buffer or indirect state (SSH)
    //! \details  Internal MHW function to add either a graphics address of a resource or
    //!           add the resource to the patch list for the requested buffer or state
    //! \param    [in] osInterface
    //!           OS interface
    //! \param    [in] cmdBuffer
    //!           If adding a resource to the command buffer, the buffer to which the resource
    //!           is added
    //! \param    [in] params
    //!           Parameters necessary to add the graphics address
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS(*AddResourceToCmd) (
        PMOS_INTERFACE                  osInterface,
        PMOS_COMMAND_BUFFER             cmdBuffer,
        PMHW_RESOURCE_PARAMS            params) = nullptr;

    //!
    //! \brief    Initializes platform related capabilities for the render engine
    //! \details  Assumes the caller checked the pointers for validity.
    //! \param    gtSystemInfo
    //!           [in] Information concerning the GPU
    //! \return   void
    //!
    void InitPlatformCaps(
        MEDIA_SYSTEM_INFO         *gtSystemInfo);

    //!
    //! \brief    Initializes preemption related registers/values
    //! \details  Initializes the MMIO register for preemption so that HW can preempt
    //!           the submitted workload if required.
    //! \return   void
    //!           If invalid SKU\WA tables detected, does not do anything
    //!
    void InitPreemption();
};

#endif // __MHW_RENDER_LEGACY_H__