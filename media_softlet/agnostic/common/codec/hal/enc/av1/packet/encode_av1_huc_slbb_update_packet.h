/*
* Copyright (c) 2026, Intel Corporation
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
//! \file     encode_av1_huc_slbb_update_packet.h
//! \brief    Defines the implementation of AV1 HuC SLBB Update packet
//!

#ifndef __ENCODE_AV1_HUC_SLBB_UPDATE_PACKET_H__
#define __ENCODE_AV1_HUC_SLBB_UPDATE_PACKET_H__

#include "media_cmd_packet.h"
#include "encode_huc.h"
#include "media_pipeline.h"
#include "encode_utils.h"
#include "encode_av1_vdenc_pipeline.h"
#include "encode_av1_basic_feature.h"
#include "encode_huc_slbb_update_pkt.h"
#include "mhw_vdbox_avp_itf.h"
#include "codec_def_common_encode.h"
#include "encode_av1_brc.h"

namespace encode
{

class AV1HucSLBBUpdatePkt : public HucSLBBUpdatePkt
{
public:
    //!
    //! \brief  Constructor
    //!
    AV1HucSLBBUpdatePkt(MediaPipeline *pipeline, MediaTask *task, CodechalHwInterfaceNext *hwInterface);

    //!
    //! \brief  Destructor
    //!
    virtual ~AV1HucSLBBUpdatePkt();

    //!
    //! \brief  Initialize the AV1HucSLBBUpdatePkt
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Init() override;

    //!
    //! \brief  Submit the AV1 HuC SLBB Update packet
    //! \param  [in] commandBuffer
    //!         Pointer to command buffer
    //! \param  [in] packetPhase
    //!         Packet phase
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS Submit(MOS_COMMAND_BUFFER *commandBuffer, uint8_t packetPhase = otherPacket) override;

    //!
    //! \brief  Calculate Command Size
    //! \param  [in, out] commandBufferSize
    //!         requested size
    //! \param  [in, out] requestedPatchListSize
    //!         requested patch list size
    //! \return MOS_STATUS
    //!         status
    //!
    virtual MOS_STATUS CalculateCommandSize(uint32_t &commandBufferSize, uint32_t &requestedPatchListSize) override;

protected:
    //!
    //! \brief  Indicates whether the runtime is an extended platform whose
    //!         driver pre-fills the VDENC_CMD2 conflict-surface fields
    //!         (vendor-specific DW63 / DW64 bits) via L2/L5 lambda. Default = false
    //!         (base platforms — HuC owns these fields via TU tables). The
    //!         extended platform subclass overrides to return true so the HuC
    //!         kernel skips those writes and the driver pre-fills survive
    //!         into the SLBB.
    //!
    virtual bool IsExtendedPlatform() const { return false; }

    //!
    //! \brief  Allocate Resources
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AllocateResources() override;

    //!
    //! \brief  Construct batch buffer for AV1-specific SLBB updates
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS ConstructBatchBuffer() override;

    //!
    //! \brief  Set constant data for HuC AV1 SLBB Update
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetConstDataHuCSlbbUpdate();

    //!
    //! \brief  Helper functions for SLBB construction
    //!

    //!
    //! \brief  Add batch buffer end command
    //! \param  [in,out] cmdBuffer
    //!         Command buffer to add the end command to
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS AddBBEnd(MOS_COMMAND_BUFFER& cmdBuffer);

    //!
    //! \brief  Add all AVP segment state commands
    //! \param  [in,out] cmdBuffer
    //!         Command buffer to add commands to
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS AddAllCmds_AVP_SEGMENT_STATE(PMOS_COMMAND_BUFFER cmdBuffer) const;

    //!
    //! \brief  Add AVP picture state commands based on tile configuration
    //! \param  [in,out] cmdBuffer
    //!         Command buffer to add commands to
    //! \param  [in,out] slbData
    //!         SLBB data structure to track offsets
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS AddAvpPicStateBaseOnTile(MOS_COMMAND_BUFFER& cmdBuffer, SlbData &slbData);

    //!
    //! \brief  Add VDENC tile/slice state commands
    //! \param  [in,out] cmdBuffer
    //!         Command buffer to add commands to
    //! \param  [in,out] slbData
    //!         SLBB data structure to track offsets
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS AddVdencTileSliceBaseOnTile(MOS_COMMAND_BUFFER& cmdBuffer, SlbData& slbData);

#if USE_CODECHAL_DEBUG_TOOL
    virtual MOS_STATUS DumpInput() override;
    virtual MOS_STATUS DumpOutput() override;
#endif

    //!
    //! \brief  Set HUC_IMEM_STATE command parameters
    //! \param  [in] params
    //!         Pointer to HUC_IMEM_STATE parameters
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MHW_SETPAR_DECL_HDR(HUC_IMEM_STATE);

    //!
    //! \brief  Set HUC_VIRTUAL_ADDR_STATE command parameters
    //! \param  [in] params
    //!         Pointer to HUC_VIRTUAL_ADDR_STATE parameters
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MHW_SETPAR_DECL_HDR(HUC_VIRTUAL_ADDR_STATE);

    //!
    //! \brief  Set VD_PIPELINE_FLUSH command parameters for AV1
    //! \param  [in] params
    //!         Pointer to VD_PIPELINE_FLUSH parameters
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MHW_SETPAR_DECL_HDR(VD_PIPELINE_FLUSH);

    MHW_SETPAR_DECL_HDR(AVP_PIC_STATE);

    MHW_SETPAR_DECL_HDR(VDENC_HEVC_VP9_TILE_SLICE_STATE);

    //!
    //! \brief  Set DMEM buffer for HuC AV1 SLBB Update
    //! \details Populates m_slbbUpdateDmemBuffer with SLBB offsets from Av1Brc 
    //!          and encoding parameters from Av1BasicFeature
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetDmem() const override;

    //!
    //! \brief  HuC AV1 SLBB Update DMEM structure (64 bytes, DWORD-aligned)
    //!
    struct HucAv1SlbbUpdateDmem
    {
        // Section 1: SLBB Size (2 bytes)
        uint16_t SlbbSize;
        
        // Section 2: Command Offsets (14 bytes, 7 uint16_t fields)
        uint16_t AvpSegmentStateOffset;
        uint16_t AvpInloopFilterStateOffset;
        uint16_t VdencCmd1Offset;
        uint16_t VdencCmd2Offset;
        uint16_t AvpPicStateOffset;
        uint16_t AvpPicStateMultiTileOffset;
        uint16_t VdencTileSliceStateOffset;
        
        // Section 3: Command Metadata (4 bytes, 2 uint16_t fields)
        uint16_t AVPPiCStateCmdNum;
        uint16_t TileNum;
        
        // Section 4: Reserved Fields (4 bytes, 2 uint16_t)
        uint16_t Reserved[2];
        
        // Section 5: Encoding Parameters (8 bytes, 8 uint8_t fields)
        uint8_t FrameType;
        uint8_t QpValue;
        uint8_t isLowDelay;
        uint8_t TargetUsage;
        uint8_t RdoEnable;
        uint8_t numRefL0;
        uint8_t numRefL1;
        uint8_t IBCEnabledForCurrentTile;  // IBC enable: allow_intrabc && (wSB64 > INTRABC_DELAY_SB64+1)

        // Section 6: Extended Encoding Parameters (32 bytes)
        uint8_t IsLossless;                // Lossless frame flag (0: normal, 1: lossless)
        uint8_t ChromaFormat;              // Chroma format (0: YUV420, 1: YUV422, 2: YUV444)
        uint8_t EnableIntraEdgeFilter;     // Intra edge filter enable from seq CodingToolFlags
        uint8_t Par65Inter;
        uint8_t Par65Intra;
        uint8_t EnablePalette;             // Palette mode enable flag (0: disabled, 1: enabled)
        uint8_t EnableIBC;                 // IBC mode enable flag from PicFlags.allow_intrabc
        uint8_t Wa_15014143531;            // Wa_15014143531: force disable IBC
        uint8_t Wa_15017726119;
        uint8_t Wa_16025947269;            // Wa_16025947269: gate the protected VDENC_CMD2 DW52 bit
                                           // (TU1/TU2 only). 1 = WA active, HuC keeps TU-table value;
                                           // 0 = HW fix in place, HuC forces field to 0.

        uint8_t AdaptiveTUEnabled;         // AdaptiveTU mode flag: 0=disabled, 1=enabled (process TU7 SLBB via Region 2->3)
        uint8_t ExtendedPlatform;     // 0 = HuC writes DW54/DW63/DW64 conflict-surface fields (base platform default).
                                           // 1 = HuC preserves driver pre-fills (extended platform — driver lambdas L2/L4/L5 own them).
        uint8_t Reserved6[20];             // Reserved for future use, must be zero-initialized
    };

    //!
    //! \brief  AV1 HuC SLBB Update kernel descriptor
    //!
    static constexpr uint32_t m_vdboxHucAv1SlbbUpdateKernelDescriptor = 21;

    //!
    //! \brief  Pointer to AV1 basic feature
    //!
    Av1BasicFeature *m_basicFeature = nullptr;

    //!
    //! \brief  Shared pointer to feature manager
    //!
    std::shared_ptr<MediaFeatureManager::ManagerLite> m_featureManager = nullptr;

    //!
    //! \brief  SLBB constant data buffer
    //!
    MOS_RESOURCE m_slbbConstDataBuffer[CODECHAL_ENCODE_RECYCLED_BUFFER_NUM] = {};

    //!
    //! \brief  Size of SLBB constant data buffer
    //!
    uint32_t m_slbbConstDataBufferSize = 2048;

MEDIA_CLASS_DEFINE_END(encode__AV1HucSLBBUpdatePkt)
};

}  // namespace encode

#endif  // !__ENCODE_AV1_HUC_SLBB_UPDATE_PACKET_H__