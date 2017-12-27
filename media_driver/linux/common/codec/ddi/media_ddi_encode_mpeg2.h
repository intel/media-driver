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
//! \file     media_ddi_encode_mpeg2.h
//! \brief    Defines class for DDI media mpeg2 encode
//!

#ifndef __MEDIA_LIBVA_ENCODER_MPEG2_H__
#define __MEDIA_LIBVA_ENCODER_MPEG2_H__

#include "media_ddi_encode_base.h"

static const uint8_t maxTimeCodePic = 59;
static const uint8_t maxTimeCodeSec = 59;
static const uint8_t maxTimeCodeMin = 59;
static const uint8_t maxTimeCodeHr  = 23;

//!
//! \struct _DDI_ENCODE_MPEG2_FRAME_RATE
//! \brief  Define Mpeg2 encode frame rate
//!
static struct _DDI_ENCODE_MPEG2_FRAME_RATE
{
    uint32_t code;  //!< Index code.
    float    value;  //!< Frame rate value.
} frameRateTable[] = {  //!< Mpeg2 encode frame rate table.
    {1, 23.976},
    {2, 24.0},
    {3, 25.0},
    {4, 29.97},
    {5, 30},
    {6, 50},
    {7, 59.94},
    {8, 60}};

//!
//! \class  DdiEncodeMpeg2
//! \brief  Ddi encode MPEG2
//!
class DdiEncodeMpeg2 : public DdiEncodeBase
{
public:
    //!
    //! \brief    Constructor
    //!
    DdiEncodeMpeg2(){};

    //!
    //! \brief    Destructor
    //!
    ~DdiEncodeMpeg2();

    //!
    //! \brief    Initialize Encode Context and CodecHal Setting for Mpeg2
    //!
    //! \param    [out] codecHalSettings
    //!           Pointer to PCODECHAL_SETTINGS
    //!
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if success, else fail reason
    //!
    VAStatus ContextInitialize(
        CODECHAL_SETTINGS *codecHalSettings) override;

    //!
    //! \brief    Parse buffer to the server.
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
    //! \brief    Encode in CodecHal for Mpeg2
    //!
    //! \param    [in] numSlices
    //!           Number of slice data structures
    //!
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if success, else fail reason
    //!
    VAStatus EncodeInCodecHal(
        uint32_t numSlices) override;

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
    VAStatus ParsePicParams(
        DDI_MEDIA_CONTEXT *mediaCtx,
        void              *ptr) override;

    uint32_t getSliceParameterBufferSize() override;

    uint32_t getSequenceParameterBufferSize() override;

    uint32_t getPictureParameterBufferSize() override;

    uint32_t getQMatrixBufferSize() override;

    //!
    //! \brief    Parse QMatrix buffer to Encode Context
    //!
    //! \param    [in] ptr
    //!           Pointer to QMatrix buffer
    //!
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if success, else fail reason
    //!
    VAStatus Qmatrix(
        void *ptr);

    //!
    //! \brief    Parse Sequence Parameter buffer to Encode Context
    //!
    //! \param    [in] ptr
    //!           Pointer to Sequence Parameter buffer
    //!
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if success, else fail reason
    //!
    VAStatus ParseSeqParams(
        void *ptr);

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
    VAStatus ParseSlcParams(
        DDI_MEDIA_CONTEXT *mediaCtx,
        void              *ptr,
        uint32_t          numSlices);

    //!
    //! \brief    Parse Packed Header Parameter buffer to Encode Context
    //!
    //! \param    [in] ptr
    //!           Pointer to Packed Header Parameter buffer
    //!
    //! return    VAStatus
    //!           VA_STATUS_SUCCESS if success, else fail reason
    //!
    VAStatus ParsePackedHeaderParams(
        void *ptr);

    //!
    //! \brief    Parse Packed Header Data buffer to Encode Context
    //!
    //! \param    [in] ptr
    //!           Pointer to Packed Header Data buffer
    //!
    //! return    VAStatus
    //!           VA_STATUS_SUCCESS if success, else fail reason
    //!
    VAStatus ParsePackedHeaderData(
        void *ptr);

    //!
    //! \brief    Parse Misc Parameter buffer to Encode Context
    //!
    //! \param    [in] ptr
    //!           Pointer to Misc Parameter buffer
    //!
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if success, else fail reason
    //!
    VAStatus ParseMiscParams(
        void *ptr);

private:
    //!
    //! \brief    Remove Mpeg2 User Data in Encode Context
    //!
    //! return    VAStatus
    //!           VA_STATUS_SUCCESS if success, else fail reason
    //!
    VAStatus RemoveUserData();

    //!
    //! \brief    Get Framerate Code from frame rate value
    //!
    //! \param    [in] frameRate
    //!           Frame rate value
    //! \param    [in] frameRateExtD
    //!           Frame rate ExtD value
    //! \param    [in] frameRateExtN
    //!           Frame rate ExtN value
    //!
    //! \return   uint32_t
    //!           Get Framerate Code
    //!
    uint32_t CalculateFramerateCode(
        float   frameRate,
        uint8_t frameRateExtD,
        uint8_t frameRateExtN);

    //!
    //! \brief    Parse Misc Param VBV Data buffer to Encode Context
    //!
    //! \param    [in] data
    //!           Pointer to Misc Param VBV Data buffer
    //!
    //! return    void
    //!
    void ParseMiscParamVBV(
        void *data);

    //!
    //! \brief    Parse Misc Param FR Data buffer to Encode Context
    //!
    //! \param    [in] data
    //!           Pointer to Misc Param FR Data buffer
    //!
    //! return    void
    //!
    void ParseMiscParamFR(
        void *data);

    //!
    //! \brief    Parse Misc Param RC Data buffer to Encode Context
    //!
    //! \param    [in] data
    //!           Pointer to Misc Param RC Data buffer
    //!
    //! return    void
    //!
    void ParseMiscParamRC(
        void *data);

    //!
    //! \brief    Parse Misc Param Max Frame Data buffer to Encode Context
    //!
    //! \param    [in] data
    //!           Pointer to Misc Param Max Frame Data buffer
    //!
    //! return    void
    //!
    void ParseMiscParamMaxFrame(
        void *data);

    //!
    //! \brief    Parse Misc Param Type Extension Data buffer to Encode Context
    //!
    //! \param    [in] data
    //!           Pointer to Misc Param Type Extension Data buffer
    //!
    //! return    void
    //!
    void ParseMiscParamTypeExtension(
        void *data);

    //!
    //! \brief    Parse Misc Param Type encode quality buffer to Encode Context
    //!
    //! \param    [in] data
    //!           Pointer to Misc Param Type Extension Data buffer
    //!
    //! return    void
    //!
    void ParseMiscParamEncQuality(
        void *data);

    //!
    //! \brief    Parse Misc Param Type encode quality level to Encode Context
    //!
    //! \param    [in] data
    //!           Pointer to Misc Param Type encode quality level.
    //!
    //! return    void
    //!
    void ParseMiscParamEncQualityLevel(
        void *data);

    //!
    //! \brief    Parse Misc Param Skip Frame Data buffer to Encode Context
    //!
    //! \param    [in] data
    //!           Pointer to Misc Param Skip Frame Data buffer
    //!
    //! return    void
    //!
    void ParseMiscParamSkipFrame(
        void *data);

    uint8_t  m_scalingLists4x4[6][16];      //!< Scaling list 4x4.
    uint8_t  m_scalingLists8x8[2][64];      //!< Scaling list 8x8.
    bool     m_newTimeCode       = false;   //!< New time code flag.
    uint32_t m_timeCode          = 0;       //!< New time code.
    void     *m_userDataListHead = nullptr; //!< User data list head.
    void     *m_userDataListTail = nullptr; //!< User data list tail.
};
#endif /* __MEDIA_LIBVA_ENCODER_MPEG2_H__ */
