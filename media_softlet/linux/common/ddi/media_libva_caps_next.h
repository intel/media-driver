/*
* Copyright (c) 2021-2022, Intel Corporation
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
//! \file     media_libva_caps_next.h
//! \brief    Header file of media caps next class on linux
//!

#ifndef __MEDIA_LIBVA_CAPS_NEXT_H__
#define __MEDIA_LIBVA_CAPS_NEXT_H__

#include "media_capstable_specific.h"
#include "media_libva_common_next.h"

class MediaLibvaCapsNext
{
public:
    MediaLibvaCapsNext(DDI_MEDIA_CONTEXT *mediaCtx);

    ~MediaLibvaCapsNext();

    //!
    //! \brief    Create CapsNext
    //!
    //! \return   MediaLibvaCapsNext*
    //!           
    static MediaLibvaCapsNext* CreateCaps(DDI_MEDIA_CONTEXT *mediaCtx);

    //!
    //! \brief    Init MediaLibvaCapsNext
    //!
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if success
    //!
    VAStatus Init();

    //!
    //! \brief    Get configlist for create configs
    //!
    //! \return   All supported ConfigLinuxList
    //!
    ConfigList* GetConfigList();

    //!
    //! \brief    Get Attrib Value
    //!
    //! \param    [in] profile
    //!           VA profile
    //!
    //! \param    [in] entrypoint
    //!           VA entrypoint
    //!
    //! \param    [in] type
    //!           VA ConfigAttribType
    //!
    //! \param    [out] value
    //!           Pointer to returned AttribType
    //!
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if success
    //!
    VAStatus GetAttribValue(
        VAProfile           profile,
        VAEntrypoint        entrypoint,
        VAConfigAttribType  type,
        uint32_t            *value);

    //!
    //! \brief    Return the maxinum number of supported image formats for current platform ipVersion
    //!
    //! \return   The maxinum number of supported image formats for current platform ipVersion
    //!
    uint32_t GetImageFormatsMaxNum();

    //!
    //! \brief    Query supported profiles
    //!
    //! \param    [in] profileList
    //!           Pointer to VAProfile array that can hold at least vaMaxNumProfile() entries
    //!
    //! \param    [out] numProfiles
    //!           Pointer to int32_t. It returns the actual number of supported profiles.
    //!
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if success
    //!
    VAStatus QueryConfigProfiles(
        VAProfile *profileList,
        int32_t   *profilesNum);

    //!
    //! \brief    Query all attributes for a given configuration
    //!
    //! \param    [in] configId
    //!           VA configuration
    //!
    //! \param    [in,out] profile
    //!           Pointer to VAProfile of the configuration
    //!
    //! \param    [in,out] entrypoint
    //!           Pointer to VAEntrypoint of the configuration
    //!
    //! \param    [in,out] attribList
    //!           Pointer to VAConfigAttrib array that can hold at least
    //!           vaMaxNumConfigAttributes() entries.
    //!
    //! \param    [in,out] numAttribs
    //!           The actual number of VAConfigAttrib returned in the array attribList
    //!
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if success
    //!
    VAStatus QueryConfigAttributes(
        VAConfigID     configId,
        VAProfile      *profile,
        VAEntrypoint   *entrypoint,
        VAConfigAttrib *attribList,
        int32_t        *numAttribs);

    //!
    //!
    //! \brief    Create a configuration
    //! \details  It passes in the attribute list that specifies the attributes it
    //!           cares about, with the rest taking default values.
    //!
    //! \param    [in] profile
    //!           VA profile
    //!
    //! \param    [in] entrypoint
    //!           VA entrypoint
    //!
    //! \param    [in] attribList
    //!           Pointer to VAConfigAttrib array that specifies the attributes
    //!
    //! \param    [in] numAttribs
    //!           Number of VAConfigAttrib in the array attribList
    //!
    //! \param    [out] configId
    //!           Pointer to returned VAConfigID if success
    //!
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if success
    //!
    VAStatus CreateConfig(
        VAProfile       profile,
        VAEntrypoint    entrypoint,
        VAConfigAttrib  *attribList,
        int32_t         numAttribs,
        VAConfigID      *configId);

    //!
    //! \brief    Destory the VAConfigID
    //!
    //! \param    [in] configId
    //!           Specify the VAConfigID
    //!
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if succeed
    //!           VA_STATUS_ERROR_INVALID_CONFIG if the conifgId is invalid
    //!
    VAStatus DestroyConfig(VAConfigID configId);

    //!
    //! \brief    Get attributes for a given profile/entrypoint pair
    //! \details  The caller must provide an "attribList" with all attributes to be
    //!           retrieved.  Upon return, the attributes in "attribList" have been
    //!           updated with their value.  Unknown attributes or attributes that are
    //!           not supported for the given profile/entrypoint pair will have their
    //!           value set to VA_ATTRIB_NOT_SUPPORTED.
    //!
    //! \param    [in] profile
    //!           VA profile
    //!
    //! \param    [in] entrypoint
    //!           VA entrypoint
    //!
    //! \param    [in,out] attribList
    //!           Pointer to VAConfigAttrib array. The attribute type is set by caller and
    //!           attribute value is set by this function.
    //!
    //! \param    [in] numAttribs
    //!           Number of VAConfigAttrib in the array attribList
    //!
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if success
    //!
    VAStatus GetConfigAttributes(
        VAProfile       profile,
        VAEntrypoint    entrypoint,
        VAConfigAttrib  *attribList,
        int32_t         numAttribs);

    //! \brief Get the general attribute
    //!
    //! \param    [in,out] attrib
    //!           Pointer to the CAConfigAttrib
    //!
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if success
    //!
    VAStatus GetGeneralConfigAttrib(VAConfigAttrib *attrib);

    //!
    //! \brief    Query supported entrypoints for a given profile
    //!
    //! \param    [in] profile
    //!           VA profile
    //!
    //! \param    [in] entrypointList
    //!           Pointer to VAEntrypoint array that can hold at least vaMaxNumEntrypoints() entries
    //!
    //! \param    [out] numEntryPoints
    //!           It returns the actual number of supported VAEntrypoints.
    //!
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if success
    //!
    VAStatus QueryConfigEntrypoints(
        VAProfile     profile,
        VAEntrypoint  *entrypointList,
        int32_t       *entrypointNum);

    //!
    //! \brief    Get surface attributes for a given config ID
    //!
    //! \param    [in] configId
    //!           VA configuration
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
    //!           VA_STATUS_ERROR_MAX_NUM_EXCEEDED if size of attribList is too small
    //!
    VAStatus QuerySurfaceAttributes(
        VAConfigID       configId,
        VASurfaceAttrib  *attribList,
        uint32_t         *numAttribs);

    //!
    //! \brief    Query the suppported image formats
    //!
    //! \param    [in,out] formatList
    //!           Pointer to a VAImageFormat array. The array size shouldn't be less than vaMaxNumImageFormats
    //!           It will return the supported image formats.
    //!
    //! \param    [in,out] num_formats
    //!           Pointer to a integer that will return the real size of formatList.
    //!
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if succeed
    //!
    VAStatus QueryImageFormats(
        VAImageFormat *formatList,
        int32_t       *numFormats);

    //!
    //! \brief    Populate the color masks info
    //!
    //! \param    [in,out] Image format
    //!           Pointer to a VAImageFormat array. Color masks information will be populated to this
    //!           structure.
    //!
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if succeed
    //!
    VAStatus PopulateColorMaskInfo(VAImageFormat *vaImgFmt);

    //!
    //! \brief  Query display attributes
    //!
    //! \param  [in] attrList
    //!         VA display attribute
    //! \param  [in] attributesNum
    //!         Number of attributes
    //!
    //! \return VAStatus
    //!     VA_STATUS_SUCCESS if success, else fail reason
    //!
    VAStatus QueryDisplayAttributes(
        VADisplayAttribute *attrList,
        int32_t            *attributesNum);

    //!
    //! \brief  Get display attributes
    //! \details    This function returns the current attribute values in "attr_list".
    //!         Only attributes returned with VA_DISPLAY_ATTRIB_GETTABLE set in the "flags" field
    //!         from vaQueryDisplayAttributes() can have their values retrieved.
    //!
    //! \param  [in] attrList
    //!         VA display attribute
    //! \param  [in] attributesNum
    //!         Number of attributes
    //!
    //! \return VAStatus
    //!     VA_STATUS_ERROR_UNIMPLEMENTED
    //!
    VAStatus GetDisplayAttributes(
        VADisplayAttribute *attrList,
        int32_t             attributesNum);

    MediaCapsTableSpecific *m_capsTable = nullptr;

protected:
    DDI_MEDIA_CONTEXT      *m_mediaCtx  = nullptr;

    //!
    //! \brief    Check attrib when create a configuration
    //!
    //! \param    [in] profile
    //!           VA profile
    //!
    //! \param    [in] entrypoint
    //!           VA entrypoint
    //!
    //! \param    [in] attrib
    //!           Pointer to VAConfigAttrib array that specifies the attributes
    //!
    //! \param    [in] numAttribs
    //!           Number of VAConfigAttrib in the array attribList
    //!
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if success
    //!
    VAStatus CheckAttribList(
        VAProfile       profile,
        VAEntrypoint    entrypoint,
        VAConfigAttrib  *attrib,
        int32_t         numAttribs);
MEDIA_CLASS_DEFINE_END(MediaLibvaCapsNext)
};

#endif //__MEDIA_LIBVA_CAPS_NEXT_H__
