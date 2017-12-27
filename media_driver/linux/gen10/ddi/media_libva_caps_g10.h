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
//! \file     media_libva_caps_g10.h
//! \brief    This file defines the C++ class/interface for gen10 media capbilities. 
//!

#ifndef __MEDIA_LIBVA_CAPS_G10_H__
#define __MEDIA_LIBVA_CAPS_G10_H__

#include "media_libva_caps.h"

//!
//! \class  MediaLibvaCapsG10
//! \brief  Media libva caps Gen10
//!
class MediaLibvaCapsG10 : public MediaLibvaCaps
{
public:
    //!
    //! \brief    Constructor
    //!
    MediaLibvaCapsG10(DDI_MEDIA_CONTEXT *mediaCtx) : MediaLibvaCaps(mediaCtx)
    {
        return;
    }

    virtual VAStatus Init()
    {
        LoadProfileEntrypoints();
        return VA_STATUS_SUCCESS;
    }   
protected:
    static const uint32_t m_maxHevcEncWidth =
        CODEC_8K_MAX_PIC_WIDTH; //!< maxinum width for HEVC encode
    static const uint32_t m_maxHevcEncHeight =
        CODEC_8K_MAX_PIC_HEIGHT; //!< maxinum height for HEVC encode

    virtual VAStatus GetPlatformSpecificAttrib(VAProfile profile,
            VAEntrypoint entrypoint,
            VAConfigAttribType type,
            unsigned int *value);

    virtual VAStatus LoadProfileEntrypoints();
    virtual VAStatus LoadVp9EncProfileEntrypoints();
    virtual VAStatus CheckEncodeResolution(
            VAProfile profile,
            uint32_t width,
            uint32_t height);
    virtual VAStatus CheckDecodeResolution(
            int32_t codecMode,
            VAProfile profile,
            uint32_t width,
            uint32_t height);

    //!
    //! \brief    Initialize HEVC low-power encode profiles, entrypoints and attributes
    //!
    //! \return VAStatus
    //!     Return VA_STATUS_SUCCESS if call success, else fail reason
    //!
    VAStatus LoadHevcEncLpProfileEntrypoints();

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
    //! \param  [in] maxNum
    //!     Maximum number
    //! \param  [in] isRoiInDeltaQP
    //!     Is ROI in delta QP
    //! 
    //! \return VAStatus
    //!     Return VA_STATUS_SUCCESS if call success, else fail reason
    //!
    VAStatus QueryAVCROIMaxNum(uint32_t rcMode, int32_t *maxNum, bool *isRoiInDeltaQP);
};
#endif
