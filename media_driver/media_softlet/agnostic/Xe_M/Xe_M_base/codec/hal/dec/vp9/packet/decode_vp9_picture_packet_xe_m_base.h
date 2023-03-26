/*
* Copyright (c) 2020-2021, Intel Corporation
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
//! \file     decode_vp9_picture_packet_xe_m_base.h
//! \brief    Defines the implementation of vp9 decode picture packet
//!

#ifndef __DECODE_VP9_PICTURE_PACKET_XE_M_BASE_H__
#define __DECODE_VP9_PICTURE_PACKET_XE_M_BASE_H__

#include "media_cmd_packet.h"
#include "decode_vp9_pipeline.h"
#include "decode_utils.h"
#include "decode_vp9_basic_feature.h"
#include "decode_downsampling_packet.h"
#include "codechal_hw_g12_X.h"

namespace decode
{
    class Vp9DecodePicPktXe_M_Base : public DecodeSubPacket
    {
    public:
        //!
        //! \brief  Vp9DecodePicPktXe_M_Base constructor
        //!
        Vp9DecodePicPktXe_M_Base(Vp9Pipeline *pipeline, CodechalHwInterface *hwInterface)
            : DecodeSubPacket(pipeline, *hwInterface), m_vp9Pipeline(pipeline)
        {
            m_hwInterface = hwInterface;
            if (m_hwInterface != nullptr)
            {
                m_miInterface  = m_hwInterface->GetMiInterface();
                m_osInterface  = m_hwInterface->GetOsInterface();
                m_hcpInterface = static_cast<CodechalHwInterfaceG12 *>(hwInterface)->GetHcpInterface();
            }
        }

        //!
        //! \brief  Vp9DecodePicPktXe_M_Base deconstructor
        //!
        virtual ~Vp9DecodePicPktXe_M_Base();

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

        //! \brief   Set current phase for packet
        //!
        //! \return   MOS_STATUS
        //!           MOS_STATUS_SUCCESS if success, else fail reason
        //!
        MOS_STATUS SetPhase(DecodePhase *phase);
        
        //! \brief   Store cabac stream out size to memory
        //!
        //! \return   MOS_STATUS
        //!           MOS_STATUS_SUCCESS if success, else fail reason
        //!
        MOS_STATUS ReportCabacStreamOutSize(MOS_COMMAND_BUFFER &cmdBuffer);

        //!
        //! \brief  Execute vp9 picture packet
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
            // MPEG2/VC1 reference address indexes
            CodechalDecodeFwdRefTop    = 0,  //!< forward reference top field
            CodechalDecodeBwdRefTop    = 1,  //!< backward reference top field
            CodechalDecodeFwdRefBottom = 2,  //!< forward reference bottom field
            CodechalDecodeBwdRefBottom = 3,  //!< backward reference bottom field
            // VP8/VP9 reference address indexes
            CodechalDecodeLastRef      = 0,  //!< last reference
            CodechalDecodeGoldenRef    = 1,  //!< golden reference
            CodechalDecodeAlternateRef = 2   //!< alternate reference
        };
        CodechalHwInterface *m_hwInterface = nullptr;
        MhwMiInterface      *m_miInterface = nullptr;
    protected:

        virtual MOS_STATUS AllocateFixedResources();
        virtual MOS_STATUS AllocateVariableResources();

        virtual MOS_STATUS SetHcpDstSurfaceParams(MHW_VDBOX_SURFACE_PARAMS &dstSurfaceParams);
        virtual MOS_STATUS SetHcpRefSurfaceParams(MHW_VDBOX_SURFACE_PARAMS refSurfaceParams[]);
        virtual MOS_STATUS AddHcpSurfacesCmd(MOS_COMMAND_BUFFER &cmdBuffer);

        virtual MOS_STATUS SetHcpSegmentStateParams(MHW_VDBOX_VP9_SEGMENT_STATE &segStateParams);
        virtual MOS_STATUS AddHcpSegmentStateCmd(MOS_COMMAND_BUFFER &cmdBuffer);

        MOS_STATUS         FixHcpPipeBufAddrParams(MHW_VDBOX_PIPE_BUF_ADDR_PARAMS &pipeBufAddrParams);
        virtual MOS_STATUS SetHcpPipeBufAddrParams(MHW_VDBOX_PIPE_BUF_ADDR_PARAMS &pipeBufAddrParams);
        virtual MOS_STATUS AddHcpPipeBufAddrCmd(MOS_COMMAND_BUFFER &cmdBuffer) = 0;

        virtual MOS_STATUS AddHcpIndObjBaseAddrCmd(MOS_COMMAND_BUFFER &cmdBuffer);
        virtual void       SetHcpIndObjBaseAddrParams(MHW_VDBOX_IND_OBJ_BASE_ADDR_PARAMS &indObjBaseAddrParams);

        virtual void       SetHcpPipeModeSelectParams(MHW_VDBOX_PIPE_MODE_SELECT_PARAMS &vdboxPipeModeSelectParams);
        virtual MOS_STATUS AddHcpPipeModeSelectCmd(MOS_COMMAND_BUFFER &cmdBuffer) = 0;

        virtual MOS_STATUS SetHcpPicStateParams(MHW_VDBOX_VP9_PIC_STATE &picStateParams);

        virtual MOS_STATUS SetHcpBsdObjectParams(MHW_VDBOX_HCP_BSD_PARAMS &bsdParams);
        virtual MOS_STATUS AddHcpBsdObjectCmd(MOS_COMMAND_BUFFER &cmdBuffer);



        //! \brief    Set Rowstore Cache offset
        //!
        //! \return   MOS_STATUS
        //!           MOS_STATUS_SUCCESS if success, else fail reason
        //!
        virtual MOS_STATUS SetRowstoreCachingOffsets();

        virtual bool       IsFrontEndPhase();
        virtual bool       IsBackEndPhase();

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
        MOS_STATUS DumpResources(MHW_VDBOX_PIPE_BUF_ADDR_PARAMS &pipeBufAddrParams, uint32_t refSize, uint32_t mvSize);

        DecodePhase                *m_phase           = nullptr;

        //Interfaces
        Vp9Pipeline                *m_vp9Pipeline     = nullptr;
        MhwVdboxHcpInterface       *m_hcpInterface    = nullptr;
        Vp9BasicFeature            *m_vp9BasicFeature = nullptr;
        DecodeAllocator            *m_allocator       = nullptr;
        DecodeMemComp *             m_mmcState        = nullptr;

#ifdef _DECODE_PROCESSING_SUPPORTED
        DecodeDownSamplingFeature *m_downSamplingFeature = nullptr;
        DecodeDownSamplingPkt *    m_downSamplingPkt     = nullptr;
#endif

        CODEC_VP9_PIC_PARAMS       *m_vp9PicParams = nullptr;  //!< Pointer to picture parameter
  
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

        uint32_t m_pictureStatesSize    = 0;    //!< Picture states size
        uint32_t m_picturePatchListSize = 0;    //!< Picture patch list size
        uint16_t chromaSamplingFormat   = 0;    //!< Chroma sampling fromat
        uint32_t m_widthInSb            = 0;    //!< Width in unit of SB
        uint32_t m_heightInSb           = 0;    //!< Height in unit of SB

      
        PMOS_BUFFER m_resDmemBuffer = nullptr;
        PMOS_BUFFER m_resDeblockingFilterLineRowStoreScratchBuffer = nullptr;
        PMOS_BUFFER m_resHvcLineRowstoreBuffer                     = nullptr;
        PMOS_BUFFER m_resHvcTileRowstoreBuffer                     = nullptr;
        PMOS_BUFFER m_resIntraPredUpRightColStoreBuffer            = nullptr;  //!< Handle of intra prediction up right column store buffer
        PMOS_BUFFER m_resIntraPredLeftReconColStoreBuffer          = nullptr;  //!< Handle of intra prediction left recon column store buffer
        PMOS_BUFFER m_resCABACSyntaxStreamOutBuffer                = nullptr;  //!< Handle of CABAC syntax stream out buffer
        PMOS_BUFFER m_resCABACStreamOutSizeBuffer                  = nullptr;  //!< Handle of CABAC stream out size buffer

MEDIA_CLASS_DEFINE_END(decode__Vp9DecodePicPktXe_M_Base)
    };

}  // namespace decode
#endif
