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
//! \file     encode_huc_la_update_packet.cpp
//! \brief    Defines the implementation of Huc Lookahead update packet
//!

#include "encode_huc_la_update_packet.h"
#include "encode_vdenc_lpla_analysis.h"

namespace encode
{
    MOS_STATUS HucLaUpdatePkt::Init()
    {
        ENCODE_FUNC_CALL();
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        HUC_CHK_STATUS_RETURN(EncodeHucPkt::Init());

        ENCODE_CHK_NULL_RETURN(m_featureManager);
        m_basicFeature = dynamic_cast<HevcBasicFeature *>(m_featureManager->GetFeature(HevcFeatureIDs::basicFeature));
        ENCODE_CHK_NULL_RETURN(m_basicFeature);

        return eStatus;
    }

    MOS_STATUS HucLaUpdatePkt::AllocateResources()
    {
        ENCODE_FUNC_CALL();
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        ENCODE_CHK_STATUS_RETURN(EncodeHucPkt::AllocateResources());

        return eStatus;
    }

    MOS_STATUS HucLaUpdatePkt::Submit(MOS_COMMAND_BUFFER *commandBuffer, uint8_t packetPhase)
    {
        ENCODE_FUNC_CALL();

        bool firstTaskInPhase = packetPhase & firstPacket;
        bool requestProlog    = false;
        bool isLaAnalysisRequired = true;

#if _SW_BRC
        if (!m_pipeline->IsFirstPass())
        {
            auto pdwData = (uint32_t *)m_allocator->LockResourceForRead(m_basicFeature->m_recycleBuf->GetBuffer(VdencBrcPakMmioBuffer, 0));
            ENCODE_CHK_NULL_RETURN(pdwData);
            isLaAnalysisRequired = (*pdwData == CODECHAL_VDENC_HEVC_BRC_HUC_STATUS_REENCODE_MASK);
            m_allocator->UnLock(m_basicFeature->m_recycleBuf->GetBuffer(VdencBrcPakMmioBuffer, 0));

            if (m_swBrc && m_swBrc->SwBrcEnabled() && !isLaAnalysisRequired)
            {
                RUN_FEATURE_INTERFACE_RETURN(VdencLplaAnalysis, HevcFeatureIDs::vdencLplaAnalysisFeature, CalculateLaRecords, true);
                return MOS_STATUS_SUCCESS;
            }
        }
#endif
        if (!m_pipeline->IsSingleTaskPhaseSupported() || firstTaskInPhase)
        {
            // Send command buffer header at the beginning (OS dependent)
            requestProlog = true;
        }

        uint16_t orig_perf_tag = (uint16_t)m_osInterface->pfnGetPerfTag(m_osInterface);
        SetPerfTag(CODECHAL_ENCODE_PERFTAG_CALL_HEVC_LA_UPDATE, (uint16_t)m_basicFeature->m_mode, (uint16_t)m_basicFeature->m_pictureCodingType);
        ENCODE_CHK_STATUS_RETURN(Execute(commandBuffer, true, requestProlog, LA_UPDATE));
        SetPerfTag(orig_perf_tag, (uint16_t)m_basicFeature->m_mode, (uint16_t)m_basicFeature->m_pictureCodingType);

#if _SW_BRC
        if (!m_swBrc || !m_swBrc->SwBrcEnabled())
        {
#endif
            if (!m_pipeline->IsLastPass())
            {
                // Write HUC_STATUS mask: DW1 (mask value)
                auto &storeDataParams            = m_miItf->MHW_GETPAR_F(MI_STORE_DATA_IMM)();
                storeDataParams                  = {};
                storeDataParams.pOsResource      = m_basicFeature->m_recycleBuf->GetBuffer(VdencBrcPakMmioBuffer, 0);
                storeDataParams.dwResourceOffset = sizeof(uint32_t);
                storeDataParams.dwValue          = CODECHAL_VDENC_HEVC_BRC_HUC_STATUS_REENCODE_MASK;
                ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_STORE_DATA_IMM)(commandBuffer));

                // store HUC_STATUS register: DW0 (actual value)
                ENCODE_CHK_COND_RETURN((m_vdboxIndex > MHW_VDBOX_NODE_1), "ERROR - vdbox index exceed the maximum");
                auto mmioRegisters             = m_hucItf->GetMmioRegisters(m_vdboxIndex);
                auto &storeRegParams           = m_miItf->MHW_GETPAR_F(MI_STORE_REGISTER_MEM)();
                storeDataParams                = {};
                storeRegParams.presStoreBuffer = m_basicFeature->m_recycleBuf->GetBuffer(VdencBrcPakMmioBuffer, 0);
                storeRegParams.dwOffset        = 0;
                storeRegParams.dwRegister      = mmioRegisters->hucStatusRegOffset;
                ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_STORE_REGISTER_MEM)(commandBuffer));
            }
#if _SW_BRC
        }
        else
        {
            //add force wake up for ladll path, to avoid that ladll path directly use MI_COPY to get data in ReadLPLAData
            if (requestProlog)
            {
                ENCODE_CHK_STATUS_RETURN(AddForceWakeup(*commandBuffer));
                ENCODE_CHK_STATUS_RETURN(SendPrologCmds(*commandBuffer));
            }
        }
#endif

        ReadLPLAData(commandBuffer);

        RUN_FEATURE_INTERFACE_RETURN(VdencLplaAnalysis, HevcFeatureIDs::vdencLplaAnalysisFeature, CalculateLaRecords, m_pipeline->IsLastPass());

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HucLaUpdatePkt::CalculateCommandSize(uint32_t &commandBufferSize, uint32_t &requestedPatchListSize)
    {
        ENCODE_FUNC_CALL();

        auto osInterface = m_hwInterface->GetOsInterface();
        ENCODE_CHK_NULL_RETURN(osInterface);

        uint32_t hucCommandsSize = 0;
        uint32_t hucPatchListSize = 0;
        MHW_VDBOX_STATE_CMDSIZE_PARAMS stateCmdSizeParams;

        ENCODE_CHK_STATUS_RETURN(m_hwInterface->GetHucStateCommandSize(
            m_basicFeature->m_mode, (uint32_t *)&hucCommandsSize, (uint32_t *)&hucPatchListSize, &stateCmdSizeParams));

        commandBufferSize      = hucCommandsSize;
        requestedPatchListSize = osInterface->bUsesPatchList ? hucPatchListSize : 0;

        if (m_pipeline->IsSingleTaskPhaseSupported())
        {
            commandBufferSize *= m_pipeline->GetPassNum();
        }

        // 4K align since allocation is in chunks of 4K bytes.
        commandBufferSize = MOS_ALIGN_CEIL(commandBufferSize, CODECHAL_PAGE_SIZE);

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HucLaUpdatePkt::Completed(void *mfxStatus, void *rcsStatus, void *statusReport)
    {
        ENCODE_FUNC_CALL();

        ENCODE_CHK_NULL_RETURN(mfxStatus);
        ENCODE_CHK_NULL_RETURN(statusReport);
        ENCODE_CHK_NULL_RETURN(m_basicFeature);

        ENCODE_CHK_STATUS_RETURN(EncodeHucPkt::Completed(mfxStatus, rcsStatus, statusReport));

        EncodeStatusMfx *       encodeStatusMfx  = (EncodeStatusMfx *)mfxStatus;
        EncodeStatusReportData *statusReportData = (EncodeStatusReportData *)statusReport;

        RUN_FEATURE_INTERFACE_RETURN(VdencLplaAnalysis, HevcFeatureIDs::vdencLplaAnalysisFeature, GetLplaStatusReport, encodeStatusMfx, statusReportData);
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HucLaUpdatePkt::DumpOutput()
    {
        ENCODE_FUNC_CALL();

#if USE_CODECHAL_DEBUG_TOOL
        RUN_FEATURE_INTERFACE_RETURN(VdencLplaAnalysis, HevcFeatureIDs::vdencLplaAnalysisFeature, DumpLaResource, m_pipeline, false);
#endif

        return MOS_STATUS_SUCCESS;
    }

#if USE_CODECHAL_DEBUG_TOOL
    MOS_STATUS HucLaUpdatePkt::DumpInput()
    {
        ENCODE_FUNC_CALL();

        RUN_FEATURE_INTERFACE_RETURN(VdencLplaAnalysis, HevcFeatureIDs::vdencLplaAnalysisFeature, DumpLaResource, m_pipeline, true);

        return MOS_STATUS_SUCCESS;
    }
#endif

    MOS_STATUS HucLaUpdatePkt::ReadLPLAData(MOS_COMMAND_BUFFER* commandBuffer)
    {
        ENCODE_FUNC_CALL();

        PMOS_RESOURCE osResource = nullptr;
        uint32_t      offset     = 0;

        ENCODE_CHK_STATUS_RETURN(m_statusReport->GetAddress(statusReportLpla, osResource, offset));

        RUN_FEATURE_INTERFACE_RETURN(VdencLplaAnalysis, HevcFeatureIDs::vdencLplaAnalysisFeature, ReadLPLAData, commandBuffer, osResource, offset);

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(HUC_IMEM_STATE, HucLaUpdatePkt)
    {
        params.kernelDescriptor = VDBOX_HUC_LA_ANALYSIS_KERNEL_DESCRIPTOR;

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(HUC_DMEM_STATE, HucLaUpdatePkt)
    {
        params.function = LA_UPDATE;
        RUN_FEATURE_INTERFACE_RETURN(VdencLplaAnalysis, HevcFeatureIDs::vdencLplaAnalysisFeature, SetLaUpdateDmemParameters, 
            params, m_pipeline->m_currRecycledBufIdx, m_pipeline->GetCurrentPass(), m_pipeline->GetPassNum());

        auto laAnalysisFeature = dynamic_cast<VdencLplaAnalysis *>(m_featureManager->GetFeature(HevcFeatureIDs::vdencLplaAnalysisFeature));
        if (laAnalysisFeature && laAnalysisFeature->IsLastPicInStream())
        {
            m_pipeline->m_currRecycledBufIdx =
                (m_pipeline->m_currRecycledBufIdx + 1) % CODECHAL_ENCODE_RECYCLED_BUFFER_NUM;
        }
        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(HUC_VIRTUAL_ADDR_STATE, HucLaUpdatePkt)
    {
        params.function = LA_UPDATE;
        return MOS_STATUS_SUCCESS;
    }

}  // namespace encode
