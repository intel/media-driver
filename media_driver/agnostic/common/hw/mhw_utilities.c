/*
* Copyright (c) 2014-2021, Intel Corporation
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
//! \file      mhw_utilities.c
//! \brief         This modules implements utilities which are shared by both the HW interface     and the state heap interface.
//!
#include "mhw_utilities.h"
#include "mhw_render_legacy.h"
#include "mhw_state_heap.h"
#include "hal_oca_interface.h"
#include "mos_interface.h"
#include "mhw_mi_itf.h"

//!
//! \brief    Inserts the generic prologue command for a command buffer
//! \details  Client facing function to add the generic prologue commands:
//!               - the command buffer header (if necessary)
//!               - flushes for the read/write caches (MI_FLUSH_DW or PIPE_CONTROL)
//!               - CP prologue if necessary
//! \param    PMOS_COMMAND_BUFFER pCmdBuffer
//!           [in] Command buffer
//! \param    PMHW_GENERIC_PROLOG_PARAMS pParams
//!           [in] Parameters necessary to add the generic prologue commands
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success, else fail reason
//!
MOS_STATUS Mhw_SendGenericPrologCmd(
    PMOS_COMMAND_BUFFER         pCmdBuffer,
    PMHW_GENERIC_PROLOG_PARAMS  pParams,
    MHW_MI_MMIOREGISTERS        *pMmioReg)
{
    PMOS_INTERFACE                  pOsInterface;
    MhwMiInterface                  *pMiInterface;
    MEDIA_FEATURE_TABLE             *pSkuTable;
    MEDIA_WA_TABLE                  *pWaTable;
    MOS_GPU_CONTEXT                 GpuContext;
    MHW_PIPE_CONTROL_PARAMS         PipeControlParams;
    MHW_MI_FLUSH_DW_PARAMS          FlushDwParams;
    bool                            bRcsEngineUsed = false;
    MOS_STATUS                      eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    MHW_CHK_NULL(pCmdBuffer);
    MHW_CHK_NULL(pParams);
    MHW_CHK_NULL(pParams->pOsInterface);

    pOsInterface = pParams->pOsInterface;

    MHW_CHK_NULL(pParams->pvMiInterface);
    pMiInterface = (MhwMiInterface *)pParams->pvMiInterface;
    MHW_CHK_NULL(pMiInterface);
    pSkuTable = pOsInterface->pfnGetSkuTable(pOsInterface);
    MHW_CHK_NULL(pSkuTable);
    pWaTable = pOsInterface->pfnGetWaTable(pOsInterface);
    MHW_CHK_NULL(pWaTable);

    GpuContext = pOsInterface->pfnGetGpuContext(pOsInterface);

    if ( pOsInterface->Component != COMPONENT_CM )
    {
        if (    GpuContext == MOS_GPU_CONTEXT_RENDER        ||
                GpuContext == MOS_GPU_CONTEXT_RENDER2       ||
                GpuContext == MOS_GPU_CONTEXT_RENDER3       ||
                GpuContext == MOS_GPU_CONTEXT_RENDER4       ||
                GpuContext == MOS_GPU_CONTEXT_VIDEO         ||
                GpuContext == MOS_GPU_CONTEXT_VIDEO2        ||
                GpuContext == MOS_GPU_CONTEXT_VIDEO3        ||
                GpuContext == MOS_GPU_CONTEXT_VIDEO4        ||
                GpuContext == MOS_GPU_CONTEXT_VDBOX2_VIDEO  ||
                GpuContext == MOS_GPU_CONTEXT_VDBOX2_VIDEO2 ||
                GpuContext == MOS_GPU_CONTEXT_VDBOX2_VIDEO3 ||
                GpuContext == MOS_GPU_CONTEXT_VEBOX         ||
                GpuContext == MOS_GPU_CONTEXT_VIDEO5        ||
                GpuContext == MOS_GPU_CONTEXT_VIDEO6        ||
                GpuContext == MOS_GPU_CONTEXT_VIDEO7        )
        {
            MHW_CHK_STATUS(pMiInterface->AddWatchdogTimerStartCmd(pCmdBuffer));
        }
    }

    bRcsEngineUsed = MOS_RCS_ENGINE_USED(GpuContext);

    if (bRcsEngineUsed)
    {
        MOS_ZeroMemory(&PipeControlParams, sizeof(PipeControlParams));

        PipeControlParams.dwFlushMode = MHW_FLUSH_WRITE_CACHE;
        MHW_CHK_STATUS(pMiInterface->AddPipeControl(
            pCmdBuffer,
            nullptr,
            &PipeControlParams));

        PipeControlParams.dwFlushMode = MHW_FLUSH_READ_CACHE;
        PipeControlParams.presDest = pParams->presStoreData;
        PipeControlParams.dwPostSyncOp = MHW_FLUSH_WRITE_IMMEDIATE_DATA;
        PipeControlParams.dwResourceOffset = pParams->dwStoreDataOffset;
        PipeControlParams.dwDataDW1 = pParams->dwStoreDataValue;
        MHW_CHK_STATUS(pMiInterface->AddPipeControl(
            pCmdBuffer,
            nullptr,
            &PipeControlParams));

        if(pCmdBuffer->Attributes.bUmdSSEUEnable)
        {
            MHW_MI_LOAD_REGISTER_IMM_PARAMS MiLoadRegImmParams;
            MHW_RENDER_PWR_CLK_STATE_PARAMS params;

            MOS_ZeroMemory(&params, sizeof(params));
            params.PowerClkStateEn  = true;
            params.SCountEn         = true;
            params.SSCountEn        = true;
            params.SliceCount       = pCmdBuffer->Attributes.dwNumRequestedEUSlices;
            params.SubSliceCount    = pCmdBuffer->Attributes.dwNumRequestedSubSlices;
            params.EUmax            = pCmdBuffer->Attributes.dwNumRequestedEUs;
            params.EUmin            = pCmdBuffer->Attributes.dwNumRequestedEUs;

            MOS_ZeroMemory(&MiLoadRegImmParams, sizeof(MiLoadRegImmParams));
            MiLoadRegImmParams.dwRegister   = MHW__PWR_CLK_STATE_REG;
            MiLoadRegImmParams.dwData       = params.Data;
            MHW_CHK_STATUS(pMiInterface->AddMiLoadRegisterImmCmd(
                pCmdBuffer,
                &MiLoadRegImmParams));
        }
    }
    else
    {
        // Send MI_FLUSH with protection bit off, which will FORCE exit protected mode for MFX
        MOS_ZeroMemory(&FlushDwParams, sizeof(FlushDwParams));
        FlushDwParams.bVideoPipelineCacheInvalidate = true;
        FlushDwParams.pOsResource = pParams->presStoreData;
        FlushDwParams.dwResourceOffset = pParams->dwStoreDataOffset;
        FlushDwParams.dwDataDW1 = pParams->dwStoreDataValue;
        MHW_CHK_STATUS(pMiInterface->AddMiFlushDwCmd(
            pCmdBuffer,
            &FlushDwParams));
    }

    MHW_CHK_STATUS(pMiInterface->AddProtectedProlog(pCmdBuffer));

    if (pMmioReg)
    {
        HalOcaInterface::On1stLevelBBStart(
            *pCmdBuffer,
            *pOsInterface->pOsContext,
            pOsInterface->CurrentGpuContextHandle,
            *pMiInterface,
            *pMmioReg);
    }

finish:

    return eStatus;
}
