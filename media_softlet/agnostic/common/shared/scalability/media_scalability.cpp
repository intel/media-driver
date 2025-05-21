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
//! \file     media_scalability.cpp
//! \brief    Defines the common interface for media scalability
//! \details  The media scalability interface is further sub-divided by component,
//!           this file is for the base interface which is shared by all components.
//!

#include "media_scalability_defs.h"
#include "media_scalability_option.h"
#include "media_scalability.h"
#include "mos_interface.h"
#include "mos_utilities.h"


MediaScalability::MediaScalability(MediaContext *mediaContext) :
    m_mediaContext(mediaContext) 
{
    if (m_mediaContext == nullptr)
    {
        SCALABILITY_ASSERTMESSAGE("mediaContext is null ptr! Construct MediaScalability failed!");
    }
}

bool MediaScalability::IsScalabilityModeMatched(ScalabilityPars *params)
{
    return m_scalabilityOption->IsScalabilityOptionMatched(params);
}

bool MediaScalability::IsScalabilityModeMatched(MediaScalabilityOption &scalabOption)
{

    return m_scalabilityOption->IsScalabilityOptionMatched(scalabOption);
}

bool MediaScalability::IsGpuCtxCreateOptionMatched(PMOS_GPUCTX_CREATOPTIONS_ENHANCED gpuCtxCreateOption1, PMOS_GPUCTX_CREATOPTIONS_ENHANCED gpuCtxCreateOption2)
{
    bool isMatched = false;
    //Current only need new GpuCtxCreateOption when LRCACount changed.
    //It can be improved if needed.
    if (gpuCtxCreateOption1->LRCACount == gpuCtxCreateOption2->LRCACount)
    {
        isMatched = true;
    }
    return isMatched;
}

MOS_STATUS MediaScalability::VerifySpaceAvailable(uint32_t requestedSize, uint32_t requestedPatchListSize, bool &singleTaskPhaseSupportedInPak)
{
    SCALABILITY_FUNCTION_ENTER;
    SCALABILITY_CHK_NULL_RETURN(m_osInterface);

    MOS_STATUS eStatus         = MOS_STATUS_SUCCESS;
    MOS_STATUS statusPatchList = MOS_STATUS_SUCCESS;
    MOS_STATUS statusCmdBuf    = MOS_STATUS_SUCCESS;
    // Try a maximum of 3 attempts to request the required sizes from OS
    // OS could reset the sizes if necessary, therefore, requires to re-verify
    for (auto i = 0; i < 3; i++)
    {
        //Experiment shows resizing CmdBuf size and PatchList size in two calls one after the other would cause previously
        //successfully requested size to fallback to wrong value, hence never satisfying the requirement. So we call pfnResize()
        //only once depending on whether CmdBuf size not enough, or PatchList size not enough, or both.
        if (requestedPatchListSize)
        {
            statusPatchList = (MOS_STATUS)m_osInterface->pfnVerifyPatchListSize(
                m_osInterface,
                requestedPatchListSize);
        }
        if (m_osInterface->pfnVerifyCommandBufferSize)
        {
            statusCmdBuf = (MOS_STATUS)m_osInterface->pfnVerifyCommandBufferSize(
                m_osInterface,
                requestedSize,
                0);
        }
        else
        {
            statusCmdBuf = MOS_STATUS_SUCCESS;
        }

        if (statusPatchList != MOS_STATUS_SUCCESS && statusCmdBuf != MOS_STATUS_SUCCESS)
        {
            SCALABILITY_CHK_STATUS_RETURN(ResizeCommandBufferAndPatchList(requestedSize + COMMAND_BUFFER_RESERVED_SPACE, requestedPatchListSize));
        }
        else if (statusPatchList != MOS_STATUS_SUCCESS)
        {
            SCALABILITY_CHK_STATUS_RETURN(ResizeCommandBufferAndPatchList(0, requestedPatchListSize));
        }
        else if (statusCmdBuf != MOS_STATUS_SUCCESS)
        {
            SCALABILITY_CHK_STATUS_RETURN(ResizeCommandBufferAndPatchList(requestedSize + COMMAND_BUFFER_RESERVED_SPACE, 0));
        }
        else
        {
            // This flag is just a hint for encode, decode/vpp don't use this flag.
            singleTaskPhaseSupportedInPak = true;
            return eStatus;
        }

    }

    if (requestedPatchListSize)
    {
        statusPatchList = (MOS_STATUS)m_osInterface->pfnVerifyPatchListSize(
            m_osInterface,
            requestedPatchListSize);
    }
    statusCmdBuf = (MOS_STATUS)m_osInterface->pfnVerifyCommandBufferSize(
        m_osInterface,
        requestedSize,
        0);

    if(statusPatchList != MOS_STATUS_SUCCESS || statusCmdBuf != MOS_STATUS_SUCCESS)
    {
        eStatus = MOS_STATUS_NO_SPACE;
    }

    return eStatus;
}

MOS_STATUS MediaScalability::Destroy()
{
    if (m_osInterface->apoMosEnabled || m_osInterface->apoMosForLegacyRuntime)
    {
        if (m_veState)
        {
            SCALABILITY_CHK_NULL_RETURN(m_osInterface->osStreamState);
            m_osInterface->osStreamState->virtualEngineInterface = m_veState;
            return m_osInterface->pfnDestroyVirtualEngineState(m_osInterface->osStreamState);
        }

        // No VE state to destroy in some scalability instances
        return MOS_STATUS_SUCCESS;
    }

    return MOS_STATUS_SUCCESS;
}
