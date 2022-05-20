/*
* Copyright (c) 2021-2022, Intel Corporation
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
//! \file     codechal_mmc_encode_avc_xe_hpm.cpp
//! \brief    Impelements the public interface for Xe_HPM CodecHal Media Memory Compression
//!

#include "codechal_mmc_encode_avc_xe_hpm.h"

MOS_STATUS CodechalMmcEncodeAvcXe_Hpm::SetPipeBufAddr(
    PMHW_VDBOX_PIPE_BUF_ADDR_PARAMS pipeBufAddrParams,
    PMOS_COMMAND_BUFFER cmdBuffer)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;
    MOS_MEMCOMP_STATE reconSurfMmcState = MOS_MEMCOMP_DISABLED;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    if (m_mmcEnabled)
    {
        pipeBufAddrParams->bMmcEnabled = true;
        CODECHAL_ENCODE_CHK_NULL_RETURN(pipeBufAddrParams->psRawSurface);
        CODECHAL_ENCODE_CHK_COND_RETURN(Mos_ResourceIsNull(&m_avcState->m_reconSurface.OsResource), "m_avcState->m_reconSurface.OsResource is null");

        // Src surface
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnGetMemoryCompressionMode(m_osInterface,
            &pipeBufAddrParams->psRawSurface->OsResource, &pipeBufAddrParams->RawSurfMmcState));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnGetMemoryCompressionFormat(m_osInterface,
            &pipeBufAddrParams->psRawSurface->OsResource, &pipeBufAddrParams->pRawSurfParam->dwCompressionFormat));

        // Ref surface
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnGetMemoryCompressionMode(m_osInterface,
            &m_avcState->m_reconSurface.OsResource, &reconSurfMmcState));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnGetMemoryCompressionFormat(m_osInterface,
            &m_avcState->m_reconSurface.OsResource, &pipeBufAddrParams->pDecodedReconParam->dwCompressionFormat));

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
        pipeBufAddrParams->bMmcEnabled = false;
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
