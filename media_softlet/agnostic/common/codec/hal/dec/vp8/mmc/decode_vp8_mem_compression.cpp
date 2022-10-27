/*
* Copyright (c) 2021, Intel Corporation
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
//! \file     decode_vp8_mem_compression.cpp
//! \brief    Defines the common interface for Vp8 decode mmc
//! \details  The mmc is to handle mmc operations,
//! including compression and decompressin of Vp8 decode
//!

#include "mos_defs.h"
#include "decode_utils.h"
#include "decode_vp8_mem_compression.h"

#ifdef _MMC_SUPPORTED

namespace decode
{
Vp8DecodeMemComp::Vp8DecodeMemComp(CodechalHwInterfaceNext *hwInterface)
{
    m_osInterface = hwInterface->GetOsInterface();
}

MOS_STATUS Vp8DecodeMemComp::CheckReferenceList(Vp8BasicFeature &vp8BasicFeature, 
                                                MOS_MEMCOMP_STATE &PostDeblockSurfMmcState,
                                                MOS_MEMCOMP_STATE &PreDeblockSurfMmcState)
{
    DECODE_FUNC_CALL();
    DECODE_CHK_NULL(m_osInterface);

    MOS_MEMCOMP_STATE mmcMode;

    DECODE_ASSERT(vp8BasicFeature.m_vp8PicParams);
    auto &vp8PicParams = *(vp8BasicFeature.m_vp8PicParams);

    //Disable MMC if self-reference is detected (mainly for error concealment)
    if ((PostDeblockSurfMmcState != MOS_MEMCOMP_DISABLED) ||
        (PreDeblockSurfMmcState != MOS_MEMCOMP_DISABLED) &&
        (vp8PicParams.key_frame != I_TYPE))
    {
        bool selfReference = false;
        if ((vp8PicParams.ucCurrPicIndex == vp8PicParams.ucLastRefPicIndex) ||
            (vp8PicParams.ucCurrPicIndex == vp8PicParams.ucGoldenRefPicIndex) ||
            (vp8PicParams.ucCurrPicIndex ==vp8PicParams.ucAltRefPicIndex))
        {
            selfReference = true;
        }

        if (selfReference)
        {
            PostDeblockSurfMmcState = MOS_MEMCOMP_DISABLED;
            PreDeblockSurfMmcState  = MOS_MEMCOMP_DISABLED;
            DECODE_NORMALMESSAGE("Self-reference is detected for P/B frames!");

            // Decompress current frame to avoid green corruptions in this error handling case
            MOS_SURFACE &destSurface = vp8BasicFeature.m_destSurface;
            DECODE_CHK_STATUS(m_osInterface->pfnGetMemoryCompressionMode(
                                m_osInterface, 
                                &destSurface.OsResource, 
                                &mmcMode));
            if (mmcMode != MOS_MEMCOMP_DISABLED)
            {
                DECODE_CHK_STATUS(m_osInterface->pfnDecompResource(
                                m_osInterface, 
                                &destSurface.OsResource));
            }
        }
        
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp8DecodeMemComp::SetPipeBufAddr(Vp8BasicFeature &vp8BasicFeature,
                                            MOS_MEMCOMP_STATE &PostDeblockSurfMmcState,
                                            MOS_MEMCOMP_STATE &PreDeblockSurfMmcState)
{
    DECODE_FUNC_CALL();
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    if (m_mmcEnabled)
    {
        DECODE_CHK_STATUS(m_osInterface->pfnGetMemoryCompressionMode(m_osInterface,
            &vp8BasicFeature.m_destSurface.OsResource,
            &PreDeblockSurfMmcState));
    }
    else
    {
        PreDeblockSurfMmcState = MOS_MEMCOMP_DISABLED;
    }
    PostDeblockSurfMmcState = PreDeblockSurfMmcState;

    vp8BasicFeature.m_destSurface.MmcState = vp8BasicFeature.m_deblockingEnabled ? PostDeblockSurfMmcState : PreDeblockSurfMmcState;

    return eStatus;
}

}  // namespace decode
#endif
