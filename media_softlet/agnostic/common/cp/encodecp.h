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
//! \file     encodecp.h
//! \brief    Defines the common interface for secure encode
//!

#ifndef __ENCODECP_H__
#define __ENCODECP_H__

#include "codec_hw_next.h"
#include "media_status_report.h"

namespace encode{
    class EncodeCp
{
public:
    EncodeCp() {}
    EncodeCp(CodechalHwInterfaceNext *hwInterface){
        m_cpInterface = hwInterface->GetCpInterface();
        m_osInterface = hwInterface->GetOsInterface();
    }
    ~EncodeCp() {}
    MOS_STATUS StartCpStatusReport(MOS_COMMAND_BUFFER *cmdBuffer);
    MOS_STATUS UpdateCpStatusReport(
        void *pStatusReportData);
    MOS_STATUS RegisterParams(void *settings);
    MOS_STATUS UpdateParams(bool input);
    bool       isCpEnabled();
    MOS_STATUS setStatusReport(MediaStatusReport *statusReport);
    MOS_STATUS patchForHM();

protected:
    bool                m_cpEnabled    = false;
    MediaStatusReport * m_statusReport = nullptr;
    MhwCpInterface *    m_cpInterface  = nullptr;
    PMOS_INTERFACE      m_osInterface  = nullptr;

MEDIA_CLASS_DEFINE_END(encode__EncodeCp)
};
}
#endif //__ENCODECP_H__

