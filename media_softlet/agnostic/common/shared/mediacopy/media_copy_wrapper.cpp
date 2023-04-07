/*
* Copyright (c) 2023, Intel Corporation
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
//! \file     media_copy_wrapper.cpp
//! \brief    Media Copy Base State Wrapper which is created in pipeline init
//! \details  Media Copy Base State Wrapper which is created in pipeline init
//!

#include "media_copy_wrapper.h"
#include "media_copy_common.h"
#include "media_interfaces_mcpy_next.h"

MediaCopyWrapper::MediaCopyWrapper(PMOS_INTERFACE osInterface) : m_osInterface(osInterface)
{
}

MediaCopyWrapper::~MediaCopyWrapper()
{
    MOS_Delete(m_mediaCopyState);
}

void MediaCopyWrapper::CreateMediaCopyState()
{
    if (nullptr == m_mediaCopyState)
    {
        PMOS_CONTEXT mos_context = nullptr;
        if (m_osInterface && m_osInterface->pfnGetMosContext)
        {
            m_osInterface->pfnGetMosContext(m_osInterface, &mos_context);
        }
        m_mediaCopyState = static_cast<MediaCopyBaseState *>(McpyDeviceNext::CreateFactory((MOS_CONTEXT_HANDLE)mos_context));
        if (m_osInterface && m_mediaCopyState && m_osInterface->pfnIsAsyncDevice(m_osInterface->osStreamState))
        {
            m_osInterface->pfnSetupCurrentCmdListAndPool(m_osInterface, m_mediaCopyState->m_osInterface->osStreamState);
        }
    }
}

MOS_STATUS MediaCopyWrapper::SetMediaCopyState(MediaCopyBaseState *mediaCopyState)
{
    if (nullptr == mediaCopyState)
    {
        return MOS_STATUS_SUCCESS;
    }
    if (nullptr == m_mediaCopyState)
    {
        m_mediaCopyState = mediaCopyState;
    }
    else
    {
        MCPY_ASSERTMESSAGE("m_mediaCopyState already exists, no need to set new MediaCopyBaseState in MediaCopyWrapper");
        return MOS_STATUS_INVALID_PARAMETER;
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MediaCopyWrapper::MediaCopy(
    PMOS_RESOURCE inputResource,
    PMOS_RESOURCE outputResource,
    MCPY_METHOD   preferMethod)
{
    MOS_STATUS    status = MOS_STATUS_SUCCESS;
    MCPY_CHK_NULL_RETURN(inputResource);
    MCPY_CHK_NULL_RETURN(outputResource);
    if (nullptr == m_mediaCopyState)
    {
        CreateMediaCopyState();
    }
    MCPY_CHK_NULL_RETURN(m_mediaCopyState);

    status = m_mediaCopyState->SurfaceCopy(
        inputResource,
        outputResource,
        preferMethod);

    return status;
}
