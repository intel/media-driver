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
//! \file     media_vebox_copy_xe_lpm_plus_base.h
//! \brief    Common Copy interface and structure used in Vebox Engine
//! \details  Common Copy interface and structure used in Vebox Engine

#ifndef __MEDIA_VEBOX_COPY_XE_LPM_PLUS_BASE_H__
#define __MEDIA_VEBOX_COPY_XE_LPM_PLUS_BASE_H__

#include "media_vebox_copy_next.h"
#include "media_interfaces_mhw_next.h"

class VeboxCopyStateXe_Lpm_Plus_Base: virtual public VeboxCopyStateNext
{
public:
    //!
    //! \brief    Vebox Copy State constructor
    //! \details  Initialize the VeboxCopyState members.
    //! \param    osInterface
    //!           [in] Pointer to MOS_INTERFACE.
    //!
    VeboxCopyStateXe_Lpm_Plus_Base(PMOS_INTERFACE    osInterface, MhwInterfacesNext* mhwInterfaces);

    virtual ~VeboxCopyStateXe_Lpm_Plus_Base();

MEDIA_CLASS_DEFINE_END(VeboxCopyStateXe_Lpm_Plus_Base)
};

#endif //__MEDIA_VEBOX_COPY_XE_LPM_PLUS_BASE_H__
