/*
* Copyright (c) 2021-2022, Intel Corporation
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
//! \file     decode_avc_picture_packet.h
//! \brief    Defines the implementation of avc decode picture packet
//!

#ifndef __DECODE_AVC_PICTURE_PACKET_H__
#define __DECODE_AVC_PICTURE_PACKET_H__

#include "media_cmd_packet.h"
#include "decode_avc_pipeline.h"
#include "decode_utils.h"
#include "decode_avc_basic_feature.h"
#include "decode_downsampling_packet.h"
#include "mhw_vdbox_mfx_itf.h"

using namespace mhw::vdbox::mfx;
namespace decode
{
class AvcDecodePicPkt : public DecodeSubPacket, public mhw::vdbox::mfx::Itf::ParSetting
{
public:
    //!
    //! \brief  AvcDecodePicPkt constructor
    //!
    AvcDecodePicPkt(AvcPipeline *pipeline, CodechalHwInterfaceNext*hwInterface)
        : DecodeSubPacket(pipeline, hwInterface), m_avcPipeline(pipeline)
    {
        if (m_hwInterface != nullptr)
        {
            m_mfxItf       = std::static_pointer_cast<mhw::vdbox::mfx::Itf>(m_hwInterface->GetMfxInterfaceNext());
            m_miItf        = std::static_pointer_cast<mhw::mi::Itf>(hwInterface->GetMiInterfaceNext());
        }
    }

    //!
    //! \brief  AvcDecodePicPkt deconstructor
    //!
    virtual ~AvcDecodePicPkt();

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
    virtual MOS_STATUS Execute(MOS_COMMAND_BUFFER &cmdBuffer) = 0;

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

    MOS_STATUS AddAllCmds_MFX_QM_STATE(PMOS_COMMAND_BUFFER cmdBuffer);

protected:
    MHW_SETPAR_DECL_HDR(MFX_PIPE_MODE_SELECT);
    MHW_SETPAR_DECL_HDR(MFX_SURFACE_STATE);
    MHW_SETPAR_DECL_HDR(MFX_PIPE_BUF_ADDR_STATE);
    MHW_SETPAR_DECL_HDR(MFX_IND_OBJ_BASE_ADDR_STATE);
    MHW_SETPAR_DECL_HDR(MFX_BSP_BUF_BASE_ADDR_STATE);
    MHW_SETPAR_DECL_HDR(MFX_AVC_IMG_STATE);
    MHW_SETPAR_DECL_HDR(MFX_AVC_DIRECTMODE_STATE);
    MHW_SETPAR_DECL_HDR(MFD_AVC_PICID_STATE);
    MHW_SETPAR_DECL_HDR(MFD_AVC_DPB_STATE);

    virtual MOS_STATUS AllocateFixedResources();
    virtual MOS_STATUS AllocateVariableResources();

    MOS_STATUS FixMfxPipeBufAddrParams() const;

    MOS_STATUS SetResAvcDmvBuffers();
    //! \brief    Set Rowstore Cache offset
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetRowstoreCachingOffsets();

    //!
    //! \brief  Free resources
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS FreeResources();

    MOS_STATUS SetSurfaceMmcState() const;

    //!
    //! \brief  Dump resources
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS DumpResources(uint32_t mvBufferSize) const;

#ifdef _DECODE_PROCESSING_SUPPORTED
    DecodeDownSamplingFeature *m_downSamplingFeature = nullptr;
    DecodeDownSamplingPkt *    m_downSamplingPkt     = nullptr;
#endif

    //Interfaces
    AvcPipeline *                         m_avcPipeline     = nullptr;
    AvcBasicFeature *                     m_avcBasicFeature = nullptr;
    DecodeAllocator *                     m_allocator       = nullptr;
    DecodeMemComp *                       m_mmcState        = nullptr;
    AvcReferenceFrames                    m_refFrames       = {};
    std::shared_ptr<mhw::vdbox::mfx::Itf> m_mfxItf          = nullptr;

    CODEC_AVC_PIC_PARAMS *m_avcPicParams = nullptr;  //!< Pointer to picture parameter
    MOS_RESOURCE          m_resAvcDmvBuffers[CODEC_AVC_NUM_DMV_BUFFERS];

    //non-temporal buffers
    PMOS_BUFFER m_resMfdDeblockingFilterRowStoreScratchBuffer = nullptr;  //!< Handle of MFD Deblocking Filter Row Store Scratch data surface
    PMOS_BUFFER m_resMfdIntraRowStoreScratchBuffer            = nullptr;  //!< Handle of MFD Intra Row Store Scratch data surface
    PMOS_BUFFER m_resBsdMpcRowStoreScratchBuffer              = nullptr;  //!< Handle of BSD/MPC Row Store Scratch data surface
    PMOS_BUFFER m_resMprRowStoreScratchBuffer                 = nullptr;  //!< Handle of MPR Row Store Scratch data surface

    uint32_t m_pictureStatesSize           = 0;  //!< Picture states size
    uint32_t m_picturePatchListSize        = 0;  //!< Picture patch list size
    uint16_t m_picWidthInMbLastMaxAlloced  = 0;  //!< Max Picture Width in MB  used for buffer allocation in past frames
    uint16_t m_picHeightInMbLastMaxAlloced = 0;  //!< Max Picture Height in MB used for buffer allocation in past frames

    enum AvcDecoderFormatMode
    {
        shortFormatMode = 0,
        longFormatMode  = 1
    };

MEDIA_CLASS_DEFINE_END(decode__AvcDecodePicPkt)
};
}  // namespace decode
#endif
