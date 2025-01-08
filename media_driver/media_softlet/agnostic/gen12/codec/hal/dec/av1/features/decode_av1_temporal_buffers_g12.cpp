/*
* Copyright (c) 2019-2024, Intel Corporation
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
//! \file     decode_av1_temporal_buffers_g12.cpp
//! \brief    Defines temporal buffers related logic for av1 decode
//!

#include "codec_def_decode_av1.h"
#include "decode_av1_basic_feature_g12.h"
#include "decode_av1_temporal_buffers_g12.h"
#include "decode_utils.h"
#include "codechal_utilities.h"

namespace decode
{
    MOS_STATUS Av1TempBufferOpInfG12::Init(void* hwInterface, DecodeAllocator& allocator,
                                       Av1BasicFeatureG12& basicFeature)
    {
        DECODE_CHK_STATUS(BufferOpInf::Init(hwInterface, allocator, basicFeature));
        m_avpInterface = ((CodechalHwInterface*)m_hwInterface)->GetAvpInterface();
        m_basicFeature = &basicFeature;
        DECODE_CHK_NULL(m_avpInterface);
        return MOS_STATUS_SUCCESS;
    }

    Av1RefAssociatedBufs* Av1TempBufferOpInfG12::Allocate()
    {
        DECODE_FUNC_CALL();

        int32_t mibSizeLog2 = av1MinMibSizeLog2;
        MhwVdboxAvpBufferSizeParams avpBufSizeParam;
        SetAvpBufSizeParam(avpBufSizeParam, mibSizeLog2);

        if (m_avpInterface->GetAv1BufferSize(mvTemporalBuf,
                                            &avpBufSizeParam) != MOS_STATUS_SUCCESS)
        {
            DECODE_ASSERTMESSAGE( "Failed to get MvTemporalBuffer size.");
        }

        Av1RefAssociatedBufs *bufs = MOS_New(Av1RefAssociatedBufs);
        bufs->mvBuf = m_allocator->AllocateBuffer(
            avpBufSizeParam.m_bufferSize, "MvTemporalBuffer", resourceInternalReadWriteCache, notLockableVideoMem);

        if (m_avpInterface->GetAv1BufferSize(segmentIdBuf,
                                            &avpBufSizeParam) != MOS_STATUS_SUCCESS)
        {
            DECODE_ASSERTMESSAGE( "Failed to get SegmentIdBuffer size.");
        }
        bufs->segIdWriteBuf.buffer = m_allocator->AllocateBuffer(
            avpBufSizeParam.m_bufferSize, "SegmentIdWriteBuffer", resourceInternalReadWriteCache, notLockableVideoMem);

        bufs->bwdAdaptCdfBuf.buffer = m_allocator->AllocateBuffer(MOS_ALIGN_CEIL(m_basicFeature->m_cdfMaxNumBytes,
            CODECHAL_PAGE_SIZE), "CdfTableBuffer", resourceInternalReadWriteCache, notLockableVideoMem);
        return bufs;
    }

    MOS_STATUS Av1TempBufferOpInfG12::Resize(Av1RefAssociatedBufs* &buffer)
    {
        DECODE_FUNC_CALL();

        if (buffer == nullptr)
        {
            DECODE_CHK_NULL(buffer = Allocate());
            return MOS_STATUS_SUCCESS;
        }

        int32_t mibSizeLog2 = m_basicFeature->m_av1PicParams->m_seqInfoFlags.m_fields.m_use128x128Superblock ? av1MaxMibSizeLog2 : av1MinMibSizeLog2;
        MhwVdboxAvpBufferSizeParams avpBufSizeParam;
        SetAvpBufSizeParam(avpBufSizeParam, mibSizeLog2);

        DECODE_CHK_STATUS(m_avpInterface->GetAv1BufferSize(
            mvTemporalBuf,
            &avpBufSizeParam));
        DECODE_CHK_STATUS(m_allocator->Resize(
            buffer->mvBuf, avpBufSizeParam.m_bufferSize, notLockableVideoMem, false));

        DECODE_CHK_STATUS(m_avpInterface->GetAv1BufferSize(
            segmentIdBuf,
            &avpBufSizeParam));
        DECODE_CHK_STATUS(m_allocator->Resize(
            buffer->segIdWriteBuf.buffer, avpBufSizeParam.m_bufferSize, notLockableVideoMem, false));

        RecordSegIdBufInfo(buffer);
        RecordCdfTableBufInfo(buffer);

        return MOS_STATUS_SUCCESS;
    }

    void Av1TempBufferOpInfG12::SetAvpBufSizeParam(MhwVdboxAvpBufferSizeParams& params, int32_t mibSizeLog2)
    {
        int32_t miCols = MOS_ALIGN_CEIL(m_basicFeature->m_width, 8) >> av1MiSizeLog2;
        int32_t miRows = MOS_ALIGN_CEIL(m_basicFeature->m_height, 8) >> av1MiSizeLog2;
        miCols         = MOS_ALIGN_CEIL(miCols, 1 << mibSizeLog2);
        miRows         = MOS_ALIGN_CEIL(miRows, 1 << mibSizeLog2);
        widthInSb      = miCols >> mibSizeLog2;
        heightInSb     = miRows >> mibSizeLog2;

        MOS_ZeroMemory(&params, sizeof(params));
        params.m_picWidth    = widthInSb;
        params.m_picHeight   = heightInSb;
        params.m_isSb128x128 = false;
        if (m_basicFeature->m_av1PicParams != nullptr)
        {
            params.m_isSb128x128 = m_basicFeature->m_av1PicParams->m_seqInfoFlags.m_fields.m_use128x128Superblock ? true : false;
        }
    }

    void Av1TempBufferOpInfG12::RecordSegIdBufInfo(Av1RefAssociatedBufs *buffer)
    {
        auto    picParams    = m_basicFeature->m_av1PicParams;
        uint8_t prevFrameIdx = m_basicFeature->m_refFrames.GetPrimaryRefIdx();
        if (picParams->m_av1SegData.m_enabled)
        {
            if (picParams->m_av1SegData.m_updateMap)
            {
                buffer->segIdBuf = RefSharedBuffer(&buffer->segIdWriteBuf);
            }
            else
            {
                auto tempBuffers = &(m_basicFeature->m_tempBuffers);
                auto prevTempBuffer = tempBuffers->GetBufferByFrameIndex(prevFrameIdx);
                if (prevTempBuffer != nullptr)
                {
                    if (m_basicFeature->m_refFrames.CheckSegForPrimFrame(*picParams))
                    {
                        buffer->segIdBuf = RefSharedBuffer(prevTempBuffer->segIdBuf);
                    }
                    else
                    {
                        buffer->segIdBuf = nullptr;
                    }
                }
            }
        }
        else
        {
            buffer->segIdBuf = nullptr;
        }
    }

    void Av1TempBufferOpInfG12::RecordCdfTableBufInfo(Av1RefAssociatedBufs *buffer)
    {
        auto    picParams              = m_basicFeature->m_av1PicParams;
        uint8_t prevFrameIdx           = m_basicFeature->m_refFrames.GetPrimaryRefIdx();
        buffer->disableFrmEndUpdateCdf = picParams->m_picInfoFlags.m_fields.m_disableFrameEndUpdateCdf;

        if (picParams->m_primaryRefFrame == av1PrimaryRefNone)
        {
            buffer->defaultCdfBuf.buffer = m_basicFeature->m_defaultCdfBufferInUse;
            buffer->initCdfBuf = RefSharedBuffer(&buffer->defaultCdfBuf);
        }
        else
        {
            auto tempBuffers = &(m_basicFeature->m_tempBuffers);
            auto prevTempBuffer = tempBuffers->GetBufferByFrameIndex(prevFrameIdx);
            if (prevTempBuffer != nullptr)
            {
                if (prevTempBuffer->disableFrmEndUpdateCdf)
                {
                    buffer->initCdfBuf = RefSharedBuffer(prevTempBuffer->initCdfBuf);
                }
                else
                {
                    buffer->initCdfBuf = RefSharedBuffer(&prevTempBuffer->bwdAdaptCdfBuf);
                }
            }
        }
    }

    MOS_STATUS Av1TempBufferOpInfG12::Deactive(Av1RefAssociatedBufs* &buffer)
    {
        DeRefSharedBuffer(buffer->segIdBuf);
        DeRefSharedBuffer(buffer->initCdfBuf);
        return MOS_STATUS_SUCCESS;
    }

    bool Av1TempBufferOpInfG12::IsAvailable(Av1RefAssociatedBufs* &buffer)
    {
        auto osInterface = m_basicFeature->GetOsInterface();
        if (osInterface != nullptr && osInterface->pfnIsMismatchOrderProgrammingSupported())
        {
            return true;
        }

        if (buffer == nullptr)
        {
            return false;
        }
        if (buffer->segIdWriteBuf.refCnt > 0)
        {
            return false;
        }
        if (buffer->bwdAdaptCdfBuf.refCnt > 0)
        {
            return false;
        }
        if (buffer->defaultCdfBuf.refCnt > 0)
        {
            return false;
        }
        return true;
    }

    void Av1TempBufferOpInfG12::Destroy(Av1RefAssociatedBufs* &buffer)
    {
        DECODE_FUNC_CALL();

        if (buffer != nullptr)
        {
            m_allocator->Destroy(buffer->mvBuf);

            m_allocator->Destroy(buffer->segIdWriteBuf.buffer);
            if (buffer->segIdWriteBuf.refCnt != 0)
            {
                DECODE_NORMALMESSAGE("The reference count is %d while destory SegId buffer", buffer->segIdWriteBuf.refCnt);
            }

            m_allocator->Destroy(buffer->bwdAdaptCdfBuf.buffer);
            if (buffer->bwdAdaptCdfBuf.refCnt != 0)
            {
                DECODE_NORMALMESSAGE("The reference count is %d while destory CDF buffer", buffer->bwdAdaptCdfBuf.refCnt);
            }

            MOS_Delete(buffer);
            buffer = nullptr;
        }
    }

    Av1SharedBuf *Av1TempBufferOpInfG12::RefSharedBuffer(Av1SharedBuf *sharedBuf)
    {
        if (sharedBuf != nullptr)
        {
            sharedBuf->refCnt++;
        }
        return sharedBuf;
    }

    Av1SharedBuf *Av1TempBufferOpInfG12::DeRefSharedBuffer(Av1SharedBuf *sharedBuf)
    {
        if (sharedBuf != nullptr)
        {
            sharedBuf->refCnt--;
            if (sharedBuf->refCnt < 0)
            {
                DECODE_ASSERTMESSAGE("AV1 shared buffer reference count is %d which should not small than zero.", sharedBuf->refCnt);
                sharedBuf->refCnt = 0;
            }
        }
        return sharedBuf;
    }
}  // namespace decode
