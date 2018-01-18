/*
* Copyright (c) 2017, Intel Corporation
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
//! \file     mhw_vdbox_vdenc_g9_skl.h
//! \brief    Defines functions for constructing Vdbox Vdenc commands on G9 SKL
//!

#ifndef __MHW_VDBOX_VDENC_G9_SKL_H__
#define __MHW_VDBOX_VDENC_G9_SKL_H__

#include "mhw_vdbox_vdenc_g9_X.h"
#include "mhw_vdbox_vdenc_hwcmd_g9_skl.h"

//!  MHW Vdbox Vdenc interface for Gen9 SKL platform
/*!
This class defines the Vdenc command construction functions for Gen9 SKL platform
*/
class MhwVdboxVdencInterfaceG9Skl : public MhwVdboxVdencInterfaceG9<mhw_vdbox_vdenc_g9_skl>
{
public:
    //!
    //! \brief    Constructor
    //!
    MhwVdboxVdencInterfaceG9Skl(PMOS_INTERFACE osInterface) : MhwVdboxVdencInterfaceG9(osInterface)
    {
        MHW_FUNCTION_ENTER;
    }

    //!
    //! \brief    Destructor
    //!
    virtual ~MhwVdboxVdencInterfaceG9Skl() { }

    MOS_STATUS AddVdencSrcSurfaceStateCmd(
        PMOS_COMMAND_BUFFER                  cmdBuffer,
        PMHW_VDBOX_SURFACE_PARAMS            params);

    MOS_STATUS AddVdencImgStateCmd(
        PMOS_COMMAND_BUFFER                  cmdBuffer,
        PMHW_BATCH_BUFFER                    batchBuffer,
        PMHW_VDBOX_AVC_IMG_PARAMS            params);

    MOS_STATUS AddVdencWalkerStateCmd(
        PMOS_COMMAND_BUFFER                  cmdBuffer,
        PMHW_VDBOX_VDENC_WALKER_STATE_PARAMS params);

    MOS_STATUS AddVdencAvcWeightsOffsetsStateCmd(
        PMOS_COMMAND_BUFFER                  cmdBuffer,
        PMHW_VDBOX_AVC_WEIGHTOFFSET_PARAMS   params)
    {
        MOS_UNUSED(cmdBuffer);
        MOS_UNUSED(params);
        return MOS_STATUS_PLATFORM_NOT_SUPPORTED;
    }

};

#endif