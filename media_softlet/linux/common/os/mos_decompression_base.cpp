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
//! \file     mos_decompression_base.cpp
//! \brief    MOS decompression functions
//! \details  MOS decompression functions
//!

#include "mos_decompression.h"
#include "media_interfaces_mmd_next.h"

MosDecompressionBase::MosDecompressionBase(PMOS_CONTEXT osDriverContext)
{
#ifdef _MMC_SUPPORTED
    MOS_OS_CHK_NULL_NO_STATUS_RETURN(osDriverContext);

    m_mediaMemDecompState = static_cast<MediaMemDecompBaseState *>(MmdDeviceNext::CreateFactory(osDriverContext));

#endif
}

MosDecompressionBase::~MosDecompressionBase()
{
#ifdef _MMC_SUPPORTED
    MOS_Delete(m_mediaMemDecompState);
#endif
}

MOS_STATUS MosDecompressionBase::MemoryDecompress(
    PMOS_RESOURCE osResource)
{
    MOS_OS_CHK_NULL_RETURN(m_mediaMemDecompState);
    m_mediaMemDecompState->MemoryDecompress(osResource);
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosDecompressionBase::MediaMemoryCopy(
    PMOS_RESOURCE inputResource,
    PMOS_RESOURCE outputResource,
    bool          outputCompressed)
{
    MOS_OS_CHK_NULL_RETURN(m_mediaMemDecompState);
    m_mediaMemDecompState->MediaMemoryCopy(
        inputResource,
        outputResource,
        outputCompressed);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosDecompressionBase::MediaMemoryCopy2D(
    PMOS_RESOURCE inputResource,
    PMOS_RESOURCE outputResource,
    uint32_t      copyWidth,
    uint32_t      copyHeight,
    uint32_t      copyInputOffset,
    uint32_t      copyOutputOffset,
    uint32_t      bpp,
    bool          outputCompressed)
{
    MOS_OS_CHK_NULL_RETURN(m_mediaMemDecompState);

    m_mediaMemDecompState->MediaMemoryCopy2D(
        inputResource,
        outputResource,
        copyWidth,
        copyHeight,
        copyInputOffset,
        copyOutputOffset,
        bpp,
        outputCompressed);

    return MOS_STATUS_SUCCESS;
}
