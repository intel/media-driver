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
//! \file     media_interfaces_vphal.h
//! \brief    Gen-specific factory creation of the vphal interfaces
//!

#ifndef __MEDIA_INTERFACES_VPHAL_H__
#define __MEDIA_INTERFACES_VPHAL_H__

#include "media_interfaces.h"
#include "vphal.h"

//!
//! \class    VphalDevice
//! \brief    Vphal device
//!
class VphalDevice
{
public:
    virtual ~VphalDevice() {}

    VphalState* m_vphalState = nullptr; //!< VpHal State created for specific gen.

    //!
    //! \brief    Creates the VpHal device.
    //! \details  Allocates all interfaces necessary for VpHal to function in the requested configuration and sets up all function pointers.
    //! \param    [in] osInterface
    //!           If an OS interface already exists, it may be passed in here for use by the VpHal device, if not, one is created.
    //! \param    [in] osDriverContext
    //!           OS context used by to initialize the MOS_INTERFACE, includes information necessary for resource management and interfacing with KMD in general
    //! \param    [out] eStatus
    //!           MOS_STATUS, return MOS_STATUS_SUCCESS if successful, otherwise failed.
    //! \return   VphalState*
    //!           returns a valid pointer if successful and nullptr if failed.
    //!
    static VphalState* CreateFactory(
        PMOS_INTERFACE  osInterface,
        PMOS_CONTEXT    osDriverContext,
        MOS_STATUS      *eStatus);

    //!
    //! \brief    Initializes platform specific state
    //! \param    [in] osInterface
    //!           If an OS interface already exists, it may be passed in here for use by the VpHal device, if not, one is created.
    //! \param    [in] osDriverContext
    //!           OS context used by to initialize the MOS_INTERFACE, includes information necessary for resource management and interfacing with KMD in general
    //! \param    [out] eStatus
    //!           MOS status, return MOS_STATUS_SUCCESS if successful, otherwise failed.
    //! \return   MOS_STATUS_SUCCESS if succeeded, else error code.
    //!
    virtual MOS_STATUS Initialize(
        PMOS_INTERFACE  osInterface,
        PMOS_CONTEXT    osDriverContext,
        MOS_STATUS      *eStatus) = 0;

    //!
    //! \brief    Destroys all created VpHal interfaces
    //! \details  If the HAL creation fails, this is used for cleanup
    //!
    void Destroy()
    {
        MOS_Delete(m_vphalState);
        m_vphalState = nullptr;
    }
};

extern template class MediaInterfacesFactory<VphalDevice>;

#endif // __MEDIA_INTERFACES_VPHAL_H__
