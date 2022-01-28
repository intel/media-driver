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
// Kernel IDs and Kernel Names
#include "vpkrnheader.h" // IDR_VP_TOTAL_NUM_KERNELS
#include "hal_kerneldll_next.h"

#if EMUL

#include "support.h"

// Search callback codes
#define CB_REASON_SEARCH_FAILED -1
#define CB_REASON_UPDATE_FAILED -2
#define CB_REASON_BEGIN_SEARCH   0
#define CB_REASON_BEGIN_UPDATE   1
#define CB_REASON_END_SEARCH     2

#else // EMUL

#endif // EMUL

#include "vphal_common.h"

#define ROUND_FLOAT(n, factor) ( (n) * (factor) + (((n) > 0.0f) ? 0.5f : -0.5f) )

#define MIN_SHORT -32768.0f
#define MAX_SHORT  32767.0f
#define FLOAT_TO_SHORT(n)      (short)(MOS_MIN(MOS_MAX(MIN_SHORT, n), MAX_SHORT))

#define DL_MAX_SEARCH_NODES_PER_KERNEL  6        // max number of search nodes for a component kernel (max tree depth)
#define DL_MAX_COMPONENT_KERNELS        25       // max number of component kernels that can be combined
#define DL_MAX_EXPORT_COUNT             64       // size of the symbol export table
#define DL_DEFAULT_COMBINED_KERNELS     4        // Default number of kernels in cache
#define DL_NEW_COMBINED_KERNELS         4        // The increased number of kernels in cache each time
#define DL_CACHE_BLOCK_SIZE             (128*1024)   // Kernel allocation block size
#define DL_COMBINED_KERNEL_CACHE_SIZE   (DL_CACHE_BLOCK_SIZE*DL_NEW_COMBINED_KERNELS) // Combined kernel size

#define DL_PROCAMP_DISABLED             -1       // procamp is disabled
#define DL_PROCAMP_MAX                   1       // 1 Procamp entry

#define DL_CSC_DISABLED                 -1       // CSC is disabled

#define DL_CSC_MAX_G5                    2       // 2 CSC matrices max for Gen5

#define DL_CHROMASITING_DISABLE         -1       // Chromasiting is disabled

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef enum _MOS_FORMAT            Kdll_Format;

// Rotation Mode
typedef enum tagKdll_Rotation
{
    Rotate_Source
} Kdll_Rotation;

#define ColorFill_Source        -1
#define ColorFill_False         0
#define ColorFill_True          1

#define LumaKey_Source          -1
#define LumaKey_False           0
#define LumaKey_True            1

#define Procamp_Source          -1

// Dynamic Linking rule definitions
#define RID_IS_MATCH(rid)    ((rid & 0xFE00) == 0x0000)
#define RID_IS_SET(rid)      ((rid & 0xFE00) == 0x0200)
#define RID_IS_EXTENDED(rid) ((rid & 0xFD00) == 0x0100)

// Parameters for RID_Op_NewEntry
#define RULE_DEFAULT        0
#define RULE_CUSTOM         1
#define RULE_NO_OVERRIDE    255

#define GROUP_DEFAULT       RULE_DEFAULT
#define GROUP_CUSTOM        RULE_CUSTOM
#define GROUP_NO_OVERRIDE   RULE_NO_OVERRIDE

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

// Kernel patches
typedef enum tagKdll_PatchKind
{
    PatchKind_None           = 0,
    PatchKind_CSC_Coeff_Src0 = 1,
    PatchKind_CSC_Coeff_Src1 = 2,
} Kdll_PatchKind;

// Patch rule entry (rule extension)
typedef struct tagKdll_PatchRuleEntry
{
    uint32_t Dest   : 16 ;   // Patch destination in bytes (LSB)
    uint32_t Source : 8  ;   // Patch data source in bytes
    uint32_t Size   : 8  ;   // Patch size in bytes (MSB)
} Kdll_PatchRuleEntry;

extern const char  *g_cInit_ComponentNames[];

//------------------------------------------------------------
// KERNEL CACHE / LINK
//------------------------------------------------------------
// Import/export structure from kernel binary file
#pragma pack(4)
typedef struct tagKdll_LinkFileHeader
{
    uint32_t dwVersion;
    uint32_t dwSize;
    uint32_t dwImports;
    uint32_t dwExports;
} Kdll_LinkFileHeader;
#pragma pack()

//---------------------------------
// Kernel DLL function prototypes
//---------------------------------

bool KernelDll_IsYUVFormat(MOS_FORMAT   format);

bool KernelDll_IsFormat(
    MOS_FORMAT      format,
    VPHAL_CSPACE     cspace,
    MOS_FORMAT      match);

VPHAL_CSPACE KernelDll_TranslateCspace(VPHAL_CSPACE cspace);

bool KernelDll_MapCSCMatrix(
    Kdll_CSCType     type,
    const float      *matrix,
    short            *coeff);

// Kernel Rule Search / State Update
bool KernelDll_FindRule(
    Kdll_State       *pState,
    Kdll_SearchState *pSearchState);

bool KernelDll_UpdateState(
    Kdll_State       *pState,
    Kdll_SearchState *pSearchState);

// Simple Hash function
uint32_t KernelDll_SimpleHash(
    void            *pData,
    int             iSize);

//---------------------------------------------------------------------------------------
// KernelDll_SetupFunctionPointers - Setup Function pointers based on platform
//
// Parameters:
//    char  *pState    - [in] Kernel Dll state
//           platform  - [in] platform
//
// Output: true  - Function pointers are set
//         false - Failed to setup function pointers (invalid platform)
//-----------------------------------------------------------------------------------------
static bool KernelDll_SetupFunctionPointers(
    Kdll_State  *pState,
    void(*ModifyFunctionPointers)(PKdll_State));

// Allocate Kernel Dll State
Kdll_State *KernelDll_AllocateStates(
    void                 *pKernelCache,
    uint32_t             uKernelCacheSize,
    void                 *pFcPatchCache,
    uint32_t             uFcPatchCacheSize,
    const Kdll_RuleEntry *pInternalRules,
    void(*ModifyFunctionPointers)(PKdll_State));

// Release Kernel Dll State
void  KernelDll_ReleaseStates(Kdll_State *pState);

// Setup Kernel Dll Procamp Parameters
void KernelDll_SetupProcampParameters(Kdll_State    *pState,
                                      Kdll_Procamp  *pProcamp,
                                      int            iProcampSize);

// Update CSC coefficients
void KernelDll_UpdateCscCoefficients(Kdll_State      *pState,
                                     Kdll_CSC_Matrix *pMatrix);

// Find Kernel in hash table
Kdll_CacheEntry *
KernelDll_GetCombinedKernel(Kdll_State       *pState,
                            Kdll_FilterEntry *iFilter,
                            int               iFilterSize,
                            uint32_t          dwHash);

// Get component/static kernel
Kdll_CacheEntry *
KernelDll_GetComponentKernel(Kdll_State *pState,
                             int         iKUID);

// Allocate cache entry for a given size
Kdll_CacheEntry *
KernelDll_AllocateCacheEntry(Kdll_KernelCache *pCache,
                             int32_t           iSize);

// Allocate more kernel cache entries
Kdll_CacheEntry *
KernelDll_AllocateAdditionalCacheEntries(Kdll_KernelCache *pCache);

//Release the additional kernel cache entries
void KernelDll_ReleaseAdditionalCacheEntries(Kdll_KernelCache *pCache);

// Add kernel to cache and hash table
Kdll_CacheEntry *
KernelDll_AddKernel(Kdll_State       *pState,
                    Kdll_SearchState *pSearchState,
                    Kdll_FilterEntry *pFilter,
                    int               iFilterSize,
                    uint32_t          dwHash);

// Search kernel, output is in pSearchState
bool KernelDll_SearchKernel(
    Kdll_State          *pState,
    Kdll_SearchState    *pSearchState);

// Build kernel in SearchState
bool KernelDll_BuildKernel(Kdll_State *pState, Kdll_SearchState *pSearchState);

bool KernelDll_SetupCSC(
    Kdll_State       *pState,
    Kdll_SearchState *pSearchState);

bool KernelDll_IsSameFormatType(MOS_FORMAT   format1, MOS_FORMAT   format2);
void KernelDll_ReleaseHashEntry(Kdll_KernelHashTable *pHashTable, uint16_t entry);
void KernelDll_ReleaseCacheEntry(Kdll_KernelCache *pCache, Kdll_CacheEntry  *pEntry);

//---------------------------------------------------------------------------------------
// KernelDll_SetupFunctionPointers_Ext - Setup Extension Function pointers
//
// Parameters:
//    KdllState  *pState    - [in/out] Kernel Dll state
//
// Output: true  - Function pointers are set
//         false - Failed to setup function pointers (invalid platform)
//-----------------------------------------------------------------------------------------
bool KernelDll_SetupFunctionPointers_Ext(
    Kdll_State  *pState);

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
