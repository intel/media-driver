/*
* Copyright (c) 2020-2023, Intel Corporation
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
//! \file     encode_vp9_pak.cpp
//! \brief    Defines the common interface for vp9 vdenc\pak features
//!

#include "encode_vp9_pak.h"
#include "codec_def_common.h"
#include "encode_vp9_vdenc_feature_manager.h"
#include "encode_vp9_vdenc_const_settings.h"
#include "encode_vp9_brc.h"
#include "encode_vp9_hpu.h"
#include "encode_vp9_segmentation.h"
#include "media_cmd_packet.h"

namespace encode
{
Vp9EncodePak::Vp9EncodePak(
    MediaFeatureManager *featureManager,
    EncodeAllocator *    allocator,
    CodechalHwInterfaceNext *hwInterface,
    void *               constSettings)
    : MediaFeature(constSettings),
      m_hwInterface(hwInterface),
      m_allocator(allocator)
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_NULL_NO_STATUS_RETURN(featureManager);

    m_featureManager = featureManager;
    auto encFeatureManager = dynamic_cast<EncodeVp9VdencFeatureManager *>(featureManager);
    ENCODE_CHK_NULL_NO_STATUS_RETURN(encFeatureManager);

    m_basicFeature = dynamic_cast<Vp9BasicFeature *>(encFeatureManager->GetFeature(FeatureIDs::basicFeature));
    ENCODE_CHK_NULL_NO_STATUS_RETURN(m_basicFeature);

    ENCODE_CHK_NULL_NO_STATUS_RETURN(m_hwInterface);

    m_hcpInterfaceNew = std::static_pointer_cast<mhw::vdbox::hcp::Itf>(m_hwInterface->GetHcpInterfaceNext());
    ENCODE_CHK_NULL_NO_STATUS_RETURN(m_hcpInterfaceNew);

    m_vdencInterfaceNew = std::static_pointer_cast<mhw::vdbox::vdenc::Itf>(m_hwInterface->GetVdencInterfaceNext());
    ENCODE_CHK_NULL_NO_STATUS_RETURN(m_vdencInterfaceNew);

    m_miItf = std::static_pointer_cast<mhw::mi::Itf>(m_hwInterface->GetMiInterfaceNext());
    ENCODE_CHK_NULL_NO_STATUS_RETURN(m_miItf);
}

MOS_STATUS Vp9EncodePak::Init(void *settings)
{
    ENCODE_FUNC_CALL();

    m_enabled = true;

    ENCODE_CHK_STATUS_RETURN(m_hwInterface->GetVdencPictureSecondLevelCommandsSize(
        CODECHAL_ENCODE_MODE_VP9, &m_vdencPicStateSecondLevelBatchBufferSize));

    ENCODE_CHK_STATUS_RETURN(AllocateResources());

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9EncodePak::Update(void *params)
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_NULL_RETURN(params);

    EncoderParams *encodeParams = (EncoderParams *)params;

    auto vp9SeqParams = static_cast<PCODEC_VP9_ENCODE_SEQUENCE_PARAMS>(encodeParams->pSeqParams);
    ENCODE_CHK_NULL_RETURN(vp9SeqParams);
    auto vp9PicParams = static_cast<PCODEC_VP9_ENCODE_PIC_PARAMS>(encodeParams->pPicParams);
    ENCODE_CHK_NULL_RETURN(vp9PicParams);

    if (m_basicFeature->m_newSeq)
    {
        ENCODE_CHK_STATUS_RETURN(SetConstSettings());
    }

    // Reset picture state 2nd level batch buffer stage
    for (auto i = 0; i < 3; ++i)
    {
        m_picState2ndLevelBBConstructed[i] = false;
    }

    if (m_basicFeature->m_ref.DysRefFrameFlags() != DYS_REF_NONE)
    {
        ENCODE_CHK_STATUS_RETURN(PreparePakObjAndCuRecord());
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9EncodePak::SetRegionsForBrcUpdate(mhw::vdbox::huc::HUC_VIRTUAL_ADDR_STATE_PAR &params, uint32_t currPass) const
{
    ENCODE_FUNC_CALL();

    // Input SLBB (second level batch buffer) - IN
    params.regionParams[3].presRegion = const_cast<PMOS_RESOURCE>(&m_resVdencPictureState2ndLevelBatchBufferRead[currPass][m_vdencPictureState2ndLevelBBIndex]);

    // Output SLBB - OUT
    params.regionParams[6].presRegion = const_cast<PMOS_RESOURCE>(&m_resVdencPictureState2ndLevelBatchBufferWrite[0]);
    params.regionParams[6].isWritable = true;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9EncodePak::SetRegionsForHucProb(mhw::vdbox::huc::HUC_VIRTUAL_ADDR_STATE_PAR &params, uint32_t currPass) const
{
    ENCODE_FUNC_CALL();

    auto brcFeature = dynamic_cast<Vp9EncodeBrc *>(m_featureManager->GetFeature(Vp9FeatureIDs::vp9BrcFeature));
    ENCODE_CHK_NULL_RETURN(brcFeature);

    // Input regions: 7, 8

    // If BRC enabled, BRC Pass 2 output SLBB -> input SLBB for HPU on pass 2 (HPU pass 1 and 3. BRC Update pass 1 and 2)
    //                 BRC Pass 1 output SLBB -> input SLBB for HPU on pass 1
    // If BRC not on, Driver prepared SLBB    -> input to HPU on both passes
    if (brcFeature->IsVdencBrcEnabled())
    {
        params.regionParams[7].presRegion = const_cast<MOS_RESOURCE *>(&m_resVdencPictureState2ndLevelBatchBufferWrite[0]);
    }
    else
    {
        params.regionParams[7].presRegion = const_cast<MOS_RESOURCE *>(&m_resVdencPictureState2ndLevelBatchBufferRead[currPass][m_vdencPictureState2ndLevelBBIndex]);
    }
    params.regionParams[8].presRegion = const_cast<MOS_RESOURCE *>(&m_resHucPakInsertUncompressedHeaderReadBuffer[m_basicFeature->m_currRecycledBufIdx]);

    // Output regions: 4, 5, 6, 10, 11

    params.regionParams[4].presRegion  = const_cast<MOS_RESOURCE *>(&m_resHucPakInsertUncompressedHeaderWriteBuffer);
    params.regionParams[4].isWritable  = true;
    params.regionParams[5].presRegion  = const_cast<MOS_RESOURCE *>(&m_resCompressedHeaderBuffer);
    params.regionParams[5].isWritable  = true;
    params.regionParams[6].presRegion  = const_cast<MOS_RESOURCE *>(&m_resVdencPictureState2ndLevelBatchBufferWrite[0]);
    params.regionParams[6].isWritable  = true;
    params.regionParams[10].presRegion = const_cast<MOS_RESOURCE *>(&m_basicFeature->m_resBitstreamBuffer);
    params.regionParams[10].isWritable = true;
    params.regionParams[11].presRegion = const_cast<MOS_RESOURCE *>(&m_resVdencDataExtensionBuffer);
    params.regionParams[11].isWritable = true;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9EncodePak::ConstructPicStateBatchBuffer(EncodePipeline* pipeline)
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_NULL_RETURN(pipeline);
    ENCODE_CHK_NULL_RETURN(m_basicFeature);

    auto currPass    = pipeline->GetCurrentPass();
    auto passNum     = pipeline->GetPassNum();
    auto isFirstPipe = pipeline->IsFirstPipe();
    auto isLastPass  = pipeline->IsLastPass();

    auto dysRefFrameFlags = m_basicFeature->m_ref.DysRefFrameFlags();
    if ((dysRefFrameFlags == DYS_REF_NONE) && m_basicFeature->m_pakOnlyModeEnabledForLastPass)
    {
        // This flag sets PAK-only mode in SLBB for rePAK pass. In single-pass mode, this flag should be disabled.
        m_basicFeature->m_vdencPakonlyMultipassEnabled = ((passNum > 1) && isLastPass) ? true : false;
    }

    // This function will call by Vp9HucProbPkt or Vp9HucBrcUpdatePkt.
    // Only construct once.
    // Check if picture state 2nd level batch buffer already constrcuted, than skip.
    if (currPass < 3 && m_picState2ndLevelBBConstructed[currPass])
    {
        return MOS_STATUS_SUCCESS;
    }

    PMOS_RESOURCE picStateBuffer = nullptr;
    if (dysRefFrameFlags != DYS_REF_NONE && m_basicFeature->m_dysVdencMultiPassEnabled)
    {
        picStateBuffer = &m_resVdencDysPictureState2ndLevelBatchBuffer;
    }
    else
    {
        if (!isFirstPipe)
        {
            return MOS_STATUS_SUCCESS;
        }

        if (currPass >= 3)
        {
            ENCODE_ASSERTMESSAGE("Index exceeds the max number, when try to get m_resVdencPictureState2ndLevelBatchBufferRead;");
            return MOS_STATUS_INVALID_PARAMETER;
        }

        picStateBuffer = &m_resVdencPictureState2ndLevelBatchBufferRead[currPass][m_vdencPictureState2ndLevelBBIndex];
    }
    ENCODE_CHK_NULL_RETURN(picStateBuffer);

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    auto vp9SeqParams = static_cast<PCODEC_VP9_ENCODE_SEQUENCE_PARAMS>(m_basicFeature->m_vp9SeqParams);
    ENCODE_CHK_NULL_RETURN(vp9SeqParams);
    auto vp9PicParams = static_cast<PCODEC_VP9_ENCODE_PIC_PARAMS>(m_basicFeature->m_vp9PicParams);
    ENCODE_CHK_NULL_RETURN(vp9PicParams);
    auto vp9SegmentParams = static_cast<PCODEC_VP9_ENCODE_SEGMENT_PARAMS>(m_basicFeature->m_vp9SegmentParams);
    ENCODE_CHK_NULL_RETURN(vp9SegmentParams);
    auto osInterface = m_hwInterface->GetOsInterface();
    ENCODE_CHK_NULL_RETURN(osInterface);

    uint8_t *data = (uint8_t *)m_allocator->LockResourceForWrite(picStateBuffer);
    ENCODE_CHK_NULL_RETURN(data);

    MOS_COMMAND_BUFFER constructedCmdBuf;
    MOS_ZeroMemory(&constructedCmdBuf, sizeof(constructedCmdBuf));
    constructedCmdBuf.pCmdBase   = (uint32_t *)data;
    constructedCmdBuf.pCmdPtr    = (uint32_t *)data;
    constructedCmdBuf.iOffset    = 0;
    constructedCmdBuf.iRemaining = m_vdencPicStateSecondLevelBatchBufferSize;

    eStatus = AddVdencCmd1Command(constructedCmdBuf);
    if (eStatus != MOS_STATUS_SUCCESS)
    {
        ENCODE_CHK_STATUS_RETURN(m_allocator->UnLock(picStateBuffer));
        ENCODE_ASSERTMESSAGE("Failed to add CODECHAL_CMD1 command.");
        return eStatus;
    }

    m_basicFeature->m_hucPicStateOffset       = (uint16_t)constructedCmdBuf.iOffset;

    SETPAR_AND_ADDCMD(HCP_VP9_PIC_STATE, m_hcpInterfaceNew, &constructedCmdBuf);

    // HCP_VP9_SEGMENT_STATE
    uint8_t segmentCount = (vp9PicParams->PicFlags.fields.segmentation_enabled) ? CODEC_VP9_MAX_SEGMENTS : 1;

    for (uint8_t i = 0; i < segmentCount; i++)
    {
        RUN_FEATURE_INTERFACE_RETURN(Vp9Segmentation, Vp9FeatureIDs::vp9Segmentation, SetSegmentId, i);
        SETPAR_AND_ADDCMD(HCP_VP9_SEGMENT_STATE, m_hcpInterfaceNew, &constructedCmdBuf);
    }

    // Adjust cmd buffer offset to have 8 segment state blocks
    if (segmentCount < CODEC_VP9_MAX_SEGMENTS)
    {
        // Max 7 segments, 32 bytes each
        uint8_t zeroBlock[Vp9Segmentation::m_segmentStateBlockSize * (CODEC_VP9_MAX_SEGMENTS - 1)];
        MOS_ZeroMemory(zeroBlock, sizeof(zeroBlock));
        Mhw_AddCommandCmdOrBB(osInterface, &constructedCmdBuf, nullptr, zeroBlock, (CODEC_VP9_MAX_SEGMENTS - segmentCount) * Vp9Segmentation::m_segmentStateBlockSize);
    }

    m_basicFeature->m_slbbImgStateOffset = (uint16_t)constructedCmdBuf.iOffset;

    eStatus = AddVdencCmd2Command(constructedCmdBuf);
    if (eStatus != MOS_STATUS_SUCCESS)
    {
        ENCODE_CHK_STATUS_RETURN(m_allocator->UnLock(picStateBuffer));
        ENCODE_ASSERTMESSAGE("Failed to add CODECHAL_CMD2 command.");
        return eStatus;
    }

    // BB_END
    eStatus = m_miItf->AddMiBatchBufferEnd(&constructedCmdBuf, nullptr);
    if (eStatus != MOS_STATUS_SUCCESS)
    {
        ENCODE_CHK_STATUS_RETURN(m_allocator->UnLock(picStateBuffer));
        ENCODE_ASSERTMESSAGE("Failed to add MI Batch Buffer End command.");
        return eStatus;
    }

    constructedCmdBuf.iOffset += 24; // padding for alignment on 64
    constructedCmdBuf.iRemaining -= 24;
    if ((constructedCmdBuf.iOffset != m_vdencPicStateSecondLevelBatchBufferSize) &&
        (constructedCmdBuf.iRemaining < 0))
    {
        ENCODE_ASSERTMESSAGE("Failed to constructed Second Level Batch Buffer: No space for padding bytes.");
        return MOS_STATUS_NO_SPACE;
    }

    m_basicFeature->m_hucSlbbSize = (uint16_t)constructedCmdBuf.iOffset;

    ENCODE_CHK_STATUS_RETURN(m_allocator->UnLock(picStateBuffer));

    // Indicated picture state 2nd level batch buffer for this pass already constructed
    m_picState2ndLevelBBConstructed[currPass] = true;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9EncodePak::SetHucPakInsertObjBatchBuffer(MHW_BATCH_BUFFER &secondLevelBatchBuffer)
{
    ENCODE_FUNC_CALL();

    secondLevelBatchBuffer.dwOffset     = 0;
    secondLevelBatchBuffer.bSecondLevel = true;

    if (!m_basicFeature->m_hucEnabled)
    {
        secondLevelBatchBuffer.OsResource = m_resHucPakInsertUncompressedHeaderReadBuffer[m_basicFeature->m_currRecycledBufIdx];
    }
    else
    {
        secondLevelBatchBuffer.OsResource = m_resHucPakInsertUncompressedHeaderWriteBuffer;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9EncodePak::SetVdencSecondLevelBatchBuffer(uint32_t currPass, MHW_BATCH_BUFFER &secondLevelBatchBuffer)
{
    ENCODE_FUNC_CALL();

    secondLevelBatchBuffer.dwOffset     = 0;
    secondLevelBatchBuffer.bSecondLevel = true;

    if (m_basicFeature->m_hucEnabled)
    {
        secondLevelBatchBuffer.OsResource = m_resVdencPictureState2ndLevelBatchBufferWrite[0];
    }
    else
    {
        auto dysRefFrameFlags         = m_basicFeature->m_ref.DysRefFrameFlags();
        auto dysVdencMultiPassEnabled = m_basicFeature->m_dysVdencMultiPassEnabled;
        if ((dysRefFrameFlags != DYS_REF_NONE) && dysVdencMultiPassEnabled)
        {
            secondLevelBatchBuffer.OsResource = m_resVdencDysPictureState2ndLevelBatchBuffer;
        }
        else
        {
            if (currPass >= 3)
            {
                ENCODE_ASSERTMESSAGE("Index exceeds the max number, when try to set vdenc second level batch buffer;");
                return MOS_STATUS_INVALID_PARAMETER;
            }

            secondLevelBatchBuffer.OsResource = m_resVdencPictureState2ndLevelBatchBufferRead[currPass][m_vdencPictureState2ndLevelBBIndex];
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9EncodePak::GetVdencPictureState2ndLevelBatchBufferSize(uint32_t &size)
{
    ENCODE_FUNC_CALL();

    size = m_vdencPicStateSecondLevelBatchBufferSize;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9EncodePak::UpdateParameters()
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_NULL_RETURN(m_basicFeature);

    if (m_basicFeature->m_hucEnabled)
    {
        // We save the index of the 2nd level batch buffer in case is a pass that needs
        m_lastVdencPictureState2ndLevelBBIndex = m_vdencPictureState2ndLevelBBIndex;
    }
    m_vdencPictureState2ndLevelBBIndex = (m_vdencPictureState2ndLevelBBIndex + 1) % CODECHAL_VP9_ENCODE_RECYCLED_BUFFER_NUM;

    return MOS_STATUS();
}

MOS_STATUS Vp9EncodePak::PakConstructPicStateBatchBuffer(PMOS_RESOURCE picStateBuffer)
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_NULL_RETURN(picStateBuffer);

    uint8_t *data = (uint8_t *)m_allocator->LockResourceForWrite(picStateBuffer);
    ENCODE_CHK_NULL_RETURN(data);

    for (int i = 0; i < CODECHAL_ENCODE_VP9_BRC_MAX_NUM_OF_PASSES; ++i)
    {
        MOS_COMMAND_BUFFER constructedCmdBuf;

        constructedCmdBuf.pCmdBase   = (uint32_t *)data;
        constructedCmdBuf.pCmdPtr    = (uint32_t *)(data + i * CODECHAL_ENCODE_VP9_PIC_STATE_BUFFER_SIZE_PER_PASS);
        constructedCmdBuf.iOffset    = 0;
        constructedCmdBuf.iRemaining = CODECHAL_ENCODE_VP9_PIC_STATE_BUFFER_SIZE_PER_PASS;

        SETPAR_AND_ADDCMD(HCP_VP9_PIC_STATE, m_hcpInterfaceNew, &constructedCmdBuf);

        // After adding pic state cmds in above function, pCmdPtr is not at the end of the picState buffer, so adjust it.
        // -1 to go back one uint32_t where BB end will be added
        constructedCmdBuf.pCmdPtr = (uint32_t *)(data + (i + 1) * CODECHAL_ENCODE_VP9_PIC_STATE_BUFFER_SIZE_PER_PASS) - 1;

        ENCODE_CHK_STATUS_RETURN(m_miItf->AddMiBatchBufferEnd(&constructedCmdBuf, nullptr));
    }

    m_allocator->UnLock(picStateBuffer);

    return MOS_STATUS_SUCCESS;
}

PMOS_RESOURCE Vp9EncodePak::GetCompressedHeaderBuffer()
{
    ENCODE_FUNC_CALL();

    return &m_resCompressedHeaderBuffer;
}

MOS_STATUS Vp9EncodePak::AllocateResources()
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_NULL_RETURN(m_basicFeature);

    MOS_RESOURCE *allocatedBuffer = nullptr;
    // Initiate allocation parameters and lock flags
    MOS_ALLOC_GFXRES_PARAMS allocParamsForBufferLinear;
    MOS_ZeroMemory(&allocParamsForBufferLinear, sizeof(MOS_ALLOC_GFXRES_PARAMS));
    allocParamsForBufferLinear.Type     = MOS_GFXRES_BUFFER;
    allocParamsForBufferLinear.TileType = MOS_TILE_LINEAR;
    allocParamsForBufferLinear.Format   = Format_Buffer;

    // VDENC picture second level batch buffer (Read/Write)
    allocParamsForBufferLinear.dwBytes  = MOS_ALIGN_CEIL(m_vdencPicStateSecondLevelBatchBufferSize, CODECHAL_PAGE_SIZE);
    allocParamsForBufferLinear.pBufName = "VDENC Picture Second Level Batch Buffer Read";
    allocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_CACHE;
    for (auto i = 0; i < CODECHAL_VP9_ENCODE_RECYCLED_BUFFER_NUM; ++i)
    {
        for (auto j = 0; j < 3; ++j)
        {
            allocatedBuffer = m_allocator->AllocateResource(allocParamsForBufferLinear, true);
            ENCODE_CHK_NULL_RETURN(allocatedBuffer);
            m_resVdencPictureState2ndLevelBatchBufferRead[j][i] = *allocatedBuffer;
        }
    }

    allocParamsForBufferLinear.pBufName = "VDENC Picture Second Level Batch Buffer Write";
    allocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_CACHE;
    for (auto i = 0; i < CODECHAL_VP9_ENCODE_RECYCLED_BUFFER_NUM; ++i)
    {
        allocatedBuffer = m_allocator->AllocateResource(allocParamsForBufferLinear, true);
        ENCODE_CHK_NULL_RETURN(allocatedBuffer);
        m_resVdencPictureState2ndLevelBatchBufferWrite[i] = *allocatedBuffer;
    }

    // Huc VP9 pak insert uncompressed header
    allocParamsForBufferLinear.dwBytes  = CODECHAL_ENCODE_VP9_PAK_INSERT_UNCOMPRESSED_HEADER;
    allocParamsForBufferLinear.pBufName = "HucPakInsertUncompressedHeaderReadBuffer";
    allocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_CACHE;
    for (auto i = 0; i < CODECHAL_ENCODE_RECYCLED_BUFFER_NUM; ++i)
    {
        allocatedBuffer = m_allocator->AllocateResource(allocParamsForBufferLinear, true);
        ENCODE_CHK_NULL_RETURN(allocatedBuffer);
        m_resHucPakInsertUncompressedHeaderReadBuffer[i] = *allocatedBuffer;
    }

    allocParamsForBufferLinear.dwBytes  = CODECHAL_ENCODE_VP9_PAK_INSERT_UNCOMPRESSED_HEADER;
    allocParamsForBufferLinear.pBufName = "HucPakInsertUncompressedHeaderWriteBuffer";
    allocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_CACHE;
    allocatedBuffer                     = m_allocator->AllocateResource(allocParamsForBufferLinear, true);
    ENCODE_CHK_NULL_RETURN(allocatedBuffer);
    m_resHucPakInsertUncompressedHeaderWriteBuffer = *allocatedBuffer;

    // Compressed header buffer
    allocParamsForBufferLinear.dwBytes  = 32 * CODECHAL_CACHELINE_SIZE;
    allocParamsForBufferLinear.pBufName = "CompressedHeaderBuffer";
    allocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ;
    allocatedBuffer                     = m_allocator->AllocateResource(allocParamsForBufferLinear, true);
    ENCODE_CHK_NULL_RETURN(allocatedBuffer);
    m_resCompressedHeaderBuffer = *allocatedBuffer;

    // Allocate data extension buffer
    allocParamsForBufferLinear.dwBytes  = CODECHAL_ENCODE_VP9_VDENC_DATA_EXTENSION_SIZE;
    allocParamsForBufferLinear.pBufName = "DataExtensionBuffer";
    allocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ;
    allocatedBuffer                     = m_allocator->AllocateResource(allocParamsForBufferLinear, true);
    ENCODE_CHK_NULL_RETURN(allocatedBuffer);
    m_resVdencDataExtensionBuffer = *allocatedBuffer;

    // Metadata line buffer
    uint32_t size                       = m_basicFeature->m_maxPicWidthInSb * 5 * CODECHAL_CACHELINE_SIZE;
    allocParamsForBufferLinear.dwBytes  = size;
    allocParamsForBufferLinear.pBufName = "MetadataLineBuffer";
    allocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ;
    allocatedBuffer                     = m_allocator->AllocateResource(allocParamsForBufferLinear, true);
    ENCODE_CHK_NULL_RETURN(allocatedBuffer);
    m_resMetadataLineBuffer = allocatedBuffer;

    // Metadata tile line buffer
    size                                = m_basicFeature->m_maxPicWidthInSb * 5 * CODECHAL_CACHELINE_SIZE;
    allocParamsForBufferLinear.dwBytes  = size;
    allocParamsForBufferLinear.pBufName = "MetadataTileLineBuffer";
    allocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ;
    allocatedBuffer                     = m_allocator->AllocateResource(allocParamsForBufferLinear, true);
    ENCODE_CHK_NULL_RETURN(allocatedBuffer);
    m_resMetadataTileLineBuffer = allocatedBuffer;

    // Metadata tile column buffer
    size                                = m_basicFeature->m_maxPicHeightInSb * 5 * CODECHAL_CACHELINE_SIZE;
    allocParamsForBufferLinear.dwBytes  = size;
    allocParamsForBufferLinear.pBufName = "MetadataTileColumnBuffer";
    allocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ;
    allocatedBuffer                     = m_allocator->AllocateResource(allocParamsForBufferLinear, true);
    ENCODE_CHK_NULL_RETURN(allocatedBuffer);
    m_resMetadataTileColumnBuffer = allocatedBuffer;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9EncodePak::ConstructPakInsertObjBatchBuffer()
{
    ENCODE_FUNC_CALL();

    ENCODE_CHK_NULL_RETURN(m_hwInterface);
    uint32_t nalUnitSize   = m_basicFeature->m_nalUnitParams[0]->uiSize;
    uint32_t nalUnitOffset = m_basicFeature->m_nalUnitParams[0]->uiOffset;
    ENCODE_ASSERT(nalUnitSize > 0 && nalUnitSize < CODECHAL_ENCODE_VP9_PAK_INSERT_UNCOMPRESSED_HEADER);

    uint8_t *data = (uint8_t *)m_allocator->LockResourceForWrite(&m_resHucPakInsertUncompressedHeaderReadBuffer[m_basicFeature->m_currRecycledBufIdx]);
    ENCODE_CHK_NULL_RETURN(data);

    MOS_COMMAND_BUFFER constructedCmdBuf;
    MOS_ZeroMemory(&constructedCmdBuf, sizeof(constructedCmdBuf));
    constructedCmdBuf.pCmdBase   = (uint32_t *)data;
    constructedCmdBuf.pCmdPtr    = (uint32_t *)data;
    constructedCmdBuf.iOffset    = 0;
    constructedCmdBuf.iRemaining = CODECHAL_ENCODE_VP9_PAK_INSERT_UNCOMPRESSED_HEADER;

    SETPAR_AND_ADDCMD(HCP_PAK_INSERT_OBJECT, m_hcpInterfaceNew, &constructedCmdBuf);

    uint32_t byteSize = (nalUnitSize * 8 + 7) >> 3;

    if (byteSize)
    {
        MHW_MI_CHK_NULL(&m_basicFeature->m_bsBuffer);
        MHW_MI_CHK_NULL(m_basicFeature->m_bsBuffer.pBase);
        uint8_t *data_bb = (uint8_t *)(m_basicFeature->m_bsBuffer.pBase + nalUnitOffset);
        MHW_MI_CHK_STATUS(Mhw_AddCommandCmdOrBB(m_hwInterface->GetOsInterface(), &constructedCmdBuf, nullptr, data_bb, byteSize));
    }

    ENCODE_CHK_STATUS_RETURN(m_miItf->AddMiBatchBufferEnd(&constructedCmdBuf, nullptr));

    ENCODE_CHK_STATUS_RETURN(m_allocator->UnLock(&m_resHucPakInsertUncompressedHeaderReadBuffer[m_basicFeature->m_currRecycledBufIdx]));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9EncodePak::PreparePakObjAndCuRecord()
{
    ENCODE_FUNC_CALL();

    auto picWidthInSb  = m_basicFeature->m_picWidthInSb;
    auto picHeightInSb = m_basicFeature->m_picHeightInSb;

    // We use PAK to perform dynamic scaling for reference frame, basically if every CU is inter and skipped, the reconstructed picture will be
    // the down scaled copy of reference frame.
    // Here driver needs to prepare pak obj and cu record, since PAK has a limitation that input picture needs to be CU boundary aligned,
    // and to simplify handling the boundary condition, we set each CU with size 8x8, inter and zero MV.
    // Segment skip needs to be turned on also.

    auto oriFrameWidth  = MOS_ALIGN_CEIL(m_basicFeature->m_oriFrameWidth, CODEC_VP9_MIN_BLOCK_WIDTH);
    auto oriFrameHeight = MOS_ALIGN_CEIL(m_basicFeature->m_oriFrameHeight, CODEC_VP9_MIN_BLOCK_HEIGHT);

    uint32_t numCuLastSbCol = (oriFrameWidth / CODEC_VP9_MIN_BLOCK_WIDTH) -
                              (picWidthInSb - 1) * (CODEC_VP9_SUPER_BLOCK_WIDTH / CODEC_VP9_MIN_BLOCK_WIDTH);
    uint32_t numCuLastSbRow = (oriFrameHeight / CODEC_VP9_MIN_BLOCK_HEIGHT) -
                              (picHeightInSb - 1) * (CODEC_VP9_SUPER_BLOCK_HEIGHT / CODEC_VP9_MIN_BLOCK_HEIGHT);

    auto data = (uint8_t *)m_allocator->LockResourceForWrite(m_basicFeature->m_resMbCodeBuffer);
    ENCODE_CHK_NULL_RETURN(data);

    HcpPakObject *pakObjData = (HcpPakObject *)data;
    CU_DATA *     cuDataPtr  = (CU_DATA *)(data + m_basicFeature->m_mvOffset);

    // Fill PAK object
    HcpPakObject pakObj;
    MOS_ZeroMemory(&pakObj, sizeof(pakObj));
    pakObj.DW0.Type        = 0x03;
    pakObj.DW0.Opcode      = 0x27;
    pakObj.DW0.SubOp       = 0x35;
    pakObj.DW0.DwordLength = 1;  // Total 4 DW. But only 3 DW are actual fields
                                 // DW0/DW1/DW2 excluded from this field as per spec definition
    pakObj.DW1.Split_flag_level0             = 1;
    pakObj.DW1.Split_flag_level1             = 0xF;
    pakObj.DW1.Split_flag_level2_level1part0 = 0xF;
    pakObj.DW1.Split_flag_level2_level1part1 = 0xF;
    pakObj.DW1.Split_flag_level2_level1part2 = 0xF;
    pakObj.DW1.Split_flag_level2_level1part3 = 0xF;

    // Fill CU data
    CU_DATA cuData;
    MOS_ZeroMemory(&cuData, sizeof(cuData));
    cuData.cu_size           = 0;                         // 8x8
    cuData.cu_pred_mode0     = cuData.cu_pred_mode1 = 1;  // Inter
    cuData.refframe_part0_l0 = cuData.refframe_part1_l0 = m_basicFeature->m_ref.GetDysRefIndex();

    for (uint32_t j = 0; j < picHeightInSb; ++j)
    {
        for (uint32_t i = 0; i < picWidthInSb; ++i)
        {
            if ((j == picHeightInSb - 1) && (i == picWidthInSb - 1))
            {
                pakObj.DW1.CU_count_minus1   = numCuLastSbCol * numCuLastSbRow - 1;
                pakObj.DW1.IsLastSBFrameflag = 1;
                pakObj.DW1.IsLastSBTileflag  = 1;
                pakObj.Reserved_DW03         = 0x05000000;  // add batch buffer end flag
            }
            else if (i == picWidthInSb - 1)
            {
                pakObj.DW1.CU_count_minus1 = numCuLastSbCol * 8 - 1;
            }
            else if (j == picHeightInSb - 1)
            {
                pakObj.DW1.CU_count_minus1 = numCuLastSbRow * 8 - 1;
            }
            else
            {
                pakObj.DW1.CU_count_minus1 = 63;
            }

            pakObj.DW2.Current_SB_X_Addr = i;
            pakObj.DW2.Current_SB_Y_Addr = j;

            *pakObjData++ = pakObj;

            for (unsigned int cuIdx = 0; cuIdx < 64; cuIdx++)
            {
                *cuDataPtr++ = cuData;
            }
        }
    }
    ENCODE_CHK_STATUS_RETURN(m_allocator->UnLock(m_basicFeature->m_resMbCodeBuffer));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9EncodePak::AddVdencCmd1Command(MOS_COMMAND_BUFFER &cmdBuffer)
{
    ENCODE_FUNC_CALL();

    ENCODE_CHK_NULL_RETURN(m_vdencInterfaceNew);

    ENCODE_CHK_NULL_RETURN(m_basicFeature);
    ENCODE_CHK_NULL_RETURN(m_basicFeature->m_vp9PicParams);

    SETPAR_AND_ADDCMD(VDENC_CMD1, m_vdencInterfaceNew, &cmdBuffer);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9EncodePak::AddVdencCmd2Command(MOS_COMMAND_BUFFER &cmdBuffer)
{
    ENCODE_FUNC_CALL();

    ENCODE_CHK_NULL_RETURN(m_vdencInterfaceNew);

    SETPAR_AND_ADDCMD(VDENC_CMD2, m_vdencInterfaceNew, &cmdBuffer);

    return MOS_STATUS_SUCCESS;
}

void Vp9EncodePak::CalculateQpLambdaValues(double &sadQpLambda, double &rdQpLambda)
{
    ENCODE_CHK_NULL_NO_STATUS_RETURN(m_basicFeature->m_vp9PicParams);

    auto   qp           = m_basicFeature->m_vp9PicParams->LumaACQIndex;
    auto   vp9FrameType = m_basicFeature->m_vp9PicParams->PicFlags.fields.frame_type;
    double QPScale      = (vp9FrameType == CODEC_VP9_KEY_FRAME) ? 0.31 : 0.33;
    double lambda       = QPScale * CODECHAL_VP9_QUANT_AC[qp] / 8;

    sadQpLambda = lambda * 4 + 0.5;
    rdQpLambda  = lambda * lambda * 4 + 0.5;
}

MHW_SETPAR_DECL_SRC(HCP_PAK_INSERT_OBJECT, Vp9EncodePak)
{
    ENCODE_FUNC_CALL();

    uint32_t nalUnitSize = m_basicFeature->m_nalUnitParams[0]->uiSize;
    ENCODE_ASSERT(nalUnitSize > 0 && nalUnitSize < CODECHAL_ENCODE_VP9_PAK_INSERT_UNCOMPRESSED_HEADER);

    uint32_t byteSize         = (nalUnitSize * 8 + 7) >> 3;
    uint32_t dataBitsInLastDw = nalUnitSize * 8 % 32;
    if (dataBitsInLastDw == 0)
    {
        dataBitsInLastDw = 32;
    }

    params.dwPadding = (MOS_ALIGN_CEIL(byteSize, sizeof(uint32_t))) / sizeof(uint32_t);

    params.bEmulationByteBitsInsert  = false;
    params.uiSkipEmulationCheckCount = m_basicFeature->m_nalUnitParams[0]->uiSkipEmulationCheckCount;
    params.dataBitsInLastDw          = dataBitsInLastDw;
    params.bEndOfSlice               = false;
    params.bLastHeader               = true;

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(HCP_IND_OBJ_BASE_ADDR_STATE, Vp9EncodePak)
{
    ENCODE_FUNC_CALL();

    params.presCompressedHeaderBuffer = const_cast<PMOS_RESOURCE>(&m_resCompressedHeaderBuffer);
    params.dwCompressedHeaderSize     = 32 * CODECHAL_CACHELINE_SIZE;

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(HCP_PIPE_BUF_ADDR_STATE, Vp9EncodePak)
{
    ENCODE_FUNC_CALL();

    params.presMetadataLineBuffer       = m_resMetadataLineBuffer;
    params.presMetadataTileLineBuffer   = m_resMetadataTileLineBuffer;
    params.presMetadataTileColumnBuffer = m_resMetadataTileColumnBuffer;

    return MOS_STATUS_SUCCESS;
}

}  // namespace encode
