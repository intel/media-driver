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
//! \file      renderhal_platform_interface_legacy.h 
//! \brief     abstract the platfrom specific APIs into one class 
//!
//!
//! \file     renderhal_platform_interface_legacy.h
//! \brief    Render Engine Interfaces shared across platforms
//! \details  Platform Independent Hardware Interfaces
//!
#ifndef __RENDERHAL_PLATFORM_INTERFACE_LEGACY_H__
#define __RENDERHAL_PLATFORM_INTERFACE_LEGACY_H__

#include "mos_os.h"
#include "renderhal_legacy.h"
#include "renderhal_platform_interface.h"
#include "media_interfaces_mhw.h"

class XRenderHal_Platform_Interface_Legacy : public XRenderHal_Platform_Interface
{
public:
    XRenderHal_Platform_Interface_Legacy() {}
    virtual ~XRenderHal_Platform_Interface_Legacy() {}

    //! \brief    Add Pipeline SelectCmd
    //! \details  Add Pipeline SelectCmd
    //! \param    PRENDERHAL_INTERFACE pRenderHal
    //!           [in] Pointer to RenderHal Interface Structure
    //! \param    PMOS_COMMAND_BUFFER pCmdBuffer
    //!           [in] Pointer to Command Buffer
    //! \return   MOS_STATUS
    MOS_STATUS AddPipelineSelectCmd(
        PRENDERHAL_INTERFACE        pRenderHal,
        PMOS_COMMAND_BUFFER         pCmdBuffer,
        bool                        gpGpuPipe);

    //! \brief    Send StateBase Address
    //! \details  Send StateBase Address
    //! \param    PRENDERHAL_INTERFACE pRenderHal
    //!           [in] Pointer to RenderHal Interface Structure
    //! \param    PMOS_COMMAND_BUFFER pCmdBuffer
    //!           [in] Pointer to Command Buffer
    //! \return   MOS_STATUS
    MOS_STATUS SendStateBaseAddress(
        PRENDERHAL_INTERFACE        pRenderHal,
        PMOS_COMMAND_BUFFER         pCmdBuffer);

    //! \brief    Add Sip State Cmd
    //! \details  Add Sip State Cmd
    //! \param    PRENDERHAL_INTERFACE pRenderHal
    //!           [in] Pointer to RenderHal Interface Structure
    //! \param    PMOS_COMMAND_BUFFER pCmdBuffer
    //!           [in] Pointer to Command Buffer
    //! \return   MOS_STATUS
    MOS_STATUS AddSipStateCmd(
        PRENDERHAL_INTERFACE        pRenderHal,
        PMOS_COMMAND_BUFFER         pCmdBuffer);

    //! \brief    Add Cfe State Cmd
    //! \details  Add Cfe State Cmd
    //! \param    PRENDERHAL_INTERFACE pRenderHal
    //!           [in] Pointer to RenderHal Interface Structure
    //! \param    PMOS_COMMAND_BUFFER pCmdBuffer
    //!           [in] Pointer to Command Buffer
    //! \return   MOS_STATUS
    MOS_STATUS AddCfeStateCmd(
        PRENDERHAL_INTERFACE        pRenderHal,
        PMOS_COMMAND_BUFFER         pCmdBuffer,
        PMHW_VFE_PARAMS             params);

    //! \brief    Send ChromaKey
    //! \details  Send ChromaKey
    //! \param    PRENDERHAL_INTERFACE pRenderHal
    //!           [in] Pointer to RenderHal Interface Structure
    //! \param    PMOS_COMMAND_BUFFER pCmdBuffer
    //!           [in] Pointer to Command Buffer
    //! \return   MOS_STATUS
    MOS_STATUS SendChromaKey(
        PRENDERHAL_INTERFACE        pRenderHal,
        PMOS_COMMAND_BUFFER         pCmdBuffer,
        PMHW_CHROMAKEY_PARAMS       pChromaKeyParams);

    //! \brief    Send Palette
    //! \details  Send Palette
    //! \param    PRENDERHAL_INTERFACE pRenderHal
    //!           [in] Pointer to RenderHal Interface Structure
    //! \param    PMOS_COMMAND_BUFFER pCmdBuffer
    //!           [in] Pointer to Command Buffer
    //! \return   MOS_STATUS
    MOS_STATUS SendPalette(
        PRENDERHAL_INTERFACE        pRenderHal,
        PMOS_COMMAND_BUFFER         pCmdBuffer,
        PMHW_PALETTE_PARAMS         pPaletteLoadParams);

    //! \brief    Set L3Cache
    //! \details  Set L3Cache
    //! \param    PRENDERHAL_INTERFACE pRenderHal
    //!           [in] Pointer to RenderHal Interface Structure
    //! \param    PMOS_COMMAND_BUFFER pCmdBuffer
    //!           [in] Pointer to Command Buffer
    //! \return   MOS_STATUS
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
        PRENDERHAL_INTERFACE       pRenderHal,
        PMOS_COMMAND_BUFFER        pCmdBuffer,
        MHW_PIPE_CONTROL_PARAMS*   params);

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
        PMOS_INTERFACE     pOsInterface,
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
        MOS_INTERFACE        *osInterface,
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

    std::shared_ptr<mhw::mi::Itf> GetMhwMiItf();

};

#endif // __RENDERHAL_PLATFORM_INTERFACE_LEGACY_H__
