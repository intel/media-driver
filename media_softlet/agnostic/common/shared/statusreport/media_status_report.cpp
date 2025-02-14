/*
* Copyright (c) 2018-2021, Intel Corporation
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
#include "mos_os.h"

MediaStatusReport::MediaStatusReport(PMOS_INTERFACE osInterface)
{
    if (osInterface && osInterface->pfnGetUserSettingInstance)
    {
        m_userSettingPtr = osInterface->pfnGetUserSettingInstance(osInterface);
    }
#if (_DEBUG || _RELEASE_INTERNAL)
    ReadUserSettingForDebug(
        m_userSettingPtr,
        m_enableVdboxIdReport,
        __MEDIA_USER_FEATURE_VALUE_ENABLE_VDBOX_ID_REPORT,
        MediaUserSetting::Group::Device);
#endif
}

MOS_STATUS MediaStatusReport::GetAddress(uint32_t statusReportType, PMOS_RESOURCE &osResource, uint32_t &offset)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    if (m_statusBufAddr == nullptr)
        return MOS_STATUS_NULL_POINTER;

    osResource = m_statusBufAddr[statusReportType].osResource;

    if (statusReportType == STATUS_REPORT_GLOBAL_COUNT)
    {
        offset = m_statusBufAddr[statusReportType].offset;
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

    Lock();
    uint32_t completedCount = *m_completedCount;
    uint32_t reportedCount = m_reportedCount;
    uint32_t reportedCountOrigin = m_reportedCount;
    uint32_t availableCount = m_submittedCount - reportedCount;
    uint32_t generatedReportCount = 0;
    uint32_t reportIndex = 0;
    bool reverseOrder = (requireNum > 1);

    while (reportedCount != completedCount && generatedReportCount < requireNum){

        // Get reverse order index to temporally fix application get status report size bigger than 2 case.
        reportIndex = reverseOrder ? CounterToIndex(completedCount + reportedCountOrigin - reportedCount -1) :
                                     CounterToIndex(reportedCount);
        // m_reportedCount is used by component. Need to assign actual index before call ParseStatus
        m_reportedCount = reportIndex;
        eStatus = ParseStatus(((uint8_t*)status + m_sizeOfReport * generatedReportCount), reportIndex);

        reportedCount++;
        generatedReportCount++;
    }

    // update incomplete/unavailable status
    uint32_t updatedCount = reportedCount;
    if (generatedReportCount < requireNum)
    {
        for (auto i = generatedReportCount; i < requireNum; i++)
        {
            eStatus = SetStatus(((uint8_t *)status + m_sizeOfReport * i),
                                CounterToIndex(updatedCount),
                                i >= availableCount);
            updatedCount++;
        }
    }

    m_reportedCount = reportedCount;
    UnLock();

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

    Lock();
    m_completeObservers.push_back(observer);
    UnLock();

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

    Lock();
    m_completeObservers.erase(it);
    UnLock();

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

#if (_DEBUG || _RELEASE_INTERNAL)
void MediaStatusReport::ParseVdboxIdsFromBuf(const uint32_t *csEngineIdRegBuf)
{
    for (auto i = 0; i < csInstanceIdMax; i++)
    {
        CsEngineId csEngineId;
        csEngineId.value = csEngineIdRegBuf[i];
        if (csEngineId.fields.classId == classIdVideoEngine)
        {
            m_usedVdboxIds |= 1 << ((csEngineId.fields.instanceId) << 2);
        }
    }
}

MOS_STATUS MediaStatusReport::ReportUsedVdboxIds()
{
    if (m_enableVdboxIdReport)
    {
        uint32_t reportedUsedID = 0;
        ReadUserSettingForDebug(
            m_userSettingPtr,
            reportedUsedID,
            __MEDIA_USER_FEATURE_VALUE_USED_VDBOX_ID,
            MediaUserSetting::Group::Device,
            0,
            false,
            MEDIA_USER_SETTING_INTERNAL_REPORT);
        ReportUserSettingForDebug(
            m_userSettingPtr,
            __MEDIA_USER_FEATURE_VALUE_USED_VDBOX_ID,
            reportedUsedID | m_usedVdboxIds,
            MediaUserSetting::Group::Device);
    }
    return MOS_STATUS_SUCCESS;
}
#endif