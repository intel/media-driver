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
//! \file     decode_av1_tile_packet.cpp
//! \brief    Defines the interface for av1 decode tile packet
//!
#include "decode_av1_tile_packet.h"

namespace decode
{
    MOS_STATUS Av1DecodeTilePkt::Init()
    {
        DECODE_FUNC_CALL();

        DECODE_CHK_NULL(m_featureManager);
        DECODE_CHK_NULL(m_hwInterface);
        DECODE_CHK_NULL(m_osInterface);
        DECODE_CHK_NULL(m_miItf);
        DECODE_CHK_NULL(m_av1Pipeline);

        m_av1BasicFeature = dynamic_cast<Av1BasicFeature*>(m_featureManager->GetFeature(FeatureIDs::basicFeature));
        DECODE_CHK_NULL(m_av1BasicFeature);

        m_allocator = m_pipeline ->GetDecodeAllocator();
        DECODE_CHK_NULL(m_allocator);

        DECODE_CHK_STATUS(CalculateTileStateCommandSize());

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1DecodeTilePkt::Prepare()
    {
        DECODE_FUNC_CALL();

        DECODE_CHK_NULL(m_av1BasicFeature->m_av1PicParams);
        DECODE_CHK_NULL(m_av1BasicFeature->m_av1TileParams);

        m_av1PicParams = m_av1BasicFeature->m_av1PicParams;
        m_av1TileParams = m_av1BasicFeature->m_av1TileParams;

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1DecodeTilePkt::CalculateCommandSize(uint32_t &commandBufferSize,
                                                      uint32_t &requestedPatchListSize)
    {
        DECODE_FUNC_CALL();

        commandBufferSize      = m_tileStatesSize;
        requestedPatchListSize = m_tilePatchListSize;

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1DecodeTilePkt::CalculateTileStateCommandSize()
    {
        DECODE_FUNC_CALL();

        // Tile Level Commands
        DECODE_CHK_STATUS(m_hwInterface->GetAvpPrimitiveCommandSize(
                            m_av1BasicFeature->m_mode,
                            &m_tileStatesSize,
                            &m_tilePatchListSize));

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(AVP_INLOOP_FILTER_STATE, Av1DecodeTilePkt)
    {
        params.loopFilterLevel[0]     = m_av1PicParams->m_filterLevel[0];
        params.loopFilterLevel[1]     = m_av1PicParams->m_filterLevel[1];
        params.loopFilterLevel[2]     = m_av1PicParams->m_filterLevelU;
        params.loopFilterLevel[3]     = m_av1PicParams->m_filterLevelV;
        params.loopFilterSharpness    = m_av1PicParams->m_loopFilterInfoFlags.m_fields.m_sharpnessLevel;
        params.loopFilterDeltaEnabled = m_av1PicParams->m_loopFilterInfoFlags.m_fields.m_modeRefDeltaEnabled;
        params.deltaLfRes             = m_av1PicParams->m_modeControlFlags.m_fields.m_log2DeltaLfRes;
        params.deltaLfMulti           = m_av1PicParams->m_modeControlFlags.m_fields.m_deltaLfMulti;
        params.loopFilterDeltaUpdate  = m_av1PicParams->m_modeControlFlags.m_fields.m_deltaLfPresentFlag;

        for (uint8_t i = 0 ; i < 8 ; i++)
        {
            //ref_deltas[0..7]
            params.loopFilterRefDeltas[i] = m_av1PicParams->m_refDeltas[i];

            //cdef strength
            params.cdefYStrength[i]       = m_av1PicParams->m_cdefYStrengths[i];
            params.cdefUVStrength[i]      = m_av1PicParams->m_cdefUvStrengths[i];
        }

        params.cdefBits                   = m_av1PicParams->m_cdefBits;
        params.cdefDampingMinus3          = m_av1PicParams->m_cdefDampingMinus3;

        //mode_deltas[0..1]
        params.loopFilterModeDeltas[0]    = m_av1PicParams->m_modeDeltas[0];
        params.loopFilterModeDeltas[1]    = m_av1PicParams->m_modeDeltas[1];

        params.LoopRestorationType[0]     = m_av1PicParams->m_loopRestorationFlags.m_fields.m_yframeRestorationType;
        params.LoopRestorationType[1]     = m_av1PicParams->m_loopRestorationFlags.m_fields.m_cbframeRestorationType;
        params.LoopRestorationType[2]     = m_av1PicParams->m_loopRestorationFlags.m_fields.m_crframeRestorationType;

        params.superresUpscaledWidthMinus1 = m_av1PicParams->m_superResUpscaledWidthMinus1;
        params.superresDenom               = m_av1PicParams->m_picInfoFlags.m_fields.m_useSuperres ?
                                              m_av1PicParams->m_superresScaleDenominator : 8;

        //LRU size for Y
        if (m_av1PicParams->m_loopRestorationFlags.m_fields.m_yframeRestorationType  == 0 &&
            m_av1PicParams->m_loopRestorationFlags.m_fields.m_cbframeRestorationType == 0 &&
            m_av1PicParams->m_loopRestorationFlags.m_fields.m_crframeRestorationType == 0)
        {
            params.LoopRestorationSizeLuma = 0;
        }
        else
        {
            params.LoopRestorationSizeLuma = m_av1PicParams->m_loopRestorationFlags.m_fields.m_lrUnitShift + 1;
        }

        //LRU size for UV
        if (m_av1PicParams->m_loopRestorationFlags.m_fields.m_yframeRestorationType == 0  &&
            m_av1PicParams->m_loopRestorationFlags.m_fields.m_cbframeRestorationType == 0 &&
            m_av1PicParams->m_loopRestorationFlags.m_fields.m_crframeRestorationType == 0)
        {
            params.UseSameLoopRestorationSizeForChroma = 0;
        }
        else
        {
            params.UseSameLoopRestorationSizeForChroma = (m_av1PicParams->m_loopRestorationFlags.m_fields.m_lrUvShift == 0) ? 1 : 0;
        }

        if (m_av1PicParams->m_picInfoFlags.m_fields.m_useSuperres)
        {
            //setup super-res step/offset for luma/chroma, per av1_upscale_normative_rows()
            if (m_av1BasicFeature->m_tileCoding.m_curTile == 0)
            {
                m_av1BasicFeature->m_tileCoding.GetUpscaleConvolveStepX0(*m_av1PicParams, false); // Luma
                m_av1BasicFeature->m_tileCoding.GetUpscaleConvolveStepX0(*m_av1PicParams, true);  // Chroma
            }

            uint16_t col = m_av1BasicFeature->m_tileCoding.m_tileDesc[m_av1BasicFeature->m_tileCoding.m_curTile].m_tileColumn;
            params.lumaPlaneXStepQn     = m_av1BasicFeature->m_tileCoding.m_lumaXStepQn;
            params.lumaPlaneX0Qn        = m_av1BasicFeature->m_tileCoding.m_lumaX0Qn[col];
            params.chromaPlaneXStepQn   = m_av1BasicFeature->m_tileCoding.m_chromaXStepQn;
            params.chromaPlaneX0Qn      = m_av1BasicFeature->m_tileCoding.m_chromaX0Qn[col];
        }

        if (m_av1PicParams->m_picInfoFlags.m_fields.m_largeScaleTile)
        {
            //set to 0 to disable
            params.loopFilterLevel[2] = 0;
            params.loopFilterLevel[3] = 0;

            //ref_deltas[0..7]
            params.loopFilterRefDeltas[0] = 1;
            params.loopFilterRefDeltas[1] = 0;
            params.loopFilterRefDeltas[2] = 0;
            params.loopFilterRefDeltas[3] = 0;
            params.loopFilterRefDeltas[4] = 0;
            params.loopFilterRefDeltas[5] = -1;
            params.loopFilterRefDeltas[6] = -1;
            params.loopFilterRefDeltas[7] = -1;
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1DecodeTilePkt::AddCmd_AVP_TILE_CODING(MOS_COMMAND_BUFFER &cmdBuffer, int16_t tileIdx)
    {
        DECODE_FUNC_CALL()

        auto &par = m_avpItf->MHW_GETPAR_F(AVP_TILE_CODING)();
        par = {};

        Av1DecodeTile::TileDesc *m_tileDesc=m_av1BasicFeature->m_tileCoding.m_tileDesc;
        uint16_t curCol = m_tileDesc[tileIdx].m_tileColumn;
        uint16_t curRow = m_tileDesc[tileIdx].m_tileRow;
        uint16_t srcTileId = curCol + curRow * m_av1PicParams->m_tileCols;

        if (m_av1PicParams->m_picInfoFlags.m_fields.m_largeScaleTile)
        {
            DECODE_ASSERT(tileIdx == m_tileDesc[tileIdx].m_tileIndex);
        }

        if (m_av1PicParams->m_picInfoFlags.m_fields.m_largeScaleTile)
        {
            par.tileId                 = srcTileId;
            par.tgTileNum              = srcTileId;
            par.tileGroupId            = 0;
            par.tileColPositionInSb    = m_av1BasicFeature->m_tileCoding.m_tileColStartSb[curCol];
            par.tileRowPositionInSb    = m_av1BasicFeature->m_tileCoding.m_tileRowStartSb[curRow];
            par.tileWidthInSbMinus1    = m_av1PicParams->m_widthInSbsMinus1[curCol];
            par.tileHeightInSbMinus1   = m_av1PicParams->m_heightInSbsMinus1[curRow];
            par.tileRowIndependentFlag = true;
            par.lastTileOfColumn       = (curRow == m_av1PicParams->m_tileRows - 1) ? true : false;
            par.lastTileOfRow          = (curCol == m_av1PicParams->m_tileCols - 1) ? true : false;
            par.firstTileOfTileGroup   = (srcTileId == 0) ? true : false;
            par.lastTileOfTileGroup    = (curCol == m_av1PicParams->m_tileCols - 1) && (curRow == m_av1PicParams->m_tileRows - 1);
            par.lastTileOfFrame        = (curCol == m_av1PicParams->m_tileCols - 1) && (curRow == m_av1PicParams->m_tileRows - 1);
        }
        else
        {
            par.tileId                 = tileIdx;
            par.tgTileNum              = m_tileDesc[tileIdx].m_tileNum;
            par.tileGroupId            = m_tileDesc[tileIdx].m_tileGroupId;
            par.tileColPositionInSb    = m_av1BasicFeature->m_tileCoding.m_tileColStartSb[curCol];
            par.tileRowPositionInSb    = m_av1BasicFeature->m_tileCoding.m_tileRowStartSb[curRow];
            par.tileWidthInSbMinus1    = m_av1PicParams->m_widthInSbsMinus1[curCol];
            par.tileHeightInSbMinus1   = m_av1PicParams->m_heightInSbsMinus1[curRow];
            par.tileRowIndependentFlag = true;
            par.lastTileOfColumn       = (curRow == m_av1PicParams->m_tileRows - 1) ? true : false;
            par.lastTileOfRow          = (curCol == m_av1PicParams->m_tileCols - 1) ? true : false;
            par.firstTileOfTileGroup   = (m_tileDesc[tileIdx].m_tileNum == 0) ? true : false;
            par.lastTileOfTileGroup    = m_tileDesc[tileIdx].m_lastInGroup;
            par.lastTileOfFrame        = (curCol == m_av1PicParams->m_tileCols - 1) && (curRow == m_av1PicParams->m_tileRows - 1);
        }

        par.disableCdfUpdateFlag             = m_av1PicParams->m_picInfoFlags.m_fields.m_disableCdfUpdate;
        par.disableFrameContextUpdateFlag    = m_av1PicParams->m_picInfoFlags.m_fields.m_disableFrameEndUpdateCdf
                                               || (tileIdx != m_av1PicParams->m_contextUpdateTileId);
        par.numOfActiveBePipes               = 1;

        if (m_av1PicParams->m_picInfoFlags.m_fields.m_largeScaleTile)
        {
            par.numOfTileColumnsInFrame = m_av1PicParams->m_outputFrameWidthInTilesMinus1 + 1;
            par.numOfTileRowsInFrame    = m_av1PicParams->m_outputFrameHeightInTilesMinus1 + 1;
            par.outputDecodedTileColPos = (m_tileDesc[tileIdx].m_tileIndex % (m_av1PicParams->m_outputFrameWidthInTilesMinus1 + 1)) *
                                                           (m_av1PicParams->m_widthInSbsMinus1[0] + 1);  //AV1 Conformance: tile width is identical for all tiles
            par.outputDecodedTileRowPos = (m_tileDesc[tileIdx].m_tileIndex / (m_av1PicParams->m_outputFrameWidthInTilesMinus1 + 1)); //tile height is exactly one SB
        }
        else
        {
            par.numOfTileColumnsInFrame = m_av1PicParams->m_tileCols;
            par.numOfTileRowsInFrame = m_av1PicParams->m_tileRows;
        }

        m_av1BasicFeature->m_frameCompletedFlag = par.lastTileOfFrame;

        DECODE_CHK_STATUS(m_avpItf->MHW_ADDCMD_F(AVP_TILE_CODING)(&cmdBuffer));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1DecodeTilePkt::AddCmd_AVP_BSD_OBJECT(MOS_COMMAND_BUFFER &cmdBuffer, int16_t tileIdx)
    {
        DECODE_FUNC_CALL()

        auto &par = m_avpItf->MHW_GETPAR_F(AVP_BSD_OBJECT)();
        par = {};
        Av1DecodeTile::TileDesc *m_tileDesc=m_av1BasicFeature->m_tileCoding.m_tileDesc;

        par.bsdDataLength = m_tileDesc[tileIdx].m_size;
        par.bsdDataStartOffset =m_tileDesc[tileIdx].m_offset;

        DECODE_CHK_STATUS(m_avpItf->MHW_ADDCMD_F(AVP_BSD_OBJECT)(&cmdBuffer));

        return MOS_STATUS_SUCCESS;
    }

    uint32_t Av1DecodeTilePkt::GetFrameUncompressSize()
    {
        uint32_t upScaleWidth = m_av1BasicFeature->m_av1PicParams->m_superResUpscaledWidthMinus1 + 1;
        uint32_t frameHeight = m_av1BasicFeature->m_av1PicParams->m_frameHeightMinus1 + 1;
        uint32_t picSizeProfileFactor = 0;
        switch (m_av1BasicFeature->m_av1PicParams->m_profile)
        {
        case 0:
            picSizeProfileFactor = 15;
            break;
        case 1:
            picSizeProfileFactor = 30;
            break;
        default:
            picSizeProfileFactor = 36;
            break;
        }
        // Calculate uncompressed frame size according to spec,
        // ( UpscaledWidth * FrameHeight * PicSizeProfileFactor ) >> 3
        return ((upScaleWidth * frameHeight * picSizeProfileFactor) >> 3);
    }

    MHW_SETPAR_DECL_SRC(AVP_FILM_GRAIN_STATE, Av1DecodeTilePkt)
    {
        auto filmGrainParams = m_av1PicParams->m_filmGrainParams;

        params.grainRandomSeed       = filmGrainParams.m_randomSeed;
        params.clipToRestrictedRange = filmGrainParams.m_filmGrainInfoFlags.m_fields.m_clipToRestrictedRange;
        params.numOfYPoints          = filmGrainParams.m_numYPoints;
        params.numOfCbPoints         = filmGrainParams.m_numCbPoints;
        params.numOfCrPoints         = filmGrainParams.m_numCrPoints;
        params.matrixCoefficients    = (m_av1PicParams->m_matrixCoefficients == 0) ? 1 : 0;
        params.grainScalingMinus8    = filmGrainParams.m_filmGrainInfoFlags.m_fields.m_grainScalingMinus8;
        params.arCoeffLag            = filmGrainParams.m_filmGrainInfoFlags.m_fields.m_arCoeffLag;
        params.arCoeffShiftMinus6    = filmGrainParams.m_filmGrainInfoFlags.m_fields.m_arCoeffShiftMinus6;
        params.grainScaleShift       = filmGrainParams.m_filmGrainInfoFlags.m_fields.m_grainScaleShift;
        params.chromaScalingFromLuma = filmGrainParams.m_filmGrainInfoFlags.m_fields.m_chromaScalingFromLuma;
        params.grainNoiseOverlap     = filmGrainParams.m_filmGrainInfoFlags.m_fields.m_overlapFlag;

        MOS_SecureMemcpy(params.pointYValue,
            sizeof(filmGrainParams.m_pointYValue),
            filmGrainParams.m_pointYValue,
            sizeof(filmGrainParams.m_pointYValue)
        );

        MOS_SecureMemcpy(params.pointYScaling,
            sizeof(filmGrainParams.m_pointYScaling),
            filmGrainParams.m_pointYScaling,
            sizeof(filmGrainParams.m_pointYScaling)
        );

        MOS_SecureMemcpy(params.pointCbValue,
            sizeof(filmGrainParams.m_pointCbValue),
            filmGrainParams.m_pointCbValue,
            sizeof(filmGrainParams.m_pointCbValue)
        );

        MOS_SecureMemcpy(params.pointCbScaling,
            sizeof(filmGrainParams.m_pointCbScaling),
            filmGrainParams.m_pointCbScaling,
            sizeof(filmGrainParams.m_pointCbScaling)
        );

        MOS_SecureMemcpy(params.pointCrValue,
            sizeof(filmGrainParams.m_pointCrValue),
            filmGrainParams.m_pointCrValue,
            sizeof(filmGrainParams.m_pointCrValue)
        );

        MOS_SecureMemcpy(params.pointCrScaling,
            sizeof(filmGrainParams.m_pointCrScaling),
            filmGrainParams.m_pointCrScaling,
            sizeof(filmGrainParams.m_pointCrScaling)
        );

        uint32_t i;
        for (i = 0; i < sizeof(filmGrainParams.m_arCoeffsY) / sizeof(int8_t); i++)
        {
            params.arCoeffsY[i] = 128 + filmGrainParams.m_arCoeffsY[i];
        }

        for (i = 0; i < sizeof(filmGrainParams.m_arCoeffsCb) / sizeof(int8_t); i++)
        {
            params.arCoeffsCb[i] = 128 + filmGrainParams.m_arCoeffsCb[i];
        }

        for (i = 0; i < sizeof(filmGrainParams.m_arCoeffsCr) / sizeof(int8_t); i++)
        {
            params.arCoeffsCr[i] = 128 + filmGrainParams.m_arCoeffsCr[i];
        }

        params.cbMult               = filmGrainParams.m_cbMult;
        params.cbLumaMult           = filmGrainParams.m_cbLumaMult;
        params.cbOffset             = filmGrainParams.m_cbOffset;
        params.crMult               = filmGrainParams.m_crMult;
        params.crLumaMult           = filmGrainParams.m_crLumaMult;
        params.crOffset             = filmGrainParams.m_crOffset;

        return MOS_STATUS_SUCCESS;
    }


}
