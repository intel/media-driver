/*
* Copyright (c) 2014-2017, Intel Corporation
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
#include "cplib_utils.h"

MhwCpInterface* Create_MhwCpInterface(PMOS_INTERFACE osInterface)
{
    MhwCpInterface* pMhwCpInterface = nullptr;
    using Create_MhwCpFuncType = MhwCpInterface* (*)(PMOS_INTERFACE osInterface);
    CPLibUtils::InvokeCpFunc<Create_MhwCpFuncType>(
        pMhwCpInterface, 
        CPLibUtils::FUNC_CREATE_MHWCP, osInterface);

    if(nullptr == pMhwCpInterface) MhwStubMessage();

    return nullptr == pMhwCpInterface? MOS_New(MhwCpInterface) : pMhwCpInterface;
}

void Delete_MhwCpInterface(MhwCpInterface* pMhwCpInterface)
{
    if(nullptr == pMhwCpInterface)
    {
        return;
    }

    if(typeid(*pMhwCpInterface) == typeid(MhwCpInterface))
    {
        MOS_Delete(pMhwCpInterface);
    }
    else
    {
        using Delete_MhwCpFuncType = void (*)(MhwCpInterface*);
        CPLibUtils::InvokeCpFunc<Delete_MhwCpFuncType>(
            CPLibUtils::FUNC_DELETE_MHWCP, 
            pMhwCpInterface);
    }
}
