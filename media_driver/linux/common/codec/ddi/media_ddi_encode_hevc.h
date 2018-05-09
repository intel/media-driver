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
//! \file     media_ddi_encode_hevc.h
//! \brief    HEVC class definition for DDI media encoder.
//!

#ifndef __MEDIA_LIBVA_ENCODER_HEVC_H__
#define __MEDIA_LIBVA_ENCODER_HEVC_H__

#include "media_ddi_encode_base.h"

static const uint8_t sliceTypeB = 0;
static const uint8_t sliceTypeP = 1;
static const uint8_t sliceTypeI = 2;

static const uint8_t numMaxRefFrame    = 15;
static const uint8_t vdencRoiBlockSize = 32;

//!
//! \class  DdiEncodeHevc
//! \brief  DDi encode HEVC
//!
class DdiEncodeHevc : public DdiEncodeBase
{
public:
    //!
    //! \brief    Constructor
    //!
    DdiEncodeHevc(){};

    //!
    //! \brief    Destructor
    //!
    ~DdiEncodeHevc();

    //!
    //! \brief    Initialize Encode Context and CodecHal Setting for Hevc
    //!
    //! \param    [out] codecHalSettings
    //!           Pointer to CodechalSetting *
    //!
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if success, else fail reason
    //!
    VAStatus ContextInitialize(CodechalSetting *codecHalSettings) override;

    //!
    //! \brief    Send required buffers to for process
    //! \details  It sends needed buffers by the process to the driver
    //!
    //! \param    [in] ctx
    //!           Pointer to VADriverContextP
    //! \param    [in] context
    //!           VA context ID
    //! \param    [in] buffers
    //!           Pointer to VABufferID
    //! \param    [in] numBuffers
    //!           Number of buffers
    //!
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if success, else fail reason
    //!
    VAStatus RenderPicture(
        VADriverContextP ctx,
        VAContextID      context,
        VABufferID       *buffers,
        int32_t          numBuffers) override;

protected:

    //!
    //! \brief    Reset Encode Context At Frame Level
    //!
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if success, else fail reason
    //!
    VAStatus ResetAtFrameLevel() override;

    //!
    //! \brief    Encode in CodecHal for Hevc
    //!
    //! \param    [in] numSlices
    //!           Number of slice data structures
    //!
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if success, else fail reason
    //!
    VAStatus EncodeInCodecHal(uint32_t numSlices) override;

    //!
    //! \brief    Parse Picture Parameter buffer to Encode Context
    //!
    //! \param    [in] mediaCtx
    //!           Pointer to DDI_MEDIA_CONTEXT
    //! \param    [in] ptr
    //!           Pointer to Picture Parameter buffer
    //!
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if success, else fail reason
    //!
    VAStatus ParsePicParams(DDI_MEDIA_CONTEXT *mediaCtx, void *ptr) override;

    uint32_t getSliceParameterBufferSize() override;

    uint32_t getSequenceParameterBufferSize() override;

    uint32_t getPictureParameterBufferSize() override;

    uint32_t getQMatrixBufferSize() override;

    //!
    //! \brief    Parse Sequence Parameter buffer to Encode Context
    //!
    //! \param    [in] ptr
    //!           Pointer to Sequence Parameter buffer
    //!
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if success, else fail reason
    //!
    VAStatus ParseSeqParams(void *ptr);

    //!
    //! \brief    Parse Slice Parameter buffer to Encode Context
    //!
    //! \param    [in] mediaCtx
    //!           Pointer to DDI_MEDIA_CONTEXT
    //! \param    [in] ptr
    //!           Pointer to Slice Parameter buffer
    //! \param    [in] numSlices
    //!           Number of slice
    //!
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if success, else fail reason
    //!
    VAStatus ParseSlcParams(DDI_MEDIA_CONTEXT *mediaCtx, void *ptr, uint32_t numSlices);

    //!
    //! \brief    Find the NAL Unit Start Codes
    //!
    //! \param    [in] buf
    //!           Pointer to packed header NAL unit data
    //! \param    [in] size
    //!           byte size of packed header NAL unit data
    //! \param    [in] startCodesOffset
    //!           Pointer to NAL unit start codes offset from the packed header
    //!           NAL unit data buf
    //! \param    [in] startCodesLength
    //!           Pointer to NAL unit start codes length
    //!
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if success,
    //!           else VA_STATUS_ERROR_INVALID_BUFFER if start codes doesn't exit
    //!
    VAStatus FindNalUnitStartCodes(
        uint8_t * buf,
        uint32_t size,
        uint32_t * startCodesOffset,
        uint32_t * startCodesLength);

    //!
    //! \brief    Parse Packed Header Parameter buffer to Encode Context
    //!
    //! \param    [in] ptr
    //!           Pointer to Packed Header Parameter buffer
    //!
    //! return    VAStatus
    //!           VA_STATUS_SUCCESS if success, else fail reason
    //!
    VAStatus ParsePackedHeaderParams(void *ptr);

    //!
    //! \brief    Parse Packed Header Data buffer to Encode Context
    //!
    //! \param    [in] ptr
    //!           Pointer to Packed Header Data buffer
    //!
    //! return    VAStatus
    //!           VA_STATUS_SUCCESS if success, else fail reason
    //!
    VAStatus ParsePackedHeaderData(void *ptr);

    //!
    //! \brief    Parse Misc Parameter buffer to Encode Context
    //!
    //! \param    [in] ptr
    //!           Pointer to Misc Parameter buffer
    //!
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if success, else fail reason
    //!
    VAStatus ParseMiscParams(void *ptr);

    uint16_t m_previousFRvalue = 0; //!< For saving FR value to be used in case of dynamic BRC reset.

private:
    //!
    //! \brief    Get Encode Codechal Picture Type from Va Slice Type
    //!
    //! \param    [in] vaSliceType
    //!           Va Slice Type
    //!
    //! \return   uint8_t
    //!           Encode Codechal Picture Type
    //!
    uint8_t CodechalPicTypeFromVaSlcType(uint8_t vaSliceType);

    //!
    //! \brief    Get Slice Reference frame Index from picture reference
    //!
    //! \param    [in] picReference
    //!           Pointer to PCODEC_PICTURE
    //! \param    [out] slcReference
    //!           Pointer to PCODEC_PICTURE
    //!
    //! \return   void
    //!
    void GetSlcRefIdx(CODEC_PICTURE *picReference, CODEC_PICTURE *slcReference);

    //!
    //! \brief    Setup params of Codechal Picture
    //!
    //! \param    [in] mediaCtx
    //!           Pointer to DdiMediaContext
    //! \param    [in] RTtbl
    //!           Pointer to DDI_CODEC_RENDER_TARGET_TABLE
    //! \param    [in] vaPicHEVC
    //!           Pointer to VAPictureHEVC
    //! \param    [in] picReference
    //!           Picture reference flag
    //! \param    [in] sliceReference
    //!           Slice Reference flag
    //! \param    [out] codecHalPic
    //!           Pointer to PCODEC_PICTURE
    //!
    //! \return   void
    //!
    void SetupCodecPicture(
        DDI_MEDIA_CONTEXT             *mediaCtx,
        DDI_CODEC_RENDER_TARGET_TABLE *RTtbl,
        CODEC_PICTURE                 *codecHalPic,
        VAPictureHEVC                 vaPicHEVC,
        bool                          picReference,
        bool                          sliceReference);

};
#endif  //__MEDIA_LIBVA_ENCODER_HEVC_H__
