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
#include "codechal_setting.h"
#include "decode_vp8_feature_manager.h"
#include "media_debug_fast_dump.h"

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

    if (picParams == nullptr)
    {
        return MOS_STATUS_SUCCESS;
    }

    if (m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrPicParams))
    {
        const char *fileName = m_debugInterface->CreateFileName(
            "_DEC",
            CodechalDbgBufferType::bufPicParams,
            CodechalDbgExtType::txt);

        if (m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrEnableFastDump))
        {
            MediaDebugFastDump::Dump(
                (uint8_t *)picParams,
                fileName,
                sizeof(CODEC_VP8_PIC_PARAMS),
                0,
                MediaDebugSerializer<CODEC_VP8_PIC_PARAMS>());
        }
        else
        {
            DumpDecodeVp8PicParams(picParams, fileName);
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp8Pipeline::DumpSliceParams(PCODEC_VP8_SLICE_PARAMS sliceParams)
{
    DECODE_FUNC_CALL();

    if (sliceParams == nullptr)
    {
        return MOS_STATUS_SUCCESS;
    }

    if (m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrSlcParams))
    {
        const char *fileName = m_debugInterface->CreateFileName(
            "_DEC",
            CodechalDbgBufferType::bufSlcParams,
            CodechalDbgExtType::txt);

        if (m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrEnableFastDump))
        {
            MediaDebugFastDump::Dump(
                (uint8_t *)sliceParams,
                fileName,
                sizeof(CODEC_VP8_SLICE_PARAMS),
                0,
                MediaDebugSerializer<CODEC_VP8_SLICE_PARAMS>());
        }
        else
        {
            DumpDecodeVp8SliceParams(sliceParams, fileName);
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp8Pipeline::DumpIQParams(PCODEC_VP8_IQ_MATRIX_PARAMS iqParams)
{
    DECODE_FUNC_CALL();

    if (iqParams == nullptr)
    {
        return MOS_STATUS_SUCCESS;
    }

    if (m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrIqParams))
    {
        const char *fileName = m_debugInterface->CreateFileName(
            "_DEC",
            CodechalDbgBufferType::bufIqParams,
            CodechalDbgExtType::txt);

        if (m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrEnableFastDump))
        {
            MediaDebugFastDump::Dump(
                (uint8_t *)iqParams,
                fileName,
                sizeof(CODEC_VP8_IQ_MATRIX_PARAMS),
                0,
                MediaDebugSerializer<CODEC_VP8_IQ_MATRIX_PARAMS>());
        }
        else
        {
            DumpDecodeVp8IqParams(iqParams, fileName);
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp8Pipeline::DumpCoefProbBuffer(PMOS_RESOURCE m_resCoefProbBuffer)
{
    DECODE_FUNC_CALL();

    if (m_resCoefProbBuffer == nullptr)
    {
        return MOS_STATUS_SUCCESS;
    }

    if (m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrCoefProb))
    {
        DECODE_CHK_STATUS(m_debugInterface->DumpBuffer(
            m_resCoefProbBuffer,
            CodechalDbgAttr::attrCoefProb,
            "_DEC_CoefProb",
            m_basicFeature->m_coefProbSize));
    }

    return MOS_STATUS_SUCCESS;
}
#endif

}  // namespace decode
