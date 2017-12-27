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
//! \file     media_libva_caps_g9_bxt.h
//! \brief    This file defines the C++ class/interface for BXT media capbilities. 
//!

#ifndef __MEDIA_LIBVA_CAPS_G9_BXT_H__
#define __MEDIA_LIBVA_CAPS_G9_BXT_H__

#include "media_libva_caps_g9.h"

//!
//! \class  MediaLibvaCapsG9Bxt
//! \brief  Media libva caps Gen9 Bxt
//!
class MediaLibvaCapsG9Bxt : public MediaLibvaCapsG9
{
public:
    //!
    //! \brief    Constructor
    //!
    MediaLibvaCapsG9Bxt(DDI_MEDIA_CONTEXT *mediaCtx);

    //!
    //! \brief    Destructor
    //!
    ~MediaLibvaCapsG9Bxt() {};

protected:
    virtual VAStatus GetMbProcessingRateEnc(
            MEDIA_FEATURE_TABLE *skuTable,
            uint32_t tuIdx,
            uint32_t codecMode,
            bool vdencActive,
            uint32_t *mbProcessingRatePerSec);
};
#endif
