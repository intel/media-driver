/*
* Copyright (c) 2017, Intel Corporation
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
//! \file      renderhal_g8_bdw.h 
//! \brief     BDW specific functions 
//!
//!
//! \file       renderhal_g8_bdw.h
//! \brief      header file of BDW hardware functions
//! \details    BDW hardware functions declare
//!
#ifndef __RENDERHAL_G8_BDW_H__
#define __RENDERHAL_G8_BDW_H__

#include "renderhal_g8.h"

//!
//! \brief      L3 cache CNTLREG value with SLM enabled on BDW GT2
//!             SLM     URB    Rest   DC       RO     I/S     C     T      Sum
//!             { 192,  128,   0,     256,     128,   0,      0,    0       }
#define RENDERHAL_L3_CACHE_CNTL_REG_VALUE_SLM_ENABLE_G8_BDW         (0x00808021)

class XRenderHal_Interface_g8_bdw : public XRenderHal_Interface_g8
{
public:
    XRenderHal_Interface_g8_bdw() {};
    ~XRenderHal_Interface_g8_bdw() {};

protected:
    virtual uint32_t GetL3CacheCntlRegWithSLM() {return RENDERHAL_L3_CACHE_CNTL_REG_VALUE_SLM_ENABLE_G8_BDW; }
};

#endif // __RENDERHAL_G8_BDW_H__
