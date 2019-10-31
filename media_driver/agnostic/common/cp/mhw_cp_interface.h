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
//! \file     mhw_cp_interface.h
//! \brief    MHW interface for content protection 
//! \details  Impelements the functionalities across all platforms for content protection
//!

#ifndef __MHW_CP_INTERFACE_H__
#define __MHW_CP_INTERFACE_H__

#include "mhw_mi.h"
#include "mos_os.h"
#include "mos_util_debug.h"
#include "codec_def_common.h"

class MhwMiInterface;

typedef int32_t CP_MODE;
#define CP_TYPE_NONE 0

enum _CP_SECURITY_TYPE: int32_t;

typedef enum _CP_SECURITY_TYPE CP_SECURITY_TYPE;

typedef struct _MHW_CP_SLICE_INFO_PARAMS
{
    PMOS_RESOURCE presDataBuffer        = nullptr;
    uint32_t      dwDataLength[2]       = {0};       // 1 is for DECE mode, 0 is for others
    uint32_t      dwDataStartOffset[2]  = {0};       // 1 is for DECE mode, 0 is for others
    uint32_t      dwSliceIndex          = 0;
    bool          bLastPass             = false;
    uint32_t      dwTotalBytesConsumed  = 0;
} MHW_CP_SLICE_INFO_PARAMS, *PMHW_CP_SLICE_INFO_PARAMS;

typedef struct _MHW_CP_COPY_PARAMS
{
    PMOS_RESOURCE presSrc;
    PMOS_RESOURCE presDst;
    uint32_t      size;
    uint16_t      lengthOfTable;
    bool          isEncodeInUse;
} MHW_CP_COPY_PARAMS, *PMHW_CP_COPY_PARAMS;

typedef struct _MHW_ADD_CP_COPY_PARAMS
{
    PMOS_RESOURCE presSrc;
    PMOS_RESOURCE presDst;
    uint32_t      size;
    uint64_t      offset;
} MHW_ADD_CP_COPY_PARAMS, *PMHW_ADD_CP_COPY_PARAMS;

static void MhwStubMessage()
{
    MOS_NORMALMESSAGE(MOS_COMPONENT_CP, MOS_CP_SUBCOMP_MHW, "This function is stubbed as it is not implemented.");
}

class MhwCpInterface
{
public:
    virtual ~MhwCpInterface() {}

    virtual MOS_STATUS AddProlog(
        PMOS_INTERFACE      osInterface,
        PMOS_COMMAND_BUFFER cmdBuffer)
    {
        MOS_UNUSED(osInterface);
        MOS_UNUSED(cmdBuffer);

        MhwStubMessage();
        return MOS_STATUS_SUCCESS;
    }

    virtual bool IsHWCounterAutoIncrementEnforced(
        PMOS_INTERFACE osInterface)
    {
        MOS_UNUSED(osInterface);

        MhwStubMessage();
        return false;
    }

    virtual MOS_STATUS AddEpilog(
        PMOS_INTERFACE      osInterface,
        PMOS_COMMAND_BUFFER cmdBuffer)
    {
        MOS_UNUSED(osInterface);
        MOS_UNUSED(cmdBuffer);

        MhwStubMessage();
        return MOS_STATUS_SUCCESS;
    }

    virtual MOS_STATUS AddCheckForEarlyExit(
        PMOS_INTERFACE      osInterface,
        PMOS_COMMAND_BUFFER cmdBuffer)
    {
        MOS_UNUSED(osInterface);
        MOS_UNUSED(cmdBuffer);

        MhwStubMessage();
        return MOS_STATUS_SUCCESS;
    }

    virtual MOS_STATUS CheckStatusReportNum(
        void *              mfxRegisters,
        uint32_t            cencBufIndex,
        PMOS_RESOURCE       resource,
        PMOS_COMMAND_BUFFER cmdBuffer)
    {
        MOS_UNUSED(mfxRegisters);
        MOS_UNUSED(cencBufIndex);
        MOS_UNUSED(resource);
        MOS_UNUSED(cmdBuffer);

        MhwStubMessage();
        return MOS_STATUS_SUCCESS;
    }

    virtual MOS_STATUS SetCpCopy(
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

    virtual MOS_STATUS AddCpCopy(
        PMOS_INTERFACE      osInterface,
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMHW_ADD_CP_COPY_PARAMS params)
    {
        MOS_UNUSED(osInterface);
        MOS_UNUSED(cmdBuffer);
        MOS_UNUSED(params);

        MhwStubMessage();
        return MOS_STATUS_SUCCESS;
    }

    virtual MOS_STATUS ReadEncodeCounterFromHW(
        PMOS_INTERFACE      osInterface,
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMOS_RESOURCE       resource,
        uint16_t            currentIndex)
    {
        MOS_UNUSED(osInterface);
        MOS_UNUSED(cmdBuffer);
        MOS_UNUSED(resource);
        MOS_UNUSED(currentIndex);

        MhwStubMessage();
        return MOS_STATUS_SUCCESS;
    }

    virtual MOS_STATUS SetProtectionSettingsForMiFlushDw(
        PMOS_INTERFACE osInterface,
        void *         cmd)
    {
        MOS_UNUSED(osInterface);
        MOS_UNUSED(cmd);

        MhwStubMessage();
        return MOS_STATUS_SUCCESS;
    }

    virtual MOS_STATUS SetProtectionSettingsForMfxWait(
        PMOS_INTERFACE osInterface,
        void *         cmd)
    {
        MOS_UNUSED(osInterface);
        MOS_UNUSED(cmd);

        MhwStubMessage();
        return MOS_STATUS_SUCCESS;
    }

    virtual MOS_STATUS SetProtectionSettingsForMfxPipeModeSelect(uint32_t *data)
    {
        MOS_UNUSED(data);

        MhwStubMessage();
        return MOS_STATUS_SUCCESS;
    }

    virtual MOS_STATUS SetProtectionSettingsForHcpPipeModeSelect(
        uint32_t *data,
        bool      scalableEncode = false)
    {
        MOS_UNUSED(data);

        MhwStubMessage();
        return MOS_STATUS_SUCCESS;
    }

    virtual MOS_STATUS SetProtectionSettingsForHucPipeModeSelect(uint32_t *data)
    {
        MOS_UNUSED(data);

        MhwStubMessage();
        return MOS_STATUS_SUCCESS;
    }

    virtual MOS_STATUS SetMfxProtectionState(
        bool                      isDecodeInUse,
        PMOS_COMMAND_BUFFER       cmdBuffer,
        PMHW_BATCH_BUFFER         batchBuffer,
        PMHW_CP_SLICE_INFO_PARAMS sliceInfoParam)
    {
        MOS_UNUSED(isDecodeInUse);
        MOS_UNUSED(cmdBuffer);
        MOS_UNUSED(batchBuffer);
        MOS_UNUSED(sliceInfoParam);

        MhwStubMessage();
        return MOS_STATUS_SUCCESS;
    }

    virtual MOS_STATUS RegisterMiInterface(
        MhwMiInterface *miInterface)
    {
        MOS_UNUSED(miInterface);

        MhwStubMessage();
        return MOS_STATUS_SUCCESS;
    }

    virtual void GetCpStateLevelCmdSize(
        uint32_t &cmdSize,
        uint32_t &patchListSize)
    {
        cmdSize       = 0;
        patchListSize = 0;

        MhwStubMessage();
        return;
    }

    virtual void GetCpSliceLevelCmdSize(
        uint32_t &cmdSize,
        uint32_t &patchListSize)
    {
        cmdSize       = 0;
        patchListSize = 0;

        MhwStubMessage();
        return;
    }

    virtual void RegisterParams(void *params)
    {
        MOS_UNUSED(params);
        MhwStubMessage();
    }

    virtual MOS_STATUS UpdateParams(bool isInput)
    {
        MOS_UNUSED(isInput);
        MhwStubMessage();
        return MOS_STATUS_SUCCESS;
    }

    virtual void SetCpSecurityType(
        CP_SECURITY_TYPE type = static_cast<CP_SECURITY_TYPE>(1))
    {
        MOS_UNUSED(type);
        MhwStubMessage();
    }

    virtual MOS_STATUS GetCounterValue(uint32_t *ctr)
    {
        MhwStubMessage();
        return MOS_STATUS_SUCCESS;
    }
};

//!
//! \brief    Create MhwCpInterface Object according CPLIB loading status
//!           Must use Delete_MhwCpInterface to delete created Object to avoid ULT Memory Leak errors
//!
//! \return   Return CP Wrapper Object if CPLIB not loaded
//!
MhwCpInterface* Create_MhwCpInterface(PMOS_INTERFACE osInterface);

//!
//! \brief    Delete the MhwCpInterface Object according CPLIB loading status
//!
//! \param    [in] *pMhwCpInterface 
//!           MhwCpInterface
//!
void Delete_MhwCpInterface(MhwCpInterface* pMhwCpInterface);
#endif
