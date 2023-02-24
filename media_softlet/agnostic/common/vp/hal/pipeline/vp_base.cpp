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
//! \file     vp_base.cpp
//! \brief    vp base clarification
//! \details  vp base clarification inlcuding:
//!           some marcro, enum, structure, function
//!
#include "vp_base.h"
#include "media_interfaces_vphal.h"
#include "mos_utilities.h"
#include "vp_utils.h"

VpBase::VpBase()
{
}

VpBase::~VpBase()
{
    if (extIntf)
    {
        MOS_Delete(extIntf);
        extIntf = nullptr;
    }
}

VpBase* VpBase::VphalStateFactory(
    PMOS_INTERFACE     osInterface,
    MOS_CONTEXT_HANDLE osDriverContext,
    MOS_STATUS         *peStatus,
    bool               clearViewMode)
{
    VP_FUNC_CALL();

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    VpBase *vpBase = VphalDevice::CreateFactoryNext(osInterface, (PMOS_CONTEXT)osDriverContext, &eStatus, clearViewMode);

    if (peStatus)
    {
        *peStatus = eStatus;
    }

    return vpBase;
}