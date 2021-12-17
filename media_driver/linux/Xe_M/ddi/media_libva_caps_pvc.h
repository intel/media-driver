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
//! \file     media_libva_caps_pvc.h
//! \brief    This file implements the C++ class/interface for pvc capbilities.
//!

#ifndef __MEDIA_LIBVA_CAPS_XE_XPM_PLUS_H__
#define __MEDIA_LIBVA_CAPS_XE_XPM_PLUS_H__

#include "media_libva.h"
#include "media_libva_caps_factory.h"

#include "media_libva_caps_g12.h"
#include "linux_system_info.h"

//!
//! \class  MediaLibvaCapsPVC
//! \brief  Media libva caps PVC
//!
class MediaLibvaCapsPVC : public MediaLibvaCapsG12
{
public:
    //!
    //! \brief    Constructor
    //!
    MediaLibvaCapsPVC(DDI_MEDIA_CONTEXT *mediaCtx) : MediaLibvaCapsG12(mediaCtx)
    {
        return;
    }

    //!
    //! \brief    Destructor
    //!
    virtual ~MediaLibvaCapsPVC() {};

    virtual VAStatus LoadAv1DecProfileEntrypoints();

    //!
    //! \brief        Initialize HEVC low-power encode profiles, entrypoints and attributes
    //!
    //! \return VAStatus
    //!     if call succeeds
    //!
    virtual VAStatus LoadHevcEncLpProfileEntrypoints() override;

    //!
    //! \brief    Check if the resolution is valid for a encode profile
    //!
    //! \param    [in] profile
    //!           Specify the VAProfile
    //!
    //! \param    [in] width
    //!           Specify the width for checking
    //!
    //! \param    [in] height
    //!           Specify the height for checking
    //!
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if the resolution is supported
    //!           VA_STATUS_ERROR_RESOLUTION_NOT_SUPPORTED if the resolution isn't valid
    //!
    virtual VAStatus CheckEncodeResolution(
        VAProfile profile,
        uint32_t width,
        uint32_t height) override;

    //!
    //! \brief    Add surface attributes for Encoding
    //!
    //! \param    [in] profile
    //!           VAProfile of the configuration
    //!
    //! \param    [in] entrypoint
    //!           VAEntrypoint of the configuration
    //!
    //! \param    [in,out] attribList
    //!           Pointer to VASurfaceAttrib array. It returns
    //!           the supported  surface attributes
    //!
    //! \param    [in,out] numAttribs
    //!           The number of elements allocated on input
    //!           Return the number of elements actually filled in output
    //!
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if success
    //!
    virtual VAStatus AddEncSurfaceAttributes(
        VAProfile profile,
        VAEntrypoint entrypoint,
        VASurfaceAttrib  *attribList,
        uint32_t &numAttribs) override;

    //!
    //! \brief    Return the platform specific value by given attribute type
    //!
    //! \param    [in] profile
    //!           VAProfile
    //!
    //! \param    [in] entrypoint
    //!           VAEntrypoint
    //!
    //! \param    [in] type
    //!           VAConfigAttribType
    //!
    //! \param    [in,out] value
    //!           Pointer to uint32_t that stores the returned value.
    //!
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if success
    //!
    virtual VAStatus GetPlatformSpecificAttrib(
        VAProfile profile,
        VAEntrypoint entrypoint,
        VAConfigAttribType type,
        unsigned int *value) override;

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
            VADisplayAttribute *attribList,
            int32_t numAttribs) override;
};
#endif
