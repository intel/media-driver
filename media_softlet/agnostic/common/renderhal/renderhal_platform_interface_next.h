/*
* Copyright (c) 2021, Intel Corporation
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
    XRenderHal_Platform_Interface_Next() {}
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
        PMOS_CONTEXT         pOsContext,
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
         PRENDERHAL_INTERFACE    pRenderHal);

    std::shared_ptr<mhw::mi::Itf> GetMhwMiItf();

protected:

    PRENDERHAL_INTERFACE              m_renderHal = nullptr;
    MediaFeatureManager               *m_featureManager = nullptr;
    PMHW_GPGPU_WALKER_PARAMS          m_gpgpuWalkerParams = nullptr;
    PMHW_ID_ENTRY_PARAMS              m_interfaceDescriptorParams = nullptr;
    std::shared_ptr<mhw::render::Itf> m_renderItf = nullptr;
    std::shared_ptr<mhw::mi::Itf>     m_miItf     = nullptr;

MEDIA_CLASS_DEFINE_END(XRenderHal_Platform_Interface_Next)
};

#endif // __RENDERHAL_PLATFORM_INTERFACE_NEXT_H__
