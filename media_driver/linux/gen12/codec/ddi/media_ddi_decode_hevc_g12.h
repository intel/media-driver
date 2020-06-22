/*
* Copyright (c) 2015-2018, Intel Corporation
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
//! \file      media_ddi_decode_hevc_g12.h 
//! \brief     libva(and its extension) decoder implementation 
//!
//!
//! \file     media_ddi_decode_hevc_g12.h
//! \brief    Defines DdiDecodeHEVCG12 class for HEVC decode
//!

#ifndef _MEDIA_DDI_DECODE_HEVC_G12_H
#define _MEDIA_DDI_DECODE_HEVC_G12_H

#include "media_ddi_decode_hevc.h"

//!
//! \class  DdiDecodeHEVCG12
//! \brief  Ddi decode HEVC for gen12 specific
//!
class DdiDecodeHEVCG12 : public DdiDecodeHEVC
{
public:
    //!
    //! \brief Constructor
    //!
    DdiDecodeHEVCG12(DDI_DECODE_CONFIG_ATTR *ddiDecodeAttr) : DdiDecodeHEVC(ddiDecodeAttr){};

    //!
    //! \brief Destructor
    //!
    virtual ~DdiDecodeHEVCG12(){};

    virtual MOS_FORMAT GetFormat() override;

    virtual VAStatus CodecHalInit(
        DDI_MEDIA_CONTEXT *mediaCtx,
        void              *ptr) override;

    virtual VAStatus AllocSliceControlBuffer(
        DDI_MEDIA_BUFFER       *buf) override;

    virtual uint8_t* GetPicParamBuf(
    DDI_CODEC_COM_BUFFER_MGR      *bufMgr) override;

    virtual bool IsRextProfile() override;

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
    //!             uint32_t
    //!
    //! \return  VA_STATUS_SUCCESS is returned if it is parsed successfully.
    //!          else fail reason
    virtual VAStatus ParseSliceParams(
        DDI_MEDIA_CONTEXT           *mediaCtx,
        VASliceParameterBufferHEVC  *slcParam,
        uint32_t                     numSlices) override;

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
        VAPictureParameterBufferHEVC *picParam) override;

    //! \brief   Alloc SliceParam content for HEVC
    //! \details Alloc/resize SlicePram content for HEVC decoding
    //!
    //! \param   [in] numSlices
    //!          uint32_t the required number of slices
    //!
    //! \return  VA_STATUS_SUCCESS is returned if it is parsed successfully.
    //!          else fail reason
    virtual VAStatus AllocSliceParamContext(
        uint32_t numSlices)         override;

    //! \brief   Init Resource buffer for HEVC
    //! \details Initialize and allocate the Resource buffer for HEVC
    //!
    //! \return  VA_STATUS_SUCCESS is returned if it is parsed successfully.
    //!          else fail reason
    virtual VAStatus InitResourceBuffer() override;

    //! \brief   Free Resource buffer for HEVC
    //!
    virtual void FreeResourceBuffer() override;

    //!
    //! \brief    if it is  hevc scc profile
    //!
    //! \return   true or false
    //!
    bool IsSccProfile();
};

#endif /* _MEDIA_DDI_DECODE_HEVC_G12_H */
