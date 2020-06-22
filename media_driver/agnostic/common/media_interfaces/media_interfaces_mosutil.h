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
//! \file     media_interfaces_mosutil.h
//! \brief    Helps with gen-specific mosutil device factory creation.
//!

#ifndef __MEDIA_INTERFACES_MOSUTIL_H__
#define __MEDIA_INTERFACES_MOSUTIL_H__

#include <map>
#include "media_user_settings_mgr.h"
#include "media_interfaces.h"

//!
//! \class    MosUtilDevice
//! \brief    MOS util device
//!
class MosUtilDevice
{
public:
    virtual ~MosUtilDevice() {}

    void* m_mosUtilDevice = nullptr; //!< Mos utilities device supporting requested platform

    //!
    //! \brief    Create Mos Utilities instance
    //! \param    [in] productFamily
    //!           Product family.
    //!
    //! \return   Pointer to Gen specific Mos util instance if
    //!           successful, otherwise return nullptr
    //!
    static void* CreateFactory(
        PRODUCT_FAMILY productFamily);

    //!
    //! \brief    Initializes platform specific Mos util interfaces
    //! \return   MOS_STATUS_SUCCESS if succeeded, else error code.
    //!
    virtual MOS_STATUS Initialize() = 0;

};

extern template class MediaInterfacesFactory<MosUtilDevice>;

#endif // __MEDIA_INTERFACES_MOSUTIL_H__
