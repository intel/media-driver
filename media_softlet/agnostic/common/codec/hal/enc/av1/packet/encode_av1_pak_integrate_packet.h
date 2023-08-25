/*
* Copyright (c) 2023, Intel Corporation
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
//! \file     encode_pak_integrate_packet.h
//! \brief    Defines the implementation of pak integrate packet
//!

#ifndef __CODECHAL_AV1_PAK_INTEGRATE_PACKET_H__
#define __CODECHAL_AV1_PAK_INTEGRATE_PACKET_H__

#include "media_cmd_packet.h"
#include "encode_huc.h"
#include "media_pipeline.h"
#include "codec_hw_next.h"
#include "encode_utils.h"
#include "encode_av1_basic_feature.h"
#include "encode_av1_tile.h"

namespace encode
{
class Av1PakIntegratePkt : public EncodeHucPkt
{
public:
#define MAX_PAK_NUM          8
#define HUC_BATCH_BUFFER_END 0x05000000
#define HUC_CMD_LIST_MODE    1

    //!
    //! \struct HucPakIntegrateDmem
    //! \brief  The struct of Huc Pak integrate Dmem
    //!
    struct HucPakIntegrateDmem
    {
        uint32_t TileSizeRecord_offset[MAX_PAK_NUM + 1];    //!< Tile Size Records, start offset  in byte, 0xffffffff means unavailable
        uint32_t VDENCSTAT_offset[MAX_PAK_NUM + 1];         //!< needed for HEVC VDEnc, VP9 VDEnc, start offset  in byte, 0xffffffff means unavailable
        uint32_t HEVC_PAKSTAT_offset[MAX_PAK_NUM + 1];      //!< needed for HEVC VDEnc, start offset  in byte, 0xffffffff means unavailable
        uint32_t HEVC_Streamout_offset[MAX_PAK_NUM + 1];    //!< needed for HEVC VDEnc, start offset  in byte, 0xffffffff means unavailable
        uint32_t VP9_PAK_STAT_offset[MAX_PAK_NUM + 1];      //!< needed for VP9 VDEnc, start offset  in byte, 0xffffffff means unavailable
        uint32_t Vp9CounterBuffer_offset[MAX_PAK_NUM + 1];  //!< needed for VP9 VDEnc, start offset  in byte, 0xffffffff means unavailable
        uint32_t LastTileBS_StartInBytes;                   //!< last tile in bitstream for region 4 and region 5
        uint32_t SliceHeaderSizeinBits;                     //!< needed for HEVC dual pipe BRC
        uint16_t TotalSizeInCommandBuffer;                  //!< Total size in bytes of valid data in the command buffer
        uint16_t OffsetInCommandBuffer;                     //!< Byte  offset of the to-be-updated Length (uint32_t) in the command buffer, 0xffff means unavailable
        uint16_t PicWidthInPixel;                           //!< Picture width in pixel
        uint16_t PicHeightInPixel;                          //!< Picture hieght in pixel
        uint16_t TotalNumberOfPAKs;                         //!< [2..4]
        uint16_t NumSlices[MAX_PAK_NUM];                    //!< this is number of slices from each PAK
        uint16_t NumTiles[MAX_PAK_NUM];                     //!< this is number of tiles from each PAK
        uint16_t PIC_STATE_StartInBytes;                    //!< offset for  region 7 and region 8
        uint8_t  Codec;                                     //!< 1: HEVC DP; 2: HEVC VDEnc; 3: VP9 VDEnc 4: AV1
        uint8_t  MAXPass;                                   //!< Max number of BRC pass >=1
        uint8_t  CurrentPass;                               //!< Current BRC pass [1..MAXPass]
        uint8_t  MinCUSize;                                 //!< Minimum CU size (3: 8x8, 4:16x16), HEVC only.
        uint8_t  CabacZeroWordFlag;                         //!< cabac zero flag, HEVC only
        uint8_t  bitdepth_luma;                             //!< luma bitdepth, HEVC only
        uint8_t  bitdepth_chroma;                           //!< chroma bitdepth, HEVC only
        uint8_t  ChromaFormatIdc;                           //!< chroma format idc, HEVC only
        uint8_t  currFrameBRClevel;                         //!< Hevc dual pipe only
        uint8_t  brcUnderFlowEnable;                        //!< Hevc dual pipe only
        uint8_t  StitchEnable;                              //!< enable stitch cmd for Hevc dual pipe
        uint8_t  reserved1;                                 //!< reserved field
        uint16_t StitchCommandOffset;                       //!< offset in region 10 which is the second level batch buffer
        uint16_t reserved2;                                 //!< reserved field
        uint32_t BBEndforStitch;                            //!< Batch buffer end for stitch
        uint8_t  RSVD[32];                                  //!< Reserved field for debug perpose
    };

    //!
    //! \struct HucInputCmd
    //! \brief  The struct of Huc input command
    //!
    struct HucInputCmd
    {
        uint8_t  SelectionForIndData = 0;
        uint8_t  CmdMode             = 0;
        uint16_t LengthOfTable       = 0;

        uint32_t SrcBaseOffset  = 0;
        uint32_t DestBaseOffset = 0;

        uint32_t Reserved[3] = {0};

        uint32_t CopySize = 0;

        uint32_t ReservedCounter[4] = {0};

        uint32_t SrcAddrBottom  = 0;
        uint32_t SrcAddrTop     = 0;
        uint32_t DestAddrBottom = 0;
        uint32_t DestAddrTop    = 0;
    };

    //!
    //! \struct HucCommandData
    //! \brief  The struct of Huc commands data
    //!
    struct HucCommandData
    {
        uint32_t TotalCommands;   //!< Total Commands in the Data buffer
        struct
        {
            uint16_t ID;          //!< Command ID, defined and order must be same as that in DMEM
            uint16_t SizeOfData;  //!< data size in uint32_t
            uint32_t data[40];
        } InputCOM[10];
    };

    Av1PakIntegratePkt(MediaPipeline *pipeline, MediaTask *task, CodechalHwInterfaceNext *hwInterface) : EncodeHucPkt(pipeline, task, hwInterface)
    {
        m_hcpItf = std::static_pointer_cast<mhw::vdbox::hcp::Itf>(hwInterface->GetHcpInterfaceNext());
        ENCODE_CHK_NULL_NO_STATUS_RETURN(m_hcpItf);
    }

    virtual ~Av1PakIntegratePkt() { FreeResources(); };

    virtual MOS_STATUS Init() override;

    MOS_STATUS Submit(MOS_COMMAND_BUFFER *commandBuffer, uint8_t packetPhase = otherPacket) override;

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
    virtual MOS_STATUS CalculateCommandSize(
        uint32_t &commandBufferSize,
        uint32_t &requestedPatchListSize) override;

    //!
    //! \brief  Get Packet Name
    //! \return std::string
    //!
    virtual std::string GetPacketName() override
    {
        return "AV1PAKINTEGRATE";
    }

    //!
    //! \brief  One frame is completed
    //! \param  [in] mfxStatus
    //!         pointer to status buffer which for MFX
    //! \param  [in] rcsStatus
    //!         pointer to status buffer which for RCS
    //! \param  [in, out] statusReport
    //!         pointer of EncoderStatusReport
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Completed(void *mfxStatus, void *rcsStatus, void *statusReport) override;

protected:
    MHW_SETPAR_DECL_HDR(HUC_IMEM_STATE);
    MHW_SETPAR_DECL_HDR(HUC_DMEM_STATE);
    MHW_SETPAR_DECL_HDR(HUC_VIRTUAL_ADDR_STATE);
    virtual MOS_STATUS AllocateResources() override;

    virtual MOS_STATUS SetDmemBuffer() const;

    MOS_STATUS ReadSseStatistics(MOS_COMMAND_BUFFER &cmdBuffer);
    MOS_STATUS ReadSliceSize(MOS_COMMAND_BUFFER &cmdBuffer);
    MOS_STATUS ReadSliceSizeForSinglePipe(MOS_COMMAND_BUFFER &cmdBuffer);

    MOS_STATUS EndStatusReport(
        uint32_t            srType,
        MOS_COMMAND_BUFFER *cmdBuffer) override;

    void UpdateParameters();

    //!
    //! \brief  Setup status data of tiles when one frame is completed
    //! \param  [in] mfxStatus
    //!         pointer to status buffer which for MFX
    //! \param  [in, out] statusReport
    //!         pointer of EncoderStatusReport
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetupTilesStatusData(void *mfxStatus, void *statusReport);

    //!
    //! \brief  Perform Software Stitch for bitstream
    //! \param  [in] tileReportData
    //!         pointer of tile report data
    //! \param  [in] tileStatusReport
    //!         pointer of tile status report
    //! \param  [in, out] statusReportData
    //!         pointer to status report data
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS PerformSwStitch(
        const EncodeReportTileData *    tileReportData,
        PakHwTileSizeRecord *           tileStatusReport,
        EncodeStatusReportData *        statusReportData);

    //!
    //! \brief  Perform Hardware Stitch for bitstream
    //! \param  [out] cmdBuffer
    //!         Cmd buffer to add hardware stitch cmds
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS PerformHwStitch(PMOS_COMMAND_BUFFER cmdBuffer);

    //!
    //! \brief  Configure stitich data buffer
    //!
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS ConfigStitchDataBuffer() const;

    //!
    //! \brief  Free resources
    //!
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS FreeResources();

    MOS_STATUS AddCondBBEndFor2ndPass(MOS_COMMAND_BUFFER &cmdBuffer);

#if USE_CODECHAL_DEBUG_TOOL
    virtual MOS_STATUS DumpInput() override;
    virtual MOS_STATUS DumpOutput() override;
#endif
    static constexpr uint32_t m_vdboxHucPakIntKernelDescriptor = 15;  //!< Huc pak integrate kernel descriptor

    EncodeMemComp *m_mmcState = nullptr;

    uint32_t      m_vdencHucPakDmemBufferSize = sizeof(HucPakIntegrateDmem);    //!< Indicate the size of Dmem buffer of Huc pak integrate kernel
    PMOS_RESOURCE m_resHucPakStitchDmemBuffer[CODECHAL_ENCODE_RECYCLED_BUFFER_NUM][CODECHAL_VDENC_BRC_NUM_OF_PASSES] = {};  //!< HuC Pak Integration Dmem data for each pass

    MOS_RESOURCE     m_resHucStitchDataBuffer[CODECHAL_ENCODE_RECYCLED_BUFFER_NUM][CODECHAL_VDENC_BRC_NUM_OF_PASSES] = {};
    MHW_BATCH_BUFFER m_HucStitchCmdBatchBuffer = {};

    static constexpr const uint32_t m_hwStitchCmdSize = 20 * sizeof(uint32_t);  //!< Cmd size for hw stitch
    bool m_vdencHucUsed = false;                 //!< Indicate if it is needed to use Huc pak integrate kernel   
    Av1BasicFeature *m_basicFeature = nullptr;  //!< Hevc Basic Feature used in each frame
    MHW_VDBOX_NODE_IND              m_vdboxIndex      = MHW_VDBOX_NODE_1;      //!< Index of VDBOX

    std::shared_ptr<mhw::vdbox::hcp::Itf> m_hcpItf = nullptr;
    
MEDIA_CLASS_DEFINE_END(encode__Av1PakIntegratePkt)
};

}  // namespace encode
#endif
