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
//! \file      mhw_state_heap_xe_xpm.h
//! \brief         This modules implements HW interface layer to be used on all platforms on
//!            all operating systems/DDIs, across MHW components.
//!

#ifndef __mhw_state_heap_xe_xpm_H__
#define __mhw_state_heap_xe_xpm_H__

#include "mhw_state_heap_g12.h"
#include "mhw_state_heap_hwcmd_xe_xpm.h"

struct MHW_STATE_HEAP_INTERFACE_XE_XPM : public MHW_STATE_HEAP_INTERFACE_G12_X
{
public:
    MHW_STATE_HEAP_INTERFACE_XE_XPM(PMOS_INTERFACE pInputOSInterface, int8_t bDynamicMode);

    ~MHW_STATE_HEAP_INTERFACE_XE_XPM();

    MOS_STATUS SetSamplerState(
      void                        *pSampler,
      PMHW_SAMPLER_STATE_PARAM    pParam);

    MOS_STATUS AddSamplerStateData(
        uint32_t                    samplerOffset,
        MemoryBlock                 *memoryBlock,
        PMHW_SAMPLER_STATE_PARAM    pParam);

    MOS_STATUS SetSurfaceStateEntry(
        PMHW_SURFACE_STATE_PARAMS   pParams);

    MOS_STATUS SetMissingShaderChannels(
        mhw_state_heap_xe_xpm::RENDER_SURFACE_STATE_CMD *pSurfaceState,
        uint32_t dwFormat);

    MOS_STATUS SetInterfaceDescriptor(
        uint32_t                         dwNumIdsToSet,
        PMHW_INTERFACE_DESCRIPTOR_PARAMS pParams);
};
#endif
