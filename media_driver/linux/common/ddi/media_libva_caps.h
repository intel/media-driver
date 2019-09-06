/*
* Copyright (c) 2017-2018, Intel Corporation
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
//! \file     media_libva_caps.h
//! \brief    This file defines the base C++ class/interface for media capbilities. 
//!

#ifndef __MEDIA_LIBVA_CAPS_H__
#define __MEDIA_LIBVA_CAPS_H__

#include "va/va.h"

#include <vector>
#include <map>

struct DDI_MEDIA_CONTEXT;
class MediaLibvaCapsCpInterface;

typedef std::map<VAConfigAttribType, uint32_t> AttribMap;

//!
//! \class  MediaLibvaCaps
//! \brief  Media libva caps
//!
class MediaLibvaCaps
{
protected:
    MediaLibvaCaps(DDI_MEDIA_CONTEXT *mediaCtx);
public:

    //!
    //! \brief    Destructor
    //!
    virtual ~MediaLibvaCaps();

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
            VAProfile profile,
            VAEntrypoint entrypoint,
            VAConfigAttrib *attribList,
            int32_t numAttribs);

    //!
    //! \brief    Check a profile valid or not
    //!
    //! \param    [in] profile 
    //!           VA profile
    //! 
    //! \return   VAStatus 
    //!           VA_STATUS_SUCCESS if success
    //!
    VAStatus CheckProfile(VAProfile profile);
    //!
    //!
    //! \brief    Create a configuration for the encode/decode/vp pipeline 
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
            VAProfile profile,
            VAEntrypoint entrypoint,
            VAConfigAttrib *attribList,
            int32_t numAttribs,
            VAConfigID *configId);

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
            int32_t *numProfiles);

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
            VAProfile profile,
            VAEntrypoint *entrypointList,
            int32_t *numEntryPoints);

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
            VAConfigID configId,
            VAProfile *profile,
            VAEntrypoint *entrypoint,
            VAConfigAttrib *attribList,
            int32_t *numAttribs);

    //!
    //! \brief    Get attributes for a given encode config ID 
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
    //! \param    [in,out] rcMode 
    //!           Return the rcMode for the config ID. 
    //!
    //! \param    [in,out] feiFunction
    //!           Return the fei function type for the config ID.
    //!
    //! \return   VAStatus 
    //!           VA_STATUS_SUCCESS if success
    //!
    VAStatus GetEncConfigAttr(
            VAConfigID configId,
            VAProfile *profile,
            VAEntrypoint *entrypoint,
            uint32_t *rcMode,
            uint32_t *feiFunction);

    //!
    //! \brief    Get attributes for a given decode config ID 
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
    //! \param    [in,out] slicemode 
    //!           Return the slice mode for the config ID. 
    //!
    //! \param    [in,out] encrypttype 
    //!           Return the encryption type for the config ID. 
    //!
    //! \param    [in,out] processmode
    //!           Return the process mode for the config ID. 
    //!
    //! \return   VAStatus 
    //!           VA_STATUS_SUCCESS if success
    //!
    VAStatus GetDecConfigAttr(
            VAConfigID configId,
            VAProfile *profile,
            VAEntrypoint *entrypoint,
            uint32_t *slicemode,
            uint32_t *encrypttype,
            uint32_t *processmode);

    //!
    //! \brief    Get attributes for a given Vp config ID 
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
    //! \return   VAStatus 
    //!           VA_STATUS_SUCCESS if success
    //!
    VAStatus GetVpConfigAttr(
            VAConfigID configId,
            VAProfile *profile,
            VAEntrypoint *entrypoint);

    //!
    //! \brief    Get process rate for a given config ID 
    //!
    //! \param    [in] config_id 
    //!           VA configuration
    //!
    //! \param    [in,out] procBuf 
    //!           Pointer to VAProcessingRateParameter 
    //!
    //! \param    [in,out] processingRate 
    //!           Return the process rate 
    //!
    //! \return   VAStatus 
    //!           VA_STATUS_SUCCESS if success
    //!
    VAStatus QueryProcessingRate(
            VAConfigID config_id,
            VAProcessingRateParameter *procBuf,
            uint32_t *processingRate);

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
    virtual VAStatus QuerySurfaceAttributes(
            VAConfigID configId,
            VASurfaceAttrib *attribList,
            uint32_t *numAttribs);

    //!
    //! \brief    Check if the resolution is valid for a given decode codec mode 
    //!
    //! \param    [in] codecMode 
    //!           Specify the codec mode 
    //!
    //! \param    [in] profile 
    //!           VA profile
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
    virtual VAStatus CheckDecodeResolution(
            int32_t codecMode,
            VAProfile profile,
            uint32_t width,
            uint32_t height);

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
            uint32_t height);

    //!
    //! \brief    Check if the give profile is VC1 
    //!
    //! \param    [in] profile 
    //!           Specify the VAProfile 
    //!
    //! \return   True if the profile is a VC1 profile
    //!           False if the profile isn't a VC1 profile
    //!
    static bool IsVc1Profile(VAProfile profile);

    //!
    //! \brief    Check if the give profile is MPEG2 
    //!
    //! \param    [in] profile 
    //!           Specify the VAProfile 
    //!
    //! \return   True if the profile is a MPEG2 profile
    //!           False if the profile isn't a MPEG2 profile
    //!
    static bool IsMpeg2Profile(VAProfile profile);

    //!
    //! \brief    Check if the give profile is AVC 
    //!
    //! \param    [in] profile 
    //!           Specify the VAProfile 
    //!
    //! \return   True if the profile is a AVC profile
    //!           False if the profile isn't a AVC profile
    //!
    static bool IsAvcProfile(VAProfile profile);

    //!
    //! \brief    Check if the give profile is HEVC 
    //!
    //! \param    [in] profile 
    //!           Specify the VAProfile 
    //!
    //! \return   True if the profile is a HEVC profile
    //!           False if the profile isn't a HEVC profile
    //!
    virtual bool IsHevcProfile(VAProfile profile);

    //!
    //! \brief    Check if the give profile is VP8 
    //!
    //! \param    [in] profile 
    //!           Specify the VAProfile 
    //!
    //! \return   true if the profile is a VP8 profile
    //!           false if the profile isn't a VP8 profile
    //!
    static bool IsVp8Profile(VAProfile profile);

    //!
    //! \brief    Check if the give profile is VP9 
    //!
    //! \param    [in] profile 
    //!           Specify the VAProfile 
    //!
    //! \return   True if the profile is a VP9 profile
    //!           False if the profile isn't a VP9 profile
    //!
    static bool IsVp9Profile(VAProfile profile);

    //!
    //! \brief    Check if the give profile is JPEG 
    //!
    //! \param    [in] profile 
    //!           Specify the VAProfile 
    //!
    //! \return   True if the profile is a JPEG profile
    //!           False if the profile isn't a JPEG profile
    //!
    static bool IsJpegProfile(VAProfile profile);

    //!
    //! \brief    Check if current FeiFuncton or give entrypoint is FEI
    //!
    //! \param    [in] entrypoint 
    //!           Specify the VAEntrypoint for checking 
    //!
    //! \param    [in] feiFunction
    //!           Specify the VA_FEI_FUNCTION for checking
    //!
    //! \return   True if the entrypoint or the feiFuncton belong to FEI
    //!           False if the entrypoint and the FeiFuncton aren't FEI
    //!
    bool IsEncFei(VAEntrypoint entrypoint, uint32_t feiFunction);

    //! 
    //! \brief    Return the CODECHAL_FUNCTION type for give profile and entrypoint 
    //!
    //! \param    [in] profile 
    //!           Specify the VAProfile 
    //!
    //! \param    [in] entrypoint 
    //!           Specify the VAEntrypoint 
    //!
    //! \param    [in] feiFunction
    //!           Specify the VA_FEI_FUNCTION
    //!
    //! \return   Codehal function
    //!
    CODECHAL_FUNCTION GetEncodeCodecFunction(VAProfile profile, VAEntrypoint entrypoint, uint32_t feiFunction);

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
    virtual CODECHAL_MODE GetEncodeCodecMode(VAProfile profile, VAEntrypoint entrypoint);

    //!
    //! \brief    Return internal decode mode for given profile 
    //!
    //! \param    [in] profile 
    //!           Specify the VAProfile 
    //!
    //! \return   Codehal mode: decode codec mode 
    //!
    virtual CODECHAL_MODE GetDecodeCodecMode(VAProfile profile);

    //!
    //! \brief    Return the decode codec key for given profile 
    //!
    //! \param    [in] profile 
    //!           Specify the VAProfile 
    //!
    //! \return   Std::string decode codec key 
    //!
    virtual std::string GetDecodeCodecKey(VAProfile profile);

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
    virtual std::string GetEncodeCodecKey(VAProfile profile, VAEntrypoint entrypoint, uint32_t feiFunction);

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
    virtual VAStatus QueryImageFormats(VAImageFormat *formatList, int32_t *num_formats) = 0;

    //!
    //! \brief    Return the maxinum number of supported image formats 
    //!
    //! \return   The maxinum number of supported image formats 
    //!
    virtual uint32_t GetImageFormatsMaxNum() = 0;

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
    virtual VAStatus PopulateColorMaskInfo(VAImageFormat *vaImgFmt) = 0;

    virtual bool IsImageSupported(uint32_t fourcc) = 0;

    //!
    //! \brief    Query AVC ROI maxinum numbers and if support ROI in delta QP 
    //!
    //! \param    [in] rcMode 
    //!           Specify the rate control mode to query 
    //!
    //! \param    [in] isVdenc
    //!           Specify whether it is vdenc or not
    //!
    //! \param    [in,out] maxNum 
    //!           Pointer to a integer that will return the maximum number of ROI.
    //!
    //! \param    [in,out] isRoiInDeltaQP 
    //!           Pointer to a bool that will return if ROI in delta QP is supported 
    //!
    //! \return   VAStatus 
    //!           VA_STATUS_SUCCESS if succeed 
    //!
    virtual VAStatus QueryAVCROIMaxNum(uint32_t rcMode, bool isVdenc, uint32_t *maxNum, bool *isRoiInDeltaQP) = 0;

    //!
    //! \brief    Check if the configID is a valid decode config 
    //!
    //! \param    [in] configId 
    //!           Specify the VAConfigID 
    //!
    //! \return   True if the configID is a valid decode config, otherwise false 
    //!
    bool IsDecConfigId(VAConfigID configId);

    //!
    //! \brief    Check if the configID is a valid encode config 
    //!
    //! \param    [in] configId 
    //!           Specify the VAConfigID 
    //!
    //! \return   True if the configID is a valid encode config, otherwise false 
    //!
    bool IsEncConfigId(VAConfigID configId);

    //!
    //! \brief    Check if the configID is a valid vp config 
    //!
    //! \param    [in] configId 
    //!           Specify the VAConfigID 
    //!
    //! \return   True if the configID is a valid vp config, otherwise false 
    //!
    bool IsVpConfigId(VAConfigID configId);

    //!
    //! \brief    Check if the entrypoint is supported by MFE
    //!
    //! \param    [in] entrypoint
    //!           Specify the VAEntrypoint
    //!
    //! \return   true if supported, otherwise false
    //!
    bool IsMfeSupportedEntrypoint(VAEntrypoint entrypoint);

    //!
    //! \brief    Check if the profile is supported by MFE
    //!
    //! \param    [in] profile
    //!           Specify the VAProfile
    //!
    //! \return   true if supported, otherwise false
    //!
    bool IsMfeSupportedProfile(VAProfile profile);

    //!
    //! \brief    Destory the VAConfigID 
    //!
    //! \param    [in] configId 
    //!           Specify the VAConfigID 
    //!
    //! \return   VAStatus
    //!       VA_STATUS_SUCCESS if succeed 
    //!           VA_STATUS_ERROR_INVALID_CONFIG if the conifgId is invalid
    //!
    VAStatus DestroyConfig(VAConfigID configId);

    //!
    //! \brief    Create MediaLibvaCaps instance for current platform 
    //!
    //! \param    [in] mediaCtx 
    //!           Pointer to DDI_MEDIA_CONTEXT
    //!
    //! \return   MediaLibvaCaps * 
    //!           Pointer to Gen specific MediaLibvaCaps if success, otherwise return nullptr
    //!
    static MediaLibvaCaps * CreateMediaLibvaCaps(DDI_MEDIA_CONTEXT *mediaCtx);

    //!
    //! \brief convert Media Format to Gmm Format for GmmResCreate parameter.
    //!
    //! \param    [in] format
    //!         Pointer to DDI_MEDIA_FORMAT
    //!
    //! \return GMM_RESOURCE_FORMAT
    //!         Pointer to gmm format type
    //!
    virtual GMM_RESOURCE_FORMAT ConvertMediaFmtToGmmFmt(DDI_MEDIA_FORMAT format);

    //!
    //! \brief convert FOURCC to Gmm Format.
    //!
    //! \param    [in] fourcc
    //!
    //! \return GMM_RESOURCE_FORMAT
    //!         Pointer to gmm format type
    //!
    virtual GMM_RESOURCE_FORMAT ConvertFourccToGmmFmt(uint32_t fourcc);

    //!
    //! \brief    Check if MFE is supported on the platform
    //!
    //! \param    [in] PLATFORM
    //!
    //! \return   true if supported, otherwise false
    //!
    virtual bool IsMfeSupportedOnPlatform(const PLATFORM &platform);

    //!
    //! \brief    Initialize the MediaLibvaCaps instance for current platform 
    //!
    //! \return   VAStatus 
    //!           return VA_STATUS_SUCCESS for success
    //!
    virtual VAStatus Init()
    {
        // do nothing by default
        return VA_STATUS_SUCCESS;
    }

protected:
    //!
    //! \class    ProfileEntrypoint
    //! \brief    Profile entrypoint
    //!
    class ProfileEntrypoint
    {
        public:
            VAProfile m_profile = VAProfileNone; //!< Profile
            VAEntrypoint m_entrypoint = (VAEntrypoint)0; //!< Entrypoint
            AttribMap *m_attributes = nullptr; //!< Pointer to attributes map
            int32_t m_configStartIdx = 0; //!< Config Id offset to the decode or encode or vp config Id base
            //! \brief  The number of config Id that this profile & entrypoint combination supports
            //!
            int32_t m_configNum = 0; //!< Number of configs that above profile & entrypoint combination supports
    };

    //!
    //! \struct   DecConfig
    //! \brief    Decode configuration
    //!
    struct DecConfig
    {
        uint32_t m_sliceMode;   //!< Decode slice mode
        uint32_t m_encryptType; //!< Decode entrypoint Type
        uint32_t m_processType; //!< Decode processing Type

        DecConfig(const uint32_t sliceMode, const uint32_t encryptType, const uint32_t processType)
        : m_sliceMode(sliceMode), m_encryptType(encryptType), m_processType(processType) {}
    };

    //!
    //! \struct   EncConfig
    //! \brief    Encode configuration
    //!
    struct EncConfig
    {
        uint32_t m_rcMode;      //!< RateControl Mode
        uint32_t m_FeiFunction; //!< Decode entrypoint Type
        EncConfig(const uint32_t rcMode, const uint32_t FeiFunction)
        : m_rcMode(rcMode), m_FeiFunction(FeiFunction) {}
    };

    //!
    //! \enum     CodecType
    //! \brief    Codec type
    //!
    enum CodecType
    {
        videoEncode, //!< Video encode
        videoDecode, //!< Video decode
        videoProcess //!< Video processing
    };

    enum EncodeFormat
    {
        AVC = 0,
        HEVC,
        VP9,
        Others = 0xff,
    };

    enum EncodeType
    {
        DualPipe = 0,
        Vdenc,
    };

    struct EncodeFormatTable
    {
        EncodeFormat    encodeFormat;
        EncodeType      encodeType;
        uint32_t        colorFormat;
    };

    static const uint16_t m_maxProfiles = 17; //!< Maximum number of supported profiles
    static const uint16_t m_maxProfileEntries = 64; //!< Maximum number of supported profile & entrypoint combinations
    static const uint32_t m_numVpSurfaceAttr = 15; //!< Number of VP surface attributes
    static const uint32_t m_numJpegSurfaceAttr = 7; //!< Number of JPEG surface attributes
    static const uint32_t m_numJpegEncSurfaceAttr = 4; //!< Number of JPEG encode surface attributes
    static const uint16_t m_maxEntrypoints = 7; //!<  Maximum number of supported entrypoints
    static const uint32_t m_decSliceMode[2]; //!< Store 2 decode slices modes
    static const uint32_t m_decProcessMode[2]; //!< Store 2 decode process modes
    static const uint32_t m_encRcMode[9]; //!< Store 9 encode rate control modes
    static const uint32_t m_vpSurfaceAttr[m_numVpSurfaceAttr]; //!< Store the VP surface attributes
    static const uint32_t m_jpegSurfaceAttr[m_numJpegSurfaceAttr]; //!< Store the JPEG surface attributes
    static const uint32_t m_jpegEncSurfaceAttr[m_numJpegEncSurfaceAttr]; //!< Store the JPEG encode surface attributes

    static const uint32_t m_decMpeg2MaxWidth = 2048; //!< Maximum width for Mpeg2 decode
    static const uint32_t m_decMpeg2MaxHeight = 2048; //!< Maximum height for Mpeg2 decode
    static const uint32_t m_decVc1MaxWidth = 3840; //!< Maximum width for VC1 decode
    static const uint32_t m_decVc1MaxHeight = 3840; //!< Maximum height for VC1 decode
    static const uint32_t m_decJpegMaxWidth = 16384;  //!< Maximum width for JPEG decode
    static const uint32_t m_decJpegMaxHeight = 16384; //!< Maximum height for JPEG decode
    static const uint32_t m_decHevcMaxWidth = 8192; //!< Maximum width for HEVC decode
    static const uint32_t m_decHevcMaxHeight = 8192; //!< Maximum height for HEVC decode
    static const uint32_t m_decVp9MaxWidth = 8192; //!< Maximum width for VP9 decode
    static const uint32_t m_decVp9MaxHeight = 8192; //!< Maximum height for VP9 decode
    static const uint32_t m_decDefaultMaxWidth = 4096; //!< Default maximum width for decode
    static const uint32_t m_decDefaultMaxHeight = 4096; //!< Default maximum height for decode

    static const uint32_t m_encMinWidth = 32; //!< Minimum width for encoding
    static const uint32_t m_encMinHeight = 32; //!< Minimum height for encoding
    static const uint32_t m_encMax4kWidth =
        CODEC_4K_MAX_PIC_WIDTH; //!< Minimum width for encoding
    static const uint32_t m_encMax4kHeight =
        CODEC_4K_MAX_PIC_HEIGHT; //!< Minimum height for encoding
    static const uint32_t m_encJpegMinWidth = 16; //!< Minimum width for encoding
    static const uint32_t m_encJpegMinHeight = 16; //!< Minimum height for encoding
    static const uint32_t m_encJpegMaxWidth =
        ENCODE_JPEG_MAX_PIC_WIDTH; //!< Maximum width for JPEG encoding
    static const uint32_t m_encJpegMaxHeight =
        ENCODE_JPEG_MAX_PIC_HEIGHT; //!< Maximum height for JPEG encoding
    DDI_MEDIA_CONTEXT *m_mediaCtx; //!< Pointer to media context

    MediaLibvaCapsCpInterface* m_CapsCp;

    //!
    //! \brief  Store all the supported encode format
    //!
    struct EncodeFormatTable* m_encodeFormatTable = nullptr;
    uint32_t m_encodeFormatCount = 0;

    //!
    //! \brief  Store all the profile and entrypoint combinations 
    //!
    ProfileEntrypoint m_profileEntryTbl[m_maxProfileEntries];
    uint16_t m_profileEntryCount = 0; //!< Count valid entries in m_profileEntryTbl

    //!
    //! \brief  Store attribute list pointers 
    //!
    std::vector<AttribMap *> m_attributeLists;

    bool m_isEntryptSupported = false; //!< If decode encryption is supported on current platform

    std::vector<EncConfig> m_encConfigs; //!< Store supported encode configs
    std::vector<DecConfig> m_decConfigs; //!< Store supported decode configs
    std::vector<uint32_t> m_vpConfigs;   //!< Store supported vp configs

    //!
    //! \brief    Check entrypoint codec type
    //!
    //! \param    [in] entrypoint
    //!       VA entrypoint
    //! \param    [in] codecType
    //!       Codec type
    //!
    //! \return   True if entrypoint match the codecType 
    //!
    bool CheckEntrypointCodecType(VAEntrypoint entrypoint, CodecType codecType);

    //!
    //! \brief    Add one decode configuration
    //!
    //! \param    [in] slicemode
    //!           VA_DEC_SLICE_MODE_xxx 
    //!
    //! \param    [in] encryptType 
    //!           Encryption Type 
    //!
    //! \param    [in]  processType 
    //!           VA_DEC_PROCESSINGxxx 
    //!
    //! \return   VAStatus 
    //!           VA_STATUS_SUCCESS if success
    //!
    VAStatus AddDecConfig(uint32_t slicemode, uint32_t encryptType, uint32_t processType);

    //!
    //! \brief    Add one encode configuration
    //!
    //! \param    [in] rcMode 
    //!           VA_RC_XXX 
    //! \param    [in] feiFunction
    //!           VA_FEI_FUNCTION_XXX [optional parameter]
    //!
    //! \return   VAStatus 
    //!           VA_STATUS_SUCCESS if success
    //!
    VAStatus AddEncConfig(uint32_t rcMode, uint32_t feiFunction = 0);

    //!
    //! \brief    Add one vp configuration
    //!
    //! \param    [in] attrib
    //!           VP attribute 
    //!
    //! \return   VAStatus 
    //!           VA_STATUS_SUCCESS if success
    //!
    VAStatus AddVpConfig(uint32_t attrib);

    //!
    //! \brief    Return profile and entrypoint for a give config ID 
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
    //! \param    [in,out] profileTableIdx 
    //!           The index in m_profileEntryTbl. Return -1 if config ID is invalid
    //!
    //! \return   VAStatus 
    //!           VA_STATUS_SUCCESS if success
    //!
    VAStatus GetProfileEntrypointFromConfigId(VAConfigID configId,
            VAProfile *profile,
            VAEntrypoint *entrypoint,
            int32_t *profileTableIdx);

    //!
    //! \brief    Add one entry to profile & entrypoint table
    //!
    //! \param    [in] profile 
    //!           Pointer to VAProfile of the configuration
    //!
    //! \param    [in] entrypoint 
    //!           Pointer to VAEntrypoint of the configuration
    //!
    //! \param    [in] attributeList
    //!           Pointer to VAConfigAttrib vector that stores attributes
    //!
    //! \param    [in] configIdxStart 
    //!           Offset of config index in m_encConfigs, m_decConfigs or m_vpConfigs
    //!
    //! \param    [in] configNum 
    //!           The number of supported configs. 
    //!
    //! \return   VAStatus 
    //!           VA_STATUS_SUCCESS if success
    //!
    VAStatus AddProfileEntry(VAProfile profile,
            VAEntrypoint entrypoint,
            AttribMap *attributeList,
            int32_t configIdxStart,
            int32_t configNum);

    //!
    //! \brief    Return the index in m_profileEntryTble by given profile and entrypoint 
    //!
    //! \param    [in] profile 
    //!           Specify VAProfile
    //!
    //! \param    [in] entrypoint 
    //!           Specify VAEntrypoint 
    //!
    //! \return   int32_t 
    //!           Equal or bigger than zero if success, otherwise return -1
    //!
    int32_t GetProfileTableIdx(VAProfile profile, VAEntrypoint entrypoint);

    //!
    //! \brief    Create attributes map 
    //!
    //! \param    [in,out] attributeList 
    //!           Return the pointer to AttribMap 
    //!
    //! \return   VAStatus 
    //!           VA_STATUS_SUCCESS if success
    //!
    VAStatus CreateAttributeList(AttribMap **attributeList);

    //!
    //! \brief    Free attribuate lists 
    //!
    VAStatus FreeAttributeList();

    //!
    //! \brief    Initialize the attribute types of a VAConfigAttrib array 
    //!
    //! \param    [in,out] attribList 
    //!           Pointer to VAConfigAttrib vector 
    //!
    //! \return   VAStatus 
    //!           VA_STATUS_SUCCESS if success
    //!
    VAStatus InitAttributeTypes(std::vector<VAConfigAttrib> *attribList);

    //!
    //! \brief    Return index of given attribute type in a VAConfigAttrib vector
    //!
    //! \param    [in] attribList 
    //!           Pointer to VAConfigAttrib vector. 
    //!
    //! \param    [in] type 
    //!           Specify the VAConfigAttribType to query 
    //!
    //! \return   int32_t
    //!           Equal or bigger than zero if success, otherwise return -1
    //!
    int32_t GetAttributeIndex(std::vector<VAConfigAttrib> *attribList, VAConfigAttribType type);

    //!
    //! \brief    Set the attribute in a VAConfigAttrib array 
    //!
    //! \param    [in,out] attributeList 
    //!           Pointer to VAConfigAttrib vector 
    //!
    //! \param    [in] type 
    //!           VAConfigAttribType 
    //!
    //! \param    [in] value 
    //!           Attribute value
    //!
    //! \return   VAStatus 
    //!           VA_STATUS_SUCCESS if success
    //!
    VAStatus SetAttribute(
            std::vector<VAConfigAttrib> *attributeList,
            VAConfigAttribType type,
            uint32_t value);

    //!
    //! \brief    Set the attribute for the given profile and entrypoint
    //!
    //! \param    [in] profile
    //!           Specify VAProfile
    //!
    //! \param    [in] entrypoint
    //!           Specify VAEntrypoint
    //!
    //! \param    [in] type
    //!           VAConfigAttribType
    //!
    //! \param    [in] value
    //!           Attribute value
    //!
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if success
    //!
    VAStatus SetAttribute(
            VAProfile profile,
            VAEntrypoint entrypoint,
            VAConfigAttribType type,
            uint32_t value);

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
    VAStatus CreateEncAttributes(
            VAProfile profile,
            VAEntrypoint entrypoint,
            AttribMap **attributeList);

    //!
    //! \brief    Create and intialize an attribute array give decode profile and entrypoint 
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
    virtual VAStatus CreateDecAttributes(
            VAProfile profile,
            VAEntrypoint entrypoint,
            AttribMap **attributeList);

    //!
    //! \brief    Initialize AVC decode profiles, entrypoints and attributes
    //!
    VAStatus LoadAvcDecProfileEntrypoints();

    //!
    //! \brief    Initialize AVC encode profiles, entrypoints and attributes
    //!
    virtual VAStatus LoadAvcEncProfileEntrypoints();

    //!
    //! \brief    Initialize AVC Low-power encode profiles, entrypoints and attributes
    //!
    VAStatus LoadAvcEncLpProfileEntrypoints();

    //!
    //! \brief    Initialize MPEG2 decode profiles, entrypoints and attributes
    //!
    VAStatus LoadMpeg2DecProfileEntrypoints();

    //!
    //! \brief    Initialize MPEG2 encode profiles, entrypoints and attributes
    //!
    VAStatus LoadMpeg2EncProfileEntrypoints();

    //!
    //! \brief    Initialize JPEG decode profiles, entrypoints and attributes
    //!
    VAStatus LoadJpegDecProfileEntrypoints();

    //!
    //! \brief    Initialize JPEG encode profiles, entrypoints and attributes
    //!
    VAStatus LoadJpegEncProfileEntrypoints();

    //!
    //! \brief    Initialize VC1 decode profiles, entrypoints and attributes
    //!
    VAStatus LoadVc1DecProfileEntrypoints();

    //!
    //! \brief    Initialize VP8 decode profiles, entrypoints and attributes
    //!
    VAStatus LoadVp8DecProfileEntrypoints();

    //!
    //! \brief    Initialize VP8 encode profiles, entrypoints and attributes
    //!
    VAStatus LoadVp8EncProfileEntrypoints();

    //!
    //! \brief    Initialize VP9 decode profiles, entrypoints and attributes
    //!
    VAStatus LoadVp9DecProfileEntrypoints();

    //!
    //! \brief    Initialize VP9 encode profiles, entrypoints and attributes
    //!
    virtual VAStatus LoadVp9EncProfileEntrypoints();

    //!
    //! \brief    Initialize HEVC decode profiles, entrypoints and attributes
    //!
    virtual VAStatus LoadHevcDecProfileEntrypoints();

    //!
    //! \brief    Initialize HEVC decode profiles, entrypoints and attributes for specified hevc profile
    //!
    VAStatus LoadDecProfileEntrypoints(VAProfile profile);

    //!
    //! \brief    Initialize HEVC encode profiles, entrypoints and attributes
    //!
    virtual VAStatus LoadHevcEncProfileEntrypoints();

    //!
    //! \brief    Initialize none profiles, entrypoints and attributes
    //!
    VAStatus LoadNoneProfileEntrypoints();

    //!
    //! \brief    Initialize Advanced decode profiles, entrypoints and attributes
    //!
    virtual VAStatus LoadAdvancedDecProfileEntrypoints();

    //!
    //! \brief    Initialize encode/decode/vp profiles, entrypoints and attributes
    //!
    virtual VAStatus LoadProfileEntrypoints() = 0;

    //!
    //! \brief    Create decode config by given attributes
    //!
    //! \param    [in] profileTableIdx 
    //!           The index in m_profileEntryTbl. 
    //!
    //! \param    [in] attribList 
    //!           Pointer to VAConfigAttrib array 
    //!
    //! \param    [in] numAttribs 
    //!           Number of VAConfigAttrib in attribList 
    //!
    //! \param    [in,out] configId 
    //!           Pointer to VAConfigID. 
    //!
    //! \return   VAStatus 
    //!           VA_STATUS_SUCCESS if success
    //!
    VAStatus CreateDecConfig(
        int32_t profileTableIdx,
        VAConfigAttrib *attribList,
        int32_t numAttribs,
        VAConfigID *configId);

    //!
    //! \brief    Create encode config by given attributes
    //!
    //! \param    [in] profileTableIdx 
    //!           The index in m_profileEntryTbl. 
    //!
    //! \param    [in] attribList 
    //!           Pointer to VAConfigAttrib array 
    //!
    //! \param    [in] numAttribs 
    //!           Number of VAConfigAttrib in attribList 
    //!
    //! \param    [in,out] configId 
    //!           Pointer to VAConfigID. 
    //!
    //! \return   VAStatus 
    //!           VA_STATUS_SUCCESS if success
    //!
    VAStatus CreateEncConfig(
        int32_t profileTableIdx,
        VAEntrypoint entrypoint,
        VAConfigAttrib *attribList,
        int32_t numAttribs,
        VAConfigID *configId);

    //!
    //! \brief    Create vp config by given attributes
    //!
    //! \param    [in] profileTableIdx 
    //!           The index in m_profileEntryTbl. 
    //!
    //! \param    [in] attribList 
    //!           Pointer to VAConfigAttrib array 
    //!
    //! \param    [in] numAttribs 
    //!           Number of VAConfigAttrib in attribList 
    //!
    //! \param    [in,out] configId 
    //!           Pointer to VAConfigID. 
    //!
    //! \return   VAStatus 
    //!           VA_STATUS_SUCCESS if success
    //!
    VAStatus CreateVpConfig(
        int32_t profileTableIdx,
        VAConfigAttrib *attribList,
        int32_t numAttribs,
        VAConfigID *configId);

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
            uint32_t *value) = 0;

    //!
    //! \brief    Return encode Mb processing rate on current platform
    //!
    //! \param    [in] skuTable 
    //!           Point to MEDIA_FEATURE_TABLE 
    //!
    //! \param    [in] tuIdx 
    //!           Specify the index of target usage 
    //!
    //! \param    [in] codecMode 
    //!           Specify the codec mode 
    //!
    //! \param    [in] vdencActive 
    //!           Specify if vdenc is used 
    //!
    //! \param    [in,out] mbProcessingRatePerSec 
    //!           Pointer to uint32_t that stores the returned value.
    //!
    //! \return   VAStatus 
    //!           VA_STATUS_SUCCESS if success
    //!
    virtual VAStatus GetMbProcessingRateEnc(
            MEDIA_FEATURE_TABLE *skuTable,
            uint32_t tuIdx,
            uint32_t codecMode,
            bool vdencActive,
            uint32_t *mbProcessingRatePerSec);

    //!
    //! \brief    Return decode Mb processing rate on current platform 
    //!
    //! \param    [in] skuTable 
    //!           Point to MEDIA_FEATURE_TABLE 
    //!
    //! \param    [in,out] mbProcessingRatePerSec 
    //!           Pointer to uint32_t that stores the returned value.
    //!
    //! \return   VAStatus 
    //!           VA_STATUS_SUCCESS if success
    //!
    virtual VAStatus GetMbProcessingRateDec(
            MEDIA_FEATURE_TABLE *skuTable,
            uint32_t *mbProcessingRatePerSec);

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
    VAStatus CheckEncRTFormat(
            VAProfile profile,
            VAEntrypoint entrypoint,
            VAConfigAttrib* attrib);
};
#endif
