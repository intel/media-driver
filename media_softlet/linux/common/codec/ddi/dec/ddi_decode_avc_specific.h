/*
* Copyright (c) 2022-2023, Intel Corporation
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
//! \file     ddi_decode_avc_specific.h
//! \brief    Defines DdiDecodeAvc class for Avc decode
//!

#ifndef __DDI_DECODE_AVC_SPECIFIC_H__
#define __DDI_DECODE_AVC_SPECIFIC_H__

#include "ddi_decode_base_specific.h"

namespace decode
{

//!
//! \class  DdiDecodeAvc
//! \brief  Ddi decode Avc
//!
class DdiDecodeAvc : public DdiDecodeBase
{
public:
    //!
    //! \brief Constructor
    //!
    DdiDecodeAvc() : DdiDecodeBase() {};

    //!
    //! \brief Destructor
    //!
    virtual ~DdiDecodeAvc(){};

    // inherited virtual function
    virtual void DestroyContext(
        VADriverContextP ctx) override;

    virtual VAStatus RenderPicture(
        VADriverContextP ctx,
        VAContextID      context,
        VABufferID       *buffers,
        int32_t          numBuffers) override;

    virtual VAStatus SetDecodeParams() override;

    virtual void ContextInit(
        int32_t picWidth,
        int32_t picHeight) override;

    virtual VAStatus CodecHalInit(
        DDI_MEDIA_CONTEXT *mediaCtx,
        void              *ptr) override;

    virtual VAStatus AllocSliceControlBuffer(
        DDI_MEDIA_BUFFER *buf) override;

    virtual uint8_t* GetPicParamBuf(
    DDI_CODEC_COM_BUFFER_MGR *bufMgr) override;

private:
    //!
    //! \brief   ParaSliceParam for Avc
    //! \details parse the sliceParam info required by Avc decoding for
    //!          each slice
    //!
    //! \param   [in] *mediaCtx
    //!          DDI_MEDIA_CONTEXT
    //! \param   [in] *slcParam
    //!          VASliceParameterBufferH264
    //! \param   [in] numSlices
    //!             uint32_t
    //!
    //! \return  VA_STATUS_SUCCESS is returned if it is parsed successfully.
    //!          else fail reason
    VAStatus ParseSliceParams(
        DDI_MEDIA_CONTEXT          *mediaCtx,
        VASliceParameterBufferH264 *slcParam,
        uint32_t                   numSlices);

    //!
    //! \brief   ParaQMatrixParam for Avc
    //! \details parse the IQMatrix info required by Avc decoding
    //!
    //! \param   [in] *mediaCtx
    //!          DDI_MEDIA_CONTEXT
    //! \param   [in] *matrix
    //!          VAIQMatrixBufferH264
    //!
    //! \param   [in] numSlices
    //!          int32_t
    //!
    //! \return  VA_STATUS_SUCCESS is returned if it is parsed successfully.
    //!          else fail reason
    VAStatus ParseIQMatrix(
        DDI_MEDIA_CONTEXT    *mediaCtx,
        VAIQMatrixBufferH264 *matrix);

    //! \brief   ParsePicParam for Avc
    //! \details parse the PicParam info required by Avc decoding
    //!
    //! \param   [in] *mediaCtx
    //!          DDI_MEDIA_CONTEXT
    //! \param   [in] picParam
    //!          VAPictureParameterBufferH264
    //!
    //! \param   [in] numSlices
    //!          int32_t
    //! \return  VA_STATUS_SUCCESS is returned if it is parsed successfully.
    //!          else fail reason
    VAStatus ParsePicParams(
        DDI_MEDIA_CONTEXT            *mediaCtx,
        VAPictureParameterBufferH264 *picParam);

    //! \brief   Alloc SliceParam content for Avc
    //! \details Alloc/resize SlicePram content for AVC decoding
    //!
    //! \param   [in] numSlices
    //!          uint32_t the required number of slices
    //!
    //! \return  VA_STATUS_SUCCESS is returned if it is parsed successfully.
    //!          else fail reason
    VAStatus AllocSliceParamContext(
        uint32_t numSlices);

    //! \brief   Init resource buffer for AVC
    //! \details Initialize and allocate the resource buffer for AVC
    //!
    //! \return  VA_STATUS_SUCCESS is returned if it is parsed successfully.
    //!          else fail reason
    VAStatus InitResourceBuffer();

    //! \brief   Free Resource buffer for AVC
    //!
    void FreeResourceBuffer();

    //!
    //! \brief    Get Slice Reference Index
    //! \details  To change the value of  sliceParams->RefPicList[][].FrameIdx
    //!           from the frame itself  to the index in picParams->RefFrameList
    //!
    //! \param    [in] picReference
    //!           Pointer to CODEC_PICTURE
    //! \param    [in] slcReference
    //!           Pointer to CODEC_PICTURE
    //!
    //! \return   void
    //!
    void GetSlcRefIdx(CODEC_PICTURE *picReference, CODEC_PICTURE *slcReference);

    //!
    //! \brief    Setup Codec Picture for AVC
    //!
    //! \param    [in] mediaCtx
    //!           Pointer to DDI_MEDIA_CONTEXT
    //! \param    [in] rtTbl
    //!           Pointer to DDI_CODEC_RENDER_TARGET_TABLE
    //! \param    [in] vaPic
    //!           H264 VAPicture structure
    //! \param    [in] fieldPicFlag
    //!           Field picture flag
    //! \param    [in] picReference
    //!           Reference picture flag
    //! \param    [in] sliceReference
    //!           Reference slice flag
    //! \param    [out] codecHalPic
    //!           Pointer to CODEC_PICTURE
    //!
    //! \return   void
    //!
    void SetupCodecPicture(
        DDI_MEDIA_CONTEXT             *mediaCtx,
        DDI_CODEC_RENDER_TARGET_TABLE *rtTbl,
        CODEC_PICTURE                 *codecHalPic,
        VAPictureH264                 vaPic,
        bool                          fieldPicFlag,
        bool                          picReference,
        bool                          sliceReference);

    void FreeResource();

    MEDIA_CLASS_DEFINE_END(decode__DdiDecodeAvc)
};
} // namespace decode

#endif /* __DDI_DECODE_AVC_SPECIFIC_H__ */
