/*
* Copyright (c) 2008-2017, Intel Corporation
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
//! \file      hal_kerneldll.h 
//! \brief         Fast Compositing dynamic kernel linking/loading definitions 
//!
#ifndef __HAL_KERNELDLL_H__
#define __HAL_KERNELDLL_H__

#include "mos_defs.h"
#include "cm_fc_ld.h"
#include "hal_kerneldll_next.h"
#include "vphal_common.h"


#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef enum _MOS_FORMAT            Kdll_Format;

//--------------------------------------------------------------
// Kernel DLL structures
//--------------------------------------------------------------

//------------------------------------------------------------
// Component kernel descriptors (equivalent to KDT)
//------------------------------------------------------------
// Component kernel linking information
typedef struct tagKdll_Linking
{
    int              iKUID;                 // Component Kernel Unique ID
    uint32_t         bExport         :  1;  // Export (1) / Import (0)
    uint32_t         bInline         :  1;  // Inline(1)  / Function (0)
    uint32_t                         :  2;  // - (MBZ)
    uint32_t         iLabelID        : 12;  // Label ID
    uint32_t         dwOffset        : 16;  // Instruction offset
} Kdll_Linking, *pKdll_Linking;

// Get component/static kernel
Kdll_CacheEntry *
KernelDll_GetComponentKernel(Kdll_State *pState,
                             int         iKUID);

bool KernelDll_IsSameFormatType(MOS_FORMAT   format1, MOS_FORMAT   format2);

#if _DEBUG || EMUL

// Debugging strings for standalone application or debug driver
const char    *KernelDll_GetLayerString        (Kdll_Layer       layer);
const char    *KernelDll_GetFormatString       (MOS_FORMAT       format);
const char    *KernelDll_GetCSpaceString       (VPHAL_CSPACE     cspace);
const char    *KernelDll_GetSamplingString     (Kdll_Sampling    sampling);
const char    *KernelDll_GetRotationString     (VPHAL_ROTATION   rotation);
const char    *KernelDll_GetProcessString      (Kdll_Processing  process);
const char    *KernelDll_GetParserStateString  (Kdll_ParserState state);
const char    *KernelDll_GetRuleIDString       (Kdll_RuleID      RID);
const char    *KernelDll_GetCoeffIDString      (Kdll_CoeffID     CID);

int32_t KernelDll_PrintRule(
    char                    *szOut,
    int                     iSize,
    const Kdll_RuleEntry    *pEntry,
    Kdll_KernelCache        *pCache);

#endif // _DEBUG || EMUL

#ifdef __cplusplus
}
#endif

#endif // __HAL_KERNELDLL_H__