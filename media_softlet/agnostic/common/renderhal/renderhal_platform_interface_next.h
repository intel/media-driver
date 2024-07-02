/*
* Copyright (c) 2021-2023, Intel Corporation
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
//! \file      renderhal_platform_interface_next.h 
//! \brief     abstract the platfrom specific APIs into one class 
//!
//!
//! \file     renderhal.h
//! \brief    Render Engine Interfaces shared across platforms
//! \details  Platform Independent Hardware Interfaces
//!
#ifndef __RENDERHAL_PLATFORM_INTERFACE_NEXT_H__
#define __RENDERHAL_PLATFORM_INTERFACE_NEXT_H__

#include <memory>
#include "media_class_trace.h"
#include "mhw_mi.h"
#include "mhw_mmio.h"
#include "mhw_render.h"
#include "mhw_state_heap.h"
#include "mhw_utilities_next.h"
#include "mos_defs.h"
#include "mos_os_specific.h"
#include "vp_common.h"
#include "renderhal_platform_interface.h"
#include "mhw_render_itf.h"
class MediaFeatureManager;
namespace mhw { namespace mi { class Itf; } }

typedef struct _RENDERHAL_GENERIC_PROLOG_PARAMS_NEXT : _RENDERHAL_GENERIC_PROLOG_PARAMS
{
    MOS_VIRTUALENGINE_HINT_PARAMS VEngineHintParams = {{0}, 0, {{0}, {0}, {0}, {0}}, {0, 0, 0, 0}};
} RENDERHAL_GENERIC_PROLOG_PARAMS_NEXT, *PRENDERHAL_GENERIC_PROLOG_PARAMS_NEXT;

class XRenderHal_Platform_Interface_Next : public XRenderHal_Platform_Interface, public mhw::render::Itf::ParSetting
{
public:
    XRenderHal_Platform_Interface_Next();
    virtual ~XRenderHal_Platform_Interface_Next() {}

    MOS_STATUS AddPipelineSelectCmd(
        PRENDERHAL_INTERFACE        pRenderHal,
        PMOS_COMMAND_BUFFER         pCmdBuffer,
        bool                        gpGpuPipe);

    MOS_STATUS SendStateBaseAddress(
        PRENDERHAL_INTERFACE        pRenderHal,
        PMOS_COMMAND_BUFFER         pCmdBuffer);

    MOS_STATUS AddSipStateCmd(
        PRENDERHAL_INTERFACE        pRenderHal,
        PMOS_COMMAND_BUFFER         pCmdBuffer);

    MOS_STATUS AddCfeStateCmd(
        PRENDERHAL_INTERFACE        pRenderHal,
        PMOS_COMMAND_BUFFER         pCmdBuffer,
        PMHW_VFE_PARAMS             params);

    MOS_STATUS SendChromaKey(
        PRENDERHAL_INTERFACE        pRenderHal,
        PMOS_COMMAND_BUFFER         pCmdBuffer,
        PMHW_CHROMAKEY_PARAMS       pChromaKeyParams);

    MOS_STATUS SendPalette(
        PRENDERHAL_INTERFACE        pRenderHal,
        PMOS_COMMAND_BUFFER         pCmdBuffer,
        PMHW_PALETTE_PARAMS         pPaletteLoadParams);

    MOS_STATUS SetL3Cache(
        PRENDERHAL_INTERFACE        pRenderHal,
        PMOS_COMMAND_BUFFER         pCmdBuffer);
    
    //!
    //! \brief    Get the size of render hal media state
    //! \return   size_t
    //!           The size of render hal media state
    //!
    size_t GetRenderHalMediaStateSize();
    
    //!
    //! \brief    Get the size of render hal state heap
    //! \return   size_t
    //!           The size of render hal state heap
    //!
    size_t GetRenderHalStateHeapSize();

    PMHW_MI_MMIOREGISTERS GetMmioRegisters(
        PRENDERHAL_INTERFACE        pRenderHal);

    MOS_STATUS EnablePreemption(
        PRENDERHAL_INTERFACE            pRenderHal,
        PMOS_COMMAND_BUFFER             pCmdBuffer);

    MOS_STATUS SendPredicationCommand(
        PRENDERHAL_INTERFACE        pRenderHal,
        PMOS_COMMAND_BUFFER         pCmdBuffer);

    //! \brief    Adds marker attributes in command buffer
    //! \param    PRENDERHAL_INTERFACE pRenderHal
    //!           [in] Pointer to RenderHal Interface Structure
    //! \param    PMOS_COMMAND_BUFFER pcmdBuffer
    //!           [in] Pointer to Command Buffer
    //! \param    bool isRender
    //!           [in] Flag of Render Engine
    //! \return   MOS_STATUS
    MOS_STATUS SendMarkerCommand(
        PRENDERHAL_INTERFACE    pRenderHal,
        PMOS_COMMAND_BUFFER     cmdBuffer,
        bool                    isRender);

    MOS_STATUS AddMiPipeControl(
        PRENDERHAL_INTERFACE    pRenderHal,
        PMOS_COMMAND_BUFFER        pCmdBuffer,
        MHW_PIPE_CONTROL_PARAMS*   params);

    //!
    //! \brief    Adds MI_LOAD_REGISTER_IMM to the command buffer
    //! \param    PRENDERHAL_INTERFACE pRenderHal
    //!           [in] Pointer to RenderHal Interface Structure
    //! \param    [in] pCmdBuffer
    //!           Command buffer to which requested command is added
    //! \param    [in] params
    //!           Parameters used to populate the requested command
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS AddMiLoadRegisterImmCmd(
        PRENDERHAL_INTERFACE             pRenderHal,
        PMOS_COMMAND_BUFFER              pCmdBuffer,
        PMHW_MI_LOAD_REGISTER_IMM_PARAMS params);

    MOS_STATUS SendGenericPrologCmd(
        PRENDERHAL_INTERFACE        pRenderHal,
        PMOS_COMMAND_BUFFER         pCmdBuffer,
        PMHW_GENERIC_PROLOG_PARAMS  pParams,
        MHW_MI_MMIOREGISTERS* pMmioReg = nullptr);

    MOS_STATUS CreateMhwInterfaces(
        PRENDERHAL_INTERFACE        pRenderHal,
        PMOS_INTERFACE              pOsInterface);

    MOS_STATUS On1stLevelBBStart(
        PRENDERHAL_INTERFACE pRenderHal,
        PMOS_COMMAND_BUFFER  pCmdBuffer,
        PMOS_CONTEXT         pOsContext,
        uint32_t             gpuContextHandle,
        MHW_MI_MMIOREGISTERS *pMmioReg);

    MOS_STATUS OnDispatch(
        PRENDERHAL_INTERFACE pRenderHal,
        PMOS_COMMAND_BUFFER  pCmdBuffer,
        PMOS_INTERFACE       pOsInterface,
        MHW_MI_MMIOREGISTERS *pMmioReg);

    MOS_STATUS CreatePerfProfiler(
        PRENDERHAL_INTERFACE pRenderHal);

    MOS_STATUS DestroyPerfProfiler(
        PRENDERHAL_INTERFACE pRenderHal);
        
    MOS_STATUS AddPerfCollectStartCmd(
        PRENDERHAL_INTERFACE pRenderHal,
        MOS_INTERFACE        *osInterface,
        MOS_COMMAND_BUFFER   *cmdBuffer);

    MOS_STATUS StartPredicate(
        PRENDERHAL_INTERFACE pRenderHal,
        PMOS_COMMAND_BUFFER  cmdBuffer);

    MOS_STATUS StopPredicate(
        PRENDERHAL_INTERFACE pRenderHal,
        PMOS_COMMAND_BUFFER  cmdBuffer);

    MOS_STATUS AddPerfCollectEndCmd(
        PRENDERHAL_INTERFACE pRenderHal,
        PMOS_INTERFACE       pOsInterface,
        MOS_COMMAND_BUFFER   *cmdBuffer);

    MOS_STATUS AddMediaVfeCmd(
        PRENDERHAL_INTERFACE    pRenderHal,
        PMOS_COMMAND_BUFFER     pCmdBuffer,
        MHW_VFE_PARAMS          *params);

    MOS_STATUS AddMediaStateFlush(
        PRENDERHAL_INTERFACE         pRenderHal,
        PMOS_COMMAND_BUFFER          pCmdBuffer,
        MHW_MEDIA_STATE_FLUSH_PARAM  *params);

    MOS_STATUS AddMiBatchBufferEnd(
        PRENDERHAL_INTERFACE         pRenderHal,
        PMOS_COMMAND_BUFFER          pCmdBuffer,
        PMHW_BATCH_BUFFER            batchBuffer);

    MOS_STATUS AddMediaObjectWalkerCmd(
        PRENDERHAL_INTERFACE         pRenderHal,
        PMOS_COMMAND_BUFFER          pCmdBuffer,
        PMHW_WALKER_PARAMS           params);

    MOS_STATUS AddGpGpuWalkerStateCmd(
        PRENDERHAL_INTERFACE     pRenderHal,
        PMOS_COMMAND_BUFFER      pCmdBuffer,
        PMHW_GPGPU_WALKER_PARAMS params);

    MOS_STATUS AllocateHeaps(
        PRENDERHAL_INTERFACE     pRenderHal,
        MHW_STATE_HEAP_SETTINGS  MhwStateHeapSettings);

    PMHW_STATE_HEAP_INTERFACE GetStateHeapInterface(
        PRENDERHAL_INTERFACE     pRenderHal);

    MOS_STATUS DestoryMhwInterface(
        PRENDERHAL_INTERFACE     pRenderHal);

    MOS_STATUS AddMediaCurbeLoadCmd(
        PRENDERHAL_INTERFACE         pRenderHal,
        PMOS_COMMAND_BUFFER          pCmdBuffer,
        PMHW_CURBE_LOAD_PARAMS       params);

    MOS_STATUS AddMediaIDLoadCmd(
        PRENDERHAL_INTERFACE         pRenderHal,
        PMOS_COMMAND_BUFFER          pCmdBuffer,
        PMHW_ID_LOAD_PARAMS          params);

    bool IsPreemptionEnabled(
        PRENDERHAL_INTERFACE         pRenderHal);

    void GetSamplerResolutionAlignUnit(
        PRENDERHAL_INTERFACE         pRenderHal,
        bool                         isAVSSampler,
        uint32_t                     &widthAlignUnit,
        uint32_t                     &heightAlignUnit);

    PMHW_RENDER_ENGINE_CAPS GetHwCaps(
        PRENDERHAL_INTERFACE         pRenderHal);

    bool IsComputeContextInUse(
         PRENDERHAL_INTERFACE    pRenderHal) override;

    //!
    //! \brief    Check if Sampler128Elements is supported
    //! \return   true of false
    //!
    bool IsSampler128ElementsSupported() override;

    //!
    //! \brief      Checks how per thread scratch space size bits in VFE state are interpreted by HW
    //! \details    For BDW GT1/2/3 A0 steppings, per thread scratch space size in VFE state
    //!             is 11 bits indicating [2k bytes, 2 Mbytes]: 0=2k, 1=4k, 2=8k ... 10=2M
    //!             BDW+ excluding A0 step is 12 bits indicating [1k bytes, 2 Mbytes]: 0=1k, 1=2k, 2=4k, 3=8k ... 11=2M
    //! \param      PRENDERHAL_INTERFACE pRenderHal
    //!             [in]    Pointer to RenderHal interface
    //! \return     true if BDW A0 stepping, false otherwise
    //!
    bool PerThreadScratchSpaceStart2K(PRENDERHAL_INTERFACE pRenderHal) override;

    //!
    //! \brief      Checks how per thread scratch space size bits in VFE state are interpreted by HW.
    //! \details    per thread scratch space size can be 2^n (n >= 6) bytes.
    //! \param      PRENDERHAL_INTERFACE pRenderHal
    //!             [in]    Pointer to RenderHal interface
    //! \return     bool.
    //!
    bool PerThreadScratchSpaceStart64Byte(RENDERHAL_INTERFACE *renderHal) override;

    //!
    //! \brief    Encode SLM Size for Interface Descriptor
    //! \details  Setup SLM size
    //! \param    uint32_t SLMSize
    //!           [in] SLM size
    //! \return   encoded output
    //!
    uint32_t EncodeSLMSize(uint32_t SLMSize) override;

    //!
    //! \brief    Calculate Preferred Slm Allocation Size for Interface Descriptor
    //! \details  Setup Preferred Slm Allocation Size size
    //! \param    PRENDERHAL_INTERFACE pRenderHal
    //!           [in]    Pointer to RenderHal interface
    //! \param    uint32_t SLMSize
    //!           [in] SLM size
    //! \return   Preferred Slm Allocation Size
    //!
    virtual uint32_t CalculatePreferredSlmAllocationSizeFromSlmSize(
        RENDERHAL_INTERFACE *renderHal, 
        uint32_t             slmSize, 
        uint32_t             numberOfThreadsPerThreadGroup);

    //!
    //! \brief    Set Chroma Direction
    //! \details  Setup Chroma Direction for hpg_base
    //! \param    PRENDERHAL_INTERFACE pRenderHal
    //!           [in]  Pointer to Hardware Interface
    //! \param    PRENDERHAL_SURFACE pRenderHalSurface
    //!           [in]  Pointer to Render Hal Surface
    //! \return   uint8_t
    //!
    uint8_t SetChromaDirection(
        PRENDERHAL_INTERFACE pRenderHal,
        PRENDERHAL_SURFACE   pRenderHalSurface) override;

    //!
    //! \brief    Initialize the default surface type and advanced surface type  per platform
    //! \param    PRENDERHAL_INTERFACE    pRenderHal
    //!           [out] Pointer to PRENDERHAL_INTERFACE
    //! \return   void
    //!
    void InitSurfaceTypes(PRENDERHAL_INTERFACE pRenderHal) override;

    //!
    //! \brief    Check if YV12 Single Pass is supported
    //! \param    PRENDERHAL_INTERFACE pRenderHal
    //!           [in]  Pointer to Hardware Interface
    //! \return   true of false
    //!
    bool IsEnableYV12SinglePass(PRENDERHAL_INTERFACE pRenderHal) override;

    //!
    //! \brief     Get the Size of AVS Sampler State
    //! \param    PRENDERHAL_INTERFACE pRenderHal
    //!           [in]  Pointer to Hardware Interface
    //! \return   size
    //!
    uint32_t GetSizeSamplerStateAvs(PRENDERHAL_INTERFACE pRenderHal) override;

    //!
    //! \brief    Set Power Option Status
    //! \param    [in] pRenderHal
    //!           Pointer to Hardware Interface
    //! \param    [in,out] pCmdBuffer
    //!           Pointer to Command Buffer
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetPowerOptionStatus(
        PRENDERHAL_INTERFACE pRenderHal,
        PMOS_COMMAND_BUFFER  pCmdBuffer) override;

    //!
    //! \brief    Set Composite Prolog CMD
    //! \param    [in] pRenderHal
    //!           Pointer to Hardware Interface
    //! \param    [in,out] pCmdBuffer
    //!           Pointer to Command Buffer
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetCompositePrologCmd(
        PRENDERHAL_INTERFACE pRenderHal,
        PMOS_COMMAND_BUFFER  pCmdBuffer) override;

    //!
    //! \brief    Send Compute Walker
    //! \details  Send Compute Walker
    //! \param    PRENDERHAL_INTERFACE pRenderHal
    //!           [in] Pointer to Hardware Interface Structure
    //! \param    PMOS_COMMAND_BUFFER pCmdBuffer
    //!           [in] Pointer to Command Buffer
    //! \param    PRENDERHAL_GPGPU_WALKER_PARAMS pGpGpuWalkerParams
    //!           [in]    Pointer to GPGPU walker parameters
    //! \return   MOS_STATUS
    //!
    MOS_STATUS SendComputeWalker(
        PRENDERHAL_INTERFACE     pRenderHal,
        PMOS_COMMAND_BUFFER      pCmdBuffer,
        PMHW_GPGPU_WALKER_PARAMS pGpGpuWalkerParams) override;

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

    //!
    //! \brief    Get Render Engine MMC Enable/Disable Flag
    //! \param    [in] pRenderHal
    //!           Pointer to Hardware Interface
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS IsRenderHalMMCEnabled(
        PRENDERHAL_INTERFACE pRenderHal) override;

    //!
    //! \brief    Check if Over ride is needed or not
    //! \param    [in] pRenderHal
    //!           Pointer to Hardware Interface
    //! \param    [in,out] pCmdBuffer
    //!           Pointer to Command Buffer
    //! \param    [in] pGenericPrologParam
    //!           Pointer to MHW generic prolog parameters
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS IsOvrdNeeded(
        PRENDERHAL_INTERFACE             pRenderHal,
        PMOS_COMMAND_BUFFER              pCmdBuffer,
        PRENDERHAL_GENERIC_PROLOG_PARAMS pGenericPrologParams) override;

    //!
    //! \brief    Allocates scratch space buffer.
    //! \details  A single scratch space buffer is allocated and used for all threads.
    //! \param    [in] perThreadScratchSpace
    //!           scratch space
    //! \param    [in] renderHal
    //!           Pointer to Hardware Interface
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS AllocateScratchSpaceBuffer(
        uint32_t                perThreadScratchSpace,
        RENDERHAL_INTERFACE     *renderHal) override;

    //!
    //! \brief    Frees scratch space buffer.
    //! \details  A single scratch space buffer is allocated and used for all threads.
    //! \param    [in] renderHal
    //!           Pointer to Hardware Interface
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS FreeScratchSpaceBuffer(
        RENDERHAL_INTERFACE *renderHal) override;

    //!
    //! \brief      Get Surface Compression support caps
    //! \param      [in] format
    //!             surface format
    //! \return     bool
    //!             true or false
    //!
    virtual bool IsFormatMMCSupported(MOS_FORMAT format);

    //!
    //! \brief      Is L8 format support
    //! \return     bool
    //!             true or false
    //!
    virtual bool IsL8FormatSupported();

    std::shared_ptr<mhw::mi::Itf> GetMhwMiItf();

    MHW_SETPAR_DECL_HDR(STATE_BASE_ADDRESS);

    MHW_SETPAR_DECL_HDR(_3DSTATE_CHROMA_KEY);

    MHW_SETPAR_DECL_HDR(STATE_SIP);

    MHW_SETPAR_DECL_HDR(COMPUTE_WALKER);

    MHW_SETPAR_DECL_HDR(_3DSTATE_BINDING_TABLE_POOL_ALLOC);

protected:

    PRENDERHAL_INTERFACE              m_renderHal = nullptr;
    MediaFeatureManager               *m_featureManager = nullptr;
    PMHW_GPGPU_WALKER_PARAMS          m_gpgpuWalkerParams = nullptr;
    PMHW_ID_ENTRY_PARAMS              m_interfaceDescriptorParams = nullptr;
    bool                              m_renderHalMMCEnabled = false;
    MOS_RESOURCE                      m_scratchSpaceResource;
    std::shared_ptr<mhw::render::Itf> m_renderItf = nullptr;
    std::shared_ptr<mhw::mi::Itf>     m_miItf     = nullptr;

MEDIA_CLASS_DEFINE_END(XRenderHal_Platform_Interface_Next)
};

#endif // __RENDERHAL_PLATFORM_INTERFACE_NEXT_H__
