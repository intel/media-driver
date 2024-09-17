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
//! \file     encode_av1_scc.cpp
//! \brief    SCC feature
//!

#include "encode_av1_scc.h"
#include "encode_av1_vdenc_const_settings.h"
#include "encode_av1_tile.h"
#include "encode_av1_segmentation.h"

namespace encode
{

    Av1Scc::Av1Scc(EncodeAllocator *allocator,
        CodechalHwInterfaceNext *hwInterface,
        void *constSettings,
        MediaFeatureManager *featureManager)
        : MediaFeature(constSettings, hwInterface ? hwInterface->GetOsInterface() : nullptr)
    {
        ENCODE_FUNC_CALL();

        ENCODE_CHK_NULL_NO_STATUS_RETURN(featureManager);
        m_featureManager = featureManager;

        m_basicFeature = dynamic_cast<Av1BasicFeature *>(featureManager->GetFeature(Av1FeatureIDs::basicFeature));
        ENCODE_CHK_NULL_NO_STATUS_RETURN(m_basicFeature);

        m_segmentation = dynamic_cast<Av1Segmentation *>(featureManager->GetFeature(Av1FeatureIDs::av1Segmentation));
        ENCODE_CHK_NULL_NO_STATUS_RETURN(m_segmentation);

        m_allocator = allocator;

        if (hwInterface)
        {
            m_osInterface = hwInterface->GetOsInterface();
        }
    }

    Av1Scc::~Av1Scc()
    {
        ENCODE_FUNC_CALL();
    }

    inline MOS_STATUS CheckIBCParams(const CODEC_AV1_ENCODE_PICTURE_PARAMS &picParams)
    {
        if (picParams.PicFlags.fields.allow_intrabc)
        {
            const auto &lr = picParams.LoopRestorationFlags.fields;
            if (!AV1_KEY_OR_INRA_FRAME(picParams.PicFlags.fields.frame_type)
                || picParams.filter_level[0] || picParams.filter_level[1]
                || picParams.cdef_bits
                || picParams.PicFlags.fields.use_superres
                || lr.yframe_restoration_type || lr.cbframe_restoration_type || lr.crframe_restoration_type)
            {
                ENCODE_ASSERTMESSAGE("All loop filters MUST be disabled when IBC is enabled");
                return MOS_STATUS_INVALID_PARAMETER;
            }
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1Scc::MakeCdfTrackedBufferLockable()
    {
        MOS_ALLOC_GFXRES_PARAMS allocParams;
        MOS_ZeroMemory(&allocParams, sizeof(MOS_ALLOC_GFXRES_PARAMS));
        allocParams.Type               = MOS_GFXRES_BUFFER;
        allocParams.TileType           = MOS_TILE_LINEAR;
        allocParams.Format             = Format_Buffer;
        allocParams.Flags.bNotLockable = 0;

        allocParams.dwBytes  = MOS_ALIGN_CEIL(m_basicFeature->m_cdfMaxNumBytes, CODECHAL_PAGE_SIZE);
        allocParams.pBufName = "bwdAdaptCdfBuffer";
        allocParams.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_CACHE;
        ENCODE_CHK_STATUS_RETURN(m_basicFeature->m_trackedBuf->RegisterParam(encode::BufferType::bwdAdaptCdfBuffer, allocParams));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1Scc::ResetMvProbsToDefault()
    {
        PMOS_RESOURCE cdfTrackedBuf = m_basicFeature->m_trackedBuf->GetBuffer(BufferType::bwdAdaptCdfBuffer, 0);
        ENCODE_CHK_NULL_RETURN(cdfTrackedBuf);

        auto data = (uint16_t *)m_allocator->LockResourceForWrite(cdfTrackedBuf);
        ENCODE_CHK_NULL_RETURN(data);
        ENCODE_CHK_STATUS_RETURN(InitDefaultFrameContextBuffer(data, 0, mvJointType, interintra));
        ENCODE_CHK_STATUS_RETURN(m_allocator->UnLock(cdfTrackedBuf));

        m_resetMvProbs            = false;

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1Scc::Update(void *params)
    {
        ENCODE_FUNC_CALL();

        EncoderParams *encodeParams = (EncoderParams *)params;
        ENCODE_CHK_NULL_RETURN(encodeParams);

        auto av1PicParams = static_cast<PCODEC_AV1_ENCODE_PICTURE_PARAMS>(encodeParams->pPicParams);
        ENCODE_CHK_NULL_RETURN(av1PicParams);

        m_enablePalette = av1PicParams->PicFlags.fields.PaletteModeEnable;

        ENCODE_CHK_STATUS_RETURN(CheckIBCParams(*av1PicParams));

        m_enableIBC = av1PicParams->PicFlags.fields.allow_intrabc;
        m_IBCPossible = m_IBCPossible || m_enableIBC;

        if (m_resetMvProbs)
        {
            ResetMvProbsToDefault();
        }

        if (m_IBCPossible && m_basicFeature->m_resolutionChanged)
        {
            // Defines 2 sets of MV probabilities: set 0 for regular Inter blocks, set 1 for IBC blocks
            // HW CDF table contains single set of MV probabilities instead of 2
            // when IBC is enabled PAK will update MV probabilities during encoding of KEY frame
            // Inter frame which follows KEY frame requires default (not updated) MV probabilities
            // CPU-lockable CDF table tracked buffer is needed to reset MV probs back to defaults
            // tracked buffer is forced to be CPU-lockable
            ENCODE_CHK_STATUS_RETURN(MakeCdfTrackedBufferLockable());
        }

        if (m_enableIBC)
        {
            // need to reset MV probs to default values after each KEY frame with IBC enabled
            m_resetMvProbs = true;
            if (m_intrabcReconSurface == nullptr)
            {
                MOS_ALLOC_GFXRES_PARAMS allocParams;
                MOS_ZeroMemory(&allocParams, sizeof(MOS_ALLOC_GFXRES_PARAMS));
                allocParams.Type            = MOS_GFXRES_2D;
                allocParams.TileType        = MOS_TILE_Y;
                allocParams.Format          = (10 == m_basicFeature->m_bitDepth) ? Format_P010 : Format_NV12;
                allocParams.dwWidth         = MOS_ALIGN_CEIL(m_basicFeature->m_frameWidth, av1SuperBlockWidth);
                allocParams.dwHeight        = MOS_ALIGN_CEIL(m_basicFeature->m_frameHeight, av1SuperBlockHeight);
                allocParams.pBufName        = "Intra Block Copy output buffer";
                allocParams.bIsCompressible = false;
                allocParams.CompressionMode = MOS_MMC_DISABLED;

                if (m_basicFeature->m_outputChromaFormat == AVP_CHROMA_FORMAT_YUV444)
                {
                    if (!m_basicFeature->m_is10Bit)
                    {
                        allocParams.Format   = Format_AYUV;
                        allocParams.dwWidth  = MOS_ALIGN_CEIL(allocParams.dwWidth, 512 / 4);
                        allocParams.dwHeight = MOS_ALIGN_CEIL(allocParams.dwHeight * 3 / 4, 8);
                    }
                    else
                    {
                        allocParams.Format    = Format_Y410;
                        //Y410Viriant surface is same as Y416Viriant
                        allocParams.dwWidth   = MOS_ALIGN_CEIL(allocParams.dwWidth, 256 / 4);
                        allocParams.dwHeight  = MOS_ALIGN_CEIL(allocParams.dwHeight * 3 / 2, 8);
                    }
                }
                else if (m_basicFeature->m_outputChromaFormat == AVP_CHROMA_FORMAT_YUV422)
                {
                    if (!m_basicFeature->m_is10Bit)
                    {
                        // allocated as YUY2, but used by HW as YUY2 Variant, which is
                        // twice the height and half the width of the YUY2 source surface
                        allocParams.Format   = Format_YUY2;
                        allocParams.dwWidth  = MOS_ALIGN_CEIL(allocParams.dwWidth >> 1, 128);
                        allocParams.dwHeight = MOS_ALIGN_CEIL(allocParams.dwHeight << 1, 8);
                    }
                    else
                    {
                        // allocated as Y210, but used by HW as Y210 Variant, which is
                        // twice the height and half the width of the Y210 source surface
                        allocParams.Format   = Format_Y210;
                        allocParams.dwWidth  = MOS_ALIGN_CEIL(allocParams.dwWidth >> 1, 128 >> 1);  // HW requires 128 byte aligned width, 2 bytes per pixel
                        allocParams.dwHeight = MOS_ALIGN_CEIL(allocParams.dwHeight << 1, 8);
                    }
                }

                m_intrabcReconSurface = m_allocator->AllocateSurface(allocParams, false, MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_CACHE);
                m_allocator->GetSurfaceInfo(m_intrabcReconSurface);
            }
        }

        ENCODE_CHK_STATUS_RETURN(UpdateIBCStatusForCurrentTile());

        return MOS_STATUS_SUCCESS;
    }

    const uint32_t INTRABC_DELAY_SB64 = 4;

    MOS_STATUS Av1Scc::UpdateIBCStatusForCurrentTile()
    {
        uint32_t wSB64 = MOS_ROUNDUP_DIVIDE(m_basicFeature->m_frameWidth, 64);

        auto tileFeature = dynamic_cast<Av1EncodeTile *>(m_featureManager->GetFeature(Av1FeatureIDs::encodeTile));

        if (tileFeature)
        {
            Av1TileInfo tileInfo = {};
            tileFeature->GetTileInfo(&tileInfo);

            wSB64 = tileInfo.tileWidthInSbMinus1 + 1;
        }

        m_IBCEnabledForCurrentTile = m_enableIBC && (wSB64 > (INTRABC_DELAY_SB64 + 1));
        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(VDENC_HEVC_VP9_TILE_SLICE_STATE, Av1Scc)
    {
        bool m_is10Bit = m_basicFeature->m_is10Bit;
        uint16_t qp    = (m_basicFeature->m_av1PicParams->base_qindex) / 4;

        uint8_t tableIdx = 0;
        if (qp <= 12)
            tableIdx = 0;
        else if (qp > 12 && qp <= 17)
            tableIdx = 1;
        else if (qp > 17 && qp <= 22)
            tableIdx = 2;
        else if (qp > 22 && qp <= 27)
            tableIdx = 3;
        else if (qp > 27 && qp <= 32)
            tableIdx = 4;
        else if (qp > 32 && qp <= 37)
            tableIdx = 5;
        else if (qp > 37 && qp <= 42)
            tableIdx = 6;
        else if (qp > 42 && qp <= 47)
            tableIdx = 7;
        else if (qp > 47 && qp <= 49)
            tableIdx = 8;
        else
            tableIdx = 9;

        if (m_enablePalette)
        {
            params.VdencHEVCVP9TileSlicePar8   = AV1table[tableIdx][0];
            params.VdencHEVCVP9TileSlicePar9   = AV1table[tableIdx][1];
            params.VdencHEVCVP9TileSlicePar5   = AV1table[tableIdx][2];
            params.VdencHEVCVP9TileSlicePar22  = 64;
            params.paletteModeEnable           = 1;
   
            if (m_is10Bit)
            {
                params.VdencHEVCVP9TileSlicePar5 = AV1table[tableIdx][2] + 2;
            }
        }

        if (m_IBCEnabledForCurrentTile)
        {
            params.ibcControl                   = IBC_ENABLED;
            params.VdencHEVCVP9TileSlicePar0    = 0;
        }

        auto numTiles = m_basicFeature->m_av1PicParams->tile_rows * m_basicFeature->m_av1PicParams->tile_cols;
        if (numTiles > 1)
        {
            auto waTable = m_osInterface->pfnGetWaTable(m_osInterface);
            if (MEDIA_IS_WA(waTable, Wa_15014143531))
            {
                params.ibcControl = IBC_DISABLED;
            }
        }

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(VDENC_CMD2, Av1Scc)
    {
        if (m_IBCEnabledForCurrentTile)
        {
            params.frameIdxL0Ref0 = 0;
        }

#if _MEDIA_RESERVED
        if (m_IBCEnabledForCurrentTile)
        {
            params.vdencCmd2Par3            = vdencCmd2Par3Value2;
            params.vdencCmd2Par136          = vdencCmd2Par136Value1;
            params.vdencCmd2Par102          = false;       
            params.vdencCmd2Par101          = false; 

            if (params.vdencCmd2Par135[1] == vdencCmd2Par135Value2)
            {
                params.vdencCmd2Par135[1] = vdencCmd2Par135Value1;
            }
        }

        if (m_segmentation->HasZeroSegmentQIndex())
        {
            params.vdencCmd2Par133 = false;
        }

        if (m_enablePalette && (m_basicFeature->m_targetUsage == 7))
        {
            params.vdencCmd2Par134[1] = 1;
        }
#else
        params.extSettings.emplace_back(
            [this](uint32_t *data) {
                if (m_IBCEnabledForCurrentTile)
                {
                    data[2]   = data[2]  & 0xFFFFFFFC | 0x2;
                    data[37]  = data[37] & 0xFFFFFF9F | 0x40;
                    data[54] &= 0xFFFFFF3F;

                    if (((data[51] >> 10) & 0b11) == vdencCmd2Par135Value1)
                    {
                        data[51] = data[51] & 0xFFFFF3FF | 0x400;
                    }
                }

                if (m_segmentation->HasZeroSegmentQIndex())
                {
                    data[2] &= 0xFFFFFFBF;
                }

                if (m_enablePalette && (m_basicFeature->m_targetUsage == 7))
                {
                    data[51] = data[51] & 0xFFFFFF3F | 0x40;
                }
                
                return MOS_STATUS_SUCCESS;
            });
#endif  // _MEDIA_RESERVED

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(VDENC_CMD1, Av1Scc)
    {
        auto setting = static_cast<Av1VdencFeatureSettings*>(m_constSettings);
        ENCODE_CHK_NULL_RETURN(setting);

        if (m_IBCEnabledForCurrentTile)
        {

            std::fill_n(&params.vdencCmd1Par15[0], 4, (uint8_t)158);
            std::fill_n(&params.vdencCmd1Par14[0], 4, (uint8_t)146);
            std::fill_n(&params.vdencCmd1Par13[0], 4, (uint8_t)146);
            std::fill_n(&params.vdencCmd1Par12[0], 4, (uint8_t)143);
            std::fill_n(&params.vdencCmd1Par11[0], 4, (uint8_t)140);
            std::fill_n(&params.vdencCmd1Par10[0], 4, (uint8_t)134);
            std::fill_n(&params.vdencCmd1Par9[0],  4, (uint8_t)134);
            std::fill_n(&params.vdencCmd1Par8[0],  4, (uint8_t)131);

            params.vdencCmd1Par5               = 8;
            params.vdencCmd1Par6               = 4;
            params.vdencCmd1Par7               = 12;

            params.vdencCmd1Par16              = 229;
            params.vdencCmd1Par17              = 146;
            params.vdencCmd1Par18              = 230;
            params.vdencCmd1Par19              = 144;
            params.vdencCmd1Par20              = 145;

            params.vdencCmd1Par90              = 0;
            params.vdencCmd1Par91              = 0;
            params.vdencCmd1Par92              = 0;
            params.vdencCmd1Par94              = 24;
        }

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(VDENC_PIPE_BUF_ADDR_STATE, Av1Scc)
    {
        if (m_enableIBC)
        {
            params.refs[0]         = &m_intrabcReconSurface->OsResource;
            params.refsDsStage2[0] = nullptr;
            params.refsDsStage1[0] = nullptr;

            params.numActiveRefL0 = 1;
        }

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(AVP_PIC_STATE, Av1Scc)
    {
        if (m_enableIBC || m_enablePalette)
        {
            params.allowScreenContentTools = true;
            params.allowIntraBC = m_enableIBC;

            if (m_enableIBC)
            {
                params.enableCDEF        = false;
                params.enableRestoration = false;
            }
        }

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(AVP_PIPE_BUF_ADDR_STATE, Av1Scc)
    {
        if (m_enableIBC)
        {
            params.intrabcDecodedOutputFrameBuffer = &m_intrabcReconSurface->OsResource;
        }

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(AVP_SURFACE_STATE, Av1Scc)
    {
        MOS_MEMCOMP_STATE mmcState = {};
        if (m_enableIBC && params.surfaceStateId == intrabcDecodedFrame)
        {
            if (m_basicFeature->m_outputChromaFormat == AVP_CHROMA_FORMAT_YUV420)
            {
                params.pitch   = m_basicFeature->m_reconSurface.dwPitch;
                params.uOffset = m_basicFeature->m_reconSurface.YoffsetForUplane;
                params.vOffset = m_basicFeature->m_reconSurface.YoffsetForVplane;
            }
            else if (m_basicFeature->m_outputChromaFormat == AVP_CHROMA_FORMAT_YUV444)
            {
                if (!m_basicFeature->m_is10Bit)
                {
                    params.pitch   = m_basicFeature->m_reconSurface.dwPitch / 4;
                    params.uOffset = (uint16_t)m_basicFeature->m_rawSurface.dwHeight;
                    params.vOffset = (uint16_t)m_basicFeature->m_rawSurface.dwHeight << 1;
                }
                else
                {
                    params.pitch   = m_basicFeature->m_reconSurface.dwPitch / 2;
                    params.uOffset = (uint16_t)m_basicFeature->m_rawSurface.dwHeight;
                    params.vOffset = (uint16_t)m_basicFeature->m_rawSurface.dwHeight << 1;
                }
            }
            else if (m_basicFeature->m_outputChromaFormat == AVP_CHROMA_FORMAT_YUV422)
            {
                params.pitch   = m_basicFeature->m_reconSurface.dwPitch;
                params.uOffset = params.vOffset = m_basicFeature->m_rawSurface.dwHeight;
            }

            m_basicFeature->GetSurfaceMmcInfo(m_intrabcReconSurface, mmcState, params.compressionFormat);
            std::fill(std::begin(params.mmcState), std::end(params.mmcState), mmcState);
        }
        return MOS_STATUS_SUCCESS;
    }

}  // namespace encode
