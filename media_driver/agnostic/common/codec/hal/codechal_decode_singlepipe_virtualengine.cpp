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
//! \file     codechal_decode_singlepipe_virtualengine.cpp
//! \brief    Implements the decode interface extension for single pipe virtual engine.
//! \details  Implements all functions required by CodecHal for single pipe decoding with virtual engine interface.
//!
#include "codechal_decode_singlepipe_virtualengine.h"
#include "mos_os_virtualengine_next.h"

//==<Functions>=======================================================
MOS_STATUS CodecHalDecodeSinglePipeVE_ConstructParmsForGpuCtxCreation(
    PCODECHAL_DECODE_SINGLEPIPE_VIRTUALENGINE_STATE pVEState,
    PMOS_GPUCTX_CREATOPTIONS_ENHANCED               gpuCtxCreatOpts,
    bool                                            SFCInuse)
{
    PMOS_VIRTUALENGINE_INTERFACE   pVEInterface;
    MOS_STATUS                     eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    CODECHAL_DECODE_CHK_NULL_RETURN(pVEState);
    CODECHAL_DECODE_CHK_NULL_RETURN(pVEState->pVEInterface);

    pVEInterface = pVEState->pVEInterface;

#if (_DEBUG || _RELEASE_INTERNAL)
    if (pVEInterface->pOsInterface->bEnableDbgOvrdInVE)
    {
        gpuCtxCreatOpts->DebugOverride      = true;
        gpuCtxCreatOpts->LRCACount          = 1;
        gpuCtxCreatOpts->UsingSFC           = SFCInuse;  // this param ignored when dbgoverride enabled
        if (g_apoMosEnabled)
        {
            CODECHAL_DECODE_CHK_NULL_RETURN(pVEInterface->veInterface);
            CODECHAL_DECODE_ASSERT(pVEInterface->veInterface->GetEngineCount() == 1);
            gpuCtxCreatOpts->EngineInstance[0] = pVEInterface->veInterface->GetEngineLogicId(0);
        }
        else
        {
            CODECHAL_DECODE_ASSERT(pVEInterface->ucEngineCount == 1);
            gpuCtxCreatOpts->EngineInstance[0] = pVEInterface->EngineLogicId[0];
        }
    }
    else
#endif
    {
        gpuCtxCreatOpts->LRCACount          = 1;
        gpuCtxCreatOpts->UsingSFC           = SFCInuse;
    }

    return eStatus;
}

MOS_STATUS CodecHalDecodeSinglePipeVE_InitInterface(
    PMOS_INTERFACE                                  pOsInterface,
    PCODECHAL_DECODE_SINGLEPIPE_VIRTUALENGINE_STATE pVEState)
{
    PMOS_VIRTUALENGINE_INTERFACE   pVEInterface;
    MOS_VIRTUALENGINE_INIT_PARAMS  VEInitParams;
    MOS_STATUS                     eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    CODECHAL_DECODE_CHK_NULL_RETURN(pOsInterface);
    CODECHAL_DECODE_CHK_NULL_RETURN(pVEState);

    //virtual engine init with singlepipe
    MOS_ZeroMemory(&VEInitParams, sizeof(VEInitParams));
    VEInitParams.bScalabilitySupported = false;
    CODECHAL_DECODE_CHK_STATUS_RETURN(Mos_VirtualEngineInterface_Initialize(pOsInterface, &VEInitParams));
    pVEState->pVEInterface = pVEInterface = pOsInterface->pVEInterf;

    if (pVEInterface->pfnVEGetHintParams)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(pVEInterface->pfnVEGetHintParams(pVEInterface, false, &pVEState->pHintParms));
    }

    return eStatus;
}

