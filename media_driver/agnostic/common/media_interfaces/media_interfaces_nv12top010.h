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
//! \file     media_interfaces_nv12top010.h
//! \brief    Gen-specific factory creation of the nv12 to p010 interfaces
//!

#ifndef __MEDIA_INTERFACES_NV12TOP010_H__
#define __MEDIA_INTERFACES_NV12TOP010_H__

// forward declaration
class CodechalDecodeNV12ToP010;

//!
//! \class    Nv12ToP010Device
//! \brief    Nv12 to P010 device
//!
class Nv12ToP010Device
{
public:
    virtual ~Nv12ToP010Device() {};

    //!
    //! \brief    Create nv12 to p010 instance
    //! \details  Entry point to create Gen specific nv12 to p010 instance
    //! \param    [in] osInterface
    //!           OS interface
    //! \return   Pointer to Gen specific nv12 to p010 instance if
    //!           successful, otherwise return nullptr
    //!
    static CodechalDecodeNV12ToP010* CreateFactory(
        PMOS_INTERFACE osInterface);

    //!
    //! \brief    Initializes platform specific nv12 to p010 states
    //! \param    [in] osInterface
    //!           OS interface
    //! \return   MOS_STATUS_SUCCESS if succeeded, else error code.
    //!
    virtual MOS_STATUS Initialize(
        PMOS_INTERFACE osInterface) = 0;

    CodechalDecodeNV12ToP010  *m_nv12ToP010device = nullptr; //!< Nv12ToP010 device

};

extern template class MediaFactory<uint32_t, Nv12ToP010Device>;

#endif // __MEDIA_INTERFACES_NV12TOP010_H__
