/*
* Copyright (c) 2019, Intel Corporation
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
//! \file     decode_marker_packet_g12.cpp
//! \brief    Defines the interface for decode marker sub packet
//!
#include "decode_marker_packet_g12.h"
#include "decode_common_feature_defs.h"

namespace decode
{

DecodeMarkerPktG12::DecodeMarkerPktG12(DecodePipeline *pipeline, CodechalHwInterface *hwInterface)
    : DecodeSubPacket(pipeline, *hwInterface)
{
    m_hwInterface = hwInterface;
}

MOS_STATUS DecodeMarkerPktG12::Init()
{
    DECODE_CHK_NULL(m_pipeline);
    DECODE_CHK_NULL(m_hwInterface);

    m_miInterface = m_hwInterface->GetMiInterface();
    DECODE_CHK_NULL(m_miInterface);

    MediaFeatureManager *featureManager = m_pipeline->GetFeatureManager();
    DECODE_CHK_NULL(featureManager);

    m_marker = dynamic_cast<DecodeMarker*>(
                    featureManager->GetFeature(DecodeFeatureIDs::decodeMarker));
    DECODE_CHK_NULL(m_marker);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS DecodeMarkerPktG12::Prepare()
{
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS DecodeMarkerPktG12::Execute(MOS_COMMAND_BUFFER &cmdBuffer)
{
    if (!m_marker->m_setMarkerEnabled)
    {
        return MOS_STATUS_SUCCESS;
    }

    if (m_pipeline->GetMediaContext()->IsRenderEngineUsed())
    {
        // Send pipe_control to get the timestamp
        MHW_PIPE_CONTROL_PARAMS             pipeControlParams;
        MOS_ZeroMemory(&pipeControlParams, sizeof(pipeControlParams));
        pipeControlParams.presDest          = &m_marker->m_markerBuffer->OsResource;
        pipeControlParams.dwResourceOffset  = 0;
        pipeControlParams.dwPostSyncOp      = MHW_FLUSH_WRITE_TIMESTAMP_REG;
        pipeControlParams.dwFlushMode       = MHW_FLUSH_WRITE_CACHE;

        DECODE_CHK_STATUS(m_miInterface->AddPipeControl(&cmdBuffer, NULL, &pipeControlParams));
    }
    else
    {
        // Send flush_dw to get the timestamp 
        MHW_MI_FLUSH_DW_PARAMS  flushDwParams;
        MOS_ZeroMemory(&flushDwParams, sizeof(flushDwParams));
        flushDwParams.pOsResource           = &m_marker->m_markerBuffer->OsResource;
        flushDwParams.dwResourceOffset      = 0;
        flushDwParams.postSyncOperation     = MHW_FLUSH_WRITE_TIMESTAMP_REG;
        flushDwParams.bQWordEnable          = 1;

        DECODE_CHK_STATUS(m_miInterface->AddMiFlushDwCmd(&cmdBuffer, &flushDwParams));
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS DecodeMarkerPktG12::CalculateCommandSize(uint32_t &commandBufferSize, uint32_t &requestedPatchListSize)
{
    commandBufferSize = 0;
    requestedPatchListSize = 0;
    return MOS_STATUS_SUCCESS;
}

}
