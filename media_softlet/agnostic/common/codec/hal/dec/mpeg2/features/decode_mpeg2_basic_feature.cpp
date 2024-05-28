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
//! \file     decode_mpeg2_basic_features.cpp
//! \brief    Defines the common interface for decode mpeg2 basic feature
//!
#include "decode_mpeg2_basic_feature.h"
#include "decode_utils.h"
#include "decode_allocator.h"
#include "decode_resource_array.h"

namespace decode{

Mpeg2BasicFeature::~Mpeg2BasicFeature()
{
    if (m_allocator != nullptr)
    {
        if (m_resMpeg2DummyBistream != nullptr)
        {
            m_allocator->Destroy(m_resMpeg2DummyBistream);
        }

        m_allocator->Destroy(m_copiedDataBufArray);
    }
}

MOS_STATUS Mpeg2BasicFeature::Init(void *setting)
{
    DECODE_FUNC_CALL();

    PERF_UTILITY_AUTO(__FUNCTION__, PERF_DECODE, PERF_LEVEL_HAL);
    DECODE_CHK_STATUS(DecodeBasicFeature::Init(setting));

    m_picWidthInMb  = (uint16_t)CODECHAL_GET_WIDTH_IN_MACROBLOCKS(m_width);
    m_picHeightInMb = (uint16_t)CODECHAL_GET_HEIGHT_IN_MACROBLOCKS(m_height);

    DECODE_CHK_STATUS(m_refFrames.Init(this, *m_allocator));

    // Dummy slice buffer
    if (m_mode == CODECHAL_DECODE_MODE_MPEG2VLD)
    {
        uint32_t size = MOS_ALIGN_CEIL(sizeof(Mpeg2DummyBsBuf), 64);
        m_resMpeg2DummyBistream = m_allocator->AllocateBuffer(
                                     size,
                                     "Mpeg2DummyBitstream",
                                     resourceInternalReadWriteCache,
                                     lockableVideoMem);
        auto data = (uint8_t *)m_allocator->LockResourceForWrite(&m_resMpeg2DummyBistream->OsResource);
        DECODE_CHK_NULL(data);

        MOS_ZeroMemory(data, size);
        MOS_SecureMemcpy(data, sizeof(Mpeg2DummyBsBuf), Mpeg2DummyBsBuf, sizeof(Mpeg2DummyBsBuf));
    }

    if (m_mode == CODECHAL_DECODE_MODE_MPEG2IDCT)
    {
        // The common indirect IT-COEFF data structure is defined as a uint32_t.
        m_copiedDataBufferSize = (m_picWidthInMb * m_picHeightInMb + 2) *
            (CODEC_NUM_BLOCK_PER_MB * CODEC_MPEG2_IDCTBLOCK_SIZE * sizeof(uint32_t));
    }
    else
    {
        // Bitstream buffer size = height * width + dummy slice + 512 (for padding)
        m_copiedDataBufferSize = (m_picWidthInMb * m_picHeightInMb * CODEC_MPEG2_BYTES_PER_MB) +
            sizeof(Mpeg2DummyBsBuf) + 512;
    }

    m_copiedDataBufArray = m_allocator->AllocateBufferArray(
        m_copiedDataBufferSize,
        "copied data buffer",
        m_mpeg2NumCopiedBufs,
        resourceInputBitstream,
        notLockableVideoMem);
    DECODE_CHK_NULL(m_copiedDataBufArray);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Mpeg2BasicFeature::Update(void *params)
{
    DECODE_FUNC_CALL();

    PERF_UTILITY_AUTO(__FUNCTION__, PERF_DECODE, PERF_LEVEL_HAL);

    DECODE_CHK_NULL(params);
    DECODE_CHK_STATUS(DecodeBasicFeature::Update(params));

    CodechalDecodeParams* decodeParams = (CodechalDecodeParams*)params;
    DECODE_CHK_NULL(decodeParams->m_picParams);
    m_mpeg2PicParams                      = (CodecDecodeMpeg2PicParams *)decodeParams->m_picParams;
    m_mpeg2SliceParams                    = (CodecDecodeMpeg2SliceParams *)decodeParams->m_sliceParams;
    m_mpeg2IqMatrixBuffer                 = (CodecMpeg2IqMatrix *)decodeParams->m_iqMatrixBuffer;
    m_mpeg2MbParams                       = (CodecDecodeMpeg2MbParams *)decodeParams->m_macroblockParams;
    m_numMacroblocks                      = decodeParams->m_numMacroblocks;
    m_mpeg2ISliceConcealmentMode          = decodeParams->m_mpeg2ISliceConcealmentMode;
    m_mpeg2PbSliceConcealmentMode         = decodeParams->m_mpeg2PBSliceConcealmentMode;
    m_mpeg2PbSlicePredBiDirMvTypeOverride = decodeParams->m_mpeg2PBSlicePredBiDirMVTypeOverride;
    m_mpeg2PbSlicePredMvOverride          = decodeParams->m_mpeg2PBSlicePredMVOverride;
    DECODE_CHK_NULL(m_mpeg2PicParams);

    if (decodeParams->m_executeCallIndex == 0)
    {
        DECODE_CHK_STATUS(ResetFrameValues());
        DECODE_CHK_STATUS(SetPictureStructs());
    }
    else
    {
        DECODE_VERBOSEMESSAGE("there is multiple execution call in current Frame = %d",
            m_mpeg2PicParams->m_currPic.FrameIdx);
    }

    if (m_mode == CODECHAL_DECODE_MODE_MPEG2VLD)
    {
        DECODE_CHK_STATUS(SetSliceStructs());
    }
    else if (m_mode == CODECHAL_DECODE_MODE_MPEG2IDCT)
    {
        DECODE_CHK_STATUS(SetMbStructs());
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Mpeg2BasicFeature::ResetFrameValues()
{
    DECODE_FUNC_CALL();

    m_copiedDataBuf = m_copiedDataBufArray->Fetch();
    DECODE_CHK_NULL(m_copiedDataBuf);

    m_incompletePicture     = false;
    m_copiedDataBufferInUse = false;
    m_copiedDataOffset      = 0;
    m_nextCopiedDataOffset  = 0;
    m_lastMbAddress         = 0;

    if (m_mode == CODECHAL_DECODE_MODE_MPEG2VLD)
    {
        m_slicesInvalid          = false;
        m_copyDummySlicePresent  = false;
        m_totalNumSlicesRecv     = 0;
        m_lastSliceMbEnd         = 0;
    }
    else
    {
        m_totalNumMbsRecv = 0;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Mpeg2BasicFeature::SetPictureStructs()
{
    DECODE_FUNC_CALL();

    m_secondField = m_mpeg2PicParams->m_secondField ? true : false;
    m_pictureCodingType = (uint16_t)m_mpeg2PicParams->m_pictureCodingType;
    m_curRenderPic = m_mpeg2PicParams->m_currPic;

    m_picWidthInMb  = CODECHAL_GET_WIDTH_IN_MACROBLOCKS(m_mpeg2PicParams->m_horizontalSize);
    m_picHeightInMb = CODECHAL_GET_HEIGHT_IN_MACROBLOCKS(m_mpeg2PicParams->m_verticalSize);

    // For some corrupted mpeg2 streams, need to use dwHeight and dwWidth because they are updated at decode
    // creation time. Horizontal_size or vertical_size may be different and wrong in first picture parameter.
    uint32_t totalMBInFrame = (m_curRenderPic.PicFlags == PICTURE_FRAME) ?
        (CODECHAL_GET_WIDTH_IN_MACROBLOCKS(m_width) * CODECHAL_GET_HEIGHT_IN_MACROBLOCKS(m_height)):
        (CODECHAL_GET_WIDTH_IN_MACROBLOCKS(m_width) * CODECHAL_GET_HEIGHT_IN_MACROBLOCKS(m_height) / 2);

    if (m_numSlices > totalMBInFrame)
    {
        DECODE_ASSERTMESSAGE("Invalid slice number due to larger than MB number.");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    m_refFrameIndexList.clear();
    m_fwdRefIdx = (uint8_t)m_mpeg2PicParams->m_forwardRefIdx;
    m_bwdRefIdx = (uint8_t)m_mpeg2PicParams->m_backwardRefIdx;
    m_refFrameIndexList.push_back(m_fwdRefIdx);
    m_refFrameIndexList.push_back(m_bwdRefIdx);

    DECODE_CHK_STATUS(m_refFrames.UpdatePicture(*m_mpeg2PicParams));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Mpeg2BasicFeature::SetSliceStructs()
{
    DECODE_FUNC_CALL();

    uint32_t numMBs              = 0;
    uint32_t firstMbAddress      = 0;
    bool     skippedSlicePresent = false;
    bool     invalidFrame        = true;
    uint16_t lastSlice           = 0;

    if (m_numSlices == 0)
    {
        DECODE_ASSERTMESSAGE("MPEG2 VLD slice data invalid, unable to determine final MB address.");
        numMBs = 0;
        return MOS_STATUS_INVALID_PARAMETER;
    }

    DECODE_CHK_NULL(m_mpeg2SliceParams);
    CodecDecodeMpeg2SliceParams *slc = m_mpeg2SliceParams;

    // For a incomplete picture, this indicates if it is the first
    // vaild slice from second execution call.
    bool firstValidSlice = m_incompletePicture ? true : false;

    uint16_t startSliceIdx = m_totalNumSlicesRecv;
    m_totalNumSlicesRecv  += (uint16_t)m_numSlices;

    if (m_sliceRecord.size() < m_totalNumSlicesRecv)
    {
        m_sliceRecord.resize(m_totalNumSlicesRecv);
    }

    uint32_t prevSliceMbEnd = m_lastSliceMbEnd;

    for (uint16_t slcIdx = startSliceIdx; slcIdx < m_totalNumSlicesRecv; slcIdx++)
    {
        uint32_t sliceStartMbOffset = slc->m_sliceHorizontalPosition +
            (slc->m_sliceVerticalPosition * m_picWidthInMb);
        uint32_t slcLength = ((slc->m_sliceDataSize + 0x0007) >> 3);

        // HW limitation and won't be fixed
        if (slcLength > 0x1FFE0)
        {
            slcLength = 0x1FFE0;
        }

        // #of bytes of header data in bitstream buffer (before video data)
        uint32_t u32Offset = ((slc->m_macroblockOffset & 0x0000fff8) >> 3);

        slcLength                               -= u32Offset;
        m_sliceRecord[slcIdx].length             = slcLength;
        m_sliceRecord[slcIdx].offset             = u32Offset + m_copiedDataOffset;
        m_sliceRecord[slcIdx].sliceStartMbOffset = sliceStartMbOffset;
        m_sliceRecord[slcIdx].prevSliceMbEnd     = prevSliceMbEnd;
        m_sliceRecord[slcIdx].recordSliceParam   = *slc;
        m_sliceRecord[slcIdx].skip               = false;
        m_sliceRecord[slcIdx].isLastSlice        = false;

        if (DetectSliceError(*slc, slcIdx, firstValidSlice))
        {
            m_sliceRecord[slcIdx].skip = true;
            skippedSlicePresent = true;
        }
        else
        {
            if (firstValidSlice)
            {
                // First MB Address
                firstMbAddress = slc->m_sliceHorizontalPosition + (slc->m_sliceVerticalPosition * m_picWidthInMb);
            }

            lastSlice = slcIdx;
            firstValidSlice = false;
            invalidFrame = false;
        }

        prevSliceMbEnd = m_sliceRecord[slcIdx].sliceStartMbOffset + slc->m_numMbsForSlice;

        if (slcIdx == m_totalNumSlicesRecv - 1)
        {
            m_lastSliceMbEnd = prevSliceMbEnd;
        }

        slc++;
    }

    slc -= m_numSlices;
    m_sliceRecord[lastSlice].isLastSlice = true;

    // Last MB Address
    slc += lastSlice;
    numMBs = (uint16_t)(m_sliceRecord[lastSlice].sliceStartMbOffset + slc->m_numMbsForSlice);

    // It means all slices in the frame are wrong, we just need skip decoding
    // for this frame and won't insert any dummy slices.
    if (invalidFrame && m_numSlices && !m_incompletePicture)
    {
        DECODE_ASSERTMESSAGE("Current frame is invalid.");
        return MOS_STATUS_UNKNOWN;
    }

    // MPEG2 Error Concealment for VLD mode from Gen6+
    m_copiedDataNeeded = m_incompletePicture ||
        (numMBs != (m_picWidthInMb * m_picHeightInMb)) ||
        (skippedSlicePresent || (firstMbAddress != 0));

    DECODE_CHK_STATUS(CheckCompletePicture(startSliceIdx));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Mpeg2BasicFeature::CheckCompletePicture(uint16_t startSliceIdx)
{
    DECODE_FUNC_CALL();

    CodecDecodeMpeg2SliceParams *slc = m_mpeg2SliceParams;
    DECODE_CHK_NULL(slc)

    for (uint16_t slcIdx = startSliceIdx; slcIdx < m_totalNumSlicesRecv; slcIdx++)
    {
        if (!m_sliceRecord[slcIdx].skip && m_sliceRecord[slcIdx].isLastSlice)
        {
            uint16_t expectedFinalMb = m_picWidthInMb * m_picHeightInMb;
            m_lastMbAddress = m_sliceRecord[slcIdx].sliceStartMbOffset + slc->m_numMbsForSlice;
            if (m_lastMbAddress < expectedFinalMb)
            {
                m_incompletePicture = true;
                m_sliceRecord[slcIdx].isLastSlice = false;
            }
            else
            {
                m_incompletePicture = false;
            }
        }
        slc++;
    }

    return MOS_STATUS_SUCCESS;
}

bool Mpeg2BasicFeature::DetectSliceError(CodecDecodeMpeg2SliceParams &slc,
                                         uint32_t slcIdx,
                                         bool firstValidSlice)
{
    bool result  = false;

    VldSliceRecord currSliceRecord = m_sliceRecord[slcIdx];

    if (currSliceRecord.length == 0 || currSliceRecord.length > (uint32_t)(1 << (sizeof(uint32_t) * 8 - 1)))
    {
        result = true;
    }
    else if ((slc.m_sliceDataOffset + currSliceRecord.length) > m_dataSize)
    {
        // error handling for garbage data
        result = true;
    }
    else if (m_slicesInvalid)
    {
        // If cp copy failed for this interation due to lack of buffer space, cannot use any
        // slices in this iteration since there is no bitstream data for them
        result = true;
    }
    else if (currSliceRecord.prevSliceMbEnd > currSliceRecord.sliceStartMbOffset ||
            slc.m_sliceVerticalPosition >= m_picHeightInMb ||
            slc.m_sliceHorizontalPosition >= m_picWidthInMb)
    {
        result = true;
        m_slicesInvalid = true;
    }
    else if (!slc.m_numMbsForSlice)
    {
        // For SNB HW will not process BSD objects where the MbCount is 0, and if an invalid
        // slice start position has been detected or the number of MBs in the current slice
        // is invalid, the rest of the data may be garbage since SNB does not have robust
        // error concealment for MPEG2 VLD, skipping these slices prevents HW going to hang
        result = true;
        m_slicesInvalid = true;
    }
    else if (slc.m_numMbsForSliceOverflow)
    {
        // Special case for last slice of an incomplete frame
        if ((slcIdx == m_totalNumSlicesRecv - 1) && !firstValidSlice &&
            ((currSliceRecord.sliceStartMbOffset + slc.m_numMbsForSlice) <
            (uint32_t)(m_picHeightInMb * m_picWidthInMb)))
        {
            slc.m_numMbsForSlice = m_picWidthInMb;
        }
        else
        {
            result = true;
            m_slicesInvalid = true;
        }
    }

    return result;
}

MOS_STATUS Mpeg2BasicFeature::SetMbStructs()
{
    DECODE_FUNC_CALL();

    DECODE_CHK_NULL(m_mpeg2MbParams);
    CodecDecodeMpeg2MbParams *mb = m_mpeg2MbParams;

    //MPEG2 Error Concealment for IT mode from Gen6+
    m_copiedDataNeeded = (m_incompletePicture || (m_numMacroblocks != (m_picWidthInMb * m_picHeightInMb)));

    uint32_t startMbIdx = m_totalNumMbsRecv;
    m_totalNumMbsRecv += m_numMacroblocks;

    if (m_mbRecord.size() < m_totalNumMbsRecv)
    {
        m_mbRecord.resize(m_totalNumMbsRecv);
    }

    uint16_t expectedMBAddress = (m_incompletePicture) ? m_lastMbAddress : 0;

    for (uint32_t mbIdx = startMbIdx; mbIdx < m_totalNumMbsRecv; mbIdx++)
    {
        uint16_t skippedMBs = 0;

        if (mb[mbIdx].m_mbAddr >= expectedMBAddress)
        {
             uint16_t skippedMBs = mb[mbIdx].m_mbAddr - expectedMBAddress;
        }

        m_mbRecord[mbIdx].recordMbParam  = mb[mbIdx];
        m_mbRecord[mbIdx].skippedMBs     = skippedMBs;
        m_mbRecord[mbIdx].expectedMBAddr = expectedMBAddress;

        expectedMBAddress = mb[mbIdx].m_mbAddr + 1;

        if (m_pictureCodingType != I_TYPE && mb[mbIdx].m_mbSkipFollowing)
        {
            uint16_t skippedMBs = mb[mbIdx].m_mbSkipFollowing;
            expectedMBAddress += skippedMBs;
        }

        // insert extra MBs to ensure expected number of MBs sent to HW
        if (mbIdx + 1 == m_numMacroblocks)
        {
            uint16_t expectedFinalMB = m_picWidthInMb * m_picHeightInMb;
            if (expectedMBAddress != expectedFinalMB)
            {
                m_incompletePicture = true;
                m_lastMbAddress     = expectedMBAddress;
            }
            else
            {
                m_incompletePicture = false;
            }
        }
    }

    return MOS_STATUS_SUCCESS;
}

}
