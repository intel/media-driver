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
//! \file     decode_av1_picture_packet.h
//! \brief    Defines the implementation of av1 decode picture packet
//!

#ifndef __DECODE_AV1_PICTURE_PACKET_H__
#define __DECODE_AV1_PICTURE_PACKET_H__

#include "media_cmd_packet.h"
#include "decode_av1_pipeline.h"
#include "decode_utils.h"
#include "decode_av1_basic_feature.h"
#include "mhw_vdbox_avp_itf.h"

using namespace mhw::vdbox::avp;
namespace decode
{
    class Av1DecodePicPkt : public DecodeSubPacket , public mhw::vdbox::avp::Itf::ParSetting
    {
    public:
        //!
        //! \brief  Av1DecodePicPkt constructor
        //!
        Av1DecodePicPkt(Av1Pipeline *pipeline, CodechalHwInterfaceNext*hwInterface)
            : DecodeSubPacket(pipeline, hwInterface), m_av1Pipeline(pipeline)
        {
            if (m_hwInterface != nullptr)
            {
                m_avpItf = std::static_pointer_cast<mhw::vdbox::avp::Itf>(m_hwInterface->GetAvpInterfaceNext());
                m_miItf  = std::static_pointer_cast<mhw::mi::Itf>(m_hwInterface->GetMiInterfaceNext());
            }
        }

        //!
        //! \brief  Av1DecodePicPkt deconstructor
        //!
        virtual ~Av1DecodePicPkt();

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
        //! \brief  Init av1 state commands
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        virtual MOS_STATUS InitAv1State(MOS_COMMAND_BUFFER& cmdBuffer) { return MOS_STATUS_SUCCESS; };

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
        virtual MOS_STATUS AllocateVariableResources();

        virtual MOS_STATUS AddAllCmds_AVP_SURFACE_STATE(MOS_COMMAND_BUFFER& cmdBuffer);
        MOS_STATUS AddAllCmds_AVP_PIPE_MODE_SELECT(MOS_COMMAND_BUFFER& cmdBuffer);
        MOS_STATUS AddAllCmds_AVP_SEGMENT_STATE(MOS_COMMAND_BUFFER& cmdBuffer);
        MOS_STATUS SetSkipModeFrameParam() const;
        MOS_STATUS SetRefPicStateParam() const;
        MOS_STATUS RefAddrErrorConcel() const;
        MOS_STATUS GetSurfaceMmcInfo(PMOS_SURFACE surface, MOS_MEMCOMP_STATE& mmcState, uint32_t& compressionFormat) const;
        virtual MOS_STATUS GetChromaFormat();

        MHW_SETPAR_DECL_HDR(AVP_PIPE_MODE_SELECT);
        MHW_SETPAR_DECL_HDR(AVP_PIC_STATE);
        MHW_SETPAR_DECL_HDR(AVP_IND_OBJ_BASE_ADDR_STATE);
        MHW_SETPAR_DECL_HDR(AVP_PIPE_BUF_ADDR_STATE);
        MHW_SETPAR_DECL_HDR(AVP_INTER_PRED_STATE);
        MHW_SETPAR_DECL_HDR(AVP_SURFACE_STATE);

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

#if USE_CODECHAL_DEBUG_TOOL
        //!
        //! \brief  Dump resources
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        MOS_STATUS DumpResources(uint32_t refSize) const;
#endif

        //Interfaces
        Av1Pipeline                *m_av1Pipeline     = nullptr;
        CodechalHwInterfaceNext    *m_hwInterfaceNext = nullptr;
        Av1BasicFeature            *m_av1BasicFeature = nullptr;
        DecodeAllocator            *m_allocator       = nullptr;
        DecodeMemComp              *m_mmcState        = nullptr;
        std::shared_ptr<mhw::vdbox::avp::Itf> m_avpItf   = nullptr;

        CodecAv1PicParams          *m_av1PicParams    = nullptr; //!< Pointer to picture parameter
        MOS_SURFACE                 m_refSurface[av1TotalRefsPerFrame];
        MOS_MEMCOMP_STATE           m_refMmcState[av1TotalRefsPerFrame] = { MOS_MEMCOMP_DISABLED };
        uint32_t                    m_refCompressionFormat = 0;
        PMOS_SURFACE    m_intrabcDecodedOutputFrameBuffer                       = nullptr;    //!< IntraBC Decoded output frame buffer

        //non-temporal buffers
        PMOS_BUFFER     m_bitstreamDecoderEncoderLineRowstoreReadWriteBuffer     = nullptr;    //!< Handle of Bitstream Decode Line Rowstore buffer, can be programmed to use Local Media Storage VMM instead of Memory 
        PMOS_BUFFER     m_bitstreamDecoderEncoderTileLineRowstoreReadWriteBuffer = nullptr;    //!< Handle of Bitstream Decode Tile Line buffer
        PMOS_BUFFER     m_intraPredictionLineRowstoreReadWriteBuffer             = nullptr;    //!< Handle of Intra Prediction Line Rowstore Read/Write Buffer
        PMOS_BUFFER     m_intraPredictionTileLineRowstoreReadWriteBuffer         = nullptr;    //!< Handle of Intra Prediction Tile Line Rowstore Read/Write Buffer 
        PMOS_BUFFER     m_spatialMotionVectorLineReadWriteBuffer                 = nullptr;    //!< Handle of Spatial Motion Vector Line rowstore buffer, can be programmed to use Local Media Storage VMM instead of Memory
        PMOS_BUFFER     m_spatialMotionVectorCodingTileLineReadWriteBuffer       = nullptr;    //!< Handle of Spatial Motion Vector Tile Line buffer
        PMOS_BUFFER     m_loopRestorationMetaTileColumnReadWriteBuffer           = nullptr;    //!< DW80..81, Loop Restoration Meta Tile Column Read/Write Buffer Address
        PMOS_BUFFER     m_loopRestorationFilterTileReadWriteLineYBuffer          = nullptr;    //!< DW83..84, Loop Restoration Filter Tile Read/Write Line Y Buffer Address
        PMOS_BUFFER     m_loopRestorationFilterTileReadWriteLineUBuffer          = nullptr;    //!< DW86..87, Loop Restoration Filter Tile Read/Write Line U Buffer Address
        PMOS_BUFFER     m_loopRestorationFilterTileReadWriteLineVBuffer          = nullptr;    //!< DW89..90, Loop Restoration Filter Tile Read/Write Line V Buffer Address
        PMOS_BUFFER     m_deblockerFilterLineReadWriteYBuffer                    = nullptr;    //!< DW92..93, Deblocker Filter Line Read/Write Y Buffer Address
        PMOS_BUFFER     m_deblockerFilterLineReadWriteUBuffer                    = nullptr;    //!< DW95..96, Deblocker Filter Line Read/Write U Buffer Address
        PMOS_BUFFER     m_deblockerFilterLineReadWriteVBuffer                    = nullptr;    //!< DW98..99, Deblocker Filter Line Read/Write V Buffer Address
        PMOS_BUFFER     m_deblockerFilterTileLineReadWriteYBuffer                = nullptr;    //!< DW101..102, Deblocker Filter Tile Line Read/Write Y Buffer Address
        PMOS_BUFFER     m_deblockerFilterTileLineReadWriteVBuffer                = nullptr;    //!< DW104..105, Deblocker Filter Tile Line Read/Write V Buffer Address
        PMOS_BUFFER     m_deblockerFilterTileLineReadWriteUBuffer                = nullptr;    //!< DW107..108, Deblocker Filter Tile Line Read/Write U Buffer Address
        PMOS_BUFFER     m_deblockerFilterTileColumnReadWriteYBuffer              = nullptr;    //!< DW110..111, Deblocker Filter Tile Column Read/Write Y Buffer Address
        PMOS_BUFFER     m_deblockerFilterTileColumnReadWriteUBuffer              = nullptr;    //!< DW113..114, Deblocker Filter Tile Column Read/Write U Buffer Address
        PMOS_BUFFER     m_deblockerFilterTileColumnReadWriteVBuffer              = nullptr;    //!< DW116..117, Deblocker Filter Tile Column Read/Write V Buffer Address
        PMOS_BUFFER     m_cdefFilterLineReadWriteBuffer                          = nullptr;    //!< DW119..120, CDEF Filter Line Read/Write Y Buffer Address
        PMOS_BUFFER     m_cdefFilterTileLineReadWriteBuffer                      = nullptr;    //!< DW128..129, CDEF Filter Tile Line Read/Write Y Buffer Address
        PMOS_BUFFER     m_cdefFilterTileColumnReadWriteBuffer                    = nullptr;    //!< DW137..138, CDEF Filter Tile Column Read/Write Y Buffer Address
        PMOS_BUFFER     m_cdefFilterMetaTileLineReadWriteBuffer                  = nullptr;    //!< DW140..141, CDEF Filter Meta Tile Line Read/Write Buffer Address
        PMOS_BUFFER     m_cdefFilterMetaTileColumnReadWriteBuffer                = nullptr;    //!< DW143..144, CDEF Filter Meta Tile Column Read/Write Buffer Address    
        PMOS_BUFFER     m_cdefFilterTopLeftCornerReadWriteBuffer                 = nullptr;    //!< DW146..147, CDEF Filter Top-Left Corner Read/Write Buffer Address
        PMOS_BUFFER     m_superResTileColumnReadWriteYBuffer                     = nullptr;    //!< DW149..150, Super-Res Tile Column Read/Write Y Buffer Address
        PMOS_BUFFER     m_superResTileColumnReadWriteUBuffer                     = nullptr;    //!< DW152..153, Super-Res Tile Column Read/Write U Buffer Address
        PMOS_BUFFER     m_superResTileColumnReadWriteVBuffer                     = nullptr;    //!< DW155..156, Super-Res Tile Column Read/Write V Buffer Address
        PMOS_BUFFER     m_loopRestorationFilterTileColumnReadWriteYBuffer        = nullptr;    //!< DW158..159, Loop Restoration Filter Tile Column Read/Write Y Buffer Address
        PMOS_BUFFER     m_loopRestorationFilterTileColumnReadWriteUBuffer        = nullptr;    //!< DW161..162, Loop Restoration Filter Tile Column Read/Write U Buffer Address
        PMOS_BUFFER     m_loopRestorationFilterTileColumnReadWriteVBuffer        = nullptr;    //!< DW164..165, Loop Restoration Filter Tile Column Read/Write V Buffer Address

        //streamout buffer
        PMOS_BUFFER     m_decodedFrameStatusErrorBuffer                          = nullptr;    //!< DW176..177, Decoded Frame Status/Error Buffer Base Address
        PMOS_BUFFER     m_decodedBlockDataStreamoutBuffer                        = nullptr;    //!< DW179..180, Decoded Block Data Streamout Buffer Address

        PMOS_BUFFER     m_resMfdDeblockingFilterRowStoreScratchBuffer            = nullptr;    //!< Handle of MFD Deblocking Filter Row Store Scratch data surface
        PMOS_BUFFER     m_resDeblockingFilterTileRowStoreScratchBuffer           = nullptr;    //!< Handle of Deblocking Filter Tile Row Store Scratch data surface
        PMOS_BUFFER     m_resDeblockingFilterColumnRowStoreScratchBuffer         = nullptr;    //!< Handle of Deblocking Filter Column Row Store Scratch data surface
        PMOS_BUFFER     m_resMetadataLineBuffer                                  = nullptr;    //!< Handle of Metadata Line data buffer
        PMOS_BUFFER     m_resMetadataTileLineBuffer                              = nullptr;    //!< Handle of Metadata Tile Line data buffer
        PMOS_BUFFER     m_resMetadataTileColumnBuffer                            = nullptr;    //!< Handle of Metadata Tile Column data buffer
        PMOS_BUFFER     m_resSaoLineBuffer                                       = nullptr;    //!< Handle of SAO Line data buffer
        PMOS_BUFFER     m_resSaoTileLineBuffer                                   = nullptr;    //!< Handle of SAO Tile Line data buffer
        PMOS_BUFFER     m_resSaoTileColumnBuffer                                 = nullptr;    //!< Handle of SAO Tile Column data buffer

        PMOS_BUFFER     m_filmGrainTileColumnDataBuf = nullptr;         //!< Film Grain tile column data read/write buffer
        PMOS_BUFFER     m_filmGrainSampleTemplateBuf = nullptr;         //!< Film Grain sample template buffer
        PMOS_BUFFER     m_loopRestorationFilterTileColumnAlignmentBuf = nullptr;    //!< Loop restoration filter tile column alignment read/write buffer

        uint32_t        m_prevFrmWidth         = 0;    //!< Frame width of the previous frame
        uint32_t        m_prevFrmHeight        = 0;    //!< Frame height of the previous frame
        uint32_t        m_pictureStatesSize    = 0;    //!< Picture states size
        uint32_t        m_picturePatchListSize = 0;    //!< Picture patch list size
        uint16_t        chromaSamplingFormat   = 0;    //!< Chroma sampling fromat
        uint32_t        m_widthInSb            = 0;    //!< Width in unit of SB
        uint32_t        m_heightInSb           = 0;    //!< Height in unit of SB

        static const uint32_t m_av1ScalingFactor = (1 << 14);    //!< AV1 Scaling factor
        mutable uint8_t       m_curAvpSurfStateId = 0;

    MEDIA_CLASS_DEFINE_END(decode__Av1DecodePicPkt)
    };

}  // namespace decode
#endif
