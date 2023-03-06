/*
* Copyright (c) 2021, Intel Corporation
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
//! \file     media_scalability_singlepipe_next.cpp
//! \brief    Defines the common interface for media scalability singlepipe mode.
//! \details  The media scalability singlepipe interface is further sub-divided by component,
//!           this file is for the base interface which is shared by all components.
//!

#include <stddef.h>
#include <memory>
#include "media_scalability_option.h"
#include "mos_os_virtualengine_next.h"
#include "mos_interface.h"
#include "mos_os_virtualengine_specific.h"
#include "mos_utilities.h"
#include <typeinfo>
#include "mos_os.h"
#include "media_scalability_defs.h"
#include "media_scalability_singlepipe_next.h"
#include "mhw_mi_itf.h"
class MediaContext;

MediaScalabilitySinglePipeNext::MediaScalabilitySinglePipeNext(void *hwInterface, MediaContext *mediaContext, uint8_t componentType) :
    MediaScalability(mediaContext)
{
    m_componentType = componentType;
}

MOS_STATUS MediaScalabilitySinglePipeNext::Initialize(const MediaScalabilityOption &option)
{
    SCALABILITY_CHK_NULL_RETURN(m_osInterface);
#if !EMUL
    if (MOS_VE_SUPPORTED(m_osInterface))
    {
        MOS_VIRTUALENGINE_INIT_PARAMS veInitParms;
        MOS_ZeroMemory(&veInitParms, sizeof(veInitParms));
        veInitParms.bScalabilitySupported = false;
        MOS_STATUS status                 = m_osInterface->pfnVirtualEngineInit(m_osInterface, &m_veHitParams, veInitParms);
        SCALABILITY_CHK_STATUS_MESSAGE_RETURN(status, "Virtual Engine Init failed");
        m_veInterface = m_osInterface->pVEInterf;
        if (m_osInterface->osStreamState && m_osInterface->osStreamState->virtualEngineInterface)
        {
            // we set m_veState here when pOsInterface->apoMosEnabled is true
            m_veState = m_osInterface->osStreamState->virtualEngineInterface;
        }
    }
#endif

    PMOS_GPUCTX_CREATOPTIONS_ENHANCED gpuCtxCreateOption = MOS_New(MOS_GPUCTX_CREATOPTIONS_ENHANCED);
    SCALABILITY_CHK_NULL_RETURN(gpuCtxCreateOption);

    gpuCtxCreateOption->RAMode = option.GetRAMode();
    gpuCtxCreateOption->ProtectMode = option.GetProtectMode();
    gpuCtxCreateOption->LRCACount = 1;
    // This setting is only for encode, please override it in decode/vpp
    gpuCtxCreateOption->UsingSFC = false;
#if (_DEBUG || _RELEASE_INTERNAL) && !EMUL
    if (m_osInterface->bEnableDbgOvrdInVE)
    {
        gpuCtxCreateOption->DebugOverride = true;
        uint8_t engineLogicId             = 0;
        if (m_osInterface->pfnGetEngineLogicId(m_osInterface, engineLogicId) == MOS_STATUS_SUCCESS)
        {
            gpuCtxCreateOption->EngineInstance[0] = engineLogicId;
        }
    }
#endif
    m_gpuCtxCreateOption = (PMOS_GPUCTX_CREATOPTIONS)gpuCtxCreateOption;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MediaScalabilitySinglePipeNext::Destroy()
{
    SCALABILITY_FUNCTION_ENTER;

    SCALABILITY_CHK_STATUS_RETURN(MediaScalability::Destroy());
    SCALABILITY_CHK_NULL_RETURN(m_osInterface);

    if (m_gpuCtxCreateOption != nullptr)
    {
        MOS_Delete(m_gpuCtxCreateOption);
    }

    if (m_scalabilityOption != nullptr)
    {
        MOS_Delete(m_scalabilityOption);
    }
#if !EMUL
    m_osInterface->pfnDestroyVeInterface(&m_veInterface);
#endif
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MediaScalabilitySinglePipeNext::GetGpuCtxCreationOption(MOS_GPUCTX_CREATOPTIONS *gpuCtxCreateOption)
{
    SCALABILITY_FUNCTION_ENTER;
    SCALABILITY_CHK_NULL_RETURN(gpuCtxCreateOption);
    SCALABILITY_CHK_NULL_RETURN(m_gpuCtxCreateOption);

    size_t size = sizeof(MOS_GPUCTX_CREATOPTIONS);

    if (typeid(*gpuCtxCreateOption) == typeid(MOS_GPUCTX_CREATOPTIONS_ENHANCED))
    {
        size = sizeof(MOS_GPUCTX_CREATOPTIONS_ENHANCED);
    }

    SCALABILITY_CHK_STATUS_MESSAGE_RETURN(MOS_SecureMemcpy(
                                              (void *)gpuCtxCreateOption,
                                              size,
                                              (void *)m_gpuCtxCreateOption,
                                              size),
        "Failed to copy gpu ctx create option");

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MediaScalabilitySinglePipeNext::VerifyCmdBuffer(uint32_t requestedSize, uint32_t requestedPatchListSize, bool &singleTaskPhaseSupportedInPak)
{
    // legacy mode & resize CommandBuffer Size for every BRC pass
    return VerifySpaceAvailable(requestedSize, requestedPatchListSize, singleTaskPhaseSupportedInPak);
}

MOS_STATUS MediaScalabilitySinglePipeNext::GetCmdBuffer(PMOS_COMMAND_BUFFER cmdBuffer, bool frameTrackingRequested)
{
    SCALABILITY_CHK_NULL_RETURN(m_osInterface);
    SCALABILITY_CHK_NULL_RETURN(cmdBuffer);
    SCALABILITY_CHK_STATUS_RETURN(m_osInterface->pfnGetCommandBuffer(m_osInterface, cmdBuffer, 0));

    if (!m_attrReady)
    {
        SCALABILITY_CHK_STATUS_RETURN(SendAttrWithFrameTracking(*cmdBuffer, frameTrackingRequested));
        m_attrReady = true;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MediaScalabilitySinglePipeNext::ReturnCmdBuffer(PMOS_COMMAND_BUFFER cmdBuffer)
{
    SCALABILITY_CHK_NULL_RETURN(m_osInterface);
    SCALABILITY_CHK_NULL_RETURN(cmdBuffer);

    m_osInterface->pfnReturnCommandBuffer(m_osInterface, cmdBuffer, 0);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MediaScalabilitySinglePipeNext::SetHintParams()
{
    SCALABILITY_FUNCTION_ENTER;

    SCALABILITY_CHK_NULL_RETURN(m_osInterface);

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
    m_osInterface->pVEInterf = m_veInterface;
#if !EMUL
    eStatus = m_osInterface->pfnSetHintParams(m_osInterface, &veParams);
#endif
    SCALABILITY_CHK_STATUS_MESSAGE_RETURN(eStatus, "SetHintParams failed");

    return eStatus;
}

MOS_STATUS MediaScalabilitySinglePipeNext::PopulateHintParams(PMOS_COMMAND_BUFFER cmdBuffer)
{
    SCALABILITY_FUNCTION_ENTER;
    SCALABILITY_CHK_NULL_RETURN(cmdBuffer);
    SCALABILITY_CHK_NULL_RETURN(m_veHitParams);
    SCALABILITY_CHK_NULL_RETURN(m_osInterface);

    MOS_STATUS            eStatus  = MOS_STATUS_SUCCESS;
    PMOS_CMD_BUF_ATTRI_VE attriVe  = m_osInterface->pfnGetAttributeVeBuffer(cmdBuffer);
    if (attriVe)
    {
        attriVe->VEngineHintParams     = *(m_veHitParams);
        attriVe->bUseVirtualEngineHint = true;
    }
    return eStatus;
}

MOS_STATUS MediaScalabilitySinglePipeNext::SubmitCmdBuffer(PMOS_COMMAND_BUFFER cmdBuffer)
{
    SCALABILITY_FUNCTION_ENTER;
    SCALABILITY_CHK_NULL_RETURN(m_osInterface);
    SCALABILITY_CHK_NULL_RETURN(cmdBuffer);

    SCALABILITY_CHK_STATUS_RETURN(GetCmdBuffer(cmdBuffer));

    if (!m_osInterface->pfnIsMismatchOrderProgrammingSupported())
    {
        SCALABILITY_CHK_STATUS_RETURN(m_miItf->AddMiBatchBufferEnd(cmdBuffer, nullptr));
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
