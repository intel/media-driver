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
//! \file     encode_hevc_vdenc_lpla_enc.cpp
//! \brief    Implementation for encode hevc lowpower lookahead(Encode Pass) feature
//!

#include "encode_hevc_vdenc_lpla_enc.h"
#include "encode_hevc_vdenc_feature_manager.h"

namespace encode
{
    HEVCVdencLplaEnc::HEVCVdencLplaEnc(
        MediaFeatureManager *featureManager,
        EncodeAllocator     *allocator,
        CodechalHwInterfaceNext *hwInterface,
        void                *constSettings) :
        MediaFeature(constSettings)
    {
    }

    HEVCVdencLplaEnc::~HEVCVdencLplaEnc()
    {
        if (m_lplaHelper)
        {
            MOS_Delete(m_lplaHelper);
            m_lplaHelper = nullptr;
        }
    }

    MOS_STATUS HEVCVdencLplaEnc::Init(void *setting)
    {
        ENCODE_FUNC_CALL();
        m_lplaHelper = MOS_New(EncodeLPLA);
        ENCODE_CHK_NULL_RETURN(m_lplaHelper);

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HEVCVdencLplaEnc::Update(void *params)
    {
        ENCODE_FUNC_CALL();
        ENCODE_CHK_NULL_RETURN(params);
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        EncoderParams *encodeParams = (EncoderParams *)params;
        m_hevcSeqParams = static_cast<PCODEC_HEVC_ENCODE_SEQUENCE_PARAMS>(encodeParams->pSeqParams);
        ENCODE_CHK_NULL_RETURN(m_hevcSeqParams);
        m_enabled = (m_hevcSeqParams->LookaheadDepth > 0) && !m_hevcSeqParams->bLookAheadPhase;
        if (!m_enabled)
        {
            return eStatus;
        }
        m_hevcPicParams = static_cast<PCODEC_HEVC_ENCODE_PICTURE_PARAMS>(encodeParams->pPicParams);
        ENCODE_CHK_NULL_RETURN(m_hevcPicParams);
        ENCODE_CHK_STATUS_RETURN(SetSequenceStructs());
        ENCODE_CHK_STATUS_RETURN(SetPictureStructs());

        return eStatus;
    }

    MOS_STATUS HEVCVdencLplaEnc::SetSequenceStructs()
    {
        ENCODE_FUNC_CALL();
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        ENCODE_CHK_NULL_RETURN(m_lplaHelper);
        ENCODE_CHK_STATUS_RETURN(m_lplaHelper->CheckFrameRate(m_hevcSeqParams->FrameRate.Numerator,
            m_hevcSeqParams->FrameRate.Denominator,
            m_hevcSeqParams->TargetBitRate, m_averageFrameSize));

        ENCODE_CHK_STATUS_RETURN(m_lplaHelper->CheckVBVBuffer(m_hevcSeqParams->VBVBufferSizeInBit,
            m_hevcSeqParams->InitVBVBufferFullnessInBit));
        if (m_targetBufferFulness == 0 && m_prevTargetFrameSize == 0)
        {
            m_targetBufferFulness = m_hevcSeqParams->VBVBufferSizeInBit - m_hevcSeqParams->InitVBVBufferFullnessInBit;
        }

        return eStatus;
    }

    MOS_STATUS HEVCVdencLplaEnc::SetPictureStructs()
    {
        ENCODE_FUNC_CALL();
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        ENCODE_CHK_NULL_RETURN(m_lplaHelper);
        ENCODE_CHK_STATUS_RETURN(m_lplaHelper->CalculateTargetBufferFullness(m_targetBufferFulness, m_prevTargetFrameSize, m_averageFrameSize));
        m_prevTargetFrameSize = m_hevcPicParams->TargetFrameSize;

        return eStatus;
    }

    MOS_STATUS HEVCVdencLplaEnc::SetDmemForInit(VdencHevcHucBrcInitDmem *hucVdencBrcInitDmem)
    {
        ENCODE_FUNC_CALL();
        hucVdencBrcInitDmem->LookaheadDepth_U8 = m_hevcSeqParams->LookaheadDepth;

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HEVCVdencLplaEnc::SetHucBrcUpdateExtBuffer(
        VdencHevcHucBrcUpdateDmem *hucVdencBrcUpdateDmem,
        bool isLastPass)
    {
        ENCODE_FUNC_CALL();
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        if (!m_enabled)
        {
            return eStatus;
        }
        hucVdencBrcUpdateDmem->TargetFulness = m_targetBufferFulness;
        ENCODE_CHK_NULL_RETURN(m_lplaHelper);
        ENCODE_CHK_STATUS_RETURN(m_lplaHelper->CalculateDeltaQP(m_hevcPicParams->QpModulationStrength, m_initDeltaQP, isLastPass,
            hucVdencBrcUpdateDmem->Delta, m_prevQpModulationStrength));

        return eStatus;
    }
}  // encode
