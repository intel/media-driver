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
//! \file     mos_os_virtualengine.cpp
//! \brief    Implements the MOS interface extension cross OS - virtual engine.
//! \details  Implements the MOS interface extension cross OS - virtual engine. Only necessary when KMD virtual engine is supported
//!

#include "mos_util_debug.h"
#include "mos_os_virtualengine_singlepipe_specific.h"
#include "mos_os_virtualengine_scalability_specific.h"
#include "mos_os_virtualengine_singlepipe_specific_next.h"
#include "mos_os_virtualengine_scalability_specific_next.h"

inline MOS_STATUS Mos_VirtualEngine_IsScalabilitySupported(
    PMOS_VIRTUALENGINE_INTERFACE   pVEInterface,
    bool                          *pbScalabilitySupported)
{
    MOS_STATUS                eStatus = MOS_STATUS_SUCCESS;

    MOS_OS_FUNCTION_ENTER;

    MOS_OS_CHK_NULL(pVEInterface);
    MOS_OS_CHK_NULL(pbScalabilitySupported);

    *pbScalabilitySupported = pVEInterface->bScalabilitySupported;

finish:
    return eStatus;
}

MOS_STATUS Mos_VirtualEngineInterface_Initialize(
    PMOS_INTERFACE                    pOsInterface,
    PMOS_VIRTUALENGINE_INIT_PARAMS    pVEInitParms)
{
    PMOS_VIRTUALENGINE_INTERFACE  pVEInterf = nullptr;
    uint32_t                      i = 0;
    MOS_STATUS                    eStatus = MOS_STATUS_SUCCESS;

    MOS_OS_CHK_NULL(pOsInterface);
    MOS_OS_CHK_NULL(pVEInitParms);

    if (!MOS_VE_SUPPORTED(pOsInterface))
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        MOS_OS_ASSERTMESSAGE("virtual engine is not supported.\n");
        goto finish;
    }

    pVEInterf = (PMOS_VIRTUALENGINE_INTERFACE)MOS_AllocAndZeroMemory(sizeof(MOS_VIRTUALENGINE_INTERFACE));
    MOS_OS_CHK_NULL(pVEInterf);

    pVEInterf->pOsInterface            = pOsInterface;
    pVEInterf->bScalabilitySupported   = pVEInitParms->bScalabilitySupported;
    pVEInterf->ucMaxNumPipesInUse      = pVEInitParms->ucMaxNumPipesInUse;

    if (pVEInterf->bScalabilitySupported &&
        (pVEInterf->ucMaxNumPipesInUse > MOS_MAX_ENGINE_INSTANCE_PER_CLASS ||
         pVEInterf->ucMaxNumPipesInUse  == 0))
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        MOS_OS_ASSERTMESSAGE("invalid max pipe number in use for scalability");
        goto finish;
    }

    pVEInterf->pfnVEIsScalabilitySupported = Mos_VirtualEngine_IsScalabilitySupported;

    pOsInterface->pVEInterf = pVEInterf;

    if (pVEInitParms->bScalabilitySupported)
    {
        MOS_OS_CHK_STATUS(Mos_Specific_VirtualEngine_Scalability_Initialize(pVEInterf, pVEInitParms));
    }
    else
    {
        MOS_OS_CHK_STATUS(Mos_Specific_VirtualEngine_SinglePipe_Initialize(pVEInterf, pVEInitParms));
    }

    if (g_apoMosEnabled)
    {
        if (pVEInitParms->bScalabilitySupported)
        {
            pVEInterf->veInterface = MOS_New(MosOsVeScalabilitySpecific);
        }
        else
        {
            pVEInterf->veInterface = MOS_New(MosOsVeSinglePipeSpecific);
        }
        MOS_OS_CHK_NULL(pVEInterf->veInterface);
        MOS_OS_CHK_NULL(pOsInterface->osStreamState);
        pVEInterf->veInterface->Initialize(pOsInterface->osStreamState, pVEInitParms);
        pOsInterface->osStreamState->virtualEngineInterface = pVEInterf->veInterface;
    }

finish:
    return eStatus;
}

