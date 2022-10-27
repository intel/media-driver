/*
* Copyright (c) 2022, Intel Corporation
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
//! \file     decode_vp9_packet_front_end.cpp
//! \brief    Defines the interface for vp9 front end decode packet
//!
#include "decode_vp9_packet_front_end.h"
#include "decode_status_report_defs.h"
#include "decode_predication_packet.h"
#include "decode_marker_packet.h"

namespace decode {

Vp9DecodeFrontEndPkt::~Vp9DecodeFrontEndPkt()
{
}

MOS_STATUS Vp9DecodeFrontEndPkt::Init()
{
    DECODE_FUNC_CALL();
    DECODE_CHK_STATUS(Vp9DecodePkt::Init());

    DECODE_CHK_STATUS(m_statusReport->RegistObserver(this));

    DecodeSubPacket* subPacket = m_vp9Pipeline->GetSubPacket(DecodePacketId(m_vp9Pipeline, vp9PictureSubPacketId));
    m_picturePkt = dynamic_cast<Vp9DecodePicPkt*>(subPacket);
    DECODE_CHK_NULL(m_picturePkt);

    subPacket = m_vp9Pipeline->GetSubPacket(DecodePacketId(m_vp9Pipeline, vp9SliceSubPacketId));
    m_slicePkt = dynamic_cast<Vp9DecodeSlcPkt*>(subPacket);
    DECODE_CHK_NULL(m_slicePkt);

    return MOS_STATUS_SUCCESS;
}

bool Vp9DecodeFrontEndPkt::IsPrologRequired()
{
    DECODE_FUNC_CALL();

    return true;
}

MOS_STATUS Vp9DecodeFrontEndPkt::CalculateCommandSize(uint32_t &commandBufferSize, uint32_t &requestedPatchListSize)
{
    DECODE_FUNC_CALL();

    DECODE_CHK_STATUS(CalculateCommandBufferSize(commandBufferSize));
    DECODE_CHK_STATUS(CalculatePatchListSize(requestedPatchListSize));
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9DecodeFrontEndPkt::CalculateCommandBufferSize(uint32_t &commandBufferSize)
{
    DECODE_FUNC_CALL();

    DECODE_CHK_STATUS(m_picturePkt->CalculateCommandSize(m_pictureStatesSize, m_picturePatchListSize));
    DECODE_CHK_STATUS(m_slicePkt->CalculateCommandSize(m_sliceStatesSize, m_slicePatchListSize));
    // VP9 m_numSlices should be 1
    commandBufferSize = m_pictureStatesSize + m_sliceStatesSize * (m_vp9BasicFeature->m_numSlices + 1) +
                        COMMAND_BUFFER_RESERVED_SPACE;
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9DecodeFrontEndPkt::CalculatePatchListSize(uint32_t &requestedPatchListSize)
{
    DECODE_FUNC_CALL();

    if (!m_osInterface->bUsesPatchList)
    {
        requestedPatchListSize = 0;
        return MOS_STATUS_SUCCESS;
    }

    requestedPatchListSize = m_picturePatchListSize +
                             (m_slicePatchListSize * (m_vp9BasicFeature->m_numSlices + 1));
    return MOS_STATUS_SUCCESS;
}
}
