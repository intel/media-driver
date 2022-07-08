/*
* Copyright (c) 2022, Intel Corporation
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
//! \file     meida_sfc_interface_legacy.cpp
//! \brief    Common interface for sfc
//! \details  Common interface for sfc
//!
#include "media_sfc_interface_legacy.h"
#include "media_sfc_render_legacy.h"
#include "vp_utils.h"

MediaSfcInterfaceLegacy::MediaSfcInterfaceLegacy(PMOS_INTERFACE osInterface, MediaMemComp *mmc) 
    : MediaSfcInterface(osInterface, mmc)
{
}

//!
//! \brief    MediaSfcInterface initialize
//! \details  Initialize the BltState, create BLT context.
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS MediaSfcInterfaceLegacy::Initialize(MEDIA_SFC_INTERFACE_MODE mode)
{
    VP_PUBLIC_CHK_NULL_RETURN(m_osInterface);
    if (m_sfcRender)
    {
        Destroy();
    }

    m_sfcRender = MOS_New(MediaSfcRenderLegacy, m_osInterface, mode, m_mmc);
    VP_PUBLIC_CHK_NULL_RETURN(m_sfcRender);
    VP_PUBLIC_CHK_STATUS_RETURN(m_sfcRender->Initialize());

    return MOS_STATUS_SUCCESS;
}