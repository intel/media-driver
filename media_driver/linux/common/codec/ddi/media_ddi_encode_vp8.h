/*
* Copyright (c) 2009-2017, Intel Corporation
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
//! \file     media_ddi_encode_vp8.h
//! \brief    Defines class for DDI media vp8 encode
//!

#include "media_ddi_encode_base.h"

class DdiEncodeVp8 : public DdiEncodeBase
{
public:
    //!
    //! \brief    Constructor
    //!
    DdiEncodeVp8(){};

    //!
    //! \brief    Destructor
    //!
    ~DdiEncodeVp8();

    //!
    //! \brief    Initialize Encode Context and CodecHal Setting for Vp8
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

    //!
    //! \brief    Report Status for coded buffer.
    //!
    //! \param    [in] mediaBuf
    //!           Pointer to DDI_MEDIA_BUFFER
    //! \param    [out] buf
    //!           Pointer to buffer
    //!
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if success, else fail reason
    //!
    VAStatus StatusReport(
        DDI_MEDIA_BUFFER *mediaBuf,
        void             **buf) override;

protected:
    //!
    //! \brief    Reset Encode Context At Frame Level
    //!
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if success, else fail reason
    //!
    VAStatus ResetAtFrameLevel() override;

    //!
    //! \brief    Encode in CodecHal for Vp8
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

    VAStatus InitCompBuffer() override;

    void FreeCompBuffer() override;

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
    //! \brief    Updating VP8StatusReport information
    //!
    //! \param    [in] buf
    //!           Pointer to Status Report Parameter buffer
    //! \param    [in] size
    //!           Size of Status Report Parameter buffer
    //!
    //! \return   uint32_t
    //!           The struct size of VAEncMiscParameterVP8Status
    //!
    uint32_t AddExtStatusReportParam(
        DDI_MEDIA_BUFFER *buf,
        uint32_t          size);

private:
    //!
    //! \brief    Parse Misc Param Buffer Quality Level to Encode Context
    //!
    //! \param    [in] data
    //!           Pointer to Misc Param Buffer Quality Level
    //!
    //! return    void
    //!
    void ParseBufferQualityLevel(
        void *data);

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
    //! \brief    Parse Misc Param Private Data buffer to Encode Context
    //!
    //! \param    [in] data
    //!           Pointer to Misc Param Private Data buffer
    //!
    //! return    void
    //!
    void ParseMiscParamPrivate(
        void *data);

    //!
    //! \brief    Parse Misc Parameter Temporal Layer Params buffer to Encode Context
    //!
    //! \param    [in] data
    //!           Pointer to Misc Parameter Temporal Layer Params buffer
    //!
    //! return    void
    //!
    void ParseMiscParameterTemporalLayerParams(
        void *data);

    //!
    //! \brief    Setup Codec Picture for Vp8
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

    uint32_t   m_mvOffset = 0; //!< Motion vector offset.
    uint32_t   m_framesPer100Sec = 0;
};
