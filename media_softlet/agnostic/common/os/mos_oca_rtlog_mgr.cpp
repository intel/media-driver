/*
* Copyright (c) 2022-2023, Intel Corporation
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
#include "oca_rtlog_section_mgr.h"
#include "mos_context_specific_next.h"

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
    // Use encode output bitstream to ensure coherency being enabled for CPU catchable surface, which
    // will be checked during MapGpuVirtualAddress w/ critical message for invalid case.
    sParams.ResUsageType            = MOS_HW_RESOURCE_USAGE_ENCODE_OUTPUT_BITSTREAM;
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
    if (iter == m_resMap.end())
    {
        return;
    }
    resInterface = iter->second;
    s_ocaMutex.Lock();
    m_resMap.erase(osDriverContext);
    s_ocaMutex.Unlock();
    if (!resInterface.osInterface || !resInterface.osInterface->pfnFreeResource)
    {
        MOS_SafeFreeMemory(resInterface.ocaRTLogResource);
        resInterface.ocaRTLogResource = nullptr;
        MOS_SafeFreeMemory(resInterface.osInterface);
        resInterface.osInterface = nullptr;
        return;
    }
    resInterface.osInterface->pfnFreeResource(resInterface.osInterface, resInterface.ocaRTLogResource);
    MOS_SafeFreeMemory(resInterface.ocaRTLogResource);
    resInterface.ocaRTLogResource = nullptr;
    Mos_DestroyInterface(resInterface.osInterface);
    MOS_SafeFreeMemory(resInterface.osInterface);
    resInterface.osInterface = nullptr;
}

MosOcaRTLogMgr::MosOcaRTLogMgr()
{
    m_heapAddr = OcaRtLogSectionMgr::GetMemAddress();

    m_isMgrInitialized = true;
}

MosOcaRTLogMgr::MosOcaRTLogMgr(MosOcaRTLogMgr &)
{
}

MosOcaRTLogMgr::~MosOcaRTLogMgr()
{
    m_globleIndex = -1;
    m_isMgrInitialized = false;
    s_isOcaRtlogMgrDestoryed = true;
}


MosOcaRTLogMgr& MosOcaRTLogMgr::operator= (MosOcaRTLogMgr&)
{
    return *this;
}

MosOcaRTLogMgr *MosOcaRTLogMgr::GetInstance()
{
    static MosOcaRTLogMgr mgr;
    return &mgr;
}

void MosOcaRTLogMgr::RegisterContext(OsContextNext *osDriverContext, MOS_CONTEXT *osContext)
{
    if (!s_enableOcaRTLog)
    {
        return;
    }
    if (!osContext)
    {
        return;
    }
    MosOcaRTLogMgr *ocaRTLogMgr = GetInstance();
    MOS_STATUS      status      = ocaRTLogMgr->RegisterCtx(osDriverContext, osContext);
    if (status != MOS_STATUS_SUCCESS)
    {
        MOS_OS_NORMALMESSAGE("MosOcaRTLogMgr RegisterContext failed!");
    }
}

void MosOcaRTLogMgr::UnRegisterContext(OsContextNext *osDriverContext)
{
    if (!s_enableOcaRTLog)
    {
        return;
    }
    if (s_isOcaRtlogMgrDestoryed)
    {
        MOS_OS_NORMALMESSAGE("MosOcaRTLogMgr have be destroyed!");
        return;
    }
    MosOcaRTLogMgr *ocaRTLogMgr = GetInstance();
    MOS_STATUS      status      = ocaRTLogMgr->UnRegisterCtx(osDriverContext);
    if (status != MOS_STATUS_SUCCESS)
    {
        MOS_OS_NORMALMESSAGE("MosOcaRTLogMgr UnRegisterContext failed!");
    }
}