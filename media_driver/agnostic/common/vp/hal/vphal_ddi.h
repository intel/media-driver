/*
* Copyright (c) 2009-2017, Intel Corporation
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
//! \file      vphal_ddi.h 
//! \brief 
//!
//!
//! \file     vphal_ddi.h
//! \brief    VPHAL related utility functions that are needed in DDI layer
//! \details  Common utility functions for different DDI layers
//!
#ifndef __VPHAL_DDI_H__
#define __VPHAL_DDI_H__

#include "vphal.h"          // Vphal interface structures and definitions

//!
//! \brief 60 to 30fps detection threshold,
//!        larger values may increase the detection time, and should be used
//!        with caution;
//!        smaller values increase probability of switching between 30 and
//!        60fps, and may cause instability if frame drops are detected
//!
#define FPS_60_TO_30_THRESHOLD 10

//!
//! \brief 30 to 60fps detection threshold,
//!        larger values may increase the detection time, and should be used
//!        with caution;
//!        smaller values increase probability of switching between 30 and
//!        60fps, and may cause instability if frame drops are detected
//!
#define FPS_30_TO_60_THRESHOLD 10

//!
//! \brief Frame rate detection states enum
//!
typedef enum _FPS_DETECTION_STATE
{
    FPS_RESET = 0,            //!< Initial state
    FPS_30_F0,                //!< 30fps, field 0
    FPS_30_F1,                //!< 30fps, field 1
    FPS_60_F0,                //!< 60fps, field 0
    FPS_60_F1,                //!< 60fps, field 1
    FPS_30_TO_60,             //!< 30fps to 60fps detection
    FPS_60_TO_30,             //!< 60fps to 30fps detection

    FPS_DETECTION_STATES      //!< Number of frame rate detection states
} FPS_DETECTION_STATE;


//!
//! \brief    Judge whether the input procamp value is default or not
//! \details  If the procamp values requested are outside one step of the 
//!           default value(to handle precision errors), then return true
//! \param    [in] ProcAmpParameters
//!           ProcAmp Parameters
//! \return   bool
//!           - true  The input procamp value is not default
//!           - false The input procamp value is default
//!
bool VpHal_DdiProcAmpValuesNotDefault(
    VPHAL_PROCAMP_PARAMS    ProcAmpParameters);

//!
//! \brief    Destroy VPHAL rendering parameters
//! \details  Free source/target surface and other parameters
//! \param    [in,out] pRenderParams
//!           Render parameter pointer
//! \return   void
//!
void VpHal_DdiReleaseRenderParams(
    PVPHAL_RENDER_PARAMS    pRenderParams);

//!
//! \brief    Delete surface at DDI layer
//! \details  Free different parameter structures in surfaces and free surfaces
//!           at DDI layer, e.g. Sourc/Target/Bwd/Fwd surface
//! \param    [in,out] pSurf
//!           VPHAL surface pointer
//! \return   void
//!
void VpHal_DdiDeleteSurface(
    PVPHAL_SURFACE          pSurf);

//!
//! \brief    Report mode of different features
//! \details  Report DI/Scaling/OutputPipe/FRC mode
//! \param    [in] pVpHalState
//!           VPHAL state pointer
//! \param    [in,out] pConfigValues
//!           Porinter to configuration report value structure,
//!           feature modes will be store in this structure.
//! \return   void
//!
void VpHal_DdiReportFeatureMode(
    VpBase*                 pVpHalState,
    PVP_CONFIG              pConfigValues);

//!
//! \brief    Set up split screen demo mode
//! \details  Allocate and initialize split-screen demo mode structure
//! \param    [in] splitDemoPosDdi
//!           The split demo position setting from DDI layer
//! \param    [in] splitDemoParaDdi
//!           The split demo parameters setting from DDI layer
//! \param    [in,out] splitScreenDemoModeParams
//!           Pointer to struct for split-screen demo mode parameters
//! \param    [in,out] disableDemoMode
//!           Return whether demo mode will be disable or not
//! \param    [in] disableDemoMode
//!           Pointer to MOS INTERFACE for OS interaction
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VpHal_DdiSetupSplitScreenDemoMode(
    uint32_t                                splitDemoPosDdi,
    uint32_t                                splitDemoParaDdi,
    PVPHAL_SPLIT_SCREEN_DEMO_MODE_PARAMS    *splitScreenDemoModeParams,
    bool                                    *disableDemoMode,
    PMOS_INTERFACE                          pOsInterface);

//!
//! \brief    Init IEF Params to their default value
//! \details  Init IEF Params to their default value
//! \param    [out] pIEFParams
//!           The IEF Params struct to be initialized
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VpHal_DdiInitIEFParams(
    PVPHAL_IEF_PARAMS       pIEFParams);

#endif   // __VPHAL_DDI_H__

