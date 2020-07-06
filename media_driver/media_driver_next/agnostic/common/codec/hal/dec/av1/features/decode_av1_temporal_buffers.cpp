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
//! \file     decode_av1_temporal_buffers.cpp
//! \brief    Defines temporal buffers related logic for av1 decode
//!

#include "codec_def_decode_av1.h"
#include "decode_av1_basic_feature.h"
#include "decode_av1_temporal_buffers.h"
#include "decode_utils.h"
#include "codechal_utilities.h"

namespace decode
{
    MOS_STATUS Av1TempBufferOpInf::Init(CodechalHwInterface& hwInterface, DecodeAllocator& allocator,
                                       Av1BasicFeature& basicFeature)
    {
        DECODE_CHK_STATUS(BufferOpInf::Init(hwInterface, allocator, basicFeature));
        m_avpInterface = static_cast<CodechalHwInterfaceG12*>(m_hwInterface)->GetAvpInterface();
        m_basicFeature = &basicFeature;
        DECODE_CHK_NULL(m_avpInterface);
        return MOS_STATUS_SUCCESS;
    }

    Av1RefAssociatedBufs* Av1TempBufferOpInf::Allocate()
    {
        DECODE_FUNC_CALL();

        int32_t mibSizeLog2 = av1MinMibSizeLog2;
        int32_t miCols = MOS_ALIGN_CEIL(m_basicFeature->m_width, 8) >> av1MiSizeLog2;
        int32_t miRows = MOS_ALIGN_CEIL(m_basicFeature->m_height, 8) >> av1MiSizeLog2;
        miCols = MOS_ALIGN_CEIL(miCols, 1 << mibSizeLog2);
        miRows = MOS_ALIGN_CEIL(miRows, 1 << mibSizeLog2);
        widthInSb = miCols >> mibSizeLog2;
        heightInSb = miRows >> mibSizeLog2;

        Av1RefAssociatedBufs *      bufs = MOS_New(Av1RefAssociatedBufs);
        MhwVdboxAvpBufferSizeParams avpBufSizeParam;
        MOS_ZeroMemory(&avpBufSizeParam, sizeof(avpBufSizeParam));
        avpBufSizeParam.m_picWidth  = widthInSb;
        avpBufSizeParam.m_picHeight = heightInSb;
        if (m_avpInterface->GetAv1BufferSize(mvTemporalBuf,
                                            &avpBufSizeParam) != MOS_STATUS_SUCCESS)
        {
            DECODE_ASSERTMESSAGE( "Failed to get MvTemporalBuffer size.");
        }
        bufs->mvBuf = m_allocator->AllocateBuffer(avpBufSizeParam.m_bufferSize, "MvTemporalBuffer");

        if (m_avpInterface->GetAv1BufferSize(segmentIdBuf,
                                            &avpBufSizeParam) != MOS_STATUS_SUCCESS)
        {
            DECODE_ASSERTMESSAGE( "Failed to get SegmentIdBuffer size.");
        }
        bufs->segIdWriteBuf = m_allocator->AllocateBuffer(avpBufSizeParam.m_bufferSize, "SegmentIdWriteBuffer");

        bufs->bwdAdaptCdfBuf = m_allocator->AllocateBuffer(MOS_ALIGN_CEIL(m_basicFeature->m_cdfMaxNumBytes,
            CODECHAL_PAGE_SIZE), "CdfTableBuffer");

        return bufs;
    }

    MOS_STATUS Av1TempBufferOpInf::Resize(Av1RefAssociatedBufs* &buffer)
    {
        DECODE_FUNC_CALL();

        if (buffer == nullptr)
        {
            DECODE_CHK_NULL(buffer = Allocate());
            return MOS_STATUS_SUCCESS;
        }

        MhwVdboxAvpBufferSizeParams avpBufSizeParam;
        MOS_ZeroMemory(&avpBufSizeParam, sizeof(avpBufSizeParam));
        avpBufSizeParam.m_picWidth  = widthInSb;
        avpBufSizeParam.m_picHeight = heightInSb;
        DECODE_CHK_STATUS(m_avpInterface->GetAv1BufferSize(
            mvTemporalBuf,
            &avpBufSizeParam));
        DECODE_CHK_STATUS(m_allocator->Resize(buffer->mvBuf, avpBufSizeParam.m_bufferSize, false));

        DECODE_CHK_STATUS(m_avpInterface->GetAv1BufferSize(
            segmentIdBuf,
            &avpBufSizeParam));
        DECODE_CHK_STATUS(m_allocator->Resize(buffer->segIdWriteBuf, avpBufSizeParam.m_bufferSize, false));

        RecordSegIdBufInfo(buffer);
        RecordCdfTableBufInfo(buffer);

        return MOS_STATUS_SUCCESS;
    }

    void Av1TempBufferOpInf::RecordSegIdBufInfo(Av1RefAssociatedBufs *buffer)
    {
        auto    picParams    = m_basicFeature->m_av1PicParams;
        uint8_t prevFrameIdx = m_basicFeature->m_refFrames.GetPrimaryRefIdx();
        if (picParams->m_av1SegData.m_enabled)
        {
            if (picParams->m_av1SegData.m_updateMap)
            {
                buffer->segIdBuf = buffer->segIdWriteBuf;
            }
            else
            {
                auto tempBuffers = &(m_basicFeature->m_tempBuffers);
                if (tempBuffers->GetBufferByFrameIndex(prevFrameIdx) != nullptr)
                {
                    if (m_basicFeature->m_refFrames.CheckSegForPrimFrame(*picParams))
                    {
                        buffer->segIdBuf = tempBuffers->GetBufferByFrameIndex(prevFrameIdx)->segIdBuf;
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

    void Av1TempBufferOpInf::RecordCdfTableBufInfo(Av1RefAssociatedBufs *buffer)
    {
        auto    picParams              = m_basicFeature->m_av1PicParams;
        uint8_t prevFrameIdx           = m_basicFeature->m_refFrames.GetPrimaryRefIdx();
        buffer->disableFrmEndUpdateCdf = picParams->m_picInfoFlags.m_fields.m_disableFrameEndUpdateCdf;
        if (picParams->m_primaryRefFrame == av1PrimaryRefNone)
        {
            buffer->initCdfBuf = m_basicFeature->m_defaultCdfBufferInUse;
        }
        else
        {
            auto tempBuffers = &(m_basicFeature->m_tempBuffers);
            if (tempBuffers->GetBufferByFrameIndex(prevFrameIdx) != nullptr)
            {
                if (tempBuffers->GetBufferByFrameIndex(prevFrameIdx)->disableFrmEndUpdateCdf)
                {
                    buffer->initCdfBuf = tempBuffers->GetBufferByFrameIndex(prevFrameIdx)->initCdfBuf;
                }
                else
                {
                    buffer->initCdfBuf = tempBuffers->GetBufferByFrameIndex(prevFrameIdx)->bwdAdaptCdfBuf;
                }

                //If init cdf buffer and bwdAdapt cdf buffer point to same memory address,
                //should get a another bwdAdapt cdf buffer for cdf stream out from avaliable buffer pool.
                if (buffer->initCdfBuf == buffer->bwdAdaptCdfBuf)
                {
                    auto        aviabuf     = tempBuffers->GetAviableBuffer();
                    PMOS_BUFFER temp        = aviabuf->bwdAdaptCdfBuf;
                    aviabuf->bwdAdaptCdfBuf = buffer->bwdAdaptCdfBuf;
                    buffer->bwdAdaptCdfBuf  = temp;
                }
            }
        }
    }

    void Av1TempBufferOpInf::Destroy(Av1RefAssociatedBufs* &buffer)
    {
        DECODE_FUNC_CALL();

        if (buffer != nullptr)
        {
            m_allocator->Destroy(buffer->mvBuf);
            m_allocator->Destroy(buffer->segIdWriteBuf);
            m_allocator->Destroy(buffer->bwdAdaptCdfBuf);

            MOS_Delete(buffer);
            buffer = nullptr;
        }
    }

}  // namespace decode
