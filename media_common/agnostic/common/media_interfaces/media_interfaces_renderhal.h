/*
* Copyright (c) 2013-2017, Intel Corporation
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
//! \file     media_interfaces_renderhal.h
//! \brief    Gen-specific factory creation of the renderhal interfaces
//!

#ifndef __MEDIA_INTERFACES_RENDERHAL_H__
#define __MEDIA_INTERFACES_RENDERHAL_H__

#include "media_factory.h"
#include "renderhal_platform_interface.h"

//!
//! \class    RenderHalDevice
//! \brief    Render hal device
//!
class RenderHalDevice
{
public:
    virtual ~RenderHalDevice() {}

    //!
    //! \brief    Create Platform-related interfaces in Render Hal
    //! \details  Entry point to create Platform-related interfaces in Render Hal
    //! \param    [in] osInterface
    //!           Pointer to the MOS interfaces
    //! \return   Pointer to Platform-related interfaces in Render Hal
    //!
    static XRenderHal_Platform_Interface* CreateFactory(
                                    PMOS_INTERFACE osInterface);

protected:
    //!
    //! \brief    Initializes platform specific Render Hal interfaces
    //! \return   MOS_STATUS_SUCCESS if succeeded, else error code.
    //!
    virtual MOS_STATUS Initialize() = 0;

    XRenderHal_Platform_Interface *m_renderhalDevice = nullptr;
    PMOS_INTERFACE                m_osInterface      = nullptr;
};

extern template class MediaFactory<uint32_t, RenderHalDevice>;

#endif // __MEDIA_INTERFACES_RENDERHAL_H__
