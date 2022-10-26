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
//! \file     mhw_mi_xe_lpm_plus_base_next.h
//! \brief    Defines functions for constructing HW commands on Gen12-based platforms
//!

#ifndef __MHW_MI_XE_LPM_PLUS_BASE_NEXT_H__
#define __MHW_MI_XE_LPM_PLUS_BASE_NEXT_H__

#include "mhw_mi_xe_lpm_plus_base.h"
#include "mhw_mi_hwcmd_xe_lpm_plus_base_next.h"
#include "mhw_mi_itf.h"
#include "mhw_mi_xe_lpm_plus_base_next_impl.h"
#include "mhw_mi_xe_lpm_plus_base.h"

//!
//! \brief    Mtl MHW MI command interface
//! \details  The Mtl MHW MI interface contains functions to add Mtl MI commands to command buffer or batch buffer
//!
struct MhwMiInterfaceXe_Lpm_Plus_Base_Next : public MhwMiInterfaceXe_Lpm_Plus_Base
{
    MhwMiInterfaceXe_Lpm_Plus_Base_Next(
        MhwCpInterface      *cpInterface,
        PMOS_INTERFACE      osInterface) :
        MhwMiInterfaceXe_Lpm_Plus_Base(cpInterface, osInterface)
        {
            MHW_FUNCTION_ENTER;
            m_cpInterface = cpInterface;
        }

    ~MhwMiInterfaceXe_Lpm_Plus_Base_Next() { MHW_FUNCTION_ENTER; };

    std::shared_ptr<void> GetNewMiInterface() override
    {
        if (!m_miItfNew)
        {
            auto ptr = std::make_shared<mhw::mi::xe_lpm_plus_base_next::Impl>(m_osInterface);
            ptr->SetCpInterface(m_cpInterface);
            m_miItfNew = ptr;
        }

        return m_miItfNew;
    }
};
#endif
