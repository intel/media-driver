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
//! \file     media_libva_caps_g11.h
//! \brief    This file defines the C++ class/interface for gen11 media capbilities. 
//!

#ifndef __MEDIA_LIBVA_CAPS_G11_H__
#define __MEDIA_LIBVA_CAPS_G11_H__

#include "media_libva_caps.h"

//!
//! \class  MediaLibvaCapsG11
//! \brief  Media libva caps Gen11
//!
class MediaLibvaCapsG11 : public MediaLibvaCaps
{
public:
    //!
    //! \brief    Constructor
    //!
    MediaLibvaCapsG11(DDI_MEDIA_CONTEXT *mediaCtx) : MediaLibvaCaps(mediaCtx)
    {
        // ICL supported Encode format
        static struct EncodeFormatTable encodeFormatTableICL[] =
        {
            {AVC, DualPipe, VA_RT_FORMAT_YUV420},
            {AVC, Vdenc, VA_RT_FORMAT_YUV420 | VA_RT_FORMAT_YUV422 | VA_RT_FORMAT_YUV444},
            {HEVC, DualPipe, VA_RT_FORMAT_YUV420 | VA_RT_FORMAT_YUV420_10BPP | VA_RT_FORMAT_YUV422 | VA_RT_FORMAT_YUV422_10},
            {HEVC, Vdenc, VA_RT_FORMAT_YUV420 | VA_RT_FORMAT_YUV420_10BPP | VA_RT_FORMAT_YUV444 | VA_RT_FORMAT_YUV444_10 | VA_RT_FORMAT_RGB32 | VA_RT_FORMAT_RGB32_10BPP},
            {VP9, Vdenc, VA_RT_FORMAT_YUV420 | VA_RT_FORMAT_YUV420_10BPP | VA_RT_FORMAT_YUV444 | VA_RT_FORMAT_YUV444_10 | VA_RT_FORMAT_RGB32 | VA_RT_FORMAT_RGB32_10BPP},
        };
        m_encodeFormatTable = (struct EncodeFormatTable*)(&encodeFormatTableICL[0]);
        m_encodeFormatCount = sizeof(encodeFormatTableICL)/sizeof(struct EncodeFormatTable);

        return;
    }

    //!
    virtual VAStatus Init() override
    {
        return LoadProfileEntrypoints();
    }

    virtual VAStatus QueryImageFormats(VAImageFormat *formatList, int32_t *num_formats) override;

    virtual uint32_t GetImageFormatsMaxNum() override;

    virtual bool IsImageSupported(uint32_t fourcc) override;

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
    virtual VAStatus PopulateColorMaskInfo(VAImageFormat *vaImgFmt) override;

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
    //! \brief    Return the decode codec key for given profile 
    //!
    //! \param    [in] profile 
    //!           Specify the VAProfile 
    //!
    //! \return   Std::string decode codec key 
    //!
    virtual std::string GetDecodeCodecKey(VAProfile profile) override;

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
    //! \brief convert Media Format to Gmm Format for GmmResCreate parameter.
    //!
    //! \param    [in] format
    //!         Pointer to DDI_MEDIA_FORMAT
    //!
    //! \return GMM_RESOURCE_FORMAT
    //!         Pointer to gmm format type
    //!
    virtual GMM_RESOURCE_FORMAT ConvertMediaFmtToGmmFmt(DDI_MEDIA_FORMAT format) override;

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
            VAConfigID configId,
            VASurfaceAttrib *attribList,
            uint32_t *numAttribs)                                                override;

protected:
    static const uint32_t m_maxHevcEncWidth =
        CODEC_8K_MAX_PIC_WIDTH; //!< maxinum width for HEVC encode
    static const uint32_t m_maxHevcEncHeight =
        CODEC_8K_MAX_PIC_HEIGHT; //!< maxinum height for HEVC encode
    static const uint32_t m_maxVp9EncWidth =
        CODEC_8K_MAX_PIC_WIDTH; //!< maxinum width for VP9 encode
    static const uint32_t m_maxVp9EncHeight =
        CODEC_8K_MAX_PIC_HEIGHT; //!< maxinum height for VP9 encode
    static const VAImageFormat m_G11ImageFormats[]; //!< Gen11 supported image formats
    static const VAConfigAttribValEncRateControlExt m_encVp9RateControlExt; //!< External enc rate control caps for VP9 encode

    virtual VAStatus GetPlatformSpecificAttrib(VAProfile profile,
            VAEntrypoint entrypoint,
            VAConfigAttribType type,
            unsigned int *value) override;

    virtual VAStatus LoadProfileEntrypoints() override;
    virtual VAStatus LoadVp9EncProfileEntrypoints() override;
    virtual VAStatus LoadHevcEncProfileEntrypoints() override;

    virtual VAStatus CheckEncodeResolution(
            VAProfile profile,
            uint32_t width,
            uint32_t height) override;
    virtual VAStatus CheckDecodeResolution(
            int32_t codecMode,
            VAProfile profile,
            uint32_t width,
            uint32_t height) override;

    virtual VAStatus CreateDecAttributes(
        VAProfile profile,
        VAEntrypoint entrypoint,
        AttribMap **attributeList) override;

    //!
    //! \brief    Initialize HEVC low-power encode profiles, entrypoints and attributes
    //!
    //! \return VAStatus
    //!     if call succeeds
    //!
    VAStatus LoadHevcEncLpProfileEntrypoints();

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
    //!     if call succeeds
    //!
    VAStatus QueryAVCROIMaxNum(uint32_t rcMode, bool isVdenc, uint32_t *maxNum, bool *isRoiInDeltaQP) override;
};
#endif
