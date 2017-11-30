/*
* Copyright (c) 2017, Intel Corporation
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
//! \file      codechal_common_vp9.c
//! \brief     This modules includes the common definitions for VP9 codec, across CODECHAL components.
//!

#include "codechal_common_vp9.h"
#include "codec_def_vp9_probs.h"

MOS_STATUS CodecHalVP9_CtxBufDiffInit(
    uint8_t             *ctxBuffer,
    bool                 setToKey)
{
    int32_t      i;
    int32_t      j;
    int32_t      k;
    uint32_t     byteCnt = CODECHAL_VP9_INTER_PROB_OFFSET;
    MOS_STATUS   eStatus = MOS_STATUS_SUCCESS;

    //inter mode probs. have to be zeros for Key frame
    for (i = 0; i < CODECHAL_VP9_INTER_MODE_CONTEXTS; i++)
    {
        for (j = 0; j < CODECHAL_VP9_INTER_MODES - 1; j++)
        {
            if (!setToKey)
            {
                ctxBuffer[byteCnt++] = DefaultInterModeProbs[i][j];
            }
            else
            {
                //zeros for key frame
                byteCnt++;
            }
        }
    }
    //switchable interprediction probs
    for (i = 0; i < CODECHAL_VP9_SWITCHABLE_FILTERS + 1; i++)
    {
        for (j = 0; j < CODECHAL_VP9_SWITCHABLE_FILTERS - 1; j++)
        {
            if (!setToKey)
            {
                ctxBuffer[byteCnt++] = DefaultSwitchableInterpProb[i][j];
            }
            else
            {
                //zeros for key frame
                byteCnt++;
            }
        }
    }
    //intra inter probs
    for (i = 0; i < CODECHAL_VP9_INTRA_INTER_CONTEXTS; i++)
    {
        if (!setToKey)
        {
            ctxBuffer[byteCnt++] = DefaultIntraInterProb[i];
        }
        else
        {
            //zeros for key frame
            byteCnt++;
        }
    }
    //comp inter probs
    for (i = 0; i < CODECHAL_VP9_COMP_INTER_CONTEXTS; i++)
    {
        if (!setToKey)
        {
            ctxBuffer[byteCnt++] = DefaultCompInterProb[i];
        }
        else
        {
            //zeros for key frame
            byteCnt++;
        }
    }
    //single ref probs
    for (i = 0; i < CODECHAL_VP9_REF_CONTEXTS; i++)
    {
        for (j = 0; j < 2; j++)
        {
            if (!setToKey)
            {
                ctxBuffer[byteCnt++] = DefaultSingleRefProb[i][j];
            }
            else
            {
                //zeros for key frame
                byteCnt++;
            }
        }
    }
    //comp ref probs
    for (i = 0; i < CODECHAL_VP9_REF_CONTEXTS; i++)
    {
        if (!setToKey)
        {
            ctxBuffer[byteCnt++] = DefaultCompRefProb[i];
        }
        else
        {
            //zeros for key frame
            byteCnt++;
        }
    }
    //y mode probs
    for (i = 0; i < CODECHAL_VP9_BLOCK_SIZE_GROUPS; i++)
    {
        for (j = 0; j < CODECHAL_VP9_INTRA_MODES - 1; j++)
        {
            if (!setToKey)
            {
                ctxBuffer[byteCnt++] = DefaultIFYProb[i][j];
            }
            else
            {
                //zeros for key frame, since HW will not use this buffer, but default right buffer.
                byteCnt++;
            }
        }
    }
    //partition probs, key & intra-only frames use key type, other inter frames use inter type
    for (i = 0; i < CODECHAL_VP9_PARTITION_CONTEXTS; i++)
    {
        for (j = 0; j < CODECHAL_VP9_PARTITION_TYPES - 1; j++)
        {
            if (setToKey)
            {
                ctxBuffer[byteCnt++] = DefaultKFPartitionProb[i][j];
            }
            else
            {
                ctxBuffer[byteCnt++] = DefaultPartitionProb[i][j];
            }
        }
    }
    //nmvc joints
    for (i = 0; i < (CODECHAL_VP9_MV_JOINTS - 1); i++)
    {
        if (!setToKey)
        {
            ctxBuffer[byteCnt++] = DefaultNmvContext.joints[i];
        }
        else
        {
            //zeros for key frame
            byteCnt++;
        }
    }
    //nmvc comps
    for (i = 0; i < 2; i++)
    {
        if (!setToKey)
        {
            ctxBuffer[byteCnt++] = DefaultNmvContext.comps[i].sign;
            for (j = 0; j < (CODECHAL_VP9_MV_CLASSES - 1); j++)
            {
                ctxBuffer[byteCnt++] = DefaultNmvContext.comps[i].classes[j];
            }
            for (j = 0; j < (CODECHAL_VP9_CLASS0_SIZE - 1); j++)
            {
                ctxBuffer[byteCnt++] = DefaultNmvContext.comps[i].class0[j];
            }
            for (j = 0; j < CODECHAL_VP9_MV_OFFSET_BITS; j++)
            {
                ctxBuffer[byteCnt++] = DefaultNmvContext.comps[i].bits[j];
            }
        }
        else
        {
            byteCnt += 1;
            byteCnt += (CODECHAL_VP9_MV_CLASSES - 1);
            byteCnt += (CODECHAL_VP9_CLASS0_SIZE - 1);
            byteCnt += (CODECHAL_VP9_MV_OFFSET_BITS);
        }
    }
    for (i = 0; i < 2; i++)
    {
        if (!setToKey)
        {
            for (j = 0; j < CODECHAL_VP9_CLASS0_SIZE; j++)
            {
                for (k = 0; k < (CODECHAL_VP9_MV_FP_SIZE - 1); k++)
                {
                    ctxBuffer[byteCnt++] = DefaultNmvContext.comps[i].class0_fp[j][k];
                }
            }
            for (j = 0; j < (CODECHAL_VP9_MV_FP_SIZE - 1); j++)
            {
                ctxBuffer[byteCnt++] = DefaultNmvContext.comps[i].fp[j];
            }
        }
        else
        {
            byteCnt += (CODECHAL_VP9_CLASS0_SIZE * (CODECHAL_VP9_MV_FP_SIZE - 1));
            byteCnt += (CODECHAL_VP9_MV_FP_SIZE - 1);
        }
    }
    for (i = 0; i < 2; i++)
    {
        if (!setToKey)
        {
            ctxBuffer[byteCnt++] = DefaultNmvContext.comps[i].class0_hp;
            ctxBuffer[byteCnt++] = DefaultNmvContext.comps[i].hp;
        }
        else
        {
            byteCnt += 2;
        }
    }

    //47 bytes of zeros
    byteCnt += 47;

    //uv mode probs
    for (i = 0; i < CODECHAL_VP9_INTRA_MODES; i++)
    {
        for (j = 0; j < CODECHAL_VP9_INTRA_MODES - 1; j++)
        {
            if (setToKey)
            {
                ctxBuffer[byteCnt++] = DefaultKFUVModeProb[i][j];
            }
            else
            {
                ctxBuffer[byteCnt++] = DefaultIFUVProbs[i][j];
            }
        }
    }

    return eStatus;
}

MOS_STATUS CodecHalVP9_ContextBufferInit(
    uint8_t             *ctxBuffer,
    bool                 setToKey)
{
    int32_t      i;
    int32_t      j;
    uint32_t     byteCnt = 0;
    uint8_t      blocktype = 0;
    uint8_t      reftype = 0;
    uint8_t      coeffbands = 0;
    uint8_t      unConstrainedNodes = 0;
    uint8_t      prevCoefCtx = 0;
    MOS_STATUS   eStatus = MOS_STATUS_SUCCESS;

    MOS_ZeroMemory(ctxBuffer, CODECHAL_VP9_SEG_PROB_OFFSET);

    //TX probs
    for (i = 0; i < CODECHAL_VP9_TX_SIZE_CONTEXTS; i++)
    {
        for (j = 0; j < CODECHAL_VP9_TX_SIZES - 3; j++)
        {
            ctxBuffer[byteCnt++] = DefaultTxProbs.p8x8[i][j];
        }
    }
    for (i = 0; i < CODECHAL_VP9_TX_SIZE_CONTEXTS; i++)
    {
        for (j = 0; j < CODECHAL_VP9_TX_SIZES - 2; j++)
        {
            ctxBuffer[byteCnt++] = DefaultTxProbs.p16x16[i][j];
        }
    }
    for (i = 0; i < CODECHAL_VP9_TX_SIZE_CONTEXTS; i++)
    {
        for (j = 0; j < CODECHAL_VP9_TX_SIZES - 1; j++)
        {
            ctxBuffer[byteCnt++] = DefaultTxProbs.p32x32[i][j];
        }
    }

    //52 bytes of zeros
    byteCnt += 52;

    //coeff probs
    for (blocktype = 0; blocktype < CODECHAL_VP9_BLOCK_TYPES; blocktype++)
    {
        for (reftype = 0; reftype < CODECHAL_VP9_REF_TYPES; reftype++)
        {
            for (coeffbands = 0; coeffbands < CODECHAL_VP9_COEF_BANDS; coeffbands++)
            {
                uint8_t numPrevCoeffCtxts = (coeffbands == 0) ? 3 : CODECHAL_VP9_PREV_COEF_CONTEXTS;
                for (prevCoefCtx = 0; prevCoefCtx < numPrevCoeffCtxts; prevCoefCtx++)
                {
                    for (unConstrainedNodes = 0; unConstrainedNodes < CODECHAL_VP9_UNCONSTRAINED_NODES; unConstrainedNodes++)
                    {
                        ctxBuffer[byteCnt++] = DefaultCoefProbs4x4[blocktype][reftype][coeffbands][prevCoefCtx][unConstrainedNodes];
                    }
                }
            }
        }
    }

    for (blocktype = 0; blocktype < CODECHAL_VP9_BLOCK_TYPES; blocktype++)
    {
        for (reftype = 0; reftype < CODECHAL_VP9_REF_TYPES; reftype++)
        {
            for (coeffbands = 0; coeffbands < CODECHAL_VP9_COEF_BANDS; coeffbands++)
            {
                uint8_t numPrevCoeffCtxts = (coeffbands == 0) ? 3 : CODECHAL_VP9_PREV_COEF_CONTEXTS;
                for (prevCoefCtx = 0; prevCoefCtx < numPrevCoeffCtxts; prevCoefCtx++)
                {
                    for (unConstrainedNodes = 0; unConstrainedNodes < CODECHAL_VP9_UNCONSTRAINED_NODES; unConstrainedNodes++)
                    {
                        ctxBuffer[byteCnt++] = DefaultCoefPprobs8x8[blocktype][reftype][coeffbands][prevCoefCtx][unConstrainedNodes];
                    }
                }
            }
        }
    }

    for (blocktype = 0; blocktype < CODECHAL_VP9_BLOCK_TYPES; blocktype++)
    {
        for (reftype = 0; reftype < CODECHAL_VP9_REF_TYPES; reftype++)
        {
            for (coeffbands = 0; coeffbands < CODECHAL_VP9_COEF_BANDS; coeffbands++)
            {
                uint8_t numPrevCoeffCtxts = (coeffbands == 0) ? 3 : CODECHAL_VP9_PREV_COEF_CONTEXTS;
                for (prevCoefCtx = 0; prevCoefCtx < numPrevCoeffCtxts; prevCoefCtx++)
                {
                    for (unConstrainedNodes = 0; unConstrainedNodes < CODECHAL_VP9_UNCONSTRAINED_NODES; unConstrainedNodes++)
                    {
                        ctxBuffer[byteCnt++] = DefaultCoefProbs16x16[blocktype][reftype][coeffbands][prevCoefCtx][unConstrainedNodes];
                    }
                }
            }
        }
    }

    for (blocktype = 0; blocktype < CODECHAL_VP9_BLOCK_TYPES; blocktype++)
    {
        for (reftype = 0; reftype < CODECHAL_VP9_REF_TYPES; reftype++)
        {
            for (coeffbands = 0; coeffbands < CODECHAL_VP9_COEF_BANDS; coeffbands++)
            {
                uint8_t numPrevCoeffCtxts = (coeffbands == 0) ? 3 : CODECHAL_VP9_PREV_COEF_CONTEXTS;
                for (prevCoefCtx = 0; prevCoefCtx < numPrevCoeffCtxts; prevCoefCtx++)
                {
                    for (unConstrainedNodes = 0; unConstrainedNodes < CODECHAL_VP9_UNCONSTRAINED_NODES; unConstrainedNodes++)
                    {
                        ctxBuffer[byteCnt++] = DefaultCoefProbs32x32[blocktype][reftype][coeffbands][prevCoefCtx][unConstrainedNodes];
                    }
                }
            }
        }
    }

    //16 bytes of zeros 
    byteCnt += 16;

    // mb skip probs            
    for (i = 0; i < CODECHAL_VP9_MBSKIP_CONTEXTS; i++)
    {
        ctxBuffer[byteCnt++] = DefaultMbskipProbs[i];
    }

    // populate prob values which are different between Key and Non-Key frame
    CodecHalVP9_CtxBufDiffInit(ctxBuffer, setToKey);

    //skip Seg tree/pred probs, updating not done in this function.
    byteCnt = CODECHAL_VP9_SEG_PROB_OFFSET;
    byteCnt += 7;
    byteCnt += 3;

    //28 bytes of zeros
    for (i = 0; i < 28; i++)
    {
        ctxBuffer[byteCnt++] = 0;
    }

    //Just a check.
    if (byteCnt > CODECHAL_VP9_PROB_MAX_NUM_ELEM)
    {
        eStatus = MOS_STATUS_NO_SPACE;
        CODECHAL_PUBLIC_ASSERTMESSAGE("Error: FrameContext array out-of-bounds, byteCnt = %d!\n", byteCnt);
        goto finish;
    }

finish:
    return eStatus;
}
