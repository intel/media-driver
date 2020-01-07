/*
* Copyright (c) 2018-2020, Intel Corporation
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
//! \file    mos_auxtable_mgr.cpp
//! \brief   Container class for GMM aux table manager wrapper
//!

#include "mos_auxtable_mgr.h"
#include "mos_utilities.h"

//!
//! \brief  Implementation of device callback for GMM PageTableMgr to allocate page table BO
//!
//! \param  [in] bufMgr
//!         MOS_BUFMGR pass to GMM PageTableMgr by media driver
//! \param  [in] size
//!         Size to be allocated
//! \param  [in] alignment
//!         Alignment requirement for the VA address of the allocated buffer
//! \param  [out] bo
//!         Poiner to the bo object allocated
//! \param  [out] cpuAddr
//!         CPU VA of the bo object allocated
//! \param  [out] gpuAddr
//!         CPU VA of the bo object allocated
//!
//! \return int
//!         Error status code. 0 if success, else errno.
//!
static int AllocateCb(void *bufMgr, size_t size, size_t alignment, void **bo, void **cpuAddr, uint64_t *gpuAddr)
{
    int             ret = 0;
    MOS_BUFMGR      *bm;
    void            *userptr;
    mos_linux_bo    *lbo;

    if (bufMgr == nullptr)
        return -EINVAL;

    bm = (MOS_BUFMGR *)bufMgr;
    userptr = MOS_AlignedAllocMemory(size, alignment);
    if (userptr == nullptr)
        return -ENOMEM;

    lbo = mos_bo_alloc_userptr(bm, "GmmAuxPageTable", userptr, I915_TILING_NONE, size, size, 0);
    if (lbo == nullptr)
    {
        MOS_FreeMemory(userptr);
        return -EAGAIN;
    }

    mos_bo_set_softpin(lbo);

    *bo = lbo;
    *cpuAddr = userptr;
    *gpuAddr = (uint64_t)lbo->offset64;

    return 0;
}

//!
//! \brief  Implementation of device callback for GMM PageTableMgr to free allocated BO
//!
//! \param  [in] bo
//!         Poiner to the bo object to be freed
//!
//! \return none
//!
static void DeallocateCb(void *bo)
{
    mos_linux_bo  *lbo = (mos_linux_bo *)bo;
    if (lbo != nullptr)
    {
        void* virt = lbo->virt;

        mos_bo_wait_rendering(lbo);
        mos_bo_unreference(lbo);
        if (virt)
        {
            MOS_FreeMemory(virt);
        }
    }
}

//!
//! \brief  Implementation of device callback for GMM PageTableMgr to wait BO idle
//!
//! \param  [in] bo
//!         Poiner to the bo object to be waited
//!
//! \return none
//!
static void WaitFromCpuCb(void *bo)
{
    mos_linux_bo  *lbo = (mos_linux_bo *)bo;
    if (lbo != nullptr)
    {
        mos_bo_wait_rendering((mos_linux_bo *)bo);
    }
}

AuxTableMgr::AuxTableMgr(MOS_BUFMGR *bufMgr)
{
    if (bufMgr)
    {
        GMM_DEVICE_CALLBACKS_INT deviceCb = {0};

        GmmExportEntries GmmFuncs;
        GMM_STATUS gmmStatus = OpenGmm(&GmmFuncs);
        if(gmmStatus != GMM_SUCCESS)
        {
            MOS_OS_ASSERTMESSAGE("gmm init failed.");
        }
        m_gmmClientContext = GmmFuncs.pfnCreateClientContext((GMM_CLIENT)GMM_LIBVA_LINUX);
        if (m_gmmClientContext == nullptr)
        {
            MOS_OS_ASSERTMESSAGE(" nullptr returned by GmmCreateClientContext");
        }
        else
        {
            deviceCb.pBufMgr = bufMgr;
            deviceCb.DevCbPtrs_.pfnAllocate = AllocateCb;
            deviceCb.DevCbPtrs_.pfnDeallocate = DeallocateCb;
            deviceCb.DevCbPtrs_.pfnWaitFromCpu = WaitFromCpuCb;

            m_gmmPageTableMgr = m_gmmClientContext->CreatePageTblMgrObject(&deviceCb, AUXTT);
            if (m_gmmPageTableMgr == nullptr)
            {
                MOS_OS_ASSERTMESSAGE(" nullptr returned by new GMM_PAGETABLE_MGR");
            }
        }
    }
}

AuxTableMgr::~AuxTableMgr()
{
    if (m_gmmPageTableMgr != nullptr)
    {
        m_gmmClientContext->DestroyPageTblMgrObject((GMM_PAGETABLE_MGR *)m_gmmPageTableMgr);
        m_gmmPageTableMgr = nullptr;
    }
    if (m_gmmClientContext != nullptr)
    {
        GmmExportEntries GmmFuncs;
        GMM_STATUS gmmStatus = OpenGmm(&GmmFuncs);
        if(gmmStatus != GMM_SUCCESS)
        {
            MOS_OS_ASSERTMESSAGE("gmm init failed.");
        }
        GmmFuncs.pfnDeleteClientContext((GMM_CLIENT_CONTEXT *)m_gmmClientContext);
        m_gmmClientContext = nullptr;
    }
}

AuxTableMgr * AuxTableMgr::CreateAuxTableMgr(MOS_BUFMGR *bufMgr, MEDIA_FEATURE_TABLE *sku)
{
    if (MEDIA_IS_SKU(sku, FtrE2ECompression) && !MEDIA_IS_SKU(sku, FtrFlatPhysCCS))
    {
        AuxTableMgr *auxTableMgr = MOS_New(AuxTableMgr, bufMgr);
        if (auxTableMgr == nullptr)
        {
            MOS_OS_ASSERTMESSAGE(" nullptr returned by creating AuxTableMgr");
        }
        return auxTableMgr;
    }
    return nullptr;
}

MOS_STATUS  AuxTableMgr::MapResource(GMM_RESOURCE_INFO *gmmResInfo, MOS_LINUX_BO *bo)
{
    if (gmmResInfo == nullptr || bo == nullptr)
    {
        return MOS_STATUS_NULL_POINTER;
    }

    GMM_RESOURCE_FLAG flags = gmmResInfo->GetResFlags(); 
    if ((flags.Info.MediaCompressed || flags.Info.RenderCompressed) && (bo->aux_mapped == false))
    {
        int ret = 0;

        ret = mos_bo_set_softpin(bo);
        if (ret != 0)
        {
            MOS_OS_ASSERTMESSAGE("Aux mapping failed: softpin failed");
            return MOS_STATUS_UNKNOWN;
        }

        GMM_DDI_UPDATEAUXTABLE  updateReq = {};

        updateReq.BaseResInfo = gmmResInfo;
        updateReq.BaseGpuVA = bo->offset64;
        updateReq.Map = 1;
        if (GMM_SUCCESS != ((GMM_PAGETABLE_MGR *)m_gmmPageTableMgr)->UpdateAuxTable(&updateReq))
        {
            MOS_OS_ASSERTMESSAGE("update AuxTable failed");
            return MOS_STATUS_UNKNOWN;
        }
        bo->aux_mapped = true;
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS  AuxTableMgr::UnmapResource(GMM_RESOURCE_INFO *gmmResInfo, MOS_LINUX_BO *bo)
{
    if (gmmResInfo && bo && bo->aux_mapped)
    {
        GMM_DDI_UPDATEAUXTABLE  updateReq = {};

        MOS_OS_ASSERT(gmmResInfo->GetResFlags().Info.MediaCompressed ||
                gmmResInfo->GetResFlags().Info.RenderCompressed);

        updateReq.BaseResInfo   = gmmResInfo;
        updateReq.BaseGpuVA     = bo->offset64;
        updateReq.Map           = 0;
        ((GMM_PAGETABLE_MGR*)m_gmmPageTableMgr)->UpdateAuxTable(&updateReq);
        bo->aux_mapped = false;
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS  AuxTableMgr::EmitAuxTableBOList(MOS_LINUX_BO *cmd_bo)
{
    // Retrieve aux table bo list from GMM and emit it to exec buffer bo list
    int boCnt = ((GMM_PAGETABLE_MGR*)m_gmmPageTableMgr)->GetNumOfPageTableBOs(AUXTT);
    if (boCnt <= 0) {
        return MOS_STATUS_SUCCESS;
    }

    mos_linux_bo **boList = (MOS_LINUX_BO **)MOS_AllocAndZeroMemory(boCnt * sizeof(mos_linux_bo*));
    if (boList == nullptr)
        return MOS_STATUS_NO_SPACE;

    ((GMM_PAGETABLE_MGR*)m_gmmPageTableMgr)->GetPageTableBOList(AUXTT, boList);
    for (int i=0; i<boCnt; i++)
    {
        int ret = mos_bo_emit_reloc(
            cmd_bo,                     // Command buffer
            0,                          // Offset in the command buffer, not used for softpin
            boList[i],                  // Allocation object for which the patch will be made.
            0,                          // Offset to the indirect state
            I915_GEM_DOMAIN_RENDER,     // Read domain
            I915_GEM_DOMAIN_CPU);       // Write domain
        if (ret != 0)
        {
            MOS_OS_ASSERTMESSAGE("Error patching alloc_bo = 0x%x, cmd_bo = 0x%x.",
                (uintptr_t)boList[i],
                (uintptr_t)cmd_bo);
            return MOS_STATUS_UNKNOWN;
        }
    }
    MOS_FreeMemory(boList);

    return MOS_STATUS_SUCCESS;
}

uint64_t  AuxTableMgr::GetAuxTableBase()
{
    return m_gmmPageTableMgr ? ((GMM_PAGETABLE_MGR*)m_gmmPageTableMgr)->GetAuxL3TableAddr() : 0;
}
