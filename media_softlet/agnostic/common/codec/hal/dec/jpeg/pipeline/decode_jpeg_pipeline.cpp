/*
* Copyright (c) 2022, Intel Corporation
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
//! \file     decode_jpeg_pipeline.cpp
//! \brief    Defines the interface for jpeg decode pipeline
//!
#include "decode_jpeg_pipeline.h"
#include "decode_utils.h"
#include "codechal_setting.h"
#include "decode_jpeg_feature_manager.h"
#include "decode_jpeg_input_bitstream.h"
#include "media_debug_fast_dump.h"

namespace decode{

JpegPipeline::JpegPipeline(
    CodechalHwInterfaceNext *hwInterface,
    CodechalDebugInterface *debugInterface)
    : DecodePipeline(hwInterface, debugInterface)
{
    MOS_STATUS m_status = InitUserSetting(m_userSettingPtr);
}

MOS_STATUS JpegPipeline::Initialize(void *settings)
{
    DECODE_FUNC_CALL();

    DECODE_CHK_STATUS(DecodePipeline::Initialize(settings));
    m_basicFeature = dynamic_cast<JpegBasicFeature*>(m_featureManager->GetFeature(FeatureIDs::basicFeature));
    DECODE_CHK_NULL(m_basicFeature);

    // Create basic GPU context
    DecodeScalabilityPars scalPars;
    MOS_ZeroMemory(&scalPars, sizeof(scalPars));
    DECODE_CHK_STATUS(m_mediaContext->SwitchContext(VdboxDecodeFunc, &scalPars, &m_scalability));
    m_decodeContext = m_osInterface->pfnGetGpuContext(m_osInterface);
    m_decodeContextHandle = m_osInterface->CurrentGpuContextHandle;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS JpegPipeline::Prepare(void *params)
{
    DECODE_FUNC_CALL();

    DECODE_CHK_NULL(params);
    DECODE_CHK_STATUS(DecodePipeline::Prepare(params));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS JpegPipeline::UserFeatureReport()
{
    DECODE_FUNC_CALL();
    DECODE_CHK_STATUS(DecodePipeline::UserFeatureReport());
#if (_DEBUG || _RELEASE_INTERNAL)
    WriteUserFeature(__MEDIA_USER_FEATURE_VALUE_APOGEIOS_JPEGD_ENABLE_ID, 1, m_osInterface->pOsContext);
#endif

#ifdef _MMC_SUPPORTED
    CODECHAL_DEBUG_TOOL(
        if (m_mmcState != nullptr) {
            m_mmcState->UpdateUserFeatureKey(&(m_basicFeature->m_destSurface));
        })
#endif
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS JpegPipeline::Uninitialize()
{
    DECODE_FUNC_CALL();
    return DecodePipeline::Uninitialize();
}

MOS_STATUS JpegPipeline::ActivateDecodePackets()
{
    DECODE_FUNC_CALL();

    bool immediateSubmit = false;


    DECODE_CHK_STATUS(ActivatePacket(DecodePacketId(this, jpegDecodePacketId), immediateSubmit, 0, 0));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS JpegPipeline::CreateFeatureManager() 
{
    DECODE_FUNC_CALL();
    m_featureManager = MOS_New(DecodeJpegFeatureManager, m_allocator, m_hwInterface, m_osInterface);
    DECODE_CHK_NULL(m_featureManager);
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS JpegPipeline::CreateSubPackets(DecodeSubPacketManager& subPacketManager, CodechalSetting &codecSettings)
{
    DECODE_FUNC_CALL();
    DECODE_CHK_STATUS(DecodePipeline::CreateSubPackets(subPacketManager, codecSettings));
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS JpegPipeline::CreatePreSubPipeLines(DecodeSubPipelineManager &subPipelineManager)
{
    m_bitstream = MOS_New(DecodeJpegInputBitstream, this, m_task, m_numVdbox);
    DECODE_CHK_NULL(m_bitstream);
    DECODE_CHK_STATUS(subPipelineManager.Register(*m_bitstream));
    return MOS_STATUS_SUCCESS;
}

#if USE_CODECHAL_DEBUG_TOOL
MOS_STATUS JpegPipeline::DumpPicParams(
    CodecDecodeJpegPicParams *picParams)
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
                sizeof(CodecDecodeJpegPicParams),
                0,
                MediaDebugSerializer<CodecDecodeJpegPicParams>());
        }
        else
        {
            DumpDecodeJpegPicParams(picParams, fileName);
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS JpegPipeline::DumpScanParams(
    CodecDecodeJpegScanParameter *scanParams)
{
    CODECHAL_DEBUG_FUNCTION_ENTER;

    if (scanParams == nullptr)
    {
        return MOS_STATUS_SUCCESS;
    }

    if (m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrScanParams))
    {
        const char *fileName = m_debugInterface->CreateFileName(
            "_DEC",
            CodechalDbgBufferType::bufScanParams,
            CodechalDbgExtType::txt);

        if (m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrEnableFastDump))
        {
            MediaDebugFastDump::Dump(
                (uint8_t *)scanParams,
                fileName,
                sizeof(CodecDecodeJpegScanParameter),
                0,
                MediaDebugSerializer<CodecDecodeJpegScanParameter>());
        }
        else
        {
            DumpDecodeJpegScanParams(scanParams, fileName);
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS JpegPipeline::DumpHuffmanTable(
    PCODECHAL_DECODE_JPEG_HUFFMAN_TABLE huffmanTable)
{
    CODECHAL_DEBUG_FUNCTION_ENTER;

    if (huffmanTable == nullptr)
    {
        return MOS_STATUS_SUCCESS;
    }

    if (m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrHuffmanTbl))
    {
        const char *fileName = m_debugInterface->CreateFileName(
            "_DEC",
            CodechalDbgBufferType::bufHuffmanTbl,
            CodechalDbgExtType::txt);

        if (m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrEnableFastDump))
        {
            MediaDebugFastDump::Dump(
                (uint8_t *)huffmanTable,
                fileName,
                sizeof(CODECHAL_DECODE_JPEG_HUFFMAN_TABLE),
                0,
                MediaDebugSerializer<CODECHAL_DECODE_JPEG_HUFFMAN_TABLE>());
        }
        else
        {
            DumpDecodeJpegHuffmanParams(huffmanTable, fileName);
        }
    }
    
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS JpegPipeline::DumpIQParams(CodecJpegQuantMatrix *iqParams)
{
    CODECHAL_DEBUG_FUNCTION_ENTER;

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
                sizeof(CodecJpegQuantMatrix),
                0,
                MediaDebugSerializer<CodecJpegQuantMatrix>());
        }
        else
        {
            DumpDecodeJpegIqParams(iqParams, fileName);
        }
    }

    return MOS_STATUS_SUCCESS;
}
#endif

}
