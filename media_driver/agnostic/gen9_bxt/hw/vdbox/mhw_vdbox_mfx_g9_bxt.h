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
//! \file     mhw_vdbox_mfx_g9_bxt.h
//! \brief    Defines functions for constructing Vdbox MFX commands on G9 BXT platform
//!

#ifndef __MHW_VDBOX_MFX_G9_BXT_H__
#define __MHW_VDBOX_MFX_G9_BXT_H__

#include "mhw_vdbox_mfx_g9_X.h"
#include "mhw_vdbox_mfx_hwcmd_g9_bxt.h"

//!  MHW Vdbox Mfx interface for Gen9 BXT platform
/*!
This class defines the Mfx command construction functions for Gen9 BXT platform
*/
class MhwVdboxMfxInterfaceG9Bxt : public MhwVdboxMfxInterfaceG9<mhw_vdbox_mfx_g9_bxt>
{
public:
    //!
    //! \brief    Constructor
    //!
    MhwVdboxMfxInterfaceG9Bxt(
        PMOS_INTERFACE osInterface,
        MhwMiInterface *miInterface,
        MhwCpInterface *cpInterface,
        bool decodeInUse) :
        MhwVdboxMfxInterfaceG9(osInterface, miInterface, cpInterface, decodeInUse)
    {
        MHW_FUNCTION_ENTER;
    }

    //!
    //! \brief    Destructor
    //!
    virtual ~MhwVdboxMfxInterfaceG9Bxt() { MHW_FUNCTION_ENTER; }

private:
    MOS_STATUS AddMfxPipeBufAddrCmd(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMHW_VDBOX_PIPE_BUF_ADDR_PARAMS params);
};

#endif
