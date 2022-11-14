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
//! \file     media_libva_caps_mtl_base.h
//! \brief    This file defines the C++ class/interface for MTL media capbilities.
//!

#ifndef __MEDIA_LIBVA_CAPS_MTL_BASE_H__
#define __MEDIA_LIBVA_CAPS_MTL_BASE_H__

#include "media_libva_caps.h"
#include "media_libva_util.h"

//!
//! \class  MediaLibvaCapsMtlBase
//! \brief  Media libva caps mtl
//!
class MediaLibvaCapsMtlBase : public MediaLibvaCaps
{
public:
    //!
    //! \brief    Constructor
    //!
    MediaLibvaCapsMtlBase(DDI_MEDIA_CONTEXT *mediaCtx) : MediaLibvaCaps(mediaCtx)
    {
        // MTL supported Encode format
        static struct EncodeFormatTable encodeFormatTableMTL[] =
        {
            {AVC, Vdenc, VA_RT_FORMAT_YUV420 | VA_RT_FORMAT_YUV422 | VA_RT_FORMAT_YUV444 | VA_RT_FORMAT_RGB32},
            {HEVC, Vdenc, VA_RT_FORMAT_YUV420 | VA_RT_FORMAT_YUV420_10BPP |
             VA_RT_FORMAT_YUV444 | VA_RT_FORMAT_YUV444_10 | VA_RT_FORMAT_RGB32 |
             VA_RT_FORMAT_RGB32_10BPP},
            {VP9, Vdenc, VA_RT_FORMAT_YUV420 | VA_RT_FORMAT_YUV420_10BPP |
             VA_RT_FORMAT_YUV444 | VA_RT_FORMAT_YUV444_10 | VA_RT_FORMAT_RGB32 |
             VA_RT_FORMAT_RGB32_10BPP}
        };
        m_encodeFormatTable = (struct EncodeFormatTable*)(&encodeFormatTableMTL[0]);
        m_encodeFormatCount = sizeof(encodeFormatTableMTL)/sizeof(struct EncodeFormatTable);

        return;
    }

    //!
    //! \brief    Destructor
    //!
    virtual ~MediaLibvaCapsMtlBase() {};

    //
    virtual VAStatus Init() override
    {
        return LoadProfileEntrypoints();
    }

    //!
    //! \brief    Initialize AVC Low-power encode profiles, entrypoints and attributes
    //!
    virtual VAStatus LoadAvcEncLpProfileEntrypoints() override;

    //!
    //! \brief        Initialize HEVC encode profiles, entrypoints and attributes
    //!
    //! \return VAStatus
    //!     if call succeeds
    //!
    virtual VAStatus LoadHevcEncProfileEntrypoints() override;

    //!
    //! \brief        Initialize AVC encode profiles, entrypoints and attributes
    //!
    //! \return VAStatus
    //!     if call succeeds
    //!
    virtual VAStatus LoadAvcEncProfileEntrypoints() override;

    //!
    //! \brief    Initialize AV1 encode profiles, entrypoints and attributes
    //!
    //! \return   VAStatus
    //!           if call succeeds
    //!
    virtual VAStatus LoadAv1EncProfileEntrypoints();

    virtual CODECHAL_MODE GetDecodeCodecMode(VAProfile profile)                  override;

    //!
    //! \brief    Return the decode codec key for given profile
    //!
    //! \param    [in] profile
    //!           Specify the VAProfile
    //!
    //! \return   Std::string decode codec key
    //!
    virtual std::string GetDecodeCodecKey(VAProfile profile)                     override;

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
    virtual std::string GetEncodeCodecKey(
        VAProfile profile,
        VAEntrypoint entrypoint,
        uint32_t feiFunction) override;

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
    virtual CODECHAL_MODE GetEncodeCodecMode(
        VAProfile profile,
        VAEntrypoint entrypoint) override;

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

    // Implementation is the same as g12.
    virtual VAStatus QueryImageFormats(VAImageFormat *formatList, int32_t *num_formats) override;

    // Implementation is the same as g12.
    virtual uint32_t GetImageFormatsMaxNum() override;

    //!
    //! \brief    Populate the color masks info
    //!
    //! \param    [in,out] Image format
    //!           Pointer to a VAImageFormat array. Color masks information will be populated to this
    //!           structure.
    //!
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if succeed
    //! Implementation is the same as g12.
    virtual VAStatus PopulateColorMaskInfo(VAImageFormat *vaImgFmt) override;

    virtual bool IsImageSupported(uint32_t fourcc) override;

    //!
    //! \brief      Query AVC ROI maximum number
    //!
    //! \param      [in] rcMode
    //!     RC mode
    //! \param      [in] isVdenc
    //!     vdenc
    //! \param      [in] maxNum
    //!     Maximum number
    //! \param      [in] isRoiInDeltaQP
    //!     Is ROI in delta QP
    //!
    //! \return VAStatus
    //!     if call succeeds
    //!
    VAStatus QueryAVCROIMaxNum(uint32_t rcMode, bool isVdenc, uint32_t *maxNum, bool *isRoiInDeltaQP) override;

    virtual VAStatus LoadProfileEntrypoints() override;

    virtual VAStatus GetPlatformSpecificAttrib(VAProfile profile,
                                               VAEntrypoint entrypoint,
                                               VAConfigAttribType type,
                                               unsigned int *value) override;

    //!
    //! \brief    Initialize AV1 decode profiles, entrypoints and attributes
    //!
    VAStatus LoadAv1DecProfileEntrypoints();

    //!
    //! \brief        Initialize HEVC low-power encode profiles, entrypoints and attributes
    //!
    //! \return VAStatus
    //!     if call succeeds
    //!
    virtual VAStatus LoadHevcEncLpProfileEntrypoints();

    static const VAConfigAttribValEncRateControlExt m_encVp9RateControlExt;  //!< External enc rate control caps for VP9 encode

    //!
    //! \brief    Initialize VP9 encode profiles, entrypoints and attributes
    //!
    virtual VAStatus LoadVp9EncProfileEntrypoints() override;

    //!
    //! \brief    Initialize HEVC decode profiles, entrypoints and attributes
    //!
    virtual VAStatus LoadHevcDecProfileEntrypoints() override;

    virtual GMM_RESOURCE_FORMAT ConvertMediaFmtToGmmFmt(DDI_MEDIA_FORMAT format) override;

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
        VAProfile        profile,
        VAEntrypoint     entrypoint,
        VASurfaceAttrib *attribList,
        uint32_t &       numAttribs);

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
        VASurfaceAttrib *attribList,
        uint32_t *       numAttribs) override;

    //!
    //! \brief    Check if the give profile is HEVC
    //!
    //! \param    [in] profile
    //!           Specify the VAProfile
    //!
    //! \return   True if the profile is a HEVC profile
    //!           False if the profile isn't a HEVC profile
    //!
    virtual bool IsHevcProfile(VAProfile profile) override;

    //! \brief    To check if target resolution support on this platform
    //!
    virtual VAStatus CheckDecodeResolution(
        int32_t codecMode,
        VAProfile profile,
        uint32_t width,
        uint32_t height) override;
    
    VAStatus CreateDecAttributes(VAProfile profile, VAEntrypoint entrypoint, AttribMap **attributeList) override;

    virtual VAStatus CreateEncAttributes(
        VAProfile profile,
        VAEntrypoint entrypoint,
        AttribMap **attributeList) override;

    //! \brief Get surface drm modifier
    //!
    //! \param    [in] mediaSurface
    //!           Pointer to the media surface
    //! \param    [out] modifier
    //!           reference of the modifier
    //!
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if success
    //!
    virtual VAStatus GetSurfaceModifier(DDI_MEDIA_SURFACE* mediaSurface, uint64_t &modifier) override;

    //! \brief Set tile format according to external surface's modifier
    //!
    //! \param    [in] mediaSurface
    //!           Pointer to the media surface
    //! \param    [out] tileformat
    //!           Reference to the tileformat
    //! \param    [out] bMemCompEnable
    //!           Reference to the memory compress flag
    //! \param    [out] bMemCompRC
    //!           Reference to the memory compress rate control
    //!
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if success
    //!
    virtual VAStatus SetExternalSurfaceTileFormat(DDI_MEDIA_SURFACE* mediaSurface,
                                                            uint32_t &tileformat,
                                                            bool &bMemCompEnable,
                                                            bool &bMemCompRC) override;

    //!
    //! \brief    Check if the give profile is AV1
    //!
    //! \param    [in] profile
    //!           Specify the VAProfile
    //!
    //! \return   True if the profile is a AV1 profile
    //!           False if the profile isn't a AV1 profile
    //!
    static bool IsAV1Profile(VAProfile profile);

protected:
    static const uint32_t m_maxHevcEncWidth =
        CODEC_16K_MAX_PIC_WIDTH; //!< maxinum width for HEVC encode
    static const uint32_t m_maxHevcEncHeight =
        CODEC_12K_MAX_PIC_HEIGHT; //!< maxinum height for HEVC encode
    static const uint32_t m_decAvcMaxWidth =
        CODEC_4K_MAX_PIC_WIDTH; //!< Maximum width for AVC decode
    static const uint32_t m_decAvcMaxHeight =
        CODEC_4K_MAX_PIC_HEIGHT; //!< Maximum height for AVC decode
    static const uint32_t m_decHevcMax16kWidth =
        CODEC_16K_MAX_PIC_WIDTH; //!< Maximum width for HEVC decode
    static const uint32_t m_decHevcMax16kHeight =
        CODEC_16K_MAX_PIC_HEIGHT; //!< Maximum height for HEVC decode
    static const uint32_t m_decVp9Max16kWidth =
        CODEC_16K_MAX_PIC_WIDTH; //!< Maximum width for VP9 decode
    static const uint32_t m_decVp9Max16kHeight =
        CODEC_16K_MAX_PIC_HEIGHT; //!< Maximum height for VP9 decode
    static const uint32_t m_decAv1Max16kWidth =
        CODEC_16K_MAX_PIC_WIDTH; //!< Maximum width for AV1 decode
    static const uint32_t m_decAv1Max16kHeight =
        CODEC_16K_MAX_PIC_HEIGHT; //!< Maximum height for AV1 decode
    static const uint32_t m_maxVp9EncWidth =
        CODEC_8K_MAX_PIC_WIDTH; //!< maximum width for VP9 encode
    static const uint32_t m_maxVp9EncHeight =
        CODEC_8K_MAX_PIC_HEIGHT; //!< maximum height for VP9 encode
    static const uint32_t m_minVp9EncWidth =
        CODEC_128_MIN_PIC_WIDTH; //!< minimum width for VP9 encode
    static const uint32_t m_minVp9EncHeight =
        CODEC_96_MIN_PIC_HEIGHT; //!< minimum height for VP9 encode
    static const uint32_t m_minAv1EncWidth =
        CODEC_128_MIN_PIC_WIDTH;  //!< minimum width for AV1 encode
    static const uint32_t m_minAv1EncHeight =
        CODEC_96_MIN_PIC_HEIGHT;  //!< minimum height for AV1 encode
MEDIA_CLASS_DEFINE_END(MediaLibvaCapsMtlBase)
};
#endif