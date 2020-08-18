/*
* Copyright (c) 2017-2020, Intel Corporation
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
//! \file     media_ddi_decode_av1_g12.h
//! \brief    Defines DdiDecodeAV1 class for AV1 decode
//!

#ifndef __MEDIA_DDI_DECODER_AV1_G12_H__
#define __MEDIA_DDI_DECODER_AV1_G12_H__

#include <va/va.h>
#include "media_ddi_decode_base.h"
#include "codec_def_decode_av1.h"

#define DECODE_ID_AV1           "VIDEO_DEC_AV1"

//!
//! \class  DdiDecodeAV1
//! \brief  Ddi decode AV1
//!

typedef struct _DDI_CODEC_BUFFER_PARAM_AV1
{
    // one picture buffer
    VADecPictureParameterBufferAV1               PicParamAV1;

    VASliceParameterBufferAV1                *pVASliceParameterBufferAV1;
} DDI_CODEC_BUFFER_PARAM_AV1;

class DdiDecodeAV1 : public DdiMediaDecode
{
public:
    //!
    //! \brief Constructor
    //!
    DdiDecodeAV1(DDI_DECODE_CONFIG_ATTR *ddiDecodeAttr) : DdiMediaDecode(ddiDecodeAttr)
    {
        MOS_ZeroMemory(&outputSurface, sizeof(outputSurface));
    };

    //!
    //! \brief Destructor
    //!
    virtual ~DdiDecodeAV1() {};

    // inherited virtual functions
    virtual void DestroyContext(
        VADriverContextP ctx) override;

    virtual VAStatus RenderPicture(
        VADriverContextP ctx,
        VAContextID      context,
        VABufferID       *buffers,
        int32_t          numBuffers) override;

    virtual VAStatus InitDecodeParams(
        VADriverContextP ctx,
        VAContextID      context) override;

    virtual VAStatus SetDecodeParams() override;

    virtual MOS_FORMAT GetFormat() override;

    /*virtual VAStatus EndPicture(
    VADriverContextP ctx,
    VAContextID      context) override;*/

    virtual void ContextInit(
        int32_t picWidth,
        int32_t picHeight) override;

    virtual VAStatus CodecHalInit(
        DDI_MEDIA_CONTEXT *mediaCtx,
        void              *ptr) override;

    virtual VAStatus AllocSliceControlBuffer(
        DDI_MEDIA_BUFFER       *buf) override;

    virtual uint8_t* GetPicParamBuf(
        DDI_CODEC_COM_BUFFER_MGR     *bufMgr) override;

private:
    //!
    //! \brief   ParseSliceParam for AV1
    //! \details parse the sliceParam info required by AV1 decoding for
    //!          each slice
    //!
    //! \param   [in] *mediaCtx
    //!          DDI_MEDIA_CONTEXT
    //! \param   [in] *slcParam
    //!          VASliceParameterBufferAV1
    //! \param   [in] numTile
    //!
    //! \return  VA_STATUS_SUCCESS is returned if it is parsed successfully.
    //!          else fail reason
    VAStatus ParseTileParams(
        DDI_MEDIA_CONTEXT             *mediaCtx,
        VASliceParameterBufferAV1     *slcParam,
        uint32_t                      numTiles);

    //! \brief   ParsePicParam for AV1
    //! \details parse the PicParam info required by AV1 decoding
    //!
    //! \param   [in] *mediaCtx
    //!          DDI_MEDIA_CONTEXT
    //! \param   [in] *qMatrix
    //!          VAIQMatrixBufferH264
    //!
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if success, else fail reason
    //! \return  VA_STATUS_SUCCESS is returned if it is parsed successfully.
    //!          else fail reason
    VAStatus ParsePicParams(
        DDI_MEDIA_CONTEXT *mediaCtx,
        VADecPictureParameterBufferAV1 *picParam);

    VAStatus ParseAv1SegFilterLevel(
        DDI_MEDIA_CONTEXT *mediaCtx,
        VADecPictureParameterBufferAV1 *picParam);

    VAStatus Av1LoopFilterFrameInit(
        DDI_MEDIA_CONTEXT *mediaCtx,
        VADecPictureParameterBufferAV1 *picParam,
        int defaultFiltLvl,
        int defaultFiltLvlR,
        int plane);

    uint32_t Av1GetQindex(
        CodecAv1SegmentsParams *segInfo,
        uint32_t segment_id,
        uint8_t base_qindex);

    int Av1Clamp(int value, int low, int high);

    //! \brief   Init Resource buffer for AV1
    //! \details Initialize and allocate the Resource buffer for AV1
    //!
    //! \return  VA_STATUS_SUCCESS is returned if it is parsed successfully.
    //!          else fail reason
    VAStatus InitResourceBuffer();

    //! \brief   Free Resource buffer for AV1
    //!
    void FreeResourceBuffer();

    //! \brief   the flag of slice data. It indicates whether slc data is passed
    bool slcFlag = false;
    //! \brief   film grain output surface
    PDDI_MEDIA_SURFACE filmGrainOutSurface = nullptr;
    //! \brief   film grain output surface structure
    MOS_SURFACE outputSurface;
};

#endif /* _MEDIA_DDI_DECODE_AV1_H */
