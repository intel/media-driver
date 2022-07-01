/*
* Copyright (c) 2020-2021, Intel Corporation
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
#include <typeinfo>
#include <iostream>
#include "mos_os.h"
#include "mos_os_virtualengine_scalability.h"
#include "media_scalability_defs.h"
#include "media_scalability_singlepipe.h"
#include "mhw_mi.h"
#include "hal_oca_interface.h"
#include "mhw_mi_itf.h"
#include "mhw_mi_cmdpar.h"
#include "media_packet.h"

MediaScalabilitySinglePipe::MediaScalabilitySinglePipe(void *hwInterface, MediaContext *mediaContext, uint8_t componentType) :
    MediaScalabilitySinglePipeNext(hwInterface, mediaContext, componentType)
{
}

MOS_STATUS MediaScalabilitySinglePipe::Initialize(const MediaScalabilityOption &option)
{
    SCALABILITY_CHK_NULL_RETURN(m_osInterface);

    MediaScalabilitySinglePipeNext::Initialize(option);

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
