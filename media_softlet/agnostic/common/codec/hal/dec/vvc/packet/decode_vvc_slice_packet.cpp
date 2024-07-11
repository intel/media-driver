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
//! \file     decode_vvc_slice_packet.cpp
//! \brief    Defines the interface for VVC decode slice packet
//!
#include "decode_vvc_slice_packet.h"

namespace decode
{
    MOS_STATUS VvcDecodeSlicePkt::Init()
    {
        DECODE_FUNC_CALL();

        DECODE_CHK_NULL(m_featureManager);
        DECODE_CHK_NULL(m_hwInterface);
        DECODE_CHK_NULL(m_osInterface);
        DECODE_CHK_NULL(m_vvcPipeline);

        m_vvcBasicFeature = dynamic_cast<VvcBasicFeature*>(m_featureManager->GetFeature(FeatureIDs::basicFeature));
        DECODE_CHK_NULL(m_vvcBasicFeature);

        m_allocator = m_pipeline ->GetDecodeAllocator();
        DECODE_CHK_NULL(m_allocator);
        m_vvcCpSubPkt = m_vvcPipeline->GetSubPacket(DecodePacketId(m_vvcPipeline, vvcCpSubPacketId));
        m_decodecp    = m_pipeline->GetDecodeCp();
        
        DECODE_CHK_STATUS(CalculateSliceStateCommandSize());

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS VvcDecodeSlicePkt::Prepare()
    {
        DECODE_FUNC_CALL();

        DECODE_CHK_NULL(m_vvcBasicFeature->m_vvcPicParams);

        m_vvcPicParams = m_vvcBasicFeature->m_vvcPicParams;

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS VvcDecodeSlicePkt::GetPartitionInfo(uint16_t sliceIdx)
    {
        DECODE_FUNC_CALL()

        DECODE_CHK_NULL(m_vvcBasicFeature);
        DECODE_CHK_NULL(m_vvcPicParams);

        DECODE_CHK_NULL(m_vvcBasicFeature->m_vvcSliceParams);

        m_curSliceParams = &m_vvcBasicFeature->m_vvcSliceParams[sliceIdx];

        //current subPic and slice
        if (!m_vvcPicParams->m_ppsFlags.m_fields.m_ppsRectSliceFlag)
        {
            m_subPicParams  = nullptr;
            m_sliceDesc     = &m_vvcBasicFeature->m_sliceDesc[sliceIdx];
        }
        else if (!m_vvcPicParams->m_ppsFlags.m_fields.m_ppsSingleSlicePerSubpicFlag)
        {
            if (m_vvcPicParams->m_spsFlags0.m_fields.m_spsSubpicInfoPresentFlag && m_vvcPicParams->m_spsNumSubpicsMinus1 > 0)
            {
                DECODE_CHK_NULL(m_vvcBasicFeature->m_subPicParams);
                uint16_t subPicId           = m_curSliceParams->m_shSubpicId;
                uint16_t subPicIdx          = m_vvcBasicFeature->GetSubPicIdxFromSubPicId(subPicId);
                uint16_t sliceIdxInSubpic   = m_curSliceParams->m_shSliceAddress;
                uint16_t sliceIdxInPic      = m_vvcBasicFeature->m_subPicParams[subPicIdx].m_sliceIdx[sliceIdxInSubpic];

                m_subPicParams  = &m_vvcBasicFeature->m_subPicParams[subPicIdx];
                m_sliceDesc     = &m_vvcBasicFeature->m_sliceDesc[sliceIdxInPic];
            }
            else
            {
                uint16_t subPicId           = m_curSliceParams->m_shSubpicId;
                DECODE_ASSERT(subPicId == 0);
                uint16_t sliceIdxInSubpic   = m_curSliceParams->m_shSliceAddress;

                m_subPicParams  = nullptr;
                m_sliceDesc     = &m_vvcBasicFeature->m_sliceDesc[sliceIdxInSubpic];
            }
        }
        else
        {
            if (m_vvcPicParams->m_spsFlags0.m_fields.m_spsSubpicInfoPresentFlag && m_vvcPicParams->m_spsNumSubpicsMinus1 > 0)
            {
                DECODE_CHK_NULL(m_vvcBasicFeature->m_subPicParams);
                uint16_t subPicId           = m_curSliceParams->m_shSubpicId;
                uint16_t subPicIdx          = m_vvcBasicFeature->GetSubPicIdxFromSubPicId(subPicId);
                DECODE_ASSERT(m_curSliceParams->m_shSliceAddress == 0);

                m_subPicParams  = &m_vvcBasicFeature->m_subPicParams[subPicIdx];
                m_sliceDesc     = &m_vvcBasicFeature->m_sliceDesc[subPicIdx];
            }
            else
            {
                m_subPicParams  = nullptr;
                DECODE_ASSERT(m_curSliceParams->m_shSubpicId == 0);
                DECODE_ASSERT(m_curSliceParams->m_shSliceAddress == 0);
                m_sliceDesc     = &m_vvcBasicFeature->m_sliceDesc[0];
            }
        }

        //The last slice flag
        if (m_vvcPicParams->m_ppsFlags.m_fields.m_ppsRectSliceFlag)
        {
            uint32_t endTileX = m_sliceDesc->m_startTileX + m_sliceDesc->m_sliceWidthInTiles - 1;
            if (endTileX >= vvcMaxTileColNum)
            {
                DECODE_ASSERTMESSAGE("Right most tile column of the slice is incorrect!\n");
                return MOS_STATUS_INVALID_PARAMETER;
            }
            uint32_t sliceWidthInCtb = m_vvcBasicFeature->m_tileCol[endTileX].m_endCtbX + 1 - m_vvcBasicFeature->m_tileCol[m_sliceDesc->m_startTileX].m_startCtbX;
            uint32_t sliceEndCtbX = m_sliceDesc->m_sliceStartCtbx + sliceWidthInCtb - 1;
            uint32_t sliceEndCtbY = 0;
            if (m_sliceDesc->m_multiSlicesInTileFlag)
            {
                sliceEndCtbY = m_sliceDesc->m_sliceStartCtby + m_sliceDesc->m_sliceHeightInCtu - 1;
            }
            else
            {
                uint32_t endTileY = m_sliceDesc->m_startTileY + m_sliceDesc->m_sliceHeightInTiles - 1;
                if (endTileY >= vvcMaxTileRowNum)
                {
                    DECODE_ASSERTMESSAGE("Bottom most tile row of the slice is incorrect!\n");
                    return MOS_STATUS_INVALID_PARAMETER;
                }
                uint32_t sliceHeightInCtb = m_vvcBasicFeature->m_tileRow[endTileY].m_endCtbY + 1 - m_vvcBasicFeature->m_tileRow[m_sliceDesc->m_startTileY].m_startCtbY;
                sliceEndCtbY = m_sliceDesc->m_sliceStartCtby + sliceHeightInCtb - 1;
            }

            m_sliceDesc->m_sliceEndCtbx = sliceEndCtbX;
            m_sliceDesc->m_sliceEndCtby = sliceEndCtbY;

            m_lastSliceOfPic = ((sliceEndCtbX == m_vvcBasicFeature->m_picWidthInCtu - 1) &&
                (sliceEndCtbY == m_vvcBasicFeature->m_picHeightInCtu - 1)) ? 1 : 0;
        }
        else
        {
            uint16_t tileOffset = m_curSliceParams->m_shSliceAddress;
            uint16_t tileNum = m_curSliceParams->m_shNumTilesInSliceMinus1+1;
            m_lastSliceOfPic = (tileOffset + tileNum == m_vvcBasicFeature->m_tileCols * m_vvcBasicFeature->m_tileRows) ? 1 : 0;
        }
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS VvcDecodeSlicePkt::Execute(MOS_COMMAND_BUFFER& cmdBuffer, uint16_t sliceIdx)
    {
        DECODE_FUNC_CALL()

        DECODE_CHK_NULL(m_vvcBasicFeature);
        DECODE_CHK_NULL(m_vvcPicParams);

        DECODE_CHK_STATUS(GetPartitionInfo(sliceIdx));

        // Slice Level Commands
        SETPAR_AND_ADDCMD(VVCP_SLICE_STATE, m_vvcpItf, &cmdBuffer);
        DECODE_CHK_STATUS(AddAllCmds_VVCP_REF_IDX_STATE(cmdBuffer));
        DECODE_CHK_STATUS(AddAllCmds_VVCP_WEIGHTOFFSET_STATE(cmdBuffer));
        // CP commands
        if (m_vvcCpSubPkt != nullptr && m_decodecp != nullptr)
        {
            DECODE_CHK_STATUS(m_decodecp->ExecuteDecodeCpIndSubPkt(m_vvcCpSubPkt, m_vvcBasicFeature->m_mode, cmdBuffer, sliceIdx));
        }
        SETPAR_AND_ADDCMD(VVCP_BSD_OBJECT, m_vvcpItf, &cmdBuffer);

        // Tile level commands
        AddAllCmds_VVCP_TILE_CODING(cmdBuffer);

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS VvcDecodeSlicePkt::ConstructLmcsReshaper() const
    {
        DECODE_FUNC_CALL()

        int32_t reshapeLUTSize = 1 << (m_vvcPicParams->m_spsBitdepthMinus8 + 8);
        int32_t pwlFwdLUTsize = vvcPicCodeCwBins;
        int32_t pwlFwdBinLen = reshapeLUTSize / vvcPicCodeCwBins;
        uint16_t initCW = (uint16_t)pwlFwdBinLen;

        CodecVvcLmcsData* lmcsData = &m_vvcBasicFeature->m_lmcsApsArray[m_vvcPicParams->m_phLmcsApsId];
        ApsLmcsReshapeInfo* sliceReshapeInfo = &m_vvcBasicFeature->m_lmcsReshaperInfo[m_vvcPicParams->m_phLmcsApsId];
        uint32_t reshaperModelMaxBinIdx = vvcPicCodeCwBins - 1 - lmcsData->m_lmcsDeltaMaxBinIdx;

        for (int32_t i = 0; i < lmcsData->m_lmcsMinBinIdx; i++)
        {
            sliceReshapeInfo->m_lmcsCW[i] = 0;
        }
        for (auto i = reshaperModelMaxBinIdx + 1; i < vvcPicCodeCwBins; i++)
        {
            sliceReshapeInfo->m_lmcsCW[i] = 0;
        }
        for (auto i = lmcsData->m_lmcsMinBinIdx; i <= reshaperModelMaxBinIdx; i++)
        {
            sliceReshapeInfo->m_lmcsCW[i] = (uint16_t)(lmcsData->m_lmcsDeltaCW[i] + (int32_t)initCW);
        }

        for (auto i = 0; i < pwlFwdLUTsize; i++)
        {
            sliceReshapeInfo->m_lmcsPivot[i + 1] = sliceReshapeInfo->m_lmcsPivot[i] + sliceReshapeInfo->m_lmcsCW[i];
            sliceReshapeInfo->m_scaleCoeff[i] = ((int32_t)sliceReshapeInfo->m_lmcsCW[i] * (1 << vvcFpPrec) + (1 << ((int32_t)log2(pwlFwdBinLen) - 1))) >> (int32_t)log2(pwlFwdBinLen);
            if (sliceReshapeInfo->m_lmcsCW[i] == 0)
            {
                sliceReshapeInfo->m_invScaleCoeff[i] = 0;
                sliceReshapeInfo->m_chromaScaleCoeff[i] = 1 << vvcCscaleFpPrec;
            }
            else
            {
                int32_t lmcsCwCrs = sliceReshapeInfo->m_lmcsCW[i] + lmcsData->m_lmcsDeltaCrs;
                if (lmcsCwCrs < (initCW >> 3) || (lmcsCwCrs > (initCW << 3) -1))
                {
                    DECODE_ASSERTMESSAGE("Error concealed: force sh_lmcs_used_flag = 0 when (lmcsCW[ i ] + lmcsDeltaCrs) out of the range (OrgCW >> 3) to ((OrgCW << 3) - 1) inclusive.\n");
                    m_curSliceParams->m_longSliceFlags.m_fields.m_shLmcsUsedFlag = 0;
                }
                else
                {
                    sliceReshapeInfo->m_invScaleCoeff[i] = (int32_t)(initCW * (1 << vvcFpPrec) / sliceReshapeInfo->m_lmcsCW[i]);
                    sliceReshapeInfo->m_chromaScaleCoeff[i] = (int32_t)(initCW * (1 << vvcFpPrec) / (sliceReshapeInfo->m_lmcsCW[i] + lmcsData->m_lmcsDeltaCrs));
                }
            }
        }

        //mark the flag
        m_vvcBasicFeature->m_lmcsReshaperReady |= 1 << m_vvcPicParams->m_phLmcsApsId;

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS VvcDecodeSlicePkt::CalcRefIdxSymLx(int8_t &RefIdxSymL0, int8_t &RefIdxSymL1)
    {
        DECODE_FUNC_CALL();

        //Calc low delay flag
        bool lowDelay = true;
        int32_t  currPOC = m_vvcPicParams->m_picOrderCntVal;
        uint8_t  refIdx = 0;

        if (!m_vvcpItf->IsVvcISlice(m_curSliceParams->m_shSliceType))
        {
            for (refIdx = 0; refIdx < m_curSliceParams->m_numRefIdxActive[0] && lowDelay; refIdx++)
            {
                uint8_t refPicIdx = m_curSliceParams->m_refPicList[0][refIdx].FrameIdx;

                if (m_vvcPicParams->m_refFramePocList[refPicIdx] > currPOC)
                {
                    lowDelay = false;
                }
            }
            if (m_vvcpItf->IsVvcBSlice(m_curSliceParams->m_shSliceType))
            {
                for (refIdx = 0; refIdx < m_curSliceParams->m_numRefIdxActive[1] && lowDelay; refIdx++)
                {
                    uint8_t refPicIdx = m_curSliceParams->m_refPicList[1][refIdx].FrameIdx;
                    if (m_vvcPicParams->m_refFramePocList[refPicIdx] > currPOC)
                    {
                        lowDelay = false;
                    }
                }
            }
        }

        //Calc RefIdxSymL0/RefIdxSymL1
        if (m_vvcPicParams->m_spsFlags1.m_fields.m_spsSmvdEnabledFlag && lowDelay == false
            && m_vvcPicParams->m_phFlags.m_fields.m_phMvdL1ZeroFlag == false)
        {
            int32_t forwardPOC = currPOC;
            int32_t backwardPOC = currPOC;
            uint8_t ref = 0;
            int8_t refIdx0 = -1;
            int8_t refIdx1 = -1;

            // search nearest forward POC in List 0
            for ( ref = 0; ref < m_curSliceParams->m_numRefIdxActive[0]; ref++ )
            {
                if (m_curSliceParams->m_refPicList[0][ref].FrameIdx >= vvcMaxNumRefFrame)
                {
                    return MOS_STATUS_INVALID_PARAMETER;
                }
                int32_t poc = m_vvcPicParams->m_refFramePocList[m_curSliceParams->m_refPicList[0][ref].FrameIdx];

                const bool isRefLongTerm = (m_curSliceParams->m_refPicList[0][ref].PicFlags == PICTURE_LONG_TERM_REFERENCE) ? true : false;
                if ( poc < currPOC && (poc > forwardPOC || refIdx0 == -1) && !isRefLongTerm )
                {
                    forwardPOC = poc;
                    refIdx0 = ref;
                }
            }

            // search nearest backward POC in List 1
            for ( ref = 0; ref < m_curSliceParams->m_numRefIdxActive[1]; ref++ )
            {
                if (m_curSliceParams->m_refPicList[1][ref].FrameIdx >= vvcMaxNumRefFrame)
                {
                    return MOS_STATUS_INVALID_PARAMETER;
                }
                int32_t poc = m_vvcPicParams->m_refFramePocList[m_curSliceParams->m_refPicList[1][ref].FrameIdx];


                const bool isRefLongTerm = (m_curSliceParams->m_refPicList[1][ref].PicFlags == PICTURE_LONG_TERM_REFERENCE) ? true : false;
                if ( poc > currPOC && (poc < backwardPOC || refIdx1 == -1) && !isRefLongTerm )
                {
                    backwardPOC = poc;
                    refIdx1 = ref;
                }
            }

            if ( !(forwardPOC < currPOC && backwardPOC > currPOC) )
            {
                forwardPOC = currPOC;
                backwardPOC = currPOC;
                refIdx0 = -1;
                refIdx1 = -1;

                // search nearest backward POC in List 0
                for ( ref = 0; ref < m_curSliceParams->m_numRefIdxActive[0]; ref++ )
                {
                    if (m_curSliceParams->m_refPicList[0][ref].FrameIdx >= vvcMaxNumRefFrame)
                    {
                        return MOS_STATUS_INVALID_PARAMETER;
                    }
                    int32_t poc = m_vvcPicParams->m_refFramePocList[m_curSliceParams->m_refPicList[0][ref].FrameIdx];

                    const bool isRefLongTerm = (m_curSliceParams->m_refPicList[0][ref].PicFlags == PICTURE_LONG_TERM_REFERENCE) ? true : false;
                    if ( poc > currPOC && (poc < backwardPOC || refIdx0 == -1) && !isRefLongTerm )
                    {
                        backwardPOC = poc;
                        refIdx0 = ref;
                    }
                }

                // search nearest forward POC in List 1
                for ( ref = 0; ref < m_curSliceParams->m_numRefIdxActive[1]; ref++ )
                {
                    if (m_curSliceParams->m_refPicList[1][ref].FrameIdx >= vvcMaxNumRefFrame)
                    {
                        return MOS_STATUS_INVALID_PARAMETER;
                    }
                    int32_t poc = m_vvcPicParams->m_refFramePocList[m_curSliceParams->m_refPicList[1][ref].FrameIdx];

                    const bool isRefLongTerm = (m_curSliceParams->m_refPicList[1][ref].PicFlags == PICTURE_LONG_TERM_REFERENCE) ? true : false;
                    if ( poc < currPOC && (poc > forwardPOC || refIdx1 == -1) && !isRefLongTerm )
                    {
                        forwardPOC = poc;
                        refIdx1 = ref;
                    }
                }
            }

            if ( forwardPOC < currPOC && backwardPOC > currPOC )
            {
                RefIdxSymL0 = refIdx0;
                RefIdxSymL1 = refIdx1;
            }
            else
            {
                RefIdxSymL0 = -1;
                RefIdxSymL1 = -1;
            }
        }
        else
        {
            RefIdxSymL0 = -1;
            RefIdxSymL1 = -1;
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS VvcDecodeSlicePkt::SetRefIdxStateParams()
    {
        DECODE_FUNC_CALL();

        auto &params = m_vvcpItf->MHW_GETPAR_F(VVCP_REF_IDX_STATE)();
        params       = {};

        if (!m_vvcpItf->IsVvcISlice(m_curSliceParams->m_shSliceType))
        {
            params.listIdx = 0;
            params.numRefForList = m_curSliceParams->m_numRefIdxActive[0];

            //Derive RefIdxSymL0/RefIdxSymL1
            DECODE_CHK_STATUS(CalcRefIdxSymLx(params.refIdxSymLx[0], params.refIdxSymLx[1]));

            DECODE_CHK_STATUS(MOS_SecureMemcpy(&params.refPicList, sizeof(params.refPicList),
                                                &m_curSliceParams->m_refPicList, sizeof(m_curSliceParams->m_refPicList)));

            for (auto i = 0; i < 2; i++)
            {
                for (auto j = 0; j < m_curSliceParams->m_numRefIdxActive[i]; j++)
                {
                    // Short term reference flag
                    params.stRefPicFlag[i][j] = (m_curSliceParams->m_refPicList[i][j].PicFlags == PICTURE_SHORT_TERM_REFERENCE) ? true : false;

                    // Derive RprConstraintsActiveFlag and Unavailable ref pic flag
                    uint8_t refPicIdx = m_curSliceParams->m_refPicList[i][j].FrameIdx;
                    if (refPicIdx < vvcMaxNumRefFrame)
                    {
                        uint8_t refFrameIdx = m_vvcPicParams->m_refFrameList[refPicIdx].FrameIdx;
                        DECODE_CHK_STATUS(m_vvcBasicFeature->m_refFrames.CalcRprConstraintsActiveFlag(refFrameIdx, params.rprConstraintsActiveFlag[i][j]));
                        params.unavailableRefPic[i][j] = (m_vvcPicParams->m_refFrameList[refPicIdx].PicFlags == PICTURE_UNAVAILABLE_FRAME) ? true : false;
                    }
                    else
                    {
                        return MOS_STATUS_INVALID_PARAMETER;
                    }

                    // DiffPicOrderCnt
                    DECODE_ASSERT(m_curSliceParams->m_refPicList[i][j].FrameIdx < vvcMaxNumRefFrame);//TODO: unavailable? associated flag?
                    int32_t refPoc = m_vvcPicParams->m_refFramePocList[m_curSliceParams->m_refPicList[i][j].FrameIdx]; //TODO: For error check, compare the poc from ref list and the value here.
                    params.diffPicOrderCnt[i][j] = m_vvcPicParams->m_picOrderCntVal - refPoc;
                }
            }
        }

        return MOS_STATUS_SUCCESS;
    }

    bool VvcDecodeSlicePkt::IsTileInRasterSlice(const uint32_t tileRow, const uint32_t tileCol) const
    {
        const uint32_t startTileIdx = m_curSliceParams->m_shSliceAddress;
        const uint32_t endTileIdx   = startTileIdx + m_curSliceParams->m_shNumTilesInSliceMinus1;

        const uint32_t tileIdx = tileRow * m_vvcBasicFeature->m_tileCols + tileCol;
        return ((tileIdx >= startTileIdx) && (tileIdx <= endTileIdx));
    }

    MOS_STATUS VvcDecodeSlicePkt::CalculateCommandSize(
        uint32_t &commandBufferSize,
        uint32_t &requestedPatchListSize)
    {
        DECODE_FUNC_CALL();

        commandBufferSize      = m_sliceStatesSize;
        requestedPatchListSize = m_slicePatchListSize;

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS VvcDecodeSlicePkt::CalculateTileCommandSize(
        uint32_t &commandBufferSize,
        uint32_t &requestedPatchListSize)
    {
        DECODE_FUNC_CALL();

        commandBufferSize      = m_tileStateSize;
        requestedPatchListSize = m_tilePatchListSize;

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS VvcDecodeSlicePkt::CalculateSliceStateCommandSize()
    {
        DECODE_FUNC_CALL();

        // Slice Level Commands
        DECODE_CHK_STATUS(m_hwInterface->GetVvcpPrimitiveCommandSize(
                                        m_vvcBasicFeature->m_mode,
                                        &m_sliceStatesSize,
                                        &m_slicePatchListSize,
                                        &m_tileStateSize,
                                        &m_tilePatchListSize));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS VvcDecodeSlicePkt::AddAllCmds_VVCP_REF_IDX_STATE(MOS_COMMAND_BUFFER &cmdBuffer)
    {
        DECODE_FUNC_CALL();

        // Issue RefIdxState command only for non-I slice
        if (m_vvcpItf->IsVvcISlice(m_curSliceParams->m_shSliceType) &&
            (!m_vvcBasicFeature->m_useDummyReference || m_osInterface->bSimIsActive))
        {
            return MOS_STATUS_SUCCESS;
        }

        // L0
        DECODE_CHK_STATUS(SetRefIdxStateParams());
        DECODE_CHK_STATUS(m_vvcpItf->MHW_ADDCMD_F(VVCP_REF_IDX_STATE)(&cmdBuffer));

        // L1
        if (m_vvcpItf->IsVvcBSlice(m_curSliceParams->m_shSliceType))
        {
            auto &params = m_vvcpItf->MHW_GETPAR_F(VVCP_REF_IDX_STATE)();

            params.listIdx       = 1;
            params.numRefForList = m_curSliceParams->m_numRefIdxActive[1];

            DECODE_CHK_STATUS(m_vvcpItf->MHW_ADDCMD_F(VVCP_REF_IDX_STATE)(&cmdBuffer));
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS VvcDecodeSlicePkt::AddAllCmds_VVCP_WEIGHTOFFSET_STATE(MOS_COMMAND_BUFFER &cmdBuffer)
    {
        DECODE_FUNC_CALL();

        bool weightedPred = m_vvcPicParams->m_ppsFlags.m_fields.m_ppsWeightedPredFlag && m_vvcpItf->IsVvcPSlice(m_curSliceParams->m_shSliceType);
        bool weightedBiPred = m_vvcPicParams->m_ppsFlags.m_fields.m_ppsWeightedBipredFlag && m_vvcpItf->IsVvcBSlice(m_curSliceParams->m_shSliceType);

        if (weightedPred || weightedBiPred)
        {            
            if(m_vvcBasicFeature->m_shortFormatInUse)
            {
                DECODE_ASSERTMESSAGE("Warning: Long format specific function, short format should not call!\n");
                return MOS_STATUS_INVALID_PARAMETER;            
            }
            auto &params = m_vvcpItf->MHW_GETPAR_F(VVCP_WEIGHTOFFSET_STATE)();
            params = {};
            params.wpInfo = &m_curSliceParams->m_wpInfo;
            DECODE_CHK_STATUS(m_vvcpItf->MHW_ADDCMD_F(VVCP_WEIGHTOFFSET_STATE)(&cmdBuffer));

            if (weightedBiPred)
            {
                params.listIdx = 1;
                DECODE_CHK_STATUS(m_vvcpItf->MHW_ADDCMD_F(VVCP_WEIGHTOFFSET_STATE)(&cmdBuffer));
            }
        }
        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(VVCP_SLICE_STATE, VvcDecodeSlicePkt)
    {
        params = {};

        if (m_curSliceParams->m_longSliceFlags.m_fields.m_shLmcsUsedFlag &&
            !(m_vvcBasicFeature->m_lmcsReshaperReady & (1 << m_vvcPicParams->m_phLmcsApsId)))
        {
            MOS_ZeroMemory(&m_vvcBasicFeature->m_lmcsReshaperInfo[m_vvcPicParams->m_phLmcsApsId], sizeof(ApsLmcsReshapeInfo));
            DECODE_CHK_STATUS(ConstructLmcsReshaper());
        }

        CodecVvcPicParams   *picParams   = m_vvcBasicFeature->m_vvcPicParams;
        CodecVvcSliceParams *sliceParams = m_curSliceParams;

        if (picParams->m_ppsFlags.m_fields.m_ppsRectSliceFlag)
        {
            MHW_MI_CHK_NULL(m_sliceDesc);
        }

        if (picParams->m_spsFlags0.m_fields.m_spsAlfEnabledFlag)
        {
            if (picParams->m_ppsFlags.m_fields.m_ppsAlfInfoInPhFlag == 1)
            {
                params.shAlfEnabledFlag     = picParams->m_phFlags.m_fields.m_phAlfEnabledFlag;
                params.shAlfCbEnabledFlag   = picParams->m_phFlags.m_fields.m_phAlfCbEnabledFlag;
                params.shAlfCrEnabledFlag   = picParams->m_phFlags.m_fields.m_phAlfCrEnabledFlag;
                params.shAlfCcCbEnabledFlag = picParams->m_phFlags.m_fields.m_phAlfCcCbEnabledFlag;
                params.shAlfCcCrEnabledFlag = picParams->m_phFlags.m_fields.m_phAlfCcCrEnabledFlag;
            }
            else
            {
                params.shAlfEnabledFlag     = sliceParams->m_longSliceFlags.m_fields.m_shAlfEnabledFlag;
                params.shAlfCbEnabledFlag   = sliceParams->m_longSliceFlags.m_fields.m_shAlfCbEnabledFlag;
                params.shAlfCrEnabledFlag   = sliceParams->m_longSliceFlags.m_fields.m_shAlfCrEnabledFlag;
                params.shAlfCcCbEnabledFlag = sliceParams->m_longSliceFlags.m_fields.m_shAlfCcCbEnabledFlag;
                params.shAlfCcCrEnabledFlag = sliceParams->m_longSliceFlags.m_fields.m_shAlfCcCrEnabledFlag;
            }
        }

        params.shLmcsUsedFlag                 = sliceParams->m_longSliceFlags.m_fields.m_shLmcsUsedFlag;
        params.shExplicitScalingListUsedFlag  = sliceParams->m_longSliceFlags.m_fields.m_shExplicitScalingListUsedFlag;
        params.shCabacInitFlag                = sliceParams->m_longSliceFlags.m_fields.m_shCabacInitFlag;
        params.shCollocatedFromL0Flag         = sliceParams->m_longSliceFlags.m_fields.m_shCollocatedFromL0Flag;
        params.shCuChromaQpOffsetEnabledFlag  = sliceParams->m_longSliceFlags.m_fields.m_shCuChromaQpOffsetEnabledFlag;
        params.shSaoLumaUsedFlag              = sliceParams->m_longSliceFlags.m_fields.m_shSaoLumaUsedFlag;
        params.shSaoChromaUsedFlag            = sliceParams->m_longSliceFlags.m_fields.m_shSaoChromaUsedFlag;
        params.shDeblockingFilterDisabledFlag = sliceParams->m_longSliceFlags.m_fields.m_shDeblockingFilterDisabledFlag;
        params.shDepQuantUsedFlag             = sliceParams->m_longSliceFlags.m_fields.m_shDepQuantUsedFlag;
        params.shSignDataHidingUsedFlag       = sliceParams->m_longSliceFlags.m_fields.m_shSignDataHidingUsedFlag;
        params.shTsResidualCodingDisabledFlag = sliceParams->m_longSliceFlags.m_fields.m_shTsResidualCodingDisabledFlag;
        params.nobackwardpredflag             = sliceParams->m_longSliceFlags.m_fields.m_noBackwardPredFlag;

        // CRC enable
        params.pVvcpDebugEnable = false;

        if (picParams->m_ppsFlags.m_fields.m_ppsRectSliceFlag)
        {
            params.dMultipleSlicesInTileFlag    = m_sliceDesc->m_multiSlicesInTileFlag;
            params.dIsbottommostsliceoftileFlag = m_sliceDesc->m_bottomSliceInTileFlag;
            params.dIstopmostsliceoftileFlag    = m_sliceDesc->m_topSliceInTileFlag;
        }

        // Partition info of SubPic v.s. Slice
        if (picParams->m_spsFlags0.m_fields.m_spsSubpicInfoPresentFlag && picParams->m_spsNumSubpicsMinus1 > 0)
        {
            params.dSubpicTreatedAsPicFlag            = m_subPicParams->m_subPicFlags.m_fields.m_spsSubpicTreatedAsPicFlag;
            params.dLoopFilterAcrossSubpicEnabledFlag = m_subPicParams->m_subPicFlags.m_fields.m_spsLoopFilterAcrossSubpicEnabledFlag;
            params.dIsRightMostSliceOfSubpicFlag      = (m_sliceDesc->m_sliceEndCtbx == m_subPicParams->m_endCtbX) ? 1 : 0;
            params.dIsLeftMostSliceOfSubpicFlag       = (m_sliceDesc->m_sliceStartCtbx == m_subPicParams->m_spsSubpicCtuTopLeftX) ? 1 : 0;
            params.dIsBottomMostSliceOfSubpicFlag     = (m_sliceDesc->m_sliceEndCtby == m_subPicParams->m_endCtbY) ? 1 : 0;
            params.dIsTopMostSliceOfSubpicFlag        = (m_sliceDesc->m_sliceStartCtby == m_subPicParams->m_spsSubpicCtuTopLeftY) ? 1 : 0;
        }
        else if (picParams->m_ppsFlags.m_fields.m_ppsRectSliceFlag)
        {
            params.dSubpicTreatedAsPicFlag            = (picParams->m_spsFlags0.m_fields.m_spsSubpicInfoPresentFlag && picParams->m_spsNumSubpicsMinus1 == 0) ? 1 : 0;
            params.dLoopFilterAcrossSubpicEnabledFlag = 0;
            params.dIsRightMostSliceOfSubpicFlag      = (m_sliceDesc->m_sliceEndCtbx == m_vvcBasicFeature->m_picWidthInCtu - 1) ? 1 : 0;
            params.dIsLeftMostSliceOfSubpicFlag       = (m_sliceDesc->m_sliceStartCtbx == 0) ? 1 : 0;
            params.dIsBottomMostSliceOfSubpicFlag     = (m_sliceDesc->m_sliceEndCtby == m_vvcBasicFeature->m_picHeightInCtu - 1) ? 1 : 0;
            params.dIsTopMostSliceOfSubpicFlag        = (m_sliceDesc->m_sliceStartCtby == 0) ? 1 : 0;
        }
        else
        {
            uint16_t startSlice = sliceParams->m_shSliceAddress;
            uint16_t endSlice   = sliceParams->m_shSliceAddress + sliceParams->m_shNumTilesInSliceMinus1;
            uint16_t startTileX = startSlice % m_vvcBasicFeature->m_tileCols;
            uint16_t endTileX   = endSlice % m_vvcBasicFeature->m_tileCols;

            params.dSubpicTreatedAsPicFlag             = 0;
            params.dLoopFilterAcrossSubpicEnabledFlag  = 0;
            params.dIsRightMostSliceOfSubpicFlag       = (startTileX + sliceParams->m_shNumTilesInSliceMinus1 < m_vvcBasicFeature->m_tileCols - 1) ? 0 : 1;
            params.dIsLeftMostSliceOfSubpicFlag        = (endTileX > sliceParams->m_shNumTilesInSliceMinus1) ? 0 : 1;
            params.dIsBottomMostSliceOfSubpicFlag      = (m_vvcBasicFeature->m_tileCols * m_vvcBasicFeature->m_tileRows - endSlice > m_vvcBasicFeature->m_tileCols) ? 0 : 1;
            params.dIsTopMostSliceOfSubpicFlag         = (startSlice < m_vvcBasicFeature->m_tileCols) ? 1 : 0;
        }

        params.dLastsliceofpicFlag     = m_lastSliceOfPic;
        params.numctusincurrslice      = m_sliceDesc->m_numCtusInCurrSlice;
        params.shNumTilesInSliceMinus1 = (picParams->m_ppsFlags.m_fields.m_ppsRectSliceFlag) ? 0 : sliceParams->m_shNumTilesInSliceMinus1;
        params.shSliceType             = sliceParams->m_shSliceType;

        if (params.shAlfEnabledFlag)
        {
            params.shNumAlfApsIdsLuma = sliceParams->m_shNumAlfApsIdsLuma;
            if (params.shAlfCbEnabledFlag || params.shAlfCrEnabledFlag)
            {
                MHW_CHK_COND(sliceParams->m_shAlfApsIdChroma >= vvcMaxAlfNum, "sh_alf_aps_id_chroma out of range!");
                MHW_CHK_COND(m_vvcBasicFeature->m_alfApsArray[sliceParams->m_shAlfApsIdChroma].m_alfFlags.m_fields.m_alfChromaFilterSignalFlag != 1, "alf_chroma_filter_signal_flag should be equal to 1!");
                params.alfChromaNumAltFiltersMinus1 = m_vvcBasicFeature->m_alfApsArray[sliceParams->m_shAlfApsIdChroma].m_alfChromaNumAltFiltersMinus1;
            }
            if (params.shAlfCcCbEnabledFlag)
            {
                MHW_CHK_COND(sliceParams->m_shAlfCcCbApsId >= vvcMaxAlfNum, "sh_alf_cc_cb_aps_id out of range!");
                MHW_CHK_COND(m_vvcBasicFeature->m_alfApsArray[sliceParams->m_shAlfCcCbApsId].m_alfFlags.m_fields.m_alfCcCbFilterSignalFlag != 1, "alf_cc_cb_filter_signal_flag should be equal to 1!");
                params.alfCcCbFiltersSignalledMinus1 = m_vvcBasicFeature->m_alfApsArray[sliceParams->m_shAlfCcCbApsId].m_alfCcCbFiltersSignalledMinus1;
            }
            if (params.shAlfCcCrEnabledFlag)
            {
                MHW_CHK_COND(sliceParams->m_shAlfCcCrApsId >= vvcMaxAlfNum, "sh_alf_cc_cr_aps_id out of range!");
                MHW_CHK_COND(m_vvcBasicFeature->m_alfApsArray[sliceParams->m_shAlfCcCrApsId].m_alfFlags.m_fields.m_alfCcCrFilterSignalFlag != 1, "alf_cc_cr_filter_signal_flag should be equal to 1!");
                params.alfCcCrFiltersSignalledMinus1 = m_vvcBasicFeature->m_alfApsArray[sliceParams->m_shAlfCcCrApsId].m_alfCcCrFiltersSignalledMinus1;
            }
            for (auto i = 0; i < sliceParams->m_shNumAlfApsIdsLuma; i++)
            {
                MHW_CHK_COND(sliceParams->m_shAlfApsIdLuma[i] >= vvcMaxAlfNum, "sh_alf_aps_id_luma[] out of range!");
                MHW_CHK_COND(m_vvcBasicFeature->m_alfApsArray[sliceParams->m_shAlfApsIdLuma[i]].m_alfFlags.m_fields.m_alfLumaFilterSignalFlag != 1, "alf_luma_filter_signal_flag should be equal to 1!");
            }
            params.shAlfApsIdLuma0  = sliceParams->m_shAlfApsIdLuma[0];
            params.shAlfApsIdLuma1  = sliceParams->m_shAlfApsIdLuma[1];
            params.shAlfApsIdLuma2  = sliceParams->m_shAlfApsIdLuma[2];
            params.shAlfApsIdLuma3  = sliceParams->m_shAlfApsIdLuma[3];
            params.shAlfApsIdLuma4  = sliceParams->m_shAlfApsIdLuma[4];
            params.shAlfApsIdLuma5  = sliceParams->m_shAlfApsIdLuma[5];
            params.shAlfApsIdLuma6  = sliceParams->m_shAlfApsIdLuma[6];
            params.shAlfApsIdChroma = (params.shAlfCbEnabledFlag || params.shAlfCrEnabledFlag) ? sliceParams->m_shAlfApsIdChroma : 0;
            params.shAlfCcCbApsId   = params.shAlfCcCbEnabledFlag ? sliceParams->m_shAlfCcCbApsId : 0;
            params.shAlfCcCrApsId   = params.shAlfCcCrEnabledFlag ? sliceParams->m_shAlfCcCrApsId : 0;
        }

        if (!m_vvcpItf->IsVvcISlice(sliceParams->m_shSliceType))
        {
            params.numrefidxactive0 = sliceParams->m_numRefIdxActive[0];
            params.numrefidxactive1 = sliceParams->m_numRefIdxActive[1];
        }

        params.shCollocatedRefIdx  = sliceParams->m_shCollocatedRefIdx;
        params.sliceqpy            = sliceParams->m_sliceQpY;
        params.shCbQpOffset        = sliceParams->m_shCbQpOffset;
        params.shCrQpOffset        = sliceParams->m_shCrQpOffset;
        params.shJointCbcrQpOffset = sliceParams->m_shJointCbcrQpOffset;

        if (!sliceParams->m_longSliceFlags.m_fields.m_shDeblockingFilterDisabledFlag)
        {
            params.shLumaBetaOffsetDiv2 = sliceParams->m_shLumaBetaOffsetDiv2;
            params.shLumaTcOffsetDiv2   = sliceParams->m_shLumaTcOffsetDiv2;
            params.shCbBetaOffsetDiv2   = sliceParams->m_shCbBetaOffsetDiv2;
            params.shCbTcOffsetDiv2     = sliceParams->m_shCbTcOffsetDiv2;
            params.shCrBetaOffsetDiv2   = sliceParams->m_shCrBetaOffsetDiv2;
            params.shCrTcOffsetDiv2     = sliceParams->m_shCrTcOffsetDiv2;
        }

        // LMCS
        params.spsLmcsEnabledFlag = picParams->m_spsFlags0.m_fields.m_spsLmcsEnabledFlag;

        if (params.spsLmcsEnabledFlag && params.shLmcsUsedFlag)
        {
            params.phLmcsApsId      = m_vvcPicParams->m_phLmcsApsId;
            params.vvcLmcsData      = m_vvcBasicFeature->m_lmcsApsArray;
            params.vvcLmcsShapeInfo = m_vvcBasicFeature->m_lmcsReshaperInfo;
        }

        // Partition info of SubPic v.s. Slice
        if (picParams->m_ppsFlags.m_fields.m_ppsRectSliceFlag &&
            picParams->m_spsFlags0.m_fields.m_spsSubpicInfoPresentFlag &&
            picParams->m_spsNumSubpicsMinus1 > 0)
        {
            params.dSubpicCtuTopLeftX  = m_subPicParams->m_spsSubpicCtuTopLeftX;
            params.dSubpicCtuTopLeftY  = m_subPicParams->m_spsSubpicCtuTopLeftY;
            params.dSubpicWidthMinus1  = m_subPicParams->m_spsSubpicWidthMinus1;
            params.dSubpicHeightMinus1 = m_subPicParams->m_spsSubpicHeightMinus1;
        }
        else
        {
            params.dSubpicCtuTopLeftX  = 0;
            params.dSubpicCtuTopLeftY  = 0;
            params.dSubpicWidthMinus1  = MOS_ROUNDUP_SHIFT(picParams->m_ppsPicWidthInLumaSamples, picParams->m_spsLog2CtuSizeMinus5 + 5) - 1;
            params.dSubpicHeightMinus1 = MOS_ROUNDUP_SHIFT(picParams->m_ppsPicHeightInLumaSamples, picParams->m_spsLog2CtuSizeMinus5 + 5) - 1;
        }

        if (picParams->m_ppsFlags.m_fields.m_ppsRectSliceFlag)
        {
            if (m_sliceDesc->m_sliceWidthInTiles <= 1 && m_sliceDesc->m_sliceHeightInTiles <= 1)
            {
                params.dSliceheightinctus = m_sliceDesc->m_sliceHeightInCtu;
            }
        }

        params.dToplefttilex   = m_sliceDesc->m_startTileX;
        params.dToplefttiley   = m_sliceDesc->m_startTileY;
        params.dSlicestartctbx = m_sliceDesc->m_sliceStartCtbx;
        params.dSlicestartctby = m_sliceDesc->m_sliceStartCtby;

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(VVCP_BSD_OBJECT, VvcDecodeSlicePkt)
    {
        params = {};

        uint32_t bsdDataLength = m_curSliceParams->m_sliceBytesInBuffer - m_curSliceParams->m_byteOffsetToSliceData;
        if (bsdDataLength != 0)
        {
            params.bsdDataLength      = bsdDataLength;
            params.bsdDataStartOffset = m_curSliceParams->m_bSNALunitDataLocation + m_curSliceParams->m_byteOffsetToSliceData;
        }
        else
        {
            params.bsdDataLength      = 4;
            params.bsdDataStartOffset = 0;
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS VvcDecodeSlicePkt::AddAllCmds_VVCP_TILE_CODING(MOS_COMMAND_BUFFER &cmdBuffer)
    {
        DECODE_FUNC_CALL();

        m_curTileIdx = 0;
        if (!m_vvcPicParams->m_ppsFlags.m_fields.m_ppsRectSliceFlag)
        {
            m_numTilesInSlice = m_curSliceParams->m_shNumTilesInSliceMinus1 + 1;
            for (int16_t i = 0; i < m_numTilesInSlice; i++)
            {
                m_curTileIdx = m_curSliceParams->m_shSliceAddress + i;
                SETPAR_AND_ADDCMD(VVCP_TILE_CODING, m_vvcpItf, &cmdBuffer);
            }
        }
        else
        {
            if (m_sliceDesc->m_multiSlicesInTileFlag)
            {
                m_numTilesInSlice = 1;
                m_curTileIdx      = m_sliceDesc->m_tileIdx;
                SETPAR_AND_ADDCMD(VVCP_TILE_CODING, m_vvcpItf, &cmdBuffer);
            }
            else
            {
                m_numTilesInSlice = m_sliceDesc->m_sliceHeightInTiles * m_sliceDesc->m_sliceWidthInTiles;
                for (int16_t y = 0; y < m_sliceDesc->m_sliceHeightInTiles; y++)
                {
                    for (int16_t x = 0; x < m_sliceDesc->m_sliceWidthInTiles; x++)
                    {
                        m_curTileIdx = (x + m_sliceDesc->m_startTileX) + (y + m_sliceDesc->m_startTileY) * m_vvcBasicFeature->m_tileCols;
                        SETPAR_AND_ADDCMD(VVCP_TILE_CODING, m_vvcpItf, &cmdBuffer);
                    }
                }
            }
        }

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(VVCP_TILE_CODING, VvcDecodeSlicePkt)
    {
        params = {};

        uint16_t col                     = m_curTileIdx % m_vvcBasicFeature->m_tileCols;
        uint16_t row                     = m_curTileIdx / m_vvcBasicFeature->m_tileCols;
        params.tilecolbdval              = m_vvcBasicFeature->m_tileCol[col].m_startCtbX;
        params.tilerowbdval              = m_vvcBasicFeature->m_tileRow[row].m_startCtbY;
        params.colwidthval               = m_vvcBasicFeature->m_tileCol[col].m_widthInCtb;
        params.rowheightval              = m_vvcBasicFeature->m_tileRow[row].m_heightInCtb;
        params.currenttilecolumnposition = col;
        params.currenttilerowposition    = row;

        //Tile v.s. Slice
        if (m_vvcBasicFeature->m_vvcPicParams->m_ppsFlags.m_fields.m_ppsRectSliceFlag)
        {
            DECODE_ASSERT(m_sliceDesc);
            if (m_sliceDesc->m_multiSlicesInTileFlag)
            {
                params.flags.m_isrightmosttileofsliceFlag  = 1;
                params.flags.m_isleftmosttileofsliceFlag   = 1;
                params.flags.m_isbottommosttileofsliceFlag = 1;
                params.flags.m_istopmosttileofsliceFlag    = 1;
            }
            else
            {
                params.flags.m_isrightmosttileofsliceFlag  = (col == m_sliceDesc->m_startTileX + m_sliceDesc->m_sliceWidthInTiles - 1) ? 1 : 0;
                params.flags.m_isleftmosttileofsliceFlag   = (col == m_sliceDesc->m_startTileX) ? 1 : 0;
                params.flags.m_isbottommosttileofsliceFlag = (row == m_sliceDesc->m_startTileY + m_sliceDesc->m_sliceHeightInTiles - 1) ? 1 : 0;
                params.flags.m_istopmosttileofsliceFlag    = (row == m_sliceDesc->m_startTileY) ? 1 : 0;
            }
        }
        else
        {
            if (col == m_vvcBasicFeature->m_tileCols - 1)  // frame right most
            {
                params.flags.m_isrightmosttileofsliceFlag = 1;
            }
            else
            {
                params.flags.m_isrightmosttileofsliceFlag = !IsTileInRasterSlice(row, col + 1);
            }

            if (col == 0)  // frame left most
            {
                params.flags.m_isleftmosttileofsliceFlag = 1;
            }
            else
            {
                params.flags.m_isleftmosttileofsliceFlag = !IsTileInRasterSlice(row, col - 1);
            }

            if (row == m_vvcBasicFeature->m_tileRows - 1)  // frame bottom most
            {
                params.flags.m_isbottommosttileofsliceFlag = 1;
            }
            else
            {
                params.flags.m_isbottommosttileofsliceFlag = !IsTileInRasterSlice(row + 1, col);
            }

            if (row == 0)  // frame top most
            {
                params.flags.m_istopmosttileofsliceFlag = 1;
            }
            else
            {
                params.flags.m_istopmosttileofsliceFlag = !IsTileInRasterSlice(row - 1, col);
            }
        }

        //tile v.s. frame
        params.flags.m_isrightmosttileofframeFlag  = (col == m_vvcBasicFeature->m_tileCols - 1) ? 1 : 0;
        params.flags.m_isleftmosttileofframeFlag   = (col == 0) ? 1 : 0;
        params.flags.m_isbottommosttileofframeFlag = (row == m_vvcBasicFeature->m_tileRows - 1) ? 1 : 0;
        params.flags.m_istopmosttileofframeFlag    = (row == 0) ? 1 : 0;

        m_vvcBasicFeature->m_frameCompletedFlag = (params.flags.m_isrightmosttileofframeFlag == 1) && (params.flags.m_isbottommosttileofframeFlag == 1);
    
        return MOS_STATUS_SUCCESS;
    }

}

