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
//! \file     mos_os_cp_specific.cpp
//! \brief    OS specific implement for CP related functions
//!

#include "mos_os_cp_specific.h"
#include "mos_os.h"
#include "mos_util_debug.h"
#include "mhw_cp.h"

static void OsStubMessage()
{
    MOS_NORMALMESSAGE(MOS_COMPONENT_CP, MOS_CP_SUBCOMP_MHW, "This function is stubbed as CP is not enabled.");
}

MosCpInterface::MosCpInterface(void  *pvOsInterface)
{
}

MosCpInterface::~MosCpInterface()
{
}

MOS_STATUS MosCpInterface::RegisterPatchForHM(
    uint32_t       *pPatchAddress,
    bool           bWrite,
    MOS_HW_COMMAND HwCommandType,
    uint32_t       forceDwordOffset,
    void           *plResource)
{
    MOS_UNUSED(pPatchAddress);
    MOS_UNUSED(bWrite);
    MOS_UNUSED(HwCommandType);
    MOS_UNUSED(forceDwordOffset);
    MOS_UNUSED(plResource);

    OsStubMessage();
    return  MOS_STATUS_SUCCESS;
}

MOS_STATUS MosCpInterface::PermeatePatchForHM(
    void    *virt,
    void    *pvCurrentPatch)
{
    MOS_UNUSED(virt);
    MOS_UNUSED(pvCurrentPatch);

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

bool MosCpInterface::IsHardwareProtectionRequired(
    void        *ppvOsResource[],
    uint32_t    uiNumOfResources,
    bool        bForceNoneCp)
{
    MOS_UNUSED(ppvOsResource);
    MOS_UNUSED(uiNumOfResources);
    MOS_UNUSED(bForceNoneCp);

    OsStubMessage();
    return false;
}

MOS_STATUS MosCpInterface::ResetHardwareProtectionState(
    void        *pvOsResource,
    bool        isShared)
{
    MOS_UNUSED(pvOsResource);
    MOS_UNUSED(isShared);

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
