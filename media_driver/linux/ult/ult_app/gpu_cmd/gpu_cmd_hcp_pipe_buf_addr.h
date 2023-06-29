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
#ifndef __GPU_CMD_HCP_PIPE_BUF_ADDR_H__
#define __GPU_CMD_HCP_PIPE_BUF_ADDR_H__

#include "gpu_cmd.h"

template<typename _CmdType>
class GpuCmdHcpPipeBufAddr : public GpuCmd<_CmdType>
{
public:

    using typename GpuCmd<_CmdType>::cmd_t;
    using GpuCmd<_CmdType>::CacheCheck1;
    using GpuCmd<_CmdType>::CacheCheck2;

protected:

    using GpuCmd<_CmdType>::m_pCmd;

    void ValidateCachePolicy(const cmd_t *pCmd) const override
    {
        CacheCheck1(m_pCmd->DecodedPictureMemoryAddressAttributes.DW0.Value,
            pCmd->DecodedPictureMemoryAddressAttributes.DW0.Value);
        CacheCheck2(m_pCmd->DeblockingFilterLineBufferMemoryAddressAttributes.DW0.Value,
            pCmd->DeblockingFilterLineBufferMemoryAddressAttributes.DW0.Value);
        CacheCheck2(m_pCmd->DeblockingFilterTileLineBufferMemoryAddressAttributes.DW0.Value,
            pCmd->DeblockingFilterTileLineBufferMemoryAddressAttributes.DW0.Value);
        CacheCheck2(m_pCmd->DeblockingFilterTileColumnBufferMemoryAddressAttributes.DW0.Value,
            pCmd->DeblockingFilterTileColumnBufferMemoryAddressAttributes.DW0.Value);
        CacheCheck2(m_pCmd->MetadataLineBufferMemoryAddressAttributes.DW0.Value,
            pCmd->MetadataLineBufferMemoryAddressAttributes.DW0.Value);
        CacheCheck2(m_pCmd->MetadataTileLineBufferMemoryAddressAttributes.DW0.Value,
            pCmd->MetadataTileLineBufferMemoryAddressAttributes.DW0.Value);
        CacheCheck2(m_pCmd->MetadataTileColumnBufferMemoryAddressAttributes.DW0.Value,
            pCmd->MetadataTileColumnBufferMemoryAddressAttributes.DW0.Value);
        CacheCheck2(m_pCmd->SaoLineBufferMemoryAddressAttributes.DW0.Value,
            pCmd->SaoLineBufferMemoryAddressAttributes.DW0.Value);
        CacheCheck2(m_pCmd->SaoTileLineBufferMemoryAddressAttributes.DW0.Value,
            pCmd->SaoTileLineBufferMemoryAddressAttributes.DW0.Value);
        CacheCheck2(m_pCmd->SaoTileColumnBufferMemoryAddressAttributes.DW0.Value,
            pCmd->SaoTileColumnBufferMemoryAddressAttributes.DW0.Value);
        CacheCheck2(m_pCmd->CurrentMotionVectorTemporalBufferMemoryAddressAttributes.DW0.Value,
            pCmd->CurrentMotionVectorTemporalBufferMemoryAddressAttributes.DW0.Value);
        CacheCheck1(m_pCmd->ReferencePictureBaseAddressMemoryAddressAttributes.DW0.Value,
            pCmd->ReferencePictureBaseAddressMemoryAddressAttributes.DW0.Value);
        CacheCheck2(m_pCmd->OriginalUncompressedPictureSourceMemoryAddressAttributes.DW0.Value,
            pCmd->OriginalUncompressedPictureSourceMemoryAddressAttributes.DW0.Value);
        CacheCheck2(m_pCmd->StreamoutDataDestinationMemoryAddressAttributes.DW0.Value,
            pCmd->StreamoutDataDestinationMemoryAddressAttributes.DW0.Value);
        CacheCheck2(m_pCmd->DecodedPictureStatusErrorBufferBaseAddressMemoryAddressAttributes.DW0.Value,
            pCmd->DecodedPictureStatusErrorBufferBaseAddressMemoryAddressAttributes.DW0.Value);
        CacheCheck2(m_pCmd->LcuIldbStreamoutBufferMemoryAddressAttributes.DW0.Value,
            pCmd->LcuIldbStreamoutBufferMemoryAddressAttributes.DW0.Value);
        CacheCheck1(m_pCmd->CollocatedMotionVectorTemporalBuffer07MemoryAddressAttributes.DW0.Value,
            pCmd->CollocatedMotionVectorTemporalBuffer07MemoryAddressAttributes.DW0.Value);
        CacheCheck2(m_pCmd->Vp9ProbabilityBufferReadWriteMemoryAddressAttributes.DW0.Value,
            pCmd->Vp9ProbabilityBufferReadWriteMemoryAddressAttributes.DW0.Value);
        CacheCheck2(m_pCmd->Vp9SegmentIdBufferReadWriteMemoryAddressAttributes.DW0.Value,
            pCmd->Vp9SegmentIdBufferReadWriteMemoryAddressAttributes.DW0.Value);
        CacheCheck2(m_pCmd->Vp9HvdLineRowstoreBufferReadWriteMemoryAddressAttributes.DW0.Value,
            pCmd->Vp9HvdLineRowstoreBufferReadWriteMemoryAddressAttributes.DW0.Value);
        CacheCheck2(m_pCmd->Vp9HvdTileRowstoreBufferReadWriteMemoryAddressAttributes.DW0.Value,
            pCmd->Vp9HvdTileRowstoreBufferReadWriteMemoryAddressAttributes.DW0.Value);
        CacheCheck2(m_pCmd->SaoStreamoutDataDestinationBufferReadWriteMemoryAddressAttributes.DW0.Value,
            pCmd->SaoStreamoutDataDestinationBufferReadWriteMemoryAddressAttributes.DW0.Value);
        CacheCheck2(m_pCmd->FrameStatisticsStreamoutDataDestinationBufferAttributesReadWrite.DW0.Value,
            pCmd->FrameStatisticsStreamoutDataDestinationBufferAttributesReadWrite.DW0.Value);
        CacheCheck2(m_pCmd->SseSourcePixelRowstoreBufferAttributesReadWrite.DW0.Value,
            pCmd->SseSourcePixelRowstoreBufferAttributesReadWrite.DW0.Value);
    }
};

#endif // __GPU_CMD_HCP_PIPE_BUF_ADDR_H__
