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
//! \file     encode_hevc_vdenc_pipeline.cpp
//! \brief    Defines the interface for hevc vdenc encode pipeline
//!
#include "encode_hevc_vdenc_pipeline.h"
#include "codec_def_encode.h"
#include "encode_utils.h"
#include "encode_tile.h"
#include "encode_hevc_brc.h"
#include "encode_vdenc_lpla_analysis.h"
#if _KERNEL_RESERVED
#include "encode_saliency_render_cmd_packet.h"
#endif

namespace encode {

HevcVdencPipeline::HevcVdencPipeline(
    CodechalHwInterfaceNext *   hwInterface,
    CodechalDebugInterface *debugInterface)
    : HevcPipeline(hwInterface, debugInterface)
{
    ENCODE_FUNC_CALL();
}

MOS_STATUS HevcVdencPipeline::Initialize(void *settings)
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_STATUS_RETURN(HevcPipeline::Initialize(settings));
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcVdencPipeline::Uninitialize()
{
    ENCODE_FUNC_CALL();
    return HevcPipeline::Uninitialize();
}

MOS_STATUS HevcVdencPipeline::UserFeatureReport()
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_STATUS_RETURN(HevcPipeline::UserFeatureReport());

#if (_DEBUG || _RELEASE_INTERNAL)
    ReportUserSettingForDebug(
        m_userSettingPtr,
        "VDENC In Use",
        1,
        MediaUserSetting::Group::Sequence);
#endif

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcVdencPipeline::Prepare(void *params)
{
    ENCODE_FUNC_CALL();
    EncoderParams *encodeParams = (EncoderParams *)params;

    ENCODE_CHK_NULL_RETURN(encodeParams);

    if (encodeParams->ExecCodecFunction != CODECHAL_FUNCTION_ENC_VDENC_PAK)
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }

    ENCODE_CHK_STATUS_RETURN(HevcPipeline::Prepare(params));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcVdencPipeline::HuCCheckAndInit()
{
    ENCODE_FUNC_CALL();

    bool immediateSubmit = !m_singleTaskPhaseSupported;
    ENCODE_CHK_STATUS_RETURN(ActivatePacket(HucBrcInit, immediateSubmit, 0, 0));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcVdencPipeline::ActivateVdencVideoPackets()
{
    ENCODE_FUNC_CALL();

    auto brcFeature = dynamic_cast<HEVCEncodeBRC*>(m_featureManager->GetFeature(HevcFeatureIDs::hevcBrcFeature));
    ENCODE_CHK_NULL_RETURN(brcFeature);
    bool immediateSubmit = !m_singleTaskPhaseSupported;

    if (brcFeature->IsBRCInitRequired())
    {
        ENCODE_CHK_STATUS_RETURN(HuCCheckAndInit());
    }

    bool tileEnabled = false;
    RUN_FEATURE_INTERFACE_RETURN(HevcEncodeTile, HevcFeatureIDs::encodeTile, IsEnabled, tileEnabled);

    for (uint8_t curPass = 0; curPass < GetPassNum(); curPass++)
    {
        auto laAnalysisFeature = dynamic_cast<VdencLplaAnalysis *>(m_featureManager->GetFeature(HevcFeatureIDs::vdencLplaAnalysisFeature));
        if (laAnalysisFeature && !laAnalysisFeature->IsLastPicInStream())
        {
            if (brcFeature->IsBRCUpdateRequired())
            {
                ENCODE_CHK_STATUS_RETURN(ActivatePacket(HucBrcUpdate, immediateSubmit, curPass, 0));
            }

            for (uint8_t curPipe = 0; curPipe < GetPipeNum(); curPipe++)
            {
                ENCODE_CHK_STATUS_RETURN(ActivatePacket(hevcVdencPacket, immediateSubmit, curPass, curPipe, GetPipeNum()));
            }
        }
       
        if (laAnalysisFeature && laAnalysisFeature->IsLaAnalysisRequired())
        {
            if (!laAnalysisFeature->IsLastPicInStream())
            {
                if (laAnalysisFeature->IsLaInitRequired())
                {
                    ENCODE_CHK_STATUS_RETURN(ActivatePacket(HucLaInit, immediateSubmit, 0, 0));
                }
                ENCODE_CHK_STATUS_RETURN(ActivatePacket(HucLaUpdate, immediateSubmit, curPass, 0));
            }
            else
            {
                // Flush the last frames
                ENCODE_CHK_STATUS_RETURN(ActivatePacket(HucLaUpdate, immediateSubmit, curPass, 0));
            }
        }

        if (tileEnabled)
        {
            ENCODE_CHK_STATUS_RETURN(ActivatePacket(hevcPakIntegrate, immediateSubmit, curPass, 0));
        }
    }

    // Last element in m_activePacketList must be immediately submitted
    m_activePacketList.back().immediateSubmit = true;

    auto basicFeature = dynamic_cast<HevcBasicFeature *>(m_featureManager->GetFeature(FeatureIDs::basicFeature));
    ENCODE_CHK_NULL_RETURN(basicFeature);
    if (basicFeature->m_422State && basicFeature->m_422State->GetFeature422Flag())
    {
        m_activePacketList.front().frameTrackingRequested = false;
        ENCODE_CHK_STATUS_RETURN(ActivatePacket(hevcVdencPacket422, true, 0, 0));
    }

    SetFrameTrackingForMultiTaskPhase();

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcVdencPipeline::ActivateVdencTileReplayVideoPackets()
{
    ENCODE_FUNC_CALL();

    auto brcFeature = dynamic_cast<HEVCEncodeBRC*>(m_featureManager->GetFeature(HevcFeatureIDs::hevcBrcFeature));
    ENCODE_CHK_NULL_RETURN(brcFeature);
    bool immediateSubmit = !m_singleTaskPhaseSupported;

    // BRC init
    if (brcFeature->IsBRCInitRequired())
    {
        ENCODE_CHK_STATUS_RETURN(ActivatePacket(HucBrcInit, immediateSubmit, 0, 0));
    }
    // BRC frame level HuC update
    if (brcFeature->IsBRCUpdateRequired())
    {
        ENCODE_CHK_STATUS_RETURN(ActivatePacket(HucBrcUpdate, immediateSubmit, 0, 0));
    }

    uint16_t numTileRows = 1;
    uint16_t numTileColumns = 1;
    RUN_FEATURE_INTERFACE_RETURN(HevcEncodeTile, HevcFeatureIDs::encodeTile, GetTileRowColumns, numTileRows, numTileColumns);

    uint8_t numPassForTileReplay = brcFeature->IsBRCUpdateRequired() ? 2 : 1;

    for (uint8_t curPipe = 0; curPipe < GetPipeNum(); curPipe++)
    {
        ENCODE_CHK_STATUS_RETURN(ActivatePacket(hevcVdencPicPacket, immediateSubmit, 0, curPipe, GetPipeNum()));
    }

    // Tile Row Level Update
    for (uint8_t curRow = 0; curRow < numTileRows; curRow++)
    {
        for (uint8_t curPass = 0; curPass < numPassForTileReplay; curPass++)
        {
            for (uint8_t curPipe = 0; curPipe < GetPipeNum(); curPipe++)
            {
                // if 2nd pass, BRC tile row level HuC update
                if (brcFeature->IsBRCUpdateRequired())
                {
                    ENCODE_CHK_STATUS_RETURN(ActivatePacket(HucBrcTileRowUpdate, immediateSubmit, 0, 0));
                }
                // BRC tile row level ENC+PAK
                ENCODE_CHK_STATUS_RETURN(ActivatePacket(hevcVdencTileRowPacket, immediateSubmit, 0, curPipe, GetPipeNum(), curPass, curRow));
            }
        }
    }

    // Frame Level Pak Integration
    ENCODE_CHK_STATUS_RETURN(ActivatePacket(hevcPakIntegrate, immediateSubmit, 0, 0));

    SetFrameTrackingForMultiTaskPhase();

    // Last element in m_activePacketList must be immediately submitted
    m_activePacketList.back().immediateSubmit = true;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcVdencPipeline::CreateFeatureManager()
{
    ENCODE_FUNC_CALL();
    m_featureManager = MOS_New(EncodeHevcVdencFeatureManager, m_allocator, m_hwInterface, m_trackedBuf, m_recycleBuf);
    ENCODE_CHK_NULL_RETURN(m_featureManager);
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcVdencPipeline::SwitchContext(uint8_t outputChromaFormat, uint16_t numTileRows, uint16_t numTileColumns, bool enableTileReplay)
{
    ENCODE_FUNC_CALL();

    if (!m_scalPars)
    {
        m_scalPars = std::make_shared<EncodeScalabilityPars>();
    }

    *m_scalPars             = {};
    m_scalPars->enableVDEnc = true;
    m_scalPars->enableVE    = MOS_VE_SUPPORTED(m_osInterface);
    m_scalPars->numVdbox    = m_numVdbox;

    m_scalPars->forceMultiPipe     = true;
    m_scalPars->outputChromaFormat = outputChromaFormat;

    m_scalPars->numTileRows    = numTileRows;
    m_scalPars->numTileColumns = numTileColumns;

    m_scalPars->IsPak = true;

    m_scalPars->enableTileReplay = enableTileReplay;

    m_mediaContext->SwitchContext(VdboxEncodeFunc, &*m_scalPars, &m_scalability);
    ENCODE_CHK_NULL_RETURN(m_scalability);

    m_scalability->SetPassNumber(m_featureManager->GetNumPass());

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcVdencPipeline::ExecuteSaliencyPackets()
{
#if _KERNEL_RESERVED
    bool saliencyEnabled = false;
    RUN_FEATURE_INTERFACE_RETURN(EncodeSaliencyFeature, FeatureIDs::saliencyFeature, IsEnabled, saliencyEnabled);
    if (saliencyEnabled)
    {
        //Switch for Kerenl.
        ScalabilityPars scalPars = {};
        ENCODE_CHK_NULL_RETURN(m_mediaContext);
        ENCODE_CHK_STATUS_RETURN(m_mediaContext->SwitchContext(ComputeVppFunc, &scalPars, &m_scalability));

        uint32_t execMode = 0;
        RUN_FEATURE_INTERFACE_RETURN(EncodeSaliencyFeature, FeatureIDs::saliencyFeature, GetExecMode, execMode);

         //Execute Saliency Kernels
        if (execMode & SALIENCY_EXEC_JND)
        {
            ENCODE_CHK_STATUS_RETURN(ActivatePacket(hevcSaliencyPacket, true, 0, 0));
            m_activePacketList.back().frameTrackingRequested = false;
            m_activePacketList.back().skipOcaBBStartInCmdTask = true;
            ENCODE_CHK_STATUS_RETURN(ExecuteActivePackets());
        }

        //Copy Anchor frame
        if (execMode & SALIENCY_EXEC_COPY)
        {
            auto saliencyFeature = dynamic_cast<EncodeSaliencyFeature *>(m_featureManager->GetFeature(FeatureIDs::saliencyFeature));
            ENCODE_CHK_NULL_RETURN(saliencyFeature);
            ENCODE_CHK_NULL_RETURN(saliencyFeature->m_saliencyKernelPar.pCurrRawSurf);
            ENCODE_CHK_NULL_RETURN(saliencyFeature->m_saliencyKernelPar.pLastSceneSurf);

            ENCODE_CHK_NULL_RETURN(m_mediaCopyWrapper);
            ENCODE_CHK_STATUS_RETURN(m_mediaCopyWrapper->MediaCopy(
                &saliencyFeature->m_saliencyKernelPar.pCurrRawSurf->OsResource,
                &saliencyFeature->m_saliencyKernelPar.pLastSceneSurf->OsResource,
                MCPY_METHOD_DEFAULT));
        }

        //Switch back to VDEnc
        auto feature = dynamic_cast<HevcBasicFeature *>(m_featureManager->GetFeature(FeatureIDs::basicFeature));
        ENCODE_CHK_NULL_RETURN(feature);
        uint16_t numTileRows    = 0;
        uint16_t numTileColumns = 0;
        RUN_FEATURE_INTERFACE_RETURN(HevcEncodeTile, FeatureIDs::encodeTile, GetTileRowColumns, numTileRows, numTileColumns);
        bool enableTileReplay = false;
        RUN_FEATURE_INTERFACE_RETURN(HevcEncodeTile, FeatureIDs::encodeTile, IsTileReplayEnabled, enableTileReplay);
        ENCODE_CHK_STATUS_RETURN(SwitchContext(feature->m_outputChromaFormat, numTileRows, numTileColumns, enableTileReplay));
    }
#endif

    return MOS_STATUS_SUCCESS;
}

}
