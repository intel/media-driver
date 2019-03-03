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
//! \file     media_ddi_encode_base.h
//! \brief    Defines base class for DDI media encode.
//!

#ifndef __MEDIA_DDI_ENCODE_BASE_H__
#define __MEDIA_DDI_ENCODE_BASE_H__

#include <va/va.h>
#include "media_ddi_base.h"
#include "media_libva_encoder.h"
#include "codechal_setting.h"

//!
//! \class  DdiEncodeBase
//! \brief  Ddi encode base
//!
class DdiEncodeBase : public DdiMediaBase
{
public:
    //! \brief chroma format
    enum ChromaFormat
    {
        monochrome  = 0,
        yuv420      = 1,
        yuv422      = 2,
        yuv444      = 3
    };

    //!
    //! \brief Constructor
    //!
    DdiEncodeBase();

    //!
    //! \brief    Initialize the encode context and do codechal setting
    //! \details  Allocate memory for pointer members of encode context, set codechal
    //!           which used by Codechal::Allocate
    //!
    //! \param    [out] codecHalSettings
    //!           CodechalSetting *
    //!
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if success, else fail reason
    //!
    virtual VAStatus ContextInitialize(CodechalSetting * codecHalSettings) = 0;

    //!
    //! \brief Destructor
    //!
    virtual ~DdiEncodeBase()
    {
        MOS_Delete(m_codechalSettings);
        m_codechalSettings = nullptr;
    };

    virtual VAStatus BeginPicture(
        VADriverContextP ctx,
        VAContextID      context,
        VASurfaceID      renderTarget);

    virtual VAStatus RenderPicture(
        VADriverContextP ctx,
        VAContextID      context,
        VABufferID *     buffers,
        int32_t          numBuffers) = 0;

    virtual VAStatus EndPicture(
        VADriverContextP ctx,
        VAContextID      context);

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
    virtual VAStatus StatusReport(
        DDI_MEDIA_BUFFER *mediaBuf,
        void             **buf);

    //!
    //! \brief    Report Status for Enc buffer.
    //!
    //! \param    [in] mediaBuf
    //!           Pointer to DDI_MEDIA_BUFFER
    //! \param    [out] buf
    //!           Pointer to buffer
    //!
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if success, else fail reason
    //!
    VAStatus EncStatusReport(
        DDI_MEDIA_BUFFER *mediaBuf,
        void             **buf);

    //!
    //! \brief    Report Status for PreEnc buffer.
    //!
    //! \param    [in] mediaBuf
    //!           Pointer to DDI_MEDIA_BUFFER
    //! \param    [out] buf
    //!           Pointer to buffer
    //!
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if success, else fail reason
    //!
    VAStatus PreEncStatusReport(
        DDI_MEDIA_BUFFER *mediaBuf,
        void             **buf);

    //!
    //! \brief    Remove Report Status from status report list.
    //!
    //! \param    [in] buf
    //!           Pointer to DDI_MEDIA_BUFFER
    //!
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if success, else fail reason
    //!
    VAStatus RemoveFromStatusReportQueue(DDI_MEDIA_BUFFER *buf);

    //!
    //! \brief    Remove Enc Report Status from status report list.
    //!
    //! \param    [in] buf
    //!           Pointer to DDI_MEDIA_BUFFER
    //! \param    [in] typeIdx
    //!           FEI Enc buffer type
    //!
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if success, else fail reason
    //!
    VAStatus RemoveFromEncStatusReportQueue(
        DDI_MEDIA_BUFFER               *buf,
        DDI_ENCODE_FEI_ENC_BUFFER_TYPE typeIdx);

    //!
    //! \brief    Remove PreEnc Report Status from status report list.
    //!
    //! \param    [in] buf
    //!           Pointer to DDI_MEDIA_BUFFER
    //! \param    [in] typeIdx
    //!           PreEnc buffer type
    //!
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if success, else fail reason
    //!
    VAStatus RemoveFromPreEncStatusReportQueue(
        DDI_MEDIA_BUFFER               *buf,
        DDI_ENCODE_PRE_ENC_BUFFER_TYPE typeIdx);

    //!
    //! \brief    Check if coded buffer exist in status report list
    //!
    //! \param    [in] buf
    //!           Pointer to DDI_MEDIA_BUFFER
    //!
    //! \return   bool
    //!           true if exist, else false
    //!
    bool CodedBufferExistInStatusReport(DDI_MEDIA_BUFFER *buf);

    //!
    //! \brief    Check if Enc buffer exist in status report list
    //!
    //! \param    [in] buf
    //!           Pointer to DDI_MEDIA_BUFFER
    //! \param    [in] typeIdx
    //!           FEI Enc buffer type
    //!
    //! \return   bool
    //!           true if exist, else false
    //!
    bool EncBufferExistInStatusReport(
        DDI_MEDIA_BUFFER               *buf,
        DDI_ENCODE_FEI_ENC_BUFFER_TYPE typeIdx);

    //!
    //! \brief    Check if PreEnc buffer exist in status report list
    //!
    //! \param    [in] buf
    //!           Pointer to DDI_MEDIA_BUFFER
    //! \param    [in] typeIdx
    //!           PreEnc buffer type
    //!
    //! \return   bool
    //!           true if exist, else false
    //!
    bool PreEncBufferExistInStatusReport(
        DDI_MEDIA_BUFFER               *buf,
        DDI_ENCODE_PRE_ENC_BUFFER_TYPE typeIdx);

    //!
    //! \brief    Create Encode buffer
    //! \details  Create Encode buffer
    //!
    //! \param    [in] ctx
    //!           Pointer to VA Driver Context
    //! \param    [in] type
    //!           Va Buffer type
    //! \param    [in] size
    //!           Size of each element in buffer
    //! \param    [in] elementsNum
    //!           Number of elements
    //! \param    [in] data
    //!           Buffer data
    //! \param    [in] bufId
    //!           Pointer to VABufferID
    //!
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if successful, else fail reason
    //!
    VAStatus CreateBuffer(
        VADriverContextP ctx,
        VABufferType     type,
        uint32_t         size,
        uint32_t         elementsNum,
        void             *data,
        VABufferID       *bufId);

    //!
    //! \brief    Init comp buffer
    //! \details  Init comp buffer
    //!
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if successful, else fail reason
    //!
    virtual VAStatus InitCompBuffer();

    //!
    //! \brief    Free comp buffer
    //! \details  Free comp buffer
    //!
    //! \return   void
    //!
    virtual void FreeCompBuffer();

    DDI_ENCODE_CONTEXT *m_encodeCtx = nullptr; //!< The referred DDI_ENCODE_CONTEXT object.
    bool m_is10Bit                  = false;   //!< 10 bit flag.
    ChromaFormat m_chromaFormat     = yuv420;  //!< HCP chroma format.
    CodechalSetting    *m_codechalSettings = nullptr;    //!< Codechal Settings
protected:
    //!
    //! \brief    Do Encode in codechal
    //! \details  Prepare encode parameters, surfaces and buffers and submit
    //!           to codechal to do encoding
    //!
    //! \param    [in] numSlices
    //!           Number of slices
    //!
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if successful, else fail reason
    //!
    virtual VAStatus EncodeInCodecHal(uint32_t numSlices) = 0;

    //!
    //! \brief    Reset the parameters before each frame
    //! \details  Called by BeginPicture, reset sps parameter, bsbuffer,
    //!           packed header information, etc
    //!
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if successful, else fail reason
    //!
    virtual VAStatus ResetAtFrameLevel() = 0;

    //!
    //! \brief    Parse picture params
    //! \details  Parse picture params called by RenderPicture
    //!
    //! \param    [in] mediaCtx
    //!           Pointer to DDI_MEDIA_CONTEXT
    //! \param    [in] ptr
    //!           Pointer to buffer VAEncPictureParameterBufferH264
    //!
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if successful, else fail reason
    //!
    virtual VAStatus ParsePicParams(
        DDI_MEDIA_CONTEXT *mediaCtx,
        void *             ptr) = 0;

    //!
    //! \brief    get Slice Parameter Buffer Size
    //!
    //! \return   uint32_t
    //!           Slice Parameter Buffer Size
    //!
    virtual uint32_t getSliceParameterBufferSize();

    //!
    //! \brief    get Sequence Parameter Buffer Size
    //!
    //! \return   uint32_t
    //!           Sequence Parameter Buffer Size
    //!
    virtual uint32_t getSequenceParameterBufferSize();

    //!
    //! \brief    get Picture Parameter Buffer Size
    //!
    //! \return   uint32_t
    //!           Picture Parameter Buffer Size
    //!
    virtual uint32_t getPictureParameterBufferSize();

    //!
    //! \brief    get QMatrix Parameter Buffer Size
    //!
    //! \return   uint32_t
    //!           QMatrix Parameter Buffer Size
    //!
    virtual uint32_t getQMatrixBufferSize();

    //!
    //! \brief    Add coded buffer information
    //! \details  Add coded buffer information to Status Report Queue. this function
    //!           called by RenderPicture
    //!
    //! \param    [in] codedBuf
    //!           Pointer to Coded buffer
    //!
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if successful, else fail reason
    //!
    VAStatus AddToStatusReportQueue(void *codedBuf);

    //!
    //! \brief    Convert rate control method in VAAPI to the term in HAL
    //!
    //! \param    [in] vaRC
    //!           Rate control method in VAAPI
    //!
    //! \return   uint8_t
    //!           Rate control method in HAL
    //!
    uint8_t VARC2HalRC(uint32_t vaRC);

    //!
    //! \brief    Update status report buffer
    //! \details  When we get coded buffer's size at StatusReport, update the size to Status
    //!           Report Buffer. There should be some buffers waiting to be updated which
    //!           are added in AddToStatusReportQueue.
    //!
    //! \param    [in] size
    //!           Coded buffer's size
    //! \param    [in] status
    //!           Buffer status
    //!
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if successful, else fail reason
    //!
    VAStatus UpdateStatusReportBuffer(
        uint32_t            size,
        uint32_t            status);

    //!
    //! \brief    Update Enc status report buffer
    //! \details  There should be some buffers waiting to be updated which
    //!           are added in AddToEncStatusReportQueue.
    //!
    //! \param    [in] status
    //!           Buffer status
    //!
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if successful, else fail reason
    //!
    VAStatus UpdateEncStatusReportBuffer(uint32_t status);

    //!
    //! \brief    Update PreEnc status report buffer
    //! \details  There should be some buffers waiting to be updated which
    //!           are added in AddToPreEncStatusReportQueue.
    //!
    //! \param    [in] status
    //!           Buffer status
    //!
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if successful, else fail reason
    //!
    VAStatus UpdatePreEncStatusReportBuffer(uint32_t status);

    //!
    //! \brief    Get Size From Status Report Buffer
    //! \details  Get the coded buffer size, status and the index from Status
    //!           Report Queue when we can match the input buffer.
    //!
    //! \param    [in] buf
    //!           Pointer to DDI_MEDIA_BUFFER
    //! \param    [out] size
    //!           Pointer to uint32_t
    //! \param    [out] status
    //!           Pointer to uint32_t
    //! \param    [out] index
    //!           Pointer to int32_t
    //!
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if successful, else fail reason
    //!
    VAStatus GetSizeFromStatusReportBuffer(
        DDI_MEDIA_BUFFER   *buf,
        uint32_t           *size,
        uint32_t           *status,
        int32_t            *index);

    //!
    //! \brief    Get Index From Enc Status Report Buffer
    //! \details  Get the Enc buffer status and the index from Status
    //!           Report Queue when we can match the input buffer.
    //!
    //! \param    [in] buf
    //!           Pointer to DDI_MEDIA_BUFFER
    //! \param    [in] typeIdx
    //!           FEI Enc buffer type
    //! \param    [out] status
    //!           Pointer to uint32_t
    //! \param    [out] index
    //!           Pointer to int32_t
    //!
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if successful, else fail reason
    //!
    VAStatus GetIndexFromEncStatusReportBuffer(
        DDI_MEDIA_BUFFER               *buf,
        DDI_ENCODE_FEI_ENC_BUFFER_TYPE typeIdx,
        uint32_t                       *status,
        int32_t                        *index);

    //!
    //! \brief    Get Index From PreEnc Status Report Buffer
    //! \details  Get the PreEnc buffer status and the index from Status
    //!           Report Queue when we can match the input buffer.
    //!
    //! \param    [in] buf
    //!           Pointer to DDI_MEDIA_BUFFER
    //! \param    [in] typeIdx
    //!           PreEnc buffer type
    //! \param    [out] status
    //!           Pointer to uint32_t
    //! \param    [out] index
    //!           Pointer to int32_t
    //!
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if successful, else fail reason
    //!
    VAStatus GetIndexFromPreEncStatusReportBuffer(
        DDI_MEDIA_BUFFER               *buf,
        DDI_ENCODE_PRE_ENC_BUFFER_TYPE typeIdx,
        uint32_t                       *status,
        int32_t                        *index);

    //!
    //! \brief    Report extra encode status for completed coded buffer.
    //!
    //! \param    [in] encodeStatusReport
    //!           Pointer to encode status reported by Codechal
    //! \param    [out] codedBufferSegment
    //!           Pointer to coded buffer segment
    //!
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if success, else fail reason
    //!
    virtual VAStatus ReportExtraStatus(
        EncodeStatusReport   *encodeStatusReport,
        VACodedBufferSegment *codedBufferSegment)
    {
        return VA_STATUS_SUCCESS;
    }

    //!
    //! \brief    Clean Up Buffer and Return
    //!
    //! \param    [in] buf
    //!           Pointer to DDI_MEDIA_BUFFER
    //!
    //! \return   void
    void CleanUpBufferandReturn(DDI_MEDIA_BUFFER *buf);

    bool    m_newSeqHeader           = false;    //!< Flag for new Sequence Header.
    bool    m_newPpsHeader           = false;    //!< Flag for new Pps Header.
    bool    m_arbitraryNumMbsInSlice = false;    //!< Flag to indicate if the sliceMapSurface needs to be programmed or not.
    uint8_t m_scalingLists4x4[6][16]{};          //!< Inverse quantization scale lists 4x4.
    uint8_t m_scalingLists8x8[2][64]{};          //!< Inverse quantization scale lists 8x8.
};
#endif /* __MEDIA_DDI_ENCODE_BASE_H__ */
