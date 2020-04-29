/*
* Copyright (c) 2009-2020, Intel Corporation
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
//! \file     mos_os_cp_interface_specific.cpp
//! \brief    OS specific implement for CP related functions
//!

#include "mos_os_cp_interface_specific.h"
#include "cp_interfaces.h"
#include "mos_os.h"

MosCpInterface* Create_MosCpInterface(void* pvOsInterface)
{
    CpInterfaces *cp_interface = CpInterfacesFactory::Create(CP_INTERFACE);
    if (nullptr == cp_interface)
    {
        MOS_NORMALMESSAGE(MOS_COMPONENT_CP, MOS_CP_SUBCOMP_OS, "NULL pointer parameters");
        return nullptr;
    }

    MosCpInterface* pMosCpInterface = nullptr;
    pMosCpInterface = cp_interface->Create_MosCpInterface(pvOsInterface);
    MOS_Delete(cp_interface);

    if (nullptr == pMosCpInterface) OsStubMessage();

    return nullptr == pMosCpInterface ? MOS_New(MosCpInterface) : pMosCpInterface;
}

void Delete_MosCpInterface(MosCpInterface* pInterface)
{
    CpInterfaces *cp_interface = CpInterfacesFactory::Create(CP_INTERFACE);
    if (pInterface != nullptr && cp_interface != nullptr)
    {
        cp_interface->Delete_MosCpInterface(pInterface);
    }
    MOS_Delete(cp_interface);
}


