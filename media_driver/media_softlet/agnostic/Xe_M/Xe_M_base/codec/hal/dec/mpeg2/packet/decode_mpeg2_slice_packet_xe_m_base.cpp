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
//! \file     decode_mpeg2_slice_packet_xe_m_base.cpp
//! \brief    Defines the interface of mpeg2 decode slice packet for Xe_M_Base
//!
#include "codechal_utilities.h"
#include "decode_mpeg2_slice_packet_xe_m_base.h"

namespace decode {

    MOS_STATUS Mpeg2DecodeSlcPktXe_M_Base::Init()
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

        m_decodecp = m_pipeline->GetDecodeCp();

        DECODE_CHK_STATUS(CalculateSliceStateCommandSize());

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Mpeg2DecodeSlcPktXe_M_Base::Prepare()
    {
        DECODE_FUNC_CALL();

        DECODE_CHK_NULL(m_mpeg2BasicFeature->m_mpeg2PicParams);
        DECODE_CHK_NULL(m_mpeg2BasicFeature->m_mpeg2SliceParams);

        m_mpeg2PicParams = m_mpeg2BasicFeature->m_mpeg2PicParams;

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Mpeg2DecodeSlcPktXe_M_Base::SetMpeg2SliceStateParams(
        MHW_VDBOX_MPEG2_SLICE_STATE& mpeg2SliceState, uint16_t slcIdx)
    {
        DECODE_FUNC_CALL();

        MOS_ZeroMemory(&mpeg2SliceState, sizeof(mpeg2SliceState));
        CodecDecodeMpeg2SliceParams* slc = &m_mpeg2BasicFeature->m_sliceRecord[slcIdx].recordSliceParam;

        mpeg2SliceState.presDataBuffer = &m_mpeg2BasicFeature->m_resDataBuffer.OsResource;
        mpeg2SliceState.wPicWidthInMb = m_mpeg2BasicFeature->m_picWidthInMb;
        mpeg2SliceState.wPicHeightInMb = m_mpeg2BasicFeature->m_picHeightInMb;
        mpeg2SliceState.pMpeg2SliceParams = slc;
        mpeg2SliceState.dwLength = m_mpeg2BasicFeature->m_sliceRecord[slcIdx].length;
        mpeg2SliceState.dwOffset = m_mpeg2BasicFeature->m_sliceRecord[slcIdx].offset;
        mpeg2SliceState.dwSliceStartMbOffset = m_mpeg2BasicFeature->m_sliceRecord[slcIdx].sliceStartMbOffset;
        mpeg2SliceState.bLastSlice = m_mpeg2BasicFeature->m_sliceRecord[slcIdx].isLastSlice;

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Mpeg2DecodeSlcPktXe_M_Base::AddBsdObj(MHW_BATCH_BUFFER& batchBuffer, uint16_t slcIdx)
    {
        DECODE_FUNC_CALL();

        MHW_VDBOX_MPEG2_SLICE_STATE mpeg2SliceState;
        DECODE_CHK_STATUS(SetMpeg2SliceStateParams(mpeg2SliceState, slcIdx));
        DECODE_CHK_STATUS(m_mfxInterface->AddMfdMpeg2BsdObject(
            nullptr,
            &batchBuffer,
            &mpeg2SliceState));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Mpeg2DecodeSlcPktXe_M_Base::InsertDummySlice(
        MHW_BATCH_BUFFER& batchBuffer,
        uint16_t startMB,
        uint16_t endMB)
    {
        DECODE_FUNC_CALL();

        MHW_VDBOX_MPEG2_SLICE_STATE mpeg2SliceState;
        CodecDecodeMpeg2SliceParams slc;
        MOS_ZeroMemory(&mpeg2SliceState, sizeof(mpeg2SliceState));
        MOS_ZeroMemory(&slc, sizeof(CodecDecodeMpeg2SliceParams));

        uint16_t intraVLDFormat = m_mpeg2PicParams->W0.m_intraVlcFormat;
        uint16_t quantizerScaleType = m_mpeg2PicParams->W0.m_quantizerScaleType;
        uint16_t dummySliceIndex = quantizerScaleType * 2 + intraVLDFormat;

        mpeg2SliceState.presDataBuffer = nullptr;
        mpeg2SliceState.wPicWidthInMb = m_mpeg2BasicFeature->m_picWidthInMb;
        mpeg2SliceState.wPicHeightInMb = m_mpeg2BasicFeature->m_picHeightInMb;
        mpeg2SliceState.dwLength = m_mpeg2BasicFeature->Mpeg2DummySliceLengths[dummySliceIndex];
        mpeg2SliceState.dwOffset = m_mpeg2BasicFeature->Mpeg2DummySliceOffsets[dummySliceIndex] +
            m_mpeg2BasicFeature->m_dummySliceDataOffset;
        // force disable cp for dummy slices
        bool isCpEnabled = false;
        if (m_decodecp)
        {
            isCpEnabled = m_decodecp->IsCpEnabled();
            if (isCpEnabled)
            {
                m_decodecp->SetCpEnabled(false);
            }
        }

        bool isLastSlice = false;
        uint16_t expectedEndMB = m_mpeg2BasicFeature->m_picWidthInMb * m_mpeg2BasicFeature->m_picHeightInMb;

        while (startMB < endMB)
        {
            slc.m_macroblockOffset = 6;
            slc.m_sliceHorizontalPosition = startMB % m_mpeg2BasicFeature->m_picWidthInMb;
            slc.m_sliceVerticalPosition = startMB / m_mpeg2BasicFeature->m_picWidthInMb;
            slc.m_quantiserScaleCode = 10;
            slc.m_numMbsForSlice = 1;

            isLastSlice = ((startMB + 1) == expectedEndMB);

            mpeg2SliceState.pMpeg2SliceParams = &slc;
            mpeg2SliceState.dwSliceStartMbOffset = startMB;
            mpeg2SliceState.bLastSlice = isLastSlice;

            DECODE_CHK_STATUS(m_mfxInterface->AddMfdMpeg2BsdObject(
                nullptr,
                &batchBuffer,
                &mpeg2SliceState));

            startMB++;
        }

        // restore Cp state
        if (m_decodecp && isCpEnabled)
        {
            m_decodecp->SetCpEnabled(true);
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Mpeg2DecodeSlcPktXe_M_Base::CalculateCommandSize(uint32_t& commandBufferSize,
        uint32_t& requestedPatchListSize)
    {
        DECODE_FUNC_CALL();

        commandBufferSize = m_sliceStatesSize;
        requestedPatchListSize = m_slicePatchListSize;

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Mpeg2DecodeSlcPktXe_M_Base::CalculateSliceStateCommandSize()
    {
        DECODE_FUNC_CALL();

        // Slice Level Commands
        DECODE_CHK_STATUS(m_hwInterface->GetMfxPrimitiveCommandsDataSize(
            m_mpeg2BasicFeature->m_mode,
            &m_sliceStatesSize,
            &m_slicePatchListSize,
            0));

        return MOS_STATUS_SUCCESS;
    }

}  // namespace decode
