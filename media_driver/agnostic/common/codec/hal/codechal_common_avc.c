/*
* Copyright (c) 2015-2017, Intel Corporation
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
//! \file      codechal_common_avc.c
//! \brief     This modules includes the common definitions for AVC codec, across CODECHAL components.
//!

#include "codechal_common_avc.h"
#include "codec_def_common_avc.h"

void CodecHalAvc_SetFrameStoreIds(
    PCODECHAL_AVC_FRAME_STORE_ID    avcFrameStoreID,
    PCODEC_REF_LIST                *avcRefList,
    uint32_t                        mode,
    uint8_t                         frameIdx)
{
    uint8_t                         index;
    uint8_t                         invalidFrame;
    uint8_t                         i;
    uint8_t                         j;

    CODECHAL_PUBLIC_CHK_NULL_NO_STATUS(avcFrameStoreID);
    CODECHAL_PUBLIC_CHK_NULL_NO_STATUS(avcRefList);

    invalidFrame = (mode == CODECHAL_DECODE_MODE_AVCVLD) ? 0x7f : 0x1f;

    for (i = 0; i < avcRefList[frameIdx]->ucNumRef; i++)
    {
        index = avcRefList[frameIdx]->RefList[i].FrameIdx;
        if (avcRefList[index]->ucFrameId == invalidFrame)
        {
            for (j = 0; j < CODEC_AVC_MAX_NUM_REF_FRAME; j++)
            {
                if (!avcFrameStoreID[j].bInUse)
                {
                    avcRefList[index]->ucFrameId = j;
                    avcFrameStoreID[j].bInUse = true;
                    break;
                }
            }
            if (j == CODEC_AVC_MAX_NUM_REF_FRAME)
            {
                // should never happen, something must be wrong
                CODECHAL_PUBLIC_ASSERT(false);
                avcRefList[index]->ucFrameId = 0;
                avcFrameStoreID[0].bInUse = true;
            }
        }
    }
finish:
    return;
}
