/*
* Copyright (c) 2017-2019, Intel Corporation
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
//! \file     codechal_mmc_encode_avc_g12.cpp
//! \brief    Impelements the public interface for Gen12 CodecHal Media Memory Compression
//!

#include "codechal_mmc_encode_avc_g12.h"

CodechalMmcEncodeAvcG12::CodechalMmcEncodeAvcG12(
    CodechalHwInterface    *hwInterface,
    void *standardState):
    CodecHalMmcStateG12(hwInterface)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    m_avcState = (CodechalEncodeAvcBase *)standardState;
    CODECHAL_HW_ASSERT(m_avcState);

    InitEncodeMmcEnable(hwInterface);
}

MOS_STATUS CodechalMmcEncodeAvcG12::SetPipeBufAddr(
    PMHW_VDBOX_PIPE_BUF_ADDR_PARAMS pipeBufAddrParams,
    PMOS_COMMAND_BUFFER cmdBuffer)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;
    MOS_MEMCOMP_STATE reconSurfMmcState = MOS_MEMCOMP_DISABLED;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    if (m_mmcEnabled)
    {
        CODECHAL_ENCODE_CHK_NULL_RETURN(pipeBufAddrParams->psRawSurface);
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnGetMemoryCompressionMode(m_osInterface,
            &pipeBufAddrParams->psRawSurface->OsResource, &pipeBufAddrParams->RawSurfMmcState));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnGetMemoryCompressionMode(m_osInterface,
            &m_avcState->m_reconSurface.OsResource, &reconSurfMmcState));

        if (m_avcState->m_deblockingEnabled)
        {
            pipeBufAddrParams->PreDeblockSurfMmcState = MOS_MEMCOMP_DISABLED;
            pipeBufAddrParams->PostDeblockSurfMmcState = reconSurfMmcState;
        }
        else
        {
            pipeBufAddrParams->PreDeblockSurfMmcState = reconSurfMmcState;
            pipeBufAddrParams->PostDeblockSurfMmcState = MOS_MEMCOMP_DISABLED;
        }
    }
    else
    {
        pipeBufAddrParams->PreDeblockSurfMmcState = MOS_MEMCOMP_DISABLED;
        pipeBufAddrParams->RawSurfMmcState = MOS_MEMCOMP_DISABLED;
    }
    if (m_avcState->m_vdencEnabled)
    {
        pipeBufAddrParams->Ps4xDsSurfMmcState = MOS_MEMCOMP_DISABLED; // pipeBufAddrParams->PostDeblockSurfMmcState;
        pipeBufAddrParams->Ps8xDsSurfMmcState = MOS_MEMCOMP_DISABLED; // pipeBufAddrParams->PostDeblockSurfMmcState;
    }

    CODECHAL_DEBUG_TOOL(
        m_avcState->m_reconSurface.MmcState = reconSurfMmcState;
    );

    return eStatus;
}

