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
//! \file     decode_mpeg2_pipeline.cpp
//! \brief    Defines the interface for mpeg2 decode pipeline
//!
#include "decode_mpeg2_pipeline.h"
#include "decode_utils.h"
#include "codechal_setting.h"
#include "decode_mpeg2_feature_manager.h"
#include "decode_huc_packet_creator_base.h"
#include "media_debug_fast_dump.h"

namespace decode{

Mpeg2Pipeline::Mpeg2Pipeline(
    CodechalHwInterfaceNext *hwInterface,
    CodechalDebugInterface *debugInterface)
    : DecodePipeline(hwInterface, debugInterface)
{
    MOS_STATUS m_status = InitUserSetting(m_userSettingPtr);
}

MOS_STATUS Mpeg2Pipeline::Initialize(void *settings)
{
    DECODE_FUNC_CALL();

    DECODE_CHK_STATUS(DecodePipeline::Initialize(settings));
    m_basicFeature = dynamic_cast<Mpeg2BasicFeature*>(m_featureManager->GetFeature(FeatureIDs::basicFeature));
    DECODE_CHK_NULL(m_basicFeature);

    // Create basic GPU context
    DecodeScalabilityPars scalPars;
    MOS_ZeroMemory(&scalPars, sizeof(scalPars));
    DECODE_CHK_STATUS(m_mediaContext->SwitchContext(VdboxDecodeFunc, &scalPars, &m_scalability));
    m_decodeContext = m_osInterface->pfnGetGpuContext(m_osInterface);
    m_decodeContextHandle = m_osInterface->CurrentGpuContextHandle;

    HucPacketCreatorBase *hucPktCreator = dynamic_cast<HucPacketCreatorBase *>(this);
    DECODE_CHK_NULL(hucPktCreator);
    m_mpeg2BsCopyPkt    = hucPktCreator->CreateHucCopyPkt(this, m_task, m_hwInterface);
    DECODE_CHK_NULL(m_mpeg2BsCopyPkt);
    MediaPacket *packet = dynamic_cast<MediaPacket *>(m_mpeg2BsCopyPkt);
    DECODE_CHK_NULL(packet);
    DECODE_CHK_STATUS(RegisterPacket(DecodePacketId(this, mpeg2BsCopyPktId), packet));
    DECODE_CHK_STATUS(packet->Init());

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Mpeg2Pipeline::Prepare(void *params)
{
    DECODE_FUNC_CALL();

    DECODE_CHK_NULL(params);
    DECODE_CHK_STATUS(DecodePipeline::Prepare(params));
    DECODE_CHK_STATUS(CopyBitstreamBuffer());

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Mpeg2Pipeline::CopyDummyBitstream()
{
    DECODE_FUNC_CALL();

    HucCopyPktItf::HucCopyParams copyParams = {};

    for (uint16_t slcIdx = 0; slcIdx < m_basicFeature->m_totalNumSlicesRecv; slcIdx++)
    {
        // Copy dummy slice to local buffer
        if (!m_basicFeature->m_copyDummySlicePresent && ((m_basicFeature->m_sliceRecord[slcIdx].prevSliceMbEnd !=
            m_basicFeature->m_sliceRecord[slcIdx].sliceStartMbOffset && !m_basicFeature->m_sliceRecord[slcIdx].skip) ||
            m_basicFeature->m_incompletePicture))
        {
            m_basicFeature->m_copyDummySlicePresent = true;
            copyParams.srcBuffer  = &(m_basicFeature->m_resMpeg2DummyBistream->OsResource);
            copyParams.srcOffset  = 0;
            copyParams.destBuffer = &(m_basicFeature->m_copiedDataBuf->OsResource);
            copyParams.destOffset = m_basicFeature->m_nextCopiedDataOffset;
            copyParams.copyLength = sizeof(m_basicFeature->Mpeg2DummyBsBuf);
            m_mpeg2BsCopyPkt->PushCopyParams(copyParams);

            m_basicFeature->m_dummySliceDataOffset = m_basicFeature->m_nextCopiedDataOffset;
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Mpeg2Pipeline::CopyBitstreamBuffer()
{
    DECODE_FUNC_CALL();

    HucCopyPktItf::HucCopyParams copyParams = {};

    if (m_basicFeature->m_copiedDataNeeded)
    {
        m_basicFeature->m_copiedDataBufferInUse = true;
        if ((m_basicFeature->m_nextCopiedDataOffset + m_basicFeature->m_dataSize) >
            m_basicFeature->m_copiedDataBufferSize)
        {
            DECODE_ASSERTMESSAGE("Copied data buffer is not large enough.");
            m_basicFeature->m_slicesInvalid = true;
            return MOS_STATUS_UNKNOWN;
        }

        uint32_t size = MOS_ALIGN_CEIL(m_basicFeature->m_dataSize, 16);
        copyParams.srcBuffer  = &(m_basicFeature->m_resDataBuffer.OsResource);
        copyParams.srcOffset  = 0;
        copyParams.destBuffer = &(m_basicFeature->m_copiedDataBuf->OsResource);
        copyParams.destOffset = m_basicFeature->m_nextCopiedDataOffset;
        copyParams.copyLength = m_basicFeature->m_dataSize;
        m_mpeg2BsCopyPkt->PushCopyParams(copyParams);

        m_basicFeature->m_copiedDataOffset = m_basicFeature->m_nextCopiedDataOffset;
        m_basicFeature->m_nextCopiedDataOffset += MOS_ALIGN_CEIL(size, MHW_CACHELINE_SIZE);

        bool immediateSubmit = true;
        DECODE_CHK_STATUS(ActivatePacket(DecodePacketId(this, mpeg2BsCopyPktId), immediateSubmit, 0, 0));
        m_activePacketList.back().frameTrackingRequested = false;
        DECODE_CHK_STATUS(ExecuteActivePackets());
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Mpeg2Pipeline::UserFeatureReport()
{
    DECODE_FUNC_CALL();
    DECODE_CHK_STATUS(DecodePipeline::UserFeatureReport());
#if (_DEBUG || _RELEASE_INTERNAL)
    WriteUserFeature(__MEDIA_USER_FEATURE_VALUE_APOGEIOS_MPEG2D_ENABLE_ID, 1, m_osInterface->pOsContext);
#endif

#ifdef _MMC_SUPPORTED
    CODECHAL_DEBUG_TOOL(
        if (m_mmcState != nullptr) {
            m_mmcState->UpdateUserFeatureKey(&(m_basicFeature->m_destSurface));
        })
#endif
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Mpeg2Pipeline::Uninitialize()
{
    DECODE_FUNC_CALL();
    return DecodePipeline::Uninitialize();
}

MOS_STATUS Mpeg2Pipeline::ActivateDecodePackets()
{
    DECODE_FUNC_CALL();

    bool immediateSubmit = false;

    if (m_basicFeature->m_copyDummySlicePresent)
    {
        DECODE_CHK_STATUS(ActivatePacket(DecodePacketId(this, mpeg2BsCopyPktId), immediateSubmit, 0, 0));
    }

    DECODE_CHK_STATUS(ActivatePacket(DecodePacketId(this, mpeg2DecodePacketId), immediateSubmit, 0, 0));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Mpeg2Pipeline::CreateFeatureManager()
{
    DECODE_FUNC_CALL();
    m_featureManager = MOS_New(DecodeMpeg2FeatureManager, m_allocator, m_hwInterface, m_osInterface);
    DECODE_CHK_NULL(m_featureManager);
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Mpeg2Pipeline::CreateSubPackets(DecodeSubPacketManager& subPacketManager, CodechalSetting &codecSettings)
{
    DECODE_FUNC_CALL();
    DECODE_CHK_STATUS(DecodePipeline::CreateSubPackets(subPacketManager, codecSettings));
    return MOS_STATUS_SUCCESS;
}

#if USE_CODECHAL_DEBUG_TOOL
MOS_STATUS Mpeg2Pipeline::DumpPicParams(
    CodecDecodeMpeg2PicParams *picParams)
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
                sizeof(CodecDecodeMpeg2PicParams),
                0,
                MediaDebugSerializer<CodecDecodeMpeg2PicParams>());
        }
        else
        {
            DumpDecodeMpeg2PicParams(picParams, fileName);
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Mpeg2Pipeline::DumpSliceParams(
    CodecDecodeMpeg2SliceParams *sliceParams,
    uint32_t                     numSlices)
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
                sizeof(CodecDecodeMpeg2SliceParams) * numSlices,
                0,
                MediaDebugSerializer<CodecDecodeMpeg2SliceParams>());
        }
        else
        {
            DumpDecodeMpeg2SliceParams(sliceParams, numSlices, fileName);
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Mpeg2Pipeline::DumpMbParams(
    CodecDecodeMpeg2MbParams *mbParams,
    uint32_t                  numMbs)
{
    DECODE_FUNC_CALL();

    if (mbParams == nullptr)
    {
        return MOS_STATUS_SUCCESS;
    }

    if (m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrMbParams))
    {
        const char *fileName = m_debugInterface->CreateFileName(
            "_DEC",
            CodechalDbgBufferType::bufMbParams,
            CodechalDbgExtType::txt);

        if (m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrEnableFastDump))
        {
            MediaDebugFastDump::Dump(
                (uint8_t *)mbParams,
                fileName,
                sizeof(CodecDecodeMpeg2MbParams) * numMbs,
                0,
                MediaDebugSerializer<CodecDecodeMpeg2MbParams>());
        }
        else
        {
            DumpDecodeMpeg2MbParams(mbParams, numMbs, fileName);
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Mpeg2Pipeline::DumpIQParams(
    CodecMpeg2IqMatrix *iqParams)
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
                sizeof(CodecMpeg2IqMatrix),
                0,
                MediaDebugSerializer<CodecMpeg2IqMatrix>());
        }
        else
        {
            DumpDecodeMpeg2IqParams(iqParams, fileName);
        }
    }
        
    return MOS_STATUS_SUCCESS;
}
#endif

}
