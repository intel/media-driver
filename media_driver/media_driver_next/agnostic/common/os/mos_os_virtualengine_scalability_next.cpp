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
//! \file     mos_os_virtualengine_scalability_next.c
//! \brief    Implements the MOS interface extension cross OS,  supporting virtual engine scalability mode.
//! \details  Implements the MOS interface extension cross OS,  supporting virtual engine scalability mode. Only necessary when KMD virtual engine is supported.
//!
#include "mos_util_debug_next.h"
#include "mos_os_virtualengine_scalability_next.h"

#if (_DEBUG || _RELEASE_INTERNAL)
MOS_STATUS MosOsVeScalability::PopulateDbgOvrdParams(
    MOS_STREAM_HANDLE stream)
{
    MOS_STATUS              eStatus = MOS_STATUS_UNKNOWN;

    MOS_OS_FUNCTION_ENTER;
    MOS_OS_CHK_NULL_RETURN(stream);

    uint8_t        ucMaxEngineCnt = 0;
    uint8_t        ui8EngineId    = 0;
    int32_t        iForceEngine   = 0;

    iForceEngine = MOS_INVALID_FORCEENGINE_VALUE;

    switch (stream->component)
    {
        case COMPONENT_Decode:
            iForceEngine   = stream->eForceVdbox;
            ucMaxEngineCnt = ucMaxNumPipesInUse + 1;
            break;
        case COMPONENT_VPCommon:
            iForceEngine   = stream->eForceVebox;
            ucMaxEngineCnt = ucMaxNumPipesInUse;
            break;
        case COMPONENT_Encode:
            iForceEngine   = stream->eForceVdbox;
            ucMaxEngineCnt = ucMaxNumPipesInUse;
            break;
        default:
            eStatus = MOS_STATUS_INVALID_PARAMETER;
            MOS_OS_ASSERTMESSAGE("Not supported MOS Component.")
            return eStatus;
    }

    MosUtilities::MosZeroMemory(EngineLogicId, sizeof(EngineLogicId));
    ucEngineCount = 0;
    if (iForceEngine == 0)
    {
        EngineLogicId[0] = 0;
        ucEngineCount    = 1;
    }
    else
    {
        while (iForceEngine != 0)
        {
            switch (iForceEngine & MOS_FORCEENGINE_MASK)
            {
                case 1:
                    ui8EngineId = 0;
                    break;
                case 2:
                    ui8EngineId = 1;
                    break;
                case 3:
                    ui8EngineId = 2;
                    break;
                case 4:
                    ui8EngineId = 3;
                    break;
                default:
                    eStatus = MOS_STATUS_INVALID_PARAMETER;
                    MOS_OS_ASSERTMESSAGE("Invalid force engine value.");
                    return eStatus;
            }
            if (ucEngineCount >= MOS_MAX_ENGINE_INSTANCE_PER_CLASS)
            {
                eStatus = MOS_STATUS_INVALID_PARAMETER;
                MOS_OS_ASSERTMESSAGE("number of engine exceeds the max value.");
                return eStatus;
            }
            EngineLogicId[ucEngineCount] = ui8EngineId;
            iForceEngine >>= MOS_FORCEENGINE_ENGINEID_BITSNUM;
            ucEngineCount++;
        }
    }

    if (ucEngineCount == 0)
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        MOS_OS_ASSERTMESSAGE("number of engine specified can not be zero.");
        return eStatus;
    }

    if (ucEngineCount > ucMaxEngineCnt)
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        MOS_OS_ASSERTMESSAGE("number of engine specified exceeds HW engine number.");
        return eStatus;
    }

    eStatus = MOS_STATUS_SUCCESS;

    return eStatus;
}
#endif //(_DEBUG || _RELEASE_INTERNAL)

