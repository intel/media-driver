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
//! \file     decode_hevc_picture_packet_xe_m_base.h
//! \brief    Defines the implementation of hevc decode picture packet
//!

#ifndef __DECODE_HEVC_PICTURE_PACKET_XE_M_BASE_H__
#define __DECODE_HEVC_PICTURE_PACKET_XE_M_BASE_H__

#include "media_cmd_packet.h"
#include "decode_hevc_pipeline.h"
#include "decode_utils.h"
#include "decode_hevc_basic_feature.h"
#include "decode_downsampling_packet.h"
#include "codechal_hw.h"

namespace decode
{

class HevcDecodePicPktXe_M_Base : public DecodeSubPacket
{
public:
    HevcDecodePicPktXe_M_Base(HevcPipeline *pipeline, CodechalHwInterface *hwInterface)
        : DecodeSubPacket(pipeline, *hwInterface), m_hevcPipeline(pipeline)
    {
        m_hwInterface = hwInterface;
        if (m_hwInterface != nullptr)
        {
            m_miInterface  = m_hwInterface->GetMiInterface();
            m_osInterface  = m_hwInterface->GetOsInterface();
            m_hcpInterface  = hwInterface->GetHcpInterface();
        }
    }
    virtual ~HevcDecodePicPktXe_M_Base();

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
    //! \brief  Execute hevc picture packet
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Execute(MOS_COMMAND_BUFFER& cmdBuffer) = 0;

protected:

    virtual MOS_STATUS AllocateFixedResources();
    virtual MOS_STATUS AllocateVariableResources();

    virtual MOS_STATUS SetHcpDstSurfaceParams(MHW_VDBOX_SURFACE_PARAMS &dstSurfaceParams);
    virtual MOS_STATUS SetHcpRefSurfaceParams(
        const MHW_VDBOX_PIPE_BUF_ADDR_PARAMS &pipeBufAddrParams, MHW_VDBOX_SURFACE_PARAMS &refSurfaceParams);
    virtual MOS_STATUS AddHcpSurfaces(MOS_COMMAND_BUFFER &cmdBuffer, const MHW_VDBOX_PIPE_BUF_ADDR_PARAMS &pipeBufAddrParams);

    MOS_STATUS         FixHcpPipeBufAddrParams(MHW_VDBOX_PIPE_BUF_ADDR_PARAMS &pipeBufAddrParams);
    virtual MOS_STATUS SetHcpPipeBufAddrParams(MHW_VDBOX_PIPE_BUF_ADDR_PARAMS &pipeBufAddrParams);

    virtual MOS_STATUS AddHcpIndObjBaseAddrCmd(MOS_COMMAND_BUFFER &cmdBuffer);
    virtual void       SetHcpIndObjBaseAddrParams(MHW_VDBOX_IND_OBJ_BASE_ADDR_PARAMS &indObjBaseAddrParams);

    virtual MOS_STATUS AddHcpQmStateCmd(MOS_COMMAND_BUFFER &cmdBuffer);
    virtual void       SetHcpQmStateParams(MHW_VDBOX_QM_PARAMS &qmParams);

    virtual void       SetHcpPipeModeSelectParams(MHW_VDBOX_PIPE_MODE_SELECT_PARAMS &vdboxPipeModeSelectParams);
    virtual MOS_STATUS AddHcpPipeModeSelectCmd(MOS_COMMAND_BUFFER &cmdBuffer) = 0;

    virtual void       SetHcpPicStateParams(MHW_VDBOX_HEVC_PIC_STATE &picStateParams);

    virtual void       SetHcpTileStateParams(MHW_VDBOX_HEVC_TILE_STATE& tileStateParams);
    virtual MOS_STATUS AddHcpTileStateCmd(MOS_COMMAND_BUFFER &cmdBuffer);

    //! \brief    Set Rowstore Cache offset
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetRowstoreCachingOffsets();

    virtual bool       IsRealTilePhase();
    virtual bool       IsFrontEndPhase();
    virtual bool       IsBackEndPhase();

    //!
    //! \brief  Free resources
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS FreeResources();

#if USE_CODECHAL_DEBUG_TOOL
    MOS_STATUS DumpResources(
        MHW_VDBOX_PIPE_BUF_ADDR_PARAMS &pipeBufAddrParams,
        uint8_t                         activeRefListSize,
        uint32_t                        mvBufferSize);
#endif

    static constexpr uint32_t sliceStateCachelinesPerSlice = 9;

    HevcPipeline          *m_hevcPipeline     = nullptr;
    MhwVdboxHcpInterface  *m_hcpInterface     = nullptr;
    HevcBasicFeature      *m_hevcBasicFeature = nullptr;
    DecodeAllocator       *m_allocator        = nullptr;

#ifdef _MMC_SUPPORTED
    DecodeMemComp         *m_mmcState         = nullptr;
#endif

#ifdef _DECODE_PROCESSING_SUPPORTED
    DecodeDownSamplingFeature *m_downSamplingFeature = nullptr;
    DecodeDownSamplingPkt     *m_downSamplingPkt     = nullptr;
#endif

    DecodePhase           *m_phase            = nullptr;

    // Parameters passed from application
    PCODEC_HEVC_PIC_PARAMS          m_hevcPicParams      = nullptr; //!< Pointer to picture parameter
    PCODECHAL_HEVC_IQ_MATRIX_PARAMS m_hevcIqMatrixParams = nullptr; //!< Pointer to IQ matrix parameter
    PCODEC_HEVC_EXT_PIC_PARAMS      m_hevcRextPicParams  = nullptr; //!< Extended pic params for Rext
    PCODEC_HEVC_SCC_PIC_PARAMS      m_hevcSccPicParams   = nullptr; //!< Pic params for SCC

    CodechalHwInterface *m_hwInterface = nullptr;
    MhwMiInterface      *m_miInterface = nullptr;

    PMOS_BUFFER m_resMfdDeblockingFilterRowStoreScratchBuffer    = nullptr; //!< Handle of MFD Deblocking Filter Row Store Scratch data surface
    PMOS_BUFFER m_resDeblockingFilterTileRowStoreScratchBuffer   = nullptr; //!< Handle of Deblocking Filter Tile Row Store Scratch data surface
    PMOS_BUFFER m_resDeblockingFilterColumnRowStoreScratchBuffer = nullptr; //!< Handle of Deblocking Filter Column Row Store Scratch data surface
    PMOS_BUFFER m_resMetadataLineBuffer                          = nullptr; //!< Handle of Metadata Line data buffer
    PMOS_BUFFER m_resMetadataTileLineBuffer                      = nullptr; //!< Handle of Metadata Tile Line data buffer
    PMOS_BUFFER m_resMetadataTileColumnBuffer                    = nullptr; //!< Handle of Metadata Tile Column data buffer
    PMOS_BUFFER m_resSaoLineBuffer                               = nullptr; //!< Handle of SAO Line data buffer
    PMOS_BUFFER m_resSaoTileLineBuffer                           = nullptr; //!< Handle of SAO Tile Line data buffer
    PMOS_BUFFER m_resSaoTileColumnBuffer                         = nullptr; //!< Handle of SAO Tile Column data buffer

    PMOS_BUFFER m_resSliceStateStreamOutBuffer                   = nullptr; //!< Handle of slice state stream out buffer
    PMOS_BUFFER m_resMvUpRightColStoreBuffer                     = nullptr; //!< Handle of MV up right column store buffer
    PMOS_BUFFER m_resIntraPredUpRightColStoreBuffer              = nullptr; //!< Handle of intra prediction up right column store buffer
    PMOS_BUFFER m_resIntraPredLeftReconColStoreBuffer            = nullptr; //!< Handle of intra prediction left recon column store buffer
    PMOS_BUFFER m_resCABACSyntaxStreamOutBuffer                  = nullptr; //!< Handle of CABAC syntax stream out buffer
    PMOS_BUFFER m_resCABACStreamOutSizeBuffer                    = nullptr; //!< Handle of CABAC stream out size buffer
MEDIA_CLASS_DEFINE_END(decode__HevcDecodePicPktXe_M_Base)
};

}  // namespace decode
#endif
