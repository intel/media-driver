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
//! \file     media_interfaces_cmhal.h
//! \brief    Gen-specific factory creation of the cmhal interfaces
//!

#ifndef __MEDIA_INTERFACES_CMHAL_H__
#define __MEDIA_INTERFACES_CMHAL_H__

#include "media_factory.h"
#include "cm_hal.h"

//!
//! \class    CMHalDevice
//! \brief    CM hal device
//!
class CMHalDevice
{
public:
    virtual ~CMHalDevice() {}

    //!
    //! \brief    Create Platform-related interfaces in CM Hal
    //! \details  Entry point to create Platform-related interfaces in CM Hal
    //! \param    [in] pCmState
    //!           pointer to the whole CM Hal state struct
    //!
    //! \return   Pointer to Platform-related interfaces in CM Hal
    //!
    static CM_HAL_GENERIC* CreateFactory(
        CM_HAL_STATE *pCmState);

protected:
    //!
    //! \brief    Initializes platform specific CM Hal interfaces
    //! \param    [in] pCmState
    //!           pointer to the whole CM Hal state struct
    //! \return   MOS_STATUS_SUCCESS if succeeded, else error code.
    //!
    virtual MOS_STATUS Initialize(
        CM_HAL_STATE *pCmState) = 0;

    CM_HAL_GENERIC* m_cmhalDevice = nullptr;
};

extern template class MediaFactory<uint32_t, CMHalDevice>;


#endif // __MEDIA_INTERFACES_CMHAL_H__
