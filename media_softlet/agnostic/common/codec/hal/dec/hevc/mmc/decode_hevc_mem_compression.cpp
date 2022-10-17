/*
* Copyright (c) 2020-2022, Intel Corporation
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
//! \file     decode_hevc_mem_compression.cpp
//! \brief    Defines the common interface for Hevc decode mmc
//! \details  The mmc is to handle mmc operations,
//! including compression and decompressin of Hevc decode
//!

#include "mos_defs.h"
#include "decode_hevc_mem_compression.h"

namespace decode
{

HevcDecodeMemComp::HevcDecodeMemComp(CodechalHwInterfaceNext *hwInterface)
{
    if (hwInterface)
    {
        m_osInterface = hwInterface->GetOsInterface();
    }
}

MOS_STATUS HevcDecodeMemComp::SetRefSurfaceMask(
    HevcBasicFeature    &hevcBasicFeature,
    const PMOS_RESOURCE *presReferences,
    uint8_t             &mmcSkipMask)
{
    if (hevcBasicFeature.m_isSCCIBCMode)
    {
        HevcReferenceFrames &refFrames = hevcBasicFeature.m_refFrames;
        DECODE_ASSERT(hevcBasicFeature.m_hevcPicParams != nullptr);
        const std::vector<uint8_t> &activeRefList = refFrames.GetActiveReferenceList(*hevcBasicFeature.m_hevcPicParams);

        uint8_t IBCRefIdx = refFrames.m_IBCRefIdx;
        DECODE_CHK_COND(activeRefList.size() <= IBCRefIdx, "Invalid IBC reference index.");
        uint8_t IBCFrameIdx = activeRefList[IBCRefIdx];

        uint8_t skipMask = 0;
        for (uint8_t i = 0; i < CODECHAL_MAX_CUR_NUM_REF_FRAME_HEVC; i++)
        {
            if (presReferences[i] == refFrames.GetReferenceByFrameIndex(IBCFrameIdx))
            {
                skipMask |= (1 << i);
            }
        }
        mmcSkipMask = skipMask;
        DECODE_NORMALMESSAGE("IBC ref index %d, MMC skip mask %d,", IBCRefIdx, skipMask);
    }

    MOS_MEMCOMP_STATE refMmcState = MOS_MEMCOMP_DISABLED;
    uint8_t           skipMask    = 0;
    for (uint16_t i = 0; i < CODECHAL_MAX_CUR_NUM_REF_FRAME_HEVC; i++)
    {
        if (presReferences[i] != nullptr)
        {
            DECODE_CHK_STATUS(m_osInterface->pfnGetMemoryCompressionMode(
                m_osInterface, presReferences[i], &refMmcState));
        }

        if (refMmcState == MOS_MEMCOMP_DISABLED)
        {
            skipMask |= (1 << i);
        }
    }
    mmcSkipMask |= skipMask;
    DECODE_NORMALMESSAGE("HEVC MMC skip mask %d\n", mmcSkipMask);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcDecodeMemComp::SetRefSurfaceCompressionFormat(
    HevcBasicFeature    &hevcBasicFeature,
    const PMOS_RESOURCE *presReferences,
    uint32_t            &mmcCompressionFormat)
{
    MOS_MEMCOMP_STATE refMmcState       = MOS_MEMCOMP_DISABLED;
    uint32_t          compressionFormat = 0;
    uint32_t          refcompressionFormat = 0;

    for (uint16_t i = 0; i < CODECHAL_MAX_CUR_NUM_REF_FRAME_HEVC; i++)
    {
        if (presReferences[i] != nullptr)
        {
            DECODE_CHK_STATUS(m_osInterface->pfnGetMemoryCompressionMode(
                m_osInterface, presReferences[i], &refMmcState));

            if (refMmcState == MOS_MEMCOMP_MC || refMmcState == MOS_MEMCOMP_RC)
            {
                DECODE_CHK_STATUS(m_osInterface->pfnGetMemoryCompressionFormat(
                    m_osInterface, presReferences[i], &compressionFormat));

                refcompressionFormat = compressionFormat;
            }
        }
    }
    mmcCompressionFormat = refcompressionFormat;
    DECODE_NORMALMESSAGE("HEVC reference surface compression format %d\n", mmcCompressionFormat);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcDecodeMemComp::CheckReferenceList(HevcBasicFeature &hevcBasicFeature,
        MOS_MEMCOMP_STATE &postDeblockSurfMmcState,
        MOS_MEMCOMP_STATE &preDeblockSurfMmcState,
        PMOS_RESOURCE     *presReferences)
{
    DECODE_FUNC_CALL();
    DECODE_CHK_NULL(m_osInterface);

    // Disable MMC if self-reference is dectected (mainly for error concealment)
    if (!hevcBasicFeature.m_refFrames.m_curIsIntra)
    {
        if (postDeblockSurfMmcState != MOS_MEMCOMP_DISABLED ||
            preDeblockSurfMmcState  != MOS_MEMCOMP_DISABLED)
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
                    MOS_MEMCOMP_STATE mmcMode = MOS_MEMCOMP_DISABLED;
                    MOS_SURFACE &destSurface  = hevcBasicFeature.m_destSurface;
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

    // Do surface decompression to make sure the MMC states are consistent in the reference list
    bool sameMmcStatus = true;
    MOS_MEMCOMP_STATE mmcModePrev  = MOS_MEMCOMP_DISABLED;
    for (uint8_t i = 0; i < CODECHAL_MAX_CUR_NUM_REF_FRAME_HEVC; i++)
    {
        if (presReferences[i] != nullptr)
        {
            MOS_MEMCOMP_STATE mmcMode = MOS_MEMCOMP_DISABLED;
            DECODE_CHK_STATUS(m_osInterface->pfnGetMemoryCompressionMode(
                m_osInterface, presReferences[i], &mmcMode));

            if (i == 0)
            {
                mmcModePrev = mmcMode;
            }
            else if (mmcModePrev != mmcMode)
            {
                sameMmcStatus = false;
                break;
            }
        }
    }

    if(!sameMmcStatus)
    {
        for (uint8_t i = 0; i < CODECHAL_MAX_CUR_NUM_REF_FRAME_HEVC; i++)
        {
            if (presReferences[i] != nullptr)
            {
                MOS_MEMCOMP_STATE mmcMode = MOS_MEMCOMP_DISABLED;
                DECODE_CHK_STATUS(m_osInterface->pfnGetMemoryCompressionMode(
                    m_osInterface, presReferences[i], &mmcMode));
                if(mmcMode != MOS_MEMCOMP_DISABLED)
                {
                    m_osInterface->pfnDecompResource(m_osInterface, presReferences[i]);
                }
            }
        }
    }

    return MOS_STATUS_SUCCESS;
}

}
