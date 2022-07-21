/*===================== begin_copyright_notice ==================================

* Copyright (c) 2021, Intel Corporation
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

======================= end_copyright_notice ==================================*/
//!
//! \file     mhw_vebox_xe_hpm.h
//! \brief    Defines functions for constructing vebox commands on Xe_HPM-based platforms
//!

#ifndef __MHW_VEBOX_XE_HPM_H__
#define __MHW_VEBOX_XE_HPM_H__

#include "mhw_vebox_g12_X.h"
#include "mhw_vebox_xe_xpm.h"
#include "mhw_vebox_hwcmd_xe_xpm.h"

//!  MHW vebox  interface for Xe_HPM
/*!
This class defines the VEBOX command interface for Xe_HPM common platforms
*/
class MhwVeboxInterfaceXe_Hpm: public MhwVeboxInterfaceXe_Xpm
{
public:
    MhwVeboxInterfaceXe_Hpm(PMOS_INTERFACE pOsInterface);
    virtual ~MhwVeboxInterfaceXe_Hpm();

    MOS_STATUS AddVeboxDndiState(
        PMHW_VEBOX_DNDI_PARAMS pVeboxDndiParams) override;

    MOS_STATUS ForceGNEParams(uint8_t *pDnDiSate);
    MOS_STATUS DumpDNDIStates(uint8_t *pDnDiSate);
    uint32_t dwLumaStadTh             = 3200;
    uint32_t dwChromaStadTh           = 1600;
    bool     bTGNEEnable              = false;
    bool     bHVSAutoBdrateEnable     = false;
    bool     bHVSAutoSubjectiveEnable = false;
    bool     bHVSfallback             = false;
    uint32_t dw4X4TGNEThCnt           = 576;
    uint32_t dwBSDThreshold           = 480;
    uint32_t dwHistoryInit            = 32;
};

#endif // __MHW_SFC_XE_HPM_H__

