/*
* Copyright (c) 2020, Intel Corporation
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
//! \file       renderhal_g11_jsl_ehl.cpp
//! \brief      implementation of Gen11 JSL and EHL hardware functions
//! \details    Render functions
//!

#include "renderhal_g11_jsl_ehl.h"

#ifndef UNUSED
#define UNUSED(x) (void)(x)
#endif // UNUSED
//!
//! \brief    Get PlaneDefinition for Format_Y216
//! \param    [in] isRenderTarget
//!           The flag to indicate if the surface is rendertarget or not
//! \param    [in] pRenderHal
//!           Pointer to Hardware Interface
//! \param    RENDERHAL_PLANE_DEFINITION PlaneDefinition
//!           [in,out] Pointer to PlaneDefinition
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success, else fail reason
//!
MOS_STATUS XRenderHal_Interface_g11_JslEhl::GetPlaneDefForFormatY216(
    bool                       isRenderTarget,
    PRENDERHAL_INTERFACE       pRenderHal,
    RENDERHAL_PLANE_DEFINITION& PlaneDefinition)
{
    UNUSED(pRenderHal);

    if (isRenderTarget)
    {
        PlaneDefinition = RENDERHAL_PLANES_Y210_RT;
    }
    else
    {
        PlaneDefinition = RENDERHAL_PLANES_Y210;
    }

    return MOS_STATUS_SUCCESS;
};
