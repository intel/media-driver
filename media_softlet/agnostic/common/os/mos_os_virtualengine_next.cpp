/*
* Copyright (c) 2019, Intel Corporation
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
//! \file     mos_os_virtualengine_next.cpp
//! \brief    Implements the MOS interface extension cross OS - virtual engine.
//! \details  Implements the MOS interface extension cross OS - virtual engine. Only necessary when KMD virtual engine is supported
//!

#include "mos_util_debug_next.h"
#include "mos_os_virtualengine_singlepipe_specific_next.h"
#include "mos_os_virtualengine_scalability_specific_next.h"

MOS_STATUS MosVeInterface::IsScalabilitySupported(
    bool                          *pbScalabilitySupported)
{
    MOS_STATUS                eStatus = MOS_STATUS_SUCCESS;

    MOS_OS_FUNCTION_ENTER;

    MOS_OS_CHK_NULL_RETURN(pbScalabilitySupported);

    *pbScalabilitySupported = bScalabilitySupported;

    return eStatus;
}

MOS_STATUS MosVeInterface::Initialize(
    MOS_STREAM_HANDLE                 stream,
    PMOS_VIRTUALENGINE_INIT_PARAMS    pVEInitParms)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MOS_OS_CHK_NULL_RETURN(pVEInitParms);
    MOS_OS_CHK_NULL_RETURN(stream);

    m_stream = stream;
    m_contextBasedScheduling = stream->ctxBasedScheduling;
    bScalabilitySupported   = pVEInitParms->bScalabilitySupported;
    ucMaxNumPipesInUse      = pVEInitParms->ucMaxNumPipesInUse;
#if _DEBUG || _RELEASE_INTERNAL
    m_enableDbgOvrdInVirtualEngine = stream->enableDbgOvrdInVirtualEngine;
#endif  // _DEBUG || _RELEASE_INTERNAL

    if (bScalabilitySupported &&
        (ucMaxNumPipesInUse > MOS_MAX_ENGINE_INSTANCE_PER_CLASS ||
         ucMaxNumPipesInUse  == 0))
    {
        MOS_OS_ASSERTMESSAGE("invalid max pipe number in use for scalability");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    return eStatus;
}
