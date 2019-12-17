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
//! \file     media_status_report.cpp
//! \brief    Defines the common interface for media status reporter
//! \details  
//!
#include <algorithm>
#include "media_status_report.h"

MOS_STATUS MediaStatusReport::GetAddress(uint32_t statusReportType, PMOS_RESOURCE &osResource, uint32_t &offset)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    if (m_statusBufAddr == nullptr)
        return MOS_STATUS_NULL_POINTER;

    osResource = m_statusBufAddr[statusReportType].osResource;

    if (statusReportType == 0x50)
    {
        offset = 0;
    }
    else
    {
        offset = m_statusBufAddr[statusReportType].offset + m_statusBufAddr[statusReportType].bufSize * CounterToIndex(m_submittedCount);
    }

    return eStatus;
}

MOS_STATUS MediaStatusReport::GetReport(uint16_t requireNum, void *status)

{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    uint32_t completedCount   = *m_completedCount;
    uint32_t reportedCount    = m_reportedCount;
    uint32_t generatedReportCount   = 0;
    uint32_t reportIndex      = 0;
    bool reverseOrder = (requireNum > 1);

    while (reportedCount != completedCount && generatedReportCount < requireNum){

        // Get reverse order index to temporally fix application get status report size bigger than 2 case. 
        reportIndex = reverseOrder ? CounterToIndex(completedCount + m_reportedCount - reportedCount -1) :
                                     CounterToIndex(reportedCount);
        eStatus     = ParseStatus(((uint8_t*)status + m_sizeOfReport * generatedReportCount), reportIndex);

        reportedCount++;
        generatedReportCount++;
    }
    if (generatedReportCount < requireNum)
    {
        for (auto i = generatedReportCount; i < requireNum; i++)
        {
            eStatus = SetStatus(((uint8_t *)status + m_sizeOfReport * i), reportedCount);
        }
    }

    m_reportedCount = reportedCount;
    return eStatus;

}

MOS_STATUS MediaStatusReport::RegistObserver(MediaStatusReportObserver *observer)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;
    std::vector<MediaStatusReportObserver *>::iterator it;

    it = std::find(m_completeObservers.begin(), m_completeObservers.end(), observer);
    if (it != m_completeObservers.end())
    {
        // the observer already in the vector
        return MOS_STATUS_SUCCESS;
    }

    m_completeObservers.push_back(observer);

    return eStatus;
}

MOS_STATUS MediaStatusReport::UnregistObserver(MediaStatusReportObserver *observer)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;
    std::vector<MediaStatusReportObserver *>::iterator it;

    it = std::find(m_completeObservers.begin(), m_completeObservers.end(), observer);
    if (it == m_completeObservers.end())
    {
        // the observer not in the vector
        return MOS_STATUS_INVALID_PARAMETER;
    }

    m_completeObservers.erase(it);
    return eStatus;
}

MOS_STATUS MediaStatusReport::NotifyObservers(void *mfxStatus, void *rcsStatus, void *statusReport)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;
    std::vector<MediaStatusReportObserver *>::iterator it;

    for (it = m_completeObservers.begin(); it != m_completeObservers.end(); it++)
    {
        eStatus = (*it)->Completed(mfxStatus, rcsStatus, statusReport);
    }

    return eStatus;
}



