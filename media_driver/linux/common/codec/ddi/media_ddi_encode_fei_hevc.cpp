/*
* Copyright (c) 2018, Intel Corporation
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
//! \file     media_ddi_encode_fei_hevc.cpp
//! \brief    Implements class for DDI media hevc fei encode
//!

#include "media_libva.h"
#include "media_libva_encoder.h"
#include "media_libva_util.h"
#include "hwinfo_linux.h"
#include "codechal_encode_hevc_base.h"
#include "media_ddi_encode_base.h"
#include "media_ddi_encode_fei_hevc.h"
#include "media_ddi_encode_const.h"
#include "media_ddi_factory.h"

extern template class MediaDdiFactoryNoArg<DdiEncodeBase>;

static bool isEncodeHevcFeiRegistered =
    MediaDdiFactoryNoArg<DdiEncodeBase>::RegisterCodec<DdiEncodeHevcFei>(ENCODE_ID_HEVCFEI);

DdiEncodeHevcFei::~DdiEncodeHevcFei()
{
    if (nullptr == m_encodeCtx)
    {
        return;
    }
    MOS_FreeMemory(m_encodeCtx->pFeiPicParams);
    m_encodeCtx->pFeiPicParams = nullptr;

}

VAStatus DdiEncodeHevcFei::ContextInitialize(CodechalSetting * codecHalSettings)
{
    VAStatus status = DdiEncodeHevc::ContextInitialize(codecHalSettings);
    if (VA_STATUS_SUCCESS != status)
    {
        return status;
    }

    codecHalSettings->codecFunction = m_encodeCtx->codecFunction;

    m_encodeCtx->pFeiPicParams = (void *)MOS_AllocAndZeroMemory(CODECHAL_HEVC_MAX_PPS_NUM * sizeof(CodecEncodeHevcFeiPicParams));
    DDI_CHK_NULL(m_encodeCtx->pFeiPicParams, "nullptr m_encodeCtx->pFeiPicParams", VA_STATUS_ERROR_ALLOCATION_FAILED);

    return VA_STATUS_SUCCESS;
}

VAStatus DdiEncodeHevcFei::EncodeInCodecHal(uint32_t numSlices)
{
    DDI_CHK_NULL(m_encodeCtx, "nullptr m_encodeCtx", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(m_encodeCtx->pCodecHal, "nullptr m_encodeCtx->pCodecHal", VA_STATUS_ERROR_INVALID_PARAMETER);

    DDI_CODEC_RENDER_TARGET_TABLE *rtTbl = &(m_encodeCtx->RTtbl);

    EncoderParams encodeParams;
    MOS_ZeroMemory(&encodeParams, sizeof(encodeParams));

    CodecEncodeHevcFeiPicParams *feiPicParams = (CodecEncodeHevcFeiPicParams *)(m_encodeCtx->pFeiPicParams);

    if(CodecHalIsFeiEncode(m_encodeCtx->codecFunction))
    {
        encodeParams.ExecCodecFunction = m_encodeCtx->codecFunction;
    }
    else
    {
        DDI_ASSERTMESSAGE("DDI:Failed in HEVC FEI Function check!");
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }

    // Raw Surface
    MOS_SURFACE rawSurface;
    MOS_ZeroMemory(&rawSurface, sizeof(rawSurface));
    rawSurface.dwOffset = 0;
    if (m_encodeCtx->vaProfile == VAProfileHEVCMain10)
    {
        rawSurface.Format = Format_P010;
    }
    else  //VAProfileHEVCMain
    {
        rawSurface.Format = Format_NV12;
    }

    DdiMedia_MediaSurfaceToMosResource(rtTbl->pCurrentRT, &(rawSurface.OsResource));

    // Recon Surface
    MOS_SURFACE reconSurface;
    MOS_ZeroMemory(&reconSurface, sizeof(reconSurface));
    reconSurface.dwOffset = 0;
    if (m_encodeCtx->vaProfile == VAProfileHEVCMain10)
    {
        reconSurface.Format = Format_P010;
    }
    else  //VAProfileHEVCMain
    {
        reconSurface.Format = Format_NV12;
    }

    DdiMedia_MediaSurfaceToMosResource(rtTbl->pCurrentReconTarget, &(reconSurface.OsResource));

    //clear registered recon/ref surface flags
    DDI_CHK_RET(ClearRefList(&m_encodeCtx->RTtbl, true), "ClearRefList failed!");

    // Bitstream surface
    MOS_RESOURCE bitstreamSurface;
    MOS_ZeroMemory(&bitstreamSurface, sizeof(bitstreamSurface));
    bitstreamSurface        = m_encodeCtx->resBitstreamBuffer;  // in render picture
    bitstreamSurface.Format = Format_Buffer;

    encodeParams.psRawSurface        = &rawSurface;
    encodeParams.psReconSurface      = &reconSurface;
    encodeParams.presBitstreamBuffer = &bitstreamSurface;

    MOS_SURFACE mbQpSurface;
    if(feiPicParams->bPerBlockQP)
    {
        // MBQp surface
        MOS_ZeroMemory(&mbQpSurface, sizeof(mbQpSurface));
        mbQpSurface.Format     = Format_Buffer_2D;
        mbQpSurface.dwOffset   = 0;
        mbQpSurface.OsResource = feiPicParams->resCTBQp;

        encodeParams.psMbQpDataSurface = &mbQpSurface;
        encodeParams.bMbQpDataEnabled  = true;
    }

    encodeParams.pSeqParams   = m_encodeCtx->pSeqParams;
    encodeParams.pVuiParams   = m_encodeCtx->pVuiParams;
    encodeParams.pPicParams   = m_encodeCtx->pPicParams;
    encodeParams.pSliceParams = m_encodeCtx->pSliceParams;
    encodeParams.pFeiPicParams = feiPicParams;

    // Sequence data
    encodeParams.bNewSeq = m_encodeCtx->bNewSeq;

    // VUI
    encodeParams.bNewVuiData = m_encodeCtx->bNewVuiData;

    // Slice level data
    encodeParams.dwNumSlices = numSlices;

    // IQmatrix params
    encodeParams.bNewQmatrixData = m_encodeCtx->bNewQmatrixData;
    encodeParams.bPicQuant       = m_encodeCtx->bPicQuant;
    encodeParams.ppNALUnitParams = m_encodeCtx->ppNALUnitParams;
    encodeParams.pSeiData        = m_encodeCtx->pSEIFromApp;
    encodeParams.pSeiParamBuffer = m_encodeCtx->pSEIFromApp->pSEIBuffer;
    encodeParams.dwSEIDataOffset = 0;

    CODECHAL_HEVC_IQ_MATRIX_PARAMS hevcIqMatrixParams;
    encodeParams.pIQMatrixBuffer = &hevcIqMatrixParams;

    // whether driver need to pack slice header
    if (m_encodeCtx->bHavePackedSliceHdr)
    {
        encodeParams.bAcceleratorHeaderPackingCaps = false;
    }
    else
    {
        encodeParams.bAcceleratorHeaderPackingCaps = true;
    }

    encodeParams.pBSBuffer      = m_encodeCtx->pbsBuffer;
    encodeParams.pSlcHeaderData = (void *)m_encodeCtx->pSliceHeaderData;

    MOS_STATUS status = m_encodeCtx->pCodecHal->Execute(&encodeParams);
    if (MOS_STATUS_SUCCESS != status)
    {
        DDI_ASSERTMESSAGE("DDI:Failed in Codechal!");
        return VA_STATUS_ERROR_ENCODING_ERROR;
    }

    return VA_STATUS_SUCCESS;

}

VAStatus DdiEncodeHevcFei::ResetAtFrameLevel()
{
    DDI_CHK_NULL(m_encodeCtx, "nullptr m_encodeCtx", VA_STATUS_ERROR_INVALID_PARAMETER);

    // Assume there is only one SPS parameter
    PCODEC_HEVC_ENCODE_SEQUENCE_PARAMS hevcSeqParams = (PCODEC_HEVC_ENCODE_SEQUENCE_PARAMS)(m_encodeCtx->pSeqParams);
    hevcSeqParams->bResetBRC                         = 0x0;

    m_encodeCtx->dwNumSlices      = 0x0;
    m_encodeCtx->indexNALUnit     = 0x0;
    m_encodeCtx->uiSliceHeaderCnt = 0x0;

    // reset bsbuffer every frame
    m_encodeCtx->pbsBuffer->pCurrent    = m_encodeCtx->pbsBuffer->pBase;
    m_encodeCtx->pbsBuffer->SliceOffset = 0x0;
    m_encodeCtx->pbsBuffer->BitOffset   = 0x0;
    m_encodeCtx->pbsBuffer->BitSize     = 0x0;

    CodecEncodeHevcFeiPicParams *feiPicParams = (CodecEncodeHevcFeiPicParams *)(m_encodeCtx->pFeiPicParams);

    m_encodeCtx->codecFunction              = CODECHAL_FUNCTION_FEI_ENC_PAK;
    feiPicParams->NumMVPredictorsL0       = 0;     // number of MV Predictors L0 provided, max is 4
    feiPicParams->NumMVPredictorsL1       = 0;     // number of MV Predictors L1 provided, max is 2
    feiPicParams->SearchPath              = 0;     // search path, default is 0, 0 and 2 mean full search, 1 means diamond search
    feiPicParams->LenSP                   = 57;    // max number of SUs per reference which is evaluated by the predetermined SUs, range is [1, 63]
    feiPicParams->MultiPredL0             = 0;     // 000: no neighbor Motion Vectors will be used as predictor for L0, 001: spatial Motion Vectors, 010 temporal Motion Vectors, others: reserved.
    feiPicParams->MultiPredL1             = 0;     // 000: no neighbor Motion Vectors will be used as predictor for L1, 001: spatial Motion Vectors, 010 temporal Motion Vectors, others: reserved.
    feiPicParams->SubPelMode              = 3;     // half/quater pixels mode, 00b integer mode search, 01b half mode search, 11b quater mode search
    feiPicParams->AdaptiveSearch          = true;  // whether adaptive searching is enabled for IME
    feiPicParams->MVPredictorInput        = 0;     // 000: disable MV Predictor input, 001: enabled per 16x16 block,
                                                    // 010: enabled per 32x32 block,
                                                    // 011: enabled per 64x64 block,
                                                    // 111: block size can vary and is determined by BlockSize in MVP
                                                    // others: reserved
    feiPicParams->bPerBlockQP             = false; // if enable per block QP input
    feiPicParams->bPerCTBInput            = false; // if enable per MB control/special input
    feiPicParams->bForceLCUSplit          = false; // specifies whether CTB should be forced to split to remove Inter big LCU
    feiPicParams->bEnableCU64Check        = true;  // specifies whether 64x64 CB should be forced to split
    feiPicParams->bEnableCU64AmpCheck     = false; // specifies if 64x64 CB AMP should be checked, 0: No CU 64x64 AMP check, 1: check LCU64x64 AMP
    feiPicParams->bCU64SkipCheckOnly      = false; // specifies if check the 64x64 merge candidate, 0: after skip check, also run merge for TU1, 1: only skip check for 64x64 for TU4
    feiPicParams->bColocatedCTBDistortion = false; // if enable, extra distortion between current CTB and co-located CTB is provided
    feiPicParams->RefWidth                = 48;
    feiPicParams->RefHeight               = 40;
    feiPicParams->SearchWindow            = 0;
    feiPicParams->MaxNumIMESearchCenter   = 6;     // specifies number [1~6] of MV predictors for IME searches
    feiPicParams->NumConcurrentEncFramePartition = 1;    // specifies number [1, 2, 4] of splits that encoder could be run concurrently
    feiPicParams->dwMaxFrameSize          = 0;     // specifies max frame size in bytes for multi-pass pak, 0 means disabling multi-pass pak

    feiPicParams->bDistortionEnable       = false;
    feiPicParams->bCTBCmdCuRecordEnable   = false;

    // clear the packed header information
    if (nullptr != m_encodeCtx->ppNALUnitParams)
    {
        MOS_ZeroMemory(m_encodeCtx->ppNALUnitParams[0], sizeof(CODECHAL_NAL_UNIT_PARAMS) * HEVC_MAX_NAL_UNIT_TYPE);
    }

    m_encodeCtx->bHavePackedSliceHdr   = false;
    m_encodeCtx->bLastPackedHdrIsSlice = false;
    m_encodeCtx->bMBQpEnable           = false;

    return VA_STATUS_SUCCESS;

}

VAStatus DdiEncodeHevcFei::RenderPicture(VADriverContextP ctx, VAContextID context, VABufferID *buffers, int32_t numBuffers)
{
    VAStatus vaStatus = VA_STATUS_SUCCESS;

    DDI_FUNCTION_ENTER();

    DDI_CHK_NULL(ctx, "nullptr context", VA_STATUS_ERROR_INVALID_CONTEXT);

    DDI_MEDIA_CONTEXT *mediaCtx = DdiMedia_GetMediaContext(ctx);
    DDI_CHK_NULL(mediaCtx, "Null mediaCtx", VA_STATUS_ERROR_INVALID_CONTEXT);

    DDI_CHK_NULL(m_encodeCtx, "Null m_encodeCtx", VA_STATUS_ERROR_INVALID_CONTEXT);

    for (int32_t i = 0; i < numBuffers; i++)
    {
        DDI_MEDIA_BUFFER *buf = DdiMedia_GetBufferFromVABufferID(mediaCtx, buffers[i]);
        DDI_CHK_NULL(buf, "Invalid buffer.", VA_STATUS_ERROR_INVALID_BUFFER);
        if (buf->uiType == VAEncMacroblockDisableSkipMapBufferType)
        {
            DdiMedia_MediaBufferToMosResource(buf, &(m_encodeCtx->resPerMBSkipMapBuffer));
            m_encodeCtx->bMbDisableSkipMapEnabled = true;
            continue;
        }
        uint32_t dataSize = buf->iSize;
        // can use internal function instead of DdiMedia_MapBuffer here?
        void *data = nullptr;
        DdiMedia_MapBuffer(ctx, buffers[i], &data);

        DDI_CHK_NULL(data, "Null data.", VA_STATUS_ERROR_INVALID_BUFFER);

        switch (buf->uiType)
        {
        case VAEncSequenceParameterBufferType:
            DDI_CHK_STATUS(ParseSeqParams(data), VA_STATUS_ERROR_INVALID_BUFFER);
            m_encodeCtx->bNewSeq = true;
            break;

        case VAEncPictureParameterBufferType:
            DDI_CHK_STATUS(ParsePicParams(mediaCtx, data), VA_STATUS_ERROR_INVALID_BUFFER);
            DDI_CHK_STATUS(
                    AddToStatusReportQueue((void *)m_encodeCtx->resBitstreamBuffer.bo),
                    VA_STATUS_ERROR_INVALID_BUFFER);
            break;

        case VAEncSliceParameterBufferType:
        {
            uint32_t numSlices = buf->uiNumElements;
            DDI_CHK_STATUS(ParseSlcParams(mediaCtx, data, numSlices), VA_STATUS_ERROR_INVALID_BUFFER);
            break;
        }

        case VAEncPackedHeaderParameterBufferType:
            vaStatus = ParsePackedHeaderParams(data);
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

        case VAEncFEIMBControlBufferType:
        case VAEncFEIMVPredictorBufferType:
        case VAEncFEIMBCodeBufferType:
        case VAEncFEICTBCmdBufferType:
        case VAEncFEICURecordBufferType:
        case VAEncFEIDistortionBufferType:
            {
                // handled in VAEncMiscParameterBufferType/VAEncMiscParameterTypeFEIFrameControlIntel case by vaBufferID
                break;
            }

        default:
            DDI_ASSERTMESSAGE("not supported buffer type.");
            break;
        }
        DdiMedia_UnmapBuffer(ctx, buffers[i]);
    }

    DDI_FUNCTION_EXIT(vaStatus);
    return vaStatus;

}

VAStatus DdiEncodeHevcFei::ParseMiscParamFeiPic(void *data)
{
    DDI_CHK_NULL(data, "nullptr data", VA_STATUS_ERROR_INVALID_PARAMETER);
    CodecEncodeHevcFeiPicParams *feiPicParams = (CodecEncodeHevcFeiPicParams *)(m_encodeCtx->pFeiPicParams);
    DDI_CHK_NULL(feiPicParams, "nullptr feiPicParams", VA_STATUS_ERROR_INVALID_PARAMETER);
    VAEncMiscParameterFEIFrameControlHEVC *vaEncMiscParamFeiPic = (VAEncMiscParameterFEIFrameControlHEVC *)data;

    m_encodeCtx->codecFunction     = CODECHAL_FUNCTION_INVALID;
    if(vaEncMiscParamFeiPic->function & VA_FEI_FUNCTION_ENC_PAK)
        m_encodeCtx->codecFunction = CODECHAL_FUNCTION_FEI_ENC_PAK;
    if(vaEncMiscParamFeiPic->function == VA_FEI_FUNCTION_ENC)
        m_encodeCtx->codecFunction = CODECHAL_FUNCTION_FEI_ENC;
    if(vaEncMiscParamFeiPic->function == VA_FEI_FUNCTION_PAK)
        m_encodeCtx->codecFunction = CODECHAL_FUNCTION_FEI_PAK;

    feiPicParams->NumMVPredictorsL0       = vaEncMiscParamFeiPic->num_mv_predictors_l0;
    feiPicParams->NumMVPredictorsL1       = vaEncMiscParamFeiPic->num_mv_predictors_l1;
    feiPicParams->SearchPath              = vaEncMiscParamFeiPic->search_path;
    feiPicParams->LenSP                   = vaEncMiscParamFeiPic->len_sp;
    feiPicParams->MultiPredL0             = vaEncMiscParamFeiPic->multi_pred_l0;
    feiPicParams->MultiPredL1             = vaEncMiscParamFeiPic->multi_pred_l1;
    feiPicParams->SubPelMode              = vaEncMiscParamFeiPic->sub_pel_mode;
    feiPicParams->AdaptiveSearch          = vaEncMiscParamFeiPic->adaptive_search;
    feiPicParams->MVPredictorInput        = vaEncMiscParamFeiPic->mv_predictor_input;
    feiPicParams->bPerBlockQP             = vaEncMiscParamFeiPic->per_block_qp;
    feiPicParams->bPerCTBInput            = vaEncMiscParamFeiPic->per_ctb_input;
    feiPicParams->bColocatedCTBDistortion = vaEncMiscParamFeiPic->colocated_ctb_distortion;
    feiPicParams->bForceLCUSplit          = vaEncMiscParamFeiPic->force_lcu_split;
    feiPicParams->bEnableCU64Check        = vaEncMiscParamFeiPic->enable_cu64_check;
    feiPicParams->bEnableCU64AmpCheck     = vaEncMiscParamFeiPic->enable_cu64_amp_check;
    feiPicParams->bCU64SkipCheckOnly      = vaEncMiscParamFeiPic->cu64_skip_check_only;
    feiPicParams->RefWidth                = vaEncMiscParamFeiPic->ref_width;
    feiPicParams->RefHeight               = vaEncMiscParamFeiPic->ref_height;
    feiPicParams->SearchWindow            = vaEncMiscParamFeiPic->search_window;
    feiPicParams->MaxNumIMESearchCenter   = vaEncMiscParamFeiPic->max_num_ime_search_center;
    feiPicParams->FastIntraMode           = vaEncMiscParamFeiPic->fast_intra_mode;
    feiPicParams->NumConcurrentEncFramePartition = vaEncMiscParamFeiPic->num_concurrent_enc_frame_partition;
    feiPicParams->dwMaxFrameSize          = vaEncMiscParamFeiPic->max_frame_size;

    DDI_MEDIA_BUFFER *mediaBuffer;
    VAStatus          status = VA_STATUS_SUCCESS;
    if(feiPicParams->bPerCTBInput)
    {
        mediaBuffer = DdiMedia_GetBufferFromVABufferID(m_encodeCtx->pMediaCtx, vaEncMiscParamFeiPic->ctb_ctrl);
        DDI_CHK_NULL(mediaBuffer, "nullptr mediaBuffer", VA_STATUS_ERROR_INVALID_PARAMETER);
        DdiMedia_MediaBufferToMosResource(mediaBuffer, &(feiPicParams->resCTBCtrl));
    }
    if(feiPicParams->MVPredictorInput)
    {
        mediaBuffer = DdiMedia_GetBufferFromVABufferID(m_encodeCtx->pMediaCtx, vaEncMiscParamFeiPic->mv_predictor);
        DDI_CHK_NULL(mediaBuffer, "nullptr mediaBuffer", VA_STATUS_ERROR_INVALID_PARAMETER);
        DdiMedia_MediaBufferToMosResource(mediaBuffer, &(feiPicParams->resMVPredictor));
    }
    else if((feiPicParams->NumMVPredictorsL0 !=0) || (feiPicParams->NumMVPredictorsL1 != 0))
    {
        DDI_ASSERTMESSAGE("feiPicParams->NumMVPredictorsL0 and NumMVPredictorsL1 should be set to 0 when feiPicParams->MVPredictorInput is false!");
        status = VA_STATUS_ERROR_INVALID_PARAMETER;
    }
    if(feiPicParams->bPerBlockQP)
    {
        mediaBuffer = DdiMedia_GetBufferFromVABufferID(m_encodeCtx->pMediaCtx, vaEncMiscParamFeiPic->qp);
        DDI_CHK_NULL(mediaBuffer, "nullptr mediaBuffer", VA_STATUS_ERROR_INVALID_PARAMETER);
        DdiMedia_MediaBufferToMosResource(mediaBuffer, &(feiPicParams->resCTBQp));
    }

    feiPicParams->bCTBCmdCuRecordEnable = false;
    if(vaEncMiscParamFeiPic->ctb_cmd != VA_INVALID_ID)
    {
        feiPicParams->bCTBCmdCuRecordEnable = true;
        mediaBuffer = DdiMedia_GetBufferFromVABufferID(m_encodeCtx->pMediaCtx, vaEncMiscParamFeiPic->ctb_cmd);
        DDI_CHK_NULL(mediaBuffer, "nullptr mediaBuffer", VA_STATUS_ERROR_INVALID_PARAMETER);
        DdiMedia_MediaBufferToMosResource(mediaBuffer, &(feiPicParams->resCTBCmd));
        if(m_encodeCtx->codecFunction == CODECHAL_FUNCTION_FEI_ENC)
        {
            RemoveFromEncStatusReportQueue(mediaBuffer, FEI_ENC_BUFFER_TYPE_MVDATA);
            if( VA_STATUS_SUCCESS != AddToEncStatusReportQueue( (void *)(feiPicParams->resCTBCmd.bo), FEI_ENC_BUFFER_TYPE_MVDATA) )
            {
                DDI_ASSERTMESSAGE("feiPicParams->resCTBCmd is invalid for FEI ENC only");
                status = VA_STATUS_ERROR_INVALID_PARAMETER;
            }
        }
    }
    if(vaEncMiscParamFeiPic->cu_record != VA_INVALID_ID)
    {
        if(feiPicParams->bCTBCmdCuRecordEnable == false)
        {
            DDI_ASSERTMESSAGE("CTB cmd and CU record should be enabled or disabled together!");
            status = VA_STATUS_ERROR_INVALID_PARAMETER;
        }
        mediaBuffer = DdiMedia_GetBufferFromVABufferID(m_encodeCtx->pMediaCtx, vaEncMiscParamFeiPic->cu_record);
        DDI_CHK_NULL(mediaBuffer, "nullptr mediaBuffer", VA_STATUS_ERROR_INVALID_PARAMETER);
        DdiMedia_MediaBufferToMosResource(mediaBuffer, &(feiPicParams->resCURecord));
        if(m_encodeCtx->codecFunction == CODECHAL_FUNCTION_FEI_ENC)
        {
            RemoveFromEncStatusReportQueue(mediaBuffer, FEI_ENC_BUFFER_TYPE_MBCODE);
            if( VA_STATUS_SUCCESS != AddToEncStatusReportQueue((void *)(feiPicParams->resCURecord.bo), FEI_ENC_BUFFER_TYPE_MBCODE) )
            {
                DDI_ASSERTMESSAGE("feiPicParams->resCURecord is invalid for FEI ENC only");
                status = VA_STATUS_ERROR_INVALID_PARAMETER;
            }
        }
    }

    if(vaEncMiscParamFeiPic->distortion != VA_INVALID_ID)
    {
        feiPicParams->bDistortionEnable = true;
        mediaBuffer = DdiMedia_GetBufferFromVABufferID(m_encodeCtx->pMediaCtx, vaEncMiscParamFeiPic->distortion);
        DDI_CHK_NULL(mediaBuffer, "nullptr mediaBuffer", VA_STATUS_ERROR_INVALID_PARAMETER);
        DdiMedia_MediaBufferToMosResource(mediaBuffer, &(feiPicParams->resDistortion));
        if(m_encodeCtx->codecFunction == CODECHAL_FUNCTION_FEI_ENC)
        {
            RemoveFromEncStatusReportQueue(mediaBuffer, FEI_ENC_BUFFER_TYPE_DISTORTION);
            if( VA_STATUS_SUCCESS != AddToEncStatusReportQueue((void *)(feiPicParams->resDistortion.bo), FEI_ENC_BUFFER_TYPE_DISTORTION) )
            {
                DDI_ASSERTMESSAGE("feiPicParams->resDistortion is invalid for FEI ENC only");
                status = VA_STATUS_ERROR_INVALID_PARAMETER;
            }
        }
    }
    if(m_encodeCtx->codecFunction == CODECHAL_FUNCTION_FEI_ENC)
    {
        AddToEncStatusReportQueueUpdatePos();
    }

    //add for mutlple pass pak
    feiPicParams->dwMaxFrameSize = vaEncMiscParamFeiPic->max_frame_size;
    if (feiPicParams->dwMaxFrameSize)
    {
        feiPicParams->dwNumPasses = vaEncMiscParamFeiPic->num_passes;
        if ((feiPicParams->dwNumPasses == 0) || (feiPicParams->dwNumPasses > feiHevcMaxPassesNum))
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

finish:
    return status;
}

VAStatus DdiEncodeHevcFei::AddToEncStatusReportQueue(
    void                           *encBuf,
    DDI_ENCODE_FEI_ENC_BUFFER_TYPE typeIdx)
{
    DDI_CHK_NULL(encBuf, "nullptr encBuf", VA_STATUS_ERROR_INVALID_PARAMETER);

    CodecEncodeHevcFeiPicParams *feiPicParams = (CodecEncodeHevcFeiPicParams *)(m_encodeCtx->pFeiPicParams);
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

VAStatus DdiEncodeHevcFei::AddToEncStatusReportQueueUpdatePos()
{
    CodecEncodeHevcFeiPicParams *feiPicParams = (CodecEncodeHevcFeiPicParams *)(m_encodeCtx->pFeiPicParams);
    DDI_CHK_NULL(feiPicParams, "nullptr feiPicParams", VA_STATUS_ERROR_INVALID_PARAMETER);

    if(m_encodeCtx->codecFunction != CODECHAL_FUNCTION_FEI_ENC)
    {
        DDI_ASSERTMESSAGE("ENC output buffers status checking is not allowed for non-FEI_ENC case! .");
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }

    int32_t i = m_encodeCtx->statusReportBuf.ulHeadPosition;
    if((m_encodeCtx->statusReportBuf.encInfos[i].uiBuffers == (feiPicParams->bCTBCmdCuRecordEnable * 2 + feiPicParams->bDistortionEnable)) &&  m_encodeCtx->statusReportBuf.encInfos[i].uiBuffers != 0)
    {
        m_encodeCtx->statusReportBuf.ulHeadPosition = (m_encodeCtx->statusReportBuf.ulHeadPosition + 1) % DDI_ENCODE_MAX_STATUS_REPORT_BUFFER;
    }

finish:
    return VA_STATUS_SUCCESS;
}

VAStatus DdiEncodeHevcFei::ParseMiscParams(void *ptr)
{
    DDI_CHK_NULL(m_encodeCtx, "nullptr m_encodeCtx", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(ptr, "nullptr ptr", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(m_encodeCtx->pSeqParams, "nullptr m_encodeCtx->pSeqParams", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(m_encodeCtx->pPicParams, "nullptr m_encodeCtx->pPicParams", VA_STATUS_ERROR_INVALID_PARAMETER);

    PCODEC_HEVC_ENCODE_SEQUENCE_PARAMS seqParams = (PCODEC_HEVC_ENCODE_SEQUENCE_PARAMS)(m_encodeCtx->pSeqParams);
    PCODEC_HEVC_ENCODE_PICTURE_PARAMS  picParams = (PCODEC_HEVC_ENCODE_PICTURE_PARAMS)(m_encodeCtx->pPicParams);

    VAEncMiscParameterBuffer *miscParamBuf = (VAEncMiscParameterBuffer *)ptr;
    DDI_CHK_NULL(miscParamBuf->data, "nullptr miscParamBuf->data", VA_STATUS_ERROR_INVALID_PARAMETER);

    switch ((int32_t)(miscParamBuf->type))
    {
    case VAEncMiscParameterTypeQualityLevel:
    {
        VAEncMiscParameterBufferQualityLevel *vaEncMiscParamQualityLevel = (VAEncMiscParameterBufferQualityLevel *)miscParamBuf->data;
        seqParams->TargetUsage                                           = (uint8_t)vaEncMiscParamQualityLevel->quality_level;

        // HEVC only supports TU=1, 4, and 7
        if (1 != seqParams->TargetUsage && 4 != seqParams->TargetUsage && 7 != seqParams->TargetUsage)
        {
            DDI_ASSERTMESSAGE("unsupported target usage in HEVC encoder.");
            return VA_STATUS_ERROR_INVALID_PARAMETER;
        }
        break;
    }
    case VAEncMiscParameterTypeHRD:
    {
        VAEncMiscParameterHRD *vaEncMiscParamHRD = (VAEncMiscParameterHRD *)miscParamBuf->data;
        seqParams->VBVBufferSizeInBit            = vaEncMiscParamHRD->buffer_size;
        seqParams->InitVBVBufferFullnessInBit    = vaEncMiscParamHRD->initial_buffer_fullness;
        seqParams->RateControlMethod             = RATECONTROL_CBR;
        break;
    }
    case VAEncMiscParameterTypeFrameRate:
    {
        VAEncMiscParameterFrameRate *vaEncMiscParamFR = (VAEncMiscParameterFrameRate *)miscParamBuf->data;
        seqParams->FrameRate.Numerator                = vaEncMiscParamFR->framerate & 0xffff;
        seqParams->FrameRate.Denominator              = (vaEncMiscParamFR->framerate >> 16 ) & 0xffff;
        if(seqParams->FrameRate.Denominator == 0)
        {
            seqParams->FrameRate.Denominator = 1;
        }
        break;
    }
    case VAEncMiscParameterTypeRateControl:
    {
        // Assume only one SPS here, modify when enable multiple SPS support
        VAEncMiscParameterRateControl *vaEncMiscParamRC = (VAEncMiscParameterRateControl *)miscParamBuf->data;

        seqParams->TargetBitRate = MOS_ROUNDUP_DIVIDE(vaEncMiscParamRC->bits_per_second, CODECHAL_ENCODE_BRC_KBPS);
        seqParams->MBBRC         = (vaEncMiscParamRC->rc_flags.bits.mb_rate_control <= mbBrcDisabled) ? vaEncMiscParamRC->rc_flags.bits.mb_rate_control : 0;
        //enable parallelBRC for Android and Linux
        seqParams->ParallelBRC = vaEncMiscParamRC->rc_flags.bits.enable_parallel_brc;

        // Assuming picParams are sent before MiscParams
        picParams->BRCMinQp = vaEncMiscParamRC->min_qp;
        picParams->BRCMaxQp = vaEncMiscParamRC->max_qp;

        if (VA_RC_NONE == m_encodeCtx->uiRCMethod || VA_RC_CQP == m_encodeCtx->uiRCMethod)
        {
            seqParams->RateControlMethod = RATECONTROL_CQP;
            seqParams->MBBRC             = 0;
        }
        else if ((VA_RC_CBR | VA_RC_MB) == m_encodeCtx->uiRCMethod || (VA_RC_CBR | VA_RC_MB | VA_RC_PARALLEL) == m_encodeCtx->uiRCMethod)
        {
            seqParams->MaxBitRate        = seqParams->TargetBitRate;
            seqParams->MinBitRate        = seqParams->TargetBitRate;
            seqParams->RateControlMethod = RATECONTROL_CBR;
        }
        else if (VA_RC_ICQ == m_encodeCtx->uiRCMethod || (VA_RC_ICQ | VA_RC_PARALLEL) == m_encodeCtx->uiRCMethod)
        {
            seqParams->ICQQualityFactor  = vaEncMiscParamRC->ICQ_quality_factor;
            seqParams->RateControlMethod = RATECONTROL_ICQ;
            seqParams->MBBRC             = 1;
        }
        else
        {
            switch (m_encodeCtx->uiRCMethod)
            {
            case (VA_RC_VCM | VA_RC_PARALLEL):
            case VA_RC_VCM:
                seqParams->RateControlMethod = RATECONTROL_VCM;
                seqParams->MBBRC             = 0;
                break;
            case (VA_RC_VBR | VA_RC_MB | VA_RC_PARALLEL):
            case (VA_RC_VBR | VA_RC_MB):
                seqParams->RateControlMethod = RATECONTROL_VBR;
                break;
            default:
                DDI_ASSERTMESSAGE("invalid RC method.");
                return VA_STATUS_ERROR_INVALID_PARAMETER;
            }

            seqParams->MaxBitRate    = seqParams->TargetBitRate;
            seqParams->MinBitRate    = seqParams->TargetBitRate * (2 * vaEncMiscParamRC->target_percentage - 100) / 100;
            seqParams->TargetBitRate = seqParams->TargetBitRate * vaEncMiscParamRC->target_percentage / 100;

            if ((m_encodeCtx->uiTargetBitRate != seqParams->TargetBitRate) ||
                (m_encodeCtx->uiMaxBitRate != seqParams->MaxBitRate))
            {
                if ((m_encodeCtx->uiTargetBitRate != 0) && (m_encodeCtx->uiMaxBitRate != 0))
                {
                    seqParams->bResetBRC = 0x1;
                }
                m_encodeCtx->uiTargetBitRate = seqParams->TargetBitRate;
                m_encodeCtx->uiMaxBitRate    = seqParams->MaxBitRate;
            }
        }

        break;
    }
    case VAEncMiscParameterTypeParallelBRC:
    {
        VAEncMiscParameterParallelRateControl *vaEncMiscParameterParallel = (VAEncMiscParameterParallelRateControl *)miscParamBuf->data;

        seqParams->NumOfBInGop[0] = vaEncMiscParameterParallel->num_b_in_gop[0];
        seqParams->NumOfBInGop[1] = vaEncMiscParameterParallel->num_b_in_gop[1];
        seqParams->NumOfBInGop[2] = vaEncMiscParameterParallel->num_b_in_gop[2];

        break;
    }
    case VAEncMiscParameterTypeRIR:
    {
        VAEncMiscParameterRIR *vaEncMiscParamRIR = (VAEncMiscParameterRIR *)miscParamBuf->data;
        picParams->bEnableRollingIntraRefresh    = vaEncMiscParamRIR->rir_flags.value & 0x3;  //only lower two bits are valid

        // Set for all frames since pic type is not known yet. Disable in slice params if its I or B type.
        if ((ROLLING_I_COLUMN == picParams->bEnableRollingIntraRefresh) || (ROLLING_I_ROW == picParams->bEnableRollingIntraRefresh))
        {
            picParams->IntraInsertionLocation  = (uint16_t)vaEncMiscParamRIR->intra_insertion_location;
            picParams->IntraInsertionSize      = (uint8_t)vaEncMiscParamRIR->intra_insert_size;
            picParams->QpDeltaForInsertedIntra = vaEncMiscParamRIR->qp_delta_for_inserted_intra;
        }
        else if ((ROLLING_I_ROW | ROLLING_I_COLUMN) == picParams->bEnableRollingIntraRefresh)  // cannot have row and column rolling
        {
            picParams->bEnableRollingIntraRefresh = ROLLING_I_DISABLED;
            return VA_STATUS_ERROR_INVALID_PARAMETER;
        }

        break;
    }
    case VAEncMiscParameterTypeROI:
    {
        VAEncMiscParameterBufferROI *vaEncMiscParamROI = (VAEncMiscParameterBufferROI *)miscParamBuf->data;
        uint32_t                     maxROIsupported   = CODECHAL_ENCODE_HEVC_MAX_NUM_ROI;
        uint8_t                      blockSize         = (m_encodeCtx->bVdencActive) ? vdencRoiBlockSize : CODECHAL_MACROBLOCK_WIDTH;

        if (vaEncMiscParamROI->num_roi)
        {
            for (uint32_t i = 0; i < vaEncMiscParamROI->num_roi; ++i)
            {
                picParams->ROI[i].PriorityLevelOrDQp = vaEncMiscParamROI->roi[i].roi_value;
                picParams->ROI[i].Top                = vaEncMiscParamROI->roi[i].roi_rectangle.y;
                picParams->ROI[i].Left               = vaEncMiscParamROI->roi[i].roi_rectangle.x;
                picParams->ROI[i].Bottom             = vaEncMiscParamROI->roi[i].roi_rectangle.y +
                                           vaEncMiscParamROI->roi[i].roi_rectangle.height - 1;
                picParams->ROI[i].Right = vaEncMiscParamROI->roi[i].roi_rectangle.x +
                                          vaEncMiscParamROI->roi[i].roi_rectangle.width - 1;

                if (m_encodeCtx->bVdencActive == false)
                {
                    // align to CTB edge (32 on Gen9) to avoid QP average issue
                    picParams->ROI[i].Left   = MOS_ALIGN_FLOOR(picParams->ROI[i].Left, 32);
                    picParams->ROI[i].Right  = MOS_ALIGN_CEIL(picParams->ROI[i].Right, 32);
                    picParams->ROI[i].Top    = MOS_ALIGN_FLOOR(picParams->ROI[i].Top, 32);
                    picParams->ROI[i].Bottom = MOS_ALIGN_CEIL(picParams->ROI[i].Bottom, 32);
                }

                // Convert from pixel units to block size units
                picParams->ROI[i].Left /= blockSize;
                picParams->ROI[i].Right /= blockSize;
                picParams->ROI[i].Top /= blockSize;
                picParams->ROI[i].Bottom /= blockSize;
            }
            picParams->NumROI = MOS_MIN(vaEncMiscParamROI->num_roi, maxROIsupported);
        }
#ifndef ANDROID
        // support DeltaQP based ROI by default
        seqParams->ROIValueInDeltaQP = vaEncMiscParamROI->roi_flags.bits.roi_value_is_qp_delta;
#endif
        break;
    }
    case VAEncMiscParameterTypeSkipFrame:
    {
        VAEncMiscParameterSkipFrame *vaEncMiscParamSkipFrame = (VAEncMiscParameterSkipFrame *)miscParamBuf->data;
        // populate skipped frame params from DDI
        if (FRAME_SKIP_NORMAL != vaEncMiscParamSkipFrame->skip_frame_flag)
        {
            DDI_ASSERTMESSAGE("unsupported misc parameter type.");
            return VA_STATUS_ERROR_INVALID_PARAMETER;
        }
        picParams->SkipFrameFlag  = vaEncMiscParamSkipFrame->skip_frame_flag;
        picParams->NumSkipFrames  = vaEncMiscParamSkipFrame->num_skip_frames;
        picParams->SizeSkipFrames = vaEncMiscParamSkipFrame->size_skip_frames;
        break;
    }
    case VAEncMiscParameterTypeMaxSliceSize:
    {
        VAEncMiscParameterMaxSliceSize *vaEncMiscParamMaxSliceSize = (VAEncMiscParameterMaxSliceSize *)miscParamBuf->data;
        m_encodeCtx->EnableSliceLevelRateCtrl                      = true;
        seqParams->SliceSizeControl                                = true;
        picParams->MaxSliceSizeInBytes                             = vaEncMiscParamMaxSliceSize->max_slice_size;
        break;
    }
    case VAEncMiscParameterTypeEncQuality:
    {
        VAEncMiscParameterEncQuality *vaEncMiscParamEncQuality        = (VAEncMiscParameterEncQuality *)miscParamBuf->data;
        picParams->bUseRawPicForRef                                = vaEncMiscParamEncQuality->useRawPicForRef;
        break;
    }
    case VAEncMiscParameterTypeFEIFrameControl:
    {
        if (VA_STATUS_SUCCESS != ParseMiscParamFeiPic((void *)miscParamBuf->data))
        {
            DDI_ASSERTMESSAGE("parse misc FEI picture parameters error.");
            return VA_STATUS_ERROR_INVALID_PARAMETER;
        }
        break;
    }
    default:
        DDI_ASSERTMESSAGE("unsupported misc parameter type.");
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }

    return VA_STATUS_SUCCESS;
}

