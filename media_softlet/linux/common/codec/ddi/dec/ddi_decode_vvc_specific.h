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
//! \file     ddi_decode_vvc_specific.h
//! \brief    Defines DdiDecodeVvc class for VVC decode
//!

#ifndef __DDI_DECODE_VVC_SPECIFIC_H__
#define __DDI_DECODE_VVC_SPECIFIC_H__

#include <va/va.h>
#include <va/va_dec_vvc.h>
#include "codec_def_decode_vvc.h"
#include "ddi_decode_base_specific.h"

#define DECODE_ID_VVC  "VIDEO_DEC_VVC"

enum VvcPicEntryType
{
    VvcPicEntryCurrFrame = 0,
    VvcPicEntryRefFrameList,
    VvcPicEntryRefPicList
};

typedef struct _DDI_CODEC_BUFFER_PARAM_VVC
{
    // one picture buffer
    VAPictureParameterBufferVVC   PicParamVVC;

    // slice param buffer pointer
    VASliceParameterBufferVVC     *pVASliceParameterBufferVVC; 
} DDI_CODEC_BUFFER_PARAM_VVC;

namespace decode
{

//!
//! \class  DdiDecodeVvc
//! \brief  Ddi Decode VVC
//!

class DdiDecodeVvc : public DdiDecodeBase
{
public:
    //!
    //! \brief Constructor
    //!
    DdiDecodeVvc() : DdiDecodeBase() 
    {
        subpic_buffer_nums       = 0;
        slice_struct_nums        = 0;
        tile_buffer_nums         = 0;
        alf_buffer_nums          = 0;
        lmcs_buffer_nums         = 0;
        scaling_list_buffer_nums = 0;
    };

    //!
    //! \brief Destructor
    //!
    virtual ~DdiDecodeVvc() {};

    // inherited virtual functions
    virtual void DestroyContext(
        VADriverContextP ctx) override;

    virtual VAStatus RenderPicture(
        VADriverContextP ctx,
        VAContextID      context,
        VABufferID       *buffers,
        int32_t          numBuffers) override;

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

    virtual VAStatus CreateBuffer(
        VABufferType type,
        uint32_t     size,
        uint32_t     numElements,
        void         *data,
        VABufferID   *bufId) override;
        
    VAStatus ParseAlfDatas(
        DDI_DECODE_CONTEXT *decodeCtx,
        VAAlfDataVVC       *alfDatas,
        uint32_t           numAlfDatas,
        uint32_t           numAlfBuffers);
    VAStatus ParseLmcsDatas(
        DDI_DECODE_CONTEXT  *decodeCtx,
        VALmcsDataVVC       *LmcsDatas,     
        uint32_t            numLmcsDatas,
        uint32_t            numLMCSBuffers);
    VAStatus ParseWeightedPredInfo(
        CodecVvcSliceParams*      sliceParams,
        VAWeightedPredInfo*       wpInfoParams);

    uint32_t    subpic_buffer_nums;
    uint32_t    slice_struct_nums;
    uint32_t    tile_buffer_nums;
    uint32_t    alf_buffer_nums;
    uint32_t    lmcs_buffer_nums;
    uint32_t    scaling_list_buffer_nums;
private:
    //!
    //! \brief   ParseSliceParam for VVC
    //! \details parse the sliceParam info required by VVC decoding for
    //!          each slice
    //!
    //! \param   [in] *mediaCtx
    //!          DDI_MEDIA_CONTEXT
    //! \param   [in] *slcParam
    //!          VASliceParameterBufferVVC
    //! \param   [in] numSlices
    //!             uint32_t
    //!
    //! \return  VA_STATUS_SUCCESS is returned if it is parsed successfully.
    //!          else fail reason
    virtual VAStatus ParseSliceParams(
        DDI_MEDIA_CONTEXT           *mediaCtx,
        VASliceParameterBufferVVC   *slcParam,
        uint32_t                    numSlices);

    //! \brief   ParsePicParam for VVC
    //! \details parse the PicParam info required by VVC decoding
    //!
    //! \param   [in] *mediaCtx
    //!          DDI_MEDIA_CONTEXT
    //! \param   [in] *picParam
    //!          VAPictureParameterBufferVVC
    //!
    //! \return  VA_STATUS_SUCCESS is returned if it is parsed successfully.
    //!          else fail reason
    virtual VAStatus ParsePicParams(
        DDI_MEDIA_CONTEXT           *mediaCtx,
        VAPictureParameterBufferVVC *picParam);

    //! \brief   ParseSubPicParam for VVC
    //! \details parse the SubPicParam info required by VVC decoding
    //!
    //! \param   [in] *mediaCtx
    //!          DDI_MEDIA_CONTEXT
    //! \param   [in] *subPicParam
    //!          VASubPicVVC
    //! \param   [in] numSubPics
    //!          uint32_t
    //!
    //! \return  VA_STATUS_SUCCESS is returned if it is parsed successfully.
    //!          else fail reason
    virtual VAStatus ParseSubPicParams(
        DDI_MEDIA_CONTEXT *mediaCtx,
        VASubPicVVC       *subPicParam,
        uint32_t          numSubPics,
        uint32_t          numSubPicbuffers);

    //! \brief   ParseTileParam for VVC
    //! \details parse the TileParam info required by VVC decoding
    //!
    //! \param   [in] *mediaCtx
    //!          DDI_MEDIA_CONTEXT
    //! \param   [in] *tileParam
    //!          VATileBufferVVC
    //! \param   [in] numTiles
    //!          uint32_t
    //!
    //! \return  VA_STATUS_SUCCESS is returned if it is parsed successfully.
    //!          else fail reason
    virtual VAStatus ParseTileParams(
        DDI_MEDIA_CONTEXT *mediaCtx,
        uint16_t          *tileParam,
        uint32_t          numTiles,
        uint32_t          numTileBuffers);

    //! \brief   ParseSliceStructParam for VVC
    //! \details parse the SliceStructParam info required by VVC decoding
    //!
    //! \param   [in] *mediaCtx
    //!          DDI_MEDIA_CONTEXT
    //! \param   [in] *sliceStructParam
    //!          VASliceStructVVC
    //! \param   [in] numSliceStructs
    //!          uint32_t
    //!
    //! \return  VA_STATUS_SUCCESS is returned if it is parsed successfully.
    //!          else fail reason
    virtual VAStatus ParseSliceStructParams(
        DDI_MEDIA_CONTEXT *mediaCtx,
        VASliceStructVVC  *sliceStructParam,
        uint32_t          numSliceStructs,
        uint32_t          slice_struct_nums);

    //! \brief   Init Resource buffer for VVC
    //! \details Initialize and allocate the Resource buffer for VVC
    //!
    //! \return  VA_STATUS_SUCCESS is returned if it is parsed successfully.
    //!          else fail reason
    VAStatus InitResourceBuffer();

    //! \brief   Free Resource buffer for VVC
    //!
    void FreeResourceBuffer();

    void FreeResource();

    //! \brief    Setup Codec Picture for VVC
    //!
    //! \param    [in] mediaCtx
    //!           Pointer to DDI_MEDIA_CONTEXT
    //! \param    [in] rtTbl
    //!           Pointer to DDI_CODEC_RENDER_TARGET_TABLE
    //! \param    [in] vaPic
    //!           VVC VAPicture structure
    //! \param    [in] bSurfaceType
    //!           VVC Picture entry type
    //! \param    [out] pCodecHalPic
    //!           Pointer to CODEC_PICTURE
    //!
    //! \return   void
    //!
    void SetupCodecPicture(
        DDI_MEDIA_CONTEXT             *mediaCtx,
        DDI_CODEC_RENDER_TARGET_TABLE *rtTbl,
        PCODEC_PICTURE                pCodecHalPic,
        VAPictureVVC                  vaPic,
        VvcPicEntryType               bSurfaceType);

    //! \brief   flag list for VVC reference surfaces
    uint32_t m_refListFlags[vvcMaxNumRefFrame];

    MEDIA_CLASS_DEFINE_END(decode__DdiDecodeVvc)
};
}; // namespace decode

#endif /* __DDI_DECODE_VVC_SPECIFIC_H__ */
