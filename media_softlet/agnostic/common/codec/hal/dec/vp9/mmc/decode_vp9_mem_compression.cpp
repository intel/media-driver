/*
* Copyright (c) 2021-2022, Intel Corporation
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
//! \file     decode_vp9_mem_compression.cpp
//! \brief    Defines the common interface for Vp9 decode mmc
//! \details  The mmc is to handle mmc operations,
//! including compression and decompressin of Vp9 decode
//!

#include "mos_defs.h"
#include "decode_utils.h"
#include "decode_vp9_mem_compression.h"

namespace decode
{
Vp9DecodeMemComp::Vp9DecodeMemComp(CodechalHwInterfaceNext *hwInterface)
{
    m_osInterface = hwInterface->GetOsInterface();
}

MOS_STATUS Vp9DecodeMemComp::CheckReferenceList(Vp9BasicFeature &vp9BasicFeature,
    MOS_MEMCOMP_STATE &postDeblockSurfMmcState,
    MOS_MEMCOMP_STATE &preDeblockSurfMmcState,
    PMOS_RESOURCE     *presReferences)
{
    DECODE_FUNC_CALL();
    DECODE_CHK_NULL(m_osInterface);
    MOS_MEMCOMP_STATE mmcMode;

    //Disable MMC if self-reference is detected (mainly for error concealment)
    if ((postDeblockSurfMmcState != MOS_MEMCOMP_DISABLED) ||
        (preDeblockSurfMmcState != MOS_MEMCOMP_DISABLED))
    {
        DECODE_ASSERT(vp9BasicFeature.m_vp9PicParams);
        auto &vp9PicParams = *(vp9BasicFeature.m_vp9PicParams);

        if (vp9PicParams.PicFlags.fields.frame_type != CODEC_VP9_KEY_FRAME &&
            !vp9PicParams.PicFlags.fields.intra_only)
        {
            bool selfReference = false;
            if ((vp9PicParams.CurrPic.FrameIdx == vp9PicParams.RefFrameList[vp9PicParams.PicFlags.fields.LastRefIdx].FrameIdx) ||
                (vp9PicParams.CurrPic.FrameIdx == vp9PicParams.RefFrameList[vp9PicParams.PicFlags.fields.GoldenRefIdx].FrameIdx) ||
                (vp9PicParams.CurrPic.FrameIdx == vp9PicParams.RefFrameList[vp9PicParams.PicFlags.fields.AltRefIdx].FrameIdx))
            {
                selfReference = true;
            }

            if (selfReference)
            {
                postDeblockSurfMmcState = MOS_MEMCOMP_DISABLED;
                preDeblockSurfMmcState  = MOS_MEMCOMP_DISABLED;
                DECODE_NORMALMESSAGE("Self-reference is detected for P/B frames!");

                // Decompress current frame to avoid green corruptions in this error handling case
                MOS_SURFACE &destSurface = vp9BasicFeature.m_destSurface;
                DECODE_CHK_STATUS(m_osInterface->pfnGetMemoryCompressionMode(
                    m_osInterface, &destSurface.OsResource, &mmcMode));
                if (mmcMode != MOS_MEMCOMP_DISABLED)
                {
                    DECODE_CHK_STATUS(m_osInterface->pfnDecompResource(m_osInterface, &destSurface.OsResource));
                }
            }
        }
    }

    // Do surface decompression to make sure the MMC states are consistent in the reference list
    MOS_MEMCOMP_STATE mmcModePrev   = MOS_MEMCOMP_DISABLED;
    bool              sameMmcStatus = true;
    bool              firstRefPic   = true;
    for (uint8_t i = 0; i < CODECHAL_MAX_CUR_NUM_REF_FRAME_VP9; i++)
    {
        if (presReferences[i])
        {
            DECODE_CHK_STATUS(m_osInterface->pfnGetMemoryCompressionMode(
                m_osInterface,
                presReferences[i],
                &mmcMode));
            if (firstRefPic)
            {
                mmcModePrev = mmcMode;
                firstRefPic = false;
            }
            else if (mmcModePrev != mmcMode)
            {
                sameMmcStatus = false;
                break;
            }
        }
    }

    if (!sameMmcStatus)
    {
        for (uint8_t i = 0; i < CODECHAL_MAX_CUR_NUM_REF_FRAME_VP9; i++)
        {
            if (presReferences[i])
            {
                DECODE_CHK_STATUS(m_osInterface->pfnGetMemoryCompressionMode(
                    m_osInterface,
                    presReferences[i],
                    &mmcMode));
                if (mmcMode != MOS_MEMCOMP_DISABLED)
                {
                    m_osInterface->pfnDecompResource(
                        m_osInterface,
                        presReferences[i]);
                }
            }
        }
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9DecodeMemComp::SetRefSurfaceMask(
    Vp9BasicFeature         &vp9BasicFeature,
    MHW_VDBOX_SURFACE_PARAMS refSurfaceParams[])
{
    m_skipMask = 0xf8;
    for (uint8_t i = 0; i < CODECHAL_MAX_CUR_NUM_REF_FRAME_VP9; i++)
    {
        if (refSurfaceParams[i].mmcState == MOS_MEMCOMP_DISABLED)
        {
            m_skipMask |= (1 << i);
        }
    }
    DECODE_NORMALMESSAGE("MMC skip mask is %d,", m_skipMask);
    for (uint8_t i = 0; i < CODECHAL_MAX_CUR_NUM_REF_FRAME_VP9; i++)
    {
        refSurfaceParams[i].mmcState    = MOS_MEMCOMP_MC;
        refSurfaceParams[i].mmcSkipMask = m_skipMask;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9DecodeMemComp::SetRefSurfaceMask(
    Vp9BasicFeature                       &vp9BasicFeature,
    mhw::vdbox::hcp::HCP_SURFACE_STATE_PAR refSurfaceParams[])
{
    m_skipMask = 0xf8;
    for (uint8_t i = 0; i < CODECHAL_MAX_CUR_NUM_REF_FRAME_VP9; i++)
    {
        if (refSurfaceParams[i].mmcState == MOS_MEMCOMP_DISABLED)
        {
            m_skipMask |= (1 << i);
        }
    }
    DECODE_NORMALMESSAGE("MMC skip mask is %d,", m_skipMask);
    for (uint8_t i = 0; i < CODECHAL_MAX_CUR_NUM_REF_FRAME_VP9; i++)
    {
        refSurfaceParams[i].mmcState    = MOS_MEMCOMP_MC;
        refSurfaceParams[i].mmcSkipMask = m_skipMask;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9DecodeMemComp::SetRefSurfaceCompressionFormat(
    Vp9BasicFeature         &vp9BasicFeature,
    MHW_VDBOX_SURFACE_PARAMS refSurfaceParams[])
{
    uint32_t compressionFormat = 0;
    for (uint8_t i = 0; i < CODECHAL_MAX_CUR_NUM_REF_FRAME_VP9; i++)
    {
        if (refSurfaceParams[i].mmcState == MOS_MEMCOMP_MC || refSurfaceParams[i].mmcState == MOS_MEMCOMP_RC)
        {
            compressionFormat = refSurfaceParams[i].dwCompressionFormat;
        }
    }

    for (uint8_t i = 0; i < CODECHAL_MAX_CUR_NUM_REF_FRAME_VP9; i++)
    {
        refSurfaceParams[i].dwCompressionFormat = compressionFormat;
    }

    DECODE_NORMALMESSAGE("Reference surfaces compression format is %d,", compressionFormat);

    return MOS_STATUS_SUCCESS;
}

}  // namespace decode
