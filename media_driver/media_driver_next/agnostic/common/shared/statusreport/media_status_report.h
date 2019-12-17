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
//! \file     media_status_report.h
//! \brief    Defines the class for media status report
//! \details  
//!
#ifndef __MEDIA_STATUS_REPORT_H__
#define __MEDIA_STATUS_REPORT_H__

#include "mos_os_specific.h"
#include "media_status_report_observer.h"

class MediaStatusReport
{
public:

    typedef enum
    {
        querySkipped = 0x00,
        queryStart   = 0x01,
        queryEnd     = 0xFF
    } ExecutingStatus;

    struct StatusBufAddr
    {
        MOS_RESOURCE *osResource;
        uint32_t     offset;
        uint32_t     bufSize;
    };

    //!
    //! \brief  Constructor
    //!
    MediaStatusReport() {};
    virtual ~MediaStatusReport() {};

    //!
    //! \brief  Create resources for status report and do initialization
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Create() = 0;
    //!
    //! \brief  Initialize the status in report for each item
    //! 
    //! \details Called per frame for normal usages.
    //!          It can be called per tilerow if needed.
    //!
    //! \param  [in] inputPar
    //!         Pointer to parameters pass to status report.
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Init(void *inputPar) = 0;
    //!
    //! \brief  Reset Status
    //! 
    //! \details Called per frame for normal usages.
    //!          It can be called per tilerow if needed.
    //!
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Reset() = 0;
    //!
    //! \brief  The entry to get status report.
    //! \param  [in] numStatus
    //!         The requested number of status reports
    //! \param  [out] status
    //!         The point to encode status
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    //!
    MOS_STATUS GetReport(uint16_t numStatus, void *status);
    //!
    //! \brief  Get address of status report.
    //! \param  [in] statusReportType
    //!         status report item type
    //! \param  [out] pOsResource
    //!         The point to PMOS_RESOURCE of each item
    //! \param  [out] offset
    //!         Offset of each item
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    //!
    MOS_STATUS GetAddress(uint32_t statusReportType, PMOS_RESOURCE &osResource, uint32_t &offset);
    //!
    //! \brief  Get submitted count of status report.
    //! \return m_submittedCount
    //!
    uint32_t GetSubmittedCount() const { return m_submittedCount; }

    //!
    //! \brief  Get completed count of status report.
    //! \return The content of m_completedCount
    //!
    uint32_t GetCompletedCount() const 
    {
        if (m_completedCount == nullptr)
        {
            return 0;
        } 
        return (*m_completedCount); 
    }

    //!
    //! \brief  Get reported count of status report.
    //! \return m_reportedCount
    //!
    uint32_t GetReportedCount() const { return m_reportedCount; }

    uint32_t GetIndex(uint32_t count) { return CounterToIndex(count); }
    //!
    //! \brief  Regist observer of complete event.
    //! \param  [in] observer
    //!         The point to StatusReportObserver who will observe the complete event
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS RegistObserver(MediaStatusReportObserver *observer);

    //!
    //! \brief  Unregist observer of complete event.
    //! \param  [in] observer
    //!         The point to StatusReportObserver
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS UnregistObserver(MediaStatusReportObserver *observer);

protected:
    //!
    //! \brief  Collect the status report information into report buffer.
    //! \param  [in] report
    //!         The report buffer address provided by DDI.
    //! \param  [in] index
    //!         The index of current requesting report.
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS ParseStatus(void *report, uint32_t index) = 0;

    //!
    //! \brief  Set unavailable status report information into report buffer.
    //! \param  [in] report
    //!         The report buffer address provided by DDI.
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetStatus(void *report, uint32_t index) = 0;
    //!
    //! \brief  Notify observers that the frame has been completed.
    //! \param  [in] statusBuffer
    //!         The point to status buffer
    //! \param  [in,out] statusReport
    //!         The point to status report
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS NotifyObservers(void *mfxStatus, void *rcsStatus, void *statusReport);

    inline uint32_t CounterToIndex(uint32_t counter)
    {
        return counter & (m_statusNum - 1);
    }

    static const uint32_t m_statusNum        = 512;

    PMOS_RESOURCE    m_completedCountBuf     = nullptr;
    uint32_t         *m_completedCount       = nullptr;
    uint32_t         m_submittedCount        = 0;
    uint32_t         m_reportedCount         = 0;
    uint32_t         m_sizeOfReport          = 0;

    StatusBufAddr    *m_statusBufAddr        = nullptr;

    std::vector<MediaStatusReportObserver *>  m_completeObservers;
};

#endif // !__MEDIA_STATUS_REPORT_H__
