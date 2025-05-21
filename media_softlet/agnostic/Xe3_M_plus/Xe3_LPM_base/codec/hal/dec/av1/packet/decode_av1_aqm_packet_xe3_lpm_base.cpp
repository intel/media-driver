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
//! \file     decode_av1_aqm_packet_xe3_lpm_base.cpp
//! \brief    Defines the interface for av1 decode picture packet
//!
#include "decode_av1_aqm_packet_xe3_lpm_base.h"
#include "codechal_debug.h"
#include "decode_common_feature_defs.h"

#ifdef _DECODE_PROCESSING_SUPPORTED

namespace decode
{
    Av1DecodeAqmPktXe3LpmBase::~Av1DecodeAqmPktXe3LpmBase()
    {
    }

    MOS_STATUS Av1DecodeAqmPktXe3LpmBase::Init()
    {
        DECODE_FUNC_CALL();

        DECODE_CHK_STATUS(Av1DecodeAqmPkt::Init());

        MediaFeatureManager *featureManager = m_pipeline->GetFeatureManager();
        DECODE_CHK_NULL(featureManager);

        m_downSampling = dynamic_cast<Av1DownSamplingFeatureXe3_Lpm_Base *>(
            featureManager->GetFeature(DecodeFeatureIDs::decodeDownSampling));
        DECODE_CHK_NULL(m_downSampling);

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1DecodeAqmPktXe3LpmBase::Execute(MOS_COMMAND_BUFFER &cmdBuffer)
    {
        DECODE_FUNC_CALL();

#ifdef _MEDIA_RESERVED
        if (m_aqmItf && m_downSampling->IsVDAQMHistogramEnabled())
        {
            SETPAR_AND_ADDCMD(AQM_VD_CONTROL_STATE, m_aqmItf, &cmdBuffer);
            SETPAR_AND_ADDCMD(AQM_HIST_STATE, m_aqmItf, &cmdBuffer);
            SETPAR_AND_ADDCMD(AQM_HIST_BUFF_ADDR_STATE, m_aqmItf, &cmdBuffer);
            SETPAR_AND_ADDCMD(AQM_FRAME_START, m_aqmItf, &cmdBuffer);
        }
#endif

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1DecodeAqmPktXe3LpmBase::Flush(MOS_COMMAND_BUFFER &cmdBuffer)
    {
        DECODE_FUNC_CALL();

#ifdef _MEDIA_RESERVED
        if (m_aqmItf && m_downSampling->IsVDAQMHistogramEnabled())
        {
            SETPAR_AND_ADDCMD(AQM_HIST_FLUSH, m_aqmItf, &cmdBuffer);
        }
#endif

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(AQM_HIST_BUFF_ADDR_STATE, Av1DecodeAqmPktXe3LpmBase)
    {
        if (m_downSampling->IsVDAQMHistogramEnabled())
        {
            Av1DecodeAqmPkt::MHW_SETPAR_F(AQM_HIST_BUFF_ADDR_STATE)(params);

            if (m_downSampling->m_histogramBufferU)
            {
                params.AqmUChannelHistogramOutputBuffer = &m_downSampling->m_histogramBufferU->OsResource;
            }
            if (m_downSampling->m_histogramBufferV)
            {
                params.AqmVChannelHistogramOutputBuffer = &m_downSampling->m_histogramBufferV->OsResource;
            }
            params.AqmStatisticsSummaryOutputBuffer = &m_downSampling->m_histogramStatisticsSummary->OsResource;
            params.MetadataStreamoutOutputBuffer    = &m_downSampling->m_histogramMetaDataStreamOut->OsResource;
            params.MetadataStreaminInputBuffer      = &m_downSampling->m_histogramMetaDataStreamIn->OsResource;
        }

        return MOS_STATUS_SUCCESS;
    }

    }  // namespace decode

#endif  //_DECODE_PROCESSING_SUPPORTED