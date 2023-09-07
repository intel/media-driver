/*
* Copyright (c) 2022, Intel Corporation
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
//! \file     ddi_encode_vp9_specific.h
//! \brief    Defines class for DDI media vp9 encode
//!


#ifndef __DDI_ENCODER_VP9_SPECIFIC_H__
#define __DDI_ENCODER_VP9_SPECIFIC_H__

#include "ddi_encode_base_specific.h"
namespace encode
{
//!
//! \class  DdiEncodeVp9
//! \brief  Ddi encode VP9
//!
class DdiEncodeVp9 : public encode::DdiEncodeBase
{
public:
    //!
    //! \brief    Constructor
    //!
    DdiEncodeVp9(){};

    //!
    //! \brief    Destructor
    //!
    ~DdiEncodeVp9();

    //!
    //! \brief    Initialize Encode Context and CodecHal Setting for Vp9
    //!
    //! \param    [out] codecHalSettings
    //!           Pointer to CodechalSetting *
    //!
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if success, else fail reason
    //!
    VAStatus ContextInitialize(
        CodechalSetting *codecHalSettings) override;

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
    //! \brief    Encode in CodecHal for Vp9
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
    //! \brief    Parse Packed Header Parameter buffer to Encode Context
    //!
    //! \param    [in] ptr
    //!           Pointer to Packed Header Parameter buffer
    //!
    //! \return   VAStatus
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
    //! \return   VAStatus
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

    //!
    //! \brief    Parse Segment Map Parameter buffer to Encode Context
    //!
    //! \param    [in] buf
    //!           Pointer to Segment Map Parameter buffer
    //!
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if success, else fail reason
    //!
    VAStatus ParseSegMapParams(
        DDI_MEDIA_BUFFER *buf);

    //!
    //! \brief    Report extra encode status for completed coded buffer.
    //!
    //! \param    [in] encodeStatusReportData
    //!           Pointer to encode status reported by Codechal
    //! \param    [out] codedBufferSegment
    //!           Pointer to coded buffer segment
    //!
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if success, else fail reason
    //!
    VAStatus ReportExtraStatus(
        EncodeStatusReportData *encodeStatusReportData,
        VACodedBufferSegment   *codedBufferSegment) override;

private:
    //!
    //! \brief    Setup Codec Picture for Vp9
    //!
    //! \param    [in] mediaCtx
    //!           Pointer to DDI_MEDIA_CONTEXT
    //! \param    [in] rtTbl
    //!           Pointer to DDI_CODEC_RENDER_TARGET_TABLE
    //! \param    [in] surfaceID
    //!           VASurface index
    //! \param    [in] picReference
    //!           Reference picture flag
    //! \param    [out] codecHalPic
    //!           Pointer to CODEC_PICTURE
    //!
    //! \return   void
    //!
    void SetupCodecPicture(
    DDI_MEDIA_CONTEXT                     *mediaCtx,
    DDI_CODEC_RENDER_TARGET_TABLE         *rtTbl,
    CODEC_PICTURE                         *codecHalPic,
    VASurfaceID                           surfaceID,
    bool                                  picReference);

    //!
    //! \brief    Parse Misc Param Buffer Quality Level to Encode Context
    //!
    //! \param    [in] data
    //!           Pointer to Misc Param Buffer Quality Level
    //!
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if success, else fail reason
    //!
    VAStatus ParseMiscParamQualityLevel(
        void *data);

    //!
    //! \brief    Parse Misc Param VBV Data buffer to Encode Context
    //!
    //! \param    [in] data
    //!           Pointer to Misc Param VBV Data buffer
    //!
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if success, else fail reason
    //!
    VAStatus ParseMiscParamVBV(
        void *data);

    //!
    //! \brief    Parse Misc Param FrameRate Data buffer to Encode Context
    //!
    //! \param    [in] data
    //!           Pointer to Misc Param FR Data buffer
    //!
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if success, else fail reason
    //!
    VAStatus ParseMiscParamFR(
        void *data);

    //!
    //! \brief    Parse Misc Param RateControl Data buffer to Encode Context
    //!
    //! \param    [in] data
    //!           Pointer to Misc Param RC Data buffer
    //!
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if success, else fail reason
    //!
    VAStatus ParseMiscParamRC(
        void *data);

    //!
    //! \brief    Parse Misc Param Enc Quality to Encode Context
    //!
    //! \param    [in] data
    //!           Pointer to Misc Param Private Data buffer
    //!
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if success, else fail reason
    //!
    VAStatus ParseMiscParamEncQuality(
        void *data);

    //!
    //! \brief    Parse Misc Parameter Temporal Layer Params buffer to Encode Context
    //!
    //! \param    [in] data
    //!           Pointer to Misc Parameter Temporal Layer Params buffer
    //!
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if success, else fail reason
    //!
    VAStatus ParseMiscParameterTemporalLayerParams(
        void *data);

    CODEC_VP9_ENCODE_SEGMENT_PARAMS *m_segParams = nullptr; //!< Segment parameters.

    VACodedBufferVP9Status *m_codedBufStatus = nullptr; //!< .Coded buffer status

private:

//!
    //! \brief    Return the CODECHAL_FUNCTION type for give profile and entrypoint
    //!
    //! \param    [in] profile
    //!           Specify the VAProfile
    //!
    //! \param    [in] entrypoint
    //!           Specify the VAEntrypoint
    //!
    //! \return   Codehal function
    //!
    CODECHAL_FUNCTION GetEncodeCodecFunction(VAProfile profile, VAEntrypoint entrypoint, bool bVDEnc) override;
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

    uint32_t savedTargetBit[CODECHAL_ENCODE_VP9_MAX_NUM_TEMPORAL_LAYERS] = { 0 };
    uint32_t savedMaxBitRate[CODECHAL_ENCODE_VP9_MAX_NUM_TEMPORAL_LAYERS] = { 0 };

    uint32_t savedFrameRate[CODECHAL_ENCODE_VP9_MAX_NUM_TEMPORAL_LAYERS] = { 0 };

    uint32_t savedGopSize = 0;

    uint32_t savedHrdSize = 0;

    uint32_t savedHrdBufFullness = 0;

    bool headerInsertFlag = 0;

    uint32_t lastPackedHeaderType = 0;

    uint8_t vp9TargetUsage = 0;

    bool isSegParamsChanged = false;

    MEDIA_CLASS_DEFINE_END(encode__DdiEncodeVp9)
};

}

#endif //__DDI_ENCODER_VP9_SPECIFIC_H__
