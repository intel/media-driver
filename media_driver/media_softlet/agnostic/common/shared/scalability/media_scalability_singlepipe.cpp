/*
* Copyright (c) 2020-2022, Intel Corporation
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
//! \file     media_scalability_singlepipe.cpp
//! \brief    Defines the common interface for media scalability singlepipe mode.
//! \details  The media scalability singlepipe interface is further sub-divided by component,
//!           this file is for the base interface which is shared by all components.
//!

#include <memory>
#include "mos_os.h"
#include "media_scalability_defs.h"
#include "media_scalability_singlepipe.h"
#include "mos_os_virtualengine.h"
#include "media_scalability_option.h"
#include "mhw_mi.h"
#include "media_packet.h"

class MediaContext;
class MediaScalabilityOption;

MediaScalabilitySinglePipe::MediaScalabilitySinglePipe(void *hwInterface, MediaContext *mediaContext, uint8_t componentType) :
    MediaScalabilitySinglePipeNext(hwInterface, mediaContext, componentType)
{
}

MOS_STATUS MediaScalabilitySinglePipe::Initialize(const MediaScalabilityOption &option)
{
    SCALABILITY_CHK_NULL_RETURN(m_osInterface);

#if !EMUL
    if (MOS_VE_SUPPORTED(m_osInterface))
    {
        MOS_VIRTUALENGINE_INIT_PARAMS VEInitParams;
        MOS_ZeroMemory(&VEInitParams, sizeof(VEInitParams));
        VEInitParams.bScalabilitySupported = false;

        SCALABILITY_CHK_STATUS_RETURN(m_osInterface->pfnVirtualEngineInit(m_osInterface, &m_veHitParams, VEInitParams));
        if (m_osInterface->apoMosEnabled)
        {
            SCALABILITY_CHK_NULL_RETURN(m_osInterface->osStreamState);
            m_veState = m_osInterface->osStreamState->virtualEngineInterface;
            SCALABILITY_CHK_NULL_RETURN(m_veState);
            SCALABILITY_CHK_NULL_RETURN(m_veHitParams);
        }
        else
        {
            m_veInterface = m_osInterface->pVEInterf;
            SCALABILITY_CHK_NULL_RETURN(m_veInterface);
            if (m_veInterface->pfnVEGetHintParams != nullptr)
            {
                SCALABILITY_CHK_NULL_RETURN(m_veHitParams);
            }
        }
    }
#endif
    PMOS_GPUCTX_CREATOPTIONS_ENHANCED gpuCtxCreateOption = MOS_New(MOS_GPUCTX_CREATOPTIONS_ENHANCED);
    SCALABILITY_CHK_NULL_RETURN(gpuCtxCreateOption);

    gpuCtxCreateOption->RAMode      = option.GetRAMode();
    gpuCtxCreateOption->ProtectMode = option.GetProtectMode();
    gpuCtxCreateOption->LRCACount   = 1;
    // This setting is only for encode, please override it in decode/vpp
    gpuCtxCreateOption->UsingSFC = false;
#if (_DEBUG || _RELEASE_INTERNAL) && !EMUL
    if (m_osInterface->bEnableDbgOvrdInVE)
    {
        gpuCtxCreateOption->DebugOverride = true;
        if (m_osInterface->apoMosEnabled)
        {
            SCALABILITY_ASSERT(m_osInterface->pfnGetVeEngineCount(m_osInterface->osStreamState) == 1);
            gpuCtxCreateOption->EngineInstance[0] = m_osInterface->pfnGetEngineLogicIdByIdx(m_osInterface->osStreamState, 0);
        }
        else
        {
            SCALABILITY_ASSERT(m_veInterface->ucEngineCount == 1);
            gpuCtxCreateOption->EngineInstance[0] = m_veInterface->EngineLogicId[0];
        }
    }
#endif
    m_gpuCtxCreateOption = (PMOS_GPUCTX_CREATOPTIONS)gpuCtxCreateOption;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MediaScalabilitySinglePipe::SubmitCmdBuffer(PMOS_COMMAND_BUFFER cmdBuffer)
{
    SCALABILITY_FUNCTION_ENTER;
    SCALABILITY_CHK_NULL_RETURN(m_osInterface);
    SCALABILITY_CHK_NULL_RETURN(cmdBuffer);

    SCALABILITY_CHK_STATUS_RETURN(GetCmdBuffer(cmdBuffer));

    if (!m_osInterface->pfnIsMismatchOrderProgrammingSupported())
    {
        if (m_miItf)
        {
            SCALABILITY_CHK_STATUS_RETURN(m_miItf->AddMiBatchBufferEnd(cmdBuffer, nullptr));
        }
        else
        {
            SCALABILITY_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferEnd(cmdBuffer, nullptr));
        }
    }

    SCALABILITY_CHK_STATUS_RETURN(Oca1stLevelBBEnd(*cmdBuffer));

    SCALABILITY_CHK_STATUS_RETURN(ReturnCmdBuffer(cmdBuffer));

    if (MOS_VE_SUPPORTED(m_osInterface))
    {
        SCALABILITY_CHK_STATUS_RETURN(SetHintParams());
        if(cmdBuffer && m_veHitParams)
        {
            SCALABILITY_CHK_STATUS_RETURN(PopulateHintParams(cmdBuffer));
        }
    }

    m_attrReady = false;
    return m_osInterface->pfnSubmitCommandBuffer(m_osInterface, cmdBuffer, false);
}

MOS_STATUS MediaScalabilitySinglePipe::SetHintParams()
{
    SCALABILITY_FUNCTION_ENTER;

    SCALABILITY_CHK_NULL_RETURN(m_osInterface);
    if (m_osInterface->apoMosEnabled)
    {
        SCALABILITY_CHK_NULL_RETURN(m_osInterface->osStreamState);
    }
    else
    {
        SCALABILITY_CHK_NULL_RETURN(m_veInterface);
    }

    MOS_STATUS                   eStatus = MOS_STATUS_SUCCESS;
    MOS_VIRTUALENGINE_SET_PARAMS veParams;
    MOS_ZeroMemory(&veParams, sizeof(veParams));

    veParams.ucScalablePipeNum = 1;
    veParams.bScalableMode     = false;

    if (!MOS_VE_CTXBASEDSCHEDULING_SUPPORTED(m_osInterface))
    {
        //not used by VE2.0
        veParams.bNeedSyncWithPrevious       = true;
        veParams.bSameEngineAsLastSubmission = false;
        veParams.bSFCInUse                   = false;
    }

    SCALABILITY_CHK_STATUS_RETURN(m_osInterface->pfnSetHintParams(m_osInterface, &veParams));

    return eStatus;
}

MOS_STATUS MediaScalabilitySinglePipe::Destroy()
{
    SCALABILITY_FUNCTION_ENTER;

    SCALABILITY_CHK_STATUS_RETURN(MediaScalabilitySinglePipeNext::Destroy());

    if (m_veInterface)
    {
        if (m_veInterface->pfnVEDestroy)
        {
            m_veInterface->pfnVEDestroy(m_veInterface);
        }
        MOS_FreeMemAndSetNull(m_veInterface);
    }
    else
    {
        if (!m_osInterface->apoMosEnabled)
        {
            // For VE not enabled/supported case, such as vp vebox on some platform, m_veInterface is nullptr.
            // MOS_STATUS_SUCCESS should be returned for such case.
            if (MOS_VE_SUPPORTED(m_osInterface))
            {
                SCALABILITY_CHK_NULL_RETURN(m_veInterface);
            }
        }
    }

    return MOS_STATUS_SUCCESS;
}
