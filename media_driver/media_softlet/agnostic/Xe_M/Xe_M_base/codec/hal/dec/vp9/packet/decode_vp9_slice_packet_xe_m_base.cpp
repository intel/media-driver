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
//! \file     decode_vp9_slice_packet_xe_m_base.cpp
//! \brief    Defines the interface for vp9 decode slice packet
//!
#include "codechal_utilities.h"
#include "decode_vp9_slice_packet_xe_m_base.h"

namespace decode
{
    MOS_STATUS Vp9DecodeSlcPktXe_M_Base::Init()
    {
        DECODE_FUNC_CALL();

        DECODE_CHK_NULL(m_featureManager);
        DECODE_CHK_NULL(m_hwInterface);
        DECODE_CHK_NULL(m_osInterface);
        DECODE_CHK_NULL(m_miInterface);
        DECODE_CHK_NULL(m_vp9Pipeline);

        m_vp9BasicFeature = dynamic_cast<Vp9BasicFeature*>(m_featureManager->GetFeature(FeatureIDs::basicFeature));
        DECODE_CHK_NULL(m_vp9BasicFeature);

        m_allocator = m_pipeline ->GetDecodeAllocator();
        DECODE_CHK_NULL(m_allocator);

        m_decodecp = m_pipeline->GetDecodeCp();

        DECODE_CHK_STATUS(CalculateSliceStateCommandSize());

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Vp9DecodeSlcPktXe_M_Base::Prepare()
    {
        DECODE_FUNC_CALL();

        DECODE_CHK_NULL(m_vp9BasicFeature->m_vp9PicParams);

        m_vp9PicParams = m_vp9BasicFeature->m_vp9PicParams;
        m_vp9SliceParams = m_vp9BasicFeature->m_vp9SliceParams;

        return MOS_STATUS_SUCCESS;
    }


    MOS_STATUS Vp9DecodeSlcPktXe_M_Base::SetBsdObjParams(
        MHW_VDBOX_HCP_BSD_PARAMS &bsdObjParams,   
        uint32_t                  sliceIdx,
        uint32_t                  subTileIdx)
    {
        bsdObjParams.dwBsdDataLength =
            m_vp9PicParams->BSBytesInBuffer - m_vp9PicParams->UncompressedHeaderLengthInBytes;
        bsdObjParams.dwBsdDataStartOffset = m_vp9PicParams->UncompressedHeaderLengthInBytes;
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Vp9DecodeSlcPktXe_M_Base::AddBsdObj(
        MOS_COMMAND_BUFFER &cmdBuffer,
        uint32_t           sliceIdx,
        uint32_t           subTileIdx)
    {
        MHW_VDBOX_HCP_BSD_PARAMS bsdObjParams;
        MOS_ZeroMemory(&bsdObjParams, sizeof(MHW_VDBOX_HCP_BSD_PARAMS));

        DECODE_CHK_STATUS(SetBsdObjParams(bsdObjParams, sliceIdx, subTileIdx));
        DECODE_CHK_STATUS(m_hcpInterface->AddHcpBsdObjectCmd(&cmdBuffer, &bsdObjParams));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Vp9DecodeSlcPktXe_M_Base::CalculateCommandSize(uint32_t &commandBufferSize,
                                                      uint32_t &requestedPatchListSize)
    {
        DECODE_FUNC_CALL();

        commandBufferSize      = m_sliceStatesSize;
        requestedPatchListSize = m_slicePatchListSize;

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Vp9DecodeSlcPktXe_M_Base::CalculateSliceStateCommandSize()
    {
        DECODE_FUNC_CALL();

        // Slice Level Commands
        DECODE_CHK_STATUS(m_hwInterface->GetHcpPrimitiveCommandSize(m_vp9BasicFeature->m_mode,
            &m_sliceStatesSize,
            &m_slicePatchListSize,
            false));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Vp9DecodeSlcPktXe_M_Base::AddHcpCpState(MOS_COMMAND_BUFFER &cmdBuffer, uint32_t sliceIdx, uint32_t subTileIdx)
    {
        if (m_decodecp)
        {
            DECODE_CHK_STATUS(m_decodecp->AddHcpState(&cmdBuffer,
                &(m_vp9BasicFeature->m_resDataBuffer.OsResource),
                m_vp9PicParams->BSBytesInBuffer - m_vp9PicParams->UncompressedHeaderLengthInBytes,
                m_vp9PicParams->UncompressedHeaderLengthInBytes,
                sliceIdx));
        }
        return MOS_STATUS_SUCCESS;
    }

}
