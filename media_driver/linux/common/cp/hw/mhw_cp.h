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
//! \file     mhw_cp.h
//! \brief    MHW interface for content protection 
//! \details  Impelements the functionalities across all platforms for content protection
//!

#ifndef __MHW_CP_H__
#define __MHW_CP_H__

#include <cstdint>
#include <map>
#include <new>

#include "mhw_mi.h"
#include "mos_os.h"
#include "mos_util_debug.h"
#include "codec_def_common.h"

class MhwMiInterface;

class CpHwUnit;

typedef enum _CP_SECURITY_TYPE
{
    CP_SECURITY_NONE            = 1,
} CP_SECURITY_TYPE;

typedef enum _CP_MODE
{
    CP_TYPE_NONE              = 0,
    CP_TYPE_BASIC             = 2,
} CP_MODE;

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

class MhwCpInterface
{
public:
    MhwCpInterface();

    ~MhwCpInterface();

    static MhwCpInterface* CpFactory(
        PMOS_INTERFACE osInterface,
        MOS_STATUS&    eStatus);

    MOS_STATUS AddProlog(
        PMOS_INTERFACE      osInterface,
        PMOS_COMMAND_BUFFER cmdBuffer);

	MOS_STATUS IsHWCounterAutoIncrementEnforced(
		PMOS_INTERFACE osInterface);

    MOS_STATUS AddEpilog(
        PMOS_INTERFACE      osInterface,
        PMOS_COMMAND_BUFFER cmdBuffer);

    MOS_STATUS AddCheckForEarlyExit(
        PMOS_INTERFACE      osInterface,
        PMOS_COMMAND_BUFFER cmdBuffer);

    MOS_STATUS SetCpCopy(
        PMOS_INTERFACE      osInterface,
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMHW_CP_COPY_PARAMS params);

    MOS_STATUS SetMfxInlineStatusRead(
        PMOS_INTERFACE      osInterface,
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMOS_RESOURCE       resource,
        uint16_t            currentIndex,
        uint32_t            writeOffset);

    MOS_STATUS SetProtectionSettingsForMiFlushDw(
        PMOS_INTERFACE osInterface,
        void           *cmd);
        
    MOS_STATUS SetProtectionSettingsForMfxWait(
        PMOS_INTERFACE osInterface,
        void           *cmd);

    MOS_STATUS SetProtectionSettingsForMfxPipeModeSelect(uint32_t *data);

    MOS_STATUS SetProtectionSettingsForHcpPipeModeSelect(uint32_t *data, bool scalableEncode = false);

    MOS_STATUS SetProtectionSettingsForHucPipeModeSelect(uint32_t *data);

    MOS_STATUS SetMfxProtectionState(
        bool                        isDecodeInUse,
        PMOS_COMMAND_BUFFER         cmdBuffer,
        PMHW_BATCH_BUFFER           batchBuffer,
        PMHW_CP_SLICE_INFO_PARAMS   sliceInfoParam);

    MOS_STATUS RegisterMiInterface(
        MhwMiInterface* miInterface);

    void GetCpStateLevelCmdSize(uint32_t& cmdSize, uint32_t& patchListSize);

    void GetCpSliceLevelCmdSize(uint32_t& cmdSize, uint32_t& patchListSize);

    void RegisterParams(void* params);

    MOS_STATUS UpdateParams(bool isInput);

    bool IsCpEnabled() const;

    bool isHMEnabled() const;

    CP_MODE GetHostEncryptMode() const;
    
    void SetCpSecurityType(CP_SECURITY_TYPE type = CP_SECURITY_NONE);

    void SetCpForceDisabled(bool disabled);
};

MOS_STATUS Mhw_Cp_InitCpCmdProps(
    PLATFORM                     platform,
    const MOS_HW_COMMAND         hwCommandType,
    MOS_CP_COMMAND_PROPERTIES*   cpCmdProps,
    const uint32_t               forceDwordOffset);

MOS_STATUS Mhw_Cp_InitInterface(
    MhwCpInterface**   cpInterface,
    PMOS_INTERFACE     osInterface);

#endif
