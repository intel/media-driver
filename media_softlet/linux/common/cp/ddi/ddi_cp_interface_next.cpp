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
//! \file     ddi_cp_interface_next.cpp
//! \brief    The class implementation of DdiCpInterfaceNext
//!

#include "ddi_cp_interface_next.h"
#include "media_libva_decoder.h"
#include "media_libva_vp.h"
#include "cp_interfaces.h"

static const bool registeredDdiCpInterfaceNext = DdiCpFactory::Register<DdiCpInterfaceNext>(DdiCpCreateType::CreateDdiCpInterface);

DdiCpInterfaceNext *CreateDdiCpNext(MOS_CONTEXT* mosCtx)
{
    DDI_CP_FUNC_ENTER;
    
    DDI_CP_CHK_NULL(mosCtx, "mosCtx is nullptr", nullptr);
    DdiCpInterfaceNext *ddiCp = nullptr;
    ddiCp = DdiCpFactory::Create(DdiCpCreateType::CreateDdiCpInterface, *mosCtx);
    if(ddiCp == nullptr)
    {
        DDI_CP_ASSERTMESSAGE("Create Cp instance failed.");
    }
    
    return ddiCp;
}
