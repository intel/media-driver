/*
* Copyright (c) 2015-2021, Intel Corporation
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
//! \file     mhw_mi_xe_xpm_base.h
//! \brief    Defines functions for constructing HW commands on Gen12-based platforms
//!

#ifndef __MHW_MI_XE_XPM_BASE_H__
#define __MHW_MI_XE_XPM_BASE_H__

#include "mhw_mi_g12_X.h"
#include "mhw_mi_hwcmd_xe_xpm_base.h"
#include "mhw_mi_itf.h"
#include "mhw_mi_xe_xpm_base_impl.h"

//!
//! \brief    MHW MI command interface
//! \details  The MHW MI interface contains functions to add MI commands to command buffer or batch buffer
//!
struct MhwMiInterfaceXe_Xpm_Base : public MhwMiInterfaceG12
{
    MhwMiInterfaceXe_Xpm_Base(
        MhwCpInterface      *cpInterface,
        PMOS_INTERFACE      osInterface) :
        MhwMiInterfaceG12(cpInterface, osInterface)
        {
            MHW_FUNCTION_ENTER;
            m_cpInterface = cpInterface;
        }

    ~MhwMiInterfaceXe_Xpm_Base() { MHW_FUNCTION_ENTER; };

    std::shared_ptr<void> GetNewMiInterface() override
    {
        if (!m_miItfNew && m_osInterface)
        {
            auto ptr = std::make_shared<mhw::mi::xe_xpm_base::Impl>(m_osInterface);
            m_miItfNew = ptr;
            ptr->SetCpInterface(m_cpInterface, std::static_pointer_cast<mhw::mi::Itf>(ptr));
        }

        return m_miItfNew;
    }
};
#endif
