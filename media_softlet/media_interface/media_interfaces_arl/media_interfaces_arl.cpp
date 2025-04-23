/*===================== begin_copyright_notice ==================================

# Copyright (c) 2023, Intel Corporation

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
//! \file     media_interfaces_arl.cpp

//! \brief    Helps with ARL factory creation.
//!

#include "media_interfaces_arl.h"

#include "codechal_common.h"
#include "codechal_debug.h"

#include "vp_pipeline_adapter_xe_lpm_plus.h"
#include "vp_platform_interface_xe_lpm_plus.h"
#include "mhw_mi_xe_lpm_plus_base_next_impl.h"
#include "mhw_blt_xe_lpm_plus_base_next_impl.h"
#include "mhw_sfc_xe_lpm_plus_base_next_impl.h"
#include "mhw_vebox_xe_lpm_plus_base_next_impl.h"
#include "mhw_render_xe_hpg_impl.h"

#if defined(ENABLE_KERNELS)
#include "igvpkrn_xe_hpg.h"
#include "igvpkrn_xe_hpg_cmfcpatch.h"
#if !defined(_FULL_OPEN_SOURCE)
#include "igvpkrn_isa_xe_hpg.h"
#endif
#endif

using namespace mhw::vdbox::avp::xe_lpm_plus_base;
using namespace mhw::vdbox::vdenc::xe_lpm_plus_base;
using namespace mhw::vdbox::huc::xe_lpm_plus_base;
using namespace mhw::vdbox::mfx::xe_lpm_plus_base;

extern template class MediaFactory<uint32_t, CodechalDeviceNext>;
extern template class MediaFactory<uint32_t, VphalDevice>;
extern template class MediaFactory<uint32_t, RenderHalDevice>;
extern template class MediaFactory<uint32_t, MediaInterfacesHwInfoDevice>;

// Swith to use new media factory template
extern template class MediaFactory<uint32_t, MhwInterfacesNext>;
extern template class MediaFactory<uint32_t, MmdDeviceNext>;

static bool arlRegisteredVphal =
    MediaFactory<uint32_t, VphalDevice>::
        Register<VphalInterfacesXe_Lpm_Plus>((uint32_t)IGFX_ARROWLAKE);

static bool arlRegisteredMhwNext =
    MediaFactory<uint32_t, MhwInterfacesNext>::
        Register<MhwInterfacesXe_Lpm_Plus_Next>((uint32_t)IGFX_ARROWLAKE);

#ifdef _MMC_SUPPORTED
static bool arlRegisteredMmd =
    MediaFactory<uint32_t, MmdDeviceNext>::
    Register<MmdDeviceXe_Lpm_Plus_Next>((uint32_t)IGFX_ARROWLAKE);
#endif

static bool arlRegisteredMcpy =
    MediaFactory<uint32_t, McpyDeviceNext>::
    Register<McpyDeviceXe_Lpm_Plus>((uint32_t)IGFX_ARROWLAKE);
    
static bool arlRegisteredCodecHal =
    MediaFactory<uint32_t, CodechalDeviceNext>::
        Register<CodechalInterfacesXe_Lpm_Plus>((uint32_t)IGFX_ARROWLAKE);

static bool arlRegisteredRenderHal =
    MediaFactory<uint32_t, RenderHalDevice>::
        Register<RenderHalInterfacesXe_Lpg>((uint32_t)IGFX_ARROWLAKE);

static bool arlRegisteredHwInfo =
    MediaFactory<uint32_t, MediaInterfacesHwInfoDevice>::
    Register<MediaInterfacesHwInfoDeviceXe_Lpm_Plus>((uint32_t)IGFX_ARROWLAKE);

