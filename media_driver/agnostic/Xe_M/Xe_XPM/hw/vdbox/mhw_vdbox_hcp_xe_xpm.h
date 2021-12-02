/*===================== begin_copyright_notice ==================================

# Copyright (c) 2020-2021, Intel Corporation

# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
# OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
# ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
# OTHER DEALINGS IN THE SOFTWARE.

======================= end_copyright_notice ==================================*/
//!
//! \file     mhw_vdbox_hcp_xe_xpm.h
//! \brief    Defines functions for constructing Vdbox HCP commands on Gen11-based platforms
//!

#ifndef __MHW_VDBOX_HCP_XE_XPM_H__
#define __MHW_VDBOX_HCP_XE_XPM_H__

#include "mhw_vdbox_hcp_g12_X.h"

//!  MHW Vdbox HCP interface for Xe_XPM
/*!
This class defines the HCP command interface for Xe_XPM platforms
*/
class MhwVdboxHcpInterfaceXe_Xpm : public MhwVdboxHcpInterfaceG12
{
public:
    MhwVdboxHcpInterfaceXe_Xpm(
        PMOS_INTERFACE  osInterface,
        MhwMiInterface *miInterface,
        MhwCpInterface *cpInterface,
        bool            decodeInUse);
    ~MhwVdboxHcpInterfaceXe_Xpm();

    virtual MOS_STATUS AddHcpEncodePicStateCmd(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMHW_VDBOX_HEVC_PIC_STATE params) override;

    MOS_STATUS AddHcpDecodeSurfaceStateCmd(
        PMOS_COMMAND_BUFFER              cmdBuffer,
        PMHW_VDBOX_SURFACE_PARAMS        params) override;

    MOS_STATUS AddHcpEncodeSurfaceStateCmd(
        PMOS_COMMAND_BUFFER              cmdBuffer,
        PMHW_VDBOX_SURFACE_PARAMS        params) override;

    MOS_STATUS AddHcpPipeBufAddrCmd(
        PMOS_COMMAND_BUFFER                cmdBuffer,
        PMHW_VDBOX_PIPE_BUF_ADDR_PARAMS    params) override;

};

#endif
