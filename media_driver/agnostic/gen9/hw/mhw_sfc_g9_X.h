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
//! \file     mhw_sfc_g9_X.h
//! \brief    Defines functions for constructing sfc commands on Gen9-based platforms
//!

#ifndef __MHW_SFC_G9_X_H__
#define __MHW_SFC_G9_X_H__

#include "mhw_sfc_generic.h"
#include "mhw_sfc_hwcmd_g9_X.h"
#include "mhw_utilities.h"
#include "mos_os.h"

class MhwSfcInterfaceG9 : public MhwSfcInterfaceGeneric<mhw_sfc_g9_X>
{
public:
    MhwSfcInterfaceG9(PMOS_INTERFACE pOsInterface);

    virtual ~MhwSfcInterfaceG9()
    {

    }

    MOS_STATUS AddSfcState(
        PMOS_COMMAND_BUFFER            pCmdBuffer,
        PMHW_SFC_STATE_PARAMS          pSfcStateParams,
        PMHW_SFC_OUT_SURFACE_PARAMS    pOutSurface);

};
#endif // __MHW_SFC_G9_X_H__
