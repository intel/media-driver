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
//! \file     media_libva_caps_dg2.h
//! \brief    This file implements the C++ class/interface for dg2 capbilities.
//!

#ifndef __MEDIA_LIBVA_CAPS_XE_HPM_H__
#define __MEDIA_LIBVA_CAPS_XE_HPM_H__

#include "media_libva.h"
#include "media_libva_caps_factory.h"

#include "media_libva_caps_g12.h"
#include "linux_system_info.h"

//!
//! \class  MediaLibvaCapsDG2
//! \brief  Media libva caps DG2
//!
class MediaLibvaCapsDG2 : public MediaLibvaCapsG12
{
public:
    //!
    //! \brief    Constructor
    //!
    MediaLibvaCapsDG2(DDI_MEDIA_CONTEXT *mediaCtx) : MediaLibvaCapsG12(mediaCtx)
    {
        // DG2 supported Encode format
        static struct EncodeFormatTable encodeFormatTableDG2[] =
        {
            {AVC, Vdenc, VA_RT_FORMAT_YUV420 | VA_RT_FORMAT_YUV422 | VA_RT_FORMAT_YUV444 | VA_RT_FORMAT_RGB32},
            {HEVC, Vdenc, VA_RT_FORMAT_YUV420 | VA_RT_FORMAT_YUV420_10BPP |
             VA_RT_FORMAT_YUV444 | VA_RT_FORMAT_YUV444_10 | VA_RT_FORMAT_RGB32 |
             VA_RT_FORMAT_RGB32_10BPP | VA_RT_FORMAT_YUV422 | VA_RT_FORMAT_YUV422_10},
            {VP9, Vdenc, VA_RT_FORMAT_YUV420 | VA_RT_FORMAT_YUV420_10BPP |
             VA_RT_FORMAT_YUV444 | VA_RT_FORMAT_YUV444_10 | VA_RT_FORMAT_RGB32 |
             VA_RT_FORMAT_RGB32_10BPP},
            {AV1, Vdenc, VA_RT_FORMAT_YUV420 | VA_RT_FORMAT_YUV420_10BPP |
             VA_RT_FORMAT_RGB32 | VA_RT_FORMAT_RGB32_10BPP}
        };
        m_encodeFormatTable = (struct EncodeFormatTable*)(&encodeFormatTableDG2[0]);
        m_encodeFormatCount = sizeof(encodeFormatTableDG2)/sizeof(struct EncodeFormatTable);
        return;
    }

    //!
    //! \brief    Destructor
    //!
    virtual ~MediaLibvaCapsDG2() {};

    //!
    //! \brief    Return the encode codec key for given profile and entrypoint
    //!
    //! \param    [in] profile
    //!           Specify the VAProfile
    //!
    //! \param    [in] entrypoint
    //!           Specify the entrypoint
    //!
    //! \param    [in] feiFunction
    //!           Specify the feiFunction
    //!
    //! \return   Std::string encode codec key
    //!
    std::string GetEncodeCodecKey(VAProfile profile, VAEntrypoint entrypoint, uint32_t feiFunction) override;

    //!
    //! \brief    Return internal encode mode for given profile and entrypoint
    //!
    //! \param    [in] profile
    //!           Specify the VAProfile
    //!
    //! \param    [in] entrypoint
    //!           Specify the VAEntrypoint
    //!
    //! \return   Codehal mode
    //!
    CODECHAL_MODE GetEncodeCodecMode(VAProfile profile, VAEntrypoint entrypoint) override;

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
    //! \brief    Check the encode RT format according to platform and encode format
    //!
    //! \param    [in] profile
    //!           VAProfile
    //!
    //! \param    [in] entrypoint
    //!           VAEntrypoint
    //!
    //! \param    [in,out] attrib
    //!           Pointer to a pointer of VAConfigAttrib that will be created
    //!
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if success
    //!
    virtual VAStatus CheckEncRTFormat(
        VAProfile profile,
        VAEntrypoint entrypoint,
        VAConfigAttrib* attrib) override;

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
    virtual VAStatus GetPlatformSpecificAttrib(VAProfile profile,
        VAEntrypoint entrypoint,
        VAConfigAttribType type,
        unsigned int *value) override;

    //!
    //! \brief    Create and intialize an attribute vector give encode profile and entrypoint
    //!
    //! \param    [in] profile
    //!           VA profile
    //!
    //! \param    [in] entrypoint
    //!           VA entrypoint
    //!
    //! \param    [in,out] attributeList
    //!           Pointer to a pointer of AttribMap that will be created
    //!
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if success
    //!
    virtual VAStatus CreateEncAttributes(
        VAProfile profile,
        VAEntrypoint entrypoint,
        AttribMap **attributeList) override;

    virtual VAStatus LoadProfileEntrypoints() override;

    //!
    //! \brief    Initialize av1 encode profiles, entrypoints and attributes
    //!
    virtual VAStatus LoadAv1EncProfileEntrypoints();
    //!
    //! \brief    Initialize AVC Low-power encode profiles, entrypoints and attributes
    //!
    virtual VAStatus LoadAvcEncLpProfileEntrypoints() override;

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
    virtual VAStatus AddEncSurfaceAttributes(
        VAProfile profile,
        VAEntrypoint entrypoint,
        VASurfaceAttrib  *attribList,
        uint32_t &numAttribs) override;

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
};
#endif
