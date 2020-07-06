/*
* Copyright (c) 2019-2020, Intel Corporation
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
//! \file     decode_av1_basic_feature.cpp
//! \brief    Defines the common interface for decode av1 parameter
//!

#include "decode_av1_basic_feature.h"
#include "decode_utils.h"
#include "codechal_utilities.h"
#include "decode_allocator.h"

namespace decode
{
    Av1BasicFeature::~Av1BasicFeature()
    {
        for (uint8_t i = 0; i < av1DefaultCdfTableNum; i++)
        {
            if (!m_allocator->ResourceIsNull(&m_tmpCdfBuffers[i]->OsResource))
            {
                m_allocator->Destroy(m_tmpCdfBuffers[i]);
            }

            if (!m_allocator->ResourceIsNull(&m_defaultCdfBuffers[i]->OsResource))
            {
                m_allocator->Destroy(m_defaultCdfBuffers[i]);
            }

        }
    }

    MOS_STATUS Av1BasicFeature::Init(void *setting)
    {
        DECODE_FUNC_CALL();
        DECODE_CHK_NULL(setting);

        DECODE_CHK_STATUS(DecodeBasicFeature::Init(setting));
        CodechalSetting *codecSettings = (CodechalSetting*)setting;

        if (codecSettings->lumaChromaDepth & CODECHAL_LUMA_CHROMA_DEPTH_8_BITS)
            m_av1DepthIndicator = 0;
        if (codecSettings->lumaChromaDepth & CODECHAL_LUMA_CHROMA_DEPTH_10_BITS)
            m_av1DepthIndicator = 1;
        if (codecSettings->lumaChromaDepth & CODECHAL_LUMA_CHROMA_DEPTH_12_BITS)
            m_av1DepthIndicator = 2;

        DECODE_CHK_STATUS(m_refFrames.Init(this, *m_allocator));
        DECODE_CHK_STATUS(m_tempBuffers.Init(*m_hwInterface, *m_allocator, *this, CODEC_NUM_REF_AV1_TEMP_BUFFERS));
        DECODE_CHK_STATUS(m_tileCoding.Init(this, codecSettings));
        DECODE_CHK_STATUS(m_internalTarget.Init(*m_allocator));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1BasicFeature::Update(void *params)
    {
        DECODE_FUNC_CALL();
        DECODE_CHK_NULL(params);

        DECODE_CHK_STATUS(DecodeBasicFeature::Update(params));

        CodechalDecodeParams* decodeParams = (CodechalDecodeParams*)params;
        m_dataSize = decodeParams->m_dataSize;
        m_av1PicParams  = static_cast<CodecAv1PicParams*>(decodeParams->m_picParams);
        DECODE_CHK_NULL(m_av1PicParams);

        m_av1TileParams = static_cast<CodecAv1TileParams*>(decodeParams->m_sliceParams);
        DECODE_CHK_NULL(m_av1TileParams);

        m_segmentParams = &m_av1PicParams->m_av1SegData;
        DECODE_CHK_NULL(m_segmentParams);

        m_tileCoding.m_numTiles = decodeParams->m_numSlices;

        m_filmGrainEnabled = m_av1PicParams->m_filmGrainParams.m_filmGrainInfoFlags.m_fields.m_applyGrain;

        DECODE_CHK_STATUS(SetPictureStructs(decodeParams));
        DECODE_CHK_STATUS(SetTileStructs());

        return MOS_STATUS_SUCCESS;
    }

    //Currently, m_bsBytesInBuffer of current bitstream buffer is not passed by application.
    MOS_STATUS Av1BasicFeature::SetRequiredBitstreamSize(uint32_t requiredSize)
    {
        DECODE_FUNC_CALL();
        m_dataSize = requiredSize;
        DECODE_NORMALMESSAGE("Estimate bitstream size in this Frame: %u", requiredSize);
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1BasicFeature::SetPictureStructs(CodechalDecodeParams *decodeParams)
    {
        DECODE_FUNC_CALL();

        m_width                         = MOS_MAX(m_width, (uint32_t)m_av1PicParams->m_frameWidthMinus1 + 1);
        m_height                        = MOS_MAX(m_height, (uint32_t)m_av1PicParams->m_frameHeightMinus1 + 1);
        m_frameWidthAlignedMinBlk       = MOS_ALIGN_CEIL(m_av1PicParams->m_frameWidthMinus1 + 1, av1MinBlockWidth);
        m_frameHeightAlignedMinBlk      = MOS_ALIGN_CEIL(m_av1PicParams->m_frameHeightMinus1 + 1, av1MinBlockHeight);

        m_refFrameIndexList.clear();
        for (auto i = 0; i < av1TotalRefsPerFrame; i++)
        {
            uint8_t index = m_av1PicParams->m_refFrameMap[i].FrameIdx;
            if (index < CODECHAL_MAX_DPB_NUM_AV1)
            {
                    m_refFrameIndexList.push_back(index);
            }
        }

        DECODE_CHK_STATUS(m_internalTarget.UpdateRefList(m_av1PicParams->m_currPic.FrameIdx, m_refFrameIndexList));

        if (m_filmGrainEnabled)
        {
            m_filmGrainProcParams = (CodecProcessingParams *)&decodeParams->m_codecProcParams;
            if (m_filmGrainProcParams->m_inputSurface == nullptr)
            {
                DECODE_CHK_STATUS(m_internalTarget.ActiveCurSurf(m_av1PicParams->m_currPic.FrameIdx, decodeParams->m_destSurface));
                m_filmGrainProcParams->m_inputSurface = m_internalTarget.GetCurSurf();
            }

            m_destSurface = *m_filmGrainProcParams->m_inputSurface;
        }

        DECODE_CHK_STATUS(UpdateDefaultCdfTable());
        DECODE_CHK_STATUS(m_refFrames.UpdatePicture(*m_av1PicParams));
        DECODE_CHK_STATUS(m_tempBuffers.UpdatePicture(m_av1PicParams->m_currPic.FrameIdx, m_refFrameIndexList));
        DECODE_CHK_STATUS(SetSegmentData(*m_av1PicParams));

        DECODE_CHK_STATUS(CalculateGlobalMotionParams());

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1BasicFeature::SetTileStructs()
    {
        DECODE_FUNC_CALL();

        DECODE_CHK_STATUS(m_tileCoding.Update(*m_av1PicParams, m_av1TileParams));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1BasicFeature::SetSegmentData(CodecAv1PicParams &picParams)
    {
        DECODE_FUNC_CALL();

        // Configure Segment ID read buffer SegmentMapIsZeroFlag
        picParams.m_av1SegData.m_segmentMapIsZeroFlag    = false;
        picParams.m_av1SegData.m_segIdBufStreamInEnable  = false;
        picParams.m_av1SegData.m_segIdBufStreamOutEnable = false;
        uint8_t prevFrameIdx = m_refFrames.GetPrimaryRefIdx();

        if (picParams.m_av1SegData.m_enabled && 
            (picParams.m_av1SegData.m_temporalUpdate || !picParams.m_av1SegData.m_updateMap))
        {
            if(picParams.m_av1SegData.m_temporalUpdate)
            {
                DECODE_ASSERT((picParams.m_primaryRefFrame != av1PrimaryRefNone) &&
                                        picParams.m_av1SegData.m_updateMap);
            }

            if (!picParams.m_av1SegData.m_updateMap)
            {
                DECODE_ASSERT((picParams.m_primaryRefFrame != av1PrimaryRefNone) &&
                                       !picParams.m_av1SegData.m_temporalUpdate);
            }

            if (m_refFrames.CheckSegForPrimFrame(*m_av1PicParams))
            {
                picParams.m_av1SegData.m_segmentMapIsZeroFlag = false;
                picParams.m_av1SegData.m_segIdBufStreamInEnable = true;
            }
            else
            {
                picParams.m_av1SegData.m_segmentMapIsZeroFlag = true;
                picParams.m_av1SegData.m_segIdBufStreamInEnable = false;
            }
        }

        if(picParams.m_av1SegData.m_enabled && picParams.m_av1SegData.m_updateMap)
        {
            picParams.m_av1SegData.m_segIdBufStreamOutEnable = true;
        }

        // Calculate m_lastActiveSegmentId/m_preSkipSegmentIdFlag
        if (!picParams.m_av1SegData.m_enabled)
        {
            picParams.m_av1SegData.m_lastActiveSegmentId = 0;
            picParams.m_av1SegData.m_preSkipSegmentIdFlag = 0;
        }
        else if (picParams.m_av1SegData.m_updateData)
        {
            picParams.m_av1SegData.m_lastActiveSegmentId = 0;
            picParams.m_av1SegData.m_preSkipSegmentIdFlag = 0;

            for (uint8_t seg = 0; seg < av1MaxSegments; seg++)
            {
                for (int lvl = 0; lvl < segLvlMax; lvl++)
                {
                    if (picParams.m_av1SegData.m_featureMask[seg] & (1 << lvl))
                    {
                        picParams.m_av1SegData.m_preSkipSegmentIdFlag |= (lvl >= segLvlRefFrame);
                        picParams.m_av1SegData.m_lastActiveSegmentId = seg;
                    }
                }
            }
        }
        else if (picParams.m_primaryRefFrame != av1PrimaryRefNone)//copy from primary_ref_frame
        {
            picParams.m_av1SegData.m_lastActiveSegmentId = m_refFrames.m_refList[prevFrameIdx]->m_lastActiveSegmentId;
            picParams.m_av1SegData.m_preSkipSegmentIdFlag = m_refFrames.m_refList[prevFrameIdx]->m_preSkipSegmentIdFlag;
        }

        //record m_lastActiveSegmentId/m_preSkipSegmentIdFlag into DPB for future frame decoding
        m_refFrames.m_currRefList->m_lastActiveSegmentId = picParams.m_av1SegData.m_lastActiveSegmentId;
        m_refFrames.m_currRefList->m_preSkipSegmentIdFlag = picParams.m_av1SegData.m_preSkipSegmentIdFlag;

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1BasicFeature::CalculateGlobalMotionParams()
    {
        DECODE_FUNC_CALL();

        for (uint32_t ref = (uint32_t)lastFrame; ref <= (uint32_t)altRefFrame; ref++)
        {
            if (m_av1PicParams->m_wm[ref - lastFrame].m_wmtype >= rotzoom)
            {
                m_av1PicParams->m_wm[ref - lastFrame].m_wmmat[2] -= (1 << av1WarpedModelPrecBits);
                m_av1PicParams->m_wm[ref - lastFrame].m_wmmat[2] >>= av1GmAlphaPrecDiff;
                m_av1PicParams->m_wm[ref - lastFrame].m_wmmat[3] >>= av1GmAlphaPrecDiff;
            }

            if (m_av1PicParams->m_wm[ref - lastFrame].m_wmtype == affine)
            {
                m_av1PicParams->m_wm[ref - lastFrame].m_wmmat[4] >>= av1GmAlphaPrecDiff;
                m_av1PicParams->m_wm[ref - lastFrame].m_wmmat[5] -= (1 << av1WarpedModelPrecBits);
                m_av1PicParams->m_wm[ref - lastFrame].m_wmmat[5] >>= av1GmAlphaPrecDiff;
            }
            else
            {
                m_av1PicParams->m_wm[ref - lastFrame].m_wmmat[4] = -m_av1PicParams->m_wm[ref - lastFrame].m_wmmat[3];
                m_av1PicParams->m_wm[ref - lastFrame].m_wmmat[5] = m_av1PicParams->m_wm[ref - lastFrame].m_wmmat[2];
            }

            if (m_av1PicParams->m_wm[ref - lastFrame].m_wmtype >= translation)
            {
                int32_t transDecFactorShift = (m_av1PicParams->m_wm[ref - lastFrame].m_wmtype == translation) ?
                    (av1GmTransOnlyPrecDiff + (m_av1PicParams->m_picInfoFlags.m_fields.m_allowHighPrecisionMv ? 0 : 1)) : av1GmTransPrecDiff;

                m_av1PicParams->m_wm[ref - lastFrame].m_wmmat[0] >>= transDecFactorShift;
                m_av1PicParams->m_wm[ref - lastFrame].m_wmmat[1] >>= transDecFactorShift;
            }
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1BasicFeature::InitDefaultFrameContextBuffer(
        uint16_t              *ctxBuffer,
        uint8_t               index)
    {
        DECODE_CHK_NULL(ctxBuffer);

        //initialize the layout and default table info for each syntax element
        struct SyntaxElementCdfTableLayout syntaxElementsLayout[syntaxElementMax] =
        {
        //m_entryCountPerCL, m_entryCountTotal, m_startCL, *m_srcInitBuffer
        //PartI: Intra
        { 30,    12  ,    0  ,    (uint16_t *)&defaultPartitionCdf8x8[0][0] },        //    partition_8x8
        { 27,    108 ,    1  ,    (uint16_t *)&defaultPartitionCdfNxN[0][0] },        //    partition
        { 28,    28  ,    5  ,    (uint16_t *)&defaultPartitionCdf128x128[0][0] },    //    partition_128x128
        { 32,    3   ,    6  ,    (uint16_t *)&defaultSkipCdfs[0][0] },               //    skip
        { 30,    3   ,    7  ,    (uint16_t *)&defaultDeltaQCdf[0] },                 //    delta_q
        { 30,    3   ,    8  ,    (uint16_t *)&defaultDeltaLfCdf[0] },                //    delta_lf
        { 30,    12  ,    9  ,    (uint16_t *)&defaultDeltaLfMultiCdf[0][0] },        //    delta_lf_multi
        { 28,    21  ,    10 ,    (uint16_t *)&defaultSpatialPredSegTreeCdf[0][0] },  //    segment_id
        { 24,    300 ,    11 ,    (uint16_t *)&defaultKfYModeCdf[0][0][0] },          //    intra_y_mode
        { 24,    156 ,    24 ,    (uint16_t *)&defaultUvModeCdf0[0][0] },             //    uv_mode_0
        { 26,    169 ,    31 ,    (uint16_t *)&defaultUvModeCdf1[0][0] },             //    uv_mode_1
        { 32,    21  ,    38 ,    (uint16_t *)&defaultPaletteYModeCdf[0][0][0] },     //    palette_y_mode
        { 32,    2   ,    39 ,    (uint16_t *)&defaultPaletteUvModeCdf[0][0] },       //    palette_uv_mode
        { 30,    42  ,    40 ,    (uint16_t *)&defaultPaletteYSizeCdf[0][0] },        //    palette_y_size
        { 30,    42  ,    42 ,    (uint16_t *)&defaultPaletteUvSizeCdf[0][0] },       //    palette_uv_size
        { 30,    312 ,    44 ,    (uint16_t *)&defaultIntraExtTxCdf1[0][0][0] },      //    intra_tx_type_1
        { 32,    208 ,    55 ,    (uint16_t *)&defaultIntraExtTxCdf2[0][0][0] },      //    intra_tx_type_2
        { 32,    3   ,    62 ,    (uint16_t *)&defaultTxSizeCdf0[0][0] },             //    depth_0
        { 32,    18  ,    63 ,    (uint16_t *)&defaultTxSizeCdf[0][0][0] },           //    depth
        { 28,    7   ,    64 ,    (uint16_t *)&defaultCflSignCdf[0] },                //    cfl_joint_sign
        { 30,    90  ,    65 ,    (uint16_t *)&defaultCflAlphaCdf[0][0] },            //    cdf_alpha
        { 30,    48  ,    68 ,    (uint16_t *)&defaultAngleDeltaCdf[0][0] },          //    angle_delta
        { 32,    5   ,    70 ,    (uint16_t *)&defaultPaletteYColorIndexCdf0[0][0] }, //    palette_y_color_idx_0
        { 32,    10  ,    71 ,    (uint16_t *)&defaultPaletteYColorIndexCdf1[0][0] }, //    palette_y_color_idx_1
        { 30,    15  ,    72 ,    (uint16_t *)&defaultPaletteYColorIndexCdf2[0][0] }, //    palette_y_color_idx_2
        { 32,    20  ,    73 ,    (uint16_t *)&defaultPaletteYColorIndexCdf3[0][0] }, //    palette_y_color_idx_3
        { 30,    25  ,    74 ,    (uint16_t *)&defaultPaletteYColorIndexCdf4[0][0] }, //    palette_y_color_idx_4
        { 30,    30  ,    75 ,    (uint16_t *)&defaultPaletteYColorIndexCdf5[0][0] }, //    palette_y_color_idx_5
        { 28,    35  ,    76 ,    (uint16_t *)&defaultPaletteYColorIndexCdf6[0][0] }, //    palette_y_color_idx_6
        { 32,    5   ,    78 ,    (uint16_t *)&defaultPaletteUvColorIndexCdf0[0][0] }, //    palette_uv_color_idx_0
        { 32,    10  ,    79 ,    (uint16_t *)&defaultPaletteUvColorIndexCdf1[0][0] }, //    palette_uv_color_idx_1
        { 30,    15  ,    80 ,    (uint16_t *)&defaultPaletteUvColorIndexCdf2[0][0] }, //    palette_uv_color_idx_2
        { 32,    20  ,    81 ,    (uint16_t *)&defaultPaletteUvColorIndexCdf3[0][0] }, //    palette_uv_color_idx_3
        { 30,    25  ,    82 ,    (uint16_t *)&defaultPaletteUvColorIndexCdf4[0][0] }, //    palette_uv_color_idx_4
        { 30,    30  ,    83 ,    (uint16_t *)&defaultPaletteUvColorIndexCdf5[0][0] }, //    palette_uv_color_idx_5
        { 28,    35  ,    84 ,    (uint16_t *)&defaultPaletteUvColorIndexCdf6[0][0] }, //    palette_uv_color_idx_6
        //coeff cdfs addressed by index
        { 32,    65  ,    86 ,    (uint16_t *)&av1DefaultTxbSkipCdfs[index][0][0][0] },               //    txb_skip
        { 32,    16  ,    89 ,    (uint16_t *)&av1DefaultEobMulti16Cdfs[index][0][0][0] },            //    eob_pt_0
        { 30,    20  ,    90 ,    (uint16_t *)&av1DefaultEobMulti32Cdfs[index][0][0][0] },            //    eob_pt_1
        { 30,    24  ,    91 ,    (uint16_t *)&av1DefaultEobMulti64Cdfs[index][0][0][0] },            //    eob_pt_2
        { 28,    28  ,    92 ,    (uint16_t *)&av1DefaultEobMulti128Cdfs[index][0][0][0] },           //    eob_pt_3
        { 32,    32  ,    93 ,    (uint16_t *)&av1DefaultEobMulti256Cdfs[index][0][0][0] },           //    eob_pt_4
        { 27,    36  ,    94 ,    (uint16_t *)&av1DefaultEobMulti512Cdfs[index][0][0][0] },           //    eob_pt_5
        { 30,    40  ,    96 ,    (uint16_t *)&av1DefaultEobMulti1024Cdfs[index][0][0][0] },          //    eob_pt_6
        { 32,    90  ,    98 ,    (uint16_t *)&av1DefaultEobExtraCdfs[index][0][0][0][0] },           //    eob_extra
        { 32,    80  ,    101,    (uint16_t *)&av1DefaultCoeffBaseEobMultiCdfs[index][0][0][0][0] },  //    coeff_base_eob
        { 30,    1260,    104,    (uint16_t *)&av1DefaultCoeffBaseMultiCdfs[index][0][0][0][0] },     //    coeff_base
        { 32,    6   ,    146,    (uint16_t *)&av1DefaultDcSignCdfs[index][0][0][0] },                //    dc_sign
        { 30,    630 ,    147,    (uint16_t *)&av1DefaultCoeffLpsMultiCdfs[index][0][0][0][0] },      //    coeff_br
        { 32,    2   ,    168,    (uint16_t *)&defaultSwitchableRestoreCdf[0] },  //    switchable_restore
        { 32,    1   ,    169,    (uint16_t *)&defaultWienerRestoreCdf[0] },      //    wiener_restore
        { 32,    1   ,    170,    (uint16_t *)&defaultSgrprojRestoreCdf[0] },     //    sgrproj_restore
        { 32,    1   ,    171,    (uint16_t *)&defaultIntrabcCdf[0] },            //    use_intrabc
        { 32,    22  ,    172,    (uint16_t *)&default_filter_intra_cdfs[0][0] }, //    use_filter_intra
        { 32,    4   ,    173,    (uint16_t *)&defaultFilterIntraModeCdf[0] },    //    filter_intra_mode
        { 30,    3   ,    174,    (uint16_t *)&defaultJointCdf[0] },              //    dv_joint_type
        { 32,    2   ,    175,    (uint16_t *)&defaultSignCdf[0][0] },            //    dv_sign
        { 32,    20  ,    176,    (uint16_t *)&defaultBitsCdf[0][0][0] },         //    dv_sbits
        { 30,    20  ,    177,    (uint16_t *)&defaultClassesCdf[0][0] },         //    dv_class
        { 32,    2   ,    178,    (uint16_t *)&defaultClass0Cdf[0][0] },          //    dv_class0
        { 30,    6   ,    179,    (uint16_t *)&defaultFpCdf[0][0] },              //    dv_fr
        { 30,    12  ,    180,    (uint16_t *)&defaultClass0FpCdf[0][0][0] },     //    dv_class0_fr
        { 32,    2   ,    181,    (uint16_t *)&defaultHpCdf[0][0] },              //    dv_hp
        { 32,    2   ,    182,    (uint16_t *)&defaultClass0HpCdf[0][0] },        //    dv_class0_hp
        //PartII: Inter
        { 32,    3   ,    183,    (uint16_t *)&defaultSkipModeCdfs[0][0] },           //    skip_mode
        { 32,    3   ,    184,    (uint16_t *)&defaultSegmentPredCdf[0][0] },         //    pred_seg_id
        { 24,    48  ,    185,    (uint16_t *)&defaultIfYModeCdf[0][0] },             //    y_mode
        { 30,    60  ,    187,    (uint16_t *)&defaultInterExtTxCdf1[0][0] },         //    inter_tx_type_1
        { 22,    44  ,    189,    (uint16_t *)&defaultInterExtTxCdf2[0][0] },         //    inter_tx_type_2
        { 32,    4   ,    191,    (uint16_t *)&defaultInterExtTxCdf3[0][0] },         //    inter_tx_type_3
        { 32,    4   ,    192,    (uint16_t *)&defaultIntraInterCdf[0][0] },          //    is_inter
        { 32,    21  ,    193,    (uint16_t *)&defaultTxfmPartitionCdf[0][0] },       //    tx_split
        { 32,    5   ,    194,    (uint16_t *)&defaultCompInterCdf[0][0] },           //    ref_mode
        { 32,    5   ,    195,    (uint16_t *)&defaultCompRefTypeCdf[0][0] },         //    comp_ref_type
        { 32,    9   ,    196,    (uint16_t *)&defaultUniCompRefCdf[0][0][0] },       //    unidir_comp_ref
        { 32,    9   ,    197,    (uint16_t *)&defaultCompRefCdf[0][0][0] },          //    ref_bit
        { 32,    6   ,    198,    (uint16_t *)&defaultCompBwdrefCdf[0][0][0] },       //    ref_bit_bwd
        { 32,    18  ,    199,    (uint16_t *)&defaultSingleRefCdf[0][0][0] },        //    single_ref_bit
        { 28,    56  ,    200,    (uint16_t *)&defaultInterCompoundModeCdf[0][0] },   // inter_compound_mode
        { 32,    6   ,    202,    (uint16_t *)&defaultNewmvCdf[0][0] },               //    is_newmv
        { 32,    2   ,    203,    (uint16_t *)&defaultZeromvCdf[0][0] },              //    is_zeromv
        { 32,    6   ,    204,    (uint16_t *)&defaultRefmvCdf[0][0] },               //    is_refmv
        { 30,    3   ,    205,    (uint16_t *)&defaultJointCdf[0] },                  //    mv_joint_type
        { 32,    2   ,    206,    (uint16_t *)&defaultSignCdf[0][0] },                //    mv_sign
        { 32,    20  ,    207,    (uint16_t *)&defaultBitsCdf[0][0][0] },             //    mv_sbits
        { 30,    20  ,    208,    (uint16_t *)&defaultClassesCdf[0][0] },             //    mv_class
        { 32,    2   ,    209,    (uint16_t *)&defaultClass0Cdf[0][0] },              //    mv_class0
        { 30,    6   ,    210,    (uint16_t *)&defaultFpCdf[0][0] },                  //    mv_fr
        { 30,    12  ,    211,    (uint16_t *)&defaultClass0FpCdf[0][0][0] },         //    mv_class0_fr
        { 32,    2   ,    212,    (uint16_t *)&defaultHpCdf[0][0] },                  //    mv_hp
        { 32,    2   ,    213,    (uint16_t *)&defaultClass0HpCdf[0][0] },            //    mv_class0_hp
        { 32,    4   ,    214,    (uint16_t *)&defaultInterintraCdf[0][0] },          //    interintra
        { 30,    12  ,    215,    (uint16_t *)&defaultInterintraModeCdf[0][0] },      //    interintra_mode
        { 32,    22  ,    216,    (uint16_t *)&defaultWedgeInterintraCdf[0][0] },     //    use_wedge_interintra
        { 30,    330 ,    217,    (uint16_t *)&defaultWedgeIdxCdf[0][0] },            //    wedge_index
        { 32,    3   ,    228,    (uint16_t *)&defaultDrlCdf[0][0] },                 //    drl_idx
        { 32,    22  ,    229,    (uint16_t *)&defaultObmcCdf[0][0] },                //    obmc_motion_mode
        { 32,    44  ,    230,    (uint16_t *)&defaultMotionModeCdf[0][0] },          //    non_obmc_motion_mode
        { 32,    6   ,    232,    (uint16_t *)&defaultCompGroupIdxCdfs[0][0] },       //    comp_group_idx
        { 32,    6   ,    233,    (uint16_t *)&defaultCompoundIdxCdfs[0][0] },        //    compound_idx
        { 32,    22  ,    234,    (uint16_t *)&defaultCompoundTypeCdf[0][0] },        //    interinter_compound_type
        { 32,    32  ,    235,    (uint16_t *)&defaultSwitchableInterpCdf[0][0] },    //    switchable_interp
        };

        for (auto idx = (uint32_t)partition8x8; idx < (uint32_t)syntaxElementMax; idx++)
        {
            DECODE_CHK_STATUS(SyntaxElementCdfTableInit(
                ctxBuffer,
                syntaxElementsLayout[idx]));
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1BasicFeature::SyntaxElementCdfTableInit(
        uint16_t                    *ctxBuffer,
        SyntaxElementCdfTableLayout SyntaxElement)
    {
        DECODE_CHK_NULL(SyntaxElement.m_srcInitBuffer);

        uint16_t    entryCountPerCL = SyntaxElement.m_entryCountPerCL;  //one entry means one uint16_t value
        uint16_t    entryCountTotal = SyntaxElement.m_entryCountTotal;  //total number of entrie for this Syntax element's CDF tables
        uint16_t    startCL         = SyntaxElement.m_startCL;

        uint16_t *src = SyntaxElement.m_srcInitBuffer;
        uint16_t *dst = ctxBuffer + startCL * 32;   //one CL equals to 32 uint16_t
        uint16_t entryCountLeft = entryCountTotal;
        while (entryCountLeft >= entryCountPerCL)
        {
            //copy one CL
            MOS_SecureMemcpy(dst, entryCountPerCL * sizeof(uint16_t), src, entryCountPerCL * sizeof(uint16_t));
            entryCountLeft -= entryCountPerCL;

            //go to next CL
            src += entryCountPerCL;
            dst += 32;
        };
        //copy the remaining which are less than a CL
        if (entryCountLeft > 0)
        {
            MOS_SecureMemcpy(dst, entryCountLeft * sizeof(uint16_t), src, entryCountLeft * sizeof(uint16_t));
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1BasicFeature :: UpdateDefaultCdfTable()
    {
        DECODE_FUNC_CALL();

        if (!m_defaultFcInitialized)
        {
            for (uint8_t index = 0; index < av1DefaultCdfTableNum; index++)
            {
                m_tmpCdfBuffers[index] = m_allocator->AllocateBuffer(MOS_ALIGN_CEIL(m_cdfMaxNumBytes, CODECHAL_PAGE_SIZE), "TempCdfTableBuffer");
                DECODE_CHK_NULL(m_tmpCdfBuffers[index]);

                auto data = (uint16_t *)m_allocator->LockResouceForWrite(&m_tmpCdfBuffers[index]->OsResource);
                DECODE_CHK_NULL(data);

                // reset all CDF tables to default values
                DECODE_CHK_STATUS(InitDefaultFrameContextBuffer(data, index));
                m_defaultCdfBuffers[index] = m_allocator->AllocateBuffer(MOS_ALIGN_CEIL(m_cdfMaxNumBytes, CODECHAL_PAGE_SIZE), "m_defaultCdfBuffers");
                DECODE_CHK_NULL(m_defaultCdfBuffers[index]);
            }

            m_defaultFcInitialized = true;//set only once, won't set again
        }

        //Calculate the current frame's Coeff CDF Q Context ID, that is the Coeff CDF Buffer index
        if (m_av1PicParams->m_baseQindex <= 20)    m_curCoeffCdfQCtx = 0;
        else if (m_av1PicParams->m_baseQindex <= 60)    m_curCoeffCdfQCtx = 1;
        else if (m_av1PicParams->m_baseQindex <= 120)   m_curCoeffCdfQCtx = 2;
        else m_curCoeffCdfQCtx = 3;

        m_defaultCdfBufferInUse = m_defaultCdfBuffers[m_curCoeffCdfQCtx];

        return MOS_STATUS_SUCCESS;
    }

}  // namespace decode
