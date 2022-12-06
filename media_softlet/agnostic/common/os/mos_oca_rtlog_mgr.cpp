/*
* Copyright (c) 2022, Intel Corporation
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
//! \file     mos_oca_rtlog_mgr.cpp
//! \brief    OCA buffer manager class
//!

#include "mos_oca_rtlog_mgr.h"
#include "mos_context_specific_next.h"

#define ADDRESS_PAGE_ALIGNMENT_MASK 0xFFFFFFFFFFFFF000ULL

bool MosOcaRTLogMgr::m_enableOcaRTLog = true;
uint8_t MosOcaRTLogMgr::s_localSysMem[MAX_OCA_RT_POOL_SIZE] = {};
MosMutex MosOcaRTLogMgr::s_ocaMutex;

/****************************************************************************************************/
/*                                      MosOcaRtLogSectionMgr                                       */
/****************************************************************************************************/
MosOcaRtLogSectionMgr::MosOcaRtLogSectionMgr()
{
}

void MosOcaRtLogSectionMgr::Init(uint8_t* logSysMem, uint32_t size, uint32_t componentSize, uint32_t offset)
{
    if (logSysMem && size && componentSize)
    {
        m_LockedHeap = logSysMem;
        m_HeapSize   = size;
        m_Offset     = offset;
        m_HeapHandle = -1;
        m_EntryCount = (MAX_OCA_RT_SUB_SIZE - sizeof(MOS_OCA_RTLOG_SECTION_HEADER))/ MOS_OCA_RTLOG_ENTRY_SIZE;

        m_IsInitialized = true;
    }
}

MosOcaRtLogSectionMgr::~MosOcaRtLogSectionMgr()
{
    m_LockedHeap    = nullptr;
    m_HeapSize      = 0;
    m_Offset        = 0;
    m_HeapHandle    = -1;
    m_IsInitialized = false;
}

int32_t MosOcaRtLogSectionMgr::AllocHeapHandle()
{
    return MosUtilities::MosAtomicIncrement(&m_HeapHandle);
}

MOS_STATUS MosOcaRtLogSectionMgr::InsertUid(MOS_OCA_RTLOG_SECTION_HEADER sectionHeader)
{
    if (0 == sectionHeader.magicNum)
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }
    MOS_OS_CHK_STATUS_RETURN(MOS_SecureMemcpy((uint8_t *)m_LockedHeap + m_Offset, sizeof(MOS_OCA_RTLOG_SECTION_HEADER), &sectionHeader, sizeof(MOS_OCA_RTLOG_SECTION_HEADER)));
    m_Offset += sizeof(MOS_OCA_RTLOG_SECTION_HEADER);
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosOcaRtLogSectionMgr::InsertData(MOS_OCA_RTLOG_HEADER header, const void *param)
{
    if (param)
    {
        if (header.paramCount * (sizeof(int32_t) + sizeof(int64_t)) > MOS_OCA_RTLOG_ENTRY_SIZE)
        {
            return MOS_STATUS_NO_SPACE;
        }
        int32_t heapHandle = AllocHeapHandle()%m_EntryCount;
        if (heapHandle < m_EntryCount)
        {
            uint8_t *copyAddr = (uint8_t *)m_LockedHeap + m_Offset + heapHandle * MOS_OCA_RTLOG_ENTRY_SIZE;
            MOS_OS_CHK_STATUS_RETURN(MOS_SecureMemcpy(copyAddr, sizeof(MOS_OCA_RTLOG_HEADER), &header, sizeof(MOS_OCA_RTLOG_HEADER)));
            uint32_t copySize = header.paramCount * (sizeof(int32_t) + sizeof(int64_t));
            MOS_OS_CHK_STATUS_RETURN(MOS_SecureMemcpy(copyAddr + sizeof(MOS_OCA_RTLOG_HEADER), copySize, param, copySize));
        }
    }
    return MOS_STATUS_SUCCESS;
}

/****************************************************************************************************/
/*                                      MosOcaRTLogMgr                                              */
/****************************************************************************************************/

MOS_STATUS MosOcaRTLogMgr::RegisterCtx(OsContextNext *osDriverContext, MOS_CONTEXT *osContext)
{
    MOS_OCA_RTLOG_RES_AND_INTERFACE resInterface = {};
    MOS_OS_CHK_STATUS_RETURN(RegisterRes(osDriverContext, &resInterface, osContext));
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosOcaRTLogMgr::UnRegisterCtx(OsContextNext *osDriverContext)
{
    if (!osDriverContext->GetOcaRTLogResource())
    {
        return MOS_STATUS_NULL_POINTER;
    }
    UnregisterRes(osDriverContext);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosOcaRTLogMgr::RegisterRes(OsContextNext *osDriverContext, MOS_OCA_RTLOG_RES_AND_INTERFACE *resInterface, MOS_CONTEXT *osContext)
{
    if (osDriverContext->GetOcaRTLogResource())
    {
        return MOS_STATUS_SUCCESS;
    }

    resInterface->osInterface = (PMOS_INTERFACE)MOS_AllocAndZeroMemory(sizeof(MOS_INTERFACE));
    MOS_OS_CHK_NULL_RETURN(resInterface->osInterface);
    MOS_STATUS status         = Mos_InitInterface(resInterface->osInterface, osContext, COMPONENT_OCA);
    if (MOS_FAILED(status))
    {
        MOS_SafeFreeMemory(resInterface->osInterface);
        MOS_OS_CHK_STATUS_RETURN(status);
    }
    
    MOS_ALLOC_GFXRES_PARAMS sParams = {};
    sParams.Type                    = MOS_GFXRES_BUFFER;
    sParams.dwBytes                 = MAX_OCA_RT_SIZE;
    sParams.pSystemMemory           = (void *)m_heapAddr;
    sParams.TileType                = MOS_TILE_LINEAR;
    sParams.Format                  = Format_Buffer;
    sParams.pBufName                = "OcaRtlog";
    sParams.bIsPersistent           = 1;
    resInterface->ocaRTLogResource  = (PMOS_RESOURCE)MOS_AllocAndZeroMemory(sizeof(MOS_RESOURCE));
    if (nullptr == resInterface->ocaRTLogResource)
    {
        Mos_DestroyInterface(resInterface->osInterface);
        MOS_SafeFreeMemory(resInterface->osInterface);
        MOS_OS_CHK_NULL_RETURN(resInterface->ocaRTLogResource);
    }

    // Allocate resource
    status = resInterface->osInterface->pfnAllocateResource(resInterface->osInterface, &sParams, resInterface->ocaRTLogResource);
    if (MOS_FAILED(status))
    {
        MOS_SafeFreeMemory(resInterface->ocaRTLogResource);
        Mos_DestroyInterface(resInterface->osInterface);
        MOS_SafeFreeMemory(resInterface->osInterface);
        MOS_OS_CHK_STATUS_RETURN(status);
    }
    status = MapGfxVa(resInterface->ocaRTLogResource, osDriverContext);
    if (MOS_FAILED(status))
    {
        resInterface->osInterface->pfnFreeResource(resInterface->osInterface, resInterface->ocaRTLogResource);
        MOS_SafeFreeMemory(resInterface->ocaRTLogResource);
        Mos_DestroyInterface(resInterface->osInterface);
        MOS_SafeFreeMemory(resInterface->osInterface);
        MOS_OS_CHK_STATUS_RETURN(status);
    }
    s_ocaMutex.Lock();
    m_resMap.insert(std::make_pair(osDriverContext, *resInterface));
    s_ocaMutex.Unlock();
    osDriverContext->SetRtLogRes(resInterface->ocaRTLogResource);
    return MOS_STATUS_SUCCESS;
}

void MosOcaRTLogMgr::UnregisterRes(OsContextNext *osDriverContext)
{
    MOS_OCA_RTLOG_RES_AND_INTERFACE resInterface = {};
    auto iter = m_resMap.find(osDriverContext);
    if (iter != m_resMap.end())
    {
        resInterface = iter->second;
    }
    s_ocaMutex.Lock();
    m_resMap.erase(osDriverContext);
    s_ocaMutex.Unlock();
    resInterface.osInterface->pfnFreeResource(resInterface.osInterface, resInterface.ocaRTLogResource);
    MOS_SafeFreeMemory(resInterface.ocaRTLogResource);
    Mos_DestroyInterface(resInterface.osInterface);
    MOS_SafeFreeMemory(resInterface.osInterface);
}

MOS_STATUS MosOcaRTLogMgr::InsertRTLog(MOS_OCA_RTLOG_COMPONENT_TPYE componentType, bool isErr, int32_t id, uint32_t paramCount, const void *param)
{
    if (!m_isMgrInitialized)
    {
        return MOS_STATUS_UNIMPLEMENTED;
    }

    if (paramCount != MOS_OCA_RTLOG_MAX_PARAM_COUNT)
    {
        return MOS_STATUS_UNIMPLEMENTED;
    }
    MosOcaRtLogSectionMgr *insMgr = nullptr;

    if (componentType < MOS_OCA_RTLOG_COMPONENT_MAX)
    {
        insMgr = &m_rtLogSectionMgr[componentType];
    
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

MosOcaRTLogMgr::MosOcaRTLogMgr()
{
    uint64_t linearAddress        = (uint64_t)s_localSysMem;
    uint64_t linearAddressAligned = 0;

    linearAddressAligned = ((linearAddress + MOS_PAGE_SIZE - 1) & ADDRESS_PAGE_ALIGNMENT_MASK);

    m_heapAddr             = (uint8_t *)linearAddressAligned;
    uint32_t offset = 0;
    for (int i = 0; i < MOS_OCA_RTLOG_COMPONENT_MAX; ++i)
    {
        m_rtLogSectionMgr[i].Init(m_heapAddr, m_heapSize, MAX_OCA_RT_SUB_SIZE, offset);
        MOS_OCA_RTLOG_SECTION_HEADER sectionHeader = {};
        sectionHeader.magicNum      = MOS_OCA_RTLOG_MAGIC_NUM;
        sectionHeader.componentType = (MOS_OCA_RTLOG_COMPONENT_TPYE)i;
        sectionHeader.freq          = 0;
        MosUtilities::MosQueryPerformanceFrequency(&sectionHeader.freq);
        m_rtLogSectionMgr[i].InsertUid(sectionHeader);
        offset += MAX_OCA_RT_SUB_SIZE;
    }

    m_isMgrInitialized = true;
}

MosOcaRTLogMgr::MosOcaRTLogMgr(MosOcaRTLogMgr &)
{
}

MosOcaRTLogMgr::~MosOcaRTLogMgr()
{
    m_globleIndex = -1;
    m_isMgrInitialized = false;
}


MosOcaRTLogMgr& MosOcaRTLogMgr::operator= (MosOcaRTLogMgr&)
{
    return *this;
}

MosOcaRTLogMgr &MosOcaRTLogMgr::GetInstance()
{
    static MosOcaRTLogMgr mgr;
    return mgr;
}

void MosOcaRTLogMgr::RegisterContext(OsContextNext *osDriverContext, MOS_CONTEXT *osContext)
{
    if (!m_enableOcaRTLog)
    {
        return;
    }
    if (!osContext)
    {
        return;
    }
    MosOcaRTLogMgr &ocaRTLogMgr = GetInstance();
    MOS_STATUS      status      = ocaRTLogMgr.RegisterCtx(osDriverContext, osContext);
    if (status != MOS_STATUS_SUCCESS)
    {
        MOS_OS_NORMALMESSAGE("MosOcaRTLogMgr RegisterContext failed!");
    }
}

void MosOcaRTLogMgr::UnRegisterContext(OsContextNext *osDriverContext)
{
    if (!m_enableOcaRTLog)
    {
        return;
    }
    MosOcaRTLogMgr &ocaRTLogMgr = GetInstance();
    MOS_STATUS      status      = ocaRTLogMgr.UnRegisterCtx(osDriverContext);
    if (status != MOS_STATUS_SUCCESS)
    {
        MOS_OS_NORMALMESSAGE("MosOcaRTLogMgr UnRegisterContext failed!");
    }
}

int32_t MosOcaRTLogMgr::GetGlobleIndex()
{
    return MosUtilities::MosAtomicIncrement(&m_globleIndex);
}