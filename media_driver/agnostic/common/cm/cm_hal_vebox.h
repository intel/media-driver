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
//! \file      cm_hal_vebox.h
//! \brief     HAL CM Vebox functions
//!

#ifndef MEDIADRIVER_AGNOSTIC_COMMON_CM_CMHALVEBOX_H_
#define MEDIADRIVER_AGNOSTIC_COMMON_CM_CMHALVEBOX_H_

#include "mos_os.h"
#include "renderhal_legacy.h"
#include "mhw_vebox.h"

struct CM_VEBOX_SURFACE
{
    uint16_t surfaceIndex;
    uint16_t surfaceCtrlBits;
};

#define VEBOX_SURFACE_NUMBER (16)     //MAX

struct CM_VEBOX_SURFACE_DATA
{
    CM_VEBOX_SURFACE surfaceEntry[VEBOX_SURFACE_NUMBER];
};
typedef CM_VEBOX_SURFACE_DATA *PCM_VEBOX_SURFACE_DATA;

struct CM_HAL_EXEC_VEBOX_TASK_PARAM
{
    uint32_t veboxVersion;   // [in] version
    CM_VEBOX_STATE cmVeboxState;
    void *veboxParam;         // CmBuffer, hold the vebox parameters
    uint32_t veboxParamIndex;  // vebox parameter surface index (UMD)
    CM_VEBOX_SURFACE_DATA veboxSurfaceData;
    CM_QUEUE_CREATE_OPTION queueOption; // cm queue info, such as rcs/ccs gpu context.
    int32_t taskIdOut;        // [out] Task ID
    void *osData;              // [out] Used for Linux OS data to pass to event
};
typedef CM_HAL_EXEC_VEBOX_TASK_PARAM *PCM_HAL_EXEC_VEBOX_TASK_PARAM;

enum VEBOX_SURF_USAGE
{
    VEBOX_CURRENT_FRAME_INPUT_SURF,
    VEBOX_PREVIOUS_FRAME_INPUT_SURF,
    VEBOX_STMM_INPUT_SURF,
    VEBOX_STMM_OUTPUT_SURF,
    VEBOX_DN_CURRENT_FRAME_OUTPUT_SURF,
    VEBOX_CURRENT_FRAME_OUTPUT_SURF,
    VEBOX_PREVIOUS_FRAME_OUTPUT_SURF,
    VEBOX_STATISTICS_OUTPUT_SURF,
    VEBOX_ALPHA_VIGNETTE_CORRECTION_SURF,
    VEBOX_LACE_ACE_RGB_HISTOGRAM_OUTPUT_SURF,
    VEBOX_SKIN_SCORE_OUTPUT_SURF
};

//!
//! \brief      build up vebox command sequence
//! \details     based on passed vebox param to build command sequence and
//!             put it into command buffer
//! \param       [in] state --- CM_HAL_STATE
//! \param       [in] execVeboxParam -- vebox setup params
//!
MOS_STATUS HalCm_ExecuteVeboxTask(
    PCM_HAL_STATE                   state,           // [in] Pointer to CM State
    PCM_HAL_EXEC_VEBOX_TASK_PARAM   execVeboxParam);

//!
//! \brief      Set up vebox surface Param
//! \details    set up vebox surface state based on parameter based from application
//!
//! \param     [in]state  -- CM_HAL_STATE
//! \param     [in]veboxSurfaceStateCmdParams  -- surface state param struct
//!
MOS_STATUS HalCm_SetVeboxSurfaceStateCmdParams(
    PCM_HAL_STATE                   state,
    PMHW_VEBOX_SURFACE_STATE_CMD_PARAMS   veboxSurfaceStateCmdParams);

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
    PCM_VEBOX_SURFACE_DATA       cmVeboxSurfaceDataInput);

//!
//| \brief     reset vebox STMM history
//! \details   Resets the portion of the Vebox STMM surface associated with
//|            motion history for temporal filtering.
//| \param     [in] osInterface  -- MOS_INTERFACE
//| \param     [in] renderHalSTMMSurface -- RENDERHAL_SURFACE
//!
MOS_STATUS HalCm_VeboxInitSTMMHistory(
    PMOS_INTERFACE          osInterface,
    PRENDERHAL_SURFACE      renderHalSTMMSurface);

#endif  // #ifndef MEDIADRIVER_AGNOSTIC_COMMON_CM_CMHALVEBOX_H_
