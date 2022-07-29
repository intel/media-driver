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
//! \file     mos_os_mock_adaptor.cpp
//! \brief    Common interface and structure used in mock adaptor.
//!

#include "mos_os.h"
#include "mos_os_specific.h"
#include "mos_os_mock_adaptor.h"
#include "mos_os_mock_adaptor_specific.h"

#define TGL_A0_REV_ID 0x00
#define TGL_B0_REV_ID 0x01
#define TGL_C0_REV_ID 0x02

MOS_STATUS  MosMockAdaptor::InitializePlatForm()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MOS_OS_CHK_NULL_RETURN(m_pPlatform);

    switch (m_productFamily)
    {
    case IGFX_TIGERLAKE_LP:
        m_pPlatform->eProductFamily     = IGFX_TIGERLAKE_LP;
        m_pPlatform->eDisplayCoreFamily = IGFX_GEN12_CORE;
        m_pPlatform->eRenderCoreFamily  = IGFX_GEN12_CORE;

        if (m_stepping[0] == 'a' || m_stepping[0] == 'A')
        {
            m_pPlatform->usRevId = TGL_A0_REV_ID;
        }
        else if (m_stepping[0] == 'b' || m_stepping[0] == 'B')
        {
            m_pPlatform->usRevId = TGL_B0_REV_ID;
        }
        else if (m_stepping[0] == 'c' || m_stepping[0] == 'C')
        {
            m_pPlatform->usRevId = TGL_C0_REV_ID;
        }
        else
        {
            MOS_OS_ASSERTMESSAGE("Invalid stepping.");
        }

        break;
    default:  //default setting
        m_pPlatform->eProductFamily     = IGFX_UNKNOWN;
        m_pPlatform->eDisplayCoreFamily = IGFX_UNKNOWN_CORE;
        m_pPlatform->eRenderCoreFamily  = IGFX_UNKNOWN_CORE;
        eStatus                      = MOS_STATUS_UNKNOWN;
        return eStatus;
    }

    m_pPlatform->usDeviceID = m_deviceId;

    if (m_stepping[1] == '0')
    {
        m_pPlatform->usRevId += 0;
    }
    else if (m_stepping[1] == '1')
    {
        m_pPlatform->usRevId += 1;
    }
    else if (m_stepping[1] == '2')
    {
        m_pPlatform->usRevId += 2;
    }
    else if (m_stepping[1] == '3')
    {
        m_pPlatform->usRevId += 3;
    }
    else
    {
        MOS_OS_ASSERTMESSAGE("Invalid stepping.");
    }

    return eStatus;
}