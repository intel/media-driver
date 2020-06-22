/*
* Copyright (c) 2017, Intel Corporation
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
//! \file      cm_hal_vebox.cpp
//! \brief     HAL CM Vebox functions
//!

#include "cm_hal.h"
#include "cm_hal_vebox.h"

//!
//! \brief      build up vebox command sequence
//! \details     based on passed vebox param to build command sequence and
//!             put it into command buffer
//! \param       [in] state --- CM_HAL_STATE
//! \param       [in] veboxTaskParam -- vebox setup params
//!
MOS_STATUS HalCm_ExecuteVeboxTask(
    PCM_HAL_STATE                   state,           // [in] Pointer to CM State
    PCM_HAL_EXEC_VEBOX_TASK_PARAM   veboxTaskParam)  // [in] Pointer to Vebox Task Param
{
    CM_VEBOX_STATE                      cmVeboxState;
    PMOS_INTERFACE                      osInterface;
    MOS_COMMAND_BUFFER                  cmdBuffer;
    MhwVeboxInterface                   *veboxInterface;
    PMHW_VEBOX_HEAP                     veboxHeap;
    MHW_VEBOX_STATE_CMD_PARAMS          veboxStateCmdParams;
    MHW_VEBOX_SURFACE_STATE_CMD_PARAMS  veboxSurfaceStateCmdParams;
    MHW_VEBOX_DI_IECP_CMD_PARAMS        veboxDiIecpCmdParams;
    MHW_MI_FLUSH_DW_PARAMS              miFlushDwParams;
    MOS_STATUS                          eStatus = MOS_STATUS_SUCCESS;
    uint32_t                            index;
    int32_t                             taskId, i, remaining, syncOffset;
    int64_t                             *taskSyncLocation;
    uint32_t                            tag;

    RENDERHAL_GENERIC_PROLOG_PARAMS     genericPrologParams = {};
    MOS_RESOURCE                        osResource;
    CM_VEBOX_SURFACE_DATA               cmVeboxSurfaceData;
    PRENDERHAL_INTERFACE                renderHal = state->renderHal;

    //-----------------------------------
    CM_CHK_NULL_RETURN_MOSERROR(state);
    CM_CHK_NULL_RETURN_MOSERROR(state->osInterface);
    CM_CHK_NULL_RETURN_MOSERROR(state->veboxInterface);
    CM_CHK_NULL_RETURN_MOSERROR(veboxTaskParam);
    //-----------------------------------

    // initialize
    MOS_ZeroMemory(&cmdBuffer, sizeof(MOS_COMMAND_BUFFER));

    veboxInterface = state->veboxInterface;

    veboxHeap = veboxInterface->m_veboxHeap;
    osInterface = state->osInterface;
    remaining = 0;

    // update Cm state settings

    state->cmVeboxSettings.dndiFirstFrame = veboxTaskParam->cmVeboxState.DNDIFirstFrame;
    state->cmVeboxSettings.iecpEnabled = veboxTaskParam->cmVeboxState.GlobalIECPEnable;
    state->cmVeboxSettings.diEnabled = veboxTaskParam->cmVeboxState.DIEnable;
    state->cmVeboxSettings.dnEnabled = veboxTaskParam->cmVeboxState.DNEnable;
    state->cmVeboxSettings.demosaicEnabled = veboxTaskParam->cmVeboxState.DemosaicEnable;
    state->cmVeboxSettings.vignetteEnabled = veboxTaskParam->cmVeboxState.VignetteEnable;
    state->cmVeboxSettings.hotPixelFilterEnabled = veboxTaskParam->cmVeboxState.HotPixelFilteringEnable;
    state->cmVeboxSettings.diOutputFrames = veboxTaskParam->cmVeboxState.DIOutputFrames;

    cmVeboxSurfaceData = veboxTaskParam->veboxSurfaceData;

    // reset states before execute
    // (clear allocations, get GSH allocation index + any additional housekeeping)
    osInterface->pfnResetOsStates(osInterface);

    // reset HW
    CM_CHK_MOSSTATUS_GOTOFINISH(state->renderHal->pfnReset(state->renderHal));

    // get the Task Id
    CM_CHK_MOSSTATUS_GOTOFINISH(HalCm_GetNewTaskId(state, &taskId));

    // get the task sync offset
    syncOffset = state->pfnGetTaskSyncLocation(state, taskId);

    // set Perf Tag
    osInterface->pfnResetPerfBufferID(osInterface);
    if (!(osInterface->pfnIsPerfTagSet(osInterface)))
    {
        osInterface->pfnSetPerfTag(osInterface, VPHAL_NONE);
    }

    // initialize the location
    taskSyncLocation = (int64_t*)(state->veboxTimeStampResource.data + syncOffset);
    *taskSyncLocation = CM_INVALID_INDEX;
    *(taskSyncLocation + 1) = CM_INVALID_INDEX;
    if (state->cbbEnabled)
    {
        *(taskSyncLocation + 2) = state->renderHal->veBoxTrackerRes.currentTrackerId;
    }

    // register Timestamp Buffer
    CM_CHK_MOSSTATUS_GOTOFINISH(osInterface->pfnRegisterResource(
        osInterface,
        &state->veboxTimeStampResource.osResource,
        true,
        true));

    // get details of the surfaces on VPHAL Surface
    for (index = 0; index < VEBOX_SURFACE_NUMBER; index++)
    {
        if (veboxTaskParam->veboxSurfaceData.surfaceEntry[index].surfaceIndex == 0xffff)
        {
            continue;
        }

        CM_CHK_MOSSTATUS_GOTOFINISH(HalCm_GetSurfaceAndRegister(
            state,
            &state->cmVeboxSurfaces[index],
            CM_ARGUMENT_SURFACE2D,
            veboxTaskParam->veboxSurfaceData.surfaceEntry[index].surfaceIndex,
            0));
        state->cmVeboxSurfaces[index].rcMaxSrc = state->cmVeboxSurfaces[index].rcSrc;
    }

    //----------------------------------
    // initialize STMM input surface
    //----------------------------------
    if ((veboxTaskParam->cmVeboxState.DNDIFirstFrame) && ((veboxTaskParam->cmVeboxState.DIEnable) || (veboxTaskParam->cmVeboxState.DNEnable)))
    {
        CM_CHK_MOSSTATUS_GOTOFINISH(HalCm_VeboxInitSTMMHistory(
            osInterface,
            &state->cmVeboxSurfaces[VEBOX_STMM_INPUT_SURF]));
    }

    //----------------------------------
    // Allocate and reset VEBOX state
    //----------------------------------
    CM_CHK_MOSSTATUS_GOTOFINISH(veboxInterface->AssignVeboxState());

    //----------------------------------
    // set vebox state heap and vebox cmd parameters
    //----------------------------------
    MOS_ZeroMemory(&veboxStateCmdParams, sizeof(MHW_VEBOX_STATE_CMD_PARAMS));

    //set vebox param buffer
    CM_CHK_MOSSTATUS_GOTOFINISH(HalCm_GetSurfaceAndRegister(
        state,
        &state->cmVebeboxParamSurf,
        CM_ARGUMENT_SURFACEBUFFER,
        veboxTaskParam->veboxParamIndex,
        0));

    veboxStateCmdParams.VeboxMode.AlphaPlaneEnable = veboxTaskParam->cmVeboxState.AlphaPlaneEnable;
    veboxStateCmdParams.VeboxMode.ColorGamutCompressionEnable = veboxTaskParam->cmVeboxState.ColorGamutCompressionEnable;
    veboxStateCmdParams.VeboxMode.ColorGamutExpansionEnable = veboxTaskParam->cmVeboxState.ColorGamutExpansionEnable;
    veboxStateCmdParams.VeboxMode.DemosaicEnable = veboxTaskParam->cmVeboxState.DemosaicEnable;
    veboxStateCmdParams.VeboxMode.DIEnable = veboxTaskParam->cmVeboxState.DIEnable;
    veboxStateCmdParams.VeboxMode.DIOutputFrames = veboxTaskParam->cmVeboxState.DIOutputFrames;
    veboxStateCmdParams.VeboxMode.DisableEncoderStatistics = veboxTaskParam->cmVeboxState.DisableEncoderStatistics;
    veboxStateCmdParams.VeboxMode.DisableTemporalDenoiseFilter = veboxTaskParam->cmVeboxState.DisableTemporalDenoiseFilter;
    veboxStateCmdParams.VeboxMode.DNDIFirstFrame = veboxTaskParam->cmVeboxState.DNDIFirstFrame;
    veboxStateCmdParams.VeboxMode.DNEnable = veboxTaskParam->cmVeboxState.DNEnable;
    veboxStateCmdParams.VeboxMode.ForwardGammaCorrectionEnable = veboxTaskParam->cmVeboxState.ForwardGammaCorrectionEnable;
    veboxStateCmdParams.VeboxMode.GlobalIECPEnable = veboxTaskParam->cmVeboxState.GlobalIECPEnable;
    veboxStateCmdParams.VeboxMode.HotPixelFilteringEnable = veboxTaskParam->cmVeboxState.HotPixelFilteringEnable;
    veboxStateCmdParams.VeboxMode.SingleSliceVeboxEnable = veboxTaskParam->cmVeboxState.SingleSliceVeboxEnable;
    veboxStateCmdParams.VeboxMode.VignetteEnable = veboxTaskParam->cmVeboxState.VignetteEnable;
    veboxStateCmdParams.pVeboxParamSurf = (PMOS_RESOURCE)&((state->cmVebeboxParamSurf).OsSurface);
    //----------------------------------
    // get vebox command buffer
    //----------------------------------
    CM_CHK_MOSSTATUS_GOTOFINISH(osInterface->pfnGetCommandBuffer(osInterface, &cmdBuffer, 0));
    remaining = cmdBuffer.iRemaining;

    //---------------------------------
    // Get the OS resource
    //---------------------------------
    osResource = state->renderHal->veBoxTrackerRes.osResource;
    tag = state->renderHal->veBoxTrackerRes.currentTrackerId;
    state->renderHal->pfnSetupPrologParams(state->renderHal, &genericPrologParams, &osResource, 0, tag);

    //---------------------------------
    // send command buffer header at the beginning (OS dependent)
    //---------------------------------
    CM_CHK_MOSSTATUS_GOTOFINISH(state->renderHal->pfnInitCommandBuffer(
        state->renderHal,
        &cmdBuffer,
        &genericPrologParams));

    //---------------------------------
    // the beginning of execution
    // issue MI_FLUSH_DW cmd to write timestamp
    //---------------------------------
    MOS_ZeroMemory(&miFlushDwParams, sizeof(miFlushDwParams));
    miFlushDwParams.pOsResource          = &state->veboxTimeStampResource.osResource;
    miFlushDwParams.dwResourceOffset     = syncOffset;
    miFlushDwParams.postSyncOperation    = MHW_FLUSH_WRITE_TIMESTAMP_REG;
    miFlushDwParams.bQWordEnable         = 1;

    CM_CHK_MOSSTATUS_GOTOFINISH(renderHal->pMhwMiInterface->AddMiFlushDwCmd(
        &cmdBuffer,
        &miFlushDwParams));

    //---------------------------------
    // update tracker tag
    //---------------------------------
    renderHal->veBoxTrackerRes.currentTrackerId++;

    //---------------------------------
    // send vebox state commands
    //---------------------------------
    CM_CHK_MOSSTATUS_GOTOFINISH(veboxInterface->AddVeboxState(
        &cmdBuffer,
        &veboxStateCmdParams, 1));

    //---------------------------------
    // send Vebox_Surface_State cmd
    //---------------------------------
    MOS_ZeroMemory(&veboxSurfaceStateCmdParams, sizeof(MHW_VEBOX_SURFACE_STATE_CMD_PARAMS));
    CM_CHK_MOSSTATUS_GOTOFINISH(HalCm_SetVeboxSurfaceStateCmdParams(state, &veboxSurfaceStateCmdParams));
    CM_CHK_MOSSTATUS_GOTOFINISH(veboxInterface->AddVeboxSurfaces(
        &cmdBuffer,
        &veboxSurfaceStateCmdParams));
    //---------------------------------
    // send Vebox_DI_IECP cmd
    //---------------------------------
    MOS_ZeroMemory(&veboxDiIecpCmdParams, sizeof(MHW_VEBOX_DI_IECP_CMD_PARAMS));
    CM_CHK_MOSSTATUS_GOTOFINISH(HalCm_SetVeboxDiIecpCmdParams(state, &veboxDiIecpCmdParams, (PCM_VEBOX_SURFACE_DATA)&cmVeboxSurfaceData));
    CM_CHK_MOSSTATUS_GOTOFINISH(veboxInterface->AddVeboxDiIecp(
        &cmdBuffer,
        &veboxDiIecpCmdParams));

    //---------------------------------
    // issue MI_FLUSH_DW cmd to write timestamp, end of execution
    //---------------------------------
    MOS_ZeroMemory(&miFlushDwParams, sizeof(miFlushDwParams));
    miFlushDwParams.pOsResource        = &state->veboxTimeStampResource.osResource;
    miFlushDwParams.dwResourceOffset   = syncOffset + sizeof(uint64_t);
    miFlushDwParams.postSyncOperation  = MHW_FLUSH_WRITE_TIMESTAMP_REG;
    miFlushDwParams.bQWordEnable       = 1;

    CM_CHK_MOSSTATUS_GOTOFINISH(renderHal->pMhwMiInterface->AddMiFlushDwCmd(
        &cmdBuffer,
        &miFlushDwParams));

    //---------------------------------
    // Write Sync tag for Vebox Heap Synchronization
    //---------------------------------
    MOS_ZeroMemory(&miFlushDwParams, sizeof(miFlushDwParams));
    miFlushDwParams.pOsResource       = &veboxHeap->DriverResource;
    miFlushDwParams.dwResourceOffset  = veboxHeap->uiOffsetSync;
    miFlushDwParams.dwDataDW1         = veboxHeap->dwNextTag;
    miFlushDwParams.bQWordEnable      = 1;
    CM_CHK_MOSSTATUS_GOTOFINISH(renderHal->pMhwMiInterface->AddMiFlushDwCmd(
        &cmdBuffer,
        &miFlushDwParams));

    // Update tracker resource
    CM_CHK_MOSSTATUS_GOTOFINISH(state->pfnUpdateTrackerResource(state, &cmdBuffer, tag));

    //---------------------------------
    // Make sure copy kernel and update kernels are finished before submitting
    // VEBOX commands
    //---------------------------------
    if ((MOS_GPU_CONTEXT)veboxTaskParam->queueOption.GPUContext != MOS_GPU_CONTEXT_CM_COMPUTE)
    {
        osInterface->pfnSyncGpuContext(
            osInterface,
            (MOS_GPU_CONTEXT)veboxTaskParam->queueOption.GPUContext,
            MOS_GPU_CONTEXT_VEBOX);
    }

    osInterface->pfnResetPerfBufferID(osInterface);
    if (!(osInterface->pfnIsPerfTagSet(osInterface)))
    {
        osInterface->pfnIncPerfFrameID(osInterface);
        osInterface->pfnSetPerfTag(osInterface, VEBOX_TASK_PERFTAG_INDEX);
    }

    //Couple to the BB_START , otherwise GPU Hang without it in KMD.
    CM_CHK_MOSSTATUS_GOTOFINISH(renderHal->pMhwMiInterface->AddMiBatchBufferEnd(&cmdBuffer, nullptr));

    //---------------------------------
    // Return unused command buffer space to OS
    //---------------------------------
    osInterface->pfnReturnCommandBuffer(
        osInterface,
        &cmdBuffer, 0);

    //---------------------------------
    // submit the command buffer
    //---------------------------------
    CM_CHK_MOSSTATUS_GOTOFINISH(osInterface->pfnSubmitCommandBuffer(
        osInterface,
        &cmdBuffer,
        state->nullHwRenderCm));

    // Set the Task ID
    veboxTaskParam->taskIdOut = taskId;

    // pass back the Command Buffer
    state->pfnReferenceCommandBuffer(&cmdBuffer.OsResource, &veboxTaskParam->osData);

    // Update the task ID table
    state->taskStatusTable[taskId] = (char)taskId;

    if (!(state->nullHwRenderCm))
    {
        // Update Vebox Sync tag info
        veboxHeap->pStates[veboxHeap->uiCurState].dwSyncTag = veboxHeap->dwNextTag++;
        veboxHeap->pStates[veboxHeap->uiCurState].bBusy = true;
    }

    eStatus = MOS_STATUS_SUCCESS;

finish:

    // Failed -> discard all changes in Command Buffer
    if (eStatus != MOS_STATUS_SUCCESS)
    {
        // Buffer overflow - display overflow size
        if (cmdBuffer.iRemaining < 0)
        {
            CM_ASSERTMESSAGE("Command Buffer overflow by %d bytes", cmdBuffer.iRemaining);
        }

        // Move command buffer back to beginning
        i = remaining - cmdBuffer.iRemaining;
        cmdBuffer.iRemaining = remaining;
        cmdBuffer.iOffset -= i;
        cmdBuffer.pCmdPtr = cmdBuffer.pCmdBase + cmdBuffer.iOffset / sizeof(uint32_t);

        // Return unused command buffer space to OS
        osInterface->pfnReturnCommandBuffer(osInterface, &cmdBuffer, 0);
    }

    return eStatus;
}

//!
//! \brief      Set up vebox surface Param
//! \details    set up vebox surface state based on parameter based from application
//!
//! \param     [in]state  -- CM_HAL_STATE
//! \param     [in]veboxSurfaceStateCmdParams  -- surface state param struct
//!
MOS_STATUS HalCm_SetVeboxSurfaceStateCmdParams(
    PCM_HAL_STATE                           state,
    PMHW_VEBOX_SURFACE_STATE_CMD_PARAMS   veboxSurfaceStateCmdParams)
{
    if ((state->cmVeboxSettings.iecpEnabled) && !((state->cmVeboxSettings.diEnabled) || (state->cmVeboxSettings.dnEnabled)))
    {
        // IECP only
        HalCm_Convert_RENDERHAL_SURFACE_To_MHW_VEBOX_SURFACE(&state->cmVeboxSurfaces[VEBOX_CURRENT_FRAME_INPUT_SURF], &veboxSurfaceStateCmdParams->SurfInput);
        HalCm_Convert_RENDERHAL_SURFACE_To_MHW_VEBOX_SURFACE(&state->cmVeboxSurfaces[VEBOX_CURRENT_FRAME_OUTPUT_SURF], &veboxSurfaceStateCmdParams->SurfOutput);
        veboxSurfaceStateCmdParams->bDIEnable = false;
        veboxSurfaceStateCmdParams->bOutputValid = true;
    }
    else
    {
        // DN only, will add other support later

        HalCm_Convert_RENDERHAL_SURFACE_To_MHW_VEBOX_SURFACE(&state->cmVeboxSurfaces[VEBOX_CURRENT_FRAME_INPUT_SURF], &veboxSurfaceStateCmdParams->SurfInput);
        HalCm_Convert_RENDERHAL_SURFACE_To_MHW_VEBOX_SURFACE(&state->cmVeboxSurfaces[VEBOX_DN_CURRENT_FRAME_OUTPUT_SURF], &veboxSurfaceStateCmdParams->SurfOutput);
        HalCm_Convert_RENDERHAL_SURFACE_To_MHW_VEBOX_SURFACE(&state->cmVeboxSurfaces[VEBOX_STMM_INPUT_SURF], &veboxSurfaceStateCmdParams->SurfSTMM);
        HalCm_Convert_RENDERHAL_SURFACE_To_MHW_VEBOX_SURFACE(&state->cmVeboxSurfaces[VEBOX_STMM_OUTPUT_SURF], &veboxSurfaceStateCmdParams->SurfDNOutput);

        veboxSurfaceStateCmdParams->bDIEnable = false;
        veboxSurfaceStateCmdParams->bOutputValid = true;
    }

    return MOS_STATUS_SUCCESS;
}

//!
//! \brief    set vebox DiIecp Command
//! \details  build up command to start processing the frames specified by
//!           VEB_SURFACE_STATE using the parameters specified by VEB_DI_STATE
//!           and VEB_IECP_STATE.
//!  \param   [in] state -- HAL_CM_STATE
//!  \param   [in] veboxDiIecpCmdParams  -- DIECP command parameter
//!  \param   [in] cmVeboxSurfaceDataInput  -- surface data such as index and control bits
//!
MOS_STATUS HalCm_SetVeboxDiIecpCmdParams(
    PCM_HAL_STATE                   state,
    PMHW_VEBOX_DI_IECP_CMD_PARAMS   veboxDiIecpCmdParams,
    PCM_VEBOX_SURFACE_DATA       cmVeboxSurfaceDataInput)
{
    uint32_t                 width;
    uint32_t                 height;
    bool                     dienable;
    MHW_VEBOX_SURFACE_PARAMS surfInput;

    CM_CHK_NULL_RETURN_MOSERROR(state->veboxInterface);

    // DN only, will add other support later
    dienable = false;

    // Align dwEndingX with surface state
    HalCm_Convert_RENDERHAL_SURFACE_To_MHW_VEBOX_SURFACE(&state->cmVeboxSurfaces[VEBOX_CURRENT_FRAME_INPUT_SURF], &surfInput);
    state->veboxInterface->VeboxAdjustBoundary(
        &surfInput,
        &width,
        &height,
        dienable);

    veboxDiIecpCmdParams->dwStartingX = 0;
    veboxDiIecpCmdParams->dwEndingX = width - 1;

    if (!state->cmVeboxSettings.dndiFirstFrame)
    {
        veboxDiIecpCmdParams->pOsResPrevInput = &state->cmVeboxSurfaces[VEBOX_PREVIOUS_FRAME_INPUT_SURF].OsSurface.OsResource;
        veboxDiIecpCmdParams->PrevInputSurfCtrl.Value = cmVeboxSurfaceDataInput->surfaceEntry[VEBOX_PREVIOUS_FRAME_INPUT_SURF].surfaceCtrlBits;
    }

    veboxDiIecpCmdParams->pOsResCurrInput = &state->cmVeboxSurfaces[VEBOX_CURRENT_FRAME_INPUT_SURF].OsSurface.OsResource;
    veboxDiIecpCmdParams->CurrInputSurfCtrl.Value = cmVeboxSurfaceDataInput->surfaceEntry[VEBOX_CURRENT_FRAME_INPUT_SURF].surfaceCtrlBits;

    if ((state->cmVeboxSettings.diEnabled) || (state->cmVeboxSettings.dnEnabled))
    {
        veboxDiIecpCmdParams->pOsResStmmInput = &state->cmVeboxSurfaces[VEBOX_STMM_INPUT_SURF].OsSurface.OsResource;
        veboxDiIecpCmdParams->StmmInputSurfCtrl.Value = cmVeboxSurfaceDataInput->surfaceEntry[VEBOX_STMM_INPUT_SURF].surfaceCtrlBits;

        veboxDiIecpCmdParams->pOsResStmmOutput = &state->cmVeboxSurfaces[VEBOX_STMM_OUTPUT_SURF].OsSurface.OsResource;
        veboxDiIecpCmdParams->StmmOutputSurfCtrl.Value = cmVeboxSurfaceDataInput->surfaceEntry[VEBOX_STMM_OUTPUT_SURF].surfaceCtrlBits;
    }

    if ((state->cmVeboxSettings.iecpEnabled) && !((state->cmVeboxSettings.diEnabled) || (state->cmVeboxSettings.dnEnabled)))
    {
        veboxDiIecpCmdParams->pOsResCurrOutput = &state->cmVeboxSurfaces[VEBOX_CURRENT_FRAME_OUTPUT_SURF].OsSurface.OsResource;
        veboxDiIecpCmdParams->CurrOutputSurfCtrl.Value = cmVeboxSurfaceDataInput->surfaceEntry[VEBOX_CURRENT_FRAME_OUTPUT_SURF].surfaceCtrlBits;
        veboxDiIecpCmdParams->pOsResLaceOrAceOrRgbHistogram = &state->cmVeboxSurfaces[VEBOX_LACE_ACE_RGB_HISTOGRAM_OUTPUT_SURF].OsSurface.OsResource;
        veboxDiIecpCmdParams->LaceOrAceOrRgbHistogramSurfCtrl.Value = cmVeboxSurfaceDataInput->surfaceEntry[VEBOX_LACE_ACE_RGB_HISTOGRAM_OUTPUT_SURF].surfaceCtrlBits;
    }

    if (state->cmVeboxSettings.dnEnabled)
    {
        veboxDiIecpCmdParams->pOsResDenoisedCurrOutput = &state->cmVeboxSurfaces[VEBOX_DN_CURRENT_FRAME_OUTPUT_SURF].OsSurface.OsResource;
        veboxDiIecpCmdParams->DenoisedCurrOutputSurfCtrl.Value = cmVeboxSurfaceDataInput->surfaceEntry[VEBOX_DN_CURRENT_FRAME_OUTPUT_SURF].surfaceCtrlBits;
    }

    if (state->cmVeboxSettings.vignetteEnabled)
    {
        veboxDiIecpCmdParams->pOsResAlphaOrVignette = &state->cmVeboxSurfaces[VEBOX_ALPHA_VIGNETTE_CORRECTION_SURF].OsSurface.OsResource;
        veboxDiIecpCmdParams->DenoisedCurrOutputSurfCtrl.Value = cmVeboxSurfaceDataInput->surfaceEntry[VEBOX_DN_CURRENT_FRAME_OUTPUT_SURF].surfaceCtrlBits;
    }

    veboxDiIecpCmdParams->pOsResStatisticsOutput = &state->cmVeboxSurfaces[VEBOX_STATISTICS_OUTPUT_SURF].OsSurface.OsResource;
    veboxDiIecpCmdParams->StatisticsOutputSurfCtrl.Value = cmVeboxSurfaceDataInput->surfaceEntry[VEBOX_STATISTICS_OUTPUT_SURF].surfaceCtrlBits;

    return MOS_STATUS_SUCCESS;
}

//| Name   : HalCm_VeboxInitSTMMHistory()
//| Purpose: Resets the portion of the Vebox STMM surface associated with
//|              motion history for temporal filtering.
//|
//| Description:
//|   This function is used by VEBox for initializing
//|   the STMM surface.  The STMM / Denoise history is a custom surface used
//|   for both input and output. Each cache line contains data for 4 4x4s.
//|   The STMM for each 4x4 is 8 bytes, while the denoise history is 1 byte
//|   and the chroma denoise history is 1 byte for each U and V.
//|   Byte    Data
//|   0       STMM for 2 luma values at luma Y=0, X=0 to 1
//|   1       STMM for 2 luma values at luma Y=0, X=2 to 3
//|   2       Luma Denoise History for 4x4 at 0,0
//|   3       Not Used
//|   4-5     STMM for luma from X=4 to 7
//|   6       Luma Denoise History for 4x4 at 0,4
//|   7       Not Used
//|   8-15    Repeat for 4x4s at 0,8 and 0,12
//|   16      STMM for 2 luma values at luma Y=1,X=0 to 1
//|   17      STMM for 2 luma values at luma Y=1, X=2 to 3
//|   18      U Chroma Denoise History
//|   19      Not Used
//|   20-31   Repeat for 3 4x4s at 1,4, 1,8 and 1,12
//|   32      STMM for 2 luma values at luma Y=2,X=0 to 1
//|   33      STMM for 2 luma values at luma Y=2, X=2 to 3
//|   34      V Chroma Denoise History
//|   35      Not Used
//|   36-47   Repeat for 3 4x4s at 2,4, 2,8 and 2,12
//|   48      STMM for 2 luma values at luma Y=3,X=0 to 1
//|   49      STMM for 2 luma values at luma Y=3, X=2 to 3
//|   50-51   Not Used
//|   36-47   Repeat for 3 4x4s at 3,4, 3,8 and 3,12
//|
//| Returns: MOS_STATUS_SUCCESS if success. Error code otherwise;
//!
MOS_STATUS HalCm_VeboxInitSTMMHistory(
    PMOS_INTERFACE          osInterface,
    PRENDERHAL_SURFACE      renderHalSTMMSurface)
{
    MOS_STATUS          eStatus = MOS_STATUS_SUCCESS;
    uint32_t            size;
    int32_t             x, y;
    uint8_t             *bytes;
    MOS_LOCK_PARAMS     lockFlags;

    PMOS_SURFACE stmmSurface = &renderHalSTMMSurface->OsSurface;

    MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));

    lockFlags.WriteOnly = 1;

    // Lock the surface for writing
    bytes = (uint8_t*)osInterface->pfnLockResource(
        osInterface,
        &stmmSurface->OsResource,
        &lockFlags);

    CM_CHK_NULL_GOTOFINISH_MOSERROR(bytes);

    size = stmmSurface->dwWidth >> 2;

    // Fill STMM surface with DN history init values.
    for (y = 0; y < (int32_t)stmmSurface->dwHeight; y++)
    {
        for (x = 0; x < (int32_t)size; x++)
        {
            MOS_FillMemory(bytes, 2, DNDI_HISTORY_INITVALUE);
            // skip denoise history init.
            bytes += 4;
        }

        bytes += stmmSurface->dwPitch - stmmSurface->dwWidth;
    }

    // Unlock the surface
    CM_CHK_HRESULT_GOTOFINISH_MOSERROR(osInterface->pfnUnlockResource(
        osInterface,
        &stmmSurface->OsResource));

finish:
    return eStatus;
}
