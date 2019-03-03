/*
* Copyright (c) 2014-2017, Intel Corporation
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
//! \file     mhw_cp_interface.cpp
//! \brief    Implement MHW interface for content protection 
//! \details  Impelements the functionalities across all platforms for content protection
//!

#include "mhw_cp_interface.h"
#include "cplib_utils.h"

static void MhwStubMessage()
{
    MOS_NORMALMESSAGE(MOS_COMPONENT_CP, MOS_CP_SUBCOMP_MHW, CP_STUB_MESSAGE);
}

MhwCpInterface* Create_MhwCpInterface(PMOS_INTERFACE osInterface)
{
    MhwCpInterface* pMhwCpInterface = nullptr;
    using Create_MhwCpFuncType = MhwCpInterface* (*)(PMOS_INTERFACE osInterface);
    CPLibUtils::InvokeCpFunc<Create_MhwCpFuncType>(
        pMhwCpInterface, 
        CPLibUtils::FUNC_CREATE_MHWCP, osInterface);

    if(nullptr == pMhwCpInterface) MhwStubMessage();

    return nullptr == pMhwCpInterface? MOS_New(MhwCpInterface) : pMhwCpInterface;
}

void Delete_MhwCpInterface(MhwCpInterface* pMhwCpInterface)
{
    if(nullptr == pMhwCpInterface)
    {
        return;
    }

    if(typeid(*pMhwCpInterface) == typeid(MhwCpInterface))
    {
        MOS_Delete(pMhwCpInterface);
    }
    else
    {
        using Delete_MhwCpFuncType = void (*)(MhwCpInterface*);
        CPLibUtils::InvokeCpFunc<Delete_MhwCpFuncType>(
            CPLibUtils::FUNC_DELETE_MHWCP, 
            pMhwCpInterface);
    }
}

MOS_STATUS MhwCpInterface::AddProlog(
    PMOS_INTERFACE      osInterface,
    PMOS_COMMAND_BUFFER cmdBuffer)
{
    MOS_UNUSED(osInterface);
    MOS_UNUSED(cmdBuffer);

    MhwStubMessage();
    return MOS_STATUS_SUCCESS;
}

bool MhwCpInterface::IsHWCounterAutoIncrementEnforced(
    PMOS_INTERFACE osInterface)
{
    MOS_UNUSED(osInterface);

    MhwStubMessage();
    return false;
}

MOS_STATUS MhwCpInterface::AddEpilog(
    PMOS_INTERFACE      osInterface,
    PMOS_COMMAND_BUFFER cmdBuffer)
{
    MOS_UNUSED(osInterface);
    MOS_UNUSED(cmdBuffer);

    MhwStubMessage();
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MhwCpInterface::AddCheckForEarlyExit(
    PMOS_INTERFACE      osInterface,
    PMOS_COMMAND_BUFFER cmdBuffer)
{
    MOS_UNUSED(osInterface);
    MOS_UNUSED(cmdBuffer);

    MhwStubMessage();
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MhwCpInterface::AddConditionalBatchBufferEndForEarlyExit(
    PMOS_INTERFACE      osInterface,
    PMOS_COMMAND_BUFFER cmdBuffer)
{
    MOS_UNUSED(osInterface);
    MOS_UNUSED(cmdBuffer);

    MhwStubMessage();
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS UpdateStatusReportNum(
    uint32_t            cencBufIndex,
    uint32_t            statusReportNum,
    uint8_t *           lockAddress,
    PMOS_RESOURCE       resource,
    PMOS_COMMAND_BUFFER cmdBuffer)
{
    MOS_UNUSED(cencBufIndex);
    MOS_UNUSED(statusReportNum);
    MOS_UNUSED(lockAddress);
    MOS_UNUSED(resource);
    MOS_UNUSED(cmdBuffer);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CheckStatusReportNum(
    void *              mfxRegisters,
    uint32_t            cencBufIndex,
    PMOS_RESOURCE       resource,
    PMOS_COMMAND_BUFFER cmdBuffer)
{
    MOS_UNUSED(mfxRegisters);
    MOS_UNUSED(cencBufIndex);
    MOS_UNUSED(resource);
    MOS_UNUSED(cmdBuffer);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MhwCpInterface::UpdateStatusReportNum(
    uint32_t            cencBufIndex,
    uint32_t            statusReportNum,
    uint8_t*            lockAddress,
    PMOS_RESOURCE       resource,
    PMOS_COMMAND_BUFFER cmdBuffer)
{
    MOS_UNUSED(cencBufIndex);
    MOS_UNUSED(statusReportNum);
    MOS_UNUSED(lockAddress);
    MOS_UNUSED(resource);
    MOS_UNUSED(cmdBuffer);

    MhwStubMessage();
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MhwCpInterface::CheckStatusReportNum(
    void*                       mfxRegisters,
    uint32_t                    cencBufIndex,
    PMOS_RESOURCE               resource,
    PMOS_COMMAND_BUFFER         cmdBuffer)
{
    MOS_UNUSED(mfxRegisters);
    MOS_UNUSED(cencBufIndex);
    MOS_UNUSED(resource);
    MOS_UNUSED(cmdBuffer);

    MhwStubMessage();
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MhwCpInterface::SetCpCopy(
    PMOS_INTERFACE      osInterface,
    PMOS_COMMAND_BUFFER cmdBuffer,
    PMHW_CP_COPY_PARAMS params)
{
    MOS_UNUSED(osInterface);
    MOS_UNUSED(cmdBuffer);
    MOS_UNUSED(params);

    MhwStubMessage();
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MhwCpInterface::SetMfxInlineStatusRead(
    PMOS_INTERFACE      osInterface,
    PMOS_COMMAND_BUFFER cmdBuffer,
    PMOS_RESOURCE       resource,
    uint16_t            currentIndex,
    uint32_t            writeOffset)
{
    MOS_UNUSED(osInterface);
    MOS_UNUSED(cmdBuffer);
    MOS_UNUSED(resource);
    MOS_UNUSED(currentIndex);
    MOS_UNUSED(writeOffset);

    MhwStubMessage();
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MhwCpInterface::SetProtectionSettingsForMiFlushDw(
    PMOS_INTERFACE osInterface,
    void  *cmd)
{
    MOS_UNUSED(osInterface);
    MOS_UNUSED(cmd);

    MhwStubMessage();
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MhwCpInterface::SetProtectionSettingsForMfxWait(
    PMOS_INTERFACE osInterface,
    void           *cmd)
{
    MOS_UNUSED(osInterface);
    MOS_UNUSED(cmd);

    MhwStubMessage();
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MhwCpInterface::SetProtectionSettingsForMfxPipeModeSelect(uint32_t *data)
{
    MOS_UNUSED(data);

    MhwStubMessage();
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MhwCpInterface::SetProtectionSettingsForHcpPipeModeSelect(uint32_t *data, bool bScalableEncode)
{
    MOS_UNUSED(data);

    MhwStubMessage();
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MhwCpInterface::SetProtectionSettingsForHucPipeModeSelect(uint32_t *data)
{
    MOS_UNUSED(data);

    MhwStubMessage();
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MhwCpInterface::SetMfxProtectionState(
    bool                             isDecodeInUse,
    PMOS_COMMAND_BUFFER              cmdBuffer,
    PMHW_BATCH_BUFFER                batchBuffer,
    PMHW_CP_SLICE_INFO_PARAMS        sliceInfoParam)
{
    MOS_UNUSED(isDecodeInUse);
    MOS_UNUSED(cmdBuffer);
    MOS_UNUSED(batchBuffer);
    MOS_UNUSED(sliceInfoParam);

    MhwStubMessage();
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MhwCpInterface::RegisterMiInterface(
    MhwMiInterface* miInterface)
{
    MOS_UNUSED(miInterface);

    MhwStubMessage();
    return MOS_STATUS_SUCCESS;
}

void MhwCpInterface::GetCpStateLevelCmdSize(uint32_t& cmdSize, uint32_t& patchListSize)
{
    cmdSize = 0;
    patchListSize = 0;

    MhwStubMessage();
    return;
}

void MhwCpInterface::GetCpSliceLevelCmdSize(uint32_t& cmdSize, uint32_t& patchListSize)
{
    cmdSize = 0;
    patchListSize = 0;

    MhwStubMessage();
    return;
}

void MhwCpInterface::RegisterParams(void* params)
{
    MOS_UNUSED(params);
    MhwStubMessage();
}

MOS_STATUS MhwCpInterface::UpdateParams(bool isInput)
{
    MOS_UNUSED(isInput);
    MhwStubMessage();
    return MOS_STATUS_SUCCESS;
}

CP_MODE MhwCpInterface::GetHostEncryptMode() const
{
    MhwStubMessage();
    return CP_TYPE_NONE;
}

void MhwCpInterface::SetCpSecurityType(CP_SECURITY_TYPE type)
{
    MOS_UNUSED(type);
    MhwStubMessage();
}
