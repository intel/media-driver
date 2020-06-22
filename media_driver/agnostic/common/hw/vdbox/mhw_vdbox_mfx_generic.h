/*
* Copyright (c) 2017-2018, Intel Corporation
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

//! \file     mhw_vdbox_mfx_generic.h
//! \brief    MHW interface for constructing MFX commands for the Vdbox engine
//! \details  Impelements shared Vdbox MFX command construction functions across all platforms as templates
//!

#ifndef _MHW_VDBOX_MFX_GENERIC_H_
#define _MHW_VDBOX_MFX_GENERIC_H_

#include "mhw_vdbox_mfx_interface.h"

//!  MHW Vdbox Mfx generic interface
/*!
This class defines the shared Mfx command construction functions across all platforms as templates
*/
template <class TMfxCmds, class TMiCmds>
class MhwVdboxMfxInterfaceGeneric : public MhwVdboxMfxInterface
{
protected:
    //!
    //! \brief    Constructor
    //!
    MhwVdboxMfxInterfaceGeneric(
        PMOS_INTERFACE osInterface,
        MhwMiInterface *miInterface,
        MhwCpInterface *cpInterface,
        bool decodeInUse) :
        MhwVdboxMfxInterface(osInterface, miInterface, cpInterface, decodeInUse)
    {
        MHW_FUNCTION_ENTER;
    }

    //!
    //! \brief   Destructor
    //!
    virtual ~MhwVdboxMfxInterfaceGeneric() {}

    MOS_STATUS AddMfdAvcPicidCmd(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMHW_VDBOX_PIC_ID_PARAMS params)
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        MHW_FUNCTION_ENTER;

        MHW_MI_CHK_NULL(cmdBuffer);
        MHW_MI_CHK_NULL(params);
        MHW_MI_CHK_NULL(params->pAvcPicIdx);

        typename TMfxCmds::MFD_AVC_PICID_STATE_CMD cmd;

        cmd.DW1.PictureidRemappingDisable = 1;
        if (params->bPicIdRemappingInUse)
        {
            uint32_t j = 0;
            cmd.DW1.PictureidRemappingDisable = 0;

            for (auto i = 0; i < (CODEC_MAX_NUM_REF_FRAME / 2); i++)
            {
                cmd.Pictureidlist1616Bits[i] = avcPicidDefault;

                if (params->pAvcPicIdx[j++].bValid)
                {
                    cmd.Pictureidlist1616Bits[i] = (cmd.Pictureidlist1616Bits[i] & 0xffff0000) | params->pAvcPicIdx[j - 1].ucPicIdx;
                }

                if (params->pAvcPicIdx[j++].bValid)
                {
                    cmd.Pictureidlist1616Bits[i] = (cmd.Pictureidlist1616Bits[i] & 0x0000ffff) | (params->pAvcPicIdx[j - 1].ucPicIdx << 16);
                }
            }
        }
        else
        {
            for (auto i = 0; i < (CODEC_MAX_NUM_REF_FRAME / 2); i++)
            {
                cmd.Pictureidlist1616Bits[i] = avcPicidDisabled;
            }
        }

        MHW_MI_CHK_STATUS(Mos_AddCommand(cmdBuffer, &cmd, sizeof(cmd)));

        return eStatus;
    }

    MOS_STATUS AddMfxQmCmd(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMHW_VDBOX_QM_PARAMS params)
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        MHW_FUNCTION_ENTER;

        MHW_MI_CHK_NULL(cmdBuffer);
        MHW_MI_CHK_NULL(params);

        typename TMfxCmds::MFX_QM_STATE_CMD cmd;

        uint8_t* qMatrix = (uint8_t*)cmd.ForwardQuantizerMatrix;

        if (params->Standard == CODECHAL_AVC)
        {
            MHW_MI_CHK_NULL(params->pAvcIqMatrix);

            for (auto i = 0; i < 16; i++)
            {
                cmd.ForwardQuantizerMatrix[i] = 0;
            }

            cmd.DW1.Obj0.Avc = avcQmIntra4x4;
            for (auto i = 0; i < 3; i++)
            {
                for (auto ii = 0; ii < 16; ii++)
                {
                    qMatrix[i * 16 + ii] = params->pAvcIqMatrix->List4x4[i][ii];
                }
            }
            MHW_MI_CHK_STATUS(Mos_AddCommand(cmdBuffer, &cmd, sizeof(cmd)));

            cmd.DW1.Obj0.Avc = avcQmInter4x4;
            for (auto i = 3; i < 6; i++)
            {
                for (auto ii = 0; ii < 16; ii++)
                {
                    qMatrix[(i - 3) * 16 + ii] = params->pAvcIqMatrix->List4x4[i][ii];
                }
            }
            MHW_MI_CHK_STATUS(Mos_AddCommand(cmdBuffer, &cmd, sizeof(cmd)));

            cmd.DW1.Obj0.Avc = avcQmIntra8x8;
            for (auto ii = 0; ii < 64; ii++)
            {
                qMatrix[ii] = params->pAvcIqMatrix->List8x8[0][ii];
            }
            MHW_MI_CHK_STATUS(Mos_AddCommand(cmdBuffer, &cmd, sizeof(cmd)));

            cmd.DW1.Obj0.Avc = avcQmInter8x8;
            for (auto ii = 0; ii < 64; ii++)
            {
                qMatrix[ii] = params->pAvcIqMatrix->List8x8[1][ii];
            }
            MHW_MI_CHK_STATUS(Mos_AddCommand(cmdBuffer, &cmd, sizeof(cmd)));
        }
        else if (params->Standard == CODECHAL_MPEG2)
        {
            if (params->Mode == CODECHAL_ENCODE_MODE_MPEG2)
            {
                cmd.DW1.Obj0.Avc = mpeg2QmIntra;
                if (params->pMpeg2IqMatrix->m_loadIntraQuantiserMatrix)
                {
                    for (auto i = 0; i < 64; i++)
                    {
                        qMatrix[i] = (uint8_t)(params->pMpeg2IqMatrix->m_intraQuantiserMatrix[m_mpeg2QuantMatrixScan[i]]);
                    }
                }
                else
                {
                    for (auto i = 0; i < 64; i++)
                    {
                        qMatrix[i] = (uint8_t)(m_mpeg2DefaultIntraQuantizerMatrix[i]);
                    }
                }
                MHW_MI_CHK_STATUS(Mos_AddCommand(cmdBuffer, &cmd, sizeof(cmd)));

                cmd.DW1.Obj0.Avc = mpeg2QmNonIntra;
                if (params->pMpeg2IqMatrix->m_loadNonIntraQuantiserMatrix)
                {
                    for (auto i = 0; i < 64; i++)
                    {
                        qMatrix[i] = (uint8_t)(params->pMpeg2IqMatrix->m_nonIntraQuantiserMatrix[m_mpeg2QuantMatrixScan[i]]);
                    }
                }
                else
                {
                    for (auto i = 0; i < 64; i++)
                    {
                        qMatrix[i] = (uint8_t)(m_mpeg2DefaultNonIntraQuantizerMatrix[i]);
                    }
                }
                MHW_MI_CHK_STATUS(Mos_AddCommand(cmdBuffer, &cmd, sizeof(cmd)));
            }
            else
            {
                cmd.DW1.Obj0.Avc = mpeg2QmIntra;
                if (params->pMpeg2IqMatrix->m_loadIntraQuantiserMatrix)
                {
                    uint8_t *src = params->pMpeg2IqMatrix->m_intraQuantiserMatrix;
                    for (auto i = 0; i < 64; i++)
                    {
                        qMatrix[i] = (uint8_t)(src[m_mpeg2QuantMatrixScan[i]]);
                    }
                }
                else
                {
                    for (auto i = 0; i < 64; i++)
                    {
                        qMatrix[i] = (uint8_t)(m_mpeg2DefaultIntraQuantizerMatrix[i]);
                    }
                }

                MHW_MI_CHK_STATUS(Mos_AddCommand(cmdBuffer, &cmd, sizeof(cmd)));

                cmd.DW1.Obj0.Avc = mpeg2QmNonIntra;
                if (params->pMpeg2IqMatrix->m_loadNonIntraQuantiserMatrix)
                {
                    uint8_t *src = params->pMpeg2IqMatrix->m_nonIntraQuantiserMatrix;
                    for (auto i = 0; i < 64; i++)
                    {
                        qMatrix[i] = (uint8_t)(src[m_mpeg2QuantMatrixScan[i]]);
                    }
                }
                else
                {
                    for (auto i = 0; i < 64; i++)
                    {
                        qMatrix[i] = (uint8_t)(m_mpeg2DefaultNonIntraQuantizerMatrix[i]);
                    }
                }

                MHW_MI_CHK_STATUS(Mos_AddCommand(cmdBuffer, &cmd, sizeof(cmd)));
            }
        }
        else if (params->Standard == CODECHAL_JPEG)
        {
            cmd.DW1.Obj0.Avc = params->pJpegQuantMatrix->m_jpegQMTableType[params->JpegQMTableSelector];
            if (params->bJpegQMRotation)
            {
                for (auto i = 0; i < 8; i++)
                {
                    for (auto ii = 0; ii < 8; ii++)
                    {
                        qMatrix[i + 8 * ii] = params->pJpegQuantMatrix->m_quantMatrix[params->JpegQMTableSelector][i * 8 + ii];
                    }
                }
            }
            else
            {
                for (auto i = 0; i < 64; i++)
                {
                    qMatrix[i] = params->pJpegQuantMatrix->m_quantMatrix[params->JpegQMTableSelector][i];
                }
            }

            MHW_MI_CHK_STATUS(Mos_AddCommand(cmdBuffer, &cmd, sizeof(cmd)));
        }
        else
        {
            eStatus = MOS_STATUS_INVALID_PARAMETER;
        }

        return eStatus;
    }

    MOS_STATUS AddMfxFqmCmd(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMHW_VDBOX_QM_PARAMS params)
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        MHW_FUNCTION_ENTER;

        MHW_MI_CHK_NULL(cmdBuffer);
        MHW_MI_CHK_NULL(params);

        typename TMfxCmds::MFX_FQM_STATE_CMD cmd;

        if (params->Standard == CODECHAL_AVC)
        {
            MHW_MI_CHK_NULL(params->pAvcIqMatrix);

            PMHW_VDBOX_AVC_QM_PARAMS iqMatrix = params->pAvcIqMatrix;
            uint16_t *fqMatrix = (uint16_t*)cmd.ForwardQuantizerMatrix;

            for (auto i = 0; i < 32; i++)
            {
                cmd.ForwardQuantizerMatrix[i] = 0;
            }

            cmd.DW1.Obj0.Avc = avcQmIntra4x4;
            for (auto i = 0; i < 3; i++)
            {
                for (auto ii = 0; ii < 16; ii++)
                {
                    fqMatrix[i * 16 + ii] =
                        GetReciprocalScalingValue(iqMatrix->List4x4[i][m_columnScan4x4[ii]]);
                }
            }
            MHW_MI_CHK_STATUS(Mos_AddCommand(cmdBuffer, &cmd, sizeof(cmd)));

            cmd.DW1.Obj0.Avc = avcQmInter4x4;
            for (auto i = 0; i < 3; i++)
            {
                for (auto ii = 0; ii < 16; ii++)
                {
                    fqMatrix[i * 16 + ii] =
                        GetReciprocalScalingValue(iqMatrix->List4x4[i + 3][m_columnScan4x4[ii]]);
                }
            }
            MHW_MI_CHK_STATUS(Mos_AddCommand(cmdBuffer, &cmd, sizeof(cmd)));

            cmd.DW1.Obj0.Avc = avcQmIntra8x8;
            for (auto i = 0; i < 64; i++)
            {
                fqMatrix[i] = GetReciprocalScalingValue(iqMatrix->List8x8[0][m_columnScan8x8[i]]);
            }
            MHW_MI_CHK_STATUS(Mos_AddCommand(cmdBuffer, &cmd, sizeof(cmd)));

            cmd.DW1.Obj0.Avc = avcQmInter8x8;
            for (auto i = 0; i < 64; i++)
            {
                fqMatrix[i] = GetReciprocalScalingValue(iqMatrix->List8x8[1][m_columnScan8x8[i]]);
            }
            MHW_MI_CHK_STATUS(Mos_AddCommand(cmdBuffer, &cmd, sizeof(cmd)));
        }
        else if (params->Standard == CODECHAL_MPEG2)
        {
            uint16_t *fqMatrix = (uint16_t*)cmd.ForwardQuantizerMatrix;

            cmd.DW1.Obj0.Avc = mpeg2QmIntra;
            if (params->pMpeg2IqMatrix->m_loadIntraQuantiserMatrix)
            {
                for (auto i = 0; i < 64; i++)
                {
                    fqMatrix[i] = GetReciprocalScalingValue(
                        (uint8_t)(params->pMpeg2IqMatrix->m_intraQuantiserMatrix[m_mpeg2QuantMatrixScan[m_columnScan8x8[i]]]));
                }
            }
            else
            {
                for (auto i = 0; i < 64; i++)
                {
                    fqMatrix[i] = GetReciprocalScalingValue(
                        (uint8_t)m_mpeg2DefaultIntraQuantizerMatrix[m_columnScan8x8[i]]);
                }
            }
            MHW_MI_CHK_STATUS(Mos_AddCommand(cmdBuffer, &cmd, sizeof(cmd)));

            cmd.DW1.Obj0.Avc = mpeg2QmNonIntra;
            if (params->pMpeg2IqMatrix->m_loadNonIntraQuantiserMatrix)
            {
                for (auto i = 0; i < 64; i++)
                {
                    fqMatrix[i] = GetReciprocalScalingValue(
                        (uint8_t)(params->pMpeg2IqMatrix->m_nonIntraQuantiserMatrix[m_mpeg2QuantMatrixScan[m_columnScan8x8[i]]]));
                }
            }
            else
            {
                for (auto i = 0; i < 64; i++)
                {
                    fqMatrix[i] = GetReciprocalScalingValue(
                        (uint8_t)m_mpeg2DefaultNonIntraQuantizerMatrix[m_columnScan8x8[i]]);
                }
            }
            MHW_MI_CHK_STATUS(Mos_AddCommand(cmdBuffer, &cmd, sizeof(cmd)));
        }

        return eStatus;
    }

    MOS_STATUS AddMfxAvcRefIdx(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMHW_BATCH_BUFFER batchBuffer,
        PMHW_VDBOX_AVC_REF_IDX_PARAMS params)
    {
        MHW_FUNCTION_ENTER;

        MHW_MI_CHK_NULL(params);

        if (cmdBuffer == nullptr && batchBuffer == nullptr)
        {
            MHW_ASSERTMESSAGE("No valid buffer to add the command to!");
            return MOS_STATUS_INVALID_PARAMETER;
        }

        typename TMfxCmds::MFX_AVC_REF_IDX_STATE_CMD cmd;

        // Need to add an empty MFX_AVC_REF_IDX_STATE_CMD for dummy reference on I-Frame
        if (!params->bDummyReference)
        {
            auto uiList = params->uiList;

            cmd.DW1.RefpiclistSelect = uiList;

            CODEC_REF_LIST  **avcRefList        = (CODEC_REF_LIST **)params->avcRefList;
            AvcRefListWrite *cmdAvcRefListWrite = (AvcRefListWrite *)&(cmd.ReferenceListEntry);

            uint8_t picIDOneOnOneMapping = 0;
            if (params->bVdencInUse && uiList == LIST_1)
            {
                picIDOneOnOneMapping += params->uiNumRefForList[LIST_0] << 1;
            }

            for (uint32_t i = 0; i < params->uiNumRefForList[uiList]; i++)
            {
                uint8_t idx = params->RefPicList[uiList][i].FrameIdx;

                if (!params->bIntelEntrypointInUse)
                {
                    if (idx >= CODEC_MAX_NUM_REF_FRAME)
                    {
                        MHW_ASSERT(false); // Idx must be within 0 to 15
                        idx = 0;
                    }

                    idx = params->pAvcPicIdx[idx].ucPicIdx;
                }

                uint8_t picID = params->bPicIdRemappingInUse ?
                    params->RefPicList[uiList][i].FrameIdx : avcRefList[idx]->ucFrameId;

                // When one on one ref idx mapping is enabled, program picID count from 0, 2 ...
                if (params->oneOnOneMapping)
                {
                    picID = picIDOneOnOneMapping;
                    picIDOneOnOneMapping += 2;
                }
                cmdAvcRefListWrite->UC[i].frameStoreID = picID;
                cmdAvcRefListWrite->UC[i].bottomField =
                    CodecHal_PictureIsBottomField(params->RefPicList[uiList][i]);
                cmdAvcRefListWrite->UC[i].fieldPicFlag =
                    CodecHal_PictureIsField(params->RefPicList[uiList][i]);
                cmdAvcRefListWrite->UC[i].longTermFlag =
                    CodecHal_PictureIsLongTermRef(avcRefList[idx]->RefPic);
                cmdAvcRefListWrite->UC[i].nonExisting = 0;
            }

            for (auto i = params->uiNumRefForList[uiList]; i < 32; i++)
            {
                cmdAvcRefListWrite->UC[i].value = 0x80;
            }
        }

        MHW_MI_CHK_STATUS(Mhw_AddCommandCmdOrBB(cmdBuffer, batchBuffer, &cmd, sizeof(cmd)));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AddMfxDecodeAvcWeightOffset(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMHW_BATCH_BUFFER batchBuffer,
        PMHW_VDBOX_AVC_WEIGHTOFFSET_PARAMS params)
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        MHW_FUNCTION_ENTER;

        MHW_MI_CHK_NULL(params);

        if (cmdBuffer == nullptr && batchBuffer == nullptr)
        {
            MHW_ASSERTMESSAGE("No valid buffer to add the command to!");
            return MOS_STATUS_INVALID_PARAMETER;
        }

        typename TMfxCmds::MFX_AVC_WEIGHTOFFSET_STATE_CMD cmd;

        cmd.DW1.WeightAndOffsetSelect = params->uiList;

        //The correct explicit calculation (like in Cantiga)
        for (auto i = 0; i < CODEC_MAX_NUM_REF_FIELD; i++)
        {
            cmd.Weightoffset[3 * i] = params->Weights[params->uiList][i][0][0] & 0xFFFF; // Y weight
            cmd.Weightoffset[3 * i] |= (params->Weights[params->uiList][i][0][1] & 0xFFFF) << 16; // Y offset
            cmd.Weightoffset[3 * i + 1] = params->Weights[params->uiList][i][1][0] & 0xFFFF; // Cb weight
            cmd.Weightoffset[3 * i + 1] |= (params->Weights[params->uiList][i][1][1] & 0xFFFF) << 16; // Cb offset
            cmd.Weightoffset[3 * i + 2] = params->Weights[params->uiList][i][2][0] & 0xFFFF; // Cr weight
            cmd.Weightoffset[3 * i + 2] |= (params->Weights[params->uiList][i][2][1] & 0xFFFF) << 16; // Cr offset
        }

        MHW_MI_CHK_STATUS(Mhw_AddCommandCmdOrBB(cmdBuffer, batchBuffer, &cmd, sizeof(cmd)));

        return eStatus;
    }

    MOS_STATUS AddMfxEncodeAvcWeightOffset(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMHW_BATCH_BUFFER batchBuffer,
        PMHW_VDBOX_AVC_WEIGHTOFFSET_PARAMS params)
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        MHW_FUNCTION_ENTER;

        MHW_MI_CHK_NULL(params);

        if (cmdBuffer == nullptr && batchBuffer == nullptr)
        {
            MHW_ASSERTMESSAGE("No valid buffer to add the command to!");
            return MOS_STATUS_INVALID_PARAMETER;
        }

        typename TMfxCmds::MFX_AVC_WEIGHTOFFSET_STATE_CMD cmd;

        cmd.DW1.WeightAndOffsetSelect = params->uiList;

        for (uint32_t i = 0; i < params->uiNumRefForList; i++)
        {
            if (params->uiLumaWeightFlag & (1 << i))
            {
                cmd.Weightoffset[3 * i] = params->Weights[params->uiList][i][0][0] & 0xFFFF; // Y weight
                cmd.Weightoffset[3 * i] |= (params->Weights[params->uiList][i][0][1] & 0xFFFF) << 16; // Y offset
            }
            else
            {
                cmd.Weightoffset[3 * i] = 1 << (params->uiLumaLogWeightDenom); // Y weight
                cmd.Weightoffset[3 * i] = cmd.Weightoffset[3 * i] | (0 << 16); // Y offset
            }

            if (params->uiChromaWeightFlag & (1 << i))
            {
                cmd.Weightoffset[3 * i + 1] = params->Weights[params->uiList][i][1][0] & 0xFFFF; // Cb weight
                cmd.Weightoffset[3 * i + 1] |= (params->Weights[params->uiList][i][1][1] & 0xFFFF) << 16; // Cb offset
                cmd.Weightoffset[3 * i + 2] = params->Weights[params->uiList][i][2][0] & 0xFFFF; // Cr weight
                cmd.Weightoffset[3 * i + 2] |= (params->Weights[params->uiList][i][2][1] & 0xFFFF) << 16; // Cr offset
            }
            else
            {
                cmd.Weightoffset[3 * i + 1] = 1 << (params->uiChromaLogWeightDenom); // Cb  weight
                cmd.Weightoffset[3 * i + 1] = cmd.Weightoffset[3 * i + 1] | (0 << 16); // Cb offset
                cmd.Weightoffset[3 * i + 2] = 1 << (params->uiChromaLogWeightDenom); // Cr  weight
                cmd.Weightoffset[3 * i + 2] = cmd.Weightoffset[3 * i + 2] | (0 << 16); // Cr offset
            }
        }

        MHW_MI_CHK_STATUS(Mhw_AddCommandCmdOrBB(cmdBuffer, batchBuffer, &cmd, sizeof(cmd)));

        return eStatus;
    }

    MOS_STATUS AddMfxDecodeAvcSlice(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMHW_BATCH_BUFFER batchBuffer,
        PMHW_VDBOX_AVC_SLICE_STATE avcSliceState)
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        MHW_FUNCTION_ENTER;

        MHW_MI_CHK_NULL(avcSliceState);
        MHW_MI_CHK_NULL(avcSliceState->pAvcPicParams);
        MHW_MI_CHK_NULL(avcSliceState->pAvcSliceParams);

        if (cmdBuffer == nullptr && batchBuffer == nullptr)
        {
            MHW_ASSERTMESSAGE("No valid buffer to add the command to!");
            return MOS_STATUS_INVALID_PARAMETER;
        }

        auto picParams = avcSliceState->pAvcPicParams;
        uint32_t mbaffMultiplier = 1;
        if (picParams->seq_fields.mb_adaptive_frame_field_flag &&
            !picParams->pic_fields.field_pic_flag)
        {
            mbaffMultiplier++;
        }

        uint16_t frameFieldHeightInMb = 0;
        CodecHal_GetFrameFieldHeightInMb(
            picParams->CurrPic,
            picParams->pic_height_in_mbs_minus1 + 1,
            frameFieldHeightInMb);

        auto sliceParams = avcSliceState->pAvcSliceParams;
        typename TMfxCmds::MFX_AVC_SLICE_STATE_CMD cmd;

        // Set MFX_AVC_SLICE_STATE_CMD
        cmd.DW1.SliceType = m_AvcBsdSliceType[sliceParams->slice_type];
        cmd.DW2.Log2WeightDenomChroma = sliceParams->chroma_log2_weight_denom;
        cmd.DW2.Log2WeightDenomLuma = sliceParams->luma_log2_weight_denom;
        cmd.DW3.WeightedPredictionIndicator = 0;
        cmd.DW3.DisableDeblockingFilterIndicator = avcSliceState->ucDisableDeblockingFilterIdc;
        cmd.DW3.CabacInitIdc10 = sliceParams->cabac_init_idc;
        cmd.DW3.SliceQuantizationParameter = 26 + picParams->pic_init_qp_minus26 + sliceParams->slice_qp_delta;
        cmd.DW3.SliceBetaOffsetDiv2 = avcSliceState->ucSliceBetaOffsetDiv2;
        cmd.DW3.SliceAlphaC0OffsetDiv2 = avcSliceState->ucSliceAlphaC0OffsetDiv2;

        auto widthInMb = picParams->pic_width_in_mbs_minus1 + 1;
        if (avcSliceState->bPhantomSlice)
        {
            cmd.DW4.SliceStartMbNum = widthInMb * frameFieldHeightInMb;
            cmd.DW4.SliceVerticalPosition = frameFieldHeightInMb;
            cmd.DW4.SliceHorizontalPosition = widthInMb;
        }
        else
        {
            cmd.DW4.SliceStartMbNum = sliceParams->first_mb_in_slice * mbaffMultiplier;
            cmd.DW4.SliceVerticalPosition = (sliceParams->first_mb_in_slice / widthInMb) * mbaffMultiplier;
            cmd.DW4.SliceHorizontalPosition = sliceParams->first_mb_in_slice % widthInMb;
        }

        if (avcSliceState->bLastSlice)
        {
            cmd.DW5.NextSliceVerticalPosition = frameFieldHeightInMb;
            cmd.DW5.NextSliceHorizontalPosition = 0;
        }
        else
        {
            cmd.DW5.NextSliceVerticalPosition = (sliceParams->first_mb_in_next_slice / widthInMb) * mbaffMultiplier;
            cmd.DW5.NextSliceHorizontalPosition = sliceParams->first_mb_in_next_slice % widthInMb;
        }

        cmd.DW6.IsLastSlice = avcSliceState->bLastSlice;

        cmd.DW9.Roundintra = 5;
        cmd.DW9.Roundintraenable = 1;
        cmd.DW9.Roundinter = 2;

        if (IsAvcPSlice(sliceParams->slice_type))
        {
            cmd.DW2.NumberOfReferencePicturesInInterPredictionList0 = sliceParams->num_ref_idx_l0_active_minus1 + 1;
            cmd.DW3.WeightedPredictionIndicator = picParams->pic_fields.weighted_pred_flag;
        }
        else if (IsAvcBSlice(sliceParams->slice_type))
        {
            cmd.DW2.NumberOfReferencePicturesInInterPredictionList1 = sliceParams->num_ref_idx_l1_active_minus1 + 1;
            cmd.DW2.NumberOfReferencePicturesInInterPredictionList0 = sliceParams->num_ref_idx_l0_active_minus1 + 1;
            cmd.DW3.WeightedPredictionIndicator = picParams->pic_fields.weighted_bipred_idc;
            cmd.DW3.DirectPredictionType = sliceParams->direct_spatial_mv_pred_flag;

            // Set MFX_AVC_WEIGHTOFFSET_STATE_CMD_G6
            if (picParams->pic_fields.weighted_bipred_idc != 1)
            {
                // luma/chroma_log2_weight_denoms need to be set to default value in the case of implicit mode
                cmd.DW2.Log2WeightDenomChroma = m_log2WeightDenomDefault;
                cmd.DW2.Log2WeightDenomLuma = m_log2WeightDenomDefault;
            }
        }

        MHW_MI_CHK_STATUS(Mhw_AddCommandCmdOrBB(cmdBuffer, batchBuffer, &cmd, sizeof(cmd)));

        return eStatus;
    }

    MOS_STATUS AddMfxEncodeAvcSlice(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMHW_BATCH_BUFFER batchBuffer,
        PMHW_VDBOX_AVC_SLICE_STATE avcSliceState)
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        MHW_FUNCTION_ENTER;

        MHW_MI_CHK_NULL(avcSliceState);
        MHW_MI_CHK_NULL(avcSliceState->pEncodeAvcSeqParams);
        MHW_MI_CHK_NULL(avcSliceState->pEncodeAvcPicParams);
        MHW_MI_CHK_NULL(avcSliceState->pEncodeAvcSliceParams);

        if (cmdBuffer == nullptr && batchBuffer == nullptr)
        {
            MHW_ASSERTMESSAGE("No valid buffer to add the command to!");
            return MOS_STATUS_INVALID_PARAMETER;
        }

        auto seqParams = avcSliceState->pEncodeAvcSeqParams;
        auto sliceParams = avcSliceState->pEncodeAvcSliceParams;
        auto picParams = avcSliceState->pEncodeAvcPicParams;

        uint16_t widthInMb = seqParams->pic_width_in_mbs_minus1 + 1;
        uint16_t frameFieldHeightInMb = avcSliceState->wFrameFieldHeightInMB;
        bool mbaffFrameFlag = seqParams->mb_adaptive_frame_field_flag ? true : false;
        uint32_t startMbNum = sliceParams->first_mb_in_slice * (1 + mbaffFrameFlag);

        typename TMfxCmds::MFX_AVC_SLICE_STATE_CMD cmd;

        //DW1
        cmd.DW1.SliceType = Slice_Type[sliceParams->slice_type];
        //DW2
        cmd.DW2.Log2WeightDenomLuma = sliceParams->luma_log2_weight_denom;
        cmd.DW2.Log2WeightDenomChroma = sliceParams->chroma_log2_weight_denom;
        cmd.DW2.NumberOfReferencePicturesInInterPredictionList0 = 0;
        cmd.DW2.NumberOfReferencePicturesInInterPredictionList1 = 0;
        //DW3
        cmd.DW3.SliceAlphaC0OffsetDiv2 = sliceParams->slice_alpha_c0_offset_div2;
        cmd.DW3.SliceBetaOffsetDiv2 = sliceParams->slice_beta_offset_div2;
        cmd.DW3.SliceQuantizationParameter = 26 + picParams->pic_init_qp_minus26 + sliceParams->slice_qp_delta;
        cmd.DW3.CabacInitIdc10 = sliceParams->cabac_init_idc;
        cmd.DW3.DisableDeblockingFilterIndicator = sliceParams->disable_deblocking_filter_idc;
        cmd.DW3.DirectPredictionType =
            IsAvcBSlice(sliceParams->slice_type) ? sliceParams->direct_spatial_mv_pred_flag : 0;
        cmd.DW3.WeightedPredictionIndicator = DEFAULT_WEIGHTED_INTER_PRED_MODE;
        //DW4
        cmd.DW4.SliceHorizontalPosition = startMbNum % widthInMb;
        cmd.DW4.SliceVerticalPosition = startMbNum / widthInMb;
        //DW5
        cmd.DW5.NextSliceHorizontalPosition = (startMbNum + sliceParams->NumMbsForSlice) % widthInMb;
        cmd.DW5.NextSliceVerticalPosition = (startMbNum + sliceParams->NumMbsForSlice) / widthInMb;
        //DW6
        cmd.DW6.StreamId10 = 0;
        cmd.DW6.SliceId30 = sliceParams->slice_id;
        cmd.DW6.Cabaczerowordinsertionenable = 1;
        cmd.DW6.Emulationbytesliceinsertenable = 1;
        cmd.DW6.IsLastSlice =
            (startMbNum + sliceParams->NumMbsForSlice) >= (uint32_t)(widthInMb * frameFieldHeightInMb);
        // Driver only programs 1st slice state, VDENC will detect the last slice
        if (avcSliceState->bVdencInUse)
        {
            cmd.DW6.TailInsertionPresentInBitstream = avcSliceState->bVdencNoTailInsertion ?
                0 : (picParams->bLastPicInSeq || picParams->bLastPicInStream);
        }
        else
        {
            cmd.DW6.TailInsertionPresentInBitstream = (picParams->bLastPicInSeq || picParams->bLastPicInStream) && cmd.DW6.IsLastSlice;
        }
        cmd.DW6.SlicedataInsertionPresentInBitstream = 1;
        cmd.DW6.HeaderInsertionPresentInBitstream = 1;
        cmd.DW6.MbTypeSkipConversionDisable = 0;
        cmd.DW6.MbTypeDirectConversionDisable = 0;
        cmd.DW6.RateControlCounterEnable = (avcSliceState->bBrcEnabled && (!avcSliceState->bFirstPass));

        if (cmd.DW6.RateControlCounterEnable == true)
        {
            // These fields are valid only when RateControlCounterEnable = 1
            cmd.DW6.RcPanicType = 1;    // CBP Panic
            cmd.DW6.RcPanicEnable =
                (avcSliceState->bRCPanicEnable &&
                (seqParams->RateControlMethod != RATECONTROL_AVBR) &&
                    (seqParams->RateControlMethod != RATECONTROL_IWD_VBR) &&
                    (seqParams->RateControlMethod != RATECONTROL_ICQ) &&
                    (seqParams->RateControlMethod != RATECONTROL_VCM) &&
                    (seqParams->RateControlMethod != RATECONTROL_CQP) &&
                    avcSliceState->bLastPass);    // Enable only in the last pass
            cmd.DW6.RcStableTolerance = 0;
            cmd.DW6.RcTriggleMode = 2;    // Loose Rate Control
            cmd.DW6.Resetratecontrolcounter = !startMbNum;
        }

        cmd.DW9.Roundinter = 2;

        if (IsAvcPSlice(sliceParams->slice_type))
        {
            cmd.DW2.NumberOfReferencePicturesInInterPredictionList0 = sliceParams->num_ref_idx_l0_active_minus1_from_DDI + 1;
            cmd.DW3.WeightedPredictionIndicator = picParams->weighted_pred_flag;

            cmd.DW9.Roundinterenable = avcSliceState->bRoundingInterEnable;
            cmd.DW9.Roundinter = avcSliceState->dwRoundingValue;
        }
        else if (IsAvcBSlice(sliceParams->slice_type))
        {
            cmd.DW2.NumberOfReferencePicturesInInterPredictionList1 = sliceParams->num_ref_idx_l1_active_minus1_from_DDI + 1;
            cmd.DW2.NumberOfReferencePicturesInInterPredictionList0 = sliceParams->num_ref_idx_l0_active_minus1_from_DDI + 1;
            cmd.DW3.WeightedPredictionIndicator = picParams->weighted_bipred_idc;
            if (picParams->weighted_bipred_idc == IMPLICIT_WEIGHTED_INTER_PRED_MODE)
            {
                if (avcSliceState->bVdencInUse)
                {
                    cmd.DW2.Log2WeightDenomLuma = 0;
                    cmd.DW2.Log2WeightDenomChroma = 0;
                }
                else
                {
                    // SNB requirement
                    cmd.DW2.Log2WeightDenomLuma = 5;
                    cmd.DW2.Log2WeightDenomChroma = 5;
                }
            }

            cmd.DW9.Roundinterenable = avcSliceState->bRoundingInterEnable;
            cmd.DW9.Roundinter = avcSliceState->dwRoundingValue;
        }

        cmd.DW9.Roundintra = avcSliceState->dwRoundingIntraValue;
        cmd.DW9.Roundintraenable = 1;

        MHW_MI_CHK_STATUS(Mhw_AddCommandCmdOrBB(cmdBuffer, batchBuffer, &cmd, sizeof(cmd)));

        return eStatus;
    }

    MOS_STATUS AddMfdAvcDpbCmd(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMHW_VDBOX_AVC_DPB_PARAMS params)
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        MHW_FUNCTION_ENTER;

        MHW_MI_CHK_NULL(cmdBuffer);
        MHW_MI_CHK_NULL(params);
        MHW_MI_CHK_NULL(params->pAvcPicParams);

        auto avcPicParams = params->pAvcPicParams;
        auto currFrameIdx = avcPicParams->CurrPic.FrameIdx;
        auto currAvcRefList = params->ppAvcRefList[currFrameIdx];

        int16_t refFrameOrder[CODEC_MAX_NUM_REF_FRAME] = { 0 };
        uint32_t usedForRef = 0;
        uint16_t nonExistingFrameFlags = 0;
        uint16_t longTermFrame = 0;

        for (uint8_t i = 0; i < currAvcRefList->ucNumRef; i++)
        {
            auto picIdx = currAvcRefList->RefList[i].FrameIdx;
            auto refAvcRefList = params->ppAvcRefList[picIdx];
            bool longTermFrameFlag = (currAvcRefList->RefList[i].PicFlags == PICTURE_LONG_TERM_REFERENCE);

            uint8_t frameID = params->bPicIdRemappingInUse ? i : refAvcRefList->ucFrameId;
            int16_t frameNum = refAvcRefList->sFrameNumber;

            refFrameOrder[frameID] = frameNum;
            usedForRef |= (((currAvcRefList->uiUsedForReferenceFlags >> (i * 2)) & 3) << (frameID * 2));
            nonExistingFrameFlags |= (((currAvcRefList->usNonExistingFrameFlags >> i) & 1) << frameID);
            longTermFrame |= (((uint16_t)longTermFrameFlag) << frameID);
        }

        typename TMfxCmds::MFD_AVC_DPB_STATE_CMD cmd;

        cmd.DW1.NonExistingframeFlag161Bit = nonExistingFrameFlags;
        cmd.DW1.LongtermframeFlag161Bit = longTermFrame;
        cmd.DW2.Value = usedForRef;

        for (auto i = 0, j = 0; i < 8; i++, j++)
        {
            cmd.Ltstframenumlist1616Bits[i] = (refFrameOrder[j++] & 0xFFFF); //FirstEntry
            cmd.Ltstframenumlist1616Bits[i] = cmd.Ltstframenumlist1616Bits[i] | ((refFrameOrder[j] & 0xFFFF) << 16);    //SecondEntry
        }

        auto mvcExtPicParams = params->pMvcExtPicParams;
        if (mvcExtPicParams)
        {
            for (auto i = 0, j = 0; i < (CODEC_MAX_NUM_REF_FRAME / 2); i++, j++)
            {
                cmd.Viewidlist1616Bits[i] = mvcExtPicParams->ViewIDList[j++];
                cmd.Viewidlist1616Bits[i] = cmd.Viewidlist1616Bits[i] | (mvcExtPicParams->ViewIDList[j] << 16);
            }

            for (auto i = 0, j = 0; i < (CODEC_MAX_NUM_REF_FRAME / 4); i++, j++)
            {
                cmd.Vieworderlistl0168Bits[i] = GetViewOrder(params, j++, LIST_0); //FirstEntry
                cmd.Vieworderlistl0168Bits[i] = cmd.Vieworderlistl0168Bits[i] | (GetViewOrder(params, j++, LIST_0) << 8);  //SecondEntry
                cmd.Vieworderlistl0168Bits[i] = cmd.Vieworderlistl0168Bits[i] | (GetViewOrder(params, j++, LIST_0) << 16); //ThirdEntry
                cmd.Vieworderlistl0168Bits[i] = cmd.Vieworderlistl0168Bits[i] | (GetViewOrder(params, j, LIST_0) << 24);   //FourthEntry
            }

            for (auto i = 0, j = 0; i < (CODEC_MAX_NUM_REF_FRAME / 4); i++, j++)
            {
                cmd.Vieworderlistl1168Bits[i] = GetViewOrder(params, j++, LIST_1); //FirstEntry
                cmd.Vieworderlistl1168Bits[i] = cmd.Vieworderlistl1168Bits[i] | (GetViewOrder(params, j++, LIST_1) << 8); //SecondEntry
                cmd.Vieworderlistl1168Bits[i] = cmd.Vieworderlistl1168Bits[i] | (GetViewOrder(params, j++, LIST_1) << 16); //ThirdEntry
                cmd.Vieworderlistl1168Bits[i] = cmd.Vieworderlistl1168Bits[i] | (GetViewOrder(params, j, LIST_1) << 24); //FourthEntry
            }
        }
        else
        {
            for (auto i = 0, j = 0; i < (CODEC_MAX_NUM_REF_FRAME / 2); i++, j++)
            {
                cmd.Viewidlist1616Bits[i] = 0;
            }

            for (auto i = 0, j = 0; i < (CODEC_MAX_NUM_REF_FRAME / 4); i++, j++)
            {
                cmd.Vieworderlistl0168Bits[i] = 0; //FirstEntry
            }

            for (auto i = 0, j = 0; i < (CODEC_MAX_NUM_REF_FRAME / 4); i++, j++)
            {
                cmd.Vieworderlistl1168Bits[i] = 0; //FirstEntry
            }
        }

        MHW_MI_CHK_STATUS(Mos_AddCommand(cmdBuffer, &cmd, sizeof(cmd)));

        return eStatus;
    }

    MOS_STATUS AddMfxAvcImgBrcBuffer(
        PMOS_RESOURCE brcImgBuffer,
        PMHW_VDBOX_AVC_IMG_PARAMS params)
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        MHW_FUNCTION_ENTER;

        MHW_MI_CHK_NULL(brcImgBuffer);
        MHW_MI_CHK_NULL(params);

        MOS_LOCK_PARAMS lockFlags;
        MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
        lockFlags.WriteOnly = 1;
        uint8_t *data = (uint8_t*)m_osInterface->pfnLockResource(m_osInterface, brcImgBuffer, &lockFlags);
        MHW_MI_CHK_NULL(data);

        MOS_COMMAND_BUFFER constructedCmdBuf;
        constructedCmdBuf.pCmdBase = (uint32_t *)data;
        constructedCmdBuf.pCmdPtr = (uint32_t *)data;
        constructedCmdBuf.iOffset = 0;
        constructedCmdBuf.iRemaining = BRC_IMG_STATE_SIZE_PER_PASS * m_numBrcPakPasses;

        MHW_MI_CHK_STATUS(AddMfxAvcImgCmd(&constructedCmdBuf, nullptr, params));

        typename TMfxCmds::MFX_AVC_IMG_STATE_CMD cmd = *(typename TMfxCmds::MFX_AVC_IMG_STATE_CMD *)data;

        for (uint32_t i = 0; i < m_numBrcPakPasses; i++)
        {
            if (i == 0)
            {
                cmd.DW4.Mbstatenabled =
                    cmd.DW5.Nonfirstpassflag = false;
            }
            else
            {
                cmd.DW4.Mbstatenabled = true;
                cmd.DW5.IntraIntermbipcmflagForceipcmcontrolmask = true;
                cmd.DW5.Nonfirstpassflag = true;
            }

            /* Setting the MbRateCtrlFlag to 0 so that the accumulative delta QP for consecutive passes is applied on top of
            the macroblock QP values in inline data. This is changed because the streamout QP behavior is causing a mismatch
            between the HW output and prototype output.*/
            cmd.DW5.MbratectrlflagMbLevelRateControlEnablingFlag = false;
            *(typename TMfxCmds::MFX_AVC_IMG_STATE_CMD *)data = cmd;

            /* add batch buffer end insertion flag */
            uint32_t* insertion = (uint32_t*)(data + (TMfxCmds::MFX_AVC_IMG_STATE_CMD::byteSize));
            *insertion = 0x05000000;

            data += BRC_IMG_STATE_SIZE_PER_PASS;
        }

        return eStatus;
    }

    MOS_STATUS AddMfxDecodeMpeg2PicCmd(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMHW_VDBOX_MPEG2_PIC_STATE params)
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        MHW_FUNCTION_ENTER;

        MHW_MI_CHK_NULL(cmdBuffer);
        MHW_MI_CHK_NULL(params);
        MHW_MI_CHK_NULL(params->pMpeg2PicParams);

        typename TMfxCmds::MFX_MPEG2_PIC_STATE_CMD cmd;
        auto picParams = params->pMpeg2PicParams;

        cmd.DW1.ScanOrder = picParams->W0.m_scanOrder;
        cmd.DW1.IntraVlcFormat = picParams->W0.m_intraVlcFormat;
        cmd.DW1.QuantizerScaleType = picParams->W0.m_quantizerScaleType;
        cmd.DW1.ConcealmentMotionVectorFlag = picParams->W0.m_concealmentMVFlag;
        cmd.DW1.FramePredictionFrameDct = picParams->W0.m_frameDctPrediction;
        cmd.DW1.TffTopFieldFirst = (CodecHal_PictureIsFrame(picParams->m_currPic)) ?
            picParams->W0.m_topFieldFirst : picParams->m_topFieldFirst;

        cmd.DW1.PictureStructure = (CodecHal_PictureIsFrame(picParams->m_currPic)) ?
            mpeg2Vc1Frame : (CodecHal_PictureIsTopField(picParams->m_currPic)) ?
            mpeg2Vc1TopField : mpeg2Vc1BottomField;
        cmd.DW1.IntraDcPrecision = picParams->W0.m_intraDCPrecision;
        cmd.DW1.FCode00 = picParams->W1.m_fcode00;
        cmd.DW1.FCode01 = picParams->W1.m_fcode01;
        cmd.DW1.FCode10 = picParams->W1.m_fcode10;
        cmd.DW1.FCode11 = picParams->W1.m_fcode11;

        cmd.DW2.PictureCodingType = picParams->m_pictureCodingType;

        if (params->Mode == CODECHAL_DECODE_MODE_MPEG2VLD)
        {
            cmd.DW2.ISliceConcealmentMode = params->dwMPEG2ISliceConcealmentMode;
            cmd.DW2.PBSliceConcealmentMode = params->dwMPEG2PBSliceConcealmentMode;
            cmd.DW2.PBSlicePredictedBidirMotionTypeOverrideBiDirectionMvTypeOverride = params->dwMPEG2PBSlicePredBiDirMVTypeOverride;
            cmd.DW2.PBSlicePredictedMotionVectorOverrideFinalMvValueOverride = params->dwMPEG2PBSlicePredMVOverride;

            cmd.DW3.SliceConcealmentDisableBit = 1;
        }

        uint16_t widthInMbs =
            (picParams->m_horizontalSize + CODECHAL_MACROBLOCK_WIDTH - 1) /
            CODECHAL_MACROBLOCK_WIDTH;

        uint16_t heightInMbs =
            (picParams->m_verticalSize + CODECHAL_MACROBLOCK_HEIGHT - 1) /
            CODECHAL_MACROBLOCK_HEIGHT;

        cmd.DW3.Framewidthinmbsminus170PictureWidthInMacroblocks = widthInMbs - 1;
        cmd.DW3.Frameheightinmbsminus170PictureHeightInMacroblocks = (CodecHal_PictureIsField(picParams->m_currPic)) ?
            ((heightInMbs * 2) - 1) : heightInMbs - 1;

        if (params->bDeblockingEnabled)
        {
            cmd.DW3.Reserved120 = 9;
        }

        cmd.DW4.Roundintradc = 3;
        cmd.DW4.Roundinterdc = 1;
        cmd.DW4.Roundintraac = 5;
        cmd.DW4.Roundinterac = 1;

        cmd.DW6.Intrambmaxsize = 0xfff;
        cmd.DW6.Intermbmaxsize = 0xfff;

        MHW_MI_CHK_STATUS(Mos_AddCommand(cmdBuffer, &cmd, sizeof(cmd)));

        return eStatus;
    }

    MOS_STATUS AddMfxEncodeMpeg2PicCmd(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMHW_VDBOX_MPEG2_PIC_STATE params)
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        MHW_FUNCTION_ENTER;

        MHW_MI_CHK_NULL(cmdBuffer);
        MHW_MI_CHK_NULL(params);
        MHW_MI_CHK_NULL(params->pEncodeMpeg2PicParams);

        typename TMfxCmds::MFX_MPEG2_PIC_STATE_CMD cmd;
        auto picParams = params->pEncodeMpeg2PicParams;

        cmd.DW1.ScanOrder = picParams->m_alternateScan;
        cmd.DW1.IntraVlcFormat = picParams->m_intraVlcFormat;
        cmd.DW1.QuantizerScaleType = picParams->m_qscaleType;
        cmd.DW1.ConcealmentMotionVectorFlag = picParams->m_concealmentMotionVectors;
        cmd.DW1.FramePredictionFrameDct = picParams->m_framePredFrameDCT;
        cmd.DW1.TffTopFieldFirst = !picParams->m_interleavedFieldBFF;
        cmd.DW1.PictureStructure = (CodecHal_PictureIsFrame(picParams->m_currOriginalPic)) ?
            mpeg2Vc1Frame : (CodecHal_PictureIsTopField(picParams->m_currOriginalPic)) ?
            mpeg2Vc1TopField : mpeg2Vc1BottomField;
        cmd.DW1.IntraDcPrecision = picParams->m_intraDCprecision;
        if (picParams->m_pictureCodingType == I_TYPE)
        {
            cmd.DW1.FCode00 = 0xf;
            cmd.DW1.FCode01 = 0xf;
        }
        else
        {
            cmd.DW1.FCode00 = picParams->m_fcode00;
            cmd.DW1.FCode01 = picParams->m_fcode01;
        }
        cmd.DW1.FCode10 = picParams->m_fcode10;
        cmd.DW1.FCode11 = picParams->m_fcode11;

        cmd.DW2.PictureCodingType = picParams->m_pictureCodingType;
        cmd.DW2.LoadslicepointerflagLoadbitstreampointerperslice = 0; // Do not reload bitstream pointer for each slice

        cmd.DW3.Framewidthinmbsminus170PictureWidthInMacroblocks = params->wPicWidthInMb - 1;
        cmd.DW3.Frameheightinmbsminus170PictureHeightInMacroblocks = params->wPicHeightInMb - 1;

        cmd.DW4.Roundintradc = 3;
        cmd.DW4.Roundinterdc = 1;
        cmd.DW4.Roundintraac = 5;
        cmd.DW4.Roundinterac = 1;
        cmd.DW4.Mbstatenabled = 0;

        cmd.DW5.Mbratecontrolmask = 0;
        cmd.DW5.Framesizecontrolmask = 0; // Disable first for PAK pass, used when MacroblockStatEnable is 1

        cmd.DW6.Intrambmaxsize = 0xfff;
        cmd.DW6.Intermbmaxsize = 0xfff;

        MHW_MI_CHK_STATUS(Mos_AddCommand(cmdBuffer, &cmd, sizeof(cmd)));

        return eStatus;
    }

    MOS_STATUS AddMfdMpeg2BsdObject(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMHW_BATCH_BUFFER batchBuffer,
        PMHW_VDBOX_MPEG2_SLICE_STATE params)
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        MHW_FUNCTION_ENTER;

        MHW_MI_CHK_NULL(params);
        MHW_MI_CHK_NULL(params->pMpeg2SliceParams);

        if (cmdBuffer == nullptr && batchBuffer == nullptr)
        {
            MHW_ASSERTMESSAGE("No valid buffer to add the command to!");
            return MOS_STATUS_INVALID_PARAMETER;
        }

        typename TMfxCmds::MFD_MPEG2_BSD_OBJECT_CMD  cmd;
        auto sliceParams = params->pMpeg2SliceParams;

        uint32_t endMb = params->dwSliceStartMbOffset + sliceParams->m_numMbsForSlice;
        uint32_t slcLocation = (uint32_t)(sliceParams->m_sliceDataOffset + params->dwOffset);

        cmd.DW1.IndirectBsdDataLength = params->dwLength;
        cmd.DW2.IndirectDataStartAddress = slcLocation;
        cmd.DW3.FirstMacroblockBitOffset = (sliceParams->m_macroblockOffset & 0x0007);

        cmd.DW3.IsLastMb = cmd.DW3.LastPicSlice = params->bLastSlice;
        cmd.DW3.Reserved100 =
            ((endMb / params->wPicWidthInMb) != sliceParams->m_sliceVerticalPosition) ? 1 : 0;

        cmd.DW3.MacroblockCount = sliceParams->m_numMbsForSlice;
        cmd.DW3.SliceHorizontalPosition = sliceParams->m_sliceHorizontalPosition;
        cmd.DW3.SliceVerticalPosition = sliceParams->m_sliceVerticalPosition;
        cmd.DW4.QuantizerScaleCode = sliceParams->m_quantiserScaleCode;

        if (cmd.DW3.IsLastMb)
        {
            cmd.DW4.NextSliceHorizontalPosition = 0;
            cmd.DW4.NextSliceVerticalPosition = params->wPicHeightInMb;
        }
        else
        {
            cmd.DW4.NextSliceHorizontalPosition = endMb % params->wPicWidthInMb;
            cmd.DW4.NextSliceVerticalPosition = endMb / params->wPicWidthInMb;
        }

        uint32_t offset = ((sliceParams->m_macroblockOffset & 0x0000fff8) >> 3); // #of bytes of header data in bitstream buffer (before video data)

        MHW_CP_SLICE_INFO_PARAMS sliceInfoParam;
        sliceInfoParam.presDataBuffer = params->presDataBuffer;
        sliceInfoParam.dwDataStartOffset[0] = sliceParams->m_sliceDataOffset + offset;

        MHW_MI_CHK_STATUS(m_cpInterface->SetMfxProtectionState(
            m_decodeInUse,
            cmdBuffer,
            batchBuffer,
            &sliceInfoParam));

        MHW_MI_CHK_STATUS(Mhw_AddCommandCmdOrBB(cmdBuffer, batchBuffer, &cmd, sizeof(cmd)));

        return eStatus;
    }

    //!
    //! \struct   MFD_MPEG2_IT_OBJECT_CMD
    //! \brief    MFD MPEG2 it object command
    //!
    struct MFD_MPEG2_IT_OBJECT_CMD
    {
        typename TMfxCmds::MFD_IT_OBJECT_CMD m_header;
        typename TMfxCmds::MFD_IT_OBJECT_MPEG2_INLINE_DATA_CMD m_inlineData;
    };

    MOS_STATUS AddMfdMpeg2ITObject(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMHW_BATCH_BUFFER batchBuffer,
        PMHW_VDBOX_MPEG2_MB_STATE params)
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        MHW_FUNCTION_ENTER;

        MHW_MI_CHK_NULL(params);

        if (cmdBuffer == nullptr && batchBuffer == nullptr)
        {
            MHW_ASSERTMESSAGE("No valid buffer to add the command to!");
            return MOS_STATUS_INVALID_PARAMETER;
        }

        MFD_MPEG2_IT_OBJECT_CMD cmd;
        cmd.m_inlineData.DW0.MacroblockIntraType = mpeg2Vc1MacroblockIntra;

        typename TMfxCmds::MFD_IT_OBJECT_MPEG2_INLINE_DATA_CMD *inlineDataMpeg2 = &(cmd.m_inlineData);
        typename TMfxCmds::MFD_IT_OBJECT_CMD *cmdMfdItObject = &(cmd.m_header);

        //------------------------------------
        // Shared indirect data
        //------------------------------------
        cmdMfdItObject->DW0.DwordLength += TMfxCmds::MFD_IT_OBJECT_MPEG2_INLINE_DATA_CMD::dwSize;

        cmdMfdItObject->DW3.IndirectItCoeffDataLength = (params->dwDCTLength) << 2;
        cmdMfdItObject->DW4.IndirectItCoeffDataStartAddressOffset = params->dwITCoffDataAddrOffset;

        //------------------------------------
        // Shared inline data
        //------------------------------------
        auto mbParams = params->pMBParams;
        inlineDataMpeg2->DW0.DctType = mbParams->MBType.m_fieldResidual;
        inlineDataMpeg2->DW0.CodedBlockPattern = mbParams->m_codedBlockPattern;
        inlineDataMpeg2->DW1.Horzorigin = mbParams->m_mbAddr % params->wPicWidthInMb;
        inlineDataMpeg2->DW1.Vertorigin = mbParams->m_mbAddr / params->wPicWidthInMb;
        inlineDataMpeg2->DW0.Lastmbinrow = (inlineDataMpeg2->DW1.Horzorigin == (params->wPicWidthInMb - 1));

        if (params->wPicCodingType != I_TYPE)
        {
            inlineDataMpeg2->DW0.MacroblockIntraType = mbParams->MBType.m_intraMb;
            inlineDataMpeg2->DW0.MacroblockMotionForward = mbParams->MBType.m_motionFwd;
            inlineDataMpeg2->DW0.MacroblockMotionBackward = mbParams->MBType.m_motionBwd;
            inlineDataMpeg2->DW0.MotionType = mbParams->MBType.m_motionType;
            inlineDataMpeg2->DW0.MotionVerticalFieldSelect = mbParams->MBType.m_mvertFieldSel;

            // Next, copy in the motion vectors
            if (mbParams->MBType.m_intraMb == 0)
            {
                uint32_t *point = (uint32_t*)(params->sPackedMVs0);
                inlineDataMpeg2->DW2.Value = *point++;
                inlineDataMpeg2->DW3.Value = *point++;

                point = (uint32_t*)(params->sPackedMVs1);
                inlineDataMpeg2->DW4.Value = *point++;
                inlineDataMpeg2->DW5.Value = *point++;
            }
        }

        MHW_MI_CHK_STATUS(Mhw_AddCommandCmdOrBB(cmdBuffer, batchBuffer, &cmd, sizeof(cmd)));

        return eStatus;
    }

    MOS_STATUS AddMfcMpeg2SliceGroupCmd(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMHW_VDBOX_MPEG2_SLICE_STATE mpeg2SliceState)
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        MHW_FUNCTION_ENTER;

        MHW_MI_CHK_NULL(cmdBuffer);
        MHW_MI_CHK_NULL(mpeg2SliceState);
        MHW_MI_CHK_NULL(mpeg2SliceState->pEncodeMpeg2PicParams);
        MHW_MI_CHK_NULL(mpeg2SliceState->pEncodeMpeg2SliceParams);
        MHW_MI_CHK_NULL(mpeg2SliceState->pSlcData);

        auto sliceParams = mpeg2SliceState->pEncodeMpeg2SliceParams;
        auto picParams = mpeg2SliceState->pEncodeMpeg2PicParams;
        auto seqParams = mpeg2SliceState->pEncodeMpeg2SeqParams;
        auto slcData = mpeg2SliceState->pSlcData;

        typename TMfxCmds::MFC_MPEG2_SLICEGROUP_STATE_CMD cmd;

        cmd.DW1.Streamid10EncoderOnly = 0;
        cmd.DW1.Sliceid30EncoderOnly = 0;
        cmd.DW1.Intrasliceflag = 1;
        cmd.DW1.Intraslice = sliceParams->m_intraSlice;
        cmd.DW1.Firstslicehdrdisabled = 0;
        cmd.DW1.TailpresentflagTailInsertionPresentInBitstreamEncoderOnly =
            (picParams->m_lastPicInStream && (slcData->SliceGroup & SLICE_GROUP_LAST));
        cmd.DW1.SlicedataPresentflagSlicedataInsertionPresentInBitstreamEncoderOnly = 1;
        cmd.DW1.HeaderpresentflagHeaderInsertionPresentInBitstreamEncoderOnly = 1;
        cmd.DW1.BitstreamoutputflagCompressedBitstreamOutputDisableFlagEncoderOnly = 0;
        cmd.DW1.Islastslicegrp = (slcData->SliceGroup & SLICE_GROUP_LAST) ? 1 : 0;
        cmd.DW1.SkipconvdisabledMbTypeSkipConversionDisableEncoderOnly = sliceParams->m_intraSlice; // Disable for I slice

        cmd.DW1.MbratectrlflagRatecontrolcounterenableEncoderOnly = (mpeg2SliceState->bBrcEnabled && (!mpeg2SliceState->bFirstPass));
        cmd.DW1.MbratectrlresetResetratecontrolcounterEncoderOnly = 1;
        cmd.DW1.RatectrlpanictypeRcPanicTypeEncoderOnly = 1; // CBP type
        cmd.DW1.MbratectrlmodeRcTriggleModeEncoderOnly = 2; // Loose Rate Control Mode
        cmd.DW1.RatectrlpanicflagRcPanicEnableEncoderOnly =
            (mpeg2SliceState->bRCPanicEnable &&
            (seqParams->m_rateControlMethod != RATECONTROL_AVBR) &&
                (seqParams->m_rateControlMethod != RATECONTROL_IWD_VBR) &&
                (seqParams->m_rateControlMethod != RATECONTROL_ICQ) &&
                (seqParams->m_rateControlMethod != RATECONTROL_VCM) &&
                (seqParams->m_rateControlMethod != RATECONTROL_CQP) &&
                mpeg2SliceState->bLastPass);    // Enable only in the last pass

        cmd.DW2.FirstmbxcntAlsoCurrstarthorzpos = sliceParams->m_firstMbX;
        cmd.DW2.FirstmbycntAlsoCurrstartvertpos = sliceParams->m_firstMbY;
        cmd.DW2.NextsgmbxcntAlsoNextstarthorzpos = slcData->NextSgMbXCnt;
        cmd.DW2.NextsgmbycntAlsoNextstartvertpos = slcData->NextSgMbYCnt;

        cmd.DW3.Slicegroupqp = sliceParams->m_quantiserScaleCode;
        cmd.DW3.Slicegroupskip = 0; // MBZ for MPEG2

        // H/W should use this start addr only for the first slice, since LoadSlicePointerFlag = 0 in PIC_STATE
        cmd.DW4.BitstreamoffsetIndirectPakBseDataStartAddressWrite = 0;

        MHW_MI_CHK_STATUS(Mos_AddCommand(cmdBuffer, &cmd, sizeof(cmd)));

        return eStatus;
    }

    MOS_STATUS AddMfcMpeg2PakInsertBrcBuffer(
        PMOS_RESOURCE brcPicHeaderInputBuffer,
        PMHW_VDBOX_PAK_INSERT_PARAMS params)
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        MHW_FUNCTION_ENTER;

        MHW_MI_CHK_NULL(brcPicHeaderInputBuffer);
        MHW_MI_CHK_NULL(params);
        MHW_MI_CHK_NULL(params->pBsBuffer);

        typename TMfxCmds::MFX_PAK_INSERT_OBJECT_CMD cmd;

        uint32_t byteSize = (params->pBsBuffer->BitSize + 7) >> 3;
        uint32_t dataBitsInLastDw = params->pBsBuffer->BitSize % 32;
        if (dataBitsInLastDw == 0)
        {
            dataBitsInLastDw = 32;
        }

        uint32_t dwordsUsed = TMfxCmds::MFX_PAK_INSERT_OBJECT_CMD::dwSize + ((byteSize + 3) >> 2);
        cmd.DW0.DwordLength = OP_LENGTH(dwordsUsed);
        cmd.DW1.BitstreamstartresetResetbitstreamstartingpos = 0;
        cmd.DW1.EndofsliceflagLastdstdatainsertcommandflag = 0;
        cmd.DW1.LastheaderflagLastsrcheaderdatainsertcommandflag = 1;
        cmd.DW1.EmulationflagEmulationbytebitsinsertenable = 0;
        cmd.DW1.SkipemulbytecntSkipEmulationByteCount = 0;
        cmd.DW1.DatabitsinlastdwSrcdataendingbitinclusion50 = dataBitsInLastDw;
        cmd.DW1.DatabyteoffsetSrcdatastartingbyteoffset10 = 0;

        MOS_LOCK_PARAMS lockFlags;
        MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
        lockFlags.WriteOnly = 1;
        uint8_t* data = (uint8_t*)m_osInterface->pfnLockResource(
            m_osInterface,
            brcPicHeaderInputBuffer,
            &lockFlags);
        MHW_MI_CHK_NULL(data);

        eStatus = MOS_SecureMemcpy(data, TMfxCmds::MFX_PAK_INSERT_OBJECT_CMD::byteSize, &cmd, TMfxCmds::MFX_PAK_INSERT_OBJECT_CMD::byteSize);
        if (eStatus != MOS_STATUS_SUCCESS)
        {
            MHW_ASSERTMESSAGE("Failed to copy memory.");
            return eStatus;
        }

        // Use the exact data byte size to make sure that we don't overrun the bit buffer.
        eStatus = MOS_SecureMemcpy(data + TMfxCmds::MFX_PAK_INSERT_OBJECT_CMD::byteSize, byteSize, params->pBsBuffer->pBase, byteSize);
        if (eStatus != MOS_STATUS_SUCCESS)
        {
            MHW_ASSERTMESSAGE("Failed to copy memory.");
            return eStatus;
        }

        // Need to make sure that the batch buffer end command begins on a dword boundary. So use
        // a dword aligned data size in the offset calculation instead of the straight byte size.
        // Note: The variable dwDwordsUsed already contains the size of the INSERT command.
        typename TMiCmds::MI_BATCH_BUFFER_END_CMD cmdMiBatchBufferEnd;
        eStatus = MOS_SecureMemcpy(data + sizeof(uint32_t)*dwordsUsed,
            sizeof(cmdMiBatchBufferEnd),
            &cmdMiBatchBufferEnd,
            cmdMiBatchBufferEnd.byteSize);
        if (eStatus != MOS_STATUS_SUCCESS)
        {
            MHW_ASSERTMESSAGE("Failed to copy memory.");
            return eStatus;
        }

        MHW_MI_CHK_STATUS(m_osInterface->pfnUnlockResource(m_osInterface, brcPicHeaderInputBuffer));

        *(params->pdwMpeg2PicHeaderTotalBufferSize) = sizeof(uint32_t)* dwordsUsed +
            cmdMiBatchBufferEnd.byteSize;
        *(params->pdwMpeg2PicHeaderDataStartOffset) = TMfxCmds::MFX_PAK_INSERT_OBJECT_CMD::byteSize;

        return eStatus;
    }

    MOS_STATUS AddMfxMpeg2PicBrcBuffer(
        PMOS_RESOURCE brcImgBuffer,
        PMHW_VDBOX_MPEG2_PIC_STATE params)
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        MHW_FUNCTION_ENTER;

        MHW_MI_CHK_NULL(brcImgBuffer);
        MHW_MI_CHK_NULL(params);

        MOS_LOCK_PARAMS lockFlags;
        MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
        lockFlags.WriteOnly = 1;
        uint8_t *data = (uint8_t*)m_osInterface->pfnLockResource(m_osInterface, brcImgBuffer, &lockFlags);
        MHW_MI_CHK_NULL(data);

        MOS_COMMAND_BUFFER constructedCmdBuf;
        constructedCmdBuf.pCmdBase = (uint32_t *)data;
        constructedCmdBuf.pCmdPtr = (uint32_t *)data;
        constructedCmdBuf.iOffset = 0;
        constructedCmdBuf.iRemaining = BRC_IMG_STATE_SIZE_PER_PASS * m_numBrcPakPasses;

        MHW_MI_CHK_STATUS(AddMfxMpeg2PicCmd(&constructedCmdBuf, params));

        typename TMfxCmds::MFX_MPEG2_PIC_STATE_CMD cmd = *(typename TMfxCmds::MFX_MPEG2_PIC_STATE_CMD *)data;

        for (uint32_t i = 0; i < m_numBrcPakPasses; i++)
        {
            cmd.DW5.Framebitratemaxreportmask = 1;
            cmd.DW5.Framebitrateminreportmask = 1;

            if (i == 0)
            {
                cmd.DW4.Mbstatenabled = 0; // Disable for first PAK pass
                cmd.DW5.Mbratecontrolmask = 0;
                cmd.DW5.Framesizecontrolmask = 0; // Disable first for PAK pass
            }
            else
            {
                cmd.DW4.Mbstatenabled = 1; // Disable for first PAK pass
                cmd.DW5.Mbratecontrolmask = 1;
                cmd.DW5.Framesizecontrolmask = 1;
            }

            cmd.DW8.Value = m_mpeg2SliceDeltaQPMax[i];
            cmd.DW9.Value = m_mpeg2InitSliceDeltaQPMin[i];
            cmd.DW10.Value = m_mpeg2FrameBitrateMinMax[i];
            cmd.DW11.Value = m_mpeg2FrameBitrateMinMaxDelta[i];

            *(typename TMfxCmds::MFX_MPEG2_PIC_STATE_CMD *)data = cmd;
            data += BRC_IMG_STATE_SIZE_PER_PASS;
        }

        return eStatus;
    }

    MOS_STATUS AddMfxVc1PredPipeCmd(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMHW_VDBOX_VC1_PRED_PIPE_PARAMS params)
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        MHW_FUNCTION_ENTER;

        MHW_MI_CHK_NULL(cmdBuffer);
        MHW_MI_CHK_NULL(params);
        MHW_MI_CHK_NULL(params->pVc1PicParams);

        auto vc1PicParams = params->pVc1PicParams;
        auto destParams = params->ppVc1RefList[vc1PicParams->CurrPic.FrameIdx];
        auto fwdRefParams = params->ppVc1RefList[vc1PicParams->ForwardRefIdx];
        auto bwdRefParams = params->ppVc1RefList[vc1PicParams->BackwardRefIdx];

        bool isPPicture = IsVc1PPicture(
            vc1PicParams->CurrPic,
            vc1PicParams->picture_fields.is_first_field,
            vc1PicParams->picture_fields.picture_type);

        bool isBPicture = IsVc1BPicture(
            vc1PicParams->CurrPic,
            vc1PicParams->picture_fields.is_first_field,
            vc1PicParams->picture_fields.picture_type);

        bool isSecondField = !vc1PicParams->picture_fields.is_first_field;

        RefBoundaryReplicationMode refBoundaryReplicationMode;
        refBoundaryReplicationMode.BY0.value = 0;

        if (isPPicture || isBPicture)
        {
            if (fwdRefParams->dwRefSurfaceFlags & CODECHAL_VC1_PROGRESSIVE)
            {
                if (!vc1PicParams->picture_fields.is_first_field)
                {
                    if (vc1PicParams->picture_fields.top_field_first)
                    {
                        refBoundaryReplicationMode.BY0.ref0 = vc1InterlacedBoundary;
                        refBoundaryReplicationMode.BY0.ref2 = vc1ProgressiveBoundary;
                    }
                    else
                    {
                        refBoundaryReplicationMode.BY0.ref2 = vc1InterlacedBoundary;
                        refBoundaryReplicationMode.BY0.ref0 = vc1ProgressiveBoundary;
                    }
                }
                else
                {
                    refBoundaryReplicationMode.BY0.ref0 = vc1ProgressiveBoundary;
                    refBoundaryReplicationMode.BY0.ref2 = vc1ProgressiveBoundary;
                }
            }
            else
            {
                refBoundaryReplicationMode.BY0.ref0 = refBoundaryReplicationMode.BY0.ref2 = vc1InterlacedBoundary;
            }
        }
        if (isBPicture)
        {
            if (bwdRefParams->dwRefSurfaceFlags & CODECHAL_VC1_PROGRESSIVE)
            {
                refBoundaryReplicationMode.BY0.ref1 = refBoundaryReplicationMode.BY0.ref3 = vc1ProgressiveBoundary;
            }
            else
            {
                refBoundaryReplicationMode.BY0.ref1 = refBoundaryReplicationMode.BY0.ref3 = vc1InterlacedBoundary;
            }
        }

        typename TMfxCmds::MFX_VC1_PRED_PIPE_STATE_CMD cmd;
        cmd.DW1.ReferenceFrameBoundaryReplicationMode = refBoundaryReplicationMode.BY0.value;

        uint32_t fwdDoubleIcEnable = 0, fwdSingleIcEnable = 0;
        uint32_t bwdDoubleIcEnable = 0, bwdSingleIcEnable = 0;
        uint8_t icField = 0;

        // Interlaced Frame & Frame
        if (!CodecHal_PictureIsField(vc1PicParams->CurrPic))
        {
            if (isPPicture)
            {
                if (vc1PicParams->picture_fields.intensity_compensation)
                {
                    if (MOS_IS_BIT_SET(fwdRefParams->dwRefSurfaceFlags, CODECHAL_VC1_TOP_FIELD_COMP))
                    {
                        fwdDoubleIcEnable = TOP_FIELD;
                        cmd.DW3.Lumscale1DoubleFwd = fwdRefParams->Vc1IcValues[icField].wICCScale1;
                        cmd.DW3.Lumshift1DoubleFwd = fwdRefParams->Vc1IcValues[icField].wICCShiftL1;
                        // IC values for the bottom out of bound pixels (replicated lines of the last
                        // line of top field)
                        cmd.DW3.Lumscale2DoubleFwd = fwdRefParams->Vc1IcValues[icField].wICCScale1;
                        cmd.DW3.Lumshift2DoubleFwd = fwdRefParams->Vc1IcValues[icField].wICCShiftL1;

                        MOS_BIT_ON(fwdRefParams->dwRefSurfaceFlags, CODECHAL_VC1_TOP_FIELD_COMP_2);
                        icField++;
                    }
                    else if (MOS_IS_BIT_SET(fwdRefParams->dwRefSurfaceFlags, CODECHAL_VC1_BOT_FIELD_COMP))
                    {
                        fwdDoubleIcEnable = BOTTOM_FIELD;
                        // IC values for the top out of bound pixels (replicated lines of the first
                        // line of bottom field)
                        cmd.DW3.Lumscale1DoubleFwd = fwdRefParams->Vc1IcValues[icField].wICCScale2;
                        cmd.DW3.Lumshift1DoubleFwd = fwdRefParams->Vc1IcValues[icField].wICCShiftL2;
                        cmd.DW3.Lumscale2DoubleFwd = fwdRefParams->Vc1IcValues[icField].wICCScale2;
                        cmd.DW3.Lumshift2DoubleFwd = fwdRefParams->Vc1IcValues[icField].wICCShiftL2;

                        MOS_BIT_ON(fwdRefParams->dwRefSurfaceFlags, CODECHAL_VC1_BOT_FIELD_COMP_2);
                        icField++;
                    }

                    uint16_t lumaScale = vc1PicParams->luma_scale;
                    uint16_t lumaShift = vc1PicParams->luma_shift;

                    MOS_BIT_ON(fwdRefParams->dwRefSurfaceFlags, CODECHAL_VC1_FRAME_COMP);

                    fwdSingleIcEnable = TOP_FIELD | BOTTOM_FIELD;
                    cmd.DW2.Lumscale1SingleFwd = lumaScale;
                    cmd.DW2.Lumshift1SingleFwd = lumaShift;

                    cmd.DW2.Lumscale2SingleFwd = lumaScale;
                    cmd.DW2.Lumshift2SingleFwd = lumaShift;

                    // Set double backward values for top and bottom out of bound pixels
                    bwdDoubleIcEnable = TOP_FIELD | BOTTOM_FIELD;
                    cmd.DW5.Lumscale1DoubleBwd = lumaScale;
                    cmd.DW5.Lumshift1DoubleBwd = lumaShift;
                    cmd.DW5.Lumscale2DoubleBwd = lumaScale;
                    cmd.DW5.Lumshift2DoubleBwd = lumaShift;

                    // Save IC
                    fwdRefParams->Vc1IcValues[icField].wICCScale1 =
                        fwdRefParams->Vc1IcValues[icField].wICCScale2 = lumaScale;
                    fwdRefParams->Vc1IcValues[icField].wICCShiftL1 =
                        fwdRefParams->Vc1IcValues[icField].wICCShiftL2 = lumaShift;
                }
                else
                {
                    // special case for interlaced field references when no IC is indicated
                    if (MOS_IS_BIT_SET(fwdRefParams->dwRefSurfaceFlags, CODECHAL_VC1_TOP_FIELD_COMP))
                    {
                        cmd.DW2.Lumscale1SingleFwd = fwdRefParams->Vc1IcValues[icField].wICCScale1;
                        cmd.DW2.Lumshift1SingleFwd = fwdRefParams->Vc1IcValues[icField].wICCShiftL1;

                        fwdSingleIcEnable = TOP_FIELD;
                        icField++;
                    }

                    if (MOS_IS_BIT_SET(fwdRefParams->dwRefSurfaceFlags, CODECHAL_VC1_BOT_FIELD_COMP))
                    {
                        cmd.DW2.Lumscale2SingleFwd = fwdRefParams->Vc1IcValues[icField].wICCScale2;
                        cmd.DW2.Lumshift2SingleFwd = fwdRefParams->Vc1IcValues[icField].wICCShiftL2;

                        fwdSingleIcEnable |= BOTTOM_FIELD;
                        icField++;
                    }

                }
            }
            else if (isBPicture)
            {
                // Forward reference IC
                if (MOS_IS_BIT_SET(fwdRefParams->dwRefSurfaceFlags, CODECHAL_VC1_TOP_FIELD_COMP_2))
                {
                    fwdDoubleIcEnable = TOP_FIELD;
                    cmd.DW3.Lumscale1DoubleFwd = fwdRefParams->Vc1IcValues[icField].wICCScale1;
                    cmd.DW3.Lumshift1DoubleFwd = fwdRefParams->Vc1IcValues[icField].wICCShiftL1;
                    // IC values for the bottom out of bound pixels (replicated lines of the last
                    // line of top field)
                    cmd.DW3.Lumscale2DoubleFwd = fwdRefParams->Vc1IcValues[icField].wICCScale1;
                    cmd.DW3.Lumshift2DoubleFwd = fwdRefParams->Vc1IcValues[icField].wICCShiftL1;
                }
                if (MOS_IS_BIT_SET(fwdRefParams->dwRefSurfaceFlags, CODECHAL_VC1_BOT_FIELD_COMP_2))
                {
                    fwdDoubleIcEnable |= BOTTOM_FIELD;
                    // IC values for the top out of bound pixels (replicated lines of the first
                    // line of bottom field)
                    cmd.DW3.Lumscale1DoubleFwd = fwdRefParams->Vc1IcValues[icField].wICCScale2;
                    cmd.DW3.Lumshift1DoubleFwd = fwdRefParams->Vc1IcValues[icField].wICCShiftL2;
                    cmd.DW3.Lumscale2DoubleFwd = fwdRefParams->Vc1IcValues[icField].wICCScale2;
                    cmd.DW3.Lumshift2DoubleFwd = fwdRefParams->Vc1IcValues[icField].wICCShiftL2;
                }
                if (fwdDoubleIcEnable)
                {
                    icField++;
                }

                if (MOS_IS_BIT_SET(fwdRefParams->dwRefSurfaceFlags, CODECHAL_VC1_TOP_FIELD_COMP))
                {
                    fwdSingleIcEnable = TOP_FIELD;
                    cmd.DW2.Lumscale1SingleFwd = fwdRefParams->Vc1IcValues[icField].wICCScale1;
                    cmd.DW2.Lumshift1SingleFwd = fwdRefParams->Vc1IcValues[icField].wICCShiftL1;
                }
                if (MOS_IS_BIT_SET(fwdRefParams->dwRefSurfaceFlags, CODECHAL_VC1_BOT_FIELD_COMP))
                {
                    fwdSingleIcEnable |= BOTTOM_FIELD;
                    cmd.DW2.Lumscale2SingleFwd = fwdRefParams->Vc1IcValues[icField].wICCScale2;
                    cmd.DW2.Lumshift2SingleFwd = fwdRefParams->Vc1IcValues[icField].wICCShiftL2;
                }

                // If the reference picture is interlaced field, set double backward values for top
                // and bottom out of bound pixels
                if (fwdSingleIcEnable == (TOP_FIELD | BOTTOM_FIELD))
                {
                    bwdDoubleIcEnable = TOP_FIELD | BOTTOM_FIELD;
                    cmd.DW5.Lumscale1DoubleBwd = fwdRefParams->Vc1IcValues[icField].wICCScale1;
                    cmd.DW5.Lumshift1DoubleBwd = fwdRefParams->Vc1IcValues[icField].wICCShiftL1;
                    cmd.DW5.Lumscale2DoubleBwd = fwdRefParams->Vc1IcValues[icField].wICCScale2;
                    cmd.DW5.Lumshift2DoubleBwd = fwdRefParams->Vc1IcValues[icField].wICCShiftL2;
                }

                // Backward reference IC
                icField = 0;
                if (MOS_IS_BIT_SET(bwdRefParams->dwRefSurfaceFlags, CODECHAL_VC1_TOP_FIELD_COMP))
                {
                    bwdSingleIcEnable = TOP_FIELD;
                    cmd.DW4.Lumscale1SingleBwd = bwdRefParams->Vc1IcValues[icField].wICCScale1;
                    cmd.DW4.Lumshift1SingleBwd = bwdRefParams->Vc1IcValues[icField].wICCShiftL1;
                }
                else if (MOS_IS_BIT_SET(bwdRefParams->dwRefSurfaceFlags, CODECHAL_VC1_BOT_FIELD_COMP))
                {
                    bwdSingleIcEnable = BOTTOM_FIELD;
                    cmd.DW4.Lumscale2SingleBwd = bwdRefParams->Vc1IcValues[icField].wICCScale2;
                    cmd.DW4.Lumshift2SingleBwd = bwdRefParams->Vc1IcValues[icField].wICCShiftL2;
                }
            }
        }
        // Interlace field
        else
        {
            if (isPPicture)
            {
                fwdSingleIcEnable =
                    vc1PicParams->picture_fields.intensity_compensation ? (TOP_FIELD | BOTTOM_FIELD) : 0;

                // Top field IC
                uint16_t lumaScale = vc1PicParams->luma_scale >> 8;
                uint16_t lumaShift = vc1PicParams->luma_shift >> 8;

                if (CodecHal_PictureIsBottomField(vc1PicParams->CurrPic) && isSecondField)
                {
                    fwdRefParams = destParams;
                }

                if (((lumaScale == 32) && (lumaShift == 0)) || !(fwdSingleIcEnable & TOP_FIELD))
                {
                    // No IC for top field
                    if (MOS_IS_BIT_SET(fwdRefParams->dwRefSurfaceFlags, CODECHAL_VC1_TOP_FIELD_COMP))
                    {
                        cmd.DW2.Lumscale1SingleFwd = fwdRefParams->Vc1IcValues[icField].wICCScale1;
                        cmd.DW2.Lumshift1SingleFwd = fwdRefParams->Vc1IcValues[icField].wICCShiftL1;
                        fwdSingleIcEnable |= TOP_FIELD;
                    }
                    else
                    {
                        fwdSingleIcEnable &= BOTTOM_FIELD;
                    }
                }
                else
                {
                    // IC for top field is enabled
                    cmd.DW2.Lumscale1SingleFwd = lumaScale;
                    cmd.DW2.Lumshift1SingleFwd = lumaShift;

                    if (MOS_IS_BIT_SET(fwdRefParams->dwRefSurfaceFlags, CODECHAL_VC1_TOP_FIELD_COMP))
                    {
                        fwdDoubleIcEnable = TOP_FIELD;
                        cmd.DW3.Lumscale1DoubleFwd = fwdRefParams->Vc1IcValues[icField].wICCScale1;
                        cmd.DW3.Lumshift1DoubleFwd = fwdRefParams->Vc1IcValues[icField].wICCShiftL1;

                        MOS_BIT_ON(fwdRefParams->dwRefSurfaceFlags, CODECHAL_VC1_TOP_FIELD_COMP_2);
                        icField++;
                    }
                    else
                    {
                        MOS_BIT_ON(fwdRefParams->dwRefSurfaceFlags, CODECHAL_VC1_TOP_FIELD_COMP);
                    }

                    // set double backward values for top out of bound pixels
                    if (vc1PicParams->picture_fields.intensity_compensation)
                    {
                        bwdDoubleIcEnable = TOP_FIELD;
                        // If the reference picture is not interlaced field and current picture is bottom field
                        // and second field, double backwards values are identical to the the single forward
                        // values used by the p interlaced field top field
                        if (!CodecHal_PictureIsField((params->ppVc1RefList[vc1PicParams->ForwardRefIdx])->RefPic) &&
                            (CodecHal_PictureIsBottomField(vc1PicParams->CurrPic) && isSecondField))
                        {
                            cmd.DW5.Lumscale1DoubleBwd =
                                (params->ppVc1RefList[vc1PicParams->ForwardRefIdx])->Vc1IcValues[icField].wICCScale1;
                            cmd.DW5.Lumshift1DoubleBwd =
                                (params->ppVc1RefList[vc1PicParams->ForwardRefIdx])->Vc1IcValues[icField].wICCShiftL1;
                        }
                        else
                        {
                            cmd.DW5.Lumscale1DoubleBwd = lumaScale;
                            cmd.DW5.Lumshift1DoubleBwd = lumaShift;
                        }
                    }

                    // Save IC
                    fwdRefParams->Vc1IcValues[icField].wICCScale1 = lumaScale;
                    fwdRefParams->Vc1IcValues[icField].wICCShiftL1 = lumaShift;
                }

                // Bottom field IC
                icField = 0;
                lumaScale = vc1PicParams->luma_scale & 0x00ff;
                lumaShift = vc1PicParams->luma_shift & 0x00ff;

                if (CodecHal_PictureIsTopField(vc1PicParams->CurrPic) && isSecondField)
                {
                    fwdRefParams = destParams;
                }
                else
                {
                    fwdRefParams = params->ppVc1RefList[vc1PicParams->ForwardRefIdx];
                }

                if (((lumaScale == 32) && (lumaShift == 0)) || !(fwdSingleIcEnable & BOTTOM_FIELD))
                {
                    // No IC for bottom field

                    if (MOS_IS_BIT_SET(fwdRefParams->dwRefSurfaceFlags, CODECHAL_VC1_BOT_FIELD_COMP))
                    {
                        cmd.DW2.Lumscale2SingleFwd = fwdRefParams->Vc1IcValues[icField].wICCScale2;
                        cmd.DW2.Lumshift2SingleFwd = fwdRefParams->Vc1IcValues[icField].wICCShiftL2;
                        fwdSingleIcEnable |= BOTTOM_FIELD;
                    }
                    else
                    {
                        fwdSingleIcEnable &= TOP_FIELD;
                    }
                }
                else
                {
                    // IC is on
                    cmd.DW2.Lumscale2SingleFwd = lumaScale;
                    cmd.DW2.Lumshift2SingleFwd = lumaShift;

                    if (MOS_IS_BIT_SET(fwdRefParams->dwRefSurfaceFlags, CODECHAL_VC1_BOT_FIELD_COMP))
                    {
                        fwdDoubleIcEnable |= BOTTOM_FIELD;
                        cmd.DW3.Lumscale2DoubleFwd = fwdRefParams->Vc1IcValues[icField].wICCScale2;
                        cmd.DW3.Lumshift2DoubleFwd = fwdRefParams->Vc1IcValues[icField].wICCShiftL2;

                        MOS_BIT_ON(fwdRefParams->dwRefSurfaceFlags, CODECHAL_VC1_BOT_FIELD_COMP_2);
                        icField++;
                    }
                    else
                    {
                        MOS_BIT_ON(fwdRefParams->dwRefSurfaceFlags, CODECHAL_VC1_BOT_FIELD_COMP);
                    }

                    // set double backward values for bottom out of bound pixels
                    if (vc1PicParams->picture_fields.intensity_compensation)
                    {
                        bwdDoubleIcEnable |= BOTTOM_FIELD;
                        // If the reference picture is not interlaced field and current picture is top field and
                        // second field, double backwards values are identical to the the single forward values
                        // used by the p interlaced field bottom field
                        if (!CodecHal_PictureIsField((params->ppVc1RefList[vc1PicParams->ForwardRefIdx])->RefPic) &&
                            (CodecHal_PictureIsTopField(vc1PicParams->CurrPic) && isSecondField))
                        {
                            cmd.DW5.Lumscale2DoubleBwd =
                                (params->ppVc1RefList[vc1PicParams->ForwardRefIdx])->Vc1IcValues[icField].wICCScale2;
                            cmd.DW5.Lumshift2DoubleBwd =
                                (params->ppVc1RefList[vc1PicParams->ForwardRefIdx])->Vc1IcValues[icField].wICCShiftL2;
                        }
                        else
                        {
                            cmd.DW5.Lumscale2DoubleBwd = lumaScale;
                            cmd.DW5.Lumshift2DoubleBwd = lumaShift;
                        }
                    }

                    // Save IC
                    fwdRefParams->Vc1IcValues[icField].wICCScale2 = lumaScale;
                    fwdRefParams->Vc1IcValues[icField].wICCShiftL2 = lumaShift;
                }
            }
            else if (isBPicture)
            {
                // Forward reference IC
                if (CodecHal_PictureIsTopField(vc1PicParams->CurrPic) || !isSecondField)
                {
                    if (MOS_IS_BIT_SET(fwdRefParams->dwRefSurfaceFlags, CODECHAL_VC1_TOP_FIELD_COMP_2))
                    {
                        cmd.DW3.Lumscale1DoubleFwd = fwdRefParams->Vc1IcValues[icField].wICCScale1;
                        cmd.DW3.Lumshift1DoubleFwd = fwdRefParams->Vc1IcValues[icField].wICCShiftL1;
                        fwdDoubleIcEnable = TOP_FIELD;
                        icField++;
                    }

                    if (MOS_IS_BIT_SET(fwdRefParams->dwRefSurfaceFlags, CODECHAL_VC1_TOP_FIELD_COMP))
                    {
                        cmd.DW2.Lumscale1SingleFwd = fwdRefParams->Vc1IcValues[icField].wICCScale1;
                        cmd.DW2.Lumshift1SingleFwd = fwdRefParams->Vc1IcValues[icField].wICCShiftL1;
                        fwdSingleIcEnable = TOP_FIELD;
                    }
                }

                if ((vc1PicParams->CurrPic.PicFlags == PICTURE_BOTTOM_FIELD) || !isSecondField)
                {
                    icField = 0;
                    if (MOS_IS_BIT_SET(fwdRefParams->dwRefSurfaceFlags, CODECHAL_VC1_BOT_FIELD_COMP_2))
                    {
                        cmd.DW3.Lumscale2DoubleFwd = fwdRefParams->Vc1IcValues[icField].wICCScale2;
                        cmd.DW3.Lumshift2DoubleFwd = fwdRefParams->Vc1IcValues[icField].wICCShiftL2;
                        fwdDoubleIcEnable |= BOTTOM_FIELD;
                        icField++;
                    }
                    if (MOS_IS_BIT_SET(fwdRefParams->dwRefSurfaceFlags, CODECHAL_VC1_BOT_FIELD_COMP))
                    {
                        cmd.DW2.Lumscale2SingleFwd = fwdRefParams->Vc1IcValues[icField].wICCScale2;
                        cmd.DW2.Lumshift2SingleFwd = fwdRefParams->Vc1IcValues[icField].wICCShiftL2;
                        fwdSingleIcEnable |= BOTTOM_FIELD;
                    }
                }

                // Backward reference IC
                icField = 0;
                if (MOS_IS_BIT_SET(bwdRefParams->dwRefSurfaceFlags, CODECHAL_VC1_TOP_FIELD_COMP))
                {
                    cmd.DW4.Lumscale1SingleBwd = bwdRefParams->Vc1IcValues[icField].wICCScale1;
                    cmd.DW4.Lumshift1SingleBwd = bwdRefParams->Vc1IcValues[icField].wICCShiftL1;
                    bwdSingleIcEnable = TOP_FIELD;
                }

                if (MOS_IS_BIT_SET(bwdRefParams->dwRefSurfaceFlags, CODECHAL_VC1_BOT_FIELD_COMP))
                {
                    cmd.DW4.Lumscale2SingleBwd = bwdRefParams->Vc1IcValues[icField].wICCScale2;
                    cmd.DW4.Lumshift2SingleBwd = bwdRefParams->Vc1IcValues[icField].wICCShiftL2;
                    bwdSingleIcEnable |= BOTTOM_FIELD;
                }
            }
        }

        cmd.DW1.VinIntensitycompDoubleFwden = fwdDoubleIcEnable;
        cmd.DW1.VinIntensitycompDoubleBwden = bwdDoubleIcEnable;
        cmd.DW1.VinIntensitycompSingleFwden = fwdSingleIcEnable;
        cmd.DW1.VinIntensitycompSingleBwden = bwdSingleIcEnable;

        MHW_MI_CHK_STATUS(Mos_AddCommand(cmdBuffer, &cmd, sizeof(cmd)));

        return eStatus;
    }

    MOS_STATUS AddMfxVc1LongPicCmd(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMHW_VDBOX_VC1_PIC_STATE vc1PicState)
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        MHW_FUNCTION_ENTER;

        MHW_MI_CHK_NULL(cmdBuffer);
        MHW_MI_CHK_NULL(vc1PicState);
        MHW_MI_CHK_NULL(vc1PicState->pVc1PicParams);

        auto vc1PicParams = vc1PicState->pVc1PicParams;

        bool isFramePicture =
            ((vc1PicParams->CurrPic.PicFlags == PICTURE_FRAME) |
            (vc1PicParams->CurrPic.PicFlags == PICTURE_INTERLACED_FRAME));

        uint16_t widthInMbs = CODECHAL_GET_WIDTH_IN_MACROBLOCKS(vc1PicParams->coded_width);
        uint16_t heightInMbs = CODECHAL_GET_HEIGHT_IN_MACROBLOCKS(vc1PicParams->coded_height);

        uint16_t frameFieldHeightInMb = 0;
        CodecHal_GetFrameFieldHeightInMb(
            vc1PicParams->CurrPic,
            heightInMbs,
            frameFieldHeightInMb);

        bool isIPicture = IsVc1IPicture(
            vc1PicParams->CurrPic,
            vc1PicParams->picture_fields.is_first_field,
            vc1PicParams->picture_fields.picture_type);

        bool isPPicture = IsVc1PPicture(
            vc1PicParams->CurrPic,
            vc1PicParams->picture_fields.is_first_field,
            vc1PicParams->picture_fields.picture_type);

        bool isBPicture = IsVc1BPicture(
            vc1PicParams->CurrPic,
            vc1PicParams->picture_fields.is_first_field,
            vc1PicParams->picture_fields.picture_type);

        bool isBIPicture = IsVc1BIPicture(
            vc1PicParams->CurrPic,
            vc1PicParams->picture_fields.is_first_field,
            vc1PicParams->picture_fields.picture_type);

        auto destParams = vc1PicState->ppVc1RefList[vc1PicParams->CurrPic.FrameIdx];
        auto fwdRefParams = vc1PicState->ppVc1RefList[vc1PicParams->ForwardRefIdx];

        typename TMfxCmds::MFD_VC1_LONG_PIC_STATE_CMD cmd;

        cmd.DW1.Picturewidthinmbsminus1PictureWidthMinus1InMacroblocks = widthInMbs - 1;
        cmd.DW1.Pictureheightinmbsminus1PictureHeightMinus1InMacroblocks = frameFieldHeightInMb - 1;

        cmd.DW2.Vc1Profile = vc1PicParams->sequence_fields.AdvancedProfileFlag;
        cmd.DW2.Secondfield = !vc1PicParams->picture_fields.is_first_field;
        cmd.DW2.OverlapSmoothingEnableFlag = vc1PicParams->sequence_fields.overlap;
        cmd.DW2.LoopfilterEnableFlag = vc1PicParams->entrypoint_fields.loopfilter;
        cmd.DW2.InterpolationRounderContro = vc1PicParams->rounding_control;
        cmd.DW2.MotionVectorMode = (vc1PicParams->mv_fields.MvMode & 0x9);

        // Simple and Main profile dynamic range adjustment
        if ((!vc1PicParams->sequence_fields.AdvancedProfileFlag) && isPPicture)
        {
            if ((destParams->dwRefSurfaceFlags & CODECHAL_WMV9_RANGE_ADJUSTMENT) &&
                !(fwdRefParams->dwRefSurfaceFlags & CODECHAL_WMV9_RANGE_ADJUSTMENT))
            {
                cmd.DW2.RangereductionEnable = 1;
                cmd.DW2.Rangereductionscale = 0;
            }
            else if (!(destParams->dwRefSurfaceFlags & CODECHAL_WMV9_RANGE_ADJUSTMENT) &&
                (fwdRefParams->dwRefSurfaceFlags & CODECHAL_WMV9_RANGE_ADJUSTMENT))
            {
                cmd.DW2.RangereductionEnable = 1;
                cmd.DW2.Rangereductionscale = 1;
            }
        }

        cmd.DW3.PquantPictureQuantizationValue = vc1PicParams->pic_quantizer_fields.pic_quantizer_scale;

        if (vc1PicState->Mode == CODECHAL_DECODE_MODE_VC1IT)
        {
            if (isIPicture || isBIPicture)
            {
                cmd.DW3.PictypePictureType = vc1IFrame; // 0 = I or I/I
            }
            else if (isPPicture)
            {
                cmd.DW3.PictypePictureType = isFramePicture ? (uint32_t)vc1PFrame: (uint32_t)vc1PPField;
            }
            else if (isBPicture)
            {
                cmd.DW3.PictypePictureType = isFramePicture ? (uint32_t)vc1BFrame: (uint32_t)vc1BBField;
            }

            if (isFramePicture)
            {
                cmd.DW3.FcmFrameCodingMode = (vc1PicParams->CurrPic.PicFlags == PICTURE_INTERLACED_FRAME);
            }
            else
            {
                cmd.DW3.FcmFrameCodingMode = (vc1PicParams->picture_fields.top_field_first) ? vc1TffFrame : vc1BffFrame;
            }

            cmd.DW4.FastuvmcflagFastUvMotionCompensationFlag = (vc1PicParams->mv_fields.MvMode & 0x1);
            cmd.DW4.Pquantuniform = 1; // uniform
            cmd.DW2.Implicitquantizer = 1; // implicit
        }
        else // CODECHAL_DECODE_MODE_VC1VLD
        {
            cmd.DW2.Syncmarker = vc1PicParams->sequence_fields.syncmarker;
            cmd.DW2.Implicitquantizer = (vc1PicParams->pic_quantizer_fields.quantizer == vc1QuantizerImplicit);
            if (isBPicture &&
                (CodecHal_PictureIsBottomField(vc1PicParams->CurrPic) ?
                    vc1PicState->bPrevOddAnchorPictureIsP : vc1PicState->bPrevEvenAnchorPictureIsP)) // OR if I not before B in decoding order
            {
                cmd.DW2.Dmvsurfacevalid = true;
            }

            if (vc1PicParams->raw_coding.bitplane_present)
            {
                cmd.DW2.BitplaneBufferPitchMinus1 = (widthInMbs - 1) >> 1;
            }

            cmd.DW3.Bscalefactor = vc1PicParams->ScaleFactor;
            cmd.DW3.AltpquantAlternativePictureQuantizationValue = vc1PicParams->pic_quantizer_fields.alt_pic_quantizer;
            cmd.DW3.FcmFrameCodingMode = vc1PicParams->picture_fields.frame_coding_mode;
            cmd.DW3.PictypePictureType = vc1PicParams->picture_fields.picture_type;
            cmd.DW3.Condover = vc1PicParams->conditional_overlap_flag;

            cmd.DW4.Pquantuniform = vc1PicParams->pic_quantizer_fields.pic_quantizer_type;
            cmd.DW4.Halfqp = vc1PicParams->pic_quantizer_fields.half_qp;
            cmd.DW4.AltpquantconfigAlternativePictureQuantizationConfiguration = vc1PicParams->pic_quantizer_fields.AltPQuantConfig;
            cmd.DW4.AltpquantedgemaskAlternativePictureQuantizationEdgeMask = vc1PicParams->pic_quantizer_fields.AltPQuantEdgeMask;

            // AltPQuant parameters must be set to 0 for I or BI pictures in simple/main profile
            if (!vc1PicParams->sequence_fields.AdvancedProfileFlag && (isIPicture || isBIPicture))
            {
                cmd.DW4.AltpquantconfigAlternativePictureQuantizationConfiguration = 0;
                cmd.DW4.AltpquantedgemaskAlternativePictureQuantizationEdgeMask = 0;
                cmd.DW3.AltpquantAlternativePictureQuantizationValue = 0;
            }

            cmd.DW4.ExtendedmvrangeExtendedMotionVectorRangeFlag = vc1PicParams->mv_fields.extended_mv_range;
            cmd.DW4.ExtendeddmvrangeExtendedDifferentialMotionVectorRangeFlag = vc1PicParams->mv_fields.extended_dmv_range;
            cmd.DW4.FwdrefdistReferenceDistance = vc1PicParams->reference_fields.reference_distance;
            cmd.DW4.BwdrefdistReferenceDistance = vc1PicParams->reference_fields.BwdReferenceDistance;

            if (!isFramePicture && isBPicture)
            {
                // For B field pictures, NumberOfReferencePictures is always 2 (i.e. set to 1).
                cmd.DW4.NumrefNumberOfReferences = 1;
            }
            else
            {
                cmd.DW4.NumrefNumberOfReferences = vc1PicParams->reference_fields.num_reference_pictures;
            }

            if (isPPicture &&
                CodecHal_PictureIsField(vc1PicParams->CurrPic) &&
                (cmd.DW4.NumrefNumberOfReferences == 0))
            {
                // Derive polarity of the reference field: Top = 0, Bottom = 1
                if (vc1PicParams->reference_fields.reference_field_pic_indicator == 0)
                {
                    // Temporally closest reference
                    if (vc1PicParams->picture_fields.is_first_field)
                    {
                        // Reference frame
                        cmd.DW4.ReffieldpicpolarityReferenceFieldPicturePolarity = vc1PicState->wPrevAnchorPictureTFF;
                    }
                    else
                    {
                        // Same frame
                        cmd.DW4.ReffieldpicpolarityReferenceFieldPicturePolarity = !vc1PicParams->picture_fields.top_field_first;
                    }
                }
                else
                {
                    // Second most temporally closest reference
                    if (vc1PicParams->picture_fields.is_first_field)
                    {
                        // First field of reference frame
                        cmd.DW4.ReffieldpicpolarityReferenceFieldPicturePolarity = !vc1PicState->wPrevAnchorPictureTFF;
                    }
                    else
                    {
                        // Second field of reference frame
                        cmd.DW4.ReffieldpicpolarityReferenceFieldPicturePolarity = vc1PicState->wPrevAnchorPictureTFF;
                    }
                }
            }

            cmd.DW4.FastuvmcflagFastUvMotionCompensationFlag = vc1PicParams->fast_uvmc_flag;
            cmd.DW4.FourmvswitchFourMotionVectorSwitch = vc1PicParams->mv_fields.four_mv_switch;
            cmd.DW4.UnifiedmvmodeUnifiedMotionVectorMode = vc1PicParams->mv_fields.UnifiedMvMode;

            // If bitplane is present (BitplanePresentFlag == 1) update the "raw" bitplane
            // flags. If bitplane is not present leave all "raw" flags to their initialized
            // value which is all bitplanes are present "raw".
            cmd.DW5.BitplanepresentflagBitplaneBufferPresentFlag = vc1PicParams->raw_coding.bitplane_present;

            cmd.DW5.Fieldtxraw = vc1RawMode;
            cmd.DW5.Acpredraw = vc1RawMode;
            cmd.DW5.Overflagsraw = vc1RawMode;
            cmd.DW5.Directmbraw = vc1RawMode;
            cmd.DW5.Skipmbraw = vc1RawMode;
            cmd.DW5.Mvtypembraw = vc1RawMode;
            cmd.DW5.Forwardmbraw = vc1RawMode;

            if (vc1PicParams->raw_coding.bitplane_present)
            {
                cmd.DW5.Fieldtxraw = vc1PicParams->raw_coding.field_tx;
                cmd.DW5.Acpredraw = vc1PicParams->raw_coding.ac_pred;
                cmd.DW5.Overflagsraw = vc1PicParams->raw_coding.overflags;
                cmd.DW5.Directmbraw = vc1PicParams->raw_coding.direct_mb;
                cmd.DW5.Skipmbraw = vc1PicParams->raw_coding.skip_mb;
                cmd.DW5.Mvtypembraw = vc1PicParams->raw_coding.mv_type_mb;
                cmd.DW5.Forwardmbraw = vc1PicParams->raw_coding.forward_mb;
            }

            cmd.DW5.CbptabCodedBlockPatternTable = vc1PicParams->cbp_table;
            cmd.DW5.TransdctabIntraTransformDcTable = vc1PicParams->transform_fields.intra_transform_dc_table;
            cmd.DW5.TransacuvPictureLevelTransformChromaAcCodingSetIndexTransactable = vc1PicParams->transform_fields.transform_ac_codingset_idx1;
            cmd.DW5.TransacyPictureLevelTransformLumaAcCodingSetIndexTransactable2 = (isIPicture || isBIPicture) ?
                vc1PicParams->transform_fields.transform_ac_codingset_idx2 : cmd.DW5.TransacuvPictureLevelTransformChromaAcCodingSetIndexTransactable;
            cmd.DW5.MbmodetabMacroblockModeTable = vc1PicParams->mb_mode_table;

            if (vc1PicParams->transform_fields.variable_sized_transform_flag == 0)
            {
                // H/W decodes TTMB, TTBLK and SUBBLKPAT if the picture level TTMBF flag is not set.
                // If the VSTRANSFORM is 0, 8x8 TransformType is used for all the pictures belonging to this Entry-Point.
                // Hence H/W overloads the TTMBF = 1 and TTFRM = 8x8 in this case.
                cmd.DW5.TranstypembflagMacroblockTransformTypeFlag = 1;
                cmd.DW5.TranstypePictureLevelTransformType = 0;
            }
            else
            {
                cmd.DW5.TranstypembflagMacroblockTransformTypeFlag = vc1PicParams->transform_fields.mb_level_transform_type_flag;
                cmd.DW5.TranstypePictureLevelTransformType = vc1PicParams->transform_fields.frame_level_transform_type;
            }
            cmd.DW5.Twomvbptab2MvBlockPatternTable = vc1PicParams->mv_fields.two_mv_block_pattern_table;
            cmd.DW5.Fourmvbptab4MvBlockPatternTable = vc1PicParams->mv_fields.four_mv_block_pattern_table;
            cmd.DW5.MvtabMotionVectorTable = vc1PicParams->mv_fields.mv_table;
        }

        MHW_MI_CHK_STATUS(Mos_AddCommand(cmdBuffer, &cmd, sizeof(cmd)));

        return eStatus;
    }

    MOS_STATUS AddMfdVc1ShortPicCmd(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMHW_VDBOX_VC1_PIC_STATE vc1PicState)
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        MHW_FUNCTION_ENTER;

        MHW_MI_CHK_NULL(cmdBuffer);
        MHW_MI_CHK_NULL(vc1PicState);
        MHW_MI_CHK_NULL(vc1PicState->pVc1PicParams);

        auto vc1PicParams = vc1PicState->pVc1PicParams;

        uint16_t widthInMbs = CODECHAL_GET_WIDTH_IN_MACROBLOCKS(vc1PicParams->coded_width);
        uint16_t heightInMbs = CODECHAL_GET_HEIGHT_IN_MACROBLOCKS(vc1PicParams->coded_height);

        uint16_t frameFieldHeightInMb = 0;
        CodecHal_GetFrameFieldHeightInMb(
            vc1PicParams->CurrPic,
            heightInMbs,
            frameFieldHeightInMb);

        bool isIPicture = IsVc1IPicture(
            vc1PicParams->CurrPic,
            vc1PicParams->picture_fields.is_first_field,
            vc1PicParams->picture_fields.picture_type);

        bool isPPicture = IsVc1PPicture(
            vc1PicParams->CurrPic,
            vc1PicParams->picture_fields.is_first_field,
            vc1PicParams->picture_fields.picture_type);

        bool isBPicture = IsVc1BPicture(
            vc1PicParams->CurrPic,
            vc1PicParams->picture_fields.is_first_field,
            vc1PicParams->picture_fields.picture_type);

        bool isBIPicture = IsVc1BIPicture(
            vc1PicParams->CurrPic,
            vc1PicParams->picture_fields.is_first_field,
            vc1PicParams->picture_fields.picture_type);

        typename TMfxCmds::MFD_VC1_SHORT_PIC_STATE_CMD cmd;

        // DW 1
        cmd.DW1.PictureWidth = widthInMbs - 1;
        cmd.DW1.PictureHeight = frameFieldHeightInMb - 1;

        // DW 2
        cmd.DW2.PictureStructure =
            (CodecHal_PictureIsTopField(vc1PicParams->CurrPic)) ?
            mpeg2Vc1TopField : (CodecHal_PictureIsBottomField(vc1PicParams->CurrPic)) ?
            mpeg2Vc1BottomField : mpeg2Vc1Frame;
        cmd.DW2.Secondfield = !vc1PicParams->picture_fields.is_first_field;
        cmd.DW2.IntraPictureFlag = isIPicture || isBIPicture;
        cmd.DW2.BackwardPredictionPresentFlag = isBPicture;

        cmd.DW2.Vc1Profile = vc1PicParams->sequence_fields.AdvancedProfileFlag;
        if (isBPicture &&
            (CodecHal_PictureIsBottomField(vc1PicParams->CurrPic) ?
                vc1PicState->bPrevOddAnchorPictureIsP : vc1PicState->bPrevEvenAnchorPictureIsP)) // OR if I not before B in decoding order
        {
            cmd.DW2.Dmvsurfacevalid = true;
        }

        cmd.DW2.MotionVectorMode = vc1PicParams->mv_fields.MvMode & 0x9;
        cmd.DW2.InterpolationRounderControl = vc1PicParams->rounding_control;
        cmd.DW2.BitplaneBufferPitchMinus1 = (vc1PicParams->coded_width <= 2048) ?
            (MHW_VDBOX_VC1_BITPLANE_BUFFER_PITCH_SMALL - 1) : (MHW_VDBOX_VC1_BITPLANE_BUFFER_PITCH_LARGE - 1);

        // DW 3
        cmd.DW3.VstransformFlag = vc1PicParams->transform_fields.variable_sized_transform_flag;
        cmd.DW3.Dquant = vc1PicParams->pic_quantizer_fields.dquant;
        cmd.DW3.ExtendedMvPresentFlag = vc1PicParams->mv_fields.extended_mv_flag;
        cmd.DW3.FastuvmcflagFastUvMotionCompensationFlag = vc1PicParams->fast_uvmc_flag;
        cmd.DW3.LoopfilterEnableFlag = vc1PicParams->entrypoint_fields.loopfilter;
        cmd.DW3.RefdistFlag = (vc1PicParams->sequence_fields.AdvancedProfileFlag) ?
            vc1PicParams->reference_fields.reference_distance_flag : 1;
        cmd.DW3.PanscanPresentFlag = vc1PicParams->entrypoint_fields.panscan_flag;

        cmd.DW3.Maxbframes = vc1PicParams->sequence_fields.max_b_frames;
        cmd.DW3.RangeredPresentFlagForSimpleMainProfileOnly = vc1PicParams->sequence_fields.rangered;
        cmd.DW3.SyncmarkerPresentFlagForSimpleMainProfileOnly = vc1PicParams->sequence_fields.syncmarker;
        cmd.DW3.MultiresPresentFlagForSimpleMainProfileOnly = vc1PicParams->sequence_fields.multires;
        cmd.DW3.Quantizer = vc1PicParams->pic_quantizer_fields.quantizer;
        cmd.DW3.PPicRefDistance = vc1PicParams->reference_fields.reference_distance;

        cmd.DW3.ProgressivePicType = (CodecHal_PictureIsFrame(vc1PicParams->CurrPic)) ? 1 : 2;
        // Dynamic range adjustment disabled
        cmd.DW3.RangeReductionEnable = 0;
        cmd.DW3.RangeReductionScale = 1;
        if (vc1PicParams->sequence_fields.AdvancedProfileFlag)
        {
            cmd.DW3.OverlapSmoothingEnableFlag = vc1PicParams->sequence_fields.overlap;
        }
        else
        {
            cmd.DW3.OverlapSmoothingEnableFlag = 1;
            if (isBPicture || (vc1PicParams->pic_quantizer_fields.pic_quantizer_scale < 9) || !vc1PicParams->sequence_fields.overlap)
            {
                cmd.DW3.OverlapSmoothingEnableFlag = 0;
            }
        }

        // DW 4
        cmd.DW4.ExtendedDmvPresentFlag = vc1PicParams->mv_fields.extended_dmv_flag;
        cmd.DW4.Psf = vc1PicParams->sequence_fields.psf;
        cmd.DW4.Finterflag = vc1PicParams->sequence_fields.finterpflag;
        cmd.DW4.Tfcntrflag = vc1PicParams->sequence_fields.tfcntrflag;
        cmd.DW4.Interlace = vc1PicParams->sequence_fields.interlace;
        cmd.DW4.Pulldown = vc1PicParams->sequence_fields.pulldown;
        cmd.DW4.PostprocFlag = vc1PicParams->post_processing;
        if (isPPicture || (isBPicture && vc1PicParams->sequence_fields.interlace))
        {
            cmd.DW4._4MvAllowedFlag = vc1PicParams->mv_fields.four_mv_allowed;
        }
        cmd.DW4.RefpicFlag = vc1PicParams->reference_fields.reference_picture_flag;
        if (isBPicture)
        {
            cmd.DW4.BfractionEnumeration = vc1PicParams->b_picture_fraction;
        }

        MHW_MI_CHK_STATUS(Mos_AddCommand(cmdBuffer, &cmd, sizeof(cmd)));

        return eStatus;
    }

    MOS_STATUS AddMfxVc1DirectmodeCmd(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMHW_VDBOX_VC1_DIRECTMODE_PARAMS params)
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        MHW_FUNCTION_ENTER;

        MHW_MI_CHK_NULL(cmdBuffer);
        MHW_MI_CHK_NULL(params);

        typename TMfxCmds::MFX_VC1_DIRECTMODE_STATE_CMD cmd;

        MHW_RESOURCE_PARAMS resourceParams;
        MOS_ZeroMemory(&resourceParams, sizeof(resourceParams));
        resourceParams.dwLsbNum = MHW_VDBOX_MFX_GENERAL_STATE_SHIFT;
        resourceParams.HwCommandType = MOS_MFX_VC1_DIRECT_MODE;

        cmd.DW3.MemoryObjectControlState =
            m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_DIRECTMV_BUFFER_CODEC].Value;

        resourceParams.presResource = params->presDmvWriteBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = &(cmd.DW1.Value);
        resourceParams.dwLocationInCmd = 1;
        resourceParams.bIsWritable = true;

        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));

        cmd.DW6.MemoryObjectControlState =
            m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_DIRECTMV_BUFFER_CODEC].Value;

        resourceParams.presResource = params->presDmvReadBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = &(cmd.DW4.Value);
        resourceParams.dwLocationInCmd = 4;
        resourceParams.bIsWritable = false;

        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));

        MHW_MI_CHK_STATUS(Mos_AddCommand(cmdBuffer, &cmd, sizeof(cmd)));

        return eStatus;
    }

    MOS_STATUS AddMfdVc1BsdObjectCmd(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMHW_VDBOX_VC1_SLICE_STATE vc1SliceState)
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        MHW_FUNCTION_ENTER;

        MHW_MI_CHK_NULL(cmdBuffer);
        MHW_MI_CHK_NULL(vc1SliceState);
        MHW_MI_CHK_NULL(vc1SliceState->pSlc);

        typename TMfxCmds::MFD_VC1_BSD_OBJECT_CMD cmd;
        auto slcParams = vc1SliceState->pSlc;

        cmd.DW1.IndirectBsdDataLength = vc1SliceState->dwLength;

        cmd.DW2.IndirectDataStartAddress = slcParams->slice_data_offset + vc1SliceState->dwOffset; // byte aligned

        cmd.DW3.SliceStartVerticalPosition = slcParams->slice_vertical_position;
        cmd.DW3.NextSliceVerticalPosition = vc1SliceState->dwNextVerticalPosition;

        cmd.DW4.FirstMbByteOffsetOfSliceDataOrSliceHeader = (slcParams->macroblock_offset >> 3) - vc1SliceState->dwOffset;
        cmd.DW4.FirstmbbitoffsetFirstMacroblockBitOffset = slcParams->macroblock_offset & 0x7; // bit offset

        MHW_CP_SLICE_INFO_PARAMS sliceInfoParam;
        sliceInfoParam.presDataBuffer = vc1SliceState->presDataBuffer;
        sliceInfoParam.dwDataStartOffset[0] = cmd.DW2.IndirectDataStartAddress;

        MHW_MI_CHK_STATUS(m_cpInterface->SetMfxProtectionState(
            m_decodeInUse,
            cmdBuffer,
            nullptr,
            &sliceInfoParam));

        MHW_MI_CHK_STATUS(Mos_AddCommand(cmdBuffer, &cmd, sizeof(cmd)));

        return eStatus;
    }

    //!
    //! \struct   MFD_VC1_IT_OBJECT_CMD
    //! \brief    MFD VC1 it object command
    //!
    struct MFD_VC1_IT_OBJECT_CMD
    {
        typename TMfxCmds::MFD_IT_OBJECT_CMD m_header;
        typename TMfxCmds::MFD_IT_OBJECT_VC1_INLINE_DATA_CMD m_inlineData;
    };

    //!
    //! \brief    Get VC1 intra flag
    //! 
    //! \param    [in] mbState
    //!           Pointer to MHW vdbox VC1 mb state 
    //! \param    [in] mbParams
    //!           Pointer to codec VC1 mb parameters
    //!
    //! \return   uint8_t
    //!           VC1 intra flag
    //!
    uint8_t GetVc1IntraFlag(PMHW_VDBOX_VC1_MB_STATE mbState, PCODEC_VC1_MB_PARAMS mbParams)
    {
        const uint8_t PATTERN_CODE_INTRA_MB = 0xF;
        uint8_t intra8x8Flag = 0;

        if(mbState == nullptr || mbParams == nullptr)
        {
            MHW_ASSERTMESSAGE("mbState or mbParams is nullptr!");
            return 0;
        }
        
        if (mbParams->mb_type.intra_mb)
        {
            intra8x8Flag = PATTERN_CODE_INTRA_MB;
        }
        else if (mbParams->mb_type.motion_4mv && (mbState->PicFlags == PICTURE_FRAME))
        {
            intra8x8Flag = mbParams->pattern_code.block_luma_intra;
        }
        else
        {
            intra8x8Flag = 0;
        }

        return intra8x8Flag;
    }

    #define GET_VC1_BLOCK(mb, i) ((mb >> (3 - i)) & 1)

    //!
    //! \brief    VC1 it object set overlap smoothing filter
    //!
    //! \param    [in] inlineDataVc1
    //!           MFD it object VC1 inline data command
    //! \param    [in] mbState
    //!           Pointer to MHW vdbox VC1 mb state 
    //! \param    [in] mbParams
    //!           Pointer to codec VC1 mb parameters
    //! \param    [in] mbHorizOrigin
    //!           Mb horizontal origin
    //! \param    [in] mbVertOrigin
    //!           Mb vertical origin
    //!
    MOS_STATUS Vc1ItObjectSetOverlapSmoothingFilter(
        typename TMfxCmds::MFD_IT_OBJECT_VC1_INLINE_DATA_CMD *inlineDataVc1,
        PMHW_VDBOX_VC1_MB_STATE mbState,
        PCODEC_VC1_MB_PARAMS mbParams,
        uint8_t mbHorizOrigin,
        uint8_t mbVertOrigin)
    {
        static const uint8_t chromaIntra[16] =
        {
            0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 1, 1
        };
        MHW_CHK_NULL_RETURN(inlineDataVc1);
        MHW_CHK_NULL_RETURN(mbState);
        MHW_CHK_NULL_RETURN(mbParams);

        auto vc1PicParams = mbState->pVc1PicParams;
        MHW_CHK_NULL_RETURN(vc1PicParams);

        //------------------------------------
        // Overlap smoothing enabled for this mb?
        //------------------------------------
        uint8_t mbOverlapSmoothing = mbParams->mb_type.h261_loopfilter;
        inlineDataVc1->DW0.Overlaptransform = mbOverlapSmoothing;

        // Horizontal origin is last mb of the row
        inlineDataVc1->DW0.Lastmbinrow = (mbHorizOrigin == (mbState->wPicWidthInMb - 1));
        // Vertical origin is last mb of the column
        inlineDataVc1->DW0.LastRowFlag = (mbVertOrigin == (mbState->wPicHeightInMb - 1));

        if (mbOverlapSmoothing)
        {
            uint8_t intra8x8 = GetVc1IntraFlag(mbState, mbParams);

            if ((vc1PicParams->picture_fields.picture_type == vc1BBField) || !intra8x8)
            {
                // Reset parameters
                inlineDataVc1->DW0.Overlaptransform = 0;
                inlineDataVc1->DW1.Osedgemaskluma = 0;
                inlineDataVc1->DW1.Osedgemaskchroma = 0;

                return MOS_STATUS_SUCCESS;
            }

            //------------------------------------
            // Set edge control bitmasks (luma & chroma)
            //------------------------------------

            // Overlap smoothing applied to an edge when
            // 1. Edge between two 8x8 luma regions or corresponding 4x4
            //    chroma regions of the same mb for which the H261loopFilter
            //    flag is equal to 1
            // 2. Edge between 8x8 luma regions or corresponding 4x4 chroma
            //    regions in different MBs for which both are true:
            //    a. H261loopFilter flag is equal to 1 in both macroblocks, and
            //    b. Edge is a vertical edge between horizontally-neighboring macroblocks,
            //       OR is a horizontal edge between vertically-neighboring macroblocks and
            //       ReservedBits flag (bit 11) is equal to 0 in the wMBtype element of the
            //       macroblock control command for the lower macroblock and the picture is not an
            //       interlaced frame (i.e., a picture with bPicStructure equal to '11' and
            //        bPicExtrapolation equal to 2).

            // Condition 1: Top Y2, Top Y3, Left Y1, Left Y3
            uint16_t edgeMaskLuma = 0, edgeMaskChroma = 0;
            edgeMaskLuma |= (GET_VC1_BLOCK(intra8x8, 0) & GET_VC1_BLOCK(intra8x8, 2)) << 2;
            edgeMaskLuma |= (GET_VC1_BLOCK(intra8x8, 1) & GET_VC1_BLOCK(intra8x8, 3)) << 3;
            edgeMaskLuma |= (GET_VC1_BLOCK(intra8x8, 0) & GET_VC1_BLOCK(intra8x8, 1)) << 5;
            edgeMaskLuma |= (GET_VC1_BLOCK(intra8x8, 2) & GET_VC1_BLOCK(intra8x8, 3)) << 7;

            // Condition 2:
            // Top Y0, Top Y1, Top Cb/Cr: horizontal edges
            if (mbVertOrigin != 0)
            {
                // Not the top row, so get upper pMB
                auto upperMbParams = mbParams - mbState->wPicWidthInMb;

                if (upperMbParams                          &&
                    upperMbParams->mb_type.h261_loopfilter &&
                    !mbParams->mb_type.reserved &&
                    (mbState->PicFlags != PICTURE_INTERLACED_FRAME))
                {
                    uint8_t adjIntra8x8 = GetVc1IntraFlag(mbState, upperMbParams);
                    edgeMaskLuma |= (GET_VC1_BLOCK(intra8x8, 0) & GET_VC1_BLOCK(adjIntra8x8, 2));
                    edgeMaskLuma |= (GET_VC1_BLOCK(intra8x8, 1) & GET_VC1_BLOCK(adjIntra8x8, 3)) << 1;
                    edgeMaskChroma |= (chromaIntra[intra8x8] & chromaIntra[adjIntra8x8]);
                }
            }

            // Left Y0, Left Y2, Left Cb/Cr: vertical edges
            if (mbHorizOrigin != 0)
            {
                // Not the first column, so get left pMB
                auto leftMbParams = mbParams - 1;

                if (leftMbParams                           &&
                    leftMbParams->mb_type.h261_loopfilter)
                {
                    uint8_t adjIntra8x8 = GetVc1IntraFlag(mbState, leftMbParams);
                    edgeMaskLuma |= (GET_VC1_BLOCK(intra8x8, 0) & GET_VC1_BLOCK(adjIntra8x8, 1)) << 4;
                    edgeMaskLuma |= (GET_VC1_BLOCK(intra8x8, 2) & GET_VC1_BLOCK(adjIntra8x8, 3)) << 6;
                    edgeMaskChroma |= (chromaIntra[intra8x8] & chromaIntra[adjIntra8x8]) << 1;
                }
            }

            // Right Y1, Right Y3, Right Cb/Cr: vertical edges
            if (mbHorizOrigin != (mbState->wPicWidthInMb - 1))
            {
                // Not the last column, so get right pMB
                auto rightMbParams = mbParams + 1;

                if (rightMbParams                           &&
                    rightMbParams->mb_type.h261_loopfilter)
                {
                    uint8_t adjIntra8x8 = GetVc1IntraFlag(mbState, rightMbParams);
                    edgeMaskLuma |= (GET_VC1_BLOCK(intra8x8, 1) & GET_VC1_BLOCK(adjIntra8x8, 0)) << 8;
                    edgeMaskLuma |= (GET_VC1_BLOCK(intra8x8, 3) & GET_VC1_BLOCK(adjIntra8x8, 2)) << 9;
                    edgeMaskChroma |= (chromaIntra[intra8x8] & chromaIntra[adjIntra8x8]) << 2;
                }
            }

            inlineDataVc1->DW1.Osedgemaskluma = edgeMaskLuma;
            inlineDataVc1->DW1.Osedgemaskchroma = edgeMaskChroma;
        }
        else
        {
            // Reset parameters
            inlineDataVc1->DW1.Osedgemaskluma = 0;
            inlineDataVc1->DW1.Osedgemaskchroma = 0;
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AddMfdVc1ItObjectCmd(
        PMHW_BATCH_BUFFER batchBuffer,
        PMHW_VDBOX_VC1_MB_STATE mbState)
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        MHW_FUNCTION_ENTER;

        MHW_MI_CHK_NULL(batchBuffer);
        MHW_MI_CHK_NULL(mbState);

        auto mbParams = mbState->pMb;
        auto vc1PicParams = mbState->pVc1PicParams;

        MFD_VC1_IT_OBJECT_CMD cmd;
        typename TMfxCmds::MFD_IT_OBJECT_CMD *cmdMfdItObject = &cmd.m_header;
        typename TMfxCmds::MFD_IT_OBJECT_VC1_INLINE_DATA_CMD *inlineDataVc1 = &cmd.m_inlineData;

        inlineDataVc1->DW0.MacroblockIntraType = mpeg2Vc1MacroblockIntra;

        cmdMfdItObject->DW0.DwordLength += TMfxCmds::MFD_IT_OBJECT_VC1_INLINE_DATA_CMD::dwSize;

        if (mbState->bSkipped)
        {
            inlineDataVc1->DW0.DctType = mbParams->mb_type.field_residual;

            if (vc1PicParams->picture_fields.picture_type != vc1IIField)
            {
                inlineDataVc1->DW0.MacroblockIntraType = mpeg2Vc1MacroblockNonintra;
                inlineDataVc1->DW0.MacroblockMotionForward = mbParams->mb_type.motion_forward;
                inlineDataVc1->DW0.MacroblockMotionBackward = mbParams->mb_type.motion_backward;
                inlineDataVc1->DW0.MotionType = mbParams->mb_type.motion_type;
                inlineDataVc1->DW0.MotionVerticalFieldSelect = (mbParams->mb_type.value & 0xF000) >> 12;
                inlineDataVc1->DW1.Horzorigin = mbState->bMbHorizOrigin;
                inlineDataVc1->DW1.Vertorigin = mbState->bMbVertOrigin;
                inlineDataVc1->DW0.Lastmbinrow =
                    ((inlineDataVc1->DW1.Horzorigin == (mbState->wPicWidthInMb - 1)) &&
                    ((inlineDataVc1->DW1.Vertorigin == (mbState->wPicHeightInMb - 1))));
            }

            MHW_MI_CHK_STATUS(Mhw_AddCommandBB(batchBuffer, &cmd, sizeof(cmd)));

            return eStatus;
        }

        // For VC-1 IT VFE Dword is Reserved : MBZ
        cmdMfdItObject->DW3.IndirectItCoeffDataLength = mbState->dwLength;
        cmdMfdItObject->DW4.IndirectItCoeffDataStartAddressOffset = mbState->dwOffset;

        // VC-1 inline data
        inlineDataVc1->DW0.MotionType = mbParams->mb_type.motion_type;
        inlineDataVc1->DW0.DctType = mbParams->mb_type.field_residual;

        inlineDataVc1->DW1.Horzorigin = mbState->bMbHorizOrigin;
        inlineDataVc1->DW1.Vertorigin = mbState->bMbVertOrigin;

        inlineDataVc1->DW7.SubblockCodeForY0 = mbParams->num_coef[0];
        inlineDataVc1->DW7.SubblockCodeForY1 = mbParams->num_coef[1];
        inlineDataVc1->DW7.SubblockCodeForY2 = mbParams->num_coef[2];
        inlineDataVc1->DW7.SubblockCodeForY3 = mbParams->num_coef[3];
        inlineDataVc1->DW8.SubblockCodeForCb = mbParams->num_coef[4];
        inlineDataVc1->DW8.SubblockCodeForCr = mbParams->num_coef[5];

        // Subblock coding information not present in bNumCoef for Interlace Frame
        // intra MBs, so when all 0 (i.e. subblock partition is 8x8) assume subblock present = 1
        if (MEDIA_IS_WA(m_waTable, WaAssumeSubblockPresent))
        {
            if ((mbState->PicFlags == PICTURE_INTERLACED_FRAME) && mbParams->mb_type.intra_mb &&
                (inlineDataVc1->DW7.Value == 0) && (inlineDataVc1->DW8.Value == 0))
            {
                inlineDataVc1->DW7.SubblockCodeForY0 |= 4;
                inlineDataVc1->DW7.SubblockCodeForY1 |= 4;
                inlineDataVc1->DW7.SubblockCodeForY2 |= 4;
                inlineDataVc1->DW7.SubblockCodeForY3 |= 4;
                inlineDataVc1->DW8.SubblockCodeForCb |= 4;
                inlineDataVc1->DW8.SubblockCodeForCr |= 4;
            }
        }

        if (vc1PicParams->picture_fields.picture_type == vc1PFrame ||
            vc1PicParams->sequence_fields.AdvancedProfileFlag)
        {
            inlineDataVc1->DW9.IldbControlDataForBlockY0 = mbState->DeblockData[0];
            inlineDataVc1->DW9.IldbControlDataForBlockY1 = mbState->DeblockData[1];
            inlineDataVc1->DW9.IldbControlDataForBlockY2 = mbState->DeblockData[2];
            inlineDataVc1->DW9.IldbControlDataForBlockY3 = mbState->DeblockData[3];
            inlineDataVc1->DW10.IldbControlDataForCbBlock = mbState->DeblockData[4];
            inlineDataVc1->DW10.IldbControlDataForCrBlock = mbState->DeblockData[5];
        }
        else if (vc1PicParams->entrypoint_fields.loopfilter)
        {
            //driver generates the edge control value for I and B frames in VC1 Simple and Main profile
            if (mbState->bMbHorizOrigin == 0 && mbState->bMbVertOrigin == 0)
            {
                inlineDataVc1->DW9.Value = MHW_VDBOX_DECODE_VC1_IT_ILDB_EDGE_CONTROL_LUMA_X0Y0;
                inlineDataVc1->DW10.Value = MHW_VDBOX_DECODE_VC1_IT_ILDB_EDGE_CONTROL_CHROMA_X0Y0;
            }
            else if (mbState->bMbHorizOrigin == 0)
            {
                inlineDataVc1->DW9.Value = MHW_VDBOX_DECODE_VC1_IT_ILDB_EDGE_CONTROL_LUMA_X0Y1;
                inlineDataVc1->DW10.Value = MHW_VDBOX_DECODE_VC1_IT_ILDB_EDGE_CONTROL_CHROMA_X0Y1;
            }
            else if (mbState->bMbVertOrigin == 0)
            {
                inlineDataVc1->DW9.Value = MHW_VDBOX_DECODE_VC1_IT_ILDB_EDGE_CONTROL_LUMA_X1Y0;
                inlineDataVc1->DW10.Value = MHW_VDBOX_DECODE_VC1_IT_ILDB_EDGE_CONTROL_CHROMA_X1Y0;
            }
            else
            {
                inlineDataVc1->DW9.Value = MHW_VDBOX_DECODE_VC1_IT_ILDB_EDGE_CONTROL_LUMA_X1Y1;
                inlineDataVc1->DW10.Value = MHW_VDBOX_DECODE_VC1_IT_ILDB_EDGE_CONTROL_CHROMA_X1Y1;
            }
        }
        else
        {
            inlineDataVc1->DW9.Value = 0;
            inlineDataVc1->DW10.Value = 0;
        }

        if (vc1PicParams->picture_fields.picture_type == vc1IFrame)
        {
            // Intra MB
            inlineDataVc1->DW0.CodedBlockPattern = 63;

            MHW_MI_CHK_STATUS(Vc1ItObjectSetOverlapSmoothingFilter(
                inlineDataVc1,
                mbState,
                mbParams,
                mbState->bMbHorizOrigin,
                mbState->bMbVertOrigin));

            MHW_MI_CHK_STATUS(Mhw_AddCommandBB(batchBuffer, &cmd, sizeof(cmd)));
        }
        else
        {
            // Motion vectors
            inlineDataVc1->DW0.MotionVerticalFieldSelect = (mbParams->mb_type.value & 0xF000) >> 12;
            inlineDataVc1->DW0.Motion4Mv = mbParams->mb_type.motion_4mv;
            inlineDataVc1->DW0.MacroblockMotionForward = mbParams->mb_type.motion_forward;
            inlineDataVc1->DW0.MacroblockMotionBackward = mbParams->mb_type.motion_backward;
            inlineDataVc1->DW0.MacroblockIntraType = mbParams->mb_type.intra_mb;
            inlineDataVc1->DW0.CodedBlockPattern =
                mbParams->mb_type.intra_mb ? 63 : mbParams->pattern_code.block_coded_pattern;

            if (mbParams->mb_type.motion_4mv && (mbState->PicFlags == PICTURE_FRAME))
            {
                uint8_t cbp = (uint8_t)mbParams->pattern_code.block_luma_intra;
                inlineDataVc1->DW0.CodedBlockPattern |= (cbp << 2); // Update luma residue blocks from intra Flags
                inlineDataVc1->DW0.LumaIntra8X8Flag = mbParams->pattern_code.block_luma_intra;
                inlineDataVc1->DW0.ChromaIntraFlag = mbParams->pattern_code.block_chroma_intra;

                if (!mbParams->mb_type.intra_mb && (cbp == 0xF)) // top 4 bits of wPatternCode all set means intra
                {
                    mbParams->mb_type.intra_mb = 1;
                    inlineDataVc1->DW0.MacroblockIntraType = 1;
                    inlineDataVc1->DW0.MotionType = 0; // Intra
                    inlineDataVc1->DW0.MacroblockMotionForward = 0;
                    inlineDataVc1->DW0.MacroblockMotionBackward = 0;
                }
            }

            // Next, copy in the motion vectors if not skipped frame
            if (!mbParams->mb_type.intra_mb && mbState->dwDataSize)
            {
                inlineDataVc1->DW2.Value = mbState->PackedLumaMvs[0];
                inlineDataVc1->DW3.Value = mbState->PackedLumaMvs[1];
                inlineDataVc1->DW4.Value = mbState->PackedLumaMvs[2];
                inlineDataVc1->DW5.Value = mbState->PackedLumaMvs[3];
                inlineDataVc1->DW6.Value =
                    (mbState->PicFlags == PICTURE_INTERLACED_FRAME) ? 0 : mbState->PackedChromaMv;

                inlineDataVc1->DW0.MotionVerticalFieldSelect = (mbParams->mb_type.value & 0xF000) >> 12;
                inlineDataVc1->DW0.Mvfieldselectchroma = mbState->bFieldPolarity;
                inlineDataVc1->DW0.Mvswitch = mbState->bMotionSwitch;
            }

            if (!mbParams->mb_skips_following)
            {
                MHW_MI_CHK_STATUS(Vc1ItObjectSetOverlapSmoothingFilter(
                    inlineDataVc1,
                    mbState,
                    mbParams,
                    mbState->bMbHorizOrigin,
                    mbState->bMbVertOrigin));

                MHW_MI_CHK_STATUS(Mhw_AddCommandBB(batchBuffer, &cmd, sizeof(cmd)));
            }
            else
            {
                // Skipped MB's are inter MB's (no residual data, only MV's) so no overlap smoothing
                uint16_t skippedMBs = (uint16_t)mbParams->mb_skips_following + 1;

                for (uint16_t num = 0; num < skippedMBs; num++)
                {
                    inlineDataVc1->DW0.CodedBlockPattern = 0;
                    inlineDataVc1->DW1.Horzorigin = (mbParams->mb_address + num) % mbState->wPicWidthInMb;
                    inlineDataVc1->DW1.Vertorigin = (mbParams->mb_address + num) / mbState->wPicWidthInMb;

                    // Reset overlap smoothing params
                    inlineDataVc1->DW0.Lastmbinrow = (inlineDataVc1->DW1.Horzorigin == (mbState->wPicWidthInMb - 1));
                    inlineDataVc1->DW0.LastRowFlag = (inlineDataVc1->DW1.Vertorigin == (mbState->wPicHeightInMb - 1));
                    inlineDataVc1->DW1.Osedgemaskluma = 0;
                    inlineDataVc1->DW1.Osedgemaskchroma = 0;

                    MHW_MI_CHK_STATUS(Mhw_AddCommandBB(batchBuffer, &cmd, sizeof(cmd)));
                }
            }
        }

        return eStatus;
    }

    MOS_STATUS AddMfxJpegHuffTableCmd(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMHW_VDBOX_HUFF_TABLE_PARAMS params)
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        MHW_FUNCTION_ENTER;

        MHW_MI_CHK_NULL(cmdBuffer);
        MHW_MI_CHK_NULL(params);

        typename TMfxCmds::MFX_JPEG_HUFF_TABLE_STATE_CMD cmd;

        cmd.DW1.Hufftableid1Bit = params->HuffTableID;

        MHW_MI_CHK_STATUS(MOS_SecureMemcpy(
            cmd.DcBits128BitArray, 
            sizeof(cmd.DcBits128BitArray), 
            params->pDCBits, 
            sizeof(cmd.DcBits128BitArray)));
        MHW_MI_CHK_STATUS(MOS_SecureMemcpy(
            cmd.DcHuffval128BitArray, 
            sizeof(cmd.DcHuffval128BitArray), 
            params->pDCValues, 
            sizeof(cmd.DcHuffval128BitArray)));
        MHW_MI_CHK_STATUS(MOS_SecureMemcpy(
            cmd.AcBits168BitArray, 
            sizeof(cmd.AcBits168BitArray), 
            params->pACBits, 
            sizeof(cmd.AcBits168BitArray)));
        MHW_MI_CHK_STATUS(MOS_SecureMemcpy(
            cmd.AcHuffval1608BitArray, 
            sizeof(cmd.AcHuffval1608BitArray), 
            params->pACValues, 
            sizeof(cmd.AcHuffval1608BitArray)));

        MHW_MI_CHK_STATUS(MOS_SecureMemcpy(
            &cmd.DW52.Value, 
            sizeof(uint16_t), 
            (uint8_t*)params->pACValues + sizeof(cmd.AcHuffval1608BitArray), 
            sizeof(uint16_t)));

        MHW_MI_CHK_STATUS(Mos_AddCommand(cmdBuffer, &cmd, sizeof(cmd)));

        return eStatus;
    }

    MOS_STATUS AddMfxJpegBsdObjCmd(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMHW_VDBOX_JPEG_BSD_PARAMS params)
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        MHW_FUNCTION_ENTER;

        MHW_MI_CHK_NULL(cmdBuffer);
        MHW_MI_CHK_NULL(params);

        typename TMfxCmds::MFD_JPEG_BSD_OBJECT_CMD cmd;

        cmd.DW1.IndirectDataLength = params->dwIndirectDataLength;
        cmd.DW2.IndirectDataStartAddress = params->dwDataStartAddress;
        cmd.DW3.ScanVerticalPosition = params->dwScanVerticalPosition;
        cmd.DW3.ScanHorizontalPosition = params->dwScanHorizontalPosition;
        cmd.DW4.McuCount = params->dwMCUCount;
        cmd.DW4.ScanComponents = params->sScanComponent;
        cmd.DW4.Interleaved = params->bInterleaved;
        cmd.DW5.Restartinterval16Bit = params->dwRestartInterval;

        MHW_MI_CHK_STATUS(Mos_AddCommand(cmdBuffer, &cmd, sizeof(cmd)));

        return eStatus;
    }

    MOS_STATUS AddMfdVp8BsdObjectCmd(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMHW_VDBOX_VP8_BSD_PARAMS params)
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        MHW_FUNCTION_ENTER;

        MHW_MI_CHK_NULL(cmdBuffer);
        MHW_MI_CHK_NULL(params);

        typename TMfxCmds::MFD_VP8_BSD_OBJECT_CMD cmd;
        auto vp8PicParams = params->pVp8PicParams;

        uint8_t numPartitions = (1 << vp8PicParams->CodedCoeffTokenPartition);

        cmd.DW1.CodedNumOfCoeffTokenPartitions = vp8PicParams->CodedCoeffTokenPartition;
        cmd.DW1.Partition0CpbacEntropyRange = vp8PicParams->uiP0EntropyRange;
        cmd.DW1.Partition0CpbacEntropyCount = vp8PicParams->ucP0EntropyCount;
        cmd.DW2.Partition0CpbacEntropyValue = vp8PicParams->ucP0EntropyValue;

        cmd.DW3.IndirectPartition0DataLength = vp8PicParams->uiPartitionSize[0] + 1;
        cmd.DW4.IndirectPartition0DataStartOffset = vp8PicParams->uiFirstMbByteOffset;

        cmd.DW5.IndirectPartition1DataLength = vp8PicParams->uiPartitionSize[1] + 1;
        cmd.DW6.IndirectPartition1DataStartOffset = cmd.DW4.IndirectPartition0DataStartOffset +
            vp8PicParams->uiPartitionSize[0] +
            (numPartitions - 1) * 3;      // Account for P Sizes: 3 bytes per partition
                                            // excluding partition 0 and last partition.

        int32_t i = 2;
        if (i < ((1 + numPartitions)))
        {
            cmd.DW7.IndirectPartition2DataLength = vp8PicParams->uiPartitionSize[i] + 1;
            cmd.DW8.IndirectPartition2DataStartOffset = cmd.DW6.IndirectPartition1DataStartOffset + vp8PicParams->uiPartitionSize[i - 1];
        }

        i = 3;
        if (i < ((1 + numPartitions)))
        {
            cmd.DW9.IndirectPartition3DataLength = vp8PicParams->uiPartitionSize[i] + 1;
            cmd.DW10.IndirectPartition3DataStartOffset = cmd.DW8.IndirectPartition2DataStartOffset + vp8PicParams->uiPartitionSize[i - 1];
        }

        i = 4;
        if (i < ((1 + numPartitions)))
        {
            cmd.DW11.IndirectPartition4DataLength = vp8PicParams->uiPartitionSize[i] + 1;
            cmd.DW12.IndirectPartition4DataStartOffset = cmd.DW10.IndirectPartition3DataStartOffset + vp8PicParams->uiPartitionSize[i - 1];
        }

        i = 5;
        if (i < ((1 + numPartitions)))
        {
            cmd.DW13.IndirectPartition5DataLength = vp8PicParams->uiPartitionSize[i] + 1;
            cmd.DW14.IndirectPartition5DataStartOffset = cmd.DW12.IndirectPartition4DataStartOffset + vp8PicParams->uiPartitionSize[i - 1];
        }

        i = 6;
        if (i < ((1 + numPartitions)))
        {
            cmd.DW15.IndirectPartition6DataLength = vp8PicParams->uiPartitionSize[i] + 1;
            cmd.DW16.IndirectPartition6DataStartOffset = cmd.DW14.IndirectPartition5DataStartOffset + vp8PicParams->uiPartitionSize[i - 1];
        }

        i = 7;
        if (i < ((1 + numPartitions)))
        {
            cmd.DW17.IndirectPartition7DataLength = vp8PicParams->uiPartitionSize[i] + 1;
            cmd.DW18.IndirectPartition7DataStartOffset = cmd.DW16.IndirectPartition6DataStartOffset + vp8PicParams->uiPartitionSize[i - 1];
        }

        i = 8;
        if (i < ((1 + numPartitions)))
        {
            cmd.DW19.IndirectPartition8DataLength = vp8PicParams->uiPartitionSize[i] + 1;
            cmd.DW20.IndirectPartition8DataStartOffset = cmd.DW18.IndirectPartition7DataStartOffset + vp8PicParams->uiPartitionSize[i - 1];
        }

        MHW_MI_CHK_STATUS(Mos_AddCommand(cmdBuffer, &cmd, sizeof(cmd)));

        return eStatus;
    }

public:
    inline uint32_t GetAvcImgStateSize()
    {
        return TMfxCmds::MFX_AVC_IMG_STATE_CMD::byteSize;
    }

};

#endif
