/*
* Copyright (c) 2018-2022, Intel Corporation
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
//! \file     media_packet_next.cpp
//! \brief    Defines the common interface for media media_packet
//! \details  The media media_packet interface is further sub-divided by component,
//!           this file is for the base interface which is shared by all components.
//!

#include "media_packet.h"
#include "media_utils.h"
#include "codec_def_common.h"
#include "media_status_report.h"
#include "mhw_mi.h"
#include "mhw_mi_cmdpar.h"
#include "mhw_mi_itf.h"
#include "null_hardware.h"

MOS_STATUS MediaPacket::StartStatusReportNext(
    uint32_t srType,
    MOS_COMMAND_BUFFER *cmdBuffer)
{
    MOS_STATUS result = MOS_STATUS_SUCCESS;

    if (m_statusReport == nullptr)
    {
        return MOS_STATUS_NULL_POINTER;
    }

    PMOS_RESOURCE osResource = nullptr;
    uint32_t      offset     = 0;

    result = m_statusReport->GetAddress(srType, osResource, offset);

    result = SetStartTagNext(osResource, offset, srType, cmdBuffer);

    MEDIA_CHK_STATUS_RETURN(NullHW::StartPredicateNext(m_osInterface, m_miItf, cmdBuffer));

    return result;
}

MOS_STATUS MediaPacket::UpdateStatusReportNext(
    uint32_t srType,
    MOS_COMMAND_BUFFER *cmdBuffer)
{
    MOS_STATUS result = MOS_STATUS_SUCCESS;

    if (m_statusReport == nullptr)
    {
        return MOS_STATUS_NULL_POINTER;
    }

    PMOS_RESOURCE osResource = nullptr;
    uint32_t      offset     = 0;

    result = m_statusReport->GetAddress(srType, osResource, offset);

    auto count = m_statusReport->GetSubmittedCount();

    auto &par           = m_miItf->MHW_GETPAR_F(MI_STORE_DATA_IMM)();
    par                 = {};
    par.pOsResource      = osResource;
    par.dwResourceOffset = offset;
    par.dwValue          = count + 1;

    MEDIA_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_STORE_DATA_IMM)(cmdBuffer));

    return result;
}

MOS_STATUS MediaPacket::EndStatusReportNext(
    uint32_t srType,
    MOS_COMMAND_BUFFER *cmdBuffer)
{
    MOS_STATUS result = MOS_STATUS_SUCCESS;

    if (m_statusReport == nullptr)
    {
        return MOS_STATUS_NULL_POINTER;
    }

    PMOS_RESOURCE osResource = nullptr;
    uint32_t      offset     = 0;

    MEDIA_CHK_STATUS_RETURN(NullHW::StopPredicateNext(m_osInterface, m_miItf, cmdBuffer));

    result = m_statusReport->GetAddress(srType, osResource, offset);

    result = SetEndTagNext(osResource, offset, srType, cmdBuffer);

    return result;
}

MOS_STATUS MediaPacket::SetStartTagNext(
    MOS_RESOURCE *osResource,
    uint32_t offset,
    uint32_t srType,
    MOS_COMMAND_BUFFER *cmdBuffer)
{
    auto &par           = m_miItf->MHW_GETPAR_F(MI_STORE_DATA_IMM)();
    par                 = {};
    par.pOsResource      = osResource;
    par.dwResourceOffset = offset;
    par.dwValue          = CODECHAL_STATUS_QUERY_START_FLAG;

    MEDIA_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_STORE_DATA_IMM)(cmdBuffer));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MediaPacket::SetEndTagNext(
    MOS_RESOURCE *osResource,
    uint32_t offset,
    uint32_t srType,
    MOS_COMMAND_BUFFER *cmdBuffer)
{
    auto &par           = m_miItf->MHW_GETPAR_F(MI_STORE_DATA_IMM)();
    par                 = {};
    par.pOsResource      = osResource;
    par.dwResourceOffset = offset;
    par.dwValue          = CODECHAL_STATUS_QUERY_END_FLAG;

    MEDIA_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_STORE_DATA_IMM)(cmdBuffer));

    return MOS_STATUS_SUCCESS;
}