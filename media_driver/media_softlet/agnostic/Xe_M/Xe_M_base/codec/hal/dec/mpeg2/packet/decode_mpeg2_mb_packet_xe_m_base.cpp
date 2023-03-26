/*
* Copyright (c) 2021, Intel Corporation
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
//! \file     decode_mpeg2_mb_packet_xe_m_base.cpp
//! \brief    Defines the interface of mpeg2 decode macroblock packet for Xe_M_Base
//!
#include "codechal_utilities.h"
#include "decode_mpeg2_mb_packet_xe_m_base.h"

namespace decode {

    MOS_STATUS Mpeg2DecodeMbPktXe_M_Base::Init()
    {
        DECODE_FUNC_CALL();

        DECODE_CHK_NULL(m_featureManager);
        DECODE_CHK_NULL(m_hwInterface);
        DECODE_CHK_NULL(m_osInterface);
        DECODE_CHK_NULL(m_miInterface);
        DECODE_CHK_NULL(m_mpeg2Pipeline);
        DECODE_CHK_NULL(m_mfxInterface);

        m_mpeg2BasicFeature = dynamic_cast<Mpeg2BasicFeature*>(m_featureManager->GetFeature(FeatureIDs::basicFeature));
        DECODE_CHK_NULL(m_mpeg2BasicFeature);

        m_allocator = m_pipeline->GetDecodeAllocator();
        DECODE_CHK_NULL(m_allocator);

        DECODE_CHK_STATUS(CalculateMbStateCommandSize());

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Mpeg2DecodeMbPktXe_M_Base::Prepare()
    {
        DECODE_FUNC_CALL();

        DECODE_CHK_NULL(m_mpeg2BasicFeature->m_mpeg2PicParams);
        m_mpeg2PicParams = m_mpeg2BasicFeature->m_mpeg2PicParams;

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Mpeg2DecodeMbPktXe_M_Base::SetMpeg2MbStateParams(
        MHW_VDBOX_MPEG2_MB_STATE& mpeg2MbState, uint32_t mbIdx)
    {
        DECODE_FUNC_CALL();

        MOS_ZeroMemory(&mpeg2MbState, sizeof(mpeg2MbState));
        CodecDecodeMpeg2MbParams* mb = &m_mpeg2BasicFeature->m_mbRecord[mbIdx].recordMbParam;

        mpeg2MbState.wPicWidthInMb = m_mpeg2BasicFeature->m_picWidthInMb;
        mpeg2MbState.wPicHeightInMb = m_mpeg2BasicFeature->m_picHeightInMb;
        mpeg2MbState.wPicCodingType = (uint16_t)m_mpeg2PicParams->m_pictureCodingType;

        // common field for MBs in I picture and PB picture .
        mpeg2MbState.pMBParams = mb;
        mpeg2MbState.dwDCTLength = 0;

        for (uint32_t i = 0; i < CODEC_NUM_BLOCK_PER_MB; i++)
        {
            mpeg2MbState.dwDCTLength += mb->m_numCoeff[i];
        }

        mpeg2MbState.dwITCoffDataAddrOffset = mb->m_mbDataLoc << 2;  // byte offset

        // only for MB in PB picture.
        if (mpeg2MbState.wPicCodingType != I_TYPE)
        {
            bool intraMB = mpeg2MbState.pMBParams->MBType.m_intraMb ? true : false;

            MOS_ZeroMemory(mpeg2MbState.sPackedMVs0, sizeof(mpeg2MbState.sPackedMVs0));
            MOS_ZeroMemory(mpeg2MbState.sPackedMVs1, sizeof(mpeg2MbState.sPackedMVs1));
            if ((!intraMB) && (mpeg2MbState.pMBParams->MBType.m_value &
                (CODEC_MPEG2_MB_MOTION_BACKWARD | CODEC_MPEG2_MB_MOTION_FORWARD)))
            {
                PackMotionVectors(m_mpeg2PicParams->m_currPic.PicFlags, &mpeg2MbState);
            }
        }

        return MOS_STATUS_SUCCESS;
    }

    void Mpeg2DecodeMbPktXe_M_Base::PackMotionVectors(CODEC_PICTURE_FLAG pic_flag, PMHW_VDBOX_MPEG2_MB_STATE mpeg2MbState)
    {
        CodecDecodeMpeg2MbParams* mbParams = mpeg2MbState->pMBParams;

        uint16_t motionType = mbParams->MBType.m_motionType;
        uint16_t intelMotionType = Mpeg2ImtNone;

        // convert to Intel Motion Type
        if (pic_flag == PICTURE_FRAME)
        {
            switch (motionType)
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
            switch (motionType)
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

        int16_t* mv = mbParams->m_motionVectors;

        switch (intelMotionType)
        {
        case Mpeg2Imt16x8:
        case Mpeg2ImtFieldField:
        case Mpeg2ImtFrameFrame:
        case Mpeg2ImtFieldDualPrime:
            mpeg2MbState->sPackedMVs0[0] = (short)mv[CodechalDecodeRstFirstForwHorz];
            mpeg2MbState->sPackedMVs0[1] = (short)mv[CodechalDecodeRstFirstForwVert];
            mpeg2MbState->sPackedMVs0[2] = (short)mv[CodechalDecodeRstFirstBackHorz];
            mpeg2MbState->sPackedMVs0[3] = (short)mv[CodechalDecodeRstFirstBackVert];
            break;

        case Mpeg2ImtFrameFiled:
        case Mpeg2ImtFrameDualPrime:
            mpeg2MbState->sPackedMVs0[0] = (short)mv[CodechalDecodeRstFirstForwHorz];
            mpeg2MbState->sPackedMVs0[1] = (short)(mv[CodechalDecodeRstFirstForwVert] >> 1);
            mpeg2MbState->sPackedMVs0[2] = (short)mv[CodechalDecodeRstFirstBackHorz];
            mpeg2MbState->sPackedMVs0[3] = (short)(mv[CodechalDecodeRstFirstBackVert] >> 1);
            break;

        default:
            break;
        }

        switch (intelMotionType)
        {
        case Mpeg2Imt16x8:
            mpeg2MbState->sPackedMVs1[0] = (short)mv[CodechalDecodeRstSecndForwHorz];
            mpeg2MbState->sPackedMVs1[1] = (short)mv[CodechalDecodeRstSecndForwVert];
            mpeg2MbState->sPackedMVs1[2] = (short)mv[CodechalDecodeRstSecndBackHorz];
            mpeg2MbState->sPackedMVs1[3] = (short)mv[CodechalDecodeRstSecndBackVert];
            break;

        case Mpeg2ImtFrameDualPrime:
            mpeg2MbState->sPackedMVs1[0] = (short)mv[CodechalDecodeRstFirstForwHorz];
            mpeg2MbState->sPackedMVs1[1] = (short)(mv[CodechalDecodeRstFirstForwVert] >> 1);
            mpeg2MbState->sPackedMVs1[2] = (short)mv[CodechalDecodeRstSecndBackHorz];
            mpeg2MbState->sPackedMVs1[3] = (short)(mv[CodechalDecodeRstSecndBackVert] >> 1);
            break;

        case Mpeg2ImtFrameFiled:
            mpeg2MbState->sPackedMVs1[0] = (short)mv[CodechalDecodeRstSecndForwHorz];
            mpeg2MbState->sPackedMVs1[1] = (short)(mv[CodechalDecodeRstSecndForwVert] >> 1);
            mpeg2MbState->sPackedMVs1[2] = (short)mv[CodechalDecodeRstSecndBackHorz];
            mpeg2MbState->sPackedMVs1[3] = (short)(mv[CodechalDecodeRstSecndBackVert] >> 1);
            break;

        default:
            break;
        }
    }

    MOS_STATUS Mpeg2DecodeMbPktXe_M_Base::AddITObj(MHW_BATCH_BUFFER& batchBuffer, uint32_t mbIdx)
    {
        DECODE_FUNC_CALL();

        MHW_VDBOX_MPEG2_MB_STATE mpeg2MbState;
        DECODE_CHK_STATUS(SetMpeg2MbStateParams(mpeg2MbState, mbIdx));
        DECODE_CHK_STATUS(m_mfxInterface->AddMfdMpeg2ITObject(
            nullptr,
            &batchBuffer,
            &mpeg2MbState));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Mpeg2DecodeMbPktXe_M_Base::InsertSkippedMacroblocks(
        MHW_BATCH_BUFFER& batchBuffer,
        uint32_t mbIdx,
        uint16_t nextMBStart,
        uint16_t skippedMBs)
    {
        DECODE_FUNC_CALL();

        MHW_VDBOX_MPEG2_MB_STATE    mpeg2MbState;
        MOS_ZeroMemory(&mpeg2MbState, sizeof(mpeg2MbState));
        mpeg2MbState.wPicWidthInMb = m_mpeg2BasicFeature->m_picWidthInMb;
        mpeg2MbState.wPicHeightInMb = m_mpeg2BasicFeature->m_picHeightInMb;
        mpeg2MbState.wPicCodingType = (uint16_t)m_mpeg2PicParams->m_pictureCodingType;

        // insert skipped Macroblocks with the first available MB params
        mpeg2MbState.pMBParams = &(m_mpeg2BasicFeature->m_mbRecord[mbIdx].recordMbParam);

        // save the original MB params, and restore the orignal MB params when function exit.
        CodechalDecodeRestoreData<CodecDecodeMpeg2MbParams> MBParamsRestore(mpeg2MbState.pMBParams);

        mpeg2MbState.dwDCTLength = 0;
        mpeg2MbState.dwITCoffDataAddrOffset = 0;
        mpeg2MbState.pMBParams->m_codedBlockPattern = 0;

        MOS_ZeroMemory(mpeg2MbState.sPackedMVs0, sizeof(mpeg2MbState.sPackedMVs0));
        MOS_ZeroMemory(mpeg2MbState.sPackedMVs1, sizeof(mpeg2MbState.sPackedMVs1));

        for (uint16_t i = 0; i < skippedMBs; i++)
        {
            mpeg2MbState.pMBParams->m_mbAddr = nextMBStart + i;
            DECODE_CHK_STATUS(m_mfxInterface->AddMfdMpeg2ITObject(
                nullptr,
                &batchBuffer,
                &mpeg2MbState));
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Mpeg2DecodeMbPktXe_M_Base::CalculateCommandSize(
        uint32_t& commandBufferSize, uint32_t& requestedPatchListSize)
    {
        DECODE_FUNC_CALL();

        commandBufferSize = m_mbStatesSize;
        requestedPatchListSize = m_mbPatchListSize;

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Mpeg2DecodeMbPktXe_M_Base::CalculateMbStateCommandSize()
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

}  // namespace decode
