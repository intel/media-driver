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
//! \brief    Defines functions for constructing Vdbox HUC commands on Gen8-based platforms
//!

#ifndef __MHW_VDBOX_HUC_G11_X_H__
#define __MHW_VDBOX_HUC_G11_X_H__

#include "mhw_vdbox_huc_generic.h"
#include "mhw_vdbox_huc_hwcmd_g11_X.h"
#include "mhw_mi_hwcmd_g11_X.h"

//!  MHW Vdbox Huc interface for Gen11
/*!
This class defines the Huc command construction functions for Gen11 based platforms
*/

class MhwVdboxHucInterfaceG11 : public MhwVdboxHucInterfaceGeneric<mhw_vdbox_huc_g11_X, mhw_mi_g11_X>
{

public:
    //!
    //! \brief  Constructor
    //!
    MhwVdboxHucInterfaceG11(
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
    virtual ~MhwVdboxHucInterfaceG11() { MHW_FUNCTION_ENTER; }

    //!
    //! \brief    Get huc product family
    //!
    //! \return   uint32_t
    //!           Huc product family.
    //!
    uint32_t GetHucProductFamily() override
    {
        return m_hucFamilyIcelake;
    }

    static const uint32_t m_hucFamilyIcelake = 6;

protected:
    MOS_STATUS AddHucPipeModeSelectCmd(
        MOS_COMMAND_BUFFER                  *cmdBuffer,
        MHW_VDBOX_PIPE_MODE_SELECT_PARAMS   *params) override;
    MOS_STATUS AddHucImemStateCmd(
        MOS_COMMAND_BUFFER                  *cmdBuffer,
        MHW_VDBOX_HUC_IMEM_STATE_PARAMS     *params) override;

    MOS_STATUS GetHucStateCommandSize(
        uint32_t                        mode,
        uint32_t                        *commandsSize,
        uint32_t                        *patchListSize,
        PMHW_VDBOX_STATE_CMDSIZE_PARAMS params) override;

    void InitMmioRegisters();
};

#endif
