/*
* Copyright (c) 2020-2022, Intel Corporation
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
//! \file     null_hardware.cpp
//! \brief    Defines interfaces for null hardware
#include "null_hardware.h"
#include "mhw_mi.h"

MOS_STATUS NullHW::StartPredicate(PMOS_INTERFACE pOsInterface, MhwMiInterface* miInterface, PMOS_COMMAND_BUFFER cmdBuffer)
{
    MOS_OS_CHK_NULL_RETURN(pOsInterface);
    if (!pOsInterface->bNullHwIsEnabled)
    {
        return MOS_STATUS_SUCCESS;
    }
    MOS_OS_CHK_NULL_RETURN(miInterface);
    MOS_OS_CHK_NULL_RETURN(cmdBuffer);

    return miInterface->AddMiSetPredicateCmd(cmdBuffer, MHW_MI_SET_PREDICATE_ENABLE_ALWAYS);
}

MOS_STATUS NullHW::StopPredicate(PMOS_INTERFACE pOsInterface, MhwMiInterface* miInterface, PMOS_COMMAND_BUFFER cmdBuffer)
{
    MOS_OS_CHK_NULL_RETURN(pOsInterface);
    if (!pOsInterface->bNullHwIsEnabled)
    {
        return MOS_STATUS_SUCCESS;
    }
    MOS_OS_CHK_NULL_RETURN(miInterface);
    MOS_OS_CHK_NULL_RETURN(cmdBuffer);

    return miInterface->AddMiSetPredicateCmd(cmdBuffer, MHW_MI_SET_PREDICATE_DISABLE);
}