/*
* Copyright (c) 2023-2025, Intel Corporation
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
//! \file     decode_vvc_pipeline_xe3p_lpm_base.cpp
//! \brief    Defines the interface for vvc decode pipeline for Xe3P_LPM_Base
//!
#include "decode_vvc_pipeline_xe3p_lpm_base.h"
#include "decode_vvc_packet.h"
#include "decode_vvc_s2l_packet_register_xe3p_lpm_base.h"
#include "decode_vvc_slice_packet_xe3p_lpm_base.h"

namespace decode
{

VvcPipelineXe3P_Lpm_Base::VvcPipelineXe3P_Lpm_Base(
    CodechalHwInterfaceNext *hwInterface,
    CodechalDebugInterface  *debugInterface)
    : VvcPipeline(hwInterface, debugInterface)
{
    m_hwInterface = hwInterface;
}

MOS_STATUS VvcPipelineXe3P_Lpm_Base::Init(void *settings)
{
    DECODE_FUNC_CALL();

    DECODE_CHK_NULL(settings);
    DECODE_CHK_STATUS(Initialize(settings));

    if (MEDIA_IS_SKU(m_skuTable, FtrWithSlimVdbox))
    {
        m_numVdbox = 1;
    }
    if (m_basicFeature->m_shortFormatInUse)
    {
        HucPacketCreator *hucPktCreator = dynamic_cast<HucPacketCreator *>(this);
        DECODE_CHK_NULL(hucPktCreator);
        m_vvcDecodeS2LPkt = hucPktCreator->CREATE_HUC_PACKET(VvcS2L, Xe3P_Lpm_Base, this, m_task, m_hwInterface);
        DECODE_CHK_NULL(m_vvcDecodeS2LPkt);
        DECODE_CHK_STATUS(RegisterPacket(DecodePacketId(this, vvcDecodeS2LPktId), m_vvcDecodeS2LPkt));
        DECODE_CHK_STATUS(m_vvcDecodeS2LPkt->Init());
    }
    m_vvcDecodePkt = MOS_New(VvcDecodePkt, this, m_task, m_hwInterface);
    DECODE_CHK_STATUS(RegisterPacket(DecodePacketId(this, vvcDecodePacketId), m_vvcDecodePkt));
    DECODE_CHK_STATUS(m_vvcDecodePkt->Init());
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VvcPipelineXe3P_Lpm_Base::AllocateResources(VvcBasicFeature &basicFeature)
{
    DECODE_FUNC_CALL();
    uint32_t vvcpSliceCommandsSize  = 0;
    uint32_t vvcpSlicePatchListSize = 0;
    uint32_t vvcpTileCommandsSize   = 0;
    uint32_t vvcpTilePatchListSize  = 0;

    DECODE_CHK_NULL(m_hwInterface);
    CodechalHwInterfaceNext *hwInterface = dynamic_cast<CodechalHwInterfaceNext *>(m_hwInterface);
    DECODE_CHK_NULL(hwInterface);
    vvcpSliceCommandsSize = m_vvcSlicePkt->GetSliceStatesSize();
    vvcpSlicePatchListSize = m_vvcSlicePkt->GetSlicePatchListSize();
    vvcpTileCommandsSize   = m_vvcSlicePkt->GetTileStatesSize();
    vvcpTilePatchListSize  = m_vvcSlicePkt->GetTilePatchListSize();
    uint32_t size     = MOS_ALIGN_CEIL(vvcpSliceCommandsSize, 64) * basicFeature.m_numSlices;
    m_sliceLvlBufSize = MOS_ALIGN_CEIL(vvcpSliceCommandsSize, 64);

    // In VVC short format decode, second level command buffer is programmed by Huc, so not need lock it.
    if (m_basicFeature->m_shortFormatInUse)
    {
        //Slice Level BB Array Allocation
        if (m_sliceLevelBBArray == nullptr)
        {
            m_sliceLevelBBArray = m_allocator->AllocateBatchBufferArray(
                size, 1, CODEC_VVC_BUFFER_ARRAY_SIZE, true, notLockableVideoMem);
            DECODE_CHK_NULL(m_sliceLevelBBArray);
            PMHW_BATCH_BUFFER &batchBuf = m_sliceLevelBBArray->Fetch();
            DECODE_CHK_NULL(batchBuf);
        }
        else
        {
            PMHW_BATCH_BUFFER &batchBuf = m_sliceLevelBBArray->Fetch();
            DECODE_CHK_NULL(batchBuf);
            DECODE_CHK_STATUS(m_allocator->Resize(
                batchBuf, size, 1, notLockableVideoMem));
        }

        //Tile Level BB Array Allocation
        uint32_t tileLvlCmdSize = 0;
        if (!m_basicFeature->m_vvcPicParams->m_ppsFlags.m_fields.m_ppsRectSliceFlag)  // Raster Scan Mode
        {
            tileLvlCmdSize = vvcpTileCommandsSize * vvcMaxTileRowNum;
        }
        else  //Rect Scan Mode
        {
            tileLvlCmdSize = (m_basicFeature->m_numSlices + m_basicFeature->m_tileCols * m_basicFeature->m_tileRows) * vvcpTileCommandsSize;
        }
        tileLvlCmdSize   = MOS_ALIGN_CEIL(tileLvlCmdSize, 64);
        m_tileLvlBufSize = tileLvlCmdSize;
        if (m_tileLevelBBArray == nullptr)
        {
            m_tileLevelBBArray          = m_allocator->AllocateBatchBufferArray(tileLvlCmdSize, 1, CODEC_VVC_BUFFER_ARRAY_SIZE, true, notLockableVideoMem);
            PMHW_BATCH_BUFFER &BatchBuf = m_tileLevelBBArray->Fetch();
            DECODE_CHK_NULL(BatchBuf);
        }
        else
        {
            PMHW_BATCH_BUFFER &batchBuf = m_tileLevelBBArray->Fetch();
            DECODE_CHK_NULL(batchBuf);
            DECODE_CHK_STATUS(m_allocator->Resize(
                batchBuf, tileLvlCmdSize, 1, notLockableVideoMem));
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VvcPipelineXe3P_Lpm_Base::CreateSubPackets(DecodeSubPacketManager &subPacketManager, CodechalSetting &codecSettings)
{
    DECODE_CHK_STATUS(DecodePipeline::CreateSubPackets(subPacketManager, codecSettings));

    VvcDecodePicPkt *pictureDecodePkt = MOS_New(VvcDecodePicPkt, this, m_hwInterface);
    DECODE_CHK_NULL(pictureDecodePkt);
    DECODE_CHK_STATUS(subPacketManager.Register(
        DecodePacketId(this, vvcPictureSubPacketId), *pictureDecodePkt));

    m_vvcSlicePkt = MOS_New(VvcDecodeSlcPktXe3P_Lpm_Base, this, m_hwInterface);
    DECODE_CHK_NULL(m_vvcSlicePkt);
    DECODE_CHK_STATUS(subPacketManager.Register(
        DecodePacketId(this, vvcSliceSubPacketId), *m_vvcSlicePkt));

    if (m_decodecp != nullptr)
    {
        auto feature = dynamic_cast<VvcBasicFeature *>(m_featureManager->GetFeature(FeatureIDs::basicFeature));
        DECODE_CHK_NULL(feature);
        DecodeSubPacket *cpSubPkt = (DecodeSubPacket *)m_decodecp->CreateDecodeCpIndSubPkt((DecodePipeline *)this, feature->m_mode, m_hwInterface);
        DECODE_CHK_NULL(cpSubPkt);
        DECODE_CHK_STATUS(subPacketManager.Register(
            DecodePacketId(this, vvcCpSubPacketId), *cpSubPkt));
    }
    return MOS_STATUS_SUCCESS;
}

}  // namespace decode