/*
* Copyright (c) 2016-2018, Intel Corporation
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
//! \file     codechal_decode_singlepipe_virtualengine.h
//! \brief    Defines the decode interface extension for single pipe virtual engine.
//! \details  Defines all types, macros, and functions required by CodecHal for single pipe virtual engine decoding. Definitions are not externally facing.
//!

#ifndef __CODECHAL_DECODER_SINGLEPIPE_VIRTUALENGINE_H__
#define __CODECHAL_DECODER_SINGLEPIPE_VIRTUALENGINE_H__

#include "codechal_decoder.h"
#include "mos_os_virtualengine_singlepipe.h"

typedef struct _CODECHAL_DECODE_SINGLEPIPE_VIRTUALENGINE_STATE
{
    PMOS_VIRTUALENGINE_INTERFACE   pVEInterface;
    PMOS_VIRTUALENGINE_HINT_PARAMS pHintParms;
}CODECHAL_DECODE_SINGLEPIPE_VIRTUALENGINE_STATE, *PCODECHAL_DECODE_SINGLEPIPE_VIRTUALENGINE_STATE;

//!
//! \brief    Set hint parameter for virtual engine interface in single pipe mode
//! \details  Set hint prameter for virtual engine interface, shared by all single pipe codechal decode formats.
//! \param    [in]  pVEState
//!                ponter to virtual engine state data structure
//! \param    [in]  pVESetParams
//!                ponter to MOS_VIRTUALENGINE_SET_PARAMS data structure including parameters related to set hint parameters
//! \return   MOS_STATUS
//!             MOS_STATUS_SUCCESS if success, else fail reason
//!
static __inline MOS_STATUS CodecHalDecodeSinglePipeVE_SetHintParams(
    PCODECHAL_DECODE_SINGLEPIPE_VIRTUALENGINE_STATE pVEState,
    PMOS_VIRTUALENGINE_SET_PARAMS                   pVESetParams)
{
    PMOS_VIRTUALENGINE_INTERFACE   pVEInterface;
    MOS_STATUS                     eStatus = MOS_STATUS_SUCCESS;

    // this function can be removed after transiting to VE2.0
    CODECHAL_DECODE_CHK_NULL_RETURN(pVEState);
    CODECHAL_DECODE_CHK_NULL_RETURN(pVEState->pVEInterface);
    pVEInterface = pVEState->pVEInterface;

    if(!MOS_VE_CTXBASEDSCHEDULING_SUPPORTED(pVEInterface->pOsInterface))
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(pVEInterface->pfnVESetHintParams(pVEInterface, pVESetParams));
    }

    return eStatus;
}

//!
//! \brief    Populate hint parameter for virtual engine interface in single pipe mode
//! \details  Populate hint prameter to primary cmd buffer attributes, this is for virtual engine interface, shared by all single pipe codechal decode formats.
//! \param    [in]  pVEState
//!                ponter to virtual engine state data structure
//! \param    [in]  pPrimCmdBuf
//!                ponter to primary cmd buffer
//! \param    [in]  bUseVirtualEngineHint
//!                true if CmdBuf need to use VE hint params, else false
//! \return   PMOS_VIRTUALENGINE_HINT_PARAMS
//!              return address of the single pipe hint parameter variable defined in MOS Virtual engine interface
//!
static __inline MOS_STATUS CodecHalDecodeSinglePipeVE_PopulateHintParams(
    PCODECHAL_DECODE_SINGLEPIPE_VIRTUALENGINE_STATE pVEState,
    PMOS_COMMAND_BUFFER                             pPrimCmdBuf,
    bool                                            bUseVirtualEngineHint)
{
    MOS_STATUS                     eStatus = MOS_STATUS_SUCCESS;
    PMOS_CMD_BUF_ATTRI_VE         pAttriVe;

    CODECHAL_DECODE_CHK_NULL_RETURN(pPrimCmdBuf);
    CODECHAL_DECODE_CHK_NULL_RETURN(pPrimCmdBuf->Attributes.pAttriVe);

    pAttriVe = (PMOS_CMD_BUF_ATTRI_VE)(pPrimCmdBuf->Attributes.pAttriVe);

    if (bUseVirtualEngineHint)
    {
        CODECHAL_DECODE_CHK_NULL_RETURN(pVEState);
        CODECHAL_DECODE_CHK_NULL_RETURN(pVEState->pHintParms);

        pAttriVe->VEngineHintParams = *(pVEState->pHintParms);
    }

    pAttriVe->bUseVirtualEngineHint = bUseVirtualEngineHint;

    return eStatus;
}

//!
//! \brief     construct gpu context creation params.
//! \param    [in]  pVEState
//!                ponter to virtual engine state data structure
//! \param    [in]  gpucontxCreatOpts
//!                ponter to gpu context creation option data structure
//! \param    [in]  SFCInuse
//!                flag to indicate if SFC is in use
//! \return    MOS_STATUS
//!           MOS_STATUS_SUCCESS if success, else fail reason
//!
MOS_STATUS CodecHalDecodeSinglePipeVE_ConstructParmsForGpuCtxCreation(
    PCODECHAL_DECODE_SINGLEPIPE_VIRTUALENGINE_STATE pVEState,
    PMOS_GPUCTX_CREATOPTIONS_ENHANCED               gpuCtxCreatOpts,
    bool                                            SFCInuse);

//!
//! \brief     initialize virtual engine interface for single pipe decode 
//! \details  initialize virtual engine interface for single pipe decode shared by all codechal decode formats
//! \param    [in]  pOsInterface
//!                poiner to os interface
//! \param    [in]  pVEState
//!                pointer to virtual engine state data structure
//! \return    MOS_STATUS
//!           MOS_STATUS_SUCCESS if success, else fail reason
//!
MOS_STATUS CodecHalDecodeSinglePipeVE_InitInterface(
    PMOS_INTERFACE                                  pOsInterface,
    PCODECHAL_DECODE_SINGLEPIPE_VIRTUALENGINE_STATE pVEState);

#endif //__CODECHAL_DECODER_SINGLEPIPE_VIRTUALENGINE_H__

