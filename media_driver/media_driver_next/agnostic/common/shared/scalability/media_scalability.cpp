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

#include "media_scalability.h"
#include "mos_os_virtualengine.h"

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
    bool isMatched = false;


#if (_DEBUG || _RELEASE_INTERNAL)

    if (m_osInterface == nullptr)
    {
        return false;
    }
    if (m_osInterface->bEnableDbgOvrdInVE)
    {
        isMatched = true;
    }
    else
#endif
    {
        isMatched = m_scalabilityOption->IsScalabilityOptionMatched(params);
    }

    return isMatched;
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
        statusCmdBuf = (MOS_STATUS)m_osInterface->pfnVerifyCommandBufferSize(
            m_osInterface,
            requestedSize,
            0);

        if (statusPatchList == MOS_STATUS_SUCCESS && statusCmdBuf == MOS_STATUS_SUCCESS)
        {
            // This flag is just a hint for encode, decode/vpp don't use this flag.
            singleTaskPhaseSupportedInPak = true;
            return eStatus;
        }

        requestedSize          = (statusCmdBuf != MOS_STATUS_SUCCESS) ? requestedSize + COMMAND_BUFFER_RESERVED_SPACE : 0;
        requestedPatchListSize = (statusPatchList != MOS_STATUS_SUCCESS) ? requestedPatchListSize : 0;

        SCALABILITY_CHK_STATUS_RETURN(ResizeCommandBufferAndPatchList(requestedSize, requestedPatchListSize));

    }
    return eStatus;
}

MOS_STATUS MediaScalability::Destroy()
{
    if (g_apoMosEnabled)
    {
        SCALABILITY_CHK_STATUS_RETURN(MosInterface::SetVirtualEngineState(m_osInterface->osStreamState, m_veState));
        return MosInterface::DestroyVirtualEngineState(m_osInterface->osStreamState);
    }

    if (m_veInterface)
    {
        if(m_veInterface->pfnVEDestroy)
        {
            m_veInterface->pfnVEDestroy(m_veInterface);
        }
        MOS_FreeMemAndSetNull(m_veInterface);
    }
    else
    {
        // For VE not enabled/supported case, such as vp vebox on some platform, m_veInterface is nullptr.
        // MOS_STATUS_SUCCESS should be returned for such case.
        if (MOS_VE_SUPPORTED(m_osInterface))
        {
            SCALABILITY_CHK_NULL_RETURN(m_veInterface);
        }
    }

    return MOS_STATUS_SUCCESS;
}
