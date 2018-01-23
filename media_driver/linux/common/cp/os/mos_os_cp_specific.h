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
//! \file     mos_os_cp_specific.h
//! \brief    OS specific implement for CP related functions
//!

#ifndef __MOS_OS_CP_SPECIFIC_H__
#define __MOS_OS_CP_SPECIFIC_H__

#include "mos_defs.h"
#include "mos_os_hw.h"

typedef void* MOS_CP_COMMAND_PROPERTIES;
typedef void* PMOS_CP_CONTEXT;

class MosCpInterface
{
public:
    MosCpInterface(void  *pvOsInterface);

    ~MosCpInterface();

    MOS_STATUS RegisterPatchForHM(
        uint32_t       *pPatchAddress,
        bool           bWrite,
        MOS_HW_COMMAND HwCommandType,
        uint32_t       forceDwordOffset,
        void           *plResource,
        void           *pPatchLocationList);

    MOS_STATUS PermeatePatchForHM(
        void        *virt,
        void        *pvCurrentPatch,
        void        *resource);

    bool IsCpEnabled();

    void SetCpEnabled(bool bIsCpEnabled);

    bool IsHMEnabled();

    bool IsIDMEnabled();

    bool IsSMEnabled();

    bool IsTearDownHappen();

    MOS_STATUS SetResourceEncryption(
        void        *pResource,
        bool        bEncryption);

    bool IsHardwareProtectionRequired(
        void        *ppvOsResource[],
        uint32_t    uiNumOfResources,
        bool        bForceNoneCp);

    MOS_STATUS ResetHardwareProtectionState(
        void        *pvOsResource,
        bool        isShared);

    bool RenderBlockedFromCp();

    MOS_STATUS GetTK(
        uint32_t                    **ppTKs,
        uint32_t                    *pTKsSize,
        uint32_t                    *pTKsUpdateCnt);
};
#endif  // __MOS_OS_CP_SPECIFIC_H__
