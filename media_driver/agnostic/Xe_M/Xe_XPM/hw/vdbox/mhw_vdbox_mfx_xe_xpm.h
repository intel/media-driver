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
//! \file     mhw_vdbox_mfx_xe_xpm.h
//! \brief    Defines functions for constructing Vdbox MFX commands on Xe_XPM platforms
//!

#ifndef __MHW_VDBOX_MFX_XE_XPM_H__
#define __MHW_VDBOX_MFX_XE_XPM_H__

#include "mhw_vdbox_mfx_g12_X.h"

//!  MHW Vdbox MFX interface for Xe_XPM
/*!
This class defines the HCP command interface for Xe_XPM platform
*/
class MhwVdboxMfxInterfaceXe_Xpm : public MhwVdboxMfxInterfaceG12
{
public:
    MhwVdboxMfxInterfaceXe_Xpm(
        PMOS_INTERFACE  osInterface,
        MhwMiInterface *miInterface,
        MhwCpInterface *cpInterface,
        bool            decodeInUse);
    ~MhwVdboxMfxInterfaceXe_Xpm();

protected:

   MOS_STATUS AddMfxSurfaceCmd(
        PMOS_COMMAND_BUFFER       cmdBuffer,
        PMHW_VDBOX_SURFACE_PARAMS params) override;

   MOS_STATUS AddMfxPipeModeSelectCmd(
       PMOS_COMMAND_BUFFER cmdBuffer,
       PMHW_VDBOX_PIPE_MODE_SELECT_PARAMS params) override;

   MOS_STATUS AddMfxPipeBufAddrCmd(
        PMOS_COMMAND_BUFFER             cmdBuffer,
        PMHW_VDBOX_PIPE_BUF_ADDR_PARAMS params) override;

   MOS_STATUS AddMfxEncodeAvcSlice(
        PMOS_COMMAND_BUFFER             cmdBuffer,
        PMHW_BATCH_BUFFER               batchBuffer,
        PMHW_VDBOX_AVC_SLICE_STATE      avcSliceState) override;
   
   uint32_t GetScaledReferenceSurfaceCachePolicy() override
   {
       return m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_SURFACE_HME_DOWNSAMPLED_ENCODE_FF].Value >> 1;
   }
};

#endif
