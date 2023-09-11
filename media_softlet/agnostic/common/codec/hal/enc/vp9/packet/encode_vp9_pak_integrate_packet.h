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
//! \file     encode_vp9_pak_integrate_packet.h
//! \brief    Defines the implementation of vp9 pak integrate packet
//!

#ifndef __CODECHAL_VP9_PAK_INTEGRATE_PACKET_H__
#define __CODECHAL_VP9_PAK_INTEGRATE_PACKET_H__

#include "media_cmd_packet.h"
#include "encode_huc.h"
#include "encode_vp9_basic_feature.h"
#include "encode_vp9_vdenc_packet.h"
#include "encode_vp9_brc.h"
#include "encode_vp9_tile.h"

namespace encode
{
#define HUC_CMD_LIST_MODE     1
#define HUC_BATCH_BUFFER_END  0x05000000
#define PAK_OBJECT_NUM        4

//!
//! \struct HucPakIntDmem
//! \brief  The struct of Huc Com Dmem
//!
struct HucPakIntDmem
{
    uint32_t tileSizeRecordOffset[PAK_OBJECT_NUM + 1];    // Tile Size Records, start offset  in byte, 0xffffffff means unavailable
    uint32_t vdencStatOffset[PAK_OBJECT_NUM + 1];         // Needed for HEVC VDEnc, VP9 VDEnc, start offset  in byte, 0xffffffff means unavailable
    uint32_t hevcPakStatOffset[PAK_OBJECT_NUM + 1];       // Needed for HEVC VDEnc, start offset  in byte, 0xffffffff means unavailable
    uint32_t hevcStreamoutOffset[PAK_OBJECT_NUM + 1];     // Needed for HEVC VDEnc, start offset  in byte, 0xffffffff means unavailable
    uint32_t vp9PakStatOffset[PAK_OBJECT_NUM + 1];        // Needed for VP9 VDEnc, start offset  in byte, 0xffffffff means unavailable
    uint32_t vp9CounterBufferOffset[PAK_OBJECT_NUM + 1];  // Needed for VP9 VDEnc, start offset  in byte, 0xffffffff means unavailable
    uint32_t lastTileBSStartInBytes;                      // Last tile in bitstream for region 4 and region 5
    uint32_t SliceHeaderSizeinBits;                       // Needed for HEVC dual pipe BRC
    uint16_t totalSizeInCommandBuffer;                    // Total size in bytes of valid data in the command buffer
    uint16_t offsetInCommandBuffer;                       // Byte  offset of the to-be-updated Length (uint32_t ) in the command buffer, 0xffff means unavailable
    uint16_t picWidthInPixel;                             // Picture width in pixel
    uint16_t picHeightInPixel;                            // Picture hieght in pixel
    uint16_t totalNumberOfPaks;                           // [2..4] for Gen11
    uint16_t numSlices[PAK_OBJECT_NUM];                   // This is number of slices in each PAK
    uint16_t numTiles[PAK_OBJECT_NUM];                    // This is number of tiles from each PAK
    uint16_t picStateStartInBytes;                        // Offset for  region 7 and region 8
    uint8_t  codec;                                       // 1: HEVC DP; 2: HEVC VDEnc; 3: VP9 VDEnc
    uint8_t  maxPass;                                     // Max number of BRC pass >=1
    uint8_t  currentPass;                                 // Current BRC pass [1..MAXPass]
    uint8_t  minCUSize;                                   // Minimum CU size (3: 8x8, 4:16x16), HEVC only.
    uint8_t  cabacZeroWordFlag;                           // Cabac zero flag, HEVC only
    uint8_t  bitdepthLuma;                                // Luma bitdepth, HEVC only
    uint8_t  bitdepthChroma;                              // Chroma bitdepth, HEVC only
    uint8_t  chromaFormatIdc;                             // Chroma format idc, HEVC only
    uint8_t  currFrameBRClevel;                           // Hevc dual pipe only
    uint8_t  brcUnderFlowEnable;                          // Hevc dual pipe only
    uint8_t  StitchEnable;                                // Enable stitch cmd for Hevc dual pipe
    uint8_t  reserved1;
    uint16_t StitchCommandOffset;                         // Offset in region 10 which is the second level batch buffer
    uint16_t reserved2;
    uint32_t BBEndforStitch;
    uint8_t  RSVD[16];
};

//!
//! \struct HucInputCmdG12
//! \brief  The struct of Huc input command
//!
struct HucInputCmdG12
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

class Vp9PakIntegratePkt : public EncodeHucPkt
{
public:
    //!
    //! \brief  Vp9PakIntegratePkt constructor
    //!
    //! \param  [in] pipeline
    //!         Pointer to the media pipeline
    //! \param  [in] task
    //!         Pointer to media task
    //! \param  [in] hwInterface
    //!         Pointer to HW interface
    //!
    Vp9PakIntegratePkt(MediaPipeline *pipeline, MediaTask *task, CodechalHwInterfaceNext *hwInterface) : EncodeHucPkt(pipeline, task, hwInterface)
    {
        ENCODE_CHK_NULL_NO_STATUS_RETURN(m_hwInterface);
        m_osInterface = m_hwInterface->GetOsInterface();
        ENCODE_CHK_NULL_NO_STATUS_RETURN(m_osInterface);
    }

    //!
    //! \brief  Vp9PakIntegratePkt destructor
    //!
    virtual ~Vp9PakIntegratePkt();

    //!
    //! \brief  Initialize the media packet, allocate required resources
    //!
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Init() override;

    //!
    //! \brief  Add the command sequence into the commandBuffer and
    //!         and return to the caller task
    //!
    //! \param  [in] commandBuffer
    //!         Pointer to the command buffer which is allocated by caller
    //! \param  [in] packetPhase
    //!         Indicate packet phase stage
    //!
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS Submit(MOS_COMMAND_BUFFER *commandBuffer, uint8_t packetPhase = otherPacket) override;

    //!
    //! \brief  Calculate Command Size
    //!
    //! \param  [in, out] commandBufferSize
    //!         requested size
    //! \param  [in, out] requestedPatchListSize
    //!         requested size
    //!
    //! \return MOS_STATUS
    //!         status
    //!
    virtual MOS_STATUS CalculateCommandSize(
        uint32_t &commandBufferSize,
        uint32_t &requestedPatchListSize) override;

    //!
    //! \brief  Dump output resources or infomation after submit
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS DumpOutput() override;

    //!
    //! \brief  Get Packet Name
    //!
    //! \return std::string
    //!
    virtual std::string GetPacketName() override
    {
        return "PAKINT_Pass" + std::to_string((uint32_t)m_pipeline->GetCurrentPass());
    }

    //!
    //! \brief  One frame is completed
    //!
    //! \param  [in] mfxStatus
    //!         pointer to status buffer which for MFX
    //! \param  [in] rcsStatus
    //!         pointer to status buffer which for RCS
    //! \param  [in, out] statusReport
    //!         pointer of EncoderStatusReport
    //!
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Completed(void *mfxStatus, void *rcsStatus, void *statusReport) override;

    MHW_SETPAR_DECL_HDR(HUC_IMEM_STATE);
    MHW_SETPAR_DECL_HDR(HUC_DMEM_STATE);
    MHW_SETPAR_DECL_HDR(HUC_VIRTUAL_ADDR_STATE);

protected:
    //!
    //! \brief  Allocate resources
    //!
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AllocateResources() override;

    //!
    //! \brief  Set huc dmem buffer
    //!
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetDmemBuffer() const;

    //!
    //! \brief    Retreive Hcp statistics
    //!
    //! \param    [in] vdboxIndex
    //!           vdbox's index
    //! \param    [in] statusReport
    //!           Pointer to status report interface
    //! \param    [in] cmdBuffer
    //!           Pointer to command buffer
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS ReadHcpStatus(
        MHW_VDBOX_NODE_IND  vdboxIndex,
        MediaStatusReport * statusReport,
        MOS_COMMAND_BUFFER &cmdBuffer);

    //!
    //! \brief  End Status Report
    //!
    //! \param  [in] srType
    //!         status report type for send cmds
    //! \param  [in, out] cmdBuffer
    //!         cmdbuffer to send cmds
    //!
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS EndStatusReport(
        uint32_t            srType,
        MOS_COMMAND_BUFFER *cmdBuffer) override;

    //!
    //! \brief  Update parameters
    //!
    void UpdateParameters();

    //!
    //! \brief  Setup status data of tiles when one frame is completed
    //!
    //! \param  [in] mfxStatus
    //!         pointer to status buffer which for MFX
    //! \param  [in, out] statusReport
    //!         pointer of EncoderStatusReport
    //!
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetupTilesStatusData(void *mfxStatus, void *statusReport);

    //!
    //! \brief  Free resources
    //!
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS FreeResources();

    //!
    //! \brief  Configure stitich data buffer
    //!
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS ConfigStitchDataBuffer() const;

#if USE_CODECHAL_DEBUG_TOOL
    //!
    //! \brief  Dump input resources or infomation after submit
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS DumpInput() override;
#endif

    static constexpr uint32_t m_vdboxHucPakIntegrationKernelDescriptor = 15;  //!< VDBox Huc PAK integration kernel descriptor

    // VDENC Pak Int related constants
    static constexpr uint32_t m_pakIntDmemOffsetsSize = 120;
    static constexpr uint32_t m_pakIntVp9CodecId      = 3;
    static constexpr uint32_t m_hwStitchCmdSize       = 20 * sizeof(uint32_t);  //!< Cmd size for hw stitch

    MOS_RESOURCE m_hucPakIntDmemBuffer[CODECHAL_ENCODE_RECYCLED_BUFFER_NUM][Vp9EncodeBrc::m_brcMaxNumPasses] = {};  //!< HUC PAK INT DMEM buffers
    MOS_RESOURCE m_hucPakIntDummyBuffer    = {0};                    //!< HuC PAK Integrateion dummy buffer
    uint32_t     m_hucPakIntDmemBufferSize = sizeof(HucPakIntDmem);  //!< Indicate the size of Dmem buffer of Huc pak integrate kernel

    MOS_RESOURCE     m_resHucStitchDataBuffer[CODECHAL_ENCODE_RECYCLED_BUFFER_NUM][CODECHAL_ENCODE_VP9_BRC_MAX_NUM_OF_PASSES] = {};
    MHW_BATCH_BUFFER m_HucStitchCmdBatchBuffer = {};

    PMOS_INTERFACE     m_osInterface = nullptr;  //!< Pointer to the os interface
    EncodeMemComp *    m_mmcState    = nullptr;  //!< Pointer to medai memory compression state

    Vp9BasicFeature *m_basicFeature = nullptr;  //!< Vp9 basic feature used in each frame

    std::shared_ptr<mhw::vdbox::hcp::Itf> m_hcpInterfaceNew = nullptr;

MEDIA_CLASS_DEFINE_END(encode__Vp9PakIntegratePkt)
};
}  // namespace encode

#endif
