/*
* Copyright (c) 2020, Intel Corporation
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
//! \file     renderhal_memdecomp.cpp
//! \brief    Defines data structures and interfaces for media memory decompression.
//! \details
//!
#include "renderhal_memdecomp.h"

MediaRenderDecompState::MediaRenderDecompState() :
    MediaMemDecompBaseState(),
    m_osInterface(nullptr),
    m_mhwMiInterface(nullptr),
    m_cpInterface(nullptr),
    m_renderInterface(nullptr)
{
}

MediaRenderDecompState::~MediaRenderDecompState()
{
    if (m_cpInterface)
    {
        if (m_osInterface)
        {
            m_osInterface->pfnDeleteMhwCpInterface(m_cpInterface);
            m_cpInterface = nullptr;
        }
        else
        {
            RENDERHAL_MEMORY_DECOMP_ASSERTMESSAGE("Failed to destroy cpInterface.");
        }
    }
    MOS_Delete(m_mhwMiInterface);
    MOS_Delete(m_renderInterface);

    if (m_osInterface)
    {
        m_osInterface->pfnDestroy(m_osInterface, false);
        MOS_FreeMemory(m_osInterface);
        m_osInterface = nullptr;
    }
}

MOS_STATUS MediaRenderDecompState::MemoryDecompress(PMOS_RESOURCE targetResource)
{
    MOS_STATUS              eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    RENDERHAL_MEMORY_DECOMP_CHK_NULL_RETURN(targetResource);

    return eStatus;
}

MOS_STATUS MediaRenderDecompState::MediaMemoryCopy(
    PMOS_RESOURCE inputResource,
    PMOS_RESOURCE outputResource,
    bool          outputCompressed)
{
    MOS_STATUS                          eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    RENDERHAL_MEMORY_DECOMP_CHK_NULL_RETURN(inputResource);
    RENDERHAL_MEMORY_DECOMP_CHK_NULL_RETURN(outputResource);

    return eStatus;
}

MOS_STATUS MediaRenderDecompState::MediaMemoryCopy2D(
    PMOS_RESOURCE inputResource,
    PMOS_RESOURCE outputResource,
    uint32_t      copyWidth,
    uint32_t      copyHeight,
    uint32_t      copyInputOffset,
    uint32_t      copyOutputOffset,
    uint32_t      bpp,
    bool          outputCompressed)
{
    MOS_STATUS                          eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    RENDERHAL_MEMORY_DECOMP_CHK_NULL_RETURN(inputResource);
    RENDERHAL_MEMORY_DECOMP_CHK_NULL_RETURN(outputResource);

    return eStatus;
}

MOS_STATUS MediaRenderDecompState::Initialize(
    PMOS_INTERFACE               osInterface,
    MhwCpInterface               *cpInterface,
    PMHW_MI_INTERFACE            mhwMiInterface,
    MhwRenderInterface           *renderInterface)
{
    MOS_STATUS                  eStatus;

    eStatus = MOS_STATUS_SUCCESS;

    RENDERHAL_MEMORY_DECOMP_CHK_NULL_RETURN(osInterface);
    RENDERHAL_MEMORY_DECOMP_CHK_NULL_RETURN(cpInterface);
    RENDERHAL_MEMORY_DECOMP_CHK_NULL_RETURN(mhwMiInterface);
    RENDERHAL_MEMORY_DECOMP_CHK_NULL_RETURN(renderInterface);

    m_osInterface     = osInterface;
    m_cpInterface     = cpInterface;
    m_mhwMiInterface  = mhwMiInterface;
    m_renderInterface = renderInterface;

    return eStatus;
}
