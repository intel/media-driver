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
//! \file     media_ddi_encode_vp9.h
//! \brief    Defines class for DDI media vp9 encode
//!

#include "media_ddi_encode_base.h"

//!
//! \class  DdiEncodeVp9
//! \brief  Ddi encode VP9
//!
class DdiEncodeVp9 : public DdiEncodeBase
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
    void ParseMiscParamEncQuality(
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
    //! \brief    Parse Frame Rate
    //!
    //! \param    [in] ptr
    //!           Pointer to void
    //!
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if success, else fail reason
    //!
    VAStatus ParseFrameRate(
        void *ptr);

    CODEC_VP9_ENCODE_SEGMENT_PARAMS *m_segParams = nullptr; //!< Segment parameters.
};
