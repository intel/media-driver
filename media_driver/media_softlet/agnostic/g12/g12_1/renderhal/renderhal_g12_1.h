/*
* Copyright (c) 2011-2021, Intel Corporation
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
//! \file       renderhal_g12_1.h
//! \brief      header file of Gen12 DG1 hardware functions
//! \details    Gen12 hardware functions declare
//!
#ifndef __RENDERHAL_G12_1_H__
#define __RENDERHAL_G12_1_H__

#include "renderhal_g12_base.h"
#include "media_class_trace.h"
#include "mos_defs.h"
#include "renderhal.h"

//! \brief      for Gen12LP/DG1 VP and MDF
//!              SLM     URB     DC      RO      Rest/L3 Client Pool
//!               0    96(fixed) 0       0       2048 (KB chunks based on GT2)
#define RENDERHAL_L3_CACHE_CONFIG_CNTLREG_VALUE_G12LP_DG1_RENDERHAL (0x00000200)

class XRenderHal_Interface_G12_1 : public XRenderHal_Interface_G12_Base
{
public:
    XRenderHal_Interface_G12_1(): XRenderHal_Interface_G12_Base()
    {

    }
    
    virtual ~XRenderHal_Interface_G12_1() {}

    //!
    //! \brief    Enables L3 cacheing flag and sets related registers/values
    //! \param    PRENDERHAL_INTERFACE    pRenderHal
    //!           [in]  Pointer to Hardware Interface
    //! \param    pCacheSettings
    //!           [in] L3 Cache Configurations
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS EnableL3Caching(
        PRENDERHAL_INTERFACE         pRenderHal,
        PRENDERHAL_L3_CACHE_SETTINGS pCacheSettings);

    //! \brief      Set L3 cache override config parameters
    //! \param      [in] pRenderHal
    //!             Pointer to RenderHal Interface Structure
    //! \param      [in,out] pCacheSettings
    //!             Pointer to pCacheSettings
    //! \param      [in] bEnableSLM
    //!             Flag to enable SLM
    //! \return     MOS_STATUS
    //!             MOS_STATUS_SUCCESS if success. Error code otherwise
    //!
    virtual MOS_STATUS SetCacheOverrideParams(
        PRENDERHAL_INTERFACE            pRenderHal,
        PRENDERHAL_L3_CACHE_SETTINGS    pCacheSettings,
        bool                            bEnableSLM);
MEDIA_CLASS_DEFINE_END(XRenderHal_Interface_G12_1)
};

#endif // __RENDERHAL_G12_1_H__
