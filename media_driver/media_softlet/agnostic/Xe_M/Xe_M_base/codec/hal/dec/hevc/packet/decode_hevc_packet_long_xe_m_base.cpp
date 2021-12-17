/*
* Copyright (c) 2020-2021, Intel Corporation
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
//! \file     decode_hevc_packet_long_xe_m_base.cpp
//! \brief    Defines the interface for hevc long format decode packet
//!
#include "codechal_utilities.h"
#include "decode_hevc_packet_long_xe_m_base.h"
#include "decode_status_report_defs.h"
#include "decode_predication_packet.h"
#include "decode_marker_packet.h"

namespace decode {

HevcDecodeLongPktXe_M_Base::~HevcDecodeLongPktXe_M_Base()
{
}

MOS_STATUS HevcDecodeLongPktXe_M_Base::Init()
{
    DECODE_FUNC_CALL();
    DECODE_CHK_STATUS(HevcDecodePktXe_M_Base::Init());

    DECODE_CHK_STATUS(m_statusReport->RegistObserver(this));

    DecodeSubPacket* subPacket = m_hevcPipeline->GetSubPacket(DecodePacketId(m_hevcPipeline, hevcPictureSubPacketId));
    m_picturePkt               = dynamic_cast<HevcDecodePicPktXe_M_Base *>(subPacket);
    DECODE_CHK_NULL(m_picturePkt);

    subPacket = m_hevcPipeline->GetSubPacket(DecodePacketId(m_hevcPipeline, hevcSliceSubPacketId));
    m_slicePkt = dynamic_cast<HevcDecodeSlcPktXe_M_Base *>(subPacket);
    DECODE_CHK_NULL(m_slicePkt);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcDecodeLongPktXe_M_Base::Destroy()
{
    m_statusReport->UnregistObserver(this);
    return MOS_STATUS_SUCCESS;
}

bool HevcDecodeLongPktXe_M_Base::IsPrologRequired()
{
    if (!m_hevcPipeline->IsShortFormat())
    {
        return true;
    }

    if (!m_hevcPipeline->IsSingleTaskPhaseSupported())
    {
        return true;
    }

    return false;
}

MOS_STATUS HevcDecodeLongPktXe_M_Base::CalculateCommandSize(uint32_t &commandBufferSize, uint32_t &requestedPatchListSize)
{
    DECODE_CHK_STATUS(CalculateCommandBufferSize(commandBufferSize));
    DECODE_CHK_STATUS(CalculatePatchListSize(requestedPatchListSize));
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcDecodeLongPktXe_M_Base::CalculateCommandBufferSize(uint32_t &commandBufferSize)
{
    // Just need consider picture level cmds for first level command buffer size,
    // since the slice level cmds are located 2nd lvl batch buffer which is
    // allocated by pipeline.
    DECODE_CHK_STATUS(m_picturePkt->CalculateCommandSize(m_pictureStatesSize, m_picturePatchListSize));
    commandBufferSize = m_pictureStatesSize + COMMAND_BUFFER_RESERVED_SPACE;
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcDecodeLongPktXe_M_Base::CalculatePatchListSize(uint32_t &requestedPatchListSize)
{
    if (!m_osInterface->bUsesPatchList)
    {
        requestedPatchListSize = 0;
        return MOS_STATUS_SUCCESS;
    }

    DECODE_CHK_STATUS(m_slicePkt->CalculateCommandSize(m_sliceStatesSize, m_slicePatchListSize));

    if (m_hevcPipeline->GetDecodeMode() == HevcPipeline::separateTileDecodeMode)
    {
        uint32_t tileNum = (1 + m_hevcBasicFeature->m_hevcPicParams->num_tile_rows_minus1) *
                           (1 + m_hevcBasicFeature->m_hevcPicParams->num_tile_columns_minus1);
        requestedPatchListSize = m_picturePatchListSize +
                                 m_slicePatchListSize * (m_hevcBasicFeature->m_numSlices + tileNum);
    }
    else
    {
        requestedPatchListSize = m_picturePatchListSize +
                                 (m_slicePatchListSize * (m_hevcBasicFeature->m_numSlices + 1));
    }

    return MOS_STATUS_SUCCESS;
}

}
