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
//! \file     mhw_vdbox_huc_g9_bxt.h
//! \brief    Defines functions for constructing Vdbox HUC commands on Gen9 Bxt platforms
//!

#ifndef __MHW_VDBOX_HUC_G9_BXT_H__
#define __MHW_VDBOX_HUC_G9_BXT_H__

#include "mhw_vdbox_huc_generic.h"
#include "mhw_vdbox_huc_hwcmd_g9_bxt.h"
#include "mhw_mi_hwcmd_g9_X.h"

//!  MHW Vdbox Huc interface for Gen9 BXT platforms
/*!
This class defines the Huc command construction functions for Gen9 BXT platform
*/

class MhwVdboxHucInterfaceG9Bxt : public MhwVdboxHucInterfaceGeneric<mhw_vdbox_huc_g9_bxt, mhw_mi_g9_X>
{

public:
    //!
    //! \brief  Constructor
    //!
    MhwVdboxHucInterfaceG9Bxt(
        PMOS_INTERFACE osInterface,
        MhwMiInterface *miInterface,
        MhwCpInterface *cpInterface) :
        MhwVdboxHucInterfaceGeneric(osInterface, miInterface, cpInterface)
    {
        MHW_FUNCTION_ENTER;

        InitMmioRegisters();
    }

    //!
    //! \brief    Destructor
    //!
    virtual ~MhwVdboxHucInterfaceG9Bxt() { MHW_FUNCTION_ENTER; }

protected:

    MOS_STATUS AddHucPipeModeSelectCmd(
        MOS_COMMAND_BUFFER                  *cmdBuffer,
        MHW_VDBOX_PIPE_MODE_SELECT_PARAMS   *params);

    MOS_STATUS GetHucStateCommandSize(
        uint32_t                        mode,
        uint32_t                        *commandsSize,
        uint32_t                        *patchListSize,
        PMHW_VDBOX_STATE_CMDSIZE_PARAMS params);

    void InitMmioRegisters();
};

#endif
