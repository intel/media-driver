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
//! \file     decode_mpeg2_mb_packet.cpp
//! \brief    Defines the interface for mpeg2 decode macroblock packet
//!
#include "decode_mpeg2_mb_packet.h"

namespace decode{

MOS_STATUS Mpeg2DecodeMbPkt::Init()
{
    DECODE_FUNC_CALL();

    DECODE_CHK_NULL(m_featureManager);
    DECODE_CHK_NULL(m_hwInterface);
    DECODE_CHK_NULL(m_osInterface);
    DECODE_CHK_NULL(m_miItf);
    DECODE_CHK_NULL(m_mpeg2Pipeline);
    DECODE_CHK_NULL(m_mfxItf);

    m_mpeg2BasicFeature = dynamic_cast<Mpeg2BasicFeature*>(m_featureManager->GetFeature(FeatureIDs::basicFeature));
    DECODE_CHK_NULL(m_mpeg2BasicFeature);

    m_allocator = m_pipeline ->GetDecodeAllocator();
    DECODE_CHK_NULL(m_allocator);

    DECODE_CHK_STATUS(CalculateMbStateCommandSize());

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Mpeg2DecodeMbPkt::Prepare()
{
    DECODE_FUNC_CALL();

    DECODE_CHK_NULL(m_mpeg2BasicFeature->m_mpeg2PicParams);
    m_mpeg2PicParams   = m_mpeg2BasicFeature->m_mpeg2PicParams;

    return MOS_STATUS_SUCCESS;
}

void Mpeg2DecodeMbPkt::PackMotionVectors(CODEC_PICTURE_FLAG pic_flag, uint32_t mbIdx, int16_t sPackedMVs0[], int16_t sPackedMVs1[])
{
    CodecDecodeMpeg2MbParams *mbParams = &m_mpeg2BasicFeature->m_mbRecord[mbIdx].recordMbParam;

    uint16_t motionType = mbParams->MBType.m_motionType;
    uint16_t intelMotionType = Mpeg2ImtNone;

    // convert to Intel Motion Type
    if (pic_flag == PICTURE_FRAME)
    {
        switch(motionType)
        {
        case CodechalDecodeMcFrame:
            intelMotionType = Mpeg2ImtFrameFrame;
            break;
        case CodechalDecodeMcField:
            intelMotionType = Mpeg2ImtFrameFiled;
            break;
        case CodechalDecodeMcDmv:
            intelMotionType = Mpeg2ImtFrameDualPrime;
            break;
        default:
            break;
        }
    }
    else // must be field picture
    {
        switch(motionType)
        {
        case CodechalDecodeMcField:
            intelMotionType = Mpeg2ImtFieldField;
            break;
        case CodechalDecodeMcDmv:
            intelMotionType = Mpeg2ImtFieldDualPrime;
            break;
        case CodechalDecodeMc16x8:
            intelMotionType = Mpeg2Imt16x8;
            break;
        default:
            break;
        }
    }

    int16_t *mv = mbParams->m_motionVectors;

    switch (intelMotionType)
    {
    case Mpeg2Imt16x8:
    case Mpeg2ImtFieldField:
    case Mpeg2ImtFrameFrame:
    case Mpeg2ImtFieldDualPrime:
        sPackedMVs0[0] = (short)mv[CodechalDecodeRstFirstForwHorz];
        sPackedMVs0[1] = (short)mv[CodechalDecodeRstFirstForwVert];
        sPackedMVs0[2] = (short)mv[CodechalDecodeRstFirstBackHorz];
        sPackedMVs0[3] = (short)mv[CodechalDecodeRstFirstBackVert];
        break;

    case Mpeg2ImtFrameFiled:
    case Mpeg2ImtFrameDualPrime:
        sPackedMVs0[0] = (short)mv[CodechalDecodeRstFirstForwHorz];
        sPackedMVs0[1] = (short)(mv[CodechalDecodeRstFirstForwVert] >> 1);
        sPackedMVs0[2] = (short)mv[CodechalDecodeRstFirstBackHorz];
        sPackedMVs0[3] = (short)(mv[CodechalDecodeRstFirstBackVert] >> 1);
        break;

    default:
        break;
    }

    switch (intelMotionType)
    {
    case Mpeg2Imt16x8:
        sPackedMVs1[0] = (short)mv[CodechalDecodeRstSecndForwHorz];
        sPackedMVs1[1] = (short)mv[CodechalDecodeRstSecndForwVert];
        sPackedMVs1[2] = (short)mv[CodechalDecodeRstSecndBackHorz];
        sPackedMVs1[3] = (short)mv[CodechalDecodeRstSecndBackVert];
        break;

    case Mpeg2ImtFrameDualPrime:
        sPackedMVs1[0] = (short)mv[CodechalDecodeRstFirstForwHorz];
        sPackedMVs1[1] = (short)(mv[CodechalDecodeRstFirstForwVert] >> 1);
        sPackedMVs1[2] = (short)mv[CodechalDecodeRstSecndBackHorz];
        sPackedMVs1[3] = (short)(mv[CodechalDecodeRstSecndBackVert] >> 1);
        break;

    case Mpeg2ImtFrameFiled:
        sPackedMVs1[0] = (short)mv[CodechalDecodeRstSecndForwHorz];
        sPackedMVs1[1] = (short)(mv[CodechalDecodeRstSecndForwVert] >> 1);
        sPackedMVs1[2] = (short)mv[CodechalDecodeRstSecndBackHorz];
        sPackedMVs1[3] = (short)(mv[CodechalDecodeRstSecndBackVert] >> 1);
        break;

    default:
        break;
    }
}

MOS_STATUS Mpeg2DecodeMbPkt::AddCmd_MFD_MPEG2_IT_OBJECT(MHW_BATCH_BUFFER &batchBuffer, uint32_t mbIdx)
{
    DECODE_FUNC_CALL();

    auto mbParams = &m_mpeg2BasicFeature->m_mbRecord[mbIdx].recordMbParam;
    MHW_MI_CHK_NULL(mbParams);

    auto &inlinePar = m_mfxItf->MHW_GETPAR_F(MFD_IT_OBJECT_MPEG2_INLINE_DATA)();
    auto &headerPar = m_mfxItf->MHW_GETPAR_F(MFD_IT_OBJECT)();
    inlinePar       = {};
    headerPar       = {};

    MOS_ZeroMemory(inlinePar.sPackedMVs0, sizeof(inlinePar.sPackedMVs0));
    MOS_ZeroMemory(inlinePar.sPackedMVs1, sizeof(inlinePar.sPackedMVs1));

    // common field for MBs in I picture and PB picture .
    uint32_t dwDCTLength = 0;
    for (uint32_t i = 0; i < CODEC_NUM_BLOCK_PER_MB; i++)
    {
        dwDCTLength += mbParams->m_numCoeff[i];
    }

    // only for MB in PB picture.
    if (m_mpeg2PicParams->m_pictureCodingType != I_TYPE)
    {
        bool intraMB = mbParams->MBType.m_intraMb ? true : false;

        if ((!intraMB) && (mbParams->MBType.m_value & (CODEC_MPEG2_MB_MOTION_BACKWARD | CODEC_MPEG2_MB_MOTION_FORWARD)))
        {
            PackMotionVectors(m_mpeg2PicParams->m_currPic.PicFlags, mbIdx, inlinePar.sPackedMVs0, inlinePar.sPackedMVs1);
        }
    }

    headerPar.DwordLength = ((m_mfxItf->MHW_GETSIZE_F(MFD_IT_OBJECT_MPEG2_INLINE_DATA)() +
                                m_mfxItf->MHW_GETSIZE_F(MFD_IT_OBJECT)()) >> 2) - 2;
    headerPar.dwDCTLength = dwDCTLength;
    headerPar.IndirectItCoeffDataStartAddressOffset = mbParams->m_mbDataLoc << 2;

    inlinePar.CodingType        = m_mpeg2PicParams->m_pictureCodingType;
    inlinePar.pMBParams         = mbParams;
    inlinePar.CodedBlockPattern = mbParams->m_codedBlockPattern;
    inlinePar.Horzorigin        = mbParams->m_mbAddr % m_mpeg2BasicFeature->m_picWidthInMb;
    inlinePar.Vertorigin        = mbParams->m_mbAddr / m_mpeg2BasicFeature->m_picWidthInMb;
    inlinePar.Lastmbinrow       = (inlinePar.Horzorigin == (m_mpeg2BasicFeature->m_picWidthInMb - 1));

    DECODE_CHK_STATUS(m_mfxItf->MHW_ADDCMD_F(MFD_IT_OBJECT)(nullptr, &batchBuffer));
    DECODE_CHK_STATUS(m_mfxItf->MHW_ADDCMD_F(MFD_IT_OBJECT_MPEG2_INLINE_DATA)(nullptr, &batchBuffer));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Mpeg2DecodeMbPkt::AddAllCmdsInsertSkippedMacroblocks(MHW_BATCH_BUFFER &batchBuffer, uint32_t mbIdx, uint16_t nextMBStart, uint16_t skippedMBs)
{
    DECODE_FUNC_CALL();

    // insert skipped Macroblocks with the first available MB params
    auto mbParams = &(m_mpeg2BasicFeature->m_mbRecord[mbIdx].recordMbParam);
    MHW_MI_CHK_NULL(mbParams);

    // save the original MB params, and restore the orignal MB params when function exit.
    CodechalDecodeRestoreData<CodecDecodeMpeg2MbParams> MBParamsRestore(mbParams);

    auto &inlinePar = m_mfxItf->MHW_GETPAR_F(MFD_IT_OBJECT_MPEG2_INLINE_DATA)();
    auto &headerPar = m_mfxItf->MHW_GETPAR_F(MFD_IT_OBJECT)();
    inlinePar       = {};
    headerPar       = {};

    headerPar.DwordLength = ((m_mfxItf->MHW_GETSIZE_F(MFD_IT_OBJECT_MPEG2_INLINE_DATA)() +
                                m_mfxItf->MHW_GETSIZE_F(MFD_IT_OBJECT)()) >> 2) - 2;

    headerPar.dwDCTLength = 0;

    inlinePar.CodingType        = m_mpeg2PicParams->m_pictureCodingType;
    inlinePar.pMBParams         = mbParams;
    inlinePar.CodedBlockPattern = 0;

    MOS_ZeroMemory(inlinePar.sPackedMVs0, sizeof(inlinePar.sPackedMVs0));
    MOS_ZeroMemory(inlinePar.sPackedMVs1, sizeof(inlinePar.sPackedMVs1));

    for (uint16_t i = 0; i < skippedMBs; i++)
    {
        mbParams->m_mbAddr = nextMBStart + i;

        inlinePar.Horzorigin        = mbParams->m_mbAddr % m_mpeg2BasicFeature->m_picWidthInMb;
        inlinePar.Vertorigin        = mbParams->m_mbAddr / m_mpeg2BasicFeature->m_picWidthInMb;
        inlinePar.Lastmbinrow       = (inlinePar.Horzorigin == (m_mpeg2BasicFeature->m_picWidthInMb - 1));

        DECODE_CHK_STATUS(m_mfxItf->MHW_ADDCMD_F(MFD_IT_OBJECT)(nullptr, &batchBuffer));
        DECODE_CHK_STATUS(m_mfxItf->MHW_ADDCMD_F(MFD_IT_OBJECT_MPEG2_INLINE_DATA)(nullptr, &batchBuffer));
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Mpeg2DecodeMbPkt::CalculateCommandSize(
    uint32_t &commandBufferSize, uint32_t &requestedPatchListSize)
{
    DECODE_FUNC_CALL();

    commandBufferSize      = m_mbStatesSize;
    requestedPatchListSize = m_mbPatchListSize;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Mpeg2DecodeMbPkt::CalculateMbStateCommandSize()
{
    DECODE_FUNC_CALL();

    // MB Level Commands
    DECODE_CHK_STATUS(m_hwInterface->GetMfxPrimitiveCommandsDataSize(
        m_mpeg2BasicFeature->m_mode,
        &m_mbStatesSize,
        &m_mbPatchListSize,
        0));

    return MOS_STATUS_SUCCESS;
}

}
