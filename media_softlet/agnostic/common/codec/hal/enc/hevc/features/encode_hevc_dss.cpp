/*
* Copyright (c) 2019-2020, Intel Corporation
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
//! \file     encode_hevc_dss.cpp
//! \brief    Defines the common interface for hevc encode dynamic slice feature
//!

#include "encode_hevc_basic_feature.h"
#include "encode_hevc_dss.h"
#include "encode_hevc_vdenc_feature_manager.h"
#include "encode_hevc_vdenc_const_settings.h"

namespace encode
{
    HevcEncodeDss::HevcEncodeDss(
        MediaFeatureManager *featureManager,
        EncodeAllocator *allocator,
        CodechalHwInterfaceNext *hwInterface,
        void *constSettings) :
        MediaFeature(constSettings),
        m_allocator(allocator),
        m_hwInterface(hwInterface)
    {
        if (!hwInterface || !constSettings || !allocator)
            return;

        auto encFeatureManager = dynamic_cast<EncodeHevcVdencFeatureManager *>(featureManager);
        if (nullptr != encFeatureManager)
        {
            m_basicFeature = dynamic_cast<EncodeBasicFeature *>(encFeatureManager->GetFeature(FeatureIDs::basicFeature));
        }
    }
    /*
    HevcEncodeDss::~HevcEncodeDss()
    {
        //FreeDSSResources();
    }
    */
    MOS_STATUS HevcEncodeDss::Init(void *setting)
    {
        ENCODE_FUNC_CALL();

        m_hcpItf = m_hwInterface->GetHcpInterfaceNext();
        ENCODE_CHK_NULL_RETURN(m_hcpItf);

        m_miItf = m_hwInterface->GetMiInterfaceNext();
        ENCODE_CHK_NULL_RETURN(m_miItf);

        m_vdencItf = m_hwInterface->GetVdencInterfaceNext();
        ENCODE_CHK_NULL_RETURN(m_vdencItf);

        ENCODE_CHK_STATUS_RETURN(AllocateResources());

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HevcEncodeDss::AllocateResources()
    {
        ENCODE_FUNC_CALL();
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        ENCODE_CHK_NULL_RETURN(m_allocator);
        ENCODE_CHK_NULL_RETURN(m_basicFeature);
        ENCODE_CHK_NULL_RETURN(m_basicFeature->m_recycleBuf);
        ENCODE_CHK_NULL_RETURN(m_hwInterface);
        ENCODE_CHK_NULL_RETURN(m_hwInterface->GetOsInterface());

        MOS_ALLOC_GFXRES_PARAMS allocParamsForBufferLinear;
        MOS_ZeroMemory(&allocParamsForBufferLinear, sizeof(MOS_ALLOC_GFXRES_PARAMS));
        allocParamsForBufferLinear.Type     = MOS_GFXRES_BUFFER;
        allocParamsForBufferLinear.TileType = MOS_TILE_LINEAR;
        allocParamsForBufferLinear.Format   = Format_Buffer;

        // Slice Count buffer 1 DW = 4 Bytes
        allocParamsForBufferLinear.dwBytes    = MOS_ALIGN_CEIL(4, CODECHAL_CACHELINE_SIZE);
        allocParamsForBufferLinear.pBufName   = "Slice Count Buffer";
        allocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_NOCACHE;
        m_resSliceCountBuffer = m_allocator->AllocateResource(allocParamsForBufferLinear, false);

        // VDEncMode Timer buffer 1 DW = 4 Bytes
        allocParamsForBufferLinear.dwBytes        = MOS_ALIGN_CEIL(4, CODECHAL_CACHELINE_SIZE);
        allocParamsForBufferLinear.pBufName       = "VDEncMode Timer Buffer";
        allocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_NOCACHE;
        m_resVDEncModeTimerBuffer = m_allocator->AllocateResource(allocParamsForBufferLinear, false);

        return eStatus;
    }

    MOS_STATUS HevcEncodeDss::Update(void *params)
    {
        ENCODE_FUNC_CALL();
        ENCODE_CHK_NULL_RETURN(params);

        EncoderParams *encodeParams = (EncoderParams *)params;
        m_hevcSeqParams             = static_cast<PCODEC_HEVC_ENCODE_SEQUENCE_PARAMS>(encodeParams->pSeqParams);

        uint32_t frameWidth         = (m_hevcSeqParams->wFrameWidthInMinCbMinus1 + 1) << (m_hevcSeqParams->log2_min_coding_block_size_minus3 + 3);
        uint32_t frameHeight        = (m_hevcSeqParams->wFrameHeightInMinCbMinus1 + 1) << (m_hevcSeqParams->log2_min_coding_block_size_minus3 + 3);
        if (m_hevcSeqParams->SliceSizeControl && frameWidth * frameHeight < ENCODE_HEVC_MIN_DSS_PIC_WIDTH * ENCODE_HEVC_MIN_DSS_PIC_HEIGHT)
        {
            MOS_STATUS eStatus = MOS_STATUS_INVALID_PARAMETER;
            ENCODE_CHK_STATUS_MESSAGE_RETURN(eStatus, "DSS is not supported when frame resolution less than 320p");
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HevcEncodeDss::ReadHcpStatus(
        MHW_VDBOX_NODE_IND  vdboxIndex,
        MOS_COMMAND_BUFFER &cmdBuffer)
    {
        ENCODE_FUNC_CALL();

        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        auto mmioRegisters = m_hcpItf->GetMmioRegisters(vdboxIndex);
        ENCODE_CHK_NULL_RETURN(mmioRegisters);

        auto AddMiStoreRegisterMemCmd = [&](PMOS_RESOURCE &res, uint32_t dwRegister) {
            auto &miStoreRegMemParams           = m_miItf->MHW_GETPAR_F(MI_STORE_REGISTER_MEM)();
            miStoreRegMemParams                 = {};
            miStoreRegMemParams.presStoreBuffer = res;
            miStoreRegMemParams.dwOffset        = 0;
            miStoreRegMemParams.dwRegister      = dwRegister;
            ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_STORE_REGISTER_MEM)(&cmdBuffer));
            return MOS_STATUS_SUCCESS;
        };

        ENCODE_CHK_STATUS_RETURN(AddMiStoreRegisterMemCmd(m_resSliceCountBuffer, mmioRegisters->hcpEncSliceCountRegOffset));
        ENCODE_CHK_STATUS_RETURN(AddMiStoreRegisterMemCmd(m_resVDEncModeTimerBuffer, mmioRegisters->hcpEncVdencModeTimerRegOffset));

        return eStatus;
    }

    MOS_STATUS HevcEncodeDss::ReadSliceSizeForSinglePipe(EncodePipeline *pipeline,  MOS_COMMAND_BUFFER &cmdBuffer)
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        ENCODE_FUNC_CALL();
        MediaStatusReport *statusReport = pipeline->GetStatusReportInstance();
        MOS_RESOURCE *osResource = nullptr;
        uint32_t      offset = 0;
        ENCODE_CHK_STATUS_RETURN(statusReport->GetAddress(statusReportSliceReport, osResource, offset));
        
        uint32_t sizeOfSliceSizesBuffer = MOS_ALIGN_CEIL(CODECHAL_HEVC_MAX_NUM_SLICES_LVL_6 * CODECHAL_CACHELINE_SIZE, CODECHAL_PAGE_SIZE);

        uint32_t currIndex = statusReport->GetIndex(statusReport->GetSubmittedCount());

        if (pipeline->IsFirstPass())
        {
            // Create/ Initialize slice report buffer once per frame, to be used across passes
            if (Mos_ResourceIsNull(&m_resSliceReport[currIndex]))
            {
                MOS_ALLOC_GFXRES_PARAMS allocParamsForBufferLinear;
                MOS_ZeroMemory(&allocParamsForBufferLinear, sizeof(MOS_ALLOC_GFXRES_PARAMS));
                allocParamsForBufferLinear.Type     = MOS_GFXRES_BUFFER;
                allocParamsForBufferLinear.TileType = MOS_TILE_LINEAR;
                allocParamsForBufferLinear.Format   = Format_Buffer;
                allocParamsForBufferLinear.dwBytes  = sizeOfSliceSizesBuffer;
                allocParamsForBufferLinear.pBufName = "SliceReport";
                allocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_NOCACHE;
                MOS_RESOURCE *allocatedresource     = m_allocator->AllocateResource(allocParamsForBufferLinear, false);
                ENCODE_CHK_NULL_RETURN(allocatedresource);
                m_resSliceReport[currIndex] = *allocatedresource;
            }

            // Clear slice size structure to be sent in EncodeStatusReport buffer
            uint8_t *data = (uint8_t *)m_allocator->LockResourceForWrite(&m_resSliceReport[currIndex]);
            ENCODE_CHK_NULL_RETURN(data);
            MOS_ZeroMemory(data, sizeOfSliceSizesBuffer);
            m_allocator->UnLock(&m_resSliceReport[currIndex]);

            // Set slice size pointer in slice size structure
            auto &miFlushDwParams            = m_miItf->MHW_GETPAR_F(MI_FLUSH_DW)();
            miFlushDwParams                  = {};
            miFlushDwParams.pOsResource      = osResource;
            miFlushDwParams.dwResourceOffset = offset + CODECHAL_OFFSETOF(EncodeStatusSliceReport, sliceSize);
            miFlushDwParams.dwDataDW1        = (uint32_t)((uint64_t)&m_resSliceReport[currIndex] & 0xFFFFFFFF);
            miFlushDwParams.dwDataDW2        = (uint32_t)(((uint64_t)&m_resSliceReport[currIndex] & 0xFFFFFFFF00000000) >> 32);
            miFlushDwParams.bQWordEnable     = 1;
            ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_FLUSH_DW)(&cmdBuffer));
        }

        // Copy Slize size data buffer from PAK to be sent back to App
        ENCODE_CHK_STATUS_RETURN(CopyDataBlock(
            m_basicFeature->m_recycleBuf->GetBuffer(LcuBaseAddressBuffer, 0),
            0,
            &m_resSliceReport[currIndex],
            0,
            sizeOfSliceSizesBuffer,
            cmdBuffer));

        auto &miCpyMemMemParams       = m_miItf->MHW_GETPAR_F(MI_COPY_MEM_MEM)();
        miCpyMemMemParams             = {};
        miCpyMemMemParams.presSrc     = m_basicFeature->m_recycleBuf->GetBuffer(FrameStatStreamOutBuffer, 0);  // Slice size overflow is in m_resFrameStatStreamOutBuffer DW0[16]
        miCpyMemMemParams.dwSrcOffset = 0;
        miCpyMemMemParams.presDst     = osResource;
        miCpyMemMemParams.dwDstOffset = offset;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_COPY_MEM_MEM)(&cmdBuffer));

        miCpyMemMemParams             = {};
        miCpyMemMemParams.presSrc     = m_resSliceCountBuffer;  // Number of slice sizes are stored in this buffer. Updated at runtime
        miCpyMemMemParams.dwSrcOffset = 0;
        miCpyMemMemParams.presDst     = osResource;
        miCpyMemMemParams.dwDstOffset = offset + 1;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_COPY_MEM_MEM)(&cmdBuffer));

        return eStatus;
    }

    MOS_STATUS HevcEncodeDss::ReadSliceSize(EncodePipeline *pipeline, MOS_COMMAND_BUFFER &cmdBuffer)
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        ENCODE_FUNC_CALL();
        MediaStatusReport *statusReport = pipeline->GetStatusReportInstance();

        // In multi-tile multi-pipe mode, use PAK integration kernel output
        // PAK integration kernel accumulates frame statistics across tiles, which should be used to setup slice size report
        MOS_RESOURCE *osResource = nullptr;
        uint32_t      offset = 0;
        
        ENCODE_CHK_STATUS_RETURN(statusReport->GetAddress(statusReportSliceReport, osResource, offset));

        uint32_t sizeOfSliceSizesBuffer = MOS_ALIGN_CEIL(CODECHAL_HEVC_MAX_NUM_SLICES_LVL_6 * CODECHAL_CACHELINE_SIZE, CODECHAL_PAGE_SIZE);

        uint32_t currIndex = statusReport->GetIndex(statusReport->GetSubmittedCount());

        if (pipeline->IsFirstPipe())
        {
            if (pipeline->IsFirstPass())
            {
                // Create/ Initialize slice report buffer once per frame, to be used across passes
                if (Mos_ResourceIsNull(&m_resSliceReport[currIndex]))
                {
                    MOS_ALLOC_GFXRES_PARAMS allocParamsForBufferLinear;
                    MOS_ZeroMemory(&allocParamsForBufferLinear, sizeof(MOS_ALLOC_GFXRES_PARAMS));
                    allocParamsForBufferLinear.Type     = MOS_GFXRES_BUFFER;
                    allocParamsForBufferLinear.TileType = MOS_TILE_LINEAR;
                    allocParamsForBufferLinear.Format   = Format_Buffer;
                    allocParamsForBufferLinear.dwBytes  = sizeOfSliceSizesBuffer;
                    allocParamsForBufferLinear.pBufName = "SliceReport";
                    allocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_NOCACHE;
                    MOS_RESOURCE *allocatedresource     = m_allocator->AllocateResource(allocParamsForBufferLinear, false);
                    ENCODE_CHK_NULL_RETURN(allocatedresource);
                    m_resSliceReport[currIndex] = *allocatedresource;
                }

                // Clear slice size structure to be sent in EncodeStatusReportNext buffer
                uint8_t *data = (uint8_t *)m_allocator->LockResourceForWrite(&m_resSliceReport[currIndex]);
                ENCODE_CHK_NULL_RETURN(data);
                MOS_ZeroMemory(data, sizeOfSliceSizesBuffer);
                m_allocator->UnLock(&m_resSliceReport[currIndex]);

                // Set slice size pointer in slice size structure
                auto &miFlushDwParams            = m_miItf->MHW_GETPAR_F(MI_FLUSH_DW)();
                miFlushDwParams                  = {};
                miFlushDwParams.pOsResource      = osResource;
                miFlushDwParams.dwResourceOffset = offset + CODECHAL_OFFSETOF(EncodeStatusSliceReport, sliceSize);
                miFlushDwParams.dwDataDW1        = (uint32_t)((uint64_t)&m_resSliceReport[currIndex] & 0xFFFFFFFF);
                miFlushDwParams.dwDataDW2        = (uint32_t)(((uint64_t)&m_resSliceReport[currIndex] & 0xFFFFFFFF00000000) >> 32);
                miFlushDwParams.bQWordEnable     = 1;
                ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_FLUSH_DW)(&cmdBuffer));
            }

            uint32_t      statBufIdx           = 0;
            MOS_RESOURCE *tileStatisticsBuffer = nullptr;
            RUN_FEATURE_INTERFACE_RETURN(HevcEncodeTile, FeatureIDs::encodeTile, GetStatisticsBufferIndex, statBufIdx);
            RUN_FEATURE_INTERFACE_RETURN(HevcEncodeTile, FeatureIDs::encodeTile, GetTileBasedStatisticsBuffer, statBufIdx, tileStatisticsBuffer);
            ENCODE_CHK_NULL_RETURN(tileStatisticsBuffer);

            HevcTileStatusInfo hevcTileStatsOffset;
            HevcTileStatusInfo hevcFrameStatsOffset;
            HevcTileStatusInfo hevcStatsSize;
            RUN_FEATURE_INTERFACE_RETURN(HevcEncodeTile, FeatureIDs::encodeTile, GetTileStatusInfo, hevcTileStatsOffset, hevcFrameStatsOffset, hevcStatsSize);

            // Copy Slize size data buffer from PAK to be sent back to App
            ENCODE_CHK_STATUS_RETURN(CopyDataBlock(
                tileStatisticsBuffer,
                hevcTileStatsOffset.hevcSliceStreamout,
                &m_resSliceReport[currIndex],
                0,
                sizeOfSliceSizesBuffer,
                cmdBuffer));

            MOS_RESOURCE *resHuCPakAggregatedFrameStatsBuffer = nullptr;
            RUN_FEATURE_INTERFACE_RETURN(HevcEncodeTile, FeatureIDs::encodeTile, GetHucPakAggregatedFrameStatsBuffer, resHuCPakAggregatedFrameStatsBuffer);
            ENCODE_CHK_NULL_RETURN(resHuCPakAggregatedFrameStatsBuffer);
            
            auto &miCpyMemMemParams       = m_miItf->MHW_GETPAR_F(MI_COPY_MEM_MEM)();
            miCpyMemMemParams             = {};
            miCpyMemMemParams.presSrc     = resHuCPakAggregatedFrameStatsBuffer;  // Slice size overflow is in m_resFrameStatStreamOutBuffer DW0[16]
            miCpyMemMemParams.dwSrcOffset = hevcFrameStatsOffset.hevcPakStatistics;
            miCpyMemMemParams.presDst     = osResource;
            miCpyMemMemParams.dwDstOffset = offset;     // Slice size overflow is at DW0 EncodeStatusSliceReport
            ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_COPY_MEM_MEM)(&cmdBuffer));
        }

        return eStatus;
    }

    MOS_STATUS HevcEncodeDss::CopyDataBlock(
        PMOS_RESOURCE       sourceSurface,
        uint32_t            sourceOffset,
        PMOS_RESOURCE       destSurface,
        uint32_t            destOffset,
        uint32_t            copySize,
        MOS_COMMAND_BUFFER &cmdBuffer)
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        ENCODE_FUNC_CALL();

        CodechalHucStreamoutParams hucStreamOutParams;
        MOS_ZeroMemory(&hucStreamOutParams, sizeof(hucStreamOutParams));

        // Ind Obj Addr command
        hucStreamOutParams.dataBuffer            = sourceSurface;
        hucStreamOutParams.dataSize              = copySize + sourceOffset;
        hucStreamOutParams.dataOffset            = MOS_ALIGN_FLOOR(sourceOffset, CODECHAL_PAGE_SIZE);
        hucStreamOutParams.streamOutObjectBuffer = destSurface;
        hucStreamOutParams.streamOutObjectSize   = copySize + destOffset;
        hucStreamOutParams.streamOutObjectOffset = MOS_ALIGN_FLOOR(destOffset, CODECHAL_PAGE_SIZE);

        // Stream object params
        hucStreamOutParams.indStreamInLength    = copySize;
        hucStreamOutParams.inputRelativeOffset  = sourceOffset - hucStreamOutParams.dataOffset;
        hucStreamOutParams.outputRelativeOffset = destOffset - hucStreamOutParams.streamOutObjectOffset;

        ENCODE_CHK_STATUS_RETURN(m_hwInterface->PerformHucStreamOut(
            &hucStreamOutParams,
            &cmdBuffer));

        // wait Huc completion (use HEVC bit for now)
        auto &vdPipeFlushParams        = m_vdencItf->MHW_GETPAR_F(VD_PIPELINE_FLUSH)();
        vdPipeFlushParams              = {};
        vdPipeFlushParams.flushHEVC    = true;
        vdPipeFlushParams.waitDoneHEVC = true;
        ENCODE_CHK_STATUS_RETURN(m_vdencItf->MHW_ADDCMD_F(VD_PIPELINE_FLUSH)(&cmdBuffer));

        // Flush the engine to ensure memory written out
        auto &flushDwParams                         = m_miItf->MHW_GETPAR_F(MI_FLUSH_DW)();
        flushDwParams                               = {};
        flushDwParams.bVideoPipelineCacheInvalidate = true;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_FLUSH_DW)(&cmdBuffer));

        return eStatus;
    }

    MOS_STATUS HevcEncodeDss::GetDssBuffer(PMOS_RESOURCE &resSliceCountBuffer, PMOS_RESOURCE &resVDEncModeTimerBuffer)
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        ENCODE_FUNC_CALL();

        ENCODE_CHK_NULL_RETURN(m_resSliceCountBuffer);
        ENCODE_CHK_NULL_RETURN(m_resVDEncModeTimerBuffer);

        resSliceCountBuffer = m_resSliceCountBuffer;
        resVDEncModeTimerBuffer = m_resVDEncModeTimerBuffer;

        return eStatus;
    }
    }