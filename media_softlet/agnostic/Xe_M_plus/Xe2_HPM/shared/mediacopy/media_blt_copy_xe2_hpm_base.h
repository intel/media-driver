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
//! \file     media_blt_copy_xe2_hpm_base.h
//! \brief    Common interface and structure used in Blitter Engine
//! \details  Common interface and structure used in Blitter Engine which are platform independent
//!
#ifndef __MEDIA_BLT_COPY_XE2_HPM_BASE_H__
#define __MEDIA_BLT_COPY_XE2_HPM_BASE_H__

#include "media_blt_copy_next.h"
#include "mhw_blt_hwcmd_xe2_hpm.h"

class BltStateXe2_Hpm_Base: virtual public BltStateNext
{
public:
    //!
    //! \brief    BltStateXe2_Hpm_Base constructor
    //! \details  Initialize the BltState members.
    //! \param    osInterface__MEDIA_BLT_COPY_XE2_HPM_BASE_H__
    //!           [in] Pointer to MOS_INTERFACE.
    //!
    BltStateXe2_Hpm_Base(PMOS_INTERFACE    osInterface);
    BltStateXe2_Hpm_Base(PMOS_INTERFACE    osInterface, MhwInterfacesNext *mhwInterfaces);

    virtual ~BltStateXe2_Hpm_Base();

    //!
    //! \brief    BltStateXe2_Hpm_Base initialize
    //! \details  Initialize the BltStateXe2_Hpm_Base, create BLT context.
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS Initialize();

MEDIA_CLASS_DEFINE_END(BltStateXe2_Hpm_Base)
};

#endif // __MEDIA_BLT_COPY_XE2_HPM_BASE_H__

