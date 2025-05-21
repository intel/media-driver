/*
* Copyright (c) 2020-2021, Intel Corporation
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
//! \file     media_interfaces_mcpy.h
//! \brief    Gen-specific factory creation of the mcpy interfaces
//!

#ifndef __MEDIA_INTERFACES_MCPY_H__
#define __MEDIA_INTERFACES_MCPY_H__

#include "media_interfaces_mhw.h"

// forward declaration
class MediaCopyBaseState;

//!
//! \class    McpyDevice
//! \brief    MCPY device
//!
class McpyDevice
{
public:
    virtual ~McpyDevice() {}

    MediaCopyBaseState *m_mcpyDevice = nullptr; //!< Media memory copy device

    //!
    //! \brief    Create Media memory copy instance
    //! \details  Entry point to create Gen specific media memory compression instance
    //! \param    [in] osDriverContext
    //!           OS context used by to initialize the MOS_INTERFACE, includes information necessary for resource management and interfacing with KMD in general
    //!
    //! \return   Pointer to Gen specific media memory compression instance if
    //!           successful, otherwise return nullptr
    //!
    static void* CreateFactory(
        PMOS_CONTEXT osDriverContext);

    //!
    //! \brief    Initializes platform specific MMD (media memory decompression) states
    //! \param    [in] osInterface
    //!           OS interface
    //! \param    [in] mhwInterfaces
    //!           HW interfaces to be used by MMD
    //! \return   MOS_STATUS_SUCCESS if succeeded, else error code.
    //!
    virtual MOS_STATUS Initialize(
        PMOS_INTERFACE osInterface) = 0;
protected:
    //!
    //! \brief    Creat platform specific MMD (media memory decompression) HW interface
    //! \param    [in] osInterface
    //!           OS interface
    //! \return   MhwInterfaces if succeeded.
    //!
    virtual MhwInterfaces* CreateMhwInterface(
        PMOS_INTERFACE osInterface);
};

extern template class MediaFactory<uint32_t, McpyDevice>;

#endif // __MEDIA_INTERFACES_MCPY_H__

