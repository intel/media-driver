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
//! \file     decode_jpeg_picture_xe_m_base_packet.h
//! \brief    Defines the implementation of jpeg decode picture packet
//!

#ifndef __DECODE_JPEG_PICTURE_XE_M_BASE_PACKET_H__
#define __DECODE_JPEG_PICTURE_XE_M_BASE_PACKET_H__

#include "media_cmd_packet.h"
#include "decode_jpeg_pipeline.h"
#include "decode_utils.h"
#include "decode_jpeg_basic_feature.h"
#include "decode_downsampling_packet.h"
#include "mhw_vdbox_g12_X.h"
#include "codechal_hw_g12_X.h"

namespace decode{
class JpegDecodePicPktXe_M_Base : public DecodeSubPacket
{
public:
    //!
    //! \brief  JpegDecodePicPkt constructor
    //!
    JpegDecodePicPktXe_M_Base(JpegPipeline *pipeline, CodechalHwInterface *hwInterface)
        : DecodeSubPacket(pipeline, *hwInterface), m_jpegPipeline(pipeline)
    {
        m_hwInterface = hwInterface;
        if (m_hwInterface != nullptr)
        {
            m_miInterface  = m_hwInterface->GetMiInterface();
            m_osInterface  = m_hwInterface->GetOsInterface();
            m_mfxInterface  =  static_cast<CodechalHwInterfaceG12*>(hwInterface)->GetMfxInterface();
        }
    }

    //!
    //! \brief  JpegDecodePicPkt deconstructor
    //!
    virtual ~JpegDecodePicPktXe_M_Base(){};

    //!
    //! \brief  Initialize the media packet, allocate required resources
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Init() override;

    //!
    //! \brief  Prepare interal parameters, should be invoked for each frame
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Prepare() override;

    //!
    //! \brief  Execute av1 picture packet
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Execute(MOS_COMMAND_BUFFER& cmdBuffer) = 0;

    //!
    //! \brief  Calculate Command Size
    //!
    //! \param  [in, out] commandBufferSize
    //!         requested size
    //! \param  [in, out] requestedPatchListSize
    //!         requested size
    //! \return MOS_STATUS
    //!         status
    //!
    MOS_STATUS CalculateCommandSize(
        uint32_t &commandBufferSize,
        uint32_t &requestedPatchListSize) override;

protected:

    virtual MOS_STATUS AllocateFixedResources();

    virtual MOS_STATUS SetMfxSurfaceParams(MHW_VDBOX_SURFACE_PARAMS &dstSurfaceParams);
    virtual MOS_STATUS SetMfxPipeBufAddrParams(MHW_VDBOX_PIPE_BUF_ADDR_PARAMS &pipeBufAddrParams);
    virtual void       SetMfxPipeModeSelectParams(MHW_VDBOX_PIPE_MODE_SELECT_PARAMS_G12 &vdboxPipeModeSelectParams);
    virtual void       SetMfxIndObjBaseAddrParams(MHW_VDBOX_IND_OBJ_BASE_ADDR_PARAMS &indObjBaseAddrParams);
    virtual void       SetMfxJpegPicStateParams(MHW_VDBOX_JPEG_DECODE_PIC_STATE &jpegPicState);
    virtual void       SetMfxQmParams(MHW_VDBOX_QM_PARAMS &qmParams);

    virtual MOS_STATUS AddMfxSurfacesCmd(MOS_COMMAND_BUFFER &cmdBuffer);
    virtual MOS_STATUS AddMfxIndObjBaseAddrCmd(MOS_COMMAND_BUFFER &cmdBuffer);
    virtual MOS_STATUS AddMfxJpegHuffTableCmd(MOS_COMMAND_BUFFER &cmdBuffer);
    virtual MOS_STATUS AddMfxJpegPicCmd(MOS_COMMAND_BUFFER &cmdBuffer);
    virtual MOS_STATUS AddMfxQmCmd(MOS_COMMAND_BUFFER &cmdBuffer);
    virtual MOS_STATUS AddMfxBsdObjectParams(MOS_COMMAND_BUFFER &cmdBuffer);

    virtual MOS_STATUS AddMfxPipeBufAddrCmd(MOS_COMMAND_BUFFER &cmdBuffer) = 0;
    virtual MOS_STATUS AddMfxPipeModeSelectCmd(MOS_COMMAND_BUFFER &cmdBuffer) = 0;
    //!
    //! \brief  Free resources
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS FreeResources();

    //!
    //! \brief  Dump resources
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS DumpResources(MHW_VDBOX_PIPE_BUF_ADDR_PARAMS& pipeBufAddrParams);
#ifdef _DECODE_PROCESSING_SUPPORTED
    DecodeDownSamplingFeature *m_downSamplingFeature = nullptr;
    DecodeDownSamplingPkt *    m_downSamplingPkt     = nullptr;
#endif
    //Interfaces
    JpegPipeline               *m_jpegPipeline     = nullptr;
    MhwVdboxMfxInterface        *m_mfxInterface      = nullptr;
    JpegBasicFeature           *m_jpegBasicFeature = nullptr;
    DecodeAllocator             *m_allocator         = nullptr;
    DecodeMemComp               *m_mmcState          = nullptr;

    CodecDecodeJpegPicParams   *m_jpegPicParams    = nullptr; //!< Pointer to picture parameter

    uint32_t m_pictureStatesSize           = 0;    //!< Picture states size
    uint32_t m_picturePatchListSize        = 0;    //!< Picture patch list size
    CodechalHwInterface *m_hwInterface                 = nullptr;
    MhwMiInterface      *m_miInterface                 = nullptr;
    MEDIA_CLASS_DEFINE_END(decode__JpegDecodePicPktXe_M_Base)
};

}  // namespace decode
#endif