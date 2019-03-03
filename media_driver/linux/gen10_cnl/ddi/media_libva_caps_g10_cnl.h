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
//! \file     media_libva_caps_g10_cnl.h
//! \brief    This file defines the C++ class/interface for gen10 media capbilities. 
//!

#ifndef __MEDIA_LIBVA_CAPS_G10_CNL_H__
#define __MEDIA_LIBVA_CAPS_G10_CNL_H__

#include "media_libva_caps_g10.h"

//!
//! \class  MediaLibvaCapsG10Cnl
//! \brief  Media libva caps Gen10 Cnl
//!
class MediaLibvaCapsG10Cnl : public MediaLibvaCapsG10
{
public:
    //!
    //! \brief    Constructor
    //!
    MediaLibvaCapsG10Cnl(DDI_MEDIA_CONTEXT *mediaCtx);

    //!
    //! \brief    Destructor
    //!
    ~MediaLibvaCapsG10Cnl() {};
protected:
    virtual VAStatus GetMbProcessingRateEnc(
        MEDIA_FEATURE_TABLE *skuTable,
        uint32_t tuIdx,
        uint32_t codecMode,
        bool vdencActive,
        uint32_t *mbProcessingRatePerSec);
};
#endif
