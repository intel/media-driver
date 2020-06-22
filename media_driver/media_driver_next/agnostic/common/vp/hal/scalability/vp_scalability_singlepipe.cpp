/*
* Copyright (c) 2019, Intel Corporation
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
//! \file     vp_scalability_singlepipe.cpp
//! \brief    Defines the common interface for media vpp scalability singlepipe mode.
//! \details  The media scalability singlepipe interface is further sub-divided by component,
//!           this file is for the base interface which is shared by all components.
//!
#include "vp_scalability_singlepipe.h"

namespace vp 
{
VpScalabilitySinglePipe::VpScalabilitySinglePipe(void * hwInterface, MediaContext  * mediaContext, uint8_t componentType) :
    MediaScalabilitySinglePipe(hwInterface, mediaContext, componentType)
{
    if (hwInterface == nullptr)
    {
        return;
    }

    m_hwInterface = (PVP_MHWINTERFACE)hwInterface;
    m_osInterface = m_hwInterface->m_osInterface;
    m_miInterface = m_hwInterface->m_mhwMiInterface;
}

VpScalabilitySinglePipe::~VpScalabilitySinglePipe()
{
    if (m_scalabilityOption)
    {
        MOS_Delete(m_scalabilityOption);
        m_scalabilityOption = nullptr;
    }
}

MOS_STATUS VpScalabilitySinglePipe::Initialize(const MediaScalabilityOption &option)
{
    SCALABILITY_CHK_NULL_RETURN(m_osInterface);

    m_scalabilityOption = MOS_New(VpScalabilityOption, (const VpScalabilityOption &)option);
    SCALABILITY_CHK_NULL_RETURN(m_scalabilityOption);

    return MediaScalabilitySinglePipe::Initialize(option);
}
}
