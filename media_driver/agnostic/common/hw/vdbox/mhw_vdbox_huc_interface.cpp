/*
* Copyright (c) 2017-2018, Intel Corporation
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

//! \file     mhw_vdbox_huc_interface.cpp
//! \brief    MHW interface for constructing Huc commands for the Vdbox engine
//! \details  Defines the interfaces for constructing MHW Vdbox Huc commands across all platforms 
//!

#include "mhw_vdbox_huc_interface.h"

MhwVdboxHucInterface::MhwVdboxHucInterface(
    PMOS_INTERFACE osInterface,
    MhwMiInterface *miInterface,
    MhwCpInterface *cpInterface)
{
    MHW_FUNCTION_ENTER;

    m_osInterface = osInterface;
    m_MiInterface = miInterface;
    m_cpInterface = cpInterface;

    MHW_ASSERT(m_osInterface);
    MHW_ASSERT(m_MiInterface);
    MHW_ASSERT(m_cpInterface);

    m_waTable = osInterface->pfnGetWaTable(osInterface);

    if (m_osInterface->bUsesGfxAddress)
    {
        AddResourceToCmd = Mhw_AddResourceToCmd_GfxAddress;
    }
    else // bUsesPatchList
    {
        AddResourceToCmd = Mhw_AddResourceToCmd_PatchList;
    }

}

