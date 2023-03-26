/*
* Copyright (c) 2020-2022, Intel Corporation
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
//! \file     decode_vp8_picture_packet.h
//! \brief    Defines the implementation of vp8 decode picture packet
//!

#ifndef __DECODE_VP8_PICTURE_PACKET_H__
#define __DECODE_VP8_PICTURE_PACKET_H__

#include "media_cmd_packet.h"
#include "decode_vp8_pipeline.h"
#include "decode_utils.h"
#include "decode_vp8_basic_feature.h"
#include "mhw_vdbox_mfx_itf.h"

using namespace mhw::vdbox::mfx;

namespace decode
{
    class Vp8DecodePicPkt : public DecodeSubPacket, public Itf::ParSetting
    {
    public:
        //!
        //! \brief  vp8DecodePicPkt constructor
        //!
        Vp8DecodePicPkt(Vp8Pipeline *pipeline, CodechalHwInterfaceNext *hwInterface)
            : DecodeSubPacket(pipeline, hwInterface), m_vp8Pipeline(pipeline)
        {
            if (m_hwInterface != nullptr)
            {
                m_mfxItf       = std::static_pointer_cast<Itf>(m_hwInterface->GetMfxInterfaceNext());
                m_miItf        = std::static_pointer_cast<mhw::mi::Itf>(hwInterface->GetMiInterfaceNext());
            }
        }

        //!
        //! \brief  Vp8DecodePicPkt deconstructor
        //!
        virtual ~Vp8DecodePicPkt();

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
        //! \brief  Execute vp8 picture packet
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

        enum CodechalDecodeRefAddrIndex
        {
            // VP8 reference address indexes
            CodechalDecodeLastRef      = 0,  //!< last reference
            CodechalDecodeGoldenRef    = 1,  //!< golden reference
            CodechalDecodeAlternateRef = 2   //!< alternate reference
        };

    protected:
        virtual MOS_STATUS SetRowstoreCachingOffsets();

        virtual MOS_STATUS SetRowStoreScratchBuffer();

        virtual MOS_STATUS SetSegmentationIdStreamBuffer();

        virtual MOS_STATUS AddMiForceWakeupCmd(MOS_COMMAND_BUFFER& cmdBuffer);

        MHW_SETPAR_DECL_HDR(MFX_PIPE_MODE_SELECT);
        MHW_SETPAR_DECL_HDR(MFX_SURFACE_STATE);
        MHW_SETPAR_DECL_HDR(MFX_PIPE_BUF_ADDR_STATE);
        MHW_SETPAR_DECL_HDR(MFX_IND_OBJ_BASE_ADDR_STATE);
        MHW_SETPAR_DECL_HDR(MFX_BSP_BUF_BASE_ADDR_STATE);
        MHW_SETPAR_DECL_HDR(MFX_VP8_PIC_STATE);
        MOS_STATUS AddAllCmds_MFX_PIPE_MODE_SELECT(MOS_COMMAND_BUFFER &cmdBuffer);

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
        MOS_STATUS DumpResources(MFX_PIPE_BUF_ADDR_STATE_PAR &pipeBufAddrParams) const;


        uint32_t m_pictureStatesSize      = 0;  //!< Picture state command size
        uint32_t m_picturePatchListSize   = 0;  //!< Picture patch list size

        // Internally maintained
        PMOS_BUFFER                 m_resBsdMpcRowStoreScratchBuffer                = nullptr;     //!< Graphics resource of BSD/MPC Row Store Scratch data surface
        PMOS_BUFFER                 m_resMprRowStoreScratchBuffer                   = nullptr;     //!< Graphics resource of MPR Row Store Scratch data surface
        PMOS_BUFFER                 m_resSegmentationIdStreamBuffer                 = nullptr;     //!< Graphics resource of Segmentation ID Stream data surface
        PMOS_BUFFER                 m_resMfdIntraRowStoreScratchBuffer              = nullptr;     //!< Graphics resource of MFD Intra Row Store Scratch data surface
        PMOS_BUFFER                 m_resMfdDeblockingFilterRowStoreScratchBuffer   = nullptr;     //!< Graphics resource of MFD Deblocking Filter Row Store Scratch data surface

        // Interfaces
        Vp8Pipeline                 *m_vp8Pipeline                                  = nullptr;
        CODEC_VP8_PIC_PARAMS        *m_vp8PicParams                                 = nullptr;      //!< Pointer to picture parameter
        Vp8BasicFeature             *m_vp8BasicFeature                              = nullptr;
        DecodeMemComp               *m_mmcState                                     = nullptr;
        DecodeAllocator             *m_allocator                                    = nullptr;

        std::shared_ptr<Itf>         m_mfxItf            = nullptr;
        
    MEDIA_CLASS_DEFINE_END(decode__Vp8DecodePicPkt)
 };

}  // namespace decode
#endif
