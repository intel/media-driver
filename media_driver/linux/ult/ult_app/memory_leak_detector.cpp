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
#include <fstream>
#include "gtest/gtest.h"
#include "memory_leak_detector.h"

using namespace std;

void MemoryLeakDetector::Detect(const DriverDllLoader &drvLoader, Platform_t platform)
{
    static bool delReport = true;
    if (delReport)
    {
        remove(MEM_LEAK_REPORT_PATH);
        delReport = false;
    }

    int32_t memNinjaCnt    = drvLoader.MOS_GetMemNinjaCounter();
    int32_t memNinjaCntGfx = drvLoader.MOS_GetMemNinjaCounterGfx();
    if (memNinjaCnt != 0 || memNinjaCntGfx != 0)
    {
        const ::testing::TestInfo* curTest = ::testing::UnitTest::GetInstance()->current_test_info();
        string title(curTest->test_case_name());
        title = title + "." + curTest->name() + " Platform: " + to_string(platform) + " System memory counter: "
            + to_string(memNinjaCnt) + " Graphic memory counter: "+ to_string(memNinjaCntGfx);

        auto detector = MemoryLeakDetectorIpl::GetInstance();
        detector->Detect(LOG_PATH);
        detector->GenerateReport(MEM_LEAK_REPORT_PATH, title);
        EXPECT_TRUE(false) << "Memory leak detected, system memory counter = " << memNinjaCnt
            << ", graphic memory counter = " << memNinjaCntGfx << ", platform = " << platform << endl;
    }

    remove(LOG_PATH);
    remove(HLT_PATH);
}

MemoryLeakDetectorIpl *MemoryLeakDetectorIpl::m_instance = nullptr;

MemoryLeakDetectorIpl * MemoryLeakDetectorIpl::GetInstance()
{
    if (m_instance == nullptr)
    {
        m_instance = new MemoryLeakDetectorIpl();
    }

    return m_instance;
}

void MemoryLeakDetectorIpl::Detect(const string &logPath)
{
    m_memPtr.clear();
    m_memInfoTable.clear();

    ifstream log(logPath);
    if (!log)
    {
        return;
    }

    string line;
    while(getline(log, line))
    {
        if (line.find("MemNinja leak detection begin") < line.size())
        {
            break;
        }
    }

    MemInfo                          memInfo;
    map<uint64_t, int32_t>::iterator pos;
    while(getline(log, line))
    {
        if (ParseLine(line, memInfo))
        {
            if (memInfo.type == MEM_INFO_TYPE_COUNT)
            {
                break;
            }

            uint64_t ptr = stoull(memInfo.ptr, 0, 16);
            if (memInfo.type == MEM_INFO_TYPE_ALLOC_SYS || memInfo.type == MEM_INFO_TYPE_ALLOC_GFX
                || (pos = m_memPtr.find(ptr)) == m_memPtr.end())
            {
                m_memPtr[ptr] = static_cast<int32_t>(m_memInfoTable.size());
            }
            else
            {
                m_memPtr.erase(pos);
            }

            m_memInfoTable.push_back(memInfo);
        }
    }
}

void MemoryLeakDetectorIpl::GenerateReport(const string &reportPath, const string &title) const
{
    ofstream report(reportPath, ios_base::app);
    string   line;

    report << title << endl;
    for (auto it = m_memPtr.cbegin(); it != m_memPtr.cend(); it++)
    {
        auto idx = it->second;
        line = m_memInfoTable[idx].ptr + ", " + m_memInfoTable[idx].functionName + ", "
            + m_memInfoTable[idx].filename + ", " + m_memInfoTable[idx].line;

        if (m_memInfoTable[idx].type == MEM_INFO_TYPE_ALLOC_SYS || m_memInfoTable[idx].type == MEM_INFO_TYPE_ALLOC_GFX)
        {
            line += ", no free, ";
        }
        else
        {
            line += ", no alloc, ";
        }

        line += to_string(GetHitNum(idx));
        TEST_COUT << "Memory leak: " << line << endl;
        report << line << endl;
    }
    report << endl;
}

bool MemoryLeakDetectorIpl::ParseLine(const string &line, MemInfo &memInfo) const
{
    if (line.find("MemNinjaSysAlloc") < line.size())
    {
        memInfo.type = MEM_INFO_TYPE_ALLOC_SYS;
    }
    else if (line.find("MemNinjaGfxAlloc") < line.size())
    {
        memInfo.type = MEM_INFO_TYPE_ALLOC_GFX;
    }
    else if (line.find("MemNinjaSysFree") < line.size())
    {
        memInfo.type = MEM_INFO_TYPE_FREE_SYS;
    }
    else if (line.find("MemNinjaGfxFree") < line.size())
    {
        memInfo.type = MEM_INFO_TYPE_FREE_GFX;
    }
    else if (line.find("MemNinja leak detection end") < line.size())
    {
        memInfo.type = MEM_INFO_TYPE_COUNT;
        return true;
    }
    else
    {
        return false;
    }
    
    auto beg             =  line.find("memPtr = ");
    beg                  += strlen("memPtr = ");
    auto end             =  line.find(",", beg);
    memInfo.ptr          =  line.substr(beg, end - beg);

    beg                  =  line.find("functionName = \"");
    beg                  += strlen("functionName = \"");
    end                  =  line.find("\"", beg);
    memInfo.functionName =  line.substr(beg, end - beg);

    beg                  =  line.find("filename = \"");
    beg                  += strlen("filename = \"");
    end                  =  line.find("\"", beg);
    memInfo.filename     =  line.substr(beg, end - beg);

    beg                  =  line.find("line = ");
    beg                  += strlen("line = ");
    end                  =  line.find("/", beg);
    memInfo.line         =  line.substr(beg, end - beg);

    return true;
}

uint32_t MemoryLeakDetectorIpl::GetHitNum(int32_t idx) const
{
    uint32_t hitNum = 1;
    for (int32_t i = idx - 1; i >= 0; i--)
    {
        if (m_memInfoTable[i].filename == m_memInfoTable[idx].filename
            && m_memInfoTable[i].line == m_memInfoTable[idx].line)
        {
            ++hitNum;
        }
    }

    return hitNum;
}
