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
//! \file     ddi_decode_av1_specific.h
//! \brief    Defines DdiDecodeAv1 class for AV1 decode
//!

#ifndef __DDI_DECODE_AV1_SPECIFIC_H__
#define __DDI_DECODE_AV1_SPECIFIC_H__

#include "ddi_decode_base_specific.h"

#define DECODE_ID_AV1             "VIDEO_DEC_AV1"
#define MAX_ANCHOR_FRAME_NUM_AV1  128

namespace decode
{

//!
//! \class  DdiDecodeAv1
//! \brief  Ddi Decode AV1
//!

typedef struct _DDI_DECODE_BUFFER_PARAM_AV1
{
    // one picture buffer
    VADecPictureParameterBufferAV1 PicParamAV1;
    VASliceParameterBufferAV1     *pVASliceParameterBufferAV1;
} DDI_DECODE_BUFFER_PARAM_AV1;

class DdiDecodeAv1 : public DdiDecodeBase
{
public:
    //!
    //! \brief Constructor
    //!
    DdiDecodeAv1() : DdiDecodeBase()
    {
        MOS_ZeroMemory(&outputSurface, sizeof(outputSurface));
    };

    //!
    //! \brief Destructor
    //!
    virtual ~DdiDecodeAv1() {};

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
        DDI_MEDIA_BUFFER *buf) override;

    virtual uint8_t* GetPicParamBuf(
        DDI_CODEC_COM_BUFFER_MGR *bufMgr) override;

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
        DDI_MEDIA_CONTEXT         *mediaCtx,
        VASliceParameterBufferAV1 *slcParam,
        uint32_t                  numTiles);

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
        DDI_MEDIA_CONTEXT              *mediaCtx,
        VADecPictureParameterBufferAV1 *picParam);

    VAStatus ParseAv1SegFilterLevel(
        DDI_MEDIA_CONTEXT              *mediaCtx,
        VADecPictureParameterBufferAV1 *picParam);

    VAStatus Av1LoopFilterFrameInit(
        DDI_MEDIA_CONTEXT              *mediaCtx,
        VADecPictureParameterBufferAV1 *picParam,
        int                            defaultFiltLvl,
        int                            defaultFiltLvlR,
        int                            plane);

    uint32_t Av1GetQindex(
        CodecAv1SegmentsParams *segInfo,
        uint32_t               segment_id,
        uint8_t                base_qindex);

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

    void FreeResource();

    //! \brief   film grain output surface
    PDDI_MEDIA_SURFACE filmGrainOutSurface = nullptr;
    //! \brief   film grain output surface structure
    MOS_SURFACE outputSurface;

    MOS_SURFACE anchorFrameList[MAX_ANCHOR_FRAME_NUM_AV1];
    VASurfaceID anchorFrameListVA[MAX_ANCHOR_FRAME_NUM_AV1] = {0};

    MEDIA_CLASS_DEFINE_END(decode__DdiDecodeAv1)
};
} // namespace decode

#endif /* __DDI_DECODE_AV1_SPECIFIC_H__ */
