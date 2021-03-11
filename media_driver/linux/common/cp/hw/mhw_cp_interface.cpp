/*
* Copyright (c) 2014-2020, Intel Corporation
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
//! \file     mhw_cp_interface.cpp
//! \brief    Implement MHW interface for content protection
//! \details  Impelements the functionalities across all platforms for content protection
//!

#include "mhw_cp_interface.h"
#include "cp_interfaces.h"

MhwCpInterface* Create_MhwCpInterface(PMOS_INTERFACE osInterface)
{
    MhwCpInterface* pInterface = nullptr;
    CpInterfaces *cp_interface = CpInterfacesFactory::Create(CP_INTERFACE);
    if (cp_interface)
    {
        pInterface = cp_interface->Create_MhwCpInterface(osInterface);
        MOS_Delete(cp_interface);
    }

    if (nullptr == pInterface) MhwStubMessage();

    return (nullptr == pInterface) ? MOS_New(MhwCpInterface) : pInterface;
}

void Delete_MhwCpInterface(MhwCpInterface* pInterface)
{
    CpInterfaces *cp_interface = CpInterfacesFactory::Create(CP_INTERFACE);
    if (pInterface != nullptr && cp_interface != nullptr)
    {
        cp_interface->Delete_MhwCpInterface(pInterface);
        pInterface = nullptr;
    }
    MOS_Delete(pInterface);
    MOS_Delete(cp_interface);
}
