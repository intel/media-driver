/*
* Copyright (c) 2018-2021, Intel Corporation
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
//! \file     encode_status_reporte.cpp
//! \brief    Defines the common interface for encode status reporter
//! \details  
//!
#include <cmath>
#include "encode_status_report.h"

namespace encode {

    const uint32_t  EncoderStatusReport::m_codecFuncToFuncIdPairs[m_maxCodecFuncNum] =
    {
        0,
        CODECHAL_ENCODE_ENC_ID,
        CODECHAL_ENCODE_PAK_ID,
        CODECHAL_ENCODE_ENC_PAK_ID,
        CODECHAL_ENCODE_ENC_ID,
        CODECHAL_ENCODE_ENC_PAK_ID,
        0,
        0,
        CODECHAL_ENCODE_FEI_PRE_ENC_ID,
        CODECHAL_ENCODE_FEI_ENC_ID,
        CODECHAL_ENCODE_FEI_PAK_ID,
        CODECHAL_ENCODE_FEI_ENC_PAK_ID
    };

    EncoderStatusReport::EncoderStatusReport(
        EncodeAllocator *allocator, PMOS_INTERFACE pOsInterface, bool enableMfx, bool enableRcs, bool enablecp):
        m_osInterface(pOsInterface),
        m_enableMfx(enableMfx),
        m_enableRcs(enableRcs),
        m_enableCp(enablecp),
        m_allocator(allocator)
    {
        m_sizeOfReport = sizeof(EncodeStatusReportData);
    }

    EncoderStatusReport::~EncoderStatusReport()
    {
        Destroy();
    }

    MOS_STATUS EncoderStatusReport::Create()
    {
        ENCODE_FUNC_CALL();

        MOS_ALLOC_GFXRES_PARAMS param;
        MOS_ZeroMemory(&param, sizeof(MOS_ALLOC_GFXRES_PARAMS));
        param.Type     = MOS_GFXRES_BUFFER;
        param.TileType = MOS_TILE_LINEAR;
        param.Format   = Format_Buffer;
        param.dwBytes  = sizeof(uint32_t) * 2;
        param.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_NOCACHE;
        param.pBufName = "StatusQueryBufferGlobalCount";
        // keeping status buffer persistent since its used in all command buffers
        param.bIsPersistent = true;

        m_completedCountBuf = m_allocator->AllocateResource(param, true);
        ENCODE_CHK_STATUS_RETURN(m_allocator->SkipResourceSync(m_completedCountBuf));

        m_completedCount = (uint32_t *)m_allocator->LockResourceForRead(m_completedCountBuf);
        ENCODE_CHK_NULL_RETURN(m_completedCount);

        if (m_enableMfx)
        {
            param.dwBytes  = m_statusBufSizeMfx * CODECHAL_ENCODE_STATUS_NUM;
            param.pBufName = "StatusQueryBufferMfx";
            // keeping status buffer persistent since its used in all command buffers
            param.bIsPersistent = true;
            m_statusBufMfx  = m_allocator->AllocateResource(param, true);
            ENCODE_CHK_STATUS_RETURN(m_allocator->SkipResourceSync(m_statusBufMfx));

            m_dataStatusMfx = (uint8_t *)m_allocator->LockResourceForRead(m_statusBufMfx);
            ENCODE_CHK_NULL_RETURN(m_dataStatusMfx);
        }

        if (m_enableRcs)
        {
            param.dwBytes  = m_statusBufSizeRcs * CODECHAL_ENCODE_STATUS_NUM;
            param.pBufName = "StatusQueryBufferRcs";
            // keeping status buffer persistent since its used in all command buffers
            param.bIsPersistent = true;
            m_statusBufRcs = m_allocator->AllocateResource(param, true);
            ENCODE_CHK_STATUS_RETURN(m_allocator->SkipResourceSync(m_statusBufRcs));

            m_dataStatusRcs = (uint8_t *)m_allocator->LockResourceForRead(m_statusBufRcs);
            ENCODE_CHK_NULL_RETURN(m_dataStatusRcs);
        }

        if (m_enableCp)  // && m_skipFrameBasedHWCounterRead == false)
        {
            param.dwBytes       = sizeof(HwCounter) * CODECHAL_ENCODE_STATUS_NUM + sizeof(HwCounter);
            param.pBufName      = "HWCounterQueryBuffer";
            param.bIsPersistent = true;  // keeping status buffer persistent since its used in all command buffers
            m_hwcounterBuf      = m_allocator->AllocateResource(param, false);
            ENCODE_CHK_STATUS_RETURN(m_allocator->SkipResourceSync(m_hwcounterBuf));

            m_hwcounterBase = (uint32_t *)m_allocator->LockResourceWithNoOverwrite(m_hwcounterBuf);
            ENCODE_CHK_NULL_RETURN(m_hwcounterBase);
        }

        m_submittedCount = 0;
        m_reportedCount  = 0;

        m_statusBufAddr  = MOS_NewArray(StatusBufAddr, statusReportMaxNum);
        ENCODE_CHK_NULL_RETURN(m_statusBufAddr);

        m_statusBufAddr[statusReportGlobalCount].osResource  = m_completedCountBuf;
        m_statusBufAddr[statusReportGlobalCount].offset      = 0;
        m_statusBufAddr[statusReportGlobalCount].bufSize     = sizeof(uint32_t) * 2;

        for (int i = statusReportRCSStart; i < statusReportRcsMaxNum; i++)
        {
            m_statusBufAddr[i].osResource  = m_statusBufRcs;
            m_statusBufAddr[i].bufSize     = m_statusBufSizeRcs;
            m_statusBufAddr[i].offset      = i * sizeof(uint32_t) * 2;
        }

        for (int i = statusReportMfx; i < statusReportMfxMaxNum; i++)
        {
            m_statusBufAddr[i].osResource = m_statusBufMfx;
            m_statusBufAddr[i].bufSize    = m_statusBufSizeMfx;
        }

        SetOffsetsForStatusBufMfx();

        return MOS_STATUS_SUCCESS;
    }

    void EncoderStatusReport::SetOffsetsForStatusBufMfx()
    {
        m_statusBufAddr[statusReportMfx].offset                                   = 0;
        m_statusBufAddr[statusReportMfxBitstreamByteCountPerFrame].offset         = CODECHAL_OFFSETOF(EncodeStatusMfx, mfcBitstreamByteCountPerFrame);
        m_statusBufAddr[statusReportMfxBitstreamSyntaxElementOnlyBitCount].offset = CODECHAL_OFFSETOF(EncodeStatusMfx, mfcBitstreamSyntaxElementOnlyBitCount);
        m_statusBufAddr[statusReportImageStatusMask].offset                       = CODECHAL_OFFSETOF(EncodeStatusMfx, imageStatusMask);
        m_statusBufAddr[statusReportImageStatusCtrl].offset                       = CODECHAL_OFFSETOF(EncodeStatusMfx, imageStatusCtrl);
        m_statusBufAddr[statusReportHucStatusRegMask].offset                      = CODECHAL_OFFSETOF(EncodeStatusMfx, hucStatusRegMask);
        m_statusBufAddr[statusReportHucStatusReg].offset                          = CODECHAL_OFFSETOF(EncodeStatusMfx, hucStatusReg);
        m_statusBufAddr[statusReportHucStatus2Reg].offset                         = CODECHAL_OFFSETOF(EncodeStatusMfx, hucStatus2Reg);
        m_statusBufAddr[statusReportNumSlices].offset                             = CODECHAL_OFFSETOF(EncodeStatusMfx, numSlices);
        m_statusBufAddr[statusReportErrorFlags].offset                            = CODECHAL_OFFSETOF(EncodeStatusMfx, errorFlags);
        m_statusBufAddr[statusReportBRCQPReport].offset                           = CODECHAL_OFFSETOF(EncodeStatusMfx, brcQPReport);
        m_statusBufAddr[statusReportNumberPasses].offset                          = CODECHAL_OFFSETOF(EncodeStatusMfx, numberPasses);
        m_statusBufAddr[statusReportHeaderBytesInserted].offset                   = CODECHAL_OFFSETOF(EncodeStatusMfx, headerBytesInserted);
        m_statusBufAddr[statusReportQPStatusCount].offset                         = CODECHAL_OFFSETOF(EncodeStatusMfx, qpStatusCount);
        m_statusBufAddr[statusReportPictureCodingType].offset                     = CODECHAL_OFFSETOF(EncodeStatusMfx, pictureCodingType);
        m_statusBufAddr[statusReportLoopFilterLevel].offset                       = CODECHAL_OFFSETOF(EncodeStatusMfx, loopFilterLevel);
        m_statusBufAddr[statusReportImageStatusCtrlOfLastBRCPass].offset          = CODECHAL_OFFSETOF(EncodeStatusMfx, imageStatusCtrlOfLastBRCPass);
        m_statusBufAddr[statusReportSceneChangedFlag].offset                      = CODECHAL_OFFSETOF(EncodeStatusMfx, sceneChangedFlag);
        m_statusBufAddr[statusReportSumSquareError].offset                        = CODECHAL_OFFSETOF(EncodeStatusMfx, sumSquareError[0]);
        m_statusBufAddr[statusReportSadLuma].offset                               = CODECHAL_OFFSETOF(EncodeStatusMfx, sadLuma);
        m_statusBufAddr[statusReportNumIntra4x4Block].offset                      = CODECHAL_OFFSETOF(EncodeStatusMfx, numIntra4x4Block);
        m_statusBufAddr[statusReportNumInterSkip4x4Block].offset                  = CODECHAL_OFFSETOF(EncodeStatusMfx, numInterSkip4x4Block);
        m_statusBufAddr[statusReportNumSkip8x8Block].offset                       = CODECHAL_OFFSETOF(EncodeStatusMfx, numSkip8x8Block);
        m_statusBufAddr[statusReportSliceReport].offset                           = CODECHAL_OFFSETOF(EncodeStatusMfx, sliceReport);
        m_statusBufAddr[statusReportLpla].offset                                  = CODECHAL_OFFSETOF(EncodeStatusMfx, lookaheadStatus);
    }

    MOS_STATUS EncoderStatusReport::Init(void *inputPar)
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        EncoderStatusParameters *inputParameters = (EncoderStatusParameters *)inputPar;
        uint32_t submitIndex                     = CounterToIndex(m_submittedCount);

        if (inputParameters)
        {
            m_statusReportData[submitIndex].usedVdBoxNumber    = inputParameters->numUsedVdbox;
            m_statusReportData[submitIndex].statusReportNumber = inputParameters->statusReportFeedbackNumber;
            m_statusReportData[submitIndex].currOriginalPic    = inputParameters->currOriginalPic;
            m_statusReportData[submitIndex].currRefList        = inputParameters->currRefList;
            m_statusReportData[submitIndex].numberTilesInFrame = inputParameters->numberTilesInFrame;

            m_statusReportData[submitIndex].av1EnableFrameOBU            = inputParameters->av1EnableFrameObu;
            m_statusReportData[submitIndex].av1FrameHdrOBUSizeByteOffset = inputParameters->av1FrameHdrOBUSizeByteOffset;
            m_statusReportData[submitIndex].frameWidth                   = inputParameters->frameWidth;
            m_statusReportData[submitIndex].frameHeight                  = inputParameters->frameHeight;

            m_statusReportData[submitIndex].pBlkQualityInfo = (encode::EncodeStatusReportData::BLOCK_QUALITY_INFO *)(inputParameters->pBlkQualityInfo);

            uint64_t pairIndex = GetIdForCodecFuncToFuncIdPairs(inputParameters->codecFunction);
            if (pairIndex >= m_maxCodecFuncNum)
            {
                return MOS_STATUS_INVALID_PARAMETER;
            }

            m_statusReportData[submitIndex].func = (CODECHAL_ENCODE_FUNCTION_ID)m_codecFuncToFuncIdPairs[pairIndex];

            m_hwWalker             = inputParameters->hwWalker;
            m_picWidthInMb         = inputParameters->picWidthInMb;
            m_frameFieldHeightInMb = inputParameters->frameFieldHeightInMb;
            m_maxNumSlicesAllowed  = inputParameters->maxNumSlicesAllowed;
        }

        if (m_enableMfx)
        {
            EncodeStatusMfx* encodeStatusMfx = (EncodeStatusMfx*)(m_dataStatusMfx + submitIndex * m_statusBufSizeMfx);
            encodeStatusMfx->status = querySkipped;
            if (inputParameters)
            {
                encodeStatusMfx->pictureCodingType = inputParameters->pictureCodingType;
            }
        }

        if (m_enableRcs)
        {
            EncodeStatusRcs* encodeStatusRcs = (EncodeStatusRcs*)(m_dataStatusRcs + submitIndex * m_statusBufSizeRcs);
            for( auto i = 0; i < statusReportRcsMaxNum; i++)
            {
                encodeStatusRcs->executingStatus[i].status = querySkipped;
            }
        }
        return eStatus;
    }

    MOS_STATUS EncoderStatusReport::Reset()
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;
        
        m_submittedCount++;

        uint32_t submitIndex = CounterToIndex(m_submittedCount);

        if (m_enableMfx)
        {
            EncodeStatusMfx *encodeStatusMfx = (EncodeStatusMfx *)(m_dataStatusMfx + submitIndex * m_statusBufSizeMfx);
            MOS_ZeroMemory((uint8_t*)encodeStatusMfx, m_statusBufSizeMfx);
        }

        if (m_enableRcs)
        {
            EncodeStatusRcs *encodeStatusRcs = (EncodeStatusRcs *)(m_dataStatusRcs + submitIndex * m_statusBufSizeRcs);
            MOS_ZeroMemory((uint8_t*)encodeStatusRcs, m_statusBufSizeRcs);
        }

        return eStatus;
    }

    PMOS_RESOURCE EncoderStatusReport::GetHwCtrBuf()
    {
        return m_hwcounterBuf;
    }

    MOS_STATUS EncoderStatusReport::GetCommonMfxReportData(EncodeStatusReportData *statusReportData, uint32_t index)
    {
        EncodeStatusMfx *encodeStatusMfx = nullptr;
        bool            completed = false;

        if (!m_enableMfx)
        {
            return MOS_STATUS_SUCCESS;
        }

        encodeStatusMfx = (EncodeStatusMfx*)(m_dataStatusMfx + index * m_statusBufSizeMfx);
        completed       = (encodeStatusMfx->status == queryEnd);

        statusReportData->bitstreamSize       =
            encodeStatusMfx->mfcBitstreamByteCountPerFrame + encodeStatusMfx->headerBytesInserted;

        statusReportData->qpY                 = encodeStatusMfx->brcQPReport.DW0.qpPrimeY;
        statusReportData->suggestedQPYDelta   = encodeStatusMfx->imageStatusCtrl.cumulativeSliceDeltaQP;
        statusReportData->numberPasses        = (uint8_t)(encodeStatusMfx->imageStatusCtrl.totalNumPass + 1);
        statusReportData->sceneChangeDetected =
            (encodeStatusMfx->sceneChangedFlag & CODECHAL_ENCODE_SCENE_CHANGE_DETECTED_MASK) ? 1 : 0;

        if (m_picWidthInMb != 0 && m_frameFieldHeightInMb != 0)
        {
            statusReportData->averageQP= (unsigned char)(((uint32_t)encodeStatusMfx->qpStatusCount.cumulativeQP)
                / (m_picWidthInMb * m_frameFieldHeightInMb));
        }
        statusReportData->panicMode = encodeStatusMfx->imageStatusCtrl.panic;

        PakNumberOfSlices *numSlices = &encodeStatusMfx->numSlices;

        // If Num slices is greater than spec limit set NumSlicesNonCompliant to 1 and report error
        if (numSlices->numberOfSlices > m_maxNumSlicesAllowed)
        {
            statusReportData->numSlicesNonCompliant = 1;
        }
        statusReportData->numberSlices = numSlices->numberOfSlices;

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS EncoderStatusReport::UpdateCodecStatus(
        EncodeStatusReportData *statusReportData,
        EncodeStatusRcs *encodeStatusRcs,
        bool completed)
    {
        ENCODE_CHK_NULL_RETURN(statusReportData);
        ENCODE_CHK_NULL_RETURN(encodeStatusRcs);

        if (statusReportData->func != CODECHAL_ENCODE_ENC_ID &&
            statusReportData->func != CODECHAL_ENCODE_FEI_ENC_ID &&
            !completed)
        {
            if(statusReportData->func == CODECHAL_ENCODE_FEI_PRE_ENC_ID)
            {
                statusReportData->codecStatus = CODECHAL_STATUS_SUCCESSFUL;
            }
            else
            {
                // Add debug info here -> Media reset may have occured.
                statusReportData->codecStatus = CODECHAL_STATUS_ERROR;
            }
        }
        else if (m_hwWalker && statusReportData->func == CODECHAL_ENCODE_ENC_ID)
        {
            // iterate over all media states and check that all of them completed
            for (auto j = 0; j < statusReportRcsMaxNum; j += 1)
            {
                if (encodeStatusRcs->executingStatus[j].status != queryEnd)
                {
                    // some media state failed to complete
                    // Add debug info here -> Error: Unable to finish encoding.
                    statusReportData->codecStatus = CODECHAL_STATUS_ERROR;
                    break;
                }
            }

            statusReportData->codecStatus = CODECHAL_STATUS_SUCCESSFUL;
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS EncoderStatusReport::ParseStatus(void *report, uint32_t index)
    {
        MOS_STATUS      eStatus          = MOS_STATUS_SUCCESS;
        EncodeStatusMfx *encodeStatusMfx = nullptr;
        EncodeStatusRcs *encodeStatusRcs = nullptr;
        bool            completed        = false;

        EncodeStatusReportData *statusReportData = &m_statusReportData[index];

        statusReportData->pFrmStatsInfo = ((EncodeStatusReportData *)report)->pFrmStatsInfo;
        statusReportData->pBlkStatsInfo = ((EncodeStatusReportData *)report)->pBlkStatsInfo;

        if (m_enableRcs)
        {
            encodeStatusRcs = (EncodeStatusRcs*)(m_dataStatusRcs + index * m_statusBufSizeRcs);
            completed       = (encodeStatusRcs->executingStatus[statusReportFrameStatus].status == queryEnd);
        }

        if (m_enableMfx)
        {
            encodeStatusMfx = (EncodeStatusMfx*)(m_dataStatusMfx + index * m_statusBufSizeMfx);
            completed       = (encodeStatusMfx->status == queryEnd);
        }

        if (m_enableCp)
        {
            m_hwcounter = (uint64_t *)(((char *)m_hwcounterBase) + (index * sizeof(HwCounter)));
            statusReportData->hwCtr = m_hwcounter;
        }
        GetCommonMfxReportData(statusReportData, index);

        statusReportData->codecStatus = CODECHAL_STATUS_SUCCESSFUL;

        // Need add GPU Hang check here
        UpdateCodecStatus(statusReportData, encodeStatusRcs, completed);

        if ((statusReportData->codecStatus == CODECHAL_STATUS_ERROR) && encodeStatusMfx && (encodeStatusMfx->lookaheadStatus.targetFrameSize != 0))
        {
            statusReportData->codecStatus = CODECHAL_STATUS_SUCCESSFUL;
        }

        // The frame is completed, notify the observers
        if (statusReportData->codecStatus == CODECHAL_STATUS_SUCCESSFUL)
        {
            eStatus = NotifyObservers(encodeStatusMfx, encodeStatusRcs, statusReportData);
        }

        NullHW::StatusReport(m_osInterface, 
                             (uint32_t &)statusReportData->codecStatus,
                             statusReportData->bitstreamSize);

        *((EncodeStatusReportData *)report) = *statusReportData;
        return eStatus;

    }

    MOS_STATUS EncoderStatusReport::SetStatus(void *report, uint32_t index, bool outOfRange)
    {
        EncodeStatusReportData *statusReportData = &m_statusReportData[index];

        statusReportData->codecStatus = outOfRange ? CODECHAL_STATUS_UNAVAILABLE : CODECHAL_STATUS_INCOMPLETE;

        *((EncodeStatusReportData *)report) = *statusReportData;
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS EncoderStatusReport::Destroy()
    {
        ENCODE_FUNC_CALL();

        if (m_completedCountBuf != nullptr)
        {
            m_allocator->UnLock(m_completedCountBuf);
            m_allocator->DestroyResource(m_completedCountBuf);
            m_completedCountBuf = nullptr;
        }

        for(auto &statusReportData: m_statusReportData)
        {
            if(statusReportData.hevcTileinfo != nullptr)
            {
                MOS_FreeMemory(statusReportData.hevcTileinfo);
                statusReportData.hevcTileinfo = nullptr;
            }
        }

        if (m_statusBufMfx != nullptr)
        {
            m_allocator->UnLock(m_statusBufMfx);
            m_allocator->DestroyResource(m_statusBufMfx);
            m_statusBufMfx = nullptr;
        }

        if (m_statusBufRcs != nullptr)
        {
            m_allocator->UnLock(m_statusBufRcs);
            m_allocator->DestroyResource(m_statusBufRcs);
            m_statusBufRcs = nullptr;
        }

        if (m_statusBufAddr != nullptr)
        {
            MOS_DeleteArray(m_statusBufAddr);
            m_statusBufAddr = nullptr;
        }


        return MOS_STATUS_SUCCESS;
    }
}
