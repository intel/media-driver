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
//! \file     mos_os_cp_interface_specific.cpp
//! \brief    OS specific implement for CP related functions
//!

#include "mos_os_cp_interface_specific.h"
#include "mos_os.h"
#include "mhw_cp_interface.h"
#include "cplib_utils.h"

static void OsStubMessage()
{
    MOS_NORMALMESSAGE(
        MOS_COMPONENT_CP, 
        MOS_CP_SUBCOMP_OS, 
        CP_STUB_MESSAGE);
}

MosCpInterface* Create_MosCpInterface(void* pvOsInterface)
{
    MosCpInterface* pMosCpInterface = nullptr;
    using Create_MosCpFuncType = MosCpInterface* (*)(void* pvOsResource);
    CPLibUtils::InvokeCpFunc<Create_MosCpFuncType>(
        pMosCpInterface, 
        CPLibUtils::FUNC_CREATE_MOSCP, pvOsInterface);

    if(nullptr == pMosCpInterface) OsStubMessage();

    return nullptr == pMosCpInterface ? MOS_New(MosCpInterface) : pMosCpInterface;
}

void Delete_MosCpInterface(MosCpInterface* pMosCpInterface)
{
    if(typeid(MosCpInterface) == typeid(*pMosCpInterface))
    {
        MOS_Delete(pMosCpInterface);
    }
    else
    {
        using Delete_MosCpFuncType = void (*)(MosCpInterface*);
        CPLibUtils::InvokeCpFunc<Delete_MosCpFuncType>(
            CPLibUtils::FUNC_DELETE_MOSCP, 
            pMosCpInterface);
    }
}

MOS_STATUS MosCpInterface::RegisterPatchForHM(
    uint32_t       *pPatchAddress,
    bool           bWrite,
    MOS_HW_COMMAND HwCommandType,
    uint32_t       forceDwordOffset,
    void           *plResource,
    void           *pPatchLocationList)
{
    MOS_UNUSED(pPatchAddress);
    MOS_UNUSED(bWrite);
    MOS_UNUSED(HwCommandType);
    MOS_UNUSED(forceDwordOffset);
    MOS_UNUSED(plResource);
    MOS_UNUSED(pPatchLocationList);

    OsStubMessage();
    return  MOS_STATUS_SUCCESS;
}

MOS_STATUS MosCpInterface::PermeatePatchForHM(
    void    *virt,
    void    *pvCurrentPatch,
    void    *resource)
{
    MOS_UNUSED(virt);
    MOS_UNUSED(pvCurrentPatch);
    MOS_UNUSED(resource);

    OsStubMessage();
    return  MOS_STATUS_SUCCESS;
}

bool MosCpInterface::IsCpEnabled()
{
    OsStubMessage();
    return false;
}

void MosCpInterface::SetCpEnabled(bool bIsCpEnabled)
{
    OsStubMessage();
}

bool MosCpInterface::IsHMEnabled()
{
    OsStubMessage();
    return false;
}

bool MosCpInterface::IsIDMEnabled()
{
    OsStubMessage();
    return false;
}

bool MosCpInterface::IsSMEnabled()
{
    OsStubMessage();
    return false;
}

bool MosCpInterface::IsTearDownHappen()
{
    OsStubMessage();
    return false;
}

MOS_STATUS MosCpInterface::SetResourceEncryption(
    void          *pvResource,
    bool          bEncryption)
{
    MOS_UNUSED(pvResource);
    MOS_UNUSED(bEncryption);

    OsStubMessage();
    return MOS_STATUS_UNIMPLEMENTED;
}

MOS_STATUS MosCpInterface::PrepareResources(
    void        *source[],
    uint32_t    sourceCount,
    void        *target[],
    uint32_t    targetCount)
{
    MOS_UNUSED(source);
    MOS_UNUSED(sourceCount);
    MOS_UNUSED(target);
    MOS_UNUSED(targetCount);

    OsStubMessage();
    return MOS_STATUS_SUCCESS;
}

bool MosCpInterface::RenderBlockedFromCp()
{
    OsStubMessage();
    return false;
}

MOS_STATUS MosCpInterface::GetTK(
    uint32_t                    **ppTKs,
    uint32_t                    *pTKsSize,
    uint32_t                    *pTKsUpdateCnt)
{
    MOS_UNUSED(ppTKs);
    MOS_UNUSED(pTKsSize);
    MOS_UNUSED(pTKsUpdateCnt);

    OsStubMessage();
    return MOS_STATUS_SUCCESS;
}
