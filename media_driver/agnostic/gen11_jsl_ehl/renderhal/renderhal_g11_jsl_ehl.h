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
//! \file       renderhal_g11_jsl_ehl.h
//! \brief      header file of Gen11 JSL and EHL hardware functions
//! \details    Gen11 JSL and EHL hardware functions declare
//!
#ifndef __RENDERHAL_G11_JSL_EHL_H__
#define __RENDERHAL_G11_JSL_EHL_H__

#include "renderhal_g11.h"

class XRenderHal_Interface_g11_JslEhl : public XRenderHal_Interface_g11
{
public:
    XRenderHal_Interface_g11_JslEhl() {}
    virtual ~XRenderHal_Interface_g11_JslEhl() {}


    //!
    //! \brief    Get PlaneDefinition for Format_Y216
    //! \param    [in] isRenderTarget
    //!           The flag to indicate if the surface is rendertarget or not
    //! \param    [in] pRenderHal
    //!           Pointer to Hardware Interface
    //! \param    [in,out] PlaneDefinition
    //!           Pointer to PlaneDefinition
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS GetPlaneDefForFormatY216(
        bool                       isRenderTarget,
        PRENDERHAL_INTERFACE       pRenderHal,
        RENDERHAL_PLANE_DEFINITION& PlaneDefinition);
};

#endif // __RENDERHAL_G11_JSL_EHL_H__
