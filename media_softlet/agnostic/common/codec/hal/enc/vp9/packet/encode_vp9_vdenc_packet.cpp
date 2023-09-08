/*
* Copyright (c) 2020-2022, Intel Corporation
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
//! \file     encode_vp9_vdenc_packet.cpp
//! \brief    Defines the interface for vp9 encode vdenc packet
//!

#include "encode_vp9_vdenc_packet.h"
#include "encode_status_report_defs.h"
#include "encode_vp9_hpu.h"
#include "encode_vp9_brc.h"
#include "encode_vp9_cqp.h"
#include "encode_vp9_pak.h"
#include "encode_vp9_tile.h"
#include "codec_def_common.h"

using namespace mhw::vdbox;
namespace encode
{
Vp9VdencPkt::~Vp9VdencPkt()
{
    FreeResources();
}

MOS_STATUS Vp9VdencPkt::Init()
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_NULL_RETURN(m_statusReport);
    ENCODE_CHK_STATUS_RETURN(CmdPacket::Init());

    m_basicFeature = dynamic_cast<Vp9BasicFeature *>(m_featureManager->GetFeature(Vp9FeatureIDs::basicFeature));
    ENCODE_CHK_NULL_RETURN(m_basicFeature);

#ifdef _MMC_SUPPORTED
    m_mmcState = m_pipeline->GetMmcState();
    ENCODE_CHK_NULL_RETURN(m_mmcState);
    m_basicFeature->m_mmcState = m_mmcState;
#endif

    m_allocator = m_pipeline->GetEncodeAllocator();
    ENCODE_CHK_STATUS_RETURN(AllocateResources());

    ENCODE_CHK_STATUS_RETURN(m_statusReport->RegistObserver(this));

    CalculatePictureStateCommandSize();

    uint32_t vdencPictureStatesSize = 0, vdencPicturePatchListSize = 0;
    GetVdencStateCommandsDataSize(vdencPictureStatesSize, vdencPicturePatchListSize);
    m_defaultPictureStatesSize += vdencPictureStatesSize;
    m_defaultPicturePatchListSize += vdencPicturePatchListSize;

    GetHxxPrimitiveCommandSize();

    m_usePatchList = m_osInterface->bUsesPatchList;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9VdencPkt::Prepare()
{
    ENCODE_FUNC_CALL();

    m_pictureStatesSize    = m_defaultPictureStatesSize;
    m_picturePatchListSize = m_defaultPicturePatchListSize;
    m_sliceStatesSize      = m_defaultSliceStatesSize;
    m_slicePatchListSize   = m_defaultSlicePatchListSize;

    Vp9Pipeline *pipeline = dynamic_cast<Vp9Pipeline *>(m_pipeline);
    ENCODE_CHK_NULL_RETURN(pipeline);

    m_vp9SeqParams     = m_basicFeature->m_vp9SeqParams;
    m_vp9PicParams     = m_basicFeature->m_vp9PicParams;
    m_vp9SegmentParams = m_basicFeature->m_vp9SegmentParams;

    ENCODE_CHK_STATUS_RETURN(ValidateVdboxIdx(m_vdboxIndex));

    ENCODE_CHK_STATUS_RETURN(SetRowstoreCachingOffsets());

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9VdencPkt::Destroy()
{
    ENCODE_FUNC_CALL();

    m_statusReport->UnregistObserver(this);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9VdencPkt::Completed(void *mfxStatus, void *rcsStatus, void *statusReport)
{
    ENCODE_FUNC_CALL();

    ENCODE_CHK_NULL_RETURN(mfxStatus);
    ENCODE_CHK_NULL_RETURN(statusReport);
    ENCODE_CHK_NULL_RETURN(m_basicFeature);

    EncodeStatusMfx *       encodeStatusMfx  = (EncodeStatusMfx *)mfxStatus;
    EncodeStatusReportData *statusReportData = (EncodeStatusReportData *)statusReport;
    if (statusReportData->hwCtr)
    {
        m_encodecp->UpdateCpStatusReport(statusReport);
    }

    // Update tile status data here if need

    if (statusReportData->numberTilesInFrame > 1 && m_basicFeature->m_scalableMode)
    {
        // When Tile/Scalable mode feature enabled, Reset is not in vdenc packet
        return MOS_STATUS_SUCCESS;
    }

    CODECHAL_DEBUG_TOOL(
        ENCODE_CHK_STATUS_RETURN(DumpResources(encodeStatusMfx, statusReportData)););

    m_basicFeature->Reset((CODEC_REF_LIST *)statusReportData->currRefList);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9VdencPkt::CalculateCommandSize(uint32_t &commandBufferSize, uint32_t &requestedPatchListSize)
{
    ENCODE_FUNC_CALL();

    m_pictureStatesSize    = m_defaultPictureStatesSize;
    m_picturePatchListSize = m_defaultPicturePatchListSize;
    m_sliceStatesSize      = m_defaultSliceStatesSize;
    m_slicePatchListSize   = m_defaultSlicePatchListSize;

    commandBufferSize      = CalculateCommandBufferSize();
    requestedPatchListSize = CalculatePatchListSize();

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9VdencPkt::CalculatePictureStateCommandSize()
{
    ENCODE_FUNC_CALL();

    MHW_VDBOX_STATE_CMDSIZE_PARAMS stateCmdSizeParams;
    stateCmdSizeParams.bHucDummyStream = true;

    uint32_t hcpCommandsSize = 0;
    uint32_t hcpPatchListSize = 0;

    hcpCommandsSize = 
        m_vdencInterfaceNew->MHW_GETSIZE_F(VD_PIPELINE_FLUSH)() +
        m_miItf->MHW_GETSIZE_F(MI_FLUSH_DW)() +
        m_hcpInterfaceNew->MHW_GETSIZE_F(HCP_PIPE_MODE_SELECT)() +
        m_hcpInterfaceNew->MHW_GETSIZE_F(HCP_SURFACE_STATE)() * 4 +
        m_hcpInterfaceNew->MHW_GETSIZE_F(HCP_PIPE_BUF_ADDR_STATE)() +
        m_hcpInterfaceNew->MHW_GETSIZE_F(HCP_IND_OBJ_BASE_ADDR_STATE)() +
        m_hcpInterfaceNew->MHW_GETSIZE_F(HCP_VP9_SEGMENT_STATE)() * 8 +
        m_hcpInterfaceNew->MHW_GETSIZE_F(HCP_BSD_OBJECT)() +
        m_miItf->MHW_GETSIZE_F(MI_LOAD_REGISTER_REG)() * 8;

    hcpPatchListSize = 
        mhw::vdbox::hcp::Itf::VD_PIPELINE_FLUSH_CMD_NUMBER_OF_ADDRESSES +
        mhw::vdbox::hcp::Itf::MI_FLUSH_DW_CMD_NUMBER_OF_ADDRESSES +
        mhw::vdbox::hcp::Itf::HCP_PIPE_MODE_SELECT_CMD_NUMBER_OF_ADDRESSES +
        mhw::vdbox::hcp::Itf::HCP_SURFACE_STATE_CMD_NUMBER_OF_ADDRESSES * 4 +
        mhw::vdbox::hcp::Itf::HCP_PIPE_BUF_ADDR_STATE_CMD_NUMBER_OF_ADDRESSES +
        mhw::vdbox::hcp::Itf::HCP_IND_OBJ_BASE_ADDR_STATE_CMD_NUMBER_OF_ADDRESSES +
        mhw::vdbox::hcp::Itf::HCP_VP9_SEGMENT_STATE_CMD_NUMBER_OF_ADDRESSES * 8 +
        mhw::vdbox::hcp::Itf::HCP_BSD_OBJECT_CMD_NUMBER_OF_ADDRESSES;

    hcpCommandsSize +=
        m_hcpInterfaceNew->MHW_GETSIZE_F(HCP_VP9_PIC_STATE)() +
        m_miItf->MHW_GETSIZE_F(MI_FLUSH_DW)() * 2 +
        m_miItf->MHW_GETSIZE_F(MI_STORE_DATA_IMM)() * 4 +
        m_miItf->MHW_GETSIZE_F(MI_STORE_REGISTER_MEM)() * 11 +
        m_miItf->MHW_GETSIZE_F(MI_COPY_MEM_MEM)() * 4 +
        m_miItf->MHW_GETSIZE_F(MI_BATCH_BUFFER_START)() * 3 +
        m_miItf->MHW_GETSIZE_F(MI_STORE_DATA_IMM)() * 2 +  // Slice level commands
        m_miItf->MHW_GETSIZE_F(MI_LOAD_REGISTER_MEM)() * 2 +
        m_hcpInterfaceNew->MHW_GETSIZE_F(HCP_PAK_INSERT_OBJECT)() * 2 +
        m_hcpInterfaceNew->MHW_GETSIZE_F(HCP_TILE_CODING)() +
        m_miItf->MHW_GETSIZE_F(MI_BATCH_BUFFER_START)() +
        m_miItf->MHW_GETSIZE_F(MI_SEMAPHORE_WAIT)() +     // Use HW wait command for each VDBOX
        m_miItf->MHW_GETSIZE_F(MI_STORE_DATA_IMM)() * 3;  // One is for reset and another one for set per VDBOX, one for wait

    hcpCommandsSize += 3 * m_miItf->MHW_GETSIZE_F(VD_CONTROL_STATE)();  // VD_CONTROL_STATE Hcp init + flush + vdenc init

    hcpPatchListSize +=
        mhw::vdbox::hcp::Itf::HCP_VP9_PIC_STATE_CMD_NUMBER_OF_ADDRESSES +
        mhw::vdbox::hcp::Itf::MI_FLUSH_DW_CMD_NUMBER_OF_ADDRESSES * 2 +
        mhw::vdbox::hcp::Itf::MI_STORE_DATA_IMM_CMD_NUMBER_OF_ADDRESSES * 4 +
        mhw::vdbox::hcp::Itf::MI_STORE_REGISTER_MEM_CMD_NUMBER_OF_ADDRESSES * 11 +
        mhw::vdbox::hcp::Itf::MI_COPY_MEM_MEM_CMD_NUMBER_OF_ADDRESSES * 4 +
        mhw::vdbox::hcp::Itf::MI_BATCH_BUFFER_START_CMD_NUMBER_OF_ADDRESSES * 3 +
        mhw::vdbox::hcp::Itf::MI_STORE_DATA_IMM_CMD_NUMBER_OF_ADDRESSES * 2 +
        mhw::vdbox::hcp::Itf::HCP_PAK_INSERT_OBJECT_CMD_NUMBER_OF_ADDRESSES * 2 +
        mhw::vdbox::hcp::Itf::HCP_TILE_CODING_COMMAND_NUMBER_OF_ADDRESSES +
        mhw::vdbox::hcp::Itf::MI_BATCH_BUFFER_START_CMD_NUMBER_OF_ADDRESSES +
        mhw::vdbox::hcp::Itf::MI_STORE_DATA_IMM_CMD_NUMBER_OF_ADDRESSES * 2;

    uint32_t cpCmdsize = 0;
    uint32_t cpPatchListSize = 0;
    if(m_hwInterface->GetCpInterface())
    {
        m_hwInterface->GetCpInterface()->GetCpStateLevelCmdSize(cpCmdsize, cpPatchListSize);
    }

    uint32_t hucCommandsSize = 0;
    uint32_t hucPatchListSize = 0;
    ENCODE_CHK_STATUS_RETURN(m_hwInterface->GetHucStateCommandSize(
        CODECHAL_ENCODE_MODE_VP9, (uint32_t *)&hucCommandsSize, (uint32_t *)&hucPatchListSize, &stateCmdSizeParams));

    m_defaultPictureStatesSize = hcpCommandsSize + hucCommandsSize + cpCmdsize;
    m_defaultPicturePatchListSize = hcpPatchListSize + hucPatchListSize + cpPatchListSize;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9VdencPkt::GetVdencStateCommandsDataSize(uint32_t &vdencPictureStatesSize, uint32_t &vdencPicturePatchListSize)
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_NULL_RETURN(m_hwInterface);
    
    vdencPictureStatesSize =
        m_vdencInterfaceNew->MHW_GETSIZE_F(VDENC_PIPE_MODE_SELECT)() +
        m_vdencInterfaceNew->MHW_GETSIZE_F(VDENC_SRC_SURFACE_STATE)() +
        m_vdencInterfaceNew->MHW_GETSIZE_F(VDENC_REF_SURFACE_STATE)() +
        m_vdencInterfaceNew->MHW_GETSIZE_F(VDENC_DS_REF_SURFACE_STATE)() +
        m_vdencInterfaceNew->MHW_GETSIZE_F(VDENC_PIPE_BUF_ADDR_STATE)() +
        m_vdencInterfaceNew->MHW_GETSIZE_F(VDENC_WALKER_STATE)() +
        m_vdencInterfaceNew->MHW_GETSIZE_F(VD_PIPELINE_FLUSH)() +
        m_miItf->MHW_GETSIZE_F(MI_FLUSH_DW)() +
        m_miItf->MHW_GETSIZE_F(MI_BATCH_BUFFER_START)();

    vdencPicturePatchListSize =
        PATCH_LIST_COMMAND(mhw::mi::Itf::MI_FLUSH_DW_CMD) +
        PATCH_LIST_COMMAND(mhw::mi::Itf::MI_BATCH_BUFFER_START_CMD) +
        PATCH_LIST_COMMAND(mhw::vdbox::vdenc::Itf::VDENC_PIPE_BUF_ADDR_STATE_CMD);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9VdencPkt::DumpOutput()
{
    ENCODE_FUNC_CALL();

#if USE_CODECHAL_DEBUG_TOOL
    // Dump output (resource, information, etc) after command submission
    CodechalDebugInterface *debugInterface = m_pipeline->GetDebugInterface();
    ENCODE_CHK_NULL_RETURN(debugInterface);

    CODEC_REF_LIST currRefList = *((CODEC_REF_LIST *)m_basicFeature->m_ref.GetCurrRefList());

    std::stringstream pipeIdxStrStream;
    pipeIdxStrStream << "_" << (int)m_pipeline->GetCurrentPipe();

    std::string bufferPassName = GetPacketName();
    bufferPassName += pipeIdxStrStream.str() + "_output";

    std::string surfacePassName = "PASS" + std::to_string((uint32_t)m_pipeline->GetCurrentPass());
    surfacePassName += pipeIdxStrStream.str() + "_output";

    if (m_basicFeature->m_ref.DysRefFrameFlags() == DYS_REF_NONE && m_resVdencPakObjCmdStreamOutBuffer != nullptr)
    {
        ENCODE_CHK_STATUS_RETURN(debugInterface->DumpBuffer(
            m_resVdencPakObjCmdStreamOutBuffer,
            CodechalDbgAttr::attrPakObjStreamout,
            bufferPassName.data(),
            m_basicFeature->m_mbCodeSize + 8 * CODECHAL_CACHELINE_SIZE,
            0,
            CODECHAL_NUM_MEDIA_STATES));
    }

    ENCODE_CHK_STATUS_RETURN(debugInterface->DumpBuffer(
        &currRefList.resBitstreamBuffer,
        CodechalDbgAttr::attrBitstream,
        bufferPassName.data(),
        m_basicFeature->m_bitstreamSize,
        0,
        CODECHAL_NUM_MEDIA_STATES));

    PMOS_RESOURCE frameStatStreamOutBuffer = m_basicFeature->m_recycleBuf->GetBuffer(FrameStatStreamOutBuffer, 0);
    ENCODE_CHK_NULL_RETURN(frameStatStreamOutBuffer);
    ENCODE_CHK_STATUS_RETURN(debugInterface->DumpBuffer(
        frameStatStreamOutBuffer,
        CodechalDbgAttr::attrFrameState,
        bufferPassName.data(),
        frameStatStreamOutBuffer->iSize,
        0,
        CODECHAL_NUM_MEDIA_STATES));

    MOS_SURFACE *ds4xSurface = m_basicFeature->m_trackedBuf->GetSurface(
        BufferType::ds4xSurface, currRefList.ucScalingIdx);
    if (ds4xSurface != nullptr)
    {
        ENCODE_CHK_STATUS_RETURN(debugInterface->DumpYUVSurface(
            ds4xSurface,
            CodechalDbgAttr::attrReconstructedSurface,
            (surfacePassName + "_4xScaledSurf").data()));
    }

    MOS_SURFACE *ds8xSurface = m_basicFeature->m_trackedBuf->GetSurface(
        BufferType::ds8xSurface, currRefList.ucScalingIdx);
    if (ds8xSurface != nullptr)
    {
        ENCODE_CHK_STATUS_RETURN(debugInterface->DumpYUVSurface(
            ds8xSurface,
            CodechalDbgAttr::attrReconstructedSurface,
            (surfacePassName + "_8xScaledSurf").data()));
    }

    MOS_RESOURCE *mbCodedBuffer = m_basicFeature->m_trackedBuf->GetBuffer(
        BufferType::mbCodedBuffer, currRefList.ucScalingIdx);
    if (mbCodedBuffer != nullptr)
    {
        ENCODE_CHK_STATUS_RETURN(debugInterface->DumpBuffer(
            mbCodedBuffer,
            CodechalDbgAttr::attrVdencOutput,
            (bufferPassName + "_MbCode").data(),
            m_basicFeature->m_mbCodeSize + 8 * CODECHAL_CACHELINE_SIZE,
            0,
            CODECHAL_NUM_MEDIA_STATES));
    }

    auto          streamInBufferSize = (MOS_ALIGN_CEIL(m_basicFeature->m_frameWidth, 64) / 32) * (MOS_ALIGN_CEIL(m_basicFeature->m_frameHeight, 64) / 32) * CODECHAL_CACHELINE_SIZE;
    PMOS_RESOURCE streamInBuffer     = m_basicFeature->m_recycleBuf->GetBuffer(RecycleResId::StreamInBuffer, m_basicFeature->m_frameNum);
    if (streamInBuffer != nullptr)
    {
        ENCODE_CHK_STATUS_RETURN(debugInterface->DumpBuffer(
            streamInBuffer,
            CodechalDbgAttr::attrStreamIn,
            bufferPassName.data(),
            streamInBufferSize,
            0,
            CODECHAL_NUM_MEDIA_STATES));
    }

    ENCODE_CHK_STATUS_RETURN(debugInterface->DumpBltOutput(
        &currRefList.sRefReconBuffer,
        CodechalDbgAttr::attrDecodeBltOutput));
    ENCODE_CHK_STATUS_RETURN(debugInterface->DumpYUVSurface(
        &currRefList.sRefReconBuffer,
        CodechalDbgAttr::attrReconstructedSurface,
        (surfacePassName + "_ReconSurf").data()));

    ENCODE_CHK_STATUS_RETURN(debugInterface->DumpBltOutput(
        &currRefList.sRefRawBuffer,
        CodechalDbgAttr::attrDecodeBltOutput));
    ENCODE_CHK_STATUS_RETURN(debugInterface->DumpYUVSurface(
        &currRefList.sRefRawBuffer,
        CodechalDbgAttr::attrEncodeRawInputSurface,
        (surfacePassName + "_SrcSurf").data()));
#endif

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9VdencPkt::GetHxxPrimitiveCommandSize()
{
    ENCODE_FUNC_CALL();

    ENCODE_CHK_NULL_RETURN(m_hwInterface);
    ENCODE_CHK_NULL_RETURN(m_pipeline);

    uint32_t hcpCommandsSize = 0;
    uint32_t hcpPatchListSize = 0;
    if (m_pipeline->IsSingleTaskPhaseSupported())
    {
        hcpCommandsSize +=
            m_hcpInterfaceNew->MHW_GETSIZE_F(HCP_VP9_SEGMENT_STATE)() * 8 +
            m_hcpInterfaceNew->MHW_GETSIZE_F(HCP_VP9_PIC_STATE)() +
            m_hcpInterfaceNew->MHW_GETSIZE_F(HCP_BSD_OBJECT)() +
            m_miItf->MHW_GETSIZE_F(MI_BATCH_BUFFER_END)();

        hcpPatchListSize =
            mhw::vdbox::hcp::Itf::HCP_VP9_SEGMENT_STATE_CMD_NUMBER_OF_ADDRESSES * 8 +
            mhw::vdbox::hcp::Itf::HCP_VP9_PIC_STATE_CMD_NUMBER_OF_ADDRESSES +
            mhw::vdbox::hcp::Itf::HCP_BSD_OBJECT_CMD_NUMBER_OF_ADDRESSES;
    }

    uint32_t cpCmdsize = 0;
    uint32_t cpPatchListSize = 0;
    if (m_hwInterface->GetCpInterface())
    {
        m_hwInterface->GetCpInterface()->GetCpSliceLevelCmdSize(cpCmdsize, cpPatchListSize);
    }

    m_defaultSliceStatesSize = hcpCommandsSize + (uint32_t)cpCmdsize;
    m_defaultSlicePatchListSize = hcpPatchListSize + (uint32_t)cpPatchListSize;

    return MOS_STATUS_SUCCESS;
}

uint32_t Vp9VdencPkt::CalculateCommandBufferSize()
{
    ENCODE_FUNC_CALL();
    uint32_t commandBufferSize = 0;

    commandBufferSize = m_pictureStatesSize + (m_sliceStatesSize * m_basicFeature->m_numSlices);
    if (m_pipeline->IsSingleTaskPhaseSupported())
    {
        commandBufferSize *= m_pipeline->GetPassNum();
    }

    // 4K align since allocation is in chunks of 4K bytes
    commandBufferSize = MOS_ALIGN_CEIL(commandBufferSize, CODECHAL_PAGE_SIZE);

    return commandBufferSize;
}

uint32_t Vp9VdencPkt::CalculatePatchListSize()
{
    ENCODE_FUNC_CALL();
    uint32_t requestedPatchListSize = 0;

    if (m_usePatchList)
    {
        requestedPatchListSize = m_picturePatchListSize + (m_slicePatchListSize * m_basicFeature->m_numSlices);
        if (m_pipeline->IsSingleTaskPhaseSupported())
        {
            requestedPatchListSize *= m_pipeline->GetPassNum();
        }

        // Multi pipes are sharing one patchlist
        requestedPatchListSize *= m_pipeline->GetPipeNum();
    }

    return requestedPatchListSize;
}

MOS_STATUS Vp9VdencPkt::AllocateResources()
{
    ENCODE_FUNC_CALL();

    ENCODE_CHK_NULL_RETURN(m_allocator);

    MOS_RESOURCE *allocatedBuffer  = nullptr;
    uint32_t      maxPicWidthInSb  = MOS_ROUNDUP_DIVIDE(m_basicFeature->m_frameWidth, CODEC_VP9_SUPER_BLOCK_WIDTH);
    uint32_t      maxPicHeightInSb = MOS_ROUNDUP_DIVIDE(m_basicFeature->m_frameHeight, CODEC_VP9_SUPER_BLOCK_HEIGHT);
    uint32_t      maxPicSizeInSb   = maxPicWidthInSb * maxPicHeightInSb;

    // Initiate allocation parameters and lock flags
    MOS_ALLOC_GFXRES_PARAMS allocParamsForBufferLinear;
    MOS_ZeroMemory(&allocParamsForBufferLinear, sizeof(MOS_ALLOC_GFXRES_PARAMS));
    allocParamsForBufferLinear.Type     = MOS_GFXRES_BUFFER;
    allocParamsForBufferLinear.TileType = MOS_TILE_LINEAR;
    allocParamsForBufferLinear.Format   = Format_Buffer;

    MOS_ALLOC_GFXRES_PARAMS allocParamsForBuffer2D;
    MOS_ZeroMemory(&allocParamsForBuffer2D, sizeof(MOS_ALLOC_GFXRES_PARAMS));
    allocParamsForBuffer2D.Type     = MOS_GFXRES_2D;
    allocParamsForBuffer2D.TileType = MOS_TILE_LINEAR;
    allocParamsForBuffer2D.Format   = Format_Buffer_2D;

    MOS_ALLOC_GFXRES_PARAMS allocParamsForBufferNV12;
    MOS_ZeroMemory(&allocParamsForBufferNV12, sizeof(MOS_ALLOC_GFXRES_PARAMS));
    allocParamsForBufferNV12.Type     = MOS_GFXRES_2D;
    allocParamsForBufferNV12.TileType = MOS_TILE_Y;
    allocParamsForBufferNV12.Format   = Format_NV12;

    if (m_basicFeature->m_encEnabled)
    {
        if (m_basicFeature->m_hmeSupported)
        {
            MOS_ZeroMemory(&m_4xMeMvDataBuffer, sizeof(MOS_SURFACE));
            m_4xMeMvDataBuffer.TileType      = MOS_TILE_LINEAR;
            m_4xMeMvDataBuffer.bArraySpacing = true;
            m_4xMeMvDataBuffer.Format        = Format_Buffer_2D;
            m_4xMeMvDataBuffer.dwWidth       = m_basicFeature->m_downscaledWidthInMb4x * 32;
            m_4xMeMvDataBuffer.dwHeight      = m_basicFeature->m_downscaledHeightInMb4x * 4 * 10;
            m_4xMeMvDataBuffer.dwPitch       = MOS_ALIGN_CEIL(m_4xMeMvDataBuffer.dwWidth, 128);

            allocParamsForBuffer2D.dwWidth  = m_4xMeMvDataBuffer.dwWidth;
            allocParamsForBuffer2D.dwHeight = m_4xMeMvDataBuffer.dwHeight;
            allocParamsForBuffer2D.pBufName = "4xME MV Data Buffer";
            allocParamsForBuffer2D.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_NOCACHE;
            allocatedBuffer                 = m_allocator->AllocateResource(allocParamsForBuffer2D, true);
            ENCODE_CHK_NULL_RETURN(allocatedBuffer);
            m_4xMeMvDataBuffer.OsResource = *allocatedBuffer;

            MOS_ZeroMemory(&m_4xMeDistortionBuffer, sizeof(MOS_SURFACE));
            m_4xMeDistortionBuffer.TileType      = MOS_TILE_LINEAR;
            m_4xMeDistortionBuffer.bArraySpacing = true;
            m_4xMeDistortionBuffer.Format        = Format_Buffer_2D;
            m_4xMeDistortionBuffer.dwWidth       = m_basicFeature->m_downscaledWidthInMb4x * 8;
            m_4xMeDistortionBuffer.dwHeight      = m_basicFeature->m_downscaledHeightInMb4x * 4 * 10;
            m_4xMeDistortionBuffer.dwPitch       = MOS_ALIGN_CEIL(m_4xMeDistortionBuffer.dwWidth, 128);

            allocParamsForBuffer2D.dwWidth  = m_4xMeDistortionBuffer.dwWidth;
            allocParamsForBuffer2D.dwHeight = m_4xMeDistortionBuffer.dwHeight;
            allocParamsForBuffer2D.pBufName = "4xME Distortion Buffer";
            allocatedBuffer                 = m_allocator->AllocateResource(allocParamsForBuffer2D, true);
            ENCODE_CHK_NULL_RETURN(allocatedBuffer);
            m_4xMeDistortionBuffer.OsResource = *allocatedBuffer;
        }

        if (m_basicFeature->m_16xMeSupported)
        {
            MOS_ZeroMemory(&m_16xMeMvDataBuffer, sizeof(MOS_SURFACE));
            m_16xMeMvDataBuffer.TileType      = MOS_TILE_LINEAR;
            m_16xMeMvDataBuffer.bArraySpacing = true;
            m_16xMeMvDataBuffer.Format        = Format_Buffer_2D;
            m_16xMeMvDataBuffer.dwWidth       = MOS_ALIGN_CEIL((m_basicFeature->m_downscaledWidthInMb16x * 32), 64);
            m_16xMeMvDataBuffer.dwHeight      = m_basicFeature->m_downscaledHeightInMb16x * 4 * 10;
            m_16xMeMvDataBuffer.dwPitch       = MOS_ALIGN_CEIL(m_16xMeMvDataBuffer.dwWidth, 128);

            allocParamsForBuffer2D.dwWidth  = m_16xMeMvDataBuffer.dwWidth;
            allocParamsForBuffer2D.dwHeight = m_16xMeMvDataBuffer.dwHeight;
            allocParamsForBuffer2D.pBufName = "16xME MV Data Buffer";
            allocParamsForBuffer2D.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_NOCACHE;
            allocatedBuffer                 = m_allocator->AllocateResource(allocParamsForBuffer2D, true);
            ENCODE_CHK_NULL_RETURN(allocatedBuffer);
            m_16xMeMvDataBuffer.OsResource = *allocatedBuffer;
        }

        // Intermediate surface to be used by the P kernel to help reduce number of SIC calls
        MOS_ZeroMemory(&m_output16X16InterModes, sizeof(MOS_SURFACE));
        m_output16X16InterModes.TileType      = MOS_TILE_LINEAR;
        m_output16X16InterModes.bArraySpacing = true;
        m_output16X16InterModes.Format        = Format_Buffer_2D;
        m_output16X16InterModes.dwWidth       = 16 * m_basicFeature->m_picWidthInMb;
        m_output16X16InterModes.dwHeight      = 8 * m_basicFeature->m_picHeightInMb;
        m_output16X16InterModes.dwPitch       = MOS_ALIGN_CEIL(m_output16X16InterModes.dwWidth, 64);

        allocParamsForBuffer2D.dwWidth  = m_output16X16InterModes.dwWidth;
        allocParamsForBuffer2D.dwHeight = m_output16X16InterModes.dwHeight;
        allocParamsForBuffer2D.pBufName = "Intermediate surface";
        allocParamsForBuffer2D.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_NOCACHE;
        allocatedBuffer                 = m_allocator->AllocateResource(allocParamsForBuffer2D, true);
        ENCODE_CHK_NULL_RETURN(allocatedBuffer);
        m_output16X16InterModes.OsResource = *allocatedBuffer;
    }

    // VDENC intra row store scratch buffer
    allocParamsForBufferLinear.dwBytes  = m_basicFeature->m_picWidthInMb * CODECHAL_CACHELINE_SIZE;
    allocParamsForBufferLinear.pBufName = "VDENC Intra Row Store Scratch Buffer";
    allocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_NOCACHE;
    allocatedBuffer                     = m_allocator->AllocateResource(allocParamsForBufferLinear, true);
    ENCODE_CHK_NULL_RETURN(allocatedBuffer);
    m_resVdencIntraRowStoreScratchBuffer = *allocatedBuffer;

    // HVC tile row store buffer
    allocParamsForBufferLinear.dwBytes  = CODECHAL_CACHELINE_SIZE * maxPicWidthInSb;
    allocParamsForBufferLinear.pBufName = "HvcTileRowStoreBuffer";
    allocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ;
    allocatedBuffer                     = m_allocator->AllocateResource(allocParamsForBufferLinear, true);
    ENCODE_CHK_NULL_RETURN(allocatedBuffer);
    m_resHvcTileRowStoreBuffer = *allocatedBuffer;

    // This stream out/stream in buffer may need to be a separate buffer on HW, in which case
    // we'll create 2 and ping-pong back and forth pre-frame. For now, though, on simulation/SW,
    // they can be the same buffer.
    allocParamsForBufferLinear.dwBytes  = CODECHAL_CACHELINE_SIZE * maxPicSizeInSb;
    allocParamsForBufferLinear.pBufName = "VDENC Segment Map Stream Out";
    allocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ;
    allocatedBuffer                     = m_allocator->AllocateResource(allocParamsForBufferLinear, true);
    ENCODE_CHK_NULL_RETURN(allocatedBuffer);
    m_resVdencSegmentMapStreamOut = *allocatedBuffer;

    // Allocate SSE source pixel row store buffer
    m_basicFeature->m_sizeOfSseSrcPixelRowStoreBufferPerLcu = ((maxPicWidthInSb + 2) << 5) * CODECHAL_CACHELINE_SIZE;

    uint32_t size                       = m_basicFeature->m_sizeOfSseSrcPixelRowStoreBufferPerLcu * m_basicFeature->m_maxTileNumber;
    allocParamsForBufferLinear.dwBytes  = size;
    allocParamsForBufferLinear.pBufName = "SseSrcPixelRowStoreBuffer";
    allocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_NOCACHE;
    allocatedBuffer                     = m_allocator->AllocateResource(allocParamsForBufferLinear, true);

    ENCODE_CHK_NULL_RETURN(allocatedBuffer);
    m_resSseSrcPixelRowStoreBuffer = *allocatedBuffer;

    // VDenc stream-in data buffer
    allocParamsForBufferLinear.dwBytes = (MOS_ALIGN_CEIL(m_basicFeature->m_frameWidth, 64) / 32) *
                                         (MOS_ALIGN_CEIL(m_basicFeature->m_frameHeight, 64) / 32) * CODECHAL_CACHELINE_SIZE;
    allocParamsForBufferLinear.pBufName = "VDEnc StreamIn Data Buffer";
    allocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_NOCACHE;
    m_basicFeature->m_recycleBuf->RegisterResource(RecycleResId::StreamInBuffer, allocParamsForBufferLinear);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9VdencPkt::FreeResources()
{
    ENCODE_FUNC_CALL();
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    return eStatus;
}

MOS_STATUS Vp9VdencPkt::ValidateVdboxIdx(const MHW_VDBOX_NODE_IND &vdboxIndex)
{
    ENCODE_FUNC_CALL();
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    if (vdboxIndex > m_hwInterface->GetMaxVdboxIndex())
    {
        // ENCODE_ASSERTMESSAGE("ERROR = vdbox index exceed the maximum");
        eStatus = MOS_STATUS_INVALID_PARAMETER;
    }

    return eStatus;
}

void Vp9VdencPkt::SetPerfTag(uint16_t type, uint16_t mode, uint16_t picCodingType)
{
    ENCODE_FUNC_CALL();

    PerfTagSetting perfTag;
    perfTag.Value             = 0;
    perfTag.Mode              = mode & CODECHAL_ENCODE_MODE_BIT_MASK;
    perfTag.CallType          = type;
    perfTag.PictureCodingType = picCodingType > 3 ? 0 : picCodingType;
    m_osInterface->pfnSetPerfTag(m_osInterface, perfTag.Value);
    m_osInterface->pfnIncPerfBufferID(m_osInterface);
}

MOS_STATUS Vp9VdencPkt::AddForceWakeup(MOS_COMMAND_BUFFER &cmdBuffer)
{
    ENCODE_FUNC_CALL();

    ENCODE_CHK_NULL_RETURN(m_miItf);

    auto &forceWakeupParams = m_miItf->MHW_GETPAR_F(MI_FORCE_WAKEUP)();

    forceWakeupParams                           = {};
    forceWakeupParams.bMFXPowerWellControl      = true;
    forceWakeupParams.bMFXPowerWellControlMask  = true;
    forceWakeupParams.bHEVCPowerWellControl     = true;
    forceWakeupParams.bHEVCPowerWellControlMask = true;

    ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_FORCE_WAKEUP)(&cmdBuffer));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9VdencPkt::SendPrologCmds(MOS_COMMAND_BUFFER &cmdBuffer)
{
    ENCODE_FUNC_CALL();

#ifdef _MMC_SUPPORTED
    ENCODE_CHK_NULL_RETURN(m_mmcState);
    ENCODE_CHK_STATUS_RETURN(m_mmcState->SendPrologCmd(&cmdBuffer, false));
#endif

    MHW_GENERIC_PROLOG_PARAMS genericPrologParams;
    MOS_ZeroMemory(&genericPrologParams, sizeof(genericPrologParams));
    genericPrologParams.pOsInterface     = m_osInterface;
    genericPrologParams.pvMiInterface    = nullptr;
    genericPrologParams.bMmcEnabled      = m_mmcState ? m_mmcState->IsMmcEnabled() : false;
    genericPrologParams.dwStoreDataValue = m_basicFeature->m_frameNum;
    ENCODE_CHK_STATUS_RETURN(Mhw_SendGenericPrologCmdNext(&cmdBuffer, &genericPrologParams, m_miItf));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9VdencPkt::SetRowstoreCachingOffsets()
{
    ENCODE_FUNC_CALL();

    MHW_VDBOX_ROWSTORE_PARAMS rowStoreParams;
    rowStoreParams.Mode             = m_basicFeature->m_mode;
    rowStoreParams.dwPicWidth       = m_basicFeature->m_frameWidth;
    rowStoreParams.ucChromaFormat   = m_basicFeature->m_chromaFormat;
    rowStoreParams.ucBitDepthMinus8 = (m_basicFeature->m_bitDepth - 8);  // 0(8bit) -> 0, 1(10bit)->2, 2(12bit)->4
    if (m_hcpInterfaceNew->IsRowStoreCachingSupported())
    {
        ENCODE_CHK_STATUS_RETURN(m_hwInterface->SetRowstoreCachingOffsets(&rowStoreParams));
    }  

    if (m_hcpInterfaceNew->IsRowStoreCachingSupported())
    {
        hcp::HcpVdboxRowStorePar rowstoreParamsHCP = {};
        rowstoreParamsHCP.Mode             = m_basicFeature->m_mode;
        rowstoreParamsHCP.dwPicWidth       = m_basicFeature->m_frameWidth;
        rowstoreParamsHCP.ucChromaFormat   = m_basicFeature->m_chromaFormat;
        rowstoreParamsHCP.ucBitDepthMinus8 = (m_basicFeature->m_bitDepth - 8);  // 0(8bit) -> 0, 1(10bit)->2, 2(12bit)->4
        ENCODE_CHK_STATUS_RETURN(m_hcpInterfaceNew->SetRowstoreCachingOffsets(rowstoreParamsHCP));
    }

    if (m_vdencInterfaceNew)
    {
        mhw::vdbox::vdenc::RowStorePar par = {};

        par.mode = mhw::vdbox::vdenc::RowStorePar::VP9;
        par.bitDepth = mhw::vdbox::vdenc::RowStorePar::DEPTH_8;
        if (rowStoreParams.ucBitDepthMinus8 == 1 || rowStoreParams.ucBitDepthMinus8 == 2)
        {
            par.bitDepth = mhw::vdbox::vdenc::RowStorePar::DEPTH_10;
        }
        else if (rowStoreParams.ucBitDepthMinus8 > 2)
        {
            par.bitDepth = mhw::vdbox::vdenc::RowStorePar::DEPTH_12;
        }
        par.frameWidth = rowStoreParams.dwPicWidth;
        switch (rowStoreParams.ucChromaFormat)
        {
        case HCP_CHROMA_FORMAT_MONOCHROME:
            par.format = mhw ::vdbox::vdenc::RowStorePar::MONOCHROME;
            break;
        case HCP_CHROMA_FORMAT_YUV420:
            par.format = mhw ::vdbox::vdenc::RowStorePar::YUV420;
            break;
        case HCP_CHROMA_FORMAT_YUV422:
            par.format = mhw ::vdbox::vdenc::RowStorePar::YUV422;
            break;
        case HCP_CHROMA_FORMAT_YUV444:
            par.format = mhw ::vdbox::vdenc::RowStorePar::YUV444;
            break;
        }

        ENCODE_CHK_STATUS_RETURN(m_vdencInterfaceNew->SetRowstoreCachingOffsets(par));
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9VdencPkt::AddHcpPipeBufAddrCmd(MOS_COMMAND_BUFFER &cmdBuffer)
{
    if (m_basicFeature->m_ref.DysRefFrameFlags() == DYS_REF_NONE)
    {
        m_resVdencPakObjCmdStreamOutBuffer = m_basicFeature->m_resMbCodeBuffer;
    }

    RUN_FEATURE_INTERFACE_RETURN(Vp9EncodeHpu, Vp9FeatureIDs::vp9HpuFeature, SetIsLastPass, m_pipeline->IsLastPass());
    SETPAR_AND_ADDCMD(HCP_PIPE_BUF_ADDR_STATE, m_hcpInterfaceNew, &cmdBuffer);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9VdencPkt::SetHcpSurfaceMMCState()
{
#ifdef _MMC_SUPPORTED
    for (uint8_t i = 0; i <= CODECHAL_HCP_ALTREF_SURFACE_ID; ++i)
    {
        SetSurfaceMmcState(&m_surfacesParams[i]);
    }
    //Get each reference surface state and be recorded by skipMask if current surface state is mmc disabled
    //In VP9 mode, Bit 8is (here is bit0 in skipMask ) for Previous Reference; 
    //Bit 9is (here is bit1 in skipMask ) for Golden Reference and Bit 10is (here is bit2 in skipMask ) for Alterante Reference; 
    //Bits11-15are unused and should be programmed to 0 (skipped)
    uint8_t skipMask = 0xf8;
    for (uint8_t i = CODECHAL_HCP_LAST_SURFACE_ID; i <= CODECHAL_HCP_ALTREF_SURFACE_ID; i++)
    {
        if (m_surfacesParams[i].mmcState == MOS_MEMCOMP_DISABLED)
        {
            skipMask |= (1 << (i - 2));
        }
    }
    ENCODE_NORMALMESSAGE("MMC skip mask is %d\n", skipMask);
    for (uint8_t i = CODECHAL_HCP_LAST_SURFACE_ID; i <= CODECHAL_HCP_ALTREF_SURFACE_ID; i++)
    {
        //Set each ref surface state as MOS_MEMCOMP_MC to satisfy MmcEnable in AddHcpSurfaceCmd
        //Because each ref surface state should be programmed as the same
        //The actual mmc state is recorded by skipMask and set each ref surface too
        m_surfacesParams[i].mmcState = MOS_MEMCOMP_MC;
        m_surfacesParams[i].mmcSkipMask = skipMask;
    }
#endif  // _MMC_SUPPORTED
    return MOS_STATUS_SUCCESS;
}

void Vp9VdencPkt::fill_pad_with_value(PMOS_SURFACE psSurface, uint32_t real_height, uint32_t aligned_height)
{
    ENCODE_CHK_NULL_NO_STATUS_RETURN(psSurface);

    // unaligned surfaces only
    if (aligned_height <= real_height || aligned_height > psSurface->dwHeight)
    {
        return;
    }

    // avoid DYS frames cases
    if (m_basicFeature->m_ref.DysRefFrameFlags() != DYS_REF_NONE && m_basicFeature->m_dysVdencMultiPassEnabled)
    {
        return;
    }

    if (psSurface->OsResource.TileType == MOS_TILE_INVALID)
    {
        return;
    }

    if (psSurface->Format == Format_NV12 || psSurface->Format == Format_P010)
    {
        uint32_t pitch         = psSurface->dwPitch;
        uint32_t UVPlaneOffset = psSurface->UPlaneOffset.iSurfaceOffset;
        uint32_t YPlaneOffset  = psSurface->dwOffset;
        uint32_t pad_rows = aligned_height - real_height;
        uint32_t y_plane_size   = pitch * real_height;
        uint32_t uv_plane_size   = pitch * real_height / 2;

        MOS_LOCK_PARAMS lockFlags;
        MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
        lockFlags.WriteOnly = 1;

        // padding for the linear format buffer.
        if (psSurface->OsResource.TileType == MOS_TILE_LINEAR)
        {
            uint8_t *src_data   = (uint8_t *)m_osInterface->pfnLockResource(m_osInterface, &(psSurface->OsResource), &lockFlags);

            if (!src_data)
                return;

            uint8_t *src_data_y     = src_data + YPlaneOffset;
            uint8_t *src_data_y_end = src_data_y + y_plane_size;
            for (uint32_t i = 0; i < pad_rows; i++)
            {
                MOS_SecureMemcpy(src_data_y_end + i * pitch, pitch, src_data_y_end - pitch, pitch);
            }

            uint8_t *src_data_uv     = src_data + UVPlaneOffset;
            uint8_t *src_data_uv_end = src_data_uv + uv_plane_size;
            for (uint32_t i = 0; i < pad_rows / 2; i++)
            {
                MOS_SecureMemcpy(src_data_uv_end + i * pitch, pitch, src_data_uv_end - pitch, pitch);
            }

            m_osInterface->pfnUnlockResource(m_osInterface, &(psSurface->OsResource));
        }
        else
        {
            // we don't copy out the whole tiled buffer to linear and padding on the tiled buffer directly.
            lockFlags.TiledAsTiled = 1;

            uint8_t *src_data   = (uint8_t *)m_osInterface->pfnLockResource(m_osInterface, &(psSurface->OsResource), &lockFlags);
            if (!src_data)
                return;

            uint8_t* padding_data = (uint8_t *)MOS_AllocMemory(pitch * pad_rows);

            // Copy last Y row data to linear padding data.
            GMM_RES_COPY_BLT    gmmResCopyBlt  = {0};
            gmmResCopyBlt.Gpu.pData      = src_data;
            gmmResCopyBlt.Gpu.OffsetX    = 0;
            gmmResCopyBlt.Gpu.OffsetY    = (YPlaneOffset + y_plane_size - pitch) / pitch;
            gmmResCopyBlt.Sys.pData      = padding_data;
            gmmResCopyBlt.Sys.RowPitch   = pitch;
            gmmResCopyBlt.Sys.BufferSize = pitch * pad_rows;
            gmmResCopyBlt.Sys.SlicePitch = pitch;
            gmmResCopyBlt.Blt.Slices     = 1;
            gmmResCopyBlt.Blt.Upload     = false;
            gmmResCopyBlt.Blt.Width      = psSurface->dwWidth;
            gmmResCopyBlt.Blt.Height     = 1;
            psSurface->OsResource.pGmmResInfo->CpuBlt(&gmmResCopyBlt);
            // Fill the remain padding lines with last Y row data.
            for (uint32_t i = 1; i < pad_rows; i++)
            {
                MOS_SecureMemcpy(padding_data + i * pitch, pitch, padding_data, pitch);
            }
            // Filling the padding for Y.
            gmmResCopyBlt.Gpu.pData      = src_data;
            gmmResCopyBlt.Gpu.OffsetX    = 0;
            gmmResCopyBlt.Gpu.OffsetY    = (YPlaneOffset + y_plane_size) / pitch;
            gmmResCopyBlt.Sys.pData      = padding_data;
            gmmResCopyBlt.Sys.RowPitch   = pitch;
            gmmResCopyBlt.Sys.BufferSize = pitch * pad_rows;
            gmmResCopyBlt.Sys.SlicePitch = pitch;
            gmmResCopyBlt.Blt.Slices     = 1;
            gmmResCopyBlt.Blt.Upload     = true;
            gmmResCopyBlt.Blt.Width      = psSurface->dwWidth;
            gmmResCopyBlt.Blt.Height     = pad_rows;
            psSurface->OsResource.pGmmResInfo->CpuBlt(&gmmResCopyBlt);

            // Copy last UV row data to linear padding data.
            gmmResCopyBlt.Gpu.pData      = src_data;
            gmmResCopyBlt.Gpu.OffsetX    = 0;
            gmmResCopyBlt.Gpu.OffsetY    = (UVPlaneOffset + uv_plane_size - pitch) / pitch;
            gmmResCopyBlt.Sys.pData      = padding_data;
            gmmResCopyBlt.Sys.RowPitch   = pitch;
            gmmResCopyBlt.Sys.BufferSize = pitch * pad_rows / 2;
            gmmResCopyBlt.Sys.SlicePitch = pitch;
            gmmResCopyBlt.Blt.Slices     = 1;
            gmmResCopyBlt.Blt.Upload     = false;
            gmmResCopyBlt.Blt.Width      = psSurface->dwWidth;
            gmmResCopyBlt.Blt.Height     = 1;
            psSurface->OsResource.pGmmResInfo->CpuBlt(&gmmResCopyBlt);
            // Fill the remain padding lines with last UV row data.
            for (uint32_t i = 1; i < pad_rows / 2; i++)
            {
                MOS_SecureMemcpy(padding_data + i * pitch, pitch, padding_data, pitch);
            }
            // Filling the padding for UV.
            gmmResCopyBlt.Gpu.pData      = src_data;
            gmmResCopyBlt.Gpu.OffsetX    = 0;
            gmmResCopyBlt.Gpu.OffsetY    = (UVPlaneOffset + uv_plane_size) / pitch;
            gmmResCopyBlt.Sys.pData      = padding_data;
            gmmResCopyBlt.Sys.RowPitch   = pitch;
            gmmResCopyBlt.Sys.BufferSize = pitch * pad_rows / 2;
            gmmResCopyBlt.Sys.SlicePitch = pitch;
            gmmResCopyBlt.Blt.Slices     = 1;
            gmmResCopyBlt.Blt.Upload     = true;
            gmmResCopyBlt.Blt.Width      = psSurface->dwWidth;
            gmmResCopyBlt.Blt.Height     = pad_rows / 2;
            psSurface->OsResource.pGmmResInfo->CpuBlt(&gmmResCopyBlt);

            MOS_FreeMemory(padding_data);
            padding_data = nullptr;
            m_osInterface->pfnUnlockResource(m_osInterface, &(psSurface->OsResource));
        }
    }
}

MOS_STATUS Vp9VdencPkt::SetHcpSurfacesParams(MHW_VDBOX_SURFACE_PARAMS *surfacesParams)
{
    ENCODE_FUNC_CALL();

    for (uint8_t i = 0; i <= CODECHAL_HCP_ALTREF_SURFACE_ID; ++i)
    {
        MOS_ZeroMemory(&surfacesParams[i], sizeof(surfacesParams[i]));
        surfacesParams[i].Mode             = m_basicFeature->m_mode;
        surfacesParams[i].ucSurfaceStateId = i;
        surfacesParams[i].ChromaType       = m_basicFeature->m_outputChromaFormat;
        surfacesParams[i].bSrc8Pak10Mode   = (m_vp9SeqParams->SeqFlags.fields.EncodedBitDepth) && (!m_vp9SeqParams->SeqFlags.fields.SourceBitDepth);

        switch (m_vp9SeqParams->SeqFlags.fields.EncodedBitDepth)
        {
        case VP9_ENCODED_BIT_DEPTH_10: {
            surfacesParams[i].ucBitDepthChromaMinus8 = 2;
            surfacesParams[i].ucBitDepthLumaMinus8   = 2;
            break;
        }
        default: {
            surfacesParams[i].ucBitDepthChromaMinus8 = 0;
            surfacesParams[i].ucBitDepthLumaMinus8   = 0;
            break;
        }
        }
    }

    ENCODE_CHK_STATUS_RETURN(m_basicFeature->m_ref.SetHcpSurfaceParams(surfacesParams));

    // Program surface params for reconstructed surface
    surfacesParams[CODECHAL_HCP_DECODED_SURFACE_ID].psSurface         = &m_basicFeature->m_reconSurface;
    surfacesParams[CODECHAL_HCP_DECODED_SURFACE_ID].dwReconSurfHeight = m_basicFeature->m_rawSurfaceToPak->dwHeight;

    // Program surface params for source surface
    surfacesParams[CODECHAL_HCP_SRC_SURFACE_ID].psSurface             = m_basicFeature->m_rawSurfaceToPak;
    surfacesParams[CODECHAL_HCP_SRC_SURFACE_ID].bDisplayFormatSwizzle = m_vp9SeqParams->SeqFlags.fields.DisplayFormatSwizzle;
    surfacesParams[CODECHAL_HCP_SRC_SURFACE_ID].dwActualWidth         = MOS_ALIGN_CEIL(m_basicFeature->m_oriFrameWidth, CODEC_VP9_MIN_BLOCK_WIDTH);
    surfacesParams[CODECHAL_HCP_SRC_SURFACE_ID].dwActualHeight        = MOS_ALIGN_CEIL(m_basicFeature->m_oriFrameHeight, CODEC_VP9_MIN_BLOCK_HEIGHT);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9VdencPkt::AddHcpIndObjBaseAddrCmd(MOS_COMMAND_BUFFER &cmdBuffer)
{
    ENCODE_FUNC_CALL();

    SETPAR_AND_ADDCMD(HCP_IND_OBJ_BASE_ADDR_STATE, m_hcpInterfaceNew, &cmdBuffer);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9VdencPkt::AddVdencPipeModeSelectCmd(MOS_COMMAND_BUFFER &cmdBuffer)
{
    ENCODE_FUNC_CALL();

    SETPAR_AND_ADDCMD(VDENC_PIPE_MODE_SELECT, m_vdencInterfaceNew, &cmdBuffer);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9VdencPkt::SetVdencDsSurfaceParams(MHW_VDBOX_SURFACE_PARAMS *dsSurfaceParams)
{
    ENCODE_FUNC_CALL();

    // 8xDS surface
    MOS_ZeroMemory(&dsSurfaceParams[0], sizeof(MHW_VDBOX_SURFACE_PARAMS));
    dsSurfaceParams[0].Mode             = m_basicFeature->m_mode;
    dsSurfaceParams[0].ucSurfaceStateId = CODECHAL_MFX_DSRECON_SURFACE_ID;
    dsSurfaceParams[0].psSurface        = m_basicFeature->m_8xDSSurface;
    // 4xDS Surface
    MOS_ZeroMemory(&dsSurfaceParams[1], sizeof(MHW_VDBOX_SURFACE_PARAMS));
    dsSurfaceParams[1].Mode             = m_basicFeature->m_mode;
    dsSurfaceParams[1].ucSurfaceStateId = CODECHAL_MFX_DSRECON_SURFACE_ID;
    dsSurfaceParams[1].psSurface        = m_basicFeature->m_4xDSSurface;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9VdencPkt::AddVdencSurfacesStateCmd(MOS_COMMAND_BUFFER &cmdBuffer)
{
    ENCODE_FUNC_CALL();

    ENCODE_CHK_STATUS_RETURN(m_basicFeature->m_ref.SetVdencSurfaceParams(m_surfacesParams));

    // Set VDENC source surface parameters
    SETPAR_AND_ADDCMD(VDENC_SRC_SURFACE_STATE, m_vdencInterfaceNew, &cmdBuffer);

    // Set VDENC recon surface parameters.
    // Check picture coding type:
    // - if I_TYPE, recon surface for reference
    // - otherwise, use LAST_SURFACE, or GOLDEN_SURFACE or ALTREF_SURFACE for reference
    if (m_basicFeature->m_pictureCodingType == I_TYPE)
    {
        m_curVdencSurfStateId = CODECHAL_HCP_DECODED_SURFACE_ID;
        SETPAR_AND_ADDCMD(VDENC_REF_SURFACE_STATE, m_vdencInterfaceNew, &cmdBuffer);
    }
    else
    {
        m_curVdencSurfStateId = CODECHAL_HCP_LAST_SURFACE_ID;
        SETPAR_AND_ADDCMD(VDENC_REF_SURFACE_STATE, m_vdencInterfaceNew, &cmdBuffer);

        auto dysRefFrameFlags = m_basicFeature->m_ref.DysRefFrameFlags();
        auto dysVdencMultiPassEnabled = m_basicFeature->m_dysVdencMultiPassEnabled;

        if ((dysRefFrameFlags != DYS_REF_NONE) && !dysVdencMultiPassEnabled)
        {
            auto refFrameFlags = m_basicFeature->m_ref.RefFrameFlags();
            if (refFrameFlags & 0x02)
            {
                m_curVdencSurfStateId = CODECHAL_HCP_GOLDEN_SURFACE_ID;
                SETPAR_AND_ADDCMD(VDENC_REF_SURFACE_STATE, m_vdencInterfaceNew, &cmdBuffer);
            }
            if (refFrameFlags & 0x04)
            {
                m_curVdencSurfStateId = CODECHAL_HCP_ALTREF_SURFACE_ID;
                SETPAR_AND_ADDCMD(VDENC_REF_SURFACE_STATE, m_vdencInterfaceNew, &cmdBuffer);
            }
        }
    }

    // Set 4x, 8x scaled surfaces
    SETPAR_AND_ADDCMD(VDENC_DS_REF_SURFACE_STATE, m_vdencInterfaceNew, &cmdBuffer);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9VdencPkt::SetVdencPipeModeSelectParams(MHW_VDBOX_PIPE_MODE_SELECT_PARAMS &vdboxPipeModeSelectParams)
{
    ENCODE_FUNC_CALL();

    auto brcFeature = dynamic_cast<Vp9EncodeBrc *>(m_featureManager->GetFeature(Vp9FeatureIDs::vp9BrcFeature));
    ENCODE_CHK_NULL_RETURN(brcFeature);

    vdboxPipeModeSelectParams                                = {};
    vdboxPipeModeSelectParams.Mode                           = m_basicFeature->m_mode;
    vdboxPipeModeSelectParams.bStreamOutEnabled              = brcFeature->IsVdencBrcEnabled();
    vdboxPipeModeSelectParams.bVdencEnabled                  = true;
    vdboxPipeModeSelectParams.bVdencPakObjCmdStreamOutEnable = m_vdencPakObjCmdStreamOutEnabled;
    vdboxPipeModeSelectParams.bTlbPrefetchEnable             = true;

    // Add 1 to compensate for VdencPipeModeSelect params values
    vdboxPipeModeSelectParams.ChromaType = m_basicFeature->m_vp9SeqParams->SeqFlags.fields.EncodedFormat + 1;
    switch (m_basicFeature->m_vp9SeqParams->SeqFlags.fields.EncodedBitDepth)
    {
    case VP9_ENCODED_BIT_DEPTH_10: {
        vdboxPipeModeSelectParams.ucVdencBitDepthMinus8 = 2;
        break;
    }
    default: {
        vdboxPipeModeSelectParams.ucVdencBitDepthMinus8 = 0;
        break;
    }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9VdencPkt::AddVdencPipeBufAddrCmd(MOS_COMMAND_BUFFER &cmdBuffer)
{
    if (m_basicFeature->m_ref.DysRefFrameFlags() == DYS_REF_NONE)
    {
        m_resVdencPakObjCmdStreamOutBuffer = m_basicFeature->m_resMbCodeBuffer;
    }

    SETPAR_AND_ADDCMD(VDENC_PIPE_BUF_ADDR_STATE, m_vdencInterfaceNew, &cmdBuffer);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9VdencPkt::SetVdencPipeBufAddrParams(MHW_VDBOX_PIPE_BUF_ADDR_PARAMS &pipeBufAddrParams)
{
    ENCODE_FUNC_CALL();

    pipeBufAddrParams.presVdencTileRowStoreBuffer                = &m_vdencTileRowStoreBuffer;
    pipeBufAddrParams.presVdencCumulativeCuCountStreamoutSurface = &m_vdencCumulativeCuCountStreamoutSurface;

    auto dysRefFrameFlags         = m_basicFeature->m_ref.DysRefFrameFlags();
    auto dysVdencMultiPassEnabled = m_basicFeature->m_dysVdencMultiPassEnabled;

    pipeBufAddrParams.bDynamicScalingEnable = (dysRefFrameFlags != DYS_REF_NONE) && !dysVdencMultiPassEnabled;
    pipeBufAddrParams.pRawSurfParam         = &m_surfacesParams[CODECHAL_HCP_SRC_SURFACE_ID];
    pipeBufAddrParams.pDecodedReconParam    = &m_surfacesParams[CODECHAL_HCP_DECODED_SURFACE_ID];

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9VdencPkt::SetPipeBufAddrMmcState(PMHW_VDBOX_PIPE_BUF_ADDR_PARAMS pipeBufAddrParams)
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_NULL_RETURN(m_mmcState);

    if (m_mmcState->IsMmcEnabled())
    {
        ENCODE_CHK_STATUS_RETURN(m_mmcState->GetSurfaceMmcState(&m_basicFeature->m_reconSurface, &pipeBufAddrParams->PreDeblockSurfMmcState));
        pipeBufAddrParams->PostDeblockSurfMmcState = pipeBufAddrParams->PreDeblockSurfMmcState;
        ENCODE_CHK_STATUS_RETURN(m_mmcState->GetSurfaceMmcState(&m_basicFeature->m_rawSurface, &pipeBufAddrParams->RawSurfMmcState));
    }
    else
    {
        pipeBufAddrParams->PreDeblockSurfMmcState  = MOS_MEMCOMP_DISABLED;
        pipeBufAddrParams->PostDeblockSurfMmcState = MOS_MEMCOMP_DISABLED;
        pipeBufAddrParams->RawSurfMmcState         = MOS_MEMCOMP_DISABLED;
    }

    CODECHAL_DEBUG_TOOL(
        m_basicFeature->m_reconSurface.MmcState = pipeBufAddrParams->PreDeblockSurfMmcState;)

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9VdencPkt::AddVdencSecondLevelBatchBufferCmd(MOS_COMMAND_BUFFER &cmdBuffer)
{
    ENCODE_FUNC_CALL();

    MHW_BATCH_BUFFER secondLevelBatchBuffer;
    MOS_ZeroMemory(&secondLevelBatchBuffer, sizeof(secondLevelBatchBuffer));
    RUN_FEATURE_INTERFACE_RETURN(Vp9EncodePak, Vp9FeatureIDs::vp9PakFeature, SetVdencSecondLevelBatchBuffer, m_pipeline->GetCurrentPass(), secondLevelBatchBuffer);
    ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_BATCH_BUFFER_START)(&cmdBuffer, &secondLevelBatchBuffer));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9VdencPkt::SetSurfaceMmcState(PMHW_VDBOX_SURFACE_PARAMS surfaceStateParams)
{
    ENCODE_CHK_NULL_RETURN(surfaceStateParams);
    ENCODE_CHK_NULL_RETURN(surfaceStateParams->psSurface);
    ENCODE_CHK_NULL_RETURN(m_mmcState);

    ENCODE_FUNC_CALL();

    if (m_mmcState->IsMmcEnabled())
    {
        ENCODE_CHK_STATUS_RETURN(m_mmcState->GetSurfaceMmcState(surfaceStateParams->psSurface, &surfaceStateParams->mmcState));
        ENCODE_CHK_STATUS_RETURN(m_mmcState->GetSurfaceMmcFormat(surfaceStateParams->psSurface, &surfaceStateParams->dwCompressionFormat));
    }
    else
    {
        surfaceStateParams->mmcState = MOS_MEMCOMP_DISABLED;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9VdencPkt::StartStatusReport(uint32_t srType, MOS_COMMAND_BUFFER *cmdBuffer)
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_NULL_RETURN(cmdBuffer);

    ENCODE_CHK_STATUS_RETURN(MediaPacket::StartStatusReportNext(srType, cmdBuffer));
    m_encodecp->StartCpStatusReport(cmdBuffer);

    MediaPerfProfiler *perfProfiler = MediaPerfProfiler::Instance();
    ENCODE_CHK_NULL_RETURN(perfProfiler);
    ENCODE_CHK_STATUS_RETURN(perfProfiler->AddPerfCollectStartCmd(
        (void *)m_pipeline, m_osInterface, m_miItf, cmdBuffer));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9VdencPkt::EndStatusReport(uint32_t srType, MOS_COMMAND_BUFFER *cmdBuffer)
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_NULL_RETURN(cmdBuffer);

    ENCODE_CHK_STATUS_RETURN(MediaPacket::EndStatusReportNext(srType, cmdBuffer));

    MediaPerfProfiler *perfProfiler = MediaPerfProfiler::Instance();
    ENCODE_CHK_NULL_RETURN(perfProfiler);
    ENCODE_CHK_STATUS_RETURN(perfProfiler->AddPerfCollectEndCmd(
        (void *)m_pipeline, m_osInterface, m_miItf, cmdBuffer));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9VdencPkt::EnsureAllCommandsExecuted(MOS_COMMAND_BUFFER &cmdBuffer)
{
    ENCODE_FUNC_CALL();

    // Flush the engine to ensure memory written out
    auto &flushDwParams                         = m_miItf->MHW_GETPAR_F(MI_FLUSH_DW)();
    flushDwParams                               = {};
    flushDwParams.bVideoPipelineCacheInvalidate = true;
    ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_FLUSH_DW)(&cmdBuffer));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9VdencPkt::ReadHcpStatus(
    MHW_VDBOX_NODE_IND  vdboxIndex,
    MediaStatusReport * statusReport,
    MOS_COMMAND_BUFFER &cmdBuffer)
{
    ENCODE_FUNC_CALL();

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODEC_HW_FUNCTION_ENTER;

    ENCODE_CHK_NULL_RETURN(statusReport);
    ENCODE_CHK_NULL_RETURN(m_hwInterface);

    ENCODE_CHK_COND_RETURN((vdboxIndex > m_hwInterface->GetMaxVdboxIndex()), "ERROR - vdbox index exceed the maximum");

    MOS_RESOURCE *osResource = nullptr;
    uint32_t      offset     = 0;

    EncodeStatusReadParams params;
    MOS_ZeroMemory(&params, sizeof(params));

    ENCODE_CHK_STATUS_RETURN(statusReport->GetAddress(encode::statusReportMfxBitstreamByteCountPerFrame, osResource, offset));
    params.resBitstreamByteCountPerFrame    = osResource;
    params.bitstreamByteCountPerFrameOffset = offset;

    auto &flushDwParams = m_miItf->MHW_GETPAR_F(MI_FLUSH_DW)();
    flushDwParams       = {};
    ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_FLUSH_DW)(&cmdBuffer));

    auto mmioRegisters = m_hcpInterfaceNew->GetMmioRegisters(vdboxIndex);
    auto &miStoreRegMemParams           = m_miItf->MHW_GETPAR_F(MI_STORE_REGISTER_MEM)();
    miStoreRegMemParams                 = {};
    miStoreRegMemParams.presStoreBuffer = params.resBitstreamByteCountPerFrame;
    miStoreRegMemParams.dwOffset        = params.bitstreamByteCountPerFrameOffset;
    miStoreRegMemParams.dwRegister      = mmioRegisters->hcpVp9EncBitstreamBytecountFrameRegOffset;
    ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_STORE_REGISTER_MEM)(&cmdBuffer));

    auto brcFeature = dynamic_cast<Vp9EncodeBrc *>(m_featureManager->GetFeature(Vp9FeatureIDs::vp9BrcFeature));
    ENCODE_CHK_NULL_RETURN(brcFeature);

    HucBrcBuffers *hucBrcBuffers = nullptr;
    ENCODE_CHK_STATUS_RETURN(brcFeature->GetHucBrcBuffers(hucBrcBuffers));
    ENCODE_CHK_NULL_RETURN(hucBrcBuffers);

    auto &copyMemMemParams       = m_miItf->MHW_GETPAR_F(MI_COPY_MEM_MEM)();
    copyMemMemParams             = {};
    copyMemMemParams.presSrc     = params.resBitstreamByteCountPerFrame;
    copyMemMemParams.dwSrcOffset = params.bitstreamByteCountPerFrameOffset;
    copyMemMemParams.presDst     = &hucBrcBuffers->resBrcBitstreamSizeBuffer;
    copyMemMemParams.dwDstOffset = CODECHAL_OFFSETOF(EncodeVp9BSBuffer, dwHcpBitstreamByteCountFrame);
    ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_COPY_MEM_MEM)(&cmdBuffer));

    // Write frame size directly to huc second pass dmem buffer.
    // It is needed for correct pipeline synchronization and dmem initialization
    PMOS_RESOURCE hucProbDmemBuffer = nullptr;
    // For BRC cases, do not overwrite the HPU probability in huc Dmen buffer in the last pass
    uint32_t idx = brcFeature->IsVdencBrcEnabled() ? 2 : 1;
    RUN_FEATURE_INTERFACE_RETURN(Vp9EncodeHpu, Vp9FeatureIDs::vp9HpuFeature, GetHucProbDmemBuffer, idx, hucProbDmemBuffer);
    ENCODE_CHK_NULL_RETURN(hucProbDmemBuffer);

    copyMemMemParams             = {};
    copyMemMemParams.presSrc     = params.resBitstreamByteCountPerFrame;
    copyMemMemParams.dwSrcOffset = params.bitstreamByteCountPerFrameOffset;
    copyMemMemParams.presDst     = hucProbDmemBuffer;
    copyMemMemParams.dwDstOffset = CODECHAL_OFFSETOF(HucProbDmem, FrameSize);
    ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_COPY_MEM_MEM)(&cmdBuffer));

    return eStatus;
}

MOS_STATUS Vp9VdencPkt::UpdateParameters()
{
    ENCODE_FUNC_CALL();

    auto isLastPipe = m_pipeline->IsLastPipe();
    auto isLastPass = m_pipeline->IsLastPass();

    if (isLastPipe && isLastPass)
    {
        m_basicFeature->m_contextFrameTypes[m_vp9PicParams->PicFlags.fields.frame_context_idx] = m_vp9PicParams->PicFlags.fields.frame_type;
        m_basicFeature->m_prevFrameSegEnabled = m_vp9PicParams->PicFlags.fields.segmentation_enabled;

        // If Huc super frame enabled, update m_prevFrameInfo in super frame pass packet
        if (!(m_basicFeature->m_vp9PicParams->PicFlags.fields.super_frame && m_basicFeature->m_tsEnabled))
        {
            ENCODE_CHK_STATUS_RETURN(m_basicFeature->UpdateParameters());
        }
    }

    if (isLastPipe)
    {
        RUN_FEATURE_INTERFACE_RETURN(Vp9EncodePak, Vp9FeatureIDs::vp9PakFeature, UpdateParameters);
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9VdencPkt::DumpInput()
{
    ENCODE_FUNC_CALL();

#if USE_CODECHAL_DEBUG_TOOL
    // Dump output (resource, information, etc) before command submitted
    CodechalDebugInterface *debugInterface = m_pipeline->GetDebugInterface();
    ENCODE_CHK_NULL_RETURN(debugInterface);

    CODEC_REF_LIST currRefList = *((CODEC_REF_LIST *)m_basicFeature->m_ref.GetCurrRefList());

    std::stringstream pipeIdxStrStream;
    pipeIdxStrStream << "_" << (int)m_pipeline->GetCurrentPipe();

    std::string bufferPassName = GetPacketName();
    bufferPassName += pipeIdxStrStream.str() + "_input";

    std::string surfacePassName = "PASS" + std::to_string((uint32_t)m_pipeline->GetCurrentPass());
    surfacePassName += pipeIdxStrStream.str() + "_input";

    ENCODE_CHK_STATUS_RETURN(debugInterface->DumpBuffer(
        &currRefList.resBitstreamBuffer,
        CodechalDbgAttr::attrBitstream,
        bufferPassName.data(),
        m_basicFeature->m_bitstreamSize,
        0,
        CODECHAL_NUM_MEDIA_STATES));

    if (m_basicFeature->m_ref.DysRefFrameFlags() == DYS_REF_NONE && m_resVdencPakObjCmdStreamOutBuffer != nullptr)
    {
        ENCODE_CHK_STATUS_RETURN(debugInterface->DumpBuffer(
            m_resVdencPakObjCmdStreamOutBuffer,
            CodechalDbgAttr::attrPakObjStreamout,
            bufferPassName.data(),
            m_basicFeature->m_mbCodeSize + 8 * CODECHAL_CACHELINE_SIZE,
            0,
            CODECHAL_NUM_MEDIA_STATES));
    }

    MOS_RESOURCE *mbCodedBuffer = m_basicFeature->m_trackedBuf->GetBuffer(
        BufferType::mbCodedBuffer, currRefList.ucScalingIdx);
    if (mbCodedBuffer != nullptr)
    {
        ENCODE_CHK_STATUS_RETURN(debugInterface->DumpBuffer(
            mbCodedBuffer,
            CodechalDbgAttr::attrVdencOutput,
            (bufferPassName + "_MbCode").data(),
            m_basicFeature->m_mbCodeSize + 8 * CODECHAL_CACHELINE_SIZE,
            0,
            CODECHAL_NUM_MEDIA_STATES));
    }

    PMOS_RESOURCE frameStatStreamOutBuffer = m_basicFeature->m_recycleBuf->GetBuffer(FrameStatStreamOutBuffer, 0);
    ENCODE_CHK_NULL_RETURN(frameStatStreamOutBuffer);
    ENCODE_CHK_STATUS_RETURN(debugInterface->DumpBuffer(
        frameStatStreamOutBuffer,
        CodechalDbgAttr::attrFrameState,
        bufferPassName.data(),
        frameStatStreamOutBuffer->iSize,
        0,
        CODECHAL_NUM_MEDIA_STATES));

    auto          streamInBufferSize = (MOS_ALIGN_CEIL(m_basicFeature->m_frameWidth, 64) / 32) * (MOS_ALIGN_CEIL(m_basicFeature->m_frameHeight, 64) / 32) * CODECHAL_CACHELINE_SIZE;
    PMOS_RESOURCE streamInBuffer     = m_basicFeature->m_recycleBuf->GetBuffer(RecycleResId::StreamInBuffer, m_basicFeature->m_frameNum);
    if (streamInBuffer != nullptr)
    {
        ENCODE_CHK_STATUS_RETURN(debugInterface->DumpBuffer(
            streamInBuffer,
            CodechalDbgAttr::attrStreamIn,
            bufferPassName.data(),
            streamInBufferSize,
            0,
            CODECHAL_NUM_MEDIA_STATES));
    }

    ENCODE_CHK_STATUS_RETURN(debugInterface->DumpBltOutput(
        &currRefList.sRefReconBuffer,
        CodechalDbgAttr::attrDecodeBltOutput));
    ENCODE_CHK_STATUS_RETURN(debugInterface->DumpYUVSurface(
        &currRefList.sRefReconBuffer,
        CodechalDbgAttr::attrReconstructedSurface,
        (surfacePassName + "_ReconSurf").data()));

    ENCODE_CHK_STATUS_RETURN(debugInterface->DumpBltOutput(
        &currRefList.sRefRawBuffer,
        CodechalDbgAttr::attrDecodeBltOutput));
    ENCODE_CHK_STATUS_RETURN(debugInterface->DumpYUVSurface(
        &currRefList.sRefRawBuffer,
        CodechalDbgAttr::attrEncodeRawInputSurface,
        (surfacePassName + "_SrcSurf").data()));

    ENCODE_CHK_STATUS_RETURN(m_basicFeature->m_ref.DumpInput(m_pipeline));
#endif

    return MOS_STATUS_SUCCESS;
}

#if USE_CODECHAL_DEBUG_TOOL

MOS_STATUS Vp9VdencPkt::DumpResources(
    EncodeStatusMfx *       encodeStatusMfx,
    EncodeStatusReportData *statusReportData)
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_NULL_RETURN(encodeStatusMfx);
    ENCODE_CHK_NULL_RETURN(statusReportData);
    ENCODE_CHK_NULL_RETURN(m_pipeline);
    ENCODE_CHK_NULL_RETURN(m_statusReport);
    ENCODE_CHK_NULL_RETURN(m_basicFeature);
    ENCODE_CHK_NULL_RETURN(m_basicFeature->m_trackedBuf);

    CodechalDebugInterface *debugInterface = m_pipeline->GetStatusReportDebugInterface();
    ENCODE_CHK_NULL_RETURN(debugInterface);

    CODEC_REF_LIST currRefList = *((CODEC_REF_LIST *)statusReportData->currRefList);
    currRefList.RefPic         = statusReportData->currOriginalPic;

    debugInterface->m_currPic            = statusReportData->currOriginalPic;
    debugInterface->m_bufferDumpFrameNum = m_statusReport->GetReportedCount() + 1;  // +1 for debug purpose with legacy
    debugInterface->m_frameType          = encodeStatusMfx->pictureCodingType;

    ENCODE_CHK_STATUS_RETURN(debugInterface->DumpBuffer(
        &currRefList.resBitstreamBuffer,
        CodechalDbgAttr::attrBitstream,
        "_PAK",
        statusReportData->bitstreamSize,
        0,
        CODECHAL_NUM_MEDIA_STATES));

    ENCODE_CHK_STATUS_RETURN(debugInterface->DumpData(
        statusReportData,
        sizeof(EncodeStatusReportData),
        CodechalDbgAttr::attrStatusReport,
        "EncodeStatusReport_Buffer"));

    ENCODE_CHK_STATUS_RETURN(debugInterface->DumpEncodeStatusReport(
        statusReportData));

    PMOS_RESOURCE frameStatStreamOutBuffer = m_basicFeature->m_recycleBuf->GetBuffer(FrameStatStreamOutBuffer, 0);
    ENCODE_CHK_NULL_RETURN(frameStatStreamOutBuffer);
    ENCODE_CHK_STATUS_RETURN(debugInterface->DumpBuffer(
        frameStatStreamOutBuffer,
        CodechalDbgAttr::attrFrameState,
        "FrameStatus",
        frameStatStreamOutBuffer->iSize,
        0,
        CODECHAL_NUM_MEDIA_STATES));

    MOS_SURFACE *ds4xSurface = m_basicFeature->m_trackedBuf->GetSurface(
        BufferType::ds4xSurface, currRefList.ucScalingIdx);
    if (ds4xSurface != nullptr)
    {
        ENCODE_CHK_STATUS_RETURN(debugInterface->DumpYUVSurface(
            ds4xSurface,
            CodechalDbgAttr::attrReconstructedSurface,
            "4xScaledSurf"));
    }

    MOS_SURFACE *ds8xSurface = m_basicFeature->m_trackedBuf->GetSurface(
        BufferType::ds8xSurface, currRefList.ucScalingIdx);
    if (ds8xSurface != nullptr)
    {
        ENCODE_CHK_STATUS_RETURN(debugInterface->DumpYUVSurface(
            ds8xSurface,
            CodechalDbgAttr::attrReconstructedSurface,
            "8xScaledSurf"));
    }

    MOS_RESOURCE *mbCodedBuffer = m_basicFeature->m_trackedBuf->GetBuffer(
        BufferType::mbCodedBuffer, currRefList.ucScalingIdx);
    if (mbCodedBuffer != nullptr)
    {
        ENCODE_CHK_STATUS_RETURN(debugInterface->DumpBuffer(
            mbCodedBuffer,
            CodechalDbgAttr::attrVdencOutput,
            "_MbCode",
            m_basicFeature->m_mbCodeSize + 8 * CODECHAL_CACHELINE_SIZE,
            0,
            CODECHAL_NUM_MEDIA_STATES));
    }

    auto          streamInBufferSize = (MOS_ALIGN_CEIL(m_basicFeature->m_frameWidth, 64) / 32) * (MOS_ALIGN_CEIL(m_basicFeature->m_frameHeight, 64) / 32) * CODECHAL_CACHELINE_SIZE;
    PMOS_RESOURCE streamInBuffer     = m_basicFeature->m_recycleBuf->GetBuffer(RecycleResId::StreamInBuffer, m_basicFeature->m_frameNum);
    if (streamInBuffer != nullptr)
    {
        ENCODE_CHK_STATUS_RETURN(debugInterface->DumpBuffer(
            streamInBuffer,
            CodechalDbgAttr::attrStreamIn,
            "StreamIn",
            streamInBufferSize,
            0,
            CODECHAL_NUM_MEDIA_STATES));
    }

    ENCODE_CHK_STATUS_RETURN(debugInterface->DumpBltOutput(
        &currRefList.sRefReconBuffer,
        CodechalDbgAttr::attrDecodeBltOutput));
    ENCODE_CHK_STATUS_RETURN(debugInterface->DumpYUVSurface(
        &currRefList.sRefReconBuffer,
        CodechalDbgAttr::attrReconstructedSurface,
        "ReconSurf"));

    ENCODE_CHK_STATUS_RETURN(debugInterface->DumpBltOutput(
        &currRefList.sRefRawBuffer,
        CodechalDbgAttr::attrDecodeBltOutput));
    ENCODE_CHK_STATUS_RETURN(debugInterface->DumpYUVSurface(
        &currRefList.sRefRawBuffer,
        CodechalDbgAttr::attrEncodeRawInputSurface,
        "SrcSurf"));

    return MOS_STATUS_SUCCESS;
}

#endif

static bool MmcEnabled(MOS_MEMCOMP_STATE state)
{
    return state == MOS_MEMCOMP_RC || state == MOS_MEMCOMP_MC;
}

static bool MmcRcEnabled(MOS_MEMCOMP_STATE state)
{
    return state == MOS_MEMCOMP_RC;
}

MHW_SETPAR_DECL_SRC(HCP_SURFACE_STATE, Vp9VdencPkt)
{
    ENCODE_FUNC_CALL();

    params.surfaceStateId = m_curHcpSurfStateId;

    auto surfParams = m_surfacesParams[m_curHcpSurfStateId];

    PMOS_SURFACE psSurface            = surfParams.psSurface;
    uint32_t     reconSurfHeight      = surfParams.dwReconSurfHeight;
    uint8_t      chromaType           = surfParams.ChromaType;
    uint8_t      ucBitDepthLumaMinus8 = surfParams.ucBitDepthLumaMinus8;

    ENCODE_CHK_NULL_RETURN(psSurface);

    params.surfacePitchMinus1 = psSurface->dwPitch - 1;

    /* Handling of reconstructed surface is different for Y410 & AYUV formats */
    if ((params.surfaceStateId != CODECHAL_HCP_SRC_SURFACE_ID) &&
        (psSurface->Format == Format_Y410))
        params.surfacePitchMinus1 = psSurface->dwPitch / 2 - 1;

    if ((params.surfaceStateId != CODECHAL_HCP_SRC_SURFACE_ID) &&
        (psSurface->Format == Format_AYUV))
        params.surfacePitchMinus1 = psSurface->dwPitch / 4 - 1;

    bool surf10bit = (psSurface->Format == Format_P010) ||
                     (psSurface->Format == Format_P210) ||
                     (psSurface->Format == Format_Y210) ||
                     (psSurface->Format == Format_Y410) ||
                     (psSurface->Format == Format_R10G10B10A2) ||
                     (psSurface->Format == Format_B10G10R10A2) ||
                     (psSurface->Format == Format_P016) ||
                     (psSurface->Format == Format_Y216);

    if (chromaType == HCP_CHROMA_FORMAT_YUV422)
    {
        if (ucBitDepthLumaMinus8 > 0)
        {
            if (params.surfaceStateId == CODECHAL_HCP_SRC_SURFACE_ID)
            {
                params.surfaceFormat = hcp::SURFACE_FORMAT::SURFACE_FORMAT_Y216VARIANT;

                if (surf10bit)
                {
                    params.surfaceFormat = hcp::SURFACE_FORMAT::SURFACE_FORMAT_Y216Y210FORMAT;
                }
            }
        }
        else
        {
            params.surfaceFormat = hcp::SURFACE_FORMAT::SURFACE_FORMAT_YUY2VARIANT;

            if (params.surfaceStateId == CODECHAL_HCP_SRC_SURFACE_ID)
            {
                params.surfaceFormat = hcp::SURFACE_FORMAT::SURFACE_FORMAT_YUY2FORMAT;
            }
        }
    }
    else if (chromaType == HCP_CHROMA_FORMAT_YUV444)
    {
        if (ucBitDepthLumaMinus8 == 0)
        {
            params.surfaceFormat = hcp::SURFACE_FORMAT::SURFACE_FORMAT_AYUV4444VARIANT;

            if (params.surfaceStateId == CODECHAL_HCP_SRC_SURFACE_ID)
            {
                params.surfaceFormat = hcp::SURFACE_FORMAT::SURFACE_FORMAT_AYUV4444FORMAT;
            }
        }
        else if (ucBitDepthLumaMinus8 <= 2)
        {
            if (params.surfaceStateId == CODECHAL_HCP_SRC_SURFACE_ID)
            {
                params.surfaceFormat = hcp::SURFACE_FORMAT::SURFACE_FORMAT_AYUV4444FORMAT;

                if (surf10bit)
                {
                    params.surfaceFormat = hcp::SURFACE_FORMAT::SURFACE_FORMAT_Y410FORMAT;
                }
            }
            else
            {
                params.surfaceFormat = hcp::SURFACE_FORMAT::SURFACE_FORMAT_Y416VARIANT;
            }
        }
        else
        {
            params.surfaceFormat = hcp::SURFACE_FORMAT::SURFACE_FORMAT_Y416FORMAT;
        }
    }
    else  //chromaType == HCP_CHROMA_FORMAT_YUV420
    {
        if (ucBitDepthLumaMinus8 > 0)
        {
            if (params.surfaceStateId == CODECHAL_HCP_SRC_SURFACE_ID)
            {
                params.surfaceFormat = hcp::SURFACE_FORMAT::SURFACE_FORMAT_PLANAR4208;

                if (surf10bit)
                {
                    params.surfaceFormat = hcp::SURFACE_FORMAT::SURFACE_FORMAT_P010;
                }
            }
            else
            {
                params.surfaceFormat = hcp::SURFACE_FORMAT::SURFACE_FORMAT_P010VARIANT;
            }
        }
        else
        {
            params.surfaceFormat = hcp::SURFACE_FORMAT::SURFACE_FORMAT_PLANAR4208;
        }
    }

    params.yOffsetForUCbInPixel = params.yOffsetForVCr =
        (uint16_t)((psSurface->UPlaneOffset.iSurfaceOffset - psSurface->dwOffset) / psSurface->dwPitch + psSurface->RenderOffset.YUV.U.YOffset);

    //Set U/V offsets for Variant surfaces
    if (params.surfaceFormat == hcp::SURFACE_FORMAT::SURFACE_FORMAT_Y416VARIANT ||
        params.surfaceFormat == hcp::SURFACE_FORMAT::SURFACE_FORMAT_AYUV4444VARIANT)
    {
        params.yOffsetForUCbInPixel = (uint16_t)reconSurfHeight;
        params.yOffsetForVCr        = (uint16_t)reconSurfHeight << 1;
    }
    else if (params.surfaceFormat == hcp::SURFACE_FORMAT::SURFACE_FORMAT_Y216VARIANT ||
             params.surfaceFormat == hcp::SURFACE_FORMAT::SURFACE_FORMAT_YUY2VARIANT)
    {
        params.yOffsetForUCbInPixel = params.yOffsetForVCr = (uint16_t)reconSurfHeight;
    }

    params.mmcState            = surfParams.mmcState;
    params.dwCompressionFormat = surfParams.dwCompressionFormat;
    params.mmcSkipMask         = surfParams.mmcSkipMask;
    params.refsMmcEnable       = (params.surfaceStateId != CODECHAL_HCP_DECODED_SURFACE_ID && MmcEnabled(surfParams.mmcState) )? 0xff : 0;
    params.refsMmcType         = (params.surfaceStateId != CODECHAL_HCP_DECODED_SURFACE_ID && MmcRcEnabled(surfParams.mmcState) ) ? 0xff : 0;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9VdencPkt::Add_HCP_SURFACE_STATE(PMOS_COMMAND_BUFFER &cmdBuffer)
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_NULL_RETURN(cmdBuffer);

    SETPAR_AND_ADDCMD(HCP_SURFACE_STATE, m_hcpInterfaceNew, cmdBuffer);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9VdencPkt::AddAllCmds_HCP_SURFACE_STATE(PMOS_COMMAND_BUFFER cmdBuffer)
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_NULL_RETURN(cmdBuffer);

    // Set HCP_SURFACE_STATE values
    SetHcpSurfacesParams(m_surfacesParams);
    SetHcpSurfaceMMCState();

    m_curHcpSurfStateId = CODECHAL_HCP_DECODED_SURFACE_ID;
    Add_HCP_SURFACE_STATE(cmdBuffer);

    m_curHcpSurfStateId = CODECHAL_HCP_SRC_SURFACE_ID;
    Add_HCP_SURFACE_STATE(cmdBuffer);
 
    auto waTable = m_osInterface == nullptr ? nullptr : m_osInterface->pfnGetWaTable(m_osInterface);
    if (waTable)
    {
        if (MEDIA_IS_WA(waTable, Wa_Vp9UnalignedHeight))
        {
            uint32_t real_height    = m_basicFeature->m_oriFrameHeight;
            uint32_t aligned_height = MOS_ALIGN_CEIL(real_height, CODEC_VP9_MIN_BLOCK_HEIGHT);

            fill_pad_with_value(m_basicFeature->m_rawSurfaceToPak, real_height, aligned_height);
        }
    }

    // Last reference picture
    if (m_surfacesParams[CODECHAL_HCP_LAST_SURFACE_ID].psSurface)
    {
        m_curHcpSurfStateId = CODECHAL_HCP_LAST_SURFACE_ID;
        Add_HCP_SURFACE_STATE(cmdBuffer);
    }

    // Golden reference picture
    if (m_surfacesParams[CODECHAL_HCP_GOLDEN_SURFACE_ID].psSurface)
    {
        m_curHcpSurfStateId = CODECHAL_HCP_GOLDEN_SURFACE_ID;
        Add_HCP_SURFACE_STATE(cmdBuffer);
    }

    // Alt reference picture
    if (m_surfacesParams[CODECHAL_HCP_ALTREF_SURFACE_ID].psSurface)
    {
        m_curHcpSurfStateId = CODECHAL_HCP_ALTREF_SURFACE_ID;
        Add_HCP_SURFACE_STATE(cmdBuffer);
    }

    return MOS_STATUS_SUCCESS;
}

MHW_VDBOX_HCP_MULTI_ENGINE_MODE Vp9VdencPkt::getMultiEngineMode() const
{
    MHW_VDBOX_HCP_MULTI_ENGINE_MODE mode = MHW_VDBOX_HCP_MULTI_ENGINE_MODE_FE_LEGACY;

    if (m_basicFeature->m_scalableMode)
    {
        // Running in the multiple VDBOX mode
        if (m_pipeline->IsFirstPipe())
        {
            mode = MHW_VDBOX_HCP_MULTI_ENGINE_MODE_LEFT;
        }
        else if (m_pipeline->IsLastPipe())
        {
            mode = MHW_VDBOX_HCP_MULTI_ENGINE_MODE_RIGHT;
        }
        else
        {
            mode = MHW_VDBOX_HCP_MULTI_ENGINE_MODE_MIDDLE;
        }
    }

    return mode;
}

MHW_VDBOX_HCP_PIPE_WORK_MODE Vp9VdencPkt::getPipeWorkMode() const
{
    if (m_basicFeature->m_scalableMode)
    {
        return MHW_VDBOX_HCP_PIPE_WORK_MODE_CODEC_BE;
    }

    return MHW_VDBOX_HCP_PIPE_WORK_MODE_LEGACY;
}

MHW_SETPAR_DECL_SRC(HCP_PIPE_MODE_SELECT, Vp9VdencPkt)
{
    ENCODE_FUNC_CALL();

    params.codecStandardSelect = CODEC_STANDARD_SELECT_VP9;
    params.codecSelect         = CODEC_SELECT_ENCODE;

    auto brcFeature = dynamic_cast<Vp9EncodeBrc *>(m_featureManager->GetFeature(Vp9FeatureIDs::vp9BrcFeature));
    ENCODE_CHK_NULL_RETURN(brcFeature);

    params.bStreamOutEnabled = brcFeature->IsVdencBrcEnabled();
    params.bVdencEnabled     = true;

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(HCP_IND_OBJ_BASE_ADDR_STATE, Vp9VdencPkt)
{
    ENCODE_FUNC_CALL();

    params.presPakBaseObjectBuffer = &m_basicFeature->m_resBitstreamBuffer;
    params.dwPakBaseObjectSize     = m_basicFeature->m_bitstreamUpperBound;

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(HCP_PIPE_BUF_ADDR_STATE, Vp9VdencPkt)
{
    ENCODE_FUNC_CALL();

    params.psPreDeblockSurface  = &m_basicFeature->m_reconSurface;
    params.psPostDeblockSurface = &m_basicFeature->m_reconSurface;
    params.psRawSurface         = m_basicFeature->m_rawSurfaceToPak;
    params.presCurMvTempBuffer  = m_basicFeature->m_resMvTemporalBuffer;

    params.presVp9SegmentIdBuffer              = m_basicFeature->m_resSegmentIdBuffer;
    params.presHvdTileRowStoreBuffer           = const_cast<PMOS_RESOURCE>(&m_resHvcTileRowStoreBuffer);
    params.ps4xDsSurface                       = m_basicFeature->m_4xDSSurface;
    params.ps8xDsSurface                       = m_basicFeature->m_8xDSSurface;
    params.presVdencIntraRowStoreScratchBuffer = const_cast<PMOS_RESOURCE>(&m_resVdencIntraRowStoreScratchBuffer);
    params.presSseSrcPixelRowStoreBuffer       = const_cast<PMOS_RESOURCE>(&m_resSseSrcPixelRowStoreBuffer);
    params.presVdencStreamInBuffer             = m_basicFeature->m_recycleBuf->GetBuffer(RecycleResId::StreamInBuffer, m_basicFeature->m_frameNum);
    params.presSegmentMapStreamOut             = const_cast<PMOS_RESOURCE>(&m_resVdencSegmentMapStreamOut);
    params.presPakCuLevelStreamoutBuffer       = const_cast<PMOS_RESOURCE>(&m_resPakcuLevelStreamoutData);
    params.presVdencPakObjCmdStreamOutBuffer   = m_resVdencPakObjCmdStreamOutBuffer;

    if ((m_basicFeature->m_ref.DysRefFrameFlags() != DYS_REF_NONE) && !m_vdencPakObjCmdStreamOutEnabled)
    {
        params.presVdencPakObjCmdStreamOutBuffer = nullptr;
    }

    m_basicFeature->m_ref.MHW_SETPAR_F(HCP_PIPE_BUF_ADDR_STATE)(params);

#ifdef _MMC_SUPPORTED
    ENCODE_CHK_NULL_RETURN(m_mmcState);

    if (m_mmcState->IsMmcEnabled())
    {
        ENCODE_CHK_STATUS_RETURN(m_mmcState->GetSurfaceMmcState(&m_basicFeature->m_reconSurface, &params.PreDeblockSurfMmcState));
        params.PostDeblockSurfMmcState = params.PreDeblockSurfMmcState;
        ENCODE_CHK_STATUS_RETURN(m_mmcState->GetSurfaceMmcState(&m_basicFeature->m_rawSurface, &params.RawSurfMmcState));
    }
    else
    {
        params.PreDeblockSurfMmcState  = MOS_MEMCOMP_DISABLED;
        params.PostDeblockSurfMmcState = MOS_MEMCOMP_DISABLED;
        params.RawSurfMmcState         = MOS_MEMCOMP_DISABLED;
    }

    CODECHAL_DEBUG_TOOL(
        m_basicFeature->m_reconSurface.MmcState = params.PreDeblockSurfMmcState;)
#endif

    return MOS_STATUS_SUCCESS;
}

static inline uint32_t GetHwTileType(MOS_TILE_TYPE tileType, MOS_TILE_MODE_GMM tileModeGMM, bool gmmTileEnabled)
{
    uint32_t tileMode = 0;

    if (gmmTileEnabled)
    {
        return tileModeGMM;
    }

    switch (tileType)
    {
    case MOS_TILE_LINEAR:
        tileMode = 0;
        break;
    case MOS_TILE_YS:
        tileMode = 1;
        break;
    case MOS_TILE_X:
        tileMode = 2;
        break;
    default:
        tileMode = 3;
        break;
    }

    return tileMode;
}

MHW_SETPAR_DECL_SRC(VDENC_REF_SURFACE_STATE, Vp9VdencPkt)
{
    ENCODE_FUNC_CALL();

    auto surfParams = m_surfacesParams[m_curVdencSurfStateId].psSurface;

    // DW2..5

    params.pitch       = surfParams->dwPitch;
    params.tileType    = surfParams->TileType;
    params.tileModeGmm = surfParams->TileModeGMM;
    params.format      = surfParams->Format;
    params.gmmTileEn   = surfParams->bGMMTileEnabled;
    params.uOffset     = surfParams->YoffsetForUplane;
    params.vOffset     = surfParams->YoffsetForVplane;
    params.height      = surfParams->dwHeight;
    params.width       = surfParams->dwWidth;
    uint32_t tileMode  = GetHwTileType(params.tileType, params.tileModeGmm, params.gmmTileEn);

    if (surfParams->Format == Format_Y410 || surfParams->Format == Format_444P || surfParams->Format == Format_AYUV)
    {
        if (surfParams->Format == Format_Y410)
        {
            params.pitch = surfParams->dwPitch / 2;
        }
        else
        {
            params.pitch = surfParams->dwPitch / 4;
        }
        params.uOffset = m_surfacesParams[m_curVdencSurfStateId].dwReconSurfHeight;
        params.vOffset = m_surfacesParams[m_curVdencSurfStateId].dwReconSurfHeight << 1;
    }
    else if (surfParams->Format == Format_Y216 || surfParams->Format == Format_YUY2 || surfParams->Format == Format_YUYV)
    {
        params.uOffset = m_surfacesParams[m_curVdencSurfStateId].dwReconSurfHeight;
        params.vOffset = m_surfacesParams[m_curVdencSurfStateId].dwReconSurfHeight;
    }

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(VDENC_PIPE_MODE_SELECT, Vp9VdencPkt)
{
    ENCODE_FUNC_CALL();

    // DW1

    params.standardSelect                           = CODECHAL_VP9;

    MHW_VDBOX_HCP_MULTI_ENGINE_MODE multiEngineMode = getMultiEngineMode();
    params.scalabilityMode                          = !(multiEngineMode == MHW_VDBOX_HCP_MULTI_ENGINE_MODE_FE_LEGACY);

    params.frameStatisticsStreamOut                 = true;
    params.pakObjCmdStreamOut                       = m_vdencPakObjCmdStreamOutEnabled;

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(VDENC_HEVC_VP9_TILE_SLICE_STATE, Vp9VdencPkt)
{
    ENCODE_FUNC_CALL();

    uint32_t dwNumberOfPipes = 0;
    switch (m_pipeline->GetPipeNum())
    {
    case 0:
    case 1:
        dwNumberOfPipes = VDENC_PIPE_SINGLE_PIPE;
        break;
    case 2:
        dwNumberOfPipes = VDENC_PIPE_TWO_PIPE;
        break;
    case 4:
        dwNumberOfPipes = VDENC_PIPE_FOUR_PIPE;
        break;
    default:
        dwNumberOfPipes = VDENC_PIPE_INVALID;
        ENCODE_ASSERT(false);
        break;
    }

    params.numPipe = dwNumberOfPipes;

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(VDENC_PIPE_BUF_ADDR_STATE, Vp9VdencPkt)
{
    ENCODE_FUNC_CALL();

    params.compressionFormatRaw   = m_surfacesParams[CODECHAL_HCP_SRC_SURFACE_ID].dwCompressionFormat;
    params.compressionFormatRecon = m_surfacesParams[CODECHAL_HCP_DECODED_SURFACE_ID].dwCompressionFormat;

    params.intraRowStoreScratchBuffer = const_cast<PMOS_RESOURCE>(&m_resVdencIntraRowStoreScratchBuffer);
    params.segmentMapStreamOutBuffer  = const_cast<PMOS_RESOURCE>(&m_resVdencSegmentMapStreamOut);
    params.segmentMapStreamInBuffer   = params.segmentMapStreamOutBuffer;
    params.pakObjCmdStreamOutBuffer   = m_resVdencPakObjCmdStreamOutBuffer;

    if (m_basicFeature->m_ref.DysRefFrameFlags() != DYS_REF_NONE && !m_resVdencPakObjCmdStreamOutBuffer)
    {
        params.pakObjCmdStreamOutBuffer = nullptr;
    }

    params.tileRowStoreBuffer               = const_cast<PMOS_RESOURCE>(&m_vdencTileRowStoreBuffer);
    params.cumulativeCuCountStreamOutBuffer = const_cast<PMOS_RESOURCE>(&m_vdencCumulativeCuCountStreamoutSurface);

    m_basicFeature->m_ref.MHW_SETPAR_F(VDENC_PIPE_BUF_ADDR_STATE)(params);

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(VDENC_CONTROL_STATE, Vp9VdencPkt)
{
    ENCODE_FUNC_CALL();

    params.vdencInitialization = true;

    return MOS_STATUS_SUCCESS;
}


MHW_SETPAR_DECL_SRC(VD_PIPELINE_FLUSH, Vp9VdencPkt)
{
    ENCODE_FUNC_CALL();

    ENCODE_CHK_NULL_RETURN(m_basicFeature);

    params.waitDoneMFX            = true;
    params.waitDoneVDCmdMsgParser = true;

    switch (m_flushCmd)
    {
    case waitVp9:
        params.waitDoneHEVC = true;
        params.flushHEVC    = true;
        break;
    case waitVdenc:
        if (m_basicFeature->m_lastPicInStream || m_basicFeature->m_lastPicInSeq)
        {
            params.waitDoneMFX = false;
        }

        params.waitDoneVDENC = true;
        params.flushVDENC    = true;
        params.flushHEVC     = true;
        break;
    case waitVp9Vdenc:
        params.waitDoneVDENC = true;
        params.flushVDENC    = true;
        params.flushHEVC     = true;
        break;
    }

    return MOS_STATUS_SUCCESS;
}

}  // namespace encode
