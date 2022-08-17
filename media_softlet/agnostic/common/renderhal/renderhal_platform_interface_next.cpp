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
//! \file      renderhal_platform_interface_next.cpp
//! \brief     abstract the platfrom specific APIs into one class
//!
//! \file     renderhal.h
//! \brief    Render Engine Interfaces shared across platforms
//! \details  Platform Independent Hardware Interfaces
//!

#include <stdint.h>
#include "renderhal_platform_interface_next.h"
#include "media_packet.h"
#include "mhw_utilities_next.h"
#include "hal_oca_interface_next.h"
#include "media_feature.h"
#include "media_interfaces_mhw_next.h"
#include "media_skuwa_specific.h"
#include "mhw_itf.h"
#include "mhw_mi_cmdpar.h"
#include "mhw_mi_itf.h"
#include "mhw_render_cmdpar.h"
#include "mos_os.h"
#include "mos_utilities.h"
#include "renderhal.h"
#include "vp_utils.h"
// Need to remove below header files, after legacy contents clean up
#include "mhw_render_legacy.h"
#include "media_perf_profiler.h"

MOS_STATUS XRenderHal_Platform_Interface_Next::AddPipelineSelectCmd(
    PRENDERHAL_INTERFACE        pRenderHal,
    PMOS_COMMAND_BUFFER         pCmdBuffer,
    bool                        gpGpuPipe)
{
    VP_FUNC_CALL();

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_RENDERHAL_CHK_NULL_RETURN(pRenderHal);
    MHW_RENDERHAL_CHK_NULL_RETURN(pCmdBuffer);
    m_renderHal = pRenderHal;

    MHW_RENDERHAL_CHK_NULL_RETURN(m_renderItf);

    auto& par = m_renderItf->MHW_GETPAR_F(PIPELINE_SELECT)();
    par = {};
    par.gpGpuPipe = gpGpuPipe;
    m_renderItf->MHW_ADDCMD_F(PIPELINE_SELECT)(pCmdBuffer);

    return eStatus;
}

MOS_STATUS XRenderHal_Platform_Interface_Next::SendStateBaseAddress(
    PRENDERHAL_INTERFACE        pRenderHal,
    PMOS_COMMAND_BUFFER         pCmdBuffer)
{
    VP_FUNC_CALL();
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_RENDERHAL_CHK_NULL_RETURN(pRenderHal);
    MHW_RENDERHAL_CHK_NULL_RETURN(pCmdBuffer);
    m_renderHal = pRenderHal;

    MHW_RENDERHAL_CHK_NULL_RETURN(m_renderItf);
    SETPAR_AND_ADDCMD(STATE_BASE_ADDRESS, m_renderItf, pCmdBuffer);

    return eStatus;
}

MOS_STATUS XRenderHal_Platform_Interface_Next::AddSipStateCmd(
    PRENDERHAL_INTERFACE        pRenderHal,
    PMOS_COMMAND_BUFFER         pCmdBuffer)
{
    VP_FUNC_CALL();
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_RENDERHAL_CHK_NULL_RETURN(pRenderHal);
    MHW_RENDERHAL_CHK_NULL_RETURN(pCmdBuffer);
    m_renderHal = pRenderHal;

    MHW_RENDERHAL_CHK_NULL_RETURN(m_renderItf);
    SETPAR_AND_ADDCMD(STATE_SIP, m_renderItf, pCmdBuffer);

    return eStatus;
}

MOS_STATUS XRenderHal_Platform_Interface_Next::AddCfeStateCmd(
    PRENDERHAL_INTERFACE        pRenderHal,
    PMOS_COMMAND_BUFFER         pCmdBuffer,
    PMHW_VFE_PARAMS             params)
{
    VP_FUNC_CALL();

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_RENDERHAL_CHK_NULL_RETURN(pRenderHal);
    MHW_RENDERHAL_CHK_NULL_RETURN(pCmdBuffer);
    m_renderHal = pRenderHal;

    MHW_RENDERHAL_CHK_NULL_RETURN(m_renderItf);
    SETPAR_AND_ADDCMD(CFE_STATE, m_renderItf, pCmdBuffer);

    return eStatus;
}

MOS_STATUS XRenderHal_Platform_Interface_Next::SendChromaKey(
    PRENDERHAL_INTERFACE        pRenderHal,
    PMOS_COMMAND_BUFFER         pCmdBuffer,
    PMHW_CHROMAKEY_PARAMS       pChromaKeyParams)
{
    VP_FUNC_CALL();
    MOS_STATUS eStatus                        = MOS_STATUS_SUCCESS;
    MEDIA_WA_TABLE               *pWaTable    = nullptr;
    MOS_GPU_CONTEXT       renderGpuContext    = {};

    MHW_RENDERHAL_CHK_NULL_RETURN(pRenderHal);
    MHW_RENDERHAL_CHK_NULL_RETURN(pRenderHal->pOsInterface);
    MHW_RENDERHAL_CHK_NULL_RETURN(pCmdBuffer);
    m_renderHal = pRenderHal;

    MHW_RENDERHAL_CHK_NULL_RETURN(pRenderHal->pOsInterface);
    pWaTable = pRenderHal->pOsInterface->pfnGetWaTable(pRenderHal->pOsInterface);
    MHW_RENDERHAL_CHK_NULL_RETURN(pWaTable);
    renderGpuContext = pRenderHal->pOsInterface->pfnGetGpuContext(pRenderHal->pOsInterface);

    // Program stalling pipecontrol with HDC pipeline flush enabled before programming 3DSTATE_CHROMA_KEY for CCS W/L.
    if ((renderGpuContext == MOS_GPU_CONTEXT_COMPUTE)    ||
        (renderGpuContext == MOS_GPU_CONTEXT_CM_COMPUTE) ||
        (renderGpuContext == MOS_GPU_CONTEXT_COMPUTE_RA))
    {
        if (MEDIA_IS_WA(pWaTable, Wa_16011481064))
        {
            MHW_RENDERHAL_CHK_NULL_RETURN(m_miItf);
            auto &par                         = m_miItf->MHW_GETPAR_F(PIPE_CONTROL)();
            par                               = {};
            par.dwFlushMode                   = MHW_FLUSH_WRITE_CACHE;
            par.bDisableCSStall               = false;
            par.bGenericMediaStateClear       = true;
            par.bIndirectStatePointersDisable = true;
            par.bHdcPipelineFlush             = true;
            m_miItf->MHW_ADDCMD_F(PIPE_CONTROL)(pCmdBuffer);
        }
    }

    MHW_RENDERHAL_CHK_NULL_RETURN(m_renderItf);
    SETPAR_AND_ADDCMD(_3DSTATE_CHROMA_KEY, m_renderItf, pCmdBuffer);

    return eStatus;
}

MOS_STATUS XRenderHal_Platform_Interface_Next::SendPalette(
    PRENDERHAL_INTERFACE        pRenderHal,
    PMOS_COMMAND_BUFFER         pCmdBuffer,
    PMHW_PALETTE_PARAMS         pPaletteLoadParams)
{
    VP_FUNC_CALL();

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS XRenderHal_Platform_Interface_Next::SetL3Cache(
    PRENDERHAL_INTERFACE        pRenderHal,
    PMOS_COMMAND_BUFFER         pCmdBuffer)
{
    MOS_STATUS eStatus    = MOS_STATUS_SUCCESS;
    MHW_RENDERHAL_CHK_NULL_RETURN(pRenderHal);
    MHW_RENDERHAL_CHK_NULL_RETURN(m_renderItf);

    m_renderHal = pRenderHal;
    MHW_RENDERHAL_CHK_STATUS_RETURN(m_renderItf->SetL3Cache(pCmdBuffer, m_miItf));

    return eStatus;
}

//!
//! \brief    Get the size of render hal media state
//! \return   size_t
//!           The size of render hal media state
//!
size_t XRenderHal_Platform_Interface_Next::GetRenderHalMediaStateSize()
{
    return sizeof(RENDERHAL_MEDIA_STATE);
}

//!
//! \brief    Get the size of render hal state heap
//! \return   size_t
//!           The size of render hal state heap
//!
size_t XRenderHal_Platform_Interface_Next::GetRenderHalStateHeapSize()
{
    return sizeof(RENDERHAL_STATE_HEAP);
}

PMHW_MI_MMIOREGISTERS XRenderHal_Platform_Interface_Next::GetMmioRegisters(
    PRENDERHAL_INTERFACE        pRenderHal)
{
    PMHW_MI_MMIOREGISTERS     pMmioRegisters = nullptr;

    if (m_renderItf)
    {
        pMmioRegisters = m_renderItf->GetMmioRegisters();
    }

    return pMmioRegisters;
}

MOS_STATUS XRenderHal_Platform_Interface_Next::EnablePreemption(
    PRENDERHAL_INTERFACE            pRenderHal,
    PMOS_COMMAND_BUFFER             pCmdBuffer)
{
    MOS_STATUS eStatus              = MOS_STATUS_SUCCESS;
    MEDIA_FEATURE_TABLE* m_skuTable = nullptr;
    MHW_RENDERHAL_CHK_NULL_RETURN(pRenderHal);
    MHW_RENDERHAL_CHK_NULL_RETURN(m_miItf);

    m_skuTable = pRenderHal->pOsInterface->pfnGetSkuTable(pRenderHal->pOsInterface);
    MHW_MI_CHK_NULL(m_skuTable);

    if (MEDIA_IS_SKU(m_skuTable, FtrPerCtxtPreemptionGranularityControl))
    {
        auto& par = m_miItf->MHW_GETPAR_F(MI_LOAD_REGISTER_IMM)();
        par = {};
        par.dwRegister = 0;
        par.dwData     = 0;
        m_miItf->MHW_ADDCMD_F(MI_LOAD_REGISTER_IMM)(pCmdBuffer);
    }

    return eStatus;
}

MOS_STATUS XRenderHal_Platform_Interface_Next::SendPredicationCommand(
    PRENDERHAL_INTERFACE        pRenderHal,
    PMOS_COMMAND_BUFFER         pCmdBuffer)
{
    MOS_STATUS  eStatus = MOS_STATUS_SUCCESS;

    MHW_RENDERHAL_CHK_NULL_RETURN(pRenderHal);
    MHW_RENDERHAL_CHK_NULL_RETURN(m_miItf);
    MHW_RENDERHAL_CHK_NULL_RETURN(m_miItf->GetMmioRegisters());

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

    MHW_CHK_STATUS_RETURN(pRenderHal->pOsInterface->pfnPerformOverlaySync(pRenderHal->pOsInterface, &syncParams));
    MHW_CHK_STATUS_RETURN(pRenderHal->pOsInterface->pfnResourceWait(pRenderHal->pOsInterface, &syncParams));

    if (pRenderHal->PredicationParams.predicationNotEqualZero)
    {
        auto mmioRegistersRender = m_miItf->GetMmioRegisters();

        auto& parFlush = m_miItf->MHW_GETPAR_F(MI_FLUSH_DW)();
        parFlush = {};
        m_miItf->MHW_ADDCMD_F(MI_FLUSH_DW)(pCmdBuffer);

        // load presPredication to general purpose register0
        auto& parRegM = m_miItf->MHW_GETPAR_F(MI_LOAD_REGISTER_MEM)();
        parRegM = {};
        parRegM.presStoreBuffer   = pRenderHal->PredicationParams.pPredicationResource;
        parRegM.dwOffset          = (uint32_t)pRenderHal->PredicationParams.predicationResOffset;
        parRegM.dwRegister        = mmioRegistersRender->generalPurposeRegister0LoOffset;
        m_miItf->MHW_ADDCMD_F(MI_LOAD_REGISTER_MEM)(pCmdBuffer);

        auto& parImm = m_miItf->MHW_GETPAR_F(MI_LOAD_REGISTER_IMM)();
        parImm = {};
        parImm.dwData            = 0;
        parImm.dwRegister        = mmioRegistersRender->generalPurposeRegister0HiOffset;
        m_miItf->MHW_ADDCMD_F(MI_LOAD_REGISTER_IMM)(pCmdBuffer);

        parImm = {};
        parImm.dwData            = 0;
        parImm.dwRegister        = mmioRegistersRender->generalPurposeRegister4LoOffset;
        m_miItf->MHW_ADDCMD_F(MI_LOAD_REGISTER_IMM)(pCmdBuffer);

        parImm = {};
        parImm.dwData            = 0;
        parImm.dwRegister        = mmioRegistersRender->generalPurposeRegister4HiOffset;
        m_miItf->MHW_ADDCMD_F(MI_LOAD_REGISTER_IMM)(pCmdBuffer);

        //perform the add operation
        mhw::mi::MHW_MI_ALU_PARAMS miAluParams[4] = {};

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

        auto& par = m_miItf->MHW_GETPAR_F(MI_MATH)();
        par = {};
        par.pAluPayload    = miAluParams;
        par.dwNumAluParams = 4; // four ALU commands needed for this substract opertaion. see following ALU commands.
        m_miItf->MHW_ADDCMD_F(MI_MATH)(pCmdBuffer);

        // if zero, the zero flag will be 0xFFFFFFFF, else zero flag will be 0x0.
        parRegM = {};
        parRegM.presStoreBuffer  = &pRenderHal->PredicationBuffer;
        parRegM.dwOffset         = 0x10;
        parRegM.dwRegister       = mmioRegistersRender->generalPurposeRegister0LoOffset;
        m_miItf->MHW_ADDCMD_F(MI_STORE_REGISTER_MEM)(pCmdBuffer);

        // Programming of 4 dummy MI_STORE_DATA_IMM commands prior to programming of MiConditionalBatchBufferEnd
        auto& parData = m_miItf->MHW_GETPAR_F(MI_STORE_DATA_IMM)();
        parData = {};
        parData.pOsResource = &pRenderHal->PredicationBuffer;
        parData.dwValue     = 1;
        m_miItf->MHW_ADDCMD_F(MI_STORE_DATA_IMM)(pCmdBuffer);

        parData.dwValue = 2;
        m_miItf->MHW_ADDCMD_F(MI_STORE_DATA_IMM)(pCmdBuffer);

        parData.dwValue = 3;
        m_miItf->MHW_ADDCMD_F(MI_STORE_DATA_IMM)(pCmdBuffer);

        parData.dwValue = 4;
        m_miItf->MHW_ADDCMD_F(MI_STORE_DATA_IMM)(pCmdBuffer);

        parFlush = {};
        parFlush.postSyncOperation = 1;
        parFlush.pOsResource       = &pRenderHal->PredicationBuffer;
        m_miItf->MHW_ADDCMD_F(MI_FLUSH_DW)(pCmdBuffer);

        auto& parBatch = m_miItf->MHW_GETPAR_F(MI_CONDITIONAL_BATCH_BUFFER_END)();
        parBatch = {};
        parBatch.presSemaphoreBuffer = &pRenderHal->PredicationBuffer;
        parBatch.dwOffset            = 0x10;
        parBatch.dwValue             = 0;
        parBatch.bDisableCompareMask = true;
        m_miItf->MHW_ADDCMD_F(MI_CONDITIONAL_BATCH_BUFFER_END)(pCmdBuffer);

        pRenderHal->PredicationParams.ptempPredicationBuffer = &pRenderHal->PredicationBuffer;
    }
    else
    {
        auto mmioRegistersRender = m_miItf->GetMmioRegisters();

        auto& parFlush = m_miItf->MHW_GETPAR_F(MI_FLUSH_DW)();
        parFlush = {};
        m_miItf->MHW_ADDCMD_F(MI_FLUSH_DW)(pCmdBuffer);

        // load presPredication to general purpose register0
        auto& parRegM = m_miItf->MHW_GETPAR_F(MI_LOAD_REGISTER_MEM)();
        parRegM = {};
        parRegM.presStoreBuffer = pRenderHal->PredicationParams.pPredicationResource;
        parRegM.dwOffset        = (uint32_t)pRenderHal->PredicationParams.predicationResOffset;
        parRegM.dwRegister      = mmioRegistersRender->generalPurposeRegister0LoOffset;
        m_miItf->MHW_ADDCMD_F(MI_LOAD_REGISTER_MEM)(pCmdBuffer);

        // if zero, the zero flag will be 0xFFFFFFFF, else zero flag will be 0x0.
        auto& parStore = m_miItf->MHW_GETPAR_F(MI_STORE_REGISTER_MEM)();
        parStore.presStoreBuffer = &pRenderHal->PredicationBuffer;
        parStore.dwOffset        = 0x10;
        parStore.dwRegister      = mmioRegistersRender->generalPurposeRegister0LoOffset;
        m_miItf->MHW_ADDCMD_F(MI_STORE_REGISTER_MEM)(pCmdBuffer);

        // Programming of 4 dummy MI_STORE_DATA_IMM commands prior to programming of MiConditionalBatchBufferEnd
        auto& parData = m_miItf->MHW_GETPAR_F(MI_STORE_DATA_IMM)();
        parData.pOsResource = &pRenderHal->PredicationBuffer;
        parData.dwValue     = 1;
        m_miItf->MHW_ADDCMD_F(MI_STORE_DATA_IMM)(pCmdBuffer);

        parData.dwValue = 2;
        m_miItf->MHW_ADDCMD_F(MI_STORE_DATA_IMM)(pCmdBuffer);

        parData.dwValue = 3;
        m_miItf->MHW_ADDCMD_F(MI_STORE_DATA_IMM)(pCmdBuffer);

        parData.dwValue = 4;
        m_miItf->MHW_ADDCMD_F(MI_STORE_DATA_IMM)(pCmdBuffer);

        parFlush = {};
        parFlush.postSyncOperation = 1;
        parFlush.pOsResource       = &pRenderHal->PredicationBuffer;
        m_miItf->MHW_ADDCMD_F(MI_FLUSH_DW)(pCmdBuffer);

        // Skip current frame if presPredication is equal to zero
        auto& parBatch = m_miItf->MHW_GETPAR_F(MI_CONDITIONAL_BATCH_BUFFER_END)();
        parBatch = {};
        parBatch.presSemaphoreBuffer = &pRenderHal->PredicationBuffer;
        parBatch.dwOffset            = 0x10;
        parBatch.dwValue             = 0;
        parBatch.bDisableCompareMask = true;
        m_miItf->MHW_ADDCMD_F(MI_CONDITIONAL_BATCH_BUFFER_END)(pCmdBuffer);
    }

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
MOS_STATUS XRenderHal_Platform_Interface_Next::SendMarkerCommand(
    PRENDERHAL_INTERFACE    pRenderHal,
    PMOS_COMMAND_BUFFER     cmdBuffer,
    bool                    isRender)
{
    MOS_STATUS  eStatus = MOS_STATUS_SUCCESS;

    //-----------------------------------------
    MHW_RENDERHAL_CHK_NULL_RETURN(pRenderHal);
    MHW_RENDERHAL_CHK_NULL_RETURN(m_miItf);
    //-----------------------------------------

    if (isRender)
    {
        // Send pipe_control to get the timestamp
        auto& params = m_miItf->MHW_GETPAR_F(PIPE_CONTROL)();
        params = {};
        params.presDest         = pRenderHal->SetMarkerParams.pSetMarkerResource;
        params.dwResourceOffset = 0;
        params.dwPostSyncOp     = MHW_FLUSH_WRITE_TIMESTAMP_REG;
        params.dwFlushMode      = MHW_FLUSH_WRITE_CACHE;
        m_miItf->MHW_ADDCMD_F(PIPE_CONTROL)(cmdBuffer);
    }
    else
    {
        // Send flush_dw to get the timestamp
        auto& parFlush = m_miItf->MHW_GETPAR_F(MI_FLUSH_DW)();
        parFlush = {};
        parFlush.pOsResource       = pRenderHal->SetMarkerParams.pSetMarkerResource;
        parFlush.dwResourceOffset  = 0;
        parFlush.postSyncOperation = MHW_FLUSH_WRITE_TIMESTAMP_REG;
        parFlush.bQWordEnable      = 1;
        m_miItf->MHW_ADDCMD_F(MI_FLUSH_DW)(cmdBuffer);
    }

    return eStatus;
}

MOS_STATUS XRenderHal_Platform_Interface_Next::AddMiPipeControl(
    PRENDERHAL_INTERFACE     pRenderHal,
    PMOS_COMMAND_BUFFER      pCmdBuffer,
    MHW_PIPE_CONTROL_PARAMS* params)
{
    MOS_STATUS  eStatus = MOS_STATUS_SUCCESS;

    //------------------------------------
    MHW_RENDERHAL_CHK_NULL_RETURN(pRenderHal);
    MHW_RENDERHAL_CHK_NULL_RETURN(pCmdBuffer);
    MHW_RENDERHAL_CHK_NULL_RETURN(params);
    MHW_RENDERHAL_CHK_NULL_RETURN(m_miItf);
    //------------------------------------

    auto& par = m_miItf->MHW_GETPAR_F(PIPE_CONTROL)();
    par = {};
    par.presDest = params->presDest;
    par.dwResourceOffset = params->dwResourceOffset;
    par.dwDataDW1 = params->dwDataDW1;
    par.dwDataDW2 = params->dwDataDW2;
    par.dwFlushMode = params->dwFlushMode;
    par.dwPostSyncOp = params->dwPostSyncOp;
    par.bDisableCSStall = params->bDisableCSStall;
    par.bInvalidateStateCache = params->bInvalidateStateCache;
    par.bInvalidateConstantCache = params->bInvalidateConstantCache;
    par.bInvalidateVFECache = params->bInvalidateVFECache;
    par.bInvalidateInstructionCache = params->bInvalidateInstructionCache;
    par.bFlushRenderTargetCache = params->bFlushRenderTargetCache;
    par.bTlbInvalidate = params->bTlbInvalidate;
    par.bInvalidateTextureCache = params->bInvalidateTextureCache;
    par.bGenericMediaStateClear = params->bGenericMediaStateClear;
    par.bIndirectStatePointersDisable = params->bIndirectStatePointersDisable;
    par.bHdcPipelineFlush = params->bHdcPipelineFlush;
    par.bKernelFenceEnabled = params->bKernelFenceEnabled;
    m_miItf->MHW_ADDCMD_F(PIPE_CONTROL)(pCmdBuffer);

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
MOS_STATUS XRenderHal_Platform_Interface_Next::AddMiLoadRegisterImmCmd(
    PRENDERHAL_INTERFACE             pRenderHal,
    PMOS_COMMAND_BUFFER              pCmdBuffer,
    PMHW_MI_LOAD_REGISTER_IMM_PARAMS params)
{
    MOS_STATUS  eStatus = MOS_STATUS_SUCCESS;

    //-----------------------------------------
    MHW_RENDERHAL_CHK_NULL_RETURN(pRenderHal);
    MHW_RENDERHAL_CHK_NULL_RETURN(m_miItf);
    MHW_RENDERHAL_CHK_NULL_RETURN(pCmdBuffer);
    MHW_RENDERHAL_CHK_NULL_RETURN(params);
    //-----------------------------------------

    auto& par = m_miItf->MHW_GETPAR_F(MI_LOAD_REGISTER_IMM)();
    par = {};
    par.dwData     = params->dwData;
    par.dwRegister = params->dwRegister;
    m_miItf->MHW_ADDCMD_F(MI_LOAD_REGISTER_IMM)(pCmdBuffer);

    // TD_CTL, force thread breakpoint enable
    // Also enable external halt/exceptions, because the source-level debugger
    // needs to be able to interrupt running EU threads.
    params->dwRegister = TD_CTL;
    params->dwData     = TD_CTL_FORCE_THREAD_BKPT_ENABLE |
                            TD_CTL_FORCE_EXT_EXCEPTION_ENABLE;

    par = {};
    par.dwData     = params->dwData;
    par.dwRegister = params->dwRegister;
    m_miItf->MHW_ADDCMD_F(MI_LOAD_REGISTER_IMM)(pCmdBuffer);

    return eStatus;
}

MOS_STATUS XRenderHal_Platform_Interface_Next::SendGenericPrologCmd(
    PRENDERHAL_INTERFACE        pRenderHal,
    PMOS_COMMAND_BUFFER         pCmdBuffer,
    PMHW_GENERIC_PROLOG_PARAMS  pParams,
    MHW_MI_MMIOREGISTERS*       pMmioReg)
{
    MOS_STATUS  eStatus = MOS_STATUS_SUCCESS;

    //-----------------------------------------
    MHW_RENDERHAL_CHK_NULL_RETURN(pRenderHal);
    MHW_RENDERHAL_CHK_NULL_RETURN(m_miItf);
    MHW_RENDERHAL_CHK_NULL_RETURN(pCmdBuffer);
    MHW_RENDERHAL_CHK_NULL_RETURN(pParams);
    //-----------------------------------------

    MHW_CHK_STATUS_RETURN(Mhw_SendGenericPrologCmdNext(pCmdBuffer, pParams, m_miItf, pMmioReg));

    return eStatus;
}

MOS_STATUS XRenderHal_Platform_Interface_Next::CreateMhwInterfaces(
    PRENDERHAL_INTERFACE        pRenderHal,
    PMOS_INTERFACE              pOsInterface)
{
    MOS_STATUS  eStatus = MOS_STATUS_SUCCESS;

    //-----------------------------------------
    MHW_RENDERHAL_CHK_NULL_RETURN(pRenderHal);
    MHW_RENDERHAL_CHK_NULL_RETURN(pOsInterface);
    //-----------------------------------------

    MhwInterfacesNext::CreateParams params;
    MOS_ZeroMemory(&params, sizeof(params));
    params.Flags.m_render = true;
    params.m_heapMode = pRenderHal->bDynamicStateHeap;
    MhwInterfacesNext *mhwInterfaces =  MhwInterfacesNext::CreateFactory(params, pOsInterface);
    MHW_RENDERHAL_CHK_NULL_RETURN(mhwInterfaces);
    MHW_RENDERHAL_CHK_NULL_RETURN(mhwInterfaces->m_cpInterface);
#if !EMUL
    MHW_RENDERHAL_CHK_NULL_RETURN(mhwInterfaces->m_miInterface);
#endif
    pRenderHal->pCpInterface = mhwInterfaces->m_cpInterface;
    pRenderHal->pMhwMiInterface = mhwInterfaces->m_miInterface;
    m_renderItf = mhwInterfaces->m_renderItf;
    m_miItf     = mhwInterfaces->m_miItf;

    // After removing MhwRenderInterface from Mhw Next, need to clean this WA delete m_renderInterface code
    if (mhwInterfaces->m_renderInterface)
    {
        MOS_Delete(mhwInterfaces->m_renderInterface);
    }

    MOS_Delete(mhwInterfaces);

    return eStatus;
}

MOS_STATUS XRenderHal_Platform_Interface_Next::On1stLevelBBStart(
    PRENDERHAL_INTERFACE pRenderHal,
    PMOS_COMMAND_BUFFER  pCmdBuffer,
    PMOS_CONTEXT         pOsContext,
    uint32_t             gpuContextHandle,
    MHW_MI_MMIOREGISTERS *pMmioReg)
{
    MOS_STATUS  eStatus = MOS_STATUS_SUCCESS;
    MHW_RENDERHAL_CHK_NULL_RETURN(pRenderHal);
    MHW_RENDERHAL_CHK_NULL_RETURN(pRenderHal->pOsInterface);
    MHW_RENDERHAL_CHK_NULL_RETURN(m_miItf);
    MHW_RENDERHAL_CHK_NULL_RETURN(pCmdBuffer);

    HalOcaInterfaceNext::On1stLevelBBStart(*pCmdBuffer, *pOsContext, pRenderHal->pOsInterface->CurrentGpuContextHandle,
        m_miItf, *pMmioReg);

    return eStatus;
}

MOS_STATUS XRenderHal_Platform_Interface_Next::OnDispatch(
    PRENDERHAL_INTERFACE pRenderHal,
    PMOS_COMMAND_BUFFER  pCmdBuffer,
    PMOS_INTERFACE       pOsInterface,
    MHW_MI_MMIOREGISTERS *pMmioReg)
{
    MOS_STATUS  eStatus = MOS_STATUS_SUCCESS;
    MHW_RENDERHAL_CHK_NULL_RETURN(pRenderHal);
    MHW_RENDERHAL_CHK_NULL_RETURN(m_miItf);
    MHW_RENDERHAL_CHK_NULL_RETURN(pCmdBuffer);
    MHW_RENDERHAL_CHK_NULL_RETURN(pOsInterface);
    MHW_RENDERHAL_CHK_NULL_RETURN(pMmioReg);

    HalOcaInterfaceNext::OnDispatch(*pCmdBuffer, *pOsInterface, m_miItf, *pMmioReg);

    return eStatus;
}

MOS_STATUS XRenderHal_Platform_Interface_Next::CreatePerfProfiler(
    PRENDERHAL_INTERFACE pRenderHal)
{
    MHW_RENDERHAL_CHK_NULL_RETURN(pRenderHal);
    MHW_RENDERHAL_CHK_NULL_RETURN(pRenderHal->pOsInterface);

    if (!pRenderHal->pPerfProfiler)
    {        
        pRenderHal->pPerfProfiler = MediaPerfProfiler::Instance();
        MHW_RENDERHAL_CHK_NULL_RETURN(pRenderHal->pPerfProfiler);

        MHW_RENDERHAL_CHK_STATUS_RETURN(pRenderHal->pPerfProfiler->Initialize((void*)pRenderHal, pRenderHal->pOsInterface));
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS XRenderHal_Platform_Interface_Next::DestroyPerfProfiler(
    PRENDERHAL_INTERFACE pRenderHal)
{
    MHW_RENDERHAL_CHK_NULL_RETURN(pRenderHal);
    MHW_RENDERHAL_CHK_NULL_RETURN(pRenderHal->pOsInterface);

    if (pRenderHal->pPerfProfiler)
    {
        MediaPerfProfiler::Destroy(pRenderHal->pPerfProfiler, (void*)pRenderHal, pRenderHal->pOsInterface);
        pRenderHal->pPerfProfiler = nullptr;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS XRenderHal_Platform_Interface_Next::AddPerfCollectStartCmd(
    PRENDERHAL_INTERFACE pRenderHal,
    MOS_INTERFACE        *osInterface,
    MOS_COMMAND_BUFFER   *cmdBuffer)
{
    MOS_STATUS  eStatus = MOS_STATUS_SUCCESS;
    MHW_RENDERHAL_CHK_NULL_RETURN(pRenderHal);
    MHW_RENDERHAL_CHK_NULL_RETURN(pRenderHal->pPerfProfiler);
    MHW_RENDERHAL_CHK_NULL_RETURN(osInterface);
    MHW_RENDERHAL_CHK_NULL_RETURN(m_miItf);
    MHW_RENDERHAL_CHK_NULL_RETURN(cmdBuffer);
    
    MHW_CHK_STATUS_RETURN(pRenderHal->pPerfProfiler->AddPerfCollectStartCmd((void *)pRenderHal, osInterface, m_miItf, cmdBuffer));

    return eStatus;
}

MOS_STATUS XRenderHal_Platform_Interface_Next::StartPredicate(
        PRENDERHAL_INTERFACE pRenderHal,
        PMOS_COMMAND_BUFFER  cmdBuffer)
{
    MOS_STATUS  eStatus = MOS_STATUS_SUCCESS;
    MHW_RENDERHAL_CHK_NULL_RETURN(pRenderHal);
    MHW_RENDERHAL_CHK_NULL_RETURN(m_miItf);
    MHW_RENDERHAL_CHK_NULL_RETURN(cmdBuffer);

    MHW_CHK_STATUS_RETURN(NullHW::StartPredicateNext(m_miItf, cmdBuffer));

    return eStatus;
}

MOS_STATUS XRenderHal_Platform_Interface_Next::StopPredicate(
        PRENDERHAL_INTERFACE pRenderHal,
        PMOS_COMMAND_BUFFER  cmdBuffer)
{
    MOS_STATUS  eStatus = MOS_STATUS_SUCCESS;
    MHW_RENDERHAL_CHK_NULL_RETURN(pRenderHal);
    MHW_RENDERHAL_CHK_NULL_RETURN(m_miItf);
    MHW_RENDERHAL_CHK_NULL_RETURN(cmdBuffer);

    MHW_CHK_STATUS_RETURN(NullHW::StopPredicateNext(m_miItf, cmdBuffer));

    return eStatus;
}

MOS_STATUS XRenderHal_Platform_Interface_Next::AddPerfCollectEndCmd(
    PRENDERHAL_INTERFACE pRenderHal,
    PMOS_INTERFACE       pOsInterface,
    MOS_COMMAND_BUFFER   *cmdBuffer)
{
    MOS_STATUS  eStatus = MOS_STATUS_SUCCESS;
    MHW_RENDERHAL_CHK_NULL_RETURN(pRenderHal);
    MHW_RENDERHAL_CHK_NULL_RETURN(pRenderHal->pPerfProfiler);
    MHW_RENDERHAL_CHK_NULL_RETURN(pOsInterface);
    MHW_RENDERHAL_CHK_NULL_RETURN(m_miItf);
    MHW_RENDERHAL_CHK_NULL_RETURN(cmdBuffer);

    MHW_CHK_STATUS_RETURN(pRenderHal->pPerfProfiler->AddPerfCollectEndCmd((void *)pRenderHal, pOsInterface, m_miItf, cmdBuffer));

    return eStatus;
}

MOS_STATUS XRenderHal_Platform_Interface_Next::AddMediaVfeCmd(
    PRENDERHAL_INTERFACE    pRenderHal,
    PMOS_COMMAND_BUFFER     pCmdBuffer,
    MHW_VFE_PARAMS          *params)
{
    MOS_STATUS  eStatus = MOS_STATUS_SUCCESS;
    MHW_RENDERHAL_CHK_NULL_RETURN(pRenderHal);
    MHW_RENDERHAL_CHK_NULL_RETURN(params);
    MHW_RENDERHAL_CHK_NULL_RETURN(m_renderItf);
    MHW_RENDERHAL_CHK_NULL_RETURN(pCmdBuffer);

    return eStatus;
}

MOS_STATUS XRenderHal_Platform_Interface_Next::AddMediaStateFlush(
    PRENDERHAL_INTERFACE         pRenderHal,
    PMOS_COMMAND_BUFFER          pCmdBuffer,
    MHW_MEDIA_STATE_FLUSH_PARAM  *flushParam)
{
    MOS_STATUS  eStatus = MOS_STATUS_SUCCESS;
    MHW_RENDERHAL_CHK_NULL_RETURN(pRenderHal);
    MHW_RENDERHAL_CHK_NULL_RETURN(flushParam);
    MHW_RENDERHAL_CHK_NULL_RETURN(m_miItf);
    MHW_RENDERHAL_CHK_NULL_RETURN(pCmdBuffer);

    auto& params = m_miItf->MHW_GETPAR_F(MEDIA_STATE_FLUSH)();
    params                              = {};
    params.ui8InterfaceDescriptorOffset = flushParam->ui8InterfaceDescriptorOffset;
    params.bFlushToGo                   = flushParam->bFlushToGo;
    m_miItf->MHW_ADDCMD_F(MEDIA_STATE_FLUSH)(pCmdBuffer);

    return eStatus;
}

MOS_STATUS XRenderHal_Platform_Interface_Next::AddMiBatchBufferEnd(
    PRENDERHAL_INTERFACE         pRenderHal,
    PMOS_COMMAND_BUFFER          pCmdBuffer,
    PMHW_BATCH_BUFFER            batchBuffer)
{
    MOS_STATUS  eStatus = MOS_STATUS_SUCCESS;
    MHW_RENDERHAL_CHK_NULL_RETURN(pRenderHal);
    MHW_RENDERHAL_CHK_NULL_RETURN(m_miItf);
    MHW_RENDERHAL_CHK_NULL_RETURN(pCmdBuffer);

    m_miItf->AddMiBatchBufferEnd(pCmdBuffer, nullptr);

    return eStatus;
}

MOS_STATUS XRenderHal_Platform_Interface_Next::AddMediaObjectWalkerCmd(
    PRENDERHAL_INTERFACE         pRenderHal,
    PMOS_COMMAND_BUFFER          pCmdBuffer,
    PMHW_WALKER_PARAMS           params)
{
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS XRenderHal_Platform_Interface_Next::AddGpGpuWalkerStateCmd(
    PRENDERHAL_INTERFACE     pRenderHal,
    PMOS_COMMAND_BUFFER      pCmdBuffer,
    PMHW_GPGPU_WALKER_PARAMS params)
{
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS XRenderHal_Platform_Interface_Next::AllocateHeaps(
    PRENDERHAL_INTERFACE     pRenderHal,
    MHW_STATE_HEAP_SETTINGS  MhwStateHeapSettings)
{
    MOS_STATUS  eStatus = MOS_STATUS_SUCCESS;
    MHW_RENDERHAL_CHK_NULL_RETURN(m_renderItf);

    MHW_CHK_STATUS_RETURN(m_renderItf->AllocateHeaps(MhwStateHeapSettings));

    return MOS_STATUS_SUCCESS;
}

PMHW_STATE_HEAP_INTERFACE XRenderHal_Platform_Interface_Next::GetStateHeapInterface(
    PRENDERHAL_INTERFACE     pRenderHal)
{
    if (m_renderItf)
    {
        return m_renderItf->GetStateHeapInterface();
    }
    else
    {
        return nullptr;
    }
}

MOS_STATUS XRenderHal_Platform_Interface_Next::DestoryMhwInterface(
    PRENDERHAL_INTERFACE     pRenderHal)
{
    MOS_STATUS  eStatus = MOS_STATUS_SUCCESS;
    MHW_RENDERHAL_CHK_NULL_RETURN(pRenderHal);
    // Destroy MHW MI Interface
    if (pRenderHal->pMhwMiInterface)
    {
        MOS_Delete(pRenderHal->pMhwMiInterface);
        pRenderHal->pMhwMiInterface = nullptr;
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS XRenderHal_Platform_Interface_Next::AddMediaCurbeLoadCmd(
    PRENDERHAL_INTERFACE         pRenderHal,
    PMOS_COMMAND_BUFFER          pCmdBuffer,
    PMHW_CURBE_LOAD_PARAMS       params)
{
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS XRenderHal_Platform_Interface_Next::AddMediaIDLoadCmd(
    PRENDERHAL_INTERFACE         pRenderHal,
    PMOS_COMMAND_BUFFER          pCmdBuffer,
    PMHW_ID_LOAD_PARAMS          params)
{
    return MOS_STATUS_SUCCESS;
}

bool XRenderHal_Platform_Interface_Next::IsPreemptionEnabled(
    PRENDERHAL_INTERFACE     pRenderHal)
{
    if (m_renderItf)
    {
        return m_renderItf->IsPreemptionEnabled();
    }
    else
    {
        return false;
    }
}

void XRenderHal_Platform_Interface_Next::GetSamplerResolutionAlignUnit(
    PRENDERHAL_INTERFACE         pRenderHal,
    bool                         isAVSSampler,
    uint32_t                     &widthAlignUnit,
    uint32_t                     &heightAlignUnit)
{
    uint32_t wAlignUnit  = 0;
    uint32_t hAlignUnit = 0;

    if (m_renderItf)
    {
        m_renderItf->GetSamplerResolutionAlignUnit(
            isAVSSampler,
            wAlignUnit,
            hAlignUnit);
    }

    widthAlignUnit = wAlignUnit;
    heightAlignUnit = hAlignUnit;

}

MHW_RENDER_ENGINE_CAPS* XRenderHal_Platform_Interface_Next::GetHwCaps(
    PRENDERHAL_INTERFACE         pRenderHal)
{
    if (m_renderItf)
    {
        return (MHW_RENDER_ENGINE_CAPS * )m_renderItf->GetHwCaps();
    }
    else
    {
        return nullptr;
    }
}

bool XRenderHal_Platform_Interface_Next::IsComputeContextInUse(PRENDERHAL_INTERFACE pRenderHal)
{
    return true;
}

std::shared_ptr<mhw::mi::Itf> XRenderHal_Platform_Interface_Next::GetMhwMiItf()
{
    return m_miItf;
}
