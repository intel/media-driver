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
//! \file      renderhal_platform_interface_legacy.cpp
//! \brief     abstract the platfrom specific APIs into one class
//!
//!
//! \file     renderhal_platform_interface_legacy.cpp
//! \brief    Render Engine Interfaces shared across platforms
//! \details  Platform Independent Hardware Interfaces
//!

#include "renderhal_platform_interface_legacy.h"
#include "hal_oca_interface.h"

//! \brief    Add Pipeline SelectCmd
//! \details  Add Pipeline SelectCmd
//! \param    PRENDERHAL_INTERFACE pRenderHal
//!           [in] Pointer to RenderHal Interface Structure
//! \param    PMOS_COMMAND_BUFFER pCmdBuffer
//!           [in] Pointer to Command Buffer
//! \return   MOS_STATUS
MOS_STATUS XRenderHal_Platform_Interface_Legacy::AddPipelineSelectCmd(
    PRENDERHAL_INTERFACE        pRenderHal,
    PMOS_COMMAND_BUFFER         pCmdBuffer,
    bool                        gpGpuPipe)
{
    MOS_STATUS                  eStatus          = MOS_STATUS_SUCCESS;
    PRENDERHAL_INTERFACE_LEGACY pRenderHalLegacy = (PRENDERHAL_INTERFACE_LEGACY)pRenderHal;

    MHW_RENDERHAL_CHK_NULL(pRenderHalLegacy);
    MHW_RENDERHAL_CHK_NULL(pRenderHalLegacy->pMhwRenderInterface);

    MHW_RENDERHAL_CHK_STATUS(pRenderHalLegacy->pMhwRenderInterface->AddPipelineSelectCmd(pCmdBuffer, gpGpuPipe));

finish:
    return eStatus;
}

//! \brief    Send StateBase Address
//! \details  Send StateBase Address
//! \param    PRENDERHAL_INTERFACE pRenderHal
//!           [in] Pointer to RenderHal Interface Structure
//! \param    PMOS_COMMAND_BUFFER pCmdBuffer
//!           [in] Pointer to Command Buffer
//! \return   MOS_STATUS
MOS_STATUS XRenderHal_Platform_Interface_Legacy::SendStateBaseAddress(
    PRENDERHAL_INTERFACE        pRenderHal,
    PMOS_COMMAND_BUFFER         pCmdBuffer)
{
    MOS_STATUS                  eStatus          = MOS_STATUS_SUCCESS;
    PRENDERHAL_INTERFACE_LEGACY pRenderHalLegacy = (PRENDERHAL_INTERFACE_LEGACY)pRenderHal;
    
    MHW_RENDERHAL_CHK_NULL(pRenderHalLegacy);

    eStatus    = pRenderHalLegacy->pMhwRenderInterface->AddStateBaseAddrCmd(pCmdBuffer,
                                                 &pRenderHalLegacy->StateBaseAddressParams);

finish:
    return eStatus;
}

//! \brief    Add Sip State Cmd
//! \details  Add Sip State Cmd
//! \param    PRENDERHAL_INTERFACE pRenderHal
//!           [in] Pointer to RenderHal Interface Structure
//! \param    PMOS_COMMAND_BUFFER pCmdBuffer
//!           [in] Pointer to Command Buffer
//! \return   MOS_STATUS
MOS_STATUS XRenderHal_Platform_Interface_Legacy::AddSipStateCmd(
    PRENDERHAL_INTERFACE        pRenderHal,
    PMOS_COMMAND_BUFFER         pCmdBuffer)
{
    MOS_STATUS                  eStatus          = MOS_STATUS_SUCCESS;
    PRENDERHAL_INTERFACE_LEGACY pRenderHalLegacy = (PRENDERHAL_INTERFACE_LEGACY)pRenderHal;

    MHW_RENDERHAL_CHK_NULL(pRenderHalLegacy);
    MHW_RENDERHAL_CHK_NULL(pRenderHalLegacy->pMhwRenderInterface);

    MHW_RENDERHAL_CHK_STATUS(pRenderHalLegacy->pMhwRenderInterface->AddSipStateCmd(pCmdBuffer, &pRenderHalLegacy->SipStateParams));

finish:
    return eStatus;
}

//! \brief    Add Cfe State Cmd
//! \details  Add Cfe State Cmd
//! \param    PRENDERHAL_INTERFACE pRenderHal
//!           [in] Pointer to RenderHal Interface Structure
//! \param    PMOS_COMMAND_BUFFER pCmdBuffer
//!           [in] Pointer to Command Buffer
//! \return   MOS_STATUS
MOS_STATUS XRenderHal_Platform_Interface_Legacy::AddCfeStateCmd(
    PRENDERHAL_INTERFACE        pRenderHal,
    PMOS_COMMAND_BUFFER         pCmdBuffer,
    PMHW_VFE_PARAMS             params)
{
    MOS_STATUS                  eStatus          = MOS_STATUS_SUCCESS;
    PRENDERHAL_INTERFACE_LEGACY pRenderHalLegacy = (PRENDERHAL_INTERFACE_LEGACY)pRenderHal;
    MHW_RENDERHAL_CHK_NULL(pRenderHalLegacy);
    MHW_RENDERHAL_CHK_NULL(pRenderHalLegacy->pMhwRenderInterface);

    MHW_RENDERHAL_CHK_STATUS(pRenderHalLegacy->pMhwRenderInterface->AddCfeStateCmd(pCmdBuffer, params));

finish:
    return eStatus;
};

//! \brief    Send ChromaKey
//! \details  Send ChromaKey
//! \param    PRENDERHAL_INTERFACE pRenderHal
//!           [in] Pointer to RenderHal Interface Structure
//! \param    PMOS_COMMAND_BUFFER pCmdBuffer
//!           [in] Pointer to Command Buffer
//! \return   MOS_STATUS
MOS_STATUS XRenderHal_Platform_Interface_Legacy::SendChromaKey(
    PRENDERHAL_INTERFACE        pRenderHal,
    PMOS_COMMAND_BUFFER         pCmdBuffer,
    PMHW_CHROMAKEY_PARAMS       pChromaKeyParams)
{
    MOS_STATUS                  eStatus          = MOS_STATUS_SUCCESS;
    PRENDERHAL_INTERFACE_LEGACY pRenderHalLegacy = (PRENDERHAL_INTERFACE_LEGACY)pRenderHal;
    MHW_RENDERHAL_CHK_NULL(pRenderHalLegacy);

    MHW_RENDERHAL_CHK_STATUS(pRenderHalLegacy->pMhwRenderInterface->AddChromaKeyCmd(pCmdBuffer, pChromaKeyParams));
finish:
    return eStatus;
}

//! \brief    Send Palette
//! \details  Send Palette
//! \param    PRENDERHAL_INTERFACE pRenderHal
//!           [in] Pointer to RenderHal Interface Structure
//! \param    PMOS_COMMAND_BUFFER pCmdBuffer
//!           [in] Pointer to Command Buffer
//! \return   MOS_STATUS
MOS_STATUS XRenderHal_Platform_Interface_Legacy::SendPalette(
    PRENDERHAL_INTERFACE        pRenderHal,
    PMOS_COMMAND_BUFFER         pCmdBuffer,
    PMHW_PALETTE_PARAMS         pPaletteLoadParams)
{
    MOS_STATUS                  eStatus          = MOS_STATUS_SUCCESS;
    PRENDERHAL_INTERFACE_LEGACY pRenderHalLegacy = (PRENDERHAL_INTERFACE_LEGACY)pRenderHal;
    MHW_RENDERHAL_CHK_NULL(pRenderHalLegacy);
    MHW_RENDERHAL_CHK_NULL(pRenderHalLegacy->pMhwRenderInterface);

    MHW_RENDERHAL_CHK_STATUS(pRenderHalLegacy->pMhwRenderInterface->AddPaletteLoadCmd(pCmdBuffer, pPaletteLoadParams));

finish:
    return eStatus;
};

//! \brief    Set L3Cache
//! \details  Set L3Cache
//! \param    PRENDERHAL_INTERFACE pRenderHal
//!           [in] Pointer to RenderHal Interface Structure
//! \param    PMOS_COMMAND_BUFFER pCmdBuffer
//!           [in] Pointer to Command Buffer
//! \return   MOS_STATUS
MOS_STATUS XRenderHal_Platform_Interface_Legacy::SetL3Cache(
    PRENDERHAL_INTERFACE        pRenderHal,
    PMOS_COMMAND_BUFFER         pCmdBuffer)
{
    MOS_STATUS                  eStatus          = MOS_STATUS_SUCCESS;
    PRENDERHAL_INTERFACE_LEGACY pRenderHalLegacy = (PRENDERHAL_INTERFACE_LEGACY)pRenderHal;
    MHW_RENDERHAL_CHK_NULL(pRenderHalLegacy);
    MHW_RENDERHAL_CHK_NULL(pRenderHalLegacy->pMhwRenderInterface);

    MHW_RENDERHAL_CHK_STATUS(pRenderHalLegacy->pMhwRenderInterface->SetL3Cache(pCmdBuffer));

finish:
    return eStatus;
}

//!
//! \brief    Get the size of render hal media state
//! \return   size_t
//!           The size of render hal media state
//!
size_t XRenderHal_Platform_Interface_Legacy::GetRenderHalMediaStateSize()
{
    return sizeof(RENDERHAL_MEDIA_STATE_LEGACY);
}

//!
//! \brief    Get the size of render hal state heap
//! \return   size_t
//!           The size of render hal state heap
//!
size_t XRenderHal_Platform_Interface_Legacy::GetRenderHalStateHeapSize()
{
    return sizeof(RENDERHAL_STATE_HEAP_LEGACY);
}

PMHW_MI_MMIOREGISTERS XRenderHal_Platform_Interface_Legacy::GetMmioRegisters(
    PRENDERHAL_INTERFACE        pRenderHal)
{
    PMHW_MI_MMIOREGISTERS       pMmioRegisters   = nullptr;
    PRENDERHAL_INTERFACE_LEGACY pRenderHalLegacy = (PRENDERHAL_INTERFACE_LEGACY)pRenderHal;
    if (pRenderHalLegacy && pRenderHalLegacy->pMhwRenderInterface)
    {
        pMmioRegisters = pRenderHalLegacy->pMhwRenderInterface->GetMmioRegisters();
    }

    return pMmioRegisters;
};

MOS_STATUS XRenderHal_Platform_Interface_Legacy::EnablePreemption(
    PRENDERHAL_INTERFACE            pRenderHal,
    PMOS_COMMAND_BUFFER             pCmdBuffer)
{
    MOS_STATUS                  eStatus          = MOS_STATUS_SUCCESS;
    PRENDERHAL_INTERFACE_LEGACY pRenderHalLegacy = (PRENDERHAL_INTERFACE_LEGACY)pRenderHal;
    MHW_RENDERHAL_CHK_NULL(pRenderHalLegacy);
    MHW_RENDERHAL_CHK_NULL(pRenderHalLegacy->pMhwRenderInterface);

    MHW_RENDERHAL_CHK_STATUS(pRenderHalLegacy->pMhwRenderInterface->EnablePreemption(pCmdBuffer));

finish:
    return eStatus;
}

MOS_STATUS XRenderHal_Platform_Interface_Legacy::SendPredicationCommand(
    PRENDERHAL_INTERFACE        pRenderHal,
    PMOS_COMMAND_BUFFER         pCmdBuffer)
{
    MOS_STATUS  eStatus = MOS_STATUS_SUCCESS;

    //-----------------------------------------
    MHW_RENDERHAL_CHK_NULL(pRenderHal);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pOsInterface);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pMhwMiInterface);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pMhwMiInterface->GetMmioRegisters());
    //-----------------------------------------

    MHW_MI_CONDITIONAL_BATCH_BUFFER_END_PARAMS  condBBEndParams;
    MOS_ZeroMemory(&condBBEndParams, sizeof(condBBEndParams));

    MOS_SYNC_PARAMS syncParams;
    MOS_ZeroMemory(&syncParams, sizeof(syncParams));
    syncParams.uiSemaphoreCount = 1;
    // Currently only sync between VEBOX and 3D, also need to consider sync between Render Engine and 3D
    // low priority since current VP Predication test case does not cover this scenario.
    syncParams.GpuContext = MOS_GPU_CONTEXT_VEBOX;
    syncParams.presSyncResource = pRenderHal->PredicationParams.pPredicationResource;
    syncParams.bReadOnly = true;
    syncParams.bDisableDecodeSyncLock = false;
    syncParams.bDisableLockForTranscode = false;

    MHW_RENDERHAL_CHK_STATUS(pRenderHal->pOsInterface->pfnPerformOverlaySync(pRenderHal->pOsInterface, &syncParams));
    MHW_RENDERHAL_CHK_STATUS(pRenderHal->pOsInterface->pfnResourceWait(pRenderHal->pOsInterface, &syncParams));

    // This function is only support VEBox right now, since register returned by pMhwMiInterface->GetMmioRegisters() is for VEBox.
    // Keep implementation same between Render and VEBox engines - for Render it is highly inefficient
    // Skip current frame if presPredication is not equal to zero
    if (pRenderHal->PredicationParams.predicationNotEqualZero)
    {
        auto mmioRegistersRender = pRenderHal->pMhwMiInterface->GetMmioRegisters();
        MHW_MI_FLUSH_DW_PARAMS  flushDwParams;
        MOS_ZeroMemory(&flushDwParams, sizeof(flushDwParams));
        MHW_RENDERHAL_CHK_STATUS(pRenderHal->pMhwMiInterface->AddMiFlushDwCmd(pCmdBuffer, &flushDwParams));

        // load presPredication to general purpose register0
        MHW_MI_STORE_REGISTER_MEM_PARAMS    loadRegisterMemParams;
        MOS_ZeroMemory(&loadRegisterMemParams, sizeof(loadRegisterMemParams));
        loadRegisterMemParams.presStoreBuffer = pRenderHal->PredicationParams.pPredicationResource;
        loadRegisterMemParams.dwOffset = (uint32_t)pRenderHal->PredicationParams.predicationResOffset;
        loadRegisterMemParams.dwRegister = mmioRegistersRender->generalPurposeRegister0LoOffset;
        MHW_RENDERHAL_CHK_STATUS(pRenderHal->pMhwMiInterface->AddMiLoadRegisterMemCmd(
            pCmdBuffer,
            &loadRegisterMemParams));

        MHW_MI_LOAD_REGISTER_IMM_PARAMS     loadRegisterImmParams;
        MOS_ZeroMemory(&loadRegisterImmParams, sizeof(loadRegisterImmParams));
        loadRegisterImmParams.dwData = 0;
        loadRegisterImmParams.dwRegister = mmioRegistersRender->generalPurposeRegister0HiOffset;
        MHW_RENDERHAL_CHK_STATUS(pRenderHal->pMhwMiInterface->AddMiLoadRegisterImmCmd(
            pCmdBuffer,
            &loadRegisterImmParams));

        // load 0 to general purpose register4
        MOS_ZeroMemory(&loadRegisterImmParams, sizeof(loadRegisterImmParams));
        loadRegisterImmParams.dwData = 0;
        loadRegisterImmParams.dwRegister = mmioRegistersRender->generalPurposeRegister4LoOffset;
        MHW_RENDERHAL_CHK_STATUS(pRenderHal->pMhwMiInterface->AddMiLoadRegisterImmCmd(
            pCmdBuffer,
            &loadRegisterImmParams));

        MOS_ZeroMemory(&loadRegisterImmParams, sizeof(loadRegisterImmParams));
        loadRegisterImmParams.dwData = 0;
        loadRegisterImmParams.dwRegister = mmioRegistersRender->generalPurposeRegister4HiOffset;
        MHW_RENDERHAL_CHK_STATUS(pRenderHal->pMhwMiInterface->AddMiLoadRegisterImmCmd(
            pCmdBuffer,
            &loadRegisterImmParams));

        //perform the add operation
        MHW_MI_MATH_PARAMS  miMathParams;
        MHW_MI_ALU_PARAMS   miAluParams[4];
        MOS_ZeroMemory(&miMathParams, sizeof(miMathParams));
        MOS_ZeroMemory(&miAluParams, sizeof(miAluParams));
        // load     srcA, reg0
        miAluParams[0].AluOpcode = MHW_MI_ALU_LOAD;
        miAluParams[0].Operand1 = MHW_MI_ALU_SRCA;
        miAluParams[0].Operand2 = MHW_MI_ALU_GPREG0;
        // load     srcB, reg4
        miAluParams[1].AluOpcode = MHW_MI_ALU_LOAD;
        miAluParams[1].Operand1 = MHW_MI_ALU_SRCB;
        miAluParams[1].Operand2 = MHW_MI_ALU_GPREG4;
        // add      srcA, srcB
        miAluParams[2].AluOpcode = MHW_MI_ALU_ADD;
        miAluParams[2].Operand1 = MHW_MI_ALU_SRCB;
        miAluParams[2].Operand2 = MHW_MI_ALU_GPREG4;
        // store      reg0, ZF
        miAluParams[3].AluOpcode = MHW_MI_ALU_STORE;
        miAluParams[3].Operand1 = MHW_MI_ALU_GPREG0;
        miAluParams[3].Operand2 = MHW_MI_ALU_ZF;
        miMathParams.pAluPayload = miAluParams;
        miMathParams.dwNumAluParams = 4; // four ALU commands needed for this substract opertaion. see following ALU commands.
        MHW_RENDERHAL_CHK_STATUS(pRenderHal->pMhwMiInterface->AddMiMathCmd(
            pCmdBuffer,
            &miMathParams));

        // if zero, the zero flag will be 0xFFFFFFFF, else zero flag will be 0x0.
        MHW_MI_STORE_REGISTER_MEM_PARAMS    storeRegParams;
        MOS_ZeroMemory(&storeRegParams, sizeof(storeRegParams));
        storeRegParams.presStoreBuffer = &pRenderHal->PredicationBuffer;
        storeRegParams.dwOffset = 0x10;
        storeRegParams.dwRegister = mmioRegistersRender->generalPurposeRegister0LoOffset;
        MHW_RENDERHAL_CHK_STATUS(pRenderHal->pMhwMiInterface->AddMiStoreRegisterMemCmd(
            pCmdBuffer,
            &storeRegParams));

        // Programming of 4 dummy MI_STORE_DATA_IMM commands prior to programming of MiConditionalBatchBufferEnd
        MHW_MI_STORE_DATA_PARAMS dataParams;
        MOS_ZeroMemory(&dataParams, sizeof(dataParams));
        dataParams.pOsResource = &pRenderHal->PredicationBuffer;
        dataParams.dwValue = 1;
        MHW_RENDERHAL_CHK_STATUS(pRenderHal->pMhwMiInterface->AddMiStoreDataImmCmd(
            pCmdBuffer,
            &dataParams));

        dataParams.dwValue = 2;
        MHW_RENDERHAL_CHK_STATUS(pRenderHal->pMhwMiInterface->AddMiStoreDataImmCmd(
            pCmdBuffer,
            &dataParams));

        dataParams.dwValue = 3;
        MHW_RENDERHAL_CHK_STATUS(pRenderHal->pMhwMiInterface->AddMiStoreDataImmCmd(
            pCmdBuffer,
            &dataParams));

        dataParams.dwValue = 4;
        MHW_RENDERHAL_CHK_STATUS(pRenderHal->pMhwMiInterface->AddMiStoreDataImmCmd(
            pCmdBuffer,
            &dataParams));

        MOS_ZeroMemory(&flushDwParams, sizeof(flushDwParams));
        flushDwParams.postSyncOperation = 1;
        flushDwParams.pOsResource = &pRenderHal->PredicationBuffer;
        MHW_RENDERHAL_CHK_STATUS(pRenderHal->pMhwMiInterface->AddMiFlushDwCmd(pCmdBuffer, &flushDwParams));

        condBBEndParams.presSemaphoreBuffer = &pRenderHal->PredicationBuffer;
        condBBEndParams.dwOffset = 0x10;
        condBBEndParams.dwValue = 0;
        condBBEndParams.bDisableCompareMask = true;
        MHW_RENDERHAL_CHK_STATUS(pRenderHal->pMhwMiInterface->AddMiConditionalBatchBufferEndCmd(
            pCmdBuffer,
            &condBBEndParams));

        pRenderHal->PredicationParams.ptempPredicationBuffer = &pRenderHal->PredicationBuffer;
    }
    else
    {
        auto mmioRegistersRender = pRenderHal->pMhwMiInterface->GetMmioRegisters();

        MHW_MI_FLUSH_DW_PARAMS  flushDwParams;
        MOS_ZeroMemory(&flushDwParams, sizeof(flushDwParams));
        MHW_RENDERHAL_CHK_STATUS(pRenderHal->pMhwMiInterface->AddMiFlushDwCmd(pCmdBuffer, &flushDwParams));

        // load presPredication to general purpose register0
        MHW_MI_STORE_REGISTER_MEM_PARAMS    loadRegisterMemParams;
        MOS_ZeroMemory(&loadRegisterMemParams, sizeof(loadRegisterMemParams));
        loadRegisterMemParams.presStoreBuffer = pRenderHal->PredicationParams.pPredicationResource;
        loadRegisterMemParams.dwOffset = (uint32_t)pRenderHal->PredicationParams.predicationResOffset;
        loadRegisterMemParams.dwRegister = mmioRegistersRender->generalPurposeRegister0LoOffset;
        MHW_RENDERHAL_CHK_STATUS(pRenderHal->pMhwMiInterface->AddMiLoadRegisterMemCmd(
            pCmdBuffer,
            &loadRegisterMemParams));

        // if zero, the zero flag will be 0xFFFFFFFF, else zero flag will be 0x0.
        MHW_MI_STORE_REGISTER_MEM_PARAMS    storeRegParams;
        MOS_ZeroMemory(&storeRegParams, sizeof(storeRegParams));
        storeRegParams.presStoreBuffer = &pRenderHal->PredicationBuffer;
        storeRegParams.dwOffset = 0x10;
        storeRegParams.dwRegister = mmioRegistersRender->generalPurposeRegister0LoOffset;
        MHW_RENDERHAL_CHK_STATUS(pRenderHal->pMhwMiInterface->AddMiStoreRegisterMemCmd(
            pCmdBuffer,
            &storeRegParams));

        // Programming of 4 dummy MI_STORE_DATA_IMM commands prior to programming of MiConditionalBatchBufferEnd
        MHW_MI_STORE_DATA_PARAMS dataParams;
        MOS_ZeroMemory(&dataParams, sizeof(dataParams));
        dataParams.pOsResource = &pRenderHal->PredicationBuffer;
        dataParams.dwValue = 1;
        MHW_RENDERHAL_CHK_STATUS(pRenderHal->pMhwMiInterface->AddMiStoreDataImmCmd(
            pCmdBuffer,
            &dataParams));

        dataParams.dwValue = 2;
        MHW_RENDERHAL_CHK_STATUS(pRenderHal->pMhwMiInterface->AddMiStoreDataImmCmd(
            pCmdBuffer,
            &dataParams));

        dataParams.dwValue = 3;
        MHW_RENDERHAL_CHK_STATUS(pRenderHal->pMhwMiInterface->AddMiStoreDataImmCmd(
            pCmdBuffer,
            &dataParams));

        dataParams.dwValue = 4;
        MHW_RENDERHAL_CHK_STATUS(pRenderHal->pMhwMiInterface->AddMiStoreDataImmCmd(
            pCmdBuffer,
            &dataParams));

        MOS_ZeroMemory(&flushDwParams, sizeof(flushDwParams));
        flushDwParams.postSyncOperation = 1;
        flushDwParams.pOsResource = &pRenderHal->PredicationBuffer;
        MHW_RENDERHAL_CHK_STATUS(pRenderHal->pMhwMiInterface->AddMiFlushDwCmd(pCmdBuffer, &flushDwParams));

        // Skip current frame if presPredication is equal to zero
        condBBEndParams.presSemaphoreBuffer = &pRenderHal->PredicationBuffer;
        condBBEndParams.dwOffset = 0x10;
        condBBEndParams.bDisableCompareMask = true;
        condBBEndParams.dwValue = 0;
        MHW_RENDERHAL_CHK_STATUS(pRenderHal->pMhwMiInterface->AddMiConditionalBatchBufferEndCmd(
            pCmdBuffer,
            &condBBEndParams));
    }

finish:
    return eStatus;
}

//!
//! \brief    Adds marker attributes in command buffer
//! \param    PRENDERHAL_INTERFACE pRenderHal
//!           [in] Pointer to RenderHal Interface Structure
//! \param    PMOS_COMMAND_BUFFER pcmdBuffer
//!           [in] Pointer to Command Buffer
//! \param    bool isRender
//!           [in] Flag of Render Engine
//! \return   MOS_STATUS
//!
MOS_STATUS XRenderHal_Platform_Interface_Legacy::SendMarkerCommand(
    PRENDERHAL_INTERFACE    pRenderHal,
    PMOS_COMMAND_BUFFER     cmdBuffer,
    bool                    isRender)
{
    MOS_STATUS  eStatus = MOS_STATUS_SUCCESS;

    //-----------------------------------------
    MHW_RENDERHAL_CHK_NULL(pRenderHal);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pMhwMiInterface);
    //-----------------------------------------

    if (isRender)
    {
        // Send pipe_control to get the timestamp
        MHW_PIPE_CONTROL_PARAMS             pipeControlParams;
        MOS_ZeroMemory(&pipeControlParams, sizeof(pipeControlParams));
        pipeControlParams.presDest         = pRenderHal->SetMarkerParams.pSetMarkerResource;
        pipeControlParams.dwResourceOffset = 0;
        pipeControlParams.dwPostSyncOp     = MHW_FLUSH_WRITE_TIMESTAMP_REG;
        pipeControlParams.dwFlushMode      = MHW_FLUSH_WRITE_CACHE;
        MHW_RENDERHAL_CHK_STATUS(pRenderHal->pMhwMiInterface->AddPipeControl(cmdBuffer, nullptr, &pipeControlParams));
    }
    else
    {
        // Send flush_dw to get the timestamp
        MHW_MI_FLUSH_DW_PARAMS  flushDwParams;
        MOS_ZeroMemory(&flushDwParams, sizeof(flushDwParams));
        flushDwParams.pOsResource           = pRenderHal->SetMarkerParams.pSetMarkerResource;
        flushDwParams.dwResourceOffset      = 0;
        flushDwParams.postSyncOperation     = MHW_FLUSH_WRITE_TIMESTAMP_REG;
        flushDwParams.bQWordEnable          = 1;
        MHW_RENDERHAL_CHK_STATUS(pRenderHal->pMhwMiInterface->AddMiFlushDwCmd(cmdBuffer, &flushDwParams));
}

finish:
    return eStatus;
}

MOS_STATUS XRenderHal_Platform_Interface_Legacy::AddMiPipeControl(
    PRENDERHAL_INTERFACE    pRenderHal,
    PMOS_COMMAND_BUFFER        pCmdBuffer,
    MHW_PIPE_CONTROL_PARAMS* params)
{
    MOS_STATUS  eStatus = MOS_STATUS_SUCCESS;

    //------------------------------------
    MHW_RENDERHAL_CHK_NULL(pRenderHal);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pMhwMiInterface);
    MHW_RENDERHAL_CHK_NULL(pCmdBuffer);
    MHW_RENDERHAL_CHK_NULL(params);
    //------------------------------------

    MHW_RENDERHAL_CHK_STATUS(pRenderHal->pMhwMiInterface->AddPipeControl(pCmdBuffer, nullptr, params));

finish:
    return eStatus;
}

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
MOS_STATUS XRenderHal_Platform_Interface_Legacy::AddMiLoadRegisterImmCmd(
    PRENDERHAL_INTERFACE             pRenderHal,
    PMOS_COMMAND_BUFFER              pCmdBuffer,
    PMHW_MI_LOAD_REGISTER_IMM_PARAMS params)
{
    MOS_STATUS  eStatus = MOS_STATUS_SUCCESS;

    //-----------------------------------------
    MHW_RENDERHAL_CHK_NULL(pRenderHal);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pMhwMiInterface);
    MHW_RENDERHAL_CHK_NULL(pCmdBuffer);
    MHW_RENDERHAL_CHK_NULL(params);
    //-----------------------------------------

    MHW_RENDERHAL_CHK_STATUS(pRenderHal->pMhwMiInterface->AddMiLoadRegisterImmCmd(pCmdBuffer, params));

finish:
    return eStatus;
}

MOS_STATUS XRenderHal_Platform_Interface_Legacy::SendGenericPrologCmd(
    PRENDERHAL_INTERFACE        pRenderHal,
    PMOS_COMMAND_BUFFER         pCmdBuffer,
    PMHW_GENERIC_PROLOG_PARAMS  pParams,
    MHW_MI_MMIOREGISTERS*       pMmioReg)
{
    MOS_STATUS  eStatus = MOS_STATUS_SUCCESS;

    //-----------------------------------------
    MHW_RENDERHAL_CHK_NULL(pRenderHal);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pMhwMiInterface);
    MHW_RENDERHAL_CHK_NULL(pCmdBuffer);
    MHW_RENDERHAL_CHK_NULL(pParams);
    //-----------------------------------------

    MHW_RENDERHAL_CHK_STATUS(Mhw_SendGenericPrologCmd(pCmdBuffer, pParams, pMmioReg));

finish:
    return eStatus;
}

MOS_STATUS XRenderHal_Platform_Interface_Legacy::CreateMhwInterfaces(
    PRENDERHAL_INTERFACE        pRenderHal,
    PMOS_INTERFACE              pOsInterface)
{
    MOS_STATUS  eStatus = MOS_STATUS_SUCCESS;
    PRENDERHAL_INTERFACE_LEGACY pRenderHalLegacy = (PRENDERHAL_INTERFACE_LEGACY)pRenderHal;
    //-----------------------------------------
    MHW_RENDERHAL_CHK_NULL_RETURN(pRenderHalLegacy);
    MHW_RENDERHAL_CHK_NULL_RETURN(pOsInterface);
    //-----------------------------------------

    MhwInterfaces::CreateParams params;
    MOS_ZeroMemory(&params, sizeof(params));
    params.Flags.m_render = true;
    params.m_heapMode = pRenderHalLegacy->bDynamicStateHeap;
    MhwInterfaces *mhwInterfaces =  MhwInterfaces::CreateFactory(params, pOsInterface);
    MHW_RENDERHAL_CHK_NULL_RETURN(mhwInterfaces);
    MHW_RENDERHAL_CHK_NULL_RETURN(mhwInterfaces->m_cpInterface);
    MHW_RENDERHAL_CHK_NULL_RETURN(mhwInterfaces->m_miInterface);
    MHW_RENDERHAL_CHK_NULL_RETURN(mhwInterfaces->m_renderInterface);
    pRenderHalLegacy->pCpInterface = mhwInterfaces->m_cpInterface;
    pRenderHalLegacy->pMhwMiInterface = mhwInterfaces->m_miInterface;
    pRenderHalLegacy->pMhwRenderInterface = mhwInterfaces->m_renderInterface;

    MOS_Delete(mhwInterfaces);

    return eStatus;
}

MOS_STATUS XRenderHal_Platform_Interface_Legacy::On1stLevelBBStart(
    PRENDERHAL_INTERFACE pRenderHal,
    PMOS_COMMAND_BUFFER  pCmdBuffer,
    PMOS_CONTEXT         pOsContext,
    uint32_t             gpuContextHandle,
    MHW_MI_MMIOREGISTERS *pMmioReg)
{
    MOS_STATUS  eStatus = MOS_STATUS_SUCCESS;
    MHW_RENDERHAL_CHK_NULL_RETURN(pRenderHal);
    MHW_RENDERHAL_CHK_NULL_RETURN(pRenderHal->pOsInterface);
    MHW_RENDERHAL_CHK_NULL_RETURN(pRenderHal->pMhwMiInterface);
    MHW_RENDERHAL_CHK_NULL_RETURN(pCmdBuffer);
    MHW_RENDERHAL_CHK_NULL_RETURN(pOsContext);
    MHW_RENDERHAL_CHK_NULL_RETURN(pMmioReg);

    HalOcaInterface::On1stLevelBBStart(*pCmdBuffer, *pOsContext, pRenderHal->pOsInterface->CurrentGpuContextHandle,
        *(pRenderHal->pMhwMiInterface), *pMmioReg);

    return eStatus;
}

MOS_STATUS XRenderHal_Platform_Interface_Legacy::OnDispatch(
    PRENDERHAL_INTERFACE pRenderHal,
    PMOS_COMMAND_BUFFER  pCmdBuffer,
    PMOS_INTERFACE       pOsInterface,
    MHW_MI_MMIOREGISTERS *pMmioReg)
{
    MOS_STATUS  eStatus = MOS_STATUS_SUCCESS;
    MHW_RENDERHAL_CHK_NULL_RETURN(pRenderHal);
    MHW_RENDERHAL_CHK_NULL_RETURN(pRenderHal->pMhwMiInterface);
    MHW_RENDERHAL_CHK_NULL_RETURN(pCmdBuffer);
    MHW_RENDERHAL_CHK_NULL_RETURN(pOsInterface);
    MHW_RENDERHAL_CHK_NULL_RETURN(pMmioReg);

    HalOcaInterface::OnDispatch(*pCmdBuffer, *pOsInterface, *pRenderHal->pMhwMiInterface, *pMmioReg);

    return eStatus;
}

MOS_STATUS XRenderHal_Platform_Interface_Legacy::CreatePerfProfiler(
    PRENDERHAL_INTERFACE pRenderHal)
{
    PRENDERHAL_INTERFACE_LEGACY pRenderHalLegacy = static_cast<PRENDERHAL_INTERFACE_LEGACY>(pRenderHal);
    
    MHW_RENDERHAL_CHK_NULL_RETURN(pRenderHalLegacy);
    MHW_RENDERHAL_CHK_NULL_RETURN(pRenderHalLegacy->pOsInterface);
    
    if (!pRenderHalLegacy->pPerfProfiler)
    {
        pRenderHalLegacy->pPerfProfiler = MediaPerfProfiler::Instance();
        MHW_RENDERHAL_CHK_NULL_RETURN(pRenderHalLegacy->pPerfProfiler);

        MHW_RENDERHAL_CHK_STATUS_RETURN(pRenderHalLegacy->pPerfProfiler->Initialize((void*)pRenderHalLegacy, pRenderHalLegacy->pOsInterface));
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS XRenderHal_Platform_Interface_Legacy::DestroyPerfProfiler(
    PRENDERHAL_INTERFACE pRenderHal)
{
    PRENDERHAL_INTERFACE_LEGACY pRenderHalLegacy = static_cast<PRENDERHAL_INTERFACE_LEGACY>(pRenderHal);
   
    MHW_RENDERHAL_CHK_NULL_RETURN(pRenderHalLegacy);
    MHW_RENDERHAL_CHK_NULL_RETURN(pRenderHalLegacy->pOsInterface);
    
    if (pRenderHalLegacy->pPerfProfiler)
    {
       MediaPerfProfiler::Destroy(pRenderHalLegacy->pPerfProfiler, (void*)pRenderHalLegacy, pRenderHalLegacy->pOsInterface);
       pRenderHalLegacy->pPerfProfiler = nullptr;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS XRenderHal_Platform_Interface_Legacy::AddPerfCollectStartCmd(
    PRENDERHAL_INTERFACE pRenderHal,
    MOS_INTERFACE        *osInterface,
    MOS_COMMAND_BUFFER   *cmdBuffer)
{
    MOS_STATUS                  eStatus          = MOS_STATUS_SUCCESS;
    PRENDERHAL_INTERFACE_LEGACY pRenderHalLegacy = static_cast<PRENDERHAL_INTERFACE_LEGACY>(pRenderHal);

    MHW_RENDERHAL_CHK_NULL_RETURN(pRenderHalLegacy);
    MHW_RENDERHAL_CHK_NULL_RETURN(osInterface);
    MHW_RENDERHAL_CHK_NULL_RETURN(pRenderHalLegacy->pPerfProfiler);
    MHW_RENDERHAL_CHK_NULL_RETURN(pRenderHalLegacy->pMhwMiInterface);
    MHW_RENDERHAL_CHK_NULL_RETURN(cmdBuffer);

    MHW_RENDERHAL_CHK_STATUS_RETURN(pRenderHalLegacy->pPerfProfiler->AddPerfCollectStartCmd((void*)pRenderHalLegacy, osInterface, pRenderHalLegacy->pMhwMiInterface, cmdBuffer));

    return eStatus;
}

MOS_STATUS XRenderHal_Platform_Interface_Legacy::StartPredicate(
        PRENDERHAL_INTERFACE pRenderHal,
        PMOS_COMMAND_BUFFER  cmdBuffer)
{
    MOS_STATUS  eStatus = MOS_STATUS_SUCCESS;
    MHW_RENDERHAL_CHK_NULL_RETURN(pRenderHal);
    MHW_RENDERHAL_CHK_NULL_RETURN(pRenderHal->pMhwMiInterface);
    MHW_RENDERHAL_CHK_NULL_RETURN(cmdBuffer);

    MHW_RENDERHAL_CHK_STATUS_RETURN(NullHW::StartPredicate(pRenderHal->pOsInterface, pRenderHal->pMhwMiInterface, cmdBuffer));

    return eStatus;
}

MOS_STATUS XRenderHal_Platform_Interface_Legacy::StopPredicate(
        PRENDERHAL_INTERFACE pRenderHal,
        PMOS_COMMAND_BUFFER  cmdBuffer)
{
    MOS_STATUS  eStatus = MOS_STATUS_SUCCESS;
    MHW_RENDERHAL_CHK_NULL_RETURN(pRenderHal);
    MHW_RENDERHAL_CHK_NULL_RETURN(pRenderHal->pMhwMiInterface);
    MHW_RENDERHAL_CHK_NULL_RETURN(cmdBuffer);

    MHW_RENDERHAL_CHK_STATUS_RETURN(NullHW::StopPredicate(pRenderHal->pOsInterface, pRenderHal->pMhwMiInterface, cmdBuffer));

    return eStatus;
}

MOS_STATUS XRenderHal_Platform_Interface_Legacy::AddPerfCollectEndCmd(
    PRENDERHAL_INTERFACE pRenderHal,
    PMOS_INTERFACE       pOsInterface,
    MOS_COMMAND_BUFFER   *cmdBuffer)
{
    MOS_STATUS                  eStatus          = MOS_STATUS_SUCCESS;
    PRENDERHAL_INTERFACE_LEGACY pRenderHalLegacy = static_cast<PRENDERHAL_INTERFACE_LEGACY>(pRenderHal);

    MHW_RENDERHAL_CHK_NULL_RETURN(pRenderHalLegacy);
    MHW_RENDERHAL_CHK_NULL_RETURN(pOsInterface);
    MHW_RENDERHAL_CHK_NULL_RETURN(cmdBuffer);
    MHW_RENDERHAL_CHK_NULL_RETURN(pRenderHalLegacy->pPerfProfiler);

    MHW_RENDERHAL_CHK_STATUS_RETURN(pRenderHalLegacy->pPerfProfiler->AddPerfCollectEndCmd((void*)pRenderHalLegacy, pOsInterface, pRenderHalLegacy->pMhwMiInterface, cmdBuffer));

    return eStatus;
}

MOS_STATUS XRenderHal_Platform_Interface_Legacy::AddMediaVfeCmd(
    PRENDERHAL_INTERFACE    pRenderHal,
    PMOS_COMMAND_BUFFER     pCmdBuffer,
    MHW_VFE_PARAMS          *params)
{
    MOS_STATUS  eStatus = MOS_STATUS_SUCCESS;
    PRENDERHAL_INTERFACE_LEGACY pRenderHalLegacy = (PRENDERHAL_INTERFACE_LEGACY)pRenderHal;
    MHW_RENDERHAL_CHK_NULL_RETURN(pRenderHalLegacy);
    MHW_RENDERHAL_CHK_NULL_RETURN(params);
    MHW_RENDERHAL_CHK_NULL_RETURN(pRenderHalLegacy->pMhwRenderInterface);
    MHW_RENDERHAL_CHK_NULL_RETURN(pCmdBuffer);

    MHW_RENDERHAL_CHK_STATUS_RETURN(pRenderHalLegacy->pMhwRenderInterface->AddMediaVfeCmd(pCmdBuffer, params));

    return eStatus;
}

MOS_STATUS XRenderHal_Platform_Interface_Legacy::AddMediaStateFlush(
    PRENDERHAL_INTERFACE         pRenderHal,
    PMOS_COMMAND_BUFFER          pCmdBuffer,
    MHW_MEDIA_STATE_FLUSH_PARAM  *params)
{
    MOS_STATUS  eStatus = MOS_STATUS_SUCCESS;
    MHW_RENDERHAL_CHK_NULL_RETURN(pRenderHal);
    MHW_RENDERHAL_CHK_NULL_RETURN(params);
    MHW_RENDERHAL_CHK_NULL_RETURN(pRenderHal->pMhwMiInterface);
    MHW_RENDERHAL_CHK_NULL_RETURN(pCmdBuffer);

   MHW_RENDERHAL_CHK_STATUS_RETURN(pRenderHal->pMhwMiInterface->AddMediaStateFlush(pCmdBuffer, nullptr, params));

    return eStatus;
}

MOS_STATUS XRenderHal_Platform_Interface_Legacy::AddMiBatchBufferEnd(
    PRENDERHAL_INTERFACE         pRenderHal,
    PMOS_COMMAND_BUFFER          pCmdBuffer,
    PMHW_BATCH_BUFFER            batchBuffer)
{
    MOS_STATUS  eStatus = MOS_STATUS_SUCCESS;
    MHW_RENDERHAL_CHK_NULL_RETURN(pRenderHal);
    MHW_RENDERHAL_CHK_NULL_RETURN(pRenderHal->pMhwMiInterface);
    MHW_RENDERHAL_CHK_NULL_RETURN(pCmdBuffer);

    MHW_RENDERHAL_CHK_STATUS_RETURN(pRenderHal->pMhwMiInterface->AddMiBatchBufferEnd(pCmdBuffer, batchBuffer));

    return eStatus;
}

MOS_STATUS XRenderHal_Platform_Interface_Legacy::AddMediaObjectWalkerCmd(
    PRENDERHAL_INTERFACE         pRenderHal,
    PMOS_COMMAND_BUFFER          pCmdBuffer,
    PMHW_WALKER_PARAMS           params)
{
    MOS_STATUS  eStatus = MOS_STATUS_SUCCESS;
    PRENDERHAL_INTERFACE_LEGACY pRenderHalLegacy = (PRENDERHAL_INTERFACE_LEGACY)pRenderHal;
    MHW_RENDERHAL_CHK_NULL_RETURN(pRenderHalLegacy);
    MHW_RENDERHAL_CHK_NULL_RETURN(pRenderHalLegacy->pMhwRenderInterface);
    MHW_RENDERHAL_CHK_NULL_RETURN(pCmdBuffer);

    MHW_RENDERHAL_CHK_STATUS_RETURN(pRenderHalLegacy->pMhwRenderInterface->AddMediaObjectWalkerCmd(
        pCmdBuffer,
        params));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS XRenderHal_Platform_Interface_Legacy::AddGpGpuWalkerStateCmd(
    PRENDERHAL_INTERFACE     pRenderHal,
    PMOS_COMMAND_BUFFER      pCmdBuffer,
    PMHW_GPGPU_WALKER_PARAMS params)
{
    MOS_STATUS  eStatus = MOS_STATUS_SUCCESS;
    PRENDERHAL_INTERFACE_LEGACY pRenderHalLegacy = (PRENDERHAL_INTERFACE_LEGACY)pRenderHal;
    MHW_RENDERHAL_CHK_NULL_RETURN(pRenderHalLegacy);
    MHW_RENDERHAL_CHK_NULL_RETURN(pRenderHalLegacy->pMhwRenderInterface);
    MHW_RENDERHAL_CHK_NULL_RETURN(pCmdBuffer);

    MHW_RENDERHAL_CHK_STATUS_RETURN(pRenderHalLegacy->pMhwRenderInterface->AddGpGpuWalkerStateCmd(
        pCmdBuffer,
        params));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS XRenderHal_Platform_Interface_Legacy::AllocateHeaps(
    PRENDERHAL_INTERFACE     pRenderHal,
    MHW_STATE_HEAP_SETTINGS  MhwStateHeapSettings)
{
    MOS_STATUS  eStatus = MOS_STATUS_SUCCESS;
    PRENDERHAL_INTERFACE_LEGACY pRenderHalLegacy = (PRENDERHAL_INTERFACE_LEGACY)pRenderHal;
    MHW_RENDERHAL_CHK_NULL_RETURN(pRenderHalLegacy);
    MHW_RENDERHAL_CHK_NULL_RETURN(pRenderHalLegacy->pMhwRenderInterface);

    MHW_RENDERHAL_CHK_STATUS_RETURN(pRenderHalLegacy->pMhwRenderInterface->AllocateHeaps(MhwStateHeapSettings));

    return MOS_STATUS_SUCCESS;
}

PMHW_STATE_HEAP_INTERFACE XRenderHal_Platform_Interface_Legacy::GetStateHeapInterface(
    PRENDERHAL_INTERFACE     pRenderHal)
{
    PRENDERHAL_INTERFACE_LEGACY pRenderHalLegacy = (PRENDERHAL_INTERFACE_LEGACY)pRenderHal;
    if (pRenderHalLegacy && pRenderHalLegacy->pMhwRenderInterface)
    {
        return pRenderHalLegacy->pMhwRenderInterface->m_stateHeapInterface;
    }
    else
    {
        return nullptr;
    }
}

MOS_STATUS XRenderHal_Platform_Interface_Legacy::DestoryMhwInterface(
    PRENDERHAL_INTERFACE     pRenderHal)
{
    MOS_STATUS  eStatus = MOS_STATUS_SUCCESS;
    PRENDERHAL_INTERFACE_LEGACY pRenderHalLegacy = (PRENDERHAL_INTERFACE_LEGACY)pRenderHal;
    MHW_RENDERHAL_CHK_NULL_RETURN(pRenderHalLegacy);
    MHW_RENDERHAL_CHK_NULL_RETURN(pRenderHalLegacy->pMhwRenderInterface);

    if (pRenderHalLegacy->pMhwRenderInterface)
    {
        MOS_Delete(pRenderHalLegacy->pMhwRenderInterface);
        pRenderHalLegacy->pMhwRenderInterface = nullptr;
    }

    // Destroy MHW MI Interface
    if (pRenderHalLegacy->pMhwMiInterface)
    {
        MOS_Delete(pRenderHalLegacy->pMhwMiInterface);
        pRenderHalLegacy->pMhwMiInterface = nullptr;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS XRenderHal_Platform_Interface_Legacy::AddMediaCurbeLoadCmd(
    PRENDERHAL_INTERFACE         pRenderHal,
    PMOS_COMMAND_BUFFER          pCmdBuffer,
    PMHW_CURBE_LOAD_PARAMS       params)
{
    MOS_STATUS  eStatus = MOS_STATUS_SUCCESS;
    PRENDERHAL_INTERFACE_LEGACY pRenderHalLegacy = (PRENDERHAL_INTERFACE_LEGACY)pRenderHal;
    MHW_RENDERHAL_CHK_NULL_RETURN(pRenderHalLegacy);
    MHW_RENDERHAL_CHK_NULL_RETURN(pRenderHalLegacy->pMhwRenderInterface);

    MHW_RENDERHAL_CHK_STATUS_RETURN(pRenderHalLegacy->pMhwRenderInterface->AddMediaCurbeLoadCmd(pCmdBuffer, params));
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS XRenderHal_Platform_Interface_Legacy::AddMediaIDLoadCmd(
    PRENDERHAL_INTERFACE         pRenderHal,
    PMOS_COMMAND_BUFFER          pCmdBuffer,
    PMHW_ID_LOAD_PARAMS          params)
{
    MOS_STATUS  eStatus = MOS_STATUS_SUCCESS;
    PRENDERHAL_INTERFACE_LEGACY pRenderHalLegacy = (PRENDERHAL_INTERFACE_LEGACY)pRenderHal;
    MHW_RENDERHAL_CHK_NULL_RETURN(pRenderHalLegacy);
    MHW_RENDERHAL_CHK_NULL_RETURN(pRenderHalLegacy->pMhwRenderInterface);

    MHW_RENDERHAL_CHK_STATUS_RETURN(pRenderHalLegacy->pMhwRenderInterface->AddMediaIDLoadCmd(pCmdBuffer, params));
    return MOS_STATUS_SUCCESS;
}

bool XRenderHal_Platform_Interface_Legacy::IsPreemptionEnabled(
    PRENDERHAL_INTERFACE     pRenderHal)
{
    PRENDERHAL_INTERFACE_LEGACY pRenderHalLegacy = (PRENDERHAL_INTERFACE_LEGACY)pRenderHal;
    if (pRenderHalLegacy && pRenderHalLegacy->pMhwRenderInterface)
    {
        return pRenderHalLegacy->pMhwRenderInterface->IsPreemptionEnabled();
    }
    else
    {
        return false;
    }
}

void XRenderHal_Platform_Interface_Legacy::GetSamplerResolutionAlignUnit(
    PRENDERHAL_INTERFACE         pRenderHal,
    bool                         isAVSSampler,
    uint32_t                     &widthAlignUnit,
    uint32_t                     &heightAlignUnit)
{
    uint32_t wAlignUnit  = 0;
    uint32_t hAlignUnit = 0;
    PRENDERHAL_INTERFACE_LEGACY pRenderHalLegacy = (PRENDERHAL_INTERFACE_LEGACY)pRenderHal;
    if (pRenderHalLegacy && pRenderHalLegacy->pMhwRenderInterface)
    {
        pRenderHalLegacy->pMhwRenderInterface->GetSamplerResolutionAlignUnit(
            isAVSSampler,
            wAlignUnit,
            hAlignUnit);
    }

    widthAlignUnit = wAlignUnit;
    heightAlignUnit = hAlignUnit;
}

PMHW_RENDER_ENGINE_CAPS XRenderHal_Platform_Interface_Legacy::GetHwCaps(
    PRENDERHAL_INTERFACE         pRenderHal)
{
    PRENDERHAL_INTERFACE_LEGACY pRenderHalLegacy = (PRENDERHAL_INTERFACE_LEGACY)pRenderHal;
    if (pRenderHalLegacy && pRenderHalLegacy->pMhwRenderInterface)
    {
        return pRenderHalLegacy->pMhwRenderInterface->GetHwCaps();
    }
    else
    {
        return nullptr;
    }
}

std::shared_ptr<mhw::mi::Itf> XRenderHal_Platform_Interface_Legacy::GetMhwMiItf()
{
    return nullptr;
}
