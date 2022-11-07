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
//! \file     ddi_cp_register.cpp
//! \brief    cp register source file
//!

#include "ddi_cp_caps_interface.h"

static const bool registeredCpCapsInterface = CpCapsFactory::Register<DdiCpCapsInterface>(CapsInterface);

DdiCpCapsInterface *CreateDdiCpCaps()
{
    DdiCpCapsInterface *ddiCpCaps = nullptr;
    ddiCpCaps = CpCapsFactory::Create(CapsInterface);
    if(ddiCpCaps == nullptr)
    {
        DDI_CP_ASSERTMESSAGE("Create Cp Caps failed.");
    }
    
    return ddiCpCaps;
}