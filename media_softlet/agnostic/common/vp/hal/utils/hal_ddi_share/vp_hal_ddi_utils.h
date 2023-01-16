/*
* Copyright (c) 2022-2023, Intel Corporation
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
#ifndef __VP_HAL_DDI_UTILS_H__
#define __VP_HAL_DDI_UTILS_H__

#include "vp_common.h"

class VpHalDDIUtils
{
public:
    //!
    //! \brief  Convert Vphal Surface to Mos Surface
    //! \param  [in]  surface
    //!         pointer to VPHAL SURFACE
    //! \return MOS_SURFACE
    //!
    static MOS_SURFACE ConvertVphalSurfaceToMosSurface(
        PVPHAL_SURFACE surface);
    
    //!
    //! \brief    Get the color pack type of a surface
    //! \details  Map mos surface format to color pack format and return.
    //!           For unknown format return VPHAL_COLORPACK_UNKNOWN
    //! \param    [in] format
    //!           MOS_FORMAT of a surface
    //! \return   VPHAL_COLORPACK
    //!           Color pack type of the surface
    //!
    static VPHAL_COLORPACK GetSurfaceColorPack(
        MOS_FORMAT format);

    //! \brief    Get the bit depth of a surface
    //! \details  Get bit depth of input mos surface format and return.
    //!           For unknown format return 0
    //! \param    [in] format
    //!           MOS_FORMAT of a surface
    //! \return   uint32_t
    //!           Bit depth of the surface
    //!
    static uint32_t GetSurfaceBitDepth(
        MOS_FORMAT format);

MEDIA_CLASS_DEFINE_END(VpHalDDIUtils)
};

#endif // __VP_HAL_DDI_UTILS_H__
