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
//! \file     mos_os_virtualengine_singlepipe.cpp
//! \brief    Implements the MOS interface extension for Cross OS, supporting virtual engine single pipe mode.
//! \details  Implements the MOS interface extension for Cross OS, supporting virtual engine single pipe mode. Only necessary when KMD supports virtual engine.
//!

#include "mos_util_debug.h"
#include "mos_os_virtualengine.h"
#include "mos_os_virtualengine_next.h"

//==<Functions>=======================================================
void Mos_VirtualEngine_SinglePipe_Destroy(
    PMOS_VIRTUALENGINE_INTERFACE    pVEInterface)
{
    MOS_OS_FUNCTION_ENTER;

    if (g_apoMosEnabled && pVEInterface->veInterface)
    {
        pVEInterface->veInterface->Destroy();
        MOS_Delete(pVEInterface->veInterface);
    }

    return;
}

#if (_DEBUG || _RELEASE_INTERNAL)
MOS_STATUS Mos_VirtualEngine_SinglePipe_PopulateDbgOvrdParams(
    PMOS_VIRTUALENGINE_INTERFACE pVEInterface)
{
    PMOS_INTERFACE          pOsInterface = nullptr;
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
            iForceEngine = pOsInterface->eForceVdbox;
            break;
        case COMPONENT_VPCommon:
            iForceEngine = pOsInterface->eForceVebox;
            break;
        case COMPONENT_Encode:
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
    }
    else
    {
        uint8_t ui8EngineId = 0;
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
        pVEInterface->EngineLogicId[0] = ui8EngineId;
    }

    pVEInterface->ucEngineCount  = 1;
    eStatus = MOS_STATUS_SUCCESS;
finish:
    return eStatus;
}
#endif //(_DEBUG || _RELEASE_INTERNAL)

