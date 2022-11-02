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
//! \file     media_interfaces_codechal_next.h
//! \brief    Gen-specific factory creation of the codechal interfaces
//!

#ifndef __MEDIA_INTERFACES_CODECHAL_NEXT_H__
#define __MEDIA_INTERFACES_CODECHAL_NEXT_H__

#include "media_interfaces_mhw_next.h"

#include "codechal_common.h"

//!
//! \class    CodechalDeviceNext
//! \brief    Cddehal device
//!
class CodechalDeviceNext
{
public:
    virtual ~CodechalDeviceNext() {}

    Codechal *m_codechalDevice = nullptr; //!< CodecHal device supporting requested mode

    //!
    //! \brief    Creates the CodecHal device.
    //! \details  Allocates all interfaces necessary for CodecHal to function in the requested configuration and sets up all function pointers.
    //! \param    [in] osInterface
    //!           If an OS interface already exists, it may be passed in here for use by the CodecHal device, if not, one is created.
    //! \param    [in] osDriverContext
    //!           OS context used by to initialize the MOS_INTERFACE, includes information necessary for resource management and interfacing with KMD in general
    //! \param    [in] standardInfo
    //!           Information required to correctly initialize the decode or encode interface for a particular standard.
    //! \param    [in] settings
    //!           Codechal setting information.
    //! \return   Codechal*
    //!           returns a valid pointer if successful and nullptr if failed.
    //!
    static Codechal* CreateFactory(
        PMOS_INTERFACE  osInterface,
        PMOS_CONTEXT    osDriverContext,
        void            *standardInfo,
        void            *settings);

    //!
    //! \brief    Initializes platform specific decode and encode states
    //! \param    [in] standardInfo
    //!           Information required to correctly initialize the decode or encode interface for a particular standard.
    //! \param    [in] settings
    //!           Codechal setting information.
    //! \param    [in] mhwInterfaces
    //!           MHW interfaces used to init codechal hw interface.
    //! \param    [in] osInterface
    //!           OS interface for codechal
    //! \return   MOS_STATUS_SUCCESS if succeeded, else error code.
    //!
    virtual MOS_STATUS Initialize(
        void *standardInfo,
        void *settings,
        MhwInterfacesNext *mhwInterfaces,
        PMOS_INTERFACE osInterface) = 0;

MEDIA_CLASS_DEFINE_END(CodechalDeviceNext)
 };

extern template class MediaFactory<uint32_t, CodechalDeviceNext>;

#endif // __MEDIA_INTERFACES_CODECHAL_NEXT_H__
