/*
* Copyright (c) 2012-2017, Intel Corporation
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
//! \file     vphal_render_sfc_g9_base.cpp
//! \brief    VPHAL SFC Gen9 rendering component
//! \details  The SFC renderer supports Scaling, IEF, CSC/ColorFill and Rotation.
//!           It's responsible for setting up HW states and generating the SFC
//!           commands.
//!
#include "vphal_render_vebox_base.h"
#include "vphal_render_sfc_g9_base.h"

#if __VPHAL_SFC_SUPPORTED

bool VphalSfcStateG9::IsInputFormatSupported(
    PVPHAL_SURFACE              srcSurface)
{
    bool ret = true;
    // Check if Input Format is supported
    if ((srcSurface->Format != Format_NV12)     &&
        (srcSurface->Format != Format_AYUV)     &&
        (srcSurface->Format != Format_P010)     &&
        (srcSurface->Format != Format_P016)     &&
        (srcSurface->Format != Format_A8B8G8R8) &&
        (srcSurface->Format != Format_X8B8G8R8) &&
        (srcSurface->Format != Format_A8R8G8B8) &&
        (srcSurface->Format != Format_X8R8G8B8) &&
        !IS_PA_FORMAT(srcSurface->Format))
    {
        VPHAL_RENDER_NORMALMESSAGE("Unsupported Source Format '0x%08x' for SFC.", srcSurface->Format);
        ret = false;
    }

    return ret;
}

bool VphalSfcStateG9::IsOutputFormatSupported(
    PVPHAL_SURFACE              outSurface)
{
    bool ret = true;
    if (!IS_RGB32_FORMAT(outSurface->Format)   &&
        // Remove RGB565 support due to quality issue, may reopen this after root cause in the future.
        //!IS_RGB16_FORMAT(outSurface->Format)   &&
        outSurface->Format != Format_NV12      &&
        outSurface->Format != Format_YUY2      &&
        outSurface->Format != Format_UYVY      &&
        outSurface->Format != Format_AYUV)
    {
        VPHAL_RENDER_NORMALMESSAGE("Unsupported Render Target Format '0x%08x' for SFC Pipe.", outSurface->Format);

        ret = false;
    }

    return ret;
}

void VphalSfcStateG9::GetInputWidthHeightAlignUnit(
    MOS_FORMAT              inputFormat,
    MOS_FORMAT              outputFormat,
    uint16_t                &widthAlignUnit,
    uint16_t                &heightAlignUnit)
{
    MOS_UNUSED(inputFormat);

    widthAlignUnit  = 1;
    heightAlignUnit = 1;

    // Apply output alignment restriction to Region of the input frame.
    GetOutputWidthHeightAlignUnit(outputFormat, widthAlignUnit, heightAlignUnit);
}

#endif // __VPHAL_SFC_SUPPORTED