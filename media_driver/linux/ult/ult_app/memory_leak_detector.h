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
#ifndef __MEMORY_LEAK_DETECTOR_H__
#define __MEMORY_LEAK_DETECTOR_H__

#include <map>
#include <vector>
#include "driver_loader.h"

#define LOG_PATH             "./igd_0.log"
#define HLT_PATH             "./igd_0.hlt"
#define MEM_LEAK_REPORT_PATH "./memory_leak_report.txt"

enum MemInfoType
{
    MEM_INFO_TYPE_ALLOC_SYS,
    MEM_INFO_TYPE_ALLOC_GFX,
    MEM_INFO_TYPE_FREE_SYS,
    MEM_INFO_TYPE_FREE_GFX,
    MEM_INFO_TYPE_COUNT
};

struct MemInfo
{
    MemInfoType type;
    std::string ptr;
    std::string functionName;
    std::string filename;
    std::string line;
};

class MemoryLeakDetector
{
public:

    static void Detect(const DriverDllLoader &drvLoader, Platform_t platform);
};

class MemoryLeakDetectorIpl
{
public:

    static MemoryLeakDetectorIpl *GetInstance();

    void Detect(const std::string &logPath);

    void GenerateReport(const std::string &reportPath, const std::string &title) const;

private:

    bool ParseLine(const std::string &line, MemInfo &memInfo) const;
    
    uint32_t GetHitNum(int32_t idx) const;

private:

    static MemoryLeakDetectorIpl *m_instance;
    std::map<uint64_t, int32_t>  m_memPtr;
    std::vector<MemInfo>         m_memInfoTable;
};

#endif // __MEMORY_LEAK_DETECTOR_H__
