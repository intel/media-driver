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
#include "media_user_settings_mgr_g12.h"
#include "codechal_setting.h"
#include "decode_jpeg_feature_manager.h"
#include "decode_jpeg_input_bitstream.h"

namespace decode{

JpegPipeline::JpegPipeline(
    CodechalHwInterface *   hwInterface,
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
    m_featureManager = MOS_New(DecodeJpegFeatureManager, m_allocator, m_hwInterface);
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

    if (!m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrPicParams))
    {
        return MOS_STATUS_SUCCESS;
    }

    DECODE_CHK_NULL(picParams);

    std::ostringstream oss;
    oss.setf(std::ios::showbase | std::ios::uppercase);

    oss << "destPic.FrameIdx: " << +picParams->m_destPic.FrameIdx << std::endl;
    oss << "destPic.PicFlags: " << +picParams->m_destPic.PicFlags << std::endl;
    oss << "frameWidth: " << +picParams->m_frameWidth << std::endl;
    oss << "frameHeight: " << +picParams->m_frameHeight << std::endl;
    oss << "numCompInFrame: " << +picParams->m_numCompInFrame << std::endl;

    //Dump componentIdentifier[jpegNumComponent]
    for (uint32_t i = 0; i < jpegNumComponent; ++i)
    {
        oss << "componentIdentifier[" << +i << "]: " << +picParams->m_componentIdentifier[i] << std::endl;
    }

    //Dump quantTableSelector[jpegNumComponent]
    for (uint32_t i = 0; i < jpegNumComponent; ++i)
    {
        oss << "quantTableSelector[" << +i << "]: " << +picParams->m_quantTableSelector[i] << std::endl;
    }
    oss << "chromaType: " << +picParams->m_chromaType << std::endl;
    oss << "rotation: " << +picParams->m_rotation << std::endl;
    oss << "totalScans: " << +picParams->m_totalScans << std::endl;
    oss << "interleavedData: " << +picParams->m_interleavedData << std::endl;
    oss << "reserved: " << +picParams->m_reserved << std::endl;
    oss << "statusReportFeedbackNumber: " << +picParams->m_statusReportFeedbackNumber << std::endl;

    const char *fileName = m_debugInterface->CreateFileName(
        "_DEC",
        CodechalDbgBufferType::bufPicParams,
        CodechalDbgExtType::txt);

    std::ofstream ofs(fileName, std::ios::out);
    ofs << oss.str();
    ofs.close();

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS JpegPipeline::DumpIQParams(CodecJpegQuantMatrix *matrixData)
{
    CODECHAL_DEBUG_FUNCTION_ENTER;

    if (!m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrIqParams))
    {
        return MOS_STATUS_SUCCESS;
    }

    CODECHAL_DEBUG_CHK_NULL(matrixData);

    std::ostringstream oss;
    oss.setf(std::ios::showbase | std::ios::uppercase);

    for (uint32_t j = 0; j < jpegNumComponent; j++)
    {
        oss << "Qmatrix " << std::dec << +j << ": " << std::endl;

        for (int8_t i = 0; i < 56; i += 8)
        {
            oss << "Qmatrix[" << std::dec << +i / 8 << "]:";
            for (uint8_t k = 0; k < 8; k++)
                oss << std::hex << +matrixData->m_quantMatrix[j][i + k] << " ";
            oss << std::endl;
        }
    }

    const char *fileName = m_debugInterface->CreateFileName(
        "_DEC",
        CodechalDbgBufferType::bufIqParams,
        CodechalDbgExtType::txt);

    std::ofstream ofs(fileName, std::ios::out);
    ofs << oss.str();
    ofs.close();

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS JpegPipeline::DumpScanParams(
    CodecDecodeJpegScanParameter *scanParams)
{
    CODECHAL_DEBUG_FUNCTION_ENTER;

    if (!m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrScanParams))
    {
        return MOS_STATUS_SUCCESS;
    }
    CODECHAL_DEBUG_CHK_NULL(scanParams);

    std::ostringstream oss;
    oss.setf(std::ios::showbase | std::ios::uppercase);

    //Dump ScanHeader[jpegNumComponent]
    for (uint32_t i = 0; i < jpegNumComponent; ++i)
    {
        oss << "ScanHeader[" << +i << "].NumComponents: " << +scanParams->ScanHeader[i].NumComponents << std::endl;
        //Dump ComponentSelector[jpegNumComponent]
        for (uint32_t j = 0; j < jpegNumComponent; ++j)
        {
            oss << "ScanHeader[" << +i << "].ComponentSelector[" << +j << "]: " << +scanParams->ScanHeader[i].ComponentSelector[j] << std::endl;
        }

        //Dump DcHuffTblSelector[jpegNumComponent]
        for (uint32_t j = 0; j < jpegNumComponent; ++j)
        {
            oss << "ScanHeader[" << +i << "].DcHuffTblSelector[" << +j << "]: " << +scanParams->ScanHeader[i].DcHuffTblSelector[j] << std::endl;
        }

        //Dump AcHuffTblSelector[jpegNumComponent]
        for (uint32_t j = 0; j < jpegNumComponent; ++j)
        {
            oss << "ScanHeader[" << +i << "].AcHuffTblSelector[" << +j << "]: " << +scanParams->ScanHeader[i].AcHuffTblSelector[j] << std::endl;
        }
        oss << "ScanHeader[" << +i << "].RestartInterval: " << +scanParams->ScanHeader[i].RestartInterval << std::endl;
        oss << "ScanHeader[" << +i << "].MCUCount: " << +scanParams->ScanHeader[i].MCUCount << std::endl;
        oss << "ScanHeader[" << +i << "].ScanHoriPosition: " << +scanParams->ScanHeader[i].ScanHoriPosition << std::endl;
        oss << "ScanHeader[" << +i << "].ScanVertPosition: " << +scanParams->ScanHeader[i].ScanVertPosition << std::endl;
        oss << "ScanHeader[" << +i << "].DataOffset: " << +scanParams->ScanHeader[i].DataOffset << std::endl;
        oss << "ScanHeader[" << +i << "].DataLength: " << +scanParams->ScanHeader[i].DataLength << std::endl;
    }

    oss << "NumScans: " << +scanParams->NumScans << std::endl;

    const char *fileName = m_debugInterface->CreateFileName(
        "_DEC",
        CodechalDbgBufferType::bufScanParams,
        CodechalDbgExtType::txt);

    std::ofstream ofs(fileName, std::ios::out);
    ofs << oss.str();
    ofs.close();
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS JpegPipeline::DumpHuffmanTable(
    PCODECHAL_DECODE_JPEG_HUFFMAN_TABLE huffmanTable)
{
    CODECHAL_DEBUG_FUNCTION_ENTER;
    if (!m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrHuffmanTbl))
    {
        return MOS_STATUS_SUCCESS;
    }
    CODECHAL_DEBUG_CHK_NULL(huffmanTable);

    std::ostringstream oss;
    oss.setf(std::ios::showbase | std::ios::uppercase);

    //Dump HuffTable[JPEG_MAX_NUM_HUFF_TABLE_INDEX]
    for (uint32_t i = 0; i < JPEG_MAX_NUM_HUFF_TABLE_INDEX; ++i)
    {
        //Dump DC_BITS[JPEG_NUM_HUFF_TABLE_DC_BITS]
        oss << "HuffTable[" << +i << "].DC_BITS[0-" << (JPEG_NUM_HUFF_TABLE_DC_BITS - 1) << "]: " << std::endl;

        for (uint32_t j = 0; j < JPEG_NUM_HUFF_TABLE_DC_BITS; ++j)
        {
            oss << +huffmanTable->HuffTable[i].DC_BITS[j] << " ";
            if (j % 6 == 5 || j == JPEG_NUM_HUFF_TABLE_DC_BITS - 1)
            {
                oss << std::endl;
            }
        }
        //Dump DC_HUFFVAL[JPEG_NUM_HUFF_TABLE_DC_HUFFVAL]
        oss << "HuffTable[" << +i << "].DC_HUFFVAL[0-" << (JPEG_NUM_HUFF_TABLE_DC_HUFFVAL - 1) << "]: " << std::endl;
        for (uint32_t j = 0; j < JPEG_NUM_HUFF_TABLE_DC_HUFFVAL; ++j)
        {
            oss << +huffmanTable->HuffTable[i].DC_HUFFVAL[j] << ' ';
            if (j % 6 == 5 || j == JPEG_NUM_HUFF_TABLE_DC_HUFFVAL - 1)
            {
                oss << std::endl;
            }
        }
        //Dump AC_BITS[JPEG_NUM_HUFF_TABLE_AC_BITS]
        oss << "HuffTable[" << +i << "].AC_BITS[0-" << (JPEG_NUM_HUFF_TABLE_AC_BITS - 1) << "]: " << std::endl;

        for (uint32_t j = 0; j < JPEG_NUM_HUFF_TABLE_AC_BITS; ++j)
        {
            oss << +huffmanTable->HuffTable[i].AC_BITS[j] << ' ';
            if (j % 8 == 7 || j == JPEG_NUM_HUFF_TABLE_AC_BITS - 1)
            {
                oss << std::endl;
            }
        }

        //Dump AC_HUFFVAL[JPEG_NUM_HUFF_TABLE_AC_HUFFVAL]
        oss << "HuffTable[" << +i << "].AC_HUFFVAL[0-" << (JPEG_NUM_HUFF_TABLE_AC_HUFFVAL - 1) << "]: " << std::endl;

        for (uint32_t j = 0; j < JPEG_NUM_HUFF_TABLE_AC_HUFFVAL; ++j)
        {
            oss << +huffmanTable->HuffTable[i].AC_HUFFVAL[j] << ' ';
            if (j % 9 == 8 || j == JPEG_NUM_HUFF_TABLE_AC_HUFFVAL - 1)
            {
                oss << std::endl;
            }
        }
    }

    const char *fileName = m_debugInterface->CreateFileName(
        "_DEC",
        CodechalDbgBufferType::bufHuffmanTbl,
        CodechalDbgExtType::txt);

    std::ofstream ofs(fileName, std::ios::out);
    ofs << oss.str();
    ofs.close();
    return MOS_STATUS_SUCCESS;
}


#endif

}
