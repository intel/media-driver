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
//! \file      media_ddi_decode_vc1.h 
//! \brief     libva(and its extension) decoder implementation  
//!

#ifndef __MEDIA_DDI_DECODER_VC1_H__
#define __MEDIA_DDI_DECODER_VC1_H__

#include <va/va.h>
#include "media_ddi_decode_base.h"

struct _DDI_MEDIA_BUFFER;

//!
//! \class  DdiDecodeVC1
//! \brief  Ddi decode VC1
//!
class DdiDecodeVC1 : public DdiMediaDecode
{
public:
    //!
    //! \brief Constructor
    //!
    DdiDecodeVC1(DDI_DECODE_CONFIG_ATTR *ddiDecodeAttr) : DdiMediaDecode(ddiDecodeAttr){};

    //!
    //! \brief Destructor
    //!
    virtual ~DdiDecodeVC1(){};

    // inherited virtual functions
    virtual VAStatus BeginPicture(
        VADriverContextP ctx,
        VAContextID      context,
        VASurfaceID      renderTarget) override;

    virtual void DestroyContext(
        VADriverContextP ctx) override;

    virtual VAStatus RenderPicture(
        VADriverContextP ctx,
        VAContextID      context,
        VABufferID       *buffers,
        int32_t          num_buffers) override;

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
     DDI_CODEC_COM_BUFFER_MGR    *bufMgr) override;
private:
    //!
    //! \brief   ParaSliceParam for VC1
    //! \details parse the sliceParam info required by VC1 decoding for
    //!          each slice
    //!
    //! \param   [in] *mediaCtx
    //!          DDI_MEDIA_CONTEXT
    //! \param   [in] *slcParam
    //!          VASliceParameterBufferVC1
    //! \param   [in] numSlices
    //!             int32_t
    //!
    //! \return  VA_STATUS_SUCCESS is returned if it is parsed successfully.
    //!          else fail reason
    VAStatus ParseSliceParams(
        DDI_MEDIA_CONTEXT          *mediaCtx,
        VASliceParameterBufferVC1  *slcParam,
        int32_t                    numSlices);

    //! \brief   ParsePicParam for VC1
    //! \details parse the PicParam info required by VC1 decoding
    //!
    //! \param   [in] *mediaCtx
    //!          DDI_MEDIA_CONTEXT
    //! \param   [in] *picParam
    //!          VAPictureParameterBufferVC1
    //!
    //! \return  VA_STATUS_SUCCESS is returned if it is parsed successfully.
    //!          else fail reason
    VAStatus ParsePicParams(
        DDI_MEDIA_CONTEXT           *mediaCtx,
        VAPictureParameterBufferVC1 *picParam);

    //! \brief   Alloc SliceParam content for VC1
    //! \details Alloc/resize SlicePram content for VC1 decoding
    //!
    //! \param   [in] numSlices
    //!          int32_t the required number of slices
    //!
    //! \return  VA_STATUS_SUCCESS is returned if it is parsed successfully.
    //!          else fail reason
    VAStatus AllocSliceParamContext(
        int32_t numSlices);

    //! \brief   Init Resource buffer for VC1
    //! \details Initialize and allocate the Resource buffer for VC1
    //!
    //! \return  VA_STATUS_SUCCESS is returned if it is parsed successfully.
    //!          else fail reason
    VAStatus InitResourceBuffer(DDI_MEDIA_CONTEXT *mediaCtx);

    //! \brief   Free Resource buffer for VC1
    //!
    void FreeResourceBuffer();

    //! \brief   Calculate the Quant Param from PicParam for VC1
    //! \details calculate and analyze the Quant Param from PicParam.
    //!          The Quant config/EdgeMask are calculated.
    //!
    //! \param   [in] *picParam
    //!          VAPictureParameterBufferVC1
    //!
    //! \param   [in] *altPquantConfig
    //!          uint32_t
    //!
    //! \param   [in] *altPquantEdgeMask
    //!          uint32_t
    void CalculateQuantParams(
        VAPictureParameterBufferVC1 *picParam,
        uint32_t                    *altPquantConfig,
        uint32_t                    *altPquantEdgeMask);

    //! \brief   Allocate the required BitPlane buffer
    VAStatus AllocBitPlaneBuffer();

    //! \brief   Parse BitPlane parameter for VC1
    //! \details Parse and Config the BitPlane buffer from the Input
    //!
    //! \param   [in] *bitPlaneBuffObject
    //!          struct _DDI_MEDIA_BUFFER
    //!          This is the destinatin buffer of Bitplane
    //!
    //! \param   [in] *buf
    //!          uint32_t
    //!
    void ParseBitPlane(
        struct _DDI_MEDIA_BUFFER *bitPlaneBuffObject,
        uint8_t                  *buf);

    //! \brief  the flag of OLPNeeded
    bool m_olpNeeded = false;

    //! \brief the Deblock Pic Idx for VC1
    uint32_t m_deblockPicIdx = 0xffffffff;

    //! \brief the current Pic Idx for VC1
    uint32_t m_currPicIdx = 0xffffffff;
    MOS_SURFACE m_deblockSurface;
};

#endif /* _MEDIA_DDI_DECODER_VC1_H */

