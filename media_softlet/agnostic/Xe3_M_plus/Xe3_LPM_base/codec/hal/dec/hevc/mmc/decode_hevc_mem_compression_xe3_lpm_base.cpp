/*
* Copyright (c) 2022-2025, Intel Corporation
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
//! \file     decode_hevc_mem_compression_xe3_lpm_base.cpp
//! \brief    Defines the common interface for Hevc decode mmc for Xe3_LPM+
//! \details  The mmc is to handle mmc operations,
//! including compression and decompressin of Hevc decode
//!

#include "decode_hevc_mem_compression_xe3_lpm_base.h"

namespace decode
{

HevcDecodeMemCompXe3_Lpm_Base::HevcDecodeMemCompXe3_Lpm_Base(CodechalHwInterfaceNext *hwInterface)
    : DecodeMemCompXe3_Lpm_Base(hwInterface), HevcDecodeMemComp(hwInterface)
{
    if (hwInterface)
    {
        m_osInterface = hwInterface->GetOsInterface();
    }
}

MOS_STATUS HevcDecodeMemCompXe3_Lpm_Base::CheckReferenceList(HevcBasicFeature &hevcBasicFeature,
    MOS_MEMCOMP_STATE                                                         &postDeblockSurfMmcState,
    MOS_MEMCOMP_STATE                                                         &preDeblockSurfMmcState,
    PMOS_RESOURCE                                                             *presReferences)
{
    DECODE_CHK_NULL(m_osInterface);
    // Disable MMC if self-reference is dectected (mainly for error concealment)
    if (!hevcBasicFeature.m_refFrames.m_curIsIntra)
    {
        if (postDeblockSurfMmcState != MOS_MEMCOMP_DISABLED ||
            preDeblockSurfMmcState != MOS_MEMCOMP_DISABLED)
        {
            DECODE_ASSERT(hevcBasicFeature.m_hevcPicParams);
            CODEC_HEVC_PIC_PARAMS &hevcPicParams = *(hevcBasicFeature.m_hevcPicParams);

            for (uint8_t i = 0; i < CODEC_MAX_NUM_REF_FRAME_HEVC; i++)
            {
                if (hevcPicParams.CurrPic.FrameIdx == hevcPicParams.RefFrameList[i].FrameIdx)
                {
                    DECODE_NORMALMESSAGE("Self-reference is detected for P/B frames!");
                    postDeblockSurfMmcState = MOS_MEMCOMP_DISABLED;
                    preDeblockSurfMmcState  = MOS_MEMCOMP_DISABLED;

                    // Decompress current frame to avoid green corruptions in this error handling case
                    MOS_MEMCOMP_STATE mmcMode     = MOS_MEMCOMP_DISABLED;
                    MOS_SURFACE      &destSurface = hevcBasicFeature.m_destSurface;
                    DECODE_CHK_STATUS(m_osInterface->pfnGetMemoryCompressionMode(
                        m_osInterface, &destSurface.OsResource, &mmcMode));
                    if (mmcMode != MOS_MEMCOMP_DISABLED)
                    {
                        DECODE_CHK_STATUS(m_osInterface->pfnDecompResource(m_osInterface, &destSurface.OsResource));
                    }

                    break;
                }
            }
        }
    }
    return MOS_STATUS_SUCCESS;
}

}
