/*
* Copyright (c) 2022-2024, Intel Corporation
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
//! \file     decode_hevc_picture_packet.cpp
//! \brief    Defines the interface for hevc decode picture packet
//!
#include "codec_utilities_next.h"
#include "decode_hevc_picture_packet.h"
#include "decode_hevc_phase_real_tile.h"
#include "decode_hevc_phase_front_end.h"
#include "decode_hevc_phase_back_end.h"
#include "decode_common_feature_defs.h"
#include "decode_hevc_mem_compression.h"
#include "mhw_vdbox_hcp_itf.h"
#include "mhw_impl.h"
#include "decode_utils.h"

using namespace mhw::vdbox::hcp;

const mhw::vdbox::hcp::Itf::HevcSliceType mhw::vdbox::hcp::Itf::m_hevcBsdSliceType[3] =
{
    mhw::vdbox::hcp::Itf::hevcSliceB,
    mhw::vdbox::hcp::Itf::hevcSliceP,
    mhw::vdbox::hcp::Itf::hevcSliceI
};

namespace decode
{
    HevcDecodePicPkt::~HevcDecodePicPkt()
    {
        FreeResources();
    }

    MOS_STATUS HevcDecodePicPkt::FreeResources()
    {
        DECODE_FUNC_CALL();

        if (m_allocator != nullptr)
        {
            m_allocator->Destroy(m_resMfdDeblockingFilterRowStoreScratchBuffer);
            m_allocator->Destroy(m_resMfdDeblockingFilterRowStoreScratchBuffer);
            m_allocator->Destroy(m_resDeblockingFilterTileRowStoreScratchBuffer);
            m_allocator->Destroy(m_resDeblockingFilterColumnRowStoreScratchBuffer);
            m_allocator->Destroy(m_resMetadataLineBuffer);
            m_allocator->Destroy(m_resMetadataTileLineBuffer);
            m_allocator->Destroy(m_resMetadataTileColumnBuffer);
            m_allocator->Destroy(m_resSaoLineBuffer);
            m_allocator->Destroy(m_resSaoTileLineBuffer);
            m_allocator->Destroy(m_resSaoTileColumnBuffer);
            m_allocator->Destroy(m_resSliceStateStreamOutBuffer);
            m_allocator->Destroy(m_resMvUpRightColStoreBuffer);
            m_allocator->Destroy(m_resIntraPredUpRightColStoreBuffer);
            m_allocator->Destroy(m_resIntraPredLeftReconColStoreBuffer);
            m_allocator->Destroy(m_resCABACSyntaxStreamOutBuffer);
            m_allocator->Destroy(m_resCABACStreamOutSizeBuffer);
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HevcDecodePicPkt::Init()
    {
        DECODE_FUNC_CALL();
        DECODE_CHK_NULL(m_featureManager);
        DECODE_CHK_NULL(m_hwInterface);
        DECODE_CHK_NULL(m_osInterface);
        DECODE_CHK_NULL(m_miItf);
        DECODE_CHK_NULL(m_hevcPipeline);
        DECODE_CHK_NULL(m_hcpItf);

        m_hevcBasicFeature = dynamic_cast<HevcBasicFeature*>(m_featureManager->GetFeature(FeatureIDs::basicFeature));
        DECODE_CHK_NULL(m_hevcBasicFeature);

#ifdef _DECODE_PROCESSING_SUPPORTED
        m_downSamplingFeature      = dynamic_cast<DecodeDownSamplingFeature *>(m_featureManager->GetFeature(DecodeFeatureIDs::decodeDownSampling));
        DecodeSubPacket *subPacket = m_hevcPipeline->GetSubPacket(DecodePacketId(m_hevcPipeline, downSamplingSubPacketId));
        m_downSamplingPkt          = dynamic_cast<DecodeDownSamplingPkt *>(subPacket);
#endif

        m_allocator = m_pipeline ->GetDecodeAllocator();
        DECODE_CHK_NULL(m_allocator);

        DECODE_CHK_STATUS(AllocateFixedResources());

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HevcDecodePicPkt::Prepare()
    {
        DECODE_FUNC_CALL();

        m_hevcPicParams      = m_hevcBasicFeature->m_hevcPicParams;
        m_hevcIqMatrixParams = m_hevcBasicFeature->m_hevcIqMatrixParams;
        m_hevcRextPicParams  = m_hevcBasicFeature->m_hevcRextPicParams;
        m_hevcSccPicParams   = m_hevcBasicFeature->m_hevcSccPicParams;

#ifdef _MMC_SUPPORTED
        m_mmcState = m_hevcPipeline->GetMmcState();
        DECODE_CHK_NULL(m_mmcState);
#endif

        DECODE_CHK_STATUS(SetRowstoreCachingOffsets());
        DECODE_CHK_STATUS(AllocateVariableResources());

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HevcDecodePicPkt::SetPhase(DecodePhase *phase)
    {
        DECODE_FUNC_CALL();
        m_phase = phase;
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HevcDecodePicPkt::ReportCabacStreamOutSize(MOS_COMMAND_BUFFER &cmdBuffer)
    {
        DECODE_FUNC_CALL();
        DECODE_CHK_NULL(m_resCABACStreamOutSizeBuffer);

        auto mmioRegistersHcp = m_hwInterface->GetHcpInterfaceNext()->GetMmioRegisters(MHW_VDBOX_NODE_1);

        auto &par = m_miItf->GETPAR_MI_STORE_REGISTER_MEM();
        par                    = {};
        par.presStoreBuffer    = &m_resCABACStreamOutSizeBuffer->OsResource;
        par.dwOffset           = 0;
        par.dwRegister         = mmioRegistersHcp->hcpDebugFEStreamOutSizeRegOffset;

        DECODE_CHK_STATUS(m_miItf->ADDCMD_MI_STORE_REGISTER_MEM(&cmdBuffer));

        return MOS_STATUS_SUCCESS;
    }

    bool HevcDecodePicPkt::IsRealTilePhase()
    {
        if (m_phase == nullptr)
        {
            return false;
        }
        HevcPhaseRealTile *realtilePhase = dynamic_cast<HevcPhaseRealTile *>(m_phase);
        return (realtilePhase != nullptr);
    }

    bool HevcDecodePicPkt::IsFrontEndPhase()
    {
        if (m_phase == nullptr)
        {
            return false;
        }
        HevcPhaseFrontEnd *frontEndPhase = dynamic_cast<HevcPhaseFrontEnd *>(m_phase);
        return (frontEndPhase != nullptr);
    }

    bool HevcDecodePicPkt::IsBackEndPhase()
    {
        if (m_phase == nullptr)
        {
            return false;
        }
        HevcPhaseBackEnd *backEndPhase = dynamic_cast<HevcPhaseBackEnd *>(m_phase);
        return (backEndPhase != nullptr);
    }

    MOS_STATUS HevcDecodePicPkt::SetRowstoreCachingOffsets()
    {
        if (m_hcpItf->IsRowStoreCachingSupported())
        {
            HcpVdboxRowStorePar rowstoreParams;
            rowstoreParams.Mode             = CODECHAL_DECODE_MODE_HEVCVLD;
            rowstoreParams.dwPicWidth       = m_hevcBasicFeature->m_width;
            rowstoreParams.bMbaff           = false;
            rowstoreParams.ucBitDepthMinus8 = (uint8_t)MOS_MAX(m_hevcPicParams->bit_depth_luma_minus8,
                m_hevcPicParams->bit_depth_chroma_minus8);
            rowstoreParams.ucChromaFormat   = m_hevcPicParams->chroma_format_idc;
            rowstoreParams.ucLCUSize        = (uint8_t)m_hevcBasicFeature->m_ctbSize;
            DECODE_CHK_STATUS(m_hcpItf->SetRowstoreCachingOffsets(rowstoreParams));
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HevcDecodePicPkt::AllocateFixedResources()
    {
        DECODE_FUNC_CALL();

        if (m_resSliceStateStreamOutBuffer == nullptr)
        {
            uint32_t sizeOfBuffer = CODECHAL_HEVC_MAX_NUM_SLICES_LVL_6 * sliceStateCachelinesPerSlice * CODECHAL_CACHELINE_SIZE;
            m_resSliceStateStreamOutBuffer = m_allocator->AllocateBuffer(
                sizeOfBuffer,
                "SliceStateStreamOut",
                resourceInternalReadWriteCache,
                notLockableVideoMem);
            DECODE_CHK_NULL(m_resSliceStateStreamOutBuffer);
        }

        if (m_resCABACStreamOutSizeBuffer == nullptr)
        {
            m_resCABACStreamOutSizeBuffer = m_allocator->AllocateBuffer(
                sizeof(uint64_t),
                "CABACStreamOutSizeBuffer",
                resourceInternalReadWriteCache,
                notLockableVideoMem);
            DECODE_CHK_NULL(m_resCABACStreamOutSizeBuffer);
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HevcDecodePicPkt::AllocateVariableResources()
    {
        DECODE_FUNC_CALL();

        HcpBufferSizePar hcpBufSizePar;
        MOS_ZeroMemory(&hcpBufSizePar, sizeof(hcpBufSizePar));

        hcpBufSizePar.ucMaxBitDepth  = m_hevcBasicFeature->m_bitDepth;
        hcpBufSizePar.ucChromaFormat = m_hevcBasicFeature->m_chromaFormat;
        hcpBufSizePar.dwCtbLog2SizeY = m_hevcPicParams->log2_diff_max_min_luma_coding_block_size +
                                       m_hevcPicParams->log2_min_luma_coding_block_size_minus3 + 3;
        hcpBufSizePar.dwPicWidth     = m_hevcBasicFeature->m_width;
        hcpBufSizePar.dwPicHeight    = m_hevcBasicFeature->m_height;
        hcpBufSizePar.dwMaxFrameSize = m_hevcBasicFeature->m_dataSize;

        auto AllocateBuffer = [&](PMOS_BUFFER &buffer, const HCP_INTERNAL_BUFFER_TYPE bufferType, const char *bufferName) {
            uint32_t bufSize = 0;
            hcpBufSizePar.bufferType = bufferType;
            DECODE_CHK_STATUS(m_hcpItf->GetHcpBufSize(hcpBufSizePar, bufSize));

            if (buffer == nullptr)
            {
                buffer = m_allocator->AllocateBuffer(bufSize, bufferName, resourceInternalReadWriteCache, notLockableVideoMem);
                DECODE_CHK_NULL(buffer);
            }
            else
            {
                DECODE_CHK_STATUS(m_allocator->Resize(buffer, bufSize, notLockableVideoMem));
            }
            return MOS_STATUS_SUCCESS;
        };

        if (!m_hcpItf->IsHevcDfRowstoreCacheEnabled())
        {
            // Deblocking Filter Row Store Scratch buffer
            DECODE_CHK_STATUS(AllocateBuffer(m_resMfdDeblockingFilterRowStoreScratchBuffer, HCP_INTERNAL_BUFFER_TYPE::DBLK_LINE, "DeblockingScratchBuffer"));
        }

        // Deblocking Filter Tile Row Store Scratch data surface
        DECODE_CHK_STATUS(AllocateBuffer(m_resDeblockingFilterTileRowStoreScratchBuffer, HCP_INTERNAL_BUFFER_TYPE::DBLK_TILE_LINE, "DeblockingTileScratchBuffer"));

        // Deblocking Filter Column Row Store Scratch data surface
        DECODE_CHK_STATUS(AllocateBuffer(m_resDeblockingFilterColumnRowStoreScratchBuffer, HCP_INTERNAL_BUFFER_TYPE::DBLK_TILE_COL, "DeblockingColumnScratchBuffer"));

        if (!m_hcpItf->IsHevcDatRowstoreCacheEnabled())
        {
            // Metadata Line buffer
            DECODE_CHK_STATUS(AllocateBuffer(m_resMetadataLineBuffer, HCP_INTERNAL_BUFFER_TYPE::META_LINE, "MetadataLineBuffer"));
        }

        // Metadata Tile Line buffer
        DECODE_CHK_STATUS(AllocateBuffer(m_resMetadataTileLineBuffer, HCP_INTERNAL_BUFFER_TYPE::META_TILE_LINE, "MetadataTileLineBuffer"));

        // Metadata Tile Column buffer
        DECODE_CHK_STATUS(AllocateBuffer(m_resMetadataTileColumnBuffer, HCP_INTERNAL_BUFFER_TYPE::META_TILE_COL, "MetadataTileColumnBuffer"));

        if (!m_hcpItf->IsHevcSaoRowstoreCacheEnabled())
        {
            // SAO Line buffer
            DECODE_CHK_STATUS(AllocateBuffer(m_resSaoLineBuffer, HCP_INTERNAL_BUFFER_TYPE::SAO_LINE, "SaoLineBuffer"));
        }

        // SAO Tile Line buffer
        DECODE_CHK_STATUS(AllocateBuffer(m_resSaoTileLineBuffer, HCP_INTERNAL_BUFFER_TYPE::SAO_TILE_LINE, "SaoTileLineBuffer"));

        // SAO Tile Column buffer
        DECODE_CHK_STATUS(AllocateBuffer(m_resSaoTileColumnBuffer, HCP_INTERNAL_BUFFER_TYPE::SAO_TILE_COL, "SaoTileColumnBuffer"));

        // MV up right column store buffer
        DECODE_CHK_STATUS(AllocateBuffer(m_resMvUpRightColStoreBuffer, HCP_INTERNAL_BUFFER_TYPE::MV_UP_RT_COL, "MVUpperRightColumnStore"));

        // Intra prediction up right column store buffer
        DECODE_CHK_STATUS(AllocateBuffer(m_resIntraPredUpRightColStoreBuffer, HCP_INTERNAL_BUFFER_TYPE::INTRA_PRED_UP_RIGHT_COL, "MVUpperRightColumnStore"));

        // Intra prediction left recon column store buffer
        DECODE_CHK_STATUS(AllocateBuffer(m_resIntraPredLeftReconColStoreBuffer, HCP_INTERNAL_BUFFER_TYPE::INTRA_PRED_LFT_RECON_COL, "IntraPredLeftReconColumnStore"));

        // Cabac stream out buffer
        DECODE_CHK_STATUS(AllocateBuffer(m_resCABACSyntaxStreamOutBuffer, HCP_INTERNAL_BUFFER_TYPE::CABAC_STREAMOUT, "CABACStreamOutBuffer"));

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(HCP_PIPE_MODE_SELECT, HevcDecodePicPkt)
    {
        DECODE_FUNC_CALL();

        params.codecSelect         = 0; // CODEC_SELECT_DECODE
        params.codecStandardSelect = CodecHal_GetStandardFromMode(m_hevcBasicFeature->m_mode) - CODECHAL_HCP_BASE;
        params.bStreamOutEnabled   = false;

        auto cpInterface = m_hwInterface->GetCpInterface();
        DECODE_CHK_NULL(cpInterface);

        bool twoPassScalable         = false; // !decodeInUse
        params.setProtectionSettings = [=](uint32_t *data) { return cpInterface->SetProtectionSettingsForHcpPipeModeSelect(data, twoPassScalable); };

        params.mediaSoftResetCounterPer1000Clocks = 0;
#if (_DEBUG || _RELEASE_INTERNAL)
        if (m_osInterface->bSoftReset)
        {
            params.mediaSoftResetCounterPer1000Clocks = 500;
        }
#endif
        auto waTable = m_osInterface->pfnGetWaTable(m_osInterface);
        DECODE_CHK_NULL(waTable);

        if (MEDIA_IS_WA(waTable, Wa_14012254246))
        {
            auto userSettingPtr = m_osInterface->pfnGetUserSettingInstance(m_osInterface);
            params.prefetchDisable = ReadUserFeature(userSettingPtr, "DisableTlbPrefetch", MediaUserSetting::Group::Sequence).Get<bool>();
        }

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(HCP_SURFACE_STATE, HevcDecodePicPkt)
    {
        DECODE_FUNC_CALL();

        uint8_t      chromaType             = m_hevcPicParams->chroma_format_idc;
        uint8_t      ucBitDepthLumaMinus8   = m_hevcPicParams->bit_depth_luma_minus8;
        uint8_t      ucBitDepthChromaMinus8 = m_hevcPicParams->bit_depth_luma_minus8;
        uint32_t     dwUVPlaneAlignment     = 1 << (m_hevcPicParams->log2_min_luma_coding_block_size_minus3 + 3);
        PMOS_SURFACE psSurface              = &m_hevcBasicFeature->m_destSurface; // For HEVC decode, reference should be same format as dest surface
        MHW_MI_CHK_NULL(psSurface);

        uint32_t uvPlaneAlignment = m_uvPlaneAlignmentLegacy;

        params.surfaceStateId     = m_curHcpSurfStateId;
        params.surfacePitchMinus1 = psSurface->dwPitch - 1;

        if (ucBitDepthLumaMinus8 == 0 && ucBitDepthChromaMinus8 == 0)
        {
            if (chromaType == HCP_CHROMA_FORMAT_YUV420 && psSurface->Format == Format_NV12) // 4:2:0 8bit surface
            {
                params.surfaceFormat = SURFACE_FORMAT::SURFACE_FORMAT_PLANAR4208;
            }
            else if (chromaType == HCP_CHROMA_FORMAT_YUV420 && psSurface->Format == Format_P010) // 4:2:0 10bit surface
            {
                params.surfaceFormat = SURFACE_FORMAT::SURFACE_FORMAT_P010;
            }
            else if (chromaType == HCP_CHROMA_FORMAT_YUV422 && psSurface->Format == Format_YUY2) // 4:2:2 8bit surface
            {
                params.surfaceFormat = SURFACE_FORMAT::SURFACE_FORMAT_YUY2FORMAT;
            }
            else if (chromaType == HCP_CHROMA_FORMAT_YUV422 && psSurface->Format == Format_Y210) // 4:2:2 10bit surface
            {
                params.surfaceFormat = SURFACE_FORMAT::SURFACE_FORMAT_Y216Y210FORMAT;
            }
            else if (chromaType == HCP_CHROMA_FORMAT_YUV444 && psSurface->Format == Format_AYUV) // 4:4:4 8bit surface
            {
                params.surfaceFormat = SURFACE_FORMAT::SURFACE_FORMAT_AYUV4444FORMAT;
            }
            else if (chromaType == HCP_CHROMA_FORMAT_YUV444 && psSurface->Format == Format_Y410) // 4:4:4 10bit surface
            {
                params.surfaceFormat = SURFACE_FORMAT::SURFACE_FORMAT_Y410FORMAT;
            }
            else if (chromaType == HCP_CHROMA_FORMAT_YUV420 && psSurface->Format == Format_P016) // 4:2:0 16bit surface
            {
                params.surfaceFormat = SURFACE_FORMAT::SURFACE_FORMAT_P016;
            }
            else if (chromaType == HCP_CHROMA_FORMAT_YUV422 && psSurface->Format == Format_Y216) // 4:2:2 16bit surface
            {
                params.surfaceFormat = SURFACE_FORMAT::SURFACE_FORMAT_Y216Y210FORMAT;
            }
            else if (chromaType == HCP_CHROMA_FORMAT_YUV444 && psSurface->Format == Format_Y416) // 4:4:4
            {
                params.surfaceFormat = SURFACE_FORMAT::SURFACE_FORMAT_Y416FORMAT;
            }
            else
            {
                return MOS_STATUS_INVALID_PARAMETER;
            }
        }
        else if ((ucBitDepthLumaMinus8 <= 2) && (ucBitDepthChromaMinus8 <= 2)) // only support bitdepth <= 10bit
        {
            if (chromaType == HCP_CHROMA_FORMAT_YUV420 && psSurface->Format == Format_P010) // 4:2:0
            {
                params.surfaceFormat = SURFACE_FORMAT::SURFACE_FORMAT_P010;
            }
            else if (chromaType == HCP_CHROMA_FORMAT_YUV420 && psSurface->Format == Format_P016) // 4:2:0
            {
                params.surfaceFormat = SURFACE_FORMAT::SURFACE_FORMAT_P016;
            }
            else if (chromaType == HCP_CHROMA_FORMAT_YUV422 && psSurface->Format == Format_Y210) // 4:2:2
            {
                params.surfaceFormat = SURFACE_FORMAT::SURFACE_FORMAT_Y216Y210FORMAT;
            }
            else if (chromaType == HCP_CHROMA_FORMAT_YUV422 && psSurface->Format == Format_Y216) // 4:2:2
            {
                params.surfaceFormat = SURFACE_FORMAT::SURFACE_FORMAT_Y216Y210FORMAT;
            }
            else if (chromaType == HCP_CHROMA_FORMAT_YUV444 && psSurface->Format == Format_Y410) // 4:4:4
            {
                params.surfaceFormat = SURFACE_FORMAT::SURFACE_FORMAT_Y410FORMAT;
            }
            else if (chromaType == HCP_CHROMA_FORMAT_YUV444 && psSurface->Format == Format_Y416) // 4:4:4
            {
                params.surfaceFormat = SURFACE_FORMAT::SURFACE_FORMAT_Y416FORMAT;
            }
            else
            {
                return MOS_STATUS_INVALID_PARAMETER;
            }
        }
        else // 12bit
        {
            if (chromaType == HCP_CHROMA_FORMAT_YUV420 && psSurface->Format == Format_P016) // 4:2:0
            {
                params.surfaceFormat = SURFACE_FORMAT::SURFACE_FORMAT_P016;
            }
            else if (chromaType == HCP_CHROMA_FORMAT_YUV422 && psSurface->Format == Format_Y216) // 4:2:2
            {
                params.surfaceFormat = SURFACE_FORMAT::SURFACE_FORMAT_Y216Y210FORMAT;
            }
            else if (chromaType == HCP_CHROMA_FORMAT_YUV444 && psSurface->Format == Format_Y416) // 4:4:4
            {
                params.surfaceFormat = SURFACE_FORMAT::SURFACE_FORMAT_Y416FORMAT;
            }
            else
            {
                return MOS_STATUS_INVALID_PARAMETER;
            }
        }

        if (m_curHcpSurfStateId == CODECHAL_HCP_SRC_SURFACE_ID)
        {
            uvPlaneAlignment = dwUVPlaneAlignment ? dwUVPlaneAlignment : m_rawUVPlaneAlignment;
        }
        else
        {
            uvPlaneAlignment = dwUVPlaneAlignment ? dwUVPlaneAlignment : m_reconUVPlaneAlignment;
        }

        params.yOffsetForUCbInPixel =
            MOS_ALIGN_CEIL((psSurface->UPlaneOffset.iSurfaceOffset - psSurface->dwOffset) / psSurface->dwPitch + psSurface->RenderOffset.YUV.U.YOffset, uvPlaneAlignment);

        if ((ucBitDepthLumaMinus8 == 4) || (ucBitDepthChromaMinus8 == 4)) // 12 bit
        {
            params.defaultAlphaValue = 0xfff0;
        }
        else
        {
            params.defaultAlphaValue = 0xffff;
        }

#ifdef _MMC_SUPPORTED
        if (m_curHcpSurfStateId == CODECHAL_HCP_DECODED_SURFACE_ID)
        {
            DECODE_CHK_STATUS(m_mmcState->SetSurfaceMmcState(psSurface));
            DECODE_CHK_STATUS(m_mmcState->GetSurfaceMmcState(psSurface, &params.mmcState));
            DECODE_CHK_STATUS(m_mmcState->GetSurfaceMmcFormat(psSurface, &params.dwCompressionFormat));
        }
        else if (m_curHcpSurfStateId == CODECHAL_HCP_REF_SURFACE_ID)
        {
            auto &pipeBufAddrPar = m_hcpItf->MHW_GETPAR_F(HCP_PIPE_BUF_ADDR_STATE)();

            HevcDecodeMemComp *hevcDecodeMemComp = dynamic_cast<HevcDecodeMemComp *>(m_mmcState);
            DECODE_CHK_NULL(hevcDecodeMemComp);
            if (m_hevcBasicFeature->m_isSCCIBCMode)
            {
                HevcReferenceFrames &refFrames = m_hevcBasicFeature->m_refFrames;
                DECODE_CHK_NULL(m_hevcBasicFeature->m_hevcPicParams);
                const std::vector<uint8_t> &activeRefList = refFrames.GetActiveReferenceList(*m_hevcBasicFeature->m_hevcPicParams);
                uint8_t                     IBCRefIdx     = refFrames.m_IBCRefIdx;
                if (activeRefList.size() <= IBCRefIdx)
                {
                    DECODE_ASSERTMESSAGE("Invalid IBC reference index.");
                }
                uint8_t IBCFrameIdx = activeRefList[IBCRefIdx];
                for (uint8_t i = 0; i < CODECHAL_MAX_CUR_NUM_REF_FRAME_HEVC; i++)
                {
                    MOS_MEMCOMP_STATE mmcState = MOS_MEMCOMP_DISABLED;

                    if (pipeBufAddrPar.presReferences[i] == nullptr || 
                        pipeBufAddrPar.presReferences[i] == refFrames.GetReferenceByFrameIndex(IBCFrameIdx))  // Skip MMC Setting use default MOS_MEMCOMP_DISABLED value to control IBC ref suraface
                    {
                        continue;
                    }
                    DECODE_CHK_STATUS(m_mmcState->GetResourceMmcState(pipeBufAddrPar.presReferences[i], mmcState));
                    params.refsMmcEnable |= (mmcState == MOS_MEMCOMP_RC || mmcState == MOS_MEMCOMP_MC) ? (1 << i) : 0;
                    params.refsMmcType |= (mmcState == MOS_MEMCOMP_RC) ? (1 << i) : 0;
                    if (m_mmcState->IsMmcEnabled())
                    {
                        DECODE_CHK_STATUS(m_mmcState->GetResourceMmcFormat(pipeBufAddrPar.presReferences[i], params.dwCompressionFormat));
                    }
                }
            }
            else
            {
                for (uint8_t i = 0; i < CODECHAL_MAX_CUR_NUM_REF_FRAME_HEVC; i++)
                {
                    MOS_MEMCOMP_STATE mmcState = MOS_MEMCOMP_DISABLED;

                    if (pipeBufAddrPar.presReferences[i] == nullptr)
                    {
                        continue;
                    }

                    DECODE_CHK_STATUS(m_mmcState->GetResourceMmcState(pipeBufAddrPar.presReferences[i], mmcState));
                    params.refsMmcEnable |= (mmcState == MOS_MEMCOMP_RC || mmcState == MOS_MEMCOMP_MC) ? (1 << i) : 0;
                    params.refsMmcType |= (mmcState == MOS_MEMCOMP_RC) ? (1 << i) : 0;
                    if (m_mmcState->IsMmcEnabled())
                    {
                        DECODE_CHK_STATUS(m_mmcState->GetResourceMmcFormat(pipeBufAddrPar.presReferences[i], params.dwCompressionFormat));
                    }
                }
            }
        }
#endif

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HevcDecodePicPkt::AddAllCmds_HCP_SURFACE_STATE(MOS_COMMAND_BUFFER &cmdBuffer)
    {
        DECODE_FUNC_CALL();

        auto &params = m_hcpItf->MHW_GETPAR_F(HCP_SURFACE_STATE)();
        params       = {};

        m_curHcpSurfStateId = CODECHAL_HCP_DECODED_SURFACE_ID;
        SETPAR_AND_ADDCMD(HCP_SURFACE_STATE, m_hcpItf, &cmdBuffer);

        params              = {};
        m_curHcpSurfStateId = CODECHAL_HCP_REF_SURFACE_ID;
        SETPAR_AND_ADDCMD(HCP_SURFACE_STATE, m_hcpItf, &cmdBuffer);

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HevcDecodePicPkt::FixHcpPipeBufAddrParams(HCP_PIPE_BUF_ADDR_STATE_PAR &params) const
    {
        DECODE_FUNC_CALL();

        if (m_hevcBasicFeature->m_refFrames.m_curIsIntra)
        {
            PMOS_RESOURCE dummyRef = &(m_hevcBasicFeature->m_dummyReference.OsResource);
            if (m_hevcBasicFeature->m_useDummyReference &&
                !m_allocator->ResourceIsNull(dummyRef))
            {
                // set all ref pic addresses to valid addresses for error concealment purpose
                for (uint32_t i = 0; i < CODECHAL_MAX_CUR_NUM_REF_FRAME_HEVC; i++)
                {
                    if (params.presReferences[i] == nullptr)
                    {
                        params.presReferences[i]                    = dummyRef;
                        m_hevcBasicFeature->m_dummyReferenceSlot[i] = true;
                    }
                }
            }
        }
        else
        {
            PMOS_RESOURCE validRef = m_hevcBasicFeature->m_refFrames.GetValidReference();
            for (uint8_t i = 0; i < CODECHAL_MAX_CUR_NUM_REF_FRAME_HEVC; i++)
            {
                // error concealment for the unset reference addresses and unset mv buffers
                if (params.presReferences[i] == nullptr)
                {
                    params.presReferences[i] = validRef;
                }
            }

            PMOS_BUFFER validMvBuf = m_hevcBasicFeature->m_mvBuffers.GetValidBufferForReference(m_hevcBasicFeature->m_refFrameIndexList);
            for (uint32_t i = 0; i < CODEC_NUM_HEVC_MV_BUFFERS; i++)
            {
                if (params.presColMvTempBuffer[i] == nullptr)
                {
                    params.presColMvTempBuffer[i] = &validMvBuf->OsResource;
                }
            }
        }

        return MOS_STATUS_SUCCESS;
    }

#if USE_CODECHAL_DEBUG_TOOL
    MOS_STATUS HevcDecodePicPkt::DumpResources(
        HCP_PIPE_BUF_ADDR_STATE_PAR &pipeBufAddrParams,
        uint8_t                      activeRefListSize,
        uint32_t                     mvBufferSize) const
    {
        DECODE_FUNC_CALL();

        CodechalDebugInterface *debugInterface = m_pipeline->GetDebugInterface();
        DECODE_CHK_NULL(debugInterface);

        for (uint32_t i = 0; i < activeRefListSize; i++)
        {
            if (pipeBufAddrParams.presReferences[i] != nullptr)
            {
                MOS_SURFACE refSurface;
                MOS_ZeroMemory(&refSurface, sizeof(MOS_SURFACE));
                refSurface.OsResource = *(pipeBufAddrParams.presReferences[i]);
                DECODE_CHK_STATUS(CodecUtilities::CodecHalGetResourceInfo(m_osInterface, &refSurface));

                std::string refSurfDumpName = "RefSurf[" + std::to_string(i) + "]";
                DECODE_CHK_STATUS(debugInterface->DumpYUVSurface(
                    &refSurface,
                    CodechalDbgAttr::attrDecodeReferenceSurfaces,
                    refSurfDumpName.c_str()));
            }

            if (pipeBufAddrParams.presColMvTempBuffer[i] != nullptr)
            {
                std::string mvBufDumpName = "_DEC_" + std::to_string(i);
                DECODE_CHK_STATUS(debugInterface->DumpBuffer(
                    pipeBufAddrParams.presColMvTempBuffer[i],
                    CodechalDbgAttr::attrMvData,
                    mvBufDumpName.c_str(),
                    mvBufferSize));
            }
        }

        return MOS_STATUS_SUCCESS;
    }
#endif

    MHW_SETPAR_DECL_SRC(HCP_PIPE_BUF_ADDR_STATE, HevcDecodePicPkt)
    {
        DECODE_FUNC_CALL();

        params.Mode = m_hevcBasicFeature->m_mode;

        PMOS_SURFACE destSurface   = &(m_hevcBasicFeature->m_destSurface);
        params.psPreDeblockSurface = destSurface;

#ifdef _MMC_SUPPORTED
        DECODE_CHK_STATUS(m_mmcState->GetSurfaceMmcState(destSurface, &params.PreDeblockSurfMmcState));
#endif

        params.presMfdDeblockingFilterRowStoreScratchBuffer    = &(m_resMfdDeblockingFilterRowStoreScratchBuffer->OsResource);
        params.presDeblockingFilterTileRowStoreScratchBuffer   = &(m_resDeblockingFilterTileRowStoreScratchBuffer->OsResource);
        params.presDeblockingFilterColumnRowStoreScratchBuffer = &(m_resDeblockingFilterColumnRowStoreScratchBuffer->OsResource);

        params.presMetadataLineBuffer       = &(m_resMetadataLineBuffer->OsResource);
        params.presMetadataTileLineBuffer   = &(m_resMetadataTileLineBuffer->OsResource);
        params.presMetadataTileColumnBuffer = &(m_resMetadataTileColumnBuffer->OsResource);
        params.presSaoLineBuffer            = &(m_resSaoLineBuffer->OsResource);
        params.presSaoTileLineBuffer        = &(m_resSaoTileLineBuffer->OsResource);
        params.presSaoTileColumnBuffer      = &(m_resSaoTileColumnBuffer->OsResource);

        auto        mvBuffers   = &(m_hevcBasicFeature->m_mvBuffers);
        PMOS_BUFFER curMvBuffer = mvBuffers->GetCurBuffer();
        DECODE_CHK_NULL(curMvBuffer);
        params.presCurMvTempBuffer = &(curMvBuffer->OsResource);

        HevcReferenceFrames        &refFrames     = m_hevcBasicFeature->m_refFrames;
        const std::vector<uint8_t> &activeRefList = refFrames.GetActiveReferenceList(*m_hevcPicParams);
        if (!refFrames.m_curIsIntra)
        {
            DECODE_ASSERT(activeRefList.size() <= 8);
            for (uint8_t i = 0; i < activeRefList.size(); i++)
            {
                uint8_t frameIdx = activeRefList[i];
                if (frameIdx >= CODECHAL_NUM_UNCOMPRESSED_SURFACE_HEVC)
                {
                    continue;
                }

                params.presReferences[i] = refFrames.GetReferenceByFrameIndex(frameIdx);
                if (params.presReferences[i] == nullptr)
                {
                    PCODEC_REF_LIST curFrameInRefList = refFrames.m_refList[m_hevcPicParams->CurrPic.FrameIdx];
                    DECODE_CHK_NULL(curFrameInRefList);
                    MOS_ZeroMemory(&curFrameInRefList->resRefPic, sizeof(MOS_RESOURCE));
                    DECODE_ASSERTMESSAGE("Reference frame for current Frame is not exist, current frame will be skipped. Thus, clear current frame resource in reference list.");
                    return MOS_STATUS_INVALID_PARAMETER;
                }
                PMOS_BUFFER mvBuf             = mvBuffers->GetBufferByFrameIndex(frameIdx);
                params.presColMvTempBuffer[i] = mvBuf ? (&mvBuf->OsResource) : nullptr;

                // Return error if reference surface's pitch * height is less than dest surface.
                MOS_SURFACE refSurface;
                refSurface.OsResource = *(params.presReferences[i]);
                DECODE_CHK_STATUS(m_allocator->GetSurfaceInfo(&refSurface));
                DECODE_CHK_COND((refSurface.dwPitch * refSurface.dwHeight) < (destSurface->dwPitch * destSurface->dwHeight),
                    "Reference surface's pitch * height is less than Dest surface.");
            }
        }

        FixHcpPipeBufAddrParams(params);

        if (m_hevcBasicFeature->m_isSCCIBCMode)
        {
            uint8_t IBCRefIdx = refFrames.m_IBCRefIdx;
            DECODE_CHK_COND(activeRefList.size() <= IBCRefIdx, "Invalid IBC reference index.");

            uint8_t refIdxMask = 0;
            for (uint8_t i = 0; i < CODECHAL_MAX_CUR_NUM_REF_FRAME_HEVC; i++)
            {
                uint8_t IBCFrameIdx = activeRefList[IBCRefIdx];
                if (params.presReferences[i] == refFrames.GetReferenceByFrameIndex(IBCFrameIdx))
                {
                    refIdxMask |= (1 << i);
                }
            }
            params.IBCRefIdxMask = refIdxMask;
        }

        CODECHAL_DEBUG_TOOL(DECODE_CHK_STATUS(DumpResources(params, activeRefList.size(), curMvBuffer->size)));        

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(HCP_IND_OBJ_BASE_ADDR_STATE, HevcDecodePicPkt)
    {
        DECODE_FUNC_CALL();

        params.bDecodeInUse   = true;
        params.dwDataSize     = m_hevcBasicFeature->m_dataSize;
        params.dwDataOffset   = m_hevcBasicFeature->m_dataOffset;
        params.presDataBuffer = &(m_hevcBasicFeature->m_resDataBuffer.OsResource);

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HevcDecodePicPkt::AddAllCmds_HCP_QM_STATE(MOS_COMMAND_BUFFER &cmdBuffer)
    {
        DECODE_FUNC_CALL();

        auto &params       = m_hcpItf->MHW_GETPAR_F(HCP_QM_STATE)();
        params             = {};
        auto pHevcIqMatrix = (PMHW_VDBOX_HEVC_QM_PARAMS)m_hevcIqMatrixParams;
        MHW_MI_CHK_NULL(pHevcIqMatrix);
        uint8_t *qMatrix = nullptr;
        qMatrix = (uint8_t *)params.quantizermatrix;

        for (uint8_t sizeId = 0; sizeId < 4; sizeId++) // 4x4, 8x8, 16x16, 32x32
        {
            for (uint8_t predType = 0; predType < 2; predType++) // Intra, Inter
            {
                for (uint8_t color = 0; color < 3; color++) // Y, Cb, Cr
                {
                    if ((sizeId == 3) && (color != 0))
                        break;

                    params.sizeid = sizeId;
                    params.predictionType = predType;
                    params.colorComponent = color;

                    switch (sizeId)
                    {
                    case SIZEID_4X4:
                    case SIZEID_8X8:
                    default:
                        params.dcCoefficient = 0;
                        break;
                    case SIZEID_16X16:
                        params.dcCoefficient = pHevcIqMatrix->ListDC16x16[3 * predType + color];
                        break;
                    case SIZEID_32X32:
                        params.dcCoefficient = pHevcIqMatrix->ListDC32x32[predType];
                        break;
                    }

                    if (sizeId == SIZEID_4X4)
                    {
                        for (uint8_t i = 0; i < 4; i++)
                        {
                            for (uint8_t ii = 0; ii < 4; ii++)
                            {
                                qMatrix[4 * i + ii] = pHevcIqMatrix->List4x4[3 * predType + color][4 * i + ii];
                            }
                        }
                    }
                    else if (sizeId == SIZEID_8X8)
                    {
                        for (uint8_t i = 0; i < 8; i++)
                        {
                            for (uint8_t ii = 0; ii < 8; ii++)
                            {
                                qMatrix[8 * i + ii] = pHevcIqMatrix->List8x8[3 * predType + color][8 * i + ii];
                            }
                        }
                    }
                    else if (sizeId == SIZEID_16X16)
                    {
                        for (uint8_t i = 0; i < 8; i++)
                        {
                            for (uint8_t ii = 0; ii < 8; ii++)
                            {
                                qMatrix[8 * i + ii] = pHevcIqMatrix->List16x16[3 * predType + color][8 * i + ii];
                            }
                        }
                    }
                    else // 32x32
                    {
                        for (uint8_t i = 0; i < 8; i++)
                        {
                            for (uint8_t ii = 0; ii < 8; ii++)
                            {
                                qMatrix[8 * i + ii] = pHevcIqMatrix->List32x32[predType][8 * i + ii];
                            }
                        }
                    }

                    DECODE_CHK_STATUS(m_hcpItf->MHW_ADDCMD_F(HCP_QM_STATE)(&cmdBuffer));
                }
            }
        }
        
        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(HCP_PIC_STATE, HevcDecodePicPkt)
    {
        DECODE_FUNC_CALL();

        DECODE_CHK_NULL(m_hevcPicParams);

        if (m_hevcRextPicParams && m_hevcRextPicParams->PicRangeExtensionFlags.fields.cabac_bypass_alignment_enabled_flag == 1)
        {
            MHW_ASSERTMESSAGE("HW decoder doesn't support HEVC High Throughput profile so far.");
            MHW_ASSERTMESSAGE("So cabac_bypass_alignment_enabled_flag cannot equal to 1.");
            return MOS_STATUS_INVALID_PARAMETER;
        }

        params.bDecodeInUse             = true;
        params.framewidthinmincbminus1  = m_hevcPicParams->PicWidthInMinCbsY - 1;
        params.frameheightinmincbminus1 = m_hevcPicParams->PicHeightInMinCbsY - 1;
        params.mincusize                = (m_hevcPicParams->log2_min_luma_coding_block_size_minus3) & 0x3;
        params.ctbsizeLcusize           = (m_hevcPicParams->log2_diff_max_min_luma_coding_block_size + m_hevcPicParams->log2_min_luma_coding_block_size_minus3) & 0x3;
        params.maxtusize                = (m_hevcPicParams->log2_diff_max_min_transform_block_size + m_hevcPicParams->log2_min_transform_block_size_minus2) & 0x3;
        params.mintusize                = (m_hevcPicParams->log2_min_transform_block_size_minus2) & 0x3;
        params.maxpcmsize               = (m_hevcPicParams->log2_diff_max_min_pcm_luma_coding_block_size + m_hevcPicParams->log2_min_pcm_luma_coding_block_size_minus3) & 0x3;
        params.minpcmsize               = (m_hevcPicParams->log2_min_pcm_luma_coding_block_size_minus3) & 0x3;

        // As per HW requirement, CurPicIsI and ColPicIsI should be set to either both correct or both zero
        // Since driver doesn't know Collocated_Ref_Idx for SF, and cannot get accurate CurPicIsI for both LF/SF
        // Have to make ColPicIsI = CurPicIsI = 0 for both LF/SF
        params.sampleAdaptiveOffsetEnabled    = m_hevcPicParams->sample_adaptive_offset_enabled_flag;
        params.pcmEnabledFlag                 = m_hevcPicParams->pcm_enabled_flag;
        params.cuQpDeltaEnabledFlag           = m_hevcPicParams->cu_qp_delta_enabled_flag;
        params.diffCuQpDeltaDepth             = m_hevcPicParams->diff_cu_qp_delta_depth;
        params.pcmLoopFilterDisableFlag       = m_hevcPicParams->pcm_loop_filter_disabled_flag;
        params.constrainedIntraPredFlag       = m_hevcPicParams->constrained_intra_pred_flag;
        params.log2ParallelMergeLevelMinus2   = m_hevcPicParams->log2_parallel_merge_level_minus2;
        params.signDataHidingFlag             = m_hevcPicParams->sign_data_hiding_enabled_flag;
        params.loopFilterAcrossTilesEnabled   = m_hevcPicParams->loop_filter_across_tiles_enabled_flag;
        params.entropyCodingSyncEnabled       = m_hevcPicParams->entropy_coding_sync_enabled_flag;
        params.tilesEnabledFlag               = m_hevcPicParams->tiles_enabled_flag;
        params.weightedPredFlag               = m_hevcPicParams->weighted_pred_flag;
        params.weightedBipredFlag             = m_hevcPicParams->weighted_bipred_flag;
        params.fieldpic                       = (m_hevcPicParams->RefFieldPicFlag >> 15) & 0x01;
        params.bottomfield                    = ((m_hevcPicParams->RefBottomFieldFlag >> 15) & 0x01) ? 0 : 1;
        params.transformSkipEnabled           = m_hevcPicParams->transform_skip_enabled_flag;
        params.ampEnabledFlag                 = m_hevcPicParams->amp_enabled_flag;
        params.transquantBypassEnableFlag     = m_hevcPicParams->transquant_bypass_enabled_flag;
        params.strongIntraSmoothingEnableFlag = m_hevcPicParams->strong_intra_smoothing_enabled_flag;

        params.picCbQpOffset                   = m_hevcPicParams->pps_cb_qp_offset & 0x1f;
        params.picCrQpOffset                   = m_hevcPicParams->pps_cr_qp_offset & 0x1f;
        params.maxTransformHierarchyDepthIntra = m_hevcPicParams->max_transform_hierarchy_depth_intra & 0x7;
        params.maxTransformHierarchyDepthInter = m_hevcPicParams->max_transform_hierarchy_depth_inter & 0x7;
        params.pcmSampleBitDepthChromaMinus1   = m_hevcPicParams->pcm_sample_bit_depth_chroma_minus1;
        params.pcmSampleBitDepthLumaMinus1     = m_hevcPicParams->pcm_sample_bit_depth_luma_minus1;

        // RExt fields
        params.chromaSubsampling        = m_hevcPicParams->chroma_format_idc;
        params.log2Maxtransformskipsize = m_hevcRextPicParams ? (m_hevcRextPicParams->log2_max_transform_skip_block_size_minus2 + 2) : 0x2;

        params.bitDepthChromaMinus8 = m_hevcPicParams->bit_depth_chroma_minus8;
        params.bitDepthLumaMinus8   = m_hevcPicParams->bit_depth_luma_minus8;

        params.requestCRC = m_hevcPicParams->RequestCRC;

        // Force to false due to definition in xe_lpm_plus
        params.sseEnable                    = false;
        params.rhodomainRateControlEnable   = false;
        params.fractionalQpAdjustmentEnable = false;

        if (m_hevcSccPicParams)
        {
            params.deblockingFilterOverrideEnabled    = m_hevcPicParams->deblocking_filter_override_enabled_flag;
            params.ppsDeblockingFilterDisabled        = m_hevcPicParams->pps_deblocking_filter_disabled_flag;
            params.chromaBitDepthEntryMinus8          = m_hevcPicParams->bit_depth_chroma_minus8;
            params.lumaBitDepthEntryMinus8            = m_hevcPicParams->bit_depth_luma_minus8;
            params.ppsCurrPicRefEnabledFlag           = m_hevcSccPicParams->PicSCCExtensionFlags.fields.pps_curr_pic_ref_enabled_flag;
            params.motionVectorResolutionControlIdc   = m_hevcSccPicParams->PicSCCExtensionFlags.fields.motion_vector_resolution_control_idc;
            params.intraBoundaryFilteringDisabledFlag = m_hevcSccPicParams->PicSCCExtensionFlags.fields.intra_boundary_filtering_disabled_flag;
            params.paletteMaxSize                     = m_hevcSccPicParams->palette_max_size;
            params.deltaPaletteMaxPredictorSize       = m_hevcSccPicParams->delta_palette_max_predictor_size;
            params.ibcConfiguration                   = m_hevcSccPicParams->PicSCCExtensionFlags.fields.pps_curr_pic_ref_enabled_flag ? 2 : 0;
            params.paletteModeEnabledFlag             = m_hevcSccPicParams->PicSCCExtensionFlags.fields.palette_mode_enabled_flag;
        }
        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(HCP_TILE_STATE, HevcDecodePicPkt)
    {
        DECODE_FUNC_CALL();

        params.pTileColWidth  = (uint16_t *)m_hevcBasicFeature->m_tileCoding.GetTileColWidth();
        params.pTileRowHeight = (uint16_t *)m_hevcBasicFeature->m_tileCoding.GetTileRowHeight();
        DECODE_CHK_NULL(params.pTileColWidth);
        DECODE_CHK_NULL(params.pTileRowHeight);
        params.numTileColumnsMinus1 = m_hevcPicParams->num_tile_columns_minus1;
        params.numTileRowsMinus1    = m_hevcPicParams->num_tile_rows_minus1;
        MHW_CHK_COND(m_hevcPicParams->num_tile_rows_minus1 >= HEVC_NUM_MAX_TILE_ROW, "num_tile_rows_minus1 is out of range!");
        MHW_CHK_COND(m_hevcPicParams->num_tile_columns_minus1 >= HEVC_NUM_MAX_TILE_COLUMN, "num_tile_columns_minus1 is out of range!");
        
        return MOS_STATUS_SUCCESS;
    }
}

