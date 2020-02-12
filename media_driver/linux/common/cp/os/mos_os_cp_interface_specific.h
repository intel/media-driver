/*
* Copyright (c) 2009-2017, Intel Corporation
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
//! \file     mos_os_cp_interface_specific.h
//! \brief    OS specific implement for CP related functions
//!

#ifndef __MOS_OS_CP_INTERFACE_SPECIFIC_H__
#define __MOS_OS_CP_INTERFACE_SPECIFIC_H__

#include "mos_defs.h"
#include "mos_os_hw.h"
#include "mos_util_debug.h"

static void OsStubMessage()
{
    MOS_NORMALMESSAGE(
        MOS_COMPONENT_CP,
        MOS_CP_SUBCOMP_OS,
        "This function is stubbed as CP is not enabled.");
}

class MosCpInterface
{
public:
    MosCpInterface() {}

    virtual ~MosCpInterface() {}

    virtual MOS_STATUS RegisterPatchForHM(
        uint32_t *     pPatchAddress,
        uint32_t       bWrite,
        MOS_HW_COMMAND HwCommandType,
        uint32_t       forceDwordOffset,
        void *         plResource,
        void *         pPatchLocationList)

    {
        MOS_UNUSED(pPatchAddress);
        MOS_UNUSED(bWrite);
        MOS_UNUSED(HwCommandType);
        MOS_UNUSED(forceDwordOffset);
        MOS_UNUSED(plResource);
        MOS_UNUSED(pPatchLocationList);

        OsStubMessage();
        return MOS_STATUS_SUCCESS;
    }

    virtual MOS_STATUS PermeatePatchForHM(
        void *virt,
        void *pvCurrentPatch,
        void *resource)
    {
        MOS_UNUSED(virt);
        MOS_UNUSED(pvCurrentPatch);
        MOS_UNUSED(resource);

        OsStubMessage();
        return MOS_STATUS_SUCCESS;
    }

    virtual bool IsCpEnabled()
    {
        OsStubMessage();
        return false;
    }

    virtual void SetCpEnabled(bool bIsCpEnabled)
    {
        OsStubMessage();
    }

    virtual bool IsHMEnabled()
    {
        OsStubMessage();
        return false;
    }

    virtual bool IsTSEnabled()
    {
        OsStubMessage();
        return false;
    }

    virtual bool IsIDMEnabled()
    {
        OsStubMessage();
        return false;
    }

    virtual bool IsSMEnabled()
    {
        OsStubMessage();
        return false;
    }

    virtual bool IsTearDownHappen()
    {
        OsStubMessage();
        return false;
    }

    virtual MOS_STATUS SetResourceEncryption(
        void *pResource,
        bool  bEncryption)

    {
        MOS_UNUSED(pResource);
        MOS_UNUSED(bEncryption);

        OsStubMessage();
        return MOS_STATUS_UNIMPLEMENTED;
    }

    virtual MOS_STATUS PrepareResources(
        void *   source[],
        uint32_t sourceCount,
        void *   target[],
        uint32_t targetCount)
    {
        MOS_UNUSED(source);
        MOS_UNUSED(sourceCount);
        MOS_UNUSED(target);
        MOS_UNUSED(targetCount);

        OsStubMessage();
        return MOS_STATUS_SUCCESS;
    }

    virtual bool RenderBlockedFromCp()
    {
        OsStubMessage();
        return false;
    }

    virtual MOS_STATUS AllocateRTEPhysicalBuffer(
        void        **ppRTEBuffer,
        uint32_t    *pBufferSize)
    {
        MOS_UNUSED(ppRTEBuffer);
        MOS_UNUSED(pBufferSize);

        OsStubMessage();
        return MOS_STATUS_SUCCESS;
    }

    virtual MOS_STATUS DeAllocateRTEPhysicalBuffer(
        void      *pRTEBuffer)
    {
        MOS_UNUSED(pRTEBuffer);

        OsStubMessage();
        return MOS_STATUS_SUCCESS;
    }

    virtual MOS_STATUS GetTK(
        uint32_t **ppTKs,
        uint32_t * pTKsSize,
        uint32_t * pTKsUpdateCnt)
    {
        MOS_UNUSED(ppTKs);
        MOS_UNUSED(pTKsSize);
        MOS_UNUSED(pTKsUpdateCnt);

        OsStubMessage();
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS ReadCtrNounceRegister(bool readCtr0, uint32_t *pCounter)
    {
        MOS_UNUSED(readCtr0);
        MOS_UNUSED(pCounter);

        OsStubMessage();
        return MOS_STATUS_UNIMPLEMENTED;
    }
};

//!
//! \brief    Create MosCpInterface Object according CPLIB loading status
//!           Must use Delete_MosCpInterface to delete created Object to avoid ULT Memory Leak errors
//!
//! \param    [in] pvOsInterface
//!           void*
//!
//! \return   Return CP Wrapper Object if CPLIB not loaded
//!
MosCpInterface* Create_MosCpInterface(void* pvOsInterface);

//!
//! \brief    Delete the MosCpInterface Object according CPLIB loading status
//!
//! \param    [in] pMosCpInterface 
//!           MosCpInterface
//!
void Delete_MosCpInterface(MosCpInterface* pMosCpInterface);

#endif  // __MOS_OS_CP_SPECIFIC_H__
