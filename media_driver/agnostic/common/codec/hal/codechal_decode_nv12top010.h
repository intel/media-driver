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
//! \file     codechal_decode_nv12top010.h
//! \brief    Defines the interface for conversion from NV12 to P010 format.
//! \details  This file is for the base interface which is shared by all platforms.
//!

#ifndef __CODECHAL_DECODE_NV12TOP010_H__
#define __CODECHAL_DECODE_NV12TOP010_H__

#include "cm_rt_umd.h"
//!
//! \class CodechalDecodeNV12ToP010
//! \brief This class converts NV12 to P010 format.
//!
class CodechalDecodeNV12ToP010
{
public:
    //!
    //! \brief    Constructor
    //!
    CodechalDecodeNV12ToP010() {};

    //!
    //! \brief    Destructor
    //!
    virtual ~CodechalDecodeNV12ToP010();

    //!
    //! \brief  The entry to convert NV12 surface to P010 surface
    //! \param  [in] srcResource
    //!         The NV12 surface
    //! \param  [out] dstResource
    //!         The P010 surface
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS Execute(
        PMOS_RESOURCE srcResource,
        PMOS_RESOURCE dstResource);

protected:
    //!
    //! \brief  Initialize the context for converting NV12 surface to P010 surface
    //! \param  [in] osInterface
    //!         The OS interface
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS Init(PMOS_INTERFACE osInterface);

protected:
    //! \brief Pointer to kernel binary
    const unsigned int *m_nv12ToP010KernelBinary = nullptr;
    //! \brief Size of kernel binary
    unsigned int m_nv12ToP010KernelSize = 0;

private:
    //! \brief Pointer to CM device
    CmDevice        *m_cmDevice = nullptr;
    //! \brief Pointer to CM queue
    CmQueue         *m_cmQueue = nullptr;
    //! \brief Pointer to CM kernel
    CmKernel        *m_cmKernel = nullptr;
    //! \brief Pointer to CM task
    CmTask          *m_cmTask = nullptr;
    //! \brief Pointer to CM thread space
    CmThreadSpace   *m_cmThreadSpace = nullptr;
    //! \brief Pointer to os interface
    PMOS_INTERFACE  m_osInterface = nullptr;
};

#endif

