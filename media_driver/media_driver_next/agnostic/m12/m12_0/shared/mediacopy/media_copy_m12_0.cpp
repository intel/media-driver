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
//! \file     media_copy_m12_0.cpp
//! \brief    Common interface and structure used in media copy
//! \details  Common interface and structure used in media copy which are platform independent
//!

#include "media_copy_m12_0.h"

MediaCopyStateM12_0::MediaCopyStateM12_0() :
    MediaCopyBaseState()
{

}

MOS_STATUS MediaCopyStateM12_0::Initialize(  PMOS_INTERFACE  osInterface, MhwInterfaces *mhwInterfaces)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MCPY_CHK_NULL_RETURN(osInterface);
    MCPY_CHK_NULL_RETURN(mhwInterfaces);

    m_osInterface   = osInterface;
    m_mhwInterfaces = mhwInterfaces;

    // blt init
    m_bltState = MOS_New(BltState, m_osInterface, m_mhwInterfaces);
    MCPY_CHK_NULL_RETURN(m_bltState);

    // vebox init

    return eStatus;
}

MediaCopyStateM12_0::~MediaCopyStateM12_0()
{
    MOS_Delete(m_bltState);
}

bool MediaCopyStateM12_0::VeboxFormatSupportCheck(PMOS_RESOURCE src, PMOS_RESOURCE dst)
{
    bool eStatus = false;
    //eStatus = checkinpuforamt(src);
    //eStatus = checkoutputformat(dst);

    return eStatus;
}

bool MediaCopyStateM12_0::RenderFormatSupportCheck(PMOS_RESOURCE src, PMOS_RESOURCE dst)
{
    bool eStatus = false;
    //eStatus = checkinpuforamt(src);
    //eStatus = checkoutputformat(dst);

    return eStatus;
}

MOS_STATUS MediaCopyStateM12_0::FeatureSupport(PMOS_RESOURCE src, PMOS_RESOURCE dst,
            MCPY_STATE_PARAMS& mcpy_src, MCPY_STATE_PARAMS& mcpy_dst, MCPY_ENGINE_CAPS& caps)
{
    // TGL has full hw enigne.
    // check CP COPYIN case. ??
    if (mcpy_src.CompressionMode == MOS_MMC_DISABLED &&
        mcpy_dst.CompressionMode == MOS_MMC_RC       &&
        mcpy_dst.CpMode          == MCPY_CPMODE_CP)
    {
        caps.engineBlt = false;
    }

    caps.engineRender = false; // currently, TGL render fast copy still uses MDF kernel.

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MediaCopyStateM12_0::MediaVeboxCopy(PMOS_RESOURCE src, PMOS_RESOURCE dst)
{
    // implementation
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MediaCopyStateM12_0::MediaBltCopy(PMOS_RESOURCE src, PMOS_RESOURCE dst)
{
    // implementation
    MCPY_CHK_STATUS_RETURN(m_bltState->CopyMainSurface(src, dst));
    return MOS_STATUS_SUCCESS;

}

MOS_STATUS MediaCopyStateM12_0::MediaRenderCopy(PMOS_RESOURCE src, PMOS_RESOURCE dst)
{
    // implementation
    // currently, still using mdf kernel.
    return MOS_STATUS_UNIMPLEMENTED;
}

