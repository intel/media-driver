/*
* Copyright (c) 2025, Intel Corporation
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
//! \file     decode_hevc_debug_packet.cpp
//! \brief    Defines the interface for HEVC decode debug sub packet
//!

#include "decode_hevc_debug_packet.h"
#include "decode_common_feature_defs.h"

#if (_DEBUG || _RELEASE_INTERNAL)

namespace decode
{

HevcDecodeDebugPkt::HevcDecodeDebugPkt(DecodePipeline *pipeline, CodechalHwInterfaceNext *hwInterface)
    : DecodeSubPacket(pipeline, hwInterface)
{
    m_hwInterface = hwInterface;
    m_pipeline = pipeline;
}

MOS_STATUS HevcDecodeDebugPkt::Init()
{
    DECODE_FUNC_CALL();

    DECODE_CHK_NULL(m_pipeline);
    DECODE_CHK_NULL(m_hwInterface);

    m_miItf = m_hwInterface->GetMiInterfaceNext();
    DECODE_CHK_NULL(m_miItf);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcDecodeDebugPkt::Prepare()
{
    DECODE_FUNC_CALL();

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcDecodeDebugPkt::CalculateCommandSize(uint32_t &commandBufferSize, uint32_t &requestedPatchListSize)
{
    DECODE_FUNC_CALL();

    commandBufferSize = 0;
    requestedPatchListSize = 0;
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcDecodeDebugPkt::Destroy()
{
    DECODE_FUNC_CALL();

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcDecodeDebugPkt::Execute(MOS_COMMAND_BUFFER& cmdBuffer, MediaStatusReport *statusReport)
{
    DECODE_FUNC_CALL();

    DECODE_CHK_NULL(statusReport);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcDecodeDebugPkt::Completed(void *mfxStatus)
{
    DECODE_FUNC_CALL();

    DECODE_CHK_NULL(mfxStatus);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcDecodeDebugPkt::AddCommandCounterCmds(MOS_COMMAND_BUFFER& cmdBuffer, MediaStatusReport *statusReport)
{
    DECODE_FUNC_CALL();

    return MOS_STATUS_SUCCESS;
}

}  // namespace decode

#endif  // _DEBUG || _RELEASE_INTERNAL