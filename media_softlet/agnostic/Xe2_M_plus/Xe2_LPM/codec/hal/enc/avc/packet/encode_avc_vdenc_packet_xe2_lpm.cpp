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
//! \file     encode_avc_vdenc_packet_xe2_lpm.cpp
//! \brief    Defines the interface for avc encode vdenc packet of Xe2_LPM
//!

#include "encode_avc_vdenc_packet_xe2_lpm.h"
#include "encode_avc_brc.h"
#include "encode_avc_vdenc_const_settings.h"
#include "encode_avc_aqm.h"

#include "media_avc_feature_defs.h"
#include "mhw_vdbox_xe2_lpm_base.h"
#include "mhw_vdbox_vdenc_hwcmd_xe2_lpm.h"
#include "mhw_vdbox_mfx_impl_xe2_lpm.h"

using namespace mhw::vdbox::xe2_lpm_base;

namespace encode
{

    MOS_STATUS AvcVdencPktXe2_Lpm::Completed(void *mfxStatus, void *rcsStatus, void *statusReport)
    {
        ENCODE_FUNC_CALL();
        auto eStatus = MOS_STATUS_SUCCESS;
        eStatus      = AvcVdencPkt::Completed(mfxStatus, rcsStatus, statusReport);

        auto aqmFeature = dynamic_cast<AvcEncodeAqm *>(m_featureManager->GetFeature(AvcFeatureIDs::avcAqm));
        ENCODE_CHK_NULL_RETURN(aqmFeature);
        if (aqmFeature->IsEnabled())
        {
            EncodeStatusReportData *statusReportData = (EncodeStatusReportData *)statusReport;
            uint32_t                statBufIdx       = statusReportData->currOriginalPic.FrameIdx;
            if (m_basicFeature->m_picParam->QualityInfoSupportFlags.fields.enable_frame)
            {
                ENCODE_CHK_STATUS_RETURN(aqmFeature->ReportQualityInfoFrame(statBufIdx, *statusReportData));
            }
#if _MEDIA_RESERVED
#if USE_CODECHAL_DEBUG_TOOL
            ENCODE_CHK_STATUS_RETURN(aqmFeature->DumpVdaqmOutput(statBufIdx, *statusReportData));

            CodechalDebugInterface *debugInterface = m_pipeline->GetStatusReportDebugInterface();
            ENCODE_CHK_NULL_RETURN(debugInterface);

            if (debugInterface->DumpIsEnabled(CodechalDbgAttr::attrQualityReport))
            {
                std::ostringstream oss;
                oss.setf(std::ios::showbase | std::ios::uppercase);

                aqmFeature->DumpVdaqmFrameStatVerbose(statBufIdx, oss);

                // Dump per frame VDAQM frame statistic file
                const char *fileName = debugInterface->CreateFileName("EncodeFrame", "VDAQMFrameStat", CodechalDbgExtType::txt);

                std::ofstream ofs(fileName, std::ios::out);
                ofs << oss.str();
                ofs.close();                
            }
#endif
#endif
        }

        return eStatus;
    }

    MOS_STATUS AvcVdencPktXe2_Lpm::AddPictureVdencCommands(MOS_COMMAND_BUFFER &cmdBuffer)
    {
        ENCODE_FUNC_CALL();

        AvcVdencPkt::AddPictureVdencCommands(cmdBuffer);

        auto aqmFeature = dynamic_cast<AvcEncodeAqm *>(m_featureManager->GetFeature(AvcFeatureIDs::avcAqm));
        ENCODE_CHK_NULL_RETURN(aqmFeature);
        if (aqmFeature->IsEnabled())
        {
            SETPAR_AND_ADDCMD(AQM_VD_CONTROL_STATE, m_aqmItf, &cmdBuffer);
            SETPAR_AND_ADDCMD(AQM_PIPE_BUF_ADDR_STATE, m_aqmItf, &cmdBuffer);
            SETPAR_AND_ADDCMD(AQM_PIC_STATE, m_aqmItf, &cmdBuffer);
            SETPAR_AND_ADDCMD(AQM_FRAME_START, m_aqmItf, &cmdBuffer);
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AvcVdencPktXe2_Lpm::SendSlice(PMOS_COMMAND_BUFFER cmdBuffer)
    {
        ENCODE_FUNC_CALL();

        AvcVdencPkt::SendSlice(cmdBuffer);

        auto aqmFeature = dynamic_cast<AvcEncodeAqm *>(m_featureManager->GetFeature(AvcFeatureIDs::avcAqm));
        ENCODE_CHK_NULL_RETURN(aqmFeature);
        if (aqmFeature->IsEnabled())
        {
            SETPAR_AND_ADDCMD(AQM_SLICE_STATE, m_aqmItf, cmdBuffer);
        }

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(VD_PIPELINE_FLUSH, AvcVdencPktXe2_Lpm)
    {
        AvcVdencPkt::MHW_SETPAR_F(VD_PIPELINE_FLUSH)(params);

        auto aqmFeature = dynamic_cast<AvcEncodeAqm *>(m_featureManager->GetFeature(AvcFeatureIDs::avcAqm));
        ENCODE_CHK_NULL_RETURN(aqmFeature);
        if (aqmFeature->IsEnabled())
        {
            params.waitDoneVDAQM = m_lastSlice ? true : false;
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AvcVdencPktXe2_Lpm::EnsureAllCommandsExecuted(MOS_COMMAND_BUFFER &cmdBuffer)
    {
        ENCODE_FUNC_CALL();

        // Send MI_FLUSH command
        auto &flushDwParams                         = m_miItf->MHW_GETPAR_F(MI_FLUSH_DW)();
        flushDwParams                               = {};
        flushDwParams.bVideoPipelineCacheInvalidate = true;

        auto *skuTable = m_hwInterface->GetSkuTable();
        if (skuTable && MEDIA_IS_SKU(skuTable, FtrEnablePPCFlush))
        {
            // Add PPC fulsh
            flushDwParams.bEnablePPCFlush = true;
        }

        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_FLUSH_DW)(&cmdBuffer));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AvcVdencPktXe2_Lpm::EndStatusReport(uint32_t srType, MOS_COMMAND_BUFFER *cmdBuffer)
    {
        ENCODE_FUNC_CALL();
        ENCODE_CHK_NULL_RETURN(cmdBuffer);
        ENCODE_CHK_STATUS_RETURN(AvcVdencPkt::EndStatusReport(srType, cmdBuffer));

        MediaPerfProfiler *perfProfiler = MediaPerfProfiler::Instance();
        ENCODE_CHK_NULL_RETURN(perfProfiler);

        //store bitstream size to UMD profiler
        ENCODE_CHK_STATUS_RETURN(ValidateVdboxIdx(m_vdboxIndex));
        MmioRegistersMfx *mmioRegisters = SelectVdboxAndGetMmioRegister(m_vdboxIndex, cmdBuffer);
        CODEC_HW_CHK_NULL_RETURN(mmioRegisters);
        ENCODE_CHK_STATUS_RETURN(perfProfiler->AddStoreBitstreamSizeCmd(
            (void *)m_pipeline, m_osInterface, m_miItf, cmdBuffer, mmioRegisters->mfcBitstreamBytecountFrameRegOffset));
#if _MEDIA_RESERVED
        //store quality metric to UMD profiler
        RUN_FEATURE_INTERFACE_RETURN(AvcEncodeAqm, AvcFeatureIDs::avcAqm, AddStoreQualityMetricCmd, (void *)m_pipeline, m_osInterface, m_miItf, cmdBuffer);
#endif
        return MOS_STATUS_SUCCESS;
    }
}

