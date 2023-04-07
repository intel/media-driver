/*===================== begin_copyright_notice ==================================

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

======================= end_copyright_notice ==================================*/
//!
//! \file       media_render_copy_xe_lpm_plus_base.cpp
//! \brief      implementation of xe hardware functions
//! \details    Render functions
//!

#include "media_render_copy_xe_lpm_plus_base.h"
#include "hal_kerneldll_next.h"
#include "media_common_defs.h"
#include "media_copy.h"
#include "mhw_render_itf.h"
#include "mhw_state_heap.h"
#include "mos_defs_specific.h"
#include "mos_os.h"
#include "mos_resource_defs.h"
#include "mos_utilities.h"
#include "umKmInc/UmKmDmaPerfTimer.h"
#include "hal_oca_interface_next.h"
#include "media_render_common.h"
#include "vpkrnheader.h"
#if defined(ENABLE_KERNELS) && !defined(_FULL_OPEN_SOURCE)
#include "igvpkrn_xe_hpg.h"
#endif

class MhwInterfacesNext;
RenderCopyXe_LPM_Plus_Base::RenderCopyXe_LPM_Plus_Base(PMOS_INTERFACE  osInterface, MhwInterfacesNext *mhwInterfaces):
    RenderCopyStateNext(osInterface, mhwInterfaces)
{
#if defined(ENABLE_KERNELS) && !defined(_FULL_OPEN_SOURCE)
    m_KernelBin     = (const void*)IGVPKRN_XE_HPG;
    m_KernelBinSize = IGVPKRN_XE_HPG_SIZE;
#else
    m_KernelBin     = nullptr;
    m_KernelBinSize = 0;
#endif
}

RenderCopyXe_LPM_Plus_Base:: ~RenderCopyXe_LPM_Plus_Base()
{
}


