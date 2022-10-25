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
//! \file     decode_jpeg_input_bitstream.cpp
//! \brief    Defines the common interface for decode jpeg input bitstream
//!
#include "decode_jpeg_input_bitstream.h"
#include "decode_pipeline.h"

namespace decode
{
DecodeJpegInputBitstream::DecodeJpegInputBitstream(DecodePipeline* pipeline, MediaTask* task, uint8_t numVdbox)
    : DecodeInputBitstream(pipeline, task, numVdbox)
{
}

MOS_STATUS DecodeJpegInputBitstream::Init(CodechalSetting &settings)
{
    DecodeInputBitstream::Init(settings);

    m_jpegBasicFeature = dynamic_cast<JpegBasicFeature *>(m_basicFeature);
    DECODE_CHK_NULL(m_jpegBasicFeature);
    return MOS_STATUS_SUCCESS;
}

bool DecodeJpegInputBitstream::IsComplete()
{
    return m_completeBitStream && m_completeJpegScan;
}

MOS_STATUS DecodeJpegInputBitstream::Append(const CodechalDecodeParams &decodeParams)
{
    bool firstExecuteCall  = (decodeParams.m_executeCallIndex == 0);
    auto numScans          = m_jpegBasicFeature->m_jpegScanParams->NumScans;
    auto totalScans        = m_jpegBasicFeature->m_jpegPicParams->m_totalScans;
    uint32_t maxBufferSize = MOS_ALIGN_CEIL(m_jpegBasicFeature->m_jpegPicParams->m_frameWidth * m_jpegBasicFeature->m_jpegPicParams->m_frameHeight * 3, 64);
    uint32_t segmentSize   = decodeParams.m_dataSize;
    if (firstExecuteCall)
    {
        auto headerSize = m_jpegBasicFeature->m_jpegScanParams->ScanHeader[numScans - 1].DataOffset +
                          m_jpegBasicFeature->m_jpegScanParams->ScanHeader[numScans - 1].DataLength;
       
        if (numScans >= totalScans && segmentSize >= headerSize)  // Bitstream complete and scan complete
        {
            m_completeBitStream = true;
            m_completeJpegScan  = true;
        }
        else if (numScans < totalScans && segmentSize > headerSize)  // Bitstream complete and scan incomplete
        {
            m_completeBitStream = true;
            m_completeJpegScan  = false;
        }
        else if (numScans >= totalScans && segmentSize < headerSize)  //Bitstream incomplete and scan complete, should catenate bitstream
        {
            m_completeBitStream = false;
            m_completeJpegScan  = true;
            m_requiredSize      = maxBufferSize;
            
            //Allocate Buffer
            DECODE_CHK_STATUS(AllocateCatenatedBuffer());
            m_basicFeature->m_resDataBuffer = *m_catenatedBuffer;
            m_basicFeature->m_dataOffset    = 0;
            DECODE_CHK_STATUS(ActivatePacket(DecodePacketId(m_pipeline, hucCopyPacketId), true, 0, 0));
            AddNewSegment(*(decodeParams.m_dataBuffer), decodeParams.m_dataOffset, decodeParams.m_dataSize);
        }
        else
        {
            return MOS_STATUS_INVALID_PARAMETER;
        }

    }
    else
    {
        if (m_completeBitStream)// Bitstream complete and scan incomplete, wait scan complete
        {
            m_completeJpegScan = (numScans >= totalScans);
        }
        else//Bitstream incomplete and scan complete, should catenate bitstream
        {
            if (m_segmentsTotalSize + segmentSize > m_requiredSize)
            {
                DECODE_ASSERTMESSAGE("Bitstream size exceeds allocated buffer size!");
                return MOS_STATUS_INVALID_PARAMETER;
            }
            DECODE_CHK_STATUS(ActivatePacket(DecodePacketId(m_pipeline, hucCopyPacketId), true, 0, 0));
            AddNewSegment(*(decodeParams.m_dataBuffer), decodeParams.m_dataOffset, decodeParams.m_dataSize);

            uint32_t totalSize = m_jpegBasicFeature->m_jpegScanParams->ScanHeader[totalScans - 1].DataOffset +
                                 m_jpegBasicFeature->m_jpegScanParams->ScanHeader[totalScans - 1].DataLength;
            if (m_segmentsTotalSize + segmentSize >= totalSize)
            {
                m_completeBitStream = true;
            }
        }
    }
    m_segmentsTotalSize += MOS_ALIGN_CEIL(segmentSize, MHW_CACHELINE_SIZE);
    return MOS_STATUS_SUCCESS;
}


}  // namespace decode

