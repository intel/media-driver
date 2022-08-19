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

bool MosOcaRTLogMgr::m_enableOcaRTLog = false;

/****************************************************************************************************/
/*                                      MosOcaRtLogSectionMgr                                       */
/****************************************************************************************************/

MosOcaRtLogSectionMgr::MosOcaRtLogSectionMgr(MOS_OCA_RTLOG_HEAP ocaRtHeap, uint32_t size, uint32_t offset)
{
    if (ocaRtHeap.ocaHeapCpuVa && ocaRtHeap.size)
    {
        m_LockedHeap = ocaRtHeap.ocaHeapCpuVa;
        m_HeapGpuVa  = ocaRtHeap.ocaHeapGpuVa;
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
    m_HeapGpuVa     = 0;
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

MOS_STATUS MosOcaRTLogMgr::Initialize(OsContextNext *osDriverContext)
{
    if (m_IsMgrInitialized)
    {
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS      status      = MOS_STATUS_SUCCESS;
    MosStreamState  streamState = {};

    MOS_OS_CHK_NULL_RETURN(osDriverContext);

    m_osContext = osDriverContext;

    streamState.osDeviceContext = m_osContext;

    m_ocaRtHeap.size = MAX_OCA_RT_SIZE;

    MOS_ALLOC_GFXRES_PARAMS params = {};
    // Initiate allocation paramters
    params.Type          = MOS_GFXRES_BUFFER;
    params.TileType      = MOS_TILE_LINEAR;
    params.Format        = Format_Buffer;
    params.dwBytes       = m_ocaRtHeap.size;
    params.pBufName      = "OcaRtLog";
    params.bIsPersistent = true;
    params.dwMemType     = MOS_MEMPOOL_SYSTEMMEMORY;
    if (nullptr == m_ocaRTLogResource)
    {
        m_ocaRTLogResource = MOS_New(MOS_RESOURCE);
        MOS_OS_CHK_NULL_RETURN(m_ocaRTLogResource);

        status = MosInterface::AllocateResource(
            &streamState,
            &params,
            m_ocaRTLogResource
#if MOS_MESSAGES_ENABLED
            ,
            __FUNCTION__,
            __FILE__,
            __LINE__
#endif
            );
        MOS_OS_CHK_STATUS_RETURN(status);
    }

    MOS_OS_CHK_STATUS_RETURN(MapGfxVa());

    MOS_LOCK_PARAMS LockFlags = {};
    LockFlags.NoOverWrite     = 1;
    LockFlags.WriteOnly       = 1;
    m_ocaRtHeap.ocaHeapCpuVa = MosInterface::LockMosResource(&streamState, m_ocaRTLogResource, &LockFlags);

    if (nullptr == m_ocaRtHeap.ocaHeapCpuVa)
    {
        Reset();
        MOS_OS_CHK_STATUS_RETURN(MOS_STATUS_NULL_POINTER);
    }
    uint32_t offset = 0;

    for (int i = 0; i < MOS_OCA_RTLOG_COMPONENTS_SIZE; ++i)
    {
        uint32_t size = MAX_OCA_RT_SUB_SIZE;

        MosOcaRtLogSectionMgr *mgr = MOS_New(MosOcaRtLogSectionMgr, m_ocaRtHeap, size, offset);
        if (nullptr == mgr)
        {
            MOS_OS_NORMALMESSAGE("Allocate m_RTLogSectionMgr failed!");
            Clean();
            MOS_OS_CHK_STATUS_RETURN(MOS_STATUS_NULL_POINTER);
        }
        MOS_OCA_RTLOG_SECTION_HEADER sectionHeader = {};
        sectionHeader.magicNum      = MOS_OCA_RTLOG_MAGIC_NUM;
        sectionHeader.componentType = (MOS_OCA_RTLOG_COMPONENT_TPYE)i;
        sectionHeader.freq          = 0;
        MosUtilities::MosQueryPerformanceFrequency(&sectionHeader.freq);
        mgr->InsertUid(sectionHeader);
        m_RTLogSectionMgr.push_back(mgr);
        offset += MAX_OCA_RT_SUB_SIZE;
    }

    m_IsMgrInitialized = true;
    return MOS_STATUS_SUCCESS;
}

void MosOcaRTLogMgr::Uninitialize()
{
    if (!m_IsMgrInitialized)
    {
        return;
    }
    
    m_IsMgrInitialized = false;
    Clean();
    m_globleIndex = -1;

    return;
}

MOS_STATUS MosOcaRTLogMgr::Clean()
{
    for (std::vector<MosOcaRtLogSectionMgr *>::iterator it = m_RTLogSectionMgr.begin();
         it != m_RTLogSectionMgr.end();
         ++it)
    {
        MosOcaRtLogSectionMgr *p = *it;
        if (p)
        {
            MOS_Delete(p);
        }
    }
    m_RTLogSectionMgr.clear();
    Reset();
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosOcaRTLogMgr::Reset()
{
    MosStreamState  streamState = {};

    MOS_OS_CHK_NULL_RETURN(m_osContext);

    streamState.osDeviceContext = m_osContext;

    if (m_ocaRTLogResource)
    {
        MosInterface::FreeResource(
            &streamState,
            m_ocaRTLogResource,
            0
#if MOS_MESSAGES_ENABLED
            ,
            __FUNCTION__,
            __FILE__,
            __LINE__
#endif
        );
        MOS_Delete(m_ocaRTLogResource);
    }

    m_ocaRtHeap = {};
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosOcaRTLogMgr::InsertRTLog(MOS_OCA_RTLOG_COMPONENT_TPYE componentType, bool isErr, int32_t id, uint32_t paramCount, const void *param)
{
    if (!m_IsMgrInitialized)
    {
        return MOS_STATUS_UNIMPLEMENTED;
    }

    if (paramCount != MOS_OCA_RTLOG_MAX_PARAM_COUNT)
    {
        return MOS_STATUS_UNIMPLEMENTED;
    }
    MosOcaRtLogSectionMgr *insMgr = nullptr;

    if (componentType < MOS_OCA_RTLOG_COMPONENTS_SIZE)
    {
        insMgr = m_RTLogSectionMgr[componentType];
    }
    
    if (insMgr)
    {
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
}

MosOcaRTLogMgr::MosOcaRTLogMgr(MosOcaRTLogMgr &)
{
}

MosOcaRTLogMgr::~MosOcaRTLogMgr()
{
}


MosOcaRTLogMgr& MosOcaRTLogMgr::operator= (MosOcaRTLogMgr&)
{
    return *this;
}

MOS_STATUS MosOcaRTLogMgr::InitMgr(MosOcaRTLogMgr *&ocaRTLogMgr, OsContextNext *osDriverContext)
{
    if (!m_enableOcaRTLog)
    {
        return MOS_STATUS_SUCCESS;
    }
    ocaRTLogMgr = MOS_New(MosOcaRTLogMgr);
    MOS_OS_CHK_NULL_RETURN(ocaRTLogMgr);
    MOS_STATUS status = ocaRTLogMgr->Initialize(osDriverContext);
    if (MOS_FAILED(status))
    {
        MOS_Delete(ocaRTLogMgr);
        MOS_OS_CHK_STATUS_RETURN(status);
    }
    return status;
}

void MosOcaRTLogMgr::UninitMgr(MosOcaRTLogMgr *&ocaRTLogMgr)
{
    if (!m_enableOcaRTLog)
    {
        return;
    }
    if (ocaRTLogMgr == nullptr)
    {
        MOS_OS_ASSERTMESSAGE("Allocate m_RTLogSectionMgr failed!");
        return;
    }
    ocaRTLogMgr->Uninitialize();
    MOS_Delete(ocaRTLogMgr);
}

int32_t MosOcaRTLogMgr::GetGlobleIndex()
{
    return MosUtilities::MosAtomicIncrement(&m_globleIndex);
}