/*
* Copyright (c) 2022-2023, Intel Corporation
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
//! \file     media_blt_copy_xe_lpm_plus_base.cpp
//! \brief    Common interface used in Blitter Engine
//! \details  Common interface used in Blitter Engine which are platform independent
//!

#include "media_blt_copy_xe_lpm_plus_base.h"
#include "mhw_cp_interface.h"
#include "media_perf_profiler.h"

//!
//! \brief    BltStateXe_Xpm_Plus constructor
//! \details  Initialize the BltStateXe_Xpm_Plus members.
//! \param    osInterface
//!           [in] Pointer to MOS_INTERFACE.
//!
BltStateXe_Lpm_Plus_Base::BltStateXe_Lpm_Plus_Base(PMOS_INTERFACE    osInterface) :
    BltStateNext(osInterface)
{
}

//!
//! \brief    BltStateXe_Xpm_Plus constructor
//! \details  Initialize the BltStateXe_Xpm_Plus members.
//! \param    osInterface
//!           [in] Pointer to MOS_INTERFACE.
//!
BltStateXe_Lpm_Plus_Base::BltStateXe_Lpm_Plus_Base(PMOS_INTERFACE    osInterface, MhwInterfacesNext *mhwInterfaces) :
    BltStateNext(osInterface, mhwInterfaces)
{
}

BltStateXe_Lpm_Plus_Base::~BltStateXe_Lpm_Plus_Base()
{
}

//!
//! \brief    BltStateXe_Xpm_Plus initialize
//! \details  Initialize the BltStateXe_Xpm_Plus, create BLT context.
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS BltStateXe_Lpm_Plus_Base::Initialize()
{
    BltStateNext::Initialize();
    initialized = true;
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS BltStateXe_Lpm_Plus_Base::SetBCSSWCTR(MOS_COMMAND_BUFFER *cmdBuffer)
{
    BLT_CHK_NULL_RETURN(m_miItf);
    BLT_CHK_NULL_RETURN(cmdBuffer);

    auto &Register       = m_miItf->MHW_GETPAR_F(MI_LOAD_REGISTER_IMM)();
    Register             = {};
    Register.dwRegister  = mhw::blt::xe_lpm_plus_next::Cmd::BCS_SWCTRL_CMD::REGISTER_OFFSET;
    mhw::blt::xe_lpm_plus_next::Cmd::BCS_SWCTRL_CMD swctrl;
    Register.dwData = swctrl.DW0.Value;
    BLT_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_LOAD_REGISTER_IMM)(cmdBuffer));

    return MOS_STATUS_SUCCESS;
}
