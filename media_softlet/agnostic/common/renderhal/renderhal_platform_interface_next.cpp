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
#include "media_perf_profiler.h"

XRenderHal_Platform_Interface_Next::XRenderHal_Platform_Interface_Next()
{
    MOS_ZeroMemory(&m_scratchSpaceResource, sizeof(m_scratchSpaceResource));
}

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
    MHW_CHK_STATUS_RETURN(m_renderItf->MHW_ADDCMD_F(PIPELINE_SELECT)(pCmdBuffer));

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
            MHW_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(PIPE_CONTROL)(pCmdBuffer));
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
    MHW_RENDERHAL_CHK_NULL_RETURN(m_renderItf);

    MHW_RENDERHAL_CHK_STATUS_RETURN(m_renderItf->EnablePreemption(pCmdBuffer, m_miItf));

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
        MHW_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_FLUSH_DW)(pCmdBuffer));

        // load presPredication to general purpose register0
        auto& parRegM = m_miItf->MHW_GETPAR_F(MI_LOAD_REGISTER_MEM)();
        parRegM = {};
        parRegM.presStoreBuffer   = pRenderHal->PredicationParams.pPredicationResource;
        parRegM.dwOffset          = (uint32_t)pRenderHal->PredicationParams.predicationResOffset;
        parRegM.dwRegister        = mmioRegistersRender->generalPurposeRegister0LoOffset;
        MHW_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_LOAD_REGISTER_MEM)(pCmdBuffer));

        auto& parImm = m_miItf->MHW_GETPAR_F(MI_LOAD_REGISTER_IMM)();
        parImm = {};
        parImm.dwData            = 0;
        parImm.dwRegister        = mmioRegistersRender->generalPurposeRegister0HiOffset;
        MHW_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_LOAD_REGISTER_IMM)(pCmdBuffer));

        parImm = {};
        parImm.dwData            = 0;
        parImm.dwRegister        = mmioRegistersRender->generalPurposeRegister4LoOffset;
        MHW_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_LOAD_REGISTER_IMM)(pCmdBuffer));

        parImm = {};
        parImm.dwData            = 0;
        parImm.dwRegister        = mmioRegistersRender->generalPurposeRegister4HiOffset;
        MHW_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_LOAD_REGISTER_IMM)(pCmdBuffer));

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
        MHW_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_MATH)(pCmdBuffer));

        // if zero, the zero flag will be 0xFFFFFFFF, else zero flag will be 0x0.
        parRegM = {};
        parRegM.presStoreBuffer  = &pRenderHal->PredicationBuffer;
        parRegM.dwOffset         = 0x10;
        parRegM.dwRegister       = mmioRegistersRender->generalPurposeRegister0LoOffset;
        MHW_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_STORE_REGISTER_MEM)(pCmdBuffer));

        // Programming of 4 dummy MI_STORE_DATA_IMM commands prior to programming of MiConditionalBatchBufferEnd
        auto& parData = m_miItf->MHW_GETPAR_F(MI_STORE_DATA_IMM)();
        parData = {};
        parData.pOsResource = &pRenderHal->PredicationBuffer;
        parData.dwValue     = 1;
        MHW_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_STORE_DATA_IMM)(pCmdBuffer));

        parData.dwValue = 2;
        MHW_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_STORE_DATA_IMM)(pCmdBuffer));

        parData.dwValue = 3;
        MHW_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_STORE_DATA_IMM)(pCmdBuffer));

        parData.dwValue = 4;
        MHW_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_STORE_DATA_IMM)(pCmdBuffer));

        parFlush = {};
        parFlush.postSyncOperation = 1;
        parFlush.pOsResource       = &pRenderHal->PredicationBuffer;
        MHW_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_FLUSH_DW)(pCmdBuffer));

        auto& parBatch = m_miItf->MHW_GETPAR_F(MI_CONDITIONAL_BATCH_BUFFER_END)();
        parBatch = {};
        parBatch.presSemaphoreBuffer = &pRenderHal->PredicationBuffer;
        parBatch.dwOffset            = 0x10;
        parBatch.dwValue             = 0;
        parBatch.bDisableCompareMask = true;
        MHW_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_CONDITIONAL_BATCH_BUFFER_END)(pCmdBuffer));

        pRenderHal->PredicationParams.ptempPredicationBuffer = &pRenderHal->PredicationBuffer;
    }
    else
    {
        auto mmioRegistersRender = m_miItf->GetMmioRegisters();

        auto& parFlush = m_miItf->MHW_GETPAR_F(MI_FLUSH_DW)();
        parFlush = {};
        MHW_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_FLUSH_DW)(pCmdBuffer));

        // load presPredication to general purpose register0
        auto& parRegM = m_miItf->MHW_GETPAR_F(MI_LOAD_REGISTER_MEM)();
        parRegM = {};
        parRegM.presStoreBuffer = pRenderHal->PredicationParams.pPredicationResource;
        parRegM.dwOffset        = (uint32_t)pRenderHal->PredicationParams.predicationResOffset;
        parRegM.dwRegister      = mmioRegistersRender->generalPurposeRegister0LoOffset;
        MHW_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_LOAD_REGISTER_MEM)(pCmdBuffer));

        // if zero, the zero flag will be 0xFFFFFFFF, else zero flag will be 0x0.
        auto& parStore = m_miItf->MHW_GETPAR_F(MI_STORE_REGISTER_MEM)();
        parStore.presStoreBuffer = &pRenderHal->PredicationBuffer;
        parStore.dwOffset        = 0x10;
        parStore.dwRegister      = mmioRegistersRender->generalPurposeRegister0LoOffset;
        MHW_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_STORE_REGISTER_MEM)(pCmdBuffer));

        // Programming of 4 dummy MI_STORE_DATA_IMM commands prior to programming of MiConditionalBatchBufferEnd
        auto& parData = m_miItf->MHW_GETPAR_F(MI_STORE_DATA_IMM)();
        parData.pOsResource = &pRenderHal->PredicationBuffer;
        parData.dwValue     = 1;
        MHW_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_STORE_DATA_IMM)(pCmdBuffer));

        parData.dwValue = 2;
        MHW_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_STORE_DATA_IMM)(pCmdBuffer));

        parData.dwValue = 3;
        MHW_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_STORE_DATA_IMM)(pCmdBuffer));

        parData.dwValue = 4;
        MHW_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_STORE_DATA_IMM)(pCmdBuffer));

        parFlush = {};
        parFlush.postSyncOperation = 1;
        parFlush.pOsResource       = &pRenderHal->PredicationBuffer;
        MHW_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_FLUSH_DW)(pCmdBuffer));

        // Skip current frame if presPredication is equal to zero
        auto& parBatch = m_miItf->MHW_GETPAR_F(MI_CONDITIONAL_BATCH_BUFFER_END)();
        parBatch = {};
        parBatch.presSemaphoreBuffer = &pRenderHal->PredicationBuffer;
        parBatch.dwOffset            = 0x10;
        parBatch.dwValue             = 0;
        parBatch.bDisableCompareMask = true;
        MHW_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_CONDITIONAL_BATCH_BUFFER_END)(pCmdBuffer));
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
        MHW_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(PIPE_CONTROL)(cmdBuffer));
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
        MHW_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_FLUSH_DW)(cmdBuffer));
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
    par.bUnTypedDataPortCacheFlush = params->bUnTypedDataPortCacheFlush;
    par.bHdcPipelineFlush = params->bHdcPipelineFlush;
    par.bKernelFenceEnabled = params->bKernelFenceEnabled;
    MHW_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(PIPE_CONTROL)(pCmdBuffer));

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
    MHW_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_LOAD_REGISTER_IMM)(pCmdBuffer));

    // TD_CTL, force thread breakpoint enable
    // Also enable external halt/exceptions, because the source-level debugger
    // needs to be able to interrupt running EU threads.
    params->dwRegister = TD_CTL;
    params->dwData     = TD_CTL_FORCE_THREAD_BKPT_ENABLE |
                            TD_CTL_FORCE_EXT_EXCEPTION_ENABLE;

    par = {};
    par.dwData     = params->dwData;
    par.dwRegister = params->dwRegister;
    MHW_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_LOAD_REGISTER_IMM)(pCmdBuffer));

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

    pRenderHal->pCpInterface = mhwInterfaces->m_cpInterface;
    m_renderItf = mhwInterfaces->m_renderItf;
    m_miItf     = mhwInterfaces->m_miItf;
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

    HalOcaInterfaceNext::On1stLevelBBStart(*pCmdBuffer, (MOS_CONTEXT_HANDLE)pOsContext, pRenderHal->pOsInterface->CurrentGpuContextHandle,
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

    MHW_CHK_STATUS_RETURN(NullHW::StartPredicateNext(pRenderHal->pOsInterface, m_miItf, cmdBuffer));

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

    MHW_CHK_STATUS_RETURN(NullHW::StopPredicateNext(pRenderHal->pOsInterface, m_miItf, cmdBuffer));

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
    MHW_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MEDIA_STATE_FLUSH)(pCmdBuffer));

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

    MHW_CHK_STATUS_RETURN(m_miItf->AddMiBatchBufferEnd(pCmdBuffer, nullptr));

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

bool XRenderHal_Platform_Interface_Next::IsSampler128ElementsSupported()
{
    return true;
}

bool XRenderHal_Platform_Interface_Next::PerThreadScratchSpaceStart2K(PRENDERHAL_INTERFACE pRenderHal)
{
    MHW_RENDERHAL_UNUSED(pRenderHal);
    return false;
}

bool XRenderHal_Platform_Interface_Next::PerThreadScratchSpaceStart64Byte(RENDERHAL_INTERFACE *renderHal)
{
    MHW_RENDERHAL_UNUSED(renderHal);
    return true;
}

uint32_t XRenderHal_Platform_Interface_Next::CalculatePreferredSlmAllocationSizeFromSlmSize(
    RENDERHAL_INTERFACE *renderHal, 
    uint32_t             slmSize, 
    uint32_t             numberOfThreadsPerThreadGroup)
{
    if (!renderHal || !renderHal->pOsInterface || !renderHal->pOsInterface->pfnGetGtSystemInfo)
    {
        MHW_RENDERHAL_ASSERTMESSAGE("renderhal or osInterface or pfnGetGtSystemInfo is nullptr");
        return 0;
    }
    if (numberOfThreadsPerThreadGroup == 0)
    {
        MHW_RENDERHAL_ASSERTMESSAGE("numberOfThreadsPerThreadGroup is 0");
        return 0;
    }

    MEDIA_SYSTEM_INFO *gtInfo = renderHal->pOsInterface->pfnGetGtSystemInfo(renderHal->pOsInterface);
    if (!gtInfo)
    {
        MHW_RENDERHAL_ASSERTMESSAGE("GtSystemInfo is nullptr");
        return 0;
    }
    if (gtInfo->SubSliceCount == 0)
    {
        MHW_RENDERHAL_ASSERTMESSAGE("SubSliceCount is 0");
        return 0;
    }
    uint32_t preferredSlmAllicationSize = 0;
    slmSize                             = slmSize / 1024 + (slmSize % 1024 != 0);
    uint32_t threadsPerDssCount         = gtInfo->ThreadCount / gtInfo->SubSliceCount;
    uint32_t workGroupCountPerDss       = (threadsPerDssCount + numberOfThreadsPerThreadGroup - 1) / numberOfThreadsPerThreadGroup;
    uint32_t slmSizePerSubSlice         = slmSize * workGroupCountPerDss;
    if (slmSize == 0)
    {
        preferredSlmAllicationSize = 0;
    }
    else if (slmSizePerSubSlice <= 16)
    {
        preferredSlmAllicationSize = 1;
    }
    else if (slmSizePerSubSlice <= 32)
    {
        preferredSlmAllicationSize = 2;
    }
    else if (slmSizePerSubSlice > 256)
    {
        if (slmSizePerSubSlice > 384)
        {
            MHW_RENDERHAL_ASSERTMESSAGE("slmSizePerSubSlice %d is bigger than max size", slmSizePerSubSlice);
        }
        else
        {
            preferredSlmAllicationSize = 10;
        }
    }
    else
    {
        preferredSlmAllicationSize = slmSizePerSubSlice / 32 + (slmSizePerSubSlice % 32 != 0) + 1;
    }
    
    return preferredSlmAllicationSize;
}

uint32_t XRenderHal_Platform_Interface_Next::EncodeSLMSize(uint32_t SLMSize)
{
    uint32_t EncodedValue;
    SLMSize = SLMSize / 1024 + (SLMSize % 1024 != 0);

    if (SLMSize <= 2)
    {
        EncodedValue = SLMSize;
    }
    else 
    {
        EncodedValue = 0;
        do
        {
            if (SLMSize != 1 && (SLMSize & 0x1) != 0)
            {
                ++SLMSize;
            }
            SLMSize >>= 1;
            EncodedValue++;
        } while (SLMSize);
    }
    return EncodedValue;
}

uint8_t XRenderHal_Platform_Interface_Next::SetChromaDirection(
    PRENDERHAL_INTERFACE pRenderHal,
    PRENDERHAL_SURFACE   pRenderHalSurface)
{
    uint8_t Direction;
    MHW_RENDERHAL_UNUSED(pRenderHal);

    MHW_RENDERHAL_ASSERT(pRenderHalSurface);

    Direction = 0;

    if (pRenderHalSurface->ChromaSiting & MHW_CHROMA_SITING_HORZ_CENTER)
    {
        Direction = CHROMA_SITING_UDIRECTION_CENTER;
    }
    else
    {
        Direction = CHROMA_SITING_UDIRECTION_LEFT;
    }

    // Combined U/V direction together in one uint8_t, 1 bit for U direction, 3 bits for V direction.
    Direction = Direction << 3;

    if (pRenderHalSurface->pDeinterlaceParams || pRenderHalSurface->bQueryVariance)
    {
        if ((pRenderHalSurface->SampleType == RENDERHAL_SAMPLE_INTERLEAVED_EVEN_FIRST_BOTTOM_FIELD) ||
            (pRenderHalSurface->SampleType == RENDERHAL_SAMPLE_INTERLEAVED_ODD_FIRST_BOTTOM_FIELD))
        {
            if (pRenderHalSurface->ChromaSiting & MHW_CHROMA_SITING_VERT_TOP)
            {
                Direction |= CHROMA_SITING_VDIRECTION_1_2;
            }
            else if (pRenderHalSurface->ChromaSiting & MHW_CHROMA_SITING_VERT_BOTTOM)
            {
                Direction |= CHROMA_SITING_VDIRECTION_1;
            }
            else
            {
                Direction |= CHROMA_SITING_VDIRECTION_3_4;
            }
        }
        else if ((pRenderHalSurface->SampleType == RENDERHAL_SAMPLE_INTERLEAVED_EVEN_FIRST_TOP_FIELD) ||
            (pRenderHalSurface->SampleType == RENDERHAL_SAMPLE_INTERLEAVED_ODD_FIRST_TOP_FIELD))
        {
            if (pRenderHalSurface->ChromaSiting & MHW_CHROMA_SITING_VERT_TOP)
            {
                Direction |= CHROMA_SITING_VDIRECTION_0;
            }
            else if (pRenderHalSurface->ChromaSiting & MHW_CHROMA_SITING_VERT_BOTTOM)
            {
                Direction |= CHROMA_SITING_VDIRECTION_1_2;
            }
            else
            {
                Direction |= CHROMA_SITING_VDIRECTION_1_4;
            }
        }
    }
    else
    {
        if (pRenderHalSurface->ChromaSiting & MHW_CHROMA_SITING_VERT_TOP)
        {
            Direction |= CHROMA_SITING_VDIRECTION_0;
        }
        else if (pRenderHalSurface->ChromaSiting & MHW_CHROMA_SITING_VERT_BOTTOM)
        {
            Direction |= CHROMA_SITING_VDIRECTION_1;
        }
        else
        {
            Direction |= CHROMA_SITING_VDIRECTION_1_2;
        }
    }

    return Direction;
}

void XRenderHal_Platform_Interface_Next::InitSurfaceTypes(PRENDERHAL_INTERFACE    pRenderHal)
{
    MHW_RENDERHAL_CHK_NULL_NO_STATUS_RETURN(pRenderHal);
    // Set default / advanced surface types
    pRenderHal->SurfaceTypeDefault            = RENDERHAL_SURFACE_TYPE_G10;
    pRenderHal->SurfaceTypeAdvanced           = RENDERHAL_SURFACE_TYPE_ADV_G10;
}

bool XRenderHal_Platform_Interface_Next::IsEnableYV12SinglePass(PRENDERHAL_INTERFACE pRenderHal)
{
    MHW_RENDERHAL_UNUSED(pRenderHal);
    return true;
}

uint32_t XRenderHal_Platform_Interface_Next::GetSizeSamplerStateAvs(PRENDERHAL_INTERFACE pRenderHal)
{
    if (pRenderHal && pRenderHal->pHwSizes)
    {
        return 2 * pRenderHal->pHwSizes->dwSizeSamplerStateAvs;  // Kernel using 1,3,5 sampler index for AVS sampler state.
    }
    else
    {
        MHW_RENDERHAL_ASSERTMESSAGE("Failed to get SizeSamplerStateAvs");
        return 0;
    }
}

MOS_STATUS XRenderHal_Platform_Interface_Next::SetPowerOptionStatus(
    PRENDERHAL_INTERFACE         pRenderHal,
    PMOS_COMMAND_BUFFER          pCmdBuffer)
{
    PMOS_INTERFACE              pOsInterface;
    MOS_STATUS                  eStatus;
    MEDIA_SYSTEM_INFO           *pGtSystemInfo;

    MHW_RENDERHAL_CHK_NULL_RETURN(pRenderHal);
    MHW_RENDERHAL_CHK_NULL_RETURN(pCmdBuffer);
    MHW_RENDERHAL_CHK_NULL_RETURN(pRenderHal->pOsInterface);

    eStatus         = MOS_STATUS_SUCCESS;
    pOsInterface    = pRenderHal->pOsInterface;
    pGtSystemInfo   = pOsInterface->pfnGetGtSystemInfo(pOsInterface);
    MHW_RENDERHAL_CHK_NULL_RETURN(pGtSystemInfo);

    // Check if Slice Shutdown can be enabled
    if (pRenderHal->bRequestSingleSlice)
    {
        pCmdBuffer->Attributes.dwNumRequestedEUSlices = 1;
    }
    else if (pRenderHal->bEUSaturationNoSSD)
    {
        pCmdBuffer->Attributes.dwNumRequestedEUSlices = 2;
    }

    if ((pRenderHal->pSkuTable) && (MEDIA_IS_SKU(pRenderHal->pSkuTable, FtrSSEUPowerGating) || MEDIA_IS_SKU(pRenderHal->pSkuTable, FtrSSEUPowerGatingControlByUMD)))
    {
        // VP does not request subslice shutdown according to the array VpHalDefaultSSEUTableGxx
        if (((pRenderHal->PowerOption.nSlice != 0) || (pRenderHal->PowerOption.nSubSlice != 0) || (pRenderHal->PowerOption.nEU != 0)) &&
            ((pGtSystemInfo->SliceCount != 0) && (pGtSystemInfo->SubSliceCount != 0)))
        {
            pCmdBuffer->Attributes.dwNumRequestedEUSlices    = MOS_MIN(pRenderHal->PowerOption.nSlice, pGtSystemInfo->SliceCount);
            pCmdBuffer->Attributes.dwNumRequestedSubSlices   = MOS_MIN(pRenderHal->PowerOption.nSubSlice, (pGtSystemInfo->SubSliceCount / pGtSystemInfo->SliceCount));
            pCmdBuffer->Attributes.dwNumRequestedEUs         = MOS_MIN(pRenderHal->PowerOption.nEU, (pGtSystemInfo->EUCount / pGtSystemInfo->SubSliceCount));
            pCmdBuffer->Attributes.bValidPowerGatingRequest  = true;
            pCmdBuffer->Attributes.bUmdSSEUEnable            = true;
        }
    }

    return eStatus;
}

MOS_STATUS XRenderHal_Platform_Interface_Next::SetCompositePrologCmd(
    PRENDERHAL_INTERFACE pRenderHal, 
    PMOS_COMMAND_BUFFER  pCmdBuffer)
{
    MOS_STATUS                            eStatus = MOS_STATUS_SUCCESS;
    uint64_t                              auxTableBaseAddr = 0;

    MHW_RENDERHAL_CHK_NULL_RETURN(pRenderHal);
    MHW_RENDERHAL_CHK_NULL_RETURN(pCmdBuffer);
    MHW_RENDERHAL_CHK_NULL_RETURN(pRenderHal->pOsInterface);
    MHW_RENDERHAL_CHK_NULL_RETURN(m_miItf);

    auxTableBaseAddr = pRenderHal->pOsInterface->pfnGetAuxTableBaseAddr(pRenderHal->pOsInterface);

    if (auxTableBaseAddr)
    {
        auto& parImm = m_miItf->MHW_GETPAR_F(MI_LOAD_REGISTER_IMM)();
        parImm = {};
        parImm.dwRegister = m_miItf->GetMmioInterfaces(mhw::mi::MHW_MMIO_RCS_AUX_TABLE_BASE_LOW);
        parImm.dwData = (auxTableBaseAddr & 0xffffffff);
        MHW_MI_CHK_STATUS(m_miItf->MHW_ADDCMD_F(MI_LOAD_REGISTER_IMM)(pCmdBuffer));

        parImm.dwRegister = m_miItf->GetMmioInterfaces(mhw::mi::MHW_MMIO_RCS_AUX_TABLE_BASE_HIGH);
        parImm.dwData = ((auxTableBaseAddr >> 32) & 0xffffffff);
        MHW_MI_CHK_STATUS(m_miItf->MHW_ADDCMD_F(MI_LOAD_REGISTER_IMM)(pCmdBuffer));

        parImm.dwRegister = m_miItf->GetMmioInterfaces(mhw::mi::MHW_MMIO_CCS0_AUX_TABLE_BASE_LOW);
        parImm.dwData = (auxTableBaseAddr & 0xffffffff);
        MHW_MI_CHK_STATUS(m_miItf->MHW_ADDCMD_F(MI_LOAD_REGISTER_IMM)(pCmdBuffer));

        parImm.dwRegister = m_miItf->GetMmioInterfaces(mhw::mi::MHW_MMIO_CCS0_AUX_TABLE_BASE_HIGH);
        parImm.dwData = ((auxTableBaseAddr >> 32) & 0xffffffff);
        MHW_MI_CHK_STATUS(m_miItf->MHW_ADDCMD_F(MI_LOAD_REGISTER_IMM)(pCmdBuffer));
    }

    return eStatus;
}

MOS_STATUS XRenderHal_Platform_Interface_Next::SendComputeWalker(
    PRENDERHAL_INTERFACE     pRenderHal,
    PMOS_COMMAND_BUFFER      pCmdBuffer,
    PMHW_GPGPU_WALKER_PARAMS pGpGpuWalkerParams)
{
    VP_FUNC_CALL();

    MOS_STATUS                eStatus = MOS_STATUS_SUCCESS;
    MHW_ID_ENTRY_PARAMS       mhwIdEntryParams;
    PRENDERHAL_KRN_ALLOCATION pKernelEntry;
    PRENDERHAL_MEDIA_STATE    pCurMediaState;

    MHW_RENDERHAL_CHK_NULL_RETURN(pRenderHal);
    MHW_RENDERHAL_CHK_NULL_RETURN(pCmdBuffer);
    MHW_RENDERHAL_CHK_NULL_RETURN(pGpGpuWalkerParams);
    MHW_RENDERHAL_CHK_NULL_RETURN(pRenderHal->pStateHeap);
    MHW_RENDERHAL_CHK_NULL_RETURN(pRenderHal->pStateHeap->pKernelAllocation);

    MOS_ZeroMemory(&mhwIdEntryParams, sizeof(mhwIdEntryParams));

    pKernelEntry   = &pRenderHal->pStateHeap->pKernelAllocation[pRenderHal->iKernelAllocationID];
    pCurMediaState = pRenderHal->pStateHeap->pCurMediaState;

    MHW_RENDERHAL_CHK_NULL_RETURN(pKernelEntry);
    MHW_RENDERHAL_CHK_NULL_RETURN(pCurMediaState);

    mhwIdEntryParams.dwKernelOffset  = pKernelEntry->dwOffset;
    mhwIdEntryParams.dwSamplerCount  = pKernelEntry->Params.Sampler_Count;
    mhwIdEntryParams.dwSamplerOffset = pCurMediaState->dwOffset +
                                       pRenderHal->pStateHeap->dwOffsetSampler +
                                       pGpGpuWalkerParams->InterfaceDescriptorOffset * pRenderHal->pStateHeap->dwSizeSamplers;
    mhwIdEntryParams.dwBindingTableOffset          = pGpGpuWalkerParams->BindingTableID * pRenderHal->pStateHeap->iBindingTableSize;
    mhwIdEntryParams.dwSharedLocalMemorySize       = m_renderHal->pfnEncodeSLMSize(m_renderHal, pGpGpuWalkerParams->SLMSize);
    if (pGpGpuWalkerParams->isGenerateLocalID && pGpGpuWalkerParams->emitLocal != MHW_EMIT_LOCAL_NONE)
    {
        //When COMPUTE_WALKER Emit Local ID is enabled, thread group number need to divide MHW_RENDER_ENGINE_NUMBER_OF_THREAD_UNIT
        uint32_t engineNumberOfThreadUnit              = (pGpGpuWalkerParams->simdSize == 16) ? 16 : MHW_RENDER_ENGINE_NUMBER_OF_THREAD_UNIT;
        mhwIdEntryParams.dwNumberofThreadsInGPGPUGroup = MOS_MAX(1, pGpGpuWalkerParams->ThreadWidth * pGpGpuWalkerParams->ThreadHeight / engineNumberOfThreadUnit);
        if (mhwIdEntryParams.dwNumberofThreadsInGPGPUGroup > 1024 / engineNumberOfThreadUnit)
        {
            MHW_RENDERHAL_ASSERTMESSAGE("Number of Threads In GpGpuGroup %d Exceeds the Max Number %d", mhwIdEntryParams.dwNumberofThreadsInGPGPUGroup, 1024 / engineNumberOfThreadUnit);
            MHW_RENDERHAL_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
        }
    }
    else
    {
        mhwIdEntryParams.dwNumberofThreadsInGPGPUGroup = pGpGpuWalkerParams->ThreadWidth * pGpGpuWalkerParams->ThreadHeight;
    }
    mhwIdEntryParams.preferredSlmAllocationSize = CalculatePreferredSlmAllocationSizeFromSlmSize(m_renderHal, pGpGpuWalkerParams->SLMSize, mhwIdEntryParams.dwNumberofThreadsInGPGPUGroup);
    //This only a WA to disable EU fusion for multi-layer blending cases or single layer do colorfill and rotation together.
    //Need remove it after kernel or compiler fix it.
    mhwIdEntryParams.bBarrierEnable              = pRenderHal->eufusionBypass ? 1 : 0;
    mhwIdEntryParams.bBarrierEnable             |= pGpGpuWalkerParams->hasBarrier;
    pGpGpuWalkerParams->IndirectDataStartAddress = pGpGpuWalkerParams->IndirectDataStartAddress + pRenderHal->pStateHeap->pCurMediaState->dwOffset;

    MHW_RENDERHAL_CHK_NULL_RETURN(m_renderItf);
    m_gpgpuWalkerParams         = pGpGpuWalkerParams;
    m_interfaceDescriptorParams = &mhwIdEntryParams;
    SETPAR_AND_ADDCMD(COMPUTE_WALKER, m_renderItf, pCmdBuffer);

    return eStatus;
}

MOS_STATUS XRenderHal_Platform_Interface_Next::SendTo3DStateBindingTablePoolAlloc(
    PRENDERHAL_INTERFACE pRenderHal,
    PMOS_COMMAND_BUFFER  pCmdBuffer)
{
    VP_FUNC_CALL();

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;
    MHW_RENDERHAL_CHK_NULL_RETURN(pRenderHal);
    MHW_RENDERHAL_CHK_NULL_RETURN(pCmdBuffer);
    MHW_RENDERHAL_CHK_NULL_RETURN(m_renderItf);
    SETPAR_AND_ADDCMD(_3DSTATE_BINDING_TABLE_POOL_ALLOC, m_renderItf, pCmdBuffer);

    return eStatus;
}

MOS_STATUS XRenderHal_Platform_Interface_Next::IsRenderHalMMCEnabled(
    PRENDERHAL_INTERFACE         pRenderHal)
{
    VP_FUNC_CALL();

    MOS_STATUS    eStatus     = MOS_STATUS_SUCCESS;
    bool          isMMCEnabled = false;

    MHW_RENDERHAL_CHK_NULL_RETURN(pRenderHal);


#if defined(LINUX) && (!defined(WDDM_LINUX))
    isMMCEnabled = !MEDIA_IS_WA(pRenderHal->pWaTable, WaDisableVPMmc) || !MEDIA_IS_WA(pRenderHal->pWaTable, WaDisableCodecMmc);
#else
    isMMCEnabled = true;  // turn on MMC
#endif

#if (_DEBUG || _RELEASE_INTERNAL)
    // Read reg key to set MMC for Fast Composition surfaces
    if (pRenderHal->userSettingPtr != nullptr)
    {
        ReadUserSettingForDebug(
            pRenderHal->userSettingPtr,
            isMMCEnabled,
            __MEDIA_USER_FEATURE_ENABLE_RENDER_ENGINE_MMC,
            MediaUserSetting::Group::Device,
            isMMCEnabled,
            true);
    }
#endif
    m_renderHalMMCEnabled    = isMMCEnabled && MEDIA_IS_SKU(pRenderHal->pSkuTable, FtrE2ECompression);
    pRenderHal->isMMCEnabled = m_renderHalMMCEnabled;

    return eStatus;
}

MOS_STATUS XRenderHal_Platform_Interface_Next::IsOvrdNeeded(
    PRENDERHAL_INTERFACE              pRenderHal,
    PMOS_COMMAND_BUFFER               pCmdBuffer,
    PRENDERHAL_GENERIC_PROLOG_PARAMS  pGenericPrologParams)
{
    PMOS_INTERFACE                        pOsInterface;
    MOS_STATUS                            eStatus;
    PMOS_CMD_BUF_ATTRI_VE                 pAttriVe;
    PRENDERHAL_GENERIC_PROLOG_PARAMS_NEXT pGenericPrologParamsNext;
    uint8_t                               i;

    MHW_RENDERHAL_CHK_NULL_RETURN(pRenderHal);
    MHW_RENDERHAL_CHK_NULL_RETURN(pCmdBuffer);
    MHW_RENDERHAL_CHK_NULL_RETURN(pRenderHal->pOsInterface);
    MHW_RENDERHAL_CHK_NULL_RETURN(pRenderHal->pRenderHalPltInterface);

    eStatus                 = MOS_STATUS_SUCCESS;
    pOsInterface            = pRenderHal->pOsInterface;
    pAttriVe               = (PMOS_CMD_BUF_ATTRI_VE)(pCmdBuffer->Attributes.pAttriVe);
    pGenericPrologParamsNext = dynamic_cast<PRENDERHAL_GENERIC_PROLOG_PARAMS_NEXT>(pGenericPrologParams);

    // Split Frame
    if (pOsInterface->VEEnable)
    {
#if !EMUL
        if (pGenericPrologParamsNext)
#else
        if (pGenericPrologParamsNext && pAttriVe != nullptr)
#endif
        {
            // Split Frame
            if (pGenericPrologParamsNext->VEngineHintParams.BatchBufferCount > 1)
            {
                pAttriVe->bUseVirtualEngineHint = true;
                pAttriVe->VEngineHintParams = pGenericPrologParamsNext->VEngineHintParams;
            }
        }

#if (_DEBUG || _RELEASE_INTERNAL)
#if !EMUL
        if (pOsInterface->bEnableDbgOvrdInVE)
#else
        if (pOsInterface->bEnableDbgOvrdInVE && pAttriVe != nullptr)
#endif
        {
            if (pOsInterface->bVeboxScalabilityMode)
            {
                pAttriVe->VEngineHintParams.DebugOverride = true;
#if !EMUL
                if (pGenericPrologParamsNext)
#else
                if (pGenericPrologParamsNext && pAttriVe != nullptr)
#endif
                {
                    pAttriVe->VEngineHintParams.BatchBufferCount = pGenericPrologParamsNext->VEngineHintParams.BatchBufferCount;
                    for (i = 0; i < pGenericPrologParamsNext->VEngineHintParams.BatchBufferCount; i++)
                    {
                        pAttriVe->VEngineHintParams.EngineInstance[i] = i;
                    }
                }
            }
            else if (pOsInterface->eForceVebox)
            {
                pAttriVe->VEngineHintParams.DebugOverride = true;
                pAttriVe->VEngineHintParams.BatchBufferCount = 1;
                pAttriVe->VEngineHintParams.EngineInstance[0] = pOsInterface->eForceVebox - 1;
            }
        }
#endif
    }

    return eStatus;
}

MOS_STATUS XRenderHal_Platform_Interface_Next::AllocateScratchSpaceBuffer(
    uint32_t                perThreadScratchSpace,
    RENDERHAL_INTERFACE     *renderHal)
{
    VP_FUNC_CALL();

    if (m_scratchSpaceResource.iSize > 0)
    {  // Already allocated.
        return MOS_STATUS_SUCCESS;
    }

    const MEDIA_SYSTEM_INFO *gt_sys_info = renderHal->pOsInterface
                                               ->pfnGetGtSystemInfo(renderHal->pOsInterface);
    uint32_t hw_threads_per_eu     = gt_sys_info->ThreadCount / gt_sys_info->EUCount;
    uint32_t scratch_space_entries = gt_sys_info->MaxEuPerSubSlice * hw_threads_per_eu * gt_sys_info->MaxSubSlicesSupported;
    uint32_t scratch_space_size    = scratch_space_entries * perThreadScratchSpace;

    MOS_ALLOC_GFXRES_PARAMS alloc_param;
    MOS_ZeroMemory(&alloc_param, sizeof(alloc_param));
    alloc_param.Type          = MOS_GFXRES_SCRATCH;
    alloc_param.Format        = Format_Buffer;
    alloc_param.dwBytes       = scratch_space_size;
    alloc_param.pSystemMemory = nullptr;
    alloc_param.TileType      = MOS_TILE_LINEAR;
    alloc_param.pBufName      = "ScratchSpaceBuffer";

    return renderHal->pOsInterface->pfnAllocateResource(
        renderHal->pOsInterface, &alloc_param, &m_scratchSpaceResource);
}

MOS_STATUS XRenderHal_Platform_Interface_Next::FreeScratchSpaceBuffer(
    RENDERHAL_INTERFACE *renderHal)
{
    MOS_GFXRES_FREE_FLAGS resFreeFlags = {0};

    resFreeFlags.AssumeNotInUse = 1;

    if (m_scratchSpaceResource.iSize <= 0)
    {
        return MOS_STATUS_SUCCESS;  // Scratch space is not allocated. No need to free resources.
    }

    renderHal->pOsInterface
            ->pfnFreeResourceWithFlag(renderHal->pOsInterface,
                                      &m_scratchSpaceResource,
                                      resFreeFlags.Value);
    renderHal->pOsInterface
            ->pfnResetResourceAllocationIndex(renderHal->pOsInterface,
                                              &m_scratchSpaceResource);
    return MOS_STATUS_SUCCESS;
}

bool XRenderHal_Platform_Interface_Next::IsFormatMMCSupported(MOS_FORMAT format)
{
    // Check if Sample Format is supported
    if ((format != Format_YUY2)             &&
        (format != Format_Y410)             &&
        (format != Format_Y216)             &&
        (format != Format_Y210)             &&
        (format != Format_Y416)             &&
        (format != Format_P010)             &&
        (format != Format_P016)             &&
        (format != Format_AYUV)             &&
        (format != Format_NV21)             &&
        (format != Format_NV12)             &&
        (format != Format_UYVY)             &&
        (format != Format_YUYV)             &&
        (format != Format_A8B8G8R8)         &&
        (format != Format_X8B8G8R8)         &&
        (format != Format_A8R8G8B8)         &&
        (format != Format_X8R8G8B8)         &&
        (format != Format_B10G10R10A2)      &&
        (format != Format_R10G10B10A2)      &&
        (format != Format_A16R16G16B16F)    &&
        (format != Format_A16B16G16R16F)    &&
        (format != Format_IMC3)             &&
        (format != Format_444P)             &&
        (format != Format_422H)             &&
        (format != Format_422V)             &&
        (format != Format_411P)             &&
        (format != Format_411R)             &&
        (format != Format_444P)             &&
        (format != Format_RGBP)             &&
        (format != Format_BGRP)             &&
        (format != Format_400P)             &&
        (format != Format_420O)             &&
        (format != Format_R8UN)             &&
        (format != Format_A8)               &&
        (format != Format_R8G8UN))
    {
        MHW_RENDERHAL_NORMALMESSAGE("Unsupported Format '0x%08x' for Render MMC.", format);
        return false;
    }

    return true;
}

bool XRenderHal_Platform_Interface_Next::IsL8FormatSupported()
{
    return true;
}

std::shared_ptr<mhw::mi::Itf> XRenderHal_Platform_Interface_Next::GetMhwMiItf()
{
    return m_miItf;
}

MHW_SETPAR_DECL_SRC(STATE_BASE_ADDRESS, XRenderHal_Platform_Interface_Next)
{
    MHW_STATE_BASE_ADDR_PARAMS* pStateBaseParams = nullptr;
    MHW_RENDERHAL_CHK_NULL_RETURN(m_renderHal);
    pStateBaseParams = &m_renderHal->StateBaseAddressParams;
    MHW_RENDERHAL_CHK_NULL_RETURN(pStateBaseParams);

    params.presGeneralState = pStateBaseParams->presGeneralState;
    params.dwGeneralStateSize = pStateBaseParams->dwGeneralStateSize;
    params.presDynamicState = pStateBaseParams->presDynamicState;
    params.dwDynamicStateSize = pStateBaseParams->dwDynamicStateSize;
    params.bDynamicStateRenderTarget = pStateBaseParams->bDynamicStateRenderTarget;
    params.presIndirectObjectBuffer = pStateBaseParams->presIndirectObjectBuffer;
    params.dwIndirectObjectBufferSize = pStateBaseParams->dwIndirectObjectBufferSize;
    params.presInstructionBuffer = pStateBaseParams->presInstructionBuffer;
    params.dwInstructionBufferSize = pStateBaseParams->dwInstructionBufferSize;
    params.mocs4InstructionCache = pStateBaseParams->mocs4InstructionCache;
    params.mocs4GeneralState = pStateBaseParams->mocs4GeneralState;
    params.mocs4DynamicState = pStateBaseParams->mocs4DynamicState;
    params.mocs4SurfaceState = pStateBaseParams->mocs4SurfaceState;
    params.mocs4IndirectObjectBuffer = pStateBaseParams->mocs4IndirectObjectBuffer;
    params.mocs4StatelessDataport = pStateBaseParams->mocs4StatelessDataport;
    params.addressDis                 = m_renderHal->isBindlessHeapInUse;

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(_3DSTATE_CHROMA_KEY, XRenderHal_Platform_Interface_Next)
{
    MOS_STATUS                  eStatus = MOS_STATUS_SUCCESS;
    PMHW_CHROMAKEY_PARAMS       pChromaKeyParams = nullptr;
    MHW_RENDERHAL_CHK_NULL_RETURN(m_renderHal);
    pChromaKeyParams = m_renderHal->ChromaKey;
    MHW_RENDERHAL_CHK_NULL_RETURN(pChromaKeyParams);

    params.dwIndex = pChromaKeyParams->dwIndex;
    params.dwLow = pChromaKeyParams->dwLow;
    params.dwHigh = pChromaKeyParams->dwHigh;

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(STATE_SIP, XRenderHal_Platform_Interface_Next)
{
    MOS_STATUS                  eStatus = MOS_STATUS_SUCCESS;
    MHW_SIP_STATE_PARAMS        SipStateParams = {};
    MHW_RENDERHAL_CHK_NULL_RETURN(m_renderHal);
    SipStateParams = m_renderHal->SipStateParams;

    params.dwSipBase = SipStateParams.dwSipBase;

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(COMPUTE_WALKER, XRenderHal_Platform_Interface_Next)
{
    MHW_RENDERHAL_CHK_NULL_RETURN(m_gpgpuWalkerParams);
    MHW_RENDERHAL_CHK_NULL_RETURN(m_interfaceDescriptorParams);

    params.simdSize                 = m_gpgpuWalkerParams->simdSize;
    params.IndirectDataLength       = m_gpgpuWalkerParams->IndirectDataLength;
    params.IndirectDataStartAddress = m_gpgpuWalkerParams->IndirectDataStartAddress;
    params.ThreadWidth              = m_gpgpuWalkerParams->ThreadWidth;
    params.ThreadHeight             = m_gpgpuWalkerParams->ThreadHeight;
    params.ThreadDepth              = m_gpgpuWalkerParams->ThreadDepth;

    params.GroupWidth     = m_gpgpuWalkerParams->GroupWidth;
    params.GroupHeight    = m_gpgpuWalkerParams->GroupHeight;
    params.GroupDepth     = m_gpgpuWalkerParams->GroupDepth;
    params.GroupStartingX = m_gpgpuWalkerParams->GroupStartingX;
    params.GroupStartingY = m_gpgpuWalkerParams->GroupStartingY;
    params.GroupStartingZ = m_gpgpuWalkerParams->GroupStartingZ;

    params.dwKernelOffset                = m_interfaceDescriptorParams->dwKernelOffset;
    params.dwSamplerCount                = m_interfaceDescriptorParams->dwSamplerCount;
    params.dwSamplerOffset               = m_interfaceDescriptorParams->dwSamplerOffset;
    params.dwBindingTableOffset          = m_interfaceDescriptorParams->dwBindingTableOffset;
    params.bBarrierEnable                = m_interfaceDescriptorParams->bBarrierEnable;
    params.dwNumberofThreadsInGPGPUGroup = m_interfaceDescriptorParams->dwNumberofThreadsInGPGPUGroup;
    params.dwSharedLocalMemorySize       = m_interfaceDescriptorParams->dwSharedLocalMemorySize;
    params.preferredSlmAllocationSize    = m_interfaceDescriptorParams->preferredSlmAllocationSize;
    params.IndirectDataStartAddress      = m_gpgpuWalkerParams->IndirectDataStartAddress;
    params.forcePreferredSLMZero         = m_gpgpuWalkerParams->ForcePreferredSLMZero;

    if (m_gpgpuWalkerParams->ThreadDepth == 0)
    {
        params.ThreadDepth = 1;
    }
    if (m_gpgpuWalkerParams->GroupDepth == 0)
    {
        params.GroupDepth = 1;
    }
    if (m_gpgpuWalkerParams->simdSize == 0)
    {
        params.simdSize = 32;
    }

    params.isEmitInlineParameter = m_gpgpuWalkerParams->isEmitInlineParameter;
    if (m_gpgpuWalkerParams->inlineDataLength > 0 && m_gpgpuWalkerParams->inlineData)
    {
        params.inlineDataLength = m_gpgpuWalkerParams->inlineDataLength;
        params.inlineData       = m_gpgpuWalkerParams->inlineData;
    }

    params.isGenerateLocalId = m_gpgpuWalkerParams->isGenerateLocalID;
    params.emitLocal         = mhw::render::MHW_EMIT_LOCAL_MODE(m_gpgpuWalkerParams->emitLocal);

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(_3DSTATE_BINDING_TABLE_POOL_ALLOC, XRenderHal_Platform_Interface_Next)
{
    MHW_STATE_BASE_ADDR_PARAMS *pStateBaseParams = nullptr;
    MHW_RENDERHAL_CHK_NULL_RETURN(m_renderHal);
    pStateBaseParams = &m_renderHal->StateBaseAddressParams;
    MHW_RENDERHAL_CHK_NULL_RETURN(pStateBaseParams);
    params.mocs4SurfaceState = pStateBaseParams->mocs4SurfaceState;

    return MOS_STATUS_SUCCESS;
}