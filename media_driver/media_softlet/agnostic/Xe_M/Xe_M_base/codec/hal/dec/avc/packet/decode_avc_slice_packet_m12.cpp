/*
* Copyright (c) 2020, Intel Corporation
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
//! \file     decode_avc_slice_packet_m12.cpp
//! \brief    Defines the interface for avc decode slice packet for GEN12
//!
#include "codechal_utilities.h"
#include "decode_avc_slice_packet_m12.h"
#include "mhw_vdbox_mfx_g12_X.h"

namespace decode
{

    AvcDecodeSlcPktM12::~AvcDecodeSlcPktM12()
    {
    }

    MOS_STATUS AvcDecodeSlcPktM12::Prepare()
    {
        DECODE_CHK_STATUS(AvcDecodeSlcPktXe_M_Base::Prepare());
        //update frame by frame for first valid slice
        m_firstValidSlice = true;
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AvcDecodeSlcPktM12::Execute(MOS_COMMAND_BUFFER& cmdBuffer, uint32_t slcIdx)
    {
        //AVC Slice Level Commands
        if (m_avcPipeline->IsShortFormat())
        {
            DECODE_CHK_STATUS(AddMfxSliceState(cmdBuffer, slcIdx));
        }
        else
        {
            PCODEC_AVC_SLICE_PARAMS slc = m_avcSliceParams + slcIdx;

            // Add phantom slice command if necessary
            if (m_firstValidSlice && slc->first_mb_in_slice)
            {
                // ensure that slc->first_mb_in_next_slice is always non-zero for this phantom slice
                uint16_t nextStartMbNum = slc->first_mb_in_next_slice;
                uint16_t startMbNum = slc->first_mb_in_slice;
                slc->first_mb_in_slice = 0;
                slc->first_mb_in_next_slice = startMbNum;

                DECODE_CHK_STATUS(AddPhantomSliceCmd(cmdBuffer, slcIdx));

                slc->first_mb_in_slice = startMbNum;
                slc->first_mb_in_next_slice = nextStartMbNum;
                m_firstValidSlice = false;
            }
            else
            {
                m_firstValidSlice = false;
            }

            if (!m_mfxInterface->IsAvcISlice(slc->slice_type))
            {
                DECODE_CHK_STATUS(AddMfxAvcRefIdx(cmdBuffer, slcIdx));
                DECODE_CHK_STATUS(AddMfxAvcWeightOffset(cmdBuffer, slcIdx));
            }
            else if (m_avcBasicFeature->m_useDummyReference && !m_osInterface->bSimIsActive)
            {
                // set dummy reference for I Frame
                MHW_VDBOX_AVC_REF_IDX_PARAMS refIdxParams;
                MOS_ZeroMemory(&refIdxParams, sizeof(MHW_VDBOX_AVC_REF_IDX_PARAMS));
                refIdxParams.bDummyReference = true;
                DECODE_CHK_STATUS(m_mfxInterface->AddMfxAvcRefIdx(&cmdBuffer, nullptr, &refIdxParams));
            }

            DECODE_CHK_STATUS(AddMfxAvcSlice(cmdBuffer, slcIdx));
        }
        DECODE_CHK_STATUS(AddBsdObj(cmdBuffer, slcIdx));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AvcDecodeSlcPktM12::AddPhantomSliceCmd(
        MOS_COMMAND_BUFFER &cmdBuffer, uint32_t slcIdx)
    {
        DECODE_FUNC_CALL();

        MHW_VDBOX_AVC_SLICE_STATE avcSliceState;
        PCODEC_AVC_SLICE_PARAMS slc = m_avcSliceParams + slcIdx;

        if (!m_mfxInterface->IsAvcISlice(slc->slice_type))
        {
            DECODE_CHK_STATUS(AddMfxAvcRefIdx(cmdBuffer, slcIdx));
            DECODE_CHK_STATUS(AddMfxAvcWeightOffset(cmdBuffer, slcIdx));
        }

        DECODE_CHK_STATUS(SetAvcPhantomSliceParams(avcSliceState, slcIdx));
        //both slice state and bsd obj are needed for phantom slice decoding
        DECODE_CHK_STATUS(m_mfxInterface->AddMfxAvcSlice(&cmdBuffer, nullptr, &avcSliceState));
        DECODE_CHK_STATUS(m_mfxInterface->AddMfdAvcBsdObjectCmd(&cmdBuffer, &avcSliceState));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AvcDecodeSlcPktM12::AddMfxSliceState(
        MOS_COMMAND_BUFFER &cmdBuffer, uint32_t slcIdx)
    {
        DECODE_FUNC_CALL();

        MHW_VDBOX_AVC_SLICE_STATE avcSliceState;

        DECODE_CHK_STATUS(SetAvcSliceStateParams(avcSliceState, slcIdx));
        if (!avcSliceState.bLastSlice)
        {
            DECODE_CHK_STATUS(m_mfxInterface->AddMfdAvcSliceAddrCmd(&cmdBuffer, &avcSliceState));
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AvcDecodeSlcPktM12::AddMfxAvcRefIdx(
        MOS_COMMAND_BUFFER &cmdBuffer, uint32_t slcIdx)
    {
        DECODE_FUNC_CALL();

        MHW_VDBOX_AVC_REF_IDX_PARAMS refIdxParams;
        PCODEC_AVC_SLICE_PARAMS slc = m_avcSliceParams + slcIdx;

        SetSliceRefIdxParams(refIdxParams, slcIdx);
        DECODE_CHK_STATUS(m_mfxInterface->AddMfxAvcRefIdx(&cmdBuffer, nullptr, &refIdxParams));
        if (m_mfxInterface->IsAvcBSlice(slc->slice_type))
        {
            refIdxParams.uiList = LIST_1;
            refIdxParams.uiNumRefForList[LIST_1] = slc->num_ref_idx_l1_active_minus1 + 1;
            DECODE_CHK_STATUS(m_mfxInterface->AddMfxAvcRefIdx(&cmdBuffer, nullptr, &refIdxParams));
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AvcDecodeSlcPktM12::AddMfxAvcWeightOffset(
        MOS_COMMAND_BUFFER &cmdBuffer, uint32_t slcIdx)
    {
        DECODE_FUNC_CALL();

        MHW_VDBOX_AVC_WEIGHTOFFSET_PARAMS weightOffsetParams;
        PCODEC_AVC_SLICE_PARAMS slc = m_avcSliceParams + slcIdx;

        if (m_mfxInterface->IsAvcPSlice(slc->slice_type) &&
            m_avcPicParams->pic_fields.weighted_pred_flag == 1)
        {
            SetSliceWeightOffsetParams(weightOffsetParams, slcIdx);
            DECODE_CHK_STATUS(m_mfxInterface->AddMfxAvcWeightOffset(&cmdBuffer, nullptr, &weightOffsetParams));
        }

        if (m_mfxInterface->IsAvcBSlice(slc->slice_type) &&
            m_avcPicParams->pic_fields.weighted_bipred_idc == 1)
        {
            SetSliceWeightOffsetParams(weightOffsetParams, slcIdx);
            DECODE_CHK_STATUS(m_mfxInterface->AddMfxAvcWeightOffset(&cmdBuffer, nullptr, &weightOffsetParams));
            weightOffsetParams.uiList = 1;
            DECODE_CHK_STATUS(m_mfxInterface->AddMfxAvcWeightOffset(&cmdBuffer, nullptr, &weightOffsetParams));
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AvcDecodeSlcPktM12::AddMfxAvcSlice(
        MOS_COMMAND_BUFFER &cmdBuffer, uint32_t slcIdx)
    {
        DECODE_FUNC_CALL();

        MHW_VDBOX_AVC_SLICE_STATE avcSliceState;
        DECODE_CHK_STATUS(SetAvcSliceStateParams(avcSliceState, slcIdx));
        DECODE_CHK_STATUS(m_mfxInterface->AddMfxAvcSlice(&cmdBuffer, nullptr, &avcSliceState));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AvcDecodeSlcPktM12::AddBsdObj(
        MOS_COMMAND_BUFFER &cmdBuffer,
        uint32_t slcIdx)
    {
        DECODE_FUNC_CALL();

        MHW_VDBOX_AVC_SLICE_STATE avcSliceState;
        DECODE_CHK_STATUS(SetAvcSliceStateParams(avcSliceState, slcIdx));
        DECODE_CHK_STATUS(m_mfxInterface->AddMfdAvcBsdObjectCmd(&cmdBuffer, &avcSliceState));

        return MOS_STATUS_SUCCESS;
    }
}
