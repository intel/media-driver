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
//! \file     decode_mpeg2_mem_compression.cpp
//! \brief    Defines the common interface for Mpeg2 decode mmc
//! \details  The mmc is to handle mmc operations,
//! including compression and decompressin of Mpeg2 decode
//!

#include "mos_defs.h"
#include "decode_mpeg2_mem_compression.h"
#include "decode_utils.h"

namespace decode
{

Mpeg2DecodeMemComp::Mpeg2DecodeMemComp(CodechalHwInterfaceNext *hwInterface)
{
    m_osInterface = hwInterface->GetOsInterface();
}

MOS_STATUS Mpeg2DecodeMemComp::CheckReferenceList(
    Mpeg2BasicFeature &mpeg2BasicFeature, MOS_MEMCOMP_STATE &preDeblockSurfMmcState, MOS_MEMCOMP_STATE &postDeblockSurfMmcState)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;
    DECODE_FUNC_CALL();
    DECODE_CHK_NULL(m_osInterface);

    if (((postDeblockSurfMmcState != MOS_MEMCOMP_DISABLED) ||
        (preDeblockSurfMmcState != MOS_MEMCOMP_DISABLED)) &&
        (mpeg2BasicFeature.m_mpeg2PicParams->m_pictureCodingType != I_TYPE))
    {
        bool selfReference = false;
        if ((mpeg2BasicFeature.m_mpeg2PicParams->m_currPic.FrameIdx == mpeg2BasicFeature.m_mpeg2PicParams->m_forwardRefIdx) || 
            (mpeg2BasicFeature.m_mpeg2PicParams->m_currPic.FrameIdx == mpeg2BasicFeature.m_mpeg2PicParams->m_backwardRefIdx))
        {
            selfReference = true;        
        }

        if (selfReference)
        {
            postDeblockSurfMmcState = MOS_MEMCOMP_DISABLED;
            preDeblockSurfMmcState  = MOS_MEMCOMP_DISABLED;
            DECODE_ASSERTMESSAGE("Self-reference is detected for P/B frames!");

            //Decompress current frame to avoid green corruption in this error handling case
            MOS_MEMCOMP_STATE mmcMode;
            DECODE_CHK_STATUS(m_osInterface->pfnGetMemoryCompressionMode(m_osInterface, 
                &mpeg2BasicFeature.m_destSurface.OsResource,
                &mmcMode));

            if (mmcMode != MOS_MEMCOMP_DISABLED)
            {
                DECODE_CHK_STATUS(m_osInterface->pfnDecompResource(
                    m_osInterface,
                    &mpeg2BasicFeature.m_destSurface.OsResource));
            }
        }
    }
    return eStatus;
}

}
