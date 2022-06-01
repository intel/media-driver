/*
* Copyright (c) 2021, Intel Corporation
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
//! \file     codechal_mmc_encode_jpeg_g12.cpp
//! \brief    Impelements the public interface for Gen12 CodecHal Media Memory Compression
//!

#include "codechal_mmc_encode_jpeg_g12.h"

CodechalMmcEncodeJpegG12::CodechalMmcEncodeJpegG12(
    CodechalHwInterface    *hwInterface,
    void *standardState):
    CodecHalMmcStateG12(hwInterface)
{
    MOS_FUNCTION_ENTER(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_ENCODE);
    InitEncodeMmcEnable(hwInterface);
}

MOS_STATUS CodechalMmcEncodeJpegG12::SetPipeBufAddr(
    PMHW_VDBOX_PIPE_BUF_ADDR_PARAMS pipeBufAddrParams,
    PMOS_COMMAND_BUFFER cmdBuffer)
{
    MOS_FUNCTION_ENTER(MOS_COMPONENT_CODEC,  MOS_CODEC_SUBCOMP_ENCODE);
    MOS_CHK_NULL_RETURN(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_ENCODE, pipeBufAddrParams);
    MOS_CHK_NULL_RETURN(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_ENCODE, pipeBufAddrParams->psRawSurface);
    MOS_CHK_NULL_RETURN(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_ENCODE, pipeBufAddrParams->pRawSurfParam);

    if (m_mmcEnabled)
    {
        pipeBufAddrParams->bMmcEnabled = true;
        MOS_CHK_STATUS_RETURN(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_ENCODE, m_osInterface->pfnGetMemoryCompressionMode(m_osInterface,
            &pipeBufAddrParams->psRawSurface->OsResource, &pipeBufAddrParams->RawSurfMmcState));
        MOS_CHK_STATUS_RETURN(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_ENCODE, m_osInterface->pfnGetMemoryCompressionFormat(m_osInterface,
            &pipeBufAddrParams->psRawSurface->OsResource, &pipeBufAddrParams->pRawSurfParam->dwCompressionFormat));
    }
    else
    {
        pipeBufAddrParams->bMmcEnabled     = false;
        pipeBufAddrParams->RawSurfMmcState = MOS_MEMCOMP_DISABLED;
    }

    return MOS_STATUS_SUCCESS;
}
