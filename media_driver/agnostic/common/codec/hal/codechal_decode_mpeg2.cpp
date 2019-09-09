/*
* Copyright (c) 2011-2018, Intel Corporation
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
//! \file     codechal_decode_mpeg2.cpp
//! \brief    Implements the decode interface extension for MPEG2.
//! \details  Implements all functions and constants required by CodecHal for MPEG2 decoding.
//!

#include "codechal_decoder.h"
#include "codechal_decode_mpeg2.h"
#include "codechal_secure_decode_interface.h"
#include "codechal_mmc_decode_mpeg2.h"
#include "hal_oca_interface.h"
#if USE_CODECHAL_DEBUG_TOOL
#include <sstream>
#include <fstream>
#include "codechal_debug.h"
#endif

#define CODECHAL_DECODE_MPEG2_IDCTBLOCK_SIZE   64

// One MB size (16x16) intra MB, color = RGB[4, 4, 4]
const uint32_t CODECHAL_DECODE_MPEG2_WaDummyBitstream[] =
{0x01010000, 0x54e29752, 0x002022a5, //Dummy Slice 0, q_scale_type = 0, intra_vlc_format = 0
 0x01010000, 0x4de29752, 0x8661341a, //Dummy Slice 1, q_scale_type = 0, intra_vlc_format = 1
 0x01010000, 0x54e2a772, 0x002022a5, //Dummy Slice 2, q_scale_type = 1, intra_vlc_format = 0
 0x01010000, 0x4de2a772, 0x8661341a  //Dummy Slice 3, q_scale_type = 1, intra_vlc_format = 1
};

const uint32_t CODECHAL_DECODE_MPEG2_WaDummySliceLengths[] = {0x8, 0x8, 0x8, 0x8};
const uint32_t CODECHAL_DECODE_MPEG2_WaDummySliceOffsets[] = {0x4, 0x10, 0x1c, 0x28};

bool CodechalDecodeMpeg2::DetectSliceError(
    uint16_t                        slcNum,
    uint32_t                        prevSliceMbEnd,
    bool                            firstValidSlice)
{
    bool                        result         = false;
    CODECHAL_VLD_SLICE_RECORD   currSliceRecord = m_vldSliceRecord[slcNum];

    if (currSliceRecord.dwLength == 0 || currSliceRecord.dwLength > (uint32_t)(1 << (sizeof(uint32_t) * 8 - 1)))
    {
        result = true;
    }
    else if ((m_sliceParams[slcNum].m_sliceDataOffset + currSliceRecord.dwLength) >
             m_dataSize)
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
    else if (prevSliceMbEnd > currSliceRecord.dwSliceStartMbOffset ||
             m_sliceParams[slcNum].m_sliceVerticalPosition >= m_picHeightInMb ||
             m_sliceParams[slcNum].m_sliceHorizontalPosition >= m_picWidthInMb)
    {
        result = true;
        m_slicesInvalid = true;
    }
    else if (!m_sliceParams[slcNum].m_numMbsForSlice)
    {
        // For SNB HW will not process BSD objects where the MbCount is 0, and if an invalid
        // slice start position has been detected or the number of MBs in the current slice
        // is invalid, the rest of the data may be garbage since SNB does not have
        // robust error concealment for MPEG2 VLD, skipping these slices prevents HW hangs

        result = true;
        m_slicesInvalid = true;
    }
    else if (m_sliceParams[slcNum].m_numMbsForSliceOverflow)
    {
        // Special case for last slice of an incomplete frame
        if ((slcNum == m_numSlices - 1) &&
            !firstValidSlice &&
            ((currSliceRecord.dwSliceStartMbOffset + m_sliceParams[slcNum].m_numMbsForSlice) <
                (uint32_t)(m_picHeightInMb * m_picWidthInMb)))
        {
            m_sliceParams[slcNum].m_numMbsForSlice = m_picWidthInMb;
        }
        else
        {
            result = true;
            m_slicesInvalid = true;
        }
    }

    return result;
}

MOS_STATUS CodechalDecodeMpeg2::InsertDummySlices(
    PMHW_BATCH_BUFFER               batchBuffer,
    uint16_t                        startMB,
    uint16_t                        endMB)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;
    bool      cpEnable = false;

    CODECHAL_DECODE_FUNCTION_ENTER;

    CODECHAL_DECODE_CHK_NULL_RETURN(batchBuffer);
    CODECHAL_DECODE_CHK_NULL_RETURN(m_osInterface);
    CODECHAL_DECODE_CHK_NULL_RETURN(m_osInterface->osCpInterface);

    //  A copied data buffer must be present
    if (m_nextCopiedDataOffset && !m_dummySliceDataPresent)
    {
        // slice header uses 4 bytes
        CODECHAL_DECODE_CHK_STATUS_RETURN(CopyDataSurface(
            sizeof(CODECHAL_DECODE_MPEG2_WaDummyBitstream),
            m_resMpeg2DummyBistream,
            &m_resCopiedDataBuffer[m_currCopiedData],
            &m_dummySliceDataOffset));

        m_dummySliceDataPresent = true;
    }

    // force disable cp for dummy slices
    cpEnable = m_osInterface->osCpInterface->IsCpEnabled();
    if (cpEnable)
    {
        m_osInterface->osCpInterface->SetCpEnabled(false);
    }

    uint16_t intraVLDFormat                = m_picParams->W0.m_intraVlcFormat;
    uint16_t quantizerScaleType            = m_picParams->W0.m_quantizerScaleType;
    uint16_t dummySliceIndex               = quantizerScaleType * 2 + intraVLDFormat;

    MHW_VDBOX_MPEG2_SLICE_STATE         mpeg2SliceState;
    mpeg2SliceState.presDataBuffer      = nullptr;
    mpeg2SliceState.wPicWidthInMb       = m_picWidthInMb;
    mpeg2SliceState.wPicHeightInMb      = m_picHeightInMb;
    mpeg2SliceState.dwLength            =
        CODECHAL_DECODE_MPEG2_WaDummySliceLengths[dummySliceIndex];
    mpeg2SliceState.dwOffset =
        m_dummySliceDataOffset + CODECHAL_DECODE_MPEG2_WaDummySliceOffsets[dummySliceIndex];

    bool isLastSlice                   = false;
    uint16_t expectedEndMB                 = m_picWidthInMb * m_picHeightInMb;

    CodecDecodeMpeg2SliceParams slc;
    MOS_ZeroMemory(&slc, sizeof(CodecDecodeMpeg2SliceParams));

    while (startMB < endMB)
    {
        slc.m_macroblockOffset           = 6;
        slc.m_sliceHorizontalPosition     = startMB % m_picWidthInMb;
        slc.m_sliceVerticalPosition       = startMB / m_picWidthInMb;
        slc.m_quantiserScaleCode        = 10;
        slc.m_numMbsForSlice              = 1;

        isLastSlice = ((startMB + 1) == expectedEndMB);

        mpeg2SliceState.pMpeg2SliceParams       = &slc;
        mpeg2SliceState.dwSliceStartMbOffset    = startMB;
        mpeg2SliceState.bLastSlice              = isLastSlice;

        CODECHAL_DECODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfdMpeg2BsdObject(
            nullptr,
            batchBuffer,
            &mpeg2SliceState));

        startMB++;
    }

    // restore Cp state
    if (cpEnable)
    {
        m_osInterface->osCpInterface->SetCpEnabled(true);
    }

    if (isLastSlice)
    {
        m_incompletePicture = false;
    }

    return eStatus;
}

MOS_STATUS CodechalDecodeMpeg2::CopyDataSurface(
    uint32_t                        dataSize,
    MOS_RESOURCE                    sourceSurface,
    PMOS_RESOURCE                   copiedSurface,
    uint32_t                        *currOffset)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    if (Mos_ResourceIsNull(copiedSurface))
    {
        // Allocate resCopiedDataBuffer if not already allocated.
        CODECHAL_DECODE_CHK_STATUS_MESSAGE_RETURN(AllocateBuffer(
                                                      copiedSurface,
                                                      m_copiedDataBufferSize,
                                                      "CopiedDataBuffer"),
            "Failed to allocate copied residual data buffer.");
    }

    if ((m_nextCopiedDataOffset + dataSize) > m_copiedDataBufferSize)
    {
        CODECHAL_DECODE_ASSERTMESSAGE("Copied data buffer is not large enough.");

        m_slicesInvalid = true;
        return MOS_STATUS_UNKNOWN;
    }

    uint32_t size = MOS_ALIGN_CEIL(dataSize, 16); // 16 byte aligned

    // HuC is not supported on BDW
    if (m_hwInterface->m_noHuC)
    {
        CodechalDataCopyParams dataCopyParams;
        MOS_ZeroMemory(&dataCopyParams, sizeof(CodechalDataCopyParams));
        dataCopyParams.srcResource = &sourceSurface;
        dataCopyParams.srcSize = size;
        dataCopyParams.srcOffset = 0;
        dataCopyParams.dstResource = copiedSurface;
        dataCopyParams.dstSize = size;
        dataCopyParams.dstOffset   = m_nextCopiedDataOffset;

        CODECHAL_DECODE_CHK_STATUS_RETURN(m_hwInterface->CopyDataSourceWithDrv(
            &dataCopyParams));

        *currOffset = m_nextCopiedDataOffset;
        m_nextCopiedDataOffset += MOS_ALIGN_CEIL(size, MHW_CACHELINE_SIZE);  // 64-byte aligned
        return MOS_STATUS_SUCCESS;
    }

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnSetGpuContext(
        m_osInterface,
        m_videoContextForWa));
    m_osInterface->pfnResetOsStates(m_osInterface);

    m_osInterface->pfnSetPerfTag(
        m_osInterface,
        (uint16_t)(((m_mode << 4) & 0xF0) | COPY_TYPE));
    m_osInterface->pfnResetPerfBufferID(m_osInterface);

    MOS_COMMAND_BUFFER cmdBuffer;
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnGetCommandBuffer(
        m_osInterface,
        &cmdBuffer,
        0));

    CODECHAL_DECODE_CHK_STATUS_RETURN(SendPrologWithFrameTracking(
        &cmdBuffer,
        false));

    // Use huc stream out to do the copy
    CODECHAL_DECODE_CHK_STATUS_RETURN(HucCopy(
        &cmdBuffer,                // pCmdBuffer
        &sourceSurface,            // presSrc
        copiedSurface,             // presDst
        size,                      // u32CopyLength
        0,                         // u32CopyInputOffset
        m_nextCopiedDataOffset));  // u32CopyOutputOffset

    MHW_MI_FLUSH_DW_PARAMS flushDwParams;
    MOS_ZeroMemory(&flushDwParams, sizeof(flushDwParams));
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiFlushDwCmd(
        &cmdBuffer,
        &flushDwParams));

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferEnd(
        &cmdBuffer,
        nullptr));

    m_osInterface->pfnReturnCommandBuffer(m_osInterface, &cmdBuffer, 0);

    if (!m_incompletePicture)
    {
        MOS_SYNC_PARAMS syncParams      = g_cInitSyncParams;
        syncParams.GpuContext           = m_videoContext;
        syncParams.presSyncResource     = &m_resSyncObjectVideoContextInUse;

        CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnEngineSignal(m_osInterface, &syncParams));

        syncParams                      = g_cInitSyncParams;
        syncParams.GpuContext           = m_videoContextForWa;
        syncParams.presSyncResource     = &m_resSyncObjectVideoContextInUse;

        CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnEngineWait(m_osInterface, &syncParams));
    }

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnSubmitCommandBuffer(
        m_osInterface,
        &cmdBuffer,
        m_videoContextForWaUsesNullHw));

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnSetGpuContext(
        m_osInterface,
        m_videoContext));

    *currOffset = m_nextCopiedDataOffset;
    m_nextCopiedDataOffset += MOS_ALIGN_CEIL(size, 64);  // offsets are 64-byte aligned

    return eStatus;
}

/*--------------------------------------------------------------------------
| Name      : CodecHalMpeg2Decode_InitializeBeginFrame
| Purpose   : Initialize MPEG2 incomplete frame values in the DecodeBeginFrame
| Arguments : IN -> pMpeg2State    : the pointer to mpeg2 state
IN -> pDecoder       : the pointer to codechal decoder structure
| Returns   : MOS_STATUS_SUCCESS : Function successfully completed its task
|             MOS_STATUS_UNKNOWN: Error condition encountered (frame will not be rendered)
\---------------------------------------------------------------------------*/
MOS_STATUS CodechalDecodeMpeg2::InitializeBeginFrame()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    m_incompletePicture  = false;
    m_copiedDataBufferInUse = false;
    m_copiedDataOffset      = 0;
    m_nextCopiedDataOffset  = 0;
    m_lastMbAddress         = 0;
    m_slicesInvalid         = false;

    m_currCopiedData =
        (m_currCopiedData + 1) % CODECHAL_DECODE_MPEG2_COPIED_SURFACES;
    m_dummySliceDataPresent = false;
    m_dummySliceDataOffset  = 0;
    m_bbInUsePerFrame       = 0;

    return eStatus;
}

MOS_STATUS CodechalDecodeMpeg2::AllocateResources ()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnCreateSyncResource(
        m_osInterface,
        &m_resSyncObjectWaContextInUse));
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnCreateSyncResource(
        m_osInterface,
        &m_resSyncObjectVideoContextInUse));

    CodecHalAllocateDataList(
        m_mpeg2RefList,
        CODECHAL_NUM_UNCOMPRESSED_SURFACE_MPEG2);

    for (uint32_t i = 0; i < CODECHAL_NUM_UNCOMPRESSED_SURFACE_MPEG2; i++)
    {
        m_mpeg2RefList[i]->RefPic.PicFlags = PICTURE_INVALID;
    }

    uint32_t numMacroblocks = m_picWidthInMb * m_picHeightInMb;

    if (m_mode == CODECHAL_DECODE_MODE_MPEG2VLD)
    {
        m_vldSliceRecord =
            (PCODECHAL_VLD_SLICE_RECORD)MOS_AllocAndZeroMemory(numMacroblocks * sizeof(CODECHAL_VLD_SLICE_RECORD));
        CODECHAL_DECODE_CHK_NULL_RETURN(m_vldSliceRecord);
    }

    for (uint16_t i = 0; i < m_bbAllocated; i++)
    {
        MOS_ZeroMemory(&m_mediaObjectBatchBuffer[i], sizeof(MHW_BATCH_BUFFER));
        uint32_t size = (m_standardDecodeSizeNeeded * numMacroblocks) + m_hwInterface->m_sizeOfCmdBatchBufferEnd;
        CODECHAL_DECODE_CHK_STATUS_RETURN(Mhw_AllocateBb(
            m_osInterface,
            &m_mediaObjectBatchBuffer[i],
            nullptr,
            size));
    }

    // Deblocking Filter Row Store Scratch buffer
    //(Num MacroBlock Width) * (Num Cachlines) * (Cachline size)
    CODECHAL_DECODE_CHK_STATUS_MESSAGE_RETURN(AllocateBuffer(
                                                  &m_resMfdDeblockingFilterRowStoreScratchBuffer,
                                                  m_picWidthInMb * 7 * CODECHAL_CACHELINE_SIZE,
                                                  "DeblockingFilterScratch"),
        "Failed to allocate BSD/MPC Row Store Scratch Buffer.");

    // MPR Row Store Scratch buffer
    // (FrameWidth in MB) * (CacheLine size per MB) * 2
    // IVB+ platforms need to have double MPR size for MBAFF
    CODECHAL_DECODE_CHK_STATUS_MESSAGE_RETURN(AllocateBuffer(
                                                  &m_resBsdMpcRowStoreScratchBuffer,
                                                  ((uint32_t)(m_picWidthInMb * CODECHAL_CACHELINE_SIZE)) * 2,
                                                  "MprScratchBuffer"),
        "Failed to allocate AVC BSD MPR Row Store Scratch Buffer.");

    m_consecutiveMbErrorConcealmentInUse = true;

    // Dummy slice buffer
    if (m_mode == CODECHAL_DECODE_MODE_MPEG2VLD)
    {
        uint32_t size = MOS_ALIGN_CEIL(sizeof(CODECHAL_DECODE_MPEG2_WaDummyBitstream), 64);
        CODECHAL_DECODE_CHK_STATUS_MESSAGE_RETURN(AllocateBuffer(
                                                      &m_resMpeg2DummyBistream,
                                                      size,
                                                      "Mpeg2DummyBitstream"),
            "Failed to allocate MPEG2 bitstream buffer for format switching WA.");

        CodechalResLock DummyBitstreamLock(m_osInterface, &m_resMpeg2DummyBistream);
        auto data = DummyBitstreamLock.Lock(CodechalResLock::writeOnly);
        
        CODECHAL_DECODE_CHK_NULL_RETURN(data);

        MOS_ZeroMemory(data, size);
        CODECHAL_DECODE_CHK_STATUS_MESSAGE_RETURN(MOS_SecureMemcpy(
            data,
            size,
            (void *)CODECHAL_DECODE_MPEG2_WaDummyBitstream,
            sizeof(CODECHAL_DECODE_MPEG2_WaDummyBitstream)),
            "Failed to copy memory.");

    }

    if (m_mode == CODECHAL_DECODE_MODE_MPEG2IDCT)
    {
        // The common indirect IT-COEFF data structure is defined as a uint32_t.
        m_copiedDataBufferSize = (numMacroblocks + 2) *
                                 (CODEC_NUM_BLOCK_PER_MB * CODECHAL_DECODE_MPEG2_IDCTBLOCK_SIZE * sizeof(uint32_t));
    }
    else
    {
        // Bitstream buffer size = height * width + dummy slice + 512 (for padding)
        m_copiedDataBufferSize =
            (m_picWidthInMb * m_picHeightInMb * CODECHAL_DECODE_MPEG2_BYTES_PER_MB) +
            sizeof(CODECHAL_DECODE_MPEG2_WaDummyBitstream) +
            512;
    }

    return eStatus;
}

CodechalDecodeMpeg2::~CodechalDecodeMpeg2 ()
{
    CODECHAL_DECODE_FUNCTION_ENTER;

    CODECHAL_DECODE_CHK_NULL_NO_STATUS_RETURN(m_osInterface);

    m_osInterface->pfnDestroySyncResource(m_osInterface, &m_resSyncObjectWaContextInUse);
    m_osInterface->pfnDestroySyncResource(m_osInterface, &m_resSyncObjectVideoContextInUse);

    CodecHalFreeDataList(m_mpeg2RefList, CODECHAL_NUM_UNCOMPRESSED_SURFACE_MPEG2);

    MOS_FreeMemory(m_vldSliceRecord);

    for (uint32_t i = 0; i < m_bbAllocated; i++)
    {
        Mhw_FreeBb(m_osInterface, &m_mediaObjectBatchBuffer[i], nullptr);
    }

    m_osInterface->pfnFreeResource(
        m_osInterface,
        &m_resMfdDeblockingFilterRowStoreScratchBuffer);

    m_osInterface->pfnFreeResource(
        m_osInterface,
        &m_resBsdMpcRowStoreScratchBuffer);

    // Dummy slice buffer
    if (!Mos_ResourceIsNull(&m_resMpeg2DummyBistream))
    {
        m_osInterface->pfnFreeResource(
            m_osInterface,
            &m_resMpeg2DummyBistream);
    }

    for (uint32_t i = 0; i < CODECHAL_DECODE_MPEG2_COPIED_SURFACES; i++)
    {
        m_osInterface->pfnFreeResource(
            m_osInterface,
            &m_resCopiedDataBuffer[i]);
    }
}

MOS_STATUS CodechalDecodeMpeg2::SetFrameStates ()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    uint32_t totalMBInFrame = 0;

    CODECHAL_DECODE_CHK_NULL_RETURN(m_decodeParams.m_destSurface);
    CODECHAL_DECODE_CHK_NULL_RETURN(m_decodeParams.m_dataBuffer);
    CODECHAL_DECODE_CHK_NULL_RETURN(m_decodeParams.m_picParams);

    m_dataSize                            = m_decodeParams.m_dataSize;
    m_dataOffset                          = m_decodeParams.m_dataOffset;
    m_numSlices                           = m_decodeParams.m_numSlices;
    m_picParams                           = (CodecDecodeMpeg2PicParams *)m_decodeParams.m_picParams;
    m_sliceParams                         = (CodecDecodeMpeg2SliceParams *)m_decodeParams.m_sliceParams;
    m_iqMatrixBuffer                      = (CodecMpeg2IqMatrix *)m_decodeParams.m_iqMatrixBuffer;
    m_destSurface                         = *m_decodeParams.m_destSurface;
    m_resDataBuffer                       = *m_decodeParams.m_dataBuffer;
    m_numMacroblocks                      = m_decodeParams.m_numMacroblocks;
    m_mbParams                            = (CodecDecodeMpeg2MbParmas *)m_decodeParams.m_macroblockParams;
    m_mpeg2ISliceConcealmentMode          = m_decodeParams.m_mpeg2ISliceConcealmentMode;
    m_mpeg2PbSliceConcealmentMode         = m_decodeParams.m_mpeg2PBSliceConcealmentMode;
    m_mpeg2PbSlicePredBiDirMvTypeOverride = m_decodeParams.m_mpeg2PBSlicePredBiDirMVTypeOverride;
    m_mpeg2PbSlicePredMvOverride          = m_decodeParams.m_mpeg2PBSlicePredMVOverride;

    m_picWidthInMb  = CODECHAL_GET_WIDTH_IN_MACROBLOCKS(m_picParams->m_horizontalSize);
    m_picHeightInMb = CODECHAL_GET_HEIGHT_IN_MACROBLOCKS(m_picParams->m_verticalSize);

    // For some corrupted mpeg2 streams, need to use dwHeight or dwWidth because they are updated at decode create time.
    // Horizontal_size or vertical_size may be different and wrong in first pic parameter.
    totalMBInFrame = (m_picParams->m_currPic.PicFlags == PICTURE_FRAME) ? (CODECHAL_GET_WIDTH_IN_MACROBLOCKS(m_width) * CODECHAL_GET_HEIGHT_IN_MACROBLOCKS(m_height))
                                                                        : (CODECHAL_GET_WIDTH_IN_MACROBLOCKS(m_width) * CODECHAL_GET_HEIGHT_IN_MACROBLOCKS(m_height) / 2);

    if (m_numSlices > totalMBInFrame)
    {
        CODECHAL_DECODE_ASSERTMESSAGE("Invalid slice number due to larger than MB number.");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    PCODEC_REF_LIST destEntry = m_mpeg2RefList[m_picParams->m_currPic.FrameIdx];
    MOS_ZeroMemory(destEntry, sizeof(CODEC_REF_LIST));
    destEntry->RefPic    = m_picParams->m_currPic;
    destEntry->resRefPic = m_destSurface.OsResource;

    m_statusReportFeedbackNumber = m_picParams->m_statusReportFeedbackNumber;

    MOS_ZeroMemory(m_vldSliceRecord, (m_numSlices * sizeof(CODECHAL_VLD_SLICE_RECORD)));

    if (IsFirstExecuteCall())
    {
        CODECHAL_DECODE_CHK_STATUS_MESSAGE_RETURN(InitializeBeginFrame(),
            "Initial Beginframe in CodecHal failed.");
    }

    uint32_t    firstMbAddress       = 0;
    bool        skippedSlicePresent    = false;
    bool        invalidFrame           = true;
    uint32_t    numMBs               = 0;

    // Determine the number of MBs for MPEG2 VLD
    if (m_mode == CODECHAL_DECODE_MODE_MPEG2VLD)
    {
        CODECHAL_DECODE_CHK_NULL_RETURN(m_sliceParams);

        CodecDecodeMpeg2SliceParams *slc = m_sliceParams;

        if (m_numSlices == 0)
        {
            CODECHAL_DECODE_ASSERTMESSAGE(
                "MPEG2 VLD slice data invalid, unable to determine final MB address.");
            numMBs = m_lastMbAddress = 0;
            return MOS_STATUS_INVALID_PARAMETER;
        }
        else
        {
            uint16_t lastSlice       = 0;
            uint32_t prevSliceMbEnd        = m_lastMbAddress;
            bool firstValidSlice       = m_incompletePicture ? true : false;

            for (uint16_t i = 0; i < m_numSlices; i++)
            {
                uint32_t sliceStartMbOffset =
                    slc->m_sliceHorizontalPosition +
                    (slc->m_sliceVerticalPosition * m_picWidthInMb);

                uint32_t slcLength = ((slc->m_sliceDataSize + 0x0007) >> 3);

                // HW limitation and won't be fixed
                if (slcLength > 0x1FFE0)
                {
                    slcLength = 0x1FFE0;
                }

                uint32_t u32Offset = ((slc ->m_macroblockOffset & 0x0000fff8) >> 3); // #of bytes of header data in bitstream buffer (before video data)

                slcLength                            -= u32Offset;
                m_vldSliceRecord[i].dwLength             = slcLength;
                m_vldSliceRecord[i].dwOffset             = u32Offset;
                m_vldSliceRecord[i].dwSliceStartMbOffset = sliceStartMbOffset;

                if (DetectSliceError(i, prevSliceMbEnd, firstValidSlice))
                {
                    m_vldSliceRecord[i].dwSkip = true;
                    skippedSlicePresent        = true;
                }
                else
                {
                    if (firstValidSlice)
                    {
                        // First MB Address
                        firstMbAddress =
                            slc->m_sliceHorizontalPosition +
                            (slc->m_sliceVerticalPosition * m_picWidthInMb);
                    }

                    lastSlice = i;
                    firstValidSlice = false;
                    invalidFrame = false;
                }

                prevSliceMbEnd =
                    m_vldSliceRecord[i].dwSliceStartMbOffset +
                    slc->m_numMbsForSlice;

                slc++;
            }

            slc -= m_numSlices;
            m_vldSliceRecord[lastSlice].bIsLastSlice = true;

            // Last MB Address
            slc += lastSlice;

            numMBs =
                (uint16_t)(m_vldSliceRecord[lastSlice].dwSliceStartMbOffset +
                           slc->m_numMbsForSlice);
        }
    }
    else if (m_mode == CODECHAL_DECODE_MODE_MPEG2IDCT)
    {
        CODECHAL_DECODE_CHK_NULL_RETURN(m_mbParams);
        numMBs = m_numMacroblocks;
    }

    // It means all slices in the frame are wrong, we just need skip decoding for this frame
    // and don't insert any dummy slices
    if (m_mode == CODECHAL_DECODE_MODE_MPEG2VLD &&
        invalidFrame && m_numSlices && !m_incompletePicture)
    {
        CODECHAL_DECODE_ASSERTMESSAGE("Current frame is invalid.");
        return MOS_STATUS_UNKNOWN;
    }

    // MPEG2 Error Concealment for Gen6+
    // Applicable for both IT and VLD modes
    bool copiedDataNeeded = (m_incompletePicture ||
                             (numMBs != (m_picWidthInMb * m_picHeightInMb)));

    if (m_mode == CODECHAL_DECODE_MODE_MPEG2VLD)
    {
        copiedDataNeeded |= (skippedSlicePresent || firstMbAddress);
    }

    if (copiedDataNeeded)
    {
        if (m_dataSize)
        {
            CODECHAL_DECODE_CHK_STATUS_RETURN(CopyDataSurface(
                m_dataSize,
                m_resDataBuffer,
                &m_resCopiedDataBuffer[m_currCopiedData],
                &m_copiedDataOffset));
        }

        m_copiedDataBufferInUse = true;
    }

    m_perfType = (uint16_t)m_picParams->m_pictureCodingType;

    m_crrPic      = m_picParams->m_currPic;
    m_secondField = m_picParams->m_secondField ? true : false;

    CODECHAL_DEBUG_TOOL(
        CODECHAL_DECODE_CHK_NULL_RETURN(m_debugInterface);
        m_debugInterface->m_currPic     = m_crrPic;
        m_debugInterface->m_secondField = m_secondField;
        m_debugInterface->m_frameType   = m_perfType;

        if (m_picParams) {
            CODECHAL_DECODE_CHK_STATUS_RETURN(DumpPicParams(
                m_picParams));
        }

        if (m_iqMatrixBuffer) {
            CODECHAL_DECODE_CHK_STATUS_RETURN(DumpIQParams(
                m_iqMatrixBuffer));
        }

        if (m_sliceParams) {
            CODECHAL_DECODE_CHK_STATUS_RETURN(DumpSliceParams(
                m_sliceParams,
                m_numSlices));
        }

        if (m_mbParams) {
            CODECHAL_DECODE_CHK_STATUS_RETURN(DumpMbParams(
                m_mbParams));
        }

    )

    return eStatus;
}

MOS_STATUS CodechalDecodeMpeg2::DecodeStateLevel()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    uint8_t fwdRefIdx = (uint8_t)m_picParams->m_forwardRefIdx;
    uint8_t bwdRefIdx = (uint8_t)m_picParams->m_backwardRefIdx;

    // Do not use data that has not been initialized
    if (CodecHal_PictureIsInvalid(m_mpeg2RefList[fwdRefIdx]->RefPic))
    {
        fwdRefIdx = m_picParams->m_currPic.FrameIdx;
    }
    if (CodecHal_PictureIsInvalid(m_mpeg2RefList[bwdRefIdx]->RefPic))
    {
        bwdRefIdx = m_picParams->m_currPic.FrameIdx;
    }

    MOS_COMMAND_BUFFER cmdBuffer;
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnGetCommandBuffer(m_osInterface, &cmdBuffer, 0));

    auto mmioRegisters = m_hwInterface->GetMfxInterface()->GetMmioRegisters(m_vdboxIndex);
    HalOcaInterface::On1stLevelBBStart(cmdBuffer, *m_osInterface->pOsContext, m_osInterface->CurrentGpuContextHandle, *m_miInterface, *mmioRegisters);

    MHW_VDBOX_PIPE_MODE_SELECT_PARAMS pipeModeSelectParams;
    pipeModeSelectParams.Mode                  = m_mode;
    pipeModeSelectParams.bStreamOutEnabled     = m_streamOutEnabled;
    pipeModeSelectParams.bPostDeblockOutEnable = m_deblockingEnabled;
    pipeModeSelectParams.bPreDeblockOutEnable  = !m_deblockingEnabled;

    MHW_VDBOX_SURFACE_PARAMS surfaceParams;
    MOS_ZeroMemory(&surfaceParams, sizeof(surfaceParams));
    surfaceParams.Mode                      = m_mode;
    surfaceParams.psSurface                 = &m_destSurface;

    MHW_VDBOX_PIPE_BUF_ADDR_PARAMS pipeBufAddrParams;
    pipeBufAddrParams.Mode                      = m_mode;
    if (m_deblockingEnabled)
    {
        pipeBufAddrParams.psPostDeblockSurface = &m_destSurface;
    }
    else
    {
        pipeBufAddrParams.psPreDeblockSurface = &m_destSurface;
    }

#ifdef _MMC_SUPPORTED
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_mmc->SetPipeBufAddr(&pipeBufAddrParams));
#endif

    // when there is not a forward or backward reference,
    // the index is set to the destination frame index
    m_presReferences[CodechalDecodeFwdRefTop] =
        m_presReferences[CodechalDecodeFwdRefBottom] =
            m_presReferences[CodechalDecodeBwdRefTop] =
                m_presReferences[CodechalDecodeBwdRefBottom] = &m_destSurface.OsResource;

    if (fwdRefIdx < CODECHAL_NUM_UNCOMPRESSED_SURFACE_MPEG2)
    {
        m_presReferences[CodechalDecodeFwdRefTop] =
            m_presReferences[CodechalDecodeFwdRefBottom] = &m_mpeg2RefList[fwdRefIdx]->resRefPic;
    }
    if (bwdRefIdx < CODECHAL_NUM_UNCOMPRESSED_SURFACE_MPEG2)
    {
        m_presReferences[CodechalDecodeBwdRefTop] =
            m_presReferences[CodechalDecodeBwdRefBottom] = &m_mpeg2RefList[bwdRefIdx]->resRefPic;
    }

    // special case for second fields
    if (m_picParams->m_secondField && m_picParams->m_pictureCodingType == P_TYPE)
    {
        if (m_picParams->m_topFieldFirst)
        {
            m_presReferences[CodechalDecodeFwdRefTop] =
                &m_destSurface.OsResource;
        }
        else
        {
            m_presReferences[CodechalDecodeFwdRefBottom] =
                &m_destSurface.OsResource;
        }
    }

    // set all ref pic addresses to valid addresses for error concealment purpose
    for (uint32_t i = 0; i < CODEC_MAX_NUM_REF_FRAME_NON_AVC; i++)
    {
        if (m_presReferences[i] == nullptr && 
            MEDIA_IS_WA(m_waTable, WaDummyReference) && 
            !Mos_ResourceIsNull(&m_dummyReference.OsResource))
        {
            m_presReferences[i] = &m_dummyReference.OsResource;
        }
    }

    CODECHAL_DECODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(
        pipeBufAddrParams.presReferences,
        sizeof(PMOS_RESOURCE) * CODEC_MAX_NUM_REF_FRAME_NON_AVC,
        m_presReferences,
        sizeof(PMOS_RESOURCE) * CODEC_MAX_NUM_REF_FRAME_NON_AVC));

    pipeBufAddrParams.presMfdDeblockingFilterRowStoreScratchBuffer =
        &m_resMfdDeblockingFilterRowStoreScratchBuffer;

    if (m_streamOutEnabled)
    {
        pipeBufAddrParams.presStreamOutBuffer =
            &(m_streamOutBuffer[m_streamOutCurrBufIdx]);
    }

#ifdef _MMC_SUPPORTED
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_mmc->CheckReferenceList(&pipeBufAddrParams));
#endif

    CODECHAL_DEBUG_TOOL(
        for (uint32_t i = 0; i < CODEC_MAX_NUM_REF_FRAME_NON_AVC; i++)
        {
            if (pipeBufAddrParams.presReferences[i])
            {
                MOS_SURFACE dstSurface;

                MOS_ZeroMemory(&dstSurface, sizeof(MOS_SURFACE));
                dstSurface.Format     = Format_NV12;
                dstSurface.OsResource = *(pipeBufAddrParams.presReferences[i]);
                CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHalGetResourceInfo(
                    m_osInterface,
                    &dstSurface));

                m_debugInterface->m_refIndex = (uint16_t)i;
                std::string refSurfName      = "RefSurf" + std::to_string(static_cast<uint32_t>(m_debugInterface->m_refIndex));
                CODECHAL_DECODE_CHK_STATUS_RETURN(m_debugInterface->DumpYUVSurface(
                    &dstSurface,
                    CodechalDbgAttr::attrReferenceSurfaces,
                    refSurfName.data()));
            }
        }
    )

    //set correctly indirect BSD object base address.
    PMOS_RESOURCE indObjBase;
    if (m_copiedDataBufferInUse)
    {
        indObjBase = &m_resCopiedDataBuffer[m_currCopiedData];
    }
    else
    {
        indObjBase = &m_resDataBuffer;
    }

    MHW_VDBOX_IND_OBJ_BASE_ADDR_PARAMS indObjBaseAddrParams;
    MOS_ZeroMemory(&indObjBaseAddrParams, sizeof(indObjBaseAddrParams));
    indObjBaseAddrParams.Mode               = m_mode;
    indObjBaseAddrParams.dwDataSize =
        m_copiedDataBufferInUse ? m_copiedDataBufferSize : m_dataSize;
    indObjBaseAddrParams.presDataBuffer     = indObjBase;

    MHW_VDBOX_BSP_BUF_BASE_ADDR_PARAMS bspBufBaseAddrParams;
    MOS_ZeroMemory(&bspBufBaseAddrParams, sizeof(bspBufBaseAddrParams));
    bspBufBaseAddrParams.presBsdMpcRowStoreScratchBuffer = &m_resBsdMpcRowStoreScratchBuffer;

    MHW_VDBOX_MPEG2_PIC_STATE mpeg2PicState;
    mpeg2PicState.Mode                                  = m_mode;
    mpeg2PicState.pMpeg2PicParams                       = m_picParams;
    mpeg2PicState.bDeblockingEnabled                    = m_deblockingEnabled;
    mpeg2PicState.dwMPEG2ISliceConcealmentMode          = m_mpeg2ISliceConcealmentMode;
    mpeg2PicState.dwMPEG2PBSliceConcealmentMode         = m_mpeg2PbSliceConcealmentMode;
    mpeg2PicState.dwMPEG2PBSlicePredBiDirMVTypeOverride = m_mpeg2PbSlicePredBiDirMvTypeOverride;
    mpeg2PicState.dwMPEG2PBSlicePredMVOverride          = m_mpeg2PbSlicePredMvOverride;

    MHW_VDBOX_QM_PARAMS qmParams;
    qmParams.Standard                       = CODECHAL_MPEG2;
    qmParams.pMpeg2IqMatrix                 = m_iqMatrixBuffer;

    CODECHAL_DECODE_CHK_STATUS_RETURN(SendPrologWithFrameTracking(
        &cmdBuffer, true));

    if (m_statusQueryReportingEnabled)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(StartStatusReport(
            &cmdBuffer));
    }

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfxPipeModeSelectCmd(
        &cmdBuffer,
        &pipeModeSelectParams));

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfxSurfaceCmd(
        &cmdBuffer,
        &surfaceParams));

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfxPipeBufAddrCmd(
        &cmdBuffer,
        &pipeBufAddrParams));

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfxIndObjBaseAddrCmd(
        &cmdBuffer,
        &indObjBaseAddrParams));

    if (CodecHalIsDecodeModeVLD(m_mode))
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfxBspBufBaseAddrCmd(
            &cmdBuffer,
            &bspBufBaseAddrParams));
    }

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfxMpeg2PicCmd(
        &cmdBuffer,
        &mpeg2PicState));

    if (CodecHalIsDecodeModeVLD(m_mode))
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfxQmCmd(
            &cmdBuffer,
            &qmParams));
    }

    m_osInterface->pfnReturnCommandBuffer(m_osInterface, &cmdBuffer, 0);

    return eStatus;
}

MOS_STATUS CodechalDecodeMpeg2::DecodePrimitiveLevel()
{
    if (m_mode == CODECHAL_DECODE_MODE_MPEG2VLD)
    {
        return SliceLevel();
    }
    else
    {
        return MacroblockLevel();
    }
}

MOS_STATUS CodechalDecodeMpeg2::SliceLevel()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    if ((m_decodePhantomMbs) || (m_incompletePicture))
    {
        if (m_bbInUsePerFrame >= m_bbAllocated / CODECHAL_DECODE_MPEG2_BATCH_BUFFERS_PER_GROUP)
        {
            m_bbAllocated += CODECHAL_DECODE_MPEG2_BATCH_BUFFERS_PER_GROUP;
            if (m_bbAllocated >= CODECHAL_DECODE_MPEG2_MAXIMUM_BATCH_BUFFERS)
            {
                CODECHAL_DECODE_ASSERTMESSAGE(
                    "The number of MPEG2 second level batch buffer is not big enough to hold the whole frame.");
                return MOS_STATUS_EXCEED_MAX_BB_SIZE;
            }

            for (uint32_t i = 0; i < CODECHAL_DECODE_MPEG2_BATCH_BUFFERS_PER_GROUP; i++)
            {
                uint32_t j = m_bbAllocated - i - 1;
                MOS_ZeroMemory(&m_mediaObjectBatchBuffer[j], sizeof(MHW_BATCH_BUFFER));

                uint32_t u32Size = m_standardDecodeSizeNeeded * CODECHAL_GET_WIDTH_IN_MACROBLOCKS(m_width) * CODECHAL_GET_HEIGHT_IN_MACROBLOCKS(m_height) +
                    m_hwInterface->m_sizeOfCmdBatchBufferEnd;

                CODECHAL_DECODE_CHK_STATUS_RETURN(Mhw_AllocateBb(
                    m_osInterface,
                    &m_mediaObjectBatchBuffer[j],
                    nullptr,
                    u32Size));
                m_mediaObjectBatchBuffer[j].bSecondLevel = true;
            }
        }
    }

    MOS_COMMAND_BUFFER cmdBuffer;
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnGetCommandBuffer(
        m_osInterface,
        &cmdBuffer,
        0));

    MHW_BATCH_BUFFER batchBuffer = m_mediaObjectBatchBuffer[m_bbInUse];

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferStartCmd(
        &cmdBuffer,
        &batchBuffer));

    CODECHAL_DECODE_CHK_STATUS_RETURN(Mhw_LockBb(
        m_osInterface,
        &batchBuffer));

    if (m_decodePhantomMbs)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(InsertDummySlices(
            &batchBuffer,
            m_lastMbAddress,
            m_picWidthInMb * m_picHeightInMb));
    }
    else
    {
        CodecDecodeMpeg2SliceParams *slc = m_sliceParams;

        uint16_t prevSliceMBEnd = m_lastMbAddress;

        for (uint16_t slcCount = 0; slcCount < m_numSlices; slcCount++)
        {
            if (!m_vldSliceRecord[slcCount].dwSkip)
            {
                if (prevSliceMBEnd != m_vldSliceRecord[slcCount].dwSliceStartMbOffset)
                {
                    CODECHAL_DECODE_CHK_STATUS_RETURN(InsertDummySlices(
                        &batchBuffer,
                        prevSliceMBEnd,
                        (uint16_t)m_vldSliceRecord[slcCount].dwSliceStartMbOffset));
                }

                if (m_vldSliceRecord[slcCount].bIsLastSlice)
                {
                    uint16_t expectedFinalMb = m_picWidthInMb * m_picHeightInMb;

                    m_lastMbAddress =
                        (uint16_t)(m_vldSliceRecord[slcCount].dwSliceStartMbOffset +
                                   slc->m_numMbsForSlice);

                    if (m_lastMbAddress < expectedFinalMb)
                    {
                        m_incompletePicture              = true;
                        m_vldSliceRecord[slcCount].bIsLastSlice = false;
                    }
                    else
                    {
                        //Indicate It's complete picture now
                        m_incompletePicture              = false;
                    }
                }

                // static MPEG2 slice parameters
                MHW_VDBOX_MPEG2_SLICE_STATE mpeg2SliceState;
                mpeg2SliceState.presDataBuffer          = &m_resDataBuffer;
                mpeg2SliceState.wPicWidthInMb           = m_picWidthInMb;
                mpeg2SliceState.wPicHeightInMb          = m_picHeightInMb;
                mpeg2SliceState.pMpeg2SliceParams       = slc;
                mpeg2SliceState.dwLength                = m_vldSliceRecord[slcCount].dwLength;
                mpeg2SliceState.dwOffset =
                    m_vldSliceRecord[slcCount].dwOffset + m_copiedDataOffset;
                mpeg2SliceState.dwSliceStartMbOffset = m_vldSliceRecord[slcCount].dwSliceStartMbOffset;
                mpeg2SliceState.bLastSlice           = m_vldSliceRecord[slcCount].bIsLastSlice;

                CODECHAL_DECODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfdMpeg2BsdObject(
                    nullptr,
                    &batchBuffer,
                    &mpeg2SliceState));

                prevSliceMBEnd =
                    (uint16_t)(m_vldSliceRecord[slcCount].dwSliceStartMbOffset +
                               slc->m_numMbsForSlice);
            }

            slc++;
        }
    }

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferEnd(
        nullptr,
        &batchBuffer));

    CODECHAL_DEBUG_TOOL(
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_debugInterface->Dump2ndLvlBatch(
            &batchBuffer,
            CODECHAL_NUM_MEDIA_STATES,
            "_DEC"));
    )

    CODECHAL_DECODE_CHK_STATUS_RETURN(Mhw_UnlockBb(
        m_osInterface,
        &batchBuffer,
        true));

    m_bbInUse = (m_bbInUse + 1) % m_bbAllocated;
    m_bbInUsePerFrame++;

    if (!m_incompletePicture)
    {
        // Check if destination surface needs to be synchronized
        MOS_SYNC_PARAMS syncParams          = g_cInitSyncParams;
        syncParams.GpuContext               = m_videoContext;
        syncParams.presSyncResource         = &m_destSurface.OsResource;
        syncParams.bReadOnly                = false;
        syncParams.bDisableDecodeSyncLock   = m_disableDecodeSyncLock;
        syncParams.bDisableLockForTranscode = m_disableLockForTranscode;

        if (!CodecHal_PictureIsField(m_picParams->m_currPic) ||
            !m_picParams->m_secondField)
        {
            CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnPerformOverlaySync(
                m_osInterface,
                &syncParams));
            CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnResourceWait(
                m_osInterface,
                &syncParams));

            // Update the resource tag (s/w tag) for On-Demand Sync
            m_osInterface->pfnSetResourceSyncTag(m_osInterface, &syncParams);
        }

        MHW_MI_FLUSH_DW_PARAMS flushDwParams;
        MOS_ZeroMemory(&flushDwParams, sizeof(flushDwParams));

        CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiFlushDwCmd(
            &cmdBuffer,
            &flushDwParams));

        // Update the tag in GPU Sync eStatus buffer (H/W Tag) to match the current S/W tag
        if (m_osInterface->bTagResourceSync &&
            (!CodecHal_PictureIsField(m_picParams->m_currPic) || m_picParams->m_secondField))
        {
            CODECHAL_DECODE_CHK_STATUS_RETURN(m_hwInterface->WriteSyncTagToResource(
                &cmdBuffer,
                &syncParams));
        }

        if (m_statusQueryReportingEnabled)
        {
            CodechalDecodeStatusReport decodeStatusReport;

            decodeStatusReport.m_statusReportNumber = m_statusReportFeedbackNumber;
            decodeStatusReport.m_currDecodedPic     = m_picParams->m_currPic;
            decodeStatusReport.m_currDeblockedPic   = m_picParams->m_currPic;
            decodeStatusReport.m_codecStatus        = CODECHAL_STATUS_UNAVAILABLE;
            decodeStatusReport.m_currDecodedPicRes  = m_mpeg2RefList[m_picParams->m_currPic.FrameIdx]->resRefPic;

            CODECHAL_DEBUG_TOOL(
                decodeStatusReport.m_secondField = m_picParams->m_secondField ? true : false;
                decodeStatusReport.m_frameType   = m_perfType;)

            CODECHAL_DECODE_CHK_STATUS_RETURN(EndStatusReport(
                decodeStatusReport,
                &cmdBuffer));
        }

        CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferEnd(
            &cmdBuffer,
            nullptr));

        m_osInterface->pfnReturnCommandBuffer(m_osInterface, &cmdBuffer, 0);

        CODECHAL_DEBUG_TOOL(
            CODECHAL_DECODE_CHK_STATUS_RETURN(m_debugInterface->DumpCmdBuffer(
                &cmdBuffer,
                CODECHAL_NUM_MEDIA_STATES,
                "_DEC"));

            //CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHal_DbgReplaceAllCommands(
            //    m_debugInterface,
            //    &cmdBuffer));
        )

        //Sync up complete frame
        syncParams                      = g_cInitSyncParams;
        syncParams.GpuContext           = m_videoContextForWa;
        syncParams.presSyncResource     = &m_resSyncObjectWaContextInUse;

        CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnEngineSignal(m_osInterface, &syncParams));

        syncParams                      = g_cInitSyncParams;
        syncParams.GpuContext           = m_videoContext;
        syncParams.presSyncResource     = &m_resSyncObjectWaContextInUse;

        CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnEngineWait(m_osInterface, &syncParams));

        HalOcaInterface::On1stLevelBBEnd(cmdBuffer, *m_osInterface->pOsContext);

        CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnSubmitCommandBuffer(
            m_osInterface,
            &cmdBuffer,
            m_videoContextUsesNullHw));

        CODECHAL_DEBUG_TOOL(
            m_mmc->UpdateUserFeatureKey(&m_destSurface);)

        if (m_statusQueryReportingEnabled)
        {
            CODECHAL_DECODE_CHK_STATUS_RETURN(ResetStatusReport(
                m_videoContextUsesNullHw));
        }

        // Needs to be re-set for Linux buffer re-use scenarios
        m_mpeg2RefList[m_picParams->m_currPic.FrameIdx]->resRefPic =
            m_destSurface.OsResource;

        // Send the signal to indicate decode completion, in case On-Demand Sync is not present
        if (!CodecHal_PictureIsField(m_picParams->m_currPic) || m_picParams->m_secondField)
        {
            syncParams                      = g_cInitSyncParams;
            syncParams.GpuContext           = m_videoContext;
            syncParams.presSyncResource     = &m_destSurface.OsResource;

            CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnResourceSignal(m_osInterface, &syncParams));
        }
    }
    else
    {
        m_osInterface->pfnReturnCommandBuffer(m_osInterface, &cmdBuffer, 0);
    }

    return eStatus;
}

void CodechalDecodeMpeg2::PackMotionVectors(
    CODEC_PICTURE_FLAG          pic_flag,
    PMHW_VDBOX_MPEG2_MB_STATE   mpeg2MbState)
{
    CodecDecodeMpeg2MbParmas *mbParams = mpeg2MbState->pMBParams;

    uint16_t motionType     = mbParams->MBType.m_motionType;
    uint16_t intelMotionType = CODECHAL_MPEG2_IMT_NONE;

    //convert to Intel Motion Type
    if (pic_flag == PICTURE_FRAME)
    {
        switch(motionType)
        {
        case CodechalDecodeMcFrame:
            intelMotionType = CODECHAL_MPEG2_IMT_FRAME_FRAME;
            break;
        case CodechalDecodeMcField:
            intelMotionType = CODECHAL_MPEG2_IMT_FRAME_FIELD;
            break;
        case CodechalDecodeMcDmv:
            intelMotionType = CODECHAL_MPEG2_IMT_FRAME_DUAL_PRIME;
            break;
        default:
            break;
        }
    }
    else // must be field picture...
    {
        switch(motionType)
        {
        case CodechalDecodeMcField:
            intelMotionType = CODECHAL_MPEG2_IMT_FIELD_FIELD;
            break;
        case CodechalDecodeMcDmv:
            intelMotionType = CODECHAL_MPEG2_IMT_FIELD_DUAL_PRIME;
            break;
        case CodechalDecodeMc16x8:
            intelMotionType = CODECHAL_MPEG2_IMT_16X8;
            break;
        default:
            break;
        }
    }

    int16_t *mv = mbParams->m_motionVectors;

    switch (intelMotionType)
    {
    case CODECHAL_MPEG2_IMT_16X8:
    case CODECHAL_MPEG2_IMT_FIELD_FIELD:
    case CODECHAL_MPEG2_IMT_FRAME_FRAME:
    case CODECHAL_MPEG2_IMT_FIELD_DUAL_PRIME:
        mpeg2MbState->sPackedMVs0[0] = (short)mv[CodechalDecodeRstFirstForwHorz];
        mpeg2MbState->sPackedMVs0[1] = (short)mv[CodechalDecodeRstFirstForwVert];
        mpeg2MbState->sPackedMVs0[2] = (short)mv[CodechalDecodeRstFirstBackHorz];
        mpeg2MbState->sPackedMVs0[3] = (short)mv[CodechalDecodeRstFirstBackVert];
        break;

    case CODECHAL_MPEG2_IMT_FRAME_FIELD:
    case CODECHAL_MPEG2_IMT_FRAME_DUAL_PRIME:
        mpeg2MbState->sPackedMVs0[0] = (short)mv[CodechalDecodeRstFirstForwHorz];
        mpeg2MbState->sPackedMVs0[1] = (short)(mv[CodechalDecodeRstFirstForwVert] >> 1) ;
        mpeg2MbState->sPackedMVs0[2] = (short)mv[CodechalDecodeRstFirstBackHorz];
        mpeg2MbState->sPackedMVs0[3] = (short)(mv[CodechalDecodeRstFirstBackVert] >> 1);
        break;

    default:
        break;
    }

    switch (intelMotionType)
    {
    case CODECHAL_MPEG2_IMT_16X8:
        mpeg2MbState->sPackedMVs1[0] = (short)mv[CodechalDecodeRstSecndForwHorz];
        mpeg2MbState->sPackedMVs1[1] = (short)mv[CodechalDecodeRstSecndForwVert];
        mpeg2MbState->sPackedMVs1[2] = (short)mv[CodechalDecodeRstSecndBackHorz];
        mpeg2MbState->sPackedMVs1[3] = (short)mv[CodechalDecodeRstSecndBackVert];
        break;

    case CODECHAL_MPEG2_IMT_FRAME_DUAL_PRIME:
        mpeg2MbState->sPackedMVs1[0] = (short)mv[CodechalDecodeRstFirstForwHorz];
        mpeg2MbState->sPackedMVs1[1] = (short)(mv[CodechalDecodeRstFirstForwVert] >> 1);
        mpeg2MbState->sPackedMVs1[2] = (short)mv[CodechalDecodeRstSecndBackHorz];
        mpeg2MbState->sPackedMVs1[3] = (short)(mv[CodechalDecodeRstSecndBackVert] >> 1);
        break;

    case CODECHAL_MPEG2_IMT_FRAME_FIELD:
        mpeg2MbState->sPackedMVs1[0] = (short)mv[CodechalDecodeRstSecndForwHorz];
        mpeg2MbState->sPackedMVs1[1] = (short)(mv[CodechalDecodeRstSecndForwVert] >> 1);
        mpeg2MbState->sPackedMVs1[2] = (short)mv[CodechalDecodeRstSecndBackHorz];
        mpeg2MbState->sPackedMVs1[3] = (short)(mv[CodechalDecodeRstSecndBackVert] >> 1);
        break;

    default:
        break;
    }
}
MOS_STATUS CodechalDecodeMpeg2::InsertSkippedMacroblocks(
    PMHW_BATCH_BUFFER               batchBuffer,
    PMHW_VDBOX_MPEG2_MB_STATE       params,
    uint16_t                        nextMBStart,
    uint16_t                        skippedMBs)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    CODECHAL_DECODE_CHK_NULL_RETURN(batchBuffer);
    CODECHAL_DECODE_CHK_NULL_RETURN(params);
    CODECHAL_DECODE_CHK_NULL_RETURN(params->pMBParams);

    //save the original MB params, and restore the orignal MB params when function exit.
    CodechalDecodeRestoreData<CodecDecodeMpeg2MbParmas> MBParamsRestore(params->pMBParams);

    params->dwDCTLength                    = 0;
    params->dwITCoffDataAddrOffset         = 0;
    params->pMBParams->m_codedBlockPattern  = 0;

    MOS_ZeroMemory(params->sPackedMVs0,sizeof(params->sPackedMVs0));
    MOS_ZeroMemory(params->sPackedMVs1,sizeof(params->sPackedMVs1));

    for (uint16_t i = 0; i < skippedMBs; i++)
    {
        params->pMBParams->m_mbAddr = nextMBStart + i;
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfdMpeg2ITObject(
            nullptr,
            batchBuffer,
            params));
    }

    return eStatus;
}

MOS_STATUS CodechalDecodeMpeg2::MacroblockLevel()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    if ((m_decodePhantomMbs) || (m_incompletePicture))
    {
        if (m_bbInUsePerFrame >= m_bbAllocated / CODECHAL_DECODE_MPEG2_BATCH_BUFFERS_PER_GROUP)
        {
            m_bbAllocated += CODECHAL_DECODE_MPEG2_BATCH_BUFFERS_PER_GROUP;
            if (m_bbAllocated >= CODECHAL_DECODE_MPEG2_MAXIMUM_BATCH_BUFFERS)
            {
                CODECHAL_DECODE_ASSERTMESSAGE(
                    "The number of MPEG2 second level batch buffer is not big enough to hold the whole frame.");
                return MOS_STATUS_EXCEED_MAX_BB_SIZE;
            }

            for (uint32_t i = 0; i < CODECHAL_DECODE_MPEG2_BATCH_BUFFERS_PER_GROUP; i++)
            {
                uint32_t j = m_bbAllocated - i - 1;
                MOS_ZeroMemory(&m_mediaObjectBatchBuffer[j], sizeof(MHW_BATCH_BUFFER));

                uint32_t u32Size = m_standardDecodeSizeNeeded * m_picWidthInMb * m_picHeightInMb +
                                   m_hwInterface->m_sizeOfCmdBatchBufferEnd;

                CODECHAL_DECODE_CHK_STATUS_RETURN(Mhw_AllocateBb(
                    m_osInterface,
                    &m_mediaObjectBatchBuffer[j],
                    nullptr,
                    u32Size));
                m_mediaObjectBatchBuffer[j].bSecondLevel = true;
            }
        }
    }

    MOS_COMMAND_BUFFER cmdBuffer;
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnGetCommandBuffer(
        m_osInterface,
        &cmdBuffer,
        0));

    MHW_BATCH_BUFFER batchBuffer = m_mediaObjectBatchBuffer[m_bbInUse];

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferStartCmd(
        &cmdBuffer,
        &batchBuffer));

    CODECHAL_DECODE_CHK_STATUS_RETURN(Mhw_LockBb(m_osInterface, &batchBuffer));

    //static member
    MHW_VDBOX_MPEG2_MB_STATE    mpeg2MbState;
    mpeg2MbState.wPicWidthInMb  = m_picWidthInMb;
    mpeg2MbState.wPicHeightInMb = m_picHeightInMb;
    mpeg2MbState.wPicCodingType = (uint16_t)m_picParams->m_pictureCodingType;

    if (m_decodePhantomMbs)
    {
        uint16_t u16NextMBStart = m_savedMpeg2MbParam.m_mbAddr + 1;  // = 1 + saved last MB's address in this picture.
        uint16_t numMBs      = (mpeg2MbState.wPicWidthInMb * mpeg2MbState.wPicHeightInMb) - u16NextMBStart;

        mpeg2MbState.pMBParams = &m_savedMpeg2MbParam;  //use saved last MB param to insert Skipped MBs.
        CODECHAL_DECODE_CHK_STATUS_RETURN(InsertSkippedMacroblocks(
            &batchBuffer,
            &mpeg2MbState,
            u16NextMBStart,
            numMBs));
        m_incompletePicture = false;
    }
    else
    {
        uint16_t expectedMBAddress = (m_incompletePicture) ? m_lastMbAddress : 0;

        for (uint16_t mbcount = 0; mbcount < m_numMacroblocks; mbcount++)
        {
            if (m_mbParams[mbcount].m_mbAddr >= expectedMBAddress)
            {
                uint16_t skippedMBs = m_mbParams[mbcount].m_mbAddr - expectedMBAddress;

                if (skippedMBs)
                {
                    //insert skipped Macroblock, use the first available MB params to insert skipped MBs.
                    mpeg2MbState.pMBParams = &(m_mbParams[mbcount]);
                    CODECHAL_DECODE_CHK_STATUS_RETURN(InsertSkippedMacroblocks(
                        &batchBuffer,
                        &mpeg2MbState,
                        expectedMBAddress,
                        skippedMBs));
                }
            }

            //common field for MBs in I picture and PB picture .
            mpeg2MbState.pMBParams   = &m_mbParams[mbcount];
            mpeg2MbState.dwDCTLength = 0;
            for (uint32_t i = 0; i < CODEC_NUM_BLOCK_PER_MB; i++)
            {
                mpeg2MbState.dwDCTLength += m_mbParams[mbcount].m_numCoeff[i];
            }

            mpeg2MbState.dwITCoffDataAddrOffset = m_copiedDataOffset + (m_mbParams[mbcount].m_mbDataLoc << 2);  // byte offset

            //only for MB in PB picture.
            if (mpeg2MbState.wPicCodingType != I_TYPE)
            {
                bool intraMB = mpeg2MbState.pMBParams->MBType.m_intraMb? true: false;

                MOS_ZeroMemory(mpeg2MbState.sPackedMVs0,sizeof(mpeg2MbState.sPackedMVs0));
                MOS_ZeroMemory(mpeg2MbState.sPackedMVs1,sizeof(mpeg2MbState.sPackedMVs1));
                if ((!intraMB) && (mpeg2MbState.pMBParams->MBType.m_value &
                    (CODECHAL_DECODE_MPEG2_MB_MOTION_BACKWARD | CODECHAL_DECODE_MPEG2_MB_MOTION_FORWARD)))
                {
                    PackMotionVectors(m_picParams->m_currPic.PicFlags, &mpeg2MbState);
                }
            }

            // add IT OBJECT command for each macroblock
            CODECHAL_DECODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfdMpeg2ITObject(
                nullptr,
                &batchBuffer,
                &mpeg2MbState));

            if (mpeg2MbState.wPicCodingType != I_TYPE && m_mbParams[mbcount].m_mbSkipFollowing)
            {
                uint16_t skippedMBs    = m_mbParams[mbcount].m_mbSkipFollowing;
                uint16_t skippedMBSart = m_mbParams[mbcount].m_mbAddr + 1;

                // Insert Skipped MBs
                CODECHAL_DECODE_CHK_STATUS_RETURN(InsertSkippedMacroblocks(
                    &batchBuffer,
                    &mpeg2MbState,
                    skippedMBSart,
                    skippedMBs));

                m_mbParams[mbcount].m_mbAddr += skippedMBs;
            }

            //save the last MB's parameters for later use to insert skipped MBs.
            m_savedMpeg2MbParam = m_mbParams[mbcount];

            expectedMBAddress = m_mbParams[mbcount].m_mbAddr + 1;

            // insert extra MBs to ensure expected number of MBs sent to HW
            if (mbcount + 1 == m_numMacroblocks)
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
    }

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferEnd(
        nullptr,
        &batchBuffer));

    CODECHAL_DEBUG_TOOL(
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_debugInterface->Dump2ndLvlBatch(
            &batchBuffer,
            CODECHAL_NUM_MEDIA_STATES,
            "_DEC"));
    )

    CODECHAL_DECODE_CHK_STATUS_RETURN(Mhw_UnlockBb(m_osInterface, &batchBuffer, true));

    m_bbInUse = (m_bbInUse + 1) % m_bbAllocated;
    m_bbInUsePerFrame++;

    if (!m_incompletePicture)
    {
        // Check if destination surface needs to be synchronized
        MOS_SYNC_PARAMS syncParams          = g_cInitSyncParams;
        syncParams.GpuContext               = m_videoContext;
        syncParams.presSyncResource         = &m_destSurface.OsResource;
        syncParams.bReadOnly                = false;
        syncParams.bDisableDecodeSyncLock   = m_disableDecodeSyncLock;
        syncParams.bDisableLockForTranscode = m_disableLockForTranscode;

        if (!CodecHal_PictureIsField(m_picParams->m_currPic) ||
            !m_picParams->m_secondField)
        {
            CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnPerformOverlaySync(
                m_osInterface,
                &syncParams));
            CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnResourceWait(
                m_osInterface,
                &syncParams));

            // Update the resource tag (s/w tag) for On-Demand Sync
            m_osInterface->pfnSetResourceSyncTag(m_osInterface, &syncParams);
        }

        MHW_MI_FLUSH_DW_PARAMS flushDwParams;
        MOS_ZeroMemory(&flushDwParams, sizeof(flushDwParams));

        CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiFlushDwCmd(
            &cmdBuffer,
            &flushDwParams));

        // Update the tag in GPU Sync eStatus buffer (H/W Tag) to match the current S/W tag
        if (m_osInterface->bTagResourceSync &&
            (!CodecHal_PictureIsField(m_picParams->m_currPic) || m_picParams->m_secondField))
        {
            CODECHAL_DECODE_CHK_STATUS_RETURN(m_hwInterface->WriteSyncTagToResource(
                &cmdBuffer,
                &syncParams));
        }

        if (m_statusQueryReportingEnabled)
        {
            CodechalDecodeStatusReport decodeStatusReport;

            decodeStatusReport.m_statusReportNumber = m_statusReportFeedbackNumber;
            decodeStatusReport.m_currDecodedPic     = m_picParams->m_currPic;
            decodeStatusReport.m_currDeblockedPic   = m_picParams->m_currPic;
            decodeStatusReport.m_codecStatus        = CODECHAL_STATUS_UNAVAILABLE;
            decodeStatusReport.m_currDecodedPicRes =
                m_mpeg2RefList[m_picParams->m_currPic.FrameIdx]->resRefPic;

            CODECHAL_DEBUG_TOOL(
                decodeStatusReport.m_secondField = m_picParams->m_secondField ? true : false;
                decodeStatusReport.m_frameType   = m_perfType;)

            CODECHAL_DECODE_CHK_STATUS_RETURN(EndStatusReport(
                decodeStatusReport,
                &cmdBuffer));
        }

        CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferEnd(
            &cmdBuffer,
            nullptr));

        m_osInterface->pfnReturnCommandBuffer(m_osInterface, &cmdBuffer, 0);

        CODECHAL_DEBUG_TOOL(
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_debugInterface->DumpCmdBuffer(
            &cmdBuffer,
            CODECHAL_NUM_MEDIA_STATES,
            "_DEC"));

        //CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHal_DbgReplaceAllCommands(
        //    m_debugInterface,
        //    &cmdBuffer));
        )

        //Sync up complete frame
        syncParams                      = g_cInitSyncParams;
        syncParams.GpuContext           = m_videoContextForWa;
        syncParams.presSyncResource     = &m_resSyncObjectWaContextInUse;

        CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnEngineSignal(m_osInterface, &syncParams));

        syncParams                      = g_cInitSyncParams;
        syncParams.GpuContext           = m_videoContext;
        syncParams.presSyncResource     = &m_resSyncObjectWaContextInUse;

        CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnEngineWait(m_osInterface, &syncParams));

        HalOcaInterface::On1stLevelBBEnd(cmdBuffer, *m_osInterface->pOsContext);

        CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnSubmitCommandBuffer(
            m_osInterface,
            &cmdBuffer,
            m_videoContextUsesNullHw));

        CODECHAL_DEBUG_TOOL(
            m_mmc->UpdateUserFeatureKey(&m_destSurface);)

        if (m_statusQueryReportingEnabled)
        {
            CODECHAL_DECODE_CHK_STATUS_RETURN(ResetStatusReport(
                m_videoContextUsesNullHw));
        }

        // Needs to be re-set for Linux buffer re-use scenarios
        m_mpeg2RefList[m_picParams->m_currPic.FrameIdx]->resRefPic = m_destSurface.OsResource;

        // Send the signal to indicate decode completion, in case On-Demand Sync is not present
        if (!CodecHal_PictureIsField(m_picParams->m_currPic) || m_picParams->m_secondField)
        {
            syncParams                      = g_cInitSyncParams;
            syncParams.GpuContext           = m_videoContext;
            syncParams.presSyncResource     = &m_destSurface.OsResource;

            CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnResourceSignal(m_osInterface, &syncParams));
        }
    }
    else
    {
        m_osInterface->pfnReturnCommandBuffer(m_osInterface, &cmdBuffer, 0);
    }

    return eStatus;
}

MOS_STATUS CodechalDecodeMpeg2::InitMmcState()
{
#ifdef _MMC_SUPPORTED
    m_mmc = MOS_New(CodechalMmcDecodeMpeg2, m_hwInterface, this);
    CODECHAL_DECODE_CHK_NULL_RETURN(m_mmc);
#endif
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalDecodeMpeg2::AllocateStandard (
    CodechalSetting *settings)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    CODECHAL_DECODE_CHK_NULL_RETURN(settings);

    CODECHAL_DECODE_CHK_STATUS_RETURN(InitMmcState());

    m_width                         = settings->width;
    m_height                        = settings->height;
    m_picWidthInMb                  = (uint16_t)CODECHAL_GET_WIDTH_IN_MACROBLOCKS(m_width);
    m_picHeightInMb                 = (uint16_t)CODECHAL_GET_HEIGHT_IN_MACROBLOCKS(m_height);
    m_bbAllocated                   = CODECHAL_DECODE_MPEG2_BATCH_BUFFERS_PER_GROUP;

    // Picture Level Commands
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_hwInterface->GetMfxStateCommandsDataSize(
        m_mode,
        &m_commandBufferSizeNeeded,
        &m_commandPatchListSizeNeeded,
        0));

    // Primitive Level Commands
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_hwInterface->GetMfxPrimitiveCommandsDataSize(
        m_mode,
        &m_standardDecodeSizeNeeded,
        &m_standardDecodePatchListSizeNeeded,
        0));

    CODECHAL_DECODE_CHK_STATUS_RETURN(AllocateResources());

    return eStatus;
}

CodechalDecodeMpeg2::CodechalDecodeMpeg2 (
    CodechalHwInterface   *hwInterface,
    CodechalDebugInterface* debugInterface,
    PCODECHAL_STANDARD_INFO standardInfo) :
    CodechalDecode(hwInterface, debugInterface, standardInfo)
{
    CODECHAL_DECODE_FUNCTION_ENTER;

    MOS_ZeroMemory(&m_destSurface, sizeof(m_destSurface));
    MOS_ZeroMemory(m_presReferences, sizeof(m_presReferences));
    MOS_ZeroMemory(&m_resDataBuffer, sizeof(m_resDataBuffer));
    MOS_ZeroMemory(&m_resMfdDeblockingFilterRowStoreScratchBuffer, sizeof(m_resMfdDeblockingFilterRowStoreScratchBuffer));
    MOS_ZeroMemory(&m_resBsdMpcRowStoreScratchBuffer, sizeof(m_resBsdMpcRowStoreScratchBuffer));
    MOS_ZeroMemory(m_mpeg2RefList, sizeof(m_mpeg2RefList));
    MOS_ZeroMemory(m_mediaObjectBatchBuffer, sizeof(m_mediaObjectBatchBuffer));
    MOS_ZeroMemory(&m_resMpeg2DummyBistream, sizeof(m_resMpeg2DummyBistream));
    MOS_ZeroMemory(m_resCopiedDataBuffer, sizeof(m_resCopiedDataBuffer));
    MOS_ZeroMemory(&m_resSyncObjectWaContextInUse, sizeof(m_resSyncObjectWaContextInUse));
    MOS_ZeroMemory(&m_resSyncObjectVideoContextInUse, sizeof(m_resSyncObjectVideoContextInUse));
    MOS_ZeroMemory(&m_savedMpeg2MbParam, sizeof(m_savedMpeg2MbParam));
}

#if USE_CODECHAL_DEBUG_TOOL
MOS_STATUS CodechalDecodeMpeg2::DumpPicParams(
    CodecDecodeMpeg2PicParams *picParams)
{
    CODECHAL_DEBUG_FUNCTION_ENTER;

    if (!m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrPicParams))
    {
        return MOS_STATUS_SUCCESS;
    }

    CODECHAL_DEBUG_CHK_NULL(picParams);

    std::ostringstream oss;
    oss.setf(std::ios::showbase | std::ios::uppercase);

    oss << "m_currPic FrameIdx: " << +picParams->m_currPic.FrameIdx << std::endl;
    oss << "m_currPic PicFlags: " << +picParams->m_currPic.PicFlags << std::endl;
    oss << "m_forwardRefIdx: " << +picParams->m_forwardRefIdx << std::endl;
    oss << "m_backwardRefIdx: " << +picParams->m_backwardRefIdx << std::endl;
    oss << "m_topFieldFirst: " << +picParams->m_topFieldFirst << std::endl;
    oss << "m_secondField: " << +picParams->m_secondField << std::endl;
    oss << "m_statusReportFeedbackNumber: " << +picParams->m_statusReportFeedbackNumber << std::endl;
    //Dump union w0
    oss << "w0 m_value: " << +picParams->W0.m_value << std::endl;
    oss << "m_scanOrder: " << +picParams->W0.m_scanOrder << std::endl;
    oss << "m_intraVlcFormat: " << +picParams->W0.m_intraVlcFormat << std::endl;
    oss << "m_quantizerScaleType: " << +picParams->W0.m_quantizerScaleType << std::endl;
    oss << "m_concealmentMVFlag: " << +picParams->W0.m_concealmentMVFlag << std::endl;
    oss << "m_frameDctPrediction: " << +picParams->W0.m_frameDctPrediction << std::endl;
    oss << "m_topFieldFirst: " << +picParams->W0.m_topFieldFirst << std::endl;
    oss << "m_intraDCPrecision: " << +picParams->W0.m_intraDCPrecision << std::endl;
    //Dump union w1
    oss << "w1 m_value: " << +picParams->W1.m_value << std::endl;
    oss << "m_fcode11: " << +picParams->W1.m_fcode11 << std::endl;
    oss << "m_fcode10: " << +picParams->W1.m_fcode10 << std::endl;
    oss << "m_fcode01: " << +picParams->W1.m_fcode01 << std::endl;
    oss << "m_fcode00: " << +picParams->W1.m_fcode00 << std::endl;
    oss << "m_horizontalSize: " << +picParams->m_horizontalSize << std::endl;
    oss << "m_verticalSize: " << +picParams->m_verticalSize << std::endl;
    oss << "m_pictureCodingType: " << +picParams->m_pictureCodingType << std::endl;

    const char *fileName = m_debugInterface->CreateFileName(
        "_DEC",
        CodechalDbgBufferType::bufPicParams,
        CodechalDbgExtType::txt);

    std::ofstream ofs(fileName, std::ios::out);
    ofs << oss.str();
    ofs.close();
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalDecodeMpeg2::DumpSliceParams(
    CodecDecodeMpeg2SliceParams *sliceParams,
    uint32_t                     numSlices)
{
    CODECHAL_DEBUG_FUNCTION_ENTER;

    if (!m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrSlcParams))
    {
        return MOS_STATUS_SUCCESS;
    }

    CODECHAL_DEBUG_CHK_NULL(sliceParams);

    const char *fileName = m_debugInterface->CreateFileName(
        "_DEC",
        CodechalDbgBufferType::bufSlcParams,
        CodechalDbgExtType::txt);

    std::ostringstream oss;
    oss.setf(std::ios::showbase | std::ios::uppercase);

    CodecDecodeMpeg2SliceParams *sliceControl = nullptr;

    for (uint16_t i = 0; i < numSlices; i++)
    {
        sliceControl = &sliceParams[i];

        oss << "====================================================================================================" << std::endl;
        oss << "Data for Slice number = " << +i << std::endl;
        oss << "m_sliceDataSize: " << +sliceControl->m_sliceDataSize << std::endl;
        oss << "m_sliceDataOffset: " << +sliceControl->m_sliceDataOffset << std::endl;
        oss << "m_macroblockOffset: " << +sliceControl->m_macroblockOffset << std::endl;
        oss << "m_sliceHorizontalPosition: " << +sliceControl->m_sliceHorizontalPosition << std::endl;
        oss << "m_sliceVerticalPosition: " << +sliceControl->m_sliceVerticalPosition << std::endl;
        oss << "m_quantiserScaleCode: " << +sliceControl->m_quantiserScaleCode << std::endl;
        oss << "m_numMbsForSlice: " << +sliceControl->m_numMbsForSlice << std::endl;
        oss << "m_numMbsForSliceOverflow: " << +sliceControl->m_numMbsForSliceOverflow << std::endl;
        oss << "m_reservedBits: " << +sliceControl->m_reservedBits << std::endl;
        oss << "m_startCodeBitOffset: " << +sliceControl->m_startCodeBitOffset << std::endl;

        std::ofstream ofs;
        if (i == 0)
        {
            ofs.open(fileName, std::ios::out);
        }
        else
        {
            ofs.open(fileName, std::ios::app);
        }
        ofs << oss.str();
        ofs.close();
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalDecodeMpeg2::DumpIQParams(
    CodecMpeg2IqMatrix *matrixData)
{
    CODECHAL_DEBUG_FUNCTION_ENTER;

    if (!m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrIqParams))
    {
        return MOS_STATUS_SUCCESS;
    }

    CODECHAL_DEBUG_CHK_NULL(matrixData);

    std::ostringstream oss;
    oss.setf(std::ios::showbase | std::ios::uppercase);

    if (matrixData->m_loadIntraQuantiserMatrix)
    {
        oss << "intra_QmatrixData:" << std::endl;

        for (uint8_t i = 0; i < 56; i += 8)
        {
            oss << "Qmatrix[" << +i / 8 << "]: ";
            for (uint8_t j = 0; j < 8; j++)
                oss << +matrixData->m_intraQuantiserMatrix[i + j] << " ";
            oss << std::endl;
        }
    }
    if (matrixData->m_loadNonIntraQuantiserMatrix)
    {
        oss << "non_intra_QmatrixData:" << std::endl;

        for (uint8_t i = 0; i < 56; i += 8)
        {
            oss << "Qmatrix[" << +i / 8 << "]: ";
            for (uint8_t j = 0; j < 8; j++)
                oss << +matrixData->m_nonIntraQuantiserMatrix[i + j] << " ";
            oss << std::endl;
        }
    }
    if (matrixData->m_loadChromaIntraQuantiserMatrix)
    {
        oss << "chroma_intra_QmatrixData:" << std::endl;

        for (uint8_t i = 0; i < 56; i += 8)
        {
            oss << "Qmatrix[" << +i / 8 << "]: ";
            for (uint8_t j = 0; j < 8; j++)
                oss << +matrixData->m_chromaIntraQuantiserMatrix[i + j] << " ";
            oss << std::endl;
        }
    }
    if (matrixData->m_loadChromaNonIntraQuantiserMatrix)
    {
        oss << "chroma_non_intra_QmatrixData:" << std::endl;

        for (uint8_t i = 0; i < 56; i += 8)
        {
            oss << "Qmatrix[" << +i / 8 << "]: ";
            for (uint8_t j = 0; j < 8; j++)
                oss << +matrixData->m_chromaNonIntraQuantiserMatrix[i + j] << " ";
            oss << std::endl;
        }
    }

    const char *fileName = m_debugInterface->CreateFileName(
        "_DEC",
        CodechalDbgBufferType::bufIqParams,
        CodechalDbgExtType::txt);

    std::ofstream ofs(fileName, std::ios::out);
    ofs << oss.str();
    ofs.close();

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalDecodeMpeg2::DumpMbParams(
    CodecDecodeMpeg2MbParmas *mbParams)
{
    CODECHAL_DEBUG_FUNCTION_ENTER;

    if (!m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrMbParams))
    {
        return MOS_STATUS_SUCCESS;
    }

    CODECHAL_DEBUG_CHK_NULL(mbParams);

    std::ostringstream oss;
    oss.setf(std::ios::showbase | std::ios::uppercase);

    oss << "m_mbAddr: " << +mbParams->m_mbAddr << std::endl;
    //Dump union MBType
    oss << "MBType.m_intraMb: " << +mbParams->MBType.m_intraMb << std::endl;
    oss << "MBType.m_motionFwd: " << +mbParams->MBType.m_motionFwd << std::endl;
    oss << "MBType.m_motionBwd: " << +mbParams->MBType.m_motionBwd << std::endl;
    oss << "MBType.m_motion4mv: " << +mbParams->MBType.m_motion4mv << std::endl;
    oss << "MBType.m_h261Lpfilter: " << +mbParams->MBType.m_h261Lpfilter << std::endl;
    oss << "MBType.m_fieldResidual: " << +mbParams->MBType.m_fieldResidual << std::endl;
    oss << "MBType.m_mbScanMethod: " << +mbParams->MBType.m_mbScanMethod << std::endl;
    oss << "MBType.m_motionType: " << +mbParams->MBType.m_motionType << std::endl;
    oss << "MBType.m_hostResidualDiff: " << +mbParams->MBType.m_hostResidualDiff << std::endl;
    oss << "MBType.m_mvertFieldSel: " << +mbParams->MBType.m_mvertFieldSel << std::endl;
    oss << "m_mbSkipFollowing: " << +mbParams->m_mbSkipFollowing << std::endl;
    oss << "m_mbDataLoc: " << +mbParams->m_mbDataLoc << std::endl;
    oss << "m_codedBlockPattern: " << +mbParams->m_codedBlockPattern << std::endl;

    //Dump NumCoeff[CODEC_NUM_BLOCK_PER_MB]
    for (uint16_t i = 0; i < CODEC_NUM_BLOCK_PER_MB; ++i)
    {
        oss << "m_numCoeff[" << +i << "]: " << +mbParams->m_numCoeff[i] << std::endl;
    }

    //Dump motion_vectors[8],printing them in 4 value chunks per line
    for (uint8_t i = 0; i < 2; ++i)
    {
        oss << "m_motionVectors[" << +i * 4 << "-" << (+i * 4) + 3 << "]: ";
        for (uint8_t j = 0; j < 4; j++)
            oss << +mbParams->m_motionVectors[i * 4 + j] << " ";
        oss << std::endl;
    }

    const char *fileName = m_debugInterface->CreateFileName(
        "_DEC",
        CodechalDbgBufferType::bufMbParams,
        CodechalDbgExtType::txt);

    std::ofstream ofs(fileName, std::ios::out);
    ofs << oss.str();
    ofs.close();
    return MOS_STATUS_SUCCESS;
}

#endif
