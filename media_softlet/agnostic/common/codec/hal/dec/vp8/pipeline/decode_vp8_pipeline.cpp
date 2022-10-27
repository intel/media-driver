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
//! \file     decode_vp8_pipeline.cpp
//! \brief    Defines the interface for vp8 decode pipeline
//!
#include "decode_vp8_pipeline.h"
#include "decode_utils.h"
//#include "media_user_settings_mgr_g12_ext.h"
#include "codechal_setting.h"
#include "decode_vp8_feature_manager.h"


namespace decode
{
Vp8Pipeline::Vp8Pipeline(
    CodechalHwInterfaceNext *   hwInterface,
    CodechalDebugInterface  *debugInterface)
    : DecodePipeline(hwInterface, debugInterface)
{
    MOS_STATUS m_status = InitUserSetting(m_userSettingPtr);
}

MOS_STATUS Vp8Pipeline::Initialize(void *settings)
{
    DECODE_FUNC_CALL();
    DECODE_CHK_STATUS(DecodePipeline::Initialize(settings));

    // Create basic GPU context
    DecodeScalabilityPars scalPars;
    MOS_ZeroMemory(&scalPars, sizeof(scalPars));
    DECODE_CHK_STATUS(m_mediaContext->SwitchContext(VdboxDecodeFunc, &scalPars, &m_scalability));
    m_decodeContext = m_osInterface->pfnGetGpuContext(m_osInterface);
    m_decodeContextHandle = m_osInterface->CurrentGpuContextHandle;

    m_basicFeature = dynamic_cast<Vp8BasicFeature *>(m_featureManager->GetFeature(FeatureIDs::basicFeature));
    DECODE_CHK_NULL(m_basicFeature);

    auto *codecSettings = (CodechalSetting *)settings;
    DECODE_CHK_NULL(codecSettings);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp8Pipeline::Prepare(void *params)
{
    DECODE_FUNC_CALL();
    DECODE_CHK_NULL(params);

    DECODE_CHK_STATUS(DecodePipeline::Prepare(params));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp8Pipeline::Uninitialize()
{
    DECODE_FUNC_CALL();

    return DecodePipeline::Uninitialize();
}

MOS_STATUS Vp8Pipeline::UserFeatureReport()
{
    DECODE_FUNC_CALL();
    DECODE_CHK_STATUS(DecodePipeline::UserFeatureReport());
#if (_DEBUG || _RELEASE_INTERNAL)
    WriteUserFeature(__MEDIA_USER_FEATURE_VALUE_APOGEIOS_VP8D_ENABLE_ID, 1, m_osInterface->pOsContext);
#endif

#ifdef _MMC_SUPPORTED
    CODECHAL_DEBUG_TOOL(
        if (m_mmcState != nullptr) {
            m_mmcState->UpdateUserFeatureKey(&(m_basicFeature->m_destSurface));
        })
#endif
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp8Pipeline::Execute()
{
    DECODE_FUNC_CALL();

    bool immediateSubmit = false;
    DECODE_CHK_STATUS(ActivatePacket(DecodePacketId(this, vp8DecodePacketId), immediateSubmit, 0, 0));
    DECODE_CHK_STATUS(ExecuteActivePackets());
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp8Pipeline::CreateFeatureManager()
{
    DECODE_FUNC_CALL();
    m_featureManager = MOS_New(DecodeVp8FeatureManager, m_allocator, m_hwInterface, m_osInterface);
    DECODE_CHK_NULL(m_featureManager);
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp8Pipeline::CreateSubPackets(DecodeSubPacketManager &subPacketManager, CodechalSetting &codecSettings)
{
    DECODE_FUNC_CALL();

    DECODE_CHK_STATUS(DecodePipeline::CreateSubPackets(subPacketManager, codecSettings));

    return MOS_STATUS_SUCCESS;
}

Vp8Pipeline::Vp8DecodeMode Vp8Pipeline::GetDecodeMode()
{
    return m_decodeMode;
}

#if USE_CODECHAL_DEBUG_TOOL
MOS_STATUS Vp8Pipeline::DumpPicParams(PCODEC_VP8_PIC_PARAMS picParams)
{
    DECODE_FUNC_CALL();

    if (!m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrPicParams))
    {
        return MOS_STATUS_SUCCESS;
    }

    DECODE_CHK_NULL(picParams);

    std::ostringstream oss;
    oss.setf(std::ios::showbase | std::ios::uppercase);

    oss<< "CurrPic FrameIdx: "<<std::hex<< +picParams->CurrPic.FrameIdx<<std::endl;
    oss<< "CurrPic PicFlags: "<<std::hex<< +picParams->CurrPic.PicFlags<<std::endl;
    oss<< "wFrameWidthInMbsMinus1: "<<std::hex<< +picParams->wFrameWidthInMbsMinus1<<std::endl;
    oss<< "wFrameHeightInMbsMinus1: "<<std::hex<< +picParams->wFrameHeightInMbsMinus1<<std::endl;
    oss<< "ucCurrPicIndex: "<<std::hex<< +picParams->ucCurrPicIndex<<std::endl;
    oss<< "ucLastRefPicIndex: "<<std::hex<< +picParams->ucLastRefPicIndex<<std::endl;
    oss<< "ucGoldenRefPicIndex: "<<std::hex<< +picParams->ucGoldenRefPicIndex<<std::endl;
    oss<< "ucAltRefPicIndex: "<<std::hex<< +picParams->ucAltRefPicIndex<<std::endl;
    oss<< "ucDeblockedPicIndex: "<<std::hex<< +picParams->ucDeblockedPicIndex<<std::endl;
    oss<< "ucReserved8Bits: "<<std::hex<< +picParams->ucReserved8Bits<<std::endl;
    oss<< "wPicFlags: "<<std::hex<< +picParams->wPicFlags<<std::endl;
    oss<< "key_frame: "<<std::hex<< +picParams->key_frame<<std::endl;
    oss<< "version: "<<std::hex<< +picParams->version<<std::endl;
    oss<< "segmentation_enabled: "<<std::hex<< +picParams->segmentation_enabled<<std::endl;
    oss<< "update_mb_segmentation_map: "<<std::hex<< +picParams->update_mb_segmentation_map<<std::endl;
    oss<< "update_segment_feature_data: "<<std::hex<< +picParams->update_segment_feature_data<<std::endl;
    oss<< "filter_type: "<<std::hex<< +picParams->filter_type<<std::endl;
    oss<< "sign_bias_golden: "<<std::hex<< +picParams->sign_bias_golden<<std::endl;
    oss<< "sign_bias_alternate: "<<std::hex<< +picParams->sign_bias_alternate<<std::endl;
    oss<< "mb_no_coeff_skip: "<<std::hex<< +picParams->mb_no_coeff_skip<<std::endl;
    oss<< "mode_ref_lf_delta_update: "<<std::hex<< +picParams->mode_ref_lf_delta_update<<std::endl;
    oss<< "CodedCoeffTokenPartition: "<<std::hex<< +picParams->CodedCoeffTokenPartition<<std::endl;
    oss<< "LoopFilterDisable: "<<std::hex<< +picParams->LoopFilterDisable<<std::endl;
    oss<< "loop_filter_adj_enable: "<<std::hex<< +picParams->loop_filter_adj_enable<<std::endl;

    for(uint8_t i=0;i<4;++i)
    {
        oss<< "ucLoopFilterLevel[" <<std::dec<<+i<<"]: "<<std::hex<< +picParams->ucLoopFilterLevel[i]<<std::endl;
    }

    for(uint8_t i=0;i<4;++i)
    {
        oss<< "cRefLfDelta["<<std::dec<<+i<<"]: "<<std::hex<< +picParams->cRefLfDelta[i]<<std::endl;
    }

    for(uint8_t i=0;i<4;++i)
    {
        oss<< "cModeLfDelta[" <<std::dec<<+i<<"]: "<<std::hex<< +picParams->cModeLfDelta[i]<<std::endl;
    }
    oss<< "ucSharpnessLevel: " <<std::dec<<std::hex<< +picParams->ucSharpnessLevel<<std::endl;

    for(uint8_t i=0;i<3;++i)
    {
        oss<< "cMbSegmentTreeProbs[" <<std::dec<<+i<<"]: "<<std::hex<< +picParams->cMbSegmentTreeProbs[i]<<std::endl;
    }
    oss<< "ucProbSkipFalse: "<<std::hex<< +picParams->ucProbSkipFalse<<std::endl;
    oss<< "ucProbIntra: "<<std::hex<< +picParams->ucProbIntra<<std::endl;
    oss<< "ucProbLast: "<<std::hex<< +picParams->ucProbLast<<std::endl;
    oss<< "ucProbGolden: "<<std::hex<< +picParams->ucProbGolden<<std::endl;

    for(uint8_t i=0;i<4;++i)
    {
        oss<< "ucYModeProbs[" <<std::dec<<+i<<"]: "<<std::hex<< +picParams->ucYModeProbs[i]<<std::endl;
    }

    for(uint8_t i=0;i<3;++i)
    {
        oss<< "ucUvModeProbs[" <<std::dec<<+i<<"]: "<<std::hex<< +picParams->ucUvModeProbs[i]<<std::endl;
    }
    oss<< "ucReserved8Bits1: "<<std::hex<< +picParams->ucReserved8Bits1<<std::endl;
    oss<< "ucP0EntropyCount: "<<std::hex<< +picParams->ucP0EntropyCount<<std::endl;
    oss<< "ucP0EntropyValue: "<<std::hex<< +picParams->ucP0EntropyValue<<std::endl;
    oss<< "uiP0EntropyRange: "<<std::hex<< +picParams->uiP0EntropyRange<<std::endl;
    oss<< "uiFirstMbByteOffset: "<<std::hex<< +picParams->uiFirstMbByteOffset<<std::endl;

    for(uint8_t i=0;i<2;++i)
    {
        for(uint8_t j=0;j<CODEC_VP8_MVP_COUNT;++j)
        {
            oss<< "ucMvUpdateProb["<<std::dec<<+i<<"]["<<std::dec<<+j<<"]: "<<std::hex<< +picParams->ucMvUpdateProb[i][j]<<std::endl;
        }
    }
    for(uint8_t i=0;i<CODEC_VP8_MAX_PARTITION_NUMBER;++i)
    {
        oss<< "uiPartitionSize["<<std::dec<<+i<<"]: "<<std::hex<< +picParams->uiPartitionSize[i]<<std::endl;
    }
    oss<< "uiStatusReportFeedbackNumber: "<<std::hex<< +picParams->uiStatusReportFeedbackNumber<<std::endl;

    const char* fileName = m_debugInterface->CreateFileName(
        "_DEC",
        CodechalDbgBufferType::bufPicParams,
        CodechalDbgExtType::txt);

    std::ofstream ofs(fileName, std::ios::out);
    ofs << oss.str();
    ofs.close();

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp8Pipeline::DumpSliceParams(PCODEC_VP8_SLICE_PARAMS sliceParams)
{
    DECODE_FUNC_CALL();

    if (!m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrSlcParams))
    {
        return MOS_STATUS_SUCCESS;
    }

    DECODE_CHK_NULL(sliceParams);

    std::ostringstream oss;
    oss.setf(std::ios::showbase | std::ios::uppercase);

    oss<< "BSNALunitDataLocation: "<<std::hex<< +sliceParams->BSNALunitDataLocation<<std::endl;
    oss<< "SliceBytesInBuffer: "<<std::hex<< +sliceParams->SliceBytesInBuffer<<std::endl;

    const char* fileName = m_debugInterface->CreateFileName(
        "_DEC",
        CodechalDbgBufferType::bufSlcParams,
        CodechalDbgExtType::txt);

    std::ofstream ofs(fileName, std::ios::out);
    ofs << oss.str();
    ofs.close();
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp8Pipeline::DumpIQParams(CODEC_VP8_IQ_MATRIX_PARAMS *matrixData)
{
    DECODE_FUNC_CALL();

    if (!m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrIqParams))
    {
        return MOS_STATUS_SUCCESS;
    }

    DECODE_CHK_NULL(matrixData);

    std::ostringstream oss;
    oss.setf(std::ios::showbase | std::ios::uppercase);

    for(uint8_t i=0;i<4;++i)
    {
        for(uint8_t j=0;j<6;++j)
        {
            oss<< "quantization_values["<< std::dec<< +i <<"]["<<+j<<"]: "<<std::hex<< +matrixData->quantization_values[i][j]<<std::endl;
        }
    }

    const char* fileName = m_debugInterface->CreateFileName(
        "_DEC",
        CodechalDbgBufferType::bufIqParams,
        CodechalDbgExtType::txt);

    std::ofstream ofs(fileName, std::ios::out);
    ofs << oss.str();
    ofs.close();
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp8Pipeline::DumpCoefProbBuffer(PMOS_RESOURCE m_resCoefProbBuffer)
{
    DECODE_FUNC_CALL();

    if (!m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrCoeffProb))
    {
        return MOS_STATUS_SUCCESS;
    }

    DECODE_CHK_STATUS(m_debugInterface->DumpBuffer(
            m_resCoefProbBuffer,
            CodechalDbgAttr::attrCoeffProb,
            "_DEC_CoefProb",
            m_basicFeature->m_coefProbSize));

    return MOS_STATUS_SUCCESS;
}

#endif

}  // namespace decode
