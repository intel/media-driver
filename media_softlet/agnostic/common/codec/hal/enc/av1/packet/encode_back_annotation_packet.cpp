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
//! \file     encode_back_annotation_packet.cpp
//! \brief    Defines the interface for AV1 back annotation packet
//!
#include "encode_back_annotation_packet.h"
#include "encode_av1_tile.h"

namespace encode {
    MOS_STATUS Av1BackAnnotationPkt::Init()
    {
        ENCODE_FUNC_CALL();
        ENCODE_CHK_STATUS_RETURN(EncodeHucPkt::Init());

        m_basicFeature = dynamic_cast<Av1BasicFeature *>(m_featureManager->GetFeature(Av1FeatureIDs::basicFeature));
        ENCODE_CHK_NULL_RETURN(m_basicFeature);

        ENCODE_CHK_NULL_RETURN(m_pipeline);
        m_statusReport = m_pipeline->GetStatusReportInstance();
        ENCODE_CHK_NULL_RETURN(m_statusReport);

        ENCODE_CHK_STATUS_RETURN(m_statusReport->RegistObserver(this));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1BackAnnotationPkt::AllocateResources()
    {
        ENCODE_CHK_STATUS_RETURN(EncodeHucPkt::AllocateResources());

        MOS_ALLOC_GFXRES_PARAMS allocParamsForBufferLinear;
        MOS_ZeroMemory(&allocParamsForBufferLinear, sizeof(MOS_ALLOC_GFXRES_PARAMS));
        allocParamsForBufferLinear.Type = MOS_GFXRES_BUFFER;
        allocParamsForBufferLinear.TileType = MOS_TILE_LINEAR;
        allocParamsForBufferLinear.Format = Format_Buffer;

        for (auto k = 0; k < CODECHAL_ENCODE_RECYCLED_BUFFER_NUM; k++)
        {
            for (auto i = 0; i < VDENC_BRC_NUM_OF_PASSES; i++)
            {
                // BRC update DMEM
                allocParamsForBufferLinear.dwBytes = MOS_ALIGN_CEIL(m_vdencbackAnnotationDmemBufferSize, CODECHAL_CACHELINE_SIZE);
                allocParamsForBufferLinear.pBufName = "AV1 Back Annotation Dmem Buffer";
                allocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_WRITE;
                m_vdencBackAnnotationDmemBuffer[k][i] = m_allocator->AllocateResource(allocParamsForBufferLinear, true);

                allocParamsForBufferLinear.dwBytes = MOS_ALIGN_CEIL(m_vdencAv1HucCtrlBufferSize, CODECHAL_CACHELINE_SIZE);
                allocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_WRITE;
                allocParamsForBufferLinear.pBufName = "AV1 Back Annotation Huc Ctrl Buffer";
                m_vdencAv1HucCtrlBuffer[k][i] = m_allocator->AllocateResource(allocParamsForBufferLinear, true);
            }
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1BackAnnotationPkt::Prepare()
    {
        ENCODE_FUNC_CALL();

        ENCODE_CHK_NULL_RETURN(m_basicFeature);

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1BackAnnotationPkt::Submit(MOS_COMMAND_BUFFER *commandBuffer, uint8_t packetPhase)
    {
        ENCODE_FUNC_CALL();

        bool firstTaskInPhase = packetPhase & firstPacket;
        bool requestProlog = false;

        ENCODE_CHK_STATUS_RETURN(Execute(commandBuffer, true, requestProlog));

        CODECHAL_DEBUG_TOOL
        (
            ENCODE_CHK_STATUS_RETURN(DumpBackAnnotation());
        )

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1BackAnnotationPkt::CalculateCommandSize(uint32_t &commandBufferSize, uint32_t &requestedPatchListSize)
    {
        ENCODE_FUNC_CALL();

        auto osInterface = m_hwInterface->GetOsInterface();
        ENCODE_CHK_NULL_RETURN(osInterface);

        uint32_t hucCommandsSize = 0;
        uint32_t hucPatchListSize = 0;
        MHW_VDBOX_STATE_CMDSIZE_PARAMS stateCmdSizeParams;

        ENCODE_CHK_STATUS_RETURN(m_hwInterface->GetHucStateCommandSize(
            m_basicFeature->m_mode, (uint32_t*)&hucCommandsSize, (uint32_t*)&hucPatchListSize, &stateCmdSizeParams));

        commandBufferSize = hucCommandsSize;
        requestedPatchListSize = osInterface->bUsesPatchList ? hucPatchListSize : 0;

        // 4K align since allocation is in chunks of 4K bytes.
        commandBufferSize = MOS_ALIGN_CEIL(commandBufferSize, CODECHAL_PAGE_SIZE);

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1BackAnnotationPkt::SetDmemBuffer()
    {
        ENCODE_FUNC_CALL();

        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        int32_t currentPass = m_pipeline->GetCurrentPass();
        if (currentPass < 0 || currentPass >= CODECHAL_VDENC_BRC_NUM_OF_PASSES)
        {
            eStatus = MOS_STATUS_INVALID_PARAMETER;
            return eStatus;
        }

        // Setup BrcInit DMEM
        auto hucBackAnnotationDmem = (VdencAv1HucBackAnnotationDmem *)
            m_allocator->LockResourceForWrite(m_vdencBackAnnotationDmemBuffer[m_pipeline->m_currRecycledBufIdx][currentPass]);
        ENCODE_CHK_NULL_RETURN(hucBackAnnotationDmem);
        MOS_ZeroMemory(hucBackAnnotationDmem, sizeof(VdencAv1HucBackAnnotationDmem));

        uint32_t numTilegroups = 0;
        PCODEC_AV1_ENCODE_TILE_GROUP_PARAMS tileGroupParams = nullptr;
        RUN_FEATURE_INTERFACE_RETURN(Av1EncodeTile, Av1FeatureIDs::encodeTile, GetTileGroupInfo, tileGroupParams, numTilegroups);

        hucBackAnnotationDmem->tileGroupNumber = static_cast<uint8_t>(numTilegroups);
        hucBackAnnotationDmem->backAnnotationType = AnnotationTypes::TileGroupOBU;
        hucBackAnnotationDmem->firstTileGroupByteOffset = m_basicFeature->GetAppHdrSizeInBytes();

        ENCODE_CHK_STATUS_RETURN(m_allocator->UnLock(m_vdencBackAnnotationDmemBuffer[m_pipeline->m_currRecycledBufIdx][currentPass]));

        return eStatus;
    }

    MOS_STATUS Av1BackAnnotationPkt::SetHucCtrlBuffer()
    {
        ENCODE_FUNC_CALL();

        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        ENCODE_CHK_NULL_RETURN(m_basicFeature);
        auto nalUnitParams = m_basicFeature->m_nalUnitParams;
        ENCODE_CHK_NULL_RETURN(nalUnitParams);

        int32_t currentPass = m_pipeline->GetCurrentPass();
        if (currentPass < 0 || currentPass >= CODECHAL_VDENC_BRC_NUM_OF_PASSES)
        {
            eStatus = MOS_STATUS_INVALID_PARAMETER;
            return eStatus;
        }

        // Setup BrcInit DMEM
        auto hucCtrlBuffer = (VdencAv1HucCtrlBigData *)
            m_allocator->LockResourceForWrite(m_vdencAv1HucCtrlBuffer[m_pipeline->m_currRecycledBufIdx][currentPass]);
        ENCODE_CHK_NULL_RETURN(hucCtrlBuffer);
        MOS_ZeroMemory(hucCtrlBuffer, sizeof(VdencAv1HucCtrlBigData));

        uint32_t nalunitPosiSize = 0;
        auto numOBUS = m_basicFeature->m_NumNalUnits;

        RUN_FEATURE_INTERFACE_RETURN(Av1EncodeTile, Av1FeatureIDs::encodeTile,
            SetHucCtrlBuffer, *hucCtrlBuffer);

        ENCODE_CHK_STATUS_RETURN(m_allocator->UnLock(m_vdencAv1HucCtrlBuffer[m_pipeline->m_currRecycledBufIdx][currentPass]));

        return eStatus;
    }

    MOS_STATUS Av1BackAnnotationPkt::Completed(void *mfxStatus, void *rcsStatus, void *statusReport)
    {
        ENCODE_FUNC_CALL();

        ENCODE_CHK_NULL_RETURN(mfxStatus);
        ENCODE_CHK_NULL_RETURN(statusReport);

        if (!m_basicFeature->m_enableSWBackAnnotation)
        {
            return MOS_STATUS_SUCCESS;
        }

        ENCODE_CHK_STATUS_RETURN(EncodeHucPkt::Completed(mfxStatus, rcsStatus, statusReport));

        EncodeStatusMfx *       encodeStatusMfx  = (EncodeStatusMfx *)mfxStatus;
        EncodeStatusReportData *statusReportData = (EncodeStatusReportData *)statusReport;

        uint32_t statBufIdx = statusReportData->currOriginalPic.FrameIdx;

        MOS_RESOURCE *tileSizeStatusBuffer = nullptr;
        RUN_FEATURE_INTERFACE_RETURN(Av1EncodeTile, Av1FeatureIDs::encodeTile, GetTileRecordBuffer, statBufIdx, tileSizeStatusBuffer);

        const Av1ReportTileGroupParams *tileGroupParams = nullptr;
        RUN_FEATURE_INTERFACE_RETURN(Av1EncodeTile, Av1FeatureIDs::encodeTile, GetTileGroupReportParams, statBufIdx, tileGroupParams);
        ENCODE_CHK_NULL_RETURN(tileGroupParams);

        uint32_t numTilegroups = tileGroupParams->TileGroupNum;

        PakHwTileSizeRecord *tileRecord =
            (PakHwTileSizeRecord *)m_allocator->LockResourceForRead(tileSizeStatusBuffer);
        ENCODE_CHK_NULL_RETURN(tileRecord);

        uint32_t obuSizeBytesOffset = 0;
        if (statusReportData->av1EnableFrameOBU)
        {
            obuSizeBytesOffset = statusReportData->av1FrameHdrOBUSizeByteOffset;
        }
        else
        {
            obuSizeBytesOffset = tileRecord[0].Length - tileRecord[0].TileSize - tileGroupParams->TileGroupOBUSizeInBytes - m_numBytesOfOBUSize;

            // tile_start_and_end_present_flag is in bitstream, indicates more than 1 tile, so tile_size_minus_1 exists in bitstream
            if (tileGroupParams->TileGroupOBUSizeInBytes && (tileGroupParams->TileGroupStart != tileGroupParams->TileGroupEnd))
            {
                obuSizeBytesOffset -= TILE_SIZE_BYTES;
            }
        }

        PCODEC_REF_LIST currRefList = (PCODEC_REF_LIST)statusReportData->currRefList;
        ENCODE_CHK_NULL_RETURN(currRefList);
        uint8_t *bitstream = (uint8_t *)m_allocator->LockResourceForWrite(
            &currRefList->resBitstreamBuffer);
        ENCODE_CHK_NULL_RETURN(bitstream);

        uint32_t payLoadSize = 0;
        uint8_t buffer[m_numBytesOfOBUSize] = {};
        for (uint32_t i = 0; i < numTilegroups; i++, tileGroupParams++)
        {
            uint32_t streamSizePerTG = 0;
            for (uint32_t j = tileGroupParams->TileGroupStart; j <= tileGroupParams->TileGroupEnd; j++)
            {
                if (tileRecord[j].Length == 0)
                {
                    statusReportData->codecStatus = CODECHAL_STATUS_INCOMPLETE;
                    return MOS_STATUS_SUCCESS;
                }
                if (j == tileGroupParams->TileGroupStart)
                {
                    auto tileSizeBytes = (j == tileGroupParams->TileGroupEnd) ?
                        tileGroupParams->TileGroupOBUSizeInBytes : tileGroupParams->TileGroupOBUSizeInBytes + TILE_SIZE_BYTES;
                    payLoadSize += tileRecord[j].TileSize + tileSizeBytes;
                }
                else
                {
                    payLoadSize += tileRecord[j].Length;
                }
            }
            streamSizePerTG += payLoadSize;

            //needs to decode the size from the one passed by MSDK
            if (statusReportData->av1EnableFrameOBU)
            {
                uint32_t frameHdrObuSize = tileRecord[0].Length - tileRecord[0].TileSize - obuSizeBytesOffset -
                    m_numBytesOfOBUSize - 1/*tile group OBU header size*/ - tileGroupParams->TileGroupOBUSizeInBytes;

                // tile_start_and_end_present_flag is in bitstream, indicates more than 1 tile, so tile_size_minus_1 exists in bitstream
                if (tileGroupParams->TileGroupOBUSizeInBytes && (tileGroupParams->TileGroupStart != tileGroupParams->TileGroupEnd))
                {
                    frameHdrObuSize -= TILE_SIZE_BYTES;
                }

                payLoadSize += frameHdrObuSize + 1/*tile group OBU header size*/;
            }

            // fixed 4 bytes solution
            uint8_t payLoadByte = 0;
            for (auto k = 0; k < m_numBytesOfOBUSize; k++)
            {
                payLoadByte = payLoadSize & 0x7f;
                payLoadSize >>= 7;

                if (k != m_numBytesOfOBUSize - 1)
                {
                    payLoadByte |= 0x80;  // Signal that more bytes follow.
                }

                *(buffer + k) = payLoadByte;
            }

            MOS_SecureMemcpy(
                bitstream + obuSizeBytesOffset,
                (uint32_t)m_numBytesOfOBUSize,
                buffer,
                (uint32_t)m_numBytesOfOBUSize);

            //reset pay load size for next TG
            payLoadSize = 0;
            // calculate next TileGroupBaseOffset
            obuSizeBytesOffset += streamSizePerTG + 1/*tile group OBU header size*/ + m_numBytesOfOBUSize;
        }

        if (bitstream)
        {
            m_allocator->UnLock(&currRefList->resBitstreamBuffer);
        }

        if (tileRecord)
        {
            // clean-up the tile status report buffer
            MOS_ZeroMemory(tileRecord, sizeof(tileRecord[0]) * statusReportData->numberTilesInFrame);
            m_allocator->UnLock(tileSizeStatusBuffer);
        }

        return MOS_STATUS_SUCCESS;
    }

#if USE_CODECHAL_DEBUG_TOOL
    MOS_STATUS Av1BackAnnotationPkt::DumpBackAnnotation()
    {
        ENCODE_FUNC_CALL();

        int32_t currentPass = m_pipeline->GetCurrentPass();

        CodechalDebugInterface *debugInterface = m_pipeline->GetDebugInterface();
        ENCODE_CHK_NULL_RETURN(debugInterface);

        ENCODE_CHK_STATUS_RETURN(debugInterface->DumpHucDmem(
            m_vdencBackAnnotationDmemBuffer[m_pipeline->m_currRecycledBufIdx][currentPass],
            m_vdencbackAnnotationDmemBufferSize,
            currentPass,
            hucRegionDumpBackAnnotation));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1BackAnnotationPkt::DumpOutput()
    {
        ENCODE_FUNC_CALL();
        ENCODE_CHK_STATUS_RETURN(DumpRegion(0, "_PakTileSizeStreamout", false, hucRegionDumpBackAnnotation));

        ENCODE_CHK_STATUS_RETURN(DumpRegion(3, "_Bitstream", false, hucRegionDumpBackAnnotation));
        ENCODE_CHK_STATUS_RETURN(DumpRegion(4, "_HucCtrl", false, hucRegionDumpBackAnnotation));

        return MOS_STATUS_SUCCESS;
    }
#endif
}
