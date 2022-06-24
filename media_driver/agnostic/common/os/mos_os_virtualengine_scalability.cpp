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
//! \file     mos_os_virtualengine_scalability.c
//! \brief    Implements the MOS interface extension cross OS,  supporting virtual engine scalability mode.
//! \details  Implements the MOS interface extension cross OS,  supporting virtual engine scalability mode. Only necessary when KMD virtual engine is supported.
//!
#include "mos_os.h"
#include "mos_os_virtualengine.h"

//==<Functions>=======================================================
#if (_DEBUG || _RELEASE_INTERNAL)
MOS_STATUS Mos_VirtualEngine_Scalability_PopulateDbgOvrdParams(
    PMOS_VIRTUALENGINE_INTERFACE pVEInterface)
{
    PMOS_INTERFACE          pOsInterface = nullptr;
    uint8_t                 ucMaxEngineCnt = 0;
    uint8_t                 ui8EngineId = 0;
    int32_t                 iForceEngine = 0;
    MOS_STATUS              eStatus = MOS_STATUS_UNKNOWN;
    MOS_OS_FUNCTION_ENTER;

    MOS_OS_CHK_NULL(pVEInterface);
    MOS_OS_CHK_NULL(pVEInterface->pOsInterface);

    pOsInterface = pVEInterface->pOsInterface;

    iForceEngine = MOS_INVALID_FORCEENGINE_VALUE;

    switch (pOsInterface->Component)
    {
        case COMPONENT_Decode:
            iForceEngine   = pOsInterface->eForceVdbox;
            ucMaxEngineCnt = pVEInterface->ucMaxNumPipesInUse + 1;
            break;
        case COMPONENT_VPCommon:
            iForceEngine   = pOsInterface->eForceVebox;
            ucMaxEngineCnt = pVEInterface->ucMaxNumPipesInUse;
            break;
        case COMPONENT_Encode:
            iForceEngine   = pOsInterface->eForceVdbox;
            ucMaxEngineCnt = pVEInterface->ucMaxNumPipesInUse;
            break;
        default:
            eStatus = MOS_STATUS_INVALID_PARAMETER;
            MOS_OS_ASSERTMESSAGE("Not supported MOS Component.")
            goto finish;
    }

    MOS_ZeroMemory(pVEInterface->EngineLogicId, sizeof(pVEInterface->EngineLogicId));
    pVEInterface->ucEngineCount = 0;
    if (iForceEngine == 0)
    {
        pVEInterface->EngineLogicId[0] = 0;
        pVEInterface->ucEngineCount    = 1;
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
                    goto finish;
            }
            if (pVEInterface->ucEngineCount >= MOS_MAX_ENGINE_INSTANCE_PER_CLASS)
            {
                eStatus = MOS_STATUS_INVALID_PARAMETER;
                MOS_OS_ASSERTMESSAGE("number of engine exceeds the max value.");
                goto finish;
            }
            pVEInterface->EngineLogicId[pVEInterface->ucEngineCount] = ui8EngineId;
            iForceEngine >>= MOS_FORCEENGINE_ENGINEID_BITSNUM;
            pVEInterface->ucEngineCount++;
        }
    }

    if (pVEInterface->ucEngineCount == 0)
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        MOS_OS_ASSERTMESSAGE("number of engine specified can not be zero.");
        goto finish;
    }

    if (pVEInterface->ucEngineCount > ucMaxEngineCnt)
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        MOS_OS_ASSERTMESSAGE("number of engine specified exceeds HW engine number.");
        goto finish;
    }

    eStatus = MOS_STATUS_SUCCESS;

finish:
    return eStatus;
}
#endif //(_DEBUG || _RELEASE_INTERNAL)

