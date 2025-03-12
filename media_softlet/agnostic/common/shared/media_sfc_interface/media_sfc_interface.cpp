/*
* Copyright (c) 2020-2021, Intel Corporation
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
//! \file     meida_sfc_interface.cpp
//! \brief    Common interface for sfc
//! \details  Common interface for sfc
//!
#include "media_sfc_interface.h"
#include "media_sfc_render.h"
#include "mos_utilities.h"
#include "vp_utils.h"

MediaSfcInterface::MediaSfcInterface(PMOS_INTERFACE osInterface, MediaMemComp *mmc) : m_osInterface(osInterface), m_mmc(mmc)
{
}

MediaSfcInterface::~MediaSfcInterface()
{
    Destroy();
}

void MediaSfcInterface::Destroy()
{
    MOS_Delete(m_sfcRender);
}

MOS_STATUS MediaSfcInterface::IsParameterSupported(
    VDBOX_SFC_PARAMS                    &sfcParam)
{
    VP_PUBLIC_CHK_NULL_RETURN(m_sfcRender);
    return m_sfcRender->IsParameterSupported(sfcParam);
}

MOS_STATUS MediaSfcInterface::IsParameterSupported(
    VEBOX_SFC_PARAMS                    &sfcParam)
{
    VP_PUBLIC_CHK_NULL_RETURN(m_sfcRender);
    return m_sfcRender->IsParameterSupported(sfcParam);
}

MOS_STATUS MediaSfcInterface::Render(VEBOX_SFC_PARAMS &param)
{
    VP_PUBLIC_CHK_NULL_RETURN(m_sfcRender);
    return m_sfcRender->Render(param);
}

MOS_STATUS MediaSfcInterface::Render(MOS_COMMAND_BUFFER *cmdBuffer, VDBOX_SFC_PARAMS &param)
{
    VP_PUBLIC_CHK_NULL_RETURN(cmdBuffer);
    VP_PUBLIC_CHK_NULL_RETURN(m_sfcRender);
    return m_sfcRender->Render(cmdBuffer, param);
}

//!
//! \brief    MediaSfcInterface initialize
//! \details  Initialize the BltState, create BLT context.
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS MediaSfcInterface::Initialize(MEDIA_SFC_INTERFACE_MODE mode)
{
    VP_PUBLIC_CHK_NULL_RETURN(m_osInterface);
    if (m_sfcRender)
    {
        Destroy();
    }

    m_sfcRender = MOS_New(MediaSfcRender, m_osInterface, mode, m_mmc);
    VP_PUBLIC_CHK_NULL_RETURN(m_sfcRender);
    VP_PUBLIC_CHK_STATUS_RETURN(m_sfcRender->Initialize());

    return MOS_STATUS_SUCCESS;
}

bool MediaSfcInterface::IsRenderInitialized()
{
    if (m_sfcRender)
    {
        return m_sfcRender->IsInitialized();
    }
    return false;
}

uint32_t MediaSfcInterface::GetSfcCommandSize()
{
    return m_sfcRender->GetSfcCommandSize();
}