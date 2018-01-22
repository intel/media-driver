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
//! \file      hal_kerneldll_g8.c 
//! \brief         Kernel Dynamic Linking/Loading routines for Gen8 
//!

#include "hal_kerneldll.h"
#include "vphal.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

//---------------------------------------------------------------------------------------
// KernelDll_StartKernelSearch_g8 - Starts kernel search
//
// Parameters:
//    Kdll_State       *pState       - [in]     Dynamic Linking State
//    Kdll_FilterEntry *pFilter      - [in]     Search filter (array of search entries)
//    int               iFilterSize  - [in]     Search filter size
//    Kdll_SearchState *pSearchState - [in/out] Kernel search state
//
// Output: none
//---------------------------------------------------------------------------------------
void KernelDll_StartKernelSearch_g8(
    Kdll_State       *pState,
    Kdll_SearchState *pSearchState,
    Kdll_FilterEntry *pFilter,
    int32_t          iFilterSize,
    uint32_t         uiIs64BInstrEnabled)
{
    int32_t nLayer;
    bool AtLeastOneSample8x8Layer  = false;
    bool AtLeastOneSampleNormLayer = false;

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

        // Set Procamp
        if (pFilter->procamp == DL_PROCAMP_DISABLED)
        {
            pSearchState->bProcamp = false;
        }
        else
        {
            pSearchState->bProcamp = true;
        }

        // Copy the render target format
        pSearchState->target_format = pSearchState->pFilter[iFilterSize - 1].format;

        // Copy the render target tile type
        pSearchState->target_tiletype = pSearchState->pFilter[iFilterSize - 1].tiletype;

        // PreComp L0 Rotation Optimization
        // In Multilayer case, if L0 is the only layer that rotates, then do preComp Rotation of L0
        // ie Rotate L0 after Scaling and DONT rotate the RT after Composition. This makes it possible to
        // perform rotation and Composition in single phase.
        pSearchState->bRTRotate = true;

        // PreComp rotation is possible only if there are more than 2 filters/layers.
        // Otherwise RT rotation is done
        if (iFilterSize > 2)
        {
            for (nLayer = 1; nLayer < iFilterSize - 1; nLayer++)
            {
                if (pSearchState->pFilter[nLayer].rotation == VPHAL_ROTATION_IDENTITY)
                {
                    // If any layer except L0 needs rotation then rotate RT and skip preComp L0 Rotation
                    pSearchState->bRTRotate = false;
                    pSearchState->pFilter[iFilterSize-1].rotation = VPHAL_ROTATION_IDENTITY;
                    break;
                 }
             }
        }

        // Gen8 requires shuffling the data returned by Sample_8x8 to match the
        // data layout of other sampler messages.
        // If at least one layer uses 8x8 sampling and one layer does not,
        // all layers that use 8x8 sampling needs their output shuffled.
        // If all layers use 8x8 sampling, only shuffle the RenderTarget layer
        // to save extra work in shuffling all layers.

        // Go through all layers except RenderTarget
        for (nLayer = 0; nLayer < iFilterSize - 1; nLayer++)
        {
            if (pSearchState->pFilter[nLayer].sampler >= Sample_Scaling_AVS)
            {
                AtLeastOneSample8x8Layer = true;
            }
            else
            {
                AtLeastOneSampleNormLayer = true;
            }
        }

        // All layers are sample 8x8 messages. Adjust the data in Render Target Layer.
        // If preComp rotation is applied to layer0 then RenderTarget does not rotate (bRTRotate=FALSE).
        // layer0 needs to be shuffled before it can be rotated.
        // So if layer0 shuffles then all layers need shuffling (Shuffle_All_8x8_Layer)
        // When PreComp rotation is NOT applied to layer0 then bRTRotate=TRUE
        // indicating only RenderTarget is rotated in which case only RenderTarget is shuffled(Shuffle_RenderTarget).
        if (AtLeastOneSample8x8Layer && !AtLeastOneSampleNormLayer && pSearchState->bRTRotate == true)
        {
            for (nLayer = 0; nLayer < iFilterSize - 1; nLayer++)
            {
                if (pSearchState->pFilter[nLayer].format == Format_NV12 && pSearchState->pFilter[nLayer].sampler == Sample_iScaling_AVS)
                {
                    pSearchState->ShuffleSamplerData = Shuffle_None;
                }
                else
                {
                    pSearchState->ShuffleSamplerData = Shuffle_RenderTarget;
                }
            }
        }
        // There are mix of sampler message types. Adjust the data in all sample 8x8 layer.
        else if (AtLeastOneSample8x8Layer && AtLeastOneSampleNormLayer)
        {
            pSearchState->ShuffleSamplerData = Shuffle_All_8x8_Layer;
        }
        else
        {
            pSearchState->ShuffleSamplerData = Shuffle_None;
        }

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

void KernelDll_ModifyFunctionPointers_g8(Kdll_State *pState)
{
    pState->pfnStartKernelSearch = KernelDll_StartKernelSearch_g8;
}
