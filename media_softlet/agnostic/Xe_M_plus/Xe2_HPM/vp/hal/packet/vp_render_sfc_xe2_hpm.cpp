/*
* Copyright (c) 2020-2024, Intel Corporation
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
//! \file     vp_render_sfc_xe2_hpm.cpp
//! \brief    SFC rendering component
//! \details  The SFC renderer supports Scaling, IEF, CSC/ColorFill and Rotation.
//!           It's responsible for setting up HW states and generating the SFC
//!           commands.
//!

#include "vp_render_sfc_xe2_hpm.h"
#include "mos_defs.h"

using namespace vp;

SfcRenderXe2_Hpm::SfcRenderXe2_Hpm(
    VP_MHWINTERFACE &vpMhwinterface,
    PVpAllocator &allocator,
    bool disbaleSfcDithering):
    SfcRenderXe_Lpm_Plus_Base(vpMhwinterface, allocator, disbaleSfcDithering)
{
}

SfcRenderXe2_Hpm::~SfcRenderXe2_Hpm()
{
}

bool SfcRenderXe2_Hpm::IsVdboxSfcOutputFormatSupported(
    CODECHAL_STANDARD codecStandard,
    MOS_FORMAT        outputFormat,
    MOS_TILE_TYPE     tileType)
{
    VP_FUNC_CALL();

    if (tileType == MOS_TILE_LINEAR && (outputFormat == Format_NV12 || outputFormat == Format_P010) &&
        !MEDIA_IS_SKU(m_skuTable, FtrSFC420LinearOutputSupport))
    {
        VP_PUBLIC_ASSERTMESSAGE("Unsupported output format '0x%08x' on tile type '0x%08x'", outputFormat, tileType);
        return false;
    }

    if (IS_PL3_RGB_FORMAT(outputFormat) || (tileType == MOS_TILE_LINEAR && IS_RGB24_FORMAT(outputFormat)))
    {
        if (!MEDIA_IS_SKU(m_skuTable, FtrSFCRGBPRGB24OutputSupport))
        {
            VP_PUBLIC_ASSERTMESSAGE("Unsupported RGB output format '0x%08x' on tile type '0x%08x'", outputFormat, tileType);
            return false;
        }

        return true;
    }

    return SfcRenderXe_Lpm_Plus_Base::IsVdboxSfcOutputFormatSupported(codecStandard, outputFormat, MOS_TILE_Y);  //Support Linear Format on this platform and hardcode here to unblock next check
}
