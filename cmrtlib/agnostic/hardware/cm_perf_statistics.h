/*
* Copyright (c) 2017, Intel Corporation
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
#ifndef CMRTLIB_AGNOSTIC_HARDWARE_CM_PERF_STATISTICS_H_
#define CMRTLIB_AGNOSTIC_HARDWARE_CM_PERF_STATISTICS_H_

#include <vector>
#include <cstdio>
#include "cm_def_hw.h"
#include "cm_include.h"

#if MDF_PROFILER_ENABLED

#define MAX_RECORD_NUM  256
#define MSG_STRING_SIZE 256
#define INIT_ARRAY_ZIE  256

struct ApiPerfStatistic
{
    char  functionName[MSG_STRING_SIZE];       // function name
    float time;                                 // accumulative api duration
    uint32_t callTimes;                           // called times
};

struct ApiCallRecord
{
    char  functionName[MSG_STRING_SIZE];       // function name
    LARGE_INTEGER  startTime;                  // start time
    LARGE_INTEGER  endTime;                    // end time
    float          duration;                    // duration
};

enum PerfLogLevel
{
    CM_RT_PERF_LOG_LEVEL_DEFAULT = 0 , // default level: only dump the statistics results when destorying cm device
    CM_RT_PERF_LOG_LEVEL_ETW     = 1 , // ETW level:     generate etw logs in each call and dump statistics results
    CM_RT_PERF_LOG_LEVEL_RECORDS = 2 , // records each call in m_log_file ;  generate etw logs ; dump statistics results
};

class CmPerfStatistics
{
public:
    CmPerfStatistics();
    ~CmPerfStatistics();

    //!
    //! \brief    Insert API call record 
    //! \details  Insert API call record which contains function name, start time, end and duration.
    //! \param    [in] functionName
    //!           pointer to function name's string
    //! \param    [in] time
    //!           function's duration
    //! \param    [in] start
    //!           function's start time
    //! \param    [in] end
    //!           function's end time
    //!
    void InsertApiCallRecord(char *functionName, float time, LARGE_INTEGER start, LARGE_INTEGER end);

    //!
    //! \brief    Insert API call record into performace statistic record
    //! \details  Insert API call record into performace statistic record.
    //! \param    [in] pRecords
    //!           pointer to API call record
    //!
    void InsertPerfStatistic(ApiCallRecord *records);

    //!
    //! \brief    Check if this profiler on or not
    //! \details  Check if this profiler on or not. 
    //!           So far, it is turned on at Debug and Release-Internal configuration.
    //! \retval   true if profiler is on; false, otherwise
    //!
    bool IsProfilerOn();

private:

    //!
    //! \brief    Check the profiler level
    //! \details  So far, this function turns the profiler on.
    //!
    void GetProfilerLevel();

    //!
    //! \brief    Dump API call records into file
    //! \details  Dump API call records into file, 
    //!           "CmPerfLog.csv" under app's location.
    //!
    void DumpApiCallRecords();

    //!
    //! \brief    Dump API call statistic records into file
    //! \details  Dump API call statistic records into file, 
    //!           "CmPerfStatistics" under app's location.
    //!
    void DumpPerfStatisticRecords();

    CSync           m_criticalSectionOnApiCallRecords;
    uint32_t        m_apiCallRecordCount;
    FILE           *m_apiCallFile;

    CSync           m_criticalSectionOnPerfStatisticRecords;
    FILE           *m_perfStatisticFile;
    uint32_t        m_perfStatisticCount;

    std::vector<ApiCallRecord*>      m_apiCallRecords;  // array to store api call records
    std::vector<ApiPerfStatistic*>   m_perfStatisticRecords; // array to store perf statistic information

    PerfLogLevel m_profilerLevel; // profiler level
    bool m_profilerOn;   // profiler on or off

private:
    CmPerfStatistics(const CmPerfStatistics &other);
    CmPerfStatistics &operator=(const CmPerfStatistics &other);

};

#endif

#endif  // #ifndef CMRTLIB_AGNOSTIC_HARDWARE_CM_PERF_STATISTICS_H_
