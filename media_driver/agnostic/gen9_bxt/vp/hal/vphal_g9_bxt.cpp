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
//! \file     vphal_g9_bxt.cpp
//! \brief    Vphal Interface Definition
//! \details  Vphal Interface Definition Including:
//!           const and function
//!
#include "vphal_g9_bxt.h"
#include "vphal_renderer_g9_bxt.h"

//!
//! \brief    Create instance of VphalRenderer
//! \details  Create instance of VphalRenderer
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VphalStateG9Bxt::CreateRenderer()
{
    MOS_STATUS eStatus = MOS_STATUS_UNKNOWN;

    // Setup rendering interface functions
    m_renderer = MOS_New(
        VphalRendererG9Bxt,
        m_renderHal,
        &eStatus);

    if (m_renderer == nullptr)
    {
        return MOS_STATUS_NULL_POINTER;
    }
    else
    {
        if (eStatus != MOS_STATUS_SUCCESS)
        {
            MOS_Delete(m_renderer);
            m_renderer = nullptr;
            return eStatus;
        }
        else
        {
            m_renderer->SetStatusReportTable(&m_statusTable);
        }
    }

    eStatus = m_renderer->InitKdllParam();
    if (eStatus != MOS_STATUS_SUCCESS)
    {
        MOS_Delete(m_renderer);
        m_renderer = nullptr;
        return eStatus;
    }

    eStatus = m_renderer->AllocateRenderComponents(
                                m_veboxInterface,
                                m_sfcInterface);

    return eStatus;
}
