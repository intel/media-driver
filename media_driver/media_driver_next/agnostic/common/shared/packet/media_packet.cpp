/*
* Copyright (c) 2018, Intel Corporation
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
//! \file     media_packet.cpp
//! \brief    Defines the common interface for media media_packet
//! \details  The media media_packet interface is further sub-divided by component,
//!           this file is for the base interface which is shared by all components.
//!

#include "media_packet.h"
#include "media_utils.h"

MOS_STATUS MediaPacket::StartStatusReport(
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

    result = SetStartTag(osResource, offset, srType, cmdBuffer);

    return result;
}

MOS_STATUS MediaPacket::UpdateStatusReport(
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

    MHW_MI_STORE_DATA_PARAMS storeDataParams;
    storeDataParams.pOsResource      = osResource;
    storeDataParams.dwResourceOffset = offset; 
    storeDataParams.dwValue          = count + 1;

    MEDIA_CHK_STATUS_RETURN(m_miInterface->AddMiStoreDataImmCmd(
        cmdBuffer,
        &storeDataParams));

    return result;
}

MOS_STATUS MediaPacket::EndStatusReport(
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

    result = SetEndTag(osResource, offset, srType, cmdBuffer);

    return result;
}

MOS_STATUS MediaPacket::SetStartTag(
    MOS_RESOURCE *osResource,
    uint32_t offset,
    uint32_t srType,
    MOS_COMMAND_BUFFER *cmdBuffer)
{
    MHW_MI_STORE_DATA_PARAMS storeDataParams;
    storeDataParams.pOsResource      = osResource;
    storeDataParams.dwResourceOffset = offset;
    storeDataParams.dwValue          = CODECHAL_STATUS_QUERY_START_FLAG;

    MEDIA_CHK_STATUS_RETURN(m_miInterface->AddMiStoreDataImmCmd(
        cmdBuffer,
        &storeDataParams));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MediaPacket::SetEndTag(
    MOS_RESOURCE *osResource,
    uint32_t offset,
    uint32_t srType,
    MOS_COMMAND_BUFFER *cmdBuffer)
{
    MHW_MI_STORE_DATA_PARAMS storeDataParams;
    storeDataParams.pOsResource      = osResource;
    storeDataParams.dwResourceOffset = offset;
    storeDataParams.dwValue          = CODECHAL_STATUS_QUERY_END_FLAG;

    MEDIA_CHK_STATUS_RETURN(m_miInterface->AddMiStoreDataImmCmd(
        cmdBuffer,
        &storeDataParams));

    return MOS_STATUS_SUCCESS;
}
