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
//! \file     mhw_vdbox_mfx_g9_skl.h
//! \brief    Defines functions for constructing Vdbox MFX commands on G9 SKL
//!

#ifndef __MHW_VDBOX_MFX_G9_SKL_H__
#define __MHW_VDBOX_MFX_G9_SKL_H__

#include "mhw_vdbox_mfx_g9_X.h"
#include "mhw_vdbox_mfx_hwcmd_g9_skl.h"

//!  MHW Vdbox Mfx interface for Gen9 SKL platform
/*!
This class defines the Mfx command construction functions for Gen9 SKL platform
*/
class MhwVdboxMfxInterfaceG9Skl : public MhwVdboxMfxInterfaceG9<mhw_vdbox_mfx_g9_skl>
{
public:
    //!
    //! \brief    Constructor
    //!
    MhwVdboxMfxInterfaceG9Skl(
        PMOS_INTERFACE osInterface,
        MhwMiInterface *pMiInterface,
        MhwCpInterface *cpInterface,
        bool decodeInUse) :
        MhwVdboxMfxInterfaceG9(osInterface, pMiInterface, cpInterface, decodeInUse)
    {
        MHW_FUNCTION_ENTER;
    }

    //!
    //! \brief    Destructor
    //!
    virtual ~MhwVdboxMfxInterfaceG9Skl() { MHW_FUNCTION_ENTER; }

protected:

    MOS_STATUS AddMfxPipeBufAddrCmd(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMHW_VDBOX_PIPE_BUF_ADDR_PARAMS params);
};

#endif
