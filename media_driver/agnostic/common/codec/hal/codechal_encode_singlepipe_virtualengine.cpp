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
//! \file     codechal_encode_singlepipe_virtualengine.cpp
//! \brief    Implements the encode interface extension for single pipe virtual engine.
//! \details  Implements all functions required by CodecHal for single pipe encoding with virtual engine interface.
//!
#include "codechal_encode_singlepipe_virtualengine.h"
#include "mos_os_virtualengine_next.h"

MOS_STATUS CodecHalEncodeSinglePipeVE_ConstructParmsForGpuCtxCreation(
    PCODECHAL_ENCODE_SINGLEPIPE_VIRTUALENGINE_STATE pVEState,
    PMOS_GPUCTX_CREATOPTIONS_ENHANCED               gpuCtxCreatOpts)
{
    PMOS_VIRTUALENGINE_INTERFACE   pVEInterface;
    MOS_STATUS                     eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(pVEState);
    CODECHAL_ENCODE_CHK_NULL_RETURN(pVEState->pVEInterface);

    pVEInterface = pVEState->pVEInterface;
    auto pOsInterface = pVEInterface->pOsInterface;
    CODECHAL_ENCODE_CHK_NULL_RETURN(pOsInterface);

    gpuCtxCreatOpts->UsingSFC  = false;
    gpuCtxCreatOpts->LRCACount = 1;

#if (_DEBUG || _RELEASE_INTERNAL)
    if (pOsInterface->bEnableDbgOvrdInVE)
    {
        gpuCtxCreatOpts->DebugOverride      = true;
        if (g_apoMosEnabled)
        {
            CODECHAL_ENCODE_CHK_NULL_RETURN(pVEInterface->veInterface);
            CODECHAL_ENCODE_ASSERT(pVEInterface->veInterface->GetEngineCount() == 1);
            gpuCtxCreatOpts->EngineInstance[0] = pVEInterface->veInterface->GetEngineLogicId(0);
        }
        else
        {
            CODECHAL_ENCODE_ASSERT(pVEInterface->ucEngineCount == 1);
            gpuCtxCreatOpts->EngineInstance[0] = pVEInterface->EngineLogicId[0];
        }
    }
#endif

    return eStatus;
}

MOS_STATUS CodecHalEncodeSinglePipeVE_InitInterface(
    CodechalHwInterface                            *pHwInterface,
    PCODECHAL_ENCODE_SINGLEPIPE_VIRTUALENGINE_STATE pVEState)
{
    PMOS_INTERFACE                 pOsInterface;
    PMOS_VIRTUALENGINE_INTERFACE   pVEInterface;
    MOS_VIRTUALENGINE_INIT_PARAMS  VEInitParams;
    MOS_STATUS                     eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(pHwInterface);
    CODECHAL_ENCODE_CHK_NULL_RETURN(pHwInterface->GetOsInterface());
    CODECHAL_ENCODE_CHK_NULL_RETURN(pVEState);
    pOsInterface = pHwInterface->GetOsInterface();
    CODECHAL_ENCODE_CHK_NULL_RETURN(pOsInterface);

    //virtual engine init with singlepipe
    MOS_ZeroMemory(&VEInitParams, sizeof(VEInitParams));
    VEInitParams.bScalabilitySupported = false;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(Mos_VirtualEngineInterface_Initialize(pOsInterface, &VEInitParams));
    pVEState->pVEInterface = pVEInterface = pOsInterface->pVEInterf;

    if(pVEInterface->pfnVEGetHintParams)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(pVEInterface->pfnVEGetHintParams(pVEInterface, false, &pVEState->pHintParms));
    }
    return eStatus;
}

