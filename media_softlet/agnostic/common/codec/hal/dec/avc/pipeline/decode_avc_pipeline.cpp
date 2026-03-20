/*
* Copyright (c) 2018-2024, Intel Corporation
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
//! \file     decode_avc_pipeline.cpp
//! \brief    Defines the interface for avc decode pipeline
//!
#include "decode_avc_pipeline.h"
#include "decode_utils.h"
#include "codechal_setting.h"
#include "decode_avc_feature_manager.h"
#include "decode_huc_packet_creator_base.h" 
#include "mos_os_cp_interface_specific.h"
#include "media_debug_fast_dump.h"

namespace decode
{
    AvcPipeline::AvcPipeline(
    CodechalHwInterfaceNext *hwInterface,
        CodechalDebugInterface *debugInterface)
        : DecodePipeline(hwInterface, debugInterface)
    {
        MOS_STATUS m_status = InitUserSetting(m_userSettingPtr);
    }

    MOS_STATUS AvcPipeline::Initialize(void *settings)
    {
        DECODE_FUNC_CALL();
        DECODE_CHK_STATUS(DecodePipeline::Initialize(settings));

        m_basicFeature = dynamic_cast<AvcBasicFeature*>(m_featureManager->GetFeature(FeatureIDs::basicFeature));
        DECODE_CHK_NULL(m_basicFeature);

        // Create basic GPU context
        DecodeScalabilityPars scalPars;
        MOS_ZeroMemory(&scalPars, sizeof(scalPars));
        DECODE_CHK_STATUS(m_mediaContext->SwitchContext(VdboxDecodeFunc, &scalPars, &m_scalability));
        m_decodeContext = m_osInterface->pfnGetGpuContext(m_osInterface);
        m_decodeContextHandle = m_osInterface->CurrentGpuContextHandle;

        auto *codecSettings = (CodechalSetting*)settings;
        DECODE_CHK_NULL(codecSettings);
        m_intelEntrypointInUse = (codecSettings->intelEntrypointInUse) ? true : false;
        m_shortFormatInUse     = (codecSettings->shortFormatInUse) ? true : false;

        HucPacketCreatorBase *hucPktCreator = dynamic_cast<HucPacketCreatorBase *>(this);
        DECODE_CHK_NULL(hucPktCreator);
        m_formatMonoPicPkt  = hucPktCreator->CreateHucCopyPkt(this, m_task, m_hwInterface);
        DECODE_CHK_NULL(m_formatMonoPicPkt);
        MediaPacket *packet = dynamic_cast<MediaPacket *>(m_formatMonoPicPkt);
        DECODE_CHK_NULL(packet);
        DECODE_CHK_STATUS(RegisterPacket(DecodePacketId(this, avcFormatMonoPicPktId), packet));
        DECODE_CHK_STATUS(packet->Init());

        return MOS_STATUS_SUCCESS;
    }

    static uint32_t LinearToYTiledAddress(
                uint32_t x,
                uint32_t y,
                uint32_t pitch)
    {
        uint32_t tileW = 128;
        uint32_t tileH = 32;

        uint32_t tileSize = tileW * tileH;

        uint32_t rowSize = (pitch / tileW) * tileSize;

        uint32_t xOffWithinTile = x % tileW;
        uint32_t yOffWithinTile = y % tileH;

        uint32_t tileNumberInX = x / tileW;
        uint32_t tileNumberInY = y / tileH;

        uint32_t tileOffset =
                    rowSize * tileNumberInY +
                    tileSize * tileNumberInX +
                    tileH * 16 * (xOffWithinTile / 16) +
                    yOffWithinTile * 16 +
                    (xOffWithinTile % 16);

        return tileOffset;
    }

    MOS_STATUS AvcPipeline::Prepare(void *params)
    {
        DECODE_FUNC_CALL();
        DECODE_CHK_NULL(params);

        DECODE_CHK_STATUS(DecodePipeline::Prepare(params));

        if (m_basicFeature->m_avcPicParams->seq_fields.chroma_format_idc == avcChromaFormatMono)
        {
            uint32_t height = m_basicFeature->m_destSurface.dwHeight;
            uint32_t pitch = m_basicFeature->m_destSurface.dwPitch;
            uint32_t chromaHeight = height >> 1;
            uint32_t frameHeight = MOS_ALIGN_CEIL(height, 16);
            uint32_t alignedFrameHeight = MOS_ALIGN_CEIL(frameHeight, MOS_YTILE_H_ALIGNMENT);
            uint32_t frameSize = pitch * MOS_ALIGN_CEIL((frameHeight + chromaHeight), MOS_YTILE_H_ALIGNMENT);

            uint32_t uvblockHeight = CODECHAL_MACROBLOCK_HEIGHT;
            uint32_t uvrowSize = pitch * uvblockHeight * 2;
            uint32_t dstOffset = 0, x = 0, uvsize = 0;

            //update decode output surface's cpTag before decode submitbuffer, pfnMediaCopyResource2D can decide clear/secure workload by output surface's cptag.
            if (m_osInterface->osCpInterface && m_osInterface->osCpInterface->IsHMEnabled())
            {
                DECODE_CHK_STATUS(m_osInterface->osCpInterface->SetResourceEncryption(&m_basicFeature->m_destSurface.OsResource, true));
            }

            HucCopyPktItf::HucCopyParams copyParams = {};

            if (frameHeight % MOS_YTILE_H_ALIGNMENT)
            {
                dstOffset = LinearToYTiledAddress(x, frameHeight, pitch);

                if (!m_basicFeature->m_usingVeRing)
                {
                    copyParams.srcBuffer  = &(m_basicFeature->m_resMonoPicChromaBuffer->OsResource);
                    copyParams.srcOffset  = 0;
                    copyParams.destBuffer = &(m_basicFeature->m_destSurface.OsResource);
                    copyParams.destOffset = dstOffset;
                    copyParams.copyLength = uvrowSize;
                    m_formatMonoPicPkt->PushCopyParams(copyParams);
                }
                else
                {
                    m_osInterface->pfnMonoSurfaceCopy(
                        m_osInterface,
                        &m_basicFeature->m_resMonoPicChromaBuffer->OsResource,
                        &m_basicFeature->m_destSurface.OsResource,
                        pitch,
                        uvblockHeight * 2,
                        0,
                        dstOffset,
                        false);
                }
            }

            dstOffset = m_basicFeature->m_destSurface.UPlaneOffset.iSurfaceOffset;
            uvsize    = frameSize - pitch * alignedFrameHeight;

            if (!m_basicFeature->m_usingVeRing)
            {
                copyParams.srcBuffer  = &(m_basicFeature->m_resMonoPicChromaBuffer->OsResource);
                copyParams.srcOffset  = 0;
                copyParams.destBuffer = &(m_basicFeature->m_destSurface.OsResource);
                copyParams.destOffset = dstOffset;
                copyParams.copyLength = uvsize;
                m_formatMonoPicPkt->PushCopyParams(copyParams);
            }
            else
            {
                m_osInterface->pfnMonoSurfaceCopy(
                    m_osInterface,
                    &m_basicFeature->m_resMonoPicChromaBuffer->OsResource,
                    &m_basicFeature->m_destSurface.OsResource,
                    pitch,
                    uvsize / pitch,
                    0,
                    dstOffset,
                    false);
            }
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AvcPipeline::UserFeatureReport()
    {
        DECODE_FUNC_CALL();
        DECODE_CHK_STATUS(DecodePipeline::UserFeatureReport());
    #if (_DEBUG || _RELEASE_INTERNAL)
        WriteUserFeature(__MEDIA_USER_FEATURE_VALUE_APOGEIOS_AVCD_ENABLE_ID, 1, m_osInterface->pOsContext);
    #endif
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AvcPipeline::Uninitialize()
    {
        DECODE_FUNC_CALL();

        return DecodePipeline::Uninitialize();
    }

    MOS_STATUS AvcPipeline::ActivateDecodePackets()
    {
        DECODE_FUNC_CALL();

        bool immediateSubmit = false;

        if (m_basicFeature->m_avcPicParams->seq_fields.chroma_format_idc == avcChromaFormatMono && !m_basicFeature->m_usingVeRing)
        {
            DECODE_CHK_STATUS(ActivatePacket(DecodePacketId(this, avcFormatMonoPicPktId), immediateSubmit, 0, 0));
        }

        for (uint8_t curPass = 0; curPass < GetPassNum(); curPass++)
        {
            DECODE_CHK_STATUS(ActivatePacket(DecodePacketId(this, avcDecodePacketId), immediateSubmit, curPass, 0));
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AvcPipeline::CreateFeatureManager()
    {
        DECODE_FUNC_CALL();
        m_featureManager = MOS_New(DecodeAvcFeatureManager, m_allocator, m_hwInterface, m_osInterface);
        DECODE_CHK_NULL(m_featureManager);
        return MOS_STATUS_SUCCESS;
    }

     MOS_STATUS AvcPipeline::CreateSubPackets(DecodeSubPacketManager& subPacketManager, CodechalSetting &codecSettings)
    {
        DECODE_FUNC_CALL();

        DECODE_CHK_STATUS(DecodePipeline::CreateSubPackets(subPacketManager, codecSettings));

        return MOS_STATUS_SUCCESS;
    }

    AvcPipeline::AvcDecodeMode AvcPipeline::GetDecodeMode()
    {
        return m_decodeMode;
    }

    bool AvcPipeline::IsShortFormat()
    {
        return m_shortFormatInUse;
    }

    MOS_STATUS AvcPipeline::SetDecodeFormat(bool isShortFormat)
    {
        DECODE_FUNC_CALL();
        DECODE_CHK_NULL(m_basicFeature);
        m_basicFeature->m_shortFormatInUse = isShortFormat;
        m_shortFormatInUse = isShortFormat;
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AvcPipeline::HandleRefOnlySurfaces()
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;
#ifdef _DECODE_PROCESSING_SUPPORTED
        DecodeDownSamplingFeature *downSamplingFeature = dynamic_cast<DecodeDownSamplingFeature *>(
            m_featureManager->GetFeature(DecodeFeatureIDs::decodeDownSampling));
        if (downSamplingFeature != nullptr)
        {
            if (downSamplingFeature->m_inputSurface != nullptr && downSamplingFeature->m_isReferenceOnlyPattern == true)
            {
                eStatus = m_osInterface->pfnDoubleBufferCopyResource(
                            m_osInterface,
                            &m_basicFeature->m_destSurface.OsResource,
                            &downSamplingFeature->m_inputSurface->OsResource,
                            false);
            }
        }
#endif
        return eStatus;
    }
#if USE_CODECHAL_DEBUG_TOOL
MOS_STATUS AvcPipeline::DumpPicParams(
    PCODEC_AVC_PIC_PARAMS picParams)
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
                sizeof(CODEC_AVC_PIC_PARAMS),
                0,
                MediaDebugSerializer<CODEC_AVC_PIC_PARAMS>());
        }
        else
        {
            DumpDecodeAvcPicParams(picParams, fileName);
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS AvcPipeline::DumpSliceParams(
    PCODEC_AVC_SLICE_PARAMS slcParams,
    uint32_t                numSlices,
    bool                    shortFormatInUse)
{
    DECODE_FUNC_CALL();

    if (slcParams == nullptr)
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
            if (shortFormatInUse)
            {
                PCODEC_AVC_SF_SLICE_PARAMS slcParamsSF = 
                    (PCODEC_AVC_SF_SLICE_PARAMS)MOS_AllocMemory(sizeof(PCODEC_AVC_SF_SLICE_PARAMS) * numSlices);

                for (uint16_t i = 0; i < numSlices; i++)
                {
                    slcParamsSF[i] = reinterpret_cast<CODEC_AVC_SF_SLICE_PARAMS &>(slcParams[i]);
                }

                MediaDebugFastDump::Dump(
                    (uint8_t *)slcParamsSF,
                    fileName,
                    sizeof(CODEC_AVC_SF_SLICE_PARAMS) * numSlices,
                    0,
                    MediaDebugSerializer<CODEC_AVC_SF_SLICE_PARAMS>());
            }
            else
            {
                MediaDebugFastDump::Dump(
                    (uint8_t *)slcParams,
                    fileName,
                    sizeof(CODEC_AVC_SLICE_PARAMS) * numSlices,
                    0,
                    MediaDebugSerializer<CODEC_AVC_SLICE_PARAMS>());
            }
        }
        else
        {
            DumpDecodeAvcSliceParams(slcParams, numSlices, fileName, shortFormatInUse);
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS AvcPipeline::DumpIQParams(
    PCODEC_AVC_IQ_MATRIX_PARAMS iqParams)
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
                sizeof(CODEC_AVC_IQ_MATRIX_PARAMS),
                0,
                MediaDebugSerializer<CODEC_AVC_IQ_MATRIX_PARAMS>());
        }
        else
        {
            DumpDecodeAvcIQParams(iqParams, fileName);
        }
    }

    return MOS_STATUS_SUCCESS;
}
#endif
}
