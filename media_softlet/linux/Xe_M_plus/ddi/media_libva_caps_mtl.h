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
//! \file     media_libva_caps_mtl.h
//! \brief    This file implements the C++ class/interface for pvc capbilities.
//!

#ifndef __MEDIA_LIBVA_CAPS_MTL_H__
#define __MEDIA_LIBVA_CAPS_MTL_H__

#include "media_libva.h"
#include "media_libva_caps_factory.h"

#include "media_libva_caps_mtl_base.h"
#include "linux_system_info.h"

//!
//! \class  MediaLibvaCapsMtl
//! \brief  Media libva caps MTL
//!
class MediaLibvaCapsMtl : public MediaLibvaCapsMtlBase
{
public:
    //!
    //! \brief    Constructor
    //!
    MediaLibvaCapsMtl(DDI_MEDIA_CONTEXT *mediaCtx) : MediaLibvaCapsMtlBase(mediaCtx)
    {
        return;
    }

    //!
    //! \brief    Destructor
    //!
    virtual ~MediaLibvaCapsMtl() {};

    //!
    //! \brief    Get display attributes
    //!           returns the current attributes values in "attribList"
    //!
    //! \param    [in, out] attribList
    //!           the attrib type should be filled.
    //!           returns the supported display attributes
    //!
    //! \param    [in] numAttribs
    //!           the number of supported attributes
    //!
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if success
    //!           VA_STATUS_ERROR_MAX_NUM_EXCEEDED if size of attribList is too small
    //!
    virtual VAStatus GetDisplayAttributes(
        VADisplayAttribute * attribList,
        int32_t numAttribs) override;

    MEDIA_CLASS_DEFINE_END(MediaLibvaCapsMtl)
};
#endif
