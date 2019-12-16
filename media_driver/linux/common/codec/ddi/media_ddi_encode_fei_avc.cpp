/*
* Copyright (c) 2017, Intel Corporation
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
//! \file     media_ddi_encode_fei_avc.cpp
//! \brief    Implements class for DDI media avc fei encode
//!

#include "media_libva.h"
#include "media_libva_encoder.h"
#include "media_libva_util.h"
#include "hwinfo_linux.h"
#include "media_ddi_encode_base.h"
#include "media_ddi_encode_fei_avc.h"
#include "media_ddi_encode_const.h"
#include "media_ddi_factory.h"
static const uint8_t feiMaxPassesNum = 4;

extern template class MediaDdiFactoryNoArg<DdiEncodeBase>;

static bool isEncodeAvcFeiRegistered =
    MediaDdiFactoryNoArg<DdiEncodeBase>::RegisterCodec<DdiEncodeAvcFei>(ENCODE_ID_AVCFEI);

DdiEncodeAvcFei::~DdiEncodeAvcFei()
{
    if (nullptr == m_encodeCtx)
    {
        return;
    }
    MOS_FreeMemory(m_encodeCtx->pFeiPicParams);
    m_encodeCtx->pFeiPicParams = nullptr;

    MOS_FreeMemory(m_encodeCtx->pPreEncParams);
    m_encodeCtx->pPreEncParams = nullptr;

    MOS_FreeMemory(iqMatrixParams);
    iqMatrixParams = nullptr;

    MOS_FreeMemory(iqWeightScaleLists);
    iqWeightScaleLists = nullptr;

}

VAStatus DdiEncodeAvcFei::ContextInitialize(CodechalSetting * codecHalSettings)
{
    VAStatus status = DdiEncodeAvc::ContextInitialize(codecHalSettings);
    if (VA_STATUS_SUCCESS != status)
    {
        return status;
    }

    codecHalSettings->codecFunction = m_encodeCtx->codecFunction;

    m_encodeCtx->pFeiPicParams = (void *)MOS_AllocAndZeroMemory(CODEC_AVC_MAX_PPS_NUM * sizeof(CodecEncodeAvcFeiPicParams));
    DDI_CHK_NULL(m_encodeCtx->pFeiPicParams, "nullptr m_encodeCtx->pFeiPicParams", VA_STATUS_ERROR_ALLOCATION_FAILED);

    m_encodeCtx->pPreEncParams = (void *)MOS_AllocAndZeroMemory(CODEC_AVC_MAX_PPS_NUM * sizeof(FeiPreEncParams));
    DDI_CHK_NULL(m_encodeCtx->pPreEncParams, "nullptr m_encodeCtx->pPreEncParams", VA_STATUS_ERROR_ALLOCATION_FAILED);

    iqMatrixParams = (PCODEC_AVC_IQ_MATRIX_PARAMS)MOS_AllocAndZeroMemory(sizeof(CODEC_AVC_IQ_MATRIX_PARAMS));
    DDI_CHK_NULL(iqMatrixParams, "nullptr iqMatrixParams", VA_STATUS_ERROR_ALLOCATION_FAILED);

    iqWeightScaleLists = (PCODEC_AVC_ENCODE_IQ_WEIGTHSCALE_LISTS)MOS_AllocAndZeroMemory(sizeof(CODEC_AVC_ENCODE_IQ_WEIGTHSCALE_LISTS));
    DDI_CHK_NULL(iqWeightScaleLists, "nullptr iqWeightScaleLists", VA_STATUS_ERROR_ALLOCATION_FAILED);

    return VA_STATUS_SUCCESS;
}

VAStatus DdiEncodeAvcFei::EncodeInCodecHal(uint32_t numSlices)
{
    uint8_t   ppsIdx, spsIdx;
    PCODEC_AVC_ENCODE_PIC_PARAMS      picParams;

    DDI_CHK_NULL(m_encodeCtx, "nullptr m_encodeCtx", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(m_encodeCtx->pCodecHal, "nullptr m_encodeCtx->pCodecHal", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(m_encodeCtx->pMediaCtx, "nullptr m_encodeCtx->pMediaCtx", VA_STATUS_ERROR_INVALID_CONTEXT);
    CodecEncodeAvcFeiPicParams *feiPicParams = (CodecEncodeAvcFeiPicParams *)(m_encodeCtx->pFeiPicParams);
    FeiPreEncParams *preEncParams = (FeiPreEncParams*)(m_encodeCtx->pPreEncParams);

    DDI_CODEC_RENDER_TARGET_TABLE *rtTbl     = &(m_encodeCtx->RTtbl);

    MOS_STATUS status;

    EncoderParams *encodeParams = &m_encodeCtx->EncodeParams;
    MOS_ZeroMemory(encodeParams, sizeof(EncoderParams));

    if ((feiPicParams != nullptr) && CodecHalIsFeiEncode(m_encodeCtx->codecFunction))
    {
        encodeParams->ExecCodecFunction = m_encodeCtx->codecFunction;
    }
    else if (m_encodeCtx->codecFunction == CODECHAL_FUNCTION_FEI_PRE_ENC)
    {
        encodeParams->ExecCodecFunction = m_encodeCtx->codecFunction;
    }
    else
    {
        encodeParams->ExecCodecFunction = CODECHAL_FUNCTION_ENC_PAK;
    }

    // Raw Surface
    PMOS_SURFACE rawSurface = &encodeParams->rawSurface;
    rawSurface->Format   = Format_NV12;
    rawSurface->dwOffset = 0;

    DdiMedia_MediaSurfaceToMosResource(rtTbl->pCurrentRT, &(rawSurface->OsResource));

    if (m_encodeCtx->codecFunction == CODECHAL_FUNCTION_FEI_PRE_ENC)
    {
        if (rtTbl->pRT[preEncParams->CurrOriginalPicture.FrameIdx] != rtTbl->pCurrentRT)
        {
            DDI_ASSERTMESSAGE("PREENC CurrOriginalPicture.FrameIdx != pCurrentRT");
        }
        rawSurface->dwWidth                  = rawSurface->OsResource.iWidth;
        rawSurface->dwHeight                 = rawSurface->OsResource.iHeight;
        rawSurface->dwPitch                  = rawSurface->OsResource.iPitch;
        rawSurface->TileType                 = rawSurface->OsResource.TileType;
        rawSurface->TileModeGMM              = rawSurface->OsResource.TileModeGMM;
        rawSurface->bGMMTileEnabled          = rawSurface->OsResource.bGMMTileEnabled;
        preEncParams->psCurrOriginalSurface  = rawSurface;
        encodeParams->pPreEncParams          = m_encodeCtx->pPreEncParams;
        DDI_CHK_RET(ClearRefList(&m_encodeCtx->RTtbl, false), "ClearRefList failed!");
    }
    else
    {
        // Recon Surface
        PMOS_SURFACE reconSurface = &encodeParams->reconSurface;
        reconSurface->Format   = Format_NV12;
        reconSurface->dwOffset = 0;

        DdiMedia_MediaSurfaceToMosResource(rtTbl->pCurrentReconTarget, &(reconSurface->OsResource));

        // Bitstream surface
        PMOS_RESOURCE bitstreamSurface = &encodeParams->resBitstreamBuffer;
        *bitstreamSurface        = m_encodeCtx->resBitstreamBuffer;  // in render picture
        bitstreamSurface->Format = Format_Buffer;

        encodeParams->psRawSurface = &encodeParams->rawSurface;
        encodeParams->psReconSurface = &encodeParams->reconSurface;
        encodeParams->presBitstreamBuffer = &encodeParams->resBitstreamBuffer;

        PMOS_SURFACE mbQpSurface = &encodeParams->mbQpSurface;
        if (m_encodeCtx->bMBQpEnable)
        {
            mbQpSurface->Format             = Format_Buffer_2D;
            mbQpSurface->dwOffset           = 0;
            mbQpSurface->OsResource         = m_encodeCtx->resMBQpBuffer;
            encodeParams->psMbQpDataSurface = &encodeParams->mbQpSurface;
            encodeParams->bMbQpDataEnabled  = true;
        }

        PMOS_SURFACE disableSkipMapSurface = &encodeParams->disableSkipMapSurface;
        encodeParams->bMbDisableSkipMapEnabled = m_encodeCtx->bMbDisableSkipMapEnabled;
        if (encodeParams->bMbDisableSkipMapEnabled)
        {
            disableSkipMapSurface->Format           = Format_Buffer;
            disableSkipMapSurface->dwOffset         = 0;
            disableSkipMapSurface->OsResource       = m_encodeCtx->resPerMBSkipMapBuffer;
            encodeParams->psMbDisableSkipMapSurface = &encodeParams->disableSkipMapSurface;
        }

        // correct some params
        if (CODECHAL_ENCODE_MODE_AVC == m_encodeCtx->wModeType)
        {
            PCODEC_AVC_ENCODE_SEQUENCE_PARAMS seqParams;
            CODECHAL_ENCODE_AVC_VUI_PARAMS    *vuiParam;

            vuiParam  = (CODECHAL_ENCODE_AVC_VUI_PARAMS *)m_encodeCtx->pVuiParams;
            seqParams = (PCODEC_AVC_ENCODE_SEQUENCE_PARAMS)(m_encodeCtx->pSeqParams);

            if (VA_RC_CQP == m_encodeCtx->uiRCMethod)
            {
                vuiParam->bit_rate_value_minus1[0]    = 0;
                vuiParam->cpb_size_value_minus1[0]    = 0;
                seqParams->TargetBitRate              = 0;
                seqParams->MaxBitRate                 = 0;
                seqParams->MinBitRate                 = 0;
                seqParams->InitVBVBufferFullnessInBit = 0;
                seqParams->VBVBufferSizeInBit         = 0;
            }

            encodeParams->uiSlcStructCaps = CODECHAL_SLICE_STRUCT_ARBITRARYMBSLICE;
        }

        ppsIdx                           = ((PCODEC_AVC_ENCODE_SLICE_PARAMS)(m_encodeCtx->pSliceParams))->pic_parameter_set_id;
        picParams                        = (PCODEC_AVC_ENCODE_PIC_PARAMS)m_encodeCtx->pPicParams + ppsIdx;
        spsIdx                           = picParams->seq_parameter_set_id;
        encodeParams->pSeqParams         = (PCODEC_AVC_ENCODE_SEQUENCE_PARAMS)m_encodeCtx->pSeqParams + spsIdx;
        encodeParams->pPicParams         = picParams;
        encodeParams->pVuiParams         = m_encodeCtx->pVuiParams;
        encodeParams->pSliceParams       = m_encodeCtx->pSliceParams;
        encodeParams->pAVCQCParams       = m_qcParams;
        encodeParams->pAVCRoundingParams = m_roundingParams;

        // Sequence data
        encodeParams->bNewSeq = m_encodeCtx->bNewSeq;
        // VUI
        encodeParams->bNewVuiData = m_encodeCtx->bNewVuiData;

        // Slice level data
        encodeParams->dwNumSlices = numSlices;

        // IQmatrix params
        encodeParams->bNewQmatrixData = m_encodeCtx->bNewQmatrixData;
        encodeParams->bPicQuant       = m_encodeCtx->bPicQuant;
        encodeParams->ppNALUnitParams = m_encodeCtx->ppNALUnitParams;
        encodeParams->pSeiData        = m_encodeCtx->pSEIFromApp;
        encodeParams->pSeiParamBuffer = m_encodeCtx->pSEIFromApp->pSEIBuffer;
        encodeParams->dwSEIDataOffset = 0;

        status = MOS_SecureMemcpy(&iqMatrixParams->ScalingList4x4,
            6 * 16 * sizeof(uint8_t),
            &m_scalingLists4x4,
            6 * 16 * sizeof(uint8_t));
        if (MOS_STATUS_SUCCESS != status)
        {
            DDI_ASSERTMESSAGE("DDI:Failed to copy scaling list 4x4!");
            return VA_STATUS_ERROR_INVALID_PARAMETER;
        }

        status = MOS_SecureMemcpy(&iqMatrixParams->ScalingList8x8,
            2 * 64 * sizeof(uint8_t),
            &m_scalingLists8x8,
            2 * 64 * sizeof(uint8_t));
        if (MOS_STATUS_SUCCESS != status)
        {
            DDI_ASSERTMESSAGE("DDI:Failed to copy scaling list 8x8!");
            return VA_STATUS_ERROR_INVALID_PARAMETER;
        }

        encodeParams->pIQMatrixBuffer = iqMatrixParams;

        status = MOS_SecureMemcpy(&iqWeightScaleLists->WeightScale4x4,
            (CODEC_AVC_WEIGHT_SCALE_4x4 * sizeof(uint32_t)),
            &m_weightScale4x4,
            (CODEC_AVC_WEIGHT_SCALE_4x4 * sizeof(uint32_t)));
        if (MOS_STATUS_SUCCESS != status)
        {
            DDI_ASSERTMESSAGE("DDI:Failed to copy weight scale list 4x4!");
            return VA_STATUS_ERROR_INVALID_PARAMETER;
        }

        status = MOS_SecureMemcpy(&iqWeightScaleLists->WeightScale8x8,
            (CODEC_AVC_WEIGHT_SCALE_8x8 * sizeof(uint32_t)),
            &m_weightScale8x8,
            (CODEC_AVC_WEIGHT_SCALE_8x8 * sizeof(uint32_t)));
        if (MOS_STATUS_SUCCESS != status)
        {
            DDI_ASSERTMESSAGE("DDI:Failed to copy weight scale list 8x8!");
            return VA_STATUS_ERROR_INVALID_PARAMETER;
        }

        encodeParams->pIQWeightScaleLists = iqWeightScaleLists;

        // whether driver need to pack slice header
        if (true == m_encodeCtx->bHavePackedSliceHdr)
        {
            encodeParams->bAcceleratorHeaderPackingCaps = false;
        }
        else
        {
            encodeParams->bAcceleratorHeaderPackingCaps = true;
        }

        encodeParams->pBSBuffer      = m_encodeCtx->pbsBuffer;
        encodeParams->pSlcHeaderData = (void *)m_encodeCtx->pSliceHeaderData;
        encodeParams->pFeiPicParams  = (CodecEncodeAvcFeiPicParams *)(m_encodeCtx->pFeiPicParams);
        //clear registered recon/ref surface flags
        DDI_CHK_RET(ClearRefList(&m_encodeCtx->RTtbl, true), "ClearRefList failed!");
    }

    CodechalEncoderState *encoder = dynamic_cast<CodechalEncoderState *>(m_encodeCtx->pCodecHal);
    DDI_CHK_NULL(encoder, "nullptr Codechal encode", VA_STATUS_ERROR_INVALID_PARAMETER);

    if (!encoder->m_mfeEnabled)
    {
        status = m_encodeCtx->pCodecHal->Execute(encodeParams);
        if (MOS_STATUS_SUCCESS != status)
        {
            DDI_ASSERTMESSAGE("DDI:Failed in Codechal!");
            return VA_STATUS_ERROR_ENCODING_ERROR;
        }
    }

    return VA_STATUS_SUCCESS;
}

VAStatus DdiEncodeAvcFei::ResetAtFrameLevel()
{
    DDI_CHK_NULL(m_encodeCtx, "nullptr m_encodeCtx", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(m_encodeCtx->pFeiPicParams, "nullptr m_encodeCtx->pFeiPicParams", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(m_encodeCtx->pPreEncParams, "nullptr m_encodeCtx->pPreEncParams", VA_STATUS_ERROR_INVALID_PARAMETER);

    // Assume there is only one SPS parameter
    PCODEC_AVC_ENCODE_SEQUENCE_PARAMS seqParams      = (PCODEC_AVC_ENCODE_SEQUENCE_PARAMS)(m_encodeCtx->pSeqParams);
    seqParams->bInitBRC                              = 0x0;
    seqParams->bResetBRC                             = 0x0;
    CodecEncodeAvcFeiPicParams         *feiPicParams = (CodecEncodeAvcFeiPicParams *)(m_encodeCtx->pFeiPicParams);
    FeiPreEncParams *preEncParams = (FeiPreEncParams*)(m_encodeCtx->pPreEncParams);

    m_encodeCtx->dwNumSlices      = 0x0;
    m_encodeCtx->indexNALUnit     = 0x0;
    m_encodeCtx->uiSliceHeaderCnt = 0x0;

    // reset bsbuffer every frame
    m_encodeCtx->pbsBuffer->pCurrent    = m_encodeCtx->pbsBuffer->pBase;
    m_encodeCtx->pbsBuffer->SliceOffset = 0x0;
    m_encodeCtx->pbsBuffer->BitOffset   = 0x0;
    m_encodeCtx->pbsBuffer->BitSize     = 0x0;

    // clear the packed header information
    if (nullptr != m_encodeCtx->ppNALUnitParams)
    {
        MOS_ZeroMemory(m_encodeCtx->ppNALUnitParams[0], sizeof(CODECHAL_NAL_UNIT_PARAMS) * CODECHAL_ENCODE_AVC_MAX_NAL_TYPE);
    }

    m_encodeCtx->bHavePackedSliceHdr      = false;
    m_encodeCtx->bLastPackedHdrIsSlice    = false;
    m_encodeCtx->bMbDisableSkipMapEnabled = false;
    m_encodeCtx->bMBQpEnable              = false;

    if (nullptr != m_roundingParams)
    {
        MOS_ZeroMemory(m_roundingParams, sizeof(CODECHAL_ENCODE_AVC_ROUNDING_PARAMS));
    }

    if (CodecHalIsFeiEncode(m_encodeCtx->codecFunction) && m_encodeCtx->codecFunction != CODECHAL_FUNCTION_FEI_PRE_ENC)
    {
        m_encodeCtx->codecFunction             = CODECHAL_FUNCTION_FEI_ENC_PAK;
        feiPicParams->NumMVPredictorsL0      = 0;      // number of MV Predictors L0 provided, max is 4
        feiPicParams->NumMVPredictorsL1      = 0;      // number of MV Predictors L1 provided, max is 2
        feiPicParams->SearchPath             = 0;      // search path, default is 0, 0 and 2 mean full search, 1 means diamond search
        feiPicParams->LenSP                  = 57;     // max number of SUs per reference which is evaluated by the predetermined SUs, range is [1, 63]
        feiPicParams->SubMBPartMask          = 0x77;   // the bit-mask for disabling sub-partition, bit 0 16x16, bit 1 16x8, bit 2 8x16 ... default value is 1110111b
        feiPicParams->IntraPartMask          = 0;      // luma intra partition mask bits, bit 0 16x16, bit 1 8x8, bit 2 4x4
        feiPicParams->MultiPredL0            = false;  // true neighbor MV will be used as predictori for L0, false will not.
        feiPicParams->MultiPredL1            = false;  // true neighbor MV will be used as predictori for L0, false will not.
        feiPicParams->SubPelMode             = 3;      // half/quater pixels mode, 00b integer mode search, 01b half mode search, 11b quater mode search
        feiPicParams->InterSAD               = 2;
        feiPicParams->IntraSAD               = 2;
        feiPicParams->DistortionType         = 0;      // output distortion type, 0 raw distortion without cost, 1 distortion with cost
        feiPicParams->RepartitionCheckEnable = false;  // true, enables the additional calls on Fraction & Bidirectional Refinement
        feiPicParams->AdaptiveSearch         = true;   // whether adaptive searching is enabled for IME
        feiPicParams->MVPredictorEnable      = false;  // if enable MV Predictor input
        feiPicParams->bMBQp                  = false;  // if enable MB per QP input
        feiPicParams->bPerMBInput            = false;  // if enable per MB control/special input
        feiPicParams->bMBSizeCtrl            = false;  // if enable MB size control when enabled per MB special input
        feiPicParams->RefWidth               = 48;
        feiPicParams->RefHeight              = 40;
        feiPicParams->SearchWindow           = 0;

        feiPicParams->DistortionEnable = false;
        feiPicParams->MbCodeMvEnable   = false;
    }

    if (m_encodeCtx->codecFunction == CODECHAL_FUNCTION_FEI_PRE_ENC)
    {
        preEncParams->dwNumPastReferences   = 1;  // Number of past reference frame
        preEncParams->dwNumFutureReferences = 0;  // Number of future reference frame
        MOS_ZeroMemory(&(preEncParams->CurrOriginalPicture), sizeof(CODEC_PICTURE));
        MOS_ZeroMemory(&(preEncParams->PastRefPicture), sizeof(CODEC_PICTURE));
        MOS_ZeroMemory(&(preEncParams->FutureRefPicture), sizeof(CODEC_PICTURE));

        preEncParams->dwFrameQp         = 25;    // Frame level QP
        preEncParams->dwLenSP           = 57;    // max number of SUs per reference which is evaluated by the predetermined SUs, range is [1, 63]
        preEncParams->dwSearchPath      = 0;     // search path, default is 0, 0 and 2 mean full search, 1 means diamond search
        preEncParams->dwSubMBPartMask   = 0x77;  // the bit-mask for disabling sub-partition, bit 0 16x16, bit 1 16x8, bit 2 8x16 ... default value is 1110111b
        preEncParams->dwIntraPartMask   = 0;
        preEncParams->dwSubPelMode      = 3;  // half/quater pixels mode, 00b integer mode search, 01b half mode search, 11b quater mode search
        preEncParams->dwInterSAD        = 2;
        preEncParams->dwIntraSAD        = 2;
        preEncParams->bAdaptiveSearch   = 1;      // whether adaptive searching is enabled for IME
        preEncParams->dwMVPredictorCtrl = 0;      // MV predictor control, 0, disable, 1, enabled for past reference, 2, enabled for future reference, 3 enabled for both
        preEncParams->bMBQp             = false;  // if enable MB per QP input
        preEncParams->bFTEnable         = false;  // if use provided QP (frame level QP and per MB QP) and obtained MV to judge the number of non zero coefficients
        preEncParams->dwRefWidth        = 48;
        preEncParams->dwRefHeight       = 40;
        preEncParams->dwSearchWindow    = 0;             // defines predefined search windows, 0, disable
                                                         // 1, Tiny 4 SUs 24x24 windows
                                                         // 2, small 9 SUs 28x28 windows
                                                         // 3, diamond 16 SUs 48x40 windows
                                                         // 4, large diamon 32SUs 48x40 windows
                                                         // 5, exhaustive 48SUs 48x40 windows
        preEncParams->bDisableMVOutput         = false;  // if disable MV output buffer
        preEncParams->bDisableStatisticsOutput = false;  // if disable Statisticts output buffer
        preEncParams->bEnable8x8Statistics     = false;  // if enable, generate block8x8 average and variance output
    }

    return VA_STATUS_SUCCESS;
}

VAStatus DdiEncodeAvcFei::RenderPicture(
    VADriverContextP ctx,
    VAContextID      context,
    VABufferID       *buffers,
    int32_t          numBuffers)
{
    DDI_FUNCTION_ENTER();

    DDI_CHK_NULL(ctx, "nullptr context in vpgEncodeRenderPicture!", VA_STATUS_ERROR_INVALID_CONTEXT);
    PDDI_MEDIA_CONTEXT mediaCtx = DdiMedia_GetMediaContext(ctx);
    DDI_CHK_NULL(mediaCtx, "nullptr mediaCtx", VA_STATUS_ERROR_INVALID_CONTEXT);

    // assume the VAContextID is encoder ID
    DDI_CHK_NULL(m_encodeCtx, "nullptr m_encodeCtx", VA_STATUS_ERROR_INVALID_CONTEXT);

    uint32_t numSlices = 0;
    VAStatus vaStatus  = VA_STATUS_SUCCESS;

    DDI_MEDIA_BUFFER *buf;
    void             *data;
    uint32_t         dataSize;
    for (int32_t i = 0; i < numBuffers; i++)
    {
        buf = DdiMedia_GetBufferFromVABufferID(mediaCtx, buffers[i]);
        DDI_CHK_NULL(buf, "Invalid buffer.", VA_STATUS_ERROR_INVALID_BUFFER);
        if (buf->uiType == VAEncMacroblockDisableSkipMapBufferType)
        {
            DdiMedia_MediaBufferToMosResource(buf, &(m_encodeCtx->resPerMBSkipMapBuffer));
            m_encodeCtx->bMbDisableSkipMapEnabled = true;
            continue;
        }
        dataSize = buf->iSize;
        // can use internal function instead of DdiMedia_MapBuffer here?
        DdiMedia_MapBuffer(ctx, buffers[i], &data);

        DDI_CHK_NULL(data, "nullptr data", VA_STATUS_ERROR_INVALID_BUFFER);

        switch (buf->uiType)
        {
        case VAIQMatrixBufferType:
        case VAQMatrixBufferType:
            DDI_CHK_STATUS(Qmatrix(data), VA_STATUS_ERROR_INVALID_BUFFER);
            break;

        case VAEncSequenceParameterBufferType:
            DDI_CHK_STATUS(ParseSeqParams(data), VA_STATUS_ERROR_INVALID_BUFFER);
            m_encodeCtx->bNewSeq = true;
            break;

        case VAEncPictureParameterBufferType:
            DDI_CHK_STATUS(ParsePicParams(mediaCtx, data), VA_STATUS_ERROR_INVALID_BUFFER);

            // it should use pFeiPicParams->function, but it may be still not set here yet
            if (m_encodeCtx->codecFunction == CODECHAL_FUNCTION_FEI_ENC)
                break;

            DDI_CHK_STATUS(
                AddToStatusReportQueue((void *)m_encodeCtx->resBitstreamBuffer.bo),
                VA_STATUS_ERROR_INVALID_BUFFER);
            break;

        case VAEncSliceParameterBufferType:
            numSlices = buf->uiNumElements;
            DDI_CHK_STATUS(ParseSlcParams(mediaCtx, data, numSlices), VA_STATUS_ERROR_INVALID_BUFFER);
            break;

        case VAEncPackedHeaderParameterBufferType:
            DDI_CHK_STATUS(ParsePackedHeaderParams(data), VA_STATUS_ERROR_INVALID_BUFFER);
            break;

        case VAEncPackedHeaderDataBufferType:
            vaStatus = ParsePackedHeaderData(data);
            break;

        case VAEncMiscParameterBufferType:
            DDI_CHK_STATUS(ParseMiscParams(data), VA_STATUS_ERROR_INVALID_BUFFER);
            break;

        case VAEncQPBufferType:
            DdiMedia_MediaBufferToMosResource(buf, &m_encodeCtx->resMBQpBuffer);
            m_encodeCtx->bMBQpEnable = true;
            break;

        case VAStatsStatisticsParameterBufferType:
            DDI_CHK_STATUS(ParseStatsParams(mediaCtx, data), VA_STATUS_ERROR_INVALID_BUFFER);
            break;

        case VAEncFEIMBControlBufferType:
        case VAEncFEIMVPredictorBufferType:
        case VAEncFEIMVBufferType:
        case VAEncFEIMBCodeBufferType:
        case VAEncFEIDistortionBufferType:
        case VAStatsMVPredictorBufferType:
        case VAStatsStatisticsBufferType:
        case VAStatsStatisticsBottomFieldBufferType:
        case VAStatsMVBufferType:
        {
            // handled in VAEncMiscParameterBufferType/VAEncMiscParameterTypeFEIFrameControlIntel case by vaBufferID
            break;
        }
        default:
            DDI_ASSERTMESSAGE("not supported buffer type in vpgEncodeRenderPicture.");
            // vaStatus = VA_STATUS_ERROR_UNSUPPORTED_BUFFERTYPE;
            break;
        }
        DdiMedia_UnmapBuffer(ctx, buffers[i]);
    }

    DDI_FUNCTION_EXIT(vaStatus);
    return vaStatus;
}

VAStatus DdiEncodeAvcFei::ParseMiscParamFeiPic(void *data)
{
    DDI_CHK_NULL(data, "nullptr data", VA_STATUS_ERROR_INVALID_PARAMETER);
    CodecEncodeAvcFeiPicParams *feiPicParams = (CodecEncodeAvcFeiPicParams *)(m_encodeCtx->pFeiPicParams);
    DDI_CHK_NULL(feiPicParams, "nullptr feiPicParams", VA_STATUS_ERROR_INVALID_PARAMETER);
    VAEncMiscParameterFEIFrameControlH264 *vaEncMiscParamFeiPic = (VAEncMiscParameterFEIFrameControlH264*)data;

    m_encodeCtx->codecFunction     = CODECHAL_FUNCTION_INVALID;
    if (vaEncMiscParamFeiPic->function & VA_FEI_FUNCTION_ENC_PAK)
        m_encodeCtx->codecFunction = CODECHAL_FUNCTION_FEI_ENC_PAK;
    if (vaEncMiscParamFeiPic->function == VA_FEI_FUNCTION_ENC)
        m_encodeCtx->codecFunction = CODECHAL_FUNCTION_FEI_ENC;
    if (vaEncMiscParamFeiPic->function == VA_FEI_FUNCTION_PAK)
        m_encodeCtx->codecFunction = CODECHAL_FUNCTION_FEI_PAK;

    feiPicParams->NumMVPredictorsL0      = vaEncMiscParamFeiPic->num_mv_predictors_l0;
    feiPicParams->NumMVPredictorsL1      = vaEncMiscParamFeiPic->num_mv_predictors_l1;
    feiPicParams->SearchPath             = vaEncMiscParamFeiPic->search_path;
    feiPicParams->LenSP                  = vaEncMiscParamFeiPic->len_sp;
    feiPicParams->SubMBPartMask          = vaEncMiscParamFeiPic->sub_mb_part_mask;
    feiPicParams->IntraPartMask          = vaEncMiscParamFeiPic->intra_part_mask;
    feiPicParams->MultiPredL0            = vaEncMiscParamFeiPic->multi_pred_l0;
    feiPicParams->MultiPredL1            = vaEncMiscParamFeiPic->multi_pred_l1;
    feiPicParams->SubPelMode             = vaEncMiscParamFeiPic->sub_pel_mode;
    feiPicParams->InterSAD               = vaEncMiscParamFeiPic->inter_sad;
    feiPicParams->IntraSAD               = vaEncMiscParamFeiPic->intra_sad;
    feiPicParams->DistortionType         = vaEncMiscParamFeiPic->distortion_type;
    feiPicParams->RepartitionCheckEnable = vaEncMiscParamFeiPic->repartition_check_enable;
    feiPicParams->AdaptiveSearch         = vaEncMiscParamFeiPic->adaptive_search;
    feiPicParams->MVPredictorEnable      = vaEncMiscParamFeiPic->mv_predictor_enable;
    feiPicParams->bMBQp                  = vaEncMiscParamFeiPic->mb_qp;
    feiPicParams->bPerMBInput            = vaEncMiscParamFeiPic->mb_input;
    feiPicParams->bMBSizeCtrl            = vaEncMiscParamFeiPic->mb_size_ctrl;
    feiPicParams->RefWidth               = vaEncMiscParamFeiPic->ref_width;
    feiPicParams->RefHeight              = vaEncMiscParamFeiPic->ref_height;
    feiPicParams->SearchWindow           = vaEncMiscParamFeiPic->search_window;

    DDI_MEDIA_BUFFER *mediaBuffer;
    VAStatus          status = VA_STATUS_SUCCESS;
    if (feiPicParams->bPerMBInput)
    {
        mediaBuffer = DdiMedia_GetBufferFromVABufferID(m_encodeCtx->pMediaCtx, vaEncMiscParamFeiPic->mb_ctrl);
        DDI_CHK_NULL(mediaBuffer, "nullptr mediaBuffer", VA_STATUS_ERROR_INVALID_PARAMETER);
        DdiMedia_MediaBufferToMosResource(mediaBuffer, &(feiPicParams->resMBCtrl));
    }
    if (feiPicParams->MVPredictorEnable == 1)
    {
        mediaBuffer = DdiMedia_GetBufferFromVABufferID(m_encodeCtx->pMediaCtx, vaEncMiscParamFeiPic->mv_predictor);
        DDI_CHK_NULL(mediaBuffer, "nullptr mediaBuffer", VA_STATUS_ERROR_INVALID_PARAMETER);
        DdiMedia_MediaBufferToMosResource(mediaBuffer, &(feiPicParams->resMVPredictor));
    }
    else if ((feiPicParams->NumMVPredictorsL0 != 0) || (feiPicParams->NumMVPredictorsL1 != 0))
    {
        DDI_ASSERTMESSAGE("feiPicParams->NumMVPredictorsL0 and NumMVPredictorsL1 should be set to 0 when feiPicParams->MVPredictorEnable is false!");
        status = VA_STATUS_ERROR_INVALID_PARAMETER;
    }
    if (feiPicParams->bMBQp)
    {
        mediaBuffer = DdiMedia_GetBufferFromVABufferID(m_encodeCtx->pMediaCtx, vaEncMiscParamFeiPic->qp);
        DDI_CHK_NULL(mediaBuffer, "nullptr mediaBuffer", VA_STATUS_ERROR_INVALID_PARAMETER);
        DdiMedia_MediaBufferToMosResource(mediaBuffer, &(feiPicParams->resMBQp));
    }

    feiPicParams->MbCodeMvEnable = false;
    if (vaEncMiscParamFeiPic->mv_data != VA_INVALID_ID)
    {
        feiPicParams->MbCodeMvEnable = true;
        mediaBuffer                  = DdiMedia_GetBufferFromVABufferID(m_encodeCtx->pMediaCtx, vaEncMiscParamFeiPic->mv_data);
        DDI_CHK_NULL(mediaBuffer, "nullptr mediaBuffer", VA_STATUS_ERROR_INVALID_PARAMETER);
        DdiMedia_MediaBufferToMosResource(mediaBuffer, &(feiPicParams->resMVData));
        if (m_encodeCtx->codecFunction == CODECHAL_FUNCTION_FEI_ENC)
        {
            RemoveFromEncStatusReportQueue(mediaBuffer, FEI_ENC_BUFFER_TYPE_MVDATA);
            if (VA_STATUS_SUCCESS != AddToEncStatusReportQueue((void *)(feiPicParams->resMVData.bo), FEI_ENC_BUFFER_TYPE_MVDATA))
            {
                DDI_ASSERTMESSAGE("feiPicParams->resMVData is invalid for FEI ENC only");
                status = VA_STATUS_ERROR_INVALID_PARAMETER;
            }
        }
    }
    if (vaEncMiscParamFeiPic->mb_code_data != VA_INVALID_ID)
    {
        if (feiPicParams->MbCodeMvEnable == false)
        {
            DDI_ASSERTMESSAGE("MV data and MB Code should be enabled or disabled together!");
            status = MOS_STATUS_INVALID_PARAMETER;
        }
        mediaBuffer = DdiMedia_GetBufferFromVABufferID(m_encodeCtx->pMediaCtx, vaEncMiscParamFeiPic->mb_code_data);
        DDI_CHK_NULL(mediaBuffer, "nullptr mediaBuffer", VA_STATUS_ERROR_INVALID_PARAMETER);
        DdiMedia_MediaBufferToMosResource(mediaBuffer, &(feiPicParams->resMBCode));
        if (m_encodeCtx->codecFunction == CODECHAL_FUNCTION_FEI_ENC)
        {
            RemoveFromEncStatusReportQueue(mediaBuffer, FEI_ENC_BUFFER_TYPE_MBCODE);
            if (MOS_STATUS_SUCCESS != AddToEncStatusReportQueue((void *)(feiPicParams->resMBCode.bo), FEI_ENC_BUFFER_TYPE_MBCODE))
            {
                DDI_ASSERTMESSAGE("feiPicParams->resMBCode is invalid for FEI ENC only");
                status = VA_STATUS_ERROR_INVALID_PARAMETER;
            }
        }
    }
    if (vaEncMiscParamFeiPic->distortion != VA_INVALID_ID)
    {
        feiPicParams->DistortionEnable = true;
        mediaBuffer                    = DdiMedia_GetBufferFromVABufferID(m_encodeCtx->pMediaCtx, vaEncMiscParamFeiPic->distortion);
        DDI_CHK_NULL(mediaBuffer, "nullptr mediaBuffer", VA_STATUS_ERROR_INVALID_PARAMETER);
        DdiMedia_MediaBufferToMosResource(mediaBuffer, &(feiPicParams->resDistortion));
        if (m_encodeCtx->codecFunction == CODECHAL_FUNCTION_FEI_ENC)
        {
            RemoveFromEncStatusReportQueue(mediaBuffer, FEI_ENC_BUFFER_TYPE_DISTORTION);
            if (MOS_STATUS_SUCCESS != AddToEncStatusReportQueue((void *)(feiPicParams->resDistortion.bo), FEI_ENC_BUFFER_TYPE_DISTORTION))
            {
                DDI_ASSERTMESSAGE("feiPicParams->resDistortion is invalid for FEI ENC only");
                status = VA_STATUS_ERROR_INVALID_PARAMETER;
            }
        }
    }
    if (m_encodeCtx->codecFunction == CODECHAL_FUNCTION_FEI_ENC)
    {
        AddToEncStatusReportQueueUpdatePos();
    }

    //add for mutlple pass pak
    feiPicParams->dwMaxFrameSize = vaEncMiscParamFeiPic->max_frame_size;
    if (feiPicParams->dwMaxFrameSize)
    {
        feiPicParams->dwNumPasses = vaEncMiscParamFeiPic->num_passes;
        if ((feiPicParams->dwNumPasses == 0) || (feiPicParams->dwNumPasses > feiMaxPassesNum))
        {
            return VA_STATUS_ERROR_INVALID_PARAMETER;
        }
        if (feiPicParams->pDeltaQp != nullptr)
        {
            MOS_FreeMemory(feiPicParams->pDeltaQp);
        }
        feiPicParams->pDeltaQp = (uint8_t *)MOS_AllocAndZeroMemory(sizeof(uint8_t) * feiPicParams->dwNumPasses);
        if (!feiPicParams->pDeltaQp)
        {
            return VA_STATUS_ERROR_INVALID_PARAMETER;
        }

        if (MOS_STATUS_SUCCESS != MOS_SecureMemcpy(feiPicParams->pDeltaQp, feiPicParams->dwNumPasses, vaEncMiscParamFeiPic->delta_qp, feiPicParams->dwNumPasses))
        {
            status = VA_STATUS_ERROR_INVALID_PARAMETER;
        }
    }

    return status;
}

VAStatus DdiEncodeAvcFei::AddToEncStatusReportQueue(
    void                           *encBuf,
    DDI_ENCODE_FEI_ENC_BUFFER_TYPE typeIdx)
{
    DDI_CHK_NULL(encBuf, "nullptr encBuf", VA_STATUS_ERROR_INVALID_PARAMETER);

    CodecEncodeAvcFeiPicParams *feiPicParams = (CodecEncodeAvcFeiPicParams *)(m_encodeCtx->pFeiPicParams);
    DDI_CHK_NULL(feiPicParams, "nullptr feiPicParams", VA_STATUS_ERROR_INVALID_PARAMETER);

    if (m_encodeCtx->codecFunction != CODECHAL_FUNCTION_FEI_ENC)
    {
        DDI_ASSERTMESSAGE("ENC output buffers status checking is not allowed for non-FEI_ENC case! .");
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }
    if ((typeIdx < 0) || (typeIdx >= FEI_ENC_BUFFER_TYPE_MAX))
    {
        DDI_ASSERTMESSAGE("ENC output buffers status checking, gets invalid buffer type index! .");
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }

    int32_t idx                                                 = m_encodeCtx->statusReportBuf.ulHeadPosition;
    m_encodeCtx->statusReportBuf.encInfos[idx].pEncBuf[typeIdx] = encBuf;
    m_encodeCtx->statusReportBuf.encInfos[idx].uiStatus         = 0;
    m_encodeCtx->statusReportBuf.encInfos[idx].uiBuffers++;

    return VA_STATUS_SUCCESS;
}

VAStatus DdiEncodeAvcFei::AddToEncStatusReportQueueUpdatePos()
{
    CodecEncodeAvcFeiPicParams *feiPicParams = (CodecEncodeAvcFeiPicParams *)(m_encodeCtx->pFeiPicParams);
    DDI_CHK_NULL(feiPicParams, "nullptr feiPicParams", VA_STATUS_ERROR_INVALID_PARAMETER);

    if (m_encodeCtx->codecFunction != CODECHAL_FUNCTION_FEI_ENC)
    {
        DDI_ASSERTMESSAGE("ENC output buffers status checking is not allowed for non-FEI_ENC case! .");
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }

    int32_t i = m_encodeCtx->statusReportBuf.ulHeadPosition;
    if ((m_encodeCtx->statusReportBuf.encInfos[i].uiBuffers == (feiPicParams->MbCodeMvEnable * 2 + feiPicParams->DistortionEnable)) &&
        m_encodeCtx->statusReportBuf.encInfos[i].uiBuffers != 0)
    {
        m_encodeCtx->statusReportBuf.ulHeadPosition = (m_encodeCtx->statusReportBuf.ulHeadPosition + 1) % DDI_ENCODE_MAX_STATUS_REPORT_BUFFER;
    }

    return VA_STATUS_SUCCESS;
}

VAStatus DdiEncodeAvcFei::AddToPreEncStatusReportQueue(
    void                           *preEncBuf,
    DDI_ENCODE_PRE_ENC_BUFFER_TYPE typeIdx)
{
    DDI_CHK_NULL(preEncBuf, "nullptr preEncBuf", VA_STATUS_ERROR_INVALID_PARAMETER);

    if (m_encodeCtx->codecFunction != CODECHAL_FUNCTION_FEI_PRE_ENC)
    {
        DDI_ASSERTMESSAGE("PRE ENC output buffers status checking is not allowed for non-PRE_ENC case! .");
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }
    if ((typeIdx < 0) || (typeIdx >= PRE_ENC_BUFFER_TYPE_MAX))
    {
        DDI_ASSERTMESSAGE("PRE ENC output buffers status checking, gets invalid buffer type index! .");
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }

    int32_t i                                                       = m_encodeCtx->statusReportBuf.ulHeadPosition;
    m_encodeCtx->statusReportBuf.preencInfos[i].pPreEncBuf[typeIdx] = preEncBuf;
    m_encodeCtx->statusReportBuf.preencInfos[i].uiStatus            = 0;
    m_encodeCtx->statusReportBuf.preencInfos[i].uiBuffers++;

    return VA_STATUS_SUCCESS;
}

VAStatus DdiEncodeAvcFei::AddToPreEncStatusReportQueueUpdatePos()
{
    FeiPreEncParams *preEncParams = (FeiPreEncParams*)(m_encodeCtx->pPreEncParams);
    DDI_CHK_NULL(preEncParams, "nullptr preEncParams", VA_STATUS_ERROR_INVALID_PARAMETER);

    if (m_encodeCtx->codecFunction != CODECHAL_FUNCTION_FEI_PRE_ENC)
    {
        DDI_ASSERTMESSAGE("PRE ENC output buffers status checking is not allowed for non-PRE_ENC case! .");
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }

    uint32_t numBuffers = (!preEncParams->bInterlaced) ? (2 - preEncParams->bDisableMVOutput - preEncParams->bDisableStatisticsOutput)
                                                       : (3 - preEncParams->bDisableMVOutput - 2 * preEncParams->bDisableStatisticsOutput);
    int32_t i = m_encodeCtx->statusReportBuf.ulHeadPosition;
    if ((m_encodeCtx->statusReportBuf.preencInfos[i].uiBuffers == numBuffers) &&
        m_encodeCtx->statusReportBuf.preencInfos[i].uiBuffers != 0)
    {
        m_encodeCtx->statusReportBuf.ulHeadPosition = (m_encodeCtx->statusReportBuf.ulHeadPosition + 1) % DDI_ENCODE_MAX_STATUS_REPORT_BUFFER;
    }

    return VA_STATUS_SUCCESS;
}

VAStatus DdiEncodeAvcFei::ParseMiscParams(void *ptr)
{
    DDI_CHK_NULL(m_encodeCtx, "nullptr m_encodeCtx", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(ptr, "nullptr ptr", VA_STATUS_ERROR_INVALID_PARAMETER);

    VAEncMiscParameterBuffer *miscParamBuf = (VAEncMiscParameterBuffer *)ptr;
    DDI_CHK_NULL(miscParamBuf->data, "nullptr miscParamBuf->data", VA_STATUS_ERROR_INVALID_PARAMETER);

    VAStatus status = MOS_STATUS_SUCCESS;
    switch ((int32_t)(miscParamBuf->type))
    {
    case VAEncMiscParameterTypeHRD:
        status = ParseMiscParamHRD((void *)miscParamBuf->data);
        break;

    case VAEncMiscParameterTypeFrameRate:
        status = ParseMiscParamFR((void *)miscParamBuf->data);
        break;

    case VAEncMiscParameterTypeRateControl:
        status = ParseMiscParamRC((void *)miscParamBuf->data);
        break;

    case VAEncMiscParameterTypeEncQuality:
        status = ParseMiscParamEncQuality((void *)miscParamBuf->data);
        break;

    case VAEncMiscParameterTypeQuantization:
        status = ParseMiscParamQuantization((void *)miscParamBuf->data);
        break;

    case VAEncMiscParameterTypeRIR:
        status = ParseMiscParameterRIR((void *)miscParamBuf->data);
        break;

    case VAEncMiscParameterTypeSkipFrame:
        status = ParseMiscParamSkipFrame((void *)miscParamBuf->data);
        break;

    case VAEncMiscParameterTypeMaxFrameSize:
        status = ParseMiscParamMaxFrameSize((void *)miscParamBuf->data);
        break;

    case VAEncMiscParameterTypeQualityLevel:
        status = ParseMiscParamQualityLevel((void *)miscParamBuf->data);
        break;

    case VAEncMiscParameterTypeMaxSliceSize:
        status = ParseMiscParamMaxSliceSize((void *)miscParamBuf->data);
        break;

    case VAEncMiscParameterTypeROI:
        status = ParseMiscParamROI((void *)miscParamBuf->data);
        break;

    case VAEncMiscParameterTypeFEIFrameControl:
        status = ParseMiscParamFeiPic((void *)miscParamBuf->data);
        break;

    case VAEncMiscParameterTypeDirtyRect:
        status = ParseMiscParamDirtyROI((void *)miscParamBuf->data);
        break;

    case VAEncMiscParameterTypeCustomRoundingControl:
        status = ParseMiscParamRounding((void *)miscParamBuf->data);
        break;

    case VAEncMiscParameterTypeSubMbPartPel:
        status = ParseMiscParamSubMbPartPel((void *)miscParamBuf->data);
        break;

    default:
        DDI_ASSERTMESSAGE("unsupported misc parameter type.");
        status = VA_STATUS_ERROR_INVALID_PARAMETER;
        break;
    }

    return status;
}

VAStatus DdiEncodeAvcFei::ParseStatsParams(PDDI_MEDIA_CONTEXT mediaCtx, void *ptr)
{
    if ((nullptr == ptr) || (nullptr == m_encodeCtx))
    {
        DDI_ASSERTMESSAGE("invalidate input parameters");
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }

    VAStatsStatisticsParameterH264 *statsParams  = (VAStatsStatisticsParameterH264 *)ptr;
    FeiPreEncParams *preEncParams  = (FeiPreEncParams*)(m_encodeCtx->pPreEncParams);
    DDI_CODEC_RENDER_TARGET_TABLE        *rtTbl        = &(m_encodeCtx->RTtbl);

    preEncParams->dwNumPastReferences      = statsParams->stats_params.num_past_references;
    preEncParams->dwNumFutureReferences    = statsParams->stats_params.num_future_references;
    preEncParams->dwFrameQp                = statsParams->frame_qp;
    preEncParams->dwLenSP                  = statsParams->len_sp;
    preEncParams->dwSearchPath             = statsParams->search_path;
    preEncParams->dwSubMBPartMask          = statsParams->sub_mb_part_mask;
    preEncParams->dwIntraPartMask          = statsParams->intra_part_mask;
    preEncParams->dwSubPelMode             = statsParams->sub_pel_mode;
    preEncParams->dwInterSAD               = statsParams->inter_sad;
    preEncParams->dwIntraSAD               = statsParams->intra_sad;
    preEncParams->bAdaptiveSearch          = statsParams->adaptive_search;
    preEncParams->dwMVPredictorCtrl        = statsParams->mv_predictor_ctrl;
    preEncParams->bMBQp                    = statsParams->mb_qp;
    preEncParams->bFTEnable                = statsParams->ft_enable;
    preEncParams->dwRefWidth               = statsParams->ref_width;
    preEncParams->dwRefHeight              = statsParams->ref_height;
    preEncParams->dwSearchWindow           = statsParams->search_window;
    preEncParams->bDisableMVOutput         = statsParams->disable_mv_output;
    preEncParams->bDisableStatisticsOutput = statsParams->disable_statistics_output;
    preEncParams->bEnable8x8Statistics     = statsParams->enable_8x8_statistics;
    preEncParams->bInputUpdated            = false;
    preEncParams->bCurPicUpdated           = false;
    preEncParams->bPastRefUpdated          = false;
    preEncParams->bFutureRefUpdated        = false;
    preEncParams->bPastRefStatsNeeded      = false;
    preEncParams->bFutureRefStatsNeeded    = false;

    if (statsParams->stats_params.input.picture_id == VA_INVALID_ID)
    {
        DDI_ASSERTMESSAGE("invalidate input parameters, current picture id is invalidate");
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }

    if ((statsParams->stats_params.input.flags & VA_PICTURE_STATS_CONTENT_UPDATED) != 0)
    {
        preEncParams->bCurPicUpdated = true;
    }

    CODEC_PICTURE_FLAG picFlags = ((statsParams->stats_params.input.flags & 0xF) == VA_PICTURE_STATS_PROGRESSIVE) ? PICTURE_FRAME : (((statsParams->stats_params.input.flags & 0xF) == VA_PICTURE_STATS_TOP_FIELD) ? PICTURE_TOP_FIELD : (((statsParams->stats_params.input.flags & 0xF) == VA_PICTURE_STATS_BOTTOM_FIELD) ? PICTURE_BOTTOM_FIELD : PICTURE_INVALID));
    preEncParams->bInterlaced   = (picFlags == PICTURE_TOP_FIELD || picFlags == PICTURE_BOTTOM_FIELD);

    DDI_MEDIA_SURFACE *currentSurface = DdiMedia_GetSurfaceFromVASurfaceID(mediaCtx, statsParams->stats_params.input.picture_id);
    if (nullptr == currentSurface)
    {
        DDI_ASSERTMESSAGE("invalidate current ref surface");
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }

    DDI_CHK_RET(RegisterRTSurfaces(&m_encodeCtx->RTtbl, currentSurface), "RegisterRTSurfaces failed!");
    preEncParams->CurrOriginalPicture.FrameIdx = (uint8_t)GetRenderTargetID(rtTbl, currentSurface);
    preEncParams->CurrOriginalPicture.PicFlags = picFlags;
    DDI_MEDIA_BUFFER *mediaBuffer;
    if ((preEncParams->dwNumPastReferences == 1) && (statsParams->stats_params.past_references[0].picture_id != VA_INVALID_ID))
    {
        if ((statsParams->stats_params.past_references->flags & VA_PICTURE_STATS_CONTENT_UPDATED) != 0)
        {
            preEncParams->bPastRefUpdated = true;
        }
        DDI_MEDIA_SURFACE *pastRefSurface = (DDI_MEDIA_SURFACE *)DdiMedia_GetSurfaceFromVASurfaceID(mediaCtx, statsParams->stats_params.past_references->picture_id);
        if (nullptr == pastRefSurface)
        {
            DDI_ASSERTMESSAGE("invalidate Future ref surface");
            return VA_STATUS_ERROR_INVALID_PARAMETER;
        }
        DDI_CHK_RET(RegisterRTSurfaces(&m_encodeCtx->RTtbl, pastRefSurface), "RegisterRTSurfaces failed!");
        preEncParams->PastRefPicture.FrameIdx = (uint8_t)GetRenderTargetID(rtTbl, pastRefSurface);
        picFlags                              = ((statsParams->stats_params.past_references->flags & 0xF) == VA_PICTURE_STATS_PROGRESSIVE) ? PICTURE_FRAME : (((statsParams->stats_params.past_references->flags & 0xF) == VA_PICTURE_STATS_TOP_FIELD) ? PICTURE_TOP_FIELD : (((statsParams->stats_params.past_references->flags & 0xF) == VA_PICTURE_STATS_BOTTOM_FIELD) ? PICTURE_BOTTOM_FIELD : PICTURE_INVALID));
        preEncParams->PastRefPicture.PicFlags = picFlags;

        DdiMedia_MediaSurfaceToMosResource(pastRefSurface, &(preEncParams->sPastRefSurface.OsResource));
        preEncParams->sPastRefSurface.dwWidth  = preEncParams->sPastRefSurface.OsResource.iWidth;
        preEncParams->sPastRefSurface.dwHeight = preEncParams->sPastRefSurface.OsResource.iHeight;
        preEncParams->sPastRefSurface.dwPitch  = preEncParams->sPastRefSurface.OsResource.iPitch;
        preEncParams->sPastRefSurface.TileType = preEncParams->sPastRefSurface.OsResource.TileType;
        preEncParams->sPastRefSurface.TileModeGMM = preEncParams->sPastRefSurface.OsResource.TileModeGMM;
        preEncParams->sPastRefSurface.bGMMTileEnabled = preEncParams->sPastRefSurface.OsResource.bGMMTileEnabled;
        preEncParams->bPastRefStatsNeeded      = false;

        if (statsParams->stats_params.past_ref_stat_buf)
        {
            if (!preEncParams->bDisableStatisticsOutput)
            {
                mediaBuffer = DdiMedia_GetBufferFromVABufferID(mediaCtx, *(statsParams->stats_params.past_ref_stat_buf));
                if (nullptr == mediaBuffer)
                {
                    DDI_ASSERTMESSAGE("invalidate statistics output media buffer for Past ref");
                    return VA_STATUS_ERROR_INVALID_PARAMETER;
                }
                DdiMedia_MediaBufferToMosResource(mediaBuffer, &(preEncParams->sPastRefStatsBuffer));
                if (picFlags == PICTURE_TOP_FIELD || picFlags == PICTURE_BOTTOM_FIELD)
                {
                    mediaBuffer = DdiMedia_GetBufferFromVABufferID(mediaCtx, *(statsParams->stats_params.past_ref_stat_buf + 1));
                    if (nullptr == mediaBuffer)
                    {
                        DDI_ASSERTMESSAGE("invalidate statistics output media buffer for Past bottom field ref");
                        return VA_STATUS_ERROR_INVALID_PARAMETER;
                    }
                    DdiMedia_MediaBufferToMosResource(mediaBuffer, &(preEncParams->sPastRefStatsBotFieldBuffer));
                }
                preEncParams->bPastRefStatsNeeded = true;
            }
            else
                preEncParams->bPastRefStatsNeeded = false;
        }
    }
    else
    {
        preEncParams->PastRefPicture.FrameIdx = (uint8_t)DDI_CODEC_INVALID_FRAME_INDEX;
        preEncParams->PastRefPicture.PicFlags = PICTURE_INVALID;
    }

    if ((preEncParams->dwNumFutureReferences == 1) && (statsParams->stats_params.future_references[0].picture_id != VA_INVALID_ID))
    {
        if ((statsParams->stats_params.future_references->flags & VA_PICTURE_STATS_CONTENT_UPDATED) != 0)
        {
            preEncParams->bFutureRefUpdated = true;
        }
        DDI_MEDIA_SURFACE *futureRefSurface = (DDI_MEDIA_SURFACE *)DdiMedia_GetSurfaceFromVASurfaceID(mediaCtx, statsParams->stats_params.future_references->picture_id);
        if (nullptr == futureRefSurface)
        {
            DDI_ASSERTMESSAGE("invalidate Future ref surface");
            return VA_STATUS_ERROR_INVALID_PARAMETER;
        }
        DDI_CHK_RET(RegisterRTSurfaces(&m_encodeCtx->RTtbl, futureRefSurface), "RegisterRTSurfaces failed!");
        preEncParams->FutureRefPicture.FrameIdx = (uint8_t)GetRenderTargetID(rtTbl, futureRefSurface);
        picFlags                                = ((statsParams->stats_params.future_references->flags & 0xF) == VA_PICTURE_STATS_PROGRESSIVE) ? PICTURE_FRAME : (((statsParams->stats_params.future_references->flags & 0xF) == VA_PICTURE_STATS_TOP_FIELD) ? PICTURE_TOP_FIELD : (((statsParams->stats_params.future_references->flags & 0xF) == VA_PICTURE_STATS_BOTTOM_FIELD) ? PICTURE_BOTTOM_FIELD : PICTURE_INVALID));
        preEncParams->FutureRefPicture.PicFlags = picFlags;

        DdiMedia_MediaSurfaceToMosResource(futureRefSurface, &(preEncParams->sFutureRefSurface.OsResource));
        preEncParams->sFutureRefSurface.dwWidth  = preEncParams->sFutureRefSurface.OsResource.iWidth;
        preEncParams->sFutureRefSurface.dwHeight = preEncParams->sFutureRefSurface.OsResource.iHeight;
        preEncParams->sFutureRefSurface.dwPitch  = preEncParams->sFutureRefSurface.OsResource.iPitch;
        preEncParams->sFutureRefSurface.TileType = preEncParams->sFutureRefSurface.OsResource.TileType;
        preEncParams->bFutureRefStatsNeeded      = false;
        if (!preEncParams->bDisableStatisticsOutput)
        {
            if (statsParams->stats_params.future_ref_stat_buf)
            {
                mediaBuffer = DdiMedia_GetBufferFromVABufferID(mediaCtx, *(statsParams->stats_params.future_ref_stat_buf));
                if (nullptr == mediaBuffer)
                {
                    DDI_ASSERTMESSAGE("invalidate statistics output media buffer for Future ref");
                    return VA_STATUS_ERROR_INVALID_PARAMETER;
                }
                DdiMedia_MediaBufferToMosResource(mediaBuffer, &(preEncParams->sFutureRefStatsBuffer));
                if (picFlags == PICTURE_TOP_FIELD || picFlags == PICTURE_BOTTOM_FIELD)
                {
                    mediaBuffer = DdiMedia_GetBufferFromVABufferID(mediaCtx, *(statsParams->stats_params.future_ref_stat_buf + 1));
                    if (nullptr == mediaBuffer)
                    {
                        DDI_ASSERTMESSAGE("invalidate statistics output media buffer for Future ref");
                        return VA_STATUS_ERROR_INVALID_PARAMETER;
                    }
                    DdiMedia_MediaBufferToMosResource(mediaBuffer, &(preEncParams->sFutureRefStatsBotFieldBuffer));
                }
                preEncParams->bFutureRefStatsNeeded = true;
            }
            else
                preEncParams->bFutureRefStatsNeeded = false;
        }
    }
    else
    {
        preEncParams->FutureRefPicture.FrameIdx = (uint8_t)DDI_CODEC_INVALID_FRAME_INDEX;
        preEncParams->FutureRefPicture.PicFlags = PICTURE_INVALID;
    }

    if (preEncParams->dwMVPredictorCtrl)
    {
        mediaBuffer = DdiMedia_GetBufferFromVABufferID(mediaCtx, statsParams->stats_params.mv_predictor);
        if (nullptr == mediaBuffer)
        {
            DDI_ASSERTMESSAGE("invalidate mv_predictor media buffer");
            return VA_STATUS_ERROR_INVALID_PARAMETER;
        }
        DdiMedia_MediaBufferToMosResource(mediaBuffer, &(preEncParams->resMvPredBuffer));
    }
    if (preEncParams->bMBQp)
    {
        mediaBuffer = DdiMedia_GetBufferFromVABufferID(mediaCtx, statsParams->stats_params.qp);
        if (nullptr == mediaBuffer)
        {
            DDI_ASSERTMESSAGE("invalidate qp media buffer");
            return VA_STATUS_ERROR_INVALID_PARAMETER;
        }
        DdiMedia_MediaBufferToMosResource(mediaBuffer, &(preEncParams->resMbQpBuffer));
    }

    uint32_t i = 0;
    if (!preEncParams->bDisableMVOutput)
    {
        mediaBuffer = DdiMedia_GetBufferFromVABufferID(mediaCtx, statsParams->stats_params.outputs[i++]);
        if (nullptr == mediaBuffer)
        {
            DDI_ASSERTMESSAGE("invalidate mv output media buffer");
            return VA_STATUS_ERROR_INVALID_PARAMETER;
        }
        DdiMedia_MediaBufferToMosResource(mediaBuffer, &(preEncParams->resMvBuffer));
        RemoveFromPreEncStatusReportQueue(mediaBuffer, PRE_ENC_BUFFER_TYPE_MVDATA);
        if (VA_STATUS_SUCCESS != AddToPreEncStatusReportQueue((void *)(preEncParams->resMvBuffer.bo), PRE_ENC_BUFFER_TYPE_MVDATA))
        {
            DDI_ASSERTMESSAGE("preEncParams->resMvBuffer is invalid for PREENC only");
            return VA_STATUS_ERROR_INVALID_PARAMETER;
        }
    }

    if (!preEncParams->bDisableStatisticsOutput)
    {
        mediaBuffer = DdiMedia_GetBufferFromVABufferID(mediaCtx, statsParams->stats_params.outputs[i++]);
        if (nullptr == mediaBuffer)
        {
            DDI_ASSERTMESSAGE("invalidate statistics output media buffer");
            return VA_STATUS_ERROR_INVALID_PARAMETER;
        }
        DdiMedia_MediaBufferToMosResource(mediaBuffer, &(preEncParams->resStatsBuffer));
        RemoveFromPreEncStatusReportQueue(mediaBuffer, PRE_ENC_BUFFER_TYPE_STATS);
        if (VA_STATUS_SUCCESS != AddToPreEncStatusReportQueue((void *)(preEncParams->resStatsBuffer.bo), PRE_ENC_BUFFER_TYPE_STATS))
        {
            DDI_ASSERTMESSAGE("preEncParams->resStatsBuffer is invalid for PREENC only");
            return VA_STATUS_ERROR_INVALID_PARAMETER;
        }

        if (preEncParams->bInterlaced)
        {
            mediaBuffer = DdiMedia_GetBufferFromVABufferID(mediaCtx, statsParams->stats_params.outputs[i++]);
            if (nullptr == mediaBuffer)
            {
                DDI_ASSERTMESSAGE("invalidate statistics output media bottom field buffer, must provide bottom field statistics buffer for both top and bottom field PreENC");
                return VA_STATUS_ERROR_INVALID_PARAMETER;
            }
            DdiMedia_MediaBufferToMosResource(mediaBuffer, &(preEncParams->resStatsBotFieldBuffer));
            RemoveFromPreEncStatusReportQueue(mediaBuffer, PRE_ENC_BUFFER_TYPE_STATS_BOT);
            if (VA_STATUS_SUCCESS != AddToPreEncStatusReportQueue((void *)(preEncParams->resStatsBotFieldBuffer.bo), PRE_ENC_BUFFER_TYPE_STATS_BOT))
            {
                DDI_ASSERTMESSAGE("preEncParams->resStatsBotFieldBuffer is invalid for PREENC only");
                return VA_STATUS_ERROR_INVALID_PARAMETER;
            }
        }
    }

    AddToPreEncStatusReportQueueUpdatePos();

    return VA_STATUS_SUCCESS;
}
