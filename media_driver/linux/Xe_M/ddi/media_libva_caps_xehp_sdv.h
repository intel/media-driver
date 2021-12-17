/*
* Copyright (c) 2021, Intel Corporation
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
//! \file     media_libva_caps_xehp_sdv.h
//! \brief    This file implements the C++ class/interface for Xe_XPM capbilities.
//!

#ifndef __MEDIA_LIBVA_CAPS_XEHP_SDV_H__
#define __MEDIA_LIBVA_CAPS_XEHP_SDV_H__

#include "media_libva.h"
#include "media_libva_caps_factory.h"

#include "media_libva_caps_g12.h"
#include "linux_system_info.h"

//!
//! \class  MediaLibvaCapsXeHP
//! \brief  Media libva caps XeHP
//!
class MediaLibvaCapsXeHP : public MediaLibvaCapsG12
{
public:
    //!
    //! \brief    Constructor
    //!
    MediaLibvaCapsXeHP(DDI_MEDIA_CONTEXT *mediaCtx) : MediaLibvaCapsG12(mediaCtx)
    {
        return;
    }

    //!
    //! \brief    Destructor
    //!
    virtual ~MediaLibvaCapsXeHP() {};

 protected:
    static const uint32_t m_hevcDPEncMaxHeight = 4320;
    virtual VAStatus GetPlatformSpecificAttrib(VAProfile profile,
                                               VAEntrypoint entrypoint,
                                               VAConfigAttribType type,
                                               unsigned int *value) override;

    virtual VAStatus CheckEncodeResolution(VAProfile profile, uint32_t width, uint32_t height) override;
};
#endif
