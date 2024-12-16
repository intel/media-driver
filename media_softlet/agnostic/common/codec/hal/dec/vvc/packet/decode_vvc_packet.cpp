/*
* Copyright (c) 2021-2024, Intel Corporation
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
//! \file     decode_vvc_packet.cpp
//! \brief    Defines the interface for vvc decode packet
//!
#include "decode_vvc_packet.h"
#include "decode_status_report_defs.h"
#include "decode_predication_packet.h"
#include "decode_marker_packet.h"
#include "decode_utils.h"
#include "decode_status_report_defs.h"
#include "mos_solo_generic.h"
#include "decode_resource_auto_lock.h"
#include "hal_oca_interface_next.h"

namespace decode {

MOS_STATUS VvcDecodePkt::Init()
{
    DECODE_FUNC_CALL();

    DECODE_CHK_NULL(m_statusReport);
    DECODE_CHK_NULL(m_featureManager);
    DECODE_CHK_NULL(m_vvcPipeline);
    DECODE_CHK_NULL(m_osInterface);
    DECODE_CHK_NULL(m_vdencItf);

    DECODE_CHK_STATUS(CmdPacket::Init());

    CalculateVvcSliceLvlCmdSize();

    m_vvcBasicFeature = dynamic_cast<VvcBasicFeature*>(m_featureManager->GetFeature(FeatureIDs::basicFeature));
    DECODE_CHK_NULL(m_vvcBasicFeature);

    m_allocator = m_vvcPipeline->GetDecodeAllocator();
    DECODE_CHK_NULL(m_allocator);

    DECODE_CHK_STATUS(m_statusReport->RegistObserver(this));

    DecodeSubPacket* subPacket = m_vvcPipeline->GetSubPacket(DecodePacketId(m_vvcPipeline, vvcPictureSubPacketId));

    m_picturePkt = dynamic_cast<VvcDecodePicPkt*>(subPacket);
    DECODE_CHK_NULL(m_picturePkt);
    DECODE_CHK_STATUS(m_picturePkt->CalculateCommandSize(m_pictureStatesSize, m_picturePatchListSize));

    subPacket = m_vvcPipeline->GetSubPacket(DecodePacketId(m_vvcPipeline, vvcSliceSubPacketId));
    m_slicePkt = dynamic_cast<VvcDecodeSlicePkt*>(subPacket);
    DECODE_CHK_NULL(m_slicePkt);
    DECODE_CHK_STATUS(m_slicePkt->CalculateCommandSize(m_sliceStatesSize, m_slicePatchListSize));
    DECODE_CHK_STATUS(m_slicePkt->CalculateTileCommandSize(m_tileStateSize, m_tilePatchListSize));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VvcDecodePkt::Prepare()
{
    DECODE_FUNC_CALL();

    DECODE_CHK_NULL(m_vvcBasicFeature->m_vvcPicParams);
    m_vvcPicParams = m_vvcBasicFeature->m_vvcPicParams;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VvcDecodePkt::Destroy()
{
    m_statusReport->UnregistObserver(this);

    return MOS_STATUS_SUCCESS;
}

void VvcDecodePkt::SetPerfTag(CODECHAL_MODE mode, uint16_t picCodingType)
{
    DECODE_FUNC_CALL();

    uint16_t perfTag = ((mode << 4) & 0xF0) | (picCodingType & 0xF);
    m_osInterface->pfnIncPerfFrameID(m_osInterface);
    m_osInterface->pfnSetPerfTag(m_osInterface, perfTag);
    m_osInterface->pfnResetPerfBufferID(m_osInterface);
}

bool VvcDecodePkt::IsPrologRequired()
{
    //If long format mode, vvc packet will be first packet to execute, need send prolog
    //If not single task mode, vvc packet will submit separatly, also need send prolog
    if (!m_vvcBasicFeature->m_shortFormatInUse || !m_vvcPipeline->IsSingleTaskPhaseSupported())
    {
        return true;
    }

    return false;
}

MOS_STATUS VvcDecodePkt::AddForceWakeup(MOS_COMMAND_BUFFER& cmdBuffer)
{
    DECODE_FUNC_CALL();

    auto &par = m_miItf->GETPAR_MI_FORCE_WAKEUP();
    MOS_ZeroMemory(&par, sizeof(mhw::mi::MI_FORCE_WAKEUP_PAR));
    par.bMFXPowerWellControl      = false;
    par.bMFXPowerWellControlMask  = true;
    par.bHEVCPowerWellControl     = true;
    par.bHEVCPowerWellControlMask = true;

    DECODE_CHK_STATUS(m_miItf->ADDCMD_MI_FORCE_WAKEUP(&cmdBuffer));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VvcDecodePkt::SendPrologWithFrameTracking(MOS_COMMAND_BUFFER& cmdBuffer, bool frameTrackingRequested)
{
    DecodeSubPacket* subPacket = m_vvcPipeline->GetSubPacket(DecodePacketId(m_vvcPipeline, markerSubPacketId));
    DecodeMarkerPkt *makerPacket = dynamic_cast<DecodeMarkerPkt*>(subPacket);
    DECODE_CHK_NULL(makerPacket);
    DECODE_CHK_STATUS(makerPacket->Execute(cmdBuffer));

#ifdef _MMC_SUPPORTED
    m_mmcState = m_vvcPipeline->GetMmcState();
    bool isMmcEnabled = (m_mmcState != nullptr && m_mmcState->IsMmcEnabled());
#endif

    MHW_GENERIC_PROLOG_PARAMS  genericPrologParams;
    MOS_ZeroMemory(&genericPrologParams, sizeof(genericPrologParams));
    genericPrologParams.pOsInterface = m_osInterface;
    genericPrologParams.pvMiInterface = nullptr;

#ifdef _MMC_SUPPORTED
    genericPrologParams.bMmcEnabled = isMmcEnabled;
#endif

    DECODE_CHK_STATUS(Mhw_SendGenericPrologCmdNext(&cmdBuffer, &genericPrologParams, m_miItf));

    subPacket = m_vvcPipeline->GetSubPacket(DecodePacketId(m_vvcPipeline, predicationSubPacketId));
    DecodePredicationPkt *predicationPacket = dynamic_cast<DecodePredicationPkt*>(subPacket);
    DECODE_CHK_NULL(predicationPacket);
    DECODE_CHK_STATUS(predicationPacket->Execute(cmdBuffer));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VvcDecodePkt::VdPipelineFlush(MOS_COMMAND_BUFFER & cmdBuffer)
{
    DECODE_FUNC_CALL();

    auto &par = m_vdencItf->MHW_GETPAR_F(VD_PIPELINE_FLUSH)();
    par       = {};

    par.waitDoneVDCmdMsgParser     = true;
#ifdef _MEDIA_RESERVED
    par.vvcpPipelineCommandFlush   = true;
    par.vvcpPipelineDone           = true;
#endif
    DECODE_CHK_STATUS(m_vdencItf->MHW_ADDCMD_F(VD_PIPELINE_FLUSH)(&cmdBuffer));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VvcDecodePkt::MiFlush(MOS_COMMAND_BUFFER & cmdBuffer)
{
    DECODE_FUNC_CALL();

    auto &par = m_miItf->GETPAR_MI_FLUSH_DW();
    MOS_ZeroMemory(&par, sizeof(mhw::mi::MI_FLUSH_DW_PAR));
    DECODE_CHK_STATUS(m_miItf->ADDCMD_MI_FLUSH_DW(&cmdBuffer));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VvcDecodePkt::Completed(void *mfxStatus, void *rcsStatus, void *statusReport)
{
    DECODE_FUNC_CALL();

    DECODE_CHK_NULL(mfxStatus);
    DECODE_CHK_NULL(statusReport);
    DECODE_CHK_NULL(m_vvcBasicFeature);

    DecodeStatusMfx *       decodeStatusMfx  = (DecodeStatusMfx *)mfxStatus;
    DecodeStatusReportData *statusReportData = (DecodeStatusReportData *)statusReport;
    DECODE_VERBOSEMESSAGE("Current Frame Index = %d", statusReportData->currDecodedPic.FrameIdx);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VvcDecodePkt::CalculateCommandSize(uint32_t &commandBufferSize, uint32_t &requestedPatchListSize)
{
    commandBufferSize = CalculateCommandBufferSize();
    requestedPatchListSize = CalculatePatchListSize();

    // Picture-level cmd / patch list size is added only one time
    if (m_picCmdSizeCalculated == false)
    {
        commandBufferSize += CalculatePicCommandSize();
        requestedPatchListSize += CalculatePicPatchListSize();
        m_picCmdSizeCalculated = true;
    }

    return MOS_STATUS_SUCCESS;
}

uint32_t VvcDecodePkt::CalculateCommandBufferSize()
{
    // Cmd buffer size added per slice
    return m_sliceStatesSize;
}

uint32_t VvcDecodePkt::CalculatePatchListSize()
{
    // Patch list size added per slice
    if (!m_osInterface->bUsesPatchList)
    {
        return 0;
    }

    return m_slicePatchListSize;
}

uint32_t VvcDecodePkt::CalculatePicCommandSize()
{
    // add tile cmd size for each frame
    uint32_t tileCmdSize = (m_vvcBasicFeature->m_tileCols * m_vvcBasicFeature->m_tileRows) * m_tileStateSize;

    return tileCmdSize + m_pictureStatesSize + COMMAND_BUFFER_RESERVED_SPACE;
}

uint32_t VvcDecodePkt::CalculatePicPatchListSize()
{
    if (!m_osInterface->bUsesPatchList)
    {
        return 0;
    }
    // add tile patch list size for each frame
    uint32_t tilePatchListSize = (m_vvcBasicFeature->m_tileCols * m_vvcBasicFeature->m_tileRows) * m_tilePatchListSize;

    return tilePatchListSize + m_picturePatchListSize;
}

MOS_STATUS VvcDecodePkt::ReadVvcpStatus(MediaStatusReport* statusReport, MOS_COMMAND_BUFFER& cmdBuffer)
{
    DECODE_FUNC_CALL();

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    DECODE_CHK_NULL(statusReport);

    //TODO: VVC MMIO has new mechanism

    return eStatus;
}

MOS_STATUS VvcDecodePkt::StartStatusReport(uint32_t srType, MOS_COMMAND_BUFFER* cmdBuffer)
{
    DECODE_FUNC_CALL();
    DECODE_CHK_NULL(cmdBuffer);
    DECODE_CHK_STATUS(MediaPacket::StartStatusReportNext(srType, cmdBuffer));

    SetPerfTag(CODECHAL_MODE(CODECHAL_DECODE_MODE_VVCVLD), m_vvcBasicFeature->m_pictureCodingType);

    MediaPerfProfiler *perfProfiler = MediaPerfProfiler::Instance();
    DECODE_CHK_NULL(perfProfiler);
    DECODE_CHK_STATUS(perfProfiler->AddPerfCollectStartCmd(
        (void *)m_vvcPipeline, m_osInterface, m_miItf, cmdBuffer));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VvcDecodePkt::EndStatusReport(uint32_t srType, MOS_COMMAND_BUFFER* cmdBuffer)
{
    DECODE_FUNC_CALL();

    DECODE_CHK_NULL(cmdBuffer);

    DECODE_CHK_STATUS(ReadVvcpStatus(m_statusReport, *cmdBuffer));
    DECODE_CHK_STATUS(MediaPacket::EndStatusReportNext(srType, cmdBuffer));

    MediaPerfProfiler *perfProfiler = MediaPerfProfiler::Instance();
    DECODE_CHK_NULL(perfProfiler);
    DECODE_CHK_STATUS(perfProfiler->AddPerfCollectEndCmd(
        (void *)m_vvcPipeline, m_osInterface, m_miItf, cmdBuffer));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VvcDecodePkt::InitPicLevelCmdBuffer(MHW_BATCH_BUFFER &batchBuffer, uint8_t *batchBufBase)
{
    DECODE_FUNC_CALL();

    auto &cmdBuffer = m_picCmdBuffer;
    MOS_ZeroMemory(&cmdBuffer, sizeof(MOS_COMMAND_BUFFER));
    cmdBuffer.pCmdBase   = (uint32_t*)batchBufBase;
    cmdBuffer.pCmdPtr    = cmdBuffer.pCmdBase;
    cmdBuffer.iRemaining = batchBuffer.iSize;
    cmdBuffer.OsResource = batchBuffer.OsResource;

    return MOS_STATUS_SUCCESS;
}

#if USE_CODECHAL_DEBUG_TOOL
MOS_STATUS VvcDecodePkt::DumpResources(
    DecodeStatusMfx *       decodeStatusMfx,
    DecodeStatusReportData *statusReportData)
{
    DECODE_FUNC_CALL();
    DECODE_CHK_NULL(decodeStatusMfx);
    DECODE_CHK_NULL(statusReportData);

    return MOS_STATUS_SUCCESS;
}
#endif

MOS_STATUS VvcDecodePkt::Submit(
    MOS_COMMAND_BUFFER* cmdBuffer,
    uint8_t packetPhase)
{
    DECODE_FUNC_CALL()

    PERF_UTILITY_AUTO(__FUNCTION__, PERF_DECODE, PERF_LEVEL_HAL);

    DECODE_CHK_NULL(cmdBuffer);
    DECODE_CHK_NULL(m_hwInterface);

    m_isFirstSliceInFrame = (m_vvcBasicFeature->m_curSlice == 0) ? true : false;
    m_isLastSliceInFrame = (m_vvcBasicFeature->m_curSlice == m_vvcBasicFeature->m_numSlices - 1) ? true : false;

    if (m_isFirstSliceInFrame)
    {
        DECODE_CHK_STATUS(m_miItf->SetWatchdogTimerThreshold(m_vvcBasicFeature->m_width, m_vvcBasicFeature->m_height, false));

        DECODE_CHK_STATUS(Mos_Solo_PreProcessDecode(m_osInterface, &m_vvcBasicFeature->m_destSurface));

        auto mmioRegisters = m_hwInterface->GetVdencInterfaceNext()->GetMmioRegisters(MHW_VDBOX_NODE_1);

        HalOcaInterfaceNext::On1stLevelBBStart(*cmdBuffer, (MOS_CONTEXT_HANDLE)m_osInterface->pOsContext, m_osInterface->CurrentGpuContextHandle, m_miItf, *mmioRegisters);
        HalOcaInterfaceNext::OnDispatch(*cmdBuffer, *m_osInterface, m_miItf, *m_miItf->GetMmioRegisters());
        if (m_vvcBasicFeature->m_shortFormatInUse)
        {
            m_tileLevelBB = m_vvcPipeline->GetTileLvlCmdBuffer();
        }
    }

    DECODE_CHK_STATUS(PackPictureLevelCmds(*cmdBuffer));

    if (!m_vvcBasicFeature->m_shortFormatInUse)
    {
        DECODE_CHK_STATUS(PackSliceLevelCmds(*cmdBuffer));
    }
    else
    {
        DECODE_CHK_STATUS(PackS2LSliceLevelCmds(*cmdBuffer));
    }

    if (m_isLastSliceInFrame)
    {
        HalOcaInterfaceNext::On1stLevelBBEnd(*cmdBuffer, *m_osInterface);
        m_picCmdSizeCalculated = false; // Reset flag for running next frame
    }

    if (m_isFirstSliceInFrame)
    {
        DECODE_CHK_STATUS(m_allocator->SyncOnResource(&m_vvcBasicFeature->m_resDataBuffer, false));
    }

    m_vvcBasicFeature->m_curSlice++; //Update current slice index

    //Set ReadyToExecute to true for the last slice of the frame
    Mos_Solo_SetReadyToExecute(m_osInterface, m_vvcBasicFeature->m_frameCompletedFlag);
    DECODE_CHK_STATUS(Mos_Solo_PostProcessDecode(m_osInterface, &m_vvcBasicFeature->m_destSurface));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VvcDecodePkt::PackPictureLevelCmds(MOS_COMMAND_BUFFER& cmdBuffer)
{
    DECODE_FUNC_CALL()

    PERF_UTILITY_AUTO(__FUNCTION__, PERF_DECODE, PERF_LEVEL_HAL);

    if (m_isFirstSliceInFrame)
    {
        if (IsPrologRequired())
        {
            DECODE_CHK_STATUS(AddForceWakeup(cmdBuffer));
            DECODE_CHK_STATUS(SendPrologWithFrameTracking(cmdBuffer, true));
        }
        DECODE_CHK_STATUS(StartStatusReport(statusReportMfx, &cmdBuffer));
    }

    DECODE_CHK_STATUS(m_picturePkt->InitVvcState(cmdBuffer));

    // For multi-slice case, picture level cmds are the same among different slices.
    // Put them into 2nd level BB to prepare them only once for the 1st slice of the frame to reduce SW latency.
    if (m_isFirstSliceInFrame)
    {
        m_picBatchBuf = m_picturePkt->GetPicLvlBB();

        if (m_picBatchBuf != nullptr)
        {
            ResourceAutoLock resLock(m_allocator, &m_picBatchBuf->OsResource);
            uint8_t *batchBufBase = (uint8_t *)resLock.LockResourceForWrite();
            DECODE_CHK_STATUS(InitPicLevelCmdBuffer(*m_picBatchBuf, batchBufBase));
            DECODE_CHK_STATUS(m_picturePkt->Execute(m_picCmdBuffer));
            DECODE_CHK_STATUS(m_miItf->AddMiBatchBufferEnd(&m_picCmdBuffer, nullptr));
        }
    }

    DECODE_CHK_STATUS(m_miItf->ADDCMD_MI_BATCH_BUFFER_START(&cmdBuffer, m_picBatchBuf));

    return MOS_STATUS_SUCCESS;
}

void VvcDecodePkt::CalculateVvcSliceLvlCmdSize()
{
    m_vvcpSliceCmdSize = m_vvcpItf->GETSIZE_VVCP_SLICE_STATE() +
                         m_vvcpItf->GETSIZE_VVCP_REF_IDX_STATE() * 2 +
                         m_vvcpItf->GETSIZE_VVCP_WEIGHTOFFSET_STATE() * 2 +
                         m_vvcpItf->GETSIZE_VVCP_BSD_OBJECT() +
                         m_miItf->GETSIZE_MI_BATCH_BUFFER_START();
}

MOS_STATUS VvcDecodePkt::PackS2LSliceLevelCmds(MOS_COMMAND_BUFFER &cmdBuffer)
{
    DECODE_FUNC_CALL()

    PERF_UTILITY_AUTO(__FUNCTION__, PERF_DECODE, PERF_LEVEL_HAL);
    uint16_t sliceIdx = m_vvcBasicFeature->m_curSlice;

    if (sliceIdx < m_vvcBasicFeature->m_numSlices)
    {
        auto buf = m_vvcPipeline->GetSliceLvlCmdBuffer();
        DECODE_CHK_NULL(buf);
        buf->dwOffset = m_vvcPipeline->GetSliceLvlBufSize() * sliceIdx;
        DECODE_CHK_STATUS(m_miItf->ADDCMD_MI_BATCH_BUFFER_START(&cmdBuffer, buf)); //Jump to Slice Batch Buffer
        DECODE_CHK_STATUS(VdMemoryFlush(cmdBuffer));
        DECODE_CHK_STATUS(VdPipelineFlush(cmdBuffer));
    }
    if (m_isLastSliceInFrame)
    {
        DECODE_CHK_STATUS(EnsureAllCommandsExecuted(cmdBuffer));
        DECODE_CHK_STATUS(EndStatusReport(statusReportMfx, &cmdBuffer));
        DECODE_CHK_STATUS(UpdateStatusReportNext(statusReportGlobalCount, &cmdBuffer));
    }

    CODECHAL_DEBUG_TOOL(
        if (m_mmcState) {
            m_mmcState->UpdateUserFeatureKey(&(m_vvcBasicFeature->m_destSurface));
        })

    if (m_isLastSliceInFrame)
    {
        DECODE_CHK_STATUS(m_miItf->AddMiBatchBufferEnd(&cmdBuffer, nullptr));
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VvcDecodePkt::PackSliceLevelCmds(MOS_COMMAND_BUFFER& cmdBuffer)
{
    DECODE_FUNC_CALL()

    PERF_UTILITY_AUTO(__FUNCTION__, PERF_DECODE, PERF_LEVEL_HAL);

    uint16_t sliceIdx = m_vvcBasicFeature->m_curSlice;
    if ( sliceIdx < m_vvcBasicFeature->m_numSlices)
    {
        DECODE_CHK_STATUS(m_slicePkt->Execute(cmdBuffer, m_vvcBasicFeature->m_sliceIdxInOrder[sliceIdx]));
        DECODE_CHK_STATUS(VdMemoryFlush(cmdBuffer));
        DECODE_CHK_STATUS(VdPipelineFlush(cmdBuffer));
    }

    if (m_isLastSliceInFrame)
    {
        DECODE_CHK_STATUS(EnsureAllCommandsExecuted(cmdBuffer));
        DECODE_CHK_STATUS(EndStatusReport(statusReportMfx, &cmdBuffer));
        DECODE_CHK_STATUS(UpdateStatusReportNext(statusReportGlobalCount, &cmdBuffer));
    }

    CODECHAL_DEBUG_TOOL(
        if (m_mmcState)
        {
            m_mmcState->UpdateUserFeatureKey(&(m_vvcBasicFeature->m_destSurface));
        })

    if (m_isLastSliceInFrame)
    {
        DECODE_CHK_STATUS(m_miItf->AddMiBatchBufferEnd(&cmdBuffer, nullptr));
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VvcDecodePkt::VdMemoryFlush(MOS_COMMAND_BUFFER& cmdBuffer)
{
    DECODE_FUNC_CALL()

    auto &par = m_vvcpItf->MHW_GETPAR_F(VVCP_VD_CONTROL_STATE)();
    par       = {};

    par.memoryImplicitFlush = true;

    DECODE_CHK_STATUS(m_vvcpItf->MHW_ADDCMD_F(VVCP_VD_CONTROL_STATE)(&cmdBuffer));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VvcDecodePkt::EnsureAllCommandsExecuted(MOS_COMMAND_BUFFER& cmdBuffer)
{
    DECODE_FUNC_CALL()

    // Send MI_FLUSH command
    auto &par = m_miItf->GETPAR_MI_FLUSH_DW();
    par       = {};
    auto *skuTable = m_vvcPipeline->GetSkuTable();
    if (skuTable && MEDIA_IS_SKU(skuTable, FtrEnablePPCFlush))
    {
        // Add PPC fulsh
        par.bEnablePPCFlush = true;
    }
    DECODE_CHK_STATUS(m_miItf->ADDCMD_MI_FLUSH_DW(&cmdBuffer));

    return MOS_STATUS_SUCCESS;
}

}
