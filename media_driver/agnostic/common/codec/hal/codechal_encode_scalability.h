/*
* Copyright (c) 2018, Intel Corporation
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
//! \file     codechal_encode_scalability.h
//! \brief    Defines the encode interface extension for scalability.
//! \details  Defines all types, macros, and functions required by CodecHal for virtual engine encode supporting both single pipe and scalable mode. Definitions are not externally facing.
//!

#ifndef __CODECHAL_ENCODER_SCALABILITY_H__
#define __CODECHAL_ENCODER_SCALABILITY_H__

#include "codechal_encoder_base.h"
#include "mos_os_virtualengine_scalability.h"

typedef struct _CODECHAL_ENCODE_SCALABILITY_SETHINT_PARMS
{
    bool                bSameEngineAsLastSubmission;
    bool                bNeedSyncWithPrevious;
    MOS_RESOURCE        veBatchBuffer[MOS_MAX_ENGINE_INSTANCE_PER_CLASS];    // Remove once Encode moves to using secondary command buffers in MOS VE interface
}CODECHAL_ENCODE_SCALABILITY_SETHINT_PARMS, *PCODECHAL_ENCODE_SCALABILITY_SETHINT_PARMS;

typedef struct _CODECHAL_ENCODE_SCALABILITY_STATE
{
    CodechalHwInterface            *pHwInterface;
    uint8_t                         ucScalablePipeNum;
    MOS_GPU_CONTEXT                 VideoContextScalable;   // Remove once legacy/ new GPU context class support context re-use
    MOS_GPU_CONTEXT                 VideoContextSinglePipe; // Remove once legacy/ new GPU context class support context re-use

    //Virtual Engine related
    PMOS_VIRTUALENGINE_INTERFACE    pVEInterface;
    PMOS_VIRTUALENGINE_HINT_PARAMS  pScalHintParms;
    PMOS_VIRTUALENGINE_HINT_PARAMS  pSingleHintParms;

}CODECHAL_ENCODE_SCALABILITY_STATE, *PCODECHAL_ENCODE_SCALABILITY_STATE;

//!
//! \brief    switch gpu context in scalability encode mode
//! \param    [in] pScalabilityState
//!                Scalability encode state
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success, else fail reason
//!
MOS_STATUS CodecHalEncodeScalability_SwitchGpuContext(
    PCODECHAL_ENCODE_SCALABILITY_STATE  pScalabilityState);

//!
//! \brief    State initialization for virtual engine encode supporting scalable and single pipe mode 
//! \param    [in]  pScalabilityState
//!                Scalability encode state
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success, else fail reason
//!
MOS_STATUS CodecHalEncodeScalability_InitializeState(
    PCODECHAL_ENCODE_SCALABILITY_STATE  pScalabilityState,
    CodechalHwInterface                 *hwInterface);

//!
//! \brief    Initiliaze Encode Parameters for virtual engine encode
//! \details  Initiliaze encode parameters for virtual engine encode. this is for encoder supporting scalability but not necessarily always running in scalable mode
//! \param    [in]  pScalabilityState
//!                Scalability encode state
//! \param    [in] pucDecPassNum
//!                pointer to encode pass number
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success, else fail reason
//!
MOS_STATUS CodecHalEncodeScalability_InitScalableParams(
    PCODECHAL_ENCODE_SCALABILITY_STATE         pScalabilityState,
    uint8_t                                   *pucDecPassNum);

//!
//! \brief     Set virtual engine hint parameters for scalable encode
//! \param    [in]  pScalabilityState
//!                Scalability encode state
//! \param    [in] pSetHintParms
//!                pointer to set hint parameter 
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success, else fail reason
//!
MOS_STATUS CodecHalEncodeScalability_SetHintParams(
    CodechalEncoderState                       *pEncoder,
    PCODECHAL_ENCODE_SCALABILITY_STATE         pScalabilityState,
    PCODECHAL_ENCODE_SCALABILITY_SETHINT_PARMS pSetHintParms);

//!
//! \brief     Populate virtual engine hint parameters
//! \details  Populate virtual engine hint parameters. Support both scalable and single pipe encode mode.
//! \param    [in]  pScalabilityState
//!                Scalability encode state
//! \param    [in] pPrimCmdBuf
//!                pointer to primary cmd buffer 
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success, else fail reason
//!
MOS_STATUS CodecHalEncodeScalability_PopulateHintParams(
    PCODECHAL_ENCODE_SCALABILITY_STATE  pScalabilityState,
    PMOS_COMMAND_BUFFER                 pPrimCmdBuf);

//! \brief    construct gpu context creation options when scalability supported
//! \param    [in]  scalabilityState
//!                Scalability encode state
//! \param    [in]  gpuCtxCreatOptions
//!                pointer to gpu context creation options
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success, else fail reason
//!
MOS_STATUS CodechalEncodeScalability_ConstructParmsForGpuCtxCreation(
    PCODECHAL_ENCODE_SCALABILITY_STATE         pScalState,
    PMOS_GPUCTX_CREATOPTIONS_ENHANCED          gpuCtxCreatOpts);

//! \brief    Check if need to recreate gpu context and if yes, do it.
//! \param    [in]  scalabilityState
//!                Scalability encode state
//! \param    [in]  gpuCtxCreatOptions
//!                pointer to gpu context creation options
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success, else fail reason
//!
MOS_STATUS CodechalEncodeScalability_ChkGpuCtxReCreation(
    CodechalEncoderState                       *pEncoder,
    PCODECHAL_ENCODE_SCALABILITY_STATE         pScalabilityState,
    PMOS_GPUCTX_CREATOPTIONS_ENHANCED          CurgpuCtxCreatOpts);

#endif //__CODECHAL_ENCODER_SCALABILITY_H__