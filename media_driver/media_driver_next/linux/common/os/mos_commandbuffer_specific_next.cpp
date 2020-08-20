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
//! \file    mos_commandbuffer_specific_next.cpp
//! \brief   Container class for the specific command buffer
//!

#include "mos_commandbuffer_specific_next.h"
#include "mos_context_specific_next.h"
#include "mos_gpucontext_specific_next.h"
#include "mos_graphicsresource_specific_next.h"

MOS_STATUS  CommandBufferSpecificNext::Allocate(OsContextNext *osContext, uint32_t size)
{
    MOS_OS_FUNCTION_ENTER;

    MOS_OS_CHK_NULL_RETURN(osContext);

    if (osContext->GetOsContextValid() == false)
    {
        MOS_OS_ASSERTMESSAGE("The OS context got is not valid.");
        return MOS_STATUS_INVALID_HANDLE;
    }

    m_osContext = osContext;

    GraphicsResourceNext::CreateParams params;
    params.m_tileType  = MOS_TILE_LINEAR;
    params.m_type      = MOS_GFXRES_BUFFER;
    params.m_format    = Format_Buffer;
    params.m_width     = size;
    params.m_height    = 1;
    params.m_depth     = 1;
    params.m_arraySize = 1;
    params.m_name      = "MOS CmdBuf";

    m_graphicsResource = GraphicsResourceNext::CreateGraphicResource(GraphicsResourceNext::osSpecificResource);
    MOS_OS_CHK_NULL_RETURN(m_graphicsResource);

    MOS_OS_CHK_STATUS_RETURN(m_graphicsResource->Allocate(osContext, params));

    m_size = m_graphicsResource->GetSize(); // get real size after allocate

    return MOS_STATUS_SUCCESS;
}

void  CommandBufferSpecificNext::Free()
{
    MOS_OS_FUNCTION_ENTER;

    if (!m_graphicsResource)
    {
        MOS_OS_ASSERTMESSAGE("graphic resource back the commnd buffer need be allocated firstly.");
        return;
    }

    m_graphicsResource->Free(m_osContext, 0);
    MOS_Delete(m_graphicsResource);
}

MOS_STATUS CommandBufferSpecificNext::BindToGpuContext(GpuContextNext *gpuContext)
{
    MOS_OS_FUNCTION_ENTER;

    MOS_OS_CHK_NULL_RETURN(gpuContext);
    MOS_OS_CHK_NULL_RETURN(m_graphicsResource);

    GraphicsResourceNext::LockParams params;
    params.m_writeRequest = true;
    m_lockAddr = static_cast<uint8_t *>(m_graphicsResource->Lock(m_osContext, params));
    MOS_OS_CHK_NULL_RETURN(m_lockAddr);

    m_gpuContext = gpuContext;

    m_readyToUse = true;
    return MOS_STATUS_SUCCESS;

}

int CommandBufferSpecificNext::isBusy()
{
    MOS_OS_FUNCTION_ENTER;

    if (m_graphicsResource == nullptr)
    {
         MOS_OS_ASSERTMESSAGE("Graphicresource is not initialized.");
            // return not busy here to avoid dead lock
         return 0;
    }

    GraphicsResourceSpecificNext * graphicsResourceSpecific = static_cast<GraphicsResourceSpecificNext *>(m_graphicsResource);
    MOS_LINUX_BO* cmdBufBo = graphicsResourceSpecific->GetBufferObject();
    if (cmdBufBo == nullptr)
    {
        MOS_OS_ASSERTMESSAGE("Failed to get the buffer object for the command buffer, check busy failed.");
        // return not busy here to avoid dead lock
        return 0;
    }

    return mos_bo_busy(cmdBufBo);
}

void CommandBufferSpecificNext::waitReady()
{
    MOS_OS_FUNCTION_ENTER;

    if (m_graphicsResource == nullptr)
    {
        MOS_OS_ASSERTMESSAGE("Graphicresource is not initialized.");
        // return not busy here to avoid dead lock
        return;
    }

    GraphicsResourceSpecificNext * graphicsResourceSpecific = static_cast<GraphicsResourceSpecificNext *>(m_graphicsResource);
    MOS_LINUX_BO* cmdBufBo = graphicsResourceSpecific->GetBufferObject();
    if (cmdBufBo == nullptr)
    {
        MOS_OS_ASSERTMESSAGE("Failed to get the buffer object for the command buffer, wait failed.");
        return;
    }

    mos_bo_wait_rendering(cmdBufBo);
}

void CommandBufferSpecificNext::UnBindToGpuContext()
{
    MOS_OS_FUNCTION_ENTER;

    // Internal state validity check
    if (m_osContext == nullptr) {
        MOS_OS_ASSERTMESSAGE("m_osContext ptr should not  be nullptr");
        return ;
    }

    if (m_osContext->GetOsContextValid() == false)
    {
        MOS_OS_ASSERTMESSAGE("The OS context got is not valid.");
        return ;
    }

    if (!m_graphicsResource)
    {
        MOS_OS_ASSERTMESSAGE("graphic resource back the commnd buffer need be allocated firstly.");
        return;
    }

    m_graphicsResource->Unlock(m_osContext);

    m_readyToUse = false;
}

MOS_STATUS CommandBufferSpecificNext:: ReSize(uint32_t newSize)
{
    MOS_OS_FUNCTION_ENTER;

    if (m_readyToUse)
    {
        // release old command buffer
        UnBindToGpuContext();
        Free();
        m_readyToUse = false;
    }

    // re-allocate new buffer
    MOS_OS_CHK_STATUS_RETURN(Allocate(m_osContext, newSize));

    MOS_OS_CHK_STATUS_RETURN(BindToGpuContext(m_gpuContext));

    return MOS_STATUS_SUCCESS;
}

