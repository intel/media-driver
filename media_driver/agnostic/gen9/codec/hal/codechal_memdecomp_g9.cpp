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
//! \file     codechal_memdecomp_g9.cpp
//! \brief    This module sets up a kernel for Gen9 media memory decompression.
//! \details

#include "codechal_memdecomp_g9.h"
#include "igcodeckrn_g9.h"

MediaMemDecompStateG9::MediaMemDecompStateG9():
    MediaMemDecompState()
{
     MHW_FUNCTION_ENTER;

     m_kernelBase = (uint8_t *)IGCODECKRN_G9;
}

MOS_STATUS MediaMemDecompStateG9::InitKernelState(
    uint32_t                 kernelStateIdx)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    if (kernelStateIdx >= decompKernelStateMax)
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        return eStatus;
    }

    uint8_t *kernelBase = nullptr;
    uint32_t kernelSize = 0;
    if ((kernelStateIdx == decompKernelStatePl2)&&
        m_osInterface->osCpInterface->IsSMEnabled())
    {
        m_osInterface->osCpInterface->GetTK(
            (uint32_t **)&kernelBase,
            &kernelSize,
            nullptr);
        if (nullptr == kernelBase ||
            0 == kernelSize)
        {
            MHW_ASSERT("Could not get TK for MMC!");
            eStatus = MOS_STATUS_INVALID_PARAMETER;
            return eStatus;
        }
    }
    else
    {
        MHW_CHK_STATUS_RETURN(GetKernelBinaryAndSize(
            m_kernelBase,
            m_krnUniId[kernelStateIdx],
            &kernelBase,
            &kernelSize));
    }

    m_kernelBinary[kernelStateIdx] = kernelBase;
    m_kernelSize[kernelStateIdx]   = kernelSize;

    m_stateHeapSettings.dwIshSize +=
        MOS_ALIGN_CEIL(kernelSize, (1 << MHW_KERNEL_OFFSET_SHIFT));
    m_stateHeapSettings.dwDshSize += MHW_CACHELINE_SIZE* m_numMemDecompSyncTags;
    m_stateHeapSettings.dwNumSyncTags += m_numMemDecompSyncTags;

    return eStatus;
}

