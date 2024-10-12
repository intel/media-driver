/*
* Copyright (c) 2023, Intel Corporation
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
//! \file     oca_rtlog_section_mgr.cpp
//! \brief    OCA rtlog section manager
//!

#include "oca_rtlog_section_mgr.h"
#include "mos_utilities.h"

#define ADDRESS_PAGE_ALIGNMENT_MASK 0xFFFFFFFFFFFFF000ULL

OcaRtLogSectionMgr  OcaRtLogSectionMgr::s_rtLogSectionMgr[MOS_OCA_RTLOG_COMPONENT_MAX] = {};
uint8_t             OcaRtLogSectionMgr::s_localSysMem[MAX_OCA_RT_POOL_SIZE] = {};

OcaRtLogSectionMgr::OcaRtLogSectionMgr()
{
}

OcaRtLogSectionMgr::~OcaRtLogSectionMgr()
{
}

uint8_t *OcaRtLogSectionMgr::GetMemAddress()
{
    static uint8_t *heapAddr = InitSectionMgrAndGetAddress();

    return heapAddr;
}

MOS_STATUS OcaRtLogSectionMgr::InsertRTLog(
    MOS_OCA_RTLOG_COMPONENT_TPYE componentType,
    bool                         isErr,
    int32_t                      id,
    uint32_t                     paramCount,
    const void                   *param)
{
    //Try to init by calling GetMemAddress.
    GetMemAddress();

    if (paramCount != MOS_OCA_RTLOG_MAX_PARAM_COUNT)
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }
    if (componentType < MOS_OCA_RTLOG_COMPONENT_MAX)
    {
        OcaRtLogSectionMgr *insMgr = &s_rtLogSectionMgr[componentType];
    
        if (insMgr->IsInitialized())
        {
            uint64_t index = 0;
            MosUtilities::MosQueryPerformanceCounter(&index);
            MOS_OCA_RTLOG_HEADER header = {};
            header.globalId             = index;
            header.id                   = id;
            header.paramCount           = paramCount;
            MOS_OS_CHK_STATUS_RETURN(insMgr->InsertData(header, (uint8_t *)param));
        }
    }
    return MOS_STATUS_SUCCESS;
}

uint8_t *OcaRtLogSectionMgr::InitSectionMgrAndGetAddress()
{
    uint64_t linearAddress = (uint64_t)s_localSysMem;

    uint8_t  *heapAddr     = (uint8_t *)((linearAddress + MOS_PAGE_SIZE - 1) & ADDRESS_PAGE_ALIGNMENT_MASK);

    uint32_t offset = 0;
    uint32_t initeSize = MAX_OCA_RT_SUB_SIZE;
    for (int i = 0; i < MOS_OCA_RTLOG_COMPONENT_MAX; ++i)
    {
        initeSize = (MOS_OCA_RTLOG_COMPONENT_TPYE)i == MOS_OCA_RTLOG_COMPONENT_COMMON ? MAX_OCA_RT_COMMON_SUB_SIZE : MAX_OCA_RT_SUB_SIZE;
        s_rtLogSectionMgr[i].Init(heapAddr, MAX_OCA_RT_SIZE, initeSize, offset);
        MOS_OCA_RTLOG_SECTION_HEADER sectionHeader = {};
        sectionHeader.magicNum      = MOS_OCA_RTLOG_MAGIC_NUM;
        sectionHeader.componentType = (MOS_OCA_RTLOG_COMPONENT_TPYE)i;
        sectionHeader.freq          = 0;
        MosUtilities::MosQueryPerformanceFrequency(&sectionHeader.freq);
        s_rtLogSectionMgr[i].InsertUid(sectionHeader);
        offset += ((MOS_OCA_RTLOG_COMPONENT_TPYE)i == MOS_OCA_RTLOG_COMPONENT_COMMON ? MAX_OCA_RT_COMMON_SUB_SIZE : MAX_OCA_RT_SUB_SIZE);
    }
    return heapAddr;
}

void OcaRtLogSectionMgr::Init(uint8_t* logSysMem, uint32_t size, uint32_t componentSize, uint32_t offset)
{
    if (logSysMem && size && componentSize)
    {
        m_LockedHeap = logSysMem;
        m_HeapSize   = size;
        m_Offset     = offset;
        m_HeapHandle = 0;
        m_EntryCount = (componentSize - sizeof(MOS_OCA_RTLOG_SECTION_HEADER))/ MOS_OCA_RTLOG_ENTRY_SIZE;
        m_IsInitialized = true;
    }
}

uint32_t OcaRtLogSectionMgr::AllocHeapHandle()
{
    return (uint32_t)MosUtilities::MosAtomicIncrement((int32_t *)&m_HeapHandle);
}

MOS_STATUS OcaRtLogSectionMgr::InsertUid(MOS_OCA_RTLOG_SECTION_HEADER sectionHeader)
{
    if (0 == sectionHeader.magicNum)
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }
    MOS_OS_CHK_STATUS_RETURN(MOS_SecureMemcpy((uint8_t *)m_LockedHeap + m_Offset, sizeof(MOS_OCA_RTLOG_SECTION_HEADER), &sectionHeader, sizeof(MOS_OCA_RTLOG_SECTION_HEADER)));
    m_Offset += sizeof(MOS_OCA_RTLOG_SECTION_HEADER);
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS OcaRtLogSectionMgr::InsertData(MOS_OCA_RTLOG_HEADER header, const void *param)
{
    if (param)
    {
        if (header.paramCount * (sizeof(int32_t) + sizeof(int64_t)) > MOS_OCA_RTLOG_ENTRY_SIZE)
        {
            return MOS_STATUS_NO_SPACE;
        }
        if (0 == m_EntryCount)
        {
            MOS_OS_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
        }
        uint32_t heapHandle = AllocHeapHandle() % m_EntryCount;
        if (heapHandle < m_EntryCount)
        {
            uint8_t *copyAddr = (uint8_t *)m_LockedHeap + m_Offset + heapHandle * MOS_OCA_RTLOG_ENTRY_SIZE;
            MOS_OS_CHK_NULL_RETURN(copyAddr);
            MOS_OS_CHK_STATUS_RETURN(MOS_SecureMemcpy(copyAddr, sizeof(MOS_OCA_RTLOG_HEADER), &header, sizeof(MOS_OCA_RTLOG_HEADER)));
            uint32_t copySize = header.paramCount * (sizeof(int32_t) + sizeof(int64_t));
            MOS_OS_CHK_STATUS_RETURN(MOS_SecureMemcpy(copyAddr + sizeof(MOS_OCA_RTLOG_HEADER), copySize, param, copySize));
        }
    }
    return MOS_STATUS_SUCCESS;
}
