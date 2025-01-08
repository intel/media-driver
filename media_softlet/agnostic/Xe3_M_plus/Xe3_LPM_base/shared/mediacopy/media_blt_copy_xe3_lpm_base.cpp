/*
* Copyright (c) 2023, Intel Corporation
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
//! \file     media_blt_copy_xe3_lpm_base.cpp
//! \brief    Common interface used in Blitter Engine
//! \details  Common interface used in Blitter Engine which are platform independent
//!

#include "media_blt_copy_xe3_lpm_base.h"
#include "mhw_cp_interface.h"
#include "media_perf_profiler.h"
#include "mhw_blt_hwcmd_xe3_lpm.h"
//!
//! \brief    BltStateXe3_Lpm constructor
//! \details  Initialize the BltStateXe3_Lpm members.
//! \param    osInterface
//!           [in] Pointer to MOS_INTERFACE.
//!
BltStateXe3_Lpm::BltStateXe3_Lpm(PMOS_INTERFACE    osInterface) :
    BltStateNext(osInterface)
{
}

//!
//! \brief    BltStateXe3_Lpm constructor
//! \details  Initialize the BltStateXe3_Lpm members.
//! \param    osInterface
//!           [in] Pointer to MOS_INTERFACE.
//!
BltStateXe3_Lpm::BltStateXe3_Lpm(PMOS_INTERFACE    osInterface, MhwInterfacesNext *mhwInterfaces) :
    BltStateNext(osInterface, mhwInterfaces)
{
}


BltStateXe3_Lpm::~BltStateXe3_Lpm()
{
}

//!
//! \brief    BltStateXe3_Lpm initialize
//! \details  Initialize the BltStateXe3_Lpm, create BLT context.
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS BltStateXe3_Lpm::Initialize()
{
    MOS_FUNCTION_ENTER(MOS_COMPONENT_MCPY, MOS_MCPY_SUBCOMP_SELF);
    initialized = true;
    return MOS_STATUS_SUCCESS;
}