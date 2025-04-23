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
//! \file     encode_jpeg_packet.cpp
//! \brief    Defines the interface for jpeg encode packet
//!

#include "encode_jpeg_packet.h"
#include "codec_def_encode_jpeg.h"
#include "mos_solo_generic.h"
#include "media_jpeg_feature_defs.h"

namespace encode {

    JpegPkt::JpegPkt(
        MediaPipeline *pipeline,
        MediaTask *task,
        CodechalHwInterfaceNext *hwInterface) :
        CmdPacket(task),
        m_pipeline(dynamic_cast<JpegPipeline *>(pipeline)),
        m_hwInterface(dynamic_cast<CodechalHwInterfaceNext *>(hwInterface))
    {
        ENCODE_CHK_NULL_NO_STATUS_RETURN(hwInterface);
        ENCODE_CHK_NULL_NO_STATUS_RETURN(m_pipeline);
        ENCODE_CHK_NULL_NO_STATUS_RETURN(m_hwInterface);

        m_osInterface    = hwInterface->GetOsInterface();
        m_statusReport   = m_pipeline->GetStatusReportInstance();
        m_featureManager = m_pipeline->GetFeatureManager();
        m_encodecp       = m_pipeline->GetEncodeCp();

        CODECHAL_DEBUG_TOOL(
            m_debugInterface = m_pipeline->GetDebugInterface();
            ENCODE_CHK_NULL_NO_STATUS_RETURN(m_debugInterface);)

        m_mfxItf = std::static_pointer_cast<mhw::vdbox::mfx::Itf>(m_hwInterface->GetMfxInterfaceNext());
        m_miItf  = std::static_pointer_cast<mhw::mi::Itf>(m_hwInterface->GetMiInterfaceNext());
    }

    MOS_STATUS JpegPkt::Init()
    {
        ENCODE_FUNC_CALL();
        ENCODE_CHK_NULL_RETURN(m_statusReport);

        ENCODE_CHK_STATUS_RETURN(CmdPacket::Init());

        m_basicFeature = dynamic_cast<JpegBasicFeature *>(m_featureManager->GetFeature(FeatureIDs::basicFeature));
        ENCODE_CHK_NULL_RETURN(m_basicFeature);

        m_jpgPkrFeature = dynamic_cast<JpegPackerFeature *>(m_featureManager->GetFeature(JpegFeatureIDs::jpegPackerFeature));
        ENCODE_CHK_NULL_RETURN(m_jpgPkrFeature);

#ifdef _MMC_SUPPORTED
        m_mmcState = m_pipeline->GetMmcState();
        ENCODE_CHK_NULL_RETURN(m_mmcState);
        m_basicFeature->m_mmcState = m_mmcState;
#endif

        ENCODE_CHK_STATUS_RETURN(m_statusReport->RegistObserver(this));

        m_usePatchList = m_osInterface->bUsesPatchList;

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS JpegPkt::Prepare()
    {
        ENCODE_FUNC_CALL();

        JpegPipeline *pipeline = dynamic_cast<JpegPipeline *>(m_pipeline);
        ENCODE_CHK_NULL_RETURN(pipeline);

        m_jpegPicParams       = m_basicFeature->m_jpegPicParams;
        m_jpegScanParams      = m_basicFeature->m_jpegScanParams;
        m_jpegQuantTables     = m_basicFeature->m_jpegQuantTables;
        m_jpegHuffmanTable    = m_basicFeature->m_jpegHuffmanTable;
        m_applicationData     = m_basicFeature->m_applicationData;
        m_numHuffBuffers      = m_basicFeature->m_numHuffBuffers;

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS JpegPkt::Destroy()
    {
        ENCODE_FUNC_CALL();
        ENCODE_CHK_STATUS_RETURN(m_statusReport->UnregistObserver(this));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS JpegPkt::Submit(
        MOS_COMMAND_BUFFER* commandBuffer,
        uint8_t packetPhase)
    {
        ENCODE_FUNC_CALL();

        MOS_COMMAND_BUFFER &cmdBuffer = *commandBuffer;
        ENCODE_CHK_STATUS_RETURN(Mos_Solo_PreProcessEncode(m_osInterface, &m_basicFeature->m_resBitstreamBuffer, &m_basicFeature->m_reconSurface));

        // Ensure the input is ready to be read.
        // Currently, mos RegisterResource has sync limitation for Raw resource.
        // Temporaly, call Resource Wait to do the sync explicitly.
        // TODO, Refine it when MOS refactor ready.
        MOS_SYNC_PARAMS syncParams;
        syncParams                  = g_cInitSyncParams;
        syncParams.GpuContext       = m_osInterface->pfnGetGpuContext(m_osInterface);
        syncParams.presSyncResource = &m_basicFeature->m_rawSurface.OsResource;
        syncParams.bReadOnly        = true;
        ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnResourceWait(m_osInterface, &syncParams));
        m_osInterface->pfnSetResourceSyncTag(m_osInterface, &syncParams);

        ENCODE_CHK_STATUS_RETURN(PatchPictureLevelCommands(packetPhase, cmdBuffer));
        ENCODE_CHK_STATUS_RETURN(PatchSliceLevelCommands(cmdBuffer, packetPhase));

        ENCODE_CHK_STATUS_RETURN(Mos_Solo_PostProcessEncode(m_osInterface, &m_basicFeature->m_resBitstreamBuffer, &m_basicFeature->m_reconSurface));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS JpegPkt::Completed(void *mfxStatus, void *rcsStatus, void *statusReport)
    {
        ENCODE_FUNC_CALL();

        ENCODE_CHK_NULL_RETURN(mfxStatus);
        ENCODE_CHK_NULL_RETURN(statusReport);

        EncodeStatusMfx        *encodeStatusMfx  = (EncodeStatusMfx *)mfxStatus;
        EncodeStatusReportData *statusReportData = (EncodeStatusReportData *)statusReport;

        if (statusReportData->hwCtr)
        {
            m_encodecp->UpdateCpStatusReport(statusReport);
        }

        statusReportData->codecStatus = CODECHAL_STATUS_SUCCESSFUL;

        CODECHAL_DEBUG_TOOL(
            ENCODE_CHK_STATUS_RETURN(DumpResources(encodeStatusMfx, statusReportData)););

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS JpegPkt::CalculateCommandSize(
        uint32_t &commandBufferSize,
        uint32_t &requestedPatchListSize)
    {
        ENCODE_FUNC_CALL();

        ENCODE_CHK_STATUS_RETURN(CalculateMfxCommandsSize());
        commandBufferSize      = CalculateCommandBufferSize();
        requestedPatchListSize = CalculatePatchListSize();
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS JpegPkt::CalculateMfxCommandsSize()
    {
        ENCODE_FUNC_CALL();

        // Picture Level Commands
        ENCODE_CHK_STATUS_RETURN(
            GetMfxStateCommandsDataSize(
                &m_pictureStatesSize,
                &m_picturePatchListSize));

        // Slice Level Commands (cannot be placed in 2nd level batch)
        ENCODE_CHK_STATUS_RETURN(
            GetMfxPrimitiveCommandsDataSize(
                &m_sliceStatesSize,
                &m_slicePatchListSize));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS JpegPkt::GetMfxPrimitiveCommandsDataSize(
        uint32_t *commandsSize,
        uint32_t *patchListSize)
    {
        ENCODE_FUNC_CALL()
        uint32_t cpCmdsize       = 0;
        uint32_t cpPatchListSize = 0;

        if (m_mfxItf)
        {
            uint32_t maxSize = 0;

            maxSize =
                m_mfxItf->MHW_GETSIZE_F(MFX_FQM_STATE)() * 3 +
                m_mfxItf->MHW_GETSIZE_F(MFC_JPEG_HUFF_TABLE_STATE)() * 2 +
                m_mfxItf->MHW_GETSIZE_F(MFC_JPEG_SCAN_OBJECT)() +
                m_mfxItf->MHW_GETSIZE_F(MFX_PAK_INSERT_OBJECT)() * 10;

            *commandsSize  = maxSize;
            *patchListSize = 0;

            m_hwInterface->GetCpInterface()->GetCpSliceLevelCmdSize(cpCmdsize, cpPatchListSize);
        }

        *commandsSize += cpCmdsize;
        *patchListSize += cpPatchListSize;

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS JpegPkt::GetMfxStateCommandsDataSize(
        uint32_t *commandsSize,
        uint32_t *patchListSize)
    {
        ENCODE_FUNC_CALL()
        ENCODE_CHK_NULL_RETURN(commandsSize);
        ENCODE_CHK_NULL_RETURN(patchListSize);

        uint32_t cpCmdsize       = 0;
        uint32_t cpPatchListSize = 0;

        if (m_mfxItf && m_miItf)
        {
            uint32_t maxSize =
                m_miItf->MHW_GETSIZE_F(MI_FLUSH_DW)() +
                m_mfxItf->MHW_GETSIZE_F(MFX_PIPE_MODE_SELECT)() +
                m_mfxItf->MHW_GETSIZE_F(MFX_SURFACE_STATE)() +
                m_mfxItf->MHW_GETSIZE_F(MFX_PIPE_BUF_ADDR_STATE)() +
                m_mfxItf->MHW_GETSIZE_F(MFX_IND_OBJ_BASE_ADDR_STATE)() +
                2 * m_miItf->MHW_GETSIZE_F(MI_STORE_DATA_IMM)() +
                2 * m_miItf->MHW_GETSIZE_F(MI_STORE_REGISTER_MEM)() +
                8 * m_miItf->MHW_GETSIZE_F(MI_LOAD_REGISTER_REG)();

            uint32_t patchListMaxSize =
                PATCH_LIST_COMMAND(mhw::vdbox::huc::Itf::MI_FLUSH_DW_CMD) +
                PATCH_LIST_COMMAND(mhw::vdbox::mfx::Itf::MFX_PIPE_MODE_SELECT_CMD) +
                PATCH_LIST_COMMAND(mhw::vdbox::mfx::Itf::MFX_SURFACE_STATE_CMD) +
                PATCH_LIST_COMMAND(mhw::vdbox::mfx::Itf::MFX_PIPE_BUF_ADDR_STATE_CMD) +
                PATCH_LIST_COMMAND(mhw::vdbox::mfx::Itf::MFX_IND_OBJ_BASE_ADDR_STATE_CMD) +
                (2 * PATCH_LIST_COMMAND(mhw::vdbox::huc::Itf::MI_STORE_DATA_IMM_CMD)) +
                (2 * PATCH_LIST_COMMAND(mhw::vdbox::huc::Itf::MI_STORE_REGISTER_MEM_CMD));

            *commandsSize  = maxSize;
            *patchListSize = patchListMaxSize;

            m_hwInterface->GetCpInterface()->GetCpStateLevelCmdSize(cpCmdsize, cpPatchListSize);
        }

        *commandsSize += cpCmdsize;
        *patchListSize += cpPatchListSize;

        return MOS_STATUS_SUCCESS;
    }

    MmioRegistersMfx *JpegPkt::SelectVdboxAndGetMmioRegister(
        MHW_VDBOX_NODE_IND  index,
        PMOS_COMMAND_BUFFER pCmdBuffer)
    {
        if (m_hwInterface->m_getVdboxNodeByUMD)
        {
            pCmdBuffer->iVdboxNodeIndex = m_osInterface->pfnGetVdboxNodeId(m_osInterface, pCmdBuffer);
            switch (pCmdBuffer->iVdboxNodeIndex)
            {
            case MOS_VDBOX_NODE_1:
                index = MHW_VDBOX_NODE_1;
                break;
            case MOS_VDBOX_NODE_2:
                index = MHW_VDBOX_NODE_2;
                break;
            case MOS_VDBOX_NODE_INVALID:
                // That's a legal case meaning that we were not assigned with per-bb index because
                // balancing algorithm can't work (forcedly diabled or miss kernel support).
                // If that's the case we just proceed with the further static context assignment.
                break;
            default:
                // That's the case when MHW and MOS enumerations mismatch. We again proceed with the
                // best effort (static context assignment, but provide debug note).
                MHW_ASSERTMESSAGE("MOS and MHW VDBOX enumerations mismatch! Adjust HW description!");
                break;
            }
        }

        auto vdencItf = m_hwInterface->GetVdencInterfaceNext();
        if (vdencItf)
        {
            return vdencItf->GetMmioRegisters(index);
        }
        else
        {
            MHW_ASSERTMESSAGE("Get vdenc interface failed!");
            return nullptr;
        }
    }

    void JpegPkt::SetPerfTag(uint16_t type, uint16_t mode, uint16_t picCodingType)
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

    MOS_STATUS JpegPkt::AddPictureMfxCommands(
        MOS_COMMAND_BUFFER & cmdBuffer)
    {
        ENCODE_FUNC_CALL();

        SETPAR_AND_ADDCMD(MFX_WAIT, m_miItf, &cmdBuffer);
        SETPAR_AND_ADDCMD(MFX_PIPE_MODE_SELECT, m_mfxItf, &cmdBuffer);
        SETPAR_AND_ADDCMD(MFX_WAIT, m_miItf, &cmdBuffer);

        SETPAR_AND_ADDCMD(MFX_SURFACE_STATE, m_mfxItf, &cmdBuffer);
        SETPAR_AND_ADDCMD(MFX_PIPE_BUF_ADDR_STATE, m_mfxItf, &cmdBuffer);
        SETPAR_AND_ADDCMD(MFX_IND_OBJ_BASE_ADDR_STATE, m_mfxItf, &cmdBuffer);
        SETPAR_AND_ADDCMD(MFX_JPEG_PIC_STATE, m_mfxItf, &cmdBuffer);

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS JpegPkt::SendPrologCmds(MOS_COMMAND_BUFFER &cmdBuffer)
    {
        ENCODE_FUNC_CALL();
        MOS_GPU_CONTEXT gpuContext = m_osInterface->pfnGetGpuContext(m_osInterface);

        // initialize command buffer attributes
        cmdBuffer.Attributes.bTurboMode               = m_hwInterface->m_turboMode;
        cmdBuffer.Attributes.bMediaPreemptionEnabled   = 0;
        cmdBuffer.Attributes.dwNumRequestedEUSlices    = m_hwInterface->m_numRequestedEuSlices;
        cmdBuffer.Attributes.dwNumRequestedSubSlices   = m_hwInterface->m_numRequestedSubSlices;
        cmdBuffer.Attributes.dwNumRequestedEUs         = m_hwInterface->m_numRequestedEus;
        cmdBuffer.Attributes.bValidPowerGatingRequest  = true;

#ifdef _MMC_SUPPORTED
        ENCODE_CHK_NULL_RETURN(m_mmcState);
        ENCODE_CHK_STATUS_RETURN(m_mmcState->SendPrologCmd(&cmdBuffer, false));
#endif

        MHW_GENERIC_PROLOG_PARAMS genericPrologParams;
        MOS_ZeroMemory(&genericPrologParams, sizeof(genericPrologParams));
        genericPrologParams.pOsInterface     = m_osInterface;
        genericPrologParams.pvMiInterface    = nullptr;
        genericPrologParams.bMmcEnabled      = m_mmcState ? m_mmcState->IsMmcEnabled() : false;
        ENCODE_CHK_STATUS_RETURN(Mhw_SendGenericPrologCmdNext(&cmdBuffer, &genericPrologParams, m_miItf));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS JpegPkt::StartStatusReport(
        uint32_t            srType,
        MOS_COMMAND_BUFFER *cmdBuffer)
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

    MOS_STATUS JpegPkt::ReadMfcStatus(MOS_COMMAND_BUFFER &cmdBuffer)
    {
        ENCODE_FUNC_CALL();

        ENCODE_CHK_NULL_RETURN(m_hwInterface);

        MOS_RESOURCE *osResource = nullptr;
        uint32_t      offset = 0;

        EncodeStatusReadParams params;
        MOS_ZeroMemory(&params, sizeof(params));

        ENCODE_CHK_STATUS_RETURN(m_statusReport->GetAddress(encode::statusReportMfxBitstreamByteCountPerFrame, osResource, offset));
        params.resBitstreamByteCountPerFrame    = osResource;
        params.bitstreamByteCountPerFrameOffset = offset;

        ENCODE_CHK_STATUS_RETURN(m_statusReport->GetAddress(encode::statusReportMfxBitstreamSyntaxElementOnlyBitCount, osResource, offset));
        params.resBitstreamSyntaxElementOnlyBitCount    = osResource;
        params.bitstreamSyntaxElementOnlyBitCountOffset = offset;

        ENCODE_CHK_STATUS_RETURN(m_statusReport->GetAddress(encode::statusReportQPStatusCount, osResource, offset));
        params.resQpStatusCount    = osResource;
        params.qpStatusCountOffset = offset;

        ENCODE_CHK_STATUS_RETURN(m_statusReport->GetAddress(encode::statusReportImageStatusMask, osResource, offset));
        params.resImageStatusMask    = osResource;
        params.imageStatusMaskOffset = offset;

        ENCODE_CHK_STATUS_RETURN(m_statusReport->GetAddress(encode::statusReportImageStatusCtrl, osResource, offset));
        params.resImageStatusCtrl    = osResource;
        params.imageStatusCtrlOffset = offset;

        ENCODE_CHK_STATUS_RETURN(m_statusReport->GetAddress(encode::statusReportNumSlices, osResource, offset));
        params.resNumSlices    = osResource;
        params.numSlicesOffset = offset;

        ENCODE_CHK_COND_RETURN((m_vdboxIndex > m_mfxItf->GetMaxVdboxIndex()), "ERROR - vdbox index exceeds the maximum");

        SETPAR_AND_ADDCMD(MI_FLUSH_DW, m_miItf, &cmdBuffer);

        MmioRegistersMfx *mmioRegisters = SelectVdboxAndGetMmioRegister(m_vdboxIndex, &cmdBuffer);
        CODEC_HW_CHK_NULL_RETURN(mmioRegisters);
        m_pResource                     = params.resBitstreamByteCountPerFrame;
        m_dwOffset                      = params.bitstreamByteCountPerFrameOffset;
        m_dwValue                       = mmioRegisters->mfcBitstreamBytecountFrameRegOffset;
        SETPAR_AND_ADDCMD(MI_STORE_REGISTER_MEM, m_miItf, &cmdBuffer);

        m_pResource = params.resBitstreamSyntaxElementOnlyBitCount;
        m_dwOffset  = params.bitstreamSyntaxElementOnlyBitCountOffset;
        m_dwValue   = mmioRegisters->mfcBitstreamSeBitcountFrameRegOffset;
        SETPAR_AND_ADDCMD(MI_STORE_REGISTER_MEM, m_miItf, &cmdBuffer);

        m_pResource = params.resQpStatusCount;
        m_dwOffset  = params.qpStatusCountOffset;
        m_dwValue   = mmioRegisters->mfcQPStatusCountOffset;
        SETPAR_AND_ADDCMD(MI_STORE_REGISTER_MEM, m_miItf, &cmdBuffer);

        ENCODE_CHK_STATUS_RETURN(ReadImageStatus(params, &cmdBuffer))
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS JpegPkt::ReadImageStatus(
        const EncodeStatusReadParams &params,
        PMOS_COMMAND_BUFFER           cmdBuffer)
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        CODEC_HW_FUNCTION_ENTER;

        CODEC_HW_CHK_NULL_RETURN(cmdBuffer);

        CODEC_HW_CHK_COND_RETURN((m_vdboxIndex > m_mfxItf->GetMaxVdboxIndex()), "ERROR - vdbox index exceeds the maximum");

        MmioRegistersMfx *mmioRegisters = SelectVdboxAndGetMmioRegister(m_vdboxIndex, cmdBuffer);
        CODEC_HW_CHK_NULL_RETURN(mmioRegisters);
        m_pResource                     = params.resImageStatusMask;
        m_dwOffset                      = params.imageStatusMaskOffset;
        m_dwValue                       = mmioRegisters->mfcImageStatusMaskRegOffset;
        SETPAR_AND_ADDCMD(MI_STORE_REGISTER_MEM, m_miItf, cmdBuffer);

        m_pResource = params.resImageStatusCtrl;
        m_dwOffset  = params.imageStatusCtrlOffset;
        m_dwValue   = mmioRegisters->mfcImageStatusCtrlRegOffset;
        SETPAR_AND_ADDCMD(MI_STORE_REGISTER_MEM, m_miItf, cmdBuffer);

        SETPAR_AND_ADDCMD(MI_FLUSH_DW, m_miItf, cmdBuffer);

        return eStatus;
    }

    MOS_STATUS JpegPkt::EndStatusReport(
        uint32_t            srType,
        MOS_COMMAND_BUFFER *cmdBuffer)
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

    MOS_STATUS JpegPkt::PatchPictureLevelCommands(const uint8_t &packetPhase, MOS_COMMAND_BUFFER  &cmdBuffer)
    {
        ENCODE_FUNC_CALL();

        ENCODE_CHK_STATUS_RETURN(m_miItf->SetWatchdogTimerThreshold(m_basicFeature->m_frameWidth, m_basicFeature->m_frameHeight, true));

        SetPerfTag(CODECHAL_ENCODE_PERFTAG_CALL_PAK_ENGINE, (uint16_t)m_basicFeature->m_mode, m_basicFeature->m_pictureCodingType);

        SETPAR_AND_ADDCMD(MI_FORCE_WAKEUP, m_miItf, &cmdBuffer);

        // Send command buffer header at the beginning (OS dependent)
        ENCODE_CHK_STATUS_RETURN(SendPrologCmds(cmdBuffer));

        if (m_pipeline->IsFirstPipe())
        {
            ENCODE_CHK_STATUS_RETURN(StartStatusReport(statusReportMfx, &cmdBuffer));
        }

        ENCODE_CHK_STATUS_RETURN(AddPictureMfxCommands(cmdBuffer));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS JpegPkt::PatchSliceLevelCommands(MOS_COMMAND_BUFFER &cmdBuffer, uint8_t packetPhase)
    {
        ENCODE_FUNC_CALL();

        if (m_basicFeature->m_numSlices != 1)
        {
            ENCODE_ASSERTMESSAGE("JPEG encode only one scan is supported.");
        }

        static_assert(JPEG_MAX_NUM_QUANT_TABLE_INDEX <= JPEG_MAX_NUM_OF_QUANTMATRIX,
            "access to CodecJpegQuantMatrix is controlled by numQuantTables");

        for (uint32_t scanCount = 0; scanCount < m_basicFeature->m_numSlices; scanCount++)
        {
            m_numQuantTables = JPEG_MAX_NUM_QUANT_TABLE_INDEX;
            ENCODE_CHK_STATUS_RETURN(InitMissedQuantTables());

            ENCODE_CHK_STATUS_RETURN(InitQuantMatrix());

            ENCODE_CHK_STATUS_RETURN(AddAllCmds_MFX_FQM_STATE(&cmdBuffer));

            ENCODE_CHK_STATUS_RETURN(InitHuffTable());

            ENCODE_CHK_STATUS_RETURN(AddAllCmds_MFC_JPEG_HUFF_TABLE_STATE(&cmdBuffer));

            SETPAR_AND_ADDCMD(MFC_JPEG_SCAN_OBJECT, m_mfxItf, &cmdBuffer);

            ENCODE_CHK_STATUS_RETURN(AddAllCmds_MFX_PAK_INSERT_OBJECT(&cmdBuffer));
        }

        ENCODE_CHK_STATUS_RETURN(ReadMfcStatus(cmdBuffer));

        if (m_pipeline->IsFirstPipe())
        {
            ENCODE_CHK_STATUS_RETURN(EndStatusReport(statusReportMfx, &cmdBuffer));
        }

        if (!m_pipeline->IsSingleTaskPhaseSupported() || m_pipeline->IsLastPass())
        {
             ENCODE_CHK_STATUS_RETURN(UpdateStatusReportNext(statusReportGlobalCount, &cmdBuffer));
        }

        ENCODE_CHK_STATUS_RETURN(m_miItf->AddMiBatchBufferEnd(&cmdBuffer, nullptr));

        std::string pakPassName = "PAK_PASS" + std::to_string(static_cast<uint32_t>(m_pipeline->GetCurrentPass()));
        CODECHAL_DEBUG_TOOL(
            CodechalDebugInterface *debugInterface = m_pipeline->GetDebugInterface();
            ENCODE_CHK_NULL_RETURN(debugInterface);
            ENCODE_CHK_STATUS_RETURN(debugInterface->DumpCmdBuffer(
                &cmdBuffer,
                CODECHAL_NUM_MEDIA_STATES,
                pakPassName.data()));
        )

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS JpegPkt::InitMissedQuantTables()
    {
        ENCODE_FUNC_CALL();

        MOS_SURFACE *surface = m_basicFeature->m_rawSurfaceToPak;
        bool useSingleDefaultQuantTable = (m_basicFeature->m_jpegQuantMatrixSent == false &&
                                           ((surface->Format == Format_A8R8G8B8) ||
                                            (surface->Format == Format_X8R8G8B8) ||
                                            (surface->Format == Format_A8B8G8R8) ||
                                            (surface->Format == Format_X8B8G8R8)));

        // For monochrome inputs there will be only 1 quantization table and huffman table sent
        if (m_jpegPicParams->m_inputSurfaceFormat == codechalJpegY8)
        {
            m_numQuantTables = 1;
            m_numHuffBuffers = 2;  //for Y8 only 2 huff tables
        }
        // If there is only 1 quantization table copy over the table to 2nd and 3rd table in JPEG state (used for frame header)
        // OR For RGB input surfaces, if the app does not send quantization tables, then use luma quant table for all 3 components
        else if (m_jpegPicParams->m_numQuantTable == 1 || useSingleDefaultQuantTable)
        {
            for (auto i = 1; i < JPEG_MAX_NUM_QUANT_TABLE_INDEX; i++)
            {
                m_jpegQuantTables->m_quantTable[i].m_precision = m_jpegQuantTables->m_quantTable[0].m_precision;
                m_jpegQuantTables->m_quantTable[i].m_tableID   = m_jpegQuantTables->m_quantTable[0].m_tableID;

                ENCODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(
                    &m_jpegQuantTables->m_quantTable[i].m_qm[0],
                    JPEG_NUM_QUANTMATRIX * sizeof(uint16_t),
                    &m_jpegQuantTables->m_quantTable[0].m_qm[0],
                    JPEG_NUM_QUANTMATRIX * sizeof(uint16_t)));
            }
        }
        // If there are 2 quantization tables copy over the second table to 3rd table in JPEG state since U and V share the same table (used for frame header)
        else if (m_jpegPicParams->m_numQuantTable == 2)
        {
            m_jpegQuantTables->m_quantTable[2].m_precision = m_jpegQuantTables->m_quantTable[1].m_precision;
            m_jpegQuantTables->m_quantTable[2].m_tableID   = m_jpegQuantTables->m_quantTable[1].m_tableID;

            ENCODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(
                &m_jpegQuantTables->m_quantTable[2].m_qm[0],
                JPEG_NUM_QUANTMATRIX * sizeof(uint16_t),
                &m_jpegQuantTables->m_quantTable[1].m_qm[0],
                JPEG_NUM_QUANTMATRIX * sizeof(uint16_t)));
        }
        // else 3 quantization tables are sent by the application for non monochrome input formats. In that case, do nothing.

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS JpegPkt::InitQuantMatrix()
    {
        ENCODE_FUNC_CALL();

        m_jpegQuantMatrix = {};

        for (uint8_t i = 0; i < m_numQuantTables; i++)
        {
            for (auto j = 0; j < JPEG_NUM_QUANTMATRIX; j++)
            {
                uint32_t k = jpeg_qm_scan_8x8[j];

                // copy over Quant matrix in raster order from zig zag
                m_jpegQuantMatrix.m_quantMatrix[i][k] = (uint8_t)m_jpegQuantTables->m_quantTable[i].m_qm[j];
            }
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS JpegPkt::InitHuffTable()
    {
        ENCODE_FUNC_CALL();

        for (uint32_t i = 0; i < m_numHuffBuffers; i++)
        {
            EncodeJpegHuffTable huffmanTable;  // intermediate table for each AC/DC component which will be copied to m_huffTableParams
            MOS_ZeroMemory(&huffmanTable, sizeof(huffmanTable));

            ENCODE_CHK_STATUS_RETURN(ConvertHuffDataToTable(m_jpegHuffmanTable->m_huffmanData[i], &huffmanTable));

            m_huffTableParams[m_jpegHuffmanTable->m_huffmanData[i].m_tableID].HuffTableID = m_jpegHuffmanTable->m_huffmanData[i].m_tableID;

            if (m_jpegHuffmanTable->m_huffmanData[i].m_tableClass == 0)  // DC table
            {
                ENCODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(
                    m_huffTableParams[m_jpegHuffmanTable->m_huffmanData[i].m_tableID].pDCCodeValues,
                    JPEG_NUM_HUFF_TABLE_DC_HUFFVAL * sizeof(uint16_t),
                    &huffmanTable.m_huffCode,
                    JPEG_NUM_HUFF_TABLE_DC_HUFFVAL * sizeof(uint16_t)));

                ENCODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(m_huffTableParams[m_jpegHuffmanTable->m_huffmanData[i].m_tableID].pDCCodeLength,
                    JPEG_NUM_HUFF_TABLE_DC_HUFFVAL * sizeof(uint8_t),
                    &huffmanTable.m_huffSize,
                    JPEG_NUM_HUFF_TABLE_DC_HUFFVAL * sizeof(uint8_t)));
            }
            else  // AC Table
            {
                ENCODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(m_huffTableParams[m_jpegHuffmanTable->m_huffmanData[i].m_tableID].pACCodeValues,
                    JPEG_NUM_HUFF_TABLE_AC_HUFFVAL * sizeof(uint16_t),
                    &huffmanTable.m_huffCode,
                    JPEG_NUM_HUFF_TABLE_AC_HUFFVAL * sizeof(uint16_t)));

                ENCODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(m_huffTableParams[m_jpegHuffmanTable->m_huffmanData[i].m_tableID].pACCodeLength,
                    JPEG_NUM_HUFF_TABLE_AC_HUFFVAL * sizeof(uint8_t),
                    &huffmanTable.m_huffSize,
                    JPEG_NUM_HUFF_TABLE_AC_HUFFVAL * sizeof(uint8_t)));
            }
        }

        // Send 2 huffman table commands - 1 for Luma and one for chroma for non-monchrome input formats
        // If only one table is sent by the app (2 buffers), send the same table for Luma and chroma
        m_repeatHuffTable = false;
        if ((m_numHuffBuffers / 2 < JPEG_MAX_NUM_HUFF_TABLE_INDEX) && (m_jpegPicParams->m_inputSurfaceFormat != codechalJpegY8))
        {
            m_repeatHuffTable = true;

            // Copy over huffman data to the other two data buffers for JPEG picture header
            for (uint32_t i = 0; i < m_numHuffBuffers; i++)
            {
                if ((i + 2) >= JPEG_NUM_ENCODE_HUFF_BUFF)
                {
                    return MOS_STATUS_INVALID_PARAMETER;
                }
                m_jpegHuffmanTable->m_huffmanData[i + 2].m_tableClass = m_jpegHuffmanTable->m_huffmanData[i].m_tableClass;
                m_jpegHuffmanTable->m_huffmanData[i + 2].m_tableID    = m_jpegHuffmanTable->m_huffmanData[i].m_tableID;

                ENCODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(&m_jpegHuffmanTable->m_huffmanData[i + 2].m_bits[0],
                    sizeof(uint8_t) * JPEG_NUM_HUFF_TABLE_AC_BITS,
                    &m_jpegHuffmanTable->m_huffmanData[i].m_bits[0],
                    sizeof(uint8_t) * JPEG_NUM_HUFF_TABLE_AC_BITS));

                ENCODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(&m_jpegHuffmanTable->m_huffmanData[i + 2].m_huffVal[0],
                    sizeof(uint8_t) * JPEG_NUM_HUFF_TABLE_AC_HUFFVAL,
                    &m_jpegHuffmanTable->m_huffmanData[i].m_huffVal[0],
                    sizeof(uint8_t) * JPEG_NUM_HUFF_TABLE_AC_HUFFVAL));
            }
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS JpegPkt::AddSOI(PMOS_COMMAND_BUFFER cmdBuffer) const
    {
        ENCODE_FUNC_CALL();
        ENCODE_CHK_NULL_RETURN(cmdBuffer);

        BSBuffer bsBuffer = {};
        ENCODE_CHK_STATUS_RETURN(m_jpgPkrFeature->PackSOI(&bsBuffer));

        uint32_t byteSize = (bsBuffer.BufferSize + 7) >> 3;
        uint32_t dataBitsInLastDw = bsBuffer.BufferSize % 32;
        if (dataBitsInLastDw == 0)
        {
            dataBitsInLastDw = 32;
        }

        auto &params = m_mfxItf->MHW_GETPAR_F(MFX_PAK_INSERT_OBJECT)();
        params = {};
        params.dwPadding = ((byteSize + 3) >> 2);
        params.bitstreamstartresetResetbitstreamstartingpos = 1;
        params.databitsinlastdwSrcdataendingbitinclusion50 = dataBitsInLastDw;

        m_mfxItf->MHW_ADDCMD_F(MFX_PAK_INSERT_OBJECT)(cmdBuffer);

        // Add actual data
        uint8_t* data = (uint8_t*)(bsBuffer.pBase);
        MOS_STATUS eStatus = Mhw_AddCommandCmdOrBB(m_osInterface, cmdBuffer, nullptr, data, byteSize);

        MOS_SafeFreeMemory(bsBuffer.pBase);

        return eStatus;
    }

    MOS_STATUS JpegPkt::AddApplicationData(PMOS_COMMAND_BUFFER cmdBuffer) const
    {
        ENCODE_FUNC_CALL();
        ENCODE_CHK_NULL_RETURN(cmdBuffer);

        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        BSBuffer bsBuffer         = {};
        uint32_t appDataChunkSize = m_basicFeature->m_appDataSize;
        auto    &params           = m_mfxItf->MHW_GETPAR_F(MFX_PAK_INSERT_OBJECT)();

        // We can write a maximum of 1020 words per command, so if the size of the app data is
        // more than 1020 we need to send multiple commands for writing out app data
        uint32_t numAppDataCmdsNeeded  = 1;
        uint32_t appDataCmdSizeResidue = 0;
        if (m_basicFeature->m_appDataSize > 1020)
        {
            numAppDataCmdsNeeded  = m_basicFeature->m_appDataSize / 1020;
            appDataCmdSizeResidue = m_basicFeature->m_appDataSize % 1020;

            appDataChunkSize = 1020;
        }

        uint8_t *appDataChunk = (uint8_t *)MOS_AllocAndZeroMemory(appDataChunkSize);
        ENCODE_CHK_NULL_RETURN(appDataChunk)

        for (uint32_t i = 0; i < numAppDataCmdsNeeded; i++)
        {
            uint8_t *copyAddress = (uint8_t *)(m_applicationData) + (i * appDataChunkSize);

            MOS_SecureMemcpy(appDataChunk, appDataChunkSize, copyAddress, appDataChunkSize);
            eStatus = m_jpgPkrFeature->PackApplicationData(&bsBuffer, appDataChunk, appDataChunkSize);
            if (eStatus != MOS_STATUS_SUCCESS)
            {
                MOS_SafeFreeMemory(appDataChunk);
                ENCODE_CHK_STATUS_RETURN(eStatus);
            }

            uint32_t byteSize = (bsBuffer.BufferSize + 7) >> 3;
            uint32_t dataBitsInLastDw = bsBuffer.BufferSize % 32;
            if (dataBitsInLastDw == 0)
            {
                dataBitsInLastDw = 32;
            }

            params = {};
            params.dwPadding = ((byteSize + 3) >> 2);
            params.bitstreamstartresetResetbitstreamstartingpos = 1;
            //if full header is included in application data, it will be the last header to insert and last chunk of it should be marked with EndOfSlice
            if ((appDataCmdSizeResidue == 0) && m_basicFeature->m_fullHeaderInAppData && (i == numAppDataCmdsNeeded - 1))
            {
                params.endofsliceflagLastdstdatainsertcommandflag = true;
                params.lastheaderflagLastsrcheaderdatainsertcommandflag = true;
            }
            else
            {
                params.endofsliceflagLastdstdatainsertcommandflag = false;
                params.lastheaderflagLastsrcheaderdatainsertcommandflag = false;
            }
            params.databitsinlastdwSrcdataendingbitinclusion50 = dataBitsInLastDw;

            m_mfxItf->MHW_ADDCMD_F(MFX_PAK_INSERT_OBJECT)(cmdBuffer);

            // Add actual data
            uint8_t* data = (uint8_t*)(bsBuffer.pBase);
            eStatus = Mhw_AddCommandCmdOrBB(m_osInterface, cmdBuffer, nullptr, data, byteSize);
            if (eStatus != MOS_STATUS_SUCCESS)
            {
                MOS_SafeFreeMemory(appDataChunk);
                ENCODE_CHK_STATUS_RETURN(eStatus);
            }
        }

        if (appDataCmdSizeResidue != 0)
        {
            uint8_t *lastAddress = (uint8_t *)(m_applicationData) + (numAppDataCmdsNeeded * appDataChunkSize);
            appDataChunkSize     = appDataCmdSizeResidue;

            MOS_SecureMemcpy(appDataChunk, appDataChunkSize, lastAddress, appDataChunkSize);

            eStatus = m_jpgPkrFeature->PackApplicationData(&bsBuffer, appDataChunk, appDataChunkSize);
            if (eStatus != MOS_STATUS_SUCCESS)
            {
                MOS_SafeFreeMemory(appDataChunk);
                ENCODE_CHK_STATUS_RETURN(eStatus);
            }

            uint32_t byteSize = (bsBuffer.BufferSize + 7) >> 3;
            uint32_t dataBitsInLastDw = bsBuffer.BufferSize % 32;
            if (dataBitsInLastDw == 0)
            {
                dataBitsInLastDw = 32;
            }

            params = {};
            params.dwPadding = ((byteSize + 3) >> 2);
            params.bitstreamstartresetResetbitstreamstartingpos = 1;
            //if full header is included in application data, it will be the last insert headers
            if (m_basicFeature->m_fullHeaderInAppData)
            {
                params.endofsliceflagLastdstdatainsertcommandflag = true;
                params.lastheaderflagLastsrcheaderdatainsertcommandflag = true;
            }
            else
            {
                params.endofsliceflagLastdstdatainsertcommandflag = false;
                params.lastheaderflagLastsrcheaderdatainsertcommandflag = false;
            }
            params.databitsinlastdwSrcdataendingbitinclusion50 = dataBitsInLastDw;

            m_mfxItf->MHW_ADDCMD_F(MFX_PAK_INSERT_OBJECT)(cmdBuffer);

            // Add actual data
            uint8_t* data = (uint8_t*)(bsBuffer.pBase);
            eStatus = Mhw_AddCommandCmdOrBB(m_osInterface, cmdBuffer, nullptr, data, byteSize);
            if (eStatus != MOS_STATUS_SUCCESS)
            {
                MOS_SafeFreeMemory(appDataChunk);
                ENCODE_CHK_STATUS_RETURN(eStatus);
            }
        }

        MOS_FreeMemory(appDataChunk);

        return eStatus;
    }

    MOS_STATUS JpegPkt::AddQuantTable(PMOS_COMMAND_BUFFER cmdBuffer, bool useSingleDefaultQuantTable) const
    {
        ENCODE_FUNC_CALL();
        ENCODE_CHK_NULL_RETURN(cmdBuffer);

        // Add Quant Table for Y
        BSBuffer bsBuffer = {};
        ENCODE_CHK_STATUS_RETURN(m_jpgPkrFeature->PackQuantTable(&bsBuffer, jpegComponentY));

        uint32_t byteSize = (bsBuffer.BufferSize + 7) >> 3;
        uint32_t dataBitsInLastDw = bsBuffer.BufferSize % 32;
        if (dataBitsInLastDw == 0)
        {
            dataBitsInLastDw = 32;
        }

        auto &params = m_mfxItf->MHW_GETPAR_F(MFX_PAK_INSERT_OBJECT)();
        params = {};
        params.dwPadding = ((byteSize + 3) >> 2);
        params.bitstreamstartresetResetbitstreamstartingpos = 1;
        params.databitsinlastdwSrcdataendingbitinclusion50 = dataBitsInLastDw;

        m_mfxItf->MHW_ADDCMD_F(MFX_PAK_INSERT_OBJECT)(cmdBuffer);

        // Add actual data
        uint8_t* data = (uint8_t*)(bsBuffer.pBase);
        MOS_STATUS eStatus = Mhw_AddCommandCmdOrBB(m_osInterface, cmdBuffer, nullptr, data, byteSize);

        MOS_SafeFreeMemory(bsBuffer.pBase);

        ENCODE_CHK_STATUS_RETURN(eStatus);

        if (!useSingleDefaultQuantTable)
        {
            // Since there is no U and V in monochrome format, donot add Quantization table header for U and V components
            if (m_jpegPicParams->m_inputSurfaceFormat != codechalJpegY8)
            {
                // Add quant table for U
                ENCODE_CHK_STATUS_RETURN(m_jpgPkrFeature->PackQuantTable(&bsBuffer, jpegComponentU));

                byteSize = (bsBuffer.BufferSize + 7) >> 3;
                dataBitsInLastDw = bsBuffer.BufferSize % 32;
                if (dataBitsInLastDw == 0)
                {
                    dataBitsInLastDw = 32;
                }

                params = {};
                params.dwPadding = ((byteSize + 3) >> 2);
                params.bitstreamstartresetResetbitstreamstartingpos = 1;
                params.databitsinlastdwSrcdataendingbitinclusion50 = dataBitsInLastDw;

                m_mfxItf->MHW_ADDCMD_F(MFX_PAK_INSERT_OBJECT)(cmdBuffer);

                // Add actual data
                data = (uint8_t*)(bsBuffer.pBase);
                eStatus = Mhw_AddCommandCmdOrBB(m_osInterface, cmdBuffer, nullptr, data, byteSize);

                MOS_SafeFreeMemory(bsBuffer.pBase);

                ENCODE_CHK_STATUS_RETURN(eStatus);

                // Add quant table for V
                ENCODE_CHK_STATUS_RETURN(m_jpgPkrFeature->PackQuantTable(&bsBuffer, jpegComponentV));

                byteSize = (bsBuffer.BufferSize + 7) >> 3;
                dataBitsInLastDw = bsBuffer.BufferSize % 32;
                if (dataBitsInLastDw == 0)
                {
                    dataBitsInLastDw = 32;
                }

                params = {};
                params.dwPadding = ((byteSize + 3) >> 2);
                params.bitstreamstartresetResetbitstreamstartingpos = 1;
                params.databitsinlastdwSrcdataendingbitinclusion50 = dataBitsInLastDw;

                m_mfxItf->MHW_ADDCMD_F(MFX_PAK_INSERT_OBJECT)(cmdBuffer);

                // Add actual data
                data = (uint8_t*)(bsBuffer.pBase);
                eStatus = Mhw_AddCommandCmdOrBB(m_osInterface, cmdBuffer, nullptr, data, byteSize);

                MOS_SafeFreeMemory(bsBuffer.pBase);

                ENCODE_CHK_STATUS_RETURN(eStatus);
            }
        }

        return eStatus;
    }

    MOS_STATUS JpegPkt::AddFrameHeader(PMOS_COMMAND_BUFFER cmdBuffer, bool useSingleDefaultQuantTable) const
    {
        ENCODE_FUNC_CALL();
        ENCODE_CHK_NULL_RETURN(cmdBuffer);

        BSBuffer bsBuffer = {};
        ENCODE_CHK_STATUS_RETURN(m_jpgPkrFeature->PackFrameHeader(&bsBuffer, useSingleDefaultQuantTable));

        uint32_t byteSize = (bsBuffer.BufferSize + 7) >> 3;
        uint32_t dataBitsInLastDw = bsBuffer.BufferSize % 32;
        if (dataBitsInLastDw == 0)
        {
            dataBitsInLastDw = 32;
        }

        auto &params = m_mfxItf->MHW_GETPAR_F(MFX_PAK_INSERT_OBJECT)();
        params = {};
        params.dwPadding = ((byteSize + 3) >> 2);
        params.bitstreamstartresetResetbitstreamstartingpos = 1;
        params.databitsinlastdwSrcdataendingbitinclusion50 = dataBitsInLastDw;

        m_mfxItf->MHW_ADDCMD_F(MFX_PAK_INSERT_OBJECT)(cmdBuffer);

        // Add actual data
        uint8_t* data = (uint8_t*)(bsBuffer.pBase);
        MOS_STATUS eStatus = Mhw_AddCommandCmdOrBB(m_osInterface, cmdBuffer, nullptr, data, byteSize);

        MOS_SafeFreeMemory(bsBuffer.pBase);

        return eStatus;
    }

    MOS_STATUS JpegPkt::AddHuffmanTable(PMOS_COMMAND_BUFFER cmdBuffer, uint32_t tblInd) const
    {
        ENCODE_FUNC_CALL();
        ENCODE_CHK_NULL_RETURN(cmdBuffer);

        BSBuffer bsBuffer = {};
        ENCODE_CHK_STATUS_RETURN(m_jpgPkrFeature->PackHuffmanTable(&bsBuffer, tblInd));

        uint32_t byteSize = (bsBuffer.BufferSize + 7) >> 3;
        uint32_t dataBitsInLastDw = bsBuffer.BufferSize % 32;
        if (dataBitsInLastDw == 0)
        {
            dataBitsInLastDw = 32;
        }

        auto &params = m_mfxItf->MHW_GETPAR_F(MFX_PAK_INSERT_OBJECT)();
        params = {};
        params.dwPadding = ((byteSize + 3) >> 2);
        params.bitstreamstartresetResetbitstreamstartingpos = 1;
        params.databitsinlastdwSrcdataendingbitinclusion50 = dataBitsInLastDw;

        m_mfxItf->MHW_ADDCMD_F(MFX_PAK_INSERT_OBJECT)(cmdBuffer);

        // Add actual data
        uint8_t* data = (uint8_t*)(bsBuffer.pBase);
        MOS_STATUS eStatus = Mhw_AddCommandCmdOrBB(m_osInterface, cmdBuffer, nullptr, data, byteSize);

        MOS_SafeFreeMemory(bsBuffer.pBase);

        return eStatus;
    }

    MOS_STATUS JpegPkt::AddRestartInterval(PMOS_COMMAND_BUFFER cmdBuffer) const
    {
        ENCODE_FUNC_CALL();
        ENCODE_CHK_NULL_RETURN(cmdBuffer);

        BSBuffer bsBuffer = {};
        ENCODE_CHK_STATUS_RETURN(m_jpgPkrFeature->PackRestartInterval(&bsBuffer));

        uint32_t byteSize = (bsBuffer.BufferSize + 7) >> 3;
        uint32_t dataBitsInLastDw = bsBuffer.BufferSize % 32;
        if (dataBitsInLastDw == 0)
        {
            dataBitsInLastDw = 32;
        }

        auto &params = m_mfxItf->MHW_GETPAR_F(MFX_PAK_INSERT_OBJECT)();
        params = {};
        params.dwPadding = ((byteSize + 3) >> 2);
        params.bitstreamstartresetResetbitstreamstartingpos = 1;
        params.databitsinlastdwSrcdataendingbitinclusion50 = dataBitsInLastDw;

        m_mfxItf->MHW_ADDCMD_F(MFX_PAK_INSERT_OBJECT)(cmdBuffer);

        // Add actual data
        uint8_t* data = (uint8_t*)(bsBuffer.pBase);
        MOS_STATUS eStatus = Mhw_AddCommandCmdOrBB(m_osInterface, cmdBuffer, nullptr, data, byteSize);

        MOS_SafeFreeMemory(bsBuffer.pBase);

        return eStatus;
    }

    MOS_STATUS JpegPkt::AddScanHeader(PMOS_COMMAND_BUFFER cmdBuffer) const
    {
        ENCODE_FUNC_CALL();
        ENCODE_CHK_NULL_RETURN(cmdBuffer);

        BSBuffer bsBuffer = {};
        ENCODE_CHK_STATUS_RETURN(m_jpgPkrFeature->PackScanHeader(&bsBuffer));

        uint32_t byteSize = (bsBuffer.BufferSize + 7) >> 3;
        uint32_t dataBitsInLastDw = bsBuffer.BufferSize % 32;
        if (dataBitsInLastDw == 0)
        {
            dataBitsInLastDw = 32;
        }

        auto &params = m_mfxItf->MHW_GETPAR_F(MFX_PAK_INSERT_OBJECT)();
        params = {};
        params.dwPadding = ((byteSize + 3) >> 2);
        params.bitstreamstartresetResetbitstreamstartingpos = 1;
        params.endofsliceflagLastdstdatainsertcommandflag = true;
        params.lastheaderflagLastsrcheaderdatainsertcommandflag = true;
        params.databitsinlastdwSrcdataendingbitinclusion50 = dataBitsInLastDw;

        m_mfxItf->MHW_ADDCMD_F(MFX_PAK_INSERT_OBJECT)(cmdBuffer);

        // Add actual data
        uint8_t* data = (uint8_t*)(bsBuffer.pBase);
        MOS_STATUS eStatus = Mhw_AddCommandCmdOrBB(m_osInterface, cmdBuffer, nullptr, data, byteSize);

        MOS_SafeFreeMemory(bsBuffer.pBase);

        return eStatus;
    }

    // Implemented based on table K.5 in JPEG spec
    uint8_t JpegPkt::MapHuffValIndex(uint8_t huffValIndex)
    {
        ENCODE_FUNC_CALL();

        uint8_t mappedIndex = 0;

        if (huffValIndex < 0xF0)
        {
            mappedIndex = (((huffValIndex >> 4) & 0x0F) * 0xA) + (huffValIndex & 0x0F);
        }
        else
        {
            mappedIndex = (((huffValIndex >> 4) & 0x0F) * 0xA) + (huffValIndex & 0x0F) + 1;
        }

        return mappedIndex;
    }

    // Implemented based on Flowchart in figure C.1 in JPEG spec
    MOS_STATUS JpegPkt::GenerateSizeTable(
        uint8_t  bits[],
        uint8_t  huffSize[],
        uint8_t &lastK)
    {
        ENCODE_FUNC_CALL();

        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        uint8_t i = 1, j = 1;
        uint8_t k = 0;
        while (i <= 16)
        {
            while (j <= (int8_t)bits[i - 1])  // bits index is from 0 to 15
            {
                huffSize[k] = i;
                k           = k + 1;
                j           = j + 1;
            }

            i++;
            j = 1;
        }

        huffSize[k] = 0;
        lastK       = k;

        return eStatus;
    }

    // Implemented based on Flowchart in figure C.2 in JPEG spec
    MOS_STATUS JpegPkt::GenerateCodeTable(
        uint8_t  huffSize[],
        uint16_t huffCode[])
    {
        ENCODE_FUNC_CALL();

        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        uint8_t  k    = 0;
        uint8_t  si   = huffSize[0];
        uint16_t code = 0;
        while (huffSize[k] != 0)
        {
            while (huffSize[k] == si)
            {
                if (code == 0xFFFF)
                {
                    // Invalid code generated - replace with all zeroes
                    code = 0x0000;
                }

                huffCode[k] = code;
                code        = code + 1;
                k           = k + 1;
            }

            code <<= 1;
            si = si + 1;
        }

        return eStatus;
    }

    // Implemented based on Flowchart in figure C.3 in JPEG spec
    MOS_STATUS JpegPkt::OrderCodes(
        uint8_t  huffVal[],
        uint8_t  huffSize[],
        uint16_t huffCode[],
        uint8_t  lastK)
    {
        ENCODE_FUNC_CALL();

        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        uint16_t eHuffCo[JPEG_NUM_HUFF_TABLE_AC_HUFFVAL];
        uint8_t  eHuffSi[JPEG_NUM_HUFF_TABLE_AC_HUFFVAL];
        MOS_ZeroMemory(&eHuffCo[0], JPEG_NUM_HUFF_TABLE_AC_HUFFVAL * sizeof(uint16_t));
        MOS_ZeroMemory(&eHuffSi[0], JPEG_NUM_HUFF_TABLE_AC_HUFFVAL * sizeof(uint8_t));

        uint8_t k = 0;
        do
        {
            uint8_t i = MapHuffValIndex((uint8_t)huffVal[k]);
            if (i >= JPEG_NUM_HUFF_TABLE_AC_HUFFVAL)
            {
                ENCODE_ASSERT(false);
                return MOS_STATUS_UNKNOWN;
            }
            eHuffCo[i] = huffCode[k];
            eHuffSi[i] = huffSize[k];
            k++;
        } while (k < lastK);

        // copy over the first 162 values of reordered arrays to Huffman Code and size arrays
        ENCODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(&huffCode[0], JPEG_NUM_HUFF_TABLE_AC_HUFFVAL * sizeof(uint16_t), &eHuffCo[0], JPEG_NUM_HUFF_TABLE_AC_HUFFVAL * sizeof(uint16_t)));
        ENCODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(&huffSize[0], JPEG_NUM_HUFF_TABLE_AC_HUFFVAL * sizeof(uint8_t), &eHuffSi[0], JPEG_NUM_HUFF_TABLE_AC_HUFFVAL * sizeof(uint8_t)));

        return eStatus;
    }

    MOS_STATUS JpegPkt::ConvertHuffDataToTable(
        CodecEncodeJpegHuffData      huffmanData,
        EncodeJpegHuffTable *   huffmanTable)
    {
        ENCODE_FUNC_CALL();

        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        huffmanTable->m_tableClass = huffmanData.m_tableClass;
        huffmanTable->m_tableID    = huffmanData.m_tableID;

        uint8_t lastK = 0;

        // Step 1 : Generate size table
        ENCODE_CHK_STATUS_RETURN(GenerateSizeTable(huffmanData.m_bits, huffmanTable->m_huffSize, lastK));

        // Step2: Generate code table
        ENCODE_CHK_STATUS_RETURN(GenerateCodeTable(huffmanTable->m_huffSize, huffmanTable->m_huffCode));

        // Step 3: Order codes
        ENCODE_CHK_STATUS_RETURN(OrderCodes(huffmanData.m_huffVal, huffmanTable->m_huffSize, huffmanTable->m_huffCode, lastK));

        return eStatus;
    }

    uint32_t JpegPkt::CalculateCommandBufferSize()
    {
        ENCODE_FUNC_CALL();
        uint32_t commandBufferSize =
            m_pictureStatesSize        +
            (m_sliceStatesSize * m_basicFeature->m_numSlices);

        // For JPEG encoder, add the size of PAK_INSERT_OBJ commands which is also part of command buffer
        if (m_basicFeature->m_standard == CODECHAL_JPEG)
        {
            // Add PAK_INSERT_OBJ for app data
            // PAK_INSERT_OBJ contains 2 DWORDS + bytes of payload data
            // There is a max of 1024 payload bytes of app data per PAK_INSERT_OBJ command, so adding 2 DWORDS for each of them
            // Total payload data is the same size as app data
            commandBufferSize += (m_basicFeature->m_appDataSize + (2 * sizeof(uint32_t) * (m_basicFeature->m_appDataSize / 1020 + 1)));  //to be consistent with how we split app data into chunks.

            // Add number of bytes of data added through PAK_INSERT_OBJ command
            commandBufferSize += (2 + // SOI = 2 bytes
                    // Frame header - add sizes of each component of CodechalEncodeJpegFrameHeader
                    (2 * sizeof(uint8_t)) + (4 * sizeof(uint16_t)) + 3 * sizeof(uint8_t)*  jpegNumComponent +
                    // AC and DC Huffman tables - 2 Huffman tables for each component, and 3 components
                    (2 * 3 * sizeof(EncodeJpegHuffmanHeader)) +
                    // Quant tables - 1 for Quant table of each component, so 3 quant tables per frame
                    (3 * sizeof(EncodeJpegQuantHeader)) +
                    // Restart interval - 1 per frame
                    sizeof(EncodeJpegRestartHeader) +
                    // Scan header - 1 per frame
                    sizeof(EncodeJpegScanHeader));
        }

        if (m_pipeline->IsSingleTaskPhaseSupported())
        {
            commandBufferSize *= (m_pipeline->GetPassNum() + 1);
        }

        // 4K align since allocation is in chunks of 4K bytes.
        commandBufferSize = MOS_ALIGN_CEIL(commandBufferSize, 0x1000);

        return commandBufferSize;
    }

    uint32_t JpegPkt::CalculatePatchListSize()
    {
        ENCODE_FUNC_CALL();
        uint32_t requestedPatchListSize = 0;
        if (m_usePatchList)
        {
            requestedPatchListSize =
                m_picturePatchListSize +
                (m_slicePatchListSize * m_basicFeature->m_numSlices);

            if (m_pipeline->IsSingleTaskPhaseSupported())
            {
                requestedPatchListSize *= m_pipeline->GetPassNum();
            }
        }
        return requestedPatchListSize;
    }

    MOS_STATUS JpegPkt::AddAllCmds_MFX_FQM_STATE(PMOS_COMMAND_BUFFER cmdBuffer) const
    {
        ENCODE_FUNC_CALL();
        ENCODE_CHK_NULL_RETURN(cmdBuffer);

        auto &params = m_mfxItf->MHW_GETPAR_F(MFX_FQM_STATE)();

        for (uint8_t i = 0; i < m_numQuantTables; i++)
        {
            params = {};
            params.qmType = i;

            auto j = 0;
            // Copy over 32 uint32_t worth of values - Each uint32_t will contain 2 16 bit quantizer values
            // where for the DWordx Bits [15: 0] = 1/QM[0][x] Bits[32:16] = 1/QM[1][x]
            for (auto k = 0; k < 8; k++)
            {
                for (auto l = k; l < 64; l += 16)
                {
                    params.quantizermatrix[j] = ((GetReciprocalScalingValue(m_jpegQuantMatrix.m_quantMatrix[i][l + 8]) << 16) |
                                                  GetReciprocalScalingValue(m_jpegQuantMatrix.m_quantMatrix[i][l]));
                    j++;
                }
            }

            m_mfxItf->MHW_ADDCMD_F(MFX_FQM_STATE)(cmdBuffer);
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS JpegPkt::AddAllCmds_MFC_JPEG_HUFF_TABLE_STATE(PMOS_COMMAND_BUFFER cmdBuffer) const
    {
        ENCODE_FUNC_CALL();
        ENCODE_CHK_NULL_RETURN(cmdBuffer);

        auto &params = m_mfxItf->MHW_GETPAR_F(MFC_JPEG_HUFF_TABLE_STATE)();

        // the number of huffman commands is half of the huffman buffers sent by the app, since AC and DC buffers are combined into one command
        for (uint32_t i = 0; i < m_numHuffBuffers / 2; i++)
        {
            params = {};
            params.huffTableId = (uint8_t)m_huffTableParams[i].HuffTableID;

            // cmd DWORDS 2:13 for DC Table
            // Format- 3Bytes: Byte0 for Code length, Byte1 and Byte2 for Code word, and Byte3 for dummy
            for (auto j = 0; j < JPEG_NUM_HUFF_TABLE_DC_HUFFVAL; j++)
            {
                params.dcTable[j] = 0;
                params.dcTable[j] = (m_huffTableParams[i].pDCCodeLength[j] & 0xFF) |
                                    ((m_huffTableParams[i].pDCCodeValues[j] & 0xFFFF) << 8);
            }

            // cmd DWORDS 14:175 for AC table
            // Format- 3Bytes: Byte0 for Code length, Byte1 and Byte2 for Code word, and Byte3 for dummy
            for (auto j = 0; j < JPEG_NUM_HUFF_TABLE_AC_HUFFVAL; j++)
            {
                params.acTable[j] = 0;
                params.acTable[j] = (m_huffTableParams[i].pACCodeLength[j] & 0xFF) |
                                    ((m_huffTableParams[i].pACCodeValues[j] & 0xFFFF) << 8);
            }

            if (m_repeatHuffTable)
            {
                m_mfxItf->MHW_ADDCMD_F(MFC_JPEG_HUFF_TABLE_STATE)(cmdBuffer);
            }
            m_mfxItf->MHW_ADDCMD_F(MFC_JPEG_HUFF_TABLE_STATE)(cmdBuffer);
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS JpegPkt::AddAllCmds_MFX_PAK_INSERT_OBJECT(PMOS_COMMAND_BUFFER cmdBuffer) const
    {
        ENCODE_FUNC_CALL();
        ENCODE_CHK_NULL_RETURN(cmdBuffer);

        MOS_SURFACE *surface = m_basicFeature->m_rawSurfaceToPak;
        bool useSingleDefaultQuantTable = (m_basicFeature->m_jpegQuantMatrixSent == false &&
                                           ((surface->Format == Format_A8R8G8B8) ||
                                            (surface->Format == Format_X8R8G8B8) ||
                                            (surface->Format == Format_A8B8G8R8) ||
                                            (surface->Format == Format_X8B8G8R8)));

        if (!m_basicFeature->m_fullHeaderInAppData)
        {
            // Add SOI (0xFFD8) (only if it was sent by the application)
            ENCODE_CHK_STATUS_RETURN(AddSOI(cmdBuffer));
        }
        // Add Application data if it was sent by application
        if (m_applicationData != nullptr)
        {
            ENCODE_CHK_STATUS_RETURN(AddApplicationData(cmdBuffer));
        }
        if (!m_basicFeature->m_fullHeaderInAppData)
        {
            ENCODE_CHK_STATUS_RETURN(AddQuantTable(cmdBuffer, useSingleDefaultQuantTable));

            ENCODE_CHK_STATUS_RETURN(AddFrameHeader(cmdBuffer, useSingleDefaultQuantTable));

            // Add Huffman Table for Y - DC table, Y- AC table, U/V - DC table, U/V - AC table
            for (uint32_t i = 0; i < m_numHuffBuffers; i++)
            {
                ENCODE_CHK_STATUS_RETURN(AddHuffmanTable(cmdBuffer, i));
            }

            // Restart Interval - Add only if the restart interval is not zero
            if (m_jpegScanParams->m_restartInterval != 0)
            {
                ENCODE_CHK_STATUS_RETURN(AddRestartInterval(cmdBuffer));
            }

            ENCODE_CHK_STATUS_RETURN(AddScanHeader(cmdBuffer));
        }

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(MI_STORE_REGISTER_MEM, JpegPkt)
    {
        params.presStoreBuffer = m_pResource;
        params.dwOffset        = m_dwOffset;
        params.dwRegister      = m_dwValue;

        return MOS_STATUS_SUCCESS;
    }

#if USE_CODECHAL_DEBUG_TOOL
    MOS_STATUS JpegPkt::DumpResources(
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

        if (m_jpegPicParams)
        {
            ENCODE_CHK_STATUS_RETURN(DumpPicParams(
                m_jpegPicParams));
        }

        if (m_jpegScanParams)
        {
            ENCODE_CHK_STATUS_RETURN(DumpScanParams(
                m_jpegScanParams));
        }

        if (m_jpegHuffmanTable)
        {
            ENCODE_CHK_STATUS_RETURN(DumpHuffmanTable(
                m_jpegHuffmanTable));
        }

        if (m_jpegQuantTables)
        {
            ENCODE_CHK_STATUS_RETURN(DumpQuantTables(
                m_jpegQuantTables));
        }

        CodechalDebugInterface *debugInterface = m_pipeline->GetStatusReportDebugInterface();
        ENCODE_CHK_NULL_RETURN(debugInterface);

        CODEC_REF_LIST currRefList = *((CODEC_REF_LIST *)statusReportData->currRefList);
        currRefList.RefPic         = statusReportData->currOriginalPic;

        debugInterface->m_currPic            = statusReportData->currOriginalPic;
        debugInterface->m_bufferDumpFrameNum = m_statusReport->GetReportedCount() + 1;  // ToDo: for debug purpose
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

        ENCODE_CHK_STATUS_RETURN(debugInterface->DumpYUVSurface(
            &currRefList.sRefRawBuffer,
            CodechalDbgAttr::attrEncodeRawInputSurface,
            "SrcSurf"))

        ENCODE_CHK_STATUS_RETURN(debugInterface->DumpEncodeStatusReport(
            statusReportData));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS JpegPkt::DumpHuffmanTable(
        CodecEncodeJpegHuffmanDataArray *huffmanTable)
    {
        ENCODE_FUNC_CALL();

        if (!m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrHuffmanTbl))
        {
            return MOS_STATUS_SUCCESS;
        }

        ENCODE_CHK_NULL_RETURN(huffmanTable);

        std::ostringstream oss;
        oss.setf(std::ios::showbase | std::ios::uppercase);

        //Dump HuffTable[JPEG_MAX_NUM_HUFF_TABLE_INDEX]
        for (uint32_t i = 0; i < JPEG_NUM_ENCODE_HUFF_BUFF; ++i)
        {
            // Dump Table Class
            oss << "TableClass: " << +huffmanTable->m_huffmanData[i].m_tableClass << std::endl;
            // Dump Table ID
            oss << "TableID: " << +huffmanTable->m_huffmanData[i].m_tableID << std::endl;
            //Dump ucBits[JPEG_NUM_HUFF_TABLE_AC_BITS]
            oss << "HuffTable[" << +i << "].ucBits[0-" << (JPEG_NUM_HUFF_TABLE_AC_BITS - 1) << "]: " << std::endl;

            for (uint32_t j = 0; j < JPEG_NUM_HUFF_TABLE_AC_BITS; ++j)
            {
                oss << +huffmanTable->m_huffmanData[i].m_bits[j];
                if (j % 6 == 5 || j == JPEG_NUM_HUFF_TABLE_AC_BITS - 1)
                {
                    oss << std::endl;
                }
            }

            //Dump ucHuffVal[JPEG_NUM_HUFF_TABLE_AC_HUFFVAL]
            oss << "HuffTable[" << +i << "].ucHuffVal[0-" << (JPEG_NUM_HUFF_TABLE_AC_HUFFVAL - 1) << "]: " << std::endl;

            for (uint32_t j = 0; j < JPEG_NUM_HUFF_TABLE_AC_HUFFVAL; ++j)
            {
                oss << +huffmanTable->m_huffmanData[i].m_huffVal[j];
                if (j % 6 == 5 || j == JPEG_NUM_HUFF_TABLE_AC_HUFFVAL - 1)
                {
                    oss << std::endl;
                }
            }
        }

        const char *fileName = m_debugInterface->CreateFileName(
            "_ENC",
            CodechalDbgBufferType::bufHuffmanTbl,
            CodechalDbgExtType::txt);

        std::ofstream ofs(fileName, std::ios::out);
        ofs << oss.str();
        ofs.close();
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS JpegPkt::DumpPicParams(
        CodecEncodeJpegPictureParams *picParams)
    {
        ENCODE_FUNC_CALL();
        if (!m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrPicParams))
        {
            return MOS_STATUS_SUCCESS;
        }

        ENCODE_CHK_NULL_RETURN(picParams);

        std::ostringstream oss;
        oss.setf(std::ios::showbase | std::ios::uppercase);

        oss << "Profile: " << +picParams->m_profile << std::endl;
        oss << "Progressive: " << +picParams->m_progressive << std::endl;
        oss << "Huffman: " << +picParams->m_huffman << std::endl;
        oss << "Interleaved: " << +picParams->m_interleaved << std::endl;
        oss << "Differential: " << +picParams->m_differential << std::endl;
        oss << "PicWidth: " << +picParams->m_picWidth << std::endl;
        oss << "PicHeight: " << +picParams->m_picHeight << std::endl;
        oss << "InputSurfaceFormat: " << +picParams->m_inputSurfaceFormat << std::endl;
        oss << "SampleBitDepth: " << +picParams->m_sampleBitDepth << std::endl;
        oss << "uiNumComponent: " << +picParams->m_numComponent << std::endl;

        //Dump componentIdentifier[jpegNumComponent]
        for (uint32_t i = 0; i < jpegNumComponent; ++i)
        {
            oss << "ComponentIdentifier[" << +i << "]: " << +picParams->m_componentID[i] << std::endl;
        }

        //Dump quantTableSelector[jpegNumComponent]
        for (uint32_t i = 0; i < jpegNumComponent; ++i)
        {
            oss << "QuantTableSelector[" << +i << "]: " << +picParams->m_quantTableSelector[i] << std::endl;
        }
        oss << "Quality: " << +picParams->m_quality << std::endl;
        oss << "NumScan: " << +picParams->m_numScan << std::endl;
        oss << "NumQuantTable: " << +picParams->m_numQuantTable << std::endl;
        oss << "NumCodingTable: " << +picParams->m_numCodingTable << std::endl;
        oss << "StatusReportFeedbackNumber: " << +picParams->m_statusReportFeedbackNumber << std::endl;

        const char *fileName = m_debugInterface->CreateFileName(
            "_ENC",
            CodechalDbgBufferType::bufPicParams,
            CodechalDbgExtType::txt);

        std::ofstream ofs(fileName, std::ios::out);
        ofs << oss.str();
        ofs.close();
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS JpegPkt::DumpScanParams(
        CodecEncodeJpegScanHeader *scanParams)
    {
        ENCODE_FUNC_CALL();

        if (!m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrScanParams))
        {
            return MOS_STATUS_SUCCESS;
        }

        ENCODE_CHK_NULL_RETURN(scanParams);

        std::ostringstream oss;
        oss.setf(std::ios::showbase | std::ios::uppercase);

        oss << "RestartInterval: " << +scanParams->m_restartInterval << std::endl;
        oss << "NumComponents: " << +scanParams->m_numComponent << std::endl;

        //Dump ComponentSelector[jpegNumComponent]
        for (uint32_t i = 0; i < jpegNumComponent; ++i)
        {
            oss << "ComponentSelector[" << +i << "]: " << +scanParams->m_componentSelector[i] << std::endl;
        }

        //Dump DcHuffTblSelector[jpegNumComponent]
        for (uint32_t i = 0; i < jpegNumComponent; ++i)
        {
            oss << "DcHuffTblSelector[" << +i << "]: " << +scanParams->m_dcCodingTblSelector[i] << std::endl;
        }

        //Dump AcHuffTblSelector[jpegNumComponent]
        for (uint32_t i = 0; i < jpegNumComponent; ++i)
        {
            oss << "AcHuffTblSelector[" << +i << "]: " << +scanParams->m_acCodingTblSelector[i] << std::endl;
        }

        const char *fileName = m_debugInterface->CreateFileName(
            "_ENC",
            CodechalDbgBufferType::bufScanParams,
            CodechalDbgExtType::txt);

        std::ofstream ofs(fileName, std::ios::out);
        ofs << oss.str();
        ofs.close();

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS JpegPkt::DumpQuantTables(
        CodecEncodeJpegQuantTable *quantTable)
    {
        ENCODE_FUNC_CALL();

        if (!m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrIqParams))
        {
            return MOS_STATUS_SUCCESS;
        }

        ENCODE_CHK_NULL_RETURN(quantTable);

        std::ostringstream oss;
        oss.setf(std::ios::showbase | std::ios::uppercase);

        //Dump quantTable[JPEG_MAX_NUM_QUANT_TABLE_INDEX]
        for (uint32_t i = 0; i < JPEG_MAX_NUM_QUANT_TABLE_INDEX; ++i)
        {
            // Dump Table ID
            oss << "TableID: " << +quantTable->m_quantTable[i].m_tableID << std::endl;
            // Dump Precision
            oss << "TableID: " << +quantTable->m_quantTable[i].m_precision << std::endl;
            //Dump usQm[JPEG_NUM_QUANTMATRIX];
            oss << "quantTable[" << +i << "].usQm[0-" << (JPEG_NUM_QUANTMATRIX - 1) << "]: " << std::endl;

            for (uint32_t j = 0; j < JPEG_NUM_QUANTMATRIX; ++j)
            {
                oss << +quantTable->m_quantTable[i].m_qm[j];
                if (j % 6 == 5 || j == JPEG_NUM_QUANTMATRIX - 1)
                {
                    oss << std::endl;
                }
            }
        }
        const char *fileName = m_debugInterface->CreateFileName(
            "_ENC",
            CodechalDbgBufferType::bufIqParams,
            CodechalDbgExtType::txt);

        std::ofstream ofs(fileName, std::ios::out);
        ofs << oss.str();
        ofs.close();

        return MOS_STATUS_SUCCESS;
    }
#endif

}
