/*
* Copyright (c) 20019, Intel Corporation
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
//! \file      hal_kerneldll_g12hp.c
//! \brief         Kernel Dynamic Linking/Loading routines for XeHP
//!

#include "hal_kerneldll.h"
#include "vphal.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

//---------------------------------------------------------------------------------------
// KernelDll_StartKernelSearch_g12hp - Starts kernel search
//
// Parameters:
//    Kdll_State       *pState       - [in]     Dynamic Linking State
//    Kdll_FilterEntry *pFilter      - [in]     Search filter (array of search entries)
//    int               iFilterSize  - [in]     Search filter size
//    Kdll_SearchState *pSearchState - [in/out] Kernel search state
//
// Output: none
//---------------------------------------------------------------------------------------
void KernelDll_StartKernelSearch_g12hp(
    Kdll_State       *pState,
    Kdll_SearchState *pSearchState,
    Kdll_FilterEntry *pFilter,
    int32_t          iFilterSize,
    uint32_t         uiIs64BInstrEnabled)
{
    int32_t nLayer;

    VPHAL_RENDER_FUNCTION_ENTER;

    // Reset all states
    MOS_ZeroMemory(pSearchState, sizeof(Kdll_SearchState));

    // Setup KDLL state
    pSearchState->pKdllState    = pState;     // KDLL state

    // Cleanup kernel table
    pSearchState->KernelCount   = 0;          // # of kernels

    // Cleanup patch data
    memset(pSearchState->Patches ,  0, sizeof(pSearchState->Patches));
    memset(pSearchState->PatchID , -1, sizeof(pSearchState->PatchID));
    pSearchState->PatchCount = 0;

    // Copy original filter; filter will be modified as part of the search
    if (pFilter && iFilterSize > 0)
    {
        MOS_SecureMemcpy(pSearchState->Filter, iFilterSize * sizeof(Kdll_FilterEntry), pFilter, iFilterSize * sizeof(Kdll_FilterEntry));
        pSearchState->pFilter      = pSearchState->Filter;
        pSearchState->iFilterSize  = iFilterSize;

        // Copy the render target format
        pSearchState->target_format = pSearchState->pFilter[iFilterSize - 1].format;

        // Copy the render target tile type
        pSearchState->target_tiletype = pSearchState->pFilter[iFilterSize - 1].tiletype;

        // Indicate whether to use 64B save kernel for render target surface
        if (uiIs64BInstrEnabled                               &&
            ((pSearchState->target_tiletype == MOS_TILE_X)    ||
            (pSearchState->target_tiletype  == MOS_TILE_LINEAR)))
        {
            pSearchState->b64BSaveEnabled = true;
        }
    }
}

#ifdef __cplusplus
}
#endif // __cplusplus

void KernelDll_ModifyFunctionPointers_g12hp(Kdll_State *pState)
{
    pState->pfnStartKernelSearch = KernelDll_StartKernelSearch_g12hp;
}
