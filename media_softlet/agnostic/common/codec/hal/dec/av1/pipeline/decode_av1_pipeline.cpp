/*
* Copyright (c) 2019-2024, Intel Corporation
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
//! \file     decode_av1_pipeline.cpp
//! \brief    Defines the interface for av1 decode pipeline
//!
#include "decode_av1_pipeline.h"
#include "decode_utils.h"
#include "codechal_setting.h"
#include "decode_av1_feature_manager.h"
#include "decode_huc_packet_creator_base.h"
#include "media_debug_fast_dump.h"

namespace decode {

Av1Pipeline::Av1Pipeline(
    CodechalHwInterfaceNext*   hwInterface,
    CodechalDebugInterface *debugInterface)
    : DecodePipeline(hwInterface, debugInterface)
{
    MOS_STATUS m_status = InitUserSetting(m_userSettingPtr);
}

MOS_STATUS Av1Pipeline::Initialize(void *settings)
{
    DECODE_FUNC_CALL();
    DECODE_CHK_STATUS(DecodePipeline::Initialize(settings));

    // Create basic GPU context
    if (m_osInterface->pfnIsMismatchOrderProgrammingSupported())
    {
        DecodeScalabilityPars scalPars;
        MOS_ZeroMemory(&scalPars, sizeof(scalPars));
        DECODE_CHK_STATUS(m_mediaContext->SwitchContext(VdboxDecodeFunc, &scalPars, &m_scalability));
        m_decodeContext       = m_osInterface->pfnGetGpuContext(m_osInterface);
        m_decodeContextHandle = m_osInterface->CurrentGpuContextHandle;
    }

    HucPacketCreatorBase *hucPktCreator = dynamic_cast<HucPacketCreatorBase *>(this);
    DECODE_CHK_NULL(hucPktCreator);

    auto *codecSettings = (CodechalSetting*)settings;
    DECODE_CHK_NULL(codecSettings);

  bool forceTileBasedDecodingRead = 0;
#if (_DEBUG || _RELEASE_INTERNAL)
    forceTileBasedDecodingRead = ReadUserFeature(m_userSettingPtr, "Force Av1 Tile Based Decode", MediaUserSetting::Group::Sequence).Get<bool>();
#endif
    m_forceTileBasedDecoding = forceTileBasedDecodingRead;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Av1Pipeline::Prepare(void *params)
{
    DECODE_FUNC_CALL();
    DECODE_CHK_NULL(params);

    auto basicFeature = dynamic_cast<Av1BasicFeature*>(m_featureManager->GetFeature(FeatureIDs::basicFeature));
    DECODE_CHK_NULL(basicFeature);
    DECODE_CHK_STATUS(DecodePipeline::Prepare(params));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Av1Pipeline::Uninitialize()
{
    DECODE_FUNC_CALL();

    return DecodePipeline::Uninitialize();
}

MOS_STATUS Av1Pipeline::UserFeatureReport()
{
    DECODE_FUNC_CALL();
    DECODE_CHK_STATUS(DecodePipeline::UserFeatureReport());
#if (_DEBUG || _RELEASE_INTERNAL)
    WriteUserFeature(__MEDIA_USER_FEATURE_VALUE_APOGEIOS_AV1D_ENABLE_ID, 1, m_osInterface->pOsContext);
#endif
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Av1Pipeline::ActivateDecodePackets()
{
    DECODE_FUNC_CALL();

    bool immediateSubmit = true;

    if (m_isFirstTileInFrm)
    {
        m_isFirstTileInFrm = false;
    }

    if (!m_forceTileBasedDecoding)
    {
        immediateSubmit = false;
    }

    for (uint16_t curPass = 0; curPass < GetPassNum(); curPass++)
    {
        DECODE_CHK_STATUS(ActivatePacket(DecodePacketId(this, av1DecodePacketId), immediateSubmit, curPass, 0));
    }

    return MOS_STATUS_SUCCESS;
}

bool Av1Pipeline::FrameBasedDecodingInUse()
{
    auto basicFeature = dynamic_cast<Av1BasicFeature*>(m_featureManager->GetFeature(FeatureIDs::basicFeature));

    bool isframeBasedDecodingUsed = false;

    if (basicFeature != nullptr)
    {
        isframeBasedDecodingUsed = ((basicFeature->m_av1PicParams->m_loopRestorationFlags.m_fields.m_yframeRestorationType > 0) &
                                   ((basicFeature->m_av1PicParams->m_loopRestorationFlags.m_fields.m_cbframeRestorationType |
                                    basicFeature->m_av1PicParams->m_loopRestorationFlags.m_fields.m_crframeRestorationType) > 0) &&
                                    basicFeature->m_av1PicParams->m_picInfoFlags.m_fields.m_useSuperres && MEDIA_IS_WA(GetWaTable(), Wa_1409820462)
                                    || !m_forceTileBasedDecoding);
    }
    return isframeBasedDecodingUsed;
}

MOS_STATUS Av1Pipeline::CreateFeatureManager()
{
    DECODE_FUNC_CALL();
    m_featureManager = MOS_New(DecodeAv1FeatureManager, m_allocator, m_hwInterface, m_osInterface);
    DECODE_CHK_NULL(m_featureManager);
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Av1Pipeline::CreateSubPackets(DecodeSubPacketManager &subPacketManager, CodechalSetting &codecSettings)
{
    DECODE_FUNC_CALL();

    DECODE_CHK_STATUS(DecodePipeline::CreateSubPackets(subPacketManager, codecSettings));

    return MOS_STATUS_SUCCESS;
}

Av1Pipeline::Av1DecodeMode Av1Pipeline::GetDecodeMode()
{
    return m_decodeMode;
}

#if USE_CODECHAL_DEBUG_TOOL
MOS_STATUS Av1Pipeline::DumpParams(Av1BasicFeature &basicFeature)
{
    m_debugInterface->m_frameType = basicFeature.m_av1PicParams->m_picInfoFlags.m_fields.m_frameType ? P_TYPE : I_TYPE;
    m_debugInterface->m_bufferDumpFrameNum = basicFeature.m_frameNum;

    DECODE_CHK_STATUS(DumpPicParams(basicFeature.m_av1PicParams));
    DECODE_CHK_STATUS(DumpTileParams(basicFeature.m_av1TileParams, basicFeature.m_tileCoding.m_numTiles));
    DECODE_CHK_STATUS(DumpBitstream(&basicFeature.m_resDataBuffer.OsResource, basicFeature.m_dataSize, 0));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Av1Pipeline::DumpPicParams(CodecAv1PicParams *picParams)
{
    CODECHAL_DEBUG_FUNCTION_ENTER;

    if (picParams == nullptr)
    {
        return MOS_STATUS_SUCCESS;
    }

    if (m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrPicParams))
    {
        const char *fileName = m_debugInterface->CreateFileName(
            "DEC",
            CodechalDbgBufferType::bufPicParams,
            CodechalDbgExtType::txt);

        if (m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrEnableFastDump))
        {
            MediaDebugFastDump::Dump(
                (uint8_t *)picParams,
                fileName,
                sizeof(CodecAv1PicParams),
                0,
                MediaDebugSerializer<CodecAv1PicParams>());
        }
        else
        {
            DumpDecodeAv1PicParams(picParams, fileName);
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Av1Pipeline::DumpTileParams(CodecAv1TileParams *tileParams, uint32_t tileNum)
{
    CODECHAL_DEBUG_FUNCTION_ENTER;

    if (tileParams == nullptr)
    {
        return MOS_STATUS_SUCCESS;
    }

    if (m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrSlcParams))
    {
        const char *fileName = m_debugInterface->CreateFileName(
            "DEC",
            "TileParams",
            CodechalDbgExtType::txt);

        if (m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrEnableFastDump))
        {
            MediaDebugFastDump::Dump(
                (uint8_t *)tileParams,
                fileName,
                sizeof(CodecAv1TileParams) * tileNum,
                0,
                MediaDebugSerializer<CodecAv1TileParams>());
        }
        else
        {
            DumpDecodeAv1TileParams(tileParams, tileNum, fileName);
        }
    }

    return MOS_STATUS_SUCCESS;
}
#endif

}
