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
//! \file     decode_avc_slice_xe_m_base_packet.cpp
//! \brief    Defines the interface for avc decode slice packet
//!
#include "codechal_utilities.h"
#include "decode_avc_slice_xe_m_base_packet.h"

namespace decode
{
    MOS_STATUS AvcDecodeSlcPktXe_M_Base::Init()
    {
        DECODE_FUNC_CALL();

        DECODE_CHK_NULL(m_featureManager);
        DECODE_CHK_NULL(m_hwInterface);
        DECODE_CHK_NULL(m_osInterface);
        DECODE_CHK_NULL(m_miInterface);
        DECODE_CHK_NULL(m_avcPipeline);
        DECODE_CHK_NULL(m_mfxInterface);

        m_avcBasicFeature = dynamic_cast<AvcBasicFeature*>(m_featureManager->GetFeature(FeatureIDs::basicFeature));
        DECODE_CHK_NULL(m_avcBasicFeature);

        m_allocator = m_pipeline ->GetDecodeAllocator();
        DECODE_CHK_NULL(m_allocator);

        DECODE_CHK_STATUS(CalculateSliceStateCommandSize());

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AvcDecodeSlcPktXe_M_Base::Prepare()
    {
        DECODE_FUNC_CALL();

        DECODE_CHK_NULL(m_avcBasicFeature->m_avcPicParams);
        DECODE_CHK_NULL(m_avcBasicFeature->m_avcSliceParams);

        m_avcPicParams = m_avcBasicFeature->m_avcPicParams;
        m_avcSliceParams = m_avcBasicFeature->m_avcSliceParams;

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AvcDecodeSlcPktXe_M_Base::SetAvcSliceStateParams(
        MHW_VDBOX_AVC_SLICE_STATE &avcSliceState, uint32_t slcIdx)
    {
        DECODE_FUNC_CALL();

        MOS_ZeroMemory(&avcSliceState, sizeof(avcSliceState));

        PCODEC_AVC_SLICE_PARAMS slc = m_avcSliceParams + slcIdx;
        uint32_t nextLength = 0;
        uint32_t nextOffset = 0;

        if (slcIdx < m_avcBasicFeature->m_lastValidSlice)
        {
            nextLength = (slc + 1)->slice_data_size;
            nextOffset = (slc + 1)->slice_data_offset;
        }

        avcSliceState.bIntelEntrypointInUse = m_avcPipeline->m_intelEntrypointInUse;
        avcSliceState.bPicIdRemappingInUse  = m_avcBasicFeature->m_picIdRemappingInUse;
        avcSliceState.bShortFormatInUse     = m_avcPipeline->IsShortFormat();
        avcSliceState.presDataBuffer        = &m_avcBasicFeature->m_resDataBuffer.OsResource;
        avcSliceState.pAvcPicParams         = m_avcPicParams;
        avcSliceState.pMvcExtPicParams      = m_avcBasicFeature->m_mvcExtPicParams;
        avcSliceState.pAvcPicIdx            = &m_avcBasicFeature->m_refFrames.m_avcPicIdx[0];
        avcSliceState.bPhantomSlice         = false;
        avcSliceState.ucDisableDeblockingFilterIdc = slc->disable_deblocking_filter_idc;
        avcSliceState.ucSliceBetaOffsetDiv2    = slc->slice_beta_offset_div2;
        avcSliceState.ucSliceAlphaC0OffsetDiv2 = slc->slice_alpha_c0_offset_div2;
        avcSliceState.pAvcSliceParams = slc;
        avcSliceState.dwOffset        = m_avcBasicFeature->m_sliceRecord[slcIdx].offset;
        avcSliceState.dwLength        = m_avcBasicFeature->m_sliceRecord[slcIdx].length;
        avcSliceState.dwNextOffset    = nextOffset;
        avcSliceState.dwNextLength    = nextLength;
        avcSliceState.dwSliceIndex    = slcIdx;
        avcSliceState.bLastSlice      = (slcIdx == m_avcBasicFeature->m_lastValidSlice);
        avcSliceState.bFullFrameData  = m_avcBasicFeature->m_fullFrameData;

        if (slcIdx > 0)
        {
            avcSliceState.dwTotalBytesConsumed = m_avcBasicFeature->m_sliceRecord[slcIdx - 1].totalBytesConsumed;
        }
        else
        {
            avcSliceState.dwTotalBytesConsumed = 0;
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AvcDecodeSlcPktXe_M_Base::SetAvcPhantomSliceParams(
        MHW_VDBOX_AVC_SLICE_STATE &avcSliceState, uint32_t slcIdx)
    {
        DECODE_FUNC_CALL();

        PCODEC_AVC_SLICE_PARAMS slc = m_avcSliceParams + slcIdx;
        MOS_ZeroMemory(&avcSliceState, sizeof(avcSliceState));

        avcSliceState.ucDisableDeblockingFilterIdc = slc->disable_deblocking_filter_idc;
        avcSliceState.ucSliceBetaOffsetDiv2    = slc->slice_beta_offset_div2;
        avcSliceState.ucSliceAlphaC0OffsetDiv2 = slc->slice_alpha_c0_offset_div2;
        avcSliceState.bIntelEntrypointInUse = m_avcPipeline->m_intelEntrypointInUse;
        avcSliceState.bPicIdRemappingInUse  = m_avcBasicFeature->m_picIdRemappingInUse;
        avcSliceState.bShortFormatInUse     = false;
        avcSliceState.presDataBuffer        = &m_avcBasicFeature->m_resDataBuffer.OsResource;
        avcSliceState.pAvcPicParams         = m_avcPicParams;
        avcSliceState.pMvcExtPicParams      = m_avcBasicFeature->m_mvcExtPicParams;
        avcSliceState.pAvcPicIdx            = &m_avcBasicFeature->m_refFrames.m_avcPicIdx[0];
        avcSliceState.bPhantomSlice         = true;
        avcSliceState.dwTotalBytesConsumed  = 0;
        avcSliceState.pAvcSliceParams       = slc;

        avcSliceState.dwOffset        = 0;
        avcSliceState.dwLength        = slc->slice_data_offset;
        avcSliceState.dwNextOffset    = slc->slice_data_offset;
        avcSliceState.dwNextLength    = slc->slice_data_size;

        return MOS_STATUS_SUCCESS;
    }

    void AvcDecodeSlcPktXe_M_Base::SetSliceWeightOffsetParams(
        MHW_VDBOX_AVC_WEIGHTOFFSET_PARAMS &weightOffsetParams,
        uint32_t slcIdx)
    {
        DECODE_FUNC_CALL();

        PCODEC_AVC_SLICE_PARAMS slc = m_avcSliceParams + slcIdx;
        MOS_ZeroMemory(&weightOffsetParams, sizeof(weightOffsetParams));

        weightOffsetParams.uiList = 0;
        MOS_SecureMemcpy(
            &weightOffsetParams.Weights,
            sizeof(weightOffsetParams.Weights),
            &slc->Weights,
            sizeof(slc->Weights));
    }

    void AvcDecodeSlcPktXe_M_Base::SetSliceRefIdxParams(
        MHW_VDBOX_AVC_REF_IDX_PARAMS &refIdxParams,
        uint32_t slcIdx)
    {
        DECODE_FUNC_CALL();

        PCODEC_AVC_SLICE_PARAMS slc = m_avcSliceParams + slcIdx;
        MOS_ZeroMemory(&refIdxParams, sizeof(refIdxParams));

        refIdxParams.CurrPic = m_avcPicParams->CurrPic;
        refIdxParams.uiList = LIST_0;
        refIdxParams.uiNumRefForList[LIST_0] = slc->num_ref_idx_l0_active_minus1 + 1;

        MOS_SecureMemcpy(
            &refIdxParams.RefPicList,
            sizeof(refIdxParams.RefPicList),
            &slc->RefPicList,
            sizeof(slc->RefPicList));

        refIdxParams.pAvcPicIdx = &m_avcBasicFeature->m_refFrames.m_avcPicIdx[0];
        refIdxParams.avcRefList = (void**)m_avcBasicFeature->m_refFrames.m_refList;
        refIdxParams.bIntelEntrypointInUse = m_avcPipeline->m_intelEntrypointInUse;
        refIdxParams.bPicIdRemappingInUse = m_avcBasicFeature->m_picIdRemappingInUse;
    }

    MOS_STATUS AvcDecodeSlcPktXe_M_Base::CalculateCommandSize(uint32_t &commandBufferSize,
                                                      uint32_t &requestedPatchListSize)
    {
        DECODE_FUNC_CALL();

        commandBufferSize      = m_sliceStatesSize;
        requestedPatchListSize = m_slicePatchListSize;

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AvcDecodeSlcPktXe_M_Base::CalculateSliceStateCommandSize()
    {
        DECODE_FUNC_CALL();

        // Slice Level Commands
        DECODE_CHK_STATUS(static_cast<CodechalHwInterfaceG12*>(m_hwInterface)->GetMfxPrimitiveCommandsDataSize(
                                                                    CODECHAL_DECODE_MODE_AVCVLD,
                                                                    &m_sliceStatesSize,
                                                                    &m_slicePatchListSize,
                                                                    m_avcPipeline->IsShortFormat()));

        return MOS_STATUS_SUCCESS;
    }

}
