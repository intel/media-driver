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
//! \file     renderhal.h
//! \brief    Render Engine Interfaces shared across platforms
//! \details  Platform Independent Hardware Interfaces
//!

#include "renderhal_platform_interface_legacy.h"

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
    MOS_STATUS                eStatus    = MOS_STATUS_SUCCESS;
    MHW_RENDERHAL_CHK_NULL(pRenderHal);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pMhwRenderInterface);

    MHW_RENDERHAL_CHK_STATUS(pRenderHal->pMhwRenderInterface->AddPipelineSelectCmd(pCmdBuffer, gpGpuPipe));

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
    MOS_STATUS                eStatus    = MOS_STATUS_SUCCESS;
    MHW_RENDERHAL_CHK_NULL(pRenderHal);

    MHW_RENDERHAL_CHK_STATUS(pRenderHal->pfnSendStateBaseAddress(pRenderHal, pCmdBuffer));

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
    MOS_STATUS                eStatus    = MOS_STATUS_SUCCESS;
    MHW_RENDERHAL_CHK_NULL(pRenderHal);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pMhwRenderInterface);

    MHW_RENDERHAL_CHK_STATUS(pRenderHal->pMhwRenderInterface->AddSipStateCmd(pCmdBuffer, &pRenderHal->SipStateParams));

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
    MOS_STATUS                eStatus    = MOS_STATUS_SUCCESS;
    MHW_RENDERHAL_CHK_NULL(pRenderHal);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pMhwRenderInterface);

    MHW_RENDERHAL_CHK_STATUS(pRenderHal->pMhwRenderInterface->AddCfeStateCmd(pCmdBuffer, params));

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
    PMOS_COMMAND_BUFFER         pCmdBuffer)
{
    MOS_STATUS                eStatus    = MOS_STATUS_SUCCESS;
    MHW_RENDERHAL_CHK_NULL(pRenderHal);

    MHW_RENDERHAL_CHK_STATUS(pRenderHal->pfnSendChromaKey(pRenderHal, pCmdBuffer));

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
    PMOS_COMMAND_BUFFER         pCmdBuffer)
{
    MOS_STATUS                eStatus    = MOS_STATUS_SUCCESS;
    MHW_RENDERHAL_CHK_NULL(pRenderHal);

    MHW_RENDERHAL_CHK_STATUS(pRenderHal->pfnSendPalette(pRenderHal, pCmdBuffer));

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
    MOS_STATUS                eStatus    = MOS_STATUS_SUCCESS;
    MHW_RENDERHAL_CHK_NULL(pRenderHal);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pMhwRenderInterface);

    MHW_RENDERHAL_CHK_STATUS(pRenderHal->pMhwRenderInterface->SetL3Cache(pCmdBuffer));

finish:
    return eStatus;
}

PMHW_MI_MMIOREGISTERS XRenderHal_Platform_Interface_Legacy::GetMmioRegisters(
    PRENDERHAL_INTERFACE        pRenderHal)
{
    PMHW_MI_MMIOREGISTERS     pMmioRegisters = nullptr;
    if (pRenderHal && pRenderHal->pMhwRenderInterface)
    {
        pMmioRegisters = pRenderHal->pMhwRenderInterface->GetMmioRegisters();
    }

    return pMmioRegisters;
};

MOS_STATUS XRenderHal_Platform_Interface_Legacy::EnablePreemption(
    PRENDERHAL_INTERFACE            pRenderHal,
    PMOS_COMMAND_BUFFER             pCmdBuffer)
{
    MOS_STATUS                eStatus    = MOS_STATUS_SUCCESS;
    MHW_RENDERHAL_CHK_NULL(pRenderHal);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pMhwRenderInterface);

    MHW_RENDERHAL_CHK_STATUS(pRenderHal->pMhwRenderInterface->EnablePreemption(pCmdBuffer));

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
    syncParams.uiSemaphoreCount         = 1;
    // Currently only sync between VEBOX and 3D, also need to consider sync between Render Engine and 3D
    // low priority since current VP Predication test case does not cover this scenario.
    syncParams.GpuContext               = MOS_GPU_CONTEXT_VEBOX;
    syncParams.presSyncResource         = pRenderHal->PredicationParams.pPredicationResource;
    syncParams.bReadOnly                = true;
    syncParams.bDisableDecodeSyncLock   = false;
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
        loadRegisterMemParams.presStoreBuffer   = pRenderHal->PredicationParams.pPredicationResource;
        loadRegisterMemParams.dwOffset          = (uint32_t)pRenderHal->PredicationParams.predicationResOffset;
        loadRegisterMemParams.dwRegister        = mmioRegistersRender->generalPurposeRegister0LoOffset;
        MHW_RENDERHAL_CHK_STATUS(pRenderHal->pMhwMiInterface->AddMiLoadRegisterMemCmd(
            pCmdBuffer,
            &loadRegisterMemParams));

        MHW_MI_LOAD_REGISTER_IMM_PARAMS     loadRegisterImmParams;
        MOS_ZeroMemory(&loadRegisterImmParams, sizeof(loadRegisterImmParams));
        loadRegisterImmParams.dwData            = 0;
        loadRegisterImmParams.dwRegister        = mmioRegistersRender->generalPurposeRegister0HiOffset;
        MHW_RENDERHAL_CHK_STATUS(pRenderHal->pMhwMiInterface->AddMiLoadRegisterImmCmd(
            pCmdBuffer,
            &loadRegisterImmParams));

        // load 0 to general purpose register4
        MOS_ZeroMemory(&loadRegisterImmParams, sizeof(loadRegisterImmParams));
        loadRegisterImmParams.dwData            = 0;
        loadRegisterImmParams.dwRegister        = mmioRegistersRender->generalPurposeRegister4LoOffset;
        MHW_RENDERHAL_CHK_STATUS(pRenderHal->pMhwMiInterface->AddMiLoadRegisterImmCmd(
            pCmdBuffer,
            &loadRegisterImmParams));

        MOS_ZeroMemory(&loadRegisterImmParams, sizeof(loadRegisterImmParams));
        loadRegisterImmParams.dwData            = 0;
        loadRegisterImmParams.dwRegister        = mmioRegistersRender->generalPurposeRegister4HiOffset;
        MHW_RENDERHAL_CHK_STATUS(pRenderHal->pMhwMiInterface->AddMiLoadRegisterImmCmd(
            pCmdBuffer,
            &loadRegisterImmParams));

        //perform the add operation
        MHW_MI_MATH_PARAMS  miMathParams;
        MHW_MI_ALU_PARAMS   miAluParams[4];
        MOS_ZeroMemory(&miMathParams, sizeof(miMathParams));
        MOS_ZeroMemory(&miAluParams, sizeof(miAluParams));
        // load     srcA, reg0
        miAluParams[0].AluOpcode    = MHW_MI_ALU_LOAD;
        miAluParams[0].Operand1     = MHW_MI_ALU_SRCA;
        miAluParams[0].Operand2     = MHW_MI_ALU_GPREG0;
        // load     srcB, reg4
        miAluParams[1].AluOpcode    = MHW_MI_ALU_LOAD;
        miAluParams[1].Operand1     = MHW_MI_ALU_SRCB;
        miAluParams[1].Operand2     = MHW_MI_ALU_GPREG4;
        // add      srcA, srcB
        miAluParams[2].AluOpcode    = MHW_MI_ALU_ADD;
        miAluParams[2].Operand1     = MHW_MI_ALU_SRCB;
        miAluParams[2].Operand2     = MHW_MI_ALU_GPREG4;
        // store      reg0, ZF
        miAluParams[3].AluOpcode    = MHW_MI_ALU_STORE;
        miAluParams[3].Operand1     = MHW_MI_ALU_GPREG0;
        miAluParams[3].Operand2     = MHW_MI_ALU_ZF;
        miMathParams.pAluPayload    = miAluParams;
        miMathParams.dwNumAluParams = 4; // four ALU commands needed for this substract opertaion. see following ALU commands.
        MHW_RENDERHAL_CHK_STATUS(pRenderHal->pMhwMiInterface->AddMiMathCmd(
            pCmdBuffer,
            &miMathParams));

        // if zero, the zero flag will be 0xFFFFFFFF, else zero flag will be 0x0.
        MHW_MI_STORE_REGISTER_MEM_PARAMS    storeRegParams;
        MOS_ZeroMemory(&storeRegParams, sizeof(storeRegParams));
        storeRegParams.presStoreBuffer  = &pRenderHal->PredicationBuffer;
        storeRegParams.dwOffset         = 0x10;
        storeRegParams.dwRegister       = mmioRegistersRender->generalPurposeRegister0LoOffset;
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
        condBBEndParams.dwOffset            = 0x10;
        condBBEndParams.dwValue             = 0;
        condBBEndParams.bDisableCompareMask = true;
        MHW_RENDERHAL_CHK_STATUS(pRenderHal->pMhwMiInterface->AddMiConditionalBatchBufferEndCmd(
            pCmdBuffer,
            &condBBEndParams));

        pRenderHal->PredicationParams.ptempPredicationBuffer = &pRenderHal->PredicationBuffer;
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

    // TD_CTL, force thread breakpoint enable
    // Also enable external halt/exceptions, because the source-level debugger
    // needs to be able to interrupt running EU threads.
    params->dwRegister = TD_CTL;
    params->dwData     = TD_CTL_FORCE_THREAD_BKPT_ENABLE |
                            TD_CTL_FORCE_EXT_EXCEPTION_ENABLE;

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