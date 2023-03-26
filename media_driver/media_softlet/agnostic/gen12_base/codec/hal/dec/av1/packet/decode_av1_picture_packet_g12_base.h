/*
* Copyright (c) 2019-2021, Intel Corporation
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
//! \file     decode_av1_picture_packet_g12_base.h
//! \brief    Defines the implementation of av1 decode picture packet
//!

#ifndef __DECODE_AV1_PICTURE_PACKET_G12_BASE_H__
#define __DECODE_AV1_PICTURE_PACKET_G12_BASE_H__

#include "media_cmd_packet.h"
#include "decode_av1_pipeline_g12_base.h"
#include "decode_utils.h"
#include "decode_av1_basic_feature_g12.h"

namespace decode
{
    class Av1DecodePicPkt_G12_Base : public DecodeSubPacket
    {
    public:
        //!
        //! \brief  Av1DecodePicPkt_G12_Base constructor
        //!
        Av1DecodePicPkt_G12_Base(Av1PipelineG12_Base *pipeline, CodechalHwInterface *hwInterface)
            : DecodeSubPacket(pipeline, *hwInterface), m_av1Pipeline(pipeline)
        {
            m_hwInterface = hwInterface;
            if (m_hwInterface != nullptr)
            {
                m_miInterface  = m_hwInterface->GetMiInterface();
                m_osInterface  = m_hwInterface->GetOsInterface();
                m_avpInterface = hwInterface->GetAvpInterface();
            }
        }

        //!
        //! \brief  Av1DecodePicPkt_G12_Base deconstructor
        //!
        virtual ~Av1DecodePicPkt_G12_Base();

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

        MOS_STATUS UpdatePipeBufAddrForDummyWL(MOS_COMMAND_BUFFER &cmdBuffer);

        MOS_STATUS UpdateIndObjAddrForDummyWL(MOS_COMMAND_BUFFER &cmdBuffer);

    protected:

        virtual MOS_STATUS AllocateFixedResources();
        virtual MOS_STATUS AllocateVariableResources();

        virtual MOS_STATUS SetAvpDstSurfaceParams(MHW_VDBOX_SURFACE_PARAMS &dstSurfaceParams);
        virtual MOS_STATUS SetAvpRefSurfaceParams(MHW_VDBOX_SURFACE_PARAMS *refSurfaceParams);
        virtual MOS_STATUS SetAvpIntraBCSurfaceParams(MHW_VDBOX_SURFACE_PARAMS &intraBCSurfaceParams);
        virtual MOS_STATUS AddAvpSurfacesCmd(MOS_COMMAND_BUFFER &cmdBuffer);

        virtual MOS_STATUS SetAvpSegmentStateParams(MhwVdboxAvpSegmentStateParams& segStateParams);
        virtual MOS_STATUS AddAvpSegmentStateCmd(MOS_COMMAND_BUFFER &cmdBuffer);

        MOS_STATUS         FixAvpPipeBufAddrParams(MhwVdboxAvpPipeBufAddrParams &pipeBufAddrParams);
        virtual MOS_STATUS SetAvpPipeBufAddrParams(MhwVdboxAvpPipeBufAddrParams &pipeBufAddrParams);
        virtual MOS_STATUS AddAvpPipeBufAddrCmd(MOS_COMMAND_BUFFER &cmdBuffer) = 0;

        MOS_STATUS SetAvpPipeBufAddrParamsForDummyWL(MhwVdboxAvpPipeBufAddrParams& pipeBufAddrParams);

        virtual MOS_STATUS AddAvpIndObjBaseAddrCmd(MOS_COMMAND_BUFFER &cmdBuffer);
        virtual void       SetAvpIndObjBaseAddrParams(MHW_VDBOX_IND_OBJ_BASE_ADDR_PARAMS &indObjBaseAddrParams);

        virtual MOS_STATUS AddAvpPipeModeSelectCmd(MOS_COMMAND_BUFFER &cmdBuffer) = 0;

        virtual MOS_STATUS SetAvpPicStateParams(MhwVdboxAvpPicStateParams &picStateParams);
        virtual MOS_STATUS SetAvpInterPredStateParams(MhwVdboxAvpPicStateParams &picStateParams);

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

        //!
        //! \brief  Dump resources
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        MOS_STATUS DumpResources(MhwVdboxAvpPipeBufAddrParams& pipeBufAddrParams, uint32_t refSize);

        //Interfaces
        Av1PipelineG12_Base        *m_av1Pipeline     = nullptr;
        MhwVdboxAvpInterface       *m_avpInterface    = nullptr;
        Av1BasicFeatureG12         *m_av1BasicFeature = nullptr;
        DecodeAllocator            *m_allocator       = nullptr;
        DecodeMemComp              *m_mmcState        = nullptr;
        CodechalHwInterface        *m_hwInterface     = nullptr;
        MhwMiInterface             *m_miInterface     = nullptr;

        CodecAv1PicParams          *m_av1PicParams    = nullptr; //!< Pointer to picture parameter
        MOS_SURFACE                 refSurface[av1TotalRefsPerFrame];

        //!
        //! \brief    Setup SkipModeFrame[0] and SkipModeFrame[1] per av1_setup_skip_mode_allowed of ref decoder
        //! \return   MOS_STATUS
        //!           MOS_STATUS_SUCCESS if success, else fail reason
        //!
        MOS_STATUS SetupSkipModeFrames(MhwVdboxAvpPicStateParams& picStateParams);

        //!
        //! \brief    Setup Frame Sign Bias
        //! \return   MOS_STATUS
        //!           MOS_STATUS_SUCCESS if success, else fail reason
        //!
        MOS_STATUS SetupFrameSignBias(MhwVdboxAvpPicStateParams& picStateParams);

        PMOS_SURFACE                m_intrabcDecodedOutputFrameBuffer                       = nullptr;    //!< IntraBC Decoded output frame buffer

        //non-temporal buffers
        PMOS_BUFFER                 m_bitstreamDecoderEncoderLineRowstoreReadWriteBuffer     = nullptr;    //!< Handle of Bitstream Decode Line Rowstore buffer, can be programmed to use Local Media Storage VMM instead of Memory 
        PMOS_BUFFER                 m_bitstreamDecoderEncoderTileLineRowstoreReadWriteBuffer = nullptr;    //!< Handle of Bitstream Decode Tile Line buffer
        PMOS_BUFFER                 m_intraPredictionLineRowstoreReadWriteBuffer             = nullptr;    //!< Handle of Intra Prediction Line Rowstore Read/Write Buffer
        PMOS_BUFFER                 m_intraPredictionTileLineRowstoreReadWriteBuffer         = nullptr;    //!< Handle of Intra Prediction Tile Line Rowstore Read/Write Buffer 
        PMOS_BUFFER                 m_spatialMotionVectorLineReadWriteBuffer                 = nullptr;    //!< Handle of Spatial Motion Vector Line rowstore buffer, can be programmed to use Local Media Storage VMM instead of Memory
        PMOS_BUFFER                 m_spatialMotionVectorCodingTileLineReadWriteBuffer       = nullptr;    //!< Handle of Spatial Motion Vector Tile Line buffer
        PMOS_BUFFER                 m_loopRestorationMetaTileColumnReadWriteBuffer           = nullptr;    //!< DW80..81, Loop Restoration Meta Tile Column Read/Write Buffer Address
        PMOS_BUFFER                 m_loopRestorationFilterTileReadWriteLineYBuffer          = nullptr;    //!< DW83..84, Loop Restoration Filter Tile Read/Write Line Y Buffer Address
        PMOS_BUFFER                 m_loopRestorationFilterTileReadWriteLineUBuffer          = nullptr;    //!< DW86..87, Loop Restoration Filter Tile Read/Write Line U Buffer Address
        PMOS_BUFFER                 m_loopRestorationFilterTileReadWriteLineVBuffer          = nullptr;    //!< DW89..90, Loop Restoration Filter Tile Read/Write Line V Buffer Address
        PMOS_BUFFER                 m_deblockerFilterLineReadWriteYBuffer                    = nullptr;    //!< DW92..93, Deblocker Filter Line Read/Write Y Buffer Address
        PMOS_BUFFER                 m_deblockerFilterLineReadWriteUBuffer                    = nullptr;    //!< DW95..96, Deblocker Filter Line Read/Write U Buffer Address
        PMOS_BUFFER                 m_deblockerFilterLineReadWriteVBuffer                    = nullptr;    //!< DW98..99, Deblocker Filter Line Read/Write V Buffer Address
        PMOS_BUFFER                 m_deblockerFilterTileLineReadWriteYBuffer                = nullptr;    //!< DW101..102, Deblocker Filter Tile Line Read/Write Y Buffer Address
        PMOS_BUFFER                 m_deblockerFilterTileLineReadWriteVBuffer                = nullptr;    //!< DW104..105, Deblocker Filter Tile Line Read/Write V Buffer Address
        PMOS_BUFFER                 m_deblockerFilterTileLineReadWriteUBuffer                = nullptr;    //!< DW107..108, Deblocker Filter Tile Line Read/Write U Buffer Address
        PMOS_BUFFER                 m_deblockerFilterTileColumnReadWriteYBuffer              = nullptr;    //!< DW110..111, Deblocker Filter Tile Column Read/Write Y Buffer Address
        PMOS_BUFFER                 m_deblockerFilterTileColumnReadWriteUBuffer              = nullptr;    //!< DW113..114, Deblocker Filter Tile Column Read/Write U Buffer Address
        PMOS_BUFFER                 m_deblockerFilterTileColumnReadWriteVBuffer              = nullptr;    //!< DW116..117, Deblocker Filter Tile Column Read/Write V Buffer Address
        PMOS_BUFFER                 m_cdefFilterLineReadWriteBuffer                          = nullptr;    //!< DW119..120, CDEF Filter Line Read/Write Y Buffer Address
        PMOS_BUFFER                 m_cdefFilterTileLineReadWriteBuffer                      = nullptr;    //!< DW128..129, CDEF Filter Tile Line Read/Write Y Buffer Address
        PMOS_BUFFER                 m_cdefFilterTileColumnReadWriteBuffer                    = nullptr;    //!< DW137..138, CDEF Filter Tile Column Read/Write Y Buffer Address
        PMOS_BUFFER                 m_cdefFilterMetaTileLineReadWriteBuffer                  = nullptr;    //!< DW140..141, CDEF Filter Meta Tile Line Read/Write Buffer Address
        PMOS_BUFFER                 m_cdefFilterMetaTileColumnReadWriteBuffer                = nullptr;    //!< DW143..144, CDEF Filter Meta Tile Column Read/Write Buffer Address    
        PMOS_BUFFER                 m_cdefFilterTopLeftCornerReadWriteBuffer                 = nullptr;    //!< DW146..147, CDEF Filter Top-Left Corner Read/Write Buffer Address
        PMOS_BUFFER                 m_superResTileColumnReadWriteYBuffer                     = nullptr;    //!< DW149..150, Super-Res Tile Column Read/Write Y Buffer Address
        PMOS_BUFFER                 m_superResTileColumnReadWriteUBuffer                     = nullptr;    //!< DW152..153, Super-Res Tile Column Read/Write U Buffer Address
        PMOS_BUFFER                 m_superResTileColumnReadWriteVBuffer                     = nullptr;    //!< DW155..156, Super-Res Tile Column Read/Write V Buffer Address
        PMOS_BUFFER                 m_loopRestorationFilterTileColumnReadWriteYBuffer        = nullptr;    //!< DW158..159, Loop Restoration Filter Tile Column Read/Write Y Buffer Address
        PMOS_BUFFER                 m_loopRestorationFilterTileColumnReadWriteUBuffer        = nullptr;    //!< DW161..162, Loop Restoration Filter Tile Column Read/Write U Buffer Address
        PMOS_BUFFER                 m_loopRestorationFilterTileColumnReadWriteVBuffer        = nullptr;    //!< DW164..165, Loop Restoration Filter Tile Column Read/Write V Buffer Address

        //streamout buffer
        PMOS_BUFFER                 m_decodedFrameStatusErrorBuffer                          = nullptr;    //!< DW176..177, Decoded Frame Status/Error Buffer Base Address
        PMOS_BUFFER                 m_decodedBlockDataStreamoutBuffer                        = nullptr;    //!< DW179..180, Decoded Block Data Streamout Buffer Address

        PMOS_BUFFER                 m_resMfdDeblockingFilterRowStoreScratchBuffer            = nullptr;    //!< Handle of MFD Deblocking Filter Row Store Scratch data surface
        PMOS_BUFFER                 m_resDeblockingFilterTileRowStoreScratchBuffer           = nullptr;    //!< Handle of Deblocking Filter Tile Row Store Scratch data surface
        PMOS_BUFFER                 m_resDeblockingFilterColumnRowStoreScratchBuffer         = nullptr;    //!< Handle of Deblocking Filter Column Row Store Scratch data surface
        PMOS_BUFFER                 m_resMetadataLineBuffer                                  = nullptr;    //!< Handle of Metadata Line data buffer
        PMOS_BUFFER                 m_resMetadataTileLineBuffer                              = nullptr;    //!< Handle of Metadata Tile Line data buffer
        PMOS_BUFFER                 m_resMetadataTileColumnBuffer                            = nullptr;    //!< Handle of Metadata Tile Column data buffer
        PMOS_BUFFER                 m_resSaoLineBuffer                                       = nullptr;    //!< Handle of SAO Line data buffer
        PMOS_BUFFER                 m_resSaoTileLineBuffer                                   = nullptr;    //!< Handle of SAO Tile Line data buffer
        PMOS_BUFFER                 m_resSaoTileColumnBuffer                                 = nullptr;    //!< Handle of SAO Tile Column data buffer

        //buffers for dummy workload
        PMOS_BUFFER                 m_curMvBufferForDummyWL                                   = nullptr;
        PMOS_BUFFER                 m_bwdAdaptCdfBufForDummyWL                                = nullptr;
        PMOS_BUFFER                 m_resDataBufferForDummyWL                                 = nullptr;
        bool                        m_dummyBsBufInited                                        = false;

        uint32_t m_prevFrmWidth         = 0;    //!< Frame width of the previous frame
        uint32_t m_prevFrmHeight        = 0;    //!< Frame height of the previous frame
        uint32_t m_pictureStatesSize    = 0;    //!< Picture states size
        uint32_t m_picturePatchListSize = 0;    //!< Picture patch list size
        uint16_t chromaSamplingFormat   = 0;    //!< Chroma sampling fromat
        uint32_t m_widthInSb            = 0;    //!< Width in unit of SB
        uint32_t m_heightInSb           = 0;    //!< Height in unit of SB
    MEDIA_CLASS_DEFINE_END(decode__Av1DecodePicPkt_G12_Base)
    };

}  // namespace decode
#endif
