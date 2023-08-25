/*
* Copyright (c) 2021, Intel Corporation
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
//! \file     encode_jpeg_basic_feature.h
//! \brief    Defines the common interface for encode jpeg basic feature
//!
#ifndef __ENCODE_JPEG_BASIC_FEATURE_H__
#define __ENCODE_JPEG_BASIC_FEATURE_H__

#include "encode_basic_feature.h"
#include "encode_jpeg_reference_frames.h"
#include "mhw_vdbox_mfx_itf.h"
#include "mhw_mi_itf.h"
#include "encode_mem_compression.h"

namespace encode
{

//!
//! \enum     CodecSelect
//! \brief    Codec select
//!
enum CodecSelect
{
    decoderCodec    = 0,
    encoderCodec    = 1
};

class JpegBasicFeature : public EncodeBasicFeature, public mhw::vdbox::mfx::Itf::ParSetting, public mhw::mi::Itf::ParSetting
{
public:
    JpegBasicFeature(EncodeAllocator * allocator,
                    CodechalHwInterfaceNext *hwInterface,
                    TrackedBuffer *trackedBuf,
                    RecycleResource *recycleBuf) :
                    EncodeBasicFeature(allocator, hwInterface, trackedBuf, recycleBuf) {}

    virtual ~JpegBasicFeature() {}

    virtual MOS_STATUS Init(void *setting) override;

    virtual MOS_STATUS Update(void *params) override;

    virtual uint32_t GetProfileLevelMaxFrameSize() override { return 0; }

    uint32_t GetJpegHorizontalSamplingFactorForY(CodecEncodeJpegInputSurfaceFormat format) const;
    uint32_t GetJpegVerticalSamplingFactorForY(CodecEncodeJpegInputSurfaceFormat format) const;

    MHW_SETPAR_DECL_HDR(MFX_PIPE_MODE_SELECT);

    MHW_SETPAR_DECL_HDR(MFX_SURFACE_STATE);

    MHW_SETPAR_DECL_HDR(MFX_PIPE_BUF_ADDR_STATE);

    MHW_SETPAR_DECL_HDR(MFX_IND_OBJ_BASE_ADDR_STATE);

    MHW_SETPAR_DECL_HDR(MFX_JPEG_PIC_STATE);

    MHW_SETPAR_DECL_HDR(MFC_JPEG_SCAN_OBJECT);

    MHW_SETPAR_DECL_HDR(MI_FORCE_WAKEUP);

    MHW_SETPAR_DECL_HDR(MFX_WAIT);

    EncodeMemComp *m_mmcState = nullptr;

    // Parameters passed from application
    CodecEncodeJpegPictureParams                *m_jpegPicParams    = nullptr;      //!< Pointer to JPEG picture parameter
    CodecEncodeJpegScanHeader                   *m_jpegScanParams   = nullptr;      //!< Pointer to JPEG slice parameter
    CodecEncodeJpegQuantTable                   *m_jpegQuantTables  = nullptr;      //!< Pointer to IQMaxtrix parameter
    CodecEncodeJpegHuffmanDataArray             *m_jpegHuffmanTable = nullptr;      //!< Pointer to IQWidght ScaleLists

    std::shared_ptr<JpegReferenceFrames>        m_ref               = nullptr;      //! Reference List
    uint32_t                                    m_bitstreamUpperBound = 0;          //!< Bitstream upper bound

    // Other
    uint32_t                                    m_appDataSize          = 0;
    bool                                        m_jpegQuantMatrixSent  = false;      //!< bool to tell if quant matrix was sent by the app or not
    bool                                        m_fullHeaderInAppData  = false;
    uint32_t                                    m_numHuffBuffers       = 0;
    void                                        *m_huffmanTable        = nullptr;
    void                                        *m_applicationData     = nullptr;    //!< Pointer to Application data size

protected:

    //!
    //! \brief    Initialize reference frames class
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS InitRefFrames();

    virtual MOS_STATUS GetTrackedBuffers() override;

MEDIA_CLASS_DEFINE_END(encode__JpegBasicFeature)
};

}  // namespace encode

#endif  // !__ENCODE_JPEG_BASIC_FEATURE_H__
