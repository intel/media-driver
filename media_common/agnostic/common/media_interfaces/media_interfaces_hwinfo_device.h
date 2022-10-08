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
//! \file     media_interfaces_hwinfo_device.h
//! \brief    Gen-specific factory creation of the hwinfo device interfaces
//!

#ifndef __MEDIA_INTERFACES_HWINFO_DEVICE_H__
#define __MEDIA_INTERFACES_HWINFO_DEVICE_H__

#include "media_factory.h"
#include "media_interfaces_hwinfo.h"
#include "igfxfmid.h"
#include "media_skuwa_specific.h"

#define IP_VERSION_UNKNOW 0

class MediaInterfacesHwInfo;

//!
//! \class    HwInfo
//! \brief    Save hw information
//!

class MediaInterfacesHwInfoDevice
{
public:
    virtual ~MediaInterfacesHwInfoDevice() { }

    MediaInterfacesHwInfo m_hwInfo;
    //!
    //! \brief    Create hw info
    //! \param    [in] productFamily
    //!           Product family.
    //!
    //! \return   Pointer to Gen specific hw info if
    //!           successful, otherwise return nullptr
    //!
    static MediaInterfacesHwInfo *CreateFactory(PLATFORM platform, MEDIA_WA_TABLE *waTable = nullptr);

    //!
    //! \brief    Refresh RevId
    //! \return   MOS_STATUS_SUCCESS if succeeded, else error code.
    //!
    virtual MOS_STATUS RefreshRevId(PLATFORM &platform, MEDIA_WA_TABLE *waTable)
    {
        return MOS_STATUS_SUCCESS;
    };

    //!
    //! \brief    Initializes platform specific hw type
    //! \return   MOS_STATUS_SUCCESS if succeeded, else error code.
    //!
    virtual MOS_STATUS Initialize(PLATFORM platform)
    {
        m_hwInfo.SetDeviceInfo(IP_VERSION_UNKNOW, 0);
        return MOS_STATUS_SUCCESS;
    };

};

extern template class MediaFactory<uint32_t, MediaInterfacesHwInfoDevice>;

#endif // __MEDIA_INTERFACES_HWINFO_DEVICE_H__
