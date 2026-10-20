/*
* Copyright (c) 2026, Intel Corporation
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
//! \file     encodecp_interface.h
//! \brief    Defines the public interface for secure encode
//!

#ifndef __ENCODE_CP_INTERFACE_H__
#define __ENCODE_CP_INTERFACE_H__

#include "codec_hw_next.h"
#include "media_status_report.h"

namespace encode{
    class EncodeCpInterface
{
public:
    virtual ~EncodeCpInterface() {}
    virtual MOS_STATUS StartCpStatusReport(MOS_COMMAND_BUFFER *cmdBuffer)       = 0;
    virtual MOS_STATUS UpdateCpStatusReport(void *pStatusReportData)           = 0;
    virtual MOS_STATUS RegisterParams(void *settings)                         = 0;
    virtual MOS_STATUS UpdateParams(bool input)                               = 0;
    virtual bool       isCpEnabled()                                         = 0;
    virtual MOS_STATUS setStatusReport(MediaStatusReport *statusReport)       = 0;
    virtual MOS_STATUS patchForHM()                                          = 0;

MEDIA_CLASS_DEFINE_END(encode__EncodeCpInterface)
};

//!
//! \brief    Create EncodeCpInterface Object
//!           Must use Delete_EncodeCpInterface to delete created Object to avoid ULT Memory Leak errors
//!
//! \return   Return CP Wrapper Object if CPLIB not loaded
//!
EncodeCpInterface *Create_EncodeCpInterface(CodechalHwInterfaceNext *hwInterface);

//!
//! \brief    Delete the EncodeCpInterface Object
//!
//! \param    [in] pEncodeCpInterface
//!           EncodeCpInterface
//!
void Delete_EncodeCpInterface(EncodeCpInterface *pEncodeCpInterface);
}
#endif //__ENCODE_CP_INTERFACE_H__
