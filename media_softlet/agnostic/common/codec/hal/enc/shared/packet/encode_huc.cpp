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
//! \file     encode_huc.cpp
//! \brief    Defines the common interface for encode huc implementation
//! \details  The encode huc interface is further sub-divided by different huc usage,
//!           this file is for the base interface which is shared by all.
//!

#include "encode_huc.h"
#include "codechal_debug.h"
#include "codechal_encoder_base.h"
#include <algorithm>

namespace encode
{
    MOS_STATUS EncodeHucBasic::Init()
    {
        HUC_CHK_STATUS_RETURN(CmdPacket::Init());
        m_allocator = m_pipeline->GetEncodeAllocator();
        HUC_CHK_STATUS_RETURN(AllocateResources());

        ENCODE_CHK_NULL_RETURN(m_pipeline);
        m_statusReport = m_pipeline->GetStatusReportInstance();
        ENCODE_CHK_NULL_RETURN(m_statusReport);
        ENCODE_CHK_STATUS_RETURN(m_statusReport->RegistObserver(this));

        HUC_CHK_NULL_RETURN(m_hwInterface);
        m_skuFtrEnableMediaKernels = MEDIA_IS_SKU(m_hwInterface->GetSkuTable(), FtrEnableMediaKernels);
        HUC_CHK_NULL_RETURN(m_hucInterface);
        m_hucStatus2ImemLoadedMask = m_hucInterface->GetHucStatus2ImemLoadedMask();

        return MOS_STATUS_SUCCESS;
    }

#if _SW_BRC
    MOS_STATUS EncodeHucBasic::InitSwBrc(HuCFunction function)
    {
        if (m_swBrc == nullptr)
        {
            EncodeBasicFeature* basicFeature = dynamic_cast<EncodeBasicFeature*>(m_featureManager->GetFeature(FeatureIDs::basicFeature));
            HUC_CHK_NULL_RETURN(basicFeature);
            m_swBrc = EncodeSwBrc::CreateFactory(basicFeature->m_mode, m_allocator, m_hwInterface->GetOsInterface(), function);
        }
        return MOS_STATUS_SUCCESS;
    }
 #endif  // !_SW_BRC

    bool EncodeHucBasic::IsHuCStsUpdNeeded()
    {
        bool enabled = false;
#if _SW_BRC
        enabled = m_swBrc && m_swBrc->SwBrcEnabled();
 #endif  // !_SW_BRC
        return enabled;
    }

    MOS_STATUS EncodeHucBasic::AllocateResources()
    {
        MOS_ALLOC_GFXRES_PARAMS allocParamsForBufferLinear;
        MOS_ZeroMemory(&allocParamsForBufferLinear, sizeof(MOS_ALLOC_GFXRES_PARAMS));
        allocParamsForBufferLinear.Type = MOS_GFXRES_BUFFER;
        allocParamsForBufferLinear.TileType = MOS_TILE_LINEAR;
        allocParamsForBufferLinear.Format = Format_Buffer;

        // HUC STATUS 2 Buffer for HuC status check in COND_BB_END
        allocParamsForBufferLinear.dwBytes = sizeof(uint64_t);
        allocParamsForBufferLinear.pBufName = "HUC STATUS 2 Buffer";
        MOS_RESOURCE *allocatedbuffer       = m_allocator->AllocateResource(allocParamsForBufferLinear, true);
        ENCODE_CHK_NULL_RETURN(allocatedbuffer);
        m_resHucStatus2Buffer = *allocatedbuffer;

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS EncodeHucBasic::Execute(PMOS_COMMAND_BUFFER cmdBuffer, bool storeHucStatus2Needed, bool prologNeeded, HuCFunction function)
    {
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS EncodeHucBasic::SetHucPipeModeSelectParameters()
    {
        HUC_CHK_NULL_RETURN(m_featureManager);
        EncodeBasicFeature* basicFeature = dynamic_cast<EncodeBasicFeature*>(m_featureManager->GetFeature(FeatureIDs::basicFeature));
        HUC_CHK_NULL_RETURN(basicFeature);

        MOS_ZeroMemory(&m_pipeModeSelectParams, sizeof(m_pipeModeSelectParams));
        m_pipeModeSelectParams.Mode = basicFeature->m_mode;
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS EncodeHucBasic::SetVdPipeFlushParameters()
    {
        MOS_ZeroMemory(&m_vdPipeFlushParams, sizeof(m_vdPipeFlushParams));
        m_vdPipeFlushParams.Flags.bFlushHEVC = 1;
        m_vdPipeFlushParams.Flags.bWaitDoneHEVC = 1;
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS EncodeHucBasic::StoreHuCStatus2Register(PMOS_COMMAND_BUFFER cmdBuffer, bool storeHucStatus2Needed)
    {
        HUC_CHK_NULL_RETURN(cmdBuffer);
        HUC_CHK_NULL_RETURN(m_statusReport);
        HUC_CHK_NULL_RETURN(m_hucInterface);

        auto mmioRegisters = m_hucInterface->GetMmioRegisters(m_vdboxIndex);
        HUC_CHK_NULL_RETURN(mmioRegisters);

        PMOS_RESOURCE osResource = nullptr;
        uint32_t      offset     = 0;

        ENCODE_CHK_STATUS_RETURN(m_statusReport->GetAddress(statusReportHucStatus2Reg, osResource, offset));

        auto &storeRegParams             = m_miItf->MHW_GETPAR_F(MI_STORE_REGISTER_MEM)();
        storeRegParams                   = {};
        storeRegParams.presStoreBuffer   = osResource;
        storeRegParams.dwOffset          = offset;
        storeRegParams.dwRegister        = mmioRegisters->hucStatus2RegOffset;
        HUC_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_STORE_REGISTER_MEM)(cmdBuffer));

        if (storeHucStatus2Needed)
        {
            // Write HUC_STATUS2 mask - bit 6 - valid IMEM loaded
            auto &storeDataParams            = m_miItf->MHW_GETPAR_F(MI_STORE_DATA_IMM)();
            storeDataParams                  = {};
            storeDataParams.pOsResource      = &m_resHucStatus2Buffer;
            storeDataParams.dwResourceOffset = 0;
            storeDataParams.dwValue          = m_hucInterface->GetHucStatus2ImemLoadedMask();
            HUC_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_STORE_DATA_IMM)(cmdBuffer));

            // Store HUC_STATUS2 register
            auto &storeRegParams             = m_miItf->MHW_GETPAR_F(MI_STORE_REGISTER_MEM)();
            storeRegParams                   = {};
            storeRegParams.presStoreBuffer   = &m_resHucStatus2Buffer;
            storeRegParams.dwOffset          = sizeof(uint32_t);
            storeRegParams.dwRegister        = mmioRegisters->hucStatus2RegOffset;
            HUC_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_STORE_REGISTER_MEM)(cmdBuffer));
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS EncodeHucBasic::SendPrologCmds(
        MOS_COMMAND_BUFFER &cmdBuffer)
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        ENCODE_FUNC_CALL();
        bool mmcEnabled = false;
#ifdef _MMC_SUPPORTED
        EncodeMemComp *mmcState = m_pipeline->GetMmcState();
        ENCODE_CHK_NULL_RETURN(mmcState);
        mmcEnabled = mmcState->IsMmcEnabled();
        ENCODE_CHK_STATUS_RETURN(mmcState->SendPrologCmd(&cmdBuffer, false));
#endif

        MHW_GENERIC_PROLOG_PARAMS genericPrologParams;
        MOS_ZeroMemory(&genericPrologParams, sizeof(genericPrologParams));
        ENCODE_CHK_NULL_RETURN(m_hwInterface);
        genericPrologParams.pOsInterface = m_hwInterface->GetOsInterface();
        ENCODE_CHK_NULL_RETURN(m_hwInterface->GetMiInterface());
        std::shared_ptr<void> m_miItf = m_hwInterface->GetMiInterface()->GetNewMiInterface();
        genericPrologParams.pvMiInterface = nullptr;
        genericPrologParams.bMmcEnabled   = mmcEnabled;
        ENCODE_CHK_STATUS_RETURN(Mhw_SendGenericPrologCmdNext(&cmdBuffer, &genericPrologParams, m_miItf));

        return eStatus;
    }

    MOS_STATUS EncodeHucBasic::AddForceWakeup(MOS_COMMAND_BUFFER &cmdBuffer)
    {
        ENCODE_FUNC_CALL();

        auto &forceWakeupParams                     = m_miItf->MHW_GETPAR_F(MI_FORCE_WAKEUP)();
        forceWakeupParams                           = {};
        forceWakeupParams.bMFXPowerWellControl      = true;
        forceWakeupParams.bMFXPowerWellControlMask  = true;
        forceWakeupParams.bHEVCPowerWellControl     = true;
        forceWakeupParams.bHEVCPowerWellControlMask = true;

        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_FORCE_WAKEUP)(&cmdBuffer));

        return MOS_STATUS_SUCCESS;
    }

    void EncodeHucBasic::SetPerfTag(uint16_t type, uint16_t mode, uint16_t picCodingType)
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

    MOS_STATUS EncodeHucBasic::StartPerfCollect(MOS_COMMAND_BUFFER& cmdBuffer)
    {
        MediaPerfProfiler *perfProfiler = MediaPerfProfiler::Instance();
        ENCODE_CHK_NULL_RETURN(perfProfiler);
        ENCODE_CHK_STATUS_RETURN(perfProfiler->AddPerfCollectStartCmd(
            (void *)m_pipeline, m_osInterface, m_miInterface, &cmdBuffer));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS EncodeHucBasic::EndPerfCollect(MOS_COMMAND_BUFFER& cmdBuffer)
    {
        MediaPerfProfiler *perfProfiler = MediaPerfProfiler::Instance();
        ENCODE_CHK_NULL_RETURN(perfProfiler);
        ENCODE_CHK_STATUS_RETURN(perfProfiler->AddPerfCollectEndCmd(
            (void *)m_pipeline, m_osInterface, m_miInterface, &cmdBuffer));

        return MOS_STATUS_SUCCESS;
    }

#if USE_CODECHAL_DEBUG_TOOL
    MOS_STATUS EncodeHucBasic::DumpRegion(
        uint32_t regionNum,
        const char *regionName,
        bool inputBuffer,
        CodechalHucRegionDumpType dumpType,
        uint32_t size)
    {
        auto bufferToDump = m_virtualAddrParams.regionParams[regionNum].presRegion;
        auto offset       = m_virtualAddrParams.regionParams[regionNum].dwOffset;

        CodechalDebugInterface *debugInterface = m_pipeline->GetDebugInterface();
        ENCODE_CHK_NULL_RETURN(debugInterface);

        if (bufferToDump)
        {
            // Dump the full region when size = 0 or exceed the region size, else dump the size indicated
            ENCODE_CHK_STATUS_RETURN(debugInterface->DumpHucRegion(
                bufferToDump,
                offset,
                size ? size : bufferToDump->iSize,
                regionNum,
                regionName,
                inputBuffer,
                m_pipeline->GetCurrentPass(),
                dumpType));
        }

        return MOS_STATUS_SUCCESS;
    }
#endif

     MOS_STATUS EncodeHucPkt::AddAllCmds_HUC_PIPE_MODE_SELECT(PMOS_COMMAND_BUFFER cmdBuffer) const
    {
        MHW_MI_CHK_NULL(cmdBuffer);

        //for gen 11, we need to add MFX wait for both KIN and VRT before and after HUC Pipemode select...
        auto &mfxWaitParams                 = m_miItf->MHW_GETPAR_F(MFX_WAIT)();
        mfxWaitParams                       = {};
        mfxWaitParams.iStallVdboxPipeline   = true;
        MHW_MI_CHK_STATUS(m_miItf->MHW_ADDCMD_F(MFX_WAIT)(cmdBuffer));

        SETPAR_AND_ADDCMD(HUC_PIPE_MODE_SELECT, m_hucItf, cmdBuffer);

        //for gen 11, we need to add MFX wait for both KIN and VRT before and after HUC Pipemode select...
        mfxWaitParams                       = {};
        mfxWaitParams.iStallVdboxPipeline   = true;
        MHW_MI_CHK_STATUS(m_miItf->MHW_ADDCMD_F(MFX_WAIT)(cmdBuffer));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS EncodeHucPkt::AddAllCmds_HUC_IMEM_STATE(PMOS_COMMAND_BUFFER cmdBuffer) const
    {
        MHW_MI_CHK_NULL(cmdBuffer);

        SETPAR_AND_ADDCMD(HUC_IMEM_STATE, m_hucItf, cmdBuffer);

        auto &mfxWaitParams                 = m_miItf->MHW_GETPAR_F(MFX_WAIT)();
        mfxWaitParams                       = {};
        mfxWaitParams.iStallVdboxPipeline   = true;
        MHW_MI_CHK_STATUS(m_miItf->MHW_ADDCMD_F(MFX_WAIT)(cmdBuffer));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS EncodeHucPkt::Execute(PMOS_COMMAND_BUFFER cmdBuffer, bool storeHucStatus2Needed, bool prologNeeded, HuCFunction function)
    {
        HUC_CHK_NULL_RETURN(cmdBuffer);

        HUC_CHK_STATUS_RETURN(SetVdPipeFlushParameters());

#if _SW_BRC
        HUC_CHK_STATUS_RETURN(InitSwBrc(function));
        if (m_swBrc && m_swBrc->SwBrcEnabled())
        {
            SETPAR(HUC_DMEM_STATE, m_hucItf);
            SETPAR(HUC_VIRTUAL_ADDR_STATE, m_hucItf);

            LoadLegacyHucRegions();
            LoadLegacyHucDmemParams();

            EncodeBasicFeature *basicFeature = dynamic_cast<EncodeBasicFeature *>(m_featureManager->GetFeature(FeatureIDs::basicFeature));
            HUC_CHK_NULL_RETURN(basicFeature);
            return m_swBrc->SwBrcImpl(
                function,
                m_virtualAddrParams,
                m_dmemParams,
                basicFeature->m_recycleBuf->GetBuffer(VdencBrcPakMmioBuffer, 0));
        }
#endif  // !_SW_BRC

        if (prologNeeded)
        {
            ENCODE_CHK_STATUS_RETURN(AddForceWakeup(*cmdBuffer));
            ENCODE_CHK_STATUS_RETURN(SendPrologCmds(*cmdBuffer));
        }

        ENCODE_CHK_STATUS_RETURN(StartPerfCollect(*cmdBuffer));

        AddAllCmds_HUC_IMEM_STATE(cmdBuffer);
        AddAllCmds_HUC_PIPE_MODE_SELECT(cmdBuffer);

        SETPAR_AND_ADDCMD(HUC_DMEM_STATE, m_hucItf, cmdBuffer);
        SETPAR_AND_ADDCMD(HUC_VIRTUAL_ADDR_STATE, m_hucItf, cmdBuffer);

        m_enableHucStatusReport = true;
        HUC_CHK_STATUS_RETURN(StoreHuCStatus2Register(cmdBuffer, storeHucStatus2Needed));

        SETPAR_AND_ADDCMD(HUC_START, m_hucItf, cmdBuffer);

        LoadLegacyHucRegions();

        CODECHAL_DEBUG_TOOL(
            ENCODE_CHK_STATUS_RETURN(DumpInput());)

        SETPAR_AND_ADDCMD(VD_PIPELINE_FLUSH, m_vdencItf, cmdBuffer);

        // Flush the engine to ensure memory written out
        auto &flushDwParams                         = m_miItf->MHW_GETPAR_F(MI_FLUSH_DW)();
        flushDwParams                               = {};
        flushDwParams.bVideoPipelineCacheInvalidate = true;
        HUC_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_FLUSH_DW)(cmdBuffer));

        ENCODE_CHK_STATUS_RETURN(EndPerfCollect(*cmdBuffer));
        HUC_CHK_STATUS_RETURN(StoreHuCStatusRegister(cmdBuffer));

        return MOS_STATUS_SUCCESS;
    }

    void EncodeHucPkt::LoadLegacyHucRegions()
    {
        auto &params = m_hucItf->MHW_GETPAR_F(HUC_VIRTUAL_ADDR_STATE)();

        m_virtualAddrParams = {};
        std::copy_n(params.regionParams, 16, m_virtualAddrParams.regionParams);
    }

    void EncodeHucPkt::LoadLegacyHucDmemParams()
    {
        auto &params = m_hucItf->MHW_GETPAR_F(HUC_DMEM_STATE)();

        m_dmemParams                   = {};
        m_dmemParams.dwDataLength      = params.dataLength;
        m_dmemParams.dwDmemOffset      = params.dmemOffset;
        m_dmemParams.presHucDataSource = params.hucDataSource;
    }

    MHW_SETPAR_DECL_SRC(VD_PIPELINE_FLUSH, EncodeHucPkt)
    {
        params.waitDoneHEVC = true;
        params.flushHEVC = true;
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS EncodeHucBasic::Completed(void *mfxStatus, void *rcsStatus, void *statusReport)
    {
        ENCODE_FUNC_CALL();

        if (!m_enableHucStatusReport)
        {
            return MOS_STATUS_SUCCESS;
        }

        ENCODE_CHK_NULL_RETURN(mfxStatus);
        ENCODE_CHK_NULL_RETURN(statusReport);

        EncodeStatusMfx *encodeStatusMfx = (EncodeStatusMfx *)mfxStatus;

        MOS_USER_FEATURE_VALUE_WRITE_DATA userFeatureWriteData;
        MOS_ZeroMemory(&userFeatureWriteData, sizeof(MOS_USER_FEATURE_VALUE_WRITE_DATA));

        if (!m_skuFtrEnableMediaKernels)
        {
            ENCODE_ASSERTMESSAGE("Failed to load HuC firmware!");

            // Reporting
            ReportUserSetting(
                "HuC Firmware Load Failed",
                1,
                MediaUserSetting::Group::Sequence,
                m_osInterface->pOsContext);

            return MOS_STATUS_HUC_KERNEL_FAILED;
        }
        else if (!(encodeStatusMfx->hucStatus2Reg & m_hucStatus2ImemLoadedMask))
        {
            ENCODE_ASSERTMESSAGE("HuC status2 indicates Valid Imem Load failed!");

            // Reporting
            ReportUserSetting(
                "HuC Valid Imem Load Failed",
                1,
                MediaUserSetting::Group::Sequence,
                m_osInterface->pOsContext);

#if (_DEBUG || _RELEASE_INTERNAL)
            ReportUserSettingForDebug(
                "Huc Status2 Value",
                encodeStatusMfx->hucStatus2Reg,
                MediaUserSetting::Group::Sequence,
                m_osInterface->pOsContext);
#endif

            return MOS_STATUS_HUC_KERNEL_FAILED;
        }
#if (_DEBUG || _RELEASE_INTERNAL)
        else
        {
            EncodeStatusReportData* encodeStatusReport = (EncodeStatusReportData *)statusReport;
            std::string hucStatusReport = "HuC Status Value is " + std::to_string(encodeStatusMfx->hucStatusReg) +
                " for Frame number #" + std::to_string(encodeStatusReport->statusReportNumber) + ".";
            ENCODE_NORMALMESSAGE(hucStatusReport.c_str());
        }
#endif

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS EncodeHucBasic::StoreHuCStatusRegister(PMOS_COMMAND_BUFFER cmdBuffer)
    {
        HUC_CHK_NULL_RETURN(cmdBuffer);
        HUC_CHK_NULL_RETURN(m_statusReport);
        HUC_CHK_NULL_RETURN(m_miInterface);
        HUC_CHK_NULL_RETURN(m_hucInterface);

        auto mmioRegisters = m_hucInterface->GetMmioRegisters(m_vdboxIndex);
        HUC_CHK_NULL_RETURN(mmioRegisters);

        PMOS_RESOURCE osResource = nullptr;
        uint32_t      offset     = 0;

        ENCODE_CHK_STATUS_RETURN(m_statusReport->GetAddress(statusReportHucStatusReg, osResource, offset));

        // Store HUC_STATUS register
        auto &storeRegParams             = m_miItf->MHW_GETPAR_F(MI_STORE_REGISTER_MEM)();
        storeRegParams                   = {};
        storeRegParams.presStoreBuffer   = osResource;
        storeRegParams.dwOffset          = offset;
        storeRegParams.dwRegister        = mmioRegisters->hucStatusRegOffset;
        HUC_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_STORE_REGISTER_MEM)(cmdBuffer));

        return MOS_STATUS_SUCCESS;
    }
}
