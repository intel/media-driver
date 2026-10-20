/*
* Copyright (c) 2024, Intel Corporation
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
//! \file     media_interfaces_huc_kernel_source.h
//! \brief    Gen-specific factory creation of the huc kernel source interfaces
//!

#ifndef __MEDIA_INTERFACES_HUC_KERNEL_SOURCE_H__
#define __MEDIA_INTERFACES_HUC_KERNEL_SOURCE_H__
#include "media_factory.h"

// forward declaration
class HucKernelSource;

//!
//! \class    HucKernelSourceDevice
//! \brief    MMD device
//!
class HucKernelSourceDevice
{
public:
    virtual ~HucKernelSourceDevice() {}

    HucKernelSource *m_hucKernelSource; //!< Media huc kernel source device

    //!
    //! \brief    Create decode huc kernel source instance
    //! \details  Entry point to create Gen specific huc kernel source instance
    //! \param    [in] osInterface
    //!           OS interface
    //! \return   Pointer to Gen specific huc kernel instance if
    //!           successful, otherwise return nullptr
    //!
    static HucKernelSource *CreateFactory(PMOS_INTERFACE osInterface);

    //!
    //! \brief    Initializes platform specific Huc kernel source states
    //! \param    [in] osInterface
    //!           OS interface
    //! \return   MOS_STATUS_SUCCESS if succeeded, else error code.
    //!
    virtual MOS_STATUS Initialize() = 0;

MEDIA_CLASS_DEFINE_END(HucKernelSourceDevice)
};

extern template class MediaFactory<uint32_t, HucKernelSourceDevice>;

#endif // __MEDIA_INTERFACES_HUC_KERNEL_SOURCE_H__
