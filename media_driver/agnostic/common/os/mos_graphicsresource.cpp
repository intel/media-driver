/*
* Copyright (c) 2017, Intel Corporation
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
//! \file    mos_graphicsresource.cpp
//! \brief   Container class for the basic gpu resource
//!

#include "mos_graphicsresource.h"
#include "mos_graphicsresource_specific.h"
#include "mos_util_debug.h"
#include "mos_utilities.h"
#include <string>
#include <fcntl.h>
#include <tuple>

uint32_t GraphicsResource::m_memAllocCounterGfx;

GraphicsResource::GraphicsResource()
{
    MOS_OS_FUNCTION_ENTER;
    m_allocationIndexMutex = MOS_CreateMutex();
    MOS_OS_CHK_NULL_NO_STATUS_RETURN(m_allocationIndexMutex);
}

GraphicsResource::~GraphicsResource()
{
    MOS_OS_FUNCTION_ENTER;
    MOS_DestroyMutex(m_allocationIndexMutex);
    m_allocationIndexMutex = nullptr;
}

MOS_STATUS GraphicsResource::Dump(OsContext* osContextPtr, uint32_t overrideOffset, uint32_t overrideSize, std::string outputFileName, std::string outputPath)
{
    MOS_OS_FUNCTION_ENTER;

    MOS_STATUS  eStatus = MOS_STATUS_SUCCESS;
    if (overrideSize == 0)
    {
        eStatus = MOS_STATUS_UNKNOWN;
        return eStatus;
    }

    char        sPath[MOS_MAX_PATH_LENGTH + 1];
    uint32_t    dwWritten = 0;
    MOS_SecureStringPrint(
        sPath,
        sizeof(sPath),
        sizeof(sPath) - 1,
        "%s%s",
        outputPath.c_str(),
        outputFileName.c_str());

    void*       hFile = nullptr;
    // Open file for writing
    eStatus = MOS_CreateFile(
        &hFile,
        sPath,
        O_WRONLY|O_CREAT);

    if (eStatus != MOS_STATUS_SUCCESS)
    {
        MOS_OS_ASSERTMESSAGE("Failed to open file '%s'.", sPath);
        eStatus = MOS_STATUS_FILE_OPEN_FAILED;
        if (hFile != nullptr)
        {
            MOS_CloseHandle(hFile);
        }
        return eStatus;
    }

    char*       pbData = nullptr;
    LockParams  params = {0};
    params.m_readRequest = true;
    pbData = (char *)this->Lock(osContextPtr, params);
    if (pbData == nullptr)
    {
        MOS_OS_ASSERTMESSAGE("Failed to lock the gpu resource");
        if (hFile != nullptr)
        {
            MOS_CloseHandle(hFile);
        }
        return MOS_STATUS_UNKNOWN;
    }

    pbData += overrideOffset;

    // Write the file
    if ((eStatus = MOS_WriteFile(
        hFile,
        pbData,
        overrideSize,
        &dwWritten,
        nullptr)) != MOS_STATUS_SUCCESS)
    {
        MOS_OS_ASSERTMESSAGE("Failed to write to file '%s'.", sPath);
        eStatus = MOS_STATUS_FILE_WRITE_FAILED;
        if (hFile != nullptr)
        {
            MOS_CloseHandle(hFile);
        }
    }

    eStatus = this->Unlock(osContextPtr);
    if (eStatus != MOS_STATUS_SUCCESS)
    {
        MOS_OS_ASSERTMESSAGE("Failed to unlock the gpu resource");
    }

    if (hFile != nullptr)
    {
        MOS_CloseHandle(hFile);
    }

    return eStatus;
}

class GraphicsResource* GraphicsResource::CreateGraphicResource(GraphicsResource::ResourceType resourceType)
{
    MOS_OS_FUNCTION_ENTER;

    class GraphicsResource* pResource = nullptr;

    switch (resourceType)
    {
    case osSpecificResource:
        pResource = MOS_New(GraphicsResourceSpecific);
        break;
    default:
        MOS_OS_ASSERTMESSAGE("Unknown Graphic Reosurce type %u passed in", resourceType);
        pResource = nullptr;
    }

    return pResource;
}

int32_t GraphicsResource::GetAllocationIndex(GPU_CONTEXT_HANDLE gpuContextHandle)
{
    MOS_OS_FUNCTION_ENTER;

    GPU_CONTEXT_HANDLE curGpuContext = 0;
    int32_t curAllocIndex            = MOS_INVALID_ALLOC_INDEX;
    int32_t ret                      = MOS_INVALID_ALLOC_INDEX; 

    MOS_LockMutex(m_allocationIndexMutex);
    for (auto& curAllocationIndexTp : m_allocationIndexArray)
    {
        std::tie(curGpuContext, curAllocIndex) = curAllocationIndexTp ;
        if (curGpuContext == gpuContextHandle)
        {
             ret = curAllocIndex;
             break;
        }
    }

    MOS_UnlockMutex(m_allocationIndexMutex);
    return ret;
}

void GraphicsResource::ResetResourceAllocationIndex()
{
    MOS_OS_FUNCTION_ENTER;
    MOS_LockMutex(m_allocationIndexMutex);
    m_allocationIndexArray.clear();
    MOS_UnlockMutex(m_allocationIndexMutex);
}

