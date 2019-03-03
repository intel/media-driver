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
//! \file     media_libva_caps_g9.h
//! \brief    This file defines the C++ class/interface for gen9 media capbilities. 
//!

#ifndef __MEDIA_LIBVA_CAPS_G9_H__
#define __MEDIA_LIBVA_CAPS_G9_H__

#include "media_libva_caps.h"

//!
//! \class  MediaLibvaCapsG9
//! \brief  Media libva caps Gen9
//!
class MediaLibvaCapsG9 : public MediaLibvaCaps
{
public:
    //!
    //! \brief    Constructor
    //!
    MediaLibvaCapsG9(DDI_MEDIA_CONTEXT *mediaCtx) : MediaLibvaCaps(mediaCtx)
    {
        return;
    }

    virtual VAStatus Init()
    {
        LoadProfileEntrypoints();
        return VA_STATUS_SUCCESS;
    }
protected:
    virtual VAStatus GetPlatformSpecificAttrib(VAProfile profile,
            VAEntrypoint entrypoint,
            VAConfigAttribType type,
            unsigned int *value);

    //! 
    //! \brief  Load profile entry points
    //! 
    //! \return VAStatus
    //!     Return VA_STATUS_SUCCESS if call success, else fail reason
    //!
    VAStatus LoadProfileEntrypoints();

    //! 
    //! \brief  Is P010 supported
    //! 
    //! \return true
    //!
    bool IsP010Supported() { return true; }

    //! 
    //! \brief  Query AVC ROI maximum number
    //! 
    //! \param  [in] rcMode
    //!     RC mode
    //! \param  [in] isVdenc
    //!     vdenc
    //! \param  [in] maxNum
    //!     Maximum number
    //! \param  [in] isRoiInDeltaQP
    //!     Is ROI in delta QP
    //! 
    //! \return VAStatus
    //!     Return VA_STATUS_SUCCESS if call success, else fail reason
    //!
    VAStatus QueryAVCROIMaxNum(uint32_t rcMode, bool isVdenc, uint32_t *maxNum, bool *isRoiInDeltaQP);
};
#endif
