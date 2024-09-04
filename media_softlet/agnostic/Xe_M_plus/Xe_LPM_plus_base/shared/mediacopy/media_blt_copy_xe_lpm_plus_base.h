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
//! \file     media_blt_copy_xe_lpm_plus_base.h
//! \brief    Common interface and structure used in Blitter Engine
//! \details  Common interface and structure used in Blitter Engine which are platform independent
//!
#ifndef __MEDIA_BLT_COPY_XE_LPM_PLUS_BASE_H__
#define __MEDIA_BLT_COPY_XE_LPM_PLUS_BASE_H__

#include "media_blt_copy_next.h"
#include "mhw_blt_hwcmd_xe_lpm_plus_next.h"

class BltStateXe_Lpm_Plus_Base: virtual public BltStateNext
{
public:
    //!
    //! \brief    BltStateXe_Xpm_Plus constructor
    //! \details  Initialize the BltState members.
    //! \param    osInterface
    //!           [in] Pointer to MOS_INTERFACE.
    //!
    BltStateXe_Lpm_Plus_Base(PMOS_INTERFACE    osInterface);
    BltStateXe_Lpm_Plus_Base(PMOS_INTERFACE    osInterface, MhwInterfacesNext *mhwInterfaces);

    virtual ~BltStateXe_Lpm_Plus_Base();

    //!
    //! \brief    BltStateXe_Xpm_Plus initialize
    //! \details  Initialize the BltStateXe_Xpm_Plus, create BLT context.
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS Initialize();

protected:
    //!
    //! \brief    Set BCS_SWCTR cmd
    //! \details  Set BCS_SWCTR for Cmdbuffer
    //! \param    PMOS_COMMAND_BUFFER
    //!           [in] Pointer to PMOS_COMMAND_BUFFER
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS SetBCSSWCTR(MOS_COMMAND_BUFFER *cmdBuffer);

MEDIA_CLASS_DEFINE_END(BltStateXe_Lpm_Plus_Base)
};

#endif // __MEDIA_BLT_COPY_XE_LPM_PLUS_BASE_H__

