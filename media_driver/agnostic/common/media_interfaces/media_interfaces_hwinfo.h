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
//! \file     media_interfaces_hwinfo.h
//! \brief    
//!

#ifndef __MEDIA_INTERFACES_HWINFO_H__
#define __MEDIA_INTERFACES_HWINFO_H__

struct HwDeviceInfo
{
    uint32_t ipVersion;      // ipVersion -Intellectual Property
    uint32_t usRevId;        // usRevId   -User Reversion ID
};

//!
//! \class    HwInfo
//! \brief    Save hw information
//!

class MediaInterfacesHwInfo
{
public:
    MediaInterfacesHwInfo() {};
    virtual ~MediaInterfacesHwInfo() {}

    HwDeviceInfo& GetDeviceInfo()
    {
        return m_deviceInfo;
    };

    void SetDeviceInfo(uint32_t ipVersion, uint32_t usRevId)
    {
        m_deviceInfo.ipVersion = ipVersion;
        m_deviceInfo.usRevId   = usRevId;
    };

    void SetDeviceInfo(HwDeviceInfo deviceInfo)
    {
        m_deviceInfo.ipVersion = deviceInfo.ipVersion;
        m_deviceInfo.usRevId   = deviceInfo.usRevId;
    };

protected:
    HwDeviceInfo m_deviceInfo = {};
};

#endif // __MEDIA_INTERFACES_HWINFO_H__