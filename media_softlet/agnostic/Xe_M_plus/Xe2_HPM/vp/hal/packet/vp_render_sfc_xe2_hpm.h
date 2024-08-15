/* Copyright (c) 2020-2024, Intel Corporation
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
//! \file     vp_render_sfc_xe2_hpm.h
//! \brief    The header file of the base class of SFC rendering component
//! \details  The SFC renderer supports Scaling, IEF, CSC/ColorFill and Rotation.
//!           It's responsible for setting up HW states and generating the SFC
//!           commands.
//!

#ifndef __VP_RENDER_SFC_XE2_HPM_H__
#define __VP_RENDER_SFC_XE2_HPM_H__

#include "vp_render_sfc_xe_lpm_plus_base.h"

namespace vp {

class SfcRenderXe2_Hpm : public SfcRenderXe_Lpm_Plus_Base
{

public:
    SfcRenderXe2_Hpm(VP_MHWINTERFACE &vpMhwinterface, PVpAllocator &allocator, bool disbaleSfcDithering);
    virtual ~SfcRenderXe2_Hpm();
    virtual bool IsVdboxSfcOutputFormatSupported(
        CODECHAL_STANDARD codecStandard,
        MOS_FORMAT        outputFormat,
        MOS_TILE_TYPE     tileType) override;

MEDIA_CLASS_DEFINE_END(vp__SfcRenderXe2_Hpm)
};

}
#endif // !__VP_RENDER_SFC_XE2_HPM_H__
