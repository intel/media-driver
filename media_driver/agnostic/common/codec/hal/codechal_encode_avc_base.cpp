/*
* Copyright (c) 2017-2019, Intel Corporation
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
//! \file     codechal_encode_avc_base.cpp
//! \brief    Defines base class for AVC encoder.
//!
#include "codechal_encode_avc_base.h"
#include "codechal_vdenc_avc.h"
#include "codechal_encode_avc.h"
#include "codechal_mmc_encode_avc.h"
#include "hal_oca_interface.h"

#define CODECHAL_ENCODE_AVC_CQP_NUM_OF_PASSES 2
#define CODECHAL_ENCODE_AVC_ICQ_NUM_OF_PASSES 2
#define CODECHAL_ENCODE_AVC_EXTENDED_SAR 255

const uint8_t CODECHAL_ENCODE_AVC_SFD_CostTable_P_FRAME[CODEC_AVC_NUM_QP] =
    {
        44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 60, 60, 60, 60, 73, 73, 73, 76, 76, 76, 88, 89, 89, 91, 92, 93, 104, 104, 106, 107, 108, 109, 120, 120, 122, 123, 124, 125, 136, 136, 138, 139, 140, 141, 143, 143};

const uint8_t CODECHAL_ENCODE_AVC_SFD_CostTable_B_FRAME[CODEC_AVC_NUM_QP] =
    {
        57, 57, 57, 57, 57, 57, 57, 57, 57, 57, 57, 57, 57, 57, 57, 57, 73, 73, 73, 73, 77, 77, 77, 89, 89, 89, 91, 93, 93, 95, 105, 106, 107, 108, 110, 111, 121, 122, 123, 124, 125, 127, 137, 138, 139, 140, 142, 143, 143, 143, 143, 143};

uint32_t CodecHalAvcEncode_GetMaxVmvR(uint8_t levelIdc)
{
    int maxVmvR = 128 * 4;

    // See JVT Spec Annex A Table A-1 Level limits for below mapping
    // maxVmvR is in luma quarter pel unit
    switch (levelIdc)
    {
    case CODEC_AVC_LEVEL_1:
    case CODEC_AVC_LEVEL_1b:
        maxVmvR = 64 * 4;
        break;
    case CODEC_AVC_LEVEL_11:
    case CODEC_AVC_LEVEL_12:
    case CODEC_AVC_LEVEL_13:
    case CODEC_AVC_LEVEL_2:
        maxVmvR = 128 * 4;
        break;
    case CODEC_AVC_LEVEL_21:
    case CODEC_AVC_LEVEL_22:
    case CODEC_AVC_LEVEL_3:
        maxVmvR = 256 * 4;
        break;
    case CODEC_AVC_LEVEL_31:
    case CODEC_AVC_LEVEL_32:
    case CODEC_AVC_LEVEL_4:
    case CODEC_AVC_LEVEL_41:
    case CODEC_AVC_LEVEL_42:
    case CODEC_AVC_LEVEL_5:
    case CODEC_AVC_LEVEL_51:
    case CODEC_AVC_LEVEL_52:
        maxVmvR = 512 * 4;
        break;
    default:
        CODECHAL_ENCODE_NORMALMESSAGE("Unsupported LevelIDC setting.");
        CODECHAL_ENCODE_ASSERT(false);
        break;
    }

    return maxVmvR;
}

uint32_t CodecHalAvcEncode_GetMaxMvLen(uint8_t levelIdc)
{
    int maxMvLen = 127;

    // See JVT Spec Annex A Table A-1 Level limits for below mapping
    // MaxVmvR is in luma quarter pel unit
    switch (levelIdc)
    {
    case CODEC_AVC_LEVEL_1:
    case CODEC_AVC_LEVEL_1b:
        maxMvLen = 63;
        break;
    case CODEC_AVC_LEVEL_11:
    case CODEC_AVC_LEVEL_12:
    case CODEC_AVC_LEVEL_13:
    case CODEC_AVC_LEVEL_2:
        maxMvLen = 127;
        break;
    case CODEC_AVC_LEVEL_21:
    case CODEC_AVC_LEVEL_22:
    case CODEC_AVC_LEVEL_3:
        maxMvLen = 255;
        break;
    case CODEC_AVC_LEVEL_31:
    case CODEC_AVC_LEVEL_32:
    case CODEC_AVC_LEVEL_4:
    case CODEC_AVC_LEVEL_41:
    case CODEC_AVC_LEVEL_42:
    case CODEC_AVC_LEVEL_5:
    case CODEC_AVC_LEVEL_51:
    case CODEC_AVC_LEVEL_52:
        maxMvLen = 511;
        break;
    default:
        CODECHAL_ENCODE_NORMALMESSAGE("Unsupported LevelIDC setting.");
        CODECHAL_ENCODE_ASSERT(false);
        break;
    }

    return maxMvLen;
}

bool CodecHalAvcEncode_GetFieldParity(
    PCODEC_AVC_ENCODE_SLICE_PARAMS slcParams,
    uint32_t                       list,
    uint32_t                       index)
{
    uint32_t fieldParity = 0;

    if (slcParams == nullptr)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Invalid (NULL) Pointer.");
        return false;
    }
    CODECHAL_ENCODE_ASSERT(list == LIST_0 || list == LIST_1);
    CODECHAL_ENCODE_ASSERT(index < CODEC_AVC_MAX_NUM_REF_FRAME * 2);

    if (!CodecHal_PictureIsInvalid(slcParams->RefPicList[list][index]))
    {
        fieldParity = CodecHal_PictureIsBottomField(slcParams->RefPicList[list][index]);
    }

    return (fieldParity ? true : false);
}

MOS_STATUS CodechalEncodeAvcBase::SendSlice(
    PMOS_COMMAND_BUFFER        cmdBuffer,
    PMHW_VDBOX_AVC_SLICE_STATE params)
{
    PCODEC_AVC_ENCODE_SLICE_PARAMS      avcSlcParams;
    MHW_VDBOX_AVC_REF_IDX_PARAMS        refIdxParams;
    MHW_VDBOX_AVC_WEIGHTOFFSET_PARAMS   weightOffsetParams;
    MHW_BATCH_BUFFER                    secondLevelBatchBuffer;
    MHW_VDBOX_PAK_INSERT_PARAMS         pakInsertObjectParams;
    MHW_VDBOX_VDENC_WALKER_STATE_PARAMS vdencWalkerStateParams;
    PMOS_COMMAND_BUFFER                 cmdBufferInUse;
    PMHW_BATCH_BUFFER                   batchBufferInUse;
    uint32_t                            bitSize, offSet;
    uint32_t                            maxBytesInPakInsertObjCmd;
    uint32_t                            nalunitPosiSize, nalunitPosiOffset;
    bool                                insertZeroByteWA = false;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(cmdBuffer);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pAvcPicIdx);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->presDataBuffer);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pEncodeAvcSeqParams);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pEncodeAvcPicParams);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pEncodeAvcSliceParams);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pBsBuffer);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->ppNalUnitParams);

    avcSlcParams              = params->pEncodeAvcSliceParams;
    maxBytesInPakInsertObjCmd = ((2 << 11) - 1) * 4;  // 12 bits for DwordLength field in PAK_INSERT_OBJ cmd
    nalunitPosiSize           = 0;
    nalunitPosiOffset         = 0;

    // VDENC does not use batch buffer for slice state
    if (params->bSingleTaskPhaseSupported && !params->bVdencInUse)
    {
        CODECHAL_ENCODE_CHK_NULL_RETURN(params->pBatchBufferForPakSlices);
        batchBufferInUse = params->pBatchBufferForPakSlices;
        cmdBufferInUse   = nullptr;
    }
    else
    {
        batchBufferInUse = nullptr;
        cmdBufferInUse   = cmdBuffer;
    }

    // Add reference index and weight offset commands
    refIdxParams.CurrPic         = params->pEncodeAvcPicParams->CurrReconstructedPic;
    refIdxParams.isEncode        = true;
    refIdxParams.bVdencInUse     = params->bVdencInUse;
    refIdxParams.pAvcPicIdx      = params->pAvcPicIdx;
    refIdxParams.avcRefList      = (void **)m_refList;
    refIdxParams.oneOnOneMapping = params->oneOnOneMapping;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(
        &refIdxParams.RefPicList,
        2 * 32 * sizeof(CODEC_PICTURE),
        avcSlcParams->RefPicList,
        2 * 32 * sizeof(CODEC_PICTURE)));
    if (Slice_Type[avcSlcParams->slice_type] == SLICE_P)
    {
        refIdxParams.uiList                  = LIST_0;
        refIdxParams.uiNumRefForList[LIST_0] = avcSlcParams->num_ref_idx_l0_active_minus1 + 1;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfxAvcRefIdx(cmdBufferInUse, batchBufferInUse, &refIdxParams));

        if (params->pEncodeAvcPicParams->weighted_pred_flag == EXPLICIT_WEIGHTED_INTER_PRED_MODE)
        {
            weightOffsetParams.uiList                 = LIST_0;
            weightOffsetParams.uiLumaLogWeightDenom   = avcSlcParams->luma_log2_weight_denom;
            weightOffsetParams.uiChromaLogWeightDenom = avcSlcParams->chroma_log2_weight_denom;
            weightOffsetParams.uiLumaWeightFlag       = avcSlcParams->luma_weight_flag[LIST_0];
            weightOffsetParams.uiChromaWeightFlag     = avcSlcParams->chroma_weight_flag[LIST_0];
            weightOffsetParams.uiNumRefForList        = avcSlcParams->num_ref_idx_l0_active_minus1 + 1;

            CODECHAL_ENCODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(
                &weightOffsetParams.Weights,
                sizeof(weightOffsetParams.Weights),
                &avcSlcParams->Weights,
                sizeof(avcSlcParams->Weights)));
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfxAvcWeightOffset(
                cmdBufferInUse,
                batchBufferInUse,
                &weightOffsetParams));
        }
    }

    else if (Slice_Type[avcSlcParams->slice_type] == SLICE_B)
    {
        refIdxParams.uiList                  = LIST_0;
        refIdxParams.uiNumRefForList[LIST_0] = avcSlcParams->num_ref_idx_l0_active_minus1 + 1;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfxAvcRefIdx(cmdBufferInUse, batchBufferInUse, &refIdxParams));

        refIdxParams.uiList                  = LIST_1;
        refIdxParams.uiNumRefForList[LIST_1] = avcSlcParams->num_ref_idx_l1_active_minus1 + 1;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfxAvcRefIdx(cmdBufferInUse, batchBufferInUse, &refIdxParams));

        if (params->pEncodeAvcPicParams->weighted_bipred_idc == EXPLICIT_WEIGHTED_INTER_PRED_MODE)
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(
                &weightOffsetParams.Weights,
                sizeof(weightOffsetParams.Weights),
                &avcSlcParams->Weights,
                sizeof(avcSlcParams->Weights)));
            weightOffsetParams.uiList                 = LIST_0;
            weightOffsetParams.uiLumaLogWeightDenom   = avcSlcParams->luma_log2_weight_denom;
            weightOffsetParams.uiChromaLogWeightDenom = avcSlcParams->chroma_log2_weight_denom;
            weightOffsetParams.uiLumaWeightFlag       = avcSlcParams->luma_weight_flag[LIST_0];
            weightOffsetParams.uiChromaWeightFlag     = avcSlcParams->chroma_weight_flag[LIST_0];
            weightOffsetParams.uiNumRefForList        = avcSlcParams->num_ref_idx_l0_active_minus1 + 1;
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfxAvcWeightOffset(
                cmdBufferInUse,
                batchBufferInUse,
                &weightOffsetParams));

            weightOffsetParams.uiList                 = LIST_1;
            weightOffsetParams.uiLumaLogWeightDenom   = avcSlcParams->luma_log2_weight_denom;
            weightOffsetParams.uiChromaLogWeightDenom = avcSlcParams->chroma_log2_weight_denom;
            weightOffsetParams.uiLumaWeightFlag       = avcSlcParams->luma_weight_flag[LIST_1];
            weightOffsetParams.uiChromaWeightFlag     = avcSlcParams->chroma_weight_flag[LIST_1];
            weightOffsetParams.uiNumRefForList        = avcSlcParams->num_ref_idx_l1_active_minus1 + 1;
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfxAvcWeightOffset(
                cmdBufferInUse,
                batchBufferInUse,
                &weightOffsetParams));
        }
    }

    // add AVC Slice state commands
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfxAvcSlice(cmdBufferInUse, batchBufferInUse, params));

    //insert AU, SPS, PSP headers before first slice header
    if (params->bInsertBeforeSliceHeaders)
    {
        uint8_t *dataBase = (uint8_t *)(params->pBsBuffer->pBase);
        uint32_t data     = ((*dataBase) << 24) + ((*(dataBase + 1)) << 16) + ((*(dataBase + 2)) << 8) + (*(dataBase + 3));
        // Only apply the WaSuperSliceHeaderPacking for the cases with 00 00 00 01 start code
        if (data == 0x00000001)
        {
            insertZeroByteWA = true;
        }

        bool     isInsert;
        uint32_t i;
        for (i = 0; i < CODECHAL_ENCODE_AVC_MAX_NAL_TYPE; i++)
        {
            nalunitPosiSize   = params->ppNalUnitParams[i]->uiSize;
            nalunitPosiOffset = params->ppNalUnitParams[i]->uiOffset;
            while (nalunitPosiSize > 0)
            {
                isInsert = params->ppNalUnitParams[i]->bInsertEmulationBytes;
                bitSize  = MOS_MIN(maxBytesInPakInsertObjCmd * 8, nalunitPosiSize * 8);
                offSet   = nalunitPosiOffset;

                MOS_ZeroMemory(&pakInsertObjectParams, sizeof(pakInsertObjectParams));
                pakInsertObjectParams.bEmulationByteBitsInsert    = isInsert;
                pakInsertObjectParams.uiSkipEmulationCheckCount   = params->ppNalUnitParams[i]->uiSkipEmulationCheckCount;
                pakInsertObjectParams.pBsBuffer                   = params->pBsBuffer;
                pakInsertObjectParams.dwBitSize                   = bitSize;
                pakInsertObjectParams.dwOffset                    = offSet;
                pakInsertObjectParams.bHeaderLengthExcludeFrmSize = true;  // Exclude header length from size calculation for accurate Cabac Zero Word Insertion

                if (pakInsertObjectParams.bEmulationByteBitsInsert)
                {
                    CODECHAL_ENCODE_VERBOSEMESSAGE("The emulation prevention bytes are not inserted by the app and are requested to be inserted by HW.");
                }

                CODECHAL_ENCODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfxPakInsertObject(
                    cmdBufferInUse, batchBufferInUse, &pakInsertObjectParams));

                if (nalunitPosiSize > maxBytesInPakInsertObjCmd)
                {
                    nalunitPosiSize -= maxBytesInPakInsertObjCmd;
                    nalunitPosiOffset += maxBytesInPakInsertObjCmd;
                }
                else
                {
                    nalunitPosiSize = 0;
                }

                insertZeroByteWA = false;
            }
        }
    }

    uint8_t *dataBase = (uint8_t *)(params->pBsBuffer->pBase + params->dwOffset);
    uint32_t data     = ((*dataBase) << 24) + ((*(dataBase + 1)) << 16) + ((*(dataBase + 2)) << 8) + (*(dataBase + 3));

    if (data == 0x00000001)
    {
        insertZeroByteWA = true;
    }

    // Insert 0x00 for super slice case when PPS/AUD is not inserted
    if (MEDIA_IS_WA(m_hwInterface->GetWaTable(), WaSuperSliceHeaderPacking) && insertZeroByteWA && params->bVdencInUse && m_hwInterface->m_isVdencSuperSliceEnabled)
    {
        MOS_ZeroMemory(&pakInsertObjectParams, sizeof(pakInsertObjectParams));
        pakInsertObjectParams.pBsBuffer = params->pBsBuffer;
        pakInsertObjectParams.dwBitSize = 8;
        pakInsertObjectParams.dwOffset  = params->dwOffset;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfxPakInsertObject(
            cmdBufferInUse, batchBufferInUse, &pakInsertObjectParams));
    }

    // Insert slice header
    MOS_ZeroMemory(&pakInsertObjectParams, sizeof(pakInsertObjectParams));
    pakInsertObjectParams.bLastHeader              = true;
    pakInsertObjectParams.bEmulationByteBitsInsert = true;

    if (params->bAcceleratorHeaderPackingCaps)
    {
        // If driver does slice header packing set the skip count to 4
        pakInsertObjectParams.uiSkipEmulationCheckCount = 4;
    }
    else
    {
        // App does the slice header packing, set the skip count passed by the app
        pakInsertObjectParams.uiSkipEmulationCheckCount = params->uiSkipEmulationCheckCount;
    }
    pakInsertObjectParams.pBsBuffer = params->pBsBuffer;
    // Slice Header Indicator should be set for VDENC for multi-slice case
    pakInsertObjectParams.bSliceHeaderIndicator = (!params->bVdencInUse) ? false : true;

    // Remove one byte of 00 for super slice case when PPS/AUD is not inserted, so that HW could patch slice header correctly
    if (MEDIA_IS_WA(m_hwInterface->GetWaTable(), WaSuperSliceHeaderPacking) && insertZeroByteWA && params->bVdencInUse && m_hwInterface->m_isVdencSuperSliceEnabled)
    {
        pakInsertObjectParams.dwBitSize = params->dwLength - 8;
        pakInsertObjectParams.dwOffset  = params->dwOffset + 1;
    }
    else
    {
        pakInsertObjectParams.dwBitSize = params->dwLength;
        pakInsertObjectParams.dwOffset  = params->dwOffset;
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfxPakInsertObject(
        cmdBufferInUse, batchBufferInUse, &pakInsertObjectParams));

    if (params->bVdencInUse)
    {
        //For CNL VDENC Walker command and WeightsOffsets cmds are sent per Super slice
        if (m_hwInterface->m_isVdencSuperSliceEnabled)
        {
            weightOffsetParams.pAvcPicParams = params->pEncodeAvcPicParams;
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_vdencInterface->AddVdencAvcWeightsOffsetsStateCmd(cmdBuffer, &weightOffsetParams));
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_vdencInterface->AddVdencSliceStateCmd(cmdBuffer, params));

            vdencWalkerStateParams.Mode          = CODECHAL_ENCODE_MODE_AVC;
            vdencWalkerStateParams.slcIdx        = params->dwSliceIndex;
            vdencWalkerStateParams.pAvcSeqParams = params->pEncodeAvcSeqParams;
            vdencWalkerStateParams.pAvcPicParams = params->pEncodeAvcPicParams;
            vdencWalkerStateParams.pAvcSlcParams = avcSlcParams;
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_vdencInterface->AddVdencWalkerStateCmd(cmdBuffer, &vdencWalkerStateParams));
        }
    }
    else
    {
        if (params->bSingleTaskPhaseSupported)
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferEnd(nullptr, batchBufferInUse));

            // Insert Batch Buffer Start command to send AVC_PAK_OBJ data for MBs in this slice
            MOS_ZeroMemory(&secondLevelBatchBuffer, sizeof(MHW_BATCH_BUFFER));
            CODECHAL_ENCODE_CHK_NULL_RETURN(batchBufferInUse);
            secondLevelBatchBuffer.OsResource   = batchBufferInUse->OsResource;
            secondLevelBatchBuffer.dwOffset     = params->dwBatchBufferForPakSlicesStartOffset;
            secondLevelBatchBuffer.bSecondLevel = true;
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferStartCmd(cmdBuffer, &secondLevelBatchBuffer));
        }

        // Insert Batch Buffer Start command to send AVC_PAK_OBJ data for MBs in this slice
        MOS_ZeroMemory(&secondLevelBatchBuffer, sizeof(MHW_BATCH_BUFFER));
        secondLevelBatchBuffer.OsResource   = *params->presDataBuffer;
        secondLevelBatchBuffer.dwOffset     = params->dwDataBufferOffset;
        secondLevelBatchBuffer.bSecondLevel = true;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferStartCmd(cmdBuffer, &secondLevelBatchBuffer));
    }

    return MOS_STATUS_SUCCESS;
}
static int32_t GetMaxMBPS(uint8_t levelIdc)
{
    int maxMBPS = 11880;

    // See JVT Spec Annex A Table A-1 Level limits for below mapping
    // maxMBPS is in MB/s
    switch (levelIdc)
    {
    case CODEC_AVC_LEVEL_1:
    case CODEC_AVC_LEVEL_1b:
        maxMBPS = 1485;
        break;
    case CODEC_AVC_LEVEL_11:
        maxMBPS = 3000;
        break;
    case CODEC_AVC_LEVEL_12:
        maxMBPS = 6000;
        break;
    case CODEC_AVC_LEVEL_13:
    case CODEC_AVC_LEVEL_2:
        maxMBPS = 11880;
        break;
    case CODEC_AVC_LEVEL_21:
        maxMBPS = 19800;
        break;
    case CODEC_AVC_LEVEL_22:
        maxMBPS = 20250;
        break;
    case CODEC_AVC_LEVEL_3:
        maxMBPS = 40500;
        break;
    case CODEC_AVC_LEVEL_31:
        maxMBPS = 108000;
        break;
    case CODEC_AVC_LEVEL_32:
        maxMBPS = 216000;
        break;
    case CODEC_AVC_LEVEL_4:
    case CODEC_AVC_LEVEL_41:
        maxMBPS = 245760;
        break;
    case CODEC_AVC_LEVEL_42:
        maxMBPS = 522240;
        break;
    case CODEC_AVC_LEVEL_5:
        maxMBPS = 589824;
        break;
    case CODEC_AVC_LEVEL_51:
        maxMBPS = 983040;
        break;
    case CODEC_AVC_LEVEL_52:
        maxMBPS = 2073600;
        break;
    default:
        maxMBPS = 0;
        break;
    }

    return maxMBPS;
}

MOS_STATUS CodecHalAvcEncode_GetProfileLevelMaxFrameSize(
    PCODEC_AVC_ENCODE_SEQUENCE_PARAMS seqParams,
    CodechalEncoderState *            encoder,
    uint32_t *                        pdwProfileLevelMaxFrame)
{
    double     bitsPerMB, tmpf;
    int32_t    iMaxMBPS, numMBPerFrame, minCR;
    uint64_t   maxBytePerPic, maxBytePerPicNot0;
    uint32_t   profileLevelMaxFrame, userMaxFrameSize;
    MOS_STATUS eStatus    = MOS_STATUS_SUCCESS;
    double     frameRateD = 100;

    CODECHAL_ENCODE_CHK_NULL_RETURN(seqParams);
    CODECHAL_ENCODE_CHK_NULL_RETURN(encoder);
    CODECHAL_ENCODE_CHK_NULL_RETURN(pdwProfileLevelMaxFrame);

    minCR = 4;

    if (seqParams->Level >= CODEC_AVC_LEVEL_31 && seqParams->Level <= CODEC_AVC_LEVEL_4)
    {
        bitsPerMB = 96;
    }
    else
    {
        bitsPerMB = 192;
        minCR     = 2;
    }

    iMaxMBPS = GetMaxMBPS(seqParams->Level);
    if (!iMaxMBPS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Unsupported LevelIDC setting.");
        return MOS_STATUS_UNKNOWN;
    }

    numMBPerFrame = encoder->m_picWidthInMb * encoder->m_frameFieldHeightInMb;

    tmpf = (double)numMBPerFrame;
    if (tmpf < iMaxMBPS / 172.)
    {
        tmpf = iMaxMBPS / 172.;
    }

    maxBytePerPic = (uint64_t)(tmpf * bitsPerMB);
    maxBytePerPicNot0 =
        (uint64_t)((((double)iMaxMBPS * frameRateD) /
                       (double)seqParams->FramesPer100Sec) *
                   bitsPerMB);
    userMaxFrameSize = seqParams->UserMaxFrameSize;

    if ((encoder->m_pictureCodingType != I_TYPE) && (seqParams->UserMaxPBFrameSize > 0))
    {
        userMaxFrameSize = seqParams->UserMaxPBFrameSize;
    }

    if (userMaxFrameSize != 0)
    {
        profileLevelMaxFrame = (uint32_t)MOS_MIN(userMaxFrameSize, maxBytePerPic);
        profileLevelMaxFrame = (uint32_t)MOS_MIN(maxBytePerPicNot0, profileLevelMaxFrame);
    }
    else
    {
        profileLevelMaxFrame = (uint32_t)MOS_MIN(maxBytePerPicNot0, maxBytePerPic);
    }

    // minCR related changes are added to match hardware behavior
    if (encoder->m_vdencEnabled)
    {
        *pdwProfileLevelMaxFrame = (uint32_t)MOS_MIN((encoder->m_frameHeight * encoder->m_frameHeight), profileLevelMaxFrame);
    }
    else
    {
        *pdwProfileLevelMaxFrame = (uint32_t)MOS_MIN(encoder->m_frameHeight * encoder->m_frameWidth * 3 / (2 * minCR), profileLevelMaxFrame);
    }

    return eStatus;
}

static void PutVLCCode(BSBuffer *bsbuffer, uint32_t code)
{
    uint8_t  leadingZeroBits, bitcount;
    uint32_t code1, bits;

    code1    = code + 1;
    bitcount = 0;
    while (code1)
    {
        code1 >>= 1;
        bitcount++;
    }

    if (bitcount == 1)
    {
        PutBit(bsbuffer, 1);
    }
    else
    {
        leadingZeroBits = bitcount - 1;
        bits            = code + 1 - (1 << leadingZeroBits);
        PutBits(bsbuffer, 1, leadingZeroBits + 1);
        PutBits(bsbuffer, bits, leadingZeroBits);
    }
}

//!
//! \brief    Pack HRD data
//!
//! \param    [in] params
//!           Pointer to codechal encode Avc pack picture header parameter
//!
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if call success, else fail reason
//!
MOS_STATUS CodecHal_PackPictureHeader_HrdParams(
    PCODECHAL_ENCODE_AVC_PACK_PIC_HEADER_PARAMS params)
{
    PCODECHAL_ENCODE_AVC_VUI_PARAMS vuiParams;
    PBSBuffer                       bsbuffer;
    int                             schedSelIdx;
    MOS_STATUS                      eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_NULL_RETURN(params);

    vuiParams = params->pAvcVuiParams;
    bsbuffer  = params->pBsBuffer;

    PutVLCCode(bsbuffer, vuiParams->cpb_cnt_minus1);
    PutBits(bsbuffer, vuiParams->bit_rate_scale, 4);
    PutBits(bsbuffer, vuiParams->cpb_size_scale, 4);

    for (schedSelIdx = 0; schedSelIdx <= vuiParams->cpb_cnt_minus1; schedSelIdx++)
    {
        PutVLCCode(bsbuffer, vuiParams->bit_rate_value_minus1[schedSelIdx]);
        PutVLCCode(bsbuffer, vuiParams->cpb_size_value_minus1[schedSelIdx]);
        PutBit(bsbuffer, ((vuiParams->cbr_flag >> schedSelIdx) & 1));
    }

    PutBits(bsbuffer, vuiParams->initial_cpb_removal_delay_length_minus1, 5);
    PutBits(bsbuffer, vuiParams->cpb_removal_delay_length_minus1, 5);
    PutBits(bsbuffer, vuiParams->dpb_output_delay_length_minus1, 5);
    PutBits(bsbuffer, vuiParams->time_offset_length, 5);

    return eStatus;
}

static void PackScalingList(BSBuffer *bsbuffer, uint8_t *scalingList, uint8_t sizeOfScalingList)
{
    uint8_t lastScale, nextScale, j;
    char    delta_scale;

    lastScale = nextScale = 8;

    for (j = 0; j < sizeOfScalingList; j++)
    {
        if (nextScale != 0)
        {
            delta_scale = (char)(scalingList[j] - lastScale);

            PutVLCCode(bsbuffer, SIGNED(delta_scale));

            nextScale = scalingList[j];
        }
        lastScale = (nextScale == 0) ? lastScale : nextScale;
    }
}

//!
//! \brief    Pack VUI data
//!
//! \param    [in] params
//!           Pointer to codechal encode Avc pack picture header parameter
//!
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if call success, else fail reason
//!
MOS_STATUS CodecHal_PackPictureHeader_VuiParams(
    PCODECHAL_ENCODE_AVC_PACK_PIC_HEADER_PARAMS params)
{
    PCODECHAL_ENCODE_AVC_VUI_PARAMS vuiParams;
    PBSBuffer                       bsbuffer;
    MOS_STATUS                      eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_NULL_RETURN(params);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pAvcVuiParams);

    vuiParams = params->pAvcVuiParams;
    bsbuffer  = params->pBsBuffer;

    PutBit(bsbuffer, vuiParams->aspect_ratio_info_present_flag);
    if (vuiParams->aspect_ratio_info_present_flag)
    {
        PutBits(bsbuffer, vuiParams->aspect_ratio_idc, 8);
        if (vuiParams->aspect_ratio_idc == CODECHAL_ENCODE_AVC_EXTENDED_SAR)
        {
            PutBits(bsbuffer, vuiParams->sar_width, 16);
            PutBits(bsbuffer, vuiParams->sar_height, 16);
        }
    }

    PutBit(bsbuffer, vuiParams->overscan_info_present_flag);
    if (vuiParams->overscan_info_present_flag)
    {
        PutBit(bsbuffer, vuiParams->overscan_appropriate_flag);
    }

    PutBit(bsbuffer, vuiParams->video_signal_type_present_flag);
    if (vuiParams->video_signal_type_present_flag)
    {
        PutBits(bsbuffer, vuiParams->video_format, 3);
        PutBit(bsbuffer, vuiParams->video_full_range_flag);
        PutBit(bsbuffer, vuiParams->colour_description_present_flag);
        if (vuiParams->colour_description_present_flag)
        {
            PutBits(bsbuffer, vuiParams->colour_primaries, 8);
            PutBits(bsbuffer, vuiParams->transfer_characteristics, 8);
            PutBits(bsbuffer, vuiParams->matrix_coefficients, 8);
        }
    }

    PutBit(bsbuffer, vuiParams->chroma_loc_info_present_flag);
    if (vuiParams->chroma_loc_info_present_flag)
    {
        PutVLCCode(bsbuffer, vuiParams->chroma_sample_loc_type_top_field);
        PutVLCCode(bsbuffer, vuiParams->chroma_sample_loc_type_bottom_field);
    }

    PutBit(bsbuffer, vuiParams->timing_info_present_flag);
    if (vuiParams->timing_info_present_flag)
    {
        PutBits(bsbuffer, vuiParams->num_units_in_tick, 32);
        PutBits(bsbuffer, vuiParams->time_scale, 32);
        PutBit(bsbuffer, vuiParams->fixed_frame_rate_flag);
    }

    PutBit(bsbuffer, vuiParams->nal_hrd_parameters_present_flag);
    if (vuiParams->nal_hrd_parameters_present_flag)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHal_PackPictureHeader_HrdParams(params));
    }

    PutBit(bsbuffer, vuiParams->vcl_hrd_parameters_present_flag);
    if (vuiParams->vcl_hrd_parameters_present_flag)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHal_PackPictureHeader_HrdParams(params));
    }

    if (vuiParams->nal_hrd_parameters_present_flag || vuiParams->vcl_hrd_parameters_present_flag)
    {
        PutBit(bsbuffer, vuiParams->low_delay_hrd_flag);
    }

    PutBit(bsbuffer, vuiParams->pic_struct_present_flag);
    PutBit(bsbuffer, vuiParams->bitstream_restriction_flag);
    if (vuiParams->bitstream_restriction_flag)
    {
        PutBit(bsbuffer, vuiParams->motion_vectors_over_pic_boundaries_flag);
        PutVLCCode(bsbuffer, vuiParams->max_bytes_per_pic_denom);
        PutVLCCode(bsbuffer, vuiParams->max_bits_per_mb_denom);
        PutVLCCode(bsbuffer, vuiParams->log2_max_mv_length_horizontal);
        PutVLCCode(bsbuffer, vuiParams->log2_max_mv_length_vertical);
        PutVLCCode(bsbuffer, vuiParams->num_reorder_frames);
        PutVLCCode(bsbuffer, vuiParams->max_dec_frame_buffering);
    }

    return eStatus;
}

static void SetNalUnit(uint8_t **bsbuffer, uint8_t refIDC, CODECHAL_ENCODE_AVC_NAL_UNIT_TYPE nalType)
{
    uint8_t *byte = *bsbuffer;

    // for SPS and PPS NAL units zero_byte should exist
    if (nalType == CODECHAL_ENCODE_AVC_NAL_UT_SPS || nalType == CODECHAL_ENCODE_AVC_NAL_UT_PPS || nalType == CODECHAL_ENCODE_AVC_NAL_UT_AUD)
    {
        *byte++ = 0;
    }

    *byte++   = 0;
    *byte++   = 0;
    *byte++   = 1;
    *byte++   = (uint8_t)((refIDC << 5) | nalType);
    *byte     = 0;  // Clear the next byte
    *bsbuffer = byte;
}

static void SetTrailingBits(BSBuffer *bsbuffer)
{
    // Write Stop Bit
    PutBits(bsbuffer, 1, 1);
    // Make byte aligned
    while (bsbuffer->BitOffset)
    {
        PutBit(bsbuffer, 0);
    }
}

static void CodecHal_PackSliceHeader_SetInitialRefPicList(
    PCODECHAL_ENCODE_AVC_PACK_SLC_HEADER_PARAMS params)
{
    PCODEC_AVC_ENCODE_SLICE_PARAMS slcParams;
    PCODEC_REF_LIST *              refList;
    CODEC_PIC_REORDER *            picOrder, pTempPicOrder[32];
    CODEC_PICTURE                  picture;
    uint8_t                        i, j, botField;
    uint32_t                       picNum, picOC;
    uint8_t                        topIdx, botIdx, listSize;
    uint32_t                       defaultPicNumOrder[32];
    bool                           reorder;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_NO_STATUS_RETURN(params);
    CODECHAL_ENCODE_CHK_NULL_NO_STATUS_RETURN(params->pAvcSliceParams);
    CODECHAL_ENCODE_CHK_NULL_NO_STATUS_RETURN(params->ppRefList);

    slcParams = params->pAvcSliceParams;
    refList   = params->ppRefList;
    reorder   = false;
    topIdx    = 0;
    botIdx    = 0;
    listSize  = 0;

    if (params->wPictureCodingType == P_TYPE)
    {
        CodecHal_PackSliceHeader_GetPicNum(params, 0);  // list 0
        picOrder = &slcParams->PicOrder[0][0];
        // Save the default pic order.
        for (i = 0; i < (slcParams->num_ref_idx_l0_active_minus1 + 1); i++)
        {
            defaultPicNumOrder[i] = picOrder[i].PicNum;
        }
        for (i = 1; i < (slcParams->num_ref_idx_l0_active_minus1 + 1); i++)
        {
            picNum  = picOrder[i].PicNum;
            picture = picOrder[i].Picture;
            picOC   = picOrder[i].POC;
            j       = i;
            while ((j > 0) && (picOrder[j - 1].PicNum < picNum))
            {
                picOrder[j].PicNum  = picOrder[j - 1].PicNum;
                picOrder[j].Picture = picOrder[j - 1].Picture;
                picOrder[j].POC     = picOrder[j - 1].POC;
                j--;
                reorder = true;
            }
            picOrder[j].PicNum  = picNum;
            picOrder[j].Picture = picture;
            picOrder[j].POC     = picOC;
        }

        // Sort picOrder[] based on polarity in field case
        if (CodecHal_PictureIsTopField(params->CurrPic))
        {
            while ((topIdx < (slcParams->num_ref_idx_l0_active_minus1 + 1)) ||
                   (botIdx < (slcParams->num_ref_idx_l0_active_minus1 + 1)))
            {
                for (; topIdx < (slcParams->num_ref_idx_l0_active_minus1 + 1); topIdx++)
                {
                    if (CodecHal_PictureIsTopField(picOrder[topIdx].Picture))  //TOP_FIELD
                    {
                        pTempPicOrder[listSize].PicNum  = picOrder[topIdx].PicNum;
                        pTempPicOrder[listSize].Picture = picOrder[topIdx].Picture;
                        pTempPicOrder[listSize].POC     = picOrder[topIdx].POC;
                        listSize++;
                        topIdx++;
                        break;
                    }
                }
                for (; botIdx < (slcParams->num_ref_idx_l0_active_minus1 + 1); botIdx++)
                {
                    if (CodecHal_PictureIsBottomField(picOrder[botIdx].Picture))  //BOTTOM_FIELD
                    {
                        pTempPicOrder[listSize].PicNum  = picOrder[botIdx].PicNum;
                        pTempPicOrder[listSize].Picture = picOrder[botIdx].Picture;
                        pTempPicOrder[listSize].POC     = picOrder[botIdx].POC;
                        listSize++;
                        botIdx++;
                        break;
                    }
                }
            }
        }
        if (CodecHal_PictureIsBottomField(params->CurrPic))
        {
            while ((topIdx < (slcParams->num_ref_idx_l0_active_minus1 + 1)) ||
                   (botIdx < (slcParams->num_ref_idx_l0_active_minus1 + 1)))
            {
                for (; botIdx < (slcParams->num_ref_idx_l0_active_minus1 + 1); botIdx++)
                {
                    if (CodecHal_PictureIsBottomField(picOrder[botIdx].Picture))  //BOTTOM_FIELD
                    {
                        pTempPicOrder[listSize].PicNum  = picOrder[botIdx].PicNum;
                        pTempPicOrder[listSize].Picture = picOrder[botIdx].Picture;
                        pTempPicOrder[listSize].POC     = picOrder[botIdx].POC;
                        listSize++;
                        botIdx++;
                        break;
                    }
                }
                for (; topIdx < (slcParams->num_ref_idx_l0_active_minus1 + 1); topIdx++)
                {
                    if (CodecHal_PictureIsTopField(picOrder[topIdx].Picture))  //TOP_FIELD
                    {
                        pTempPicOrder[listSize].PicNum  = picOrder[topIdx].PicNum;
                        pTempPicOrder[listSize].Picture = picOrder[topIdx].Picture;
                        pTempPicOrder[listSize].POC     = picOrder[topIdx].POC;
                        listSize++;
                        topIdx++;
                        break;
                    }
                }
            }
        }

        if (!CodecHal_PictureIsFrame(params->CurrPic))
        {
            listSize = MOS_MIN(listSize, 32);
            // Copy temp array back to picOrder[]
            for (i = 0; i < listSize; i++)
            {
                picOrder[i].PicNum  = pTempPicOrder[i].PicNum;
                picOrder[i].Picture = pTempPicOrder[i].Picture;
                picOrder[i].POC     = pTempPicOrder[i].POC;
            }

            // Check if picOrder[] has been shuffled compared to the original list
            reorder = false;
            for (i = 0; i < (slcParams->num_ref_idx_l0_active_minus1 + 1); i++)
            {
                if (defaultPicNumOrder[i] != picOrder[i].PicNum)
                {
                    reorder = true;
                    break;
                }
            }
        }

        if (reorder)
        {
            slcParams->ref_pic_list_reordering_flag_l0 = 1;
        }
        else
        {
            slcParams->ref_pic_list_reordering_flag_l0 = 0;
        }
        for (i = 0; i < (slcParams->num_ref_idx_l0_active_minus1 + 1); i++)
        {
            botField                                                         = (CodecHal_PictureIsBottomField(picOrder[i].Picture)) ? 1 : 0;
            refList[picOrder[i].Picture.FrameIdx]->ucInitialIdx[0][botField] = i;
        }
    }
    if (params->wPictureCodingType == B_TYPE)
    {
    }

    return;
}

static void CodecHal_PackSliceHeader_GetPicNum(
    PCODECHAL_ENCODE_AVC_PACK_SLC_HEADER_PARAMS params,
    uint8_t                                     list)
{
    PCODEC_AVC_ENCODE_SLICE_PARAMS slcParams;
    PCODEC_REF_LIST *              refList;
    uint32_t                       frameNum, frameNumWrap, picNum;
    uint8_t                        i, botField, size;
    CODEC_PICTURE                  picture;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_NO_STATUS_RETURN(params);
    CODECHAL_ENCODE_CHK_NULL_NO_STATUS_RETURN(params->pAvcSliceParams);
    CODECHAL_ENCODE_CHK_NULL_NO_STATUS_RETURN(params->ppRefList);

    slcParams = params->pAvcSliceParams;
    refList   = params->ppRefList;

    size = list ? (slcParams->num_ref_idx_l1_active_minus1 + 1) : (slcParams->num_ref_idx_l0_active_minus1 + 1);

    for (i = 0; i < size; i++)
    {
        picture                                               = slcParams->PicOrder[list][i].Picture;
        botField                                              = (CodecHal_PictureIsBottomField(picture)) ? 1 : 0;
        refList[picture.FrameIdx]->ucFinalIdx[list][botField] = i;
        frameNum                                              = refList[picture.FrameIdx]->sFrameNumber;
        if (frameNum > (uint32_t)refList[params->CurrReconPic.FrameIdx]->sFrameNumber)
        {
            frameNumWrap = frameNum - slcParams->MaxFrameNum;
        }
        else
        {
            frameNumWrap = frameNum;
        }

        if (CodecHal_PictureIsFrame(params->CurrPic))
        {
            picNum = frameNumWrap;
        }
        else if ((CodecHal_PictureIsTopField(params->CurrPic) &&
                     CodecHal_PictureIsTopField(picture)) ||
                 (CodecHal_PictureIsBottomField(params->CurrPic) &&
                     CodecHal_PictureIsBottomField(picture)))
        {
            // Same polarity
            picNum = (frameNumWrap << 1) + 1;
        }
        else
        {
            picNum = frameNumWrap << 1;
        }
        slcParams->PicOrder[list][i].PicNum = picNum;
        slcParams->PicOrder[list][i].POC =
            refList[picture.FrameIdx]->iFieldOrderCnt[botField];
    }

    return;
}

static MOS_STATUS CodecHal_PackSliceHeader_SetRefPicListParam(
    PCODECHAL_ENCODE_AVC_PACK_SLC_HEADER_PARAMS params,
    uint8_t                                     list)
{
    PCODEC_AVC_ENCODE_SLICE_PARAMS slcParams;
    PCODEC_REF_LIST *              refList;
    PCODEC_PIC_REORDER             picOrder;
    uint8_t                        i, j, idx, picIdx, numReorder, numActiveMinus1, refPolarity;
    uint32_t                       picNumPred, currPicNum, picNumNoWrap, maxPicNum;
    int16_t                        frameNum;
    MOS_STATUS                     eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(params);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pAvcSliceParams);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->ppRefList);

    slcParams = params->pAvcSliceParams;
    refList   = params->ppRefList;
    frameNum  = refList[params->CurrReconPic.FrameIdx]->sFrameNumber;

    currPicNum = (CodecHal_PictureIsFrame(params->CurrPic)) ? frameNum : 2 * frameNum + 1;
    picNumPred = currPicNum;
    maxPicNum  = (CodecHal_PictureIsFrame(params->CurrPic)) ? slcParams->MaxFrameNum : (2 * slcParams->MaxFrameNum);

    numActiveMinus1 = list ? slcParams->num_ref_idx_l1_active_minus1 : slcParams->num_ref_idx_l0_active_minus1;

    picOrder = &slcParams->PicOrder[list][0];

    idx         = 0;
    picIdx      = picOrder[idx].Picture.FrameIdx;
    refPolarity = (CodecHal_PictureIsBottomField(picOrder[idx].Picture)) ? 1 : 0;
    if (refList[picIdx]->ucFinalIdx[list][refPolarity] ==
        refList[picIdx]->ucInitialIdx[list][refPolarity])
    {
        // Should never happen, something must be wrong in CodecHal_PackSliceHeader_SetInitialRefPicList()
        CODECHAL_ENCODE_ASSERT(false);
        if (list)  // L1
        {
            slcParams->ref_pic_list_reordering_flag_l1 = 0;
        }
        else  // L0
        {
            slcParams->ref_pic_list_reordering_flag_l0 = 0;
        }
        eStatus = MOS_STATUS_UNKNOWN;
        return eStatus;
    }

    numReorder = refList[picIdx]->ucFinalIdx[list][refPolarity] - refList[picIdx]->ucInitialIdx[list][refPolarity];
    if (numReorder > numActiveMinus1)
    {
        numReorder = numActiveMinus1;
    }
    slcParams->NumReorder = numReorder;
    do
    {
        for (i = (idx + 1); i <= numActiveMinus1; i++)
        {
            picIdx      = picOrder[i].Picture.FrameIdx;
            refPolarity = (CodecHal_PictureIsBottomField(picOrder[i].Picture)) ? 1 : 0;
            if (refList[picIdx]->ucFinalIdx[list][refPolarity] == idx)
            {
                break;
            }
        }
        if (i == (numActiveMinus1 + 1))
        {
            // Should never happen, something must be wrong
            CODECHAL_ENCODE_ASSERT(false);
            if (list)  // L1
            {
                slcParams->ref_pic_list_reordering_flag_l1 = 0;
            }
            else  // L0
            {
                slcParams->ref_pic_list_reordering_flag_l0 = 0;
            }
            eStatus = MOS_STATUS_UNKNOWN;
            return eStatus;
        }

        if (picOrder[i].PicNum > picNumPred)
        {
            picOrder[idx].ReorderPicNumIDC = 1;
        }
        else
        {
            picOrder[idx].ReorderPicNumIDC = 0;
        }

        if (picOrder[i].PicNum > currPicNum)
        {
            picNumNoWrap = picOrder[i].PicNum + maxPicNum;
        }
        else
        {
            picNumNoWrap = picOrder[i].PicNum;
        }

        if (picOrder[idx].ReorderPicNumIDC == 0)
        {
            if (picNumPred > picNumNoWrap)
            {
                picOrder[idx].DiffPicNumMinus1 = picNumPred - picNumNoWrap - 1;
            }
            else
            {
                picOrder[idx].DiffPicNumMinus1 = picNumPred + maxPicNum - picNumNoWrap - 1;
            }
        }
        else
        {
            if (picNumNoWrap > picNumPred)
            {
                picOrder[idx].DiffPicNumMinus1 = picNumNoWrap - picNumPred - 1;
            }
            else
            {
                picOrder[idx].DiffPicNumMinus1 = picNumNoWrap + maxPicNum - picNumPred - 1;
            }
        }
        picNumPred = picNumNoWrap;

        for (j = i; j > idx; j--)
        {
            picOrder[j].Picture = picOrder[j - 1].Picture;
            picOrder[j].PicNum  = picOrder[j - 1].PicNum;
            picOrder[j].POC     = picOrder[j - 1].POC;
        }

        idx++;
    } while (--numReorder);
    picOrder[idx].ReorderPicNumIDC = 3;

    return eStatus;
}

static MOS_STATUS CodecHal_PackSliceHeader_PredWeightTable(
    PCODECHAL_ENCODE_AVC_PACK_SLC_HEADER_PARAMS params)
{
    PCODEC_AVC_ENCODE_SLICE_PARAMS slcParams;
    PBSBuffer                      bsbuffer;
    int16_t                        weight, offset, weight2, offset2;
    uint8_t                        i, weight_flag, chromaIDC;
    MOS_STATUS                     eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_NULL_RETURN(params);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pSeqParams);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pAvcSliceParams);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pBsBuffer);

    bsbuffer  = params->pBsBuffer;
    slcParams = params->pAvcSliceParams;
    chromaIDC = params->pSeqParams->chroma_format_idc;

    PutVLCCode(bsbuffer, slcParams->luma_log2_weight_denom);

    if (chromaIDC)
    {
        PutVLCCode(bsbuffer, slcParams->chroma_log2_weight_denom);
    }

    for (i = 0; i <= slcParams->num_ref_idx_l0_active_minus1; i++)
    {
        // Luma
        weight      = slcParams->Weights[0][i][0][0];
        offset      = slcParams->Weights[0][i][0][1];
        weight_flag = (weight != (1 << slcParams->luma_log2_weight_denom)) || (offset != 0);
        PutBit(bsbuffer, weight_flag);
        if (weight_flag)
        {
            PutVLCCode(bsbuffer, SIGNED(weight));
            PutVLCCode(bsbuffer, SIGNED(offset));
        }

        // Chroma
        if (chromaIDC)
        {
            weight      = slcParams->Weights[0][i][1][0];
            offset      = slcParams->Weights[0][i][1][1];
            weight2     = slcParams->Weights[0][i][2][0];
            offset2     = slcParams->Weights[0][i][2][1];
            weight_flag = (weight != (1 << slcParams->chroma_log2_weight_denom)) ||
                          (weight2 != (1 << slcParams->chroma_log2_weight_denom)) ||
                          (offset != 0) || (offset2 != 0);
            PutBit(bsbuffer, weight_flag);
            if (weight_flag)
            {
                PutVLCCode(bsbuffer, SIGNED(weight));
                PutVLCCode(bsbuffer, SIGNED(offset));
                PutVLCCode(bsbuffer, SIGNED(weight2));
                PutVLCCode(bsbuffer, SIGNED(offset2));
            }
        }
    }

    if (Slice_Type[slcParams->slice_type] == SLICE_B)
    {
        for (i = 0; i <= slcParams->num_ref_idx_l1_active_minus1; i++)
        {
            // Luma
            weight      = slcParams->Weights[1][i][0][0];
            offset      = slcParams->Weights[1][i][0][1];
            weight_flag = (weight != (1 << slcParams->luma_log2_weight_denom)) || (offset != 0);
            PutBit(bsbuffer, weight_flag);
            if (weight_flag)
            {
                PutVLCCode(bsbuffer, SIGNED(weight));
                PutVLCCode(bsbuffer, SIGNED(offset));
            }

            // Chroma
            if (chromaIDC)
            {
                weight      = slcParams->Weights[1][i][1][0];
                offset      = slcParams->Weights[1][i][1][1];
                weight2     = slcParams->Weights[1][i][2][0];
                offset2     = slcParams->Weights[1][i][2][1];
                weight_flag = (weight != (1 << slcParams->chroma_log2_weight_denom)) ||
                              (weight2 != (1 << slcParams->chroma_log2_weight_denom)) ||
                              (offset != 0) || (offset2 != 0);
                PutBit(bsbuffer, weight_flag);
                if (weight_flag)
                {
                    PutVLCCode(bsbuffer, SIGNED(weight));
                    PutVLCCode(bsbuffer, SIGNED(offset));
                    PutVLCCode(bsbuffer, SIGNED(weight2));
                    PutVLCCode(bsbuffer, SIGNED(offset2));
                }
            }
        }
    }

    return eStatus;
}

//!
//! \brief    Pack AUD parameters
//!
//! \param    [in] params
//!           Pointer to codechal encode Avc pack picture header parameter
//!
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if call success, else fail reason
//!
MOS_STATUS CodecHal_PackPictureHeader_AUDParams(
    PCODECHAL_ENCODE_AVC_PACK_PIC_HEADER_PARAMS params)
{
    uint32_t   picType;
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_NULL_RETURN(params);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pBsBuffer);

    // refer table 7-5 in H.264 spec on primary_pic_type
    // Here I,P,B types are included
    // According BD Spec 9.5.1.1, 0 - I; 1 - P; 2 - B

    picType = (uint32_t)(params->wPictureCodingType) - 1;
    PutBits(params->pBsBuffer, picType, 3);

    return eStatus;
}

// Refer to H264 picture reordering session
static MOS_STATUS CodecHal_PackSliceHeader_RefPicListReordering(
    PCODECHAL_ENCODE_AVC_PACK_SLC_HEADER_PARAMS params)
{
    PCODEC_AVC_ENCODE_SLICE_PARAMS slcParams;
    PBSBuffer                      bsbuffer;
    CODEC_PIC_REORDER *            picOrder;
    uint8_t                        sliceType;
    MOS_STATUS                     eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_NULL_RETURN(params);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pAvcSliceParams);

    slcParams = params->pAvcSliceParams;
    bsbuffer  = params->pBsBuffer;
    sliceType = Slice_Type[slcParams->slice_type];

    // Generate the initial reference list
    CodecHal_PackSliceHeader_SetInitialRefPicList(params);

    if (sliceType != SLICE_I && sliceType != SLICE_SI)
    {
        if (slcParams->ref_pic_list_reordering_flag_l0)
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHal_PackSliceHeader_SetRefPicListParam(params, 0));

            PutBit(bsbuffer, slcParams->ref_pic_list_reordering_flag_l0);

            if (slcParams->ref_pic_list_reordering_flag_l0)
            {
                picOrder = &slcParams->PicOrder[0][0];
                do
                {
                    PutVLCCode(bsbuffer, picOrder->ReorderPicNumIDC);
                    if (picOrder->ReorderPicNumIDC == 0 ||
                        picOrder->ReorderPicNumIDC == 1)
                    {
                        PutVLCCode(bsbuffer, picOrder->DiffPicNumMinus1);
                    }
                } while ((picOrder++)->ReorderPicNumIDC != 3);
            }
        }
        else
        {
            PutBit(bsbuffer, slcParams->ref_pic_list_reordering_flag_l0);
        }
    }
    if (sliceType == SLICE_B)
    {
        if (slcParams->ref_pic_list_reordering_flag_l1)
        {
            CodecHal_PackSliceHeader_SetRefPicListParam(params, 1);

            PutBit(bsbuffer, slcParams->ref_pic_list_reordering_flag_l1);

            if (slcParams->ref_pic_list_reordering_flag_l1)
            {
                picOrder = &slcParams->PicOrder[1][0];
                do
                {
                    PutVLCCode(bsbuffer, picOrder->ReorderPicNumIDC);
                    if (picOrder->ReorderPicNumIDC == 0 ||
                        picOrder->ReorderPicNumIDC == 1)
                    {
                        PutVLCCode(bsbuffer, picOrder->DiffPicNumMinus1);
                    }
                } while ((picOrder++)->ReorderPicNumIDC != 3);
            }
        }
        else
        {
            PutBit(bsbuffer, slcParams->ref_pic_list_reordering_flag_l1);
        }
    }

    return eStatus;
}

//!
//! \brief    Pack sequence parameters
//!
//! \param    [in] params
//!           Pointer to codechal encode Avc pack picture header parameter
//!
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if call success, else fail reason
//!
MOS_STATUS CodecHal_PackPictureHeader_SeqParams(
    PCODECHAL_ENCODE_AVC_PACK_PIC_HEADER_PARAMS params)
{
    PCODEC_AVC_ENCODE_SEQUENCE_PARAMS seqParams;
    BSBuffer *                        bsbuffer;
    uint8_t                           i;
    MOS_STATUS                        eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_NULL_RETURN(params);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pBsBuffer);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pSeqParams);

    seqParams = params->pSeqParams;
    bsbuffer  = params->pBsBuffer;

    PutBits(bsbuffer, seqParams->Profile, 8);

    PutBit(bsbuffer, seqParams->constraint_set0_flag);
    PutBit(bsbuffer, seqParams->constraint_set1_flag);
    PutBit(bsbuffer, seqParams->constraint_set2_flag);
    PutBit(bsbuffer, seqParams->constraint_set3_flag);

    PutBits(bsbuffer, 0, 4);
    PutBits(bsbuffer, seqParams->Level, 8);
    PutVLCCode(bsbuffer, seqParams->seq_parameter_set_id);

    if (seqParams->Profile == CODEC_AVC_HIGH_PROFILE ||
        seqParams->Profile == CODEC_AVC_HIGH10_PROFILE ||
        seqParams->Profile == CODEC_AVC_HIGH422_PROFILE ||
        seqParams->Profile == CODEC_AVC_HIGH444_PROFILE ||
        seqParams->Profile == CODEC_AVC_CAVLC444_INTRA_PROFILE ||
        seqParams->Profile == CODEC_AVC_SCALABLE_BASE_PROFILE ||
        seqParams->Profile == CODEC_AVC_SCALABLE_HIGH_PROFILE)
    {
        PutVLCCode(bsbuffer, seqParams->chroma_format_idc);
        if (seqParams->chroma_format_idc == 3)
        {
            PutBit(bsbuffer, seqParams->separate_colour_plane_flag);
        }
        PutVLCCode(bsbuffer, seqParams->bit_depth_luma_minus8);
        PutVLCCode(bsbuffer, seqParams->bit_depth_chroma_minus8);
        PutBit(bsbuffer, seqParams->qpprime_y_zero_transform_bypass_flag);
        PutBit(bsbuffer, seqParams->seq_scaling_matrix_present_flag);
        if (seqParams->seq_scaling_matrix_present_flag)
        {
            //Iterate thro' the scaling lists. Refer to ITU-T H.264 std. section 7.3.2.1
            for (i = 0; i < 8; i++)
            {
                // scaling list present flag
                PutBit(bsbuffer, seqParams->seq_scaling_list_present_flag[i]);
                if (seqParams->seq_scaling_list_present_flag[i])
                {
                    if (i < 6)
                    {
                        PackScalingList(bsbuffer, &params->pAvcIQMatrixParams->ScalingList4x4[i][0], 16);
                    }
                    else
                    {
                        PackScalingList(bsbuffer, &params->pAvcIQMatrixParams->ScalingList8x8[i - 6][0], 64);
                    }
                }
            }
        }
    }

    PutVLCCode(bsbuffer, seqParams->log2_max_frame_num_minus4);
    PutVLCCode(bsbuffer, seqParams->pic_order_cnt_type);
    if (seqParams->pic_order_cnt_type == 0)
    {
        PutVLCCode(bsbuffer, seqParams->log2_max_pic_order_cnt_lsb_minus4);
    }
    else if (seqParams->pic_order_cnt_type == 1)
    {
        PutBit(bsbuffer, seqParams->delta_pic_order_always_zero_flag);
        PutVLCCode(bsbuffer, SIGNED(seqParams->offset_for_non_ref_pic));
        PutVLCCode(bsbuffer, SIGNED(seqParams->offset_for_top_to_bottom_field));
        PutVLCCode(bsbuffer, seqParams->num_ref_frames_in_pic_order_cnt_cycle);
        for (i = 0; i < seqParams->num_ref_frames_in_pic_order_cnt_cycle; i++)
        {
            PutVLCCode(bsbuffer, SIGNED(seqParams->offset_for_ref_frame[i]));
        }
    }

    PutVLCCode(bsbuffer, seqParams->NumRefFrames);
    PutBit(bsbuffer, seqParams->gaps_in_frame_num_value_allowed_flag);
    PutVLCCode(bsbuffer, seqParams->pic_width_in_mbs_minus1);
    PutVLCCode(bsbuffer, seqParams->pic_height_in_map_units_minus1);
    PutBit(bsbuffer, seqParams->frame_mbs_only_flag);

    if (!seqParams->frame_mbs_only_flag)
    {
        PutBit(bsbuffer, seqParams->mb_adaptive_frame_field_flag);
    }

    PutBit(bsbuffer, seqParams->direct_8x8_inference_flag);

    if ((!seqParams->frame_cropping_flag) &&
        (params->dwFrameHeight != params->dwOriFrameHeight))
    {
        seqParams->frame_cropping_flag = 1;
        seqParams->frame_crop_bottom_offset =
            (int16_t)((params->dwFrameHeight - params->dwOriFrameHeight) >>
                      (2 - seqParams->frame_mbs_only_flag));  // 4:2:0
    }

    PutBit(bsbuffer, seqParams->frame_cropping_flag);

    if (seqParams->frame_cropping_flag)
    {
        PutVLCCode(bsbuffer, seqParams->frame_crop_left_offset);
        PutVLCCode(bsbuffer, seqParams->frame_crop_right_offset);
        PutVLCCode(bsbuffer, seqParams->frame_crop_top_offset);
        PutVLCCode(bsbuffer, seqParams->frame_crop_bottom_offset);
    }

    PutBit(bsbuffer, seqParams->vui_parameters_present_flag);

    if (seqParams->vui_parameters_present_flag)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHal_PackPictureHeader_VuiParams(params));
    }

    *params->pbNewSeqHeader = 1;

    return eStatus;
}

//!
//! \brief    Pack picture parameters
//!
//! \param    [in] params
//!           Pointer to codechal encode Avc pack picture header parameter
//!
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if call success, else fail reason
//!
MOS_STATUS CodecHal_PackPictureHeader_PicParams(
    PCODECHAL_ENCODE_AVC_PACK_PIC_HEADER_PARAMS params)
{
    PCODEC_AVC_ENCODE_SEQUENCE_PARAMS seqParams;
    PCODEC_AVC_ENCODE_PIC_PARAMS      picParams;
    PBSBuffer                         bsbuffer;
    MOS_STATUS                        eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_NULL_RETURN(params);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pSeqParams);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pPicParams);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pBsBuffer);

    seqParams = params->pSeqParams;
    picParams = params->pPicParams;
    bsbuffer  = params->pBsBuffer;

    PutVLCCode(bsbuffer, picParams->pic_parameter_set_id);
    PutVLCCode(bsbuffer, picParams->seq_parameter_set_id);

    PutBit(bsbuffer, picParams->entropy_coding_mode_flag);
    PutBit(bsbuffer, picParams->pic_order_present_flag);

    PutVLCCode(bsbuffer, picParams->num_slice_groups_minus1);

    PutVLCCode(bsbuffer, picParams->num_ref_idx_l0_active_minus1);
    PutVLCCode(bsbuffer, picParams->num_ref_idx_l1_active_minus1);

    PutBit(bsbuffer, picParams->weighted_pred_flag);
    PutBits(bsbuffer, picParams->weighted_bipred_idc, 2);

    PutVLCCode(bsbuffer, SIGNED(picParams->pic_init_qp_minus26));
    PutVLCCode(bsbuffer, SIGNED(picParams->pic_init_qs_minus26));
    PutVLCCode(bsbuffer, SIGNED(picParams->chroma_qp_index_offset));

    PutBit(bsbuffer, picParams->deblocking_filter_control_present_flag);
    PutBit(bsbuffer, picParams->constrained_intra_pred_flag);
    PutBit(bsbuffer, picParams->redundant_pic_cnt_present_flag);

    // The syntax elements transform_8x8_mode_flag, pic_scaling_matrix_present_flag, and second_chroma_qp_index_offset
    // shall not be present for main profile
    if (seqParams->Profile == CODEC_AVC_MAIN_PROFILE || seqParams->Profile == CODEC_AVC_BASE_PROFILE)
    {
        return eStatus;
    }

    PutBit(bsbuffer, picParams->transform_8x8_mode_flag);
    PutBit(bsbuffer, picParams->pic_scaling_matrix_present_flag);
    if (picParams->pic_scaling_matrix_present_flag)
    {
        uint8_t i;

        //Iterate thro' the scaling lists. Refer to ITU-T H.264 std. section 7.3.2.2
        for (i = 0; i < 6 + 2 * picParams->transform_8x8_mode_flag; i++)
        {
            //Put scaling list present flag
            PutBit(bsbuffer, picParams->pic_scaling_list_present_flag[i]);
            if (picParams->pic_scaling_list_present_flag[i])
            {
                if (i < 6)
                {
                    PackScalingList(bsbuffer, &params->pAvcIQMatrixParams->ScalingList4x4[i][0], 16);
                }
                else
                {
                    PackScalingList(bsbuffer, &params->pAvcIQMatrixParams->ScalingList8x8[i - 6][0], 64);
                }
            }
        }
    }

    PutVLCCode(bsbuffer, SIGNED(picParams->second_chroma_qp_index_offset));

    *params->pbNewPPSHeader = 1;

    return eStatus;
}

MOS_STATUS CodecHalAvcEncode_PackPictureHeader(
    PCODECHAL_ENCODE_AVC_PACK_PIC_HEADER_PARAMS params)
{
    PBSBuffer  bsbuffer;
    uint32_t   indexNALUnit;
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_NULL_RETURN(params);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pBsBuffer);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pSeqParams);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->ppNALUnitParams);

    bsbuffer              = params->pBsBuffer;
    *(bsbuffer->pBase)    = 0;  // init first byte to 0
    bsbuffer->pCurrent    = bsbuffer->pBase;
    bsbuffer->SliceOffset = 0;
    bsbuffer->BitOffset   = 0;
    bsbuffer->BitSize     = 0;

    MOS_ZeroMemory(params->ppNALUnitParams[0], sizeof(CODECHAL_NAL_UNIT_PARAMS) * CODECHAL_ENCODE_AVC_MAX_NAL_TYPE);
    indexNALUnit = 0;

    // AU_Delimiter
    // nal_ref_idc to be 0 for all nal_unit_type equal to 6, 9, 10, 11 or12
    params->ppNALUnitParams[indexNALUnit]->uiOffset                  = 0;
    params->ppNALUnitParams[indexNALUnit]->uiNalUnitType             = CODECHAL_ENCODE_AVC_NAL_UT_AUD;
    params->ppNALUnitParams[indexNALUnit]->bInsertEmulationBytes     = true;
    params->ppNALUnitParams[indexNALUnit]->uiSkipEmulationCheckCount = 4;
    SetNalUnit(&bsbuffer->pCurrent, 0, CODECHAL_ENCODE_AVC_NAL_UT_AUD);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHal_PackPictureHeader_AUDParams(params));
    SetTrailingBits(bsbuffer);
    //NAL unit are byte aligned, bsbuffer->BitOffset should be 0
    params->ppNALUnitParams[indexNALUnit]->uiSize =
        (uint32_t)(bsbuffer->pCurrent -
                   bsbuffer->pBase -
                   params->ppNALUnitParams[indexNALUnit]->uiOffset);
    indexNALUnit++;

    // If this is a new sequence, write the seq set
    if (params->bNewSeq && !params->pSeqParams->bNoAcceleratorSPSInsertion)
    {
        // Pack SPS
        params->ppNALUnitParams[indexNALUnit]->uiOffset                  = (uint32_t)(bsbuffer->pCurrent - bsbuffer->pBase);
        params->ppNALUnitParams[indexNALUnit]->uiNalUnitType             = CODECHAL_ENCODE_AVC_NAL_UT_SPS;
        params->ppNALUnitParams[indexNALUnit]->bInsertEmulationBytes     = true;
        params->ppNALUnitParams[indexNALUnit]->uiSkipEmulationCheckCount = 4;
        SetNalUnit(&bsbuffer->pCurrent, 1, CODECHAL_ENCODE_AVC_NAL_UT_SPS);
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHal_PackPictureHeader_SeqParams(params));
        SetTrailingBits(bsbuffer);
        params->ppNALUnitParams[indexNALUnit]->uiSize =
            (uint32_t)(bsbuffer->pCurrent -
                       bsbuffer->pBase -
                       params->ppNALUnitParams[indexNALUnit]->uiOffset);
        indexNALUnit++;
    }

    // Pack PPS
    params->ppNALUnitParams[indexNALUnit]->uiOffset                  = (uint32_t)(bsbuffer->pCurrent - bsbuffer->pBase);
    params->ppNALUnitParams[indexNALUnit]->uiNalUnitType             = CODECHAL_ENCODE_AVC_NAL_UT_PPS;
    params->ppNALUnitParams[indexNALUnit]->bInsertEmulationBytes     = true;
    params->ppNALUnitParams[indexNALUnit]->uiSkipEmulationCheckCount = 4;
    SetNalUnit(&bsbuffer->pCurrent, 1, CODECHAL_ENCODE_AVC_NAL_UT_PPS);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHal_PackPictureHeader_PicParams(params));
    SetTrailingBits(bsbuffer);
    params->ppNALUnitParams[indexNALUnit]->uiSize =
        (uint32_t)(bsbuffer->pCurrent -
                   bsbuffer->pBase -
                   params->ppNALUnitParams[indexNALUnit]->uiOffset);
    indexNALUnit++;

    // Pack SEI
    if (params->pSeiData->newSEIData)
    {
        params->ppNALUnitParams[indexNALUnit]->uiOffset                  = (uint32_t)(bsbuffer->pCurrent - bsbuffer->pBase);
        params->ppNALUnitParams[indexNALUnit]->uiNalUnitType             = CODECHAL_ENCODE_AVC_NAL_UT_SEI;
        params->ppNALUnitParams[indexNALUnit]->bInsertEmulationBytes     = false;
        params->ppNALUnitParams[indexNALUnit]->uiSkipEmulationCheckCount = 4;
        eStatus                                                          = MOS_SecureMemcpy(bsbuffer->pCurrent,
            params->pSeiData->dwSEIBufSize,
            params->pSeiData->pSEIBuffer,
            params->pSeiData->dwSEIBufSize);
        if (eStatus != MOS_STATUS_SUCCESS)
        {
            CODECHAL_ENCODE_ASSERTMESSAGE("Failed to copy memory.");
            return eStatus;
        }
        bsbuffer->pCurrent += params->pSeiData->dwSEIDataSize;
        params->pSeiData->newSEIData = false;
        params->ppNALUnitParams[indexNALUnit]->uiSize =
            (uint32_t)(bsbuffer->pCurrent -
                       bsbuffer->pBase -
                       params->ppNALUnitParams[indexNALUnit]->uiOffset);
        indexNALUnit++;
    }

    bsbuffer->SliceOffset = (uint32_t)(bsbuffer->pCurrent - bsbuffer->pBase);

    return eStatus;
}

uint16_t CodecHalAvcEncode_GetMaxNumSlicesAllowed(
    CODEC_AVC_PROFILE_IDC profileIdc,
    CODEC_AVC_LEVEL_IDC   levelIdc,
    uint32_t              framesPer100Sec)
{
    uint16_t maxAllowedNumSlices = 0;

    if ((profileIdc == CODEC_AVC_MAIN_PROFILE) ||
        (profileIdc == CODEC_AVC_HIGH_PROFILE) ||
        (profileIdc == CODEC_AVC_HIGH10_PROFILE) ||
        (profileIdc == CODEC_AVC_HIGH422_PROFILE) ||
        (profileIdc == CODEC_AVC_HIGH444_PROFILE) ||
        (profileIdc == CODEC_AVC_CAVLC444_INTRA_PROFILE))
    {
        switch (levelIdc)
        {
        case CODEC_AVC_LEVEL_3:
            maxAllowedNumSlices = (uint16_t)(40500.0 * 100 / 22.0 / framesPer100Sec);
            break;
        case CODEC_AVC_LEVEL_31:
            maxAllowedNumSlices = (uint16_t)(108000.0 * 100 / 60.0 / framesPer100Sec);
            break;
        case CODEC_AVC_LEVEL_32:
            maxAllowedNumSlices = (uint16_t)(216000.0 * 100 / 60.0 / framesPer100Sec);
            break;
        case CODEC_AVC_LEVEL_4:
        case CODEC_AVC_LEVEL_41:
            maxAllowedNumSlices = (uint16_t)(245760.0 * 100 / 24.0 / framesPer100Sec);
            break;
        case CODEC_AVC_LEVEL_42:
            maxAllowedNumSlices = (uint16_t)(522240.0 * 100 / 24.0 / framesPer100Sec);
            break;
        case CODEC_AVC_LEVEL_5:
            maxAllowedNumSlices = (uint16_t)(589824.0 * 100 / 24.0 / framesPer100Sec);
            break;
        case CODEC_AVC_LEVEL_51:
            maxAllowedNumSlices = (uint16_t)(983040.0 * 100 / 24.0 / framesPer100Sec);
            break;
        case CODEC_AVC_LEVEL_52:
            maxAllowedNumSlices = (uint16_t)(2073600.0 * 100 / 24.0 / framesPer100Sec);
            break;
        default:
            maxAllowedNumSlices = 0;
        }
    }

    return maxAllowedNumSlices;
}

//Pack Slice Header
MOS_STATUS CodecHalAvcEncode_PackSliceHeader(
    PCODECHAL_ENCODE_AVC_PACK_SLC_HEADER_PARAMS params)
{
    PCODEC_AVC_ENCODE_SEQUENCE_PARAMS seqParams;
    PCODEC_AVC_ENCODE_PIC_PARAMS      picParams;
    PCODEC_AVC_ENCODE_SLICE_PARAMS    slcParams;
    PBSBuffer                         bsbuffer;
    uint8_t                           sliceType;
    CODECHAL_ENCODE_AVC_NAL_UNIT_TYPE nalType;
    bool                              ref;
    MOS_STATUS                        eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_NULL_RETURN(params);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pSeqParams);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pPicParams);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pAvcSliceParams);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pBsBuffer);

    slcParams = params->pAvcSliceParams;
    picParams = params->pPicParams;
    seqParams = params->pSeqParams;
    bsbuffer  = params->pBsBuffer;
    sliceType = Slice_Type[slcParams->slice_type];
    nalType   = params->NalUnitType;
    ref       = params->ppRefList[params->CurrReconPic.FrameIdx]->bUsedAsRef;

    // Make slice header uint8_t aligned
    while (bsbuffer->BitOffset)
    {
        PutBit(bsbuffer, 0);
    }

    // zero byte shall exist when the byte stream NAL unit syntax structure contains the first
    // NAL unit of an access unit in decoding order, as specified by subclause 7.4.1.2.3.
    // VDEnc Slice header packing handled by PAK does not need the 0 byte inserted
    if (params->UserFlags.bDisableAcceleratorHeaderPacking && (!params->bVdencEnabled))
    {
        *bsbuffer->pCurrent = 0;
        bsbuffer->pCurrent++;
    }

    SetNalUnit(&bsbuffer->pCurrent, (uint8_t)ref, nalType);

    PutVLCCode(bsbuffer, slcParams->first_mb_in_slice);
    PutVLCCode(bsbuffer, slcParams->slice_type);
    PutVLCCode(bsbuffer, slcParams->pic_parameter_set_id);

    if (seqParams->separate_colour_plane_flag)
    {
        PutBits(bsbuffer, slcParams->colour_plane_id, 2);
    }

    PutBits(bsbuffer, slcParams->frame_num, seqParams->log2_max_frame_num_minus4 + 4);

    if (!seqParams->frame_mbs_only_flag)
    {
        PutBit(bsbuffer, slcParams->field_pic_flag);
        if (slcParams->field_pic_flag)
        {
            PutBit(bsbuffer, slcParams->bottom_field_flag);
        }
    }

    if (nalType == CODECHAL_ENCODE_AVC_NAL_UT_IDR_SLICE)
    {
        PutVLCCode(bsbuffer, slcParams->idr_pic_id);
    }

    if (seqParams->pic_order_cnt_type == 0)
    {
        PutBits(bsbuffer, slcParams->pic_order_cnt_lsb, seqParams->log2_max_pic_order_cnt_lsb_minus4 + 4);
        if (picParams->pic_order_present_flag && !slcParams->field_pic_flag)
        {
            PutVLCCode(bsbuffer, SIGNED(slcParams->delta_pic_order_cnt_bottom));
        }
    }

    if (seqParams->pic_order_cnt_type == 1 && !seqParams->delta_pic_order_always_zero_flag)
    {
        PutVLCCode(bsbuffer, SIGNED(slcParams->delta_pic_order_cnt[0]));
        if (picParams->pic_order_present_flag && !slcParams->field_pic_flag)
        {
            PutVLCCode(bsbuffer, SIGNED(slcParams->delta_pic_order_cnt[1]));
        }
    }

    if (picParams->redundant_pic_cnt_present_flag)
    {
        PutVLCCode(bsbuffer, slcParams->redundant_pic_cnt);
    }

    if (sliceType == SLICE_B)
    {
        PutBit(bsbuffer, slcParams->direct_spatial_mv_pred_flag);
    }

    if (sliceType == SLICE_P || sliceType == SLICE_SP || sliceType == SLICE_B)
    {
        PutBit(bsbuffer, slcParams->num_ref_idx_active_override_flag);
        if (slcParams->num_ref_idx_active_override_flag)
        {
            PutVLCCode(bsbuffer, slcParams->num_ref_idx_l0_active_minus1);
            if (sliceType == SLICE_B)
            {
                PutVLCCode(bsbuffer, slcParams->num_ref_idx_l1_active_minus1);
            }
        }
    }

    // ref_pic_list_reordering()
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHal_PackSliceHeader_RefPicListReordering(params));

    if ((picParams->weighted_pred_flag &&
            (sliceType == SLICE_P || sliceType == SLICE_SP)) ||
        (picParams->weighted_bipred_idc == EXPLICIT_WEIGHTED_INTER_PRED_MODE &&
            sliceType == SLICE_B))
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHal_PackSliceHeader_PredWeightTable(params));
    }

    // dec_ref_pic_marking()
    if (nalType == CODECHAL_ENCODE_AVC_NAL_UT_IDR_SLICE)
    {
        PutBit(bsbuffer, slcParams->no_output_of_prior_pics_flag);
        PutBit(bsbuffer, slcParams->long_term_reference_flag);
    }
    else if (ref)
    {
        PutBit(bsbuffer, slcParams->adaptive_ref_pic_marking_mode_flag);
    }

    if (picParams->entropy_coding_mode_flag && sliceType != SLICE_I && sliceType != SLICE_SI)
    {
        PutVLCCode(bsbuffer, slcParams->cabac_init_idc);
    }

    PutVLCCode(bsbuffer, SIGNED(slcParams->slice_qp_delta));

    if (sliceType == SLICE_SP || sliceType == SLICE_SI)
    {
        if (sliceType == SLICE_SP)
        {
            PutBit(bsbuffer, slcParams->sp_for_switch_flag);
        }
        PutVLCCode(bsbuffer, SIGNED(slcParams->slice_qs_delta));
    }

    if (picParams->deblocking_filter_control_present_flag)
    {
        PutVLCCode(bsbuffer, slcParams->disable_deblocking_filter_idc);
        if (slcParams->disable_deblocking_filter_idc != 1)
        {
            PutVLCCode(bsbuffer, SIGNED(slcParams->slice_alpha_c0_offset_div2));
            PutVLCCode(bsbuffer, SIGNED(slcParams->slice_beta_offset_div2));
        }
    }

    bsbuffer->BitSize =
        (uint32_t)((bsbuffer->pCurrent - bsbuffer->SliceOffset - bsbuffer->pBase) * 8 + bsbuffer->BitOffset);
    bsbuffer->SliceOffset =
        (uint32_t)(bsbuffer->pCurrent - bsbuffer->pBase + (bsbuffer->BitOffset != 0));

    return eStatus;
}

MOS_STATUS CodechalEncodeAvcBase::InitMmcState()
{
#ifdef _MMC_SUPPORTED
    m_mmcState = MOS_New(CodechalMmcEncodeAvc, m_hwInterface, this);
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_mmcState);
#endif
    return MOS_STATUS_SUCCESS;
}

CodechalEncodeAvcBase::CodechalEncodeAvcBase(
    CodechalHwInterface *   hwInterface,
    CodechalDebugInterface *debugInterface,
    PCODECHAL_STANDARD_INFO standardInfo) : CodechalEncoderState(hwInterface, debugInterface, standardInfo)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    MOS_ZeroMemory(m_avcPicParams, CODEC_AVC_MAX_PPS_NUM * sizeof(PCODEC_AVC_ENCODE_PIC_PARAMS));
    MOS_ZeroMemory(m_avcSeqParams, CODEC_AVC_MAX_SPS_NUM * sizeof(PCODEC_AVC_ENCODE_SEQUENCE_PARAMS));
    MOS_ZeroMemory(&m_userFlags, sizeof(CODEC_AVC_ENCODE_USER_FLAGS));
    MOS_ZeroMemory(m_picIdx, CODEC_AVC_MAX_NUM_REF_FRAME * sizeof(CODEC_PIC_ID));
    MOS_ZeroMemory(m_refList, CODEC_AVC_NUM_UNCOMPRESSED_SURFACE * sizeof(PCODEC_REF_LIST));
    MOS_ZeroMemory(m_avcFrameStoreID, CODEC_AVC_MAX_NUM_REF_FRAME * sizeof(CODEC_AVC_FRAME_STORE_ID));
    MOS_ZeroMemory(&m_nalUnitType, sizeof(CODECHAL_ENCODE_AVC_NAL_UNIT_TYPE));
    MOS_ZeroMemory(&m_trellisQuantParams, sizeof(CODECHAL_ENCODE_AVC_TQ_PARAMS));
    MOS_ZeroMemory(m_distScaleFactorList0, 2 * CODEC_AVC_MAX_NUM_REF_FRAME * sizeof(uint32_t));
    MOS_ZeroMemory(m_batchBufferForVdencImgStat, CODECHAL_ENCODE_RECYCLED_BUFFER_NUM * sizeof(MHW_BATCH_BUFFER));
    MOS_ZeroMemory(&m_4xMeMvDataBuffer, sizeof(MOS_SURFACE));
    MOS_ZeroMemory(&m_16xMeMvDataBuffer, sizeof(MOS_SURFACE));
    MOS_ZeroMemory(&m_32xMeMvDataBuffer, sizeof(MOS_SURFACE));
    MOS_ZeroMemory(&m_4xMeDistortionBuffer, sizeof(MOS_SURFACE));
    MOS_ZeroMemory(&m_intraRowStoreScratchBuffer, sizeof(MOS_RESOURCE));
    MOS_ZeroMemory(m_batchBufferForPakSlices, CODECHAL_ENCODE_RECYCLED_BUFFER_NUM * sizeof(MHW_BATCH_BUFFER));
    MOS_ZeroMemory(&m_pakSliceSizeStreamoutBuffer, sizeof(MOS_RESOURCE));

    m_hwInterface->GetStateHeapSettings()->dwNumSyncTags = CODECHAL_ENCODE_AVC_NUM_SYNC_TAGS;
    m_hwInterface->GetStateHeapSettings()->dwDshSize     = CODECHAL_ENCODE_AVC_INIT_DSH_SIZE;
}

CodechalEncodeAvcBase::~CodechalEncodeAvcBase()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    // Release Ref Lists
    CodecHalFreeDataList(m_refList, CODEC_AVC_NUM_UNCOMPRESSED_SURFACE);

    for (uint8_t i = 0; i < CODECHAL_ENCODE_RECYCLED_BUFFER_NUM; i++)
    {
        ReleaseBatchBufferForPakSlices(i);
    }

    m_osInterface->pfnFreeResource(m_osInterface, &m_intraRowStoreScratchBuffer);

    if (m_encEnabled)
    {
        if (m_hmeSupported)
        {
            if (m_hmeKernel)
            {
                MOS_Delete(m_hmeKernel);
            }
            else
            {
                HmeParams hmeParams;

                MOS_ZeroMemory(&hmeParams, sizeof(hmeParams));
                hmeParams.b4xMeDistortionBufferSupported = true;
                hmeParams.ps16xMeMvDataBuffer            = &m_16xMeMvDataBuffer;
                hmeParams.ps32xMeMvDataBuffer            = &m_32xMeMvDataBuffer;
                hmeParams.ps4xMeDistortionBuffer         = &m_4xMeDistortionBuffer;
                hmeParams.ps4xMeMvDataBuffer             = &m_4xMeMvDataBuffer;
                DestroyMeResources(&hmeParams);
            }
        }
    }

    if (m_sliceSizeStreamoutSupported)
    {
        m_osInterface->pfnFreeResource(m_osInterface, &m_pakSliceSizeStreamoutBuffer);
    }
}

MOS_STATUS CodechalEncodeAvcBase::Initialize(CodechalSetting *settings)
{
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitMmcState());
    return CodechalEncoderState::Initialize(settings);
}

MOS_STATUS CodechalEncodeAvcBase::UserFeatureKeyReport()
{
    return CodechalEncoderState::UserFeatureKeyReport();
}

MOS_STATUS CodechalEncodeAvcBase::Initialize()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    m_hwInterface->GetMfxPrimitiveCommandsDataSize(
        m_mode,
        &m_pakSliceSize,
        &m_pakSlicePatchListSize,
        false);

    // overwrite, must be zero, no patching into second level batch buffer allowed
    m_pakSlicePatchListSize = 0;

    return eStatus;
}

void CodechalEncodeAvcBase::ScalingListFlat()
{
    // 4x4 block
    for (uint8_t idx2 = 0; idx2 < 6; idx2++)
    {
        for (uint8_t idx1 = 0; idx1 < 16; idx1++)
        {
            m_avcIQWeightScaleLists->WeightScale4x4[idx2][idx1] = 16;
        }
    }
    // 8x8 block
    for (uint8_t idx2 = 0; idx2 < 2; idx2++)
    {
        for (uint8_t idx1 = 0; idx1 < 64; idx1++)
        {
            m_avcIQWeightScaleLists->WeightScale8x8[idx2][idx1] = 16;
        }
    }
}

void CodechalEncodeAvcBase::ScalingListFallbackRuleA()
{
    for (uint8_t idx1 = 0; idx1 < 16; idx1++)
    {
        for (uint8_t idx2 = 0; idx2 < 3; idx2++)
        {
            m_avcIQWeightScaleLists->WeightScale4x4[idx2][CODEC_AVC_Qmatrix_scan_4x4[idx1]] =
                CODEC_AVC_Default_4x4_Intra[idx1];
        }
        for (uint8_t idx2 = 3; idx2 < 6; idx2++)
        {
            m_avcIQWeightScaleLists->WeightScale4x4[idx2][CODEC_AVC_Qmatrix_scan_4x4[idx1]] =
                CODEC_AVC_Default_4x4_Inter[idx1];
        }
    }
    // 8x8 block
    for (uint8_t idx1 = 0; idx1 < 64; idx1++)
    {
        m_avcIQWeightScaleLists->WeightScale8x8[0][CODEC_AVC_Qmatrix_scan_8x8[idx1]] =
            CODEC_AVC_Default_8x8_Intra[idx1];
        m_avcIQWeightScaleLists->WeightScale8x8[1][CODEC_AVC_Qmatrix_scan_8x8[idx1]] =
            CODEC_AVC_Default_8x8_Inter[idx1];
    }
}

void CodechalEncodeAvcBase::GetDistScaleFactor()
{
    auto picParams = m_avcPicParam;
    auto refList   = &(m_refList[0]);
    auto picIdx    = &(m_picIdx[0]);

    bool bottom  = CodecHal_PictureIsBottomField(picParams->CurrOriginalPic);
    int  pocCurr = picParams->CurrFieldOrderCnt[bottom];

    MOS_ZeroMemory(m_distScaleFactorList0, sizeof(uint32_t) * CODEC_AVC_MAX_NUM_REF_FRAME * 2);
    for (unsigned int index = 0; index <= m_avcSliceParams->num_ref_idx_l0_active_minus1; index++)
    {
        auto picture = m_avcSliceParams->RefPicList[LIST_0][index];
        if (!CodecHal_PictureIsInvalid(picture))
        {
            auto pictureIdx = picIdx[picture.FrameIdx].ucPicIdx;
            int  pocPic0    = CodecHal_PictureIsBottomField(picture) ? refList[pictureIdx]->iFieldOrderCnt[1] : refList[pictureIdx]->iFieldOrderCnt[0];
            picture         = m_avcSliceParams->RefPicList[LIST_1][0];
            pictureIdx      = picIdx[picture.FrameIdx].ucPicIdx;
            int pocPic1     = CodecHal_PictureIsBottomField(picture) ? refList[pictureIdx]->iFieldOrderCnt[1] : refList[pictureIdx]->iFieldOrderCnt[0];
            int tb          = CodecHal_Clip3(-128, 127, (pocCurr - pocPic0));
            int td          = CodecHal_Clip3(-128, 127, (pocPic1 - pocPic0));
            if (td == 0)
            {
                td = 1;
            }
            int tx = (16384 + ABS(td / 2)) / td;

            m_distScaleFactorList0[index] = CodecHal_Clip3(-1024, 1023, (tb * tx + 32) >> 6);
        }
    }
}

MOS_STATUS CodechalEncodeAvcBase::AllocateBatchBufferForPakSlices(
    uint32_t numSlices,
    uint8_t  numPakPasses,
    uint8_t  currRecycledBufIdx)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MOS_ZeroMemory(
        &m_batchBufferForPakSlices[currRecycledBufIdx],
        sizeof(MHW_BATCH_BUFFER));

    // Get the slice size
    uint32_t size = (numPakPasses + 1) * numSlices * m_pakSliceSize;

    m_batchBufferForPakSlices[currRecycledBufIdx].bSecondLevel = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(Mhw_AllocateBb(
        m_osInterface,
        &m_batchBufferForPakSlices[currRecycledBufIdx],
        nullptr,
        size));

    MOS_LOCK_PARAMS lockFlags;
    MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
    lockFlags.WriteOnly = 1;
    uint8_t *data       = (uint8_t *)m_osInterface->pfnLockResource(
        m_osInterface,
        &m_batchBufferForPakSlices[currRecycledBufIdx].OsResource,
        &lockFlags);

    CODECHAL_ENCODE_CHK_NULL_RETURN(data);

    MOS_ZeroMemory(data, size);
    m_osInterface->pfnUnlockResource(
        m_osInterface,
        &m_batchBufferForPakSlices[currRecycledBufIdx].OsResource);

    return eStatus;
}

MOS_STATUS CodechalEncodeAvcBase::ReleaseBatchBufferForPakSlices(
    uint8_t currRecycledBufIdx)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    if (m_batchBufferForPakSlices[currRecycledBufIdx].iSize)
    {
        Mhw_FreeBb(m_osInterface, &m_batchBufferForPakSlices[currRecycledBufIdx], nullptr);
    }

    return eStatus;
}

//==<Functions>=======================================================

// Calculate BiWeight for AVC B frame
// for AVC, we cannot use the CodecHal_GetBiWeight function since AVC can use B as reference and
// also supports PAFF
int32_t CodechalEncodeAvcBase::GetBiWeight(
    uint32_t distScaleFactorRefID0List0,
    uint16_t weightedBiPredIdc)
{
    int32_t biWeight = 32;  // default value
    if (weightedBiPredIdc != IMPLICIT_WEIGHTED_INTER_PRED_MODE)
    {
        biWeight = 32;
    }
    else
    {
        biWeight = (distScaleFactorRefID0List0 + 2) >> 2;

        if (biWeight != 16 && biWeight != 21 &&
            biWeight != 32 && biWeight != 43 && biWeight != 48)
        {
            biWeight = 32;  // If # of B-pics between two refs is more than 3. VME does not support it.
        }
    }

    return biWeight;
}

MOS_STATUS CodechalEncodeAvcBase::AllocateEncResources()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    uint32_t fieldNumMBs = m_picWidthInMb * ((m_picHeightInMb + 1) >> 1);

    // to be used in CodecHalEncode_TrackedBuffer_AllocateMbCodeMvDataResources() later
    m_mbCodeSize = MOS_ALIGN_CEIL(fieldNumMBs * 16 * 4, CODECHAL_PAGE_SIZE) + fieldNumMBs * 16 * 4;
    m_mvDataSize = MOS_ALIGN_CEIL(fieldNumMBs * (32 * 4), CODECHAL_PAGE_SIZE) +  // top field MV + 4K align for bottom field MV
                   fieldNumMBs * (32 * 4);                                       // bottom field MV

    // allocate 3 + 2 buffers initially
    if ((m_codecFunction == CODECHAL_FUNCTION_ENC_PAK) && (!m_vdencEnabled))
    {
        for (uint8_t j = 0; j < 3; j++)
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_trackedBuf->AllocateMbCodeResources(j));
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_trackedBuf->AllocateMvDataResources(j));
        }

        for (uint8_t k = 0; k < 2; k++)
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_trackedBuf->AllocateMbCodeResources(CODEC_NUM_REF_BUFFERS + k));
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_trackedBuf->AllocateMvDataResources(CODEC_NUM_REF_BUFFERS + k));
        }
    }

    if (m_encEnabled && m_hmeSupported)
    {
        if (m_hmeKernel)
        {
            m_hmeKernel->AllocateResources();
        }
        else
        {
            HmeParams hmeParams;
            MOS_ZeroMemory(&hmeParams, sizeof(hmeParams));
            hmeParams.b4xMeDistortionBufferSupported = true;
            hmeParams.ps4xMeDistortionBuffer         = &m_4xMeDistortionBuffer;
            hmeParams.ps4xMeMvDataBuffer             = &m_4xMeMvDataBuffer;
            hmeParams.ps16xMeMvDataBuffer            = &m_16xMeMvDataBuffer;
            hmeParams.ps32xMeMvDataBuffer            = &m_32xMeMvDataBuffer;

            CODECHAL_ENCODE_CHK_STATUS_RETURN(AllocateResources4xMe(&hmeParams));
            if (m_16xMeSupported)
            {
                CODECHAL_ENCODE_CHK_STATUS_RETURN(AllocateResources16xMe(&hmeParams));
            }
            if (m_32xMeSupported)
            {
                CODECHAL_ENCODE_CHK_STATUS_RETURN(AllocateResources32xMe(&hmeParams));
            }
        }
    }

    return eStatus;
}

MOS_STATUS CodechalEncodeAvcBase::AllocateResources()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodechalEncoderState::AllocateResources());

    // initiate allocation parameters and lock flags
    MOS_ALLOC_GFXRES_PARAMS allocParamsForBufferLinear;
    MOS_ZeroMemory(&allocParamsForBufferLinear, sizeof(MOS_ALLOC_GFXRES_PARAMS));
    allocParamsForBufferLinear.Type     = MOS_GFXRES_BUFFER;
    allocParamsForBufferLinear.TileType = MOS_TILE_LINEAR;
    allocParamsForBufferLinear.Format   = Format_Buffer;

    AllocateEncResources();

    // Allocate Ref Lists
    CodecHalAllocateDataList(
        m_refList,
        CODEC_AVC_NUM_UNCOMPRESSED_SURFACE);

    if (m_pakEnabled && m_mfxInterface->IsIntraRowstoreCacheEnabled() == false)
    {
        // Intra Row Store Scratch buffer
        // 1 cacheline per MB
        allocParamsForBufferLinear.dwBytes  = m_picWidthInMb * CODECHAL_CACHELINE_SIZE;
        allocParamsForBufferLinear.pBufName = "Intra Row Store Scratch Buffer";

        eStatus = (MOS_STATUS)m_osInterface->pfnAllocateResource(
            m_osInterface,
            &allocParamsForBufferLinear,
            &m_intraRowStoreScratchBuffer);

        if (eStatus != MOS_STATUS_SUCCESS)
        {
            CODECHAL_ENCODE_ASSERTMESSAGE("Failed to allocate Intra Row Store Scratch Buffer.");
            return eStatus;
        }
    }

    if (m_sliceSizeStreamoutSupported)
    {
        // PAK Slice Size Streamout Buffer
        allocParamsForBufferLinear.dwBytes  = MOS_ALIGN_CEIL(CODECHAL_ENCODE_SLICESIZE_BUF_SIZE, CODECHAL_PAGE_SIZE);
        allocParamsForBufferLinear.pBufName = "PAK Slice Size Streamout Buffer";

        eStatus = (MOS_STATUS)m_osInterface->pfnAllocateResource(
            m_osInterface,
            &allocParamsForBufferLinear,
            &m_pakSliceSizeStreamoutBuffer);

        if (eStatus != MOS_STATUS_SUCCESS)
        {
            CODECHAL_ENCODE_ASSERTMESSAGE("%s: Failed to allocate Slice Size Streamout Buffer\n", __FUNCTION__);
            return eStatus;
        }
    }

    return eStatus;
}

MOS_STATUS CodechalEncodeAvcBase::SetSequenceStructs()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    auto seqParams = m_avcSeqParam;

    // seq_scaling_matrix_present_flag and chroma_format_idc
    // shall not be present for main profile
    if (seqParams->Profile == CODEC_AVC_MAIN_PROFILE)
    {
        seqParams->seq_scaling_matrix_present_flag = 0;
        for (uint8_t i = 0; i < 12; i++)
        {
            seqParams->seq_scaling_list_present_flag[i] = 0;
        }
        seqParams->chroma_format_idc = 1;
    }
    // high profile chroma_format_idc in the range of 0 to 1 inclusive
    if (seqParams->chroma_format_idc > 1)
    {
        seqParams->chroma_format_idc = 1;
    }

    // main & high profile support only 8bpp
    seqParams->bit_depth_luma_minus8   = 0;
    seqParams->bit_depth_chroma_minus8 = 0;

    seqParams->NumRefFrames = seqParams->NumRefFrames * 2;

    // setup parameters corresponding to H264 bit stream definition
    seqParams->pic_height_in_map_units_minus1       = seqParams->frame_mbs_only_flag ? CODECHAL_GET_HEIGHT_IN_MACROBLOCKS(seqParams->FrameHeight) - 1 : (CODECHAL_GET_HEIGHT_IN_MACROBLOCKS(seqParams->FrameHeight) + 1) / 2 - 1;
    seqParams->pic_width_in_mbs_minus1              = CODECHAL_GET_WIDTH_IN_MACROBLOCKS(seqParams->FrameWidth) - 1;
    seqParams->constraint_set0_flag                 = 0;
    seqParams->constraint_set1_flag                 = (seqParams->Profile == CODEC_AVC_BASE_PROFILE) ? 1 : 0;
    seqParams->constraint_set2_flag                 = 0;
    seqParams->constraint_set3_flag                 = 0;
    seqParams->gaps_in_frame_num_value_allowed_flag = 0;
    seqParams->qpprime_y_zero_transform_bypass_flag = 0;
    seqParams->separate_colour_plane_flag           = 0;

    // setup internal parameters
    m_picWidthInMb  = CODECHAL_GET_WIDTH_IN_MACROBLOCKS(seqParams->FrameWidth);
    m_picHeightInMb = CODECHAL_GET_HEIGHT_IN_MACROBLOCKS(seqParams->FrameHeight);
    m_frameWidth    = m_picWidthInMb * CODECHAL_MACROBLOCK_WIDTH;
    m_frameHeight   = m_picHeightInMb * CODECHAL_MACROBLOCK_HEIGHT;

    // HME Scaling WxH
    m_downscaledWidthInMb4x =
        CODECHAL_GET_WIDTH_IN_MACROBLOCKS(m_frameWidth / SCALE_FACTOR_4x);
    m_downscaledHeightInMb4x =
        CODECHAL_GET_HEIGHT_IN_MACROBLOCKS(m_frameHeight / SCALE_FACTOR_4x);
    m_downscaledWidth4x =
        m_downscaledWidthInMb4x * CODECHAL_MACROBLOCK_WIDTH;
    m_downscaledHeight4x =
        m_downscaledHeightInMb4x * CODECHAL_MACROBLOCK_HEIGHT;

    // SuperHME Scaling WxH
    m_downscaledWidthInMb16x =
        CODECHAL_GET_WIDTH_IN_MACROBLOCKS(m_frameWidth / SCALE_FACTOR_16x);
    m_downscaledHeightInMb16x =
        CODECHAL_GET_HEIGHT_IN_MACROBLOCKS(m_frameHeight / SCALE_FACTOR_16x);
    m_downscaledWidth16x =
        m_downscaledWidthInMb16x * CODECHAL_MACROBLOCK_WIDTH;
    m_downscaledHeight16x =
        m_downscaledHeightInMb16x * CODECHAL_MACROBLOCK_HEIGHT;

    // UltraHME Scaling WxH
    m_downscaledWidthInMb32x =
        CODECHAL_GET_WIDTH_IN_MACROBLOCKS(m_frameWidth / SCALE_FACTOR_32x);
    m_downscaledHeightInMb32x =
        CODECHAL_GET_HEIGHT_IN_MACROBLOCKS(m_frameHeight / SCALE_FACTOR_32x);
    m_downscaledWidth32x =
        m_downscaledWidthInMb32x * CODECHAL_MACROBLOCK_WIDTH;
    m_downscaledHeight32x =
        m_downscaledHeightInMb32x * CODECHAL_MACROBLOCK_HEIGHT;

    MotionEstimationDisableCheck();

    m_kernelMode =
        CodecHal_TargetUsageToMode_AVC[seqParams->TargetUsage & 0x7];
    m_targetUsage = seqParams->TargetUsage & 0x7;

    if (!seqParams->frame_cropping_flag)
    {
        // do cropping only when the picture dimension is not MB aligned...
        seqParams->frame_crop_left_offset = 0;
        seqParams->frame_crop_top_offset  = 0;

        if (m_frameWidth != seqParams->FrameWidth ||
            m_frameHeight != seqParams->FrameHeight)
        {
            seqParams->frame_cropping_flag      = 1;
            seqParams->frame_crop_right_offset  = (uint16_t)((m_frameWidth - seqParams->FrameWidth) >> 1);                                       // 4:2:0
            seqParams->frame_crop_bottom_offset = (uint16_t)((m_frameHeight - seqParams->FrameHeight) >> (2 - seqParams->frame_mbs_only_flag));  // 4:2:0
        }
        else
        {
            seqParams->frame_cropping_flag      = 0;
            seqParams->frame_crop_right_offset  = 0;
            seqParams->frame_crop_bottom_offset = 0;
        }
    }

    if (m_mfxInterface->IsRowStoreCachingSupported())
    {
        MHW_VDBOX_ROWSTORE_PARAMS rowstoreParams;
        MOS_ZeroMemory(&rowstoreParams, sizeof(rowstoreParams));
        rowstoreParams.Mode       = CODECHAL_ENCODE_MODE_AVC;
        rowstoreParams.dwPicWidth = m_frameWidth;
        rowstoreParams.bIsFrame   = (seqParams->frame_mbs_only_flag == 1);
        m_hwInterface->SetRowstoreCachingOffsets(&rowstoreParams);
    }

    return eStatus;
}

MOS_STATUS CodechalEncodeAvcBase::SetPictureStructs()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    auto picParams  = m_avcPicParam;
    auto seqParams  = m_avcSeqParam;
    auto avcRefList = &m_refList[0];
    auto avcPicIdx  = &m_picIdx[0];
    auto slcParams  = m_avcSliceParams;

    if (seqParams->Profile == CODEC_AVC_MAIN_PROFILE || seqParams->Profile == CODEC_AVC_BASE_PROFILE)
    {
        picParams->transform_8x8_mode_flag         = 0;
        picParams->pic_scaling_matrix_present_flag = 0;
        for (uint8_t i = 0; i < 12; i++)
        {
            picParams->pic_scaling_list_present_flag[i] = 0;
        }
        picParams->second_chroma_qp_index_offset = picParams->chroma_qp_index_offset;
    }
    if (picParams->QpY < 0)
    {
        picParams->QpY = 25;  // Set to default, recommended value used in simulation.
    }
    else if (picParams->QpY > CODECHAL_ENCODE_AVC_MAX_SLICE_QP)
    {
        picParams->QpY = CODECHAL_ENCODE_AVC_MAX_SLICE_QP;  // Crop to 51 if larger
    }
    picParams->pic_init_qp_minus26 = picParams->QpY - 26;

    if (!seqParams->seq_scaling_matrix_present_flag)
    {
        if (!picParams->pic_scaling_matrix_present_flag)
            ScalingListFlat();
        else if (!picParams->pic_scaling_list_present_flag[0])
            ScalingListFallbackRuleA();
    }
    else if (!seqParams->seq_scaling_list_present_flag[0] &&
             !picParams->pic_scaling_list_present_flag[0])
    {  // fall-back rule A
        ScalingListFallbackRuleA();
    }

    picParams->num_slice_groups_minus1                = 0;  // do not support flexible MB ordering
    picParams->deblocking_filter_control_present_flag = 1;  // always set to 1
    picParams->redundant_pic_cnt_present_flag         = 0;
    picParams->pic_init_qs_minus26                    = 0;  // not used

    m_userFlags       = picParams->UserFlags;
    m_nalUnitType     = picParams->bIdrPic ? CODECHAL_ENCODE_AVC_NAL_UT_IDR_SLICE : CODECHAL_ENCODE_AVC_NAL_UT_SLICE;
    m_frameNum        = picParams->frame_num;
    m_lastPicInSeq    = picParams->bLastPicInSeq;
    m_lastPicInStream = picParams->bLastPicInStream;
    // App handles tail insertion for VDEnc dynamic slice in non-cp case
    if (m_vdencNoTailInsertion)
    {
        m_lastPicInSeq = m_lastPicInStream = 0;
    }
    m_statusReportFeedbackNumber = picParams->StatusReportFeedbackNumber;

    auto prevPic    = m_currOriginalPic;
    auto prevIdx    = prevPic.FrameIdx;
    auto currPic    = picParams->CurrOriginalPic;
    auto currIdx    = currPic.FrameIdx;
    auto prevRefIdx = m_currReconstructedPic.FrameIdx;
    auto currRefIdx = picParams->CurrReconstructedPic.FrameIdx;

    m_prevReconFrameIdx = m_currReconFrameIdx;
    m_currReconFrameIdx = picParams->CurrReconstructedPic.FrameIdx;

    avcRefList[currRefIdx]->sRefReconBuffer = m_reconSurface;
    avcRefList[currRefIdx]->sRefRawBuffer   = m_rawSurface;
    avcRefList[currRefIdx]->sFrameNumber    = picParams->frame_num;
    avcRefList[currRefIdx]->RefPic          = currPic;
    m_currOriginalPic                       = currPic;
    m_currReconstructedPic                  = picParams->CurrReconstructedPic;

    if (picParams->FieldCodingFlag)
    {
        m_frameFieldHeight                  = ((m_frameHeight + 1) >> 1);
        m_frameFieldHeightInMb              = ((m_picHeightInMb + 1) >> 1);
        m_downscaledFrameFieldHeightInMb4x  = ((m_downscaledHeightInMb4x + 1) >> 1);
        m_downscaledFrameFieldHeightInMb16x = ((m_downscaledHeightInMb16x + 1) >> 1);
        m_downscaledFrameFieldHeightInMb32x = ((m_downscaledHeightInMb32x + 1) >> 1);
        m_currEncBbSet                      = MB_ENC_Field_BB;
        if (CodecHal_PictureIsFrame(prevPic) || prevIdx != currIdx ||
            ((prevPic.PicFlags != currPic.PicFlags) && !(slcParams->pic_order_cnt_lsb & 1)))
        {
            m_firstField = 1;
            // Enable self referencing for both IDR and I pics
            m_firstFieldIdrPic = (picParams->CodingType == I_TYPE) ? 1 : 0;
        }
        else
        {
            m_firstField = 0;
        }
    }
    else
    {
        m_frameFieldHeight                  = m_frameHeight;
        m_frameFieldHeightInMb              = m_picHeightInMb;
        m_downscaledFrameFieldHeightInMb4x  = m_downscaledHeightInMb4x;
        m_downscaledFrameFieldHeightInMb16x = m_downscaledHeightInMb16x;
        m_downscaledFrameFieldHeightInMb32x = m_downscaledHeightInMb32x;
        m_firstField                        = 1;
        m_currEncBbSet                      = MB_ENC_Frame_BB;
    }

    if (picParams->FieldFrameCodingFlag)
    {  // Mbaff
        seqParams->mb_adaptive_frame_field_flag = 1;
        m_mbaffEnabled                          = 1;
    }
    else
    {
        seqParams->mb_adaptive_frame_field_flag = 0;
        m_mbaffEnabled                          = 0;
    }

    // P/B frames with empty ref lists are internally encoded as I frames,
    // while picture header packing remains the original value
    m_pictureCodingType = picParams->CodingType;

    bool emptyRefFrmList = true;
    for (uint8_t i = 0; i < CODEC_AVC_MAX_NUM_REF_FRAME; i++)
    {
        if (picParams->RefFrameList[i].PicFlags != PICTURE_INVALID)
        {
            emptyRefFrmList = false;
            break;
        }
    }

    if (emptyRefFrmList && m_pictureCodingType != I_TYPE)
    {
        m_pictureCodingType = I_TYPE;
    }

    avcRefList[currRefIdx]->bUsedAsRef         = picParams->RefPicFlag;
    avcRefList[currRefIdx]->resBitstreamBuffer = m_resBitstreamBuffer;

    for (uint8_t i = 0; i < 16; i++)
    {
        m_avcFrameStoreID[i].inUse = false;
    }

    for (uint8_t i = 0; i < 16; i++)
    {
        avcPicIdx[i].bValid = false;
        if (picParams->RefFrameList[i].PicFlags != PICTURE_INVALID)
        {
            auto    index         = picParams->RefFrameList[i].FrameIdx;
            uint8_t duplicatedIdx = 0;
            for (uint8_t ii = 0; ii < i; ii++)
            {
                if (avcPicIdx[ii].bValid && index == picParams->RefFrameList[ii].FrameIdx)
                {
                    duplicatedIdx = 1;
                    break;
                }
            }
            if (duplicatedIdx)
            {
                continue;
            }

            avcRefList[index]->RefPic.PicFlags =
                CodecHal_CombinePictureFlags(avcRefList[index]->RefPic, picParams->RefFrameList[i]);
            avcRefList[index]->iFieldOrderCnt[0] = picParams->FieldOrderCntList[i][0];
            avcRefList[index]->iFieldOrderCnt[1] = picParams->FieldOrderCntList[i][1];
            avcPicIdx[i].bValid                  = true;
            avcPicIdx[i].ucPicIdx                = index;
            if (prevPic.PicFlags != PICTURE_INVALID)
            {
                uint8_t ii;
                for (ii = 0; ii < avcRefList[prevRefIdx]->ucNumRef; ii++)
                {
                    if (index == avcRefList[prevRefIdx]->RefList[ii].FrameIdx)
                    {
                        if (avcRefList[index]->ucFrameId == 0x1f)
                        {
                            // Should never happen, something must be wrong
                            CODECHAL_ENCODE_ASSERT(false);
                            avcRefList[index]->ucFrameId = 0;
                        }
                        m_avcFrameStoreID[avcRefList[index]->ucFrameId].inUse = true;
                        break;
                    }
                }
                if (ii == avcRefList[prevRefIdx]->ucNumRef)
                {
                    avcRefList[index]->ucFrameId = 0x1f;
                }
            }
        }
    }

    // Save the current RefList
    uint8_t ii = 0;
    for (uint8_t i = 0; i < 16; i++)
    {
        if (avcPicIdx[i].bValid)
        {
            avcRefList[currRefIdx]->RefList[ii] = picParams->RefFrameList[i];
            ii++;
        }
    }
    avcRefList[currRefIdx]->ucNumRef = ii;
    m_currRefList                    = avcRefList[currRefIdx];

    if (m_codecFunction == CODECHAL_FUNCTION_ENC)
    {
        CODECHAL_ENCODE_CHK_NULL_RETURN(m_encodeParams.presMbCodeSurface);
        m_resMbCodeSurface = *(m_encodeParams.presMbCodeSurface);
    }
    else if (m_codecFunction == CODECHAL_FUNCTION_ENC_PAK)
    {
        // the actual MbCode/MvData surface to be allocated later
        m_trackedBuf->SetAllocationFlag(true);
    }
    else if (CodecHalIsFeiEncode(m_codecFunction))
    {
        CodecEncodeAvcFeiPicParams *feiPicParams;

        feiPicParams = (CodecEncodeAvcFeiPicParams *)m_encodeParams.pFeiPicParams;
        CODECHAL_ENCODE_CHK_NULL_RETURN(feiPicParams);

        // Use app provided buffer if available. For FEI_ENC and FEI_PAK, MBCodeMvEnable needs to be set to true
        if (feiPicParams->MbCodeMvEnable)
        {
            m_resMbCodeSurface = feiPicParams->resMBCode;
            m_resMvDataSurface = feiPicParams->resMVData;

            // Inside the AvcRefList, mbCodesurface and mvdatasurface are stored at frame basis,
            // For FEI, mbcode and mv data buffer are provided separately for each field picture and we need to store them separately.
            if (CodecHal_PictureIsTopField(picParams->CurrOriginalPic))
            {
                avcRefList[currRefIdx]->resRefTopFieldMbCodeBuffer = m_resMbCodeSurface;
                avcRefList[currRefIdx]->resRefTopFieldMvDataBuffer = m_resMvDataSurface;
            }
            else if (CodecHal_PictureIsBottomField(picParams->CurrOriginalPic))
            {
                avcRefList[currRefIdx]->resRefBotFieldMbCodeBuffer = m_resMbCodeSurface;
                avcRefList[currRefIdx]->resRefBotFieldMvDataBuffer = m_resMvDataSurface;
            }

            avcRefList[currRefIdx]->resRefMbCodeBuffer = m_resMbCodeSurface;
            avcRefList[currRefIdx]->resRefMvDataBuffer = m_resMvDataSurface;
        }
        else
        {
            // the actual MbCode/MvData surface to be allocated later
            m_trackedBuf->SetAllocationFlag(true);
        }

        if (feiPicParams->DistortionEnable)
        {
            m_resDistortionBuffer = feiPicParams->resDistortion;
        }
    }

    SetFrameStoreIds(currRefIdx);

    avcRefList[currRefIdx]->iFieldOrderCnt[0]     = picParams->CurrFieldOrderCnt[0];
    avcRefList[currRefIdx]->iFieldOrderCnt[1]     = picParams->CurrFieldOrderCnt[1];
    m_refList[currRefIdx]->ucAvcPictureCodingType = (CodecHal_PictureIsFrame(picParams->CurrOriginalPic)) ? CODEC_AVC_PIC_CODING_TYPE_FRAME : ((picParams->CurrFieldOrderCnt[0] < picParams->CurrFieldOrderCnt[1]) ? CODEC_AVC_PIC_CODING_TYPE_TFF_FIELD : CODEC_AVC_PIC_CODING_TYPE_BFF_FIELD);

    m_hmeEnabled   = m_hmeSupported && m_pictureCodingType != I_TYPE;
    m_16xMeEnabled = m_16xMeSupported && m_pictureCodingType != I_TYPE;
    m_32xMeEnabled = m_32xMeSupported && m_pictureCodingType != I_TYPE;

    if (m_pictureCodingType == B_TYPE)
    {
        GetDistScaleFactor();
        m_biWeight = GetBiWeight(
            m_distScaleFactorList0[0],
            m_avcPicParam->weighted_bipred_idc);
    }

    m_verticalLineStride            = CODECHAL_VLINESTRIDE_FRAME;
    m_verticalLineStrideOffset      = CODECHAL_VLINESTRIDEOFFSET_TOP_FIELD;
    m_mbcodeBottomFieldOffset       = 0;
    m_mvBottomFieldOffset           = 0;
    m_meDistortionBottomFieldOffset = 0;
    m_meMvBottomFieldOffset         = 0;
    m_meMv16xBottomFieldOffset      = 0;
    m_meMv32xBottomFieldOffset      = 0;
    m_sliceMapBottomFieldOffset     = 0;
    if (m_hmeKernel)
    {
        m_hmeKernel->Set4xMeMvBottomFieldOffset(0);
        m_hmeKernel->Set16xMeMvBottomFieldOffset(0);
        m_hmeKernel->Set32xMeMvBottomFieldOffset(0);
        m_hmeKernel->SetDistortionBottomFieldOffset(0);
    }

    if (CodecHal_PictureIsField(m_currOriginalPic))
    {
        m_verticalLineStride = CODECHAL_VLINESTRIDE_FIELD;
        m_frameHeight        = m_frameFieldHeightInMb * 2 * 16;
        m_picHeightInMb      = (uint16_t)(m_frameHeight / 16);
        if (CodecHal_PictureIsBottomField(m_currOriginalPic))
        {
            m_sliceMapBottomFieldOffset = MOS_ALIGN_CEIL((m_picWidthInMb + 1) * sizeof(uint32_t), 64) * m_frameFieldHeightInMb;
            m_verticalLineStrideOffset  = CODECHAL_VLINESTRIDEOFFSET_BOT_FIELD;

            if (CodecHalIsFeiEncode(m_codecFunction))
            {
                CodecEncodeAvcFeiPicParams *feiPicParams;

                feiPicParams = (CodecEncodeAvcFeiPicParams *)m_encodeParams.pFeiPicParams;
                CODECHAL_ENCODE_CHK_NULL_RETURN(feiPicParams);

                // for user provided MbCode and Mv data buffer, set BottomFieldOffset to 0
                if (feiPicParams->MbCodeMvEnable)
                {
                    m_mbcodeBottomFieldOffset = 0;
                    m_mvBottomFieldOffset     = 0;
                }
                else
                {
                    m_mbcodeBottomFieldOffset = m_frameFieldHeightInMb * m_picWidthInMb * 64;
                    m_mvBottomFieldOffset =
                        MOS_ALIGN_CEIL(m_frameFieldHeightInMb * m_picWidthInMb * (32 * 4), CODECHAL_PAGE_SIZE);
                }
            }
            else
            {
                m_mbcodeBottomFieldOffset = m_frameFieldHeightInMb * m_picWidthInMb * 64;
                m_mvBottomFieldOffset =
                    MOS_ALIGN_CEIL(m_frameFieldHeightInMb * m_picWidthInMb * (32 * 4), CODECHAL_PAGE_SIZE);
            }

            m_meDistortionBottomFieldOffset =
                MOS_ALIGN_CEIL((m_downscaledWidthInMb4x * 8), 64) *
                MOS_ALIGN_CEIL((m_downscaledFrameFieldHeightInMb4x * 4 * 10), 8);
            m_meMvBottomFieldOffset =
                MOS_ALIGN_CEIL((m_downscaledWidthInMb4x * 32), 64) *
                (m_downscaledFrameFieldHeightInMb4x * 4);
            if (m_16xMeEnabled)
            {
                m_meMv16xBottomFieldOffset =
                    MOS_ALIGN_CEIL((m_downscaledWidthInMb16x * 32), 64) *
                    (m_downscaledFrameFieldHeightInMb16x * 4);
                if (m_32xMeEnabled)
                {
                    m_meMv32xBottomFieldOffset =
                        MOS_ALIGN_CEIL((m_downscaledWidthInMb32x * 32), 64) *
                        (m_downscaledFrameFieldHeightInMb32x * 4);
                }
            }

            if (m_hmeKernel)
            {
                m_hmeKernel->SetDistortionBottomFieldOffset(
                    MOS_ALIGN_CEIL((m_downscaledWidthInMb4x * 8), 64) *
                    MOS_ALIGN_CEIL((m_downscaledFrameFieldHeightInMb4x * 4 * 10), 8));
                m_hmeKernel->Set4xMeMvBottomFieldOffset(
                    MOS_ALIGN_CEIL((m_downscaledWidthInMb4x * 32), 64) *
                    (m_downscaledFrameFieldHeightInMb4x * 4));
                if (m_hmeKernel->Is16xMeEnabled())
                {
                    m_hmeKernel->Set16xMeMvBottomFieldOffset(
                        MOS_ALIGN_CEIL((m_downscaledWidthInMb16x * 32), 64) *
                        (m_downscaledFrameFieldHeightInMb16x * 4));
                    if (m_hmeKernel->Is32xMeEnabled())
                    {
                        m_hmeKernel->Set32xMeMvBottomFieldOffset(
                            MOS_ALIGN_CEIL((m_downscaledWidthInMb32x * 32), 64) *
                            (m_downscaledFrameFieldHeightInMb32x * 4));
                    }
                }
            }
        }
    }

    return eStatus;
}

MOS_STATUS CodechalEncodeAvcBase::EncodeMeKernel(
    EncodeBrcBuffers *brcBuffers,
    HmeLevel          hmeLevel)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    PerfTagSetting perfTag;
    perfTag.Value             = 0;
    perfTag.Mode              = (uint16_t)m_mode & CODECHAL_ENCODE_MODE_BIT_MASK;
    perfTag.CallType          = m_singleTaskPhaseSupported ? CODECHAL_ENCODE_PERFTAG_CALL_SCALING_KERNEL : CODECHAL_ENCODE_PERFTAG_CALL_ME_KERNEL;
    perfTag.PictureCodingType = m_pictureCodingType;
    m_osInterface->pfnSetPerfTag(m_osInterface, perfTag.Value);
    // Each ME kernel buffer counts as a separate perf task
    m_osInterface->pfnResetPerfBufferID(m_osInterface);

    CODECHAL_MEDIA_STATE_TYPE encFunctionType = (hmeLevel == HME_LEVEL_32x) ? CODECHAL_MEDIA_STATE_32X_ME : (hmeLevel == HME_LEVEL_16x) ? CODECHAL_MEDIA_STATE_16X_ME : CODECHAL_MEDIA_STATE_4X_ME;

    bool vdencMeInUse = false;
    if (m_vdencEnabled && (encFunctionType == CODECHAL_MEDIA_STATE_4X_ME))
    {
        vdencMeInUse    = true;
        encFunctionType = CODECHAL_MEDIA_STATE_ME_VDENC_STREAMIN;
    }

    uint32_t krnStateIdx = vdencMeInUse ? CODECHAL_ENCODE_ME_IDX_VDENC : ((m_pictureCodingType == P_TYPE) ? CODECHAL_ENCODE_ME_IDX_P : CODECHAL_ENCODE_ME_IDX_B);
    auto     kernelState = &m_meKernelStates[krnStateIdx];

    // If Single Task Phase is not enabled, use BT count for the kernel state.
    if (m_firstTaskInPhase || !m_singleTaskPhaseSupported)
    {
        uint32_t maxBtCount = m_singleTaskPhaseSupported ? m_maxBtCount : kernelState->KernelParams.iBTCount;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnRequestSshSpaceForCmdBuf(
            m_stateHeapInterface,
            maxBtCount));
        m_vmeStatesSize = m_hwInterface->GetKernelLoadCommandSize(maxBtCount);
        CODECHAL_ENCODE_CHK_STATUS_RETURN(VerifySpaceAvailable());
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->AssignDshAndSshSpace(
        m_stateHeapInterface,
        kernelState,
        false,
        0,
        false,
        m_storeData));

    MHW_INTERFACE_DESCRIPTOR_PARAMS idParams;
    MOS_ZeroMemory(&idParams, sizeof(idParams));
    idParams.pKernelState = kernelState;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnSetInterfaceDescriptor(
        m_stateHeapInterface,
        1,
        &idParams));

    // Setup AVC Curbe
    MeCurbeParams meParams;
    MOS_ZeroMemory(&meParams, sizeof(meParams));
    meParams.hmeLvl       = hmeLevel;
    meParams.pKernelState = kernelState;

    if (!m_useCommonKernel)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(SetCurbeMe(&meParams));
    }

    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpKernelRegion(
            encFunctionType,
            MHW_DSH_TYPE,
            kernelState));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpCurbe(
            encFunctionType,
            kernelState));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpKernelRegion(
            encFunctionType,
            MHW_ISH_TYPE,
            kernelState));)

    MOS_COMMAND_BUFFER cmdBuffer;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnGetCommandBuffer(m_osInterface, &cmdBuffer, 0));

    SendKernelCmdsParams sendKernelCmdsParams = SendKernelCmdsParams();
    sendKernelCmdsParams.EncFunctionType      = encFunctionType;
    sendKernelCmdsParams.pKernelState         = kernelState;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(SendGenericKernelCmds(&cmdBuffer, &sendKernelCmdsParams));

    // Add binding table
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnSetBindingTable(
        m_stateHeapInterface,
        kernelState));

    //Add surface states
    MeSurfaceParams meSurfaceParams;
    MOS_ZeroMemory(&meSurfaceParams, sizeof(meSurfaceParams));
    meSurfaceParams.dwNumRefIdxL0ActiveMinus1       = m_avcSliceParams->num_ref_idx_l0_active_minus1;
    meSurfaceParams.dwNumRefIdxL1ActiveMinus1       = m_avcSliceParams->num_ref_idx_l1_active_minus1;
    meSurfaceParams.pL0RefFrameList                 = &(m_avcSliceParams->RefPicList[LIST_0][0]);
    meSurfaceParams.pL1RefFrameList                 = &(m_avcSliceParams->RefPicList[LIST_1][0]);
    meSurfaceParams.ppRefList                       = &m_refList[0];
    meSurfaceParams.pPicIdx                         = &m_picIdx[0];
    meSurfaceParams.pCurrOriginalPic                = &m_currOriginalPic;
    meSurfaceParams.ps4xMeMvDataBuffer              = &m_4xMeMvDataBuffer;
    meSurfaceParams.dw4xMeMvBottomFieldOffset       = (uint32_t)m_meMvBottomFieldOffset;
    meSurfaceParams.ps16xMeMvDataBuffer             = &m_16xMeMvDataBuffer;
    meSurfaceParams.dw16xMeMvBottomFieldOffset      = (uint32_t)m_meMv16xBottomFieldOffset;
    meSurfaceParams.ps32xMeMvDataBuffer             = &m_32xMeMvDataBuffer;
    meSurfaceParams.dw32xMeMvBottomFieldOffset      = (uint32_t)m_meMv32xBottomFieldOffset;
    meSurfaceParams.dw4xScaledBottomFieldOffset     = (uint32_t)m_scaledBottomFieldOffset;
    meSurfaceParams.dw16xScaledBottomFieldOffset    = (uint32_t)m_scaled16xBottomFieldOffset;
    meSurfaceParams.dw32xScaledBottomFieldOffset    = (uint32_t)m_scaled32xBottomFieldOffset;
    meSurfaceParams.psMeDistortionBuffer            = &m_4xMeDistortionBuffer;
    meSurfaceParams.dwMeDistortionBottomFieldOffset = (uint32_t)m_meDistortionBottomFieldOffset;
    if (nullptr != brcBuffers)
    {
        meSurfaceParams.psMeBrcDistortionBuffer            = &brcBuffers->sMeBrcDistortionBuffer;
        meSurfaceParams.dwMeBrcDistortionBottomFieldOffset = brcBuffers->dwMeBrcDistortionBottomFieldOffset;
    }
    meSurfaceParams.psMeVdencStreamInBuffer    = &m_resVdencStreamInBuffer[m_currRecycledBufIdx];
    meSurfaceParams.dwDownscaledWidthInMb      = (hmeLevel == HME_LEVEL_32x) ? m_downscaledWidthInMb32x : (hmeLevel == HME_LEVEL_16x) ? m_downscaledWidthInMb16x : m_downscaledWidthInMb4x;
    meSurfaceParams.dwDownscaledHeightInMb     = (hmeLevel == HME_LEVEL_32x) ? m_downscaledFrameFieldHeightInMb32x : (hmeLevel == HME_LEVEL_16x) ? m_downscaledFrameFieldHeightInMb16x : m_downscaledFrameFieldHeightInMb4x;
    meSurfaceParams.dwVerticalLineStride       = m_verticalLineStride;
    meSurfaceParams.dwVerticalLineStrideOffset = m_verticalLineStrideOffset;
    meSurfaceParams.b32xMeInUse                = (hmeLevel == HME_LEVEL_32x) ? true : false;
    meSurfaceParams.b16xMeInUse                = (hmeLevel == HME_LEVEL_16x) ? true : false;
    meSurfaceParams.b32xMeEnabled              = m_32xMeEnabled;
    meSurfaceParams.b16xMeEnabled              = m_16xMeEnabled;
    meSurfaceParams.bVdencStreamInEnabled      = m_vdencEnabled && (m_16xMeSupported || m_staticFrameDetectionInUse);
    meSurfaceParams.pMeBindingTable            = &m_meBindingTable;
    meSurfaceParams.pKernelState               = kernelState;
    meSurfaceParams.dwVDEncStreamInSurfaceSize = MOS_BYTES_TO_DWORDS(m_picHeightInMb * m_picWidthInMb * 64);

    if (!m_useCommonKernel)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(SendMeSurfaces(&cmdBuffer, &meSurfaceParams));
    }

    // Dump SSH for ME kernel
    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpKernelRegion(
            encFunctionType,
            MHW_SSH_TYPE,
            kernelState)));

    /* zero out the mv data memory and me distortion buffer for the driver ULT
    kernel only writes out this data used for current frame, in some cases the the data used for
    previous frames would be left in the buffer (for example, the L1 mv for B frame would still show
    in the P frame mv data buffer */

    /* Zeroing out the buffers has perf impact, so zero it out only when dumps are actually enabled */

    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_NULL_RETURN(m_debugInterface);
        uint8_t * data;
        uint32_t size;
        bool     driverMeDumpEnabled;

        driverMeDumpEnabled = m_debugInterface->DumpIsEnabled(CodechalDbgKernel::kernel4xMe) ||
                              m_debugInterface->DumpIsEnabled(CodechalDbgKernel::kernel16xMe) ||
                              m_debugInterface->DumpIsEnabled(CodechalDbgKernel::kernel32xMe);

        if (driverMeDumpEnabled) {
            MOS_LOCK_PARAMS lockFlags;
            MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
            lockFlags.WriteOnly = 1;

            switch (hmeLevel)
            {
            case HME_LEVEL_32x:
                data = (uint8_t *)m_osInterface->pfnLockResource(
                    m_osInterface,
                    &m_32xMeMvDataBuffer.OsResource,
                    &lockFlags);
                CODECHAL_ENCODE_CHK_NULL_RETURN(data);
                size = MOS_ALIGN_CEIL((m_downscaledWidthInMb32x * 32), 64) *
                       (m_downscaledHeightInMb32x * 2 * 4 * CODECHAL_ENCODE_ME_DATA_SIZE_MULTIPLIER);
                MOS_ZeroMemory(data, size);
                m_osInterface->pfnUnlockResource(
                    m_osInterface,
                    &m_32xMeMvDataBuffer.OsResource);
                break;
            case HME_LEVEL_16x:
                data = (uint8_t *)m_osInterface->pfnLockResource(
                    m_osInterface,
                    &m_16xMeMvDataBuffer.OsResource,
                    &lockFlags);
                CODECHAL_ENCODE_CHK_NULL_RETURN(data);
                size = MOS_ALIGN_CEIL((m_downscaledWidthInMb16x * 32), 64) *
                       (m_downscaledHeightInMb16x * 2 * 4 * CODECHAL_ENCODE_ME_DATA_SIZE_MULTIPLIER);
                MOS_ZeroMemory(data, size);
                m_osInterface->pfnUnlockResource(
                    m_osInterface,
                    &m_16xMeMvDataBuffer.OsResource);
                break;
            case HME_LEVEL_4x:
                if (!m_vdencEnabled)
                {
                    data = (uint8_t *)m_osInterface->pfnLockResource(
                        m_osInterface,
                        &m_4xMeMvDataBuffer.OsResource,
                        &lockFlags);
                    CODECHAL_ENCODE_CHK_NULL_RETURN(data);
                    size = MOS_ALIGN_CEIL((m_downscaledWidthInMb4x * 32), 64) *
                           (m_downscaledHeightInMb4x * 2 * 4 * CODECHAL_ENCODE_ME_DATA_SIZE_MULTIPLIER);
                    MOS_ZeroMemory(data, size);
                    m_osInterface->pfnUnlockResource(
                        m_osInterface,
                        &m_4xMeMvDataBuffer.OsResource);
                }
                break;
            default:
                return MOS_STATUS_INVALID_PARAMETER;
            }

            // zeroing out ME dist buffer
            if (m_4xMeDistortionBufferSupported)
            {
                data = (uint8_t *)m_osInterface->pfnLockResource(
                    m_osInterface, &m_4xMeDistortionBuffer.OsResource, &lockFlags);
                CODECHAL_ENCODE_CHK_NULL_RETURN(data);
                size = m_4xMeDistortionBuffer.dwHeight * m_4xMeDistortionBuffer.dwPitch;
                MOS_ZeroMemory(data, size);
                m_osInterface->pfnUnlockResource(
                    m_osInterface,
                    &m_4xMeDistortionBuffer.OsResource);
            }
        });

    uint32_t scalingFactor = (hmeLevel == HME_LEVEL_32x) ? SCALE_FACTOR_32x : (hmeLevel == HME_LEVEL_16x) ? SCALE_FACTOR_16x : SCALE_FACTOR_4x;

    uint32_t resolutionX = CODECHAL_GET_WIDTH_IN_MACROBLOCKS(m_frameWidth / scalingFactor);
    uint32_t resolutionY = CODECHAL_GET_HEIGHT_IN_MACROBLOCKS(m_frameFieldHeight / scalingFactor);

    CODECHAL_WALKER_CODEC_PARAMS walkerCodecParams;
    MOS_ZeroMemory(&walkerCodecParams, sizeof(walkerCodecParams));
    walkerCodecParams.WalkerMode              = m_walkerMode;
    walkerCodecParams.dwResolutionX           = resolutionX;
    walkerCodecParams.dwResolutionY           = resolutionY;
    walkerCodecParams.bNoDependency           = true;
    walkerCodecParams.bMbaff                  = m_mbaffEnabled;
    walkerCodecParams.bGroupIdSelectSupported = m_groupIdSelectSupported;
    walkerCodecParams.ucGroupId               = m_groupId;

    MHW_WALKER_PARAMS walkerParams;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalInitMediaObjectWalkerParams(
        m_hwInterface,
        &walkerParams,
        &walkerCodecParams));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_renderEngineInterface->AddMediaObjectWalkerCmd(
        &cmdBuffer,
        &walkerParams));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(EndStatusReport(&cmdBuffer, encFunctionType));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnSubmitBlocks(
        m_stateHeapInterface,
        kernelState));
    if (!m_singleTaskPhaseSupported || m_lastTaskInPhase)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnUpdateGlobalCmdBufId(
            m_stateHeapInterface));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferEnd(&cmdBuffer, nullptr));
    }

    CODECHAL_DEBUG_TOOL(CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpCmdBuffer(
        &cmdBuffer,
        encFunctionType,
        nullptr)));

    m_hwInterface->UpdateSSEuForCmdBuffer(&cmdBuffer, m_singleTaskPhaseSupported, m_lastTaskInPhase);

    m_osInterface->pfnReturnCommandBuffer(m_osInterface, &cmdBuffer, 0);

    if (!m_singleTaskPhaseSupported || m_lastTaskInPhase)
    {
        HalOcaInterface::On1stLevelBBEnd(cmdBuffer, *m_osInterface->pOsContext);
        m_osInterface->pfnSubmitCommandBuffer(m_osInterface, &cmdBuffer, m_renderContextUsesNullHw);
        m_lastTaskInPhase = false;
    }

    return eStatus;
}

MOS_STATUS CodechalEncodeAvcBase::SetSliceStructs()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;
    CODECHAL_ENCODE_FUNCTION_ENTER;

    auto slcParams = m_avcSliceParams;
    auto seqParams = m_avcSeqParam;
    auto picParams = m_avcPicParam;

    // Save the QP value
    if (CodecHal_PictureIsBottomField(picParams->CurrOriginalPic))
    {
        m_refList[m_currReconstructedPic.FrameIdx]->ucQPValue[1] =
            picParams->pic_init_qp_minus26 + 26 + slcParams->slice_qp_delta;
    }
    else
    {
        m_refList[m_currReconstructedPic.FrameIdx]->ucQPValue[0] =
            picParams->pic_init_qp_minus26 + 26 + slcParams->slice_qp_delta;
    }

    uint32_t numMbsInPrevSlice = slcParams->NumMbsForSlice;  // Initiailize to num mbs in first slice
    uint32_t numMbsForFirstSlice;

    for (uint32_t sliceCount = 0; sliceCount < m_numSlices; sliceCount++)
    {
        if (m_sliceStructCaps != CODECHAL_SLICE_STRUCT_ARBITRARYMBSLICE)
        {
            if (sliceCount == 0)
            {
                numMbsForFirstSlice = slcParams->NumMbsForSlice;
                // In current kernel, the same MB number in the slice must be number of MB rows.
                if (numMbsForFirstSlice % m_picWidthInMb)
                {
                    eStatus = MOS_STATUS_INVALID_PARAMETER;
                    return eStatus;
                }
                m_sliceHeight = numMbsForFirstSlice / m_picWidthInMb;
                // Slice height should be in power of 2
                if (m_sliceStructCaps == CODECHAL_SLICE_STRUCT_POW2ROWS && (m_sliceHeight & (m_sliceHeight - 1)))
                {
                    // app can only pass orig numMBs in picture for single slice, set slice height to the nearest pow2
                    if (m_numSlices == 1)
                    {
                        uint16_t sliceHeightPow2 = 1;
                        while (sliceHeightPow2 < m_sliceHeight)
                        {
                            sliceHeightPow2 <<= 1;
                        }
                        m_sliceHeight = sliceHeightPow2;
                    }
                    else
                    {
                        eStatus = MOS_STATUS_INVALID_PARAMETER;
                        return eStatus;
                    }
                }
            }
            // In current kernel, all slices should have the same MBs except the last one, the last one should have no more MBs than the previous
            else
            {
                if ((sliceCount < m_numSlices - 1 && numMbsForFirstSlice != slcParams->NumMbsForSlice) ||
                    (sliceCount == m_numSlices - 1 && numMbsForFirstSlice < slcParams->NumMbsForSlice))
                {
                    eStatus = MOS_STATUS_INVALID_PARAMETER;
                    return eStatus;
                }
            }

            if (slcParams->first_mb_in_slice != numMbsForFirstSlice * sliceCount)
            {
                eStatus = MOS_STATUS_INVALID_PARAMETER;
                return eStatus;
            }
        }
        else  // SLICE_STRUCT_ARBITRARYMBSLICE
        {
            uint8_t ppsIdx          = m_avcSliceParams->pic_parameter_set_id;
            uint8_t refPicListIdx   = m_avcSliceParams[ppsIdx].RefPicList[0][0].FrameIdx;
            uint8_t refFrameListIdx = m_avcPicParam[ppsIdx].RefFrameList[refPicListIdx].FrameIdx;

            bool dirtyRoiEnabled = (m_pictureCodingType == P_TYPE && m_avcPicParams[ppsIdx]->NumDirtyROI > 0 && m_prevReconFrameIdx == refFrameListIdx);

            if (m_mfeEnabled && m_numSlices > 1)
            {
                m_arbitraryNumMbsInSlice = 1;
                if (sliceCount == 0)
                {
                    m_sliceHeight = slcParams->NumMbsForSlice / m_picWidthInMb;
                }
            }
            else if ((slcParams->NumMbsForSlice % m_picWidthInMb) ||                                          // If slice is partial MB row,
                     ((sliceCount < m_numSlices - 1) && (numMbsInPrevSlice != slcParams->NumMbsForSlice)) ||  // OR not the last slice and num mbs is not same as prev slice
                     ((sliceCount == m_numSlices - 1) && ((numMbsInPrevSlice < slcParams->NumMbsForSlice) ||  // OR it is the last slice and num mbs is not less than prev slice
                                                             (MEDIA_IS_WA(m_waTable, WaArbitraryNumMbsInSlice) && (m_numSlices > 16) &&
                                                                 (!m_vdencEnabled) && (!dirtyRoiEnabled)))))
            {
                m_arbitraryNumMbsInSlice = 1;  // then set flag to use sliceMapSurface
                m_sliceHeight            = 1;  // Slice height doesn't matter if using slicemap just set to any non-zero value.
            }
            else if ((m_numSlices == 1) || (sliceCount == 0))
            {
                m_sliceHeight            = slcParams->NumMbsForSlice / m_picWidthInMb;
                m_arbitraryNumMbsInSlice = 0;
            }
            numMbsInPrevSlice = slcParams->NumMbsForSlice;
        }

        if ((picParams->pic_init_qp_minus26 + 26 + slcParams->slice_qp_delta) > CODECHAL_ENCODE_AVC_MAX_SLICE_QP)
        {
            slcParams->slice_qp_delta = CODECHAL_ENCODE_AVC_MAX_SLICE_QP - (picParams->pic_init_qp_minus26 + 26);
        }
        else
        {
            slcParams->slice_qp_delta = slcParams->slice_qp_delta;
        }
        slcParams->redundant_pic_cnt                  = 0;
        slcParams->sp_for_switch_flag                 = 0;
        slcParams->slice_qs_delta                     = 0;
        slcParams->ref_pic_list_reordering_flag_l0    = 0;
        slcParams->ref_pic_list_reordering_flag_l1    = 0;
        slcParams->adaptive_ref_pic_marking_mode_flag = 0;
        slcParams->no_output_of_prior_pics_flag       = 0;
        slcParams->redundant_pic_cnt                  = 0;

        slcParams->MaxFrameNum =
            1 << (seqParams[picParams->seq_parameter_set_id].log2_max_frame_num_minus4 + 4);
        slcParams->frame_num      = m_frameNum;
        slcParams->field_pic_flag = picParams->FieldCodingFlag;
        slcParams->bottom_field_flag =
            (CodecHal_PictureIsBottomField(picParams->CurrOriginalPic)) ? 1 : 0;

        if (m_pictureCodingType != I_TYPE)
        {
            for (uint8_t i = 0; i < (slcParams->num_ref_idx_l0_active_minus1 + 1); i++)
            {
                slcParams->PicOrder[0][i].Picture.FrameIdx =
                    m_picIdx[slcParams->RefPicList[0][i].FrameIdx].ucPicIdx;
                slcParams->PicOrder[0][i].Picture.PicFlags =
                    slcParams->RefPicList[0][i].PicFlags;
            }
        }
        if (m_pictureCodingType == B_TYPE)
        {
            for (uint8_t i = 0; i < (slcParams->num_ref_idx_l1_active_minus1 + 1); i++)
            {
                slcParams->PicOrder[1][i].Picture.FrameIdx =
                    m_picIdx[slcParams->RefPicList[1][i].FrameIdx].ucPicIdx;
                slcParams->PicOrder[1][i].Picture.PicFlags =
                    slcParams->RefPicList[1][i].PicFlags;
            }
        }
        slcParams++;
    }

    if (eStatus == MOS_STATUS_INVALID_PARAMETER)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Invalid slice parameters.");
    }
    return eStatus;
}

void CodechalEncodeAvcBase::SetMfxPipeModeSelectParams(
    const CODECHAL_ENCODE_AVC_GENERIC_PICTURE_LEVEL_PARAMS &genericParam,
    MHW_VDBOX_PIPE_MODE_SELECT_PARAMS &                     param)
{
    // set MFX_PIPE_MODE_SELECT values
    param                           = {};
    param.Mode                      = m_mode;
    param.bStreamOutEnabled         = (m_currPass != m_numPasses);  // Disable Stream Out for final pass; its important for multiple passes, because , next pass will take the qp from stream out
    param.bVdencEnabled             = m_vdencEnabled;
    param.bDeblockerStreamOutEnable = genericParam.bDeblockerStreamOutEnable;
    param.bPostDeblockOutEnable     = genericParam.bPostDeblockOutEnable;
    param.bPreDeblockOutEnable      = genericParam.bPreDeblockOutEnable;
    param.bDynamicSliceEnable       = m_avcSeqParam->EnableSliceLevelRateCtrl;
    param.bVdencStreamInEnable      = m_vdencStreamInEnabled;
    param.bTlbPrefetchEnable        = m_tlbPrefetchEnable;
    param.ChromaType                = m_avcSeqParam->chroma_format_idc;
    param.Format                    = m_rawSurfaceToPak->Format;
}

MOS_STATUS CodechalEncodeAvcBase::SetMfxPipeBufAddrStateParams(
    CODECHAL_ENCODE_AVC_GENERIC_PICTURE_LEVEL_PARAMS genericParam,
    MHW_VDBOX_PIPE_BUF_ADDR_PARAMS &                 param)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    param.Mode                 = m_mode;
    param.psPreDeblockSurface  = genericParam.psPreDeblockSurface;
    param.psPostDeblockSurface = genericParam.psPostDeblockSurface;

    param.psRawSurface                                 = m_rawSurfaceToPak;
    param.presStreamOutBuffer                          = &m_resStreamOutBuffer[m_currRecycledBufIdx];
    param.presMfdDeblockingFilterRowStoreScratchBuffer = &m_resDeblockingFilterRowStoreScratchBuffer;
    param.presMfdIntraRowStoreScratchBuffer            = &m_intraRowStoreScratchBuffer;
    param.bVdencEnabled                                = m_vdencEnabled;
    param.presMacroblockIldbStreamOutBuffer1           = genericParam.presMacroblockIldbStreamOutBuffer1;
    param.presMacroblockIldbStreamOutBuffer2           = genericParam.presMacroblockIldbStreamOutBuffer2;

    CODECHAL_DEBUG_TOOL(
        // PAK Input Raw Surface
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpYUVSurface(
            m_rawSurfaceToPak,
            CodechalDbgAttr::attrEncodeRawInputSurface,
            "PAK_Input_SrcSurf")););

    auto firstValidFrame = &m_reconSurface.OsResource;

    // Setting invalid entries to nullptr
    for (uint8_t i = 0; i < CODEC_AVC_MAX_NUM_REF_FRAME; i++)
        param.presReferences[i] = nullptr;

    uint8_t firstValidFrameId = CODEC_AVC_MAX_NUM_REF_FRAME;
    uint8_t numrefL0          = m_avcPicParam->num_ref_idx_l0_active_minus1 + 1;
    uint8_t numrefL1          = m_avcPicParam->num_ref_idx_l1_active_minus1 + 1;

    for (uint8_t i = 0; i < CODEC_AVC_MAX_NUM_REF_FRAME; i++)
    {
        if (m_picIdx[i].bValid)
        {
            auto picIdx       = m_picIdx[i].ucPicIdx;
            auto frameStoreId = m_refList[picIdx]->ucFrameId;

            CodecHalGetResourceInfo(m_osInterface, &(m_refList[picIdx]->sRefReconBuffer));
            param.presReferences[frameStoreId] = &(m_refList[picIdx]->sRefReconBuffer.OsResource);

            if (picIdx < firstValidFrameId)
            {
                firstValidFrameId = picIdx;
                firstValidFrame   = param.presReferences[picIdx];
            }

        }
    }
CODECHAL_DEBUG_TOOL(
        MOS_SURFACE refSurface;
    for (uint8_t i = 0; i < numrefL0; i++)
    {
        if (m_pictureCodingType != I_TYPE && m_avcSliceParams->RefPicList[0][i].PicFlags != PICTURE_INVALID)
        {
            auto refPic         = m_avcSliceParams->RefPicList[0][i];
            auto picIdx         = m_picIdx[refPic.FrameIdx].ucPicIdx;
            auto frameStoreId   = m_refList[picIdx]->ucFrameId;
                CODECHAL_ENCODE_CHK_NULL_RETURN(m_debugInterface);
                
                MOS_ZeroMemory(&refSurface, sizeof(refSurface));
                refSurface.Format     = Format_NV12;
                refSurface.OsResource = *(param.presReferences[frameStoreId]);
                CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalGetResourceInfo(
                    m_osInterface,
                    &refSurface));
                m_debugInterface->m_refIndex = frameStoreId;
                std::string refSurfName      = "RefSurfL0[" + std::to_string(static_cast<uint32_t>(i)) + "]";
                CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpYUVSurface(
                    &refSurface,
                    CodechalDbgAttr::attrReferenceSurfaces,
                    refSurfName.c_str()));
        }
    } 
    for (uint8_t i = 0; i < numrefL1; i++) 
    {
        if (m_pictureCodingType == B_TYPE && m_avcSliceParams->RefPicList[1][i].PicFlags != PICTURE_INVALID)
        {
            auto refPic       = m_avcSliceParams->RefPicList[1][i];
            auto picIdx       = m_picIdx[refPic.FrameIdx].ucPicIdx;
            auto frameStoreId = m_refList[picIdx]->ucFrameId;
                CODECHAL_ENCODE_CHK_NULL_RETURN(m_debugInterface);
                //MOS_SURFACE refSurface;
                MOS_ZeroMemory(&refSurface, sizeof(refSurface));
                refSurface.Format     = Format_NV12;
                refSurface.OsResource = *(param.presReferences[frameStoreId]);
                CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalGetResourceInfo(
                    m_osInterface,
                    &refSurface));
                m_debugInterface->m_refIndex = frameStoreId;
                std::string refSurfName      = "RefSurfL1[" + std::to_string(static_cast<uint32_t>(i)) + "]";
                CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpYUVSurface(
                    &refSurface,
                    CodechalDbgAttr::attrReferenceSurfaces,
                    refSurfName.c_str()));
        }
    }

);

    for (uint8_t i = 0; i < CODEC_AVC_MAX_NUM_REF_FRAME; i++)
    {
        // error concealment for the unset reference addresses
        if (!param.presReferences[i])
        {
            param.presReferences[i] = firstValidFrame;
        }
    }

    if (m_sliceSizeStreamoutSupported)
    {
        param.presSliceSizeStreamOutBuffer = &m_pakSliceSizeStreamoutBuffer;
    }
    return eStatus;
}

void CodechalEncodeAvcBase::SetMfxIndObjBaseAddrStateParams(MHW_VDBOX_IND_OBJ_BASE_ADDR_PARAMS &param)
{
    MOS_ZeroMemory(&param, sizeof(param));
    param.Mode                    = CODECHAL_ENCODE_MODE_AVC;
    param.presPakBaseObjectBuffer = &m_resBitstreamBuffer;
    param.dwPakBaseObjectSize     = m_bitstreamUpperBound;
}

void CodechalEncodeAvcBase::SetMfxBspBufBaseAddrStateParams(MHW_VDBOX_BSP_BUF_BASE_ADDR_PARAMS &param)
{
    MOS_ZeroMemory(&param, sizeof(param));
    param.presBsdMpcRowStoreScratchBuffer = &m_resMPCRowStoreScratchBuffer;
}

void CodechalEncodeAvcBase::SetMfxQmStateParams(MHW_VDBOX_QM_PARAMS &qmParams, MHW_VDBOX_QM_PARAMS &fqmParams)
{
    MOS_ZeroMemory(&qmParams, sizeof(qmParams));
    MOS_ZeroMemory(&fqmParams, sizeof(fqmParams));

    qmParams.Standard     = CODECHAL_AVC;
    qmParams.pAvcIqMatrix = (PMHW_VDBOX_AVC_QM_PARAMS)m_avcIQWeightScaleLists;

    fqmParams.Standard     = CODECHAL_AVC;
    fqmParams.pAvcIqMatrix = (PMHW_VDBOX_AVC_QM_PARAMS)m_avcIQWeightScaleLists;
}

void CodechalEncodeAvcBase::SetMfxAvcImgStateParams(MHW_VDBOX_AVC_IMG_PARAMS &param)
{
    param                            = {};
    param.ucCurrPass                 = m_currPass;
    param.pEncodeAvcPicParams        = m_avcPicParam;
    param.pEncodeAvcSeqParams        = m_avcSeqParam;
    param.pEncodeAvcSliceParams      = m_avcSliceParams;
    param.wPicWidthInMb              = m_picWidthInMb;
    param.wPicHeightInMb             = m_picHeightInMb;
    param.ppRefList                  = &(m_refList[0]);
    param.pPicIdx                    = &(m_picIdx[0]);
    param.dwTqEnabled                = m_trellisQuantParams.dwTqEnabled;
    param.dwTqRounding               = m_trellisQuantParams.dwTqRounding;
    param.ucKernelMode               = m_kernelMode;
    param.wSlcHeightInMb             = m_sliceHeight;
    param.dwMaxVmvR                  = CodecHalAvcEncode_GetMaxVmvR(m_avcSeqParam->Level);
    param.bVdencStreamInEnabled      = m_vdencStreamInEnabled;
    param.bSliceSizeStreamOutEnabled = m_sliceSizeStreamoutSupported;
    param.bCrePrefetchEnable         = m_crePrefetchEnable;

    if (m_currPass && (m_currPass == m_numPasses) && (!m_vdencBrcEnabled))
    {
        // Enable IPCM pass, excluding VDENC BRC case
        param.bIPCMPass = true;
    }
}

void CodechalEncodeAvcBase::UpdateSSDSliceCount()
{
    m_setRequestedEUSlices = ((m_frameHeight * m_frameWidth) >= m_ssdResolutionThreshold &&
                                 m_targetUsage <= m_ssdTargetUsageThreshold)
                                 ? true
                                 : false;

    m_hwInterface->m_numRequestedEuSlices = (m_setRequestedEUSlices) ? m_sliceShutdownRequestState : m_sliceShutdownDefaultState;
}

MOS_STATUS CodechalEncodeAvcBase::AddIshSize(uint32_t kuid, uint8_t *kernelBase)
{
    uint8_t *kernelBinary;
    uint32_t kernelSize;

    MOS_STATUS status = CodecHalGetKernelBinaryAndSize(kernelBase, kuid, &kernelBinary, &kernelSize);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(status);
    m_hwInterface->GetStateHeapSettings()->dwIshSize += MOS_ALIGN_CEIL(kernelSize, (1 << MHW_KERNEL_OFFSET_SHIFT));
    return status;
}

MOS_STATUS CodechalEncodeAvcBase::StoreNumPasses(
    EncodeStatusBuffer *encodeStatusBuf,
    MhwMiInterface *    miInterface,
    PMOS_COMMAND_BUFFER cmdBuffer,
    uint32_t            currPass)
{
    MHW_MI_STORE_DATA_PARAMS storeDataParams;
    uint32_t                 offset;
    MOS_STATUS               eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(encodeStatusBuf);
    CODECHAL_ENCODE_CHK_NULL_RETURN(miInterface);
    CODECHAL_ENCODE_CHK_NULL_RETURN(cmdBuffer);

    offset =
        (encodeStatusBuf->wCurrIndex * encodeStatusBuf->dwReportSize) +
        encodeStatusBuf->dwNumPassesOffset +  // Num passes offset
        sizeof(uint32_t) * 2;                 // pEncodeStatus is offset by 2 DWs in the resource

    storeDataParams.pOsResource      = &encodeStatusBuf->resStatusBuffer;
    storeDataParams.dwResourceOffset = offset;
    storeDataParams.dwValue          = currPass + 1;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(miInterface->AddMiStoreDataImmCmd(cmdBuffer, &storeDataParams));

    return MOS_STATUS_SUCCESS;
}

#if USE_CODECHAL_DEBUG_TOOL
static MOS_STATUS DumpEncodePicReorder(
    std::ostringstream &oss,
    uint32_t            x,
    uint32_t            y,
    CODEC_PIC_REORDER * picReorder)
{
    uint8_t botField;

    CODECHAL_DEBUG_CHK_NULL(picReorder);

    botField = CodecHal_PictureIsBottomField(picReorder->Picture) ? 1 : 0;

    oss << "# PicOrder[" << std::dec << +x << "][" << std::dec << +y << "] =" << std::endl;
    oss << "# \tPicNum = " << std::dec << +picReorder->PicNum << std::endl;
    oss << "# \tPOC = " << std::dec << +picReorder->POC << std::endl;
    oss << "# \tReorderPicNumIDC = " << std::dec << +picReorder->ReorderPicNumIDC << std::endl;
    oss << "# \tDiffPicNumMinus1 = " << std::dec << +picReorder->DiffPicNumMinus1 << std::endl;
    oss << "# \tFrameIdx = " << std::dec << +picReorder->Picture.FrameIdx << std::endl;
    oss << "# \tBotField = " << std::dec << +botField << std::endl;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalEncodeAvcBase::DumpSeqParams(
    PCODEC_AVC_ENCODE_SEQUENCE_PARAMS seqParams,
    PCODEC_AVC_IQ_MATRIX_PARAMS       matrixParams)
{
    CODECHAL_DEBUG_FUNCTION_ENTER;

    if (!m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrSeqParams))
    {
        return MOS_STATUS_SUCCESS;
    }

    CODECHAL_DEBUG_CHK_NULL(seqParams);

    std::ostringstream oss;
    oss.setf(std::ios::showbase | std::ios::uppercase);

    oss << "# DDI Parameters:" << std::endl;
    oss << "FrameWidth = " << +seqParams->FrameWidth << std::endl;
    oss << "FrameHeight = " << +seqParams->FrameHeight << std::endl;
    oss << "Profile = " << +seqParams->Profile << std::endl;
    oss << "Level = " << +seqParams->Level << std::endl;
    oss << "GopPicSize = " << +seqParams->GopPicSize << std::endl;
    oss << "GopRefDist = " << +seqParams->GopRefDist << std::endl;
    oss << "GopOptFlag = " << +seqParams->GopOptFlag << std::endl;
    oss << "TargetUsage = " << +seqParams->TargetUsage << std::endl;
    oss << "RateControlMethod = " << +seqParams->RateControlMethod << std::endl;
    oss << "TargetBitRate = " << +seqParams->TargetBitRate << std::endl;
    oss << "MaxBitRate = " << +seqParams->MaxBitRate << std::endl;
    oss << "MinBitRate = " << +seqParams->MinBitRate << std::endl;
    oss << "FramesPer100Sec = " << +seqParams->FramesPer100Sec << std::endl;
    oss << "InitVBVBufferFullnessInBit = " << +seqParams->InitVBVBufferFullnessInBit << std::endl;
    oss << "VBVBufferSizeInBit = " << +seqParams->VBVBufferSizeInBit << std::endl;
    oss << "NumRefFrames = " << +seqParams->NumRefFrames / 2 << std::endl;  // this prints the value passed from DDI
    oss << "# NumRefFrames (Actual Value in CodecHal is twice the value passed from DDI) = "
        << +seqParams->NumRefFrames << std::endl;  // this prints the actual value in CodecHal seq param structure
    oss << "seq_parameter_set_id = " << +seqParams->seq_parameter_set_id << std::endl;
    oss << "chroma_format_idc = " << +seqParams->chroma_format_idc << std::endl;
    oss << "bit_depth_luma_minus8 = " << +seqParams->bit_depth_luma_minus8 << std::endl;
    oss << "bit_depth_chroma_minus8 = " << +seqParams->bit_depth_chroma_minus8 << std::endl;
    oss << "log2_max_frame_num_minus4 = " << +seqParams->log2_max_frame_num_minus4 << std::endl;
    oss << "pic_order_cnt_type = " << +seqParams->pic_order_cnt_type << std::endl;
    oss << "log2_max_pic_order_cnt_lsb_minus4 = " << +seqParams->log2_max_pic_order_cnt_lsb_minus4 << std::endl;
    oss << "num_ref_frames_in_pic_order_cnt_cycle = " << +seqParams->num_ref_frames_in_pic_order_cnt_cycle << std::endl;
    oss << "offset_for_non_ref_pic = " << +seqParams->offset_for_non_ref_pic << std::endl;
    oss << "offset_for_top_to_bottom_field = " << +seqParams->offset_for_top_to_bottom_field << std::endl;

    // Conditionally printed (only when pic_order_cnt_type = 1).  Contains 256 elements.
    if (seqParams->pic_order_cnt_type == 1)
    {
        for (uint16_t i = 0; i < 256; ++i)
        {
            oss << "offset_for_ref_frame[" << +i << "] = " << +seqParams->offset_for_ref_frame[i] << std::endl;
        }
    }

    oss << "frame_crop_left_offset = " << +seqParams->frame_crop_left_offset << std::endl;
    oss << "frame_crop_right_offset = " << +seqParams->frame_crop_right_offset << std::endl;
    oss << "frame_crop_top_offset = " << +seqParams->frame_crop_top_offset << std::endl;
    oss << "frame_crop_bottom_offset = " << +seqParams->frame_crop_bottom_offset << std::endl;
    oss << "seq_scaling_matrix_present_flag = " << +seqParams->seq_scaling_matrix_present_flag << std::endl;
    oss << "seq_scaling_list_present_flag = " << +seqParams->seq_scaling_list_present_flag[0] << std::endl;

    // seq_scaling_list_present_flag with 12 elements (only 1 element acknowledged in DDI doc)
    oss << "# seq_scaling_list_present_flag[1-11]:";
    for (uint8_t i = 1; i < 12; i++)
        oss << +seqParams->seq_scaling_list_present_flag[i] << " ";
    oss << std::endl;

    oss << "delta_pic_order_always_zero_flag = " << +seqParams->delta_pic_order_always_zero_flag << std::endl;
    oss << "frame_mbs_only_flag = " << +seqParams->frame_mbs_only_flag << std::endl;
    oss << "direct_8x8_inference_flag = " << +seqParams->direct_8x8_inference_flag << std::endl;
    oss << "vui_parameters_present_flag = " << +seqParams->vui_parameters_present_flag << std::endl;
    oss << "frame_cropping_flag = " << +seqParams->frame_cropping_flag << std::endl;
    oss << "EnableSliceLevelRateCtrl = " << +seqParams->EnableSliceLevelRateCtrl << std::endl;
    oss << "ICQQualityFactor = " << +seqParams->ICQQualityFactor << std::endl;
    oss << "InputColorSpace = " << +seqParams->InputColorSpace << std::endl;

    // begining of union/struct
    oss << "# bResetBRC = " << +seqParams->bResetBRC << std::endl;
    oss << "# bNoAcceleratorSPSInsertion = " << +seqParams->bNoAcceleratorSPSInsertion << std::endl;
    oss << "# GlobalSearch = " << +seqParams->GlobalSearch << std::endl;
    oss << "# LocalSearch = " << +seqParams->LocalSearch << std::endl;
    oss << "# EarlySkip = " << +seqParams->EarlySkip << std::endl;
    oss << "# Trellis = " << +seqParams->Trellis << std::endl;
    oss << "# MBBRC = " << +seqParams->MBBRC << std::endl;
    oss << "# bTemporalScalability = " << +seqParams->bTemporalScalability << std::endl;
    oss << "# ROIValueInDeltaQP = " << +seqParams->ROIValueInDeltaQP << std::endl;
    oss << "# bAutoMaxPBFrameSizeForSceneChange = " << +seqParams->bAutoMaxPBFrameSizeForSceneChange << std::endl;
    oss << "sFlags = " << +seqParams->sFlags << std::endl;

    // end of union/struct
    oss << "UserMaxIFrameSize = " << +seqParams->UserMaxFrameSize << std::endl;
    oss << "UserMaxPBFrameSize = " << +seqParams->UserMaxPBFrameSize << std::endl;

    // Parameters not defined in DDI (Any non-DDI parameters printed should be preceeded by #)
    oss << "# Non-DDI Parameters:" << std::endl;
    oss << "# constraint_set0_flag = " << std::hex << +seqParams->constraint_set0_flag << std::endl;
    oss << "# constraint_set1_flag = " << std::hex << +seqParams->constraint_set1_flag << std::endl;
    oss << "# constraint_set2_flag = " << std::hex << +seqParams->constraint_set2_flag << std::endl;
    oss << "# constraint_set3_flag = " << std::hex << +seqParams->constraint_set3_flag << std::endl;

    oss << "# separate_colour_plane_flag = " << std::hex << +seqParams->separate_colour_plane_flag << std::endl;
    oss << "# qpprime_y_zero_transform_bypass_flag = " << std::hex << +seqParams->qpprime_y_zero_transform_bypass_flag << std::endl;
    oss << "# gaps_in_frame_num_value_allowed_flag = " << std::hex << +seqParams->gaps_in_frame_num_value_allowed_flag << std::endl;
    oss << "# pic_width_in_mbs_minus1 = " << std::hex << +seqParams->pic_width_in_mbs_minus1 << std::endl;
    oss << "# pic_height_in_map_units_minus1 = " << std::hex << +seqParams->pic_height_in_map_units_minus1 << std::endl;
    oss << "# mb_adaptive_frame_field_flag = " << std::hex << +seqParams->mb_adaptive_frame_field_flag << std::endl;

    // Dump ScalingList4x4 (6 x 16)
    for (uint8_t i = 0; i < 16; ++i)
    {
        oss << "# ScalingList4x4[" << std::dec << +i * 6 << "-" << (+i * 6) + 5 << "][" << +i << "]";
        for (uint8_t j = 0; j < 6; j++)
            oss << std::hex << +matrixParams->ScalingList4x4[j][i] << " ";
        oss << std::endl;
    }

    // ScalingList8x8 (2 x 64)
    for (uint8_t i = 0; i < 64; ++i)
    {
        oss << "# ScalingList8x8[0 / 1][ " << std::dec << +i << "] = ";
        oss << +matrixParams->ScalingList8x8[0][i] << " / " << +matrixParams->ScalingList8x8[1][i];
        oss << std::endl;
    }

    const char *fileName = m_debugInterface->CreateFileName(
        "_DDIEnc",
        CodechalDbgBufferType::bufSeqParams,
        CodechalDbgExtType::txt);

    std::ofstream ofs(fileName, std::ios::out);
    ofs << oss.str();
    ofs.close();

    if (m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrDriverUltDump))
    {
        if (!m_debugInterface->m_ddiFileName.empty())
        {
            std::ofstream ofs(m_debugInterface->m_ddiFileName, std::ios::app);
            ofs << "SeqParamFile"
                << " = \"" << m_debugInterface->m_fileName << "\"" << std::endl;
            ofs.close();
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalEncodeAvcBase::DumpPicParams(
    PCODEC_AVC_ENCODE_PIC_PARAMS picParams,
    PCODEC_AVC_IQ_MATRIX_PARAMS  matrixParams)
{
    CODECHAL_DEBUG_FUNCTION_ENTER;

    if (!m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrPicParams))
    {
        return MOS_STATUS_SUCCESS;
    }

    CODECHAL_DEBUG_CHK_NULL(picParams);

    std::ostringstream oss;
    oss.setf(std::ios::showbase | std::ios::uppercase);

    oss << "# DDI Parameters:" << std::endl;
    oss << "CurrOriginalPic = " << +picParams->CurrOriginalPic.PicEntry << std::endl;
    oss << "CurrReconstructedPic = " << +picParams->CurrReconstructedPic.PicEntry << std::endl;
    oss << "CodingType = " << +picParams->CodingType << std::endl;
    oss << "FieldCodingFlag = " << +picParams->FieldCodingFlag << std::endl;
    oss << "FieldFrameCodingFlag = " << +picParams->FieldFrameCodingFlag << std::endl;
    oss << "NumSlice = " << +picParams->NumSlice << std::endl;
    oss << "QpY = " << +picParams->QpY << std::endl;

    for (uint8_t i = 0; i < 16; ++i)
    {
        oss << "RefFrameList[" << +i << "] = " << +picParams->RefFrameList[i].PicEntry << std::endl;
    }

    oss << "UsedForReferenceFlags = " << +picParams->UsedForReferenceFlags << std::endl;
    oss << "CurrFieldOrderCnt[0] = " << +picParams->CurrFieldOrderCnt[0] << std::endl;
    oss << "CurrFieldOrderCnt[1] = " << +picParams->CurrFieldOrderCnt[1] << std::endl;

    for (uint8_t i = 0; i < 16; ++i)
    {
        for (uint8_t j = 0; j < 2; ++j)
        {
            oss << "FieldOrderCntList[" << +i << "]"
                << "[" << +j << "] = " << +picParams->FieldOrderCntList[i][j] << std::endl;
        }
    }

    oss << "frame_num = " << +picParams->frame_num << std::endl;
    oss << "bLastPicInSeq = " << +picParams->bLastPicInSeq << std::endl;
    oss << "bLastPicInStream = " << +picParams->bLastPicInStream << std::endl;

    // User Flags parameters
    oss << "# bUseRawPicForRef = " << +picParams->UserFlags.bUseRawPicForRef << std::endl;
    oss << "# bDisableAcceleratorHeaderPacking = " << +picParams->UserFlags.bDisableAcceleratorHeaderPacking << std::endl;
    oss << "# bDisableSubMBPartition = " << +picParams->UserFlags.bDisableSubMBPartition << std::endl;
    oss << "# bEmulationByteInsertion = " << +picParams->UserFlags.bEmulationByteInsertion << std::endl;
    oss << "# bEnableRollingIntraRefresh = " << +picParams->UserFlags.bEnableRollingIntraRefresh << std::endl;
    oss << "ForceRepartitionCheck =" << +picParams->UserFlags.ForceRepartitionCheck << std::endl;
    oss << "UserFlags = " << +picParams->UserFlags.Value << std::endl;
    oss << "StatusReportFeedbackNumber = " << +picParams->StatusReportFeedbackNumber << std::endl;
    oss << "bIdrPic = " << +picParams->bIdrPic << std::endl;
    oss << "pic_parameter_set_id = " << +picParams->pic_parameter_set_id << std::endl;
    oss << "seq_parameter_set_id = " << +picParams->seq_parameter_set_id << std::endl;
    oss << "num_ref_idx_l0_active_minus1 = " << +picParams->num_ref_idx_l0_active_minus1 << std::endl;
    oss << "num_ref_idx_l1_active_minus1 = " << +picParams->num_ref_idx_l1_active_minus1 << std::endl;
    oss << "chroma_qp_index_offset = " << +picParams->chroma_qp_index_offset << std::endl;
    oss << "second_chroma_qp_index_offset = " << +picParams->second_chroma_qp_index_offset << std::endl;
    oss << "entropy_coding_mode_flag = " << +picParams->entropy_coding_mode_flag << std::endl;
    oss << "pic_order_present_flag = " << +picParams->pic_order_present_flag << std::endl;
    oss << "weighted_pred_flag = " << +picParams->weighted_pred_flag << std::endl;
    oss << "weighted_bipred_idc = " << +picParams->weighted_bipred_idc << std::endl;
    oss << "constrained_intra_pred_flag = " << +picParams->constrained_intra_pred_flag << std::endl;
    oss << "transform_8x8_mode_flag = " << +picParams->transform_8x8_mode_flag << std::endl;
    oss << "pic_scaling_matrix_present_flag = " << +picParams->pic_scaling_matrix_present_flag << std::endl;
    oss << "pic_scaling_list_present_flag = " << +picParams->pic_scaling_list_present_flag[0] << std::endl;

    // pic_scaling_list_present_flag buffer contains 12 elements (only 1 acknowledged DDI document)
    oss << "# pic_scaling_list_present_flag[1-11]:";
    oss << +picParams->pic_scaling_list_present_flag[1] << " ";
    oss << +picParams->pic_scaling_list_present_flag[2] << " ";
    oss << +picParams->pic_scaling_list_present_flag[3] << " ";
    oss << +picParams->pic_scaling_list_present_flag[4] << " ";
    oss << +picParams->pic_scaling_list_present_flag[5] << " ";
    oss << +picParams->pic_scaling_list_present_flag[6] << " ";
    oss << +picParams->pic_scaling_list_present_flag[7] << " ";
    oss << +picParams->pic_scaling_list_present_flag[8] << " ";
    oss << +picParams->pic_scaling_list_present_flag[9] << " ";
    oss << +picParams->pic_scaling_list_present_flag[10] << " ";
    oss << +picParams->pic_scaling_list_present_flag[11] << std::endl;

    oss << "RefPicFlag = " << +picParams->RefPicFlag << std::endl;
    oss << "BRCPrecision = " << +picParams->BRCPrecision << std::endl;
    oss << "IntraInsertionLocation = " << +picParams->IntraRefreshMBNum << std::endl;
    oss << "IntraInsertionSize = " << +picParams->IntraRefreshUnitinMB << std::endl;
    oss << "QpDeltaForInsertedIntra = " << +picParams->IntraRefreshQPDelta << std::endl;
    oss << "SliceSizeInBytes = " << +picParams->SliceSizeInBytes << std::endl;
    oss << "bDisableRollingIntraRefreshOverlap = " << +picParams->bDisableRollingIntraRefreshOverlap << std::endl;
    oss << "NumROI = " << +picParams->NumROI << std::endl;
    oss << "MinDeltaQp = " << +picParams->MinDeltaQp << std::endl;
    oss << "MaxDeltaQp = " << +picParams->MaxDeltaQp << std::endl;

    // Dump ROI coordinates and PriorityLevelOrDQp
    for (uint16_t i = 0; i < picParams->NumROI; ++i)
    {
        oss << "ROI[" << +i << "] = [";
        oss << +picParams->ROI[i].Top << ",";
        oss << +picParams->ROI[i].Bottom << ",";
        oss << +picParams->ROI[i].Left << ",";
        oss << +picParams->ROI[i].Right << "], ";
        oss << "PriorityLevelOrDQp = " << +picParams->ROI[i].PriorityLevelOrDQp << std::endl;
    }

    oss << "NumDirtyROI = " << +picParams->NumDirtyROI << std::endl;

    // Dump Dirty ROI coordinates and PriorityLevelOrDQp
    for (uint16_t i = 0; i < picParams->NumDirtyROI; ++i)
    {
        oss << "DirtyROI[" << +i << "] = [";
        oss << +picParams->DirtyROI[i].Top << ",";
        oss << +picParams->DirtyROI[i].Bottom << ",";
        oss << +picParams->DirtyROI[i].Left << ",";
        oss << +picParams->DirtyROI[i].Right << "], ";
        oss << "PriorityLevelOrDQp = " << +picParams->DirtyROI[i].PriorityLevelOrDQp << std::endl;
    }

    oss << "SkipFrameFlag = " << +picParams->SkipFrameFlag << std::endl;
    oss << "NumSkipFrames = " << +picParams->NumSkipFrames << std::endl;
    oss << "SizeSkipFrames = " << +picParams->SizeSkipFrames << std::endl;

    // Dump Min/Max QP params
    oss << "BRCMinQp = " << +picParams->ucMinimumQP << std::endl;
    oss << "BRCMaxQp = " << +picParams->ucMaximumQP << std::endl;

    // Dump SFD threshold
    oss << "dwZMvThreshold = " << +picParams->dwZMvThreshold << std::endl;

    // Dump HME offset
    for (uint8_t i = 0; i < 16; ++i)
    {
        for (uint8_t j = 0; j < 2; ++j)
        {
            for (uint8_t k = 0; k < 2; ++k)
            {
                oss << "HMEOffset[" << +i << "][" << +j << "][" << +k << "] = " << +picParams->HMEOffset[i][j][k] << std::endl;
            }
        }
    }

    // Parameters not defined in DDI (Any non-DDI parameters printed should be preceeded by #)
    oss << "# Non-DDI Parameters:" << std::endl;
    oss << "# num_slice_groups_minus1 = " << +picParams->num_slice_groups_minus1 << std::endl;
    oss << "# pic_init_qp_minus26 = " << +picParams->pic_init_qp_minus26 << std::endl;
    oss << "# pic_init_qs_minus26 = " << +picParams->pic_init_qs_minus26 << std::endl;
    oss << "# deblocking_filter_control_present_flag = " << +picParams->deblocking_filter_control_present_flag << std::endl;
    oss << "# redundant_pic_cnt_present_flag = " << +picParams->redundant_pic_cnt_present_flag << std::endl;
    oss << "# EnableRollingIntraRefresh = " << +picParams->EnableRollingIntraRefresh << std::endl;
    oss << "# IntraRefreshMBx = " << +picParams->IntraRefreshMBx << std::endl;
    oss << "# IntraRefreshMBy = " << +picParams->IntraRefreshMBy << std::endl;
    oss << "# IntraRefreshUnitinMB = " << +picParams->IntraRefreshUnitinMB << std::endl;
    oss << "# IntraRefreshQPDelta = " << +picParams->IntraRefreshQPDelta << std::endl;

    // Dump ScalingList4x4 (6 x 16)
    for (uint8_t i = 0; i < 16; ++i)
    {
        oss << "# ScalingList4x4[" << +i << "] = ";
        oss << +matrixParams->ScalingList4x4[0][i] << " ";
        oss << +matrixParams->ScalingList4x4[1][i] << " ";
        oss << +matrixParams->ScalingList4x4[2][i] << " ";
        oss << +matrixParams->ScalingList4x4[3][i] << " ";
        oss << +matrixParams->ScalingList4x4[4][i] << " ";
        oss << +matrixParams->ScalingList4x4[5][i] << std::endl;
    }

    // ScalingList8x8 (2 x 64)
    for (uint8_t i = 0; i < 64; ++i)
    {
        oss << "# ScalingList8x8[0/1][" << +i << "] = " << +matrixParams->ScalingList8x8[0][i] << " / " << +matrixParams->ScalingList8x8[1][i] << std::endl;
    }

    const char *fileName = m_debugInterface->CreateFileName(
        "_DDIEnc",
        CodechalDbgBufferType::bufPicParams,
        CodechalDbgExtType::txt);

    std::ofstream ofs(fileName, std::ios::out);
    ofs << oss.str();
    ofs.close();

    if (m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrDriverUltDump))
    {
        if (!m_debugInterface->m_ddiFileName.empty())
        {
            std::ofstream ofs(m_debugInterface->m_ddiFileName, std::ios::app);
            ofs << "PicNum"
                << " = \"" << m_debugInterface->m_bufferDumpFrameNum << "\"" << std::endl;
            ofs << "PicParamFile"
                << " = \"" << m_debugInterface->m_fileName << "\"" << std::endl;
            ofs.close();
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalEncodeAvcBase::DumpFeiPicParams(
    CodecEncodeAvcFeiPicParams *feiPicParams)
{
    CODECHAL_DEBUG_FUNCTION_ENTER;

    if (!m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrFeiPicParams))
    {
        return MOS_STATUS_SUCCESS;
    }

    CODECHAL_DEBUG_CHK_NULL(feiPicParams);

    std::ostringstream oss;
    oss.setf(std::ios::showbase | std::ios::uppercase);

    oss << "# DDI Parameters:" << std::endl;
    oss << "SearchPath = " << +feiPicParams->SearchPath << std::endl;
    oss << "LenSP = " << +feiPicParams->LenSP << std::endl;
    oss << "SubMBPartMask = " << +feiPicParams->SubMBPartMask << std::endl;
    oss << "IntraPartMask = " << +feiPicParams->IntraPartMask << std::endl;
    oss << "MultiPredL0 = " << +feiPicParams->MultiPredL0 << std::endl;
    oss << "MultiPredL1 = " << +feiPicParams->MultiPredL1 << std::endl;
    oss << "SubPelMode = " << +feiPicParams->SubPelMode << std::endl;
    oss << "InterSAD = " << +feiPicParams->InterSAD << std::endl;
    oss << "IntraSAD = " << +feiPicParams->IntraSAD << std::endl;
    oss << "NumMVPredictors = " << +feiPicParams->NumMVPredictorsL0 << std::endl;
    oss << "NumMVPredictorsL1 = " << +feiPicParams->NumMVPredictorsL1 << std::endl;
    oss << "DistortionType = " << +feiPicParams->DistortionType << std::endl;
    oss << "DistortionEnable = " << +feiPicParams->DistortionEnable << std::endl;
    oss << "RepartitionCheckEnable = " << +feiPicParams->RepartitionCheckEnable << std::endl;
    oss << "AdaptiveSearch = " << +feiPicParams->AdaptiveSearch << std::endl;
    oss << "MVPredictorEnable = " << +feiPicParams->MVPredictorEnable << std::endl;
    oss << "bMBQp = " << +feiPicParams->bMBQp << std::endl;
    oss << "bPerMBInput = " << +feiPicParams->bPerMBInput << std::endl;
    oss << "bMBSizeCtrl = " << +feiPicParams->bMBSizeCtrl << std::endl;
    oss << "RefWidth = " << +feiPicParams->RefWidth << std::endl;
    oss << "RefHeight = " << +feiPicParams->RefHeight << std::endl;
    oss << "SearchWindow = " << +feiPicParams->SearchWindow << std::endl;

    const char *fileName = m_debugInterface->CreateFileName(
        "_DDIEnc",
        CodechalDbgBufferType::bufFeiPicParams,
        CodechalDbgExtType::txt);

    std::ofstream ofs(fileName, std::ios::out);
    ofs << oss.str();
    ofs.close();

    if (m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrDriverUltDump))
    {
        if (!m_debugInterface->m_ddiFileName.empty())
        {
            std::ofstream ofs(m_debugInterface->m_ddiFileName, std::ios::app);
            ofs << "PicNum"
                << " = \"" << m_debugInterface->m_bufferDumpFrameNum << "\"" << std::endl;
            ofs << "FeiPicParamFile"
                << " = \"" << m_debugInterface->m_fileName << "\"" << std::endl;
            ofs.close();
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalEncodeAvcBase::DumpSliceParams(
    PCODEC_AVC_ENCODE_SLICE_PARAMS sliceParams,
    PCODEC_AVC_ENCODE_PIC_PARAMS   picParams)
{
    CODECHAL_DEBUG_FUNCTION_ENTER;

    if (!m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrSlcParams))
    {
        return MOS_STATUS_SUCCESS;
    }

    CODECHAL_DEBUG_CHK_NULL(sliceParams);

    m_debugInterface->m_sliceId = sliceParams->slice_id;  // set here for constructing debug file name

    std::ostringstream oss;
    oss.setf(std::ios::showbase | std::ios::uppercase);

    oss << "# DDI Parameters:" << std::endl;
    oss << "NumMbsForSlice = " << +sliceParams->NumMbsForSlice << std::endl;

    // RefPicList (2 x 32)
    for (uint8_t i = 0; i < 2; ++i)
    {
        for (uint8_t j = 0; j < 32; ++j)
        {
            oss << "RefPicList[" << +i << "][" << +j << "] = " << +sliceParams->RefPicList[i][j].PicEntry << std::endl;
        }
    }

    // Conditionally printed (only when picture parameters weighted_pred_flag or weighted_bipred_idc are set).
    // Weights contains 192 elements (2 x 32 x 3 x 2).
    if (picParams->weighted_pred_flag || picParams->weighted_bipred_idc)
    {
        for (uint8_t i = 0; i < 2; ++i)
        {
            for (uint8_t j = 0; j < 32; ++j)
            {
                for (uint8_t k = 0; k < 3; ++k)
                {
                    for (uint8_t l = 0; l < 2; ++l)
                    {
                        oss << "Weights[" << +i << "][" << +j << "][" << +k << "][" << +l << "]: " << +sliceParams->Weights[i][j][k][l] << std::endl;
                    }
                }
            }
        }
    }

    oss << "first_mb_in_slice = " << +sliceParams->first_mb_in_slice << std::endl;
    oss << "slice_type = " << +sliceParams->slice_type << std::endl;
    oss << "pic_parameter_set_id = " << +sliceParams->pic_parameter_set_id << std::endl;
    oss << "direct_spatial_mv_pred_flag = " << +sliceParams->direct_spatial_mv_pred_flag << std::endl;
    oss << "num_ref_idx_active_override_flag = " << +sliceParams->num_ref_idx_active_override_flag << std::endl;
    oss << "long_term_reference_flag = " << +sliceParams->long_term_reference_flag << std::endl;
    oss << "idr_pic_id = " << +sliceParams->idr_pic_id << std::endl;
    oss << "pic_order_cnt_lsb = " << +sliceParams->pic_order_cnt_lsb << std::endl;
    oss << "delta_pic_order_cnt_bottom = " << +sliceParams->delta_pic_order_cnt_bottom << std::endl;
    oss << "delta_pic_order_cnt[0] = " << +sliceParams->delta_pic_order_cnt[0] << std::endl;
    oss << "delta_pic_order_cnt[1] = " << +sliceParams->delta_pic_order_cnt[1] << std::endl;
    oss << "num_ref_idx_l0_active_minus1 = " << +sliceParams->num_ref_idx_l0_active_minus1 << std::endl;
    oss << "num_ref_idx_l1_active_minus1 = " << +sliceParams->num_ref_idx_l1_active_minus1 << std::endl;
    oss << "luma_log2_weight_denom = " << +sliceParams->luma_log2_weight_denom << std::endl;
    oss << "chroma_log2_weight_denom = " << +sliceParams->chroma_log2_weight_denom << std::endl;
    oss << "cabac_init_idc = " << +sliceParams->cabac_init_idc << std::endl;
    oss << "slice_qp_delta = " << +sliceParams->slice_qp_delta << std::endl;
    oss << "disable_deblocking_filter_idc = " << +sliceParams->disable_deblocking_filter_idc << std::endl;
    oss << "slice_alpha_c0_offset_div2 = " << +sliceParams->slice_alpha_c0_offset_div2 << std::endl;
    oss << "slice_beta_offset_div2 = " << +sliceParams->slice_beta_offset_div2 << std::endl;
    oss << "slice_id = " << +sliceParams->slice_id << std::endl;
    oss << "luma_weight_flag[0] = " << +sliceParams->luma_weight_flag[0] << std::endl;
    oss << "luma_weight_flag[1] = " << +sliceParams->luma_weight_flag[1] << std::endl;
    oss << "chroma_weight_flag[0] = " << +sliceParams->chroma_weight_flag[0] << std::endl;
    oss << "chroma_weight_flag[1] = " << +sliceParams->chroma_weight_flag[1] << std::endl;

    // Parameters not in DDI (Any non-DDI parameters printed should be preceeded by #)
    oss << "# Non-DDI Parameters:" << std::endl;

    // PicOrder (2 x 32) - Dump in 32 blocks of 2 chunks per line
    for (uint16_t i = 0; i < 32; ++i)
    {
        CODECHAL_DEBUG_CHK_STATUS(DumpEncodePicReorder(
            oss,
            0,
            i,
            &(sliceParams->PicOrder[0][i])));
        CODECHAL_DEBUG_CHK_STATUS(DumpEncodePicReorder(
            oss,
            1,
            i,
            &(sliceParams->PicOrder[1][i])));
    }

    oss << "# colour_plane_id = " << +sliceParams->colour_plane_id << std::endl;
    oss << "# frame_num = " << +sliceParams->frame_num << std::endl;
    oss << "# field_pic_flag = " << std::hex << +sliceParams->field_pic_flag << std::endl;
    oss << "# bottom_field_flag = " << std::hex << +sliceParams->bottom_field_flag << std::endl;
    oss << "# redundant_pic_cnt = " << std::dec << +sliceParams->redundant_pic_cnt << std::endl;
    oss << "# sp_for_switch_flag = " << std::hex << +sliceParams->sp_for_switch_flag << std::endl;
    oss << "# slice_qs_delta = " << std::dec << +sliceParams->slice_qs_delta << std::endl;
    oss << "# ref_pic_list_reordering_flag_l0 = " << std::hex << +sliceParams->ref_pic_list_reordering_flag_l0 << std::endl;
    oss << "# ref_pic_list_reordering_flag_l1 = " << std::hex << +sliceParams->ref_pic_list_reordering_flag_l1 << std::endl;
    oss << "# no_output_of_prior_pics_flag = " << std::hex << +sliceParams->no_output_of_prior_pics_flag << std::endl;
    oss << "# adaptive_ref_pic_marking_mode_flag = " << std::hex << +sliceParams->adaptive_ref_pic_marking_mode_flag << std::endl;
    oss << "# MaxFrameNum = " << std::dec << +sliceParams->MaxFrameNum << std::endl;
    oss << "# NumReorder = " << std::dec << +sliceParams->NumReorder << std::endl;

    const char *fileName = m_debugInterface->CreateFileName(
        "_DDIEnc",
        CodechalDbgBufferType::bufSlcParams,
        CodechalDbgExtType::txt);

    std::ofstream ofs(fileName, std::ios::out);
    ofs << oss.str();
    ofs.close();

    if (m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrDriverUltDump))
    {
        if (!m_debugInterface->m_ddiFileName.empty())
        {
            std::ofstream ofs(m_debugInterface->m_ddiFileName, std::ios::app);
            ofs << "SlcParamFile"
                << " = \"" << m_debugInterface->m_fileName << "\"" << std::endl;
            ofs.close();
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalEncodeAvcBase::DumpVuiParams(
    PCODECHAL_ENCODE_AVC_VUI_PARAMS vuiParams)
{
    CODECHAL_DEBUG_FUNCTION_ENTER;

    if (!m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrVuiParams))
    {
        return MOS_STATUS_SUCCESS;
    }

    CODECHAL_DEBUG_CHK_NULL(vuiParams);

    std::ostringstream oss;
    oss.setf(std::ios::showbase | std::ios::uppercase);

    oss << "# DDI Parameters:" << std::endl;
    oss << "aspect_ratio_info_present_flag = " << +vuiParams->aspect_ratio_info_present_flag << std::endl;
    oss << "overscan_info_present_flag = " << +vuiParams->overscan_info_present_flag << std::endl;
    oss << "overscan_appropriate_flag = " << +vuiParams->overscan_appropriate_flag << std::endl;
    oss << "video_signal_type_present_flag = " << +vuiParams->video_signal_type_present_flag << std::endl;
    oss << "video_full_range_flag = " << +vuiParams->video_full_range_flag << std::endl;
    oss << "colour_description_present_flag = " << +vuiParams->colour_description_present_flag << std::endl;
    oss << "chroma_loc_info_present_flag = " << +vuiParams->chroma_loc_info_present_flag << std::endl;
    oss << "timing_info_present_flag = " << +vuiParams->timing_info_present_flag << std::endl;
    oss << "fixed_frame_rate_flag = " << +vuiParams->fixed_frame_rate_flag << std::endl;
    oss << "nal_hrd_parameters_present_flag = " << +vuiParams->nal_hrd_parameters_present_flag << std::endl;
    oss << "vcl_hrd_parameters_present_flag = " << +vuiParams->vcl_hrd_parameters_present_flag << std::endl;
    oss << "low_delay_hrd_flag = " << +vuiParams->low_delay_hrd_flag << std::endl;
    oss << "pic_struct_present_flag = " << +vuiParams->pic_struct_present_flag << std::endl;
    oss << "bitstream_restriction_flag = " << +vuiParams->bitstream_restriction_flag << std::endl;
    oss << "motion_vectors_over_pic_boundaries_flag = " << +vuiParams->motion_vectors_over_pic_boundaries_flag << std::endl;
    oss << "sar_width = " << +vuiParams->sar_width << std::endl;
    oss << "sar_height = " << +vuiParams->sar_height << std::endl;
    oss << "aspect_ratio_idc = " << +vuiParams->aspect_ratio_idc << std::endl;
    oss << "video_format = " << +vuiParams->video_format << std::endl;
    oss << "colour_primaries = " << +vuiParams->colour_primaries << std::endl;
    oss << "transfer_characteristics = " << +vuiParams->transfer_characteristics << std::endl;
    oss << "matrix_coefficients = " << +vuiParams->matrix_coefficients << std::endl;
    oss << "chroma_sample_loc_type_top_field = " << +vuiParams->chroma_sample_loc_type_top_field << std::endl;
    oss << "chroma_sample_loc_type_bottom_field = " << +vuiParams->chroma_sample_loc_type_bottom_field << std::endl;
    oss << "max_bytes_per_pic_denom = " << +vuiParams->max_bytes_per_pic_denom << std::endl;
    oss << "max_bits_per_mb_denom = " << +vuiParams->max_bits_per_mb_denom << std::endl;
    oss << "log2_max_mv_length_horizontal = " << +vuiParams->log2_max_mv_length_horizontal << std::endl;
    oss << "log2_max_mv_length_vertical = " << +vuiParams->log2_max_mv_length_vertical << std::endl;
    oss << "num_reorder_frames = " << +vuiParams->num_reorder_frames << std::endl;
    oss << "num_units_in_tick = " << +vuiParams->num_units_in_tick << std::endl;
    oss << "time_scale = " << +vuiParams->time_scale << std::endl;
    oss << "max_dec_frame_buffering = " << +vuiParams->max_dec_frame_buffering << std::endl;
    oss << "cpb_cnt_minus1 = " << +vuiParams->cpb_cnt_minus1 << std::endl;
    oss << "bit_rate_scale = " << +vuiParams->bit_rate_scale << std::endl;
    oss << "cpb_size_scale = " << +vuiParams->cpb_size_scale << std::endl;

    // bit_rate_value_minus1 (32 in size)
    for (uint8_t i = 0; i < 32; ++i)
    {
        oss << "bit_rate_value_minus1[" << +i << "] = " << +vuiParams->bit_rate_value_minus1[i] << std::endl;
    }

    // cpb_size_value_minus1 (32 in size)
    for (uint8_t i = 0; i < 32; ++i)
    {
        oss << "cpb_size_value_minus1[" << +i << "] = " << +vuiParams->cpb_size_value_minus1[i] << std::endl;
    }

    oss << "cbr_flag = " << +vuiParams->cbr_flag << std::endl;
    oss << "initial_cpb_removal_delay_length_minus1 = " << +vuiParams->initial_cpb_removal_delay_length_minus1 << std::endl;
    oss << "cpb_removal_delay_length_minus1 = " << +vuiParams->cpb_removal_delay_length_minus1 << std::endl;
    oss << "dpb_output_delay_length_minus1 = " << +vuiParams->dpb_output_delay_length_minus1 << std::endl;
    oss << "time_offset_length = " << +vuiParams->time_offset_length << std::endl;

    const char *fileName = m_debugInterface->CreateFileName(
        "_DDIEnc",
        CodechalDbgBufferType::bufVuiParams,
        CodechalDbgExtType::txt);

    std::ofstream ofs(fileName, std::ios::out);
    ofs << oss.str();
    ofs.close();

    if (m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrDriverUltDump))
    {
        if (!m_debugInterface->m_ddiFileName.empty())
        {
            std::ofstream ofs(m_debugInterface->m_ddiFileName, std::ios::app);
            ofs << "VuiParamFile"
                << " = \"" << m_debugInterface->m_fileName << "\"" << std::endl;
            ofs.close();
        }
    }

    return MOS_STATUS_SUCCESS;
}

bool SearchNALHeader(
    PMHW_VDBOX_AVC_SLICE_STATE sliceState,
    uint32_t                   startCode)
{
    CODECHAL_DEBUG_FUNCTION_ENTER;

    for (auto i = 0; i < CODECHAL_ENCODE_AVC_MAX_NAL_TYPE; i++)
    {
        if (sliceState->ppNalUnitParams[i]->uiSize > 0)
        {
            uint32_t offset   = 0;
            uint8_t *dataBase = (uint8_t *)(sliceState->pBsBuffer->pBase + sliceState->ppNalUnitParams[i]->uiOffset);

            while (offset < sliceState->ppNalUnitParams[i]->uiSize)
            {
                uint8_t *dataBuf = dataBase + offset;

                if (*dataBuf == 0)
                {
                    uint32_t data = ((*dataBuf) << 24) + ((*(dataBuf + 1)) << 16) + ((*(dataBuf + 2)) << 8) + (*(dataBuf + 3));

                    if ((data & 0xFFFFFF00) == 0x00000100)
                    {
                        if (data == startCode)
                        {
                            return true;
                        }

                        offset += 4;
                    }
                    else
                    {
                        offset++;
                    }
                }
                else
                {
                    offset++;
                }
            }
        }
    }

    return false;
}

MOS_STATUS CodechalEncodeAvcBase::CreateAvcPar()
{
    CODECHAL_DEBUG_FUNCTION_ENTER;

    CODECHAL_DEBUG_CHK_NULL(m_debugInterface);

    if (m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrDumpEncodePar))
    {
        CODECHAL_DEBUG_CHK_NULL(m_avcPar = MOS_New(EncodeAvcPar));
        MOS_ZeroMemory(m_avcPar, sizeof(EncodeAvcPar));
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalEncodeAvcBase::DestroyAvcPar()
{
    CODECHAL_DEBUG_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(DumpSeqParFile());
    MOS_Delete(m_avcPar);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalEncodeAvcBase::PopulateConstParam()
{
    CODECHAL_DEBUG_FUNCTION_ENTER;

    CODECHAL_DEBUG_CHK_NULL(m_debugInterface);

    if (!m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrDumpEncodePar))
    {
        return MOS_STATUS_SUCCESS;
    }

    if (m_encodeParState->m_isConstDumped)
    {
        return MOS_STATUS_SUCCESS;
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(PopulateTargetUsage());

    std::ostringstream oss;
    oss.setf(std::ios::showbase | std::ios::uppercase);

    // Constant
    oss << "EnableStatistics = 0" << std::endl;
    oss << "KernelDumpEnable = 16" << std::endl;
    oss << "Kernel = 1" << std::endl;
    oss << "StartMB = -1" << std::endl;
    oss << "EndMB = -1" << std::endl;
    oss << "GOPMode = 0" << std::endl;
    oss << "BoxFilter = 1" << std::endl;
    oss << "EnableBpyramid = 0" << std::endl;
    oss << "EnableIPCM = 1" << std::endl;
    oss << "EnableMVUnpacked = 1" << std::endl;
    oss << "EnableNewModeCost = 1" << std::endl;
    oss << "EnablePAFF = 0" << std::endl;
    oss << "EnableSkip = 3" << std::endl;
    oss << "EnableSkipBiasAdjustment = 1" << std::endl;
    oss << "HierarchicalMEFlag = 3" << std::endl;
    oss << "NumRefStartB = 1" << std::endl;
    oss << "PAFFThreshold = 10000" << std::endl;
    oss << "PicScalingList = 0" << std::endl;
    oss << "SeqScalingList = 0" << std::endl;
    oss << "SkipChromaCBPDetection = 1" << std::endl;
    oss << "IdrInterval = 0" << std::endl;
    oss << "AddEoSquenceNAL = 1" << std::endl;
    oss << "AddEoStreamNAL = 1" << std::endl;
    oss << "CabacZeroWordFlag = 0" << std::endl;
    oss << "EncodeFrameSizeTolerance = 0" << std::endl;
    oss << "ForceIntraDC = 0" << std::endl;
    oss << "HMECoarseShape = 2" << std::endl;
    oss << "HMESubPelMode = 3" << std::endl;
    oss << "IntraDirectionBias = 1" << std::endl;
    oss << "IntraPrediction = 0" << std::endl;
    oss << "InterSADMeasure = 2" << std::endl;
    oss << "IntraSADMeasure = 2" << std::endl;
    oss << "CostingFeature = 3" << std::endl;
    oss << "CostingType = 1" << std::endl;
    oss << "ModeCosting = 1" << std::endl;
    oss << "HighQPMvCostEnable = 1" << std::endl;
    oss << "HighQPHMECostEnable = 1" << std::endl;
    oss << "RefIDCostMode = 0" << std::endl;
    oss << "EnableAdaptiveLambdaOffset = 1" << std::endl;

    m_encodeParState->m_isConstDumped = true;

    const char *fileName = m_debugInterface->CreateFileName(
        "EncodeSequence",
        "EncodePar",
        CodechalDbgExtType::par);

    std::ofstream ofs(fileName, std::ios::app);
    ofs << oss.str();
    ofs.close();

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalEncodeAvcBase::PopulateTargetUsage()
{
    CODECHAL_DEBUG_FUNCTION_ENTER;

    if (m_populateTargetUsage == false)
    {
        return MOS_STATUS_SUCCESS;
    }

    const char *fileName = m_debugInterface->CreateFileName(
        "EncodeSequence",
        "EncodePar",
        CodechalDbgExtType::par);

    std::ifstream ifs(fileName);
    std::string   str((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
    ifs.close();
    std::ofstream ofs(fileName, std::ios::trunc);
    ofs << "TargetUsage = " << static_cast<uint32_t>(m_avcSeqParam->TargetUsage) << std::endl;
    ofs << str;
    ofs.close();

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalEncodeAvcBase::PopulateDdiParam(
    PCODEC_AVC_ENCODE_SEQUENCE_PARAMS avcSeqParams,
    PCODEC_AVC_ENCODE_PIC_PARAMS      avcPicParams,
    PCODEC_AVC_ENCODE_SLICE_PARAMS    avcSlcParams)
{
    CODECHAL_DEBUG_FUNCTION_ENTER;

    CODECHAL_DEBUG_CHK_NULL(m_debugInterface);

    if (!m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrDumpEncodePar))
    {
        return MOS_STATUS_SUCCESS;
    }

    if (Slice_Type[avcSlcParams->slice_type] == SLICE_I || Slice_Type[avcSlcParams->slice_type] == SLICE_SI)
    {
        m_avcPar->ProfileIDC           = avcSeqParams->Profile;
        m_avcPar->LevelIDC             = avcSeqParams->Level;
        m_avcPar->DisableVUIHeader     = !avcSeqParams->vui_parameters_present_flag;
        m_avcPar->ChromaFormatIDC      = avcSeqParams->chroma_format_idc;
        m_avcPar->ChromaQpOffset       = avcPicParams->chroma_qp_index_offset;
        m_avcPar->SecondChromaQpOffset = avcPicParams->second_chroma_qp_index_offset;

        uint8_t pictureCodingType   = CodecHal_PictureIsFrame(avcPicParams->CurrOriginalPic) ? 0 : (CodecHal_PictureIsTopField(avcPicParams->CurrOriginalPic) ? 1 : 3);
        m_avcPar->PictureCodingType = pictureCodingType;

        uint16_t gopP  = (avcSeqParams->GopRefDist) ? ((avcSeqParams->GopPicSize - 1) / avcSeqParams->GopRefDist) : 0;
        uint16_t gopB  = avcSeqParams->GopPicSize - 1 - gopP;
        m_avcPar->NumP = gopP;
        m_avcPar->NumB = ((gopP > 0) ? (gopB / gopP) : 0);

        if ((avcPicParams->NumSlice * m_sliceHeight) >=
            (uint32_t)(avcSeqParams->pic_height_in_map_units_minus1 + 1 + m_sliceHeight))
        {
            m_avcPar->NumSlices = avcPicParams->NumSlice - 1;
        }
        else
        {
            m_avcPar->NumSlices = avcPicParams->NumSlice;
        }

        m_avcPar->ISliceQP   = avcPicParams->QpY + avcSlcParams->slice_qp_delta;
        m_avcPar->FrameRateM = ((avcSeqParams->FramesPer100Sec % 100) == 0) ? (avcSeqParams->FramesPer100Sec / 100) : avcSeqParams->FramesPer100Sec;
        m_avcPar->FrameRateD = ((avcSeqParams->FramesPer100Sec % 100) == 0) ? 1 : 100;

        uint8_t brcMethod = 0;
        uint8_t brcType   = 0;

        if (CodecHalIsRateControlBrc(avcSeqParams->RateControlMethod, CODECHAL_AVC))
        {
            brcMethod = 2;

            switch (avcSeqParams->RateControlMethod)
            {
            case RATECONTROL_ICQ:
                brcMethod = m_vdencEnabled ? 2 : 3;
                brcType   = 16;
                break;
            case RATECONTROL_QVBR:
                brcMethod = m_vdencEnabled ? 2 : 4;
                brcType   = 2;
                break;
            case RATECONTROL_CBR:
                brcType = 1;
                break;
            case RATECONTROL_VBR:
                brcType = 2;
                break;
            case RATECONTROL_VCM:
                brcType = m_vdencEnabled ? 4 : 3;
                break;
            default:
                brcMethod = 0;
                brcType   = 0;
                break;
            }

            if (avcSeqParams->FrameSizeTolerance == EFRAMESIZETOL_EXTREMELY_LOW)
            {
                // low delay mode
                brcType = 8;
            }
        }

        m_avcPar->BRCMethod = brcMethod;
        m_avcPar->BRCType   = brcType;

        m_avcPar->DeblockingIDC         = avcSlcParams->disable_deblocking_filter_idc;
        m_avcPar->DeblockingFilterAlpha = avcSlcParams->slice_alpha_c0_offset_div2 << 1;
        m_avcPar->DeblockingFilterBeta  = avcSlcParams->slice_beta_offset_div2 << 1;
        m_avcPar->EntropyCodingMode     = avcPicParams->entropy_coding_mode_flag;
        m_avcPar->DirectInference       = avcSeqParams->direct_8x8_inference_flag;
        m_avcPar->Transform8x8Mode      = avcPicParams->transform_8x8_mode_flag;
        m_avcPar->CRFQualityFactor      = avcSeqParams->ICQQualityFactor;
        m_avcPar->ConstrainedIntraPred  = avcPicParams->constrained_intra_pred_flag;

        // This is only for header matching although I frame doesn't have references
        if (avcSlcParams->num_ref_idx_active_override_flag)
        {
            m_avcPar->MaxRefIdxL0 = MOS_MAX(m_avcPar->MaxRefIdxL0, avcSlcParams->num_ref_idx_l0_active_minus1);
            m_avcPar->MaxRefIdxL1 = MOS_MAX(m_avcPar->MaxRefIdxL1, avcSlcParams->num_ref_idx_l1_active_minus1);
        }
        else
        {
            m_avcPar->MaxRefIdxL0 = MOS_MAX(m_avcPar->MaxRefIdxL0, avcPicParams->num_ref_idx_l0_active_minus1);
            m_avcPar->MaxRefIdxL1 = MOS_MAX(m_avcPar->MaxRefIdxL1, avcPicParams->num_ref_idx_l1_active_minus1);
        }

        if (m_vdencEnabled)
        {
            m_avcPar->SliceMode = avcSeqParams->EnableSliceLevelRateCtrl ? 2 : 0;
        }
    }
    else if (Slice_Type[avcSlcParams->slice_type] == SLICE_P || Slice_Type[avcSlcParams->slice_type] == SLICE_SP)
    {
        m_avcPar->PSliceQP     = avcPicParams->QpY + avcSlcParams->slice_qp_delta;
        m_avcPar->CabacInitIDC = avcSlcParams->cabac_init_idc;

        if (avcSlcParams->num_ref_idx_active_override_flag)
        {
            m_avcPar->MaxRefIdxL0 = MOS_MAX(m_avcPar->MaxRefIdxL0, avcSlcParams->num_ref_idx_l0_active_minus1);
            m_avcPar->MaxRefIdxL1 = MOS_MAX(m_avcPar->MaxRefIdxL1, avcSlcParams->num_ref_idx_l1_active_minus1);
        }
        else
        {
            m_avcPar->MaxRefIdxL0 = MOS_MAX(m_avcPar->MaxRefIdxL0, avcPicParams->num_ref_idx_l0_active_minus1);
            m_avcPar->MaxRefIdxL1 = MOS_MAX(m_avcPar->MaxRefIdxL1, avcPicParams->num_ref_idx_l1_active_minus1);
        }

        if (m_avcPar->NumB == 0)  // There's no B frame
        {
            m_avcPar->EnableWeightPredictionDetection = avcPicParams->weighted_pred_flag > 0 ? 1 : 0;
        }
        m_avcPar->WeightedPred    = avcPicParams->weighted_pred_flag;
        m_avcPar->UseOrigAsRef    = avcPicParams->UserFlags.bUseRawPicForRef;
        m_avcPar->BiSubMbPartMask = avcPicParams->UserFlags.bDisableSubMBPartition ? (bool)(CODECHAL_ENCODE_AVC_DISABLE_4X4_SUB_MB_PARTITION | CODECHAL_ENCODE_AVC_DISABLE_4X8_SUB_MB_PARTITION | CODECHAL_ENCODE_AVC_DISABLE_8X4_SUB_MB_PARTITION) : 0;

        if (m_vdencEnabled)
        {
            if (m_targetUsage == 1 || m_targetUsage == 2)
            {
                m_avcPar->StaticFrameZMVPercent = avcPicParams->dwZMvThreshold;
            }
            else
            {
                m_avcPar->StaticFrameZMVPercent = 100;
            }

            if (avcPicParams->bEnableHMEOffset)
            {
                m_avcPar->hme0XOffset = MOS_CLAMP_MIN_MAX(avcPicParams->HMEOffset[0][0][0], -128, 127);
                m_avcPar->hme0YOffset = MOS_CLAMP_MIN_MAX(avcPicParams->HMEOffset[0][0][1], -128, 127);
                m_avcPar->hme1XOffset = MOS_CLAMP_MIN_MAX(avcPicParams->HMEOffset[1][0][0], -128, 127);
                m_avcPar->hme1YOffset = MOS_CLAMP_MIN_MAX(avcPicParams->HMEOffset[1][0][1], -128, 127);
            }
        }
    }
    else if (Slice_Type[avcSlcParams->slice_type] == SLICE_B)
    {
        m_avcPar->BSliceQP = avcPicParams->QpY + avcSlcParams->slice_qp_delta;

        if (avcSlcParams->num_ref_idx_active_override_flag)
        {
            m_avcPar->MaxBRefIdxL0 = MOS_MAX(m_avcPar->MaxBRefIdxL0, avcSlcParams->num_ref_idx_l0_active_minus1);
        }
        else
        {
            m_avcPar->MaxBRefIdxL0 = MOS_MAX(m_avcPar->MaxBRefIdxL0, avcPicParams->num_ref_idx_l0_active_minus1);
        }

        m_avcPar->EnableWeightPredictionDetection = (avcPicParams->weighted_bipred_idc | avcPicParams->weighted_pred_flag) > 0 ? 1 : 0;
        m_avcPar->WeightedBiPred                  = avcPicParams->weighted_bipred_idc;
        m_avcPar->BiWeight                        = m_biWeight;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalEncodeAvcBase::PopulateSliceStateParam(
    bool                       adaptiveRoundingInterEnable,
    PMHW_VDBOX_AVC_SLICE_STATE sliceState)
{
    CODECHAL_DEBUG_FUNCTION_ENTER;

    CODECHAL_DEBUG_CHK_NULL(m_debugInterface);

    if (!m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrDumpEncodePar))
    {
        return MOS_STATUS_SUCCESS;
    }

    if (m_pictureCodingType == I_TYPE)
    {
        m_avcPar->RoundingIntraEnabled = true;
        m_avcPar->RoundingIntra        = 5;
        // Set to 1 first, we don't consider 0 case as we only dump I frame param once
        m_avcPar->FrmHdrEncodingFrequency = 1;  // 1: picture header in every IDR frame

        // Search SEI NAL
        m_avcPar->EnableSEI = SearchNALHeader(sliceState, CODECHAL_DEBUG_ENCODE_AVC_NAL_START_CODE_SEI);
    }
    else if (m_pictureCodingType == P_TYPE)
    {
        m_avcPar->EnableAdaptiveRounding = adaptiveRoundingInterEnable;
        m_avcPar->RoundingInterEnabled   = sliceState->bRoundingInterEnable;
        m_avcPar->RoundingInter          = sliceState->dwRoundingValue;

        // Search PPS NAL
        m_avcPar->FrmHdrEncodingFrequency =
            SearchNALHeader(sliceState, CODECHAL_DEBUG_ENCODE_AVC_NAL_START_CODE_PPS) ? 2 : m_avcPar->FrmHdrEncodingFrequency;
    }
    else if (m_pictureCodingType == B_TYPE)
    {
        m_avcPar->RoundingInterB = sliceState->dwRoundingValue;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalEncodeAvcBase::PopulateSfdParam(
    void *cmd)
{
    CODECHAL_DEBUG_FUNCTION_ENTER;

    CODECHAL_DEBUG_CHK_NULL(m_debugInterface);

    if (!m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrDumpEncodePar))
    {
        return MOS_STATUS_SUCCESS;
    }

    CODECHAL_ENCODE_AVC_SFD_CURBE_COMMON *curbe = (CODECHAL_ENCODE_AVC_SFD_CURBE_COMMON *)cmd;

    if (m_pictureCodingType == P_TYPE)
    {
        m_avcPar->EnableIntraCostScalingForStaticFrame = curbe->DW0.EnableIntraCostScalingForStaticFrame;
        m_avcPar->StaticFrameIntraCostScalingRatioP    = 240;
        m_avcPar->AdaptiveMvStreamIn                   = curbe->DW0.EnableAdaptiveMvStreamIn;
        m_avcPar->LargeMvThresh                        = curbe->DW3.LargeMvThresh;
        m_avcPar->LargeMvPctThreshold                  = 1;
    }

    return MOS_STATUS_SUCCESS;
}
#endif

MOS_STATUS CodechalEncodeAvcBase::SetFrameStoreIds(uint8_t frameIdx)
{
    uint8_t invalidFrame = (m_mode == CODECHAL_DECODE_MODE_AVCVLD) ? 0x7f : 0x1f;

    for (uint8_t i = 0; i < m_refList[frameIdx]->ucNumRef; i++)
    {
        uint8_t index;
        index = m_refList[frameIdx]->RefList[i].FrameIdx;
        if (m_refList[index]->ucFrameId == invalidFrame)
        {
            uint8_t j;
            for (j = 0; j < CODEC_AVC_MAX_NUM_REF_FRAME; j++)
            {
                if (!m_avcFrameStoreID[j].inUse)
                {
                    m_refList[index]->ucFrameId = j;
                    m_avcFrameStoreID[j].inUse  = true;
                    break;
                }
            }
            if (j == CODEC_AVC_MAX_NUM_REF_FRAME)
            {
                // should never happen, something must be wrong
                CODECHAL_PUBLIC_ASSERT(false);
                m_refList[index]->ucFrameId = 0;
                m_avcFrameStoreID[0].inUse  = true;
            }
        }
    }
    return MOS_STATUS_SUCCESS;
}
