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
//! \file     encode_av1_vdenc_lpla_enc.cpp
//! \brief    Implementation for encode av1 lowpower lookahead(Encode Pass) feature
//!

#include "encode_av1_vdenc_lpla_enc.h"
#include "encode_av1_vdenc_feature_manager.h"

namespace encode
{
    AV1VdencLplaEnc::AV1VdencLplaEnc(
        MediaFeatureManager *featureManager,
        EncodeAllocator     *allocator,
        CodechalHwInterfaceNext *hwInterface,
        void                *constSettings) :
        MediaFeature(constSettings),
        m_allocator(allocator)
    {
    }

    AV1VdencLplaEnc::~AV1VdencLplaEnc()
    {
        if (m_lplaHelper)
        {
            MOS_Delete(m_lplaHelper);
            m_lplaHelper = nullptr;
        }
    }

    MOS_STATUS AV1VdencLplaEnc::Init(void *setting)
    {
        ENCODE_FUNC_CALL();
        m_lplaHelper = MOS_New(EncodeLPLA);
        ENCODE_CHK_NULL_RETURN(m_lplaHelper);

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AV1VdencLplaEnc::Update(void *params)
    {
        ENCODE_FUNC_CALL();
        ENCODE_CHK_NULL_RETURN(params);

        EncoderParams *encodeParams = (EncoderParams *)params;
        m_av1SeqParams = static_cast<PCODEC_AV1_ENCODE_SEQUENCE_PARAMS>(encodeParams->pSeqParams);
        ENCODE_CHK_NULL_RETURN(m_av1SeqParams);
        m_enabled = (m_av1SeqParams->LookaheadDepth > 0) && (m_av1SeqParams->RateControlMethod != RATECONTROL_CQP);
        if (!m_enabled)
        {
            return MOS_STATUS_SUCCESS;
        }
        m_av1PicParams = static_cast<PCODEC_AV1_ENCODE_PICTURE_PARAMS>(encodeParams->pPicParams);
        ENCODE_CHK_NULL_RETURN(m_av1PicParams);
        ENCODE_CHK_STATUS_RETURN(SetSequenceStructs());

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AV1VdencLplaEnc::SetSequenceStructs()
    {
        ENCODE_FUNC_CALL();
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        ENCODE_CHK_NULL_RETURN(m_lplaHelper);
        ENCODE_CHK_STATUS_RETURN(m_lplaHelper->CheckFrameRate(m_av1SeqParams->FrameRate[0].Numerator,
            m_av1SeqParams->FrameRate[0].Denominator,
            m_av1SeqParams->TargetBitRate[0], m_averageFrameSize));

        ENCODE_CHK_STATUS_RETURN(m_lplaHelper->CheckVBVBuffer(m_av1SeqParams->VBVBufferSizeInBit,
            m_av1SeqParams->InitVBVBufferFullnessInBit));
        if (m_targetBufferFulness == 0 && m_prevTargetFrameSize == 0)
        {
            m_targetBufferFulness = m_av1SeqParams->VBVBufferSizeInBit - m_av1SeqParams->InitVBVBufferFullnessInBit;
        }
        ENCODE_CHK_STATUS_RETURN(m_lplaHelper->CalculateTargetBufferFullness(m_targetBufferFulness, m_prevTargetFrameSize, m_averageFrameSize));
        m_prevTargetFrameSize = m_av1PicParams->TargetFrameSize;

        return eStatus;
    }

    MHW_SETPAR_DECL_SRC(HUC_DMEM_STATE, AV1VdencLplaEnc)
    {
        ENCODE_FUNC_CALL();

        if (!m_enabled)
        {
            return MOS_STATUS_SUCCESS;
        }

        ENCODE_CHK_NULL_RETURN(params.hucDataSource);
        ENCODE_CHK_NULL_RETURN(m_lplaHelper);

        if (params.function == BRC_UPDATE)
        {
            auto dmem = (VdencAv1HucBrcUpdateDmem*)m_allocator->LockResourceForWrite(params.hucDataSource);
            ENCODE_CHK_NULL_RETURN(dmem);
            dmem->UPD_LA_TargetFULNESS = m_targetBufferFulness;
            dmem->UPD_LALength = m_av1SeqParams->LookaheadDepth;
            dmem->UPD_TR_TargetSize = m_av1PicParams->TargetFrameSize << 3;  // byte to bit
            bool isLastPass = (params.currentPass ==  (params.passNum - 1));
            ENCODE_CHK_STATUS_RETURN(m_lplaHelper->CalculateDeltaQP(m_av1PicParams->QpModulationStrength, m_initDeltaQP, isLastPass,
                dmem->UPD_Delta, m_prevQpModulationStrength));

            ENCODE_CHK_STATUS_RETURN(m_allocator->UnLock(params.hucDataSource));
        }

        return MOS_STATUS_SUCCESS;
    }

}  // encode
