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
//! \param       [in] pState --- CM_HAL_STATE
//! \param       [in] pExecVeboxParam -- vebox setup params
//!
MOS_STATUS HalCm_ExecuteVeboxTask(
	PCM_HAL_STATE                   pState,           // [in] Pointer to CM State
	PCM_HAL_EXEC_VEBOX_TASK_PARAM   pExecVeboxParam)  // [in] Pointer to Vebox Task Param
{
    CM_VEBOX_STATE                      CmVeboxState;
    PMOS_INTERFACE                      pOsInterface;
    MOS_COMMAND_BUFFER                  CmdBuffer;
    MhwVeboxInterface                   *pVeboxInterface;
    PMHW_VEBOX_HEAP                     pVeboxHeap;
    MHW_VEBOX_STATE_CMD_PARAMS          veboxStateCmdParams;
    MHW_VEBOX_SURFACE_STATE_CMD_PARAMS  veboxSurfaceStateCmdParams;
    MHW_VEBOX_DI_IECP_CMD_PARAMS        veboxDiIecpCmdParams;
    MHW_MI_FLUSH_DW_PARAMS              miFlushDwParams;
    MOS_STATUS                          hr;
    uint32_t                            index;
    int32_t                             iTaskId, i, iRemaining, iSyncOffset;
    int64_t                             *pTaskSyncLocation;
    RENDERHAL_GENERIC_PROLOG_PARAMS     genericPrologParams;
    MOS_RESOURCE                        OsResource;
    CM_VEBOX_SURFACE_DATA               CmVeboxSurfaceData;
    PRENDERHAL_INTERFACE                pRenderHal = pState->pRenderHal;

	//-----------------------------------
	CM_PUBLIC_ASSERT(pState);
	CM_PUBLIC_ASSERT(pExecVeboxParam);
	//-----------------------------------

	hr = MOS_STATUS_SUCCESS;

	// initialize
	MOS_ZeroMemory(&CmdBuffer, sizeof(MOS_COMMAND_BUFFER));
	MOS_ZeroMemory(&genericPrologParams, sizeof(genericPrologParams));

	pVeboxInterface = pState->pVeboxInterface;
	pVeboxHeap = pVeboxInterface->m_veboxHeap;
	pOsInterface = pState->pOsInterface;
	iRemaining = 0;

	// update Cm state settings

	pState->cmVeboxSettings.bCmDnDiFirstFrame = pExecVeboxParam->cmVeboxState.DNDIFirstFrame;
	pState->cmVeboxSettings.bCmIECPEnable = pExecVeboxParam->cmVeboxState.GlobalIECPEnable;
	pState->cmVeboxSettings.bCmDIEnable = pExecVeboxParam->cmVeboxState.DIEnable;
	pState->cmVeboxSettings.bCmDNEnable = pExecVeboxParam->cmVeboxState.DNEnable;
	pState->cmVeboxSettings.bDemosaicEnable = pExecVeboxParam->cmVeboxState.DemosaicEnable;
	pState->cmVeboxSettings.bVignetteEnable = pExecVeboxParam->cmVeboxState.VignetteEnable;
	pState->cmVeboxSettings.bHotPixelFilterEnable = pExecVeboxParam->cmVeboxState.HotPixelFilteringEnable;
	pState->cmVeboxSettings.DIOutputFrames = pExecVeboxParam->cmVeboxState.DIOutputFrames;


	CmVeboxSurfaceData = pExecVeboxParam->CmVeboxSurfaceData;
	// switch GPU context to VEBOX
	pOsInterface->pfnSetGpuContext(pOsInterface, MOS_GPU_CONTEXT_VEBOX);

	// reset states before execute 
	// (clear allocations, get GSH allocation index + any additional housekeeping)
	pOsInterface->pfnResetOsStates(pOsInterface);

	// reset HW
	CM_CHK_MOSSTATUS(pState->pRenderHal->pfnReset(pState->pRenderHal));

	// get the Task Id
	CM_CHK_MOSSTATUS(HalCm_GetNewTaskId(pState, &iTaskId));

	// get the task sync offset
	iSyncOffset = pState->pfnGetTaskSyncLocation(iTaskId);

	// set Perf Tag
	pOsInterface->pfnResetPerfBufferID(pOsInterface);
	if (!(pOsInterface->pfnIsPerfTagSet(pOsInterface)))
	{
		pOsInterface->pfnSetPerfTag(pOsInterface, VPHAL_NONE);
	}

	// initialize the location
	pTaskSyncLocation = (int64_t*)(pState->Vebox_TsResource.pData + iSyncOffset);
	*pTaskSyncLocation = CM_INVALID_INDEX;
	*(pTaskSyncLocation + 1) = CM_INVALID_INDEX;
	if (pState->bCBBEnabled)
	{
		*(pTaskSyncLocation + 2) = CM_INVALID_TAG;
	}

	// register Timestamp Buffer
	CM_CHK_MOSSTATUS(pOsInterface->pfnRegisterResource(
		pOsInterface,
		&pState->Vebox_TsResource.OsResource,
		true,
		true));

	// get details of the surfaces on VPHAL Surface 
	for (index = 0; index < VEBOX_SURFACE_NUMBER; index++)
	{
		if (pExecVeboxParam->CmVeboxSurfaceData.surfaceEntry[index].wSurfaceIndex == 0xffff)
		{
			continue;
		}

		CM_CHK_MOSSTATUS(HalCm_GetSurfaceAndRegister(
			pState,
			&pState->cmVeboxSurfaces[index],
			CM_ARGUMENT_SURFACE2D,
			pExecVeboxParam->CmVeboxSurfaceData.surfaceEntry[index].wSurfaceIndex,
			0));
		pState->cmVeboxSurfaces[index].rcMaxSrc = pState->cmVeboxSurfaces[index].rcSrc;
	}

	//----------------------------------
	// initialize STMM input surface 
	//----------------------------------
	if ((pExecVeboxParam->cmVeboxState.DNDIFirstFrame) && ((pExecVeboxParam->cmVeboxState.DIEnable) || (pExecVeboxParam->cmVeboxState.DNEnable)))
	{
		CM_CHK_MOSSTATUS(HalCm_VeboxInitSTMMHistory(
			pOsInterface,
			&pState->cmVeboxSurfaces[VEBOX_STMM_INPUT_SURF]));
	}

	//----------------------------------
	// Allocate and reset VEBOX state
	//----------------------------------
	CM_CHK_MOSSTATUS(pVeboxInterface->AssignVeboxState());

	//----------------------------------
	// set vebox state heap and vebox cmd parameters
	//----------------------------------
	MOS_ZeroMemory(&veboxStateCmdParams, sizeof(MHW_VEBOX_STATE_CMD_PARAMS));

	//set vebox param buffer 
	CM_CHK_MOSSTATUS(HalCm_GetSurfaceAndRegister(
		pState,
		&pState->cmVebeboxParamSurf,
		CM_ARGUMENT_SURFACEBUFFER,
		pExecVeboxParam->veboxParamIndex,
		0));


	veboxStateCmdParams.VeboxMode.AlphaPlaneEnable = pExecVeboxParam->cmVeboxState.AlphaPlaneEnable;
	veboxStateCmdParams.VeboxMode.ColorGamutCompressionEnable = pExecVeboxParam->cmVeboxState.ColorGamutCompressionEnable;
	veboxStateCmdParams.VeboxMode.ColorGamutExpansionEnable = pExecVeboxParam->cmVeboxState.ColorGamutExpansionEnable;
	veboxStateCmdParams.VeboxMode.DemosaicEnable = pExecVeboxParam->cmVeboxState.DemosaicEnable;
	veboxStateCmdParams.VeboxMode.DIEnable = pExecVeboxParam->cmVeboxState.DIEnable;
	veboxStateCmdParams.VeboxMode.DIOutputFrames = pExecVeboxParam->cmVeboxState.DIOutputFrames;
	veboxStateCmdParams.VeboxMode.DisableEncoderStatistics = pExecVeboxParam->cmVeboxState.DisableEncoderStatistics;
	veboxStateCmdParams.VeboxMode.DisableTemporalDenoiseFilter = pExecVeboxParam->cmVeboxState.DisableTemporalDenoiseFilter;
	veboxStateCmdParams.VeboxMode.DNDIFirstFrame = pExecVeboxParam->cmVeboxState.DNDIFirstFrame;
	veboxStateCmdParams.VeboxMode.DNEnable = pExecVeboxParam->cmVeboxState.DNEnable;
	veboxStateCmdParams.VeboxMode.ForwardGammaCorrectionEnable = pExecVeboxParam->cmVeboxState.ForwardGammaCorrectionEnable;
	veboxStateCmdParams.VeboxMode.GlobalIECPEnable = pExecVeboxParam->cmVeboxState.GlobalIECPEnable;
	veboxStateCmdParams.VeboxMode.HotPixelFilteringEnable = pExecVeboxParam->cmVeboxState.HotPixelFilteringEnable;
	veboxStateCmdParams.VeboxMode.SingleSliceVeboxEnable = pExecVeboxParam->cmVeboxState.SingleSliceVeboxEnable;
	veboxStateCmdParams.VeboxMode.VignetteEnable = pExecVeboxParam->cmVeboxState.VignetteEnable;
	veboxStateCmdParams.pVeboxParamSurf = (PMOS_RESOURCE)&((pState->cmVebeboxParamSurf).OsSurface);
	//----------------------------------
	// get vebox command buffer
	//----------------------------------
	CM_CHK_MOSSTATUS(pOsInterface->pfnGetCommandBuffer(pOsInterface, &CmdBuffer, 0));
	iRemaining = CmdBuffer.iRemaining;

	//---------------------------------
	// Get the OS resource
	//---------------------------------
	pOsInterface->pfnGetGpuStatusBufferResource(pOsInterface, &OsResource);

	//---------------------------------
	// register the buffer
	//---------------------------------
	pOsInterface->pfnRegisterResource(pOsInterface, &OsResource, true, true);

	genericPrologParams.presMediaFrameTrackingSurface = &OsResource;
	genericPrologParams.dwMediaFrameTrackingAddrOffset = pOsInterface->pfnGetGpuStatusTagOffset(pOsInterface, MOS_GPU_CONTEXT_VEBOX);
	genericPrologParams.dwMediaFrameTrackingTag = pOsInterface->pfnGetGpuStatusTag(pOsInterface, MOS_GPU_CONTEXT_VEBOX);
	genericPrologParams.bEnableMediaFrameTracking = true;

	//---------------------------------
	// send command buffer header at the beginning (OS dependent)
	//---------------------------------
	CM_CHK_MOSSTATUS(pState->pRenderHal->pfnInitCommandBuffer(
		pState->pRenderHal,
		&CmdBuffer,
		&genericPrologParams));

	//---------------------------------
	// the beginning of execution
	// issue MI_FLUSH_DW cmd to write timestamp
	//---------------------------------
	MOS_ZeroMemory(&miFlushDwParams, sizeof(miFlushDwParams));
	miFlushDwParams.pOsResource          = &pState->Vebox_TsResource.OsResource;
    miFlushDwParams.dwResourceOffset     = iSyncOffset;
    miFlushDwParams.postSyncOperation    = MHW_FLUSH_WRITE_TIMESTAMP_REG;
    miFlushDwParams.bQWordEnable         = 1;

	CM_CHK_MOSSTATUS(pRenderHal->pMhwMiInterface->AddMiFlushDwCmd(
		&CmdBuffer,
		&miFlushDwParams));

	//---------------------------------
	// issue MI_FLUSH_DW cmd to write GPU status tag to CM resource
	//---------------------------------
	CM_CHK_MOSSTATUS(pState->pfnWriteGPUStatusTagToCMTSResource(pState, &CmdBuffer, iTaskId, true));

	//---------------------------------
	// update GPU sync tag
	//---------------------------------
	pOsInterface->pfnIncrementGpuStatusTag(pOsInterface, MOS_GPU_CONTEXT_VEBOX);

	//---------------------------------
	// send vebox state commands
	//---------------------------------
	CM_CHK_MOSSTATUS(pVeboxInterface->AddVeboxState(
		&CmdBuffer,
		&veboxStateCmdParams, 1));

	//---------------------------------
	// send Vebox_Surface_State cmd
	//---------------------------------
	MOS_ZeroMemory(&veboxSurfaceStateCmdParams, sizeof(MHW_VEBOX_SURFACE_STATE_CMD_PARAMS));
	CM_CHK_MOSSTATUS(HalCm_SetVeboxSurfaceStateCmdParams(pState, &veboxSurfaceStateCmdParams));
	CM_CHK_MOSSTATUS(pVeboxInterface->AddVeboxSurfaces(
		&CmdBuffer,
		&veboxSurfaceStateCmdParams));
	//---------------------------------
	// send Vebox_DI_IECP cmd
	//---------------------------------
	MOS_ZeroMemory(&veboxDiIecpCmdParams, sizeof(MHW_VEBOX_DI_IECP_CMD_PARAMS));
	CM_CHK_MOSSTATUS(HalCm_SetVeboxDiIecpCmdParams(pState, &veboxDiIecpCmdParams, (PCM_VEBOX_SURFACE_DATA)&CmVeboxSurfaceData));
	CM_CHK_MOSSTATUS(pVeboxInterface->AddVeboxDiIecp(
		&CmdBuffer,
		&veboxDiIecpCmdParams));


	//---------------------------------
	// issue MI_FLUSH_DW cmd to write timestamp, end of execution
	//---------------------------------
	MOS_ZeroMemory(&miFlushDwParams, sizeof(miFlushDwParams));
    miFlushDwParams.pOsResource        = &pState->Vebox_TsResource.OsResource;
    miFlushDwParams.dwResourceOffset   = iSyncOffset + sizeof(uint64_t);
    miFlushDwParams.postSyncOperation  = MHW_FLUSH_WRITE_TIMESTAMP_REG;
    miFlushDwParams.bQWordEnable       = 1;

    CM_CHK_MOSSTATUS(pRenderHal->pMhwMiInterface->AddMiFlushDwCmd(
        &CmdBuffer,
        &miFlushDwParams));
	
	//---------------------------------
	// Write Sync tag for Vebox Heap Synchronization
	//---------------------------------
	MOS_ZeroMemory(&miFlushDwParams, sizeof(miFlushDwParams));
    miFlushDwParams.pOsResource       = &pVeboxHeap->DriverResource;
    miFlushDwParams.dwResourceOffset  = pVeboxHeap->uiOffsetSync;
    miFlushDwParams.dwDataDW1         = pVeboxHeap->dwNextTag;
    miFlushDwParams.bQWordEnable      = 1;
    CM_CHK_MOSSTATUS(pRenderHal->pMhwMiInterface->AddMiFlushDwCmd(
        &CmdBuffer,
        &miFlushDwParams));

	//---------------------------------
	// Make sure copy kernel and update kernels are finished before submitting
	// VEBOX commands
	//---------------------------------
	   pOsInterface->pfnSyncGpuContext(
		pOsInterface,
		MOS_GPU_CONTEXT_RENDER3,
		MOS_GPU_CONTEXT_VEBOX);  

    pOsInterface->pfnResetPerfBufferID(pOsInterface);
    if (!(pOsInterface->pfnIsPerfTagSet(pOsInterface)))
    {
        pOsInterface->pfnIncPerfFrameID(pOsInterface);
        pOsInterface->pfnSetPerfTag(pOsInterface, VEBOX_TASK_PERFTAG_INDEX);
    }

    // Add PipeControl to invalidate ISP and MediaState to avoid PageFault issue
    MHW_PIPE_CONTROL_PARAMS PipeControlParams;

    MOS_ZeroMemory(&PipeControlParams, sizeof(PipeControlParams));
    PipeControlParams.dwFlushMode = MHW_FLUSH_WRITE_CACHE;
    PipeControlParams.bGenericMediaStateClear = true;
    PipeControlParams.bIndirectStatePointersDisable = true;
    PipeControlParams.bDisableCSStall = false;
    CM_CHK_MOSSTATUS(pRenderHal->pMhwMiInterface->AddPipeControl(&CmdBuffer, nullptr, &PipeControlParams));

    if (MEDIA_IS_WA(pRenderHal->pWaTable, WaSendDummyVFEafterPipelineSelect))
    {
        MHW_VFE_PARAMS VfeStateParams;

        MOS_ZeroMemory(&VfeStateParams, sizeof(VfeStateParams));
        VfeStateParams.dwNumberofURBEntries = 1;
        CM_CHK_MOSSTATUS(pRenderHal->pMhwRenderInterface->AddMediaVfeCmd(&CmdBuffer, &VfeStateParams));
    }

	//Couple to the BB_START , otherwise GPU Hang without it in KMD.
	CM_CHK_MOSSTATUS(pRenderHal->pMhwMiInterface->AddMiBatchBufferEnd(&CmdBuffer, nullptr));

	//---------------------------------
	// Return unused command buffer space to OS
	//---------------------------------
	pOsInterface->pfnReturnCommandBuffer(
		pOsInterface,
		&CmdBuffer, 0);

	//---------------------------------
	// submit the command buffer
	//---------------------------------
	CM_CHK_MOSSTATUS(pOsInterface->pfnSubmitCommandBuffer(
		pOsInterface,
		&CmdBuffer,
		pState->bNullHwRenderCm));

	// Set the Task ID
	pExecVeboxParam->iTaskIdOut = iTaskId;

	// pass back the Command Buffer
	pState->pfnReferenceCommandBuffer(&CmdBuffer.OsResource, &pExecVeboxParam->OsData);

	// Update the task ID table
	pState->pTaskStatusTable[iTaskId] = (char)iTaskId;

	if (!(pState->bNullHwRenderCm))
	{
		// Update Vebox Sync tag info
		pVeboxHeap->pStates[pVeboxHeap->uiCurState].dwSyncTag = pVeboxHeap->dwNextTag++;
		pVeboxHeap->pStates[pVeboxHeap->uiCurState].bBusy = true;
	}

	hr = MOS_STATUS_SUCCESS;

finish:

	// Failed -> discard all changes in Command Buffer
	if (hr != MOS_STATUS_SUCCESS)
	{
		// Buffer overflow - display overflow size
		if (CmdBuffer.iRemaining < 0)
		{
			CM_PUBLIC_ASSERTMESSAGE("Command Buffer overflow by %d bytes", CmdBuffer.iRemaining);
		}

		// Move command buffer back to beginning
		i = iRemaining - CmdBuffer.iRemaining;
		CmdBuffer.iRemaining = iRemaining;
		CmdBuffer.iOffset -= i;
		CmdBuffer.pCmdPtr = CmdBuffer.pCmdBase + CmdBuffer.iOffset / sizeof(uint32_t);

		// Return unused command buffer space to OS
		pOsInterface->pfnReturnCommandBuffer(pOsInterface, &CmdBuffer, 0);
	}

	return hr;
}


//!
//! \brief      Set up vebox surface Param
//! \details    set up vebox surface state based on parameter based from application
//!
//! \param     [in]pState  -- CM_HAL_STATE
//! \param     [in]pVeboxSurfaceStateCmdParams  -- surface state param struct
//!
MOS_STATUS HalCm_SetVeboxSurfaceStateCmdParams(
	PCM_HAL_STATE                           pState,
	PMHW_VEBOX_SURFACE_STATE_CMD_PARAMS   pVeboxSurfaceStateCmdParams)
{
	if ((pState->cmVeboxSettings.bCmIECPEnable) && !((pState->cmVeboxSettings.bCmDIEnable) || (pState->cmVeboxSettings.bCmDNEnable)))
	{
		// IECP only
		HalCm_Convert_RENDERHAL_SURFACE_To_MHW_VEBOX_SURFACE(&pState->cmVeboxSurfaces[VEBOX_CURRENT_FRAME_INPUT_SURF], &pVeboxSurfaceStateCmdParams->SurfInput);
		HalCm_Convert_RENDERHAL_SURFACE_To_MHW_VEBOX_SURFACE(&pState->cmVeboxSurfaces[VEBOX_CURRENT_FRAME_OUTPUT_SURF], &pVeboxSurfaceStateCmdParams->SurfOutput);
		pVeboxSurfaceStateCmdParams->bDIEnable = false;
		pVeboxSurfaceStateCmdParams->bOutputValid = true;
	}
	else
	{
		// DN only, will add other support later

		HalCm_Convert_RENDERHAL_SURFACE_To_MHW_VEBOX_SURFACE(&pState->cmVeboxSurfaces[VEBOX_CURRENT_FRAME_INPUT_SURF], &pVeboxSurfaceStateCmdParams->SurfInput);
		HalCm_Convert_RENDERHAL_SURFACE_To_MHW_VEBOX_SURFACE(&pState->cmVeboxSurfaces[VEBOX_DN_CURRENT_FRAME_OUTPUT_SURF], &pVeboxSurfaceStateCmdParams->SurfOutput);
		HalCm_Convert_RENDERHAL_SURFACE_To_MHW_VEBOX_SURFACE(&pState->cmVeboxSurfaces[VEBOX_STMM_INPUT_SURF], &pVeboxSurfaceStateCmdParams->SurfSTMM);
		HalCm_Convert_RENDERHAL_SURFACE_To_MHW_VEBOX_SURFACE(&pState->cmVeboxSurfaces[VEBOX_STMM_OUTPUT_SURF], &pVeboxSurfaceStateCmdParams->SurfDNOutput);

		pVeboxSurfaceStateCmdParams->bDIEnable = false;
		pVeboxSurfaceStateCmdParams->bOutputValid = true;
	}

	return MOS_STATUS_SUCCESS;
}


//!
//! \brief    set vebox DiIecp Command
//! \details  build up command to start processing the frames specified by
//!           VEB_SURFACE_STATE using the parameters specified by VEB_DI_STATE
//!           and VEB_IECP_STATE.
//!  \param   [in] pState -- HAL_CM_STATE
//!  \param   [in] pVeboxDiIecpCmdParams  -- DIECP command parameter
//!  \param   [in] pCmVeboxSurfaceDataInput  -- surface data such as index and control bits
//!
MOS_STATUS HalCm_SetVeboxDiIecpCmdParams(
	PCM_HAL_STATE                   pState,
	PMHW_VEBOX_DI_IECP_CMD_PARAMS   pVeboxDiIecpCmdParams,
	PCM_VEBOX_SURFACE_DATA       pCmVeboxSurfaceDataInput)
{
	uint32_t                 dwWidth;
    uint32_t                 dwHeight;
	bool                     bDIEnable;
	MHW_VEBOX_SURFACE_PARAMS SurfInput;

	// DN only, will add other support later
	bDIEnable = false;

	// Align dwEndingX with surface state
	HalCm_Convert_RENDERHAL_SURFACE_To_MHW_VEBOX_SURFACE(&pState->cmVeboxSurfaces[VEBOX_CURRENT_FRAME_INPUT_SURF], &SurfInput);
	pState->pVeboxInterface->VeboxAdjustBoundary(
		&SurfInput,
		&dwWidth, 
        &dwHeight,
		bDIEnable);

	pVeboxDiIecpCmdParams->dwStartingX = 0;
	pVeboxDiIecpCmdParams->dwEndingX = dwWidth - 1;

	if (!pState->cmVeboxSettings.bCmDnDiFirstFrame)
	{
		pVeboxDiIecpCmdParams->pOsResPrevInput = &pState->cmVeboxSurfaces[VEBOX_PREVIOUS_FRAME_INPUT_SURF].OsSurface.OsResource;
		pVeboxDiIecpCmdParams->PrevInputSurfCtrl.Value = pCmVeboxSurfaceDataInput->surfaceEntry[VEBOX_PREVIOUS_FRAME_INPUT_SURF].wSurfaceCtrlBits;
	}

	pVeboxDiIecpCmdParams->pOsResCurrInput = &pState->cmVeboxSurfaces[VEBOX_CURRENT_FRAME_INPUT_SURF].OsSurface.OsResource;
	pVeboxDiIecpCmdParams->CurrInputSurfCtrl.Value = pCmVeboxSurfaceDataInput->surfaceEntry[VEBOX_CURRENT_FRAME_INPUT_SURF].wSurfaceCtrlBits;


	if ((pState->cmVeboxSettings.bCmDIEnable) || (pState->cmVeboxSettings.bCmDNEnable))
	{
		pVeboxDiIecpCmdParams->pOsResStmmInput = &pState->cmVeboxSurfaces[VEBOX_STMM_INPUT_SURF].OsSurface.OsResource;
		pVeboxDiIecpCmdParams->StmmInputSurfCtrl.Value = pCmVeboxSurfaceDataInput->surfaceEntry[VEBOX_STMM_INPUT_SURF].wSurfaceCtrlBits;

		pVeboxDiIecpCmdParams->pOsResStmmOutput = &pState->cmVeboxSurfaces[VEBOX_STMM_OUTPUT_SURF].OsSurface.OsResource;
		pVeboxDiIecpCmdParams->StmmOutputSurfCtrl.Value = pCmVeboxSurfaceDataInput->surfaceEntry[VEBOX_STMM_OUTPUT_SURF].wSurfaceCtrlBits;
	}

	if ((pState->cmVeboxSettings.bCmIECPEnable) && !((pState->cmVeboxSettings.bCmDIEnable) || (pState->cmVeboxSettings.bCmDNEnable)))
	{
		pVeboxDiIecpCmdParams->pOsResCurrOutput = &pState->cmVeboxSurfaces[VEBOX_CURRENT_FRAME_OUTPUT_SURF].OsSurface.OsResource;
		pVeboxDiIecpCmdParams->CurrOutputSurfCtrl.Value = pCmVeboxSurfaceDataInput->surfaceEntry[VEBOX_CURRENT_FRAME_OUTPUT_SURF].wSurfaceCtrlBits;
        pVeboxDiIecpCmdParams->pOsResLaceOrAceOrRgbHistogram = &pState->cmVeboxSurfaces[VEBOX_LACE_ACE_RGB_HISTOGRAM_OUTPUT_SURF].OsSurface.OsResource;
        pVeboxDiIecpCmdParams->LaceOrAceOrRgbHistogramSurfCtrl.Value = pCmVeboxSurfaceDataInput->surfaceEntry[VEBOX_LACE_ACE_RGB_HISTOGRAM_OUTPUT_SURF].wSurfaceCtrlBits;
	}

	if (pState->cmVeboxSettings.bCmDNEnable)
	{
		pVeboxDiIecpCmdParams->pOsResDenoisedCurrOutput = &pState->cmVeboxSurfaces[VEBOX_DN_CURRENT_FRAME_OUTPUT_SURF].OsSurface.OsResource;
		pVeboxDiIecpCmdParams->DenoisedCurrOutputSurfCtrl.Value = pCmVeboxSurfaceDataInput->surfaceEntry[VEBOX_DN_CURRENT_FRAME_OUTPUT_SURF].wSurfaceCtrlBits;
	}

	if (pState->cmVeboxSettings.bVignetteEnable)
	{
		pVeboxDiIecpCmdParams->pOsResAlphaOrVignette = &pState->cmVeboxSurfaces[VEBOX_ALPHA_VIGNETTE_CORRECTION_SURF].OsSurface.OsResource;
		pVeboxDiIecpCmdParams->DenoisedCurrOutputSurfCtrl.Value = pCmVeboxSurfaceDataInput->surfaceEntry[VEBOX_DN_CURRENT_FRAME_OUTPUT_SURF].wSurfaceCtrlBits;
	}

	pVeboxDiIecpCmdParams->pOsResStatisticsOutput = &pState->cmVeboxSurfaces[VEBOX_STATISTICS_OUTPUT_SURF].OsSurface.OsResource;
	pVeboxDiIecpCmdParams->StatisticsOutputSurfCtrl.Value = pCmVeboxSurfaceDataInput->surfaceEntry[VEBOX_STATISTICS_OUTPUT_SURF].wSurfaceCtrlBits;

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
	PMOS_INTERFACE          pOsInterface,
	PRENDERHAL_SURFACE      pRenderHalSTMMSurface)
{
	MOS_STATUS          hr;
	uint32_t            dwSize;
	int32_t             x, y;
	uint8_t             *pByte;
	MOS_LOCK_PARAMS     LockFlags;

	hr = MOS_STATUS_SUCCESS;
	PMOS_SURFACE pSTMMSurface = &pRenderHalSTMMSurface->OsSurface;

	MOS_ZeroMemory(&LockFlags, sizeof(MOS_LOCK_PARAMS));

	LockFlags.WriteOnly = 1;

	// Lock the surface for writing
	pByte = (uint8_t*)pOsInterface->pfnLockResource(
		pOsInterface,
		&pSTMMSurface->OsResource,
		&LockFlags);

	CM_CHK_NULL_RETURN_MOSSTATUS(pByte);

	dwSize = pSTMMSurface->dwWidth >> 2;

	// Fill STMM surface with DN history init values.
	for (y = 0; y < (int32_t)pSTMMSurface->dwHeight; y++)
	{
		for (x = 0; x < (int32_t)dwSize; x++)
		{
			MOS_FillMemory(pByte, 2, DNDI_HISTORY_INITVALUE);
			// skip denoise history init.
			pByte += 4;
		}

		pByte += pSTMMSurface->dwPitch - pSTMMSurface->dwWidth;
	}

	// Unlock the surface
	CM_HRESULT2MOSSTATUS_AND_CHECK(pOsInterface->pfnUnlockResource(
		pOsInterface,
		&pSTMMSurface->OsResource));

finish:
	return hr;
}
