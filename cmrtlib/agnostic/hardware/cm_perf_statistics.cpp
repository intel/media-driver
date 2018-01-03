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
#include "cm_perf_statistics.h"
#include "cm_mem.h"
#include "cm_sdk_provider.h"

#if MDF_PROFILER_ENABLED

CmPerfStatistics::CmPerfStatistics()
{

    m_apiCallRecordCount  = 0;
    m_apiCallFile         = nullptr;

    m_perfStatisticFile   = nullptr;
    m_perfStatisticCount = 0;

    m_profilerOn      = false;
    m_profilerLevel    = CM_RT_PERF_LOG_LEVEL_DEFAULT;

    GetProfilerLevel(); // get profiler level from env variable "CM_RT_PERF_LOG"

    if(m_profilerLevel >= CM_RT_PERF_LOG_LEVEL_ETW)
    {   // Register 'MDF Provider' in System
        EventRegisterMDF_PROVIDER();
    }

}

CmPerfStatistics::~CmPerfStatistics()
{
    DumpApiCallRecords();

    DumpPerfStatisticRecords();
}

void CmPerfStatistics::GetProfilerLevel()
{   // Enabled Profiler in Debug Mode
    m_profilerLevel = CM_RT_PERF_LOG_LEVEL_RECORDS;
    m_profilerOn   = true;
    return;
}

//! Insert API Call Records Into m_apiCallRecords Array and Update Perf Statistic Array accordingly
void CmPerfStatistics::InsertApiCallRecord(char *functionName, float time, LARGE_INTEGER start, LARGE_INTEGER end)
{
    CLock locker(m_criticalSectionOnApiCallRecords);

    ApiCallRecord *records = new ApiCallRecord ;

    records->startTime = start;
    records->endTime   = end;
    records->duration   = time;

    CM_STRCPY(records->functionName, MSG_STRING_SIZE, functionName);

    m_apiCallRecords.push_back(records);
    m_apiCallRecordCount++;

    InsertPerfStatistic(records);

}

//Update Perf Statistic Array
void CmPerfStatistics::InsertPerfStatistic(ApiCallRecord *record)
{
    uint32_t index = 0;

    for( index = 0 ; index < m_perfStatisticCount ; index ++)
    {
        ApiPerfStatistic *perfStatisticRecords = m_perfStatisticRecords[index];
        if(perfStatisticRecords != nullptr)
        {
            if(!strcmp(record->functionName, perfStatisticRecords->functionName))
            { // existing
               perfStatisticRecords->callTimes ++ ;
               perfStatisticRecords->time += record->duration;

               //Update statistic records
               m_perfStatisticRecords[index] = perfStatisticRecords;

               break;
            }
        }
    }

    if(index == m_perfStatisticCount)
    { // record does not exist, create new entry
        ApiPerfStatistic *perfStatisticRecords = new ApiPerfStatistic;
        CM_STRCPY(perfStatisticRecords->functionName, MSG_STRING_SIZE, record->functionName);

        perfStatisticRecords->callTimes = 1;
        perfStatisticRecords->time       = record->duration;

        m_perfStatisticRecords.push_back(perfStatisticRecords);
        m_perfStatisticCount ++;
    }

    return ;
}

//Dump APICall Records and Release m_apiCallRecords Array
void CmPerfStatistics::DumpApiCallRecords()
{
    if(!m_profilerOn)
    {
        return ;
    }

    CM_FOPEN(m_apiCallFile, "CmPerfLog.csv", "wb");
    if(! m_apiCallFile )
    {
        fprintf(stdout, "Fail to create file CmPerfLog.csv \n ");
        return ;
    }
    fprintf(m_apiCallFile,  "%-40s %s \t %s \t %s \n", "FunctionName", "StartTime", "EndTime", "Duration");

    for(uint32_t i=0 ; i< m_apiCallRecordCount ; i++)
    {
        ApiCallRecord *records = m_apiCallRecords[i];

        fprintf(m_apiCallFile,  "%-40s  %lld \t %lld \t %fms \n", records->functionName,
           records->startTime.QuadPart, records->endTime.QuadPart, records->duration);

        CmSafeRelease(records);
    }

    m_apiCallRecords.clear();

    fclose(m_apiCallFile);

}

//Dump Perf Statistic Records and Release m_perfStatisticRecords Array
void CmPerfStatistics::DumpPerfStatisticRecords()
{
    if(!m_profilerOn)
    {
        return ;
    }

    CM_FOPEN(m_perfStatisticFile, "CmPerfStatistics.txt","wb");
    if(!m_perfStatisticFile )
    {
        fprintf(stdout, "Fail to create file CmPerfStatistics.txt \n ");
        return ;
    }
    fprintf(m_perfStatisticFile,  "%-40s %s \t %s \n", "FunctionName", "Total Time(ms)", "Called Times");

    for(uint32_t i=0 ; i< m_perfStatisticCount; i++)
    {
        ApiPerfStatistic *perfStatisticRecords = m_perfStatisticRecords[i];

        fprintf(m_perfStatisticFile,  "%-40s %fms \t %d \n", perfStatisticRecords->functionName,
           perfStatisticRecords->time, perfStatisticRecords->callTimes);

        CmSafeRelease(perfStatisticRecords);
    }

    m_perfStatisticRecords.clear();

    fclose(m_perfStatisticFile);

}

bool CmPerfStatistics::IsProfilerOn()
{
    return m_profilerOn;
}

#endif
