/*
* Copyright (c) 2015-2017, Intel Corporation
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
//! \file      media_ddi_decode_hevc.h 
//! \brief     libva(and its extension) decoder implementation 
//!
//!
//! \file     media_ddi_decode_hevc.h
//! \brief    Defines DdiDecodeHEVC class for HEVC decode
//!

#ifndef __MEDIA_DDI_DECODER_HEVC_H__
#define __MEDIA_DDI_DECODER_HEVC_H__

#include <va/va.h>
#include "media_ddi_decode_base.h"

//!
//! \class  DdiDecodeHEVC
//! \brief  Ddi decode HEVC
//!
class DdiDecodeHEVC : public DdiMediaDecode
{
public:
    //!
    //! \brief Constructor
    //!
    DdiDecodeHEVC(DDI_DECODE_CONFIG_ATTR *ddiDecodeAttr) : DdiMediaDecode(ddiDecodeAttr){};

    //!
    //! \brief Destructor
    //!
    virtual ~DdiDecodeHEVC(){};

    // inherited virtual functions
    virtual void DestroyContext(
        VADriverContextP ctx) override;

    virtual VAStatus RenderPicture(
        VADriverContextP ctx,
        VAContextID      context,
        VABufferID       *buffers,
        int32_t          numBuffers) override;

    virtual MOS_FORMAT GetFormat() override;

    virtual VAStatus SetDecodeParams() override;

    virtual void ContextInit(
        int32_t picWidth,
        int32_t picHeight) override;

    virtual VAStatus CodecHalInit(
        DDI_MEDIA_CONTEXT *mediaCtx,
        void              *ptr) override;

    virtual VAStatus AllocSliceControlBuffer(
        DDI_MEDIA_BUFFER       *buf) override;

    virtual uint8_t* GetPicParamBuf(
    DDI_CODEC_COM_BUFFER_MGR      *bufMgr) override;

protected:
    //!
    //! \brief   ParaSliceParam for HEVC
    //! \details parse the sliceParam info required by HEVC decoding for
    //!          each slice
    //!
    //! \param   [in] *mediaCtx
    //!          DDI_MEDIA_CONTEXT
    //! \param   [in] *slcParam
    //!          VASliceParameterBufferHEVC
    //! \param   [in] numSlices
    //!             int32_t
    //!
    //! \return  VA_STATUS_SUCCESS is returned if it is parsed successfully.
    //!          else fail reason
    virtual VAStatus ParseSliceParams(
        DDI_MEDIA_CONTEXT           *mediaCtx,
        VASliceParameterBufferHEVC  *slcParam,
        int32_t                     numSlices);

    //!
    //! \brief   ParseIQMatrixParam for HEVC
    //! \details parse the QMatrix info required by HEVC decoding
    //!
    //! \param   [in] *mediaCtx
    //!          DDI_MEDIA_CONTEXT
    //! \param   [in] *qMatrix
    //!          VAIQMatrixBufferHEVC
    //!
    //! \return  VA_STATUS_SUCCESS is returned if it is parsed successfully.
    //!          else fail reason
    VAStatus ParseIQMatrix(
        DDI_MEDIA_CONTEXT    *mediaCtx,
        VAIQMatrixBufferHEVC *matrix);

    //! \brief   ParsePicParam for HEVC
    //! \details parse the PicParam info required by HEVC decoding
    //!
    //! \param   [in] *mediaCtx
    //!          DDI_MEDIA_CONTEXT
    //! \param   [in] *qMatrix
    //!          VAIQMatrixBufferH264
    //!
    //! \return  VA_STATUS_SUCCESS is returned if it is parsed successfully.
    //!          else fail reason
    virtual VAStatus ParsePicParams(
        DDI_MEDIA_CONTEXT            *mediaCtx,
        VAPictureParameterBufferHEVC *picParam);

    //! \brief   Alloc SliceParam content for HEVC
    //! \details Alloc/resize SlicePram content for HEVC decoding
    //!
    //! \param   [in] numSlices
    //!          int32_t the required number of slices
    //!
    //! \return  VA_STATUS_SUCCESS is returned if it is parsed successfully.
    //!          else fail reason
    virtual VAStatus AllocSliceParamContext(
        int32_t numSlices);

    //! \brief   Init Resource buffer for HEVC
    //! \details Initialize and allocate the Resource buffer for HEVC
    //!
    //! \return  VA_STATUS_SUCCESS is returned if it is parsed successfully.
    //!          else fail reason
    virtual VAStatus InitResourceBuffer();

    //! \brief   Free Resource buffer for HEVC
    //!
    virtual void FreeResourceBuffer();

    //!
    //! \brief    Setup Codec Picture for Hevc
    //!
    //! \param    [in] mediaCtx
    //!           Pointer to DDI_MEDIA_CONTEXT
    //! \param    [in] rtTbl
    //!           Pointer to DDI_CODEC_RENDER_TARGET_TABLE
    //! \param    [in] vaPic
    //!           HEVC VAPicture structure
    //! \param    [in] fieldPicFlag
    //!           Field picture flag
    //! \param    [in] bottomFieldFlag
    //!           Bottom field flag
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
    VAPictureHEVC                         vaPic,
    bool                                  fieldPicFlag,
    bool                                  bottomFieldFlag,
    bool                                  picReference);
};

#endif /* _MEDIA_DDI_DECODE_HEVC_H */
