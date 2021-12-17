/*
* Copyright (c) 2020, Intel Corporation
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
//! \file      hal_kerneldll_xe_xpm_plus.c
//! \brief         Kernel Dynamic Linking/Loading routines for XeHP
//!

#include "hal_kerneldll.h"
#include "vphal.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/*----------------------------------------------------------------------------
| Name      : KernelDll_FindRule_Xe_Xpm_Plus
| Purpose   : Find a rule that matches the current search/input state
|
| Input     : pState       - Kernel Dll state
|             pSearchState - current DL search state
|
| Return    :
\---------------------------------------------------------------------------*/
bool KernelDll_FindRule_Xe_Xpm_Plus(
    Kdll_State       *pState,
    Kdll_SearchState *pSearchState)
{
    uint32_t parser_state = (uint32_t)pSearchState->state;
    Kdll_RuleEntrySet    *pRuleSet;
    const Kdll_RuleEntry *pRuleEntry;
    int32_t              iRuleCount;
    int32_t              iMatchCount;
    bool                 bLayerFormatMatched;
    bool                 bSrc0FormatMatched;
    bool                 bSrc1FormatMatched;
    bool                 bTargetFormatMatched;
    bool                 bSrc0SampingMatched;
    bool                 bScalingRatioMatched;
    bool                 bCoeffMatched;

    VPHAL_RENDER_FUNCTION_ENTER;

    // All Custom states are handled as a single group
    if (parser_state >= Parser_Custom)
    {
        parser_state = Parser_Custom;
    }

    pRuleSet   = pState->pDllRuleTable[parser_state];
    iRuleCount = pState->iDllRuleCount[parser_state];

    if (pRuleSet == nullptr || iRuleCount == 0)
    {
        VPHAL_RENDER_NORMALMESSAGE("Search rules undefined.");
        pSearchState->pMatchingRuleSet = nullptr;
        return false;
    }

    // Search matching entry
    for ( ; iRuleCount > 0; iRuleCount--, pRuleSet++)
    {
        // Points to the first rule, get number of matches
        pRuleEntry  = pRuleSet->pRuleEntry;
        iMatchCount = pRuleSet->iMatchCount;

        // Initialize for each Ruleset
        bLayerFormatMatched  = false;
        bSrc0FormatMatched   = false;
        bSrc1FormatMatched   = false;
        bTargetFormatMatched = false;
        bSrc0SampingMatched  = false;
        bScalingRatioMatched  = false;
        bCoeffMatched         = false;

        // Match all rules within the same RuleSet
        for (; iMatchCount > 0; iMatchCount--, pRuleEntry++)
        {
            switch (pRuleEntry->id)
            {
                // Match current Parser State
                case RID_IsParserState:
                    if (pSearchState->state == (Kdll_ParserState) pRuleEntry->value)
                    {
                        continue;
                    }
                    else
                    {
                        break;
                    }

                // Match render method
                case RID_IsRenderMethod:
                    if (pSearchState->pFilter->RenderMethod == (Kdll_RenderMethod)pRuleEntry->value)
                    {
                        continue;
                    }
                    else
                    {
                        break;
                    }

                // Match target color space
                case RID_IsTargetCspace:
                    if (KernelDll_IsCspace(pSearchState->cspace, (VPHAL_CSPACE) pRuleEntry->value))
                    {
                        continue;
                    }
                    else
                    {
                        break;
                    }

                // Match current layer ID
                case RID_IsLayerID:
                    if (pSearchState->pFilter->layer == (Kdll_Layer) pRuleEntry->value)
                    {
                        continue;
                    }
                    else
                    {
                        break;
                    }

                // Match current layer format
                case RID_IsLayerFormat:
                    if (pRuleEntry->logic == Kdll_Or && bLayerFormatMatched)
                    {
                        // Already found matching format in the ruleset
                        continue;
                    }
                    else
                    {
                        // Check if the layer format matches the rule
                        if (KernelDll_IsFormat(pSearchState->pFilter->format,
                                                pSearchState->pFilter->cspace,
                                                (MOS_FORMAT  ) pRuleEntry->value))
                        {
                            bLayerFormatMatched = true;
                        }

                        if (pRuleEntry->logic == Kdll_None && !bLayerFormatMatched)
                        {
                            // Last entry and No matching format was found
                            break;
                        }
                        else
                        {
                            continue;
                        }
                    }

                // Match shuffling requirement
                case RID_IsShuffling:
                    if (pSearchState->ShuffleSamplerData == (Kdll_Shuffling) pRuleEntry->value)
                    {
                        continue;
                    }
                    else
                    {
                        break;
                    }

                // Check if RT rotates
                case RID_IsRTRotate:
                    if (pSearchState->bRTRotate == (pRuleEntry->value ? true : false) )
                    {
                        continue;
                    }
                    else
                    {
                        break;
                    }

                // Match current layer rotation
                case RID_IsLayerRotation:
                    if (pSearchState->pFilter->rotation == (VPHAL_ROTATION) pRuleEntry->value)
                    {
                        continue;
                    }
                    else
                    {
                        break;
                    }

                // Match Src0 source format (surface)
                case RID_IsSrc0Format:
                    if (pRuleEntry->logic == Kdll_Or && bSrc0FormatMatched)
                    {
                        // Already found matching format in the ruleset
                        continue;
                    }
                    else
                    {
                        // Check if the source 0 format matches the rule
                        // The intermediate colorspace is used to determine
                        // if palettized input is given in RGB or YUV format.
                        if (KernelDll_IsFormat(pSearchState->src0_format,
                                                pSearchState->cspace,
                                                (MOS_FORMAT  ) pRuleEntry->value))
                        {
                            bSrc0FormatMatched = true;
                        }

                        if (pRuleEntry->logic == Kdll_None && !bSrc0FormatMatched)
                        {
                            // Last entry and No matching format was found
                            break;
                        }
                        else
                        {
                            continue;
                        }
                    }

                // Match Src0 sampling mode
                case RID_IsSrc0Sampling:
                    // Check if the layer format matches the rule
                    if (pSearchState->src0_sampling == (Kdll_Sampling) pRuleEntry->value)
                    {
                        bSrc0SampingMatched = true;
                        continue;
                    }
                    else if (bSrc0SampingMatched || pRuleEntry->logic == Kdll_Or)
                    {
                        continue;
                    }
                    else if ((Kdll_Sampling) pRuleEntry->value == Sample_Any &&
                            pSearchState->src0_sampling != Sample_None)
                    {
                        continue;
                    }
                    else
                    {
                        break;
                    }

                // Match Src0 rotation
                case RID_IsSrc0Rotation:
                    if (pSearchState->src0_rotation == (VPHAL_ROTATION) pRuleEntry->value)
                    {
                        continue;
                    }
                    else
                    {
                        break;
                    }

                // Match Src0 Colorfill
                case RID_IsSrc0ColorFill:
                    if (pSearchState->src0_colorfill == (int32_t)pRuleEntry->value)
                    {
                        continue;
                    }
                    else
                    {
                        break;
                    }

                // Match Src0 Luma Key
                case RID_IsSrc0LumaKey:
                    if (pSearchState->src0_lumakey == (int32_t)pRuleEntry->value)
                    {
                        continue;
                    }
                    else
                    {
                        break;
                    }

                // Match Src0 Procamp
                case RID_IsSrc0Procamp:
                    if (pSearchState->pFilter->procamp == (int32_t)pRuleEntry->value)
                    {
                        continue;
                    }
                    else
                    {
                        break;
                    }

                // Match Src0 CSC coefficients
                case RID_IsSrc0Coeff:
                      if (pRuleEntry->logic == Kdll_Or && bCoeffMatched)
                    {
                        // Already found matching coeff in the ruleset
                        continue;
                    }
                    else
                    {
                        if (pSearchState->src0_coeff == (Kdll_CoeffID)pRuleEntry->value)
                        {
                             bCoeffMatched = true;
                        }
                        else if ((Kdll_CoeffID)pRuleEntry->value == CoeffID_Any &&
                            pSearchState->src0_coeff != CoeffID_None)
                        {
                             bCoeffMatched = true;
                        }

                        if (pRuleEntry->logic == Kdll_None && !bCoeffMatched)
                        {
                            // Last entry and No matching format was found
                            break;
                        }
                        else
                        {
                            continue;
                        }
                    }

                // Match Src0 CSC coefficients setting mode
                case RID_IsSetCoeffMode:
                    if (pSearchState->pFilter->SetCSCCoeffMode == (Kdll_SetCSCCoeffMethod) pRuleEntry->value)
                    {
                        continue;
                    }
                    else
                    {
                        break;
                    }

                // Match Src0 processing mode
                case RID_IsSrc0Processing:
                    if (pSearchState->src0_process == (Kdll_Processing) pRuleEntry->value)
                    {
                        continue;
                    }
                    if ((Kdll_Processing) pRuleEntry->value == Process_Any &&
                        pSearchState->src0_process != Process_None)
                    {
                        continue;
                    }
                    else
                    {
                        break;
                    }

                // Match Src0 chromasiting mode
                case RID_IsSrc0Chromasiting:
                    if (pSearchState->Filter->chromasiting == (int32_t)pRuleEntry->value)
                    {
                        continue;
                    }
                    else
                    {
                        break;
                    }

                // Match Src1 source format (surface)
                case RID_IsSrc1Format:
                    if (pRuleEntry->logic == Kdll_Or && bSrc1FormatMatched)
                    {
                        // Already found matching format in the ruleset
                        continue;
                    }
                    else
                    {
                        // Check if the source 1 format matches the rule
                        // The intermediate colorspace is used to determine
                        // if palettized input is given in RGB or YUV format.
                        if (KernelDll_IsFormat(pSearchState->src1_format,
                                                pSearchState->cspace,
                                                (MOS_FORMAT) pRuleEntry->value))
                        {
                            bSrc1FormatMatched = true;
                        }

                        if (pRuleEntry->logic == Kdll_None && !bSrc1FormatMatched)
                        {
                            // Last entry and No matching format was found
                            break;
                        }
                        else
                        {
                            continue;
                        }
                    }
                // Match Src1 sampling mode
                case RID_IsSrc1Sampling:
                    if (pSearchState->src1_sampling == (Kdll_Sampling) pRuleEntry->value)
                    {
                        continue;
                    }
                    else if ((Kdll_Sampling) pRuleEntry->value == Sample_Any &&
                            pSearchState->src1_sampling != Sample_None)
                    {
                        continue;
                    }
                    else
                    {
                        break;
                    }

                // Match Src1 Luma Key
                case RID_IsSrc1LumaKey:
                    if (pSearchState->src1_lumakey == (int32_t)pRuleEntry->value)
                    {
                        continue;
                    }
                    else
                    {
                        break;
                    }
                // Match Src1 Sampler LumaKey
                case RID_IsSrc1SamplerLumaKey:
                    if (pSearchState->src1_samplerlumakey == (int32_t)pRuleEntry->value)
                    {
                        continue;
                    }
                    else
                    {
                        break;
                    }

                // Match Src1 Procamp
                case RID_IsSrc1Procamp:
                    if (pSearchState->pFilter->procamp == (int32_t)pRuleEntry->value)
                    {
                        continue;
                    }
                    else
                    {
                        break;
                    }

                // Match Src1 CSC coefficients
                case RID_IsSrc1Coeff:
                    if (pSearchState->src1_coeff == (Kdll_CoeffID) pRuleEntry->value)
                    {
                        continue;
                    }
                    else if ((Kdll_CoeffID) pRuleEntry->value == CoeffID_Any &&
                            pSearchState->src1_coeff != CoeffID_None)
                    {
                        continue;
                    }
                    else
                    {
                        break;
                    }

                // Match Src1 processing mode
                case RID_IsSrc1Processing:
                    if (pSearchState->src1_process == (Kdll_Processing) pRuleEntry->value)
                    {
                        continue;
                    }
                    if ((Kdll_Processing) pRuleEntry->value == Process_Any &&
                        pSearchState->src1_process != Process_None)
                    {
                        continue;
                    }
                    else
                    {
                        break;
                    }

                // Match Src1 chromasiting mode
                case RID_IsSrc1Chromasiting:
                    //pSearchState->pFilter is pointed to the real sub layer
                    if (pSearchState->pFilter->chromasiting == (int32_t)pRuleEntry->value)
                    {
                        continue;
                    }
                    else
                    {
                        break;
                    }

                // Match Layer number
                case RID_IsLayerNumber:
                    if (pSearchState->layer_number == (int32_t) pRuleEntry->value)
                    {
                        continue;
                    }
                    else
                    {
                        break;
                    }

                // Match quadrant
                case RID_IsQuadrant:
                    if (pSearchState->quadrant == (int32_t) pRuleEntry->value)
                    {
                        continue;
                    }
                    else
                    {
                        break;
                    }

                // Set CSC flag before Mix
                case RID_IsCSCBeforeMix:
                    if (pSearchState->bCscBeforeMix == (pRuleEntry->value ? true : false))
                    {
                        continue;
                    }
                    else
                    {
                        break;
                    }

                case RID_IsDualOutput:
                    if (pSearchState->pFilter->dualout == (pRuleEntry->value ? true : false))
                    {
                        continue;
                    }
                    else
                    {
                        break;
                    }

                case RID_IsTargetFormat:
                    if (pRuleEntry->logic == Kdll_Or && bTargetFormatMatched)
                    {
                        // Already found matching format in the ruleset
                        continue;
                    }
                    else
                    {
                        if (pSearchState->target_format == (MOS_FORMAT) pRuleEntry->value)
                        {
                            bTargetFormatMatched = true;
                        }

                        if (pRuleEntry->logic == Kdll_None && !bTargetFormatMatched)
                        {
                            // Last entry and No matching format was found
                            break;
                        }
                        else
                        {
                            continue;
                        }
                    }

                case RID_Is64BSaveEnabled:
                    if (pSearchState->b64BSaveEnabled == (pRuleEntry->value ? true : false))
                    {
                        continue;
                    }
                    else
                    {
                        break;
                    }

                case RID_IsTargetTileType:
                    if (pRuleEntry->logic == Kdll_None &&
                        pSearchState->target_tiletype == (MOS_TILE_TYPE) pRuleEntry->value)
                    {
                        continue;
                    }
                    else if (pRuleEntry->logic == Kdll_Not &&
                             pSearchState->target_tiletype != (MOS_TILE_TYPE) pRuleEntry->value)
                    {
                        continue;
                    }
                    else
                    {
                        break;
                    }

                case RID_IsProcampEnabled:
                    if (pSearchState->bProcamp == (pRuleEntry->value ? true : false))
                    {
                        continue;
                    }
                    else
                    {
                        break;
                    }

                case RID_IsConstOutAlpha:
                    if (pSearchState->pFilter->bFillOutputAlphaWithConstant == (pRuleEntry->value ? true : false))
                    {
                        continue;
                    }
                    else
                    {
                        break;
                    }

                case RID_IsDitherNeeded:
                    if (pSearchState->pFilter->bIsDitherNeeded == (pRuleEntry->value ? true : false))
                    {
                        continue;
                    }
                    else
                    {
                        break;
                    }
                case RID_IsScalingRatio:
                    if (pRuleEntry->logic == Kdll_Or && bScalingRatioMatched)
                    {
                        // Already found matching format in the ruleset
                        continue;
                    }
                    else
                    {
                        if (pSearchState->pFilter->ScalingRatio == pRuleEntry->value)
                        {
                            bScalingRatioMatched = true;
                        }

                        if (pRuleEntry->logic == Kdll_None && !bScalingRatioMatched)
                        {
                            // Last entry and No matching format was found
                            break;
                        }
                        else
                        {
                            continue;
                        }
                    }

                // Undefined search rule will fail
                default:
                    VPHAL_RENDER_ASSERTMESSAGE("Invalid rule %d @ layer %d, state %d.", pRuleEntry->id, pSearchState->layer_number, pSearchState->state);
                    MT_ERR3(MT_VP_KERNEL_RULE, MT_VP_KERNEL_RULE_ID, pRuleEntry->id, MT_VP_KERNEL_RULE_LAYERNUM, 
                        pSearchState->layer_number, MT_VP_KERNEL_RULE_SEARCH_STATE, pSearchState->state);
                    break;
            }  // End of switch to deal with all matching rule IDs

            // Rule didn't match - try another RuleSet
            break;
        } // End of file loop to test all rules for the current RuleSet

        // Match
        if (iMatchCount == 0)
        {
            pSearchState->pMatchingRuleSet = pRuleSet;
            return true;
        }
    }   // End of for loop to test all RuleSets for the current parser state

    // Failed to find a matching rule -> kernel search will fail
    VPHAL_RENDER_NORMALMESSAGE("Fail to find a matching rule @ layer %d, state %d.", pSearchState->layer_number, pSearchState->state);
    MT_ERR2(MT_VP_KERNEL_RULE, MT_VP_KERNEL_RULE_LAYERNUM, pSearchState->layer_number, MT_VP_KERNEL_RULE_SEARCH_STATE, pSearchState->state);

    // No match -> return
    pSearchState->pMatchingRuleSet = nullptr;
    return false;
}

#ifdef __cplusplus
}
#endif // __cplusplus

void KernelDll_ModifyFunctionPointers_Xe_Xpm_Plus(Kdll_State *pState)
{
    pState->pfnFindRule = KernelDll_FindRule_Xe_Xpm_Plus;
}
